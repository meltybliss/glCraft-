#include "Debugs/DebugUI.h"
#include <algorithm>
#include <utility>



DebugActions DebugUI::Draw() {

	DebugActions actions;

	if (!m_opening) {
		return actions;
	}

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	const float availableWidth = std::max(
		320.0f,
		viewport->WorkSize.x - 24.0f
	);
	const float availableHeight = std::max(
		360.0f,
		viewport->WorkSize.y - 24.0f
	);

	const ImVec2 desiredSize(
		std::min(680.0f, availableWidth),
		std::min(900.0f, availableHeight)
	);
	const ImVec2 minimumSize(
		std::min(560.0f, availableWidth),
		std::min(640.0f, availableHeight)
	);

	ImGui::SetNextWindowSize(desiredSize, ImGuiCond_Once);
	ImGui::SetNextWindowSizeConstraints(
		minimumSize,
		ImVec2(availableWidth, availableHeight)
	);

	ImGui::Begin("Debug");

	if (ImGui::CollapsingHeader(
		"Performance",
		ImGuiTreeNodeFlags_DefaultOpen)) {

		ImGui::Text("Main thread");
		ImGui::Text("FPS: %.1f", m_performanceStats.mainFps);
		ImGui::Text(
			"Frame time: %.2f ms",
			m_performanceStats.mainFrameTimeMs
		);

		ImGui::Separator();
		ImGui::Text("World thread");
		ImGui::Text(
			"Utilization: %.1f%%",
			m_performanceStats.worldThreadUtilization
		);
		ImGui::ProgressBar(
			std::clamp(
				m_performanceStats.worldThreadUtilization / 100.0f,
				0.0f,
				1.0f
			),
			ImVec2(-1.0f, 0.0f)
		);
		ImGui::Text(
			"Loop rate: %llu / sec",
			static_cast<unsigned long long>(
				m_performanceStats.worldIterationsPerSecond
			)
		);

		ImGui::Separator();
		ImGui::Text("Chunk workers");
		ImGui::Text(
			"Active: %d / %d",
			m_performanceStats.activeWorkers,
			m_performanceStats.workerCount
		);
		ImGui::Text(
			"Utilization: %.1f%%",
			m_performanceStats.workerUtilization
		);
		ImGui::ProgressBar(
			std::clamp(
				m_performanceStats.workerUtilization / 100.0f,
				0.0f,
				1.0f
			),
			ImVec2(-1.0f, 0.0f)
		);
		ImGui::Text(
			"Throughput: %.1f tasks / sec",
			m_performanceStats.workerTasksPerSecond
		);
		ImGui::Text(
			"Average task: %.2f ms",
			m_performanceStats.averageWorkerTaskMs
		);

		const char* workerState = "Caught up";
		if (m_performanceStats.queuedWorkerTasks > 0) {
			workerState =
				m_performanceStats.workerUtilization >= 85.0f ?
				"Saturated" : "Processing";
		}
		ImGui::Text("State: %s", workerState);

		ImGui::Separator();
		ImGui::Text(
			"Worker queue: %zu  (create %zu, terrain %zu, mesh %zu)",
			m_performanceStats.queuedWorkerTasks,
			m_performanceStats.queuedCreateTasks,
			m_performanceStats.queuedTerrainTasks,
			m_performanceStats.queuedMeshTasks
		);
		ImGui::Text(
			"Ready results: terrain %zu, mesh %zu",
			m_performanceStats.readyGenerateResults,
			m_performanceStats.readyMeshResults
		);
		ImGui::Text(
			"Light tasks: %zu normal, %zu urgent",
			m_performanceStats.normalLightTasks,
			m_performanceStats.urgentLightTasks
		);
		ImGui::Text(
			"Mesh tasks: %zu dirty, %zu awaiting upload",
			m_performanceStats.dirtyMeshTasks,
			m_performanceStats.pendingMeshUploads
		);
		ImGui::Text(
			"Chunks: %zu loaded, %zu pending",
			m_performanceStats.loadedChunks,
			m_performanceStats.pendingChunkLoads
		);
	}

	ImGui::Separator();

	ImGui::Text("Block");

	if (ImGui::InputInt("Selected Block Id", &m_settings.selectedBlockId)) {

		m_settings.selectedBlockId = std::max(m_settings.selectedBlockId, 0);

		actions.selectedBlockId = m_settings.selectedBlockId;
	}


	ImGui::Separator();

	ImGui::Text("Day / Night");

	if (ImGui::Checkbox("Pause Time", &m_settings.timePaused)) {

		actions.timePaused = m_settings.timePaused;

	}

	constexpr double minTime = 0.0;
	constexpr double maxTime = 1.0;

	if (ImGui::SliderScalar(
		"Time Of Day",
		ImGuiDataType_Double,
		&m_settings.timeOfDay,
		&minTime,
		&maxTime,
		"%.3f")) {


		actions.timeOfDay = m_settings.timeOfDay;


	}


	constexpr double minLength = 1.0;
	constexpr double maxLength = 3600.0;

	if (ImGui::SliderScalar(
		"DayLength Seconds",
		ImGuiDataType_Double,
		&m_settings.dayLengthSeconds,
		&minLength,
		&maxLength,
		"%.1f seconds")) {


		actions.dayLengthSeconds = m_settings.dayLengthSeconds;

	}


	constexpr double minScale = 0.1;
	constexpr double maxScale = 10.0;

	if (ImGui::SliderScalar(
		"TimeScale",
		ImGuiDataType_Double,
		&m_settings.timeScale,
		&minScale,
		&maxScale,
		"%.1fx"

	)) {

		actions.timeScale = m_settings.timeScale;

	}

	
	if (ImGui::Button("Midnight")) {
		m_settings.timeOfDay = 0.0;
		actions.timeOfDay = 0.0;
	}


	ImGui::End();


	return actions;

}


void DebugUI::ReceiveSettings(DebugSettings&& settings) {


	m_settings = std::move(settings);

}


void DebugUI::ReceivePerformanceStats(
	const DebugPerformanceStats& stats
) {
	m_performanceStats = stats;
}
