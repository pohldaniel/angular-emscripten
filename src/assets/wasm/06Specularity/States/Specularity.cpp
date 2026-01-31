
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_wgpu.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>

#include <WebGPU/WgpContext.h>
#include "Specularity.h"
#include "Application.h"
#include "Mouse.h"

Specularity::Specularity(StateMachine& machine) : State(machine, States::SPECULARITY) {

    m_uniformBuffer.createBuffer(sizeof(Uniforms), WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform);
	wgpContext.addSampler(wgpCreateSampler());
	m_texture = wgpCreateTexture(512, 512, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst, WGPUTextureFormat::WGPUTextureFormat_RGBA8Unorm);
	m_textureView = wgpCreateTextureView(WGPUTextureFormat::WGPUTextureFormat_RGBA8Unorm, WGPUTextureAspect::WGPUTextureAspect_All, m_texture);
 
    wgpContext.addSahderModule("BOAT", "res/shader/shader_new.wgsl");
	wgpContext.createRenderPipeline("BOAT", "RP_PTNC", VL_PTNC, std::bind(&Specularity::OnBindGroupLayout, this));

    m_boat.loadModel("res/models/fourareen.obj", false, false, false, false, false, true);
	m_boat.generateColors();
	m_wgpBoat.create(m_boat, m_textureView, m_uniformBuffer);
	m_wgpBoat.createBindGroup("RP_PTNC");

    wgpContext.OnDraw = std::bind(&Specularity::OnDraw, this, std::placeholders::_1);
	
    float cx = cos(m_cameraState.angles.x);
	float sx = sin(m_cameraState.angles.x);
	float cy = cos(m_cameraState.angles.y);
	float sy = sin(m_cameraState.angles.y);
	glm::vec3 position = glm::vec3(cx * cy, sx * cy, sy) * std::exp(-m_cameraState.zoom);

	m_uniforms.modelMatrix = glm::mat4x4(1.0);
	m_uniforms.viewMatrix = glm::lookAt(position, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	m_uniforms.projectionMatrix = glm::perspective(glm::radians(45.0f), static_cast<float>(Application::Width) / static_cast<float>(Application::Height), 0.01f, 100.0f);
	m_uniforms.color = { 0.0f, 1.0f, 0.4f, 1.0f };
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), 0, &m_uniforms, sizeof(Uniforms));
}

Specularity::~Specularity() {
	m_uniformBuffer.markForDelete();

	wgpuTextureDestroy(m_texture);
	wgpuTextureRelease(m_texture);
	m_texture = NULL;

	wgpuTextureViewRelease(m_textureView);
	m_textureView = NULL;
}

void Specularity::fixedUpdate() {

}

void Specularity::update() {
	updateDragInertia();
}

void Specularity::render() {
    wgpDraw();
}

void Specularity::OnDraw(const WGPURenderPassEncoder& renderPassEncoder) {
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, projectionMatrix), &m_uniforms.projectionMatrix, sizeof(Uniforms::projectionMatrix));
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, viewMatrix), &m_uniforms.viewMatrix, sizeof(Uniforms::viewMatrix));
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, modelMatrix), &m_uniforms.modelMatrix, sizeof(Uniforms::modelMatrix));
	
	wgpuRenderPassEncoderSetViewport(renderPassEncoder, 0.0f, 0.0f, static_cast<float>(Application::Width), static_cast<float>(Application::Height), 0.0f, 1.0f);
	wgpuRenderPassEncoderSetPipeline(renderPassEncoder, wgpContext.renderPipelinesC.at("RP_PTNC"));

	m_wgpBoat.drawRaw(renderPassEncoder);

	//if (m_drawUi)
		//renderUi(renderPassEncoder);
}

void Specularity::OnMouseButtonDown(const Event::MouseButtonEvent& event) {
	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT) {
		Mouse::instance().attach(Application::Window, true, false, false);
	}else if(event.button == Event::MouseButtonEvent::BUTTON_LEFT){
		m_drag.active = true;
		m_drag.startMouse = glm::vec2(-(float)event.x, (float)event.y);
		m_drag.startCameraState = m_cameraState;
	}	
}

void Specularity::OnMouseButtonUp(const Event::MouseButtonEvent& event) {
	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT) {
		Mouse::instance().detach();
	}else if(event.button == Event::MouseButtonEvent::BUTTON_LEFT){
		m_drag.active = false;	
	}		
}

void Specularity::OnMouseMotion(const Event::MouseMoveEvent& event) {
	if (m_drag.active) {
		glm::vec2 currentMouse = glm::vec2(-(float)event.x, (float)event.y);
		glm::vec2 delta = (currentMouse - m_drag.startMouse) * m_drag.sensitivity;
		m_cameraState.angles = m_drag.startCameraState.angles + delta;
		m_cameraState.angles.y = glm::clamp(m_cameraState.angles.y, -glm::pi<float>() / 2 + 1e-5f, glm::pi<float>() / 2 - 1e-5f);
		updateViewMatrix();
		m_drag.velocity = delta - m_drag.previousDelta;
		m_drag.previousDelta = delta;
	}
}

void Specularity::OnScroll(double xoffset, double yoffset){
	m_cameraState.zoom += m_drag.scrollSensitivity * (float)yoffset;
	m_cameraState.zoom = glm::clamp(m_cameraState.zoom, -2.0f, 2.0f);
	updateViewMatrix();
}

void Specularity::OnKeyDown(const Event::KeyboardEvent& event){

}

void Specularity::OnKeyUp(const Event::KeyboardEvent& event) {

}

void Specularity::resize(int deltaW, int deltaH) {
	float ratio = static_cast<float>(Application::Width) / static_cast<float>(Application::Height);
	m_uniforms.projectionMatrix = glm::perspective(glm::radians(45.0f), ratio, 0.01f, 100.0f);
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, projectionMatrix), &m_uniforms.projectionMatrix, sizeof(Uniforms::projectionMatrix));
}

void Specularity::renderUi(const WGPURenderPassEncoder& renderPassEncoder) {
// Start the Dear ImGui frame
	ImGui_ImplWGPU_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	{ // Build our UI
		static float f = 0.0f;
		static int counter = 0;
		static bool show_demo_window = true;
		static bool show_another_window = false;
		static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		ImGui::Begin("Hello, world!");                                // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");                     // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);            // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);                  // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color);       // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                                  // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGuiIO& io = ImGui::GetIO();
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();
	}

	// Draw the UI
	ImGui::EndFrame();
	// Convert the UI defined above into low-level drawing commands
	ImGui::Render();
	// Execute the low-level drawing commands on the WebGPU backend
	ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPassEncoder);
}

WGPUBindGroupLayout Specularity::OnBindGroupLayout() {
	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries(3);

	WGPUBindGroupLayoutEntry& uniformLayout = bindingLayoutEntries[0];
	uniformLayout.binding = 0;
	uniformLayout.visibility = WGPUShaderStage::WGPUShaderStage_Vertex | WGPUShaderStage::WGPUShaderStage_Fragment;
	uniformLayout.buffer.type = WGPUBufferBindingType::WGPUBufferBindingType_Uniform;
	uniformLayout.buffer.minBindingSize = sizeof(Uniforms);

	WGPUBindGroupLayoutEntry& textureBindingLayout = bindingLayoutEntries[1];
	textureBindingLayout.binding = 1;
	textureBindingLayout.visibility = WGPUShaderStage::WGPUShaderStage_Fragment;
	textureBindingLayout.texture.sampleType = WGPUTextureSampleType::WGPUTextureSampleType_Float;
	textureBindingLayout.texture.viewDimension = WGPUTextureViewDimension::WGPUTextureViewDimension_2D;

	WGPUBindGroupLayoutEntry& samplerBindingLayout = bindingLayoutEntries[2];
	samplerBindingLayout.binding = 2;
	samplerBindingLayout.visibility = WGPUShaderStage::WGPUShaderStage_Fragment;
	samplerBindingLayout.sampler.type = WGPUSamplerBindingType::WGPUSamplerBindingType_Filtering;

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = {};
	bindGroupLayoutDescriptor.entryCount = (uint32_t)bindingLayoutEntries.size();
	bindGroupLayoutDescriptor.entries = bindingLayoutEntries.data();

	return wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor);
}

void Specularity::updateViewMatrix() {
	float cx = cos(m_cameraState.angles.x);
	float sx = sin(m_cameraState.angles.x);
	float cy = cos(m_cameraState.angles.y);
	float sy = sin(m_cameraState.angles.y);
	glm::vec3 position = glm::vec3(cx * cy, sx * cy, sy) * std::exp(-m_cameraState.zoom);
	m_uniforms.viewMatrix = glm::lookAt(position, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, viewMatrix), &m_uniforms.viewMatrix, sizeof(Uniforms::viewMatrix));
}

void Specularity::updateDragInertia() {
	constexpr float eps = 1e-4f;
	if (!m_drag.active) {
		if (std::abs(m_drag.velocity.x) < eps && std::abs(m_drag.velocity.y) < eps) {
			return;
		}
		m_cameraState.angles += m_drag.velocity;
		m_cameraState.angles.y = glm::clamp(m_cameraState.angles.y, -glm::pi<float>() / 2 + 1e-5f, glm::pi<float>() / 2 - 1e-5f);
		m_drag.velocity *= m_drag.intertia;
		updateViewMatrix();
	}
}
