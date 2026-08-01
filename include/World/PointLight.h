#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>
#include <unordered_map>
#include <stdint.h>


//ある場所に周りに減衰しながら広がる光オブジェクトを置くためにつかう。主に光源ブロックに置く。
struct PointLight {

	glm::i64vec3 position;
	glm::vec3 color;
	float radius;

	float intensity;


}; 


struct PointLightsStruct {
	std::vector<PointLight> pointLights;
	size_t count;

};

struct PointLightsSnapshot {

	std::unordered_map<uint64_t, PointLightsStruct> pointLightsMap;

};