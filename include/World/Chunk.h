#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "World/BlockData.h"
#include "Render/ChunkMesh.h"
#include <array>



struct Chunk {

	Chunk(int32_t x, int32_t z) : cx(x), cz(z)  {
		blocks.fill(BlockType::AIR);
	}
	
	int32_t cx = 0;
	int32_t cz = 0;

	ChunkMesh mesh;

	bool dirty = true;
	bool urgentUpdateMesh = false;

	constexpr static int CHUNK_WIDTH = 16;
	constexpr static int CHUNK_DEPTH = 16;
	constexpr static int CHUNK_HEIGHT = 256;

	constexpr static int CHUNK_SIZE = CHUNK_WIDTH * CHUNK_DEPTH * CHUNK_HEIGHT;


	std::array<BlockType, CHUNK_SIZE> blocks;
	std::array<uint8_t, CHUNK_SIZE> blockLights{};

	static int Index(int x, int y, int z) {
		return x + CHUNK_WIDTH * z + y * CHUNK_WIDTH * CHUNK_DEPTH;
	}

	[[nodiscard]] unsigned int GetBlock(int x, int y, int z) const {
		if (!InBounds(x, y, z)) {
			return 0;
		}

		return (unsigned int)blocks[Index(x, y, z)];
	}

	[[nodiscard]] uint8_t GetLight(int x, int y, int z) const {
		if (!InBounds(x, y, z)) {
			return 0;
		}

		return blockLights[Index(x, y, z)];
	}

	void SetBlock(int x, int y, int z, BlockType b) {
		if (!InBounds(x, y, z)) {
			return;
		}

		blocks[Index(x, y, z)] = b;

		dirty = true;
	}

	bool SetLight(int x, int y, int z, uint8_t level) {
		if (!InBounds(x, y, z)) {
			return false;
		}

		blockLights[Index(x, y, z)] = level;
		return true;
	}



	void SetBlockForGenerator(int x, int y, int z, BlockType b) {
		if (!InBounds(x, y, z)) {
			return;
		}
		blocks[Index(x, y, z)] = b;

	}

	static bool InBounds(int x, int y, int z) {
		return (x < CHUNK_WIDTH && x >= 0 && y < CHUNK_HEIGHT && y >= 0 &&
			z < CHUNK_DEPTH && z >= 0);
	}

};