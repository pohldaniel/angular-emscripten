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

ShapeState::ShapeState(StateMachine& machine) : State(machine, States::SHAPE), m_currentShape(m_torus) {

  shaderTexure = new Shader("res/shader/shader.vert", "res/shader/texture.frag");
  shaderNormal = new Shader("res/shader/shader.vert", "res/shader/normal.frag");
  //shaderGeometry = new Shader("res/shader/shader.vert", "res/shader/normalGS.frag", "res/shader/normalGS.gem");
  shaderTangent = new Shader("res/shader/shader.vert", "res/shader/tangent.frag");
  shaderBitangent = new Shader("res/shader/shader.vert", "res/shader/bitangent.frag");

  vertex1 = Shader::LoadShaderProgram(GL_VERTEX_SHADER, "res/shader/gauss.vert");
  vertex2 = Shader::LoadShaderProgram(GL_VERTEX_SHADER, "res/shader/gauss.vert");
  recompileShader();
  object = new Shader("res/shader/object.vert", "res/shader/object.frag");

  glClearColor(0.494f, 0.686f, 0.796f, 1.0f);
  glClearDepth(1.0f);

  m_camera.perspective(45.0f, static_cast<float>(Application::Width) / static_cast<float>(Application::Height), 0.1f, 1000.0f);
  m_camera.lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0, 1.0, 0.0));
  m_camera.setRotationSpeed(0.01f);
  m_camera.setMovingSpeed(5.0f);

  m_sphere.buildSphere(1.0f, glm::vec3(0.0f, 0.0f, 0.0f), 49, 49, true, true, true);
  m_sphere.markForDelete();

  m_torus.buildTorus(0.5f, 0.25f, glm::vec3(0.0f, 0.0f, 0.0f), 49, 49, true, true, true);
  m_torus.markForDelete();

  m_torusknot.buildTorusKnot(1.0f, 0.25f, 2, 3, glm::vec3(0.0f, 0.0f, 0.0f), 100, 16, true, true, true);
  m_torusknot.markForDelete();

  m_grid.loadFromFile("res/textures/grid512.png", true);
  m_hdriCross.loadCrossHDRIFromFile("res/textures/grace_new_cross.hdr");

  Mouse::instance().attach(Application::Window, false, false, false);

  m_trackball.reshape(Application::Width, Application::Height);
  m_currentShader = shaderTexure;

  createBuffers(bufferTokens[currentBuffer], rbTokens[currentBuffer], aaModes[currentMode]);
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
  m_currentShader->use();
  m_currentShader->loadMatrix("u_projection", m_camera.getPerspectiveMatrix());
  m_currentShader->loadMatrix("u_view", m_camera.getViewMatrix());
  m_currentShader->loadMatrix("u_model", m_transform);
  m_currentShader->loadMatrix("u_normal", glm::inverseTranspose(m_transform));
 
  m_currentShape.get().drawRaw();

  m_currentShader->unuse();

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

    int currentModel = model;
	  if (ImGui::Combo("Model", &currentModel, "Torus\0Sphere\0Torus Knot\0\0")) {
		  model = static_cast<Model>(currentModel);
		  switch (model) {
        case Model::TORUS:
			    m_currentShape = std::ref(m_torus);
			    break;
        case Model::SPHERE:
			    m_currentShape = std::ref(m_sphere);
			    break;
        case Model::TORUSKNOT:
			    m_currentShape = std::ref(m_torusknot);
			    break;
      }
    }

    int currentRenderMode = renderMode;
	  if (ImGui::Combo("Render Mode", &currentRenderMode, "Texture\0Normal\0Tangent\0Bitangent\0Geometry\0Hdr Cross\0Hdr Env\0\0")) {
		  renderMode = static_cast<RenderMode>(currentRenderMode);
		  switch (renderMode) {
		    case RenderMode::TEXTURE:
			    m_currentShader = shaderTexure;
			    break;
		    case RenderMode::NORMAL:
			    m_currentShader = shaderNormal;
			    break;
        case RenderMode::GEOMETRY:
			    //m_currentShader = shaderGeometry;
			    break;
        case RenderMode::TANGENT:
			    m_currentShader = shaderTangent;
			    break;
        case RenderMode::BITANGENT:
			    m_currentShader = shaderBitangent;
			    break;
		  }
	  }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ShapeState::createBuffers(AttachmentTex::AttachmentTex texFormat, AttachmentRB::AttachmentRB rbFormat, aaInfo aaMode){
  sceneBuffer.cleanup();
	msaaBuffer.cleanup();

	for (int i = 0; i<DOWNSAMPLE_BUFFERS; i++) {
		downsampleBuffer[i].cleanup();
	}

	for (int i = 0; i<BLUR_BUFFERS; i++) {
		blurBuffer[i].cleanup();
	}

	sceneBuffer.create(Application::Width, Application::Height);
	sceneBuffer.attachTexture(texFormat);
	sceneBuffer.attachRenderbuffer(AttachmentRB::DEPTH24);

	if (aaMode.samples > 0) {
		// create multisampled fbo
		msaaBuffer.create(Application::Width, Application::Height);
		msaaBuffer.attachRenderbuffer(rbFormat, aaMode.samples);
		msaaBuffer.attachRenderbuffer(AttachmentRB::DEPTH24, aaMode.samples);
	}

	for (int i = 0; i < BLUR_BUFFERS; i++) {
		blurBuffer[i].create(Application::Width / 4, Application::Height / 4);
		blurBuffer[i].attachTexture(texFormat);
	}

	int w = Application::Width;
	int h = Application::Height;
	for (int i = 0; i < DOWNSAMPLE_BUFFERS; i++) {
		w /= 2;
		h /= 2;
		downsampleBuffer[i].create(w, h);
		downsampleBuffer[i].attachTexture(texFormat);
	}
}

void ShapeState::recompileShader() {
	float *weights;
	int width;
	weights = generateGaussianWeights(m_blurWidth, width);
	generate1DConvolutionFP_filter(blurH, weights, width, false, true, Application::Width / 2, Application::Height / 2);
	generate1DConvolutionFP_filter(blurV, weights, width, true, true, Application::Width / 2, Application::Height / 2);
}

// 1d Gaussian distribution, s is standard deviation
float gaussian(float x, float s) {
	return expf(-x*x / (2.0f*s*s)) / (s*sqrtf(2.0f * glm::pi<float>()));
}

// generate array of weights for Gaussian blur
float* ShapeState::generateGaussianWeights(float s, int &width) {
	width = (int)floor(3.0f*s) - 1;
	int size = width * 2 + 1;
	float *weight = new float[size];

	float sum = 0.0;
	int x;
	for (x = 0; x<size; x++) {
		weight[x] = gaussian((float)x - width, s);
		sum += weight[x];
	}

	for (x = 0; x<size; x++) {
		weight[x] /= sum;
	}
	return weight;
}

float* generateTriangleWeights(int width) {
	float *weights = new float[width];
	float sum = 0.0f;
	for (int i = 0; i<width; i++) {
		float t = i / (float)(width - 1);
		weights[i] = 1.0f - abs(t - 0.5f)*2.0f;
		sum += weights[i];
	}

	for (int i = 0; i<width; i++) {
		weights[i] /= sum;
	}
	return weights;
}

void ShapeState::generate1DConvolutionFP_filter(Shader*& shader, float *weights, int width, bool vertical, bool tex2D, int img_width, int img_height) {
	// calculate new set of weights and offsets
	int nsamples = 2 * width + 1;
	int nsamples2 = (int)ceilf(nsamples / 2.0f);
	float *weights2 = new float[nsamples2];
	float *offsets = new float[nsamples2];

	for (int i = 0; i<nsamples2; i++) {
		float a = weights[i * 2];
		float b;
		if (i * 2 + 1 > nsamples - 1)
			b = 0;
		else
			b = weights[i * 2 + 1];
		weights2[i] = a + b;
		offsets[i] = b / (a + b);
	}

	std::ostringstream ost;
	ost << "#version 300 es" << std::endl
    << "precision mediump float;" << std::endl
    << "precision mediump int;" << std::endl
    << "precision mediump sampler2DArray;" << std::endl << std::endl
		<< "in vec2 vTexCoord;" << std::endl
		<< "uniform sampler2D TexSampler;" << std::endl
		<< "out vec4 color;" << std::endl << std::endl
		<< "void main(){" << std::endl
		<< "vec3 sum = vec3(0.0,0.0,0.0);" << std::endl
		<< "\tvec2 texcoord;" << std::endl;
		for (int i = 0; i < nsamples2; i++) {
			float x_offset = 0, y_offset = 0;
			if (vertical) {
				y_offset = (i * 2) - width + offsets[i];
			}
			else {
				x_offset = (i * 2) - width + offsets[i];
			}
			if (tex2D) {
				x_offset = x_offset / img_width;
				y_offset = y_offset / img_height;
			}
			float weight = weights2[i];
			ost << "\ttexcoord = vTexCoord + vec2(" << x_offset << ", " << y_offset << ");" << std::endl;
			ost << "\tsum += texture(TexSampler, texcoord).rgb *" << weight << ";" << std::endl;
		}
	ost << "\tcolor = vec4(sum, 1.0);" << std::endl;
	ost << "}";
	
	delete[] weights2;
	delete[] offsets;
	
	if (shader)
		delete shader;

	shader = new Shader();

	shader->attachShader(vertical ? vertex2 : vertex1);
	shader->attachShader(Shader::LoadShaderProgram(GL_FRAGMENT_SHADER, ost.str()));
	shader->linkShaders();
}