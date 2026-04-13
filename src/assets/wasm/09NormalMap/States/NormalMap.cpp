
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_wgpu.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>

#include <glm/ext.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <glm/gtx/polar_coordinates.hpp>

#include <WebGPU/WgpContext.h>
#include "NormalMap.h"
#include "Application.h"
#include "Mouse.h"

NormalMap::NormalMap(StateMachine& machine) : State(machine, States::NORMAL_MAP) {

	m_camera.perspective(glm::radians(72.0f), static_cast<float>(Application::Width) / static_cast<float>(Application::Height), 0.1f, 1000.0f);
	m_camera.orthographic(0.0f, static_cast<float>(Application::Width), 0.0f, static_cast<float>(Application::Height), -1.0f, 1.0f);
	m_camera.lookAt(glm::vec3(0.0f, 1.6f, 2.8f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	m_camera.setRotationSpeed(0.125f);
	m_camera.setMovingSpeed(10.0f);

	m_uniformBuffer.createBuffer(sizeof(Uniforms), WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform);
	m_normalUniformBuffer.createBuffer(sizeof(NormalUniforms), WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform);
	wgpContext.addSampler(wgpCreateSampler());

	m_textureAW.loadFromFile("res/textures/wood_albedo.png");
	m_textureAW.markForDelete();

	m_textureNS.loadFromFile("res/textures/spiral_normal.png");
	m_textureNS.markForDelete();

	m_textureHS.loadFromFile("res/textures/spiral_height.png");
	m_textureHS.markForDelete();

	m_textureNT.loadFromFile("res/textures/toybox_normal.png");
	m_textureNT.markForDelete();

	m_textureHT.loadFromFile("res/textures/toybox_height.png");
	m_textureHT.markForDelete();

	m_textureAB.loadFromFile("res/textures/brickwall_albedo.jpg");
	m_textureAB.markForDelete();

	m_textureNB.loadFromFile("res/textures/brickwall_normal.jpg");
	m_textureNB.markForDelete();

	m_textureHB.loadFromFile("res/textures/brickwall_height.jpg");
	m_textureHB.markForDelete();

	wgpContext.addSahderModule("NORMAL", "res/shader/normal.wgsl");
	wgpContext.createRenderPipeline("NORMAL", "RP_PTNTB", VL_PTNTB, std::bind(&NormalMap::OnBindGroupLayouts, this));

	m_cube.buildCube({ -0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f }, 1u, 1u, true, true, true);
	m_wgpCube.create(m_cube);
	m_wgpCube.setBindGroups("BG", std::bind(&NormalMap::OnBindGroups, this));

	m_sphere.buildSphere({ 0.0f, 0.0f, 0.0f }, 1.0f, 49u, 49u, true, true, true);
	m_wgpSphere.create(m_sphere);
	m_wgpSphere.setBindGroups("BG", std::bind(&NormalMap::OnBindGroups, this));

	m_torus.buildTorus({ 0.0f, 0.0f, 0.0f }, 0.5f, 0.25f, 49u, 49u, true, true, true);
	m_wgpTorus.create(m_torus);
	m_wgpTorus.setBindGroups("BG", std::bind(&NormalMap::OnBindGroups, this));

	m_torusKnot.buildTorusKnot({ 0.0f, 0.0f, 0.0f }, 1.0f, 0.4f, 2u, 3u, 100u, 16u, true, true, true);
	m_wgpTorusKnot.create(m_torusKnot);
	m_wgpTorusKnot.setBindGroups("BG", std::bind(&NormalMap::OnBindGroups, this));

	m_spiral.buildSpiral({ 0.0f, -0.75f, 0.0f }, 0.5f, 0.25f, 1.5f, 2u, true, 49u, 49u, true, true, true);
	m_wgpSpiral.create(m_spiral);
	m_wgpSpiral.setBindGroups("BG", std::bind(&NormalMap::OnBindGroups, this));

	m_trackball.reshape(Application::Width, Application::Height);
	m_trackball.setTrackballScale(0.5f);

	m_uniforms.projectionMatrix = m_camera.getPerspectiveMatrix();
	m_uniforms.viewMatrix = glm::mat4(1.0f);
	m_uniforms.modelMatrix = glm::mat4(1.0f);
	m_uniforms.normalMatrix = glm::mat4(1.0f);
	m_uniforms.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_uniforms.camPosition = glm::vec3(0.0f);
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), 0, &m_uniforms, sizeof(Uniforms));

	wgpContext.OnDraw = std::bind(&NormalMap::OnDraw, this, std::placeholders::_1);

	glm::vec3 lightPos = glm_vec3_transform_mat4(m_camera.getViewMatrix(), glm::vec3(-1.7f, 0.7f, 1.9f));
	m_normalUniforms.light_pos_vs = { lightPos[0], lightPos[1], lightPos[2] };
	m_normalUniforms.light_intensity = 5.0f;
	m_normalUniforms.depth_scale = 0.05f;
	m_normalUniforms.depth_layers = 16.0f;
	m_normalUniforms.mode = 3u;

	wgpuQueueWriteBuffer(wgpContext.queue, m_normalUniformBuffer.getBuffer(), 0, &m_normalUniforms, sizeof(NormalUniforms));

	m_bindgroups.resize(3);

	std::vector<WGPUBindGroupEntry> bindGroupEntries0(3);

	bindGroupEntries0[0].binding = 0u;
	bindGroupEntries0[0].textureView = m_textureAW.getTextureView();

	bindGroupEntries0[1].binding = 1u;
	bindGroupEntries0[1].textureView = m_textureNS.getTextureView();

	bindGroupEntries0[2].binding = 2u;
	bindGroupEntries0[2].textureView = m_textureHS.getTextureView();

	WGPUBindGroupDescriptor bindGroupDesc0 = {};
	bindGroupDesc0.layout = wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at("RP_PTNTB"), 1u);
	bindGroupDesc0.entryCount = (uint32_t)bindGroupEntries0.size();
	bindGroupDesc0.entries = bindGroupEntries0.data();
	m_bindgroups[0] = wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc0);


	std::vector<WGPUBindGroupEntry> bindGroupEntries1(3);

	bindGroupEntries1[0].binding = 0u;
	bindGroupEntries1[0].textureView = m_textureAW.getTextureView();

	bindGroupEntries1[1].binding = 1u;
	bindGroupEntries1[1].textureView = m_textureNT.getTextureView();

	bindGroupEntries1[2].binding = 2u;
	bindGroupEntries1[2].textureView = m_textureHT.getTextureView();

	WGPUBindGroupDescriptor bindGroupDesc1 = {};
	bindGroupDesc1.layout = wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at("RP_PTNTB"), 1u);
	bindGroupDesc1.entryCount = (uint32_t)bindGroupEntries1.size();
	bindGroupDesc1.entries = bindGroupEntries1.data();
	m_bindgroups[1] = wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc1);

	std::vector<WGPUBindGroupEntry> bindGroupEntries2(3);

	bindGroupEntries2[0].binding = 0u;
	bindGroupEntries2[0].textureView = m_textureAB.getTextureView();

	bindGroupEntries2[1].binding = 1u;
	bindGroupEntries2[1].textureView = m_textureNB.getTextureView();

	bindGroupEntries2[2].binding = 2u;
	bindGroupEntries2[2].textureView = m_textureHB.getTextureView();

	WGPUBindGroupDescriptor bindGroupDesc2 = {};
	bindGroupDesc2.layout = wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at("RP_PTNTB"), 1u);
	bindGroupDesc2.entryCount = (uint32_t)bindGroupEntries2.size();
	bindGroupDesc2.entries = bindGroupEntries2.data();
	m_bindgroups[2] = wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc2);

	wgpContext.OnDraw = std::bind(&NormalMap::OnDraw, this, std::placeholders::_1);
}

NormalMap::~NormalMap() {
	m_uniformBuffer.markForDelete();
}

void NormalMap::fixedUpdate() {

}

void NormalMap::update() {
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

	m_uniforms.projectionMatrix = m_camera.getPerspectiveMatrix();
	m_uniforms.viewMatrix = m_camera.getViewMatrix();
    m_uniforms.normalMatrix = Camera::GetNormalMatrix(m_camera.getViewMatrix() * m_uniforms.modelMatrix);

    glm::vec3 lightPos = glm_vec3_transform_mat4(m_camera.getViewMatrix(), glm::vec3(-1.7f, 0.7f, 1.9f));
    m_normalUniforms.light_pos_vs = { lightPos[0], lightPos[1], lightPos[2] };
}

void NormalMap::render() {
    wgpDraw();
}

void NormalMap::OnDraw(const WGPURenderPassEncoder& renderPassEncoder) {
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, projectionMatrix), &m_uniforms.projectionMatrix, sizeof(Uniforms::projectionMatrix));
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, viewMatrix), &m_uniforms.viewMatrix, sizeof(Uniforms::viewMatrix));
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, modelMatrix), &m_uniforms.modelMatrix, sizeof(Uniforms::modelMatrix));
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getBuffer(), offsetof(Uniforms, normalMatrix), &m_uniforms.normalMatrix, sizeof(Uniforms::normalMatrix));

	wgpuQueueWriteBuffer(wgpContext.queue, m_normalUniformBuffer.getBuffer(), 0u, &m_normalUniforms.light_pos_vs, sizeof(NormalUniforms::light_pos_vs));

	wgpuRenderPassEncoderSetViewport(renderPassEncoder, 0.0f, 0.0f, static_cast<float>(Application::Width), static_cast<float>(Application::Height), 0.0f, 1.0f);

	wgpuRenderPassEncoderSetPipeline(renderPassEncoder, wgpContext.renderPipelines.at("RP_PTNTB"));
	m_wgpCube.draw(renderPassEncoder);
	

	if (m_drawUi)
		renderUi(renderPassEncoder);
}

void NormalMap::OnMouseButtonDown(const Event::MouseButtonEvent& event) {
	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT) {
		Mouse::instance().attach(Application::Window, true, false, false);
	}

	if (event.button == Event::MouseButtonEvent::BUTTON_LEFT) {
		m_trackball.mouse(TrackBall::Button::ELeftButton, TrackBall::Modifier::ENoModifier, true, event.x, event.y);
		applyTransformation(m_trackball);
	}
}

void NormalMap::OnMouseButtonUp(const Event::MouseButtonEvent& event) {
	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT) {
		Mouse::instance().detach();
	} 

	if (event.button == Event::MouseButtonEvent::BUTTON_LEFT) {
		m_trackball.mouse(TrackBall::Button::ELeftButton, TrackBall::Modifier::ENoModifier, false, event.x, event.y);
		applyTransformation(m_trackball);
	} 
}

void NormalMap::OnMouseMotion(const Event::MouseMoveEvent& event) {
	m_trackball.motion(event.x, event.y);
	applyTransformation(m_trackball);
}

void NormalMap::OnScroll(double xoffset, double yoffset) {

}

void NormalMap::OnKeyDown(const Event::KeyboardEvent& event ){

}

void NormalMap::OnKeyUp(const Event::KeyboardEvent& event) {

}

void NormalMap::resize(int deltaW, int deltaH) {
	m_camera.perspective(glm::radians(72.0f), static_cast<float>(Application::Width) / static_cast<float>(Application::Height), 0.1f, 1000.0f);
	m_camera.orthographic(0.0f, static_cast<float>(Application::Width), 0.0f, static_cast<float>(Application::Height), -1.0f, 1.0f);
}

void NormalMap::applyTransformation(const TrackBall& arc) {
  m_uniforms.modelMatrix = arc.getTransform();
}

void NormalMap::renderUi(const WGPURenderPassEncoder& renderPassEncoder) {
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
	int currentTexturePack = m_texturePack;
	if (ImGui::Combo("Texturepack", &currentTexturePack, "Spiral\0Toybox\0BrickWall\0\0")) {
		m_texturePack = static_cast<TexturePack>(currentTexturePack);
		m_wgpCube.getMeshes().begin()->getBindGroups("BG")[1] = m_bindgroups[currentTexturePack];
	}
	ImGui::End();

	ImGui::Render();
	ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPassEncoder);
}

std::vector <WGPUBindGroupLayout> NormalMap::OnBindGroupLayouts() {
	std::vector<WGPUBindGroupLayout> bindingLayouts(2);

	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries0(3);
	WGPUBindGroupLayoutEntry& uniformLayout = bindingLayoutEntries0[0];
	uniformLayout.binding = 0u;
	uniformLayout.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
	uniformLayout.buffer.type = WGPUBufferBindingType::WGPUBufferBindingType_Uniform;
	uniformLayout.buffer.minBindingSize = sizeof(Uniforms);

	WGPUBindGroupLayoutEntry& normalUniformLayout = bindingLayoutEntries0[1];
	normalUniformLayout.binding = 1u;
	normalUniformLayout.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
	normalUniformLayout.buffer.type = WGPUBufferBindingType::WGPUBufferBindingType_Uniform;
	normalUniformLayout.buffer.minBindingSize = sizeof(NormalUniforms);

	WGPUBindGroupLayoutEntry& samplerBindingLayout = bindingLayoutEntries0[2];
	samplerBindingLayout.binding = 2u;
	samplerBindingLayout.visibility = WGPUShaderStage_Fragment;
	samplerBindingLayout.sampler.type = WGPUSamplerBindingType::WGPUSamplerBindingType_Filtering;

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor0 = {};
	bindGroupLayoutDescriptor0.entryCount = (uint32_t)bindingLayoutEntries0.size();
	bindGroupLayoutDescriptor0.entries = bindingLayoutEntries0.data();

	bindingLayouts[0] = wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor0);

	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries1(3);
	WGPUBindGroupLayoutEntry& textureBindingLayoutA = bindingLayoutEntries1[0];
	textureBindingLayoutA.binding = 0u;
	textureBindingLayoutA.visibility = WGPUShaderStage_Fragment;
	textureBindingLayoutA.texture.sampleType = WGPUTextureSampleType::WGPUTextureSampleType_Float;
	textureBindingLayoutA.texture.viewDimension = WGPUTextureViewDimension::WGPUTextureViewDimension_2D;

	WGPUBindGroupLayoutEntry& textureBindingLayoutN = bindingLayoutEntries1[1];
	textureBindingLayoutN.binding = 1u;
	textureBindingLayoutN.visibility = WGPUShaderStage_Fragment;
	textureBindingLayoutN.texture.sampleType = WGPUTextureSampleType::WGPUTextureSampleType_Float;
	textureBindingLayoutN.texture.viewDimension = WGPUTextureViewDimension::WGPUTextureViewDimension_2D;

	WGPUBindGroupLayoutEntry& textureBindingLayoutH = bindingLayoutEntries1[2];
	textureBindingLayoutH.binding = 2u;
	textureBindingLayoutH.visibility = WGPUShaderStage_Fragment;
	textureBindingLayoutH.texture.sampleType = WGPUTextureSampleType::WGPUTextureSampleType_Float;
	textureBindingLayoutH.texture.viewDimension = WGPUTextureViewDimension::WGPUTextureViewDimension_2D;

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor1 = {};
	bindGroupLayoutDescriptor1.entryCount = (uint32_t)bindingLayoutEntries1.size();
	bindGroupLayoutDescriptor1.entries = bindingLayoutEntries1.data();

	bindingLayouts[1] = wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor1);

	return bindingLayouts;
}

std::vector<WGPUBindGroup> NormalMap::OnBindGroups() {
	std::vector<WGPUBindGroup> bindGroups(2);

	std::vector<WGPUBindGroupEntry> bindGroupEntries0(3);

	bindGroupEntries0[0].binding = 0u;
	bindGroupEntries0[0].buffer = m_uniformBuffer.getBuffer();
	bindGroupEntries0[0].offset = 0u;
	bindGroupEntries0[0].size = sizeof(Uniforms);

	bindGroupEntries0[1].binding = 1u;
	bindGroupEntries0[1].buffer = m_normalUniformBuffer.getBuffer();
	bindGroupEntries0[1].offset = 0u;
	bindGroupEntries0[1].size = sizeof(NormalUniforms);

	bindGroupEntries0[2].binding = 2u;
	bindGroupEntries0[2].sampler = wgpContext.getSampler(SS_LINEAR_CLAMP);

	WGPUBindGroupDescriptor bindGroupDesc0 = {};
	bindGroupDesc0.layout = wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at("RP_PTNTB"), 0u);
	bindGroupDesc0.entryCount = (uint32_t)bindGroupEntries0.size();
	bindGroupDesc0.entries = bindGroupEntries0.data();
	bindGroups[0] = wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc0);

	std::vector<WGPUBindGroupEntry> bindGroupEntries1(3);

	bindGroupEntries1[0].binding = 0u;
	bindGroupEntries1[0].textureView = m_textureAW.getTextureView();

	bindGroupEntries1[1].binding = 1u;
	bindGroupEntries1[1].textureView = m_textureNS.getTextureView();

	bindGroupEntries1[2].binding = 2u;
	bindGroupEntries1[2].textureView = m_textureHS.getTextureView();

	WGPUBindGroupDescriptor bindGroupDesc1 = {};
	bindGroupDesc1.layout = wgpuRenderPipelineGetBindGroupLayout(wgpContext.renderPipelines.at("RP_PTNTB"), 1u);
	bindGroupDesc1.entryCount = (uint32_t)bindGroupEntries1.size();
	bindGroupDesc1.entries = bindGroupEntries1.data();
	bindGroups[1] = wgpuDeviceCreateBindGroup(wgpContext.device, &bindGroupDesc1);

	return bindGroups;
}

glm::vec3 NormalMap::glm_vec3_transform_mat4(const glm::mat4& m, const glm::vec3& v){
  const float x = v[0];
  const float y = v[1];
  const float z = v[2];
  const float w = m[0][3] * x + m[1][3] * y + m[2][3] * z + m[3][3];

  return glm::vec3((m[0][0] * x + m[1][0] * y + m[2][0] * z + m[3][0]) / w,
                   (m[0][1] * x + m[1][1] * y + m[2][1] * z + m[3][1]) / w,
                   (m[0][2] * x + m[1][2] * y + m[2][2] * z + m[3][2]) / w);
}