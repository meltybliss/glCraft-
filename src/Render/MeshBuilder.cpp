#include "Render/MeshBuilder.h"

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


				if (CheckNeighborAir(x - 1, y, z)) {
					std::array<std::array<float, 3>, 4> pointsSet = { {
						{ (float)x, (float)y + s, (float)z     }, // 0: LeftTop
						{ (float)x, (float)y,     (float)z     }, // 1: LeftBottom
						{ (float)x, (float)y,     (float)z + s }, // 2: RightBottom
						{ (float)x, (float)y + s, (float)z + s }  // 3: RightTop
					} };

					std::vector<float> vertices_buffer;

					AddFace(pointsSet, (BlockType)b, BlockFace::LEFT, vertices_buffer);
					AddLightToVertex(x, y, z, BlockFace::LEFT, vertices_buffer, v, indices, snapshot);
				}
				if (CheckNeighborAir(x + 1, y, z)) {
					std::array<std::array<float, 3>, 4> pointsSet = { {
						{ (float)x + s, (float)y + s, (float)z + s }, // 0: LeftTop
						{ (float)x + s, (float)y,     (float)z + s }, // 1: LeftBottom
						{ (float)x + s, (float)y,     (float)z     }, // 2: RightBottom
						{ (float)x + s, (float)y + s, (float)z     }  // 3: RightTop
					} };

					std::vector<float> vertices_buffer;

					AddFace(pointsSet, (BlockType)b, BlockFace::RIGHT, vertices_buffer);
					AddLightToVertex(x, y, z, BlockFace::RIGHT, vertices_buffer, v, indices, snapshot);
				}

				if (CheckNeighborAir(x, y - 1, z)) {
					std::array<std::array<float, 3>, 4> pointsSet = { {
						{ (float)x,     (float)y, (float)z     },
						{ (float)x + s, (float)y, (float)z     },
						{ (float)x + s, (float)y, (float)z + s },
						{ (float)x,     (float)y, (float)z + s }
					} };


					std::vector<float> vertices_buffer;

					AddFace(pointsSet, (BlockType)b, BlockFace::BOTTOM, vertices_buffer);
					AddLightToVertex(x, y, z, BlockFace::BOTTOM, vertices_buffer, v, indices, snapshot);

				}

				if (CheckNeighborAir(x, y + 1, z)) {
					std::array<std::array<float, 3>, 4> pointsSet = { {
						{ (float)x,     (float)y + s, (float)z + s },
						{ (float)x + s, (float)y + s, (float)z + s },
						{ (float)x + s, (float)y + s, (float)z     },
						{ (float)x,     (float)y + s, (float)z     }
					} };

					std::vector<float> vertices_buffer;

					AddFace(pointsSet, (BlockType)b, BlockFace::TOP, vertices_buffer);
					AddLightToVertex(x, y, z, BlockFace::TOP, vertices_buffer, v, indices, snapshot);
				}


				if (CheckNeighborAir(x, y, z - 1)) {
					std::array<std::array<float, 3>, 4> pointsSet = { {
						{ (float)x + s, (float)y + s, (float)z }, // 0: LeftTop
						{ (float)x + s, (float)y,     (float)z }, // 1: LeftBottom
						{ (float)x,     (float)y,     (float)z }, // 2: RightBottom
						{ (float)x,     (float)y + s, (float)z }  // 3: RightTop
					} };


					std::vector<float> vertices_buffer;

					AddFace(pointsSet, (BlockType)b, BlockFace::FRONT, vertices_buffer);
					AddLightToVertex(x, y, z, BlockFace::FRONT, vertices_buffer, v, indices, snapshot);
				}

				if (CheckNeighborAir(x, y, z + 1)) {
					std::array<std::array<float, 3>, 4> pointsSet = { {
						{ (float)x,     (float)y + s, (float)z + s }, // 0: LeftTop
						{ (float)x,     (float)y,     (float)z + s }, // 1: LeftBottom
						{ (float)x + s, (float)y,     (float)z + s }, // 2: RightBottom
						{ (float)x + s, (float)y + s, (float)z + s }  // 3: RightTop
					} };


					std::vector<float> vertices_buffer;

					AddFace(pointsSet, (BlockType)b, BlockFace::BACK, vertices_buffer);
					AddLightToVertex(x, y, z, BlockFace::BACK, vertices_buffer, v, indices, snapshot);
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

		default: {
			return GetTileVertexUV(index, 0, 0);
		}
	}


}

UV MeshBuilder::GetTileVertexUV(uint8_t index, int tileX, int tileY) {
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


float MeshBuilder::BuildAOLight(int x, int y, int z, ChunkMeshSnapshot& snapShot, AoPoint point) {
	
	const auto& left = snapShot.left;
	const auto& right = snapShot.right;
	const auto& front = snapShot.front;
	const auto& back = snapShot.back;

	int leftNx = x - 1;
	int rightNx = x + 1;
	int frontNz = z + 1;
	int backNz = z - 1;
	
	bool side1 = false;
	bool side2 = false;
	bool corner = false;
	switch (point) {
		case AoPoint::LeftFrontBottom: {
			side1 = isOpaque(GetBlockForAO(x - 1, y - 1, z, snapShot));//leftBottom
			side2 = isOpaque(GetBlockForAO(x, y - 1, z + 1, snapShot));//frontBottom
			corner = isOpaque(GetBlockForAO(x - 1, y - 1, z + 1, snapShot));//leftfrontBottom

			break;
		}
		case AoPoint::LeftFrontTop: {
			side1 = isOpaque(GetBlockForAO(x - 1, y + 1, z, snapShot));//leftTop
			side2 = isOpaque(GetBlockForAO(x, y + 1, z + 1, snapShot));//frontTop
			corner = isOpaque(GetBlockForAO(x - 1, y + 1, z + 1, snapShot));//leftFrontTop

			break;

		}
		case AoPoint::RightFrontBottom: {
			side1 = isOpaque(GetBlockForAO(x + 1, y - 1, z, snapShot));//rightBottom
			side2 = isOpaque(GetBlockForAO(x, y - 1, z + 1, snapShot));//frontBottom
			corner = isOpaque(GetBlockForAO(x + 1, y - 1, z + 1, snapShot));//rightFrontBottom

			break;
		}
		case AoPoint::RightFrontTop: {
			side1 = isOpaque(GetBlockForAO(x + 1, y + 1, z, snapShot));//rightTop
			side2 = isOpaque(GetBlockForAO(x, y + 1, z + 1, snapShot));//frontTop
			corner = isOpaque(GetBlockForAO(x + 1, y + 1, z + 1, snapShot));//rightFrontTop

			break;

		}
		case AoPoint::LeftBackBottom: {
			side1 = isOpaque(GetBlockForAO(x - 1, y - 1, z, snapShot));//leftBottom
			side2 = isOpaque(GetBlockForAO(x, y - 1, z - 1, snapShot));//backBottom
			corner = isOpaque(GetBlockForAO(x - 1, y - 1, z - 1, snapShot));//leftBackBottom

			break;
		}
		case AoPoint::LeftBackTop: {
			side1 = isOpaque(GetBlockForAO(x - 1, y + 1, z, snapShot));//leftTop
			side2 = isOpaque(GetBlockForAO(x, y + 1, z - 1, snapShot));//backTop
			corner = isOpaque(GetBlockForAO(x - 1, y + 1, z - 1, snapShot));//leftBackTop

			break;
		}
		case AoPoint::RightBackBottom: {
			side1 = isOpaque(GetBlockForAO(x + 1, y - 1, z, snapShot));//rightBottom
			side2 = isOpaque(GetBlockForAO(x, y - 1, z - 1, snapShot));//backBottom
			corner = isOpaque(GetBlockForAO(x + 1, y - 1, z - 1, snapShot));//rightBackBottom

			break;
		}

		case AoPoint::RightBackTop: {
			side1 = isOpaque(GetBlockForAO(x + 1, y + 1, z, snapShot));//rightTop
			side2 = isOpaque(GetBlockForAO(x, y + 1, z - 1, snapShot));//backTop
			corner = isOpaque(GetBlockForAO(x + 1, y + 1, z - 1, snapShot));//rightBackTop

			break;
		}
	}


	return GetAOBrightness(side1, side2, corner);
}


void MeshBuilder::AddLightToVertex(
	int x, 
	int y, 
	int z, 
	const BlockFace face,
	std::vector<float>& buffer, 
	std::vector<float>& v, 
	std::vector<unsigned int>& indices, 
	ChunkMeshSnapshot& snapShot) {



	unsigned int base = static_cast<unsigned int>(v.size() / 7);


	const auto& centerLights = snapShot.centerLights;
	const auto& centerSkyLights = snapShot.centerSkyLights;


	uint8_t next_lightLevel = 0;
	uint8_t next_skyLightLevel = 0;

	int tx = x;//target x
	int ty = y;
	int tz = z;

	switch (face) {
		case BlockFace::RIGHT: {
			tx = x + 1;//target x
			ty = y;
			tz = z;

			break;

		}
		case BlockFace::LEFT: {
			tx = x - 1;
			ty = y;
			tz = z;

		
			break;

		}
		case BlockFace::FRONT: {
			tx = x;
			ty = y;
			tz = z - 1;

			break;
		}
		case BlockFace::BACK: {
			tx = x; 
			ty = y;
			tz = z + 1;


			break;

		}
		case BlockFace::TOP: {
			tx = x;
			ty = y + 1;
			tz = z;

			
			break;

		}
		case BlockFace::BOTTOM: {
			tx = x;
			ty = y - 1;
			tz = z;

			
			break;
		}

	}

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
	


	for (int i = 4; i >= 1; --i) {
		int point = i * 5;

		buffer.insert(
			buffer.begin() + point,
			{
				static_cast<float>(next_lightLevel),
				static_cast<float>(next_skyLightLevel)
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

	constexpr float AO_STRENGTH = 0.45f;

	return 1.0f - occlusion * AO_STRENGTH;
}