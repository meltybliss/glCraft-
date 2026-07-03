#include "Render/MeshBuilder.h"
#include <iostream>


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
	{
		//left
		std::array<std::array<float, 3>, 4> pointsSet{ {
			{centerX - halfWidthX, (float)y + maxY, centerZ - halfWidthZ},
			{centerX - halfWidthX, (float)y, centerZ - halfWidthZ},
			{centerX - halfWidthX, (float)y, centerZ + halfWidthZ},
			{centerX - halfWidthX, (float)y + maxY, centerZ + halfWidthZ}

		} };

		std::vector<float> vertices_buffer;
		AddFace(pointsSet, BlockType::TORCH, BlockFace::LEFT, vertices_buffer);
		AddLightToVertex(x, y, z, BlockFace::LEFT, BlockType::TORCH, vertices_buffer, v, indices, snapShot);
	}

	//right
	{

		std::array<std::array<float, 3>, 4> pointsSet{ {
			{centerX + halfWidthX, (float)y + maxY, centerZ + halfWidthZ},
			{centerX + halfWidthX, (float)y, centerZ + halfWidthZ},
			{centerX + halfWidthX, (float)y, centerZ - halfWidthZ},
			{centerX + halfWidthX, (float)y + maxY, centerZ - halfWidthZ}

		} };

		std::vector<float> vertices_buffer;
		AddFace(pointsSet, BlockType::TORCH, BlockFace::RIGHT, vertices_buffer);
		AddLightToVertex(x, y, z, BlockFace::RIGHT, BlockType::TORCH, vertices_buffer, v, indices, snapShot);

	}


	//front
	{
		std::array<std::array<float, 3>, 4> pointsSet{ {
			{ centerX + halfWidthX, (float)y + maxY, centerZ - halfWidthZ },
			{ centerX + halfWidthX, (float)y,        centerZ - halfWidthZ },
			{ centerX - halfWidthX, (float)y,        centerZ - halfWidthZ },
			{ centerX - halfWidthX, (float)y + maxY, centerZ - halfWidthZ }
		} };

		std::vector<float> vertices_buffer;
		AddFace(pointsSet, BlockType::TORCH, BlockFace::FRONT, vertices_buffer);
		AddLightToVertex(x, y, z, BlockFace::FRONT, BlockType::TORCH, vertices_buffer, v, indices, snapShot);


	}

	//back

	{
		std::array<std::array<float, 3>, 4> pointsSet{ {
			{ centerX - halfWidthX, (float)y + maxY, centerZ + halfWidthZ },
			{ centerX - halfWidthX, (float)y,        centerZ + halfWidthZ },
			{ centerX + halfWidthX, (float)y,        centerZ + halfWidthZ },
			{ centerX + halfWidthX, (float)y + maxY, centerZ + halfWidthZ }
		} };

		std::vector<float> vertices_buffer;
		AddFace(pointsSet, BlockType::TORCH, BlockFace::BACK, vertices_buffer);
		AddLightToVertex(x, y, z, BlockFace::BACK, BlockType::TORCH, vertices_buffer, v, indices, snapShot);

	}

	//top
	{
		std::array<std::array<float, 3>, 4> pointsSet{ {
			{ centerX - halfWidthX, (float)y + maxY, centerZ - halfWidthZ },
			{ centerX - halfWidthX, (float)y + maxY, centerZ + halfWidthZ },
			{ centerX + halfWidthX, (float)y + maxY, centerZ + halfWidthZ },
			{ centerX + halfWidthX, (float)y + maxY, centerZ - halfWidthZ }
		} };

		std::vector<float> vertices_buffer;
		AddFace(pointsSet, BlockType::TORCH, BlockFace::TOP, vertices_buffer);
		AddLightToVertex(x, y, z, BlockFace::TOP, BlockType::TORCH, vertices_buffer, v, indices, snapShot);

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
		for (int x = 0; x < Chunk::CHUNK_WIDTH; x++) {
			for (int z = 0; z < Chunk::CHUNK_DEPTH; z++) {


				unsigned int b = (unsigned int)center[Chunk::Index(x, y, z)];

				if (b == 0) {//AIR
					continue;
				}

				//0----3
				//|	   |
				//1----2

				if ((BlockType)b != BlockType::TORCH) {

					{
						std::array<std::array<float, 3>, 4> pointsSet = { {
							{ (float)x, (float)y + s, (float)z     }, // 0: LeftTop
							{ (float)x, (float)y,     (float)z     }, // 1: LeftBottom
							{ (float)x, (float)y,     (float)z + s }, // 2: RightBottom
							{ (float)x, (float)y + s, (float)z + s }  // 3: RightTop
						} };

						std::vector<float> vertices_buffer;

						if (!CheckNeighborTorch(x - 1, y, z)) {
							if (CheckNeighborAir(x - 1, y, z)) {
								AddFace(pointsSet, (BlockType)b, BlockFace::LEFT, vertices_buffer);
								AddLightToVertex(x, y, z, BlockFace::LEFT, (BlockType)b, vertices_buffer, v, indices, snapshot);
							}
						}
						else {
							AddFace(pointsSet, (BlockType)b, BlockFace::LEFT, vertices_buffer);
							AddLightToVertex(x, y, z, BlockFace::LEFT, (BlockType)b, vertices_buffer, v, indices, snapshot);
						
						}

					}

					{
						std::array<std::array<float, 3>, 4> pointsSet = { {
							{ (float)x + s, (float)y + s, (float)z + s }, // 0: LeftTop
							{ (float)x + s, (float)y,     (float)z + s }, // 1: LeftBottom
							{ (float)x + s, (float)y,     (float)z     }, // 2: RightBottom
							{ (float)x + s, (float)y + s, (float)z     }  // 3: RightTop
						} };

						std::vector<float> vertices_buffer;


						if (!CheckNeighborTorch(x + 1, y, z)) {
							if (CheckNeighborAir(x + 1, y, z)) {

								AddFace(pointsSet, (BlockType)b, BlockFace::RIGHT, vertices_buffer);
								AddLightToVertex(x, y, z, BlockFace::RIGHT, (BlockType)b, vertices_buffer, v, indices, snapshot);
							}
						}
						else {
							AddFace(pointsSet, (BlockType)b, BlockFace::RIGHT, vertices_buffer);
							AddLightToVertex(x, y, z, BlockFace::RIGHT, (BlockType)b, vertices_buffer, v, indices, snapshot);
						
						}
					}


					{

						std::array<std::array<float, 3>, 4> pointsSet = { {
							{ (float)x,     (float)y, (float)z     },
							{ (float)x + s, (float)y, (float)z     },
							{ (float)x + s, (float)y, (float)z + s },
							{ (float)x,     (float)y, (float)z + s }
						} };


						std::vector<float> vertices_buffer;
					

						if (!CheckNeighborTorch(x, y - 1, z)) {
							if (CheckNeighborAir(x, y - 1, z)) {

								AddFace(pointsSet, (BlockType)b, BlockFace::BOTTOM, vertices_buffer);
								AddLightToVertex(x, y, z, BlockFace::BOTTOM, (BlockType)b, vertices_buffer, v, indices, snapshot);
							}
						}
						else {
							AddFace(pointsSet, (BlockType)b, BlockFace::BOTTOM, vertices_buffer);
							AddLightToVertex(x, y, z, BlockFace::BOTTOM, (BlockType)b, vertices_buffer, v, indices, snapshot);
						}
					}

					{
						std::array<std::array<float, 3>, 4> pointsSet = { {
							{ (float)x,     (float)y + s, (float)z + s },
							{ (float)x + s, (float)y + s, (float)z + s },
							{ (float)x + s, (float)y + s, (float)z     },
							{ (float)x,     (float)y + s, (float)z     }
						} };
						

						std::vector<float> vertices_buffer;

						if (!CheckNeighborTorch(x, y + 1, z)) {
							if (CheckNeighborAir(x, y + 1, z)) {
								
								AddFace(pointsSet, (BlockType)b, BlockFace::TOP, vertices_buffer);
								AddLightToVertex(x, y, z, BlockFace::TOP, (BlockType)b, vertices_buffer, v, indices, snapshot);
							}
						}
						else {
							
							AddFace(pointsSet, (BlockType)b, BlockFace::TOP, vertices_buffer);
							AddLightToVertex(x, y, z, BlockFace::TOP, (BlockType)b, vertices_buffer, v, indices, snapshot);
						}
					}


					{
						std::array<std::array<float, 3>, 4> pointsSet = { {
							{ (float)x + s, (float)y + s, (float)z }, // 0: LeftTop
							{ (float)x + s, (float)y,     (float)z }, // 1: LeftBottom
							{ (float)x,     (float)y,     (float)z }, // 2: RightBottom
							{ (float)x,     (float)y + s, (float)z }  // 3: RightTop
						} };


						std::vector<float> vertices_buffer;


						if (!CheckNeighborTorch(x, y, z - 1)) {
							if (CheckNeighborAir(x, y, z - 1)) {

								AddFace(pointsSet, (BlockType)b, BlockFace::FRONT, vertices_buffer);
								AddLightToVertex(x, y, z, BlockFace::FRONT, (BlockType)b, vertices_buffer, v, indices, snapshot);
							}
						}
						else {
							AddFace(pointsSet, (BlockType)b, BlockFace::FRONT, vertices_buffer);
							AddLightToVertex(x, y, z, BlockFace::FRONT, (BlockType)b, vertices_buffer, v, indices, snapshot);
						}
					}


					{

						std::array<std::array<float, 3>, 4> pointsSet = { {
							{ (float)x,     (float)y + s, (float)z + s }, // 0: LeftTop
							{ (float)x,     (float)y,     (float)z + s }, // 1: LeftBottom
							{ (float)x + s, (float)y,     (float)z + s }, // 2: RightBottom
							{ (float)x + s, (float)y + s, (float)z + s }  // 3: RightTop
						} };


						std::vector<float> vertices_buffer;


						if (!CheckNeighborTorch(x, y, z + 1)) {
							if (CheckNeighborAir(x, y, z + 1)) {

								AddFace(pointsSet, (BlockType)b, BlockFace::BACK, vertices_buffer);
								AddLightToVertex(x, y, z, BlockFace::BACK, (BlockType)b, vertices_buffer, v, indices, snapshot);

							}
						}
						else {
							AddFace(pointsSet, (BlockType)b, BlockFace::BACK, vertices_buffer);
							AddLightToVertex(x, y, z, BlockFace::BACK, (BlockType)b, vertices_buffer, v, indices, snapshot);

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


void MeshBuilder::AddFace(
	std::array<std::array<float, 3>, 4>& pointsSet, 
	const BlockType b, 
	const BlockFace face,
	std::vector<float>& buffer)
{


	//base+0,1,2,3
	for (int i = 0; i < 4; i++) {
		UV uv = GetUV((BlockType)b, i, face);
		const std::array<float, 3>& points = pointsSet[i];

		buffer.insert(buffer.end(), { points[0], points[1], points[2], uv.u, uv.v });

	}

	

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


void MeshBuilder::AddLightToVertex(
	int x, 
	int y, 
	int z, 
	const BlockFace face,
	const BlockType type,
	std::vector<float>& buffer, 
	std::vector<float>& v, 
	std::vector<unsigned int>& indices, 
	ChunkMeshSnapshot& snapShot) {



	unsigned int base = static_cast<unsigned int>(v.size() / 8);


	const auto& centerLights = snapShot.centerLights;
	const auto& centerSkyLights = snapShot.centerSkyLights;


	uint8_t next_lightLevel = 0;
	uint8_t next_skyLightLevel = 0;

	int tx = x;//target x
	int ty = y;
	int tz = z;

	

	std::array<float, 4> AO{};

	switch (face) {
		case BlockFace::RIGHT: {
			tx = x + 1;//target x
			ty = y;
			tz = z;

			AO[0] = BuildAOLight(x, y, z, snapShot, BlockFace::RIGHT, AoPoint::RightBackTop);
			AO[1] = BuildAOLight(x, y, z, snapShot, BlockFace::RIGHT, AoPoint::RightBackBottom);
			AO[2] = BuildAOLight(x, y, z, snapShot, BlockFace::RIGHT, AoPoint::RightFrontBottom);
			AO[3] = BuildAOLight(x, y, z, snapShot, BlockFace::RIGHT, AoPoint::RightFrontTop);

			

			break;

		}
		case BlockFace::LEFT: {
			tx = x - 1;
			ty = y;
			tz = z;

			AO[0] = BuildAOLight(x, y, z, snapShot, BlockFace::LEFT, AoPoint::LeftFrontTop);
			AO[1] = BuildAOLight(x, y, z, snapShot, BlockFace::LEFT, AoPoint::LeftFrontBottom);
			AO[2] = BuildAOLight(x, y, z, snapShot, BlockFace::LEFT, AoPoint::LeftBackBottom);
			AO[3] = BuildAOLight(x, y, z, snapShot, BlockFace::LEFT, AoPoint::LeftBackTop);

		
			break;

		}
		case BlockFace::FRONT: {
			tx = x;
			ty = y;
			tz = z - 1;

			AO[0] = BuildAOLight(x, y, z, snapShot, BlockFace::FRONT, AoPoint::RightFrontTop);
			AO[1] = BuildAOLight(x, y, z, snapShot, BlockFace::FRONT, AoPoint::RightFrontBottom);
			AO[2] = BuildAOLight(x, y, z, snapShot, BlockFace::FRONT, AoPoint::LeftFrontBottom);
			AO[3] = BuildAOLight(x, y, z, snapShot, BlockFace::FRONT, AoPoint::LeftFrontTop);

			break;
		}
		case BlockFace::BACK: {
			tx = x; 
			ty = y;
			tz = z + 1;


			AO[0] = BuildAOLight(x, y, z, snapShot, BlockFace::BACK, AoPoint::LeftBackTop);
			AO[1] = BuildAOLight(x, y, z, snapShot, BlockFace::BACK, AoPoint::LeftBackBottom);
			AO[2] = BuildAOLight(x, y, z, snapShot, BlockFace::BACK, AoPoint::RightBackBottom);
			AO[3] = BuildAOLight(x, y, z, snapShot, BlockFace::BACK, AoPoint::RightBackTop);

			break;

		}
		case BlockFace::TOP: {
			tx = x;
			ty = y + 1;
			tz = z;

			AO[0] = BuildAOLight(x, y, z, snapShot, BlockFace::TOP, AoPoint::LeftBackTop);
			AO[1] = BuildAOLight(x, y, z, snapShot, BlockFace::TOP, AoPoint::RightBackTop);
			AO[2] = BuildAOLight(x, y, z, snapShot, BlockFace::TOP, AoPoint::RightFrontTop);
			AO[3] = BuildAOLight(x, y, z, snapShot, BlockFace::TOP, AoPoint::LeftFrontTop);
			
			break;

		}
		case BlockFace::BOTTOM: {
			tx = x;
			ty = y - 1;
			tz = z;

			AO[0] = BuildAOLight(x, y, z, snapShot, BlockFace::BOTTOM, AoPoint::LeftFrontBottom);
			AO[1] = BuildAOLight(x, y, z, snapShot, BlockFace::BOTTOM, AoPoint::RightFrontBottom);
			AO[2] = BuildAOLight(x, y, z, snapShot, BlockFace::BOTTOM, AoPoint::RightBackBottom);
			AO[3] = BuildAOLight(x, y, z, snapShot, BlockFace::BOTTOM, AoPoint::LeftBackBottom);

			
			break;
		}

	}



	uint8_t emission = GetEmission(type);

	if (ty >= Chunk::CHUNK_HEIGHT ||
		ty < 0) {

		next_lightLevel = 0;
		next_skyLightLevel = 0;
	}
	else if (tx >= Chunk::CHUNK_WIDTH ||
		tx < 0) {

		next_lightLevel = snapShot.GetBoundaryLight(tx, ty, tz, true);
		next_skyLightLevel = snapShot.GetBoundary_SkyLight(tx, ty, tz, true);
	}
	else if (tz >= Chunk::CHUNK_DEPTH ||
		tz < 0) {

		next_lightLevel = snapShot.GetBoundaryLight(tx, ty, tz, false);
		next_skyLightLevel = snapShot.GetBoundary_SkyLight(tx, ty, tz, false);
	}
	else {

		next_lightLevel = centerLights[Chunk::Index(tx, ty, tz)];
		next_skyLightLevel = centerSkyLights[Chunk::Index(tx, ty, tz)];
	}
	

	uint8_t self_light = std::max(next_lightLevel, emission);

	for (int i = 4; i >= 1; --i) {
		int point = i * 5;

		buffer.insert(
			buffer.begin() + point,
			{
				static_cast<float>(self_light),
				static_cast<float>(next_skyLightLevel),
				AO[i-1]
			}
		);

	}

	v.insert(v.end(), buffer.begin(), buffer.end());

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