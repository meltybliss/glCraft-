#pragma once
#include "World/Chunk.h"
#include <array>
#include <cassert>
#include <type_traits>

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

		centerLights = other.centerLights;
		leftLights = other.leftSkyLights;
		rightLights = other.rightLights;
		frontLights = other.frontSkyLights;
		backLights = other.backSkyLights;

		centerSkyLights = other.centerSkyLights;
		leftSkyLights = other.leftSkyLights;
		rightSkyLights = other.rightSkyLights;
		frontSkyLights = other.frontSkyLights;
		backSkyLights = other.backSkyLights;

		hasLeft = other.hasLeft;
		hasRight = other.hasRight;
		hasFront = other.hasFront;
		hasBack = other.hasBack;

		return *this;
	}


	//blocks
	std::array<BlockType, Chunk::CHUNK_SIZE> center;

	//store the boundary blocks of surrounding chunks that face the target chunk
	std::array<BlockType, Chunk::CHUNK_HEIGHT * Chunk::CHUNK_DEPTH> left;
	std::array<BlockType, Chunk::CHUNK_HEIGHT * Chunk::CHUNK_DEPTH> right;
	std::array<BlockType, Chunk::CHUNK_HEIGHT * Chunk::CHUNK_WIDTH> front;
	std::array<BlockType, Chunk::CHUNK_HEIGHT * Chunk::CHUNK_WIDTH> back;

	std::array<BlockType, Chunk::CHUNK_HEIGHT> rightFrontCorner;
	std::array<BlockType, Chunk::CHUNK_HEIGHT> rightBackCorner;
	std::array<BlockType, Chunk::CHUNK_HEIGHT> leftFrontCorner;
	std::array<BlockType, Chunk::CHUNK_HEIGHT> leftBackCorner;


	// block light
	std::array<uint8_t, Chunk::CHUNK_SIZE> centerLights{};

	std::array<uint8_t, Chunk::CHUNK_HEIGHT * Chunk::CHUNK_DEPTH> leftLights{};
	std::array<uint8_t, Chunk::CHUNK_HEIGHT * Chunk::CHUNK_DEPTH> rightLights{};
	std::array<uint8_t, Chunk::CHUNK_HEIGHT * Chunk::CHUNK_WIDTH> frontLights{};
	std::array<uint8_t, Chunk::CHUNK_HEIGHT * Chunk::CHUNK_WIDTH> backLights{};

	// sky light
	std::array<uint8_t, Chunk::CHUNK_SIZE> centerSkyLights{};

	std::array<uint8_t, Chunk::CHUNK_HEIGHT * Chunk::CHUNK_DEPTH> leftSkyLights{};
	std::array<uint8_t, Chunk::CHUNK_HEIGHT * Chunk::CHUNK_DEPTH> rightSkyLights{};
	std::array<uint8_t, Chunk::CHUNK_HEIGHT * Chunk::CHUNK_WIDTH> frontSkyLights{};
	std::array<uint8_t, Chunk::CHUNK_HEIGHT * Chunk::CHUNK_WIDTH> backSkyLights{};


	bool hasLeft = false;
	bool hasRight = false;
	bool hasFront = false;
	bool hasBack = false;

	bool hasLeftFront = false;
	bool hasLeftBack = false;
	bool hasRightFront = false;
	bool hasRightBack = false;
	
	BlockType GetBlockFromCenter(int x, int y, int z) {

		return center[IndexXYZ(x, y, z)];
	}


	BlockType GetBlockFromCorner(int y, bool isRight, bool isFront) {

		if (y >= Chunk::CHUNK_HEIGHT || y < 0) {
			return BlockType::AIR;
		}

		if (isRight) {
			if (isFront) {
				if (!hasRightFront) return BlockType::AIR;
				return rightFrontCorner[y];
			}
			else {
				if (!hasRightBack) return BlockType::AIR;
				return rightBackCorner[y];
			}
		}
		else {
			if (isFront) {
				if (!hasLeftFront) return BlockType::AIR;
				return leftFrontCorner[y];
			}
			else {
				if (!hasLeftBack) return BlockType::AIR;
				return leftBackCorner[y];
			}
		}

	}

	BlockType GetBlockFromYXArray(int y, int x, bool isFront) {

		if (isFront) {

			return front[IndexYX(y, x)];
		}
		else {
			return back[IndexYX(y, x)];
		}


	}

	BlockType GetBlockFromYZArray(int y, int z, bool isRight) {

		if (isRight) {
			return right[IndexYZ(y, z)];
		}
		else {
			return left[IndexYZ(y, z)];
		}
	}


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

	uint8_t GetBoundary_SkyLight(int x, int y, int z, bool did_X_exceed) {
		//もしxが範囲外のものならzをつかう。z方向にはみ出してるならxを使う仕組みです。

		int index = 0;
		if (did_X_exceed) {
			index = IndexYZ(y, z);
		}
		else {
			index = IndexYX(y, x);
		}

		uint8_t level = 0;
		if (did_X_exceed) {
			if (x < 0) {
				if (!hasLeft) return 0;
				level = leftSkyLights[index];
			}
			else if (x >= Chunk::CHUNK_WIDTH) {
				if (!hasRight) return 0;
				level = rightSkyLights[index];
			}
		}
		else {
			if (z < 0) {
				if (!hasBack) return 0;
				level = backSkyLights[index];
			}
			else if (z >= Chunk::CHUNK_DEPTH) {
				if (!hasFront) return 0;
				level = frontSkyLights[index];
			}
		}


		return level;

	}

	uint8_t GetBoundaryLight(int x, int y, int z, bool did_X_exceed) {
		//もしxが範囲外のものならzをつかう。z方向にはみ出してるならxを使う仕組みです。

		int index = 0;
		if (did_X_exceed) {
			index = IndexYZ(y, z);
		}
		else {
			index = IndexYX(y, x);
		}

		uint8_t level = 0;
		if (did_X_exceed) {
			if (x < 0) {
				if (!hasLeft) return 0;
				level = leftLights[index];
			}
			else if (x >= Chunk::CHUNK_WIDTH) {
				if (!hasRight) return 0;
				level = rightLights[index];
			}
		}
		else {
			if (z < 0) {
				if (!hasBack) return 0;
				level = backLights[index];
			}
			else if (z >= Chunk::CHUNK_DEPTH) {
				if (!hasFront) return 0;
				level = frontLights[index];
			}
		}


		return level;

	}

	static int IndexYZ(int y, int z) {
		return z + Chunk::CHUNK_DEPTH * y;
	}

	static int IndexYX(int y, int x) {
		return x + Chunk::CHUNK_WIDTH * y;
	}

	static int IndexXYZ(int x, int y, int z) {
		return x + Chunk::CHUNK_WIDTH * z + y * Chunk::CHUNK_WIDTH * Chunk::CHUNK_DEPTH;
	}
};