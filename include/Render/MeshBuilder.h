#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <vector>

#include "World/Chunk.h"
#include "BlockFace.h"
#include "Math/Vec2.h"
#include "Math/Vec3.h"

using UV = Vec2;

enum class UVPoint {
	LeftTop,
	LeftBottom,
	RightBottom,
	RightTop
};

class MeshBuilder {
public:

	MeshData BuildChunkMesh(const Chunk& c);

private:

	int atlasTilesX = 32;
	int atlasTilesY = 15;
private:
	

	UV GetUV(const BlockType b, uint8_t index, BlockFace face);
	UV GetBlockTile(const BlockType b, uint8_t index, BlockFace face);
	UV GetBlockUV(uint8_t index, int tileX, int tileY) const;

};