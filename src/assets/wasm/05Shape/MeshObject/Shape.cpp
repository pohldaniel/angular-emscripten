#include "Shape.h"
#include "MeshSphere.h"
#include "MeshTorus.h"
#include "MeshTorusKnot.h"

Shape::Shape() : m_markForDelete(false) { 

}

Shape::Shape(Shape const& rhs) {
	m_vao = rhs.m_vao;
	m_vbo[0] = rhs.m_vbo[0];
	m_vbo[1] = rhs.m_vbo[1];
	m_vbo[2] = rhs.m_vbo[2];
	m_vbo[3] = rhs.m_vbo[3];
	m_vbo[4] = rhs.m_vbo[4];
	m_vbo[5] = rhs.m_vbo[5];
	m_vbo[6] = rhs.m_vbo[6];
	m_drawCount = rhs.m_drawCount;
	m_positions.insert(m_positions.end(), rhs.m_positions.begin(),rhs.m_positions.end());
	m_indexBuffer.insert(m_indexBuffer.end(), rhs.m_indexBuffer.begin(), rhs.m_indexBuffer.end());
	m_markForDelete = false;
}

Shape::Shape(Shape&& rhs) {
	m_vao = rhs.m_vao;
	m_vbo[0] = rhs.m_vbo[0];
	m_vbo[1] = rhs.m_vbo[1];
	m_vbo[2] = rhs.m_vbo[2];
	m_vbo[3] = rhs.m_vbo[3];
	m_vbo[4] = rhs.m_vbo[4];
	m_vbo[5] = rhs.m_vbo[5];
	m_vbo[6] = rhs.m_vbo[6];
	m_drawCount = rhs.m_drawCount;
	m_positions = rhs.m_positions;
	m_indexBuffer = rhs.m_indexBuffer;
	m_markForDelete = false;
}

Shape& Shape::operator=(const Shape& rhs) {
	m_vao = rhs.m_vao;
	m_vbo[0] = rhs.m_vbo[0];
	m_vbo[1] = rhs.m_vbo[1];
	m_vbo[2] = rhs.m_vbo[2];
	m_vbo[3] = rhs.m_vbo[3];
	m_vbo[4] = rhs.m_vbo[4];
	m_vbo[5] = rhs.m_vbo[5];
	m_vbo[6] = rhs.m_vbo[6];
	m_drawCount = rhs.m_drawCount;
	m_positions.insert(m_positions.end(), rhs.m_positions.begin(), rhs.m_positions.end());
	m_indexBuffer.insert(m_indexBuffer.end(), rhs.m_indexBuffer.begin(), rhs.m_indexBuffer.end());
	m_markForDelete = false;
	return *this;
}

Shape& Shape::operator=(Shape&& rhs) {
	m_vao = rhs.m_vao;
	m_vbo[0] = rhs.m_vbo[0];
	m_vbo[1] = rhs.m_vbo[1];
	m_vbo[2] = rhs.m_vbo[2];
	m_vbo[3] = rhs.m_vbo[3];
	m_vbo[4] = rhs.m_vbo[4];
	m_vbo[5] = rhs.m_vbo[5];
	m_vbo[6] = rhs.m_vbo[6];
	m_drawCount = rhs.m_drawCount;
	m_positions = rhs.m_positions;
	m_indexBuffer = rhs.m_indexBuffer;
	m_markForDelete = false;
	return *this;
}

void Shape::buildSphere(float radius, const glm::vec3& position, int uResolution, int vResolution, bool generateTexels, bool generateNormals, bool generateTangents) {
	MeshSphere::BuildMesh(radius, position, uResolution, vResolution, generateTexels, generateNormals, generateTangents, m_positions, m_texels, m_normals, m_indexBuffer, m_tangents, m_bitangents);
	createBuffer();
}

void Shape::buildTorus(float radius, float tubeRadius, const glm::vec3& position, int uResolution, int vResolution, bool generateTexels, bool generateNormals, bool generateTangents) {
	MeshTorus::BuildMesh(radius, tubeRadius, position, uResolution, vResolution, generateTexels, generateNormals, generateTangents, m_positions, m_texels, m_normals, m_indexBuffer, m_tangents, m_bitangents);
	createBuffer();
}

void Shape::buildTorusKnot(float radius, float tubeRadius, int p, int q, const glm::vec3& position, int uResolution, int vResolution, bool generateTexels, bool generateNormals, bool generateTangents) {
	MeshTorusKnot::BuildMesh(radius, tubeRadius, p, q, position, uResolution, vResolution, generateTexels, generateNormals, generateTangents, m_positions, m_texels, m_normals, m_indexBuffer, m_tangents, m_bitangents);
	createBuffer();
}

const std::vector<glm::vec3>& Shape::getPositions() const{
	return m_positions;
}

const std::vector<unsigned int>& Shape::getIndexBuffer() const {
	return m_indexBuffer;
}

unsigned int Shape::getNumberOfTriangles() const {
	return m_drawCount / 3;
}

Shape::~Shape() {
	if (m_markForDelete) {
		cleanup();
	}
}

void Shape::cleanup() {

	if (m_vao)
		glDeleteVertexArrays(1, &m_vao);

	if (m_vbo[0])
		glDeleteBuffers(1, &m_vbo[0]);

	if (m_vbo[1])
		glDeleteBuffers(1, &m_vbo[1]);

	if (m_vbo[2])
		glDeleteBuffers(1, &m_vbo[2]);

	if (m_vbo[3])
		glDeleteBuffers(1, &m_vbo[3]);

	if (m_vbo[4])
		glDeleteBuffers(1, &m_vbo[4]);

	if (m_vbo[5])
		glDeleteBuffers(1, &m_vbo[5]);

	if (m_vbo[6])
		glDeleteBuffers(1, &m_vbo[6]);

	m_positions.clear();
	m_positions.shrink_to_fit();
	m_indexBuffer.clear();
	m_indexBuffer.shrink_to_fit();
}

void Shape::markForDelete() {
	m_markForDelete = true;
}

void Shape::createBuffer() {

	m_drawCount = m_indexBuffer.size();

	unsigned int ibo;
	glGenBuffers(1, &ibo);
	glGenBuffers(!m_positions.empty() + !m_texels.empty() + !m_normals.empty() + !m_tangents.empty() + !m_bitangents.empty(), m_vbo);

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	//Position
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, m_positions.size() * sizeof(m_positions[0]), &m_positions[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	unsigned short index = 1;
	//Texture Coordinates
	if (!m_texels.empty()) {
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo[index]);
		glBufferData(GL_ARRAY_BUFFER, m_texels.size() * sizeof(m_texels[0]), &m_texels[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
		index++;
	}

	//Normals
	if (!m_normals.empty()) {
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo[index]);
		glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(m_normals[0]), &m_normals[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		index++;
	}

	//tangents
	if (!m_tangents.empty()) {
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo[index]);
		glBufferData(GL_ARRAY_BUFFER, m_tangents.size() * sizeof(m_tangents[0]), &m_tangents[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
		index++;

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo[index]);
		glBufferData(GL_ARRAY_BUFFER, m_bitangents.size() * sizeof(m_bitangents[0]), &m_bitangents[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);
		index++;
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer.size() * sizeof(m_indexBuffer[0]), &m_indexBuffer[0], GL_STATIC_DRAW);
	glBindVertexArray(0);

	glDeleteBuffers(1, &ibo);
	//m_positions.clear();
	//m_positions.shrink_to_fit();
	//m_indexBuffer.clear();
	//m_indexBuffer.shrink_to_fit();
	m_texels.clear();
	m_texels.shrink_to_fit();
	m_normals.clear();
	m_normals.shrink_to_fit();
	m_tangents.clear();
	m_tangents.shrink_to_fit();
	m_bitangents.clear();
	m_bitangents.shrink_to_fit();
}

void Shape::drawRaw() const {
	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, m_drawCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}