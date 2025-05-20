#include <functional>
#include <glm/gtc/matrix_inverse.hpp>
#include <States/StateMachine.h>
#include <MeshObject/Shape.h>
#include "Mouse.h"
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "TrackBall.h"

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
	GEOMETRY
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

	bool m_initUi = true;
	bool m_drawUi = true;

    Shader *shaderTexure, *shaderNormal, *shaderGeometry, *shaderTangent, *shaderBitangent;
	Shape m_sphere, m_torus, m_torusknot;
	Shader* m_currentShader;
	std::reference_wrapper<Shape> m_currentShape;
	Texture m_grid;
	Camera m_camera;
	TrackBall m_trackball;
	glm::mat4 m_transform;
	Model model = Model::TORUS;
	RenderMode renderMode = RenderMode::TEXTURE;
};