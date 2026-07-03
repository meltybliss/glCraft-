#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <vector>

#include "World/Chunk.h"
#include "MeshData.h"
#include "World/World.h"
#include "World/ChunkMeshSnapshot.h"
#include "BlockFace.h"
#include "Math/UV.h"
#include "Math/Vec3.h"


enum class UVPoint {
	LeftTop,
	LeftBottom,
	RightBottom,
	RightTop
};


enum class AoPoint {
	LeftFrontBottom,
	LeftFrontTop,
	LeftBackTop,
	LeftBackBottom,

	RightFrontBottom,
	RightFrontTop,
	RightBackTop,
	RightBackBottom
};

class MeshBuilder {
public:

	static MeshData BuildChunkMesh(ChunkMeshSnapshot& snapshot);

private:

	static constexpr int atlasTilesX = 32;
	static constexpr int atlasTilesY = 16;

	static constexpr int atlasPixelWidth = 512;
	static constexpr int atlasPixelHeight = 256;
private:
	
	static void BuildTorchMesh(int x, int y, int z, std::vector<float>& v, std::vector<unsigned int>& indices, ChunkMeshSnapshot& snapShot);

	static BlockType GetBlockForAO(int x, int y, int z, ChunkMeshSnapshot& snapShot);

	static UV GetUV(const BlockType b, uint8_t index, BlockFace face);
	static UV GetBlockFaceUV(const BlockType b, uint8_t index, BlockFace face);
	static UV GetTileVertexUV(uint8_t index, int tileX, int tileY);
	static UV GetTileVertexUVForTorch(uint8_t index, int tileX, int tileY);

	static int GetAOLevel(
		bool side1Opaque,
		bool side2Opaque,
		bool cornerOpaque
	);

	static float GetAOBrightness(
		bool side1Opaque,
		bool side2Opaque,
		bool cornerOpaque
	);

	static void AddFace(std::array<std::array<float, 3>, 4>& pointsSet, const BlockType b, const BlockFace face, std::vector<float>& buffer);
	static float BuildAOLight(int x, int y, int z, ChunkMeshSnapshot& snapShot, const BlockFace face, AoPoint point);
	static void AddLightToVertex(int x, int y, int z, const BlockFace face, const BlockType type, std::vector<float>& buffer, std::vector<float>& v, std::vector<unsigned int>& indices, ChunkMeshSnapshot& snapShot);
};