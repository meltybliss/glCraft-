#include "Core/Application.h"
#include <iostream>


void Application::UpdateStreamCenter() {

	int32_t centerCx = static_cast<int32_t>(
		floorDiv(m_camera.position.x, Chunk::CHUNK_WIDTH)
	);

	int32_t centerCz = static_cast<int32_t>(
		floorDiv(m_camera.position.z, Chunk::CHUNK_DEPTH)
	);


	m_worldThread.SetDesiredStreamCenter(centerCx, centerCz);
}

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

		UpdateRayHit();//raycast
		UpdateStreamCenter();

		m_world.Tick(dt, m_camera);

		blockAtlas->Bind(0);

		m_wRenderer.RebuildDrityChunkMesh(m_world);
		m_wRenderer.UploadPendingMeshData(m_world);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		baseShader->Use();
		m_wRenderer.RenderWorld(m_world, *baseShader, m_camera);

		RenderOutline();//switch shader


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

	glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods) {
		auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
		if (app) {
			app->OnMouseButton(button, action);
		}

	});


	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glEnable(GL_DEPTH_TEST);

	glViewport(0, 0, 800, 600);
	glClearColor(0.1f, 0.2f, 0.3f, 1.0f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	blockAtlas = std::make_unique<Texture>("assets/textures/block_atlas.png");
	

	//Initialise shaders that require Init
	m_outlineRenderer.Init();


	//shader build
	baseShader.emplace(
		"assets/Shaders/basic.vert",
		"assets/Shaders/basic.frag"
	);

	selectionOutlineShader.emplace(
		"assets/Shaders/selectionOutline.vert",
		"assets/Shaders/selectionOutline.frag"
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


void Application::OnMouseButton(int button, int action) {
	if (action != GLFW_PRESS) {
		return;
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && lastHit.isHit) {
		m_world.SetBlockGlobal_User(lastHit.hitX, lastHit.hitY, lastHit.hitZ, BlockType::AIR);
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && lastHit.isHit) {
		m_world.SetBlockGlobal_User(lastHit.previousX, lastHit.previousY, lastHit.previousZ, BlockType::STONE);
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
	glm::vec3 rayDir = glm::normalize(m_camera.front);

	float distance = 4.0f;

	lastHit = m_world.Raycast(origin, rayDir, distance);

}


void Application::RenderOutline() {
	if (lastHit.isHit) {
		selectionOutlineShader->Use();

		m_outlineRenderer.RenderOutline(
			lastHit.hitX,
			lastHit.hitY,
			lastHit.hitZ,
			m_camera,
			*selectionOutlineShader
		);

	}

}