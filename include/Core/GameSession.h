#pragma once
#include "World/WorldThread.h"
#include "Util/RandomUtil.h"

#include "SnapshotExchanger.h"
#include "Persistence/PersistenceIO.h"

class GameSession {
public:

	GameSession(SnapshotExchanger& exchanger, PersistenceIO& pIO)
		: m_worldThread(exchanger, pIO) {}

	void CreateNewWorld();

	void LoadWorld(WorldSaveData& saveData);


	void Start(WorldSelectionResult& w_info);
	void Stop();


	WorldThread& GetWorldThread() {
		return m_worldThread;
	}

	const WorldThread& GetWorldThread() const {
		return m_worldThread;
	}

private:

	WorldThread m_worldThread;

};