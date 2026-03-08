#include "MeshTorus.h"

MeshTorus::MeshTorus(int uResolution, int vResolution) : MeshTorus(glm::vec3(0.0f, 0.0f, 0.0f), 0.5f, 0.25f, true, true, false, uResolution, vResolution) {}

MeshTorus::MeshTorus(bool generateTexels, bool generateNormals, bool generateTangents, int uResolution, int vResolution) : MeshTorus(glm::vec3(0.0f, 0.0f, 0.0f), 0.5f, 0.25f, generateTexels, generateNormals, generateTangents, uResolution, vResolution) {}

MeshTorus::MeshTorus(const glm::vec3& position, float radius, float tubeRadius, bool generateTexels, bool generateNormals, bool generateTangents, int uResolution, int vResolution) {
	
	m_radius = radius;
	m_tubeRadius = tubeRadius;

	m_position = position;
	m_generateNormals = generateNormals;
	m_generateTexels = generateTexels;
	m_generateTangents = generateTangents;

	m_uResolution = uResolution;
	m_vResolution = vResolution;

	m_numBuffers = 1 + generateTexels + generateNormals + 2 * generateTangents;
	BuildMesh(m_radius, m_tubeRadius, m_position, m_uResolution, m_vResolution, m_generateTexels, m_generateNormals, m_generateTangents, m_positions, m_texels, m_normals, m_indexBuffer, m_tangents, m_bitangents);
	createBuffer();
}

MeshTorus::~MeshTorus() {
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

void MeshTorus::setPrecision(int uResolution, int vResolution) {
	m_uResolution = uResolution;
	m_vResolution = vResolution;
}

void MeshTorus::BuildMesh(float radius, float tubeRadius, const glm::vec3& position, int uResolution, int vResolution, bool generateTexels, bool generateNormals, bool generateTangents, std::vector<glm::vec3>& positions, std::vector<glm::vec2>& texels, std::vector<glm::vec3>& normals, std::vector<unsigned int>& indexBuffer, std::vector<glm::vec3>& tangents, std::vector<glm::vec3>& bitangents) {
float mainSegmentAngleStep = 1.0f / float(uResolution);
	float tubeSegmentAngleStep = 1.0f / float(vResolution);

	float currentMainSegmentAngle = 0.0f;
	for (unsigned int i = 0; i <= uResolution; i++) {

		// Calculate sine and cosine of main segment angle
		float sinMainSegment = sinf((2.0f * glm::pi<float>()) * currentMainSegmentAngle);
		float cosMainSegment = cosf((2.0f * glm::pi<float>()) * currentMainSegmentAngle);
		float currentTubeSegmentAngle = 0.0f;

		for (unsigned int j = 0; j <= vResolution; j++) {

			// Calculate sine and cosine of tube segment angle
			float sinTubeSegment = sinf((2.0f * glm::pi<float>()) * currentTubeSegmentAngle);
			float cosTubeSegment = cosf((2.0f * glm::pi<float>()) * currentTubeSegmentAngle);

			// Calculate vertex position on the surface of torus
			float x = (radius + tubeRadius * cosTubeSegment) * cosMainSegment;
			float y = tubeRadius * sinTubeSegment;
			float z = (radius + tubeRadius * cosTubeSegment) * sinMainSegment;
			
			glm::vec3 surfacePosition = glm::vec3(x, y, z) + position;
			positions.push_back(surfacePosition);

			if(generateTexels) {
				texels.push_back(glm::vec2(1.0f - currentMainSegmentAngle, currentTubeSegmentAngle));
			}

			if(generateNormals) {
				normals.push_back(glm::vec3(cosMainSegment * cosTubeSegment, sinTubeSegment, sinMainSegment * cosTubeSegment));
			}

			if(generateTangents) {
				tangents.push_back(glm::vec3(sinMainSegment, 0.0f, cosMainSegment));
				bitangents.push_back(glm::vec3(-sinTubeSegment *cosMainSegment, cosTubeSegment, -sinTubeSegment *sinMainSegment));				
			}

			// Update current tube angle
			currentTubeSegmentAngle += tubeSegmentAngleStep;
		}

		// Update main segment angle
		currentMainSegmentAngle += mainSegmentAngleStep;
	}

	//calculate the indices
	unsigned int currentVertexOffset = 0;
	for (unsigned int i = 0; i < uResolution; i++) {

		for (unsigned int j = 0; j <= vResolution; j++) {

			unsigned int vertexIndexA, vertexIndexB, vertexIndexC, vertexIndexD, vertexIndexE, vertexIndexF;

			if ((j > 0) && ((j + 1) % (vResolution + 1)) == 0) {
				currentVertexOffset = ((i + 1) * (vResolution + 1));
			} else {

				vertexIndexA = currentVertexOffset;
				vertexIndexB = currentVertexOffset + vResolution + 1;
				vertexIndexC = currentVertexOffset + 1;

				vertexIndexD = currentVertexOffset + vResolution + 1;
				vertexIndexF = currentVertexOffset + vResolution + 2;
				vertexIndexE = currentVertexOffset + 1;

				indexBuffer.push_back(vertexIndexA); indexBuffer.push_back(vertexIndexC); indexBuffer.push_back(vertexIndexB);
				indexBuffer.push_back(vertexIndexD); indexBuffer.push_back(vertexIndexE); indexBuffer.push_back(vertexIndexF);
				currentVertexOffset++;
			}
		}
	}
}

std::vector<glm::vec3>& MeshTorus::getPositions() {
	return m_positions;
}

std::vector<unsigned int>& MeshTorus::getIndexBuffer() {
	return m_indexBuffer;
}

int MeshTorus::getNumberOfTriangles() {
	return m_drawCount / 3;
}

void MeshTorus::createBuffer() {
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

void MeshTorus::drawRaw() {
	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, m_drawCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}