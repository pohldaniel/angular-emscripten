#include <SOIL.h>
#include <GL/glew.h>
#include "Texture.h"

std::map<unsigned int, unsigned int> Texture::ActiveTextures;

Texture::Texture(std::string fileName, const bool flipVertical, unsigned int internalFormat, unsigned int format, int paddingLeft, int paddingRight, int paddingTop, int paddingBottom , unsigned int SOIL_FLAG) {
  loadFromFile(fileName, flipVertical, internalFormat, format, paddingLeft, paddingRight, paddingTop, paddingBottom, SOIL_FLAG);
}

Texture::~Texture() {
  if (m_markForDelete) {
    cleanup();
  }
}

void Texture::cleanup() {
  if (m_texture) {	
    glDeleteTextures(1, &m_texture);
    m_texture = 0;
  }
}

void Texture::markForDelete() {
  m_markForDelete = true;
}

void Texture::flipVertical(unsigned char* data, unsigned int padWidth, unsigned int height) {
	std::vector<unsigned char> srcPixels(padWidth * height);
	memcpy(&srcPixels[0], data, padWidth * height);

	unsigned char *pSrcRow = 0;
	unsigned char *pDestRow = 0;

	for (unsigned int i = 0; i < height; ++i) {

		pSrcRow = &srcPixels[(height - 1 - i) * padWidth];
		pDestRow = &data[i * padWidth];
		memcpy(pDestRow, pSrcRow, padWidth);
	}
}

void Texture::loadFromFile(std::string fileName, const bool flipVertical, unsigned int internalFormat, unsigned int format, int paddingLeft, int paddingRight, int paddingTop, int paddingBottom, unsigned int SOIL_FLAG) {
  loadFromFileCpu(fileName, flipVertical, internalFormat, format, paddingLeft, paddingRight, paddingTop, paddingBottom, SOIL_FLAG);
  loadFromFileGpu();
}

void Texture::loadFromFileCpu(std::string fileName, const bool _flipVertical, unsigned int _internalFormat, unsigned int _format, int paddingLeft, int paddingRight, int paddingTop, int paddingBottom, unsigned int SOIL_FLAG) {
	int width, height, numComponents;
	imageData = SOIL_load_image(fileName.c_str(), &width, &height, &numComponents, SOIL_FLAG);

	if (numComponents == 1 && (_format == GL_RGB || _format == GL_RGBA )) {
		SOIL_free_image_data(imageData);
		SOIL_FLAG = _format == GL_RGB ? 3u : 4u;
		imageData = SOIL_load_image(fileName.c_str(), &width, &height, 0, SOIL_FLAG);
		numComponents = _format == GL_RGB ? 3 : 4;
	}
	
	m_internalFormat = _internalFormat == 0 && numComponents == 1 ? GL_R8 : _internalFormat == 0 && numComponents == 3 ? GL_RGB8 : _internalFormat == 0 ? GL_RGBA8 : _internalFormat;
	m_format = _format == 0 && numComponents == 1 ? GL_R : _format == 0 && numComponents == 3 ? GL_RGB : _format == 0 ? GL_RGBA : _format;
	m_type = GL_UNSIGNED_BYTE;
	m_target = GL_TEXTURE_2D;

	if (_flipVertical)
		flipVertical(imageData, numComponents * width, height);

	//imageData = AddRemoveLeftPadding(imageData, width, height, numComponents, paddingLeft);
	//imageData = AddRemoveRightPadding(imageData, width, height, numComponents, paddingRight);
	//imageData = AddRemoveTopPadding(imageData, width, height, numComponents, paddingTop);
	//imageData = AddRemoveBottomPadding(imageData, width, height, numComponents, paddingBottom);

	m_width = width;
	m_height = height;
	m_channels = numComponents;
}

void Texture::loadFromFileGpu() {

	/*if (m_target == GL_TEXTURE_CUBE_MAP) {
		glGenTextures(1, &m_texture);
		glBindTexture(m_target, m_texture);

		glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(m_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(m_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(m_target, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);

		// load face data
		for (int i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, m_internalFormat, m_width, m_height, 0, m_format, m_type, facData[i]);
		}

		glBindTexture(m_target, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		for (int i = 0; i < 6; i++) {
			free(facData[i]);
			facData[i] = nullptr;
		}

	}else {*/

		glGenTextures(1, &m_texture);
		glBindTexture(m_target, m_texture);
		glTexParameterf(m_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(m_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(m_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(m_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		if (m_channels == 3 || m_channels == 1)
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTexImage2D(m_target, 0, m_internalFormat, m_width, m_height, 0, m_format, m_type, imageData);
		glBindTexture(m_target, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);		
	//}

    SOIL_free_image_data(imageData);
    imageData = nullptr;
}

void Texture::loadCrossHDRIFromFile(std::string fileName, const bool _flipVertical, unsigned int internalFormat, unsigned int format, int paddingLeft, int paddingRight, int paddingTop, int paddingBottom) {
	int width, height, numComponents;
	unsigned char* imageData = reinterpret_cast<unsigned char *>(SOIL_load_image_f(fileName.c_str(), &width, &height, &numComponents, 0));
	
	m_internalFormat = GL_RGBA16F;
	m_format = GL_RGBA;
	m_type = GL_FLOAT;
	m_target = GL_TEXTURE_CUBE_MAP;

	if (_flipVertical)
		flipVertical(imageData, numComponents * sizeof(float) * width, height);

	m_width = width;
	m_height = height;
	m_channels = numComponents;

	//pointers to the levels
	std::vector<unsigned char*> facData;

	//get the source data
	unsigned char *data = imageData;

	int fWidth = m_width / 3;
	int fHeight = m_height / 4;
	int elementSize = 3 * sizeof(float);

	unsigned char *face = new unsigned char[fWidth * fHeight * elementSize];
	unsigned char *ptr;

	//extract the faces

	// positive X
	ptr = face;
	for (int j = 0; j<fHeight; j++) {
		memcpy(ptr, &data[((m_height - (fHeight + j + 1))*m_width + 2 * fWidth) * elementSize], fWidth * elementSize);
		ptr += fWidth * elementSize;
	}
	facData.push_back(face);

	// negative X
	face = new unsigned char[fWidth * fHeight * elementSize];
	ptr = face;
	for (int j = 0; j<fHeight; j++) {
		memcpy(ptr, &data[(m_height - (fHeight + j + 1))*m_width * elementSize], fWidth * elementSize);
		ptr += fWidth * elementSize;
	}
	facData.push_back(face);

	// positive Y
	face = new unsigned char[fWidth * fHeight * elementSize];
	ptr = face;
	for (int j = 0; j<fHeight; j++) {
		memcpy(ptr, &data[((4 * fHeight - j - 1)*m_width + fWidth) * elementSize], fWidth * elementSize);
		ptr += fWidth * elementSize;
	}
	facData.push_back(face);

	// negative Y
	face = new unsigned char[fWidth * fHeight * elementSize];
	ptr = face;
	for (int j = 0; j<fHeight; j++) {
		memcpy(ptr, &data[((2 * fHeight - j - 1)*m_width + fWidth) * elementSize], fWidth * elementSize);
		ptr += fWidth * elementSize;
	}
	facData.push_back(face);

	// positive Z
	face = new unsigned char[fWidth * fHeight * elementSize];
	ptr = face;
	for (int j = 0; j<fHeight; j++) {
		memcpy(ptr, &data[((m_height - (fHeight + j + 1))*m_width + fWidth) * elementSize], fWidth * elementSize);
		ptr += fWidth * elementSize;
	}
	facData.push_back(face);

	// negative Z
	face = new unsigned char[fWidth * fHeight * elementSize];
	ptr = face;
	for (int j = 0; j<fHeight; j++) {
		for (int i = 0; i<fWidth; i++) {
			memcpy(ptr, &data[(j*m_width + 2 * fWidth - (i + 1)) * elementSize], elementSize);
			ptr += elementSize;
		}
	}
	facData.push_back(face);

	//set the new # of faces, width and height
	m_width = fWidth;
	m_height = fHeight;

	glGenTextures(1, &m_texture);
	glBindTexture(m_target, m_texture);

	glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(m_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(m_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// load face data
	for (int i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, m_internalFormat, m_width, m_height, 0, m_format, m_type, facData[i]);
	}
	glGenerateMipmap(m_target);
	glBindTexture(m_target, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	for (int i = 0; i < 6; i++) {
		free(facData[i]);
	}

	SOIL_free_image_data(imageData);
}

void Texture::bind(unsigned int unit, bool forceBind) const {
	if (ActiveTextures[unit] != m_texture || forceBind) {
		ActiveTextures[unit] = m_texture;
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(m_target, m_texture);
	}
}

void Texture::unbind(unsigned int unit) const {	
	ActiveTextures[unit] = 0u;
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(m_target, 0);	
}