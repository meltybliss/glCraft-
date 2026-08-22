#include "Render/DataTexture3D.h"
#include "Snapshot/LightVolumeSnapshot.h"


DataTexture3D::DataTexture3D(
	GLenum internalFormat,
	GLenum format,
	GLenum type
) {


}


bool DataTexture3D::Create(
	GLenum internalFormat,
	GLenum format,
	GLenum type
) {

	constexpr int channel_count = 4;

	using namespace LIGHT_VOLUME_SIZE;

	std::vector<float> emptyData(
		LIGHT_VOLUME_WIDTH *
		LIGHT_VOLUME_HEIGHT *
		LIGHT_VOLUME_DEPTH *
		channel_count,
		0.0f
	);


	glTexImage3D(
		GL_TEXTURE_3D,
		0,
		internalFormat,
		LIGHT_VOLUME_WIDTH,
		LIGHT_VOLUME_HEIGHT,
		LIGHT_VOLUME_DEPTH,
		0,
		format,
		type,
		emptyData.data()
	);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);



	glBindTexture(GL_TEXTURE_3D, 0);


	return true;
}


void DataTexture3D::Bind(unsigned int unit) const {

	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_3D, m_id);


}


void DataTexture3D::Unbind() const {


	glBindTexture(GL_TEXTURE_3D, 0);
}


void DataTexture3D::UpdateSub(const float* data) const {

	using namespace LIGHT_VOLUME_SIZE;


	glTexSubImage3D(
		GL_TEXTURE_3D,
		0,
		0, 0, 0,
		LIGHT_VOLUME_WIDTH,
		LIGHT_VOLUME_HEIGHT,
		LIGHT_VOLUME_DEPTH,
		GL_RGBA,
		GL_FLOAT,
		data

	);

}