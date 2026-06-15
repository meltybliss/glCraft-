#include "Core/Application.h"

void Application::Run() {
	
	float lastTime = (float)glfwGetTime();
	while (!glfwWindowShouldClose(m_window)) {
		float curTime = (float)glfwGetTime();
		float dt = curTime - lastTime;

		glfwPollEvents();

		m_world.Tick(dt, m_camera);


		m_wRenderer.RebuildDrityChunkMesh(m_world);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_wRenderer.RenderWorld(m_world, *baseShader, m_camera);


		glfwSwapBuffers(m_window);
	}

	glfwDestroyWindow(m_window);
	glfwTerminate();

}


bool Application::InitGL() {

	if (!glfwInit()) {
		return false;
	}

	m_window = glfwCreateWindow(800, 600, "glCraft++", nullptr, nullptr);

	if (!m_window) {
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(m_window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		glfwDestroyWindow(m_window);
		glfwTerminate();
		return false;
	}


	glEnable(GL_DEPTH_TEST);

	glViewport(0, 0, 800, 600);
	glClearColor(0.1f, 0.2f, 0.3f, 1.0f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//shader build
	baseShader.emplace(
		"assets/Shaders/basic.vert",
		"assets/Shaders/basic.frag"
	);
	

	return true;

}