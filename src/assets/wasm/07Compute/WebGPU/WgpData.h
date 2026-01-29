#pragma once
#include <array>
#include <glm/glm.hpp>

struct Uniforms {
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 modelMatrix;
	std::array<float, 4> color;
};

struct Uniforms_Compute {
  glm::mat3x4 kernel = glm::mat3x4(0.0);
  float test = 0.5f;
  uint32_t filterType = 0;
  float _pad[2];
};