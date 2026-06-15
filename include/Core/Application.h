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

class Application {
public:
	Application() = default;
	

	bool InitGL();
	void Run();
private:


private:
	GLFWwindow* m_window = nullptr;

	World m_world;
	WorldRenderer m_wRenderer;
	Camera m_camera;

	std::optional<Shader> baseShader;
	std::unique_ptr<Texture> blockAtlas;

};