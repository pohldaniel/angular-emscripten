#include <States/StateMachine.h>
#include <MeshObject/Shape.h>
#include "Shader.h"

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

	Shader* shader;
	bool m_initUi = true;
	bool m_drawUi = true;
	bool m_rotate = false;

	glm::mat4 projection = glm::mat4(1.0);
	glm::mat4 view = glm::mat4(1.0);
	Shape m_sphere;
};