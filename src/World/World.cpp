
#include "World/World.h"
#include "World/TerrainGenerator.h"
#include "World/ChunkPipeline.h"
#include "Render/Camera.h"
#include <iostream>



void World::Tick(float dt, const Camera& cam) {
	curCx = static_cast<int32_t>(floorDiv(cam.position.x, Chunk::CHUNK_WIDTH));
	curCz = static_cast<int32_t>(floorDiv(cam.position.z, Chunk::CHUNK_DEPTH));

	bool enteredNewChunk =
		curCx != m_lastStreamCx ||
		curCz != m_lastStreamCz;

	if (enteredNewChunk) {
		m_chunkPipeline.SetStreamCenter(curCx, curCz);

		std::vector<uint64_t> canceledKey = 
			m_chunkPipeline.CancelQueuedOutside_ChunkJob();
		
		for (auto& key : canceledKey) {
			m_pendingChunkKeys.erase(key);
		}

		UpdateChunksAround(cam);
	}

	ProcessChunkResult();
}

World::World() : m_chunkPipeline(this, 114514) {//send seed value
	m_chunkPipeline.StartWorkerThread();
}

World::~World() {
	m_chunkPipeline.StopWorkerThread();
}

void World::UpdateChunksAround(const Camera& cam) {
	
	int createBudget = MAX_CHUNK_CREATE_PER_TICK;
	int destroyBudget = MAX_CHUNK_DESTROY_PER_TICK;

	bool createDone = false;

	for (int32_t r = 0; r <= LOAD_CHUNKS_DISTANCE && !createDone; ++r) {//これ頭いい自分の周囲からloadするアルゴリズムだわ
		for (int32_t dx = -r; dx <= r && !createDone; ++dx) {
			for (int32_t dz = -r; dz <= r; ++dz) {
				if (std::max(std::llabs(dx), std::llabs(dz)) != r) {//内側は処理済みなので外周だけ
					continue;
				}

				if (createBudget <= 0) {
					createDone = true;
					break;
				}

				int32_t cx = curCx + dx;
				int32_t cz = curCz + dz;

				uint64_t key = Index(cx, cz);

				if (chunks.find(key) != chunks.end()) {
					continue;
				}

				if (m_pendingChunkKeys.find(key) != m_pendingChunkKeys.end()) {
					continue;
				}

				if (createBudget <= 0) {
					createDone = true;
					break;
				}

				ChunkJob job;
				job.cx = cx;
				job.cz = cz;
				job.type = JobType::CREATE_CHUNK;
				

				m_pendingChunkKeys.insert(key);
				m_chunkPipeline.EnqueueJob(std::move(job));


				createBudget--;

			}
		}
	}

	/*for (int64_t cx = curCx - LOAD_CHUNKS_DISTANCE; cx <= curCx + LOAD_CHUNKS_DISTANCE && !createDone; ++cx) {
		for (int64_t cz = curCz - LOAD_CHUNKS_DISTANCE; cz <= curCz + LOAD_CHUNKS_DISTANCE; ++cz) {
			uint64_t key = Index(cx, cz);

			if (createBudget <= 0) {
				createDone = true;
				break;
			}

			if (chunks.find(key) != chunks.end()) {
				continue;
			}

			std::unique_ptr<Chunk> c = std::make_unique<Chunk>(cx, cz);

			TerrainGenerator::GenerateTerrain(*c);
			chunks[key] = std::move(c);

			createBudget--;
		}
	}*/


	for (auto it = chunks.begin(); it != chunks.end();) {
		const auto& c = it->second;

		if (destroyBudget <= 0) {
			break;
		}

		bool shouldDestroy = false;
		if (!c) {
			shouldDestroy = true;
		}
		else {

			int32_t dx = c->cx - curCx;
			int32_t dz = c->cz - curCz;


			if (std::abs(dx) >= UNLOAD_CHUNKS_DISTANCE ||
				std::abs(dz) >= UNLOAD_CHUNKS_DISTANCE) {

				shouldDestroy = true;
			}
		}


		if (shouldDestroy) {
			it = chunks.erase(it);
			destroyBudget--;
		}
		else {
			++it;//消したときは要素が自動で詰められるから消してないときだけitを増やして次の要素
		}

	}

}



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


void World::ProcessChunkResult() {
	MeshChunkResult meshResult;
	GeneratedChunkResult genResult;

	while (m_chunkPipeline.PopFrontMeshResult(meshResult)) {

		const auto& key = meshResult.key;

		int32_t cx = 0;
		int32_t cz = 0;
		

		if (meshResult.meshData) {
			m_pendingMeshData.push_back({ std::move(*meshResult.meshData), key });
		}

		if (meshResult.wasNewChunk) {
			MarkNeighborChunksDirty(cx, cz);
		}

	}


	while (m_chunkPipeline.PopFrontGenResult(genResult)) {

		const auto& key = genResult.key;

		int32_t cx = 0;
		int32_t cz = 0;


		m_pendingChunkKeys.erase(key);



		if (!genResult.chunk) {
			
			assert(false && "GenerateChunkResult doesnt have Chunk pointer");
			
		}

		chunks[key] = std::move(genResult.chunk);

		EnqueueMeshJob(*chunks[key].get());

	}
}


bool World::PopPendingMeshData(PendingMesh& out) {
	if (m_pendingMeshData.empty()) {
		return false;
	}

	out = std::move(m_pendingMeshData.front());
	m_pendingMeshData.pop_front();
	return true;
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
	}

	for (int32_t z = cz - 1; z <= cz + 1; ++z) {
		if (z == cz) continue;

		uint64_t key = Index(cx, z);
		auto it = chunks.find(key);

		if (it == chunks.end() || !it->second) {
			continue;
		}

		it->second->dirty = true;
	}
}



std::unique_ptr<ChunkMeshSnapshot> World::CreateMeshSnapshot(Chunk& c) {

	std::unique_ptr<ChunkMeshSnapshot> snapshot = std::make_unique<ChunkMeshSnapshot>();

	int32_t cx = c.cx;
	int32_t cz = c.cz;

	//center
	
	snapshot->center = c.blocks;

	//center lights
	snapshot->centerLights = c.blockLights;

	

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


std::unique_ptr<ChunkMeshSnapshot> World::CreateMeshSnapshotFromKey(uint64_t key) {
	std::unique_ptr<ChunkMeshSnapshot> snapshot = std::make_unique<ChunkMeshSnapshot>();

	auto& c = *GetTargetChunkFromKey(key);

	int32_t cx = c.cx;
	int32_t cz = c.cz;

	//center

	snapshot->center = c.blocks;

	//center lights
	snapshot->centerLights = c.blockLights;



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

void World::EnqueueMeshJob(Chunk& c) {

	ChunkJob job;
	job.cx = c.cx;
	job.cz = c.cz;
	job.snapshot = CreateMeshSnapshot(c);
	job.type = JobType::BUILD_MESH;

	job.isNewChunk = false;

	if (c.urgentUpdateMesh) {
		c.urgentUpdateMesh = false;
		job.urgent = true;
	}


	m_chunkPipeline.EnqueueJob(std::move(job));
	
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