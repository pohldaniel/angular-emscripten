#ifndef _MESHTORUSKNOT_H
#define _MESHTORUSKNOT_H

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

class MeshTorusKnot {

public:

	MeshTorusKnot(int uResolution = 100, int vResolution = 16);
	MeshTorusKnot(bool generateTexels, bool generateNormals, bool generateTangents, int uResolution = 100, int vResolution = 16);
	MeshTorusKnot(const glm::vec3& position, float radius, float tubeRadius, int p, int q, bool generateTexels, bool generateNormals, bool generateTangents, int uResolution = 100, int vResolution = 16);
	~MeshTorusKnot();

	void drawRaw();
	void setPrecision(int uResolution, int vResolution);
	int getNumberOfTriangles();
	std::vector<glm::vec3>& getPositions();
	std::vector<unsigned int>& getIndexBuffer();

	static void BuildMesh(float radius, float tubeRadius, int p, int q, const glm::vec3& position, int uResolution, int vResolution, bool generateTexels, bool generateNormals, bool generateTangents, std::vector<glm::vec3>& positions, std::vector<glm::vec2>& texels, std::vector<glm::vec3>& normals, std::vector<unsigned int>& indexBuffer, std::vector<glm::vec3>& tangents, std::vector<glm::vec3>& bitangents);

private:

	int m_uResolution;
	int m_vResolution;
	float m_radius;
	float m_tubeRadius;
	int m_p, m_q;
	glm::vec3 m_position;

	bool m_generateNormals;
	bool m_generateTexels;
	bool m_generateTangents;

	short m_numBuffers;
	unsigned int m_vao;
	unsigned int m_vbo[5];
	unsigned int m_drawCount;
	unsigned int m_vboInstances = 0;

	std::vector<unsigned int> m_indexBuffer;
	std::vector<glm::vec3> m_positions;
	std::vector<glm::vec2> m_texels;
	std::vector<glm::vec3> m_normals;
	std::vector<glm::vec3> m_tangents;
	std::vector<glm::vec3> m_bitangents;

	void createBuffer();

	static void CalculatePositionOnCurve(float u, float p, float q, float radius, glm::vec3& position);
	static void CalculatePositionOnCurve(float u, float p, float q, float r1, float r2, glm::vec3& position);
};
#endif