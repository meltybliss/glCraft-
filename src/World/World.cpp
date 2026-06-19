
#pragma once
#include "World/Chunk.h"
#include <array>
#include <cassert>

struct ChunkMeshSnapshot {
	explicit ChunkMeshSnapshot(Chunk& chunk) : c(chunk) {
		left.fill(BlockType::AIR);
		right.fill(BlockType::AIR);
		front.fill(BlockType::AIR);
		back.fill(BlockType::AIR);
	}

	ChunkMeshSnapshot& operator=(const ChunkMeshSnapshot& other) {
		assert(&c == &other.c);

		left = other.left;
		right = other.right;
		front = other.front;
		back = other.back;

		hasLeft = other.hasLeft;
		hasRight = other.hasRight;
		hasFront = other.hasFront;
		hasBack = other.hasBack;

		return *this;
	}

	Chunk& c;//対象のchunk

	//store the boundary blocks of surrounding chunks that face the target chunk
	std::array<BlockType, Chunk::CHUNK_HEIGHT* Chunk::CHUNK_DEPTH> left;
	std::array<BlockType, Chunk::CHUNK_HEIGHT* Chunk::CHUNK_DEPTH> right;
	std::array<BlockType, Chunk::CHUNK_HEIGHT* Chunk::CHUNK_WIDTH> front;
	std::array<BlockType, Chunk::CHUNK_HEIGHT* Chunk::CHUNK_WIDTH> back;


	bool hasLeft = false;
	bool hasRight = false;
	bool hasFront = false;
	bool hasBack = false;

	unsigned int GetBoundaryBlock(int x, int y, int z, bool did_X_exceed) {
		//もしxが範囲外のものならzをつかう。z方向にはみ出してるならxを使う仕組みです。

		int index = 0;
		if (did_X_exceed) {
			index = IndexYZ(y, z);
		}
		else {
			index = IndexYX(y, x);
		}

		BlockType type = BlockType::AIR;
		if (did_X_exceed) {
			if (x < 0) {
				if (!hasLeft) return 0;
				type = left[index];
			}
			else if (x >= Chunk::CHUNK_WIDTH) {
				if (!hasRight) return 0;
				type = right[index];
			}
		}
		else {
			if (z < 0) {
				if (!hasBack) return 0;
				type = back[index];
			}
			else if (z >= Chunk::CHUNK_DEPTH) {
				if (!hasFront) return 0;
				type = front[index];
			}
		}


		return (unsigned int)type;
	}

	static int IndexYZ(int y, int z) {
		return z + Chunk::CHUNK_DEPTH * y;
	}

	static int IndexYX(int y, int x) {
		return x + Chunk::CHUNK_WIDTH * y;
	}

};


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
		auto it = chunks.find(key);
		if (it == chunks.end()) {
			if (!result.chunk) {
				std::cerr << "[ProcChunkResult] Couldnt find actual chunk data from worker thread\n";
			}
			chunks[key] = std::move(result.chunk);
		}


		if (result.meshData) {
			m_pendingMeshData.push_back({ std::move(*result.meshData), key });
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



ChunkMeshSnapshot World::CreateMeshSnapshot(Chunk& c) {
	ChunkMeshSnapshot snapshot(c);

	int32_t cx = c.cx;
	int32_t cz = c.cz;

	//left
	{
		uint64_t key = Index(cx - 1, cz);
		auto it = chunks.find(key);
		if (it != chunks.end() && it->second) {
			Chunk* c = it->second.get();

			snapshot.hasLeft = true;

			for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
				for (int z = 0; z < Chunk::CHUNK_DEPTH; ++z) {
					unsigned int b = c->GetBlock(Chunk::CHUNK_WIDTH - 1, y, z);

					snapshot.left[ChunkMeshSnapshot::IndexYZ(y, z)] =
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

			snapshot.hasRight = true;

			for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
				for (int z = 0; z < Chunk::CHUNK_DEPTH; ++z) {
					unsigned int b = c->GetBlock(0, y, z);

					snapshot.left[ChunkMeshSnapshot::IndexYZ(y, z)] =
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

			snapshot.hasFront = true;

			for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
				for (int x = 0; x < Chunk::CHUNK_WIDTH; ++x) {
					unsigned int b = c->GetBlock(x, y, 0);

					snapshot.left[ChunkMeshSnapshot::IndexYX(y, x)] =
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

			snapshot.hasBack = true;

			for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
				for (int x = 0; x < Chunk::CHUNK_WIDTH; ++x) {
					unsigned int b = c->GetBlock(x, y, Chunk::CHUNK_DEPTH - 1);

					snapshot.left[ChunkMeshSnapshot::IndexYX(y, x)] =
						static_cast<BlockType>(b);
				}
			}
		}
	}

	return snapshot;
}