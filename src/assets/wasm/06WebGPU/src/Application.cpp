#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu.hpp>

#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>
#include <GL/glew.h>
#include <emscripten.h>

#include <imgui.h>
#include <imgui_impl_wgpu.h>
#include <imgui_impl_glfw.h>

#include <States/Shape.h>

#include "WebGpuUtils.h"
#include "Application.h"
#include "ResourceManager.h"

GLFWwindow* Application::Window = nullptr;
StateMachine* Application::Machine = nullptr;
wgpu::Device Application::Device;
wgpu::Queue Application::Queue;
wgpu::Surface Application::Surface;

int Application::Width;
int Application::Height;
double Application::Time;

constexpr float PI = 3.14159265358979323846f;

void Application::MessageLopp(void *arg) {
  Application* application  = reinterpret_cast<Application*>(arg);

  Time = glfwGetTime();
  application->dt = float(Time - application->last);
  application->last = Time;

  application->messageLopp();
}

// The raw GLFW callback
void onWindowResize(GLFWwindow* m_window, int /* width */, int /* height */) {
	auto pApp = reinterpret_cast<Application*>(glfwGetWindowUserPointer(m_window));
	if (pApp != nullptr) pApp->onResize();
}

void onWindowMouseMove(GLFWwindow* m_window, double xpos, double ypos) {
	auto pApp = reinterpret_cast<Application*>(glfwGetWindowUserPointer(m_window));
	if (pApp != nullptr) pApp->onMouseMove(xpos, ypos);
}
void onWindowMouseButton(GLFWwindow* m_window, int button, int action, int mods) {
	auto pApp = reinterpret_cast<Application*>(glfwGetWindowUserPointer(m_window));
	if (pApp != nullptr) pApp->onMouseButton(button, action, mods);
}
void onWindowScroll(GLFWwindow* m_window, double xoffset, double yoffset) {
	auto pApp = reinterpret_cast<Application*>(glfwGetWindowUserPointer(m_window));
	if (pApp != nullptr) pApp->onScroll(xoffset, yoffset);
}


Application::Application(float& dt, float& fdt) : fdt(fdt), dt(dt), last(0.0) {
  Application::Width = 640;
  Application::Height = 480;
  initWindow();
  initWebGPU();
  initStates();
}

Application::~Application() {
  vertexBuffer.destroy();
  vertexBuffer.release();

  textureView.release();
  texture.destroy();
  texture.release();

  depthTextureView.release();
  depthTexture.destroy();
  depthTexture.release();

  SwapChain.release();
  Queue.release();
  Device.release();
  adapter.release();
  instance.release();
  Surface.release();

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
  wgpu::InstanceDescriptor desc = {};
  instance = wgpu::createInstance();
  if (!instance) {
    std::cout << "Could not initialize WebGPU!" << std::endl;
    return;
  }

  std::cout << "Requesting adapter..." << std::endl;
  Surface = glfwGetWGPUSurface(instance, Window);
  wgpu::RequestAdapterOptions adapterOpts = {};
  adapterOpts.compatibleSurface = Surface;
  adapter = instance.requestAdapter(adapterOpts);
  std::cout << "Got adapter: " << adapter << std::endl;

  wgpu::SupportedLimits supportedLimits;
  #ifdef __EMSCRIPTEN__
	// Error in Chrome: Aborted(TODO: wgpuAdapterGetLimits unimplemented)
	// (as of September 4, 2023), so we hardcode values:
	// These work for 99.95% of clients (source: https://web3dsurvey.com/webgpu)
	supportedLimits.limits.minStorageBufferOffsetAlignment = 256;
	supportedLimits.limits.minUniformBufferOffsetAlignment = 256;
#else
	adapter.getLimits(&supportedLimits);
#endif

	std::cout << "Requesting device..." << std::endl;
	wgpu::RequiredLimits requiredLimits = wgpu::Default;
	requiredLimits.limits.maxVertexAttributes = 4;
	requiredLimits.limits.maxVertexBuffers = 1;
	requiredLimits.limits.maxBufferSize = 150000 * sizeof(VertexAttributes);
	requiredLimits.limits.maxVertexBufferArrayStride = sizeof(VertexAttributes);
	requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;
	requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
	requiredLimits.limits.maxInterStageShaderComponents = WGPU_LIMIT_U32_UNDEFINED;
	requiredLimits.limits.maxBindGroups = 2;
	//                                    ^ This was a 1
	requiredLimits.limits.maxUniformBuffersPerShaderStage = 1;
	requiredLimits.limits.maxUniformBufferBindingSize = 16 * 4 * sizeof(float);
	// Allow textures up to 2K
	requiredLimits.limits.maxTextureDimension1D = 2048;
	requiredLimits.limits.maxTextureDimension2D = 2048;
	requiredLimits.limits.maxTextureArrayLayers = 1;
	requiredLimits.limits.maxSampledTexturesPerShaderStage = 1;
	requiredLimits.limits.maxSamplersPerShaderStage = 1;

	wgpu::DeviceDescriptor deviceDesc;
	deviceDesc.label = "My Device";
	deviceDesc.requiredFeatureCount = 0;
	deviceDesc.requiredLimits = &requiredLimits;
	deviceDesc.defaultQueue.label = "The default queue";
	Device = adapter.requestDevice(deviceDesc);
	std::cout << "Got device: " << Device << std::endl;

	// Add an error callback for more debug info
	errorCallbackHandle = Device.setUncapturedErrorCallback([](wgpu::ErrorType type, char const* message) {
		std::cout << "Device error: type " << type;
		if (message) std::cout << " (message: " << message << ")";
		std::cout << std::endl;
	});

	Queue = Device.getQueue();

	buildSwapChain();
	buildDepthBuffer();

	std::cout << "Creating shader module..." << std::endl;
	shaderModule = ResourceManager::loadShaderModule("res/shader/shader.wgsl", Device);
	std::cout << "Shader module: " << shaderModule << std::endl;

	std::cout << "Creating render pipeline..." << std::endl;
	wgpu::RenderPipelineDescriptor pipelineDesc;

	// Vertex fetch
	std::vector<wgpu::VertexAttribute> vertexAttribs(4);

	// Position attribute
	vertexAttribs[0].shaderLocation = 0;
	vertexAttribs[0].format = wgpu::VertexFormat::Float32x3;
	vertexAttribs[0].offset = 0;

	// Normal attribute
	vertexAttribs[1].shaderLocation = 1;
	vertexAttribs[1].format = wgpu::VertexFormat::Float32x3;
	vertexAttribs[1].offset = offsetof(VertexAttributes, normal);

	// Color attribute
	vertexAttribs[2].shaderLocation = 2;
	vertexAttribs[2].format = wgpu::VertexFormat::Float32x3;
	vertexAttribs[2].offset = offsetof(VertexAttributes, color);

	// UV attribute
	vertexAttribs[3].shaderLocation = 3;
	vertexAttribs[3].format = wgpu::VertexFormat::Float32x2;
	vertexAttribs[3].offset = offsetof(VertexAttributes, uv);

	wgpu::VertexBufferLayout vertexBufferLayout;
	vertexBufferLayout.attributeCount = (uint32_t)vertexAttribs.size();
	vertexBufferLayout.attributes = vertexAttribs.data();
	vertexBufferLayout.arrayStride = sizeof(VertexAttributes);
	vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;

	pipelineDesc.vertex.bufferCount = 1;
	pipelineDesc.vertex.buffers = &vertexBufferLayout;

	pipelineDesc.vertex.module = shaderModule;
	pipelineDesc.vertex.entryPoint = "vs_main";
	pipelineDesc.vertex.constantCount = 0;
	pipelineDesc.vertex.constants = nullptr;

	pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
	pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
	pipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW;
	pipelineDesc.primitive.cullMode = wgpu::CullMode::None;

	wgpu::FragmentState fragmentState;
	pipelineDesc.fragment = &fragmentState;
	fragmentState.module = shaderModule;
	fragmentState.entryPoint = "fs_main";
	fragmentState.constantCount = 0;
	fragmentState.constants = nullptr;

	wgpu::BlendState blendState;
	blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
	blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
	blendState.color.operation = wgpu::BlendOperation::Add;
	blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
	blendState.alpha.dstFactor = wgpu::BlendFactor::One;
	blendState.alpha.operation = wgpu::BlendOperation::Add;

	wgpu::ColorTargetState colorTarget;
	colorTarget.format = swapChainFormat;
	colorTarget.blend = &blendState;
	colorTarget.writeMask = wgpu::ColorWriteMask::All;

	fragmentState.targetCount = 1;
	fragmentState.targets = &colorTarget;

	wgpu::DepthStencilState depthStencilState = wgpu::Default;
	depthStencilState.depthCompare = wgpu::CompareFunction::Less;
	depthStencilState.depthWriteEnabled = true;
	depthStencilState.format = depthTextureFormat;
	depthStencilState.stencilReadMask = 0;
	depthStencilState.stencilWriteMask = 0;
	
	pipelineDesc.depthStencil = &depthStencilState;

	pipelineDesc.multisample.count = 1;
	pipelineDesc.multisample.mask = ~0u;
	pipelineDesc.multisample.alphaToCoverageEnabled = false;

	// Create binding layouts

	std::vector<wgpu::BindGroupLayoutEntry> bindingLayoutEntries(3, wgpu::Default);

	// The uniform buffer binding that we already had
	wgpu::BindGroupLayoutEntry& bindingLayout = bindingLayoutEntries[0];
	bindingLayout.binding = 0;
	bindingLayout.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
	bindingLayout.buffer.type = wgpu::BufferBindingType::Uniform;
	bindingLayout.buffer.minBindingSize = sizeof(MyUniforms);

	// The texture binding
	wgpu::BindGroupLayoutEntry& textureBindingLayout = bindingLayoutEntries[1];
	textureBindingLayout.binding = 1;
	textureBindingLayout.visibility = wgpu::ShaderStage::Fragment;
	textureBindingLayout.texture.sampleType = wgpu::TextureSampleType::Float;
	textureBindingLayout.texture.viewDimension = wgpu::TextureViewDimension::_2D;

	// The texture sampler binding
	wgpu::BindGroupLayoutEntry& samplerBindingLayout = bindingLayoutEntries[2];
	samplerBindingLayout.binding = 2;
	samplerBindingLayout.visibility = wgpu::ShaderStage::Fragment;
	samplerBindingLayout.sampler.type = wgpu::SamplerBindingType::Filtering;

	// Create a bind group layout
	wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc = {};
	bindGroupLayoutDesc.entryCount = (uint32_t)bindingLayoutEntries.size();
	bindGroupLayoutDesc.entries = bindingLayoutEntries.data();
	wgpu::BindGroupLayout bindGroupLayout = Device.createBindGroupLayout(bindGroupLayoutDesc);

	// Create the pipeline layout
	wgpu::PipelineLayoutDescriptor layoutDesc = {};
	layoutDesc.bindGroupLayoutCount = 1;
	layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&bindGroupLayout;
	wgpu::PipelineLayout layout = Device.createPipelineLayout(layoutDesc);
	pipelineDesc.layout = layout;

	pipeline = Device.createRenderPipeline(pipelineDesc);
	std::cout << "Render pipeline: " << pipeline << std::endl;

	// Create a sampler
	wgpu::SamplerDescriptor samplerDesc;
	samplerDesc.addressModeU = wgpu::AddressMode::Repeat;
	samplerDesc.addressModeV = wgpu::AddressMode::Repeat;
	samplerDesc.addressModeW = wgpu::AddressMode::Repeat;
	samplerDesc.magFilter = wgpu::FilterMode::Linear;
	samplerDesc.minFilter = wgpu::FilterMode::Linear;
	samplerDesc.mipmapFilter = wgpu::MipmapFilterMode::Linear;
	samplerDesc.lodMinClamp = 0.0f;
	samplerDesc.lodMaxClamp = 8.0f;
	samplerDesc.compare = wgpu::CompareFunction::Undefined;
	samplerDesc.maxAnisotropy = 1;
	sampler = Device.createSampler(samplerDesc);

	// Create a texture
	texture = ResourceManager::loadTexture("res/textures/fourareen2K_albedo.jpg", Device, &textureView);
	if (!texture) {
		std::cout << "Could not load texture!" << std::endl;
		return;
	}
	std::cout << "Texture: " << texture << std::endl;
	std::cout << "Texture view: " << textureView << std::endl;

	// Load mesh data from OBJ file
	std::vector<VertexAttributes> vertexData;
	bool success = ResourceManager::loadGeometryFromObj("res/models/fourareen.obj", vertexData);
	if (!success) {
		std::cout << "Could not load geometry!" << std::endl;
		return;
	}

	// Create vertex buffer
	wgpu::BufferDescriptor bufferDesc;
	bufferDesc.size = vertexData.size() * sizeof(VertexAttributes);
	bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex;
	bufferDesc.mappedAtCreation = false;
	vertexBuffer = Device.createBuffer(bufferDesc);
	Queue.writeBuffer(vertexBuffer, 0, vertexData.data(), bufferDesc.size);

	m_vertexCount = static_cast<int>(vertexData.size());
	
	// Create uniform buffer
	bufferDesc.size = sizeof(MyUniforms);
	bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
	bufferDesc.mappedAtCreation = false;
	uniformBuffer = Device.createBuffer(bufferDesc);

	// Upload the initial value of the uniforms
	m_uniforms.modelMatrix = glm::mat4x4(1.0);
	m_uniforms.viewMatrix = glm::lookAt(glm::vec3(-2.0f, -3.0f, 2.0f), glm::vec3(0.0f), glm::vec3(0, 0, 1));
	m_uniforms.projectionMatrix = glm::perspective(45 * PI / 180, static_cast<float>(Application::Width) / static_cast<float>(Application::Height), 0.01f, 100.0f);
	m_uniforms.time = 1.0f;
	m_uniforms.color = { 0.0f, 1.0f, 0.4f, 1.0f };
	Queue.writeBuffer(uniformBuffer, 0, &m_uniforms, sizeof(MyUniforms));

	// Create a binding
	std::vector<wgpu::BindGroupEntry> bindings(3);

	bindings[0].binding = 0;
	bindings[0].buffer = uniformBuffer;
	bindings[0].offset = 0;
	bindings[0].size = sizeof(MyUniforms);

	bindings[1].binding = 1;
	bindings[1].textureView = textureView;

	bindings[2].binding = 2;
	bindings[2].sampler = sampler;

	wgpu::BindGroupDescriptor bindGroupDesc;
	bindGroupDesc.layout = bindGroupLayout;
	bindGroupDesc.entryCount = (uint32_t)bindings.size();
	bindGroupDesc.entries = bindings.data();
	bindGroup = Device.createBindGroup(bindGroupDesc);

	// Setup GLFW callbacks
    glfwSetWindowUserPointer(Window, this);
    glfwSetFramebufferSizeCallback(Window, onWindowResize);
	glfwSetCursorPosCallback(Window, onWindowMouseMove);
	glfwSetMouseButtonCallback(Window, onWindowMouseButton);
	glfwSetScrollCallback(Window, onWindowScroll);

	initGui();
}

bool Application::isRunning(){
  messageLopp();
  return glfwWindowShouldClose(Window);
}

void Application::messageLopp(){
  glfwPollEvents();
  updateDragInertia();

  // Update uniform buffer
	m_uniforms.time = static_cast<float>(glfwGetTime());
	Queue.writeBuffer(uniformBuffer, offsetof(MyUniforms, time), &m_uniforms.time, sizeof(MyUniforms::time));
	
	wgpu::TextureView nextTexture = SwapChain.getCurrentTextureView();
	if (!nextTexture) {
		std::cerr << "Cannot acquire next swap chain texture" << std::endl;
		return;
	}

	wgpu::CommandEncoderDescriptor commandEncoderDesc;
	commandEncoderDesc.label = "Command Encoder";
	wgpu::CommandEncoder encoder = Device.createCommandEncoder(commandEncoderDesc);
	
	wgpu::RenderPassDescriptor renderPassDesc = {};

	wgpu::RenderPassColorAttachment renderPassColorAttachment = {};
	renderPassColorAttachment.view = nextTexture;
	renderPassColorAttachment.resolveTarget = nullptr;
	renderPassColorAttachment.loadOp = wgpu::LoadOp::Clear;
	renderPassColorAttachment.storeOp = wgpu::StoreOp::Store;
	renderPassColorAttachment.clearValue = wgpu::Color{ 0.05, 0.05, 0.05, 1.0 };
	renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
	renderPassDesc.colorAttachmentCount = 1;
	renderPassDesc.colorAttachments = &renderPassColorAttachment;

	wgpu::RenderPassDepthStencilAttachment depthStencilAttachment;
	depthStencilAttachment.view = depthTextureView;
	depthStencilAttachment.depthClearValue = 1.0f;
	depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Clear;
	depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
	depthStencilAttachment.depthReadOnly = false;
	depthStencilAttachment.stencilClearValue = 0;
#ifdef WEBGPU_BACKEND_WGPU
	depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Clear;
	depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Store;
#else
	depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Undefined;
	depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Undefined;
#endif
	depthStencilAttachment.stencilReadOnly = true;

	renderPassDesc.depthStencilAttachment = &depthStencilAttachment;

	//renderPassDesc.timestampWriteCount = 0;
	renderPassDesc.timestampWrites = nullptr;
	wgpu::RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);

	renderPass.setPipeline(pipeline);

	renderPass.setVertexBuffer(0, vertexBuffer, 0, m_vertexCount * sizeof(VertexAttributes));

	// Set binding group
	renderPass.setBindGroup(0, bindGroup, 0, nullptr);

	renderPass.draw(m_vertexCount, 1, 0, 0);

	// We add the GUI drawing commands to the render pass
	updateGui(renderPass);

	renderPass.end();
	renderPass.release();
	
	nextTexture.release();

	wgpu::CommandBufferDescriptor cmdBufferDescriptor = {};
	cmdBufferDescriptor.label = "Command buffer";
	wgpu::CommandBuffer command = encoder.finish(cmdBufferDescriptor);
	encoder.release();
	Queue.submit(command);
	command.release();

#ifndef __EMSCRIPTEN__
	SwapChain.present();
#endif

#ifdef WEBGPU_BACKEND_DAWN
	// Check for pending error callbacks
	Device.tick();
#endif
}

void Application::initStates(){
    Machine = new StateMachine(dt, fdt);
    Machine->addStateAtTop(new Shape(*Machine));
}

/* ***** Private methods ***** */

void Application::buildSwapChain() {
	int width, height;
	glfwGetFramebufferSize(Window, &width, &height);

	// Destroy previously allocated swap chain
	if (SwapChain != nullptr) {
		SwapChain.release();
	}

	std::cout << "Creating swapchain..." << std::endl;
#ifdef WEBGPU_BACKEND_WGPU
	swapChainFormat = Surface.getPreferredFormat(adapter);
#else
	swapChainFormat = wgpu::TextureFormat::BGRA8Unorm;
#endif
	swapChainDesc.width = static_cast<uint32_t>(width);
	swapChainDesc.height = static_cast<uint32_t>(height);
	swapChainDesc.usage = wgpu::TextureUsage::RenderAttachment;
	swapChainDesc.format = swapChainFormat;
	swapChainDesc.presentMode = wgpu::PresentMode::Fifo;
	SwapChain = Device.createSwapChain(Surface, swapChainDesc);
	std::cout << "Swapchain: " << SwapChain << std::endl;
}

void Application::buildDepthBuffer() {
	// Destroy previously allocated texture
	if (depthTexture != nullptr) {
		depthTextureView.release();
		depthTexture.destroy();
		depthTexture.release();
	}

	// Create the depth texture
	wgpu::TextureDescriptor depthTextureDesc;
	depthTextureDesc.dimension = wgpu::TextureDimension::_2D;
	depthTextureDesc.format = depthTextureFormat;
	depthTextureDesc.mipLevelCount = 1;
	depthTextureDesc.sampleCount = 1;
	depthTextureDesc.size = { swapChainDesc.width, swapChainDesc.height, 1};
	depthTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;
	depthTextureDesc.viewFormatCount = 1;
	depthTextureDesc.viewFormats = (WGPUTextureFormat*)&depthTextureFormat;
	depthTexture = Device.createTexture(depthTextureDesc);
	std::cout << "Depth texture: " << depthTexture << std::endl;

	// Create the view of the depth texture manipulated by the rasterizer
	wgpu::TextureViewDescriptor depthTextureViewDesc;
	depthTextureViewDesc.aspect = wgpu::TextureAspect::DepthOnly;
	depthTextureViewDesc.baseArrayLayer = 0;
	depthTextureViewDesc.arrayLayerCount = 1;
	depthTextureViewDesc.baseMipLevel = 0;
	depthTextureViewDesc.mipLevelCount = 1;
	depthTextureViewDesc.dimension = wgpu::TextureViewDimension::_2D;
	depthTextureViewDesc.format = depthTextureFormat;
	depthTextureView = depthTexture.createView(depthTextureViewDesc);
	std::cout << "Depth texture view: " << depthTextureView << std::endl;
}

void Application::updateViewMatrix() {
	float cx = cos(m_cameraState.angles.x);
	float sx = sin(m_cameraState.angles.x);
	float cy = cos(m_cameraState.angles.y);
	float sy = sin(m_cameraState.angles.y);
	glm::vec3 position = glm::vec3(cx * cy, sx * cy, sy) * std::exp(-m_cameraState.zoom);
	m_uniforms.viewMatrix = glm::lookAt(position, glm::vec3(0.0f), glm::vec3(0, 0, 1));
	Queue.writeBuffer(
		uniformBuffer,
		offsetof(MyUniforms, viewMatrix),
		&m_uniforms.viewMatrix,
		sizeof(MyUniforms::viewMatrix)
	);
}

void Application::updateDragInertia() {
	constexpr float eps = 1e-4f;
	// Apply inertia only when the user released the click.
	if (!m_drag.active) {
		// Avoid updating the matrix when the velocity is no longer noticeable
		if (std::abs(m_drag.velocity.x) < eps && std::abs(m_drag.velocity.y) < eps) {
			return;
		}
		m_cameraState.angles += m_drag.velocity;
		m_cameraState.angles.y = glm::clamp(m_cameraState.angles.y, -PI / 2 + 1e-5f, PI / 2 - 1e-5f);
		// Dampen the velocity so that it decreases exponentially and stops
		// after a few frames.
		m_drag.velocity *= m_drag.intertia;
		updateViewMatrix();
	}
}

void Application::onResize() {
	buildSwapChain();
	buildDepthBuffer();

	float ratio = swapChainDesc.width / (float)swapChainDesc.height;
	m_uniforms.projectionMatrix = glm::perspective(45 * PI / 180, ratio, 0.01f, 100.0f);
	Queue.writeBuffer(
		uniformBuffer,
		offsetof(MyUniforms, projectionMatrix),
		&m_uniforms.projectionMatrix,
		sizeof(MyUniforms::projectionMatrix)
	);
}

void Application::onMouseMove(double xpos, double ypos) {
	if (m_drag.active) {
		glm::vec2 currentMouse = glm::vec2(-(float)xpos, (float)ypos);
		glm::vec2 delta = (currentMouse - m_drag.startMouse) * m_drag.sensitivity;
		m_cameraState.angles = m_drag.startCameraState.angles + delta;
		// Clamp to avoid going too far when orbitting up/down
		m_cameraState.angles.y = glm::clamp(m_cameraState.angles.y, -PI / 2 + 1e-5f, PI / 2 - 1e-5f);
		updateViewMatrix();

		// Inertia
		m_drag.velocity = delta - m_drag.previousDelta;
		m_drag.previousDelta = delta;
	}
}

void Application::onMouseButton(int button, int action, [[maybe_unused]] int modifiers) {
	//ImGuiIO& io = ImGui::GetIO();
	//if (io.WantCaptureMouse) {
		// Don't rotate the camera if the mouse is already captured by an ImGui
		// interaction at this frame.
		//return;
	//}

	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		switch (action) {
		case GLFW_PRESS:
			m_drag.active = true;
			double xpos, ypos;
			glfwGetCursorPos(Window, &xpos, &ypos);
			m_drag.startMouse = glm::vec2(-(float)xpos, (float)ypos);
			m_drag.startCameraState = m_cameraState;
			break;
		case GLFW_RELEASE:
			m_drag.active = false;
			break;
		}
	}
}

void Application::onScroll([[maybe_unused]] double xoffset, double yoffset) {
	m_cameraState.zoom += m_drag.scrollSensitivity * (float)yoffset;
	m_cameraState.zoom = glm::clamp(m_cameraState.zoom, -2.0f, 2.0f);
	updateViewMatrix();
}

void Application::initGui(){
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOther(Window, true);
	ImGui_ImplWGPU_Init(Device, 3, swapChainFormat, depthTextureFormat);
}

void Application::updateGui(wgpu::RenderPassEncoder renderPass){
// Start the Dear ImGui frame
	ImGui_ImplWGPU_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	{ // Build our UI
		static float f = 0.0f;
		static int counter = 0;
		static bool show_demo_window = true;
		static bool show_another_window = false;
		static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		ImGui::Begin("Hello, world!");                                // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");                     // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);            // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);                  // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color);       // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                                  // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGuiIO& io = ImGui::GetIO();
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();
	}

	// Draw the UI
	ImGui::EndFrame();
	// Convert the UI defined above into low-level drawing commands
	ImGui::Render();
	// Execute the low-level drawing commands on the WebGPU backend
	ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPass);
}