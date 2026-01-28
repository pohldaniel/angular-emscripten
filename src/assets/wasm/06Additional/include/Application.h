#pragma once

#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <webgpu.hpp>
#include <States/StateMachine.h>

struct MyUniforms {
  glm::mat4x4 projectionMatrix;
  glm::mat4x4 viewMatrix;
  glm::mat4x4 modelMatrix;
  glm::vec4 color;
  float time;
  float _pad[3];
};

struct CameraState {
  glm::vec2 angles = { 0.8f, 0.5f };
  float zoom = -1.2f;
};

struct DragState {
  bool active = false;
  glm::vec2 startMouse;
  CameraState startCameraState;
  glm::vec2 velocity = { 0.0, 0.0 };
  glm::vec2 previousDelta;
  float intertia = 0.9f;
  float sensitivity = 0.01f;
  float scrollSensitivity = 0.1f;
};

struct GLFWwindow;
class Application {

public:
	Application(float& dt, float& fdt);
	~Application();

	bool isRunning();

	static void MessageLopp(void *arg);

	static int Width;
	static int Height;
	static double Time;
	
	static wgpu::Device Device;
	static wgpu::Queue Queue;

	//bool onInit();
	//void onFrame();
	//void onFinish();
	//bool isRunning();
	void onResize();
	void onMouseMove(double xpos, double ypos);
	void onMouseButton(int button, int action, int mods);
	void onScroll(double xoffset, double yoffset);

	void initGui();
	void updateGui(wgpu::RenderPassEncoder renderPass);

private:

	void initWindow();
	void initWebGPU();
	void initStates();
	void messageLopp();
	
 	void buildSwapChain();
    void buildDepthBuffer();
	void updateViewMatrix();
	void updateDragInertia();

	float& fdt;
	float& dt;

	double last;

    static GLFWwindow* Window;
	static StateMachine* Machine;
	static wgpu::Surface Surface;

	wgpu::Adapter adapter;
	wgpu::Instance instance;
	wgpu::SwapChain SwapChain;
	wgpu::Buffer uniformBuffer;
	wgpu::TextureView depthTextureView;
	wgpu::RenderPipeline pipeline;
	wgpu::Buffer vertexBuffer;
	wgpu::BindGroup bindGroup;
	wgpu::Texture depthTexture;
	wgpu::ShaderModule shaderModule;
	wgpu::Sampler sampler;
	wgpu::Texture texture;
	wgpu::TextureView textureView;
	std::unique_ptr<wgpu::ErrorCallback> errorCallbackHandle;

	wgpu::TextureFormat swapChainFormat = wgpu::TextureFormat::Undefined;
	wgpu::TextureFormat depthTextureFormat = wgpu::TextureFormat::Depth24Plus;
	wgpu::SwapChainDescriptor swapChainDesc;

	int m_vertexCount = 0;
	MyUniforms m_uniforms;
	CameraState m_cameraState;
	DragState m_drag;
};