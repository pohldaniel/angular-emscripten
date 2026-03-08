#include "MeshSphere.h"

MeshSphere::MeshSphere(int uResolution, int vResolution) : MeshSphere(glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, true, true, false, uResolution, vResolution) { }

MeshSphere::MeshSphere(bool generateTexels, bool generateNormals, bool generateTangents, int uResolution, int vResolution) : MeshSphere(glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, generateTexels, generateNormals, generateTangents, uResolution, vResolution) { }

MeshSphere::MeshSphere(const glm::vec3& position, float radius, bool generateTexels, bool generateNormals, bool generateTangents, int uResolution, int vResolution) {
	
	m_radius = radius;
	m_position = position;
	m_generateNormals = generateNormals;
	m_generateTexels = generateTexels;
	m_generateTangents = generateTangents;

	m_uResolution = uResolution;
	m_vResolution = vResolution;

	m_numBuffers = 1 + generateTexels + generateNormals + generateTangents * 2;

	BuildMesh(m_radius, m_position, m_uResolution, m_vResolution, m_generateTexels, m_generateNormals, m_generateTangents, m_positions, m_texels, m_normals, m_indexBuffer, m_tangents, m_bitangents);
	createBuffer();
}

MeshSphere::~MeshSphere() {
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

	if (m_vboInstances)
		glDeleteBuffers(1, &m_vboInstances);
}

void MeshSphere::setPrecision(int uResolution, int vResolution) {
	m_uResolution = uResolution;
	m_vResolution = vResolution;
}

void MeshSphere::BuildMesh(float radius, const glm::vec3& position, int uResolution, int vResolution, bool generateTexels, bool generateNormals, bool generateTangents, std::vector<glm::vec3>& positions, std::vector<glm::vec2>& texels, std::vector<glm::vec3>& normals, std::vector<unsigned int>& indexBuffer, std::vector<glm::vec3>& tangents, std::vector<glm::vec3>& bitangents) {
    
    float uAngleStep = (2.0f * glm::pi<float>()) / float(uResolution);
	float vAngleStep = glm::pi<float>() / float(vResolution);
	float vSegmentAngle, uSegmentAngle;

	for (int i = 0; i <= vResolution; ++i) {
		vSegmentAngle = glm::pi<float>() * 0.5f - i * vAngleStep;
		
		float cosVSegment = cosf(vSegmentAngle);
		float sinVSegment = sinf(vSegmentAngle);
													
		for (int j = 0; j <= uResolution; ++j) {
			uSegmentAngle = j * uAngleStep;
			
			float cosUSegment = cosf(uSegmentAngle);
			float sinUSegment = sinf(uSegmentAngle);

			float x = cosVSegment * cosUSegment;
			float z = cosVSegment * sinUSegment;
			positions.push_back(glm::vec3(radius * x, radius * sinVSegment, radius * z) + position);

			if (generateTexels)
				texels.push_back(glm::vec2(1.0f - (float)j / uResolution, 1.0f - (float)i / vResolution));

			if (generateNormals)
				normals.push_back(glm::vec3(x, sinVSegment, z));

			if (generateTangents) {
				tangents.push_back(glm::vec3(sinUSegment , 0.0, cosUSegment)); // rotate 180 z Axis to match texture space
				bitangents.push_back(glm::vec3(-sinVSegment*cosUSegment, cosVSegment, -sinVSegment*sinUSegment));
			}
		}
	}

	unsigned int k1, k2;
	for (int i = 0; i < vResolution; ++i){
		k1 = i * (uResolution + 1);
		k2 = k1 + uResolution + 1;

		for (int j = 0; j < uResolution; ++j, ++k1, ++k2) {

			if (i != 0) {
				indexBuffer.push_back(k1); indexBuffer.push_back(k1 + 1); indexBuffer.push_back(k2);
			}

			if (i != (vResolution - 1)) {
				indexBuffer.push_back(k1 + 1);  indexBuffer.push_back(k2 + 1); indexBuffer.push_back(k2);
			}	
		}
	}
}

int MeshSphere::getNumberOfTriangles() {
	return m_drawCount / 3;
}

std::vector<glm::vec3>& MeshSphere::getPositions() {
	return m_positions;
}

std::vector<unsigned int>& MeshSphere::getIndexBuffer() {
	return m_indexBuffer;
}

void MeshSphere::createBuffer() {
	m_drawCount = m_indexBuffer.size();

	unsigned int ibo;
	glGenBuffers(1, &ibo);
	glGenBuffers(m_numBuffers, m_vbo);

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	//Position
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, m_positions.size() * sizeof(m_positions[0]), &m_positions[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//Texture Coordinates
	if (m_generateTexels) {
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, m_texels.size() * sizeof(m_texels[0]), &m_texels[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}

	//Normals
	if (m_generateNormals) {
		glBindBuffer(GL_ARRAY_BUFFER, m_generateTexels ? m_vbo[2] : m_vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(m_normals[0]), &m_normals[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	//tangents
	if (m_generateTangents) {
		glBindBuffer(GL_ARRAY_BUFFER, (m_generateTexels && m_generateNormals) ? m_vbo[3] : (m_generateTexels || m_generateNormals) ? m_vbo[2] : m_vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, m_tangents.size() * sizeof(m_tangents[0]), &m_tangents[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, (m_generateTexels && m_generateNormals) ? m_vbo[4] : (m_generateTexels || m_generateNormals) ? m_vbo[3] : m_vbo[2]);
		glBufferData(GL_ARRAY_BUFFER, m_bitangents.size() * sizeof(m_bitangents[0]), &m_bitangents[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);
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

void MeshSphere::drawRaw() {
	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, m_drawCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}