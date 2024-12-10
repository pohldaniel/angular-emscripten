#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>
#include <GL/glew.h>
#include "webgpu-utils.h"
#include "Application.h"
#include <emscripten.h>

GLFWwindow* Application::Window = nullptr;
int Application::Width;
int Application::Height;

void Application::MessageLopp(void *arg) {
  Application* application  = reinterpret_cast<Application*>(arg);
  application->messageLopp();
}

Application::Application(){
  Application::Width = 512;
  Application::Height = 512;
  initWindow();
  initWebGPU();
}

Application::~Application() {
  wgpuSurfaceUnconfigure(surface);
  wgpuQueueRelease(queue);
  wgpuSurfaceRelease(surface);
  wgpuDeviceRelease(device);
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
  instance = wgpuCreateInstance(nullptr);
  adapter = requestAdapterSync(instance, nullptr);

  /*WGPUSurfaceDescriptorFromCanvasHTMLSelector canvasDesc = {};
  canvasDesc.chain.next = NULL;
  canvasDesc.chain.sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;
  canvasDesc.selector = "#canvas";

  WGPUSurfaceDescriptor surfaceDesc = {};
  surfaceDesc.nextInChain = &canvasDesc.chain;
  surfaceDesc.label = NULL;
  surface = wgpuInstanceCreateSurface(instance, &surfaceDesc);*/
  surface = glfwGetWGPUSurface(instance, Window);

  WGPUDeviceDescriptor deviceDesc = {};
  deviceDesc.nextInChain = nullptr;
  deviceDesc.label = "My Device";
  deviceDesc.requiredFeatureCount = 0;
  deviceDesc.requiredLimits = nullptr;
  deviceDesc.defaultQueue.nextInChain = nullptr;
  deviceDesc.defaultQueue.label = "The default queue";
	
  device = requestDeviceSync(adapter, &deviceDesc);
  queue = wgpuDeviceGetQueue(device);
  
  // Configure the surface
  WGPUSurfaceConfiguration config = {};
  config.nextInChain = nullptr;

  // Configuration of the textures created for the underlying swap chain
  config.width = Width;
  config.height = Height;
  config.usage = WGPUTextureUsage_RenderAttachment;
  WGPUTextureFormat surfaceFormat = wgpuSurfaceGetPreferredFormat(surface, adapter);
  config.format = surfaceFormat;

  // And we do not need any particular view format:
  config.viewFormatCount = 0;
  config.viewFormats = nullptr;
  config.device = device;
  config.presentMode = WGPUPresentMode_Fifo;
  config.alphaMode = WGPUCompositeAlphaMode_Auto;

  wgpuSurfaceConfigure(surface, &config);

  // Release the adapter only after it has been fully utilized
  wgpuAdapterRelease(adapter);
}

bool Application::isRunning(){
  messageLopp();
  return glfwWindowShouldClose(Window);
}

void Application::update(){

}

void Application::render(){
  // Get the next target texture view
  WGPUTextureView targetView = GetNextSurfaceTextureView();
  if (!targetView) return;

  // Create a command encoder for the draw call
  WGPUCommandEncoderDescriptor encoderDesc = {};
  encoderDesc.nextInChain = nullptr;
  encoderDesc.label = "My command encoder";
  WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDesc);

  // Create the render pass that clears the screen with our color
  WGPURenderPassDescriptor renderPassDesc = {};
  renderPassDesc.nextInChain = nullptr;

  // The attachment part of the render pass descriptor describes the target texture of the pass
  WGPURenderPassColorAttachment renderPassColorAttachment = {};
  renderPassColorAttachment.view = targetView;
  renderPassColorAttachment.resolveTarget = nullptr;
  renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
  renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
  renderPassColorAttachment.clearValue = WGPUColor{ 0.0, 1.0, 0.2, 1.0 };
#ifndef WEBGPU_BACKEND_WGPU
  renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif // NOT WEBGPU_BACKEND_WGPU

  renderPassDesc.colorAttachmentCount = 1;
  renderPassDesc.colorAttachments = &renderPassColorAttachment;
  renderPassDesc.depthStencilAttachment = nullptr;
  renderPassDesc.timestampWrites = nullptr;

  // Create the render pass and end it immediately (we only clear the screen but do not draw anything)
  WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
  wgpuRenderPassEncoderEnd(renderPass);
  wgpuRenderPassEncoderRelease(renderPass);

  // Finally encode and submit the render pass
  WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
  cmdBufferDescriptor.nextInChain = nullptr;
  cmdBufferDescriptor.label = "Command buffer";
  WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
  wgpuCommandEncoderRelease(encoder);

  wgpuQueueSubmit(queue, 1, &command);
  wgpuCommandBufferRelease(command);

  // At the end of the frame
  wgpuTextureViewRelease(targetView);
#ifndef __EMSCRIPTEN__
  wgpuSurfacePresent(surface);
#endif

#if defined(WEBGPU_BACKEND_DAWN)
  wgpuDeviceTick(device);
#elif defined(WEBGPU_BACKEND_WGPU)
  wgpuDevicePoll(device, false, nullptr);
#endif
}

void Application::messageLopp(){
  glfwPollEvents();
  render();
}

WGPUTextureView Application::GetNextSurfaceTextureView() {
  WGPUSurfaceTexture surfaceTexture;
  wgpuSurfaceGetCurrentTexture(surface, &surfaceTexture);
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