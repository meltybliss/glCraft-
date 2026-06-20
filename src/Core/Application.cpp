#include "Core/Application.h"

void Application::Run() {
	
	float lastTime = (float)glfwGetTime();

	float fpsTimer = 0.f;
	int frameCount = 0;

	while (!glfwWindowShouldClose(m_window)) {
		float curTime = (float)glfwGetTime();
		float dt = curTime - lastTime;
		lastTime = curTime;

		glfwPollEvents();

		ProcessInput(dt);



		m_world.Tick(dt, m_camera);

		blockAtlas->Bind(0);

		m_wRenderer.RebuildDrityChunkMesh(m_world);
		m_wRenderer.UploadPendingMeshData(m_world);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_wRenderer.RenderWorld(m_world, *baseShader, m_camera);


		glfwSwapBuffers(m_window);

		//display fps
		frameCount++;
		fpsTimer += dt;

		if (fpsTimer >= 0.5f) {
			float fps = static_cast<float>(frameCount) / fpsTimer;

			std::string title =
				"glCraft++ | FPS: " + std::to_string(static_cast<int>(fps));

			glfwSetWindowTitle(m_window, title.c_str());

			frameCount = 0;
			fpsTimer = 0.f;
		}

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


	glfwSetWindowUserPointer(m_window, this);
	glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xpos, double ypos) {//register callBack
		auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
		if (app) {
			app->OnMouseMove(xpos, ypos);
		}
	});

	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glEnable(GL_DEPTH_TEST);

	glViewport(0, 0, 800, 600);
	glClearColor(0.1f, 0.2f, 0.3f, 1.0f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	blockAtlas = std::make_unique<Texture>("assets/textures/block_atlas.png");

	//shader build
	baseShader.emplace(
		"assets/Shaders/basic.vert",
		"assets/Shaders/basic.frag"
	);

	baseShader->Use();
	baseShader->SetInt("u_Texture", 0);

	

	return true;

}



void Application::ProcessInput(float dt) {
	float velocity = m_camera.moveSpeed * dt;

	if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) {
		m_camera.position += m_camera.front * velocity;
	}
	if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) {
		m_camera.position -= m_camera.front * velocity;
	}
	if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) {
		m_camera.position -= m_camera.right * velocity;
	}
	if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) {
		m_camera.position += m_camera.right * velocity;
	}
	if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		m_camera.position += m_camera.worldUp * velocity;
	}
	if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		m_camera.position -= m_camera.worldUp * velocity;
	}


	if (glfwGetKey(m_window, GLFW_KEY_TAB) == GLFW_PRESS) {
		m_world.DebugChunkInfo();
	}
}


void Application::OnMouseMove(double xpos, double ypos) {
	if (m_firstMouse) {
		m_lastMouseX = static_cast<float>(xpos);
		m_lastMouseY = static_cast<float>(ypos);
		m_firstMouse = false;
	}

	float xoffset = static_cast<float>(xpos) - m_lastMouseX;
	float yoffset = m_lastMouseY - static_cast<float>(ypos);

	m_lastMouseX = static_cast<float>(xpos);
	m_lastMouseY = static_cast<float>(ypos);

	xoffset *= m_camera.mouseSensitivity;
	yoffset *= m_camera.mouseSensitivity;

	m_camera.yaw += xoffset;
	m_camera.pitch += yoffset;

	if (m_camera.pitch > 89.0f) {
		m_camera.pitch = 89.0f;
	}

	if (m_camera.pitch < -89.0f) {
		m_camera.pitch = -89.0f;
	}

	m_camera.UpdateVectors();
}



void Application::UpdateRayHit() {

	glm::vec3 origin = m_camera.position;
	glm::vec3 rayDir = m_camera.front;

	float distance = 4.0f;

	lastHit = m_world.Raycast(origin, rayDir, distance);

}