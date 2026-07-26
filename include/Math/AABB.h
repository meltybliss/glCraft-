#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct AABB {
	glm::vec3 min;
	glm::vec3 max;
};


struct WorldAABB {

	glm::i64vec3 originBlock;
	glm::dvec3 min;
	glm::dvec3 max;

};