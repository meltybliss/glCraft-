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
						{ (float)x, (float)y,     (float)z + s },
						{ (float)x, (float)y,     (float)z     },
						{ (float)x, (float)y + s, (float)z     },
						{ (float)x, (float)y + s, (float)z + s }
					} };

					AddFace(pointsSet, (BlockType)b, BlockFace::LEFT, indices, v);
				}
				if (CheckNeighborAir(x + 1, y, z)) {
					std::array<std::array<float, 3>, 4> pointsSet = { {
						{ (float)x + s, (float)y,     (float)z     },
						{ (float)x + s, (float)y,     (float)z + s },
						{ (float)x + s, (float)y + s, (float)z + s },
						{ (float)x + s, (float)y + s, (float)z     }
					} };

					AddFace(pointsSet, (BlockType)b, BlockFace::RIGHT, indices, v);
				}

				if (CheckNeighborAir(x, y - 1, z)) {
					std::array<std::array<float, 3>, 4> pointsSet = { {
						{ (float)x,     (float)y, (float)z     },
						{ (float)x + s, (float)y, (float)z     },
						{ (float)x + s, (float)y, (float)z + s },
						{ (float)x,     (float)y, (float)z + s }
					} };



					AddFace(pointsSet, (BlockType)b, BlockFace::BOTTOM, indices, v);

				}

				if (CheckNeighborAir(x, y + 1, z)) {
					std::array<std::array<float, 3>, 4> pointsSet = { {
						{ (float)x,     (float)y + s, (float)z + s },
						{ (float)x + s, (float)y + s, (float)z + s },
						{ (float)x + s, (float)y + s, (float)z     },
						{ (float)x,     (float)y + s, (float)z     }
					} };


					AddFace(pointsSet, (BlockType)b, BlockFace::TOP, indices, v);
				}


				if (CheckNeighborAir(x, y, z - 1)) {
					std::array<std::array<float, 3>, 4> pointsSet = { {
						{ (float)x + s, (float)y,     (float)z },
						{ (float)x,     (float)y,     (float)z },
						{ (float)x,     (float)y + s, (float)z },
						{ (float)x + s, (float)y + s, (float)z }
					} };


					AddFace(pointsSet, (BlockType)b, BlockFace::FRONT, indices, v);
				}

				if (CheckNeighborAir(x, y, z + 1)) {
					std::array<std::array<float, 3>, 4> pointsSet = { {
						{ (float)x,     (float)y,     (float)z + s },
						{ (float)x + s, (float)y,     (float)z + s },
						{ (float)x + s, (float)y + s, (float)z + s },
						{ (float)x,     (float)y + s, (float)z + s }
					} };

					AddFace(pointsSet, (BlockType)b, BlockFace::BACK, indices, v);
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

	float insetU = pixelU * 0.94f;
	float insetV = pixelV * 0.94f;

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
	std::vector<unsigned int>& indices, std::vector<float>& v)
{

	unsigned int base = static_cast<unsigned int>(v.size() / 5);

	//base+0,1,2,3
	for (int i = 0; i < 4; i++) {
		UV uv = GetUV((BlockType)b, i, face);
		const std::array<float, 3>& points = pointsSet[i];

		v.insert(v.end(), { points[0], points[1], points[2], uv.u, uv.v });

	}

	indices.insert(
		indices.end(),
		{base + 1, base + 2, base + 3,
		 base + 0, base + 1, base + 3}
	);

}