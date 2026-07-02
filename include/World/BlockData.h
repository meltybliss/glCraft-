#pragma once

enum class BlockType {
	AIR = 0,
	GRASS,
	DIRT,
	STONE,
	TORCH,
	GLOWSTONE,

};

constexpr float BLOCK_SIZE = 1.f;


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