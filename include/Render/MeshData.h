#pragma once
#include <vector>
#include <stdint.h>

struct MeshData {
	std::vector<float> vertices;
	std::vector<uint8_t> indices;
};