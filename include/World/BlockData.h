#pragma once
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class BlockType {
	AIR = 0,
	GRASS,
	DIRT,
	STONE,
	TORCH,
	GLOWSTONE,
	COUNT
};

constexpr float BLOCK_SIZE = 1.f;//Must be a whole number
constexpr std::array<float, static_cast<size_t>(BlockType::COUNT)> emissionDataForBloom{//it corresponds to the block types
	0.0f,
	0.0f,
	0.0f,
	0.0f,
	4.0f,
	4.0f


};


constexpr std::array<glm::vec3, static_cast<size_t>(BlockType::COUNT)> lightColor{//used for blocklight color and point light color

	glm::vec3(0.0f),
	glm::vec3(0.0f),
	glm::vec3(0.0f),
	glm::vec3(0.0f),
	glm::vec3(1.0f, 0.42f, 0.12f),
	glm::vec3(1.0f, 0.42f, 0.12f),


};


constexpr std::array<float, static_cast<size_t>(BlockType::COUNT)> pointLightRadius{
	0.0f,
	0.0f,
	0.0f,
	0.0f,
	3.0f,
	4.0f


};



inline uint8_t GetEmission(BlockType b) {
	switch (b) {
		case BlockType::TORCH: return 14;
		case BlockType::GLOWSTONE: return 15;
	
		default: return 0;

	}

}


inline bool isOpaque(BlockType b) {
	switch (b) {
	case BlockType::AIR: 
	case BlockType::TORCH: return false;

	default: return true;


	}
}


inline bool isLightSourceBlock(BlockType b) {
	return b == BlockType::TORCH || b == BlockType::GLOWSTONE;
}