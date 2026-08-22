#pragma once

#include <string>
#include "TextureObject.h"

class Texture1D : public TextureObject{
public:

	Texture1D() = default;
	explicit Texture1D(int size);

	bool Create(int size);
	void Bind(unsigned int unit = 0) const;
	void Unbind() const;

	void UpdateSub(const float* data) const;

private:

	int m_size = 0;
};