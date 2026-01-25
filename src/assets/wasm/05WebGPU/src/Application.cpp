#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>
#include <WebGPU/WgpContext.h>
#include <GL/glew.h>
#include <emscripten.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_wgpu.h>

#include <States/Shape.h>
#include <States/Default.h>

#include "WebGpuUtils.h"
#include "Application.h"
#include "Event.h"

constexpr float PI = 3.14159265358979323846f;

GLFWwindow* Application::Window = nullptr;
StateMachine* Application::Machine = nullptr;
WGPUDevice Application::Device;
WGPUQueue Application::Queue;
WGPUSurface Application::Surface;

int Application::Width;
int Application::Height;
double Application::Time;

void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void glfwMouseMoveCallback(GLFWwindow* window, double xpos, double ypos);

void Application::MessageLopp(void *arg) {
  Application* application  = reinterpret_cast<Application*>(arg);

  Time = glfwGetTime();
  application->dt = float(Time - application->last);
  application->last = Time;

  application->messageLopp();
}

Application::Application(float& dt, float& fdt) : fdt(fdt), dt(dt), last(0.0) {
  Application::Width = 640;
  Application::Height = 480;
  initWindow();
  initWebGPU();
  initGui();
  initStates();

  glfwSetWindowUserPointer(Window, this);
  glfwSetKeyCallback(Window, glfwKeyCallback);
  glfwSetMouseButtonCallback(Window, glfwMouseButtonCallback);
  glfwSetCursorPosCallback(Window, glfwMouseMoveCallback);
}

Application::~Application() {
  wgpuSurfaceUnconfigure(Surface);
  wgpuQueueRelease(Queue);
  wgpuSurfaceRelease(Surface);
  wgpuDeviceRelease(Device);
  glfwDestroyWindow(Window);
  glfwTerminate();
}

void Application::initWindow() {
  if (!glfwInit()) {
    return;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  Window = glfwCreateWindow(Width, Height, "WebGPU window", nullptr, nullptr);
}

void Application::initWebGPU(){
  wgpInit(Window);
  instance = wgpContext.instance;
  adapter = wgpContext.adapter;
  Device = wgpContext.device;
  Surface = wgpContext.surface;
  SwapChain = wgpContext.swapChain;
  Queue = wgpContext.queue;

  
	std::cout << "Creating shader module..." << std::endl;
	WGPUShaderModule shaderModule = loadShaderModule("res/shader/shader.wgsl");
	std::cout << "Shader module: " << shaderModule << std::endl;

	std::cout << "Creating render pipeline..." << std::endl;

// Vertex fetch
	std::vector<WGPUVertexAttribute> vertexAttribs(3);
	//                                         ^ This was a 2

	// Position attribute
	vertexAttribs[0].shaderLocation = 0;
	vertexAttribs[0].format = WGPUVertexFormat ::WGPUVertexFormat_Float32x3;
	vertexAttribs[0].offset = 0;

	// Normal attribute
	vertexAttribs[1].shaderLocation = 1;
	vertexAttribs[1].format = WGPUVertexFormat ::WGPUVertexFormat_Float32x3;
	vertexAttribs[1].offset = offsetof(VertexAttributes, normal);

	// Color attribute
	vertexAttribs[2].shaderLocation = 2;
	vertexAttribs[2].format = WGPUVertexFormat ::WGPUVertexFormat_Float32x3;
	vertexAttribs[2].offset = offsetof(VertexAttributes, color);

	WGPUVertexBufferLayout vertexBufferLayout;
	vertexBufferLayout.attributeCount = (uint32_t)vertexAttribs.size();
	vertexBufferLayout.attributes = vertexAttribs.data();
	vertexBufferLayout.arrayStride = sizeof(VertexAttributes);
	//                               ^^^^^^^^^^^^^^^^^^^^^^^^ This was 6 * sizeof(float)
	vertexBufferLayout.stepMode = WGPUVertexStepMode::WGPUVertexStepMode_Vertex;

	WGPURenderPipelineDescriptor pipelineDesc;
  pipelineDesc.vertex.bufferCount = 1;
	pipelineDesc.vertex.buffers = &vertexBufferLayout;

	pipelineDesc.vertex.module = shaderModule;
	pipelineDesc.vertex.entryPoint = "vs_main";
	pipelineDesc.vertex.constantCount = 0;
	pipelineDesc.vertex.constants = nullptr;

	pipelineDesc.primitive.topology = WGPUPrimitiveTopology::WGPUPrimitiveTopology_TriangleList;
	pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat::WGPUIndexFormat_Undefined;
	pipelineDesc.primitive.frontFace = WGPUFrontFace::WGPUFrontFace_CCW;
	pipelineDesc.primitive.cullMode = WGPUCullMode::WGPUCullMode_Front;

  WGPUFragmentState fragmentState;
	pipelineDesc.fragment = &fragmentState;
	fragmentState.module = shaderModule;
	fragmentState.entryPoint = "fs_main";
	fragmentState.constantCount = 0;
	fragmentState.constants = nullptr;

  WGPUBlendState blendState;
	blendState.color.srcFactor = WGPUBlendFactor::WGPUBlendFactor_SrcAlpha;
	blendState.color.dstFactor = WGPUBlendFactor::WGPUBlendFactor_OneMinusSrcAlpha;
	blendState.color.operation = WGPUBlendOperation::WGPUBlendOperation_Add;
	blendState.alpha.srcFactor = WGPUBlendFactor::WGPUBlendFactor_Zero;
	blendState.alpha.dstFactor = WGPUBlendFactor::WGPUBlendFactor_One;
	blendState.alpha.operation = WGPUBlendOperation::WGPUBlendOperation_Add;

	WGPUColorTargetState colorTarget;
	colorTarget.format = WGPUTextureFormat::WGPUTextureFormat_BGRA8Unorm;
	colorTarget.blend = &blendState;
	colorTarget.writeMask = WGPUColorWriteMask::WGPUColorWriteMask_All;

	fragmentState.targetCount = 1;
	fragmentState.targets = &colorTarget;

  WGPUDepthStencilState depthStencilState = {};
  setDefault(depthStencilState);
	depthStencilState.depthCompare = WGPUCompareFunction::WGPUCompareFunction_Less;
	depthStencilState.depthWriteEnabled = true;

	WGPUTextureFormat depthTextureFormat = WGPUTextureFormat::WGPUTextureFormat_Depth24Plus;
	depthStencilState.format = depthTextureFormat;
	depthStencilState.stencilReadMask = 0;
	depthStencilState.stencilWriteMask = 0;
	
	pipelineDesc.depthStencil = &depthStencilState;

	pipelineDesc.multisample.count = 1;
	pipelineDesc.multisample.mask = ~0u;
	pipelineDesc.multisample.alphaToCoverageEnabled = false;

	// Create binding layout (don't forget to = Default)
	WGPUBindGroupLayoutEntry bindingLayout = {};
  setDefault(bindingLayout);
	bindingLayout.binding = 0;
	bindingLayout.visibility = WGPUShaderStage::WGPUShaderStage_Vertex | WGPUShaderStage::WGPUShaderStage_Fragment;
	bindingLayout.buffer.type = WGPUBufferBindingType::WGPUBufferBindingType_Uniform;
	bindingLayout.buffer.minBindingSize = sizeof(MyUniforms);

  // Create a bind group layout
	WGPUBindGroupLayoutDescriptor bindGroupLayoutDesc = {};
	bindGroupLayoutDesc.entryCount = 1;
	bindGroupLayoutDesc.entries = &bindingLayout;
	//BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDesc);
  WGPUBindGroupLayout bindGroupLayout = wgpuDeviceCreateBindGroupLayout(Device, &bindGroupLayoutDesc);

	// Create the pipeline layout
	WGPUPipelineLayoutDescriptor layoutDesc = {};
	layoutDesc.bindGroupLayoutCount = 1;
	layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&bindGroupLayout;
	//PipelineLayout layout = device.createPipelineLayout(layoutDesc);
  WGPUPipelineLayout layout = wgpuDeviceCreatePipelineLayout(Device, &layoutDesc);
	pipelineDesc.layout = layout;

	pipeline = wgpuDeviceCreateRenderPipeline(Device, &pipelineDesc);
	std::cout << "Render pipeline: " << pipeline << std::endl;

  // Create the depth texture
	WGPUTextureDescriptor depthTextureDesc;
	depthTextureDesc.dimension = WGPUTextureDimension::WGPUTextureDimension_2D;
	depthTextureDesc.format = depthTextureFormat;
	depthTextureDesc.mipLevelCount = 1;
	depthTextureDesc.sampleCount = 1;
	depthTextureDesc.size = {static_cast<uint32_t>(Application::Width), static_cast<uint32_t>(Application::Height), 1};
	depthTextureDesc.usage = WGPUTextureUsage::WGPUTextureUsage_RenderAttachment;
	depthTextureDesc.viewFormatCount = 1;
	depthTextureDesc.viewFormats = (WGPUTextureFormat*)&depthTextureFormat;
	//Texture depthTexture = device.createTexture(depthTextureDesc);
  WGPUTexture depthTexture = wgpuDeviceCreateTexture(Device, &depthTextureDesc);
	std::cout << "Depth texture: " << depthTexture << std::endl;

	// Create the view of the depth texture manipulated by the rasterizer
	WGPUTextureViewDescriptor depthTextureViewDesc;
	depthTextureViewDesc.aspect = WGPUTextureAspect::WGPUTextureAspect_DepthOnly;
	depthTextureViewDesc.baseArrayLayer = 0;
	depthTextureViewDesc.arrayLayerCount = 1;
	depthTextureViewDesc.baseMipLevel = 0;
	depthTextureViewDesc.mipLevelCount = 1;
	depthTextureViewDesc.dimension = WGPUTextureViewDimension::WGPUTextureViewDimension_2D;
	depthTextureViewDesc.format = depthTextureFormat;
	//TextureView depthTextureView = depthTexture.createView(depthTextureViewDesc);
  depthTextureView = wgpuTextureCreateView(depthTexture, &depthTextureViewDesc);
	std::cout << "Depth texture view: " << depthTextureView << std::endl;

  std::vector<float> pointData;
	std::vector<uint16_t> indexData;

	// Load mesh data from OBJ file
	std::vector<VertexAttributes> vertexData;
	//bool success = loadGeometryFromObj("res/models/pyramid.obj", vertexData);
  bool success = loadGeometryFromObj("res/models/mammoth.obj", vertexData);
	if (!success) {
		std::cerr << "Could not load geometry!" << std::endl;
		return;
	}

	// Create vertex buffer
	WGPUBufferDescriptor bufferDesc;
  bufferDesc.label = "vertex_buf";
	bufferDesc.size = vertexData.size() * sizeof(VertexAttributes); // changed
	bufferDesc.usage = WGPUBufferUsage::WGPUBufferUsage_CopyDst | WGPUBufferUsage::WGPUBufferUsage_Vertex;
	bufferDesc.mappedAtCreation = false;
	//Buffer vertexBuffer = device.createBuffer(bufferDesc);
  vertexBuffer = wgpuDeviceCreateBuffer(Device, &bufferDesc);
	//queue.writeBuffer(vertexBuffer, 0, vertexData.data(), bufferDesc.size); // changed
  wgpuQueueWriteBuffer(Queue, vertexBuffer, 0, vertexData.data(), bufferDesc.size);
	indexCount = static_cast<int>(vertexData.size()); // changed
	
	// Create uniform buffer
  bufferDesc.label = "uniform_buf";
	bufferDesc.size = sizeof(MyUniforms);
	bufferDesc.usage = WGPUBufferUsage::WGPUBufferUsage_CopyDst | WGPUBufferUsage::WGPUBufferUsage_Uniform ;
	bufferDesc.mappedAtCreation = false;
	//Buffer uniformBuffer = device.createBuffer(bufferDesc);
  uniformBuffer = wgpuDeviceCreateBuffer(Device, &bufferDesc);

  // Upload the initial value of the uniforms


	// Build transform matrices

	// Translate the view
	glm::vec3 focalPoint(0.0, 0.0, -1.0);
	// Rotate the object
	 // arbitrary time
	// Rotate the view point
	float angle2 = 3.0f * PI / 4.0f;

	S = glm::scale(glm::mat4x4(1.0), glm::vec3(0.3f));
	T1 = glm::mat4x4(1.0);
	R1 = glm::rotate(glm::mat4x4(1.0), angle1, glm::vec3(0.0, 0.0, 1.0));
	uniforms.modelMatrix = R1 * T1 * S;

	R2 = glm::rotate(glm::mat4x4(1.0), -angle2, glm::vec3(1.0, 0.0, 0.0));
	T2 = glm::translate(glm::mat4x4(1.0), -focalPoint);
	uniforms.viewMatrix = T2 * R2;

	float ratio = static_cast<float>(Application::Width) / static_cast<float>(Application::Height);
	float focalLength = 2.0;
	float near = 0.01f;
	float far = 100.0f;
	float divider = 1 / (focalLength * (far - near));
	uniforms.projectionMatrix = glm::transpose(glm::mat4x4(
		1.0, 0.0, 0.0, 0.0,
		0.0, ratio, 0.0, 0.0,
		0.0, 0.0, far * divider, -far * near * divider,
		0.0, 0.0, 1.0 / focalLength, 0.0
	));

	uniforms.time = 1.0f;
	uniforms.color = { 0.0f, 1.0f, 0.4f, 1.0f };
  //queue.writeBuffer(uniformBuffer, 0, &uniforms, sizeof(MyUniforms));
  wgpuQueueWriteBuffer(Queue, uniformBuffer, 0, &uniforms, sizeof(MyUniforms));

  // Create a binding
	WGPUBindGroupEntry binding = {};
	binding.binding = 0;
	binding.buffer = uniformBuffer;
	binding.offset = 0;
	binding.size = sizeof(MyUniforms);

	// A bind group contains one or multiple bindings
	WGPUBindGroupDescriptor bindGroupDesc;
	bindGroupDesc.layout = bindGroupLayout;
	bindGroupDesc.entryCount = bindGroupLayoutDesc.entryCount;
	bindGroupDesc.entries = &binding;
	//BindGroup bindGroup = device.createBindGroup(bindGroupDesc);
  bindGroup = wgpuDeviceCreateBindGroup(Device, &bindGroupDesc);

  // Configure the surface
  WGPUSurfaceConfiguration config = {};
  config.nextInChain = nullptr;

  // Configuration of the textures created for the underlying swap chain
  config.width = Width;
  config.height = Height;
  config.usage = WGPUTextureUsage_RenderAttachment;
  WGPUTextureFormat surfaceFormat = wgpuSurfaceGetPreferredFormat(Surface, adapter);
  config.format = surfaceFormat;

  // And we do not need any particular view format:
  config.viewFormatCount = 0;
  config.viewFormats = nullptr;
  config.device = Device;
  config.presentMode = WGPUPresentMode_Fifo;
  config.alphaMode = WGPUCompositeAlphaMode_Auto;

  wgpuSurfaceConfigure(Surface, &config);

  // Release the adapter only after it has been fully utilized
  wgpuAdapterRelease(adapter);
}

bool Application::isRunning(){
  messageLopp();
  return glfwWindowShouldClose(Window);
}

void Application::messageLopp(){
    glfwPollEvents();
    //Machine->update();
    //Machine->render();

    // Update uniform buffer
		uniforms.time = static_cast<float>(glfwGetTime()); // glfwGetTime returns a double
		// Only update the 1-st float of the buffer
		//queue.writeBuffer(uniformBuffer, offsetof(MyUniforms, time), &uniforms.time, sizeof(MyUniforms::time));
    wgpuQueueWriteBuffer(Queue, uniformBuffer, offsetof(MyUniforms, time), &uniforms.time, sizeof(MyUniforms::time));
		// Update view matrix
		angle1 = uniforms.time;
		R1 = glm::rotate(glm::mat4x4(1.0), angle1, glm::vec3(0.0, 0.0, 1.0));
		uniforms.modelMatrix = R1 * T1 * S;
		//queue.writeBuffer(uniformBuffer, offsetof(MyUniforms, modelMatrix), &uniforms.modelMatrix, sizeof(MyUniforms::modelMatrix));
    wgpuQueueWriteBuffer(Queue, uniformBuffer, offsetof(MyUniforms, modelMatrix), &uniforms.modelMatrix, sizeof(MyUniforms::modelMatrix));

		//WGPUTextureView nextTexture = SwapChain.getCurrentTextureView();
    WGPUTextureView nextTexture = wgpuSwapChainGetCurrentTextureView(SwapChain);
		if (!nextTexture) {
			std::cerr << "Cannot acquire next swap chain texture" << std::endl;
			return;
		}

    WGPUCommandEncoderDescriptor commandEncoderDesc = {};
		commandEncoderDesc.label = "Command Encoder";
		//CommandEncoder encoder = device.createCommandEncoder(commandEncoderDesc);
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(Device, &commandEncoderDesc);
		
		WGPURenderPassDescriptor renderPassDesc = {};

		WGPURenderPassColorAttachment renderPassColorAttachment = {};
		renderPassColorAttachment.view = nextTexture;
		renderPassColorAttachment.resolveTarget = nullptr;
		renderPassColorAttachment.loadOp = WGPULoadOp::WGPULoadOp_Clear;
		renderPassColorAttachment.storeOp = WGPUStoreOp::WGPUStoreOp_Store;
		renderPassColorAttachment.clearValue = WGPUColor{ 0.2, 0.2, 0.2, 1.0 };
    renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
		renderPassDesc.colorAttachmentCount = 1;
		renderPassDesc.colorAttachments = &renderPassColorAttachment;

		WGPURenderPassDepthStencilAttachment depthStencilAttachment;
		depthStencilAttachment.view = depthTextureView;
		depthStencilAttachment.depthClearValue = 1.0f;
		depthStencilAttachment.depthLoadOp = WGPULoadOp::WGPULoadOp_Clear;
		depthStencilAttachment.depthStoreOp = WGPUStoreOp::WGPUStoreOp_Store;
		depthStencilAttachment.depthReadOnly = false;
		depthStencilAttachment.stencilClearValue = 0;

#ifdef WEBGPU_BACKEND_WGPU
		depthStencilAttachment.stencilLoadOp = WGPULoadOp::WGPULoadOp_Clear;
		depthStencilAttachment.stencilStoreOp = WGPUStoreOp::WGPUStoreOp_Store;
#else
		depthStencilAttachment.stencilLoadOp = WGPULoadOp::WGPULoadOp_Undefined;
		depthStencilAttachment.stencilStoreOp = WGPUStoreOp::WGPUStoreOp_Undefined;
#endif

    depthStencilAttachment.stencilReadOnly = true;

		renderPassDesc.depthStencilAttachment = &depthStencilAttachment;

		//renderPassDesc.timestampWriteCount = 0;
		renderPassDesc.timestampWrites = nullptr;
		//RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);
    WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);

		//renderPass.setPipeline(pipeline);
    wgpuRenderPassEncoderSetPipeline(renderPass, pipeline);

		//renderPass.setVertexBuffer(0, vertexBuffer, 0, vertexData.size() * sizeof(VertexAttributes)); // changed
    wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, vertexBuffer, 0, wgpuBufferGetSize(vertexBuffer));

		// Set binding group
		//renderPass.setBindGroup(0, bindGroup, 0, nullptr);
    wgpuRenderPassEncoderSetBindGroup(renderPass, 0, bindGroup, 0, nullptr);

		//renderPass.draw(indexCount, 1, 0, 0); // changed
    wgpuRenderPassEncoderDraw(renderPass, indexCount, 1, 0, 0);
		//renderPass.end();
    wgpuRenderPassEncoderEnd(renderPass);
		//renderPass.release();
    wgpuRenderPassEncoderRelease(renderPass);
		
		//nextTexture.release();
    wgpuTextureViewRelease(nextTexture);

    WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
		cmdBufferDescriptor.label = "Command buffer";
		//CommandBuffer command = encoder.finish(cmdBufferDescriptor);
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);

    //encoder.release();
	  wgpuCommandEncoderRelease(encoder);

		//queue.submit(command);
    wgpuQueueSubmit(Queue, 1, &command);

		//command.release();
    wgpuCommandBufferRelease(command);

		//swapChain.present();
    //wgpuSwapChainPresent(SwapChain);

#ifdef WEBGPU_BACKEND_DAWN
		// Check for pending error callbacks
		//device.tick();
    wgpuDeviceTick(Device);
#endif
}

void Application::initGui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui_ImplGlfw_InitForOther(Window, true);
	ImGui_ImplWGPU_Init(Device, 3, wgpContext.colorformat, WGPUTextureFormat::WGPUTextureFormat_Undefined);
}

void Application::initStates(){
    Machine = new StateMachine(dt, fdt);
    //Machine->addStateAtTop(new Shape(*Machine));
	Machine->addStateAtTop(new Default(*Machine));
}

void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(ImGui::GetIO().WantCaptureMouse)
      ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    else{
      Event event;
      event.data.keyboard.keyCode = static_cast<unsigned int>(scancode);

      if (action == GLFW_PRESS){
        event.type = Event::KEYDOWN;
        Application::Machine->getStates().top()->OnKeyDown(event.data.keyboard);
      }

      if(action == GLFW_RELEASE){
        event.type = Event::KEYUP;
        Application::Machine->getStates().top()->OnKeyUp(event.data.keyboard);
      }
      ImGui::GetIO().WantCaptureMouse = true;
    }
}

void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) { 
    if(ImGui::GetIO().WantCaptureMouse)
        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    else{
      double xpos, ypos;
      glfwGetCursorPos(window, &xpos, &ypos);
    
      Event event; 
      event.data.mouseButton.x = static_cast<int>(xpos);
      event.data.mouseButton.y = static_cast<int>(ypos);
      event.data.mouseButton.button = (button == GLFW_MOUSE_BUTTON_RIGHT) ? Event::MouseButtonEvent::MouseButton::BUTTON_RIGHT : Event::MouseButtonEvent::MouseButton::BUTTON_LEFT;

      if (action == GLFW_PRESS){
        event.type = Event::MOUSEBUTTONDOWN;
        Application::Machine->getStates().top()->OnMouseButtonDown(event.data.mouseButton);
      }

      if(action == GLFW_RELEASE){
        event.type = Event::MOUSEBUTTONUP;
        Application::Machine->getStates().top()->OnMouseButtonUp(event.data.mouseButton);
      }
      ImGui::GetIO().WantCaptureMouse = true;
    }
}

void glfwMouseMoveCallback(GLFWwindow* window, double xpos, double ypos){
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
    
    Event event;
    event.type = Event::MOUSEMOTION;
    event.data.mouseMove.x = static_cast<int>(xpos);
    event.data.mouseMove.y = static_cast<int>(ypos);
    //event.data.mouseMove.button = (button == GLFW_MOUSE_BUTTON_RIGHT) ? Event::MouseButtonEvent::MouseButton::BUTTON_RIGHT : Event::MouseButtonEvent::MouseButton::BUTTON_LEFT;
    
    Application::Machine->getStates().top()->OnMouseMotion(event.data.mouseMove);   
}

WGPUTextureView Application::GetNextSurfaceTextureView() {
  WGPUSurfaceTexture surfaceTexture;
  wgpuSurfaceGetCurrentTexture(Surface, &surfaceTexture);
  if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
    return nullptr;
  }

  WGPUTextureViewDescriptor viewDescriptor;
  viewDescriptor.nextInChain = nullptr;
  viewDescriptor.label = "Surface texture view";
  viewDescriptor.format = wgpuTextureGetFormat(surfaceTexture.texture);
  viewDescriptor.dimension = WGPUTextureViewDimension_2D;
  viewDescriptor.baseMipLevel = 0;
  viewDescriptor.mipLevelCount = 1;
  viewDescriptor.baseArrayLayer = 0;
  viewDescriptor.arrayLayerCount = 1;
  viewDescriptor.aspect = WGPUTextureAspect_All;
  WGPUTextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);

#ifndef WEBGPU_BACKEND_WGPU
  wgpuTextureRelease(surfaceTexture.texture);
#endif
  return targetView;
}

WGPURequiredLimits Application::GetRequiredLimits(WGPUAdapter adapter) {
	// Get adapter supported limits, in case we need them
	WGPUSupportedLimits supportedLimits;
	supportedLimits.nextInChain = nullptr;
	wgpuAdapterGetLimits(adapter, &supportedLimits);

	WGPURequiredLimits requiredLimits{};
	setDefault(requiredLimits.limits);

	// We use at most 1 vertex attribute for now
	requiredLimits.limits.maxVertexAttributes = 1;
	// We should also tell that we use 1 vertex buffers
	requiredLimits.limits.maxVertexBuffers = 1;
	// Maximum size of a buffer is 6 vertices of 2 float each
	requiredLimits.limits.maxBufferSize = 6 * 2 * sizeof(float);
	// Maximum stride between 2 consecutive vertices in the vertex buffer
	requiredLimits.limits.maxVertexBufferArrayStride = 2 * sizeof(float);

	// These two limits are different because they are "minimum" limits,
	// they are the only ones we are may forward from the adapter's supported
	// limits.
	requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
	requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;

	return requiredLimits;
}

void Application::setDefault(WGPULimits &limits) {
	limits.maxTextureDimension1D = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxTextureDimension2D = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxTextureDimension3D = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxTextureArrayLayers = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxBindGroups = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxBindGroupsPlusVertexBuffers = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxBindingsPerBindGroup = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxDynamicUniformBuffersPerPipelineLayout = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxDynamicStorageBuffersPerPipelineLayout = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxSampledTexturesPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxSamplersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxStorageBuffersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxStorageTexturesPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxUniformBuffersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxUniformBufferBindingSize = WGPU_LIMIT_U64_UNDEFINED;
	limits.maxStorageBufferBindingSize = WGPU_LIMIT_U64_UNDEFINED;
	limits.minUniformBufferOffsetAlignment = WGPU_LIMIT_U32_UNDEFINED;
	limits.minStorageBufferOffsetAlignment = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxVertexBuffers = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxBufferSize = WGPU_LIMIT_U64_UNDEFINED;
	limits.maxVertexAttributes = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxVertexBufferArrayStride = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxInterStageShaderComponents = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxInterStageShaderVariables = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxColorAttachments = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxColorAttachmentBytesPerSample = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxComputeWorkgroupStorageSize = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxComputeInvocationsPerWorkgroup = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxComputeWorkgroupSizeX = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxComputeWorkgroupSizeY = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxComputeWorkgroupSizeZ = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxComputeWorkgroupsPerDimension = WGPU_LIMIT_U32_UNDEFINED;
}

void Application::setDefault(WGPUBindGroupLayoutEntry& bindingLayout) {
	bindingLayout.buffer.nextInChain = nullptr;
	bindingLayout.buffer.type = WGPUBufferBindingType_Undefined;
	bindingLayout.buffer.hasDynamicOffset = false;

	bindingLayout.sampler.nextInChain = nullptr;
	bindingLayout.sampler.type = WGPUSamplerBindingType_Undefined;

	bindingLayout.storageTexture.nextInChain = nullptr;
	bindingLayout.storageTexture.access = WGPUStorageTextureAccess_Undefined;
	bindingLayout.storageTexture.format = WGPUTextureFormat_Undefined;
	bindingLayout.storageTexture.viewDimension = WGPUTextureViewDimension_Undefined;

	bindingLayout.texture.nextInChain = nullptr;
	bindingLayout.texture.multisampled = false;
	bindingLayout.texture.sampleType = WGPUTextureSampleType_Undefined;
	bindingLayout.texture.viewDimension = WGPUTextureViewDimension_Undefined;
}

void Application::setDefault(WGPUDepthStencilState& depthStencilState){
  depthStencilState.nextInChain = nullptr;
  depthStencilState.format = WGPUTextureFormat::WGPUTextureFormat_Undefined;
  depthStencilState.depthWriteEnabled = true;
  depthStencilState.depthCompare = WGPUCompareFunction::WGPUCompareFunction_Less;

  //depthStencilState.stencilFront = WGPUStencilFaceState::WGPUCompareFunction;
  //depthStencilState.stencilBack = WGPUStencilFaceState::WGPUCompareFunction;

  depthStencilState.stencilReadMask = 0;
  depthStencilState.stencilWriteMask = 0;

	depthStencilState.depthBias = 0;
  
  depthStencilState.depthBiasSlopeScale = 0.0f;
  depthStencilState.depthBiasClamp = 0.0f;
}

WGPUShaderModule Application::loadShaderModule(const std::filesystem::path& path) {
	std::ifstream file(path);
	if (!file.is_open()) {
		return nullptr;
	}
	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	std::string shaderSource(size, ' ');
	file.seekg(0);
	file.read(shaderSource.data(), size);

	WGPUShaderModuleWGSLDescriptor shaderCodeDesc;
	shaderCodeDesc.chain.next = nullptr;
	shaderCodeDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
	shaderCodeDesc.code = shaderSource.c_str();

	WGPUShaderModuleDescriptor shaderDesc;
	shaderDesc.nextInChain = &shaderCodeDesc.chain;
#ifdef WEBGPU_BACKEND_WGPU
	shaderDesc.hintCount = 0;
	shaderDesc.hints = nullptr;
#endif

	//return Application::Device.createShaderModule(shaderDesc);
  return wgpuDeviceCreateShaderModule(Application::Device, &shaderDesc);
}

bool Application::loadGeometryFromObj(const std::filesystem::path& path, std::vector<VertexAttributes>& vertexData) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;

	// Call the core loading procedure of TinyOBJLoader
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str());

	// Check errors
	if (!warn.empty()) {
		std::cout << warn << std::endl;
	}

	if (!err.empty()) {
		std::cerr << err << std::endl;
	}

	if (!ret) {
		return false;
	}

	// Filling in vertexData:
	vertexData.clear();
	for (const auto& shape : shapes) {
		size_t offset = vertexData.size();
		vertexData.resize(offset + shape.mesh.indices.size());

		for (size_t i = 0; i < shape.mesh.indices.size(); ++i) {
			const tinyobj::index_t& idx = shape.mesh.indices[i];

			vertexData[offset + i].position = {
				attrib.vertices[3 * idx.vertex_index + 0],
				-attrib.vertices[3 * idx.vertex_index + 2], // Add a minus to avoid mirroring
				attrib.vertices[3 * idx.vertex_index + 1]
			};

			// Also apply the transform to normals!!
			vertexData[offset + i].normal = {
				attrib.normals[3 * idx.normal_index + 0],
				-attrib.normals[3 * idx.normal_index + 2],
				attrib.normals[3 * idx.normal_index + 1]
			};

			vertexData[offset + i].color = {
				attrib.colors[3 * idx.vertex_index + 0],
				attrib.colors[3 * idx.vertex_index + 1],
				attrib.colors[3 * idx.vertex_index + 2]
			};
		}
	}

	return true;
}
