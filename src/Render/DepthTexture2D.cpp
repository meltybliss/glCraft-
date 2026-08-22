#include "Render/DepthTexture2D.h"

DepthTexture2D::DepthTexture2D(int w, int h) {

	Create(w, h);

}

bool DepthTexture2D::Create(int w, int h) {

	glGenTextures(1, &m_id);
	glBindTexture(GL_TEXTURE_2D, m_id);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_DEPTH_COMPONENT,
		w,
		h,
		0,
		GL_DEPTH_COMPONENT,
		GL_FLOAT,
		nullptr

	);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_COMPARE_MODE,
		GL_COMPARE_REF_TO_TEXTURE
	);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_COMPARE_FUNC,
		GL_LEQUAL
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
		GL_CLAMP_TO_BORDER
	);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_WRAP_T,
		GL_CLAMP_TO_BORDER
	);

	const float borderColor[] = {
		1.0f, 1.0f, 1.0f, 1.0f
	};

	glTexParameterfv(
		GL_TEXTURE_2D,
		GL_TEXTURE_BORDER_COLOR,
		borderColor
	);

	glBindTexture(GL_TEXTURE_2D, 0);

}



void DepthTexture2D::Bind(unsigned int unit) const {

	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, m_id);

}


void DepthTexture2D::Unbind() const {

	glBindTexture(GL_TEXTURE_2D, 0);

}