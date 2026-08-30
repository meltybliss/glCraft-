#pragma once
#include "TextureObject.h"
#include <glm/glm.hpp>

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
	void UpdateSubRegion(
		const glm::ivec3& offset,
		const glm::ivec3& size,
		const float* data,
		int sourceRowLength,
		int sourceImageHeight
	) const;


	void Bind(unsigned int unit = 0) const;
	void Unbind() const;
	

 
};
