#include <functional>
#include <glm/gtc/matrix_inverse.hpp>
#include <States/StateMachine.h>
#include <MeshObject/Shape.h>
#include "Mouse.h"
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "TrackBall.h"
#include "Framebuffer.h"

enum Model {
	TORUS,
	SPHERE,
	TORUSKNOT
};

enum RenderMode {
	TEXTURE,
	NORMAL,	
	TANGENT,
	BITANGENT,
	GEOMETRY,
	HDR1,
	HDR2
};

struct aaInfo {
	int samples;
};

class ShapeState : public State {

public:

	ShapeState(StateMachine& machine);
	~ShapeState();

	void fixedUpdate() override;
	void update() override;
	void render() override;
	//void resize(int deltaW, int deltaH) override;

	void OnMouseMotion(const Event::MouseMoveEvent& event) override;
	void OnMouseButtonDown(const Event::MouseButtonEvent& event) override;
	void OnMouseButtonUp(const Event::MouseButtonEvent& event) override;
	void OnKeyDown(const Event::KeyboardEvent& event) override;
	void OnKeyUp(const Event::KeyboardEvent& event) override;

private:

	void renderUi();
    void applyTransformation(const TrackBall& arc);
    void createBuffers(AttachmentTex::AttachmentTex texFormat, AttachmentRB::AttachmentRB rbFormat, aaInfo aaMode);
	void recompileShader();
	void generate1DConvolutionFP_filter(Shader*& shader, float *weights, int width, bool vertical, bool tex2D, int img_width, int img_height);
	float *generateGaussianWeights(float s, int &width);

	bool m_initUi = true;
	bool m_drawUi = true;

    Shader *shaderTexure, *shaderNormal, *shaderGeometry, *shaderTangent, *shaderBitangent;

    Shader* tone;
	Shader* blurH = nullptr;
	Shader* blurV = nullptr;
	Shader* object;
	Shader* down2;
	Shader* down4;
    GLuint vertex1, vertex2;

	Shape m_sphere, m_torus, m_torusknot;
	Shader* m_currentShader;
	std::reference_wrapper<Shape> m_currentShape;
	Texture m_grid, m_hdriCross;
	Camera m_camera;
	TrackBall m_trackball;
	glm::mat4 m_transform;
	Model model = Model::TORUS;
	RenderMode renderMode = RenderMode::TEXTURE;

	Framebuffer sceneBuffer;
	Framebuffer blurBuffer[2];
	Framebuffer downsampleBuffer[2];
	Framebuffer msaaBuffer;

    int currentMode = 2;
	aaInfo aaModes[6] = { { 0 },{ 2 },{ 4 },{ 4 },{ 8 },{ 8 } };
	const char* aaModesLabel[6] = { "None", "2x", "4x", "8xCSAA", "8x", "16xCSAA" };

    int currentBuffer = 1;
	AttachmentTex::AttachmentTex bufferTokens[4] = { AttachmentTex::AttachmentTex::RGBA, AttachmentTex::AttachmentTex::RGBA16F, AttachmentTex::AttachmentTex::RGBA32F, AttachmentTex::AttachmentTex::R11FG11FB10F };
	AttachmentRB::AttachmentRB rbTokens[4] = { AttachmentRB::AttachmentRB::RGBA, AttachmentRB::AttachmentRB::RGBA16F, AttachmentRB::AttachmentRB::RGBA32F, AttachmentRB::AttachmentRB::R11FG11FB10F };

	float m_blurWidth = 3.0f;
};