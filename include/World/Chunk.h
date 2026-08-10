#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "World/BlockData.h"
#include "PointLight.h"

#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>



struct Chunk {

	Chunk(int32_t x, int32_t z) : cx(x), cz(z)  {
		blocks.fill(BlockType::AIR);
	}
	

	void ReceiveBlocksVector(std::vector<BlockType> blocksVector) {

		std::copy_n(
			blocksVector.begin(),
			CHUNK_SIZE,
			blocks.begin()

		);

	}

	int32_t cx = 0;
	int32_t cz = 0;


	bool dirty = false;
	bool dirtyToSave = false;
	bool urgentUpdateMesh = false;


	constexpr static uint32_t CHUNK_WIDTH = 16;
	constexpr static uint32_t CHUNK_DEPTH = 16;
	constexpr static uint32_t CHUNK_HEIGHT = 256;

	constexpr static uint32_t CHUNK_SIZE = CHUNK_WIDTH * CHUNK_DEPTH * CHUNK_HEIGHT;


	std::array<BlockType, CHUNK_SIZE> blocks;
	std::array<uint8_t, CHUNK_SIZE> blockLights{0};
	std::array<uint8_t, CHUNK_SIZE> skyLights{0};
	std::vector<PointLight> pointLights{};
	std::array<glm::vec3, CHUNK_SIZE> blockLightColors{glm::vec3(0.f)};



	static int Index(int x, int y, int z) {
		return x + CHUNK_WIDTH * z + y * CHUNK_WIDTH * CHUNK_DEPTH;
	}

	[[nodiscard]] unsigned int GetBlock(int x, int y, int z) const {
		if (!InBounds(x, y, z)) {
			return 0;
		}

		return (unsigned int)blocks[Index(x, y, z)];
	}

	[[nodiscard]] uint8_t GetBlockLight(int x, int y, int z) const {
		if (!InBounds(x, y, z)) {
			return 0;
		}

		return blockLights[Index(x, y, z)];
	}

	[[nodiscard]] uint8_t GetSkyLight(int x, int y, int z) const {
		if (!InBounds(x, y, z)) {
			return 0;
		}

		return skyLights[Index(x, y, z)];
	}



	[[nodiscard]] glm::vec3 GetBlockLightColor(int x, int y, int z) const {
		if (!InBounds(x, y, z)) {
			return glm::vec3(0.0f);
		}

		return blockLightColors[Index(x, y, z)];
	}


	void SetBlock(int x, int y, int z, BlockType b) {
		if (!InBounds(x, y, z)) {
			return;
		}

		blocks[Index(x, y, z)] = b;

		dirty = true;
	}

	bool SetBlockLight(int x, int y, int z, uint8_t level, const glm::vec3& lightColor) {
		if (!InBounds(x, y, z)) {
			return false;
		}

		blockLights[Index(x, y, z)] = level;
		blockLightColors[Index(x, y, z)] = lightColor;


		return true;
	}


	bool SetSkyLights(int x, int y, int z, uint8_t level) {
		if (!InBounds(x, y, z)) {
			return false;
		}

		skyLights[Index(x, y, z)] = level;
		return true;
	}


	void SetBlockForGenerator(int x, int y, int z, BlockType b) {
		if (!InBounds(x, y, z)) {
			return;
		}
		blocks[Index(x, y, z)] = b;

	}


	void SetPointLight(const glm::i64vec3& pos, const glm::vec3& color, float radius, float intensity) {
		
		pointLights.emplace_back(pos, color, radius, intensity);
	}

	void RemovePointLight(const glm::i64vec3& pos) {

		auto it = std::find_if(
			pointLights.begin(),
			pointLights.end(),
			[&](const PointLight& light) {

				return light.position == pos;
			}

		);

		if (it != pointLights.end()) {
			pointLights.erase(it);
		}

	}

	static bool InBounds(int x, int y, int z) {
		return (x < CHUNK_WIDTH && x >= 0 && y < CHUNK_HEIGHT && y >= 0 &&
			z < CHUNK_DEPTH && z >= 0);
	}

};