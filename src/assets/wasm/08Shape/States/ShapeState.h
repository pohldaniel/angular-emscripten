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
	
private:

	void renderUi();

	bool m_initUi = true;
	bool m_drawUi = true;
	bool m_rotate = false;

    Shader* shader;
	Shape m_sphere;
	Texture m_grid;
	Camera m_camera;
};