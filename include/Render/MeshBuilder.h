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

	static MeshData BuildChunkMesh(const World& w, const Chunk& c);

private:

	static constexpr int atlasTilesX = 32;
	static constexpr int atlasTilesY = 15;
private:
	

	static UV GetUV(const BlockType b, uint8_t index, BlockFace face) const;
	static UV GetBlockFaceUV(const BlockType b, uint8_t index, BlockFace face) const;
	static UV GetTileVertexUV(uint8_t index, int tileX, int tileY) const;

	static void AddFace(std::array<std::array<float, 3>, 4>& pointsSet, const BlockType b, const BlockFace face, std::vector<unsigned int>& indices, std::vector<float>& v);

};