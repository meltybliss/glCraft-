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
#include <optional>
#include <memory>
#include "World/RaycastHit.h"
#include "World/WorldThread.h"
#include "WindowSize.h"
#include "Snapshot/SnapshotExchanger.h"

#include "Debugs/DebugDataBuilder.h"
#include "Debugs/DebugUI.h"
#include "Render/ImGuiRenderer.h"
#include "Persistence/PersistenceIO.h"

#include "UI/WorldSelectionUI.h"

#include "AppState.h"
#include "GameSession.h"



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

	void CreateWorldDebugUI();

	float CalcInterpolationAlpha(const PlayerSnapshot& previous, const PlayerSnapshot& current) const;

	
	void UpdateSnapshots();

	void EnterWorld();

	void Tick_main();
	void Tick_playing();

	
	void Render_main();
	void Render_playing();
private:
	GLFWwindow* m_window = nullptr;
	AppState m_state = AppState::WORLD_SELECTION;

	bool m_firstMouse = true;
	float m_lastMouseX = 400.0f;
	float m_lastMouseY = 300.0f;


	GameSession m_session;
	WorldSelectionUI m_selectionUI;

	WorldRenderer m_wRenderer;
	SelectionOutlineRenderer m_outlineRenderer;
	SnapshotExchanger m_exchanger;
	PersistenceIO m_persistenceIO;

	Camera m_camera;

	//snapshots
	std::optional<PlayerRenderSnapshot> m_plrRenderSnap;
	//

	RaycastHit lastHit;


	int selectedBlockId = 3;//‚¢‚¸‚êinventory‚ÉˆÚ‚·
	DebugDataBuilder m_debugBuilder;
	std::unique_ptr<DebugUI> m_debugUI;

	ImGuiRenderer m_imguiRenderer;

	bool wasSpacePressed = false;
	bool wasTabPressed = false;
	bool wasAltPressed = false;

	double lastSpacePressTime = -1000.0;
	double spectateDoubleTime = 0.5;
	
};