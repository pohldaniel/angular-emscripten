#include <States/StateMachine.h>
#include "Shader.h"

class Shape : public State {

public:

	Shape(StateMachine& machine);
	~Shape();

	void fixedUpdate() override;
	void update() override;
	void render() override;
	//void resize(int deltaW, int deltaH) override;
	
private:
	Shader* shader;

	const int size = 100;
	// shader matrix uniform
	glm::mat4 projection = glm::mat4(1.0);
	glm::mat4 view = glm::mat4(1.0);

	// VBO/VAO/ibo
	GLuint vao, vbo, ibo;
};