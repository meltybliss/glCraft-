#pragma once
#include "TextureObject.h"

class DepthTexture2D : public TextureObject{
public:

	DepthTexture2D() = default;
	DepthTexture2D(int w, int h);

	bool Create(int w, int h);

	void Bind(unsigned int unit) const;
	void Unbind() const;
private:


};