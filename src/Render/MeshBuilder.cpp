#include "Render/MeshBuilder.h"
#include <iostream>
#include <chrono>

void MeshBuilder::BuildTorchMesh(
	int x,
	int y,
	int z,
	std::vector<float>& v,
	std::vector<unsigned int>& indices,
	ChunkMeshSnapshot& snapShot) {

	constexpr float halfWidthX = BLOCK_SIZE / 16.0f;
	constexpr float halfWidthZ = BLOCK_SIZE / 16.0f;

	constexpr float minY = 0.0f;
	constexpr float maxY = 0.75f;

	float centerX = (float)x + 0.5f;
	float centerZ = (float)z + 0.5f;
	//LEFT
	{
		const std::array<std::array<float, 3>, 4> pointsSet{ {
			{ centerX - halfWidthX, static_cast<float>(y) + maxY, centerZ - halfWidthZ },
			{ centerX - halfWidthX, static_cast<float>(y),        centerZ - halfWidthZ },
			{ centerX - halfWidthX, static_cast<float>(y),        centerZ + halfWidthZ },
			{ centerX - halfWidthX, static_cast<float>(y) + maxY, centerZ + halfWidthZ }
		} };

		AppendFace(
			pointsSet,
			x, y, z,
			BlockType::TORCH,
			BlockFace::LEFT,
			v, indices,
			snapShot
		);
	}

	//RIGHT
	{
		const std::array<std::array<float, 3>, 4> pointsSet{ {
			{ centerX + halfWidthX, static_cast<float>(y) + maxY, centerZ + halfWidthZ },
			{ centerX + halfWidthX, static_cast<float>(y),        centerZ + halfWidthZ },
			{ centerX + halfWidthX, static_cast<float>(y),        centerZ - halfWidthZ },
			{ centerX + halfWidthX, static_cast<float>(y) + maxY, centerZ - halfWidthZ }
		} };

		AppendFace(
			pointsSet,
			x, y, z,
			BlockType::TORCH,
			BlockFace::RIGHT,
			v, indices,
			snapShot
		);
	}

	//FRONT
	{
		const std::array<std::array<float, 3>, 4> pointsSet{ {
			{ centerX + halfWidthX, static_cast<float>(y) + maxY, centerZ - halfWidthZ },
			{ centerX + halfWidthX, static_cast<float>(y),        centerZ - halfWidthZ },
			{ centerX - halfWidthX, static_cast<float>(y),        centerZ - halfWidthZ },
			{ centerX - halfWidthX, static_cast<float>(y) + maxY, centerZ - halfWidthZ }
		} };

		AppendFace(
			pointsSet,
			x, y, z,
			BlockType::TORCH,
			BlockFace::FRONT,
			v, indices,
			snapShot
		);
	}

	//BACK
	{
		const std::array<std::array<float, 3>, 4> pointsSet{ {
			{ centerX - halfWidthX, static_cast<float>(y) + maxY, centerZ + halfWidthZ },
			{ centerX - halfWidthX, static_cast<float>(y),        centerZ + halfWidthZ },
			{ centerX + halfWidthX, static_cast<float>(y),        centerZ + halfWidthZ },
			{ centerX + halfWidthX, static_cast<float>(y) + maxY, centerZ + halfWidthZ }
		} };

		AppendFace(
			pointsSet,
			x, y, z,
			BlockType::TORCH,
			BlockFace::BACK,
			v, indices,
			snapShot
		);
	}

	//TOP
	{
		const std::array<std::array<float, 3>, 4> pointsSet{ {
			{ centerX - halfWidthX, static_cast<float>(y) + maxY, centerZ - halfWidthZ },
			{ centerX - halfWidthX, static_cast<float>(y) + maxY, centerZ + halfWidthZ },
			{ centerX + halfWidthX, static_cast<float>(y) + maxY, centerZ + halfWidthZ },
			{ centerX + halfWidthX, static_cast<float>(y) + maxY, centerZ - halfWidthZ }
		} };

		AppendFace(
			pointsSet,
			x, y, z,
			BlockType::TORCH,
			BlockFace::TOP,
			v, indices,
			snapShot
		);
	}


	//bottom
	/*{
		std::array<std::array<float, 3>, 4> pointsSet{ {
			{centerX - halfWidthX, (float)y, centerZ + halfWidthZ},
			{centerX - halfWidthX, (float)y, centerZ - halfWidthZ},
			{centerX + halfWidthX, (float)y, centerZ - halfWidthZ},
			{centerX + halfWidthX, (float)y, centerZ + halfWidthZ}

		} };

		std::vector<float> vertices_buffer;
		AddFace(pointsSet, BlockType::TORCH, BlockFace::BOTTOM, vertices_buffer);
		AddLightToVertex(x, y, z, BlockFace::BOTTOM, BlockType::TORCH, vertices_buffer, v, indices, snapShot);


	}*/

}

MeshData MeshBuilder::BuildChunkMesh(ChunkMeshSnapshot& snapshot) {
	MeshData result;

	auto& center = snapshot.center;
	auto& v = result.vertices;
	auto& indices = result.indices;


	const auto& s = BLOCK_SIZE;
	


	auto CheckNeighborAir = [&](int nx, int ny, int nz) -> bool {
		if (ny < 0 || ny >= Chunk::CHUNK_HEIGHT) {
			return true;
		}

		if (Chunk::InBounds(nx, ny, nz)) {
			
			return center[Chunk::Index(nx, ny, nz)] == (BlockType)0;
		}

		if (nx < 0 || nx >= Chunk::CHUNK_WIDTH) {
			return snapshot.GetBoundaryBlock(nx, ny, nz, true) == 0;
		}
		else if (nz < 0 || nz >= Chunk::CHUNK_DEPTH) {
			return snapshot.GetBoundaryBlock(nx, ny, nz, false) == 0;
		}

		return true;
	};

	auto CheckNeighborTorch = [&](int x, int y, int z) -> bool {
		if (y < 0 || y >= Chunk::CHUNK_HEIGHT) {
			return true;
		}

		if (Chunk::InBounds(x, y, z)) {

			return center[Chunk::Index(x, y, z)] == BlockType::TORCH;
		}

		if (x < 0 || x >= Chunk::CHUNK_WIDTH) {
			return snapshot.GetBoundaryBlock(x, y, z, true) == static_cast<unsigned int>(BlockType::TORCH);
		}
		else if (z < 0 || z >= Chunk::CHUNK_DEPTH) {
			return snapshot.GetBoundaryBlock(x, y, z, false) == static_cast<unsigned int>(BlockType::TORCH);
		}


		return false;
	};


	for (int y = 0; y < Chunk::CHUNK_HEIGHT; y++) {
		for (int z = 0; z < Chunk::CHUNK_DEPTH; z++) {
			for (int x = 0; x < Chunk::CHUNK_WIDTH; x++) {


				unsigned int b = (unsigned int)center[Chunk::Index(x, y, z)];

				if (b == 0) {//AIR
					continue;
				}

				//頂点の順番
				//0----3
				//|	   |
				//1----2

				if (static_cast<BlockType>(b) != BlockType::TORCH) {

					const BlockType blockType = static_cast<BlockType>(b);

					//LEFT
					{
						const bool visible =
							CheckNeighborTorch(x - 1, y, z) ||
							CheckNeighborAir(x - 1, y, z);

						if (visible) {
							const std::array<std::array<float, 3>, 4> pointsSet{ {
								{ static_cast<float>(x),     static_cast<float>(y) + s, static_cast<float>(z)     },
								{ static_cast<float>(x),     static_cast<float>(y),     static_cast<float>(z)     },
								{ static_cast<float>(x),     static_cast<float>(y),     static_cast<float>(z) + s },
								{ static_cast<float>(x),     static_cast<float>(y) + s, static_cast<float>(z) + s }
							} };

							AppendFace(
								pointsSet,
								x, y, z,
								blockType,
								BlockFace::LEFT,
								v, indices,
								snapshot
							);
						}
					}

					//RIGHT
					{
						const bool visible =
							CheckNeighborTorch(x + 1, y, z) ||
							CheckNeighborAir(x + 1, y, z);

						if (visible) {
							const std::array<std::array<float, 3>, 4> pointsSet{ {
								{ static_cast<float>(x) + s, static_cast<float>(y) + s, static_cast<float>(z) + s },
								{ static_cast<float>(x) + s, static_cast<float>(y),     static_cast<float>(z) + s },
								{ static_cast<float>(x) + s, static_cast<float>(y),     static_cast<float>(z)     },
								{ static_cast<float>(x) + s, static_cast<float>(y) + s, static_cast<float>(z)     }
							} };

							AppendFace(
								pointsSet,
								x, y, z,
								blockType,
								BlockFace::RIGHT,
								v, indices,
								snapshot
							);
						}
					}

					//BOTTOM
					{
						const bool visible =
							CheckNeighborTorch(x, y - 1, z) ||
							CheckNeighborAir(x, y - 1, z);

						if (visible) {
							const std::array<std::array<float, 3>, 4> pointsSet{ {
								{ static_cast<float>(x),     static_cast<float>(y), static_cast<float>(z)     },
								{ static_cast<float>(x) + s, static_cast<float>(y), static_cast<float>(z)     },
								{ static_cast<float>(x) + s, static_cast<float>(y), static_cast<float>(z) + s },
								{ static_cast<float>(x),     static_cast<float>(y), static_cast<float>(z) + s }
							} };

							AppendFace(
								pointsSet,
								x, y, z,
								blockType,
								BlockFace::BOTTOM,
								v, indices,
								snapshot
							);
						}
					}

					//TOP
					{
						const bool visible =
							CheckNeighborTorch(x, y + 1, z) ||
							CheckNeighborAir(x, y + 1, z);

						if (visible) {
							const std::array<std::array<float, 3>, 4> pointsSet{ {
								{ static_cast<float>(x),     static_cast<float>(y) + s, static_cast<float>(z) + s },
								{ static_cast<float>(x) + s, static_cast<float>(y) + s, static_cast<float>(z) + s },
								{ static_cast<float>(x) + s, static_cast<float>(y) + s, static_cast<float>(z)     },
								{ static_cast<float>(x),     static_cast<float>(y) + s, static_cast<float>(z)     }
							} };

							AppendFace(
								pointsSet,
								x, y, z,
								blockType,
								BlockFace::TOP,
								v, indices,
								snapshot
							);
						}
					}

					//FRONT
					{
						const bool visible =
							CheckNeighborTorch(x, y, z - 1) ||
							CheckNeighborAir(x, y, z - 1);

						if (visible) {
							const std::array<std::array<float, 3>, 4> pointsSet{ {
								{ static_cast<float>(x) + s, static_cast<float>(y) + s, static_cast<float>(z) },
								{ static_cast<float>(x) + s, static_cast<float>(y),     static_cast<float>(z) },
								{ static_cast<float>(x),     static_cast<float>(y),     static_cast<float>(z) },
								{ static_cast<float>(x),     static_cast<float>(y) + s, static_cast<float>(z) }
							} };

							AppendFace(
								pointsSet,
								x, y, z,
								blockType,
								BlockFace::FRONT,
								v, indices,
								snapshot
							);
						}
					}

					//BACK
					{
						const bool visible =
							CheckNeighborTorch(x, y, z + 1) ||
							CheckNeighborAir(x, y, z + 1);

						if (visible) {
							const std::array<std::array<float, 3>, 4> pointsSet{ {
								{ static_cast<float>(x),     static_cast<float>(y) + s, static_cast<float>(z) + s },
								{ static_cast<float>(x),     static_cast<float>(y),     static_cast<float>(z) + s },
								{ static_cast<float>(x) + s, static_cast<float>(y),     static_cast<float>(z) + s },
								{ static_cast<float>(x) + s, static_cast<float>(y) + s, static_cast<float>(z) + s }
							} };

							AppendFace(
								pointsSet,
								x, y, z,
								blockType,
								BlockFace::BACK,
								v, indices,
								snapshot
							);
						}
					}
				}
				else {

					BuildTorchMesh(x, y, z, v, indices, snapshot);
				}
			}

		}

	}


	return result;


}


UV MeshBuilder::GetUV(const BlockType b, uint8_t index, BlockFace face) {
	if (b == BlockType::AIR) {
		return { 0.f, 0.f };
	}

	return GetBlockFaceUV(b, index, face);

}

UV MeshBuilder::GetBlockFaceUV(const BlockType b, uint8_t index, BlockFace face) {
	switch (b) {
		case BlockType::GRASS: {
			if (face == BlockFace::TOP) {
				return GetTileVertexUV(
					index,
					2,
					0
				);
			}
			if (face == BlockFace::BOTTOM) {
				return GetTileVertexUV(
					index,
					18,
					1
				);
			}

			return GetTileVertexUV(
				index,
				3,
				0
			);
		}

		case BlockType::DIRT: {
			return GetTileVertexUV(index, 18, 1);
		}

		case BlockType::STONE: {
			return GetTileVertexUV(index, 19, 0);
		}

		case BlockType::TORCH: {
			if (face == BlockFace::TOP) {
				return GetTileVertexUVForTorch(index, 6, 14);
			}
			if (face == BlockFace::BOTTOM) {
				return GetTileVertexUVForTorch(index, 4, 14);
			}
			

			return GetTileVertexUVForTorch(index, 5, 14);
		}

		case BlockType::GLOWSTONE: {

			return GetTileVertexUV(index, 5, 7);
		}

		default: {
			return GetTileVertexUV(index, 0, 0);
		}
	}


}



UV MeshBuilder::GetTileVertexUVForTorch(uint8_t index, int tileX, int tileY) {

	//indexはuvの4ポイントのどれかを表す
	UVPoint p = (UVPoint)index;

	float tileUV_X = 1.0f / atlasTilesX;
	float tileUV_Y = 1.0f / atlasTilesY;

	float torchSideUV_X = tileUV_X / 4.0f;//松明はtexture内に4つ入ってるから

	float u0 = tileUV_X * tileX;
	float v1 = 1.0f - (tileUV_Y * tileY);

	float u1 = u0 + torchSideUV_X;
	float v0 = v1 - tileUV_Y;


	if (p == UVPoint::LeftTop) {
		return { u0, v1 };
	}
	if (p == UVPoint::LeftBottom) {
		return { u0, v0 };
	}
	if (p == UVPoint::RightBottom) {
		return { u1, v0 };
	}
	if (p == UVPoint::RightTop) {
		return { u1, v1 };
	}

	return { 0.f, 0.f };
}


UV MeshBuilder::GetTileVertexUV(uint8_t index, int tileX, int tileY) {
	//indexはuvの4ポイントのどれかを表す
	UVPoint p = (UVPoint)index;

	float pixelU = 1.0f / atlasPixelWidth;
	float pixelV = 1.0f / atlasPixelHeight;

	float insetU = pixelU * 0.99f;
	float insetV = pixelV * 0.99f;

	float tileUV_X = 1.0f / atlasTilesX;
	float tileUV_Y = 1.0f / atlasTilesY;

	float u0 = tileUV_X * tileX;
	float v1 = 1.0f - (tileUV_Y * tileY);

	float u1 = u0 + tileUV_X;
	float v0 = v1 - tileUV_Y;

	u0 += insetU;
	u1 -= insetU;
	v1 -= insetV;
	v0 += insetV;

	if (p == UVPoint::LeftTop) {
		return { u0, v1 };
	}
	if (p == UVPoint::LeftBottom) {
		return { u0, v0 };
	}
	if (p == UVPoint::RightBottom) {
		return { u1, v0 };
	}
	if (p == UVPoint::RightTop) {
		return { u1, v1 };
	}

	return { 0.f, 0.f };
}




glm::vec3 MeshBuilder::GetFaceNormal(const BlockFace face) {

	const glm::vec3 dirs[6] = {
		glm::vec3{0.f, 1.f, 0.f},
		glm::vec3{0.f, -1.f, 0.f},
		glm::vec3{1.f, 0.f, 0.f},
		glm::vec3{-1.f, 0.f, 0.f},
		glm::vec3{0.f, 0.f, -1.f},
		glm::vec3{0.f, 0.f, 1.f}


	};

	return dirs[static_cast<int>(face)];
}



BlockType MeshBuilder::GetBlockForAO(int x, int y, int z, ChunkMeshSnapshot& snapShot) {

	if (y >= Chunk::CHUNK_HEIGHT || y < 0) {
		return BlockType::AIR;
	}

	

	if (x < Chunk::CHUNK_WIDTH && x >= 0 &&
		z < Chunk::CHUNK_DEPTH && z >= 0) {

		return snapShot.GetBlockFromCenter(x, y, z);

	}


	if (x >= Chunk::CHUNK_WIDTH &&
		z >= Chunk::CHUNK_DEPTH) {

		return snapShot.GetBlockFromCorner(y, true, true);
	}
	else if (x >= Chunk::CHUNK_WIDTH &&
			 z < 0) {
		return snapShot.GetBlockFromCorner(y, true, false);
	}
	else if (x >= Chunk::CHUNK_WIDTH) {
		return snapShot.GetBlockFromYZArray(y, z, true);
	}

	if (x < 0 &&
		z >= Chunk::CHUNK_DEPTH) {

		return snapShot.GetBlockFromCorner(y, false, true);

	}
	else if (x < 0 &&
		z < 0) {

		return snapShot.GetBlockFromCorner(y, false, false);

	}
	else if (x < 0) {
		return snapShot.GetBlockFromYZArray(y, z, false);
	}

	if (z >= Chunk::CHUNK_DEPTH) {
		return snapShot.GetBlockFromYXArray(y, x, true);
	}
	else if (z < 0) {
		return snapShot.GetBlockFromYXArray(y, x, false);
	}


	return BlockType::AIR;
}


float MeshBuilder::BuildAOLight(int x, int y, int z, ChunkMeshSnapshot& snapShot, const BlockFace face, AoPoint point) {
	
	int sx = 0;
	int sy = 0;
	int sz = 0;

	switch (point) {
	case AoPoint::LeftFrontBottom:
		sx = -1; sy = -1; sz = -1;
		break;

	case AoPoint::LeftFrontTop:
		sx = -1; sy = +1; sz = -1;
		break;

	case AoPoint::RightFrontBottom:
		sx = +1; sy = -1; sz = -1;
		break;

	case AoPoint::RightFrontTop:
		sx = +1; sy = +1; sz = -1;
		break;

	case AoPoint::LeftBackBottom:
		sx = -1; sy = -1; sz = +1;
		break;

	case AoPoint::LeftBackTop:
		sx = -1; sy = +1; sz = +1;
		break;

	case AoPoint::RightBackBottom:
		sx = +1; sy = -1; sz = +1;
		break;

	case AoPoint::RightBackTop:
		sx = +1; sy = +1; sz = +1;
		break;
	}

	
	auto opaqueAt = [&](int dx, int dy, int dz) {
		return isOpaque(GetBlockForAO(
			x + dx,
			y + dy,
			z + dz,
			snapShot
		));
	};

	bool side1 = false;
	bool side2 = false;
	bool corner = false;

	switch (face) {
	case BlockFace::TOP: 

		side1 = opaqueAt(sx, +1, 0);
		side2 = opaqueAt(0, +1, sz);
		corner = opaqueAt(sx, +1, sz);
		break;

	case BlockFace::BOTTOM:
		side1 = opaqueAt(sx, -1, 0);
		side2 = opaqueAt(0, -1, sz);
		corner = opaqueAt(sx, -1, sz);
		break;

	case BlockFace::LEFT:
		side1 = opaqueAt(-1, sy, 0);
		side2 = opaqueAt(-1, 0, sz);
		corner = opaqueAt(-1, sy, sz);
		break;

	case BlockFace::RIGHT:
		side1 = opaqueAt(+1, sy, 0);
		side2 = opaqueAt(+1, 0, sz);
		corner = opaqueAt(+1, sy, sz);
		break;

	case BlockFace::FRONT:
		side1 = opaqueAt(sx, 0, -1);
		side2 = opaqueAt(0, sy, -1);
		corner = opaqueAt(sx, sy, -1);
		break;

	case BlockFace::BACK:
		side1 = opaqueAt(sx, 0, +1);
		side2 = opaqueAt(0, sy, +1);
		corner = opaqueAt(sx, sy, +1);
		break;
	}

	

	float value = GetAOBrightness(side1, side2, corner);

	return value;

}


void MeshBuilder::AppendFace(
	const std::array<std::array<float, 3>, 4>& positions,
	int x,
	int y,
	int z,
	BlockType block,
	BlockFace face,
	std::vector<float>& vertices,
	std::vector<unsigned int>& indices,
	ChunkMeshSnapshot& snapshot
) {


	unsigned int base = static_cast<unsigned int>(vertices.size() / 11);


	const auto& centerLights = snapshot.centerLights;
	const auto& centerSkyLights = snapshot.centerSkyLights;

	

	//normal
	const glm::vec3 normal = GetFaceNormal(face);


	int tx = 0;
	int ty = 0;
	int tz = 0;
	//ao
	std::array<float, 4> AO{};

	switch (face) {
		case BlockFace::RIGHT: {
			tx = x + 1;//target x
			ty = y;
			tz = z;

			AO[0] = BuildAOLight(x, y, z, snapshot, BlockFace::RIGHT, AoPoint::RightBackTop);
			AO[1] = BuildAOLight(x, y, z, snapshot, BlockFace::RIGHT, AoPoint::RightBackBottom);
			AO[2] = BuildAOLight(x, y, z, snapshot, BlockFace::RIGHT, AoPoint::RightFrontBottom);
			AO[3] = BuildAOLight(x, y, z, snapshot, BlockFace::RIGHT, AoPoint::RightFrontTop);



			break;

		}
		case BlockFace::LEFT: {
			tx = x - 1;
			ty = y;
			tz = z;

			AO[0] = BuildAOLight(x, y, z, snapshot, BlockFace::LEFT, AoPoint::LeftFrontTop);
			AO[1] = BuildAOLight(x, y, z, snapshot, BlockFace::LEFT, AoPoint::LeftFrontBottom);
			AO[2] = BuildAOLight(x, y, z, snapshot, BlockFace::LEFT, AoPoint::LeftBackBottom);
			AO[3] = BuildAOLight(x, y, z, snapshot, BlockFace::LEFT, AoPoint::LeftBackTop);


			break;

		}
		case BlockFace::FRONT: {
			tx = x;
			ty = y;
			tz = z - 1;

			AO[0] = BuildAOLight(x, y, z, snapshot, BlockFace::FRONT, AoPoint::RightFrontTop);
			AO[1] = BuildAOLight(x, y, z, snapshot, BlockFace::FRONT, AoPoint::RightFrontBottom);
			AO[2] = BuildAOLight(x, y, z, snapshot, BlockFace::FRONT, AoPoint::LeftFrontBottom);
			AO[3] = BuildAOLight(x, y, z, snapshot, BlockFace::FRONT, AoPoint::LeftFrontTop);

			break;
		}
		case BlockFace::BACK: {
			tx = x;
			ty = y;
			tz = z + 1;


			AO[0] = BuildAOLight(x, y, z, snapshot, BlockFace::BACK, AoPoint::LeftBackTop);
			AO[1] = BuildAOLight(x, y, z, snapshot, BlockFace::BACK, AoPoint::LeftBackBottom);
			AO[2] = BuildAOLight(x, y, z, snapshot, BlockFace::BACK, AoPoint::RightBackBottom);
			AO[3] = BuildAOLight(x, y, z, snapshot, BlockFace::BACK, AoPoint::RightBackTop);

			break;

		}
		case BlockFace::TOP: {
			tx = x;
			ty = y + 1;
			tz = z;

			AO[0] = BuildAOLight(x, y, z, snapshot, BlockFace::TOP, AoPoint::LeftBackTop);
			AO[1] = BuildAOLight(x, y, z, snapshot, BlockFace::TOP, AoPoint::RightBackTop);
			AO[2] = BuildAOLight(x, y, z, snapshot, BlockFace::TOP, AoPoint::RightFrontTop);
			AO[3] = BuildAOLight(x, y, z, snapshot, BlockFace::TOP, AoPoint::LeftFrontTop);

			break;

		}
		case BlockFace::BOTTOM: {
			tx = x;
			ty = y - 1;
			tz = z;

			AO[0] = BuildAOLight(x, y, z, snapshot, BlockFace::BOTTOM, AoPoint::LeftFrontBottom);
			AO[1] = BuildAOLight(x, y, z, snapshot, BlockFace::BOTTOM, AoPoint::RightFrontBottom);
			AO[2] = BuildAOLight(x, y, z, snapshot, BlockFace::BOTTOM, AoPoint::RightBackBottom);
			AO[3] = BuildAOLight(x, y, z, snapshot, BlockFace::BOTTOM, AoPoint::LeftBackBottom);


			break;
		}

	}

	//light
	uint8_t next_lightLevel = 0;
	uint8_t next_skyLightLevel = 0;

	uint8_t emission = GetEmission(block);
	glm::vec3 blockLightColor;

	if (ty >= Chunk::CHUNK_HEIGHT ||
		ty < 0) {

		next_lightLevel = 0;
		next_skyLightLevel = 0;

		blockLightColor = glm::vec3(0.f);
	}
	else if (tx >= Chunk::CHUNK_WIDTH ||
		tx < 0) {

		next_lightLevel = snapshot.GetBoundaryLight(tx, ty, tz, true);
		next_skyLightLevel = snapshot.GetBoundary_SkyLight(tx, ty, tz, true);


	}
	else if (tz >= Chunk::CHUNK_DEPTH ||
		tz < 0) {

		next_lightLevel = snapshot.GetBoundaryLight(tx, ty, tz, false);
		next_skyLightLevel = snapshot.GetBoundary_SkyLight(tx, ty, tz, false);

	}
	else {

		next_lightLevel = centerLights[Chunk::Index(tx, ty, tz)];
		next_skyLightLevel = centerSkyLights[Chunk::Index(tx, ty, tz)];


	}


	uint8_t faceBlockLightLevel = std::max(next_lightLevel, emission);

	for (int i = 0; i < 4; ++i) {

		const auto& p = positions[i];
		const UV uv = GetUV(block, i, face);

		vertices.insert(
			vertices.end(),
			{
				p[0], p[1], p[2],
				uv.u, uv.v,
				normal.x, normal.y, normal.z,
				static_cast<float>(faceBlockLightLevel),
				static_cast<float>(next_skyLightLevel),
				AO[i],
				
			}
		);

	}
	

	indices.insert(
		indices.end(),
		{ base + 1, base + 2, base + 3,
		 base + 0, base + 1, base + 3 }
	);

}



int MeshBuilder::GetAOLevel(
	bool side1Opaque,
	bool side2Opaque,
	bool cornerOpaque
) {

	if (side1Opaque && side2Opaque) {
		return 3;
	}

	return
		static_cast<int>(side1Opaque) +
		static_cast<int>(side2Opaque) +
		static_cast<int>(cornerOpaque);

}


float MeshBuilder::GetAOBrightness(
	bool side1Opaque,
	bool side2Opaque,
	bool cornerOpaque
) {

	int level = GetAOLevel(
		side1Opaque,
		side2Opaque,
		cornerOpaque
	);


	float occlusion = static_cast<float>(level) / 3.0f;

	constexpr float AO_STRENGTH = 0.42f;

	return 1.0f - occlusion * AO_STRENGTH;
}



UVMinMax MeshBuilder::GetTorchUVMinMax() {

	float tileUV_x = 1.0f / atlasTilesX;
	float tileUV_y = 1.0f / atlasTilesY;


	float uvXMin = tileUV_x * 5;
	float uvYMax = 1.f - tileUV_y * 14;

	float uvXMax = uvXMin + (tileUV_x / 4.0f);
	float uvYMin = uvYMax - tileUV_y;

	return { glm::vec2(uvXMin, uvYMin), glm::vec2(uvXMax, uvYMax) };

}


