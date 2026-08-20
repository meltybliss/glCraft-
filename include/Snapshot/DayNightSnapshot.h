#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
struct DayNightSnapshot {

	glm::vec3 directionToSun{ 0.0f, 1.0f, 0.0f };

	float dayFactor = 1.0f;
	float sunHeight = 1.0f;

	float sunIntensity = 1.0f;
	float skyStrength = 1.0f;

};