#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
#include <glm/glm.hpp>
#include <iostream>

#include "Mouse.h"
#include "ShapeState.h"
#include "Application.h"

ShapeState::ShapeState(StateMachine& machine) : State(machine, States::SHAPE) {

  shader = new Shader("res/shader/shader.vert", "res/shader/shader.frag");
  glClearColor(0.494f, 0.686f, 0.796f, 1.0f);
  glClearDepth(1.0f);

  m_camera.perspective(45.0f, static_cast<float>(Application::Width) / static_cast<float>(Application::Height), 0.1f, 1000.0f);
  m_camera.lookAt(glm::vec3(0.0f, 0.0f, 15.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0, 1.0, 0.0));
  m_camera.setRotationSpeed(0.01f);
  m_camera.setMovingSpeed(5.0f);

  m_sphere.buildSphere(5.0f, glm::vec3(0.0f, 0.0f, 0.0f), 49, 49, true, false, false);
  m_sphere.markForDelete();

  m_grid.loadFromFile("res/textures/grid512.png", true);

  Mouse::instance().attach(Application::Window, false, false, false);

  m_trackball.reshape(Application::Width, Application::Height);
}

ShapeState::~ShapeState() {
	
}

void ShapeState::fixedUpdate() {

}

void ShapeState::update() {
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

  Mouse &mouse = Mouse::instance();
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
}

void ShapeState::render() {

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  m_grid.bind();
  shader->use();
  shader->loadMatrix("u_projection", m_camera.getPerspectiveMatrix());
  shader->loadMatrix("u_view", m_camera.getViewMatrix());
  shader->loadMatrix("u_model", m_transform);
  shader->loadVector("u_color", glm::vec4(1.0f));
 
  m_sphere.drawRaw();

  shader->unuse();

  if (m_drawUi)
		renderUi();
}

void ShapeState::OnMouseButtonDown(const Event::MouseButtonEvent& event) {
	if (event.button == 1u) {
		m_trackball.mouse(TrackBall::Button::ELeftButton, TrackBall::Modifier::ENoModifier, true, event.x, event.y);
		applyTransformation(m_trackball);
	}
}

void ShapeState::OnMouseButtonUp(const Event::MouseButtonEvent& event) {
	if (event.button == 1u) {
		m_trackball.mouse(TrackBall::Button::ELeftButton, TrackBall::Modifier::ENoModifier, false, event.x, event.y);
		applyTransformation(m_trackball);
	} 
}

void ShapeState::OnMouseMotion(const Event::MouseMoveEvent& event) {
  m_trackball.motion(event.x, event.y);
	applyTransformation(m_trackball);
}

void ShapeState::OnKeyDown(const Event::KeyboardEvent& event){

}

void ShapeState::OnKeyUp(const Event::KeyboardEvent& event) {

}

void ShapeState::applyTransformation(const TrackBall& arc) {
  m_transform = arc.getTransform();
}

void ShapeState::renderUi() {
    ImGui_ImplOpenGL3_NewFrame();
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
        ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Right, 0.2f, nullptr, &dockSpaceId);
        ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Down, 0.2f, nullptr, &dockSpaceId);
        ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Up, 0.2f, nullptr, &dockSpaceId);
        ImGui::DockBuilderDockWindow("Settings", dock_id_left);
    }

    // render widgets
    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
            1000.0/static_cast<double>(ImGui::GetIO().Framerate), static_cast<double>(ImGui::GetIO().Framerate));
    ImGui::Checkbox("Rotate", &m_rotate);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}