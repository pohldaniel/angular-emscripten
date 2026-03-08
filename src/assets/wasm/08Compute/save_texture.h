#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <filesystem>
#include <string>

#include <webgpu/webgpu.h>
#include <WebGPU/WgpTexture.h>

struct UData {
	bool& done;
	bool& success;
	WGPUBufferDescriptor pixelBufferDesc;
	WGPUBuffer pixelBuffer;
	std::filesystem::path path;
	uint32_t width;
	uint32_t height;
	uint32_t channels;
	uint32_t paddedBytesPerRow;
};

bool saveTexture(const std::filesystem::path path, const WGPUDevice& device, const WgpTexture& texture, int mipLevel) {	
	uint32_t width = texture.getWidth() / (1 << mipLevel);
	uint32_t height = texture.getHeight() / (1 << mipLevel);
	uint32_t channels = 4; // TODO: infer from format
	uint32_t componentByteSize = 1; // TODO: infer from format
	uint32_t bytesPerRow = componentByteSize * channels * width;
	uint32_t paddedBytesPerRow = std::max(256u, bytesPerRow);

	WGPUBufferDescriptor pixelBufferDesc = {};
	pixelBufferDesc.mappedAtCreation = false;
	pixelBufferDesc.usage = WGPUBufferUsage_MapRead | WGPUBufferUsage_CopyDst;
	pixelBufferDesc.size = paddedBytesPerRow * height;
	WGPUBuffer pixelBuffer = wgpuDeviceCreateBuffer(device, &pixelBufferDesc);

	WGPUQueue queue = wgpContext.queue;
	WGPUCommandEncoderDescriptor commandEncoderDesc = {};
	commandEncoderDesc.label = "command_encoder";
	WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(wgpContext.device, &commandEncoderDesc);

	WGPUImageCopyTexture source = {};
	source.texture = texture.getTexture();
	source.mipLevel = mipLevel;
	WGPUImageCopyBuffer destination = {};
	destination.buffer = pixelBuffer;
	destination.layout.bytesPerRow = paddedBytesPerRow;
	destination.layout.offset = 0;
	destination.layout.rowsPerImage = height;
	WGPUExtent3D size = { width, height, 1 };
	wgpuCommandEncoderCopyTextureToBuffer(encoder, &source, &destination, &size);

	WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, NULL);
	wgpuQueueSubmit(queue, 1, &command);

	bool done = false;
	bool success = false;
    UData userdata = {done, success, pixelBufferDesc, pixelBuffer, path, width, height, channels, paddedBytesPerRow};
	auto callbackHandle = [](WGPUBufferMapAsyncStatus status, void* userdata) {
       UData* userData = reinterpret_cast<UData*>(userdata);
	   if (status != WGPUBufferMapAsyncStatus::WGPUBufferMapAsyncStatus_Success) {
			userData->success = false;
		}else {
			const unsigned char* pixelData = (const unsigned char*)wgpuBufferGetConstMappedRange(userData->pixelBuffer, 0, userData->pixelBufferDesc.size);
			int writeSuccess = stbi_write_png(userData->path.string().c_str(), (int)userData->width, (int)userData->height, (int)userData->channels, pixelData, userData->paddedBytesPerRow);

			wgpuBufferUnmap(userData->pixelBuffer);

			userData->success = writeSuccess != 0;
		}
		userData->done = true;
	};
	wgpuBufferMapAsync(pixelBuffer, WGPUMapMode::WGPUMapMode_Read, 0, pixelBufferDesc.size, callbackHandle, (void*)&userdata);

	while (!userdata.done) {
#ifdef WEBGPU_BACKEND_WGPU
		wgpuQueueSubmit(queue, 0, nullptr);
#else
		//device.tick();
#endif
	}
	wgpuBufferDestroy(pixelBuffer);

#ifdef WEBGPU_BACKEND_WGPU
	wgpuBufferDrop(pixelBuffer);
#else
	wgpuBufferRelease(pixelBuffer);
	wgpuCommandEncoderRelease(encoder);
	wgpuCommandBufferRelease(command);
	wgpuQueueRelease(queue);
#endif

	return true;
}
