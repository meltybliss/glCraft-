#pragma once
#include "Render/DayNightSnapshot.h"
#include <mutex>
#include <atomic>

//currently its compatible only with daynightSnapshot 
class SnapshotExchanger {
public:

	void publish(DayNightSnapshot& snapshot) {//from worldThread
		{
			std::scoped_lock<std::mutex> lock(snapShotMutex);

			m_snapshot = std::move(snapshot);
		}

		hasUpdatedSnapshot.store(true);
	}


	bool acquire(DayNightSnapshot& out) {//from worldRenderer
		if (!hasUpdatedSnapshot.load()) return false;

		{
			std::scoped_lock<std::mutex> lock(snapShotMutex);

			out = m_snapshot;
		}
		

		hasUpdatedSnapshot.store(false);
		return true;

	}


private:

	std::mutex snapShotMutex;
	std::atomic<bool> hasUpdatedSnapshot;


	DayNightSnapshot m_snapshot;

};