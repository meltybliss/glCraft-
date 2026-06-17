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

}



void World::UpdateChunksAround(const Camera& cam) {
	int32_t curCx = static_cast<int32_t>(floorDiv(cam.position.x, Chunk::CHUNK_WIDTH));
	int32_t curCz = static_cast<int32_t>(floorDiv(cam.position.z, Chunk::CHUNK_DEPTH));

	int createBudget = MAX_CHUNK_CREATE_PER_TICK;
	int destroyBudget = MAX_CHUNK_DESTROY_PER_TICK;

	bool createDone = false;

	for (int32_t r = 0; r <= LOAD_CHUNKS_DISTANCE && !createDone; ++r) {//Ç±ÇÍì™Ç¢Ç¢é©ï™ÇÃé¸àÕÇ©ÇÁloadÇ∑ÇÈÉAÉãÉSÉäÉYÉÄÇæÇÌ
		for (int32_t dx = -r; dx <= r && !createDone; ++dx) {
			for (int32_t dz = -r; dz <= r; ++dz) {
				if (std::max(std::llabs(dx), std::llabs(dz)) != r) {//ì‡ë§ÇÕèàóùçœÇ›Ç»ÇÃÇ≈äOé¸ÇæÇØ
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


				std::unique_ptr<Chunk> c = std::make_unique<Chunk>(
					static_cast<int32_t>(cx), 
					static_cast<int32_t>(cz)
				);

				TerrainGenerator::GenerateTerrain(*c);
				chunks[key] = std::move(c);

				//é¸àÕè\éö4chunkÇÃmeshÇ‡DirtyÇ…ÇµÇƒê≥ÇµÇ≠ó◊Ç™AIRÇ©îªífÇ∑ÇÈ
				for (int32_t x = cx - 1; x <= cx + 1; ++x) {
					if (x == cx) continue;

					uint64_t key = Index(x, cz);
					auto it = chunks.find(key);
					if (it == chunks.end()) continue;

					it->second->dirty = true;
				}

				for (int32_t z = cz - 1; z <= cz + 1; ++z) {
					if (z == cz) continue;

					uint64_t key = Index(cx, z);
					auto it = chunks.find(key);
					if (it == chunks.end()) continue;

					it->second->dirty = true;
				}


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
			++it;//è¡ÇµÇΩÇ∆Ç´ÇÕóvëfÇ™é©ìÆÇ≈ãlÇﬂÇÁÇÍÇÈÇ©ÇÁè¡ÇµÇƒÇ»Ç¢Ç∆Ç´ÇæÇØitÇëùÇ‚ÇµÇƒéüÇÃóvëf
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
	bool ok = m_chunkPipeline.PopFrontResult(result);
	if (!ok) return;

	const auto& key = result.key;

	chunks[key] = std::move(result.chunk);

	if (result.meshData) {
		m_pendingMeshData.push_back({ std::move(*result.meshData), key });
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