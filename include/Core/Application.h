#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "Gameplay/PlayerInput.h"
#include "Gameplay/Player.h"
#include "Render/Camera.h"
#include "World/World.h"
#include "Render/WorldRenderer.h"
#include "Render/SelectionOutlineRenderer.h"
#include "Render/Shader.h"
#include "Render/Texture.h"
#include <optional>
#include <memory>
#include "World/RaycastHit.h"
#include "World/WorldThread.h"
#include "WindowSize.h"
#include "Core/SnapshotExchanger.h"

#include "Debugs/DebugDataBuilder.h"
#include "Debugs/DebugUI.h"
#include "Render/ImGuiRenderer.h"

class Application {
public:
	Application();
	

	bool InitGL();
	void Run();

	const int GetSelectedBlock() const {
		return selectedBlockId;//test
	}
private:

	void OnMouseMove(double xpos, double ypos);
	void ProcessInput();
	void OnMouseButton(int button, int action);

	void UpdateRayHit();
	void RenderOutline();

	void UpdateStreamCenter();

	void ApplyCameraStatus();

	void ApplyDebugActions(const DebugActions& actions);
private:
	GLFWwindow* m_window = nullptr;

	bool m_firstMouse = true;
	float m_lastMouseX = 400.0f;
	float m_lastMouseY = 300.0f;

	WorldThread m_worldThread;
	WorldRenderer m_wRenderer;
	SelectionOutlineRenderer m_outlineRenderer;
	SnapshotExchanger m_exchanger;

	Camera m_camera;



	RaycastHit lastHit;


	int selectedBlockId = 1;//‚¢‚¸‚êinventory‚ÉˆÚ‚·
	DebugDataBuilder m_debugBuilder;
	std::unique_ptr<DebugUI> m_debugUI;

	ImGuiRenderer m_imguiRenderer;

	bool wasSpacePressed = false;
	double lastSpacePressTime = -1000.0;
	double spectateDoubleTime = 0.5;
};