#include "Render/Texture.h"

#include <iostream>
#include "glad/glad.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

Texture::Texture(const std::string& path) {
	LoadFromFile(path);
}

bool Texture::LoadFromFile(const std::string& path) {
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

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
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


void Texture::Bind(unsigned int unit) const {
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, m_id);
}


void Texture::Destroy() {
	if (m_id != 0) {
		glDeleteTextures(1, &m_id);
		m_id = 0;
	}
}