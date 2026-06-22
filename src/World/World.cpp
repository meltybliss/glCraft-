
#include "World/World.h"
#include "World/TerrainGenerator.h"
#include "World/ChunkPipeline.h"
#include "Render/Camera.h"
#include <iostream>

int64_t floorDiv(int64_t a, int64_t b) {
	int64_t q = a / b;
	int64_t r = a % b;

	if (r != 0 && ((r < 0) != (b < 0))) {
		--q;
	}

	return q;

}

int floorMod(int64_t a, int b) {//-17, 16 -> 
	int64_t r = a % b;
	if (r < 0) {
		r += b;
	}

	return static_cast<int>(r);
}


void World::Tick(float dt, const Camera& cam) {

	UpdateChunksAround(cam);
	ProcessChunkResult();
}

World::World() : m_chunkPipeline(this) {
	m_chunkPipeline.StartWorkerThread();
}

World::~World() {
	m_chunkPipeline.StopWorkerThread();
}

void World::UpdateChunksAround(const Camera& cam) {
	int32_t curCx = static_cast<int32_t>(floorDiv(cam.position.x, Chunk::CHUNK_WIDTH));
	int32_t curCz = static_cast<int32_t>(floorDiv(cam.position.z, Chunk::CHUNK_DEPTH));

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
				job.meshSource = MeshBuildSource::INSTANCE_NEW_CHUNK;

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

	c->dirty = true;
}


void World::DebugChunkInfo() {
	int count = 0;

	for (const auto& [key, c] : chunks) {
		count++;
		std::cout << count << "," << c->cx << "," << c->cz << "," << c->dirty << "\n";
	}
}


void World::ProcessChunkResult() {
	ChunkResult result;

	while (m_chunkPipeline.PopFrontResult(result)) {

		const auto& key = result.key;

		bool registerdNewChunk = false;
		int32_t cx = 0;
		int32_t cz = 0;
		if (result.chunk) {//INSTANCE_NEW_CHUNKで、新規chunk生成とmesh生成がsetで起こった時
			m_pendingChunkKeys.erase(key);

			auto it = chunks.find(key);
			if (it == chunks.end()) {//workerで新規生成されたもの。meshだけjobしたならここには入らない
				if (!result.chunk) {
					std::cerr << "[ProcChunkResult] Couldnt find actual chunk data from worker thread\n";
					continue;
				}


				cx = result.chunk->cx;
				cz = result.chunk->cz;

				chunks[key] = std::move(result.chunk);
				registerdNewChunk = true;
			}



		}
		

		if (result.meshData) {
			m_pendingMeshData.push_back({ std::move(*result.meshData), key });
		}

		if (registerdNewChunk) {
			MarkNeighborChunksDirty(cx, cz);
		}

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

					snapshot->left[ChunkMeshSnapshot::IndexYZ(y, z)] =
						static_cast<BlockType>(b);
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

					snapshot->right[ChunkMeshSnapshot::IndexYZ(y, z)] =
						static_cast<BlockType>(b);
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

					snapshot->front[ChunkMeshSnapshot::IndexYX(y, x)] =
						static_cast<BlockType>(b);
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

					snapshot->back[ChunkMeshSnapshot::IndexYX(y, x)] =
						static_cast<BlockType>(b);
				}
			}
		}
	}

	return snapshot;
}


void World::EnqueueMeshJobFrom_Outside(Chunk& c) {

	ChunkJob job;
	job.cx = c.cx;
	job.cz = c.cz;
	job.snapshot = CreateMeshSnapshot(c);
	job.type = JobType::BUILD_MESH;
	job.meshSource = MeshBuildSource::SNAPSHOT;

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