#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "World/WorldSelectionResult.h"


class WorldSelectionUI {
public:

    void Render();

    WorldSelectionResult Get_SelectionResult() const {
        return m_result;
    }
private:

    WorldSelectionResult m_result;
};