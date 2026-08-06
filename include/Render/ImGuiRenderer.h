#pragma once


#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"


class ImGuiRenderer {
public:

	void BeginFrame();
	void EndFrame();


};