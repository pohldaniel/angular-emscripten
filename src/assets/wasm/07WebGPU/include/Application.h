#pragma once

#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <webgpu.hpp>
#include <States/StateMachine.h>

struct Uniforms {
  glm::mat3x4 kernel = glm::mat3x4(0.0);
  float test = 0.5f;
  uint32_t filterType = 0;
  float _pad[2];
};

enum class FilterType {
  Sum,
  Maximum,
  Minimum,
};

struct Parameters {
  FilterType filterType = FilterType::Sum;
  glm::mat3x4 kernel = glm::mat3x4(1.0);
  bool normalize = true;
};

struct Settings {
  float scale = 0.5f;
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

	void onResize();
	void onGui(wgpu::RenderPassEncoder renderPass, bool force = false);
    void onCompute();

private:

	void initWindow();
	void initWebGPU();
	void terminateDevice();
	void initStates();
	void messageLopp();
	
 	void initSwapChain();
	void terminateSwapChain();

	void initGui();
	void terminateGui();

	void initBuffers();
	void terminateBuffers();

	void initTextures();
	void terminateTextures();

	void initTextureViews();
	void terminateTextureViews();

	void initBindGroup();
	void terminateBindGroup();

	void initBindGroupLayout();
	void terminateBindGroupLayout();

	void initComputePipeline();
	void terminateComputePipeline();

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
	
	wgpu::Buffer vertexBuffer;
	wgpu::Texture depthTexture;
	wgpu::ShaderModule shaderModule;
	wgpu::Sampler sampler;
	
	wgpu::TextureView textureView;
	std::unique_ptr<wgpu::ErrorCallback> errorCallbackHandle;

	wgpu::Texture inputTexture;
	wgpu::Texture outputTexture;
	wgpu::TextureView inputTextureView;
	wgpu::TextureView outputTextureView;
	wgpu::BindGroup bindGroup;
	wgpu::BindGroupLayout bindGroupLayout;
	wgpu::PipelineLayout pipelineLayout;
	wgpu::ComputePipeline pipeline;

	wgpu::TextureFormat swapChainFormat = wgpu::TextureFormat::Undefined;
	wgpu::SwapChainDescriptor swapChainDesc;

	std::unique_ptr<wgpu::ErrorCallback> m_uncapturedErrorCallback;
	std::unique_ptr<wgpu::DeviceLostCallback> m_deviceLostCallback;

	Uniforms m_uniforms;
	Parameters m_parameters;
	Settings m_settings;
	bool m_shouldCompute = true;
	bool m_force = true;
};