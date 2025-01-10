#include <string>
#include <vector>
#include <map>

class Texture{

public:
  Texture() = default;
  Texture(std::string fileName, const bool flipVertical = false, unsigned int internalFormat = 0u, unsigned int format = 0u, int paddingLeft = 0, int paddingRight = 0, int paddingTop = 0, int paddingBottom = 0, unsigned int SOIL_FLAG = 0u);
	
  ~Texture();

  void loadFromFile(std::string fileName, const bool flipVertical = false, unsigned int internalFormat = 0u, unsigned int format = 0u, int paddingLeft = 0, int paddingRight = 0, int paddingTop = 0, int paddingBottom = 0, unsigned int SOIL_FLAG = 0u);
  void loadFromFileCpu(std::string fileName, const bool flipVertical = false, unsigned int internalFormat = 0u, unsigned int format = 0u, int paddingLeft = 0, int paddingRight = 0, int paddingTop = 0, int paddingBottom = 0, unsigned int SOIL_FLAG = 0u);
  void loadFromFileGpu();void markForDelete();
  void flipVertical(unsigned char* data, unsigned int padWidth, unsigned int height);
  void cleanup();

  void bind(unsigned int unit = 0u, bool forceBind = false) const;
  void unbind(unsigned int unit = 0u) const;

private:

  unsigned int m_texture = 0u;
  bool m_markForDelete = false;
  unsigned char* imageData = nullptr;
  unsigned int m_format = 0u;
  unsigned int m_internalFormat = 0u;
  unsigned int m_type = 0u;
  unsigned int m_target;
  unsigned int m_width = 0u;
  unsigned int m_height = 0u;
  unsigned short m_channels = 0u;

  static std::map<unsigned int, unsigned int> ActiveTextures;
};