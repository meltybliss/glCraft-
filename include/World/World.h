#pragma once
#include "World/Chunk.h"
#include "World/ChunkUtil.h"
#include "World/ChunkResult.h"
#include "Render/PendingMesh.h"
#include "World/RaycastHit.h"
#include "Snapshot/ChunkMeshSnapshot.h"
#include "Snapshot/LightVolumeSnapshot.h"
#include "Util/ThreadSafeLogUtils.h"
#include "Gameplay/DayNightState.h"
#include "Debugs/DebugActions.h"

#include "WorldSelectionResult.h"

#include <unordered_map>
#include <memory>
#include <unordered_set>
#include <stdint.h>
#include <deque>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using ChunkMapKey = ChunkCoord;
using namespace ChunkUtil;

struct Camera;
class ChunkPipeline;


struct SetBlockResult {

	bool blockChanged = false;
	bool pointLightChanged = false;


};


class World {
public:

	World() = default;

	using ChunkMap = std::unordered_map<ChunkMapKey, std::unique_ptr<Chunk>, ChunkCoordHash>;

	[[nodiscard]] unsigned int GetBlockGlobal(int64_t x, int64_t y, int64_t z) const;
	[[nodiscard]] uint8_t GetBlockLightGlobal(int64_t x, int64_t y, int64_t z) const;
	[[nodiscard]] uint8_t GetSkyLightGlobal(int64_t x, int64_t y, int64_t z) const;
	[[nodiscard]] glm::vec3 GetBlockLightColorGlobal(int64_t x, int64_t y, int64_t z) const;


	SetBlockResult SetBlockGlobal(int64_t x, int64_t y, int64_t z, BlockType b);
	SetBlockResult SetBlockGlobal_User(int64_t x, int64_t y, int64_t z, BlockType b);
	bool SetBlockLightGlobal(int64_t x, int64_t y, int64_t z, uint8_t level, const glm::vec3& lightColor);
	bool SetSkyLightGlobal(int64_t x, int64_t y, int64_t z, uint8_t level);

	[[nodiscard]] ChunkMap& GetChunks() {
		return chunks;
	}

	[[nodiscard]] const ChunkMap& GetChunks() const {
		return chunks;
	}

	[[nodiscard]] Chunk* GetTargetChunk(ChunkCoord coord) {
		auto it = chunks.find(coord);

		if (it == chunks.end()) return nullptr;

		return it->second.get();
	}

	[[nodiscard]] Chunk* GetTargetChunkFromKey(ChunkCoord key) {

		auto it = chunks.find(key);

		if (it == chunks.end()) return nullptr;

		return it->second.get();
	}

	[[nodiscard]] const Chunk* GetTargetChunkFromKey(ChunkCoord key) const {

		auto it = chunks.find(key);

		if (it == chunks.end()) return nullptr;

		return it->second.get();
	}

	[[nodiscard]] const Chunk* GetTargetChunk(ChunkCoord coord) const {
		auto it = chunks.find(coord);

		if (it == chunks.end()) return nullptr;

		return it->second.get();
	}

	[[nodiscard]] RaycastHit Raycast(const WorldPos& origin, const glm::vec3& direction, float distance) const;

	void DebugChunkInfo();


	std::unique_ptr<ChunkMeshSnapshot> CreateMeshSnapshot(Chunk& c);

	std::unique_ptr<ChunkMeshSnapshot> CreateMeshSnapshotFromKey(ChunkCoord key);

	std::unique_ptr<LightVolumeSnapshot> CreateLightVSnapshot(const glm::i64vec3 camBlockPos) const;


	bool CanCollideBlock(int64_t x, int64_t y, int64_t z) const {
		BlockType block = static_cast<BlockType>(
			GetBlockGlobal(x, y, z)
			);


		return block != BlockType::AIR && block != BlockType::TORCH;
	}


	void SelectOptimalPointLights(ChunkCoord coord, std::array<PointLight*, 16>& out, size_t& count);


	void SetWorldTime(double time) {
		m_dayNight.timeOfDay = time;
	}

	DayNightState& GetDayNightState() { return m_dayNight; }
	const DayNightState& GetDayNightState() const { return m_dayNight; }

	DayNightState const GetDayNightStateForDebug();
	void SetDebugStateFromDebug(const DebugActions& actions);

	void SetWorldSeed(uint64_t seed);
	[[nodiscard]] uint64_t GetWorldSeed() const { return m_seed; }


	void SetWorldSelectionResult(WorldSelectionResult& info) { m_worldInfo = info; }
	[[nodiscard]] WorldSelectionResult GetWorldInfo() const { return m_worldInfo; }
private:
private:

	ChunkMap chunks;
	DayNightState m_dayNight;

	uint64_t m_seed = 114514;

	std::mutex dayNightStateMutex;

	WorldSelectionResult m_worldInfo;
};
