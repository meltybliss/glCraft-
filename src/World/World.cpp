
#include "World/World.h"
#include "World/TerrainGenerator.h"
#include "World/ChunkPipeline.h"
#include "Render/Camera.h"
#include <iostream>



unsigned int World::GetBlockGlobal(int64_t x, int64_t y, int64_t z) const {
	int32_t cx = floorDiv(x, Chunk::CHUNK_WIDTH);
	int32_t cz = floorDiv(z, Chunk::CHUNK_DEPTH);

	int lx = floorMod(x, Chunk::CHUNK_WIDTH);
	int ly = y;
	int lz = floorMod(z, Chunk::CHUNK_DEPTH);

	auto it = chunks.find(Index(cx, cz));
	if (it == chunks.end() || !it->second) {
		return 0;
	}

	auto* c = it->second.get();

	return c->GetBlock(lx, ly, lz);

}

uint8_t World::GetBlockLightGlobal(int64_t x, int64_t y, int64_t z) const {

	int32_t cx = floorDiv(x, Chunk::CHUNK_WIDTH);
	int32_t cz = floorDiv(z, Chunk::CHUNK_DEPTH);

	int lx = floorMod(x, Chunk::CHUNK_WIDTH);
	int ly = y;
	int lz = floorMod(z, Chunk::CHUNK_DEPTH);

	auto it = chunks.find(Index(cx, cz));
	if (it == chunks.end() || !it->second) {
		return 0;
	}


	auto* c = it->second.get();

	return c->GetBlockLight(lx, ly, lz);
}


uint8_t World::GetSkyLightGlobal(int64_t x, int64_t y, int64_t z) const {
	int32_t cx = floorDiv(x, Chunk::CHUNK_WIDTH);
	int32_t cz = floorDiv(z, Chunk::CHUNK_DEPTH);

	int lx = floorMod(x, Chunk::CHUNK_WIDTH);
	int ly = y;
	int lz = floorMod(z, Chunk::CHUNK_DEPTH);

	auto it = chunks.find(Index(cx, cz));
	if (it == chunks.end() || !it->second) {
		return 0;
	}


	auto* c = it->second.get();

	return c->GetSkyLight(lx, ly, lz);

}


void World::SetBlockGlobal(int64_t x, int64_t y, int64_t z, BlockType b) {
	int32_t cx = floorDiv(x, Chunk::CHUNK_WIDTH);
	int32_t cz = floorDiv(z, Chunk::CHUNK_DEPTH);

	int lx = floorMod(x, Chunk::CHUNK_WIDTH);
	int ly = y;
	int lz = floorMod(z, Chunk::CHUNK_DEPTH);

	auto it = chunks.find(Index(cx, cz));
	if (it == chunks.end() || !it->second) {
		return;
	}

	auto* c = it->second.get();

	c->SetBlock(lx, ly, lz, b);

}


bool World::SetBlockLightGlobal(int64_t x, int64_t y, int64_t z, uint8_t level) {
	int32_t cx = floorDiv(x, Chunk::CHUNK_WIDTH);
	int32_t cz = floorDiv(z, Chunk::CHUNK_DEPTH);

	int lx = floorMod(x, Chunk::CHUNK_WIDTH);
	int ly = y;
	int lz = floorMod(z, Chunk::CHUNK_DEPTH);

	auto it = chunks.find(Index(cx, cz));
	if (it == chunks.end() || !it->second) {
		return false;
	}

	auto* c = it->second.get();

	return c->SetBlockLight(lx, ly, lz, level);

}


bool World::SetSkyLightGlobal(int64_t x, int64_t y, int64_t z, uint8_t level) {

	int32_t cx = floorDiv(x, Chunk::CHUNK_WIDTH);
	int32_t cz = floorDiv(z, Chunk::CHUNK_DEPTH);

	int lx = floorMod(x, Chunk::CHUNK_WIDTH);
	int ly = y;
	int lz = floorMod(z, Chunk::CHUNK_DEPTH);

	auto it = chunks.find(Index(cx, cz));
	if (it == chunks.end() || !it->second) {
		return false;
	}

	auto* c = it->second.get();

	return c->SetSkyLights(lx, ly, lz, level);
}


void World::SetBlockGlobal_User(int64_t x, int64_t y, int64_t z, BlockType b) {
	int32_t cx = floorDiv(x, Chunk::CHUNK_WIDTH);
	int32_t cz = floorDiv(z, Chunk::CHUNK_DEPTH);

	int lx = floorMod(x, Chunk::CHUNK_WIDTH);
	int ly = y;
	int lz = floorMod(z, Chunk::CHUNK_DEPTH);

	auto it = chunks.find(Index(cx, cz));
	if (it == chunks.end() || !it->second) {
		return;
	}

	auto* c = it->second.get();

	if (b != (BlockType)0) {
		if (c->GetBlock(lx, ly, lz) != 0) {
			return;
		}
	}

	c->SetBlock(lx, ly, lz, b);

	/*c->dirty = true;
	c->urgentUpdateMesh = true;*/
}


void World::DebugChunkInfo() {
	int count = 0;

	for (const auto& [key, c] : chunks) {
		count++;
		std::cout << count << "," << c->cx << "," << c->cz << "," << c->dirty << "\n";
	}
}



void World::MarkNeighborChunksDirty(const int32_t cx, const int32_t cz) {
	for (int32_t x = cx - 1; x <= cx + 1; ++x) {
		if (x == cx) continue;

		uint64_t key = Index(x, cz);
		auto it = chunks.find(key);

		if (it == chunks.end() || !it->second) {
			continue;
		}

		it->second->dirty = true;
		it->second->readyForMesh = true;
	}

	for (int32_t z = cz - 1; z <= cz + 1; ++z) {
		if (z == cz) continue;

		uint64_t key = Index(cx, z);
		auto it = chunks.find(key);

		if (it == chunks.end() || !it->second) {
			continue;
		}

		it->second->dirty = true;
		it->second->readyForMesh = true;
	}
}

void World::MarkNeighborChunksUrgentDirty(const int32_t cx, const int32_t cz) {
	for (int32_t x = cx - 1; x <= cx + 1; ++x) {
		if (x == cx) continue;

		uint64_t key = Index(x, cz);
		auto it = chunks.find(key);

		if (it == chunks.end() || !it->second) {
			continue;
		}

		it->second->urgentUpdateMesh = true;
		it->second->dirty = true;
		it->second->readyForMesh = true;
	}

	for (int32_t z = cz - 1; z <= cz + 1; ++z) {
		if (z == cz) continue;

		uint64_t key = Index(cx, z);
		auto it = chunks.find(key);

		if (it == chunks.end() || !it->second) {
			continue;
		}

		it->second->urgentUpdateMesh = true;
		it->second->dirty = true;
		it->second->readyForMesh = true;
	}

}


std::unique_ptr<ChunkMeshSnapshot> World::CreateMeshSnapshot(Chunk& c) {

	std::unique_ptr<ChunkMeshSnapshot> snapshot =
		std::make_unique<ChunkMeshSnapshot>();

	int32_t cx = c.cx;
	int32_t cz = c.cz;

	// center
	snapshot->center = c.blocks;

	// center lights
	snapshot->centerLights = c.blockLights;
	snapshot->centerSkyLights = c.skyLights; 

	//leftFront
	{
		uint64_t key = Index(cx - 1, cz + 1);

		auto it = chunks.find(key);

		if (it != chunks.end() && it->second) {
			Chunk* neighbor = it->second.get();
			snapshot->hasLeftFront = true;

			for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
				unsigned int b =
					neighbor->GetBlock(Chunk::CHUNK_WIDTH, y, 0);

				snapshot->leftFrontCorner[y] = (BlockType)b;
			}

		}
	}

	//leftBack
	{
		uint64_t key = Index(cx - 1, cz - 1);
		auto it = chunks.find(key);

		if (it != chunks.end() && it->second) {
			Chunk* neighbor = it->second.get();
			snapshot->hasLeftBack = true;

			for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
				unsigned int b =
					neighbor->GetBlock(Chunk::CHUNK_WIDTH, y, Chunk::CHUNK_DEPTH);

				snapshot->leftBackCorner[y] = (BlockType)b;
			}

		}
	}

	//rightFront
	{
		uint64_t key = Index(cx + 1, cz + 1);
		auto it = chunks.find(key);

		if (it != chunks.end() && it->second) {
			Chunk* neighbor = it->second.get();
			snapshot->hasRightFront = true;

			for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
				unsigned int b =
					neighbor->GetBlock(0, y, 0);

				snapshot->rightFrontCorner[y] = (BlockType)b;
			}

		}
	}

	//rightBack
	{
		uint64_t key = Index(cx + 1, cz - 1);
		auto it = chunks.find(key);

		if (it != chunks.end() && it->second) {
			Chunk* neighbor = it->second.get();
			snapshot->hasRightBack = true;

			for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
				unsigned int b =
					neighbor->GetBlock(0, y, Chunk::CHUNK_DEPTH);

				snapshot->rightBackCorner[y] = (BlockType)b;
			}

		}
	}

	// left
	{
		uint64_t key = Index(cx - 1, cz);
		auto it = chunks.find(key);

		if (it != chunks.end() && it->second) {
			Chunk* neighbor = it->second.get();
			snapshot->hasLeft = true;

			for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
				for (int z = 0; z < Chunk::CHUNK_DEPTH; ++z) {
					unsigned int b =
						neighbor->GetBlock(Chunk::CHUNK_WIDTH - 1, y, z);

					uint8_t l =
						neighbor->GetBlockLight(Chunk::CHUNK_WIDTH - 1, y, z);

					uint8_t sl =
						neighbor->GetSkyLight(Chunk::CHUNK_WIDTH - 1, y, z);

					const int index = ChunkMeshSnapshot::IndexYZ(y, z);

					snapshot->left[index] =
						static_cast<BlockType>(b);

					snapshot->leftLights[index] = l;
					snapshot->leftSkyLights[index] = sl;
				}
			}
		}
	}

	// right
	{
		uint64_t key = Index(cx + 1, cz);
		auto it = chunks.find(key);

		if (it != chunks.end() && it->second) {
			Chunk* neighbor = it->second.get();
			snapshot->hasRight = true;

			for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
				for (int z = 0; z < Chunk::CHUNK_DEPTH; ++z) {
					unsigned int b = neighbor->GetBlock(0, y, z);
					uint8_t l = neighbor->GetBlockLight(0, y, z);
					uint8_t sl = neighbor->GetSkyLight(0, y, z);

					const int index = ChunkMeshSnapshot::IndexYZ(y, z);

					snapshot->right[index] =
						static_cast<BlockType>(b);

					snapshot->rightLights[index] = l;
					snapshot->rightSkyLights[index] = sl;
				}
			}
		}
	}

	// front
	{
		uint64_t key = Index(cx, cz + 1);
		auto it = chunks.find(key);

		if (it != chunks.end() && it->second) {
			Chunk* neighbor = it->second.get();
			snapshot->hasFront = true;

			for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
				for (int x = 0; x < Chunk::CHUNK_WIDTH; ++x) {
					unsigned int b = neighbor->GetBlock(x, y, 0);
					uint8_t l = neighbor->GetBlockLight(x, y, 0);
					uint8_t sl = neighbor->GetSkyLight(x, y, 0);

					const int index = ChunkMeshSnapshot::IndexYX(y, x);

					snapshot->front[index] =
						static_cast<BlockType>(b);

					snapshot->frontLights[index] = l;
					snapshot->frontSkyLights[index] = sl;
				}
			}
		}
	}

	// back
	{
		uint64_t key = Index(cx, cz - 1);
		auto it = chunks.find(key);

		if (it != chunks.end() && it->second) {
			Chunk* neighbor = it->second.get();
			snapshot->hasBack = true;

			for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
				for (int x = 0; x < Chunk::CHUNK_WIDTH; ++x) {
					unsigned int b =
						neighbor->GetBlock(x, y, Chunk::CHUNK_DEPTH - 1);

					uint8_t l =
						neighbor->GetBlockLight(x, y, Chunk::CHUNK_DEPTH - 1);

					uint8_t sl =
						neighbor->GetSkyLight(x, y, Chunk::CHUNK_DEPTH - 1);

					const int index = ChunkMeshSnapshot::IndexYX(y, x);

					snapshot->back[index] =
						static_cast<BlockType>(b);

					snapshot->backLights[index] = l;
					snapshot->backSkyLights[index] = sl;
				}
			}
		}
	}

	return snapshot;
}

std::unique_ptr<ChunkMeshSnapshot> World::CreateMeshSnapshotFromKey(uint64_t key) {
	std::unique_ptr<ChunkMeshSnapshot> snapshot = std::make_unique<ChunkMeshSnapshot>();

	auto& c = *GetTargetChunkFromKey(key);

	int32_t cx = c.cx;
	int32_t cz = c.cz;

	//center

	snapshot->center = c.blocks;

	//center lights
	snapshot->centerLights = c.blockLights;


	snapshot->centerSkyLights = c.skyLights;

	//leftFront
	{
		uint64_t key = Index(cx - 1, cz + 1);

		auto it = chunks.find(key);

		if (it != chunks.end() && it->second) {
			Chunk* neighbor = it->second.get();
			snapshot->hasLeftFront = true;

			for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
				unsigned int b =
					neighbor->GetBlock(Chunk::CHUNK_WIDTH, y, 0);

				snapshot->leftFrontCorner[y] = (BlockType)b;
			}

		}
	}

	//leftBack
	{
		uint64_t key = Index(cx - 1, cz - 1);
		auto it = chunks.find(key);

		if (it != chunks.end() && it->second) {
			Chunk* neighbor = it->second.get();
			snapshot->hasLeftBack = true;

			for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
				unsigned int b =
					neighbor->GetBlock(Chunk::CHUNK_WIDTH, y, Chunk::CHUNK_DEPTH);

				snapshot->leftBackCorner[y] = (BlockType)b;
			}

		}
	}

	//rightFront
	{
		uint64_t key = Index(cx + 1, cz + 1);
		auto it = chunks.find(key);

		if (it != chunks.end() && it->second) {
			Chunk* neighbor = it->second.get();
			snapshot->hasRightFront = true;

			for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
				unsigned int b =
					neighbor->GetBlock(0, y, 0);

				snapshot->rightFrontCorner[y] = (BlockType)b;
			}

		}
	}

	//rightBack
	{
		uint64_t key = Index(cx + 1, cz - 1);
		auto it = chunks.find(key);

		if (it != chunks.end() && it->second) {
			Chunk* neighbor = it->second.get();
			snapshot->hasRightBack = true;

			for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
				unsigned int b =
					neighbor->GetBlock(0, y, Chunk::CHUNK_DEPTH);

				snapshot->rightBackCorner[y] = (BlockType)b;
			}

		}
	}

	//left
	{
		uint64_t key = Index(cx - 1, cz);
		auto it = chunks.find(key);
		if (it != chunks.end() && it->second) {
			Chunk* c = it->second.get();

			snapshot->hasLeft = true;


			for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
				for (int z = 0; z < Chunk::CHUNK_DEPTH; ++z) {
					unsigned int b = c->GetBlock(Chunk::CHUNK_WIDTH - 1, y, z);
					uint8_t l = c->GetBlockLight(Chunk::CHUNK_WIDTH - 1, y, z);
					uint8_t sl = c->GetSkyLight(Chunk::CHUNK_WIDTH - 1, y, z);

					snapshot->left[ChunkMeshSnapshot::IndexYZ(y, z)] =
						static_cast<BlockType>(b);

					snapshot->leftLights[ChunkMeshSnapshot::IndexYZ(y, z)] = l;
					snapshot->leftSkyLights[ChunkMeshSnapshot::IndexYZ(y, z)] = sl;
				}
			}
		}
	}

	//right
	{
		uint64_t key = Index(cx + 1, cz);
		auto it = chunks.find(key);
		if (it != chunks.end() && it->second) {
			Chunk* c = it->second.get();

			snapshot->hasRight = true;

			for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
				for (int z = 0; z < Chunk::CHUNK_DEPTH; ++z) {
					unsigned int b = c->GetBlock(0, y, z);
					uint8_t l = c->GetBlockLight(0, y, z);
					uint8_t sl = c->GetSkyLight(0, y, z);

					snapshot->right[ChunkMeshSnapshot::IndexYZ(y, z)] =
						static_cast<BlockType>(b);

					snapshot->rightLights[ChunkMeshSnapshot::IndexYZ(y, z)] = l;
					snapshot->rightSkyLights[ChunkMeshSnapshot::IndexYZ(y, z)] = sl;
				}
			}
		}
	}

	//front
	{
		uint64_t key = Index(cx, cz + 1);
		auto it = chunks.find(key);
		if (it != chunks.end() && it->second) {
			Chunk* c = it->second.get();

			snapshot->hasFront = true;

			for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
				for (int x = 0; x < Chunk::CHUNK_WIDTH; ++x) {
					unsigned int b = c->GetBlock(x, y, 0);
					uint8_t l = c->GetBlockLight(x, y, 0);
					uint8_t sl = c->GetSkyLight(x, y, 0);

					snapshot->front[ChunkMeshSnapshot::IndexYX(y, x)] =
						static_cast<BlockType>(b);

					snapshot->frontLights[ChunkMeshSnapshot::IndexYX(y, x)] = l;
					snapshot->frontSkyLights[ChunkMeshSnapshot::IndexYX(y, x)] = sl;
				}
			}
		}
	}


	//back
	{
		uint64_t key = Index(cx, cz - 1);
		auto it = chunks.find(key);
		if (it != chunks.end() && it->second) {
			Chunk* c = it->second.get();

			snapshot->hasBack = true;

			for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
				for (int x = 0; x < Chunk::CHUNK_WIDTH; ++x) {
					unsigned int b = c->GetBlock(x, y, Chunk::CHUNK_DEPTH - 1);
					uint8_t l = c->GetBlockLight(x, y, Chunk::CHUNK_DEPTH - 1);
					uint8_t sl = c->GetSkyLight(x, y, Chunk::CHUNK_DEPTH - 1);

					snapshot->back[ChunkMeshSnapshot::IndexYX(y, x)] =
						static_cast<BlockType>(b);

					snapshot->backLights[ChunkMeshSnapshot::IndexYX(y, x)] = l;
					snapshot->backSkyLights[ChunkMeshSnapshot::IndexYX(y, x)] = sl;
				}
			}
		}
	}

	return snapshot;
}



RaycastHit World::Raycast(
	const glm::vec3& origin, 
	const glm::vec3& dir, 
	float distance) const {


	double originX = static_cast<double>(origin.x);
	double originY = static_cast<double>(origin.y);
	double originZ = static_cast<double>(origin.z);

	const double dirX = static_cast<double>(dir.x);
	const double dirY = static_cast<double>(dir.y);
	const double dirZ = static_cast<double>(dir.z);

	int64_t x = static_cast<int64_t>(std::floor(originX));
	int64_t y = static_cast<int64_t>(std::floor(originY));
	int64_t z = static_cast<int64_t>(std::floor(originZ));

	if (GetBlockGlobal(x, y, z) != 0) {
		return RaycastHit{ true, x, y, z, x, y, z };
	}

	int64_t previousX = static_cast<int64_t>(std::floor(originX));
	int64_t previousY = static_cast<int64_t>(std::floor(originY));
	int64_t previousZ = static_cast<int64_t>(std::floor(originZ));

	
	const double inf = std::numeric_limits<double>::infinity();

	int stepX = 0;
	int stepY = 0;
	int stepZ = 0;

	double tMaxX = inf;
	double tMaxY = inf;
	double tMaxZ = inf;

	double tDeltaX = inf;
	double tDeltaY = inf;
	double tDeltaZ = inf;

	if (dirX > 0.0f) {
		stepX = 1;
		double nextBoundaryX = static_cast<double>(x + 1);
		tMaxX = (nextBoundaryX - originX) / dirX;
		tDeltaX = 1.0f / dirX;
	}
	else if (dirX < 0.0f) {
		stepX = -1;
		double nextBoundaryX = static_cast<double>(x);
		tMaxX = (nextBoundaryX - originX) / dirX;
		tDeltaX = -1.0f / dirX;
	}

	if (dirY > 0.0f) {
		stepY = 1;
		double nextBoundaryY = static_cast<double>(y + 1);
		tMaxY = (nextBoundaryY - originY) / dirY;
		tDeltaY = 1.0f / dirY;
	}
	else if (dirY < 0.0f) {
		stepY = -1;
		double nextBoundaryY = static_cast<double>(y);
		tMaxY = (nextBoundaryY - originY) / dirY;
		tDeltaY = -1.0f / dirY;
	}

	if (dirZ > 0.0f) {
		stepZ = 1;
		double nextBoundaryZ = static_cast<double>(z + 1);
		tMaxZ = (nextBoundaryZ - originZ) / dirZ;
		tDeltaZ = 1.0f / dirZ;
	}
	else if (dirZ < 0.0f) {
		stepZ = -1;
		double nextBoundaryZ = static_cast<double>(z);
		tMaxZ = (nextBoundaryZ - originZ) / dirZ;
		tDeltaZ = -1.0f / dirZ;
	}


	while (std::min({ tMaxX, tMaxY, tMaxZ }) <= distance) {

		previousX = x;
		previousY = y;
		previousZ = z;


		if (tMaxX < tMaxY && tMaxX < tMaxZ) {
			x += stepX;
			tMaxX += tDeltaX;
		}
		else if (tMaxY < tMaxZ) {
			y += stepY;
			tMaxY += tDeltaY;
		}
		else {
			z += stepZ;
			tMaxZ += tDeltaZ;
		}

		if (GetBlockGlobal(x, y, z) != 0) {
			return RaycastHit{ true, x, y, z, previousX, previousY, previousZ };
		}
		
	}

	return RaycastHit{ false, x, y, z, previousX, previousY, previousZ };
}