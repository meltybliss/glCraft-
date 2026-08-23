#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace SkyRenderingConfig {

	inline const glm::vec3 dayHorizonColor = glm::vec3(0.35f, 0.65f, 0.95f);
	inline const glm::vec3 nightHorizonColor = glm::vec3(0.018f, 0.035f, 0.085f);

	inline const glm::vec3 datTopColor = glm::vec3(0.20f, 0.50f, 0.95f);
	inline const glm::vec3 nightTopColor = glm::vec3(0.012f, 0.022f, 0.065f);
}