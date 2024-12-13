#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu.hpp>

#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>
#include <GL/glew.h>
#include <emscripten.h>

#include <States/Shape.h>

#include "WebGpuUtils.h"
#include "Application.h"
constexpr float PI = 3.14159265358979323846f;

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

Application::Application(float& dt, float& fdt) : fdt(fdt), dt(dt), last(0.0) {
  Application::Width = 640;
  Application::Height = 480;
  initWindow();
  initWebGPU();
  initStates();
}

Application::~Application() {
  //wgpuSurfaceUnconfigure(Surface);
  //wgpuQueueRelease(Queue);
  //wgpuSurfaceRelease(Surface);
  //wgpuDeviceRelease(Device);
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
  instance = wgpu::createInstance(desc);
}

bool Application::isRunning(){
  messageLopp();
  return glfwWindowShouldClose(Window);
}

void Application::messageLopp(){
    glfwPollEvents();
   
}

void Application::initStates(){
    Machine = new StateMachine(dt, fdt);
    Machine->addStateAtTop(new Shape(*Machine));
}

wgpu::ShaderModule Application::loadShaderModule(const std::filesystem::path& path) {
	
  std::ifstream file(path);
	if (!file.is_open()) {
		return nullptr;
	}
	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	std::string shaderSource(size, ' ');
	file.seekg(0);
	file.read(shaderSource.data(), size);

	wgpu::ShaderModuleWGSLDescriptor shaderCodeDesc;
	shaderCodeDesc.chain.next = nullptr;
	shaderCodeDesc.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
	shaderCodeDesc.code = shaderSource.c_str();
	wgpu::ShaderModuleDescriptor shaderDesc;
	shaderDesc.nextInChain = &shaderCodeDesc.chain;
#ifdef WEBGPU_BACKEND_WGPU
	shaderDesc.hintCount = 0;
	shaderDesc.hints = nullptr;
#endif

	return Device.createShaderModule(shaderDesc);
}

bool Application::loadGeometryFromObj(const std::filesystem::path& path, std::vector<VertexAttributes>& vertexData) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;

	// Call the core loading procedure of TinyOBJLoader
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str());

	// Check errors
	if (!warn.empty()) {
		std::cout << warn << std::endl;
	}

	if (!err.empty()) {
		std::cerr << err << std::endl;
	}

	if (!ret) {
		return false;
	}

	// Filling in vertexData:
	vertexData.clear();
	for (const auto& shape : shapes) {
		size_t offset = vertexData.size();
		vertexData.resize(offset + shape.mesh.indices.size());

		for (size_t i = 0; i < shape.mesh.indices.size(); ++i) {
			const tinyobj::index_t& idx = shape.mesh.indices[i];

			vertexData[offset + i].position = {
				attrib.vertices[3 * idx.vertex_index + 0],
				-attrib.vertices[3 * idx.vertex_index + 2], // Add a minus to avoid mirroring
				attrib.vertices[3 * idx.vertex_index + 1]
			};

			// Also apply the transform to normals!!
			vertexData[offset + i].normal = {
				attrib.normals[3 * idx.normal_index + 0],
				-attrib.normals[3 * idx.normal_index + 2],
				attrib.normals[3 * idx.normal_index + 1]
			};

			vertexData[offset + i].color = {
				attrib.colors[3 * idx.vertex_index + 0],
				attrib.colors[3 * idx.vertex_index + 1],
				attrib.colors[3 * idx.vertex_index + 2]
			};
		}
	}

	return true;
}
