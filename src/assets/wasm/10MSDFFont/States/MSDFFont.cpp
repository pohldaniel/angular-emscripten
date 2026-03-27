
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_wgpu.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>

#include <glm/ext.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <glm/gtx/polar_coordinates.hpp>

#include <WebGPU/WgpContext.h>
#include <WebGPU/WgpBatchRenderer.h>
#include "MSDFFont.h"
#include "Application.h"
#include "Mouse.h"

MSDFFont::MSDFFont(StateMachine& machine) : State(machine, States::MSDF_FONT) {
    WgpBatchRenderer::Get().init();

	m_camera.perspective(45.0f, static_cast<float>(Application::Width) / static_cast<float>(Application::Height), 0.1f, 1000.0f);
	m_camera.orthographic(0.0f, static_cast<float>(Application::Width), 0.0f, static_cast<float>(Application::Height), -1.0f, 1.0f);
	m_camera.lookAt(glm::vec3(0.0f, 1.6f, 2.8f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	m_camera.setRotationSpeed(0.125f);
	m_camera.setMovingSpeed(10.0f);

	m_uniformBuffer.createBuffer(sizeof(Uniforms), WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform);
	wgpContext.addSampler(wgpCreateSampler());

	wgpContext.addSahderModule("BATCH", "res/shader/batch.wgsl");
	wgpContext.createRenderPipeline("BATCH", "RP_BATCH", VL_BATCH, std::bind(&MSDFFont::OnBindGroupLayouts, this));
	
	m_trackball.reshape(Application::Width, Application::Height);
	m_trackball.setTrackballScale(0.5f);

	m_uniforms.projectionMatrix = m_camera.getOrthographicMatrix();
	m_uniforms.viewMatrix = glm::mat4(1.0f);
	m_uniforms.modelMatrix = glm::mat4(1.0f);
	m_uniforms.normalMatrix = glm::mat4(1.0f);
	m_uniforms.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_uniforms.camPosition = glm::vec3(0.0f);
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), 0, &m_uniforms, sizeof(Uniforms));

	wgpContext.OnDraw = std::bind(&MSDFFont::OnDraw, this, std::placeholders::_1);

    WgpBatchRenderer::Get().setBindGroups(std::bind(&MSDFFont::OnBindGroups, this));
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

    m_trackball.idle();
	applyTransformation(m_trackball);
	
	m_uniforms.projectionMatrix = m_camera.getOrthographicMatrix();
	m_uniforms.viewMatrix = m_camera.getViewMatrix();
    m_uniforms.normalMatrix = GetNormalMatrix(m_camera.getViewMatrix() * m_uniforms.modelMatrix);
}

void MSDFFont::render() {
    wgpDraw();
}

void MSDFFont::OnDraw(const WGPURenderPassEncoder& renderPassEncoder) {
	float pos = 0.0f;

	for(int i = 0; i < 16; i++) {
		WgpBatchRenderer::Get().addQuadAA({ pos,   pos,   50.0f, 50.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 0u);
		pos = pos + 50.0f;
	}

	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, projectionMatrix), &m_uniforms.projectionMatrix, sizeof(Uniforms::projectionMatrix));
	wgpuRenderPassEncoderSetViewport(renderPassEncoder, 0.0f, 0.0f, static_cast<float>(Application::Width), static_cast<float>(Application::Height), 0.0f, 1.0f);
	wgpuRenderPassEncoderSetPipeline(renderPassEncoder, wgpContext.renderPipelines.at("RP_BATCH"));

	WgpBatchRenderer::Get().draw(renderPassEncoder);

	if (m_drawUi)
		renderUi(renderPassEncoder);
}

void MSDFFont::OnMouseButtonDown(const Event::MouseButtonEvent& event) {
	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT) {
		Mouse::instance().attach(Application::Window, true, false, false);
	}

	if (event.button == Event::MouseButtonEvent::BUTTON_LEFT) {
		m_trackball.mouse(TrackBall::Button::ELeftButton, TrackBall::Modifier::ENoModifier, true, event.x, event.y);
		applyTransformation(m_trackball);
	}
}

void MSDFFont::OnMouseButtonUp(const Event::MouseButtonEvent& event) {
	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT) {
		Mouse::instance().detach();
	} 

	if (event.button == Event::MouseButtonEvent::BUTTON_LEFT) {
		m_trackball.mouse(TrackBall::Button::ELeftButton, TrackBall::Modifier::ENoModifier, false, event.x, event.y);
		applyTransformation(m_trackball);
	} 
}

void MSDFFont::OnMouseMotion(const Event::MouseMoveEvent& event) {
	m_trackball.motion(event.x, event.y);
	applyTransformation(m_trackball);
}

void MSDFFont::OnScroll(double xoffset, double yoffset) {

}

void MSDFFont::OnKeyDown(const Event::KeyboardEvent& event ){

}

void MSDFFont::OnKeyUp(const Event::KeyboardEvent& event) {

}

void MSDFFont::resize(int deltaW, int deltaH) {
	m_camera.perspective(45.0f, static_cast<float>(Application::Width) / static_cast<float>(Application::Height), 0.1f, 1000.0f);
	m_camera.orthographic(0.0f, static_cast<float>(Application::Width), 0.0f, static_cast<float>(Application::Height), -1.0f, 1.0f);
}

void MSDFFont::applyTransformation(const TrackBall& arc) {
  m_uniforms.modelMatrix = arc.getTransform();
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
	std::vector<WGPUBindGroupLayout> bindingLayouts(1);

    std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries0(1);
    WGPUBindGroupLayoutEntry& uniformLayout = bindingLayoutEntries0[0];
    uniformLayout.binding = 0u;
    uniformLayout.visibility = WGPUShaderStage_Vertex;
    uniformLayout.buffer.type = WGPUBufferBindingType::WGPUBufferBindingType_Uniform;
    uniformLayout.buffer.minBindingSize = sizeof(Uniforms);

    WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor0 = {};
    bindGroupLayoutDescriptor0.entryCount = (uint32_t)bindingLayoutEntries0.size();
    bindGroupLayoutDescriptor0.entries = bindingLayoutEntries0.data();

    bindingLayouts[0] = wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor0);

    return bindingLayouts;
}

std::vector<WGPUBindGroup> MSDFFont::OnBindGroups() {
	std::vector<WGPUBindGroup> bindGroups(1);

    std::vector<WGPUBindGroupEntry> bindGroupEntries0(1);

    bindGroupEntries0[0].binding = 0u;
    bindGroupEntries0[0].buffer = m_uniformBuffer.getBuffer();
    bindGroupEntries0[0].offset = 0u;
    bindGroupEntries0[0].size = sizeof(Uniforms);

    WGPUBindGroupDescriptor bindGroupDesc0 = {};
    bindGroupDesc0.layout = wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at("RP_BATCH"), 0u);
    bindGroupDesc0.entryCount = (uint32_t)bindGroupEntries0.size();
    bindGroupDesc0.entries = bindGroupEntries0.data();

    bindGroups[0] = wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc0);

    return bindGroups;
}

glm::mat4 MSDFFont::GetNormalMatrix(const glm::mat4& m) {

	glm::mat4 normalMatrix;
	float det;
	float invDet;

	det = m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2]) +
		  m[0][1] * (m[1][2] * m[2][0] - m[2][2] * m[1][0]) +
		  m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

	invDet = 1.0f / det;

	normalMatrix[0][0] = (m[1][1] * m[2][2] - m[2][1] * m[1][2]) * invDet;
	normalMatrix[1][0] = (m[2][1] * m[0][2] - m[2][2] * m[0][1]) * invDet;
	normalMatrix[2][0] = (m[0][1] * m[1][2] - m[1][1] * m[0][2]) * invDet;
	normalMatrix[3][0] = 0.0f;

	normalMatrix[0][1] = (m[2][0] * m[1][2] - m[1][0] * m[2][2]) * invDet;
	normalMatrix[1][1] = (m[0][0] * m[2][2] - m[2][0] * m[0][2]) * invDet;
	normalMatrix[2][1] = (m[1][0] * m[0][2] - m[1][2] * m[0][0]) * invDet;
	normalMatrix[3][1] = 0.0f;

	normalMatrix[0][2] = (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * invDet;
	normalMatrix[1][2] = (m[2][0] * m[0][1] - m[0][0] * m[2][1]) * invDet;
	normalMatrix[2][2] = (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * invDet;
	normalMatrix[3][2] = 0.0f;

	normalMatrix[0][3] = 0.0f;
	normalMatrix[1][3] = 0.0f;
	normalMatrix[2][3] = 0.0f;
	normalMatrix[3][3] = 1.0f;

	return normalMatrix;
}