#pragma once

#include <string>
#include "TextureObject.h"

class ImageTexture2D : public TextureObject {
public:
	ImageTexture2D() = default;
	explicit ImageTexture2D(const std::string& path);

	bool LoadFromFile(const std::string& path);
	void Bind(unsigned int unit = 0) const;
	void Unbind() const;

private:

	int m_width = 0;
	int m_height = 0;
	int m_channels = 0;

};