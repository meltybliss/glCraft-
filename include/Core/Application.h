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


class Application {
public:
	Application() = default;
	

	bool InitGL();
	void Run();
private:

	void OnMouseMove(double xpos, double ypos);
	void ProcessInput();
	void OnMouseButton(int button, int action);

	void UpdateRayHit();
	void RenderOutline();

	void UpdateStreamCenter();

	void ApplyCameraStatus();
private:
	GLFWwindow* m_window = nullptr;

	bool m_firstMouse = true;
	float m_lastMouseX = 400.0f;
	float m_lastMouseY = 300.0f;

	WorldThread m_worldThread;
	WorldRenderer m_wRenderer;
	SelectionOutlineRenderer m_outlineRenderer;

	Camera m_camera;



	RaycastHit lastHit;


	bool wasSpacePressed = false;
	double lastSpacePressTime = -1000.0;
	double spectateDoubleTime = 0.5;
};