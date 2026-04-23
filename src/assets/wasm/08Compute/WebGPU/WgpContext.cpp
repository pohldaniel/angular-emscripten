#include <iostream>
#include "WgpContext.h"
#include "../include/Application.h"

#define WGPU_STR(str) { str, sizeof(str) - 1 }

WgpContext wgpContext = {};
std::unordered_map<VertexLayoutSlot, std::vector<WGPUVertexAttribute>> wgpVertexAttributes;
std::unordered_map<VertexLayoutSlot, WGPUVertexBufferLayout> wgpVertexBufferLayouts;

void OnRequestAdapter(WGPURequestAdapterStatus status, WGPUAdapter adapter, char const* message, void* userdata) {
	std::pair<WgpContext&, bool>* userData = reinterpret_cast<std::pair<WgpContext&, bool>*>(userdata);
	if (status == WGPURequestAdapterStatus_Success) {
			userData->first.adapter = adapter;
	} else {
			std::cout << "Could not get WebGPU adapter: " << message << std::endl;
	}
	userData->second = true;
}

void OnRequestDevice(WGPURequestDeviceStatus status, WGPUDevice device, char const* message, void* userdata) {
	std::pair<WgpContext&, bool>* userData = reinterpret_cast<std::pair<WgpContext&, bool>*>(userdata);
	if (status == WGPURequestDeviceStatus_Success) {
			userData->first.device = device;
	} else {
			std::cout << "Could not get WebGPU adapter: " << message << std::endl;
	}
	userData->second = true;
}

void OnErrorDevice(const WGPUDevice* device, WGPUErrorType type, char const* message, void* userdata) {
	std::cout << "Error: " << type << " - message: " << message << "\n";
}

void setDefault(WGPULimits& limits) {
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
	limits.maxInterStageShaderVariables = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxColorAttachments = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxColorAttachmentBytesPerSample = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxComputeWorkgroupStorageSize = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxComputeInvocationsPerWorkgroup = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxComputeWorkgroupSizeX = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxComputeWorkgroupSizeY = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxComputeWorkgroupSizeZ = WGPU_LIMIT_U32_UNDEFINED;
	limits.maxComputeWorkgroupsPerDimension = WGPU_LIMIT_U32_UNDEFINED;	
	limits.maxInterStageShaderComponents = WGPU_LIMIT_U32_UNDEFINED;	
}

void setDefault(WGPUStencilFaceState& stencilFaceState) {
	stencilFaceState.compare = WGPUCompareFunction::WGPUCompareFunction_Always;
	stencilFaceState.failOp = WGPUStencilOperation::WGPUStencilOperation_Keep;
	stencilFaceState.depthFailOp = WGPUStencilOperation::WGPUStencilOperation_Keep;
	stencilFaceState.passOp = WGPUStencilOperation::WGPUStencilOperation_Keep;
}

void setDefault(WGPUDepthStencilState& depthStencilState) {
	depthStencilState.nextInChain = NULL;
	depthStencilState.format = WGPUTextureFormat::WGPUTextureFormat_Undefined;
	depthStencilState.depthWriteEnabled = true;
	depthStencilState.depthCompare = WGPUCompareFunction::WGPUCompareFunction_Less;
	setDefault(depthStencilState.stencilFront);
	setDefault(depthStencilState.stencilBack);
	depthStencilState.stencilReadMask = 0;
	depthStencilState.stencilWriteMask = 0;
	depthStencilState.depthBias = 0;
	depthStencilState.depthBiasSlopeScale = 0.0f;
	depthStencilState.depthBiasClamp = 0.0f;
}

void setDefault(WGPUBindGroupLayoutEntry& bindingLayout) {
	bindingLayout.buffer.nextInChain = NULL;
	bindingLayout.buffer.type = WGPUBufferBindingType_Undefined;
	bindingLayout.buffer.hasDynamicOffset = false;

	bindingLayout.sampler.nextInChain = NULL;
	bindingLayout.sampler.type = WGPUSamplerBindingType_Undefined;

	bindingLayout.storageTexture.nextInChain = NULL;
	bindingLayout.storageTexture.access = WGPUStorageTextureAccess_Undefined;
	bindingLayout.storageTexture.format = WGPUTextureFormat_Undefined;
	bindingLayout.storageTexture.viewDimension = WGPUTextureViewDimension_Undefined;

	bindingLayout.texture.nextInChain = NULL;
	bindingLayout.texture.multisampled = false;
	bindingLayout.texture.sampleType = WGPUTextureSampleType_Undefined;
	bindingLayout.texture.viewDimension = WGPUTextureViewDimension_Undefined;
}

void wgpRequestAdapterSync(WGPUInstance instance, const WGPURequestAdapterOptions* requestAdapterOptions) {	
	std::pair<WgpContext&, bool> userdata = {wgpContext, false};
	wgpuInstanceRequestAdapter(instance, requestAdapterOptions, OnRequestAdapter, (void*)&userdata);

#ifdef __EMSCRIPTEN__
	while (!userdata.second) {
		emscripten_sleep(100);
	}
#endif // __EMSCRIPTEN__
}

void wgpRequestDeviceSync(WGPUAdapter instance, const WGPUDeviceDescriptor* deviceDescriptor) {
	std::pair<WgpContext&, bool> userdata = {wgpContext, false};
	wgpuAdapterRequestDevice(instance, deviceDescriptor, OnRequestDevice, (void*)&userdata);

#ifdef __EMSCRIPTEN__
	while (!userdata.second) {
		emscripten_sleep(100);
	}
#endif // __EMSCRIPTEN__
}

void wgpInit(void* window, uint32_t msaaSampleCount) {
	wgpCreateDevice(window, msaaSampleCount);
}

bool wgpCreateDevice(void* window, uint32_t msaaSampleCount) {
	wgpContext.instance = wgpuCreateInstance(NULL);
	wgpRequestAdapterSync(wgpContext.instance, NULL);

	WGPURequiredLimits requiredLimits = {};
	setDefault(requiredLimits.limits);
	requiredLimits.nextInChain = NULL;
	requiredLimits.limits.maxTextureDimension1D = 2048;
	requiredLimits.limits.maxTextureDimension2D = 2048;
	requiredLimits.limits.maxTextureDimension3D = 2048;
	requiredLimits.limits.maxSamplersPerShaderStage = 1;

	//WGPUUncapturedErrorCallbackInfo errorCallbackInfo = {};
	//errorCallbackInfo.callback = OnErrorDevice;

	WGPUDeviceDescriptor deviceDescriptor = {};
	deviceDescriptor.requiredLimits = &requiredLimits;
	//deviceDescriptor.uncapturedErrorCallbackInfo = errorCallbackInfo;
	deviceDescriptor.nextInChain = NULL;
    deviceDescriptor.label = "device";
    deviceDescriptor.requiredFeatureCount = 0;
    deviceDescriptor.requiredFeatures = NULL;
    deviceDescriptor.defaultQueue.nextInChain = NULL;
    deviceDescriptor.defaultQueue.label = "queue";

    wgpRequestDeviceSync(wgpContext.adapter, &deviceDescriptor);

	WGPUSurfaceDescriptorFromCanvasHTMLSelector surfaceDescriptorFromCanvasHTMLSelector = {};
    surfaceDescriptorFromCanvasHTMLSelector.chain.next = NULL;
    surfaceDescriptorFromCanvasHTMLSelector.chain.sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;
    surfaceDescriptorFromCanvasHTMLSelector.selector = "canvas";

    WGPUSurfaceDescriptor surfaceDescriptor;
    surfaceDescriptor.nextInChain = &surfaceDescriptorFromCanvasHTMLSelector.chain;
    surfaceDescriptor.label = NULL;

    wgpContext.surface = wgpuInstanceCreateSurface(wgpContext.instance, &surfaceDescriptor);
	wgpContext.surfaceCapabilities = { 0 };
	wgpuSurfaceGetCapabilities(wgpContext.surface, wgpContext.adapter, &wgpContext.surfaceCapabilities);
	//wgpContext.colorformat = wgpContext.surfaceCapabilities.formats[0];

	wgpContext.queue = wgpuDeviceGetQueue(wgpContext.device);
	wgpContext.depthTexture = wgpCreateTexture(static_cast<uint32_t>(Application::Width), static_cast<uint32_t>(Application::Height), 1u, WGPUTextureUsage_RenderAttachment, wgpContext.depthformat, 1u, wgpContext.msaaSampleCount, wgpContext.depthformat);
	wgpContext.depthTextureView = wgpCreateTextureView(wgpContext.depthTexture, WGPUTextureAspect::WGPUTextureAspect_All);
    wgpConfigureSurface();

	wgpCreateVertexBufferLayout(VL_P);
	wgpCreateVertexBufferLayout(VL_PTN);
	wgpCreateVertexBufferLayout(VL_PTNC);
	wgpCreateVertexBufferLayout(VL_PTNTB);
	wgpCreateVertexBufferLayout(VL_BATCH);

	wgpContext.addSampler(wgpCreateSampler(WGPUFilterMode_Linear, WGPUAddressMode_ClampToEdge), SS_LINEAR_CLAMP);
	wgpContext.addSampler(wgpCreateSampler(WGPUFilterMode_Linear, WGPUAddressMode_Repeat), SS_LINEAR_REPEAT);
	wgpContext.addSampler(wgpCreateSampler(WGPUFilterMode_Nearest, WGPUAddressMode_ClampToEdge), SS_NEAREST_CLAMP);
	wgpContext.addSampler(wgpCreateSampler(WGPUFilterMode_Nearest, WGPUAddressMode_Repeat), SS_NEAREST_REPEAT);

	wgpContext.setMSAASampleCount(msaaSampleCount);
    return true;
}

void wgpConfigureSurface() {
	wgpContext.config = {};
	wgpContext.config.nextInChain = NULL;
	wgpContext.config.format = wgpContext.colorformat;
	wgpContext.config.width = Application::Width;
	wgpContext.config.height = Application::Height;
	wgpContext.config.usage = WGPUTextureUsage_RenderAttachment;
	wgpContext.config.viewFormatCount = 0;
	wgpContext.config.viewFormats = NULL;
	wgpContext.config.device = wgpContext.device;
	wgpContext.config.presentMode = WGPUPresentMode_Fifo;
	wgpContext.config.alphaMode = wgpContext.surfaceCapabilities.alphaModes[0];
	wgpuSurfaceConfigure(wgpContext.surface, &wgpContext.config);
}

WGPUBuffer wgpCreateBuffer(const void* data, uint32_t size, WGPUBufferUsageFlags bufferUsageFlags) {
	const WGPUDevice& device = wgpContext.device;
    WGPUBufferDescriptor bufferDesc = {};
	bufferDesc.label = "buf";

	if (bufferUsageFlags & WGPUBufferUsage_Uniform)
		bufferDesc.label = "uniform_buf";

	if (bufferUsageFlags & WGPUBufferUsage_Vertex)
		bufferDesc.label = "vertex_buf";

	if (bufferUsageFlags & WGPUBufferUsage_Index)
		bufferDesc.label = "index_buf";

	bufferDesc.size = size;
	bufferDesc.usage = bufferUsageFlags;
	bufferDesc.mappedAtCreation = true;

	WGPUBuffer buffer = wgpuDeviceCreateBuffer(device, &bufferDesc);
	void* mapping = wgpuBufferGetMappedRange(buffer, 0, size);
	memcpy(mapping, data, size);
	wgpuBufferUnmap(buffer);
	return buffer;
}

WGPUBuffer wgpCreateEmptyBuffer(uint32_t size, WGPUBufferUsageFlags bufferUsageFlags) {
	const WGPUDevice& device = wgpContext.device;
	WGPUBufferDescriptor bufferDesc = {};
	bufferDesc.label = "buf";

	if (bufferUsageFlags & WGPUBufferUsage_Uniform)
		bufferDesc.label = "uniform_buf";

	if (bufferUsageFlags & WGPUBufferUsage_Vertex)
		bufferDesc.label = "vertex_buf";

	if (bufferUsageFlags & WGPUBufferUsage_Index)
		bufferDesc.label = "index_buf";

	bufferDesc.size = size;
	bufferDesc.usage = bufferUsageFlags;
	bufferDesc.mappedAtCreation = false;
	return wgpuDeviceCreateBuffer(device, &bufferDesc);
}

WGPUTexture wgpCreateTexture(uint32_t width, uint32_t height, uint32_t depth, WGPUTextureUsageFlags textureUsageFlags, WGPUTextureFormat textureFormat, uint32_t mipLevelCount, uint32_t sampleCount, WGPUTextureFormat viewFormat) {
	const WGPUDevice& device = wgpContext.device;
	WGPUTextureDescriptor textureDescriptor = {};
	textureDescriptor.label = "texture";
	textureDescriptor.dimension = WGPUTextureDimension::WGPUTextureDimension_2D;
	textureDescriptor.size = { width, height, depth };
	textureDescriptor.format = textureFormat;
	textureDescriptor.usage = textureUsageFlags;
	textureDescriptor.mipLevelCount = mipLevelCount;
	textureDescriptor.sampleCount = sampleCount;	
	textureDescriptor.nextInChain = NULL;
	if (viewFormat != WGPUTextureFormat_Undefined) {
		textureDescriptor.viewFormatCount = 1u;
		textureDescriptor.viewFormats = &viewFormat;
	}
	return wgpuDeviceCreateTexture(device, &textureDescriptor);
}

WGPUTextureView wgpCreateTextureView(const WGPUTexture& texture, WGPUTextureAspect aspect) {
	WGPUTextureViewDescriptor textureViewDescriptor = {};
	textureViewDescriptor.label = "texture_view";
	textureViewDescriptor.aspect = aspect;
	textureViewDescriptor.baseArrayLayer = 0u;
	textureViewDescriptor.arrayLayerCount = wgpuTextureGetDepthOrArrayLayers(texture);
	textureViewDescriptor.baseMipLevel = 0u;
	textureViewDescriptor.mipLevelCount = wgpuTextureGetMipLevelCount(texture);
	textureViewDescriptor.dimension = textureViewDescriptor.arrayLayerCount == 6u ? WGPUTextureViewDimension::WGPUTextureViewDimension_Cube : WGPUTextureViewDimension::WGPUTextureViewDimension_2D;
	textureViewDescriptor.format = wgpuTextureGetFormat(texture);
	textureViewDescriptor.nextInChain = NULL;
	return wgpuTextureCreateView(texture, &textureViewDescriptor);
}

WGPUSampler wgpCreateSampler(WGPUFilterMode filterMode, WGPUAddressMode addressMode, uint16_t maxAnisotropy) {
	const WGPUDevice& device = wgpContext.device;
	WGPUSamplerDescriptor samplerDescriptor = {};
	samplerDescriptor.label = "sampler";
	samplerDescriptor.addressModeU = addressMode;
	samplerDescriptor.addressModeV = addressMode;
	samplerDescriptor.addressModeW = addressMode;
	samplerDescriptor.magFilter = filterMode;
	samplerDescriptor.minFilter = filterMode;
	samplerDescriptor.mipmapFilter = (filterMode == WGPUFilterMode_Nearest) ? WGPUMipmapFilterMode_Nearest : WGPUMipmapFilterMode_Linear;
	samplerDescriptor.lodMinClamp = 0.0f;
	samplerDescriptor.lodMaxClamp = 1.0f;
	samplerDescriptor.compare = WGPUCompareFunction_Undefined;
	samplerDescriptor.maxAnisotropy = maxAnisotropy;
	return wgpuDeviceCreateSampler(device, &samplerDescriptor);
}

WGPUShaderModule wgpCreateShader(std::string path) {
	std::ifstream file(path);
	if (!file.is_open()) {
		return NULL;
	}
	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	std::string shaderSource(size, ' ');
	file.seekg(0);
	file.read(shaderSource.data(), size);

	WGPUShaderModuleWGSLDescriptor shaderModuleWGSLDescriptor = {};
	shaderModuleWGSLDescriptor.chain.next = NULL;
	shaderModuleWGSLDescriptor.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
	shaderModuleWGSLDescriptor.code = shaderSource.c_str();

	WGPUShaderModuleDescriptor shaderModuleDescriptor = {};
	shaderModuleDescriptor.nextInChain = &shaderModuleWGSLDescriptor.chain;

    return wgpuDeviceCreateShaderModule(wgpContext.device, &shaderModuleDescriptor);
}

void wgpCreateVertexBufferLayout(VertexLayoutSlot slot) {
	if (wgpVertexBufferLayouts.count(VL_P) == 0 && slot == VL_P) {
		std::vector<WGPUVertexAttribute>& wgpVertexAttribute = wgpVertexAttributes[VL_P];
		wgpVertexAttribute.resize(1);

		wgpVertexAttribute[0].shaderLocation = 0;
		wgpVertexAttribute[0].format = WGPUVertexFormat::WGPUVertexFormat_Float32x3;
		wgpVertexAttribute[0].offset = 0;
		
		WGPUVertexBufferLayout wgpVertexBufferLayout = {};
		wgpVertexBufferLayout.attributeCount = (uint32_t)wgpVertexAttribute.size();
		wgpVertexBufferLayout.attributes = wgpVertexAttribute.data();
		wgpVertexBufferLayout.arrayStride = 3 * sizeof(float);
		wgpVertexBufferLayout.stepMode = WGPUVertexStepMode::WGPUVertexStepMode_Vertex;
		wgpVertexBufferLayouts.emplace(VL_P, wgpVertexBufferLayout);

	}else if (wgpVertexBufferLayouts.count(VL_PTN) == 0 && slot == VL_PTN) {
		std::vector<WGPUVertexAttribute>& wgpVertexAttribute = wgpVertexAttributes[VL_PTN];
		wgpVertexAttribute.resize(3);

		wgpVertexAttribute[0].shaderLocation = 0;
		wgpVertexAttribute[0].format = WGPUVertexFormat::WGPUVertexFormat_Float32x3;
		wgpVertexAttribute[0].offset = 0;

		wgpVertexAttribute[1].shaderLocation = 1;
		wgpVertexAttribute[1].format = WGPUVertexFormat::WGPUVertexFormat_Float32x2;
		wgpVertexAttribute[1].offset = 3 * sizeof(float);

		wgpVertexAttribute[2].shaderLocation = 2;
		wgpVertexAttribute[2].format = WGPUVertexFormat::WGPUVertexFormat_Float32x3;
		wgpVertexAttribute[2].offset = 5 * sizeof(float);

		WGPUVertexBufferLayout wgpVertexBufferLayout = {};
		wgpVertexBufferLayout.attributeCount = (uint32_t)wgpVertexAttribute.size();
		wgpVertexBufferLayout.attributes = wgpVertexAttribute.data();
		wgpVertexBufferLayout.arrayStride = 8 * sizeof(float);
		wgpVertexBufferLayout.stepMode = WGPUVertexStepMode::WGPUVertexStepMode_Vertex;
		wgpVertexBufferLayouts.emplace(VL_PTN, wgpVertexBufferLayout);

	}else if (wgpVertexBufferLayouts.count(VL_PTNC) == 0 && slot == VL_PTNC) {
		std::vector<WGPUVertexAttribute>& wgpVertexAttribute = wgpVertexAttributes[VL_PTNC];
		wgpVertexAttribute.resize(4);

		wgpVertexAttribute[0].shaderLocation = 0;
		wgpVertexAttribute[0].format = WGPUVertexFormat::WGPUVertexFormat_Float32x3;
		wgpVertexAttribute[0].offset = 0;

		wgpVertexAttribute[1].shaderLocation = 1;
		wgpVertexAttribute[1].format = WGPUVertexFormat::WGPUVertexFormat_Float32x2;
		wgpVertexAttribute[1].offset = 3 * sizeof(float);

		wgpVertexAttribute[2].shaderLocation = 2;
		wgpVertexAttribute[2].format = WGPUVertexFormat::WGPUVertexFormat_Float32x3;
		wgpVertexAttribute[2].offset = 5 * sizeof(float);

		wgpVertexAttribute[3].shaderLocation = 3;
		wgpVertexAttribute[3].format = WGPUVertexFormat::WGPUVertexFormat_Float32x4;
		wgpVertexAttribute[3].offset = 8 * sizeof(float);

		WGPUVertexBufferLayout wgpVertexBufferLayout = {};
		wgpVertexBufferLayout.attributeCount = (uint32_t)wgpVertexAttribute.size();
		wgpVertexBufferLayout.attributes = wgpVertexAttribute.data();
		wgpVertexBufferLayout.arrayStride = 12 * sizeof(float);
		wgpVertexBufferLayout.stepMode = WGPUVertexStepMode::WGPUVertexStepMode_Vertex;
		wgpVertexBufferLayouts.emplace(VL_PTNC, wgpVertexBufferLayout);

	}else if (wgpVertexBufferLayouts.count(VL_PTNTB) == 0 && slot == VL_PTNTB) {
		std::vector<WGPUVertexAttribute>& wgpVertexAttribute = wgpVertexAttributes[VL_PTNTB];
		wgpVertexAttribute.resize(5);

		wgpVertexAttribute[0].shaderLocation = 0;
		wgpVertexAttribute[0].format = WGPUVertexFormat::WGPUVertexFormat_Float32x3;
		wgpVertexAttribute[0].offset = 0;

		wgpVertexAttribute[1].shaderLocation = 1;
		wgpVertexAttribute[1].format = WGPUVertexFormat::WGPUVertexFormat_Float32x2;
		wgpVertexAttribute[1].offset = 3 * sizeof(float);

		wgpVertexAttribute[2].shaderLocation = 2;
		wgpVertexAttribute[2].format = WGPUVertexFormat::WGPUVertexFormat_Float32x3;
		wgpVertexAttribute[2].offset = 5 * sizeof(float);

		wgpVertexAttribute[3].shaderLocation = 3;
		wgpVertexAttribute[3].format = WGPUVertexFormat::WGPUVertexFormat_Float32x3;
		wgpVertexAttribute[3].offset = 8 * sizeof(float);

		wgpVertexAttribute[4].shaderLocation = 4;
		wgpVertexAttribute[4].format = WGPUVertexFormat::WGPUVertexFormat_Float32x3;
		wgpVertexAttribute[4].offset = 11 * sizeof(float);

		WGPUVertexBufferLayout wgpVertexBufferLayout = {};
		wgpVertexBufferLayout.attributeCount = (uint32_t)wgpVertexAttribute.size();
		wgpVertexBufferLayout.attributes = wgpVertexAttribute.data();
		wgpVertexBufferLayout.arrayStride = 14 * sizeof(float);
		wgpVertexBufferLayout.stepMode = WGPUVertexStepMode::WGPUVertexStepMode_Vertex;
		wgpVertexBufferLayouts.emplace(VL_PTNTB, wgpVertexBufferLayout);
		
	}else if (wgpVertexBufferLayouts.count(VL_BATCH) == 0 && slot == VL_BATCH) {
		std::vector<WGPUVertexAttribute>& wgpVertexAttribute = wgpVertexAttributes[VL_BATCH];
		wgpVertexAttribute.resize(4);

		wgpVertexAttribute[0].shaderLocation = 0u;
		wgpVertexAttribute[0].format = WGPUVertexFormat::WGPUVertexFormat_Float32x3;
		wgpVertexAttribute[0].offset = 0u;

		wgpVertexAttribute[1].shaderLocation = 1u;
		wgpVertexAttribute[1].format = WGPUVertexFormat::WGPUVertexFormat_Float32x2;
		wgpVertexAttribute[1].offset = 3 * sizeof(float);

		wgpVertexAttribute[2].shaderLocation = 2u;
		wgpVertexAttribute[2].format = WGPUVertexFormat::WGPUVertexFormat_Float32x4;
		wgpVertexAttribute[2].offset = 5 * sizeof(float);

		wgpVertexAttribute[3].shaderLocation = 3u;
		wgpVertexAttribute[3].format = WGPUVertexFormat::WGPUVertexFormat_Uint32;
		wgpVertexAttribute[3].offset = 9 * sizeof(float);

		WGPUVertexBufferLayout wgpVertexBufferLayout = {};
		wgpVertexBufferLayout.attributeCount = (uint32_t)wgpVertexAttribute.size();
		wgpVertexBufferLayout.attributes = wgpVertexAttribute.data();
		wgpVertexBufferLayout.arrayStride = 9 * sizeof(float) + sizeof(unsigned int);
		wgpVertexBufferLayout.stepMode = WGPUVertexStepMode::WGPUVertexStepMode_Vertex;
		wgpVertexBufferLayouts.emplace(VL_BATCH, wgpVertexBufferLayout);
	}
}

void wgpPipelineLayoutsRelease() {
	for (auto& it : wgpContext.pipelineLayouts) {
		wgpuPipelineLayoutRelease(it.second);
	}

	wgpContext.pipelineLayouts.clear();
	wgpContext.pipelineLayouts.rehash(0u);
}

void wgpPipelinesRelease() {
	for (auto& it : wgpContext.renderPipelines) {
		WGPUBindGroupLayout bindGroupLayout = wgpuRenderPipelineGetBindGroupLayout(it.second, 0);
		wgpuBindGroupLayoutRelease(bindGroupLayout);
		wgpuRenderPipelineRelease(it.second);
	}

	wgpContext.renderPipelines.clear();
	wgpContext.renderPipelines.rehash(0u);

	for (auto& it : wgpContext.computePipelines) {
		WGPUBindGroupLayout bindGroupLayout = wgpuComputePipelineGetBindGroupLayout(it.second, 0);
		wgpuBindGroupLayoutRelease(bindGroupLayout);
		wgpuComputePipelineRelease(it.second);
	}

	wgpContext.computePipelines.clear();
	wgpContext.computePipelines.rehash(0u);
}

void wgpSamplersRelease() {
	for (auto& it : wgpContext.samplers) {
		wgpuSamplerRelease(it.second);
	}

	wgpContext.samplers.clear();
	wgpContext.samplers.rehash(0u);
}

void wgpShaderModulesRelease() {
	for (auto& it : wgpContext.shaderModules) {
		wgpuShaderModuleRelease(it.second);
	}

	wgpContext.shaderModules.clear();
	wgpContext.shaderModules.rehash(0u);
}

void wgpShutDown() {
	wgpPipelineLayoutsRelease();
	wgpPipelinesRelease();
	wgpSamplersRelease();
	wgpShaderModulesRelease();

	wgpuTextureViewRelease(wgpContext.depthTextureView);
	wgpContext.depthTextureView = NULL;

	wgpuTextureDestroy(wgpContext.depthTexture);
	wgpuTextureRelease(wgpContext.depthTexture);
	wgpContext.depthTexture = NULL;

	if (wgpContext.msaaTextureView) {
		wgpuTextureViewRelease(wgpContext.msaaTextureView);
		wgpContext.msaaTextureView = NULL;
	}

	if (wgpContext.msaaTexture) {
		wgpuTextureDestroy(wgpContext.msaaTexture);
		wgpuTextureRelease(wgpContext.msaaTexture);
		wgpContext.msaaTexture = NULL;
	}

	wgpuQueueRelease(wgpContext.queue);
	wgpContext.queue = NULL;

	wgpuAdapterRelease(wgpContext.adapter);
	wgpContext.adapter = NULL;

	wgpuSurfaceRelease(wgpContext.surface);
	wgpContext.surface = NULL;

	wgpuInstanceRelease(wgpContext.instance);
	wgpContext.instance = NULL;

	wgpuDeviceDestroy(wgpContext.device);
	wgpuDeviceRelease(wgpContext.device);
	wgpContext.device = NULL;
}

void wgpResize(uint32_t width, uint32_t height) {
	if (wgpContext.surface) {
		wgpuTextureViewRelease(wgpContext.depthTextureView);
		wgpuTextureDestroy(wgpContext.depthTexture);
		wgpuTextureRelease(wgpContext.depthTexture);

		wgpContext.depthTexture = wgpCreateTexture(width, height, 1u, WGPUTextureUsage_RenderAttachment, wgpContext.depthformat, 1u, wgpContext.msaaSampleCount, wgpContext.depthformat);
		wgpContext.depthTextureView = wgpCreateTextureView(wgpContext.depthTexture, WGPUTextureAspect::WGPUTextureAspect_All);

		if (wgpContext.msaaSampleCount > 1u) {
			wgpuTextureViewRelease(wgpContext.msaaTextureView);
			wgpuTextureDestroy(wgpContext.msaaTexture);
			wgpuTextureRelease(wgpContext.msaaTexture);

			wgpContext.msaaTexture = wgpCreateTexture(width, height, 1u, WGPUTextureUsage_RenderAttachment, wgpContext.colorformat, 1u, wgpContext.msaaSampleCount, wgpContext.colorformat);
			wgpContext.msaaTextureView = wgpCreateTextureView(wgpContext.msaaTexture, WGPUTextureAspect::WGPUTextureAspect_All);
		}

		wgpContext.config.width = width;
		wgpContext.config.height = height;
		wgpuSurfaceConfigure(wgpContext.surface, &wgpContext.config);
	}
}

void wgpToggleVerticalSync() {
	if (wgpContext.surface) {
		wgpContext.config.presentMode = wgpContext.config.presentMode == WGPUPresentMode_Fifo ? WGPUPresentMode_Immediate : WGPUPresentMode_Fifo;
		wgpuSurfaceConfigure(wgpContext.surface, &wgpContext.config);
	}
}

void wgpSetSurfaceColorFormat(WGPUTextureFormat textureFormat) {
	if (wgpContext.surface) {
		wgpContext.colorformat = textureFormat;
		wgpContext.config.format = wgpContext.colorformat;
		wgpuSurfaceConfigure(wgpContext.surface, &wgpContext.config);

		if (wgpContext.msaaSampleCount > 1u) {
			uint32_t width = wgpuTextureGetWidth(wgpContext.msaaTexture);
			uint32_t height = wgpuTextureGetHeight(wgpContext.msaaTexture);

			wgpuTextureViewRelease(wgpContext.msaaTextureView);
			wgpuTextureDestroy(wgpContext.msaaTexture);
			wgpuTextureRelease(wgpContext.msaaTexture);

			wgpContext.msaaTexture = wgpCreateTexture(width, height, 1u, WGPUTextureUsage_RenderAttachment, wgpContext.colorformat, 1u, wgpContext.msaaSampleCount, wgpContext.colorformat);
			wgpContext.msaaTextureView = wgpCreateTextureView(wgpContext.msaaTexture, WGPUTextureAspect::WGPUTextureAspect_All);
		}
	}
}

void wgpDraw() {
	WGPUSurfaceTexture surfaceTexture;
	wgpuSurfaceGetCurrentTexture(wgpContext.surface, &surfaceTexture);
	switch (surfaceTexture.status) {
		case WGPUSurfaceGetCurrentTextureStatus_Success:
			break;
		case WGPUSurfaceGetCurrentTextureStatus_Timeout:
		case WGPUSurfaceGetCurrentTextureStatus_Outdated:
		case WGPUSurfaceGetCurrentTextureStatus_Lost: {
			if (surfaceTexture.texture != NULL) {
				wgpuTextureRelease(surfaceTexture.texture);
				wgpuSurfaceConfigure(wgpContext.surface, &wgpContext.config);
			}
			return;
		}
		default:
			break;
	}

	WGPUTextureView texureView = wgpuTextureCreateView(surfaceTexture.texture, NULL);
	WGPURenderPassColorAttachment renderPassColorAttachment = {};
	renderPassColorAttachment.view = (wgpContext.msaaSampleCount == 1u) ? texureView  : wgpContext.msaaTextureView;
	renderPassColorAttachment.resolveTarget = (wgpContext.msaaSampleCount == 1u) ? NULL : texureView;
	renderPassColorAttachment.loadOp = WGPULoadOp::WGPULoadOp_Clear;
	renderPassColorAttachment.storeOp = WGPUStoreOp::WGPUStoreOp_Store;
	renderPassColorAttachment.clearValue = wgpContext.clearColor;
	renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
	
	WGPURenderPassDepthStencilAttachment depthStencilAttachment = {};
	depthStencilAttachment.view = wgpContext.depthTextureView;
	depthStencilAttachment.depthClearValue = 1.0f;
	depthStencilAttachment.depthLoadOp = WGPULoadOp::WGPULoadOp_Clear;
	depthStencilAttachment.depthStoreOp = WGPUStoreOp::WGPUStoreOp_Store;
	depthStencilAttachment.depthReadOnly = false;
	depthStencilAttachment.stencilClearValue = 0u;
	depthStencilAttachment.stencilLoadOp = WGPULoadOp::WGPULoadOp_Undefined;
	depthStencilAttachment.stencilStoreOp = WGPUStoreOp::WGPUStoreOp_Undefined;
	depthStencilAttachment.stencilReadOnly = true;

	WGPURenderPassDescriptor renderPassDesc = {};
	renderPassDesc.colorAttachmentCount = 1;
	renderPassDesc.colorAttachments = &renderPassColorAttachment;
	renderPassDesc.depthStencilAttachment = &depthStencilAttachment;
	renderPassDesc.timestampWrites = NULL;

	WGPUCommandEncoderDescriptor commandEncoderDesc = {};
	commandEncoderDesc.label = "command_encoder";
	WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(wgpContext.device, &commandEncoderDesc);
	WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
	wgpContext.OnDraw(renderPass);

	wgpuRenderPassEncoderEnd(renderPass);
	wgpuRenderPassEncoderRelease(renderPass);
	wgpuTextureViewRelease(texureView);

	WGPUCommandBufferDescriptor commandBufferDescriptor = {};
	commandBufferDescriptor.label = "command_buffer";
	WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &commandBufferDescriptor);
	wgpuQueueSubmit(wgpContext.queue, 1, &command );

	wgpuCommandBufferRelease(command);
	wgpuCommandEncoderRelease(encoder);
	
	wgpuTextureRelease(surfaceTexture.texture);
}

void WgpContext::createVertexBufferLayout(VertexLayoutSlot slot) {
	wgpCreateVertexBufferLayout(slot);
}

void WgpContext::addSampler(const WGPUSampler& sampler, SamplerSlot samplerSlot) {
	if (samplers.count(samplerSlot) == 0)
		samplers[samplerSlot] = sampler;
}

const WGPUSampler& WgpContext::getSampler(SamplerSlot samplerSlot) {
	return samplers.at(samplerSlot);
}

void WgpContext::addSahderModule(const std::string& shaderModuleName, const std::string& shaderModulePath) {
	shaderModules[shaderModuleName] = wgpCreateShader(shaderModulePath);
}

const WGPUShaderModule& WgpContext::getShaderModule(std::string shaderModuleName) {
	return shaderModules.at(shaderModuleName);
}

void WgpContext::setClearColor(const WGPUColor& _clearColor) {
	clearColor = _clearColor;
}

void WgpContext::setMSAASampleCount(const uint32_t count) {
	msaaSampleCount = count;

	uint32_t width = wgpuTextureGetWidth(depthTexture);
	uint32_t height = wgpuTextureGetHeight(depthTexture);

	msaaTexture = wgpCreateTexture(width, height, 1u, WGPUTextureUsage_RenderAttachment, colorformat, 1u, msaaSampleCount, colorformat);
	msaaTextureView = wgpCreateTextureView(msaaTexture, WGPUTextureAspect::WGPUTextureAspect_All);

	wgpuTextureViewRelease(depthTextureView);
	wgpuTextureDestroy(depthTexture);
	wgpuTextureRelease(depthTexture);

	depthTexture = wgpCreateTexture(width, height, 1u, WGPUTextureUsage_RenderAttachment, depthformat, 1u, msaaSampleCount, depthformat);
	depthTextureView = wgpCreateTextureView(depthTexture, WGPUTextureAspect::WGPUTextureAspect_All);
}

void WgpContext::createComputePipeline(std::string shaderModuleName, std::string pipelineLayoutName, const std::function<std::vector<WGPUBindGroupLayout>()>& onBindGroupLayouts) {
	std::vector<WGPUBindGroupLayout> bindGroupLayouts = onBindGroupLayouts();

	WGPUPipelineLayoutDescriptor pipelineLayoutDescriptor = {};
	pipelineLayoutDescriptor.bindGroupLayoutCount = bindGroupLayouts.size();
	pipelineLayoutDescriptor.bindGroupLayouts = bindGroupLayouts.data();
	pipelineLayouts[pipelineLayoutName] = wgpuDeviceCreatePipelineLayout(wgpContext.device, &pipelineLayoutDescriptor);

	WGPUComputePipelineDescriptor computePipelineDesc = {};
	computePipelineDesc.layout = pipelineLayouts.at(pipelineLayoutName);
	computePipelineDesc.compute.module = shaderModules.at(shaderModuleName);
	//computePipelineDesc.compute.entryPoint = "computeFilter";
	computePipelineDesc.compute.entryPoint = "computeSobelX";
	computePipelineDesc.compute.constantCount = 0u;
	computePipelineDesc.compute.constants = NULL;
	
	wgpContext.computePipelines[pipelineLayoutName] = wgpuDeviceCreateComputePipeline(wgpContext.device, &computePipelineDesc);
}

void WgpContext::createRenderPipeline(std::string shaderModuleName, std::string pipelineLayoutName, const VertexLayoutSlot vertexLayoutSlot, const std::function<std::vector<WGPUBindGroupLayout>()>& onBindGroupLayouts, uint32_t msaaSampleCount, WGPUPrimitiveTopology primitiveTopology) {
	std::vector<WGPUBindGroupLayout> bindGroupLayouts = onBindGroupLayouts();

	WGPUPipelineLayoutDescriptor pipelineLayoutDescriptor = {};
	pipelineLayoutDescriptor.bindGroupLayoutCount = bindGroupLayouts.size();
	pipelineLayoutDescriptor.bindGroupLayouts = bindGroupLayouts.data();
	pipelineLayouts[pipelineLayoutName] = wgpuDeviceCreatePipelineLayout(wgpContext.device, &pipelineLayoutDescriptor);

	WGPUVertexState vertexState = {};
	vertexState.module = shaderModules.at(shaderModuleName);
	vertexState.entryPoint = "vs_main";
	vertexState.constantCount = 0u;
	vertexState.constants = NULL;
	vertexState.bufferCount = vertexLayoutSlot == VertexLayoutSlot::VL_NONE ? 0u : 1u;
	if(vertexLayoutSlot != VertexLayoutSlot::VL_NONE)
		vertexState.buffers = &wgpVertexBufferLayouts.at(vertexLayoutSlot);

	WGPUBlendState blendState = {};
	blendState.color.srcFactor = WGPUBlendFactor::WGPUBlendFactor_SrcAlpha;
	blendState.color.dstFactor = WGPUBlendFactor::WGPUBlendFactor_OneMinusSrcAlpha;
	blendState.color.operation = WGPUBlendOperation::WGPUBlendOperation_Add;
	blendState.alpha.srcFactor = WGPUBlendFactor::WGPUBlendFactor_Zero;
	blendState.alpha.dstFactor = WGPUBlendFactor::WGPUBlendFactor_One;
	blendState.alpha.operation = WGPUBlendOperation::WGPUBlendOperation_Add;

	WGPUColorTargetState colorTarget = {};
	colorTarget.format = colorformat;
	colorTarget.blend = &blendState;
	colorTarget.writeMask = WGPUColorWriteMask_All;

	WGPUFragmentState fragmentState = {};
	fragmentState.module = shaderModules.at(shaderModuleName);
	fragmentState.entryPoint = "fs_main";
	fragmentState.constantCount = 0;
	fragmentState.constants = NULL;
	fragmentState.targetCount = 1;
	fragmentState.targets = &colorTarget;

	WGPUDepthStencilState depthStencilState = {};
	setDefault(depthStencilState);
	depthStencilState.depthCompare = WGPUCompareFunction::WGPUCompareFunction_Less;
	depthStencilState.depthWriteEnabled = true;
	depthStencilState.format = wgpContext.depthformat;
	depthStencilState.stencilReadMask = 0;
	depthStencilState.stencilWriteMask = 0;

	WGPURenderPipelineDescriptor renderPipelineDescriptor = {};
	renderPipelineDescriptor.layout = pipelineLayouts.at(pipelineLayoutName);
	renderPipelineDescriptor.multisample.count = msaaSampleCount;
	renderPipelineDescriptor.multisample.mask = ~0u;
	renderPipelineDescriptor.multisample.alphaToCoverageEnabled = false;

	renderPipelineDescriptor.vertex = vertexState;
	renderPipelineDescriptor.fragment = &fragmentState;
	renderPipelineDescriptor.depthStencil = &depthStencilState;

	renderPipelineDescriptor.primitive.topology = primitiveTopology;
	renderPipelineDescriptor.primitive.stripIndexFormat = WGPUIndexFormat::WGPUIndexFormat_Undefined;
	renderPipelineDescriptor.primitive.frontFace = WGPUFrontFace::WGPUFrontFace_CCW;
	renderPipelineDescriptor.primitive.cullMode = WGPUCullMode::WGPUCullMode_Back;

	wgpContext.renderPipelines[pipelineLayoutName] = wgpuDeviceCreateRenderPipeline(wgpContext.device, &renderPipelineDescriptor);
}