#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <iostream>

#include "glError.h"
#include "Shape.h"
#include "Application.h"

struct VertexType {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec4 color;
};

float heightMap(const glm::vec2 position) {
  return 2.0 * sin(position.x) * sin(position.y);
}

VertexType getHeightMap(const glm::vec2 position) {
  const glm::vec2 dx(1.0, 0.0);
  const glm::vec2 dy(0.0, 1.0);

  VertexType v;
  float h = heightMap(position);
  float hx = 100.f * (heightMap(position + 0.01f * dx) - h);
  float hy = 100.f * (heightMap(position + 0.01f * dy) - h);

  v.position = glm::vec3(position, h);
  v.normal = glm::normalize(glm::vec3(-hx, -hy, 1.0));

  float c = sin(h * 5.f) * 0.5 + 0.5;
  v.color = glm::vec4(c, 1.0 - c, 1.0, 1.0);
  return v;
}

Shape::Shape(StateMachine& machine) : State(machine, States::SHAPE) {

  shader = new Shader("res/shader/shader.vert", "res/shader/shader.frag");
	glClearColor(0.494f, 0.686f, 0.796f, 1.0f);
	glClearDepth(1.0f);

	glCheckError(__FILE__, __LINE__);

  // creation of the mesh ------------------------------------------------------
  std::vector<VertexType> vertices;
  std::vector<GLuint> index;

  for (int y = 0; y <= size; ++y)
    for (int x = 0; x <= size; ++x) {
      float xx = (x - size / 2) * 0.1f;
      float yy = (y - size / 2) * 0.1f;
      vertices.push_back(getHeightMap({xx, yy}));
    }

  for (int y = 0; y < size; ++y)
    for (int x = 0; x < size; ++x) {
      index.push_back((x + 0) + (size + 1) * (y + 0));
      index.push_back((x + 1) + (size + 1) * (y + 0));
      index.push_back((x + 1) + (size + 1) * (y + 1));

      index.push_back((x + 1) + (size + 1) * (y + 1));
      index.push_back((x + 0) + (size + 1) * (y + 1));
      index.push_back((x + 0) + (size + 1) * (y + 0));
    }

  std::cout << "vertices=" << vertices.size() << std::endl;
  std::cout << "index=" << index.size() << std::endl;

  // creation of the vertex array buffer----------------------------------------

  // vbo
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexType),
               vertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // ibo
  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, index.size() * sizeof(GLuint),
               index.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // vao
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // bind vbo
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexType), (void*)0);

  glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexType), (void*)offsetof(VertexType, normal));

  glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexType), (void*)offsetof(VertexType, color));

  // bind the ibo
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

  // vao end
  glBindVertexArray(0);
}

Shape::~Shape() {
	
}

void Shape::fixedUpdate() {

}

void Shape::update() {
	
}

void Shape::render() {

  // set matrix : projection + view
  projection = glm::perspective(float(2.0 * atan(Application::Height / 1920.f)),
                                float(Application::Width) / float(Application::Height), 0.1f, 100.f);
  view = glm::lookAt(glm::vec3(20.0 * sin(m_dt), 20.0 * cos(m_dt), 20.0),
                     glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 1.0));

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  shader->use();
  shader->loadMatrix("p", projection);
  shader->loadMatrix("v", view);
 

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

  glDrawElements(GL_TRIANGLES, size * size * 2 * 3, GL_UNSIGNED_INT, NULL);

  glBindVertexArray(0);

  shader->unuse();
}