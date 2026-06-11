#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"

class Application {
public:
	Application() = default;
	

	bool InitGL();
	void Run();
private:


private:
	GLFWwindow* m_window = nullptr;
};