#pragma once
#include "World/Chunk.h"
#include <array>
#include <cassert>

struct ChunkMeshSnapshot {
	ChunkMeshSnapshot() {
		center.fill(BlockType::AIR);

		left.fill(BlockType::AIR);
		right.fill(BlockType::AIR);
		front.fill(BlockType::AIR);
		back.fill(BlockType::AIR);
	}

	ChunkMeshSnapshot& operator=(const ChunkMeshSnapshot& other) {
		//assert(&c == &other.c);

		center = other.center;
		left = other.left;
		right = other.right;
		front = other.front;
		back = other.back;

		hasLeft = other.hasLeft;
		hasRight = other.hasRight;
		hasFront = other.hasFront;
		hasBack = other.hasBack;

		return *this;
	}

	//Chunk& c;//対象のchunk

	std::array<BlockType, Chunk::CHUNK_SIZE> center;

	//store the boundary blocks of surrounding chunks that face the target chunk
	std::array<BlockType, Chunk::CHUNK_HEIGHT * Chunk::CHUNK_DEPTH> left;
	std::array<BlockType, Chunk::CHUNK_HEIGHT * Chunk::CHUNK_DEPTH> right;
	std::array<BlockType, Chunk::CHUNK_HEIGHT * Chunk::CHUNK_WIDTH> front;
	std::array<BlockType, Chunk::CHUNK_HEIGHT * Chunk::CHUNK_WIDTH> back;


	bool hasLeft = false;
	bool hasRight = false;
	bool hasFront = false;
	bool hasBack = false;

	unsigned int GetBoundaryBlock(int x, int y, int z, bool did_X_exceed) {
		//もしxが範囲外のものならzをつかう。z方向にはみ出してるならxを使う仕組みです。
		
		int index = 0;
		if (did_X_exceed) {
			index = IndexYZ(y, z);
		}
		else {
			index = IndexYX(y, x);
		}

		BlockType type = BlockType::AIR;
		if (did_X_exceed) {
			if (x < 0) {
				if (!hasLeft) return 0;
				type = left[index];
			}
			else if (x >= Chunk::CHUNK_WIDTH) {
				if (!hasRight) return 0;
				type = right[index];
			}
		}
		else {
			if (z < 0) {
				if (!hasBack) return 0;
				type = back[index];
			}
			else if (z >= Chunk::CHUNK_DEPTH) {
				if (!hasFront) return 0;
				type = front[index];
			}
		}
	

		return (unsigned int)type;
	}

	static int IndexYZ(int y, int z) {
		return z + Chunk::CHUNK_DEPTH * y;
	}

	static int IndexYX(int y, int x) {
		return x + Chunk::CHUNK_WIDTH * y;
	}

};