#pragma once

#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <States/StateMachine.h>

struct VertexAttributes {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
};

struct MyUniforms {
	// We add transform matrices
    glm::mat4x4 projectionMatrix;
    glm::mat4x4 viewMatrix;
    glm::mat4x4 modelMatrix;
    std::array<float, 4> color;
    float time;
    float _pad[3];
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

	MyUniforms uniforms;
	float angle1 = 2.0f;
	glm::mat4x4 S;
	glm::mat4x4 T1;
	glm::mat4x4 R1;
	glm::mat4x4 R2;
	glm::mat4x4 T2;

private:

	void initWindow();
	void initWebGPU();
	void initStates();
	void messageLopp();
	//WGPURequiredLimits GetRequiredLimits(WGPUAdapter adapter);
	//
	//void setDefault(WGPULimits &limits);
	//void setDefault(WGPUBindGroupLayoutEntry& bindingLayout);
	//void setDefault(WGPUDepthStencilState& depthStencilState);

	wgpu::ShaderModule loadShaderModule(const std::filesystem::path& path);
	bool loadGeometryFromObj(const std::filesystem::path& path, std::vector<VertexAttributes>& vertexData);

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
	int indexCount;
};