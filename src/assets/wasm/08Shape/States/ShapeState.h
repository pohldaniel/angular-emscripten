#include <States/StateMachine.h>
#include <MeshObject/Shape.h>
#include "Mouse.h"
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "TrackBall.h"

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
	bool m_checked = false;

    Shader* shader;
	Shape m_sphere;
	Texture m_grid;
	Camera m_camera;
	TrackBall m_trackball;
	glm::mat4 m_transform;
};