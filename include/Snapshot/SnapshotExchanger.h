#pragma once
#include "Snapshot/DayNightSnapshot.h"
#include "Snapshot/SnapshotChannel.h"
#include "Snapshot/ChunkMeshSnapshot.h"
#include "Snapshot/LightVolumeSnapshot.h"
#include "Snapshot/PlayerSnapshot.h"


#include <mutex>
#include <atomic>

class SnapshotExchanger {
public:
	//worldThread
	void PublishDaynightSnap(DayNightSnapshot s) {
		m_daynightC.publish(std::move(s));
	}

	void PublishChunkMeshSnap(ChunkMeshSnapshot s) {
		m_chunkMeshC.publish(std::move(s));
	}

	void PublishLightVolumeSnap(std::unique_ptr<LightVolumeSnapshot> s) {
		m_lightVolumeC.publish(std::move(s));
	}

	void PublishPointLightSnap(std::unique_ptr<PointLightsSnapshot> s) {
		m_pointLightC.publish(std::move(s));
	}

	void PublishPlrRenderSnap(PlayerRenderSnapshot s) {
		m_plrRenderC.publish(std::move(s));
	}

	//render
	bool AcquireDayNightSnap(DayNightSnapshot& out) {
		return m_daynightC.acquire(out);
	}

	bool AcquireChunkMeshSnap(ChunkMeshSnapshot& out) {
		return m_chunkMeshC.acquire(out);
	}

	bool AcquireLightVolumeSnap(std::unique_ptr<LightVolumeSnapshot>& out) {
		return m_lightVolumeC.acquire(out);
	}


	bool AcquirePointLightSnap(std::unique_ptr<PointLightsSnapshot>& out) {
		return m_pointLightC.acquire(out);
	}

	bool AcquirePlrRenderSnap(PlayerRenderSnapshot& out) {
		return m_plrRenderC.acquire(out);
	}

private:
	
	SnapshotChannel<DayNightSnapshot> m_daynightC;
	SnapshotChannel<ChunkMeshSnapshot> m_chunkMeshC;
	SnapshotChannel<std::unique_ptr<LightVolumeSnapshot>> m_lightVolumeC;
	SnapshotChannel<std::unique_ptr<PointLightsSnapshot>> m_pointLightC;
	SnapshotChannel<PlayerRenderSnapshot> m_plrRenderC;
};