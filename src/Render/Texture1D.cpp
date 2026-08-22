#include "Render/Texture1D.h"


Texture1D::Texture1D(int size) {

	Create(size);
}


bool Texture1D::Create(int size) {
	
	m_size = size;

	glGenTextures(1, &m_id);
	glBindTexture(GL_TEXTURE_1D, m_id);


	glTexParameteri(
		GL_TEXTURE_1D, 
		GL_TEXTURE_MIN_FILTER, 
		GL_LINEAR
	);

	glTexParameteri(
		GL_TEXTURE_1D, 
		GL_TEXTURE_MAG_FILTER, 
		GL_LINEAR
	);


	glTexParameteri(
		GL_TEXTURE_1D,
		GL_TEXTURE_WRAP_S,
		GL_REPEAT
	);


	glTexImage1D(

		GL_TEXTURE_1D,
		0,
		GL_R32F,
		size,
		0,
		GL_RED,
		GL_FLOAT,
		nullptr

	);


	glBindTexture(GL_TEXTURE_1D, 0);

	return true;
}


void Texture1D::Bind(unsigned int unit) const {

	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_1D, m_id);


}


void Texture1D::Unbind() const {

	glBindTexture(GL_TEXTURE_1D, 0);

}



void Texture1D::UpdateSub(const float* data) const {

	glTexSubImage1D(
		GL_TEXTURE_1D,
		0,
		0,
		m_size,
		GL_RED,
		GL_FLOAT,
		data
	);


}