#pragma once
#include "Snapshot/DayNightSnapshot.h"
#include "Snapshot/SnapshotChannel.h"
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


	void PublishLightVolumeSnap(std::unique_ptr<LightVolumeSnapshot> s) {
		if (s->fullUpdate) {
			// A full snapshot contains all state, so older queued deltas are obsolete.
			m_lightVolumeC.replace(std::move(s));
		}
		else {
			// Movement deltas depend on order and must not overwrite each other.
			m_lightVolumeC.publish(std::move(s));
		}
	}
	void ClearLightVolumeSnaps() {
		m_lightVolumeC.clear();
	}

	void PublishPointLightSnap(std::unique_ptr<PointLightsSnapshot> s) {
		m_pointLightC.publish(std::move(s));
	}

	void PublishPlrRenderSnap(PlayerRenderSnapshot s) {
		m_plrRenderC.publish(std::move(s));
	}

	//render
	std::optional<DayNightSnapshot>
	AcquireDayNightSnap() {
		return m_daynightC.acquire();
	}

	

	std::unique_ptr<LightVolumeSnapshot>
	AcquireLightVolumeSnap() {
		return m_lightVolumeC.acquire();
	}


	std::unique_ptr<PointLightsSnapshot>
	AcquirePointLightSnap() {
		return m_pointLightC.acquire();
	}

	std::optional<PlayerRenderSnapshot>
	AcquirePlrRenderSnap() {
		return m_plrRenderC.acquire();
	}

private:
	
	SnapshotChannel<DayNightSnapshot> m_daynightC;
	PtrSnapshotQueueChannel<LightVolumeSnapshot> m_lightVolumeC;

	PtrSnapshotChannel<PointLightsSnapshot> m_pointLightC;
	SnapshotChannel<PlayerRenderSnapshot> m_plrRenderC;
};
