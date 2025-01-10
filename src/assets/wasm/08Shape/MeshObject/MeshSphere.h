#ifndef _MESHSPHERE_H
#define _MESHSPHERE_H

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

class MeshSphere {

public:

	MeshSphere(int uResolution = 49, int vResolution = 49);
	MeshSphere(bool generateTexels, bool generateNormals, bool generateTangents, int uResolution = 49, int vResolution = 49);
	MeshSphere(const glm::vec3& position, float radius, bool generateTexels, bool generateNormals, bool generateTangents, int uResolution = 49, int vResolution = 49);
	~MeshSphere();

	void drawRaw();
	void drawRawInstanced();
	void createInstancesStatic(const std::vector<glm::mat4>& modelMTX);
	void addInstance(const glm::mat4& modelMTX);
	void setPrecision(int uResolution, int vResolution);
	int getNumberOfTriangles();
	std::vector<glm::vec3>& getPositions();
	std::vector<unsigned int>& getIndexBuffer();

	static void BuildMesh(float radius, const glm::vec3& position, int uResolution, int vResolution, bool generateTexels, bool generateNormals, bool generateTangents, std::vector<glm::vec3>& positions, std::vector<glm::vec2>& texels, std::vector<glm::vec3>& normals, std::vector<unsigned int>& indexBuffer, std::vector<glm::vec3>& tangents, std::vector<glm::vec3>& bitangents);

private:

	int m_uResolution;
	int m_vResolution;
	float m_radius;

	glm::vec3 m_position;

	bool m_generateNormals;
	bool m_generateTexels;
	bool m_generateTangents;

	short m_numBuffers;
	unsigned int m_vao;
	unsigned int m_vbo[5];
	unsigned int m_drawCount;
	unsigned int m_vboInstances = 0;
	unsigned int m_instanceCount = 0;

	std::vector<unsigned int> m_indexBuffer;
	std::vector<glm::vec3> m_positions;
	std::vector<glm::vec2> m_texels;
	std::vector<glm::vec3> m_normals;
	std::vector<glm::vec3> m_tangents;
	std::vector<glm::vec3> m_bitangents;
	std::vector<glm::mat4> m_instances;

	glm::vec3 m_min;
	glm::vec3 m_max;

	void createBuffer();
};
#endif