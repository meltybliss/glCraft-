#pragma once
#include "TextureObject.h"

class DataTexture3D : public TextureObject {
public:

	DataTexture3D() = default;
	DataTexture3D(
		GLenum internalFormat,
		GLenum format,
		GLenum type
	);

	bool Create(
		GLenum internalFormat,
		GLenum format,
		GLenum type
	);

	void UpdateSub(const float* data) const;


	void Bind(unsigned int unit = 0) const;
	void Unbind() const;
	

 
};