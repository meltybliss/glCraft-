#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"


#include "DebugSettings.h"
#include "DebugActions.h"

class DebugUI {
public:
	DebugUI() = default;
	DebugUI(const DebugUI& ui) = delete;
	DebugUI& operator=(const DebugUI& ui) = delete;
	
	DebugActions Draw();

	void ReceiveSettings(DebugSettings&& settings);
	void ReceivePerformanceStats(const DebugPerformanceStats& stats);
	bool GetIsOpening() const { return m_opening; }
	void SetIsOpening(bool value) { m_opening = value; }
private:

	DebugSettings m_settings;
	DebugPerformanceStats m_performanceStats;
	bool m_opening = false;
};
