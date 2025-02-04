#include <GL/glew.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
#include <glm/glm.hpp>

#include "ShapeState.h"
#include "Application.h"

ShapeState::ShapeState(StateMachine& machine) : State(machine, States::SHAPE) {

  shader = new Shader("res/shader/shader.vert", "res/shader/shader.frag");
  glClearColor(0.494f, 0.686f, 0.796f, 1.0f);
  glClearDepth(1.0f);

  m_camera.perspective(45.0f, static_cast<float>(Application::Width) / static_cast<float>(Application::Height), 0.1f, 1000.0f);
  m_camera.lookAt(glm::vec3(0.0f, 0.0f, 1.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0, 1.0, 0.0));

  m_sphere.buildSphere(5.0f, glm::vec3(0.0f, 0.0f, 0.0f), 49, 49, true, false, false);
  m_sphere.markForDelete();

  m_grid.loadFromFile("res/textures/grid512.png", true);
}

ShapeState::~ShapeState() {
	
}

void ShapeState::fixedUpdate() {

}

void ShapeState::update() {
	
}

void ShapeState::render() {

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  m_grid.bind();
  shader->use();
  shader->loadMatrix("u_projection", m_camera.getPerspectiveMatrix());
  shader->loadMatrix("u_view", m_camera.getViewMatrix());
  shader->loadMatrix("u_model", glm::mat4(1.0f));
  shader->loadVector("u_color", glm::vec4(1.0f));
 
  m_sphere.drawRaw();

  shader->unuse();

  if (m_drawUi)
		renderUi();
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
    ImGui::Checkbox("Rotate", &m_rotate);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}