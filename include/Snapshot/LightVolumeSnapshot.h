#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

namespace LIGHT_VOLUME_SIZE {
	constexpr int LIGHT_VOLUME_WIDTH = 32;
	constexpr int LIGHT_VOLUME_HEIGHT = 32;
	constexpr int LIGHT_VOLUME_DEPTH = 32;
}
struct LightVolumeSnapshot {

	std::vector<float> pixels;
	glm::i64vec3 origin;

};


