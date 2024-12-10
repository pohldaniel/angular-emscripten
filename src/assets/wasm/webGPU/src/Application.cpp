#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>
#include <GL/glew.h>
#include <emscripten.h>

#include <States/Shape.h>

#include "WebGpuUtils.h"
#include "Application.h"


GLFWwindow* Application::Window = nullptr;
StateMachine* Application::Machine = nullptr;
WGPUDevice Application::Device;
WGPUQueue Application::Queue;
WGPUSurface Application::Surface;

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

Application::Application(float& dt, float& fdt) : fdt(fdt), dt(dt), last(0.0) {
  Application::Width = 512;
  Application::Height = 512;
  initWindow();
  initWebGPU();
  initStates();
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
  instance = wgpuCreateInstance(nullptr);
  adapter = requestAdapterSync(instance, nullptr);

  /*WGPUSurfaceDescriptorFromCanvasHTMLSelector canvasDesc = {};
  canvasDesc.chain.next = NULL;
  canvasDesc.chain.sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;
  canvasDesc.selector = "#canvas";

  WGPUSurfaceDescriptor surfaceDesc = {};
  surfaceDesc.nextInChain = &canvasDesc.chain;
  surfaceDesc.label = NULL;
  Surface = wgpuInstanceCreateSurface(instance, &surfaceDesc);*/
  Surface = glfwGetWGPUSurface(instance, Window);

  WGPUDeviceDescriptor deviceDesc = {};
  deviceDesc.nextInChain = nullptr;
  deviceDesc.label = "My Device";
  deviceDesc.requiredFeatureCount = 0;
  deviceDesc.requiredLimits = nullptr;
  deviceDesc.defaultQueue.nextInChain = nullptr;
  deviceDesc.defaultQueue.label = "The default queue";
	
  Device = requestDeviceSync(adapter, &deviceDesc);
  Queue = wgpuDeviceGetQueue(Device);
  
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
    Machine->update();
    Machine->render();
}

void Application::initStates(){
    Machine = new StateMachine(dt, fdt);
    Machine->addStateAtTop(new Shape(*Machine));
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