
#include "World/World.h"
#include "World/TerrainGenerator.h"
#include "World/ChunkPipeline.h"
#include "Render/Camera.h"
#include <iostream>
#include <algorithm>



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



glm::vec3 World::GetBlockLightColorGlobal(int64_t x, int64_t y, int64_t z) const {
	int32_t cx = floorDiv(x, Chunk::CHUNK_WIDTH);
	int32_t cz = floorDiv(z, Chunk::CHUNK_DEPTH);

	int lx = floorMod(x, Chunk::CHUNK_WIDTH);
	int ly = y;
	int lz = floorMod(z, Chunk::CHUNK_DEPTH);

	auto it = chunks.find(Index(cx, cz));
	if (it == chunks.end() || !it->second) {
		return glm::vec3(0.0f);
	}


	auto* c = it->second.get();
	if (!c) return glm::vec3(0.0f);

	return c->GetBlockLightColor(lx, ly, lz);
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


	BlockType oldB = (BlockType)c->GetBlock(lx, ly, lz);

	c->SetBlock(lx, ly, lz, b);


	if (b == BlockType::AIR &&
		isLightSourceBlock(oldB)) {//test

		c->RemovePointLight(
			glm::i64vec3(x, y, z)
		);

		return;

	}


	//test
	if (isLightSourceBlock(b)) {
		
		c->SetPointLight(
			glm::i64vec3(x, y, z),
			lightColor[(int)b],
			pointLightRadius[(int)b],
			4.5f);



	}

}


bool World::SetBlockLightGlobal(int64_t x, int64_t y, int64_t z, uint8_t level, const glm::vec3& lightColor) {
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



	return c->SetBlockLight(lx, ly, lz, level, lightColor);

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

	BlockType oldB = (BlockType)c->GetBlock(lx, ly, lz);

	c->SetBlock(lx, ly, lz, b);


	if (b == BlockType::AIR &&
		isLightSourceBlock(oldB)) {//test

		c->RemovePointLight(
			glm::i64vec3(x, y, z)
		);

		return;

	}

	//test
	if (isLightSourceBlock(b)) {

		c->SetPointLight(
			glm::i64vec3(x, y, z),
			lightColor[(int)b],
			pointLightRadius[(int)b],
			4.5f);



	}

	/*c->dirty = true;
	c->urgentUpdateMesh = true;*/
}



void World::SelectOptimalPointLights(int32_t cx, int32_t cz, std::array<PointLight*, 16>& out, size_t& count) {

	std::vector<PointLight*> candidates;

	const int64_t chunkMinX =
		cx * static_cast<int64_t>(Chunk::CHUNK_WIDTH);

	const int64_t chunkMinZ =
		cz * static_cast<int64_t>(Chunk::CHUNK_DEPTH);


	const int64_t chunkMaxX =
		chunkMinX + Chunk::CHUNK_WIDTH;

	const int64_t chunkMaxZ =
		chunkMinZ + Chunk::CHUNK_DEPTH;




	for (int x = -1; x <= 1; ++x) {
		for (int z = -1; z <= 1; ++z) {

			auto it = chunks.find(Index(cx + x, cz + z));
			if (it == chunks.end() || !it->second) continue;

			auto* c = it->second.get();
			
			for (auto& pl : c->pointLights) {

				const int64_t closestX = std::clamp(
					pl.position.x,
					chunkMinX,
					chunkMaxX
				);

				const int64_t closestZ = std::clamp(
					pl.position.z,
					chunkMinZ,
					chunkMaxZ
				);


				const double dxToChunk =
					static_cast<double>(
						pl.position.x - closestX
					);

				const double dzToChunk =
					static_cast<double>(
						pl.position.z - closestZ
					);


				const double distanceSquared =
					dxToChunk * dxToChunk +
					dzToChunk * dzToChunk;

				const double radiusSquared =
					static_cast<double>(pl.radius) *
					static_cast<double>(pl.radius);


				if (distanceSquared > radiusSquared) continue;

				candidates.push_back(&pl);
				
			}

		}
	}


	std::sort(candidates.begin(), candidates.end(),
		[&](const PointLight* a, const PointLight* b) {

			auto distanceSquared = [&](const PointLight* light) {

				const int64_t closestX = std::clamp(
					light->position.x,
					chunkMinX,
					chunkMaxX
				);


				const int64_t closestZ = std::clamp(
					light->position.z,
					chunkMinZ,
					chunkMaxZ
				);

				const double dx =
					static_cast<double>(light->position.x - closestX);

				const double dz =
					static_cast<double>(light->position.z - closestZ);


				return dx * dx + dz * dz;
			};


			return distanceSquared(a) < distanceSquared(b);
		}
	);


	size_t c = std::min(candidates.size(), out.size());
	count = c;

	for (size_t i = 0; i < c; ++i)
		out[i] = candidates[i];


}




void World::DebugChunkInfo() {
	int count = 0;

	for (const auto& [key, c] : chunks) {
		count++;
		std::cout << count << "," << c->cx << "," << c->cz << "," << c->dirty << "\n";
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

	snapshot->centerBlockLightColors =
		c.blockLightColors;

	//leftFront
	{
		uint64_t key = Index(cx - 1, cz + 1);

		auto it = chunks.find(key);

		if (it != chunks.end() && it->second) {
			Chunk* neighbor = it->second.get();
			snapshot->hasLeftFront = true;

			

			for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
				unsigned int b =
					neighbor->GetBlock(Chunk::CHUNK_WIDTH-1, y, 0);

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
					neighbor->GetBlock(Chunk::CHUNK_WIDTH-1, y, Chunk::CHUNK_DEPTH-1);

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
					neighbor->GetBlock(0, y, Chunk::CHUNK_DEPTH-1);

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

					const size_t src =
						Chunk::Index(
							Chunk::CHUNK_WIDTH - 1,
							y,
							z
						);

					const size_t dst =
						ChunkMeshSnapshot::IndexYZ(y, z);

					snapshot->leftBlockLightColors[dst] =
						neighbor->blockLightColors[src];
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

					const size_t src =
						Chunk::Index(
							0,
							y,
							z
						);

					const size_t dst =
						ChunkMeshSnapshot::IndexYZ(y, z);

					snapshot->rightBlockLightColors[dst] =
						neighbor->blockLightColors[src];
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

					const size_t src =
						Chunk::Index(
							x,
							y,
							0
						);

					const size_t dst =
						ChunkMeshSnapshot::IndexYX(y, x);

					snapshot->frontBlockLightColors[dst] =
						neighbor->blockLightColors[src];
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


					const size_t src =
						Chunk::Index(
							x,
							y,
							Chunk::CHUNK_DEPTH-1
						);

					const size_t dst =
						ChunkMeshSnapshot::IndexYX(y, x);

					snapshot->backBlockLightColors[dst] =
						neighbor->blockLightColors[src];
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

	snapshot->centerBlockLightColors = c.blockLightColors;

	//leftFront
	{
		uint64_t key = Index(cx - 1, cz + 1);

		auto it = chunks.find(key);

		if (it != chunks.end() && it->second) {
			Chunk* neighbor = it->second.get();
			snapshot->hasLeftFront = true;

			for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
				unsigned int b =
					neighbor->GetBlock(Chunk::CHUNK_WIDTH-1, y, 0);

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
					neighbor->GetBlock(Chunk::CHUNK_WIDTH-1, y, Chunk::CHUNK_DEPTH-1);

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
					neighbor->GetBlock(0, y, Chunk::CHUNK_DEPTH-1);

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

					const size_t src =
						Chunk::Index(
							Chunk::CHUNK_WIDTH - 1,
							y,
							z
						);

					const size_t dst =
						ChunkMeshSnapshot::IndexYZ(y, z);

					snapshot->leftBlockLightColors[dst] =
						c->blockLightColors[src];
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


					const size_t src =
						Chunk::Index(
							0,
							y,
							z
						);

					const size_t dst =
						ChunkMeshSnapshot::IndexYZ(y, z);

					snapshot->rightBlockLightColors[dst] =
						c->blockLightColors[src];
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

					const size_t src =
						Chunk::Index(
							x,
							y,
							0
						);

					const size_t dst =
						ChunkMeshSnapshot::IndexYX(y, x);

					snapshot->frontBlockLightColors[dst] =
						c->blockLightColors[src];
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



					const size_t src =
						Chunk::Index(
							x,
							y,
							Chunk::CHUNK_DEPTH - 1
						);

					const size_t dst =
						ChunkMeshSnapshot::IndexYX(y, x);

					snapshot->backBlockLightColors[dst] =
						c->blockLightColors[src];
				}
			}
		}
	}

	return snapshot;
}



RaycastHit World::Raycast(
	const WorldPos& origin,
	const glm::vec3& direction,
	float distance) const {


	int64_t x = origin.block.x;
	int64_t y = origin.block.y;
	int64_t z = origin.block.z;

	const double localX = origin.local.x;
	const double localY = origin.local.y;
	const double localZ = origin.local.z;

	const glm::dvec3 dir =
		glm::normalize(glm::dvec3(direction));


	if (GetBlockGlobal(x, y, z) != 0) {
		return RaycastHit{ true, x, y, z, x, y, z };
	}

	int64_t previousX = x;
	int64_t previousY = y;
	int64_t previousZ = z;

	
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

	if (dir.x > 0.0) {
		stepX = 1;
		tMaxX = (1.0 - localX) / dir.x;
		tDeltaX = 1.0 / dir.x;
	}
	else if (dir.x < 0.0) {
		stepX = -1;
		tMaxX = localX / -dir.x;
		tDeltaX = 1.0 / -dir.x;
	}

	if (dir.y > 0.0f) {
		stepY = 1;
		tMaxY = (1.0 - localY) / dir.y;
		tDeltaY = 1.0 / dir.y;
	}
	else if (dir.y < 0.0f) {
		stepY = -1;
		tMaxY = localY / -dir.y;
		tDeltaY = 1.0 / -dir.y;
	}

	if (dir.z > 0.0)
	{
		stepZ = 1;
		tMaxZ = (1.0 - localZ) / dir.z;
		tDeltaZ = 1.0 / dir.z;
	}
	else if (dir.z < 0.0)
	{
		stepZ = -1;
		tMaxZ = localZ / -dir.z;
		tDeltaZ = 1.0 / -dir.z;
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



std::unique_ptr<LightVolumeSnapshot> World::CreateLightVSnapshot(const glm::i64vec3 camBlockPos) const {

	std::unique_ptr<LightVolumeSnapshot> out;

	constexpr int channel_count = 4;

	using namespace LIGHT_VOLUME_SIZE;

	std::vector<float> pixels(
		static_cast<size_t>(LIGHT_VOLUME_WIDTH) *
		static_cast<size_t>(LIGHT_VOLUME_HEIGHT) *
		static_cast<size_t>(LIGHT_VOLUME_DEPTH) *
		channel_count,
		0.0f
	);



	auto GetPixelsIndex = [=](int x, int y, int z) -> size_t {

		size_t texelIndex = (size_t)x + (size_t)y * LIGHT_VOLUME_WIDTH + (size_t)z * LIGHT_VOLUME_WIDTH * LIGHT_VOLUME_HEIGHT;

		return texelIndex * channel_count;

	};


	auto SetPixel = [&](size_t index, const glm::vec3& color, uint8_t blockLightLevel) -> void {

		float intensity = blockLightLevel / 15.0f;

		const glm::vec3 rgb = color * intensity;


		pixels[index] = rgb.r;
		pixels[index + 1] = rgb.g;
		pixels[index + 2] = rgb.b;
		pixels[index + 3] = 1.0f;


	};


	glm::i64vec3 m_lightVolumeOrigin = glm::i64vec3(
		camBlockPos.x
		- LIGHT_VOLUME_WIDTH / 2,

		camBlockPos.y
		- LIGHT_VOLUME_HEIGHT / 2,

		camBlockPos.z
		- LIGHT_VOLUME_DEPTH / 2
	);


	for (int z = 0; z < LIGHT_VOLUME_DEPTH; ++z) {
		for (int y = 0; y < LIGHT_VOLUME_HEIGHT; ++y) {
			for (int x = 0; x < LIGHT_VOLUME_WIDTH; ++x) {

				glm::i64vec3 texelToWorldPos = m_lightVolumeOrigin + glm::i64vec3(x, y, z);

				size_t index = GetPixelsIndex(x, y, z);


				if (texelToWorldPos.y >= Chunk::CHUNK_HEIGHT ||
					texelToWorldPos.y < 0) continue;

				glm::vec3 lightColor = GetBlockLightColorGlobal(
					texelToWorldPos.x,
					texelToWorldPos.y,
					texelToWorldPos.z
				);
				uint8_t lightLevel = GetBlockLightGlobal(
					texelToWorldPos.x,
					texelToWorldPos.y,
					texelToWorldPos.z
				);



				SetPixel(index, lightColor, lightLevel);

			}
		}
	}


	out = std::make_unique<LightVolumeSnapshot>(
		pixels, m_lightVolumeOrigin
	);

	return out;
}



DayNightState const World::GetDayNightStateForDebug() {

	std::lock_guard<std::mutex> lock(dayNightStateMutex);

	return m_dayNight;

}


void World::SetDebugStateFromDebug(const DebugActions& actions) {


	std::lock_guard<std::mutex> lock(dayNightStateMutex);
	
	if (actions.timePaused) {
		m_dayNight.paused = actions.timePaused.value();
	}

	if (actions.timeOfDay) {
		m_dayNight.timeOfDay = actions.timeOfDay.value();
	}

	if (actions.dayLengthSeconds) {
		m_dayNight.dayLengthSeconds = actions.dayLengthSeconds.value();
	}

	if (actions.timeScale) {
		m_dayNight.timeScale = actions.timeScale.value();
	}


}



void World::SetWorldSeed(uint64_t seed) {
	m_seed = seed;
}