#ifndef _SHAPE_H
#define _SHAPE_H

#include <vector>
#include <array>
#include <cctype>
#include <iterator>
#include <map>

#include <GL/glew.h>
#include <glm/glm.hpp>

class Shape {

public:

	Shape();
	Shape(Shape const& rhs);
	Shape(Shape&& rhs);
	Shape& operator=(const Shape& rhs);
	Shape& operator=(Shape&& rhs);
	~Shape();

	void buildSphere(float radius = 1.0f, const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f), int uResolution = 49, int vResolution = 49, bool generateTexels = true, bool generateNormals = true, bool generateTangents = false);	
	void buildTorus(float radius = 0.5f, float tubeRadius = 0.25f, const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f), int uResolution = 49, int vResolution = 49, bool generateTexels = true, bool generateNormals = true, bool generateTangents = false);
	void buildTorusKnot(float radius = 1.0f, float tubeRadius = 0.4f, int p =2, int q =3, const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f), int uResolution = 100, int vResolution = 16, bool generateTexels = true, bool generateNormals = true, bool generateTangents = false);
	void drawRaw() const;

	unsigned int getNumberOfTriangles() const;
	const std::vector<glm::vec3>& getPositions() const;
	const std::vector<unsigned int>& getIndexBuffer() const;

	void cleanup();
	void markForDelete();

private:

	unsigned int m_vao;
	unsigned int m_vbo[7] = { 0u };
	unsigned int m_drawCount;


	std::vector<unsigned int> m_indexBuffer;
	std::vector<glm::vec3> m_positions;
	std::vector<glm::vec2> m_texels;
	std::vector<glm::vec3> m_normals;
	std::vector<glm::vec3> m_tangents;
	std::vector<glm::vec3> m_bitangents;
	bool m_markForDelete;

	void createBuffer();
};
#endif