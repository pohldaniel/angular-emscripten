

#include "Shape.h"
#include "Application.h"

struct VertexType {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec4 color;
};

float heightMap(const glm::vec2 position) {
  return 2.0 * sin(position.x) * sin(position.y);
}

VertexType getHeightMap(const glm::vec2 position) {
  const glm::vec2 dx(1.0, 0.0);
  const glm::vec2 dy(0.0, 1.0);

  VertexType v;
  float h = heightMap(position);
  float hx = 100.f * (heightMap(position + 0.01f * dx) - h);
  float hy = 100.f * (heightMap(position + 0.01f * dy) - h);

  v.position = glm::vec3(position, h);
  v.normal = glm::normalize(glm::vec3(-hx, -hy, 1.0));

  float c = sin(h * 5.f) * 0.5 + 0.5;
  v.color = glm::vec4(c, 1.0 - c, 1.0, 1.0);
  return v;
}

Shape::Shape(StateMachine& machine) : State(machine, States::SHAPE) {

 
}

Shape::~Shape() {
	
}

void Shape::fixedUpdate() {

}

void Shape::update() {
	
}

void Shape::render() {

 // Get the next target texture view
  WGPUTextureView targetView = Application::GetNextSurfaceTextureView();
  if (!targetView) return;

  // Create a command encoder for the draw call
  WGPUCommandEncoderDescriptor encoderDesc = {};
  encoderDesc.nextInChain = nullptr;
  encoderDesc.label = "My command encoder";
  WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(Application::Device, &encoderDesc);

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

  wgpuQueueSubmit(Application::Queue, 1, &command);
  wgpuCommandBufferRelease(command);

  // At the end of the frame
  wgpuTextureViewRelease(targetView);
#ifndef __EMSCRIPTEN__
  wgpuSurfacePresent(surface);
#endif

#if defined(WEBGPU_BACKEND_DAWN)
  wgpuDeviceTick(Application::Device);
#elif defined(WEBGPU_BACKEND_WGPU)
  wgpuDevicePoll(Application::Device, false, nullptr);
#endif
}


/*bool Shape::loadGeometryFromObj(const std::filesystem::path& path, std::vector<VertexAttributes>& vertexData) {
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
}*/