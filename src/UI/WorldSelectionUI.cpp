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


   
    const ImVec2 windowSize =
        ImGui::GetContentRegionAvail();

    const float buttonWidth = 320.0f;
    const float buttonHeight = 70.0f;
    const float buttonSpacing = 20.0f;

    const char* title = "glVoxel++";


    const float titleHeight = 50.0f;

    const float totalHeight =
        titleHeight +
        buttonHeight * 2.0f +
        buttonSpacing;


   

    float startY =
        (windowSize.y - totalHeight) * 0.5f;

    if (startY < 0.0f)
        startY = 0.0f;

    ImGui::SetCursorPosY(startY);


   

    ImVec2 titleSize =
        ImGui::CalcTextSize(title);

    ImGui::SetCursorPosX(
        (windowSize.x - titleSize.x) * 0.5f
    );

    ImGui::TextUnformatted(title);

    ImGui::Dummy(ImVec2(0.0f, 25.0f));



    ImGui::SetCursorPosX(
        (windowSize.x - buttonWidth) * 0.5f
    );

    if (ImGui::Button(
        "New World",
        ImVec2(buttonWidth, buttonHeight)
    )) {
        m_result.action =
            WorldSelectionAction::CreateNew;
    }


    ImGui::Dummy(
        ImVec2(0.0f, buttonSpacing)
    );


    

    ImGui::SetCursorPosX(
        (windowSize.x - buttonWidth) * 0.5f
    );

    if (ImGui::Button(
        "Load World",
        ImVec2(buttonWidth, buttonHeight)
    )) {
        m_result.action =
            WorldSelectionAction::LoadWorld;
    }


    ImGui::End();


}

