#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_wgpu.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>

#include <WebGPU/WgpContext.h>
#include "Default.h"
#include "Application.h"

Default::Default(StateMachine& machine) : State(machine, States::DEFAULT) {

	m_uniformBuffer.createBuffer(sizeof(Uniforms), WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform);
	wgpContext.addSampler(wgpCreateSampler());
	m_texture = wgpCreateTexture(512, 512, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst, WGPUTextureFormat::WGPUTextureFormat_RGBA8Unorm);
	m_textureView = wgpCreateTextureView(WGPUTextureFormat::WGPUTextureFormat_RGBA8Unorm, WGPUTextureAspect::WGPUTextureAspect_All, m_texture);

	wgpContext.addSahderModule("PTN", "res/shader/shader.wgsl");
	wgpContext.createRenderPipelinePTN("PTN", std::bind(&Default::OnBindGroupLayoutPTN, this));

	wgpContext.addSahderModule("wireframe", "res/shader/wireframe.wgsl");
	wgpContext.createRenderPipelineWireframe("wireframe", std::bind(&Default::OnBindGroupLayoutWireframe, this));

	m_camera.perspective(45.0f, static_cast<float>(Application::Width) / static_cast<float>(Application::Height), 0.1f, 1000.0f);
	m_camera.orthographic(0.0f, static_cast<float>(Application::Width), 0.0f, static_cast<float>(Application::Height), -1.0f, 1.0f);
	m_camera.lookAt(glm::vec3(1.0f, 1.0f, 3.0f), glm::vec3(0.2f, 0.2f, 1.5f) + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	m_camera.setRotationSpeed(0.125f);
	m_camera.setMovingSpeed(10.0f);

	m_mammoth.loadModel("res/models/mammoth.obj");
	m_wgpMammoth.create(m_mammoth, m_textureView, m_uniformBuffer);

	m_dragon.loadModel("res/models/dragon/dragon.obj", glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, glm::vec3(0.0f, -1.0f, 0.0f), 0.1f, false, false, false, false, false, true);
	m_dragon.rewind();
	m_wgpDragon.create(m_dragon, m_textureView, m_uniformBuffer);

	m_trackball.reshape(Application::Width, Application::Height);
	m_trackball.setTrackballScale(0.5f);
	m_uniforms.projectionMatrix = glm::mat4(1.0f);
	m_uniforms.viewMatrix = glm::mat4(1.0f);
	m_uniforms.modelMatrix = glm::mat4(1.0f);
	m_uniforms.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getWgpuBuffer(), 0, &m_uniforms, sizeof(Uniforms));
	wgpContext.OnDraw = std::bind(&Default::OnDraw, this, std::placeholders::_1);
}

Default::~Default() {
	
}

void Default::fixedUpdate() {

}

void Default::update() {
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
		std::cout << "Delta: " << dx << "  " << dy << std::endl;
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
}

void Default::render() {
	wgpDraw();
}

void Default::OnDraw(const WGPURenderPassEncoder& renderPassEncoder) {

	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getWgpuBuffer(), offsetof(Uniforms, projectionMatrix), &m_uniforms.projectionMatrix, sizeof(Uniforms::projectionMatrix));
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getWgpuBuffer(), offsetof(Uniforms, viewMatrix), &m_uniforms.viewMatrix, sizeof(Uniforms::viewMatrix));
	wgpuQueueWriteBuffer(wgpContext.queue, m_uniformBuffer.getWgpuBuffer(), offsetof(Uniforms, modelMatrix), &m_uniforms.modelMatrix, sizeof(Uniforms::modelMatrix));
	
	wgpuRenderPassEncoderSetViewport(renderPassEncoder, 0.0f, 0.0f, static_cast<float>(Application::Width), static_cast<float>(Application::Height), 0.0f, 1.0f);
	
	m_model == MAMMOTH ? m_wgpMammoth.draw(renderPassEncoder) : m_wgpDragon.draw(renderPassEncoder);

	if (m_drawUi)
		renderUi(renderPassEncoder);
}


void Default::OnMouseButtonDown(const Event::MouseButtonEvent& event) {
	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT) {
		Mouse::instance().attach(Application::Window, true, false, false);
	}

	if (event.button == Event::MouseButtonEvent::BUTTON_LEFT) {
		m_trackball.mouse(TrackBall::Button::ELeftButton, TrackBall::Modifier::ENoModifier, true, event.x, event.y);
		applyTransformation(m_trackball);
	}
}

void Default::OnMouseButtonUp(const Event::MouseButtonEvent& event) {
	if (event.button == Event::MouseButtonEvent::BUTTON_RIGHT) {
		Mouse::instance().detach();
	} 

	if (event.button == Event::MouseButtonEvent::BUTTON_LEFT) {
		m_trackball.mouse(TrackBall::Button::ELeftButton, TrackBall::Modifier::ENoModifier, false, event.x, event.y);
		applyTransformation(m_trackball);
	} 
}

void Default::OnMouseMotion(const Event::MouseMoveEvent& event) {
    m_trackball.motion(event.x, event.y);
	applyTransformation(m_trackball);
}

void Default::OnKeyDown(const Event::KeyboardEvent& event){
	//if (event.keyCode == GLFW_KEY_T) {
	//	if(glfwGetInputMode(Application::Window, GLFW_CURSOR) == GLFW_CURSOR_HIDDEN){
	//		glfwSetInputMode(Application::Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	//	}else{
	//		glfwSetInputMode(Application::Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	//	}
	//}
}

void Default::OnKeyUp(const Event::KeyboardEvent& event) {

}

void Default::applyTransformation(const TrackBall& arc) {
  m_uniforms.modelMatrix = arc.getTransform();
}

void Default::renderUi(const WGPURenderPassEncoder& renderPassEncoder) {
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
		ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Left, 0.2f, nullptr, &dockSpaceId);
		//ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Right, 0.2f, nullptr, &dockSpaceId);
		//ImGuiID dock_id_down = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Down, 0.2f, nullptr, &dockSpaceId);
		//ImGuiID dock_id_up = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Up, 0.2f, nullptr, &dockSpaceId);
		ImGui::DockBuilderDockWindow("Settings", dock_id_left);
	}

	// render widgets
	ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("Application average FPS %.1f", static_cast<double>(ImGui::GetIO().Framerate));
	if (ImGui::Checkbox("Draw Wirframe", &StateMachine::GetWireframeEnabled())  || StateMachine::IsWireframeToggled()) {
		m_wgpMammoth.setRenderPipelineSlot(StateMachine::GetWireframeEnabled() ? RP_WIREFRAME : RP_PTN);
		m_wgpDragon.setRenderPipelineSlot(StateMachine::GetWireframeEnabled() ? RP_WIREFRAME : RP_PTN);
	}

	int currentModel = m_model;
	if (ImGui::Combo("Model", &currentModel, "Mammoth\0Dragon\0\0")) {
		m_model = static_cast<Model>(currentModel);
	}
	ImGui::End();

	ImGui::Render();
	ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPassEncoder);
}

WGPUBindGroupLayout Default::OnBindGroupLayoutPTN() {
	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries(3);

	bindingLayoutEntries.resize(3);
	WGPUBindGroupLayoutEntry& bindingLayout = bindingLayoutEntries[0];
	bindingLayout.binding = 0;
	bindingLayout.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
	bindingLayout.buffer.type = WGPUBufferBindingType_Uniform;
	bindingLayout.buffer.minBindingSize = sizeof(Uniforms);

	WGPUBindGroupLayoutEntry& textureBindingLayout = bindingLayoutEntries[1];
	textureBindingLayout.binding = 1;
	textureBindingLayout.visibility = WGPUShaderStage_Fragment;
	textureBindingLayout.texture.sampleType = WGPUTextureSampleType_Float;
	textureBindingLayout.texture.viewDimension = WGPUTextureViewDimension_2D;

	WGPUBindGroupLayoutEntry& samplerBindingLayout = bindingLayoutEntries[2];
	samplerBindingLayout.binding = 2;
	samplerBindingLayout.visibility = WGPUShaderStage_Fragment;
	samplerBindingLayout.sampler.type = WGPUSamplerBindingType_Filtering;

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = {};
	bindGroupLayoutDescriptor.entryCount = (uint32_t)bindingLayoutEntries.size();
	bindGroupLayoutDescriptor.entries = bindingLayoutEntries.data();
	return wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor);
}

WGPUBindGroupLayout Default::OnBindGroupLayoutWireframe() {
	std::vector<WGPUBindGroupLayoutEntry> bindingLayoutEntries(4);

	WGPUBindGroupLayoutEntry& bindingLayout = bindingLayoutEntries[0];
	bindingLayout.binding = 0;
	bindingLayout.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
	bindingLayout.buffer.type = WGPUBufferBindingType_Uniform;
	bindingLayout.buffer.minBindingSize = sizeof(Uniforms);

	WGPUBindGroupLayoutEntry& indiceBindingLayout = bindingLayoutEntries[1];
	indiceBindingLayout.binding = 1;
	indiceBindingLayout.visibility = WGPUShaderStage_Vertex;
	indiceBindingLayout.buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
	indiceBindingLayout.buffer.hasDynamicOffset = false;

	WGPUBindGroupLayoutEntry& vertexBindingLayout = bindingLayoutEntries[2];
	vertexBindingLayout.binding = 2;
	vertexBindingLayout.visibility = WGPUShaderStage_Vertex;
	vertexBindingLayout.buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
	vertexBindingLayout.buffer.hasDynamicOffset = false;

	WGPUBindGroupLayoutEntry& colorBindingLayout = bindingLayoutEntries[3];
	colorBindingLayout.binding = 3;
	colorBindingLayout.visibility = WGPUShaderStage_Vertex;
	colorBindingLayout.buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
	colorBindingLayout.buffer.hasDynamicOffset = false;

	WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = {};
	bindGroupLayoutDescriptor.entryCount = (uint32_t)bindingLayoutEntries.size();
	bindGroupLayoutDescriptor.entries = bindingLayoutEntries.data();
	return wgpuDeviceCreateBindGroupLayout(wgpContext.device, &bindGroupLayoutDescriptor);
}