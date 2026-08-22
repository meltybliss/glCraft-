#include "Render/ImageTexture2D.h"

#include <iostream>
#include "glad/glad.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

ImageTexture2D::ImageTexture2D(const std::string& path) {
	LoadFromFile(path);
}

bool ImageTexture2D::LoadFromFile(const std::string& path) {
	stbi_set_flip_vertically_on_load(true);

	unsigned char* data = stbi_load(
		path.c_str(),
		&m_width,
		&m_height,
		&m_channels,
		4

	);

	if (!data) {
		std::cerr << "Failed to load texture: " << path << "\n";
		return false;
	}

	glGenTextures(1, &m_id);
	glBindTexture(GL_TEXTURE_2D, m_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_SRGB8_ALPHA8,
		m_width,
		m_height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		data

	);


	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}


void ImageTexture2D::Bind(unsigned int unit) const {
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, m_id);
}


void ImageTexture2D::Unbind() const {

	glBindTexture(GL_TEXTURE_2D, 0);
}