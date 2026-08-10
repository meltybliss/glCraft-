#include "UI/WorldSelectionUI.h"

void WorldSelectionUI::Render() {

    ImGuiViewport* viewport =
        ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin("WorldSelection", nullptr, flags);

    if (ImGui::Button("New World")) {
        m_result.action =
            WorldSelectionAction::CreateNew;
    }

    if (ImGui::Button("Load World")) {
        m_result.action =
            WorldSelectionAction::LoadWorld;
    }

    ImGui::End();


}

