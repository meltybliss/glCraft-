#include "Core/GameSession.h"


void GameSession::CreateNewWorld() {

	uint64_t seed = RandomUtil::CreateRandomSeed();

	m_worldThread.CreateNewWorld(seed);

}


void GameSession::LoadWorld(WorldSaveData& saveData) {

	m_worldThread.ApplyLoadedWorld(saveData);


}


void GameSession::Start(WorldSelectionResult& w_info) {

	m_worldThread.SetWorldSelectionResult(w_info);

	m_worldThread.StartThread();

}


void GameSession::Stop() {

	m_worldThread.StopThread();
}