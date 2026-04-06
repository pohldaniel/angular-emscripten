
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_wgpu.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>

#include <glm/ext.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <glm/gtx/polar_coordinates.hpp>

#include <WebGPU/WgpContext.h>
#include "MSDFFont.h"
#include "Application.h"
#include "Mouse.h"

MSDFFont::MSDFFont(StateMachine& machine) : State(machine, States::MSDF_FONT) {
    WgpFontRenderer::Get().init(2400u);

	m_camera.perspective(glm::radians(72.0f), static_cast<float>(Application::Width) / static_cast<float>(Application::Height), 0.1f, 1000.0f);
	m_camera.orthographic(0.0f, static_cast<float>(Application::Width), 0.0f, static_cast<float>(Application::Height), -1.0f, 1.0f);
	m_camera.lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	m_camera.setRotationSpeed(0.125f);
	m_camera.setMovingSpeed(10.0f);

	m_characterSet.loadMsdfBmFromFile("res/fonts/YaHei_msdf_bm.json", "res/fonts/YaHei_msdf_bm.png");
	m_uniformBuffer.createBuffer(sizeof(Uniforms), WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform);

	wgpContext.addSampler(wgpCreateSampler());
	wgpContext.setClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
	wgpContext.addSahderModule("FONT", "res/shader/font.wgsl");
	wgpContext.createRenderPipeline("FONT", "RP_FONT", VL_BATCH, std::bind(&MSDFFont::OnBindGroupLayouts, this));
	wgpContext.OnDraw = std::bind(&MSDFFont::OnDraw, this, std::placeholders::_1);

	m_uniforms.projectionMatrix = m_camera.getPerspectiveMatrix();
	m_uniforms.viewMatrix = m_camera.getViewMatrix();
	m_uniforms.modelMatrix = glm::mat4(1.0f);
	m_uniforms.normalMatrix = glm::mat4(1.0f);
	m_uniforms.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_uniforms.camPosition = glm::vec3(0.0f);
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), 0, &m_uniforms, sizeof(Uniforms));
	
    WgpFontRenderer::Get().setBindGroups(std::bind(&MSDFFont::OnBindGroups, this));
	static const char* text
		= "\n"
		"WebGPU exposes an API for performing operations, such as rendering\n"
		"and computation, on a Graphics Processing Unit.\n"
		"\n"
		"Graphics Processing Units, or GPUs for short, have been essential\n"
		"in enabling rich rendering and computational applications in personal\n"
		"computing. WebGPU is an API that exposes the capabilities of GPU\n"
		"hardware for the Web. The API is designed from the ground up to\n"
		"efficiently map to (post-2014) native GPU APIs. WebGPU is not related\n"
		"to WebGL and does not explicitly target OpenGL ES.\n"
		"\n"
		"WebGPU sees physical GPU hardware as GPUAdapters. It provides a\n"
		"connection to an adapter via GPUDevice, which manages resources, and\n"
		"the device's GPUQueues, which execute commands. GPUDevice may have\n"
		"its own memory with high-speed access to the processing units.\n"
		"GPUBuffer and GPUTexture are the physical resources backed by GPU\n"
		"memory. GPUCommandBuffer and GPURenderBundle are containers for\n"
		"user-recorded commands. GPUShaderModule contains shader code. The\n"
		"other resources, such as GPUSampler or GPUBindGroup, configure the\n"
		"way physical resources are used by the GPU.\n"
		"\n"
		"GPUs execute commands encoded in GPUCommandBuffers by feeding data\n"
		"through a pipeline, which is a mix of fixed-function and programmable\n"
		"stages. Programmable stages execute shaders, which are special\n"
		"programs designed to run on GPU hardware. Most of the state of a\n"
		"pipeline is defined by a GPURenderPipeline or a GPUComputePipeline\n"
		"object. The state not included in these pipeline objects is set\n"
		"during encoding with commands, such as beginRenderPass() or\n"
		"setBlendConstant().";

	m_formatedText.create(text);
}

MSDFFont::~MSDFFont() {
	m_uniformBuffer.markForDelete();
}

void MSDFFont::fixedUpdate() {

}

void MSDFFont::update() {
	Mouse &mouse = Mouse::instance();

	glm::vec3 direction = glm::vec3();

	float dx = 0.0f;
	float dy = 0.0f;
	bool move = false;

	if (glfwGetKey(Application::Window, GLFW_KEY_W) == GLFW_PRESS) {
		direction += glm::vec3(0.0f, 0.0f, 1.0f);
		move |= true;
	}

	if (glfwGetKey(Application::Window, GLFW_KEY_S) == GLFW_PRESS) {
		direction += glm::vec3(0.0f, 0.0f, -1.0f);
		move |= true;
	}

	if (glfwGetKey(Application::Window, GLFW_KEY_A) == GLFW_PRESS) {
		direction += glm::vec3(-1.0f, 0.0f, 0.0f);
		move |= true;
	}

	if (glfwGetKey(Application::Window, GLFW_KEY_D) == GLFW_PRESS) {
		direction += glm::vec3(1.0f, 0.0f, 0.0f);
		move |= true;
	}

	if (glfwGetKey(Application::Window, GLFW_KEY_Q) == GLFW_PRESS) {
		direction += glm::vec3(0.0f, -1.0f, 0.0f);
		move |= true;
	}

	if (glfwGetKey(Application::Window, GLFW_KEY_E) == GLFW_PRESS) {
		direction += glm::vec3(0.0f, 1.0f, 0.0f);
		move |= true;
	}

    if (glfwGetMouseButton(Application::Window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {	
		dx = mouse.xDelta();
		dy = mouse.yDelta();
	}

    if (move || dx != 0.0f || dy != 0.0f) {
		if (dx || dy) {			
			m_camera.rotate(dx, dy);
		}

		if (move) {
			m_camera.move(direction * m_dt);
		}
	}

	double sec = glfwGetTime();
	float crawl = fmodf((float)sec / 2.5f, 14.0f);

	m_uniforms.projectionMatrix = m_camera.getPerspectiveMatrix();
	m_uniforms.viewMatrix = m_camera.getViewMatrix();

	m_model = glm::mat4(1.0f);
	m_model = glm::rotate(m_model, glm::radians(-22.5f), glm::vec3(1.0f, 0.0f, 0.0f));
	m_model = glm::translate(m_model, glm::vec3(0.0f, crawl - 3.0f, 0.0f));
}

void MSDFFont::render() {
    wgpDraw();
}

void MSDFFont::OnDraw(const WGPURenderPassEncoder& renderPassEncoder) {
	WgpFontRenderer::Get().reset();

	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, projectionMatrix), &m_uniforms.projectionMatrix, sizeof(Uniforms::projectionMatrix));
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, viewMatrix), &m_uniforms.viewMatrix, sizeof(Uniforms::viewMatrix));
	//wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, modelMatrix), &m_uniforms.modelMatrix, sizeof(Uniforms::modelMatrix));

	wgpuRenderPassEncoderSetViewport(renderPassEncoder, 0.0f, 0.0f, static_cast<float>(Application::Width), static_cast<float>(Application::Height), 0.0f, 1.0f);
	wgpuRenderPassEncoderSetPipeline(renderPassEncoder, wgpContext.renderPipelines.at("RP_FONT"));

	glm::mat4 transOrigin = glm::mat4(1.0f);
	
	transOrigin = glm::translate(transOrigin, glm::vec3(-m_characterSet.getWidth("WebGPU") * 0.5f * largeScale, -m_characterSet.lineHeight * 0.5f * largeScale, 0.0f));
	WgpFontRenderer::Get().addTextTransformed(m_characterSet, "WebGPU", glm::value_ptr((m_model * transOrigin)[0]), {1.0f, 1.0f, 1.0f, 1.0f}, largeScale);
	WgpFontRenderer::Get().draw(renderPassEncoder);

	transOrigin = glm::mat4(1.0f);
	transOrigin = glm::translate(transOrigin, glm::vec3(-3.0f, -0.1f - (m_characterSet.lineHeight * 0.5f * largeScale), 0.0f));
	WgpFontRenderer::Get().addTextTransformed(m_characterSet, m_formatedText, glm::value_ptr((m_model * transOrigin)[0]), { 1.0f, 1.0f, 1.0f, 1.0f }, smallScale);
	WgpFontRenderer::Get().draw(renderPassEncoder);

	if (m_drawUi)
		renderUi(renderPassEncoder);
}

void MSDFFont::OnMouseButtonDown(const Event::MouseButtonEvent& event) {
	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT) {
		Mouse::instance().attach(Application::Window, true, false, false);
	}
}

void MSDFFont::OnMouseButtonUp(const Event::MouseButtonEvent& event) {
	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT) {
		Mouse::instance().detach();
	} 
}

void MSDFFont::OnMouseMotion(const Event::MouseMoveEvent& event) {

}

void MSDFFont::OnScroll(double xoffset, double yoffset) {
	if(yoffset < 0){
		m_fontSize = m_fontSize - 0.05f;
		m_fontSize = glm::clamp(m_fontSize, 0.0f, 5.0f);
	}

	if (yoffset > 0) {
		m_fontSize = m_fontSize + 0.05f;
		m_fontSize = glm::clamp(m_fontSize, 0.0f, 5.0f);
	}
}

void MSDFFont::OnKeyDown(const Event::KeyboardEvent& event ){

}

void MSDFFont::OnKeyUp(const Event::KeyboardEvent& event) {

}

void MSDFFont::resize(int deltaW, int deltaH) {
	m_camera.perspective(glm::radians(72.0f), static_cast<float>(Application::Width) / static_cast<float>(Application::Height), 0.1f, 1000.0f);
	m_camera.orthographic(0.0f, static_cast<float>(Application::Width), 0.0f, static_cast<float>(Application::Height), -1.0f, 1.0f);
}

void MSDFFont::renderUi(const WGPURenderPassEncoder& renderPassEncoder) {
	ImGui_ImplWGPU_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
		ImGuiWindowFlags_NoBackground;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("InvisibleWindow", nullptr, windowFlags);
	ImGui::PopStyleVar(3);

	ImGuiID dockSpaceId = ImGui::GetID("MainDockSpace");
	ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::End();

	if (m_initUi) {
		m_initUi = false;
		ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Left, 0.4f, nullptr, &dockSpaceId);
		ImGui::DockBuilderDockWindow("Settings", dock_id_left);
	}

	// render widgets
	ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::End();

	ImGui::Render();
	ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPassEncoder);
}

std::vector <WGPUBindGroupLayout> MSDFFont::OnBindGroupLayouts() {
	std::vector<WGPUBindGroupLayout> bindingLayouts(2);

	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries0(2);
	WGPUBindGroupLayoutEntry& uniformLayout = bindingLayoutEntries0[0];
	uniformLayout.binding = 0u;
	uniformLayout.visibility = WGPUShaderStage_Vertex;
	uniformLayout.buffer.type = WGPUBufferBindingType::WGPUBufferBindingType_Uniform;
	uniformLayout.buffer.minBindingSize = sizeof(Uniforms);

	WGPUBindGroupLayoutEntry& samplerBindingLayout = bindingLayoutEntries0[1];
	samplerBindingLayout.binding = 1u;
	samplerBindingLayout.visibility = WGPUShaderStage_Fragment;
	samplerBindingLayout.sampler.type = WGPUSamplerBindingType::WGPUSamplerBindingType_Filtering;

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor0 = {};
	bindGroupLayoutDescriptor0.entryCount = (uint32_t)bindingLayoutEntries0.size();
	bindGroupLayoutDescriptor0.entries = bindingLayoutEntries0.data();

	bindingLayouts[0] = wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor0);

	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries1(1);
	WGPUBindGroupLayoutEntry& textureBindingLayout = bindingLayoutEntries1[0];
	textureBindingLayout.binding = 0u;
	textureBindingLayout.visibility = WGPUShaderStage_Fragment;
	textureBindingLayout.texture.sampleType = WGPUTextureSampleType::WGPUTextureSampleType_Float;
	textureBindingLayout.texture.viewDimension = WGPUTextureViewDimension::WGPUTextureViewDimension_2D;

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor1 = {};
	bindGroupLayoutDescriptor1.entryCount = (uint32_t)bindingLayoutEntries1.size();
	bindGroupLayoutDescriptor1.entries = bindingLayoutEntries1.data();

	bindingLayouts[1] = wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor1);

	return bindingLayouts;
}

std::vector<WGPUBindGroup> MSDFFont::OnBindGroups() {
	std::vector<WGPUBindGroup> bindGroups(2);

	std::vector<WGPUBindGroupEntry> bindGroupEntries0(2);
	bindGroupEntries0[0].binding = 0u;
	bindGroupEntries0[0].buffer = m_uniformBuffer.getBuffer();
	bindGroupEntries0[0].offset = 0u;
	bindGroupEntries0[0].size = sizeof(Uniforms);

	bindGroupEntries0[1].binding = 1u;
	bindGroupEntries0[1].sampler = wgpContext.getSampler(SS_LINEAR);

	WGPUBindGroupDescriptor bindGroupDesc0 = {};
	bindGroupDesc0.layout = wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at("RP_FONT"), 0u);
	bindGroupDesc0.entryCount = (uint32_t)bindGroupEntries0.size();
	bindGroupDesc0.entries = bindGroupEntries0.data();
	bindGroups[0] = wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc0);

	std::vector<WGPUBindGroupEntry> bindGroupEntries1(1);
	bindGroupEntries1[0].binding = 0u;
	bindGroupEntries1[0].textureView = m_characterSet.m_texture.getTextureView();

	WGPUBindGroupDescriptor bindGroupDesc1 = {};
	bindGroupDesc1.layout = wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at("RP_FONT"), 1u);
	bindGroupDesc1.entryCount = (uint32_t)bindGroupEntries1.size();
	bindGroupDesc1.entries = bindGroupEntries1.data();
	bindGroups[1] = wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc1);

	return bindGroups;
}