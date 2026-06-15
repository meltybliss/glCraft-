#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <vector>

#include "World/Chunk.h"
#include "World/World.h"
#include "BlockFace.h"
#include "Math/UV.h"
#include "Math/Vec3.h"


enum class UVPoint {
	LeftTop,
	LeftBottom,
	RightBottom,
	RightTop
};

class MeshBuilder {
public:

	MeshData BuildChunkMesh(const World& w, const Chunk& c);

private:

	int atlasTilesX = 32;
	int atlasTilesY = 15;
private:
	

	UV GetUV(const BlockType b, uint8_t index, BlockFace face) const;
	UV GetBlockFaceUV(const BlockType b, uint8_t index, BlockFace face) const;
	UV GetTileVertexUV(uint8_t index, int tileX, int tileY) const;

	void AddFace(std::array<std::array<float, 3>, 4>& pointsSet, const BlockType b, const BlockFace face, std::vector<unsigned int>& indices, std::vector<float>& v);

};