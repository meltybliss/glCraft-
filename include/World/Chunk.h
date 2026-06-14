#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "World/BlockData.h"
#include <array>

struct Chunk {
	unsigned int vao = -1;
	unsigned int vbo = -1;

	bool dirty = true;

	constexpr static int CHUNK_WIDTH = 16;
	constexpr static int CHUNK_DEPTH = 16;
	constexpr static int CHUNK_HEIGHT = 256;

	constexpr static int CHUNK_SIZE = CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT;


	std::array<BlockType, CHUNK_SIZE> blocks;

	int Index(int x, int y, int z) {
		return x + CHUNK_WIDTH * z + y * CHUNK_WIDTH * CHUNK_DEPTH;
	}

	unsigned int GetBlock(int x, int y, int z) {
		return (unsigned int)blocks[Index(x, y, z)];
	}

	void SetBlock(int x, int y, int z, BlockType b) {
		blocks[Index(x, y, z)] = b;

		dirty = true;
	}

};