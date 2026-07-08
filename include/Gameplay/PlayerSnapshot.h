#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


struct PlayerSnapshot {

	glm::vec3 pos{0.f};
	glm::vec3 front{0.f};
	glm::vec3 right{0.f};
	glm::vec3 up{0.f};

};