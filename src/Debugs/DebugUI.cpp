#include "Debugs/DebugUI.h"
#include <utility>



DebugActions DebugUI::Draw() {

	DebugActions actions;

	if (!m_opening) {
		return actions;
	}

	ImGui::Begin("Debug");

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