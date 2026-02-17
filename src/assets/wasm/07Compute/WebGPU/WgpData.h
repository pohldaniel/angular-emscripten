#pragma once
#include <array>
#include <glm/glm.hpp>

struct Uniforms {
  glm::mat4 projectionMatrix;
  glm::mat4 viewMatrix;
  glm::mat4 modelMatrix;
  std::array<float, 4> color;
};

struct ComputeUniforms {
  glm::mat3x4 kernel = glm::mat3x4(0.0);
  float test = 0.5f;
  uint32_t filterType = 0;
  float _pad[2];
};

struct LightingUniforms {
	std::array<glm::vec4, 2> directions;
	std::array<glm::vec4, 2> colors;
	float hardness = 32.0f;
	float kd = 1.0f;
	float ks = 0.5f;
	float _pad[1];
};