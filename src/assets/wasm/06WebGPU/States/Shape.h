#pragma once

#include <filesystem>
#include <webgpu/webgpu.h>
#include <States/StateMachine.h>
#include "Shader.h"





class Shape : public State {

public:

	Shape(StateMachine& machine);
	~Shape();

	void fixedUpdate() override;
	void update() override;
	void render() override;
	
private:

};