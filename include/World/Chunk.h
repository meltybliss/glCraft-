#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "World/BlockData.h"
#include "Render/ChunkMesh.h"
#include <array>

struct Chunk {
	
	int32_t cx = 0;
	int32_t cz = 0;

	ChunkMesh mesh;

	bool dirty = true;

	constexpr static int CHUNK_WIDTH = 16;
	constexpr static int CHUNK_DEPTH = 16;
	constexpr static int CHUNK_HEIGHT = 256;

	constexpr static int CHUNK_SIZE = CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT;


	std::array<BlockType, CHUNK_SIZE> blocks;

	static int Index(int x, int y, int z) {
		return x + CHUNK_WIDTH * z + y * CHUNK_WIDTH * CHUNK_DEPTH;
	}

	[[nodiscard]] unsigned int GetBlock(int x, int y, int z) const {
		return (unsigned int)blocks[Index(x, y, z)];
	}

	void SetBlock(int x, int y, int z, BlockType b) {
		blocks[Index(x, y, z)] = b;

		dirty = true;
	}

	void SetBlockForGenerator(int x, int y, int z, BlockType b) {
		blocks[Index(x, y, z)] = b;

	}

	static bool InBounds(int x, int y, int z) {
		return (x < CHUNK_WIDTH && x >= 0 && y < CHUNK_HEIGHT && y >= 0 &&
			z < CHUNK_DEPTH && z >= 0);
	}

};