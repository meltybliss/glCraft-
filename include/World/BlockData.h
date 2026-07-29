#pragma once
#include <array>


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
	3.0f,
	3.0f


};



inline uint8_t GetEmission(BlockType b) {
	switch (b) {
		case BlockType::TORCH: return 14;
		case BlockType::GLOWSTONE: return 15;
	
		default: return 0;

	}

}


inline bool isOpaque(BlockType b) {
	return b != BlockType::AIR;
}

inline bool isLightSourceBlock(BlockType b) {
	return b == BlockType::TORCH || b == BlockType::GLOWSTONE;
}