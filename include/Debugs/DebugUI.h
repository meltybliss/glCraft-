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

	explicit DebugUI(GLFWwindow* window);
	~DebugUI();

	DebugUI(const DebugUI& ui) = delete;
	DebugUI& operator=(const DebugUI& ui) = delete;
	
	DebugActions Draw();

	void ReceiveSettings(DebugSettings&& settings);
	bool GetIsOpening() const { return m_opening; }
	void SetIsOpening(bool value) { m_opening = value; }
private:

	GLFWwindow* m_window;

	DebugSettings m_settings;
	bool m_opening = false;
};