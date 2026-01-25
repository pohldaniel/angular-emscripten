#pragma once
#include <array>
#include <glm/glm.hpp>

struct Uniforms {
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 modelMatrix;
	std::array<float, 4> color;
};