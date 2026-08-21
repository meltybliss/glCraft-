#pragma once

#include <cstddef>
#include <cstdint>



struct DebugSettings {

	int selectedBlockId = -1;

	bool timePaused = false;
	double timeOfDay = 0.5;
	double dayLengthSeconds = 30.0;
	double timeScale = 1.0;

};


struct DebugPerformanceStats {
	float mainFps = 0.0f;
	float mainFrameTimeMs = 0.0f;

	float worldThreadUtilization = 0.0f;
	uint64_t worldIterationsPerSecond = 0;

	int workerCount = 0;
	int activeWorkers = 0;
	float workerUtilization = 0.0f;
	float workerTasksPerSecond = 0.0f;
	float averageWorkerTaskMs = 0.0f;

	size_t queuedWorkerTasks = 0;
	size_t queuedCreateTasks = 0;
	size_t queuedTerrainTasks = 0;
	size_t queuedMeshTasks = 0;
	size_t readyGenerateResults = 0;
	size_t readyMeshResults = 0;

	size_t loadedChunks = 0;
	size_t pendingChunkLoads = 0;
	size_t normalLightTasks = 0;
	size_t urgentLightTasks = 0;
	size_t dirtyMeshTasks = 0;
	size_t pendingMeshUploads = 0;
};



struct ChunkPipelineDebugStats {
	int workerCount = 0;
	int activeWorkers = 0;

	size_t queuedTasks = 0;
	size_t queuedCreateTasks = 0;
	size_t queuedTerrainTasks = 0;
	size_t queuedMeshTasks = 0;
	size_t readyGenerateResults = 0;
	size_t readyMeshResults = 0;

	uint64_t completedTasks = 0;
	uint64_t busyTimeNs = 0;
};


struct WorldThreadDebugStats {
	ChunkPipelineDebugStats pipeline;

	uint64_t busyTimeNs = 0;
	uint64_t loopIterations = 0;

	size_t loadedChunks = 0;
	size_t pendingChunkLoads = 0;
	size_t normalLightTasks = 0;
	size_t urgentLightTasks = 0;
	size_t dirtyMeshTasks = 0;
	size_t pendingMeshUploads = 0;
};
