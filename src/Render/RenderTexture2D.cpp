#include "Render/RenderTexture2D.h"

RenderTexture2D::RenderTexture2D(
	int width,
	int height,
	GLenum internalFormat,
	GLenum format,
	GLenum type
) {

	Create(width, height, internalFormat, format, type);

}


bool RenderTexture2D::Create(
	int width,
	int height,
	GLenum internalFormat,
	GLenum format,
	GLenum type
) {

	glGenTextures(1, &m_id);

	glBindTexture(GL_TEXTURE_2D, m_id);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		internalFormat,
		(GLsizei)width,
		(GLsizei)height,
		0,
		format,
		type,
		nullptr


	);



	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MIN_FILTER,
		GL_LINEAR
	);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MAG_FILTER,
		GL_LINEAR
	);


	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_WRAP_S,
		GL_CLAMP_TO_EDGE
	);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_WRAP_T,
		GL_CLAMP_TO_EDGE
	);


	glBindTexture(GL_TEXTURE_2D, 0);


	return true;
}




void RenderTexture2D::Bind(unsigned int unit) const {

	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, m_id);

}



void RenderTexture2D::Unbind() const {

	glBindTexture(GL_TEXTURE_2D, 0);

}