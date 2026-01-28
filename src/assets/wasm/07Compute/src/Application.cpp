#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu.hpp>

#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>
#include <GL/glew.h>
#include <emscripten.h>

#include <imgui.h>
#include <imgui_impl_wgpu.h>
#include <imgui_impl_glfw.h>

#include <stb_image.h>

#include <States/Shape.h>

#include "WebGpuUtils.h"
#include "Application.h"
#include "ResourceManager.h"
#include "save_texture.h"

GLFWwindow* Application::Window = nullptr;
StateMachine* Application::Machine = nullptr;
wgpu::Device Application::Device;
wgpu::Queue Application::Queue;
wgpu::Surface Application::Surface;

int Application::Width;
int Application::Height;
double Application::Time;

void Application::MessageLopp(void *arg) {
  Application* application  = reinterpret_cast<Application*>(arg);

  Time = glfwGetTime();
  application->dt = float(Time - application->last);
  application->last = Time;

  application->messageLopp();
}

// Equivalent of std::bit_width that is available from C++20 onward
uint32_t bit_width(uint32_t m) {
	if (m == 0) return 0;
	else { uint32_t w = 0; while (m >>= 1) ++w; return w; }
}

uint32_t getMaxMipLevelCount(const wgpu::Extent3D& textureSize) {
	return bit_width(std::max(textureSize.width, textureSize.height));
}

// == GLFW Callbacks == //

void onWindowResize(GLFWwindow* window, int width, int height) {
	(void)width; (void)height;
	auto pApp = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	if (pApp != nullptr) pApp->onResize();
}


Application::Application(float& dt, float& fdt) : fdt(fdt), dt(dt), last(0.0) {
  Application::Width = 1024;
  Application::Height = 480;
  initWindow();
  initWebGPU();
  initSwapChain();
  initBindGroupLayout();
  initComputePipeline();
  initBuffers();
  initTextures();
  initTextureViews();
  initBindGroup();
  initGui();
  initStates();
  
}

Application::~Application() {
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
    std::cerr << "Could not initialize WebGPU!" << std::endl;
    return;
  }

	// Create surface and adapter
	std::cout << "Requesting adapter..." << std::endl;
	Surface = glfwGetWGPUSurface(instance, Window);
	wgpu::RequestAdapterOptions adapterOpts = {};
	adapterOpts.compatibleSurface = nullptr;
	adapterOpts.compatibleSurface = Surface;
	adapter = instance.requestAdapter(adapterOpts);
	std::cout << "Got adapter: " << adapter << std::endl;

	std::cout << "Requesting device..." << std::endl;
	wgpu::SupportedLimits supportedLimits;
	adapter.getLimits(&supportedLimits);
	wgpu::RequiredLimits requiredLimits = wgpu::Default;
	requiredLimits.limits.maxVertexAttributes = 6;
	requiredLimits.limits.maxVertexBuffers = 1;
	requiredLimits.limits.maxBindGroups = 2;
	requiredLimits.limits.maxUniformBuffersPerShaderStage = 2;
	requiredLimits.limits.maxUniformBufferBindingSize = 16 * 4 * sizeof(float);
	requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;
	requiredLimits.limits.maxBufferSize = 80;
	requiredLimits.limits.maxTextureDimension1D = 2048;
	requiredLimits.limits.maxTextureDimension2D = 2048;
	requiredLimits.limits.maxTextureDimension3D = 2048;
	requiredLimits.limits.maxTextureArrayLayers = 1;
	requiredLimits.limits.maxSampledTexturesPerShaderStage = 3;
	requiredLimits.limits.maxSamplersPerShaderStage = 1;
	requiredLimits.limits.maxVertexBufferArrayStride = 68;
	requiredLimits.limits.maxInterStageShaderComponents = WGPU_LIMIT_U32_UNDEFINED;
	requiredLimits.limits.maxStorageBuffersPerShaderStage = 2;
	requiredLimits.limits.maxComputeWorkgroupSizeX = 8;
	requiredLimits.limits.maxComputeWorkgroupSizeY = 8;
	requiredLimits.limits.maxComputeWorkgroupSizeZ = 1;
	requiredLimits.limits.maxComputeInvocationsPerWorkgroup = 64;
	requiredLimits.limits.maxComputeWorkgroupsPerDimension = 2;
	requiredLimits.limits.maxStorageBufferBindingSize = 0;
	requiredLimits.limits.maxStorageTexturesPerShaderStage = 1;

	// Create device
	wgpu::DeviceDescriptor deviceDesc = {};
	deviceDesc.label = "My Device";
	deviceDesc.requiredFeatureCount = 0;
	deviceDesc.requiredLimits = &requiredLimits;
	deviceDesc.defaultQueue.label = "The default queue";
	Device = adapter.requestDevice(deviceDesc);
	std::cout << "Got device: " << Device << std::endl;

	// Add an error callback for more debug info
	m_uncapturedErrorCallback = Device.setUncapturedErrorCallback([](wgpu::ErrorType type, char const* message) {
		std::cout << "Device error: type " << type;
		if (message) std::cout << " (message: " << message << ")";
		std::cout << std::endl;
	});

	//m_deviceLostCallback = Device.setDeviceLostCallback([](wgpu::DeviceLostReason reason, char const* message) {
	//	std::cout << "Device lost: reason " << reason;
	//	if (message) std::cout << " (message: " << message << ")";
	//	std::cout << std::endl;
	//});

	Queue = Device.getQueue();

	// Add window callbacks
	glfwSetWindowUserPointer(Window, this);
	glfwSetFramebufferSizeCallback(Window, onWindowResize);

}

void Application::terminateDevice() {
#ifndef WEBGPU_BACKEND_WGPU
  wgpuQueueRelease(Queue);
#endif

  wgpuDeviceRelease(Device);
  wgpuInstanceRelease(instance);
}

bool Application::isRunning(){
  messageLopp();
  return glfwWindowShouldClose(Window);
}

void Application::messageLopp(){
  glfwPollEvents();

  wgpu::TextureView nextTexture = SwapChain.getCurrentTextureView();
	if (!nextTexture) {
		std::cerr << "Cannot acquire next swap chain texture" << std::endl;
		return;
	}

	wgpu::RenderPassDescriptor renderPassDesc = wgpu::Default;
	wgpu::RenderPassColorAttachment renderPassColorAttachment = {};
	renderPassColorAttachment.view = nextTexture;
	renderPassColorAttachment.resolveTarget = nullptr;
	renderPassColorAttachment.loadOp = wgpu::LoadOp::Clear;
	renderPassColorAttachment.storeOp = wgpu::StoreOp::Store;
	renderPassColorAttachment.clearValue = wgpu::Color{ 0.0, 0.0, 0.0, 1.0 };
	renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
	renderPassDesc.colorAttachmentCount = 1;
	renderPassDesc.colorAttachments = &renderPassColorAttachment;

	wgpu::CommandEncoder encoder = Device.createCommandEncoder(wgpu::Default);
	wgpu::RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);
	onGui(renderPass, m_force);
	renderPass.end();

	wgpu::CommandBuffer command = encoder.finish(wgpu::CommandBufferDescriptor{});
	Queue.submit(command);

#ifndef __EMSCRIPTEN__
	SwapChain.present();
#endif

#if !defined(WEBGPU_BACKEND_WGPU)
	wgpuCommandBufferRelease(command);
	wgpuCommandEncoderRelease(encoder);
	wgpuRenderPassEncoderRelease(renderPass);
#endif

	wgpuTextureViewRelease(nextTexture);
#ifdef WEBGPU_BACKEND_WGPU
	wgpuQueueSubmit(Queue, 0, nullptr);
#else
	//Device.tick();
#endif
  if (m_shouldCompute) {
    onCompute();
  }
}

void Application::initStates(){
    Machine = new StateMachine(dt, fdt);
    Machine->addStateAtTop(new Shape(*Machine));
}

void Application::onResize() {
	
}

void Application::initGui() {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOther(Window, true);
	ImGui_ImplWGPU_Init(Device, 3, swapChainFormat, wgpu::TextureFormat::Undefined);
}

void Application::terminateGui() {
	ImGui_ImplWGPU_Shutdown();
	ImGui_ImplGlfw_Shutdown();
}

void Application::onGui(wgpu::RenderPassEncoder renderPass, bool force){
ImGui_ImplWGPU_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Display images
	{
		ImDrawList* drawList = ImGui::GetBackgroundDrawList();
		float offset = 0.0f;
		float width = 0.0f;

		// Input image
		width = inputTexture.getWidth() * m_settings.scale;
		drawList->AddImage((ImTextureID)inputTextureView, { offset, 0 }, {
			offset + width,
			inputTexture.getHeight() * m_settings.scale
		});
		offset += width;

		// Output image
		width = outputTexture.getWidth() * m_settings.scale;
		drawList->AddImage((ImTextureID)outputTextureView, { offset, 0 }, {
			offset + width,
			outputTexture.getHeight() * m_settings.scale
		});
		offset += width;
	}

	bool changed = false;
	ImGui::Begin("Parameters");
	float minimum = m_parameters.normalize  ? 0.0f : -2.0f;
	float maximum = m_parameters.normalize ? 4.0f : 2.0f;
	changed = ImGui::Combo("Filter Type", (int*)&m_parameters.filterType, "Sum\0Maximum\0Minimum\0") || changed;
	changed = ImGui::SliderFloat3("Kernel X", glm::value_ptr(m_parameters.kernel[0]), minimum, maximum) || changed;
	changed = ImGui::SliderFloat3("Kernel Y", glm::value_ptr(m_parameters.kernel[1]), minimum, maximum) || changed;
	changed = ImGui::SliderFloat3("Kernel Z", glm::value_ptr(m_parameters.kernel[2]), minimum, maximum) || changed;
	changed = ImGui::Checkbox("Normalize", &m_parameters.normalize) || changed;
	ImGui::End();

	if (changed) {
		float sum = glm::dot(glm::vec4(1.0, 1.0, 1.0, 0.0), m_parameters.kernel * glm::vec3(1.0));
		m_uniforms.kernel = m_parameters.normalize && std::abs(sum) > 1e-6
			? m_parameters.kernel / sum
			: m_parameters.kernel;
		m_uniforms.filterType = (uint32_t)m_parameters.filterType;
	}
	m_shouldCompute = changed || force;

	ImGui::Begin("Settings");
	ImGui::SliderFloat("Scale", &m_settings.scale, 0.0f, 2.0f);
	/*if (ImGui::Button("Save Output")) {
		std::filesystem::path path = "res/textures/output.png";
		saveTexture(path, Device, outputTexture, 0);
	}*/
	ImGui::End();

	ImGui::Render();
	ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPass);
}

void Application::initSwapChain() {
#ifdef WEBGPU_BACKEND_DAWN
	swapChainFormat = wgpu::TextureFormat::BGRA8Unorm;
#else
	swapChainFormat = Surface.getPreferredFormat(adapter);
#endif

	int width, height;
	glfwGetFramebufferSize(Window, &width, &height);

	std::cout << "Creating swapchain..." << std::endl;
	swapChainDesc.width = (uint32_t)width;
	swapChainDesc.height = (uint32_t)height;
	swapChainDesc.usage = wgpu::TextureUsage::RenderAttachment;
	swapChainDesc.format = swapChainFormat;
	swapChainDesc.presentMode = wgpu::PresentMode::Fifo;
	SwapChain = Device.createSwapChain(Surface, swapChainDesc);
	std::cout << "Swapchain: " << SwapChain << std::endl;
}

void Application::terminateSwapChain() {
	wgpuSwapChainRelease(SwapChain);
}

void Application::initBuffers() {
	wgpu::BufferDescriptor desc;
	desc.label = "Uniforms";
	desc.mappedAtCreation = false;
	desc.size = sizeof(Uniforms);
	desc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
	uniformBuffer = Device.createBuffer(desc);
}

void Application::terminateBuffers() {
	uniformBuffer.destroy();
	wgpuBufferRelease(uniformBuffer);
}

void Application::initTextures() {
	// Load image data
	int width, height, channels;
	uint8_t* pixelData = stbi_load("res/textures/input.jpg", &width, &height, &channels, 4 /* force 4 channels */);
	if (nullptr == pixelData) throw std::runtime_error("Could not load input texture!");
	wgpu::Extent3D textureSize = { (uint32_t)width, (uint32_t)height, 1 };

	// Create texture
	wgpu::TextureDescriptor textureDesc;
	textureDesc.dimension = wgpu::TextureDimension::_2D;
	textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
	textureDesc.size = textureSize;
	textureDesc.sampleCount = 1;
	textureDesc.viewFormatCount = 0;
	textureDesc.viewFormats = nullptr;
	textureDesc.mipLevelCount = 1;

	textureDesc.label = "Input";
	textureDesc.usage = (
		wgpu::TextureUsage::TextureBinding | // to bind the texture in a shader
		wgpu::TextureUsage::CopyDst // to upload the input data
	);
	inputTexture = Device.createTexture(textureDesc);

	textureDesc.label = "Output";
	textureDesc.usage = (
		wgpu::TextureUsage::TextureBinding | // to bind the texture in a shader
		wgpu::TextureUsage::StorageBinding | // to write the texture in a shader
		wgpu::TextureUsage::CopySrc // to save the output data
	);
	outputTexture = Device.createTexture(textureDesc);

	// Upload texture data for MIP level 0 to the GPU
	wgpu::ImageCopyTexture destination;
	destination.texture = inputTexture;
	destination.origin = { 0, 0, 0 };
	destination.aspect = wgpu::TextureAspect::All;
	destination.mipLevel = 0;
	wgpu::TextureDataLayout source;
	source.offset = 0;
	source.bytesPerRow = 4 * textureSize.width;
	source.rowsPerImage = textureSize.height;
	Queue.writeTexture(destination, pixelData, (size_t)(4 * width * height), source, textureSize);

	// Free CPU-side data
	stbi_image_free(pixelData);
}

void Application::terminateTextures() {
	inputTexture.destroy();
	wgpuTextureRelease(inputTexture);

	outputTexture.destroy();
	wgpuTextureRelease(outputTexture);
}

void Application::initTextureViews() {
	wgpu::TextureViewDescriptor textureViewDesc;
	textureViewDesc.aspect = wgpu::TextureAspect::All;
	textureViewDesc.baseArrayLayer = 0;
	textureViewDesc.arrayLayerCount = 1;
	textureViewDesc.dimension = wgpu::TextureViewDimension::_2D;
	textureViewDesc.format = wgpu::TextureFormat::RGBA8Unorm;
	textureViewDesc.mipLevelCount = 1;
	textureViewDesc.baseMipLevel = 0;

	textureViewDesc.label = "Input";
	inputTextureView = inputTexture.createView(textureViewDesc);

	textureViewDesc.label = "Output";
	outputTextureView = outputTexture.createView(textureViewDesc);
}

void Application::terminateTextureViews() {
	wgpuTextureViewRelease(inputTextureView);
	wgpuTextureViewRelease(outputTextureView);
}

void Application::initBindGroup() {
	// Create compute bind group
	std::vector<wgpu::BindGroupEntry> entries(3, wgpu::Default);

	// Input buffer
	entries[0].binding = 0;
	entries[0].textureView = inputTextureView;

	// Output buffer
	entries[1].binding = 1;
	entries[1].textureView = outputTextureView;

	// Uniforms
	entries[2].binding = 2;
	entries[2].buffer = uniformBuffer;
	entries[2].offset = 0;
	entries[2].size = sizeof(Uniforms);

	wgpu::BindGroupDescriptor bindGroupDesc;
	bindGroupDesc.layout = bindGroupLayout;
	bindGroupDesc.entryCount = (uint32_t)entries.size();
	bindGroupDesc.entries = (WGPUBindGroupEntry*)entries.data();
	bindGroup = Device.createBindGroup(bindGroupDesc);
}

void Application::terminateBindGroup() {
	wgpuBindGroupRelease(bindGroup);
}

void Application::initBindGroupLayout() {
	// Create bind group layout
	std::vector<wgpu::BindGroupLayoutEntry> bindings(3, wgpu::Default);

	// Input image: MIP level 0 of the texture
	bindings[0].binding = 0;
	bindings[0].texture.sampleType = wgpu::TextureSampleType::Float;
	bindings[0].texture.viewDimension = wgpu::TextureViewDimension::_2D;
	bindings[0].visibility = wgpu::ShaderStage::Compute;

	// Output image: MIP level 1 of the texture
	bindings[1].binding = 1;
	bindings[1].storageTexture.access = wgpu::StorageTextureAccess::WriteOnly;
	bindings[1].storageTexture.format = wgpu::TextureFormat::RGBA8Unorm;
	bindings[1].storageTexture.viewDimension = wgpu::TextureViewDimension::_2D;
	bindings[1].visibility = wgpu::ShaderStage::Compute;

	// Uniforms
	bindings[2].binding = 2;
	bindings[2].buffer.type = wgpu::BufferBindingType::Uniform;
	bindings[2].buffer.minBindingSize = sizeof(Uniforms);
	bindings[2].visibility = wgpu::ShaderStage::Compute;

	wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc;
	bindGroupLayoutDesc.entryCount = (uint32_t)bindings.size();
	bindGroupLayoutDesc.entries = bindings.data();
	bindGroupLayout = Device.createBindGroupLayout(bindGroupLayoutDesc);
}

void Application::terminateBindGroupLayout() {
	wgpuBindGroupLayoutRelease(bindGroupLayout);
}

void Application::initComputePipeline() {
	// Load compute shader
	wgpu::ShaderModule computeShaderModule = ResourceManager::loadShaderModule("res/shader/compute-shader.wgsl", Device);

	// Create compute pipeline layout
	wgpu::PipelineLayoutDescriptor pipelineLayoutDesc;
	pipelineLayoutDesc.bindGroupLayoutCount = 1;
	pipelineLayoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&bindGroupLayout;
	pipelineLayout = Device.createPipelineLayout(pipelineLayoutDesc);

	// Create compute pipeline
	wgpu::ComputePipelineDescriptor computePipelineDesc;
	computePipelineDesc.compute.constantCount = 0;
	computePipelineDesc.compute.constants = nullptr;
	//computePipelineDesc.compute.entryPoint = "computeFilter";
	computePipelineDesc.compute.entryPoint = "computeSobelX";
	computePipelineDesc.compute.module = computeShaderModule;
	computePipelineDesc.layout = pipelineLayout;
	pipeline = Device.createComputePipeline(computePipelineDesc);
}

void Application::terminateComputePipeline() {
	wgpuComputePipelineRelease(pipeline);
	wgpuPipelineLayoutRelease(pipelineLayout);
}

void Application::onCompute() {
	std::cout << "Computing..." << std::endl;

	// Update uniforms
	Queue.writeBuffer(uniformBuffer, 0, &m_uniforms, sizeof(Uniforms));

	// Initialize a command encoder
	wgpu::CommandEncoderDescriptor encoderDesc = wgpu::Default;
	wgpu::CommandEncoder encoder = Device.createCommandEncoder(encoderDesc);

	// Create compute pass
	wgpu::ComputePassDescriptor computePassDesc;
	//computePassDesc.timestampWriteCount = 0;
	computePassDesc.timestampWrites = nullptr;
	wgpu::ComputePassEncoder computePass = encoder.beginComputePass(computePassDesc);

	computePass.setPipeline(pipeline);

	for (uint32_t i = 0; i < 1; ++i) {
		computePass.setBindGroup(0, bindGroup, 0, nullptr);

		uint32_t invocationCountX = inputTexture.getWidth();
		uint32_t invocationCountY = inputTexture.getHeight();
		uint32_t workgroupSizePerDim = 8;
		// This ceils invocationCountX / workgroupSizePerDim
		uint32_t workgroupCountX = (invocationCountX + workgroupSizePerDim - 1) / workgroupSizePerDim;
		uint32_t workgroupCountY = (invocationCountY + workgroupSizePerDim - 1) / workgroupSizePerDim;
		computePass.dispatchWorkgroups(workgroupCountX, workgroupCountY, 1);
	}

	// Finalize compute pass
	computePass.end();

	// Encode and submit the GPU commands
	wgpu::CommandBuffer commands = encoder.finish(wgpu::CommandBufferDescriptor{});
	Queue.submit(commands);

#if !defined(WEBGPU_BACKEND_WGPU)
	wgpuCommandBufferRelease(commands);
	wgpuCommandEncoderRelease(encoder);
	wgpuComputePassEncoderRelease(computePass);
#endif

	m_shouldCompute = false;
	m_force = false;
}