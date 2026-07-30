#pragma once
#include <stdint.h>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>



struct LightNode {
	int64_t x = 0;
	int64_t y = 0;
	int64_t z = 0;
	uint8_t lightLevel = 0;

	glm::vec3 color;
};
