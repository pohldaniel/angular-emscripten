#pragma once

#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <webgpu/webgpu.h>
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

	static WGPUTextureView GetNextSurfaceTextureView();

	static int Width;
	static int Height;
	static double Time;
	
	static WGPUDevice Device;
	static WGPUQueue Queue;

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
	WGPURequiredLimits GetRequiredLimits(WGPUAdapter adapter);
	WGPUShaderModule loadShaderModule(const std::filesystem::path& path);
	void setDefault(WGPULimits &limits);
	void setDefault(WGPUBindGroupLayoutEntry& bindingLayout);
	void setDefault(WGPUDepthStencilState& depthStencilState);
	bool loadGeometryFromObj(const std::filesystem::path& path, std::vector<VertexAttributes>& vertexData);

	WGPUAdapter adapter;
	WGPUInstance instance;

	float& fdt;
	float& dt;

	double last;

    static GLFWwindow* Window;
	static StateMachine* Machine;
	static WGPUSurface Surface;

	WGPUSwapChain SwapChain;
	WGPUBuffer uniformBuffer;
	WGPUTextureView depthTextureView;
	WGPURenderPipeline pipeline;
	WGPUBuffer vertexBuffer;
	WGPUBindGroup bindGroup;
	int indexCount;
};