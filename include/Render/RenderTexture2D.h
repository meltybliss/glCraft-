#pragma once
#include "TextureObject.h"

class RenderTexture2D : public TextureObject{
public:

	RenderTexture2D() = default;
	RenderTexture2D(
		int width,
		int height,
		GLenum internalFormat,
		GLenum format,
		GLenum type
	);

	bool Create(
		int width,
		int height,
		GLenum internalFormat,
		GLenum format,
		GLenum type
	);
	void Bind(unsigned int unit = 0) const;
	void Unbind() const;
private:

};