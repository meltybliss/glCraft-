#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "Render/Camera.h"
#include "World/World.h"
#include "Render/WorldRenderer.h"
#include "Render/Shader.h"
#include "Render/Texture.h"
#include <optional>
#include <memory>
#include "World/RaycastHit.h"

class Application {
public:
	Application() = default;
	

	bool InitGL();
	void Run();
private:

	void OnMouseMove(double xpos, double ypos);
	void ProcessInput(float dt);

	void UpdateRayHit();

private:
	GLFWwindow* m_window = nullptr;
	bool m_firstMouse = true;
	float m_lastMouseX = 400.0f;
	float m_lastMouseY = 300.0f;

	World m_world;
	WorldRenderer m_wRenderer;
	Camera m_camera;

	std::optional<Shader> baseShader;
	std::unique_ptr<Texture> blockAtlas;


	RaycastHit lastHit;
};