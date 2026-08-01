#include "Core/Application.h"
#include <iostream>


void Application::UpdateStreamCenter() {

	int32_t centerCx = static_cast<int32_t>(
		floorDiv(m_camera.position.block.x, Chunk::CHUNK_WIDTH)
	);

	int32_t centerCz = static_cast<int32_t>(
		floorDiv(m_camera.position.block.z, Chunk::CHUNK_DEPTH)
	);


	m_worldThread.SetDesiredStreamCenter(centerCx, centerCz);
}

void Application::Run() {
	
	float lastTime = (float)glfwGetTime();

	float fpsTimer = 0.f;
	int frameCount = 0;


	m_worldThread.StartLoop();

	while (!glfwWindowShouldClose(m_window)) {
		float curTime = (float)glfwGetTime();
		float dt = curTime - lastTime;
		lastTime = curTime;

		glfwPollEvents();

		ProcessInput();


		ApplyCameraStatus();

		UpdateRayHit();//raycast
		UpdateStreamCenter();



		//m_wRenderer.RebuildDrityChunkMesh(m_world);
		m_wRenderer.UploadPendingMeshData(m_worldThread);

		m_worldThread.RequestCreateLightVSnap(m_camera);

		//äÆê¨çœÇ›SnapshotÇ™Ç†ÇÍÇŒéÛÇØéÊÇÈ
		std::unique_ptr<LightVolumeSnapshot> lightSnapshot;

		if (m_worldThread.PopCreatedLightVSnapshot(lightSnapshot)) {
			m_wRenderer.UpdateLightVolume(*lightSnapshot);//test
		}


		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		m_wRenderer.DeleteMeshes(m_worldThread);

		m_wRenderer.RenderShadowPass(m_camera);



		m_wRenderer.BeginHDRScene();

		m_wRenderer.RenderSky(m_camera);
		m_wRenderer.RenderWorld(m_camera, m_worldThread.GetWorldPtr());
		

		RenderOutline();//switch shader
		

		m_wRenderer.EndHDRScene();


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

	m_worldThread.StopLoop();

	glfwDestroyWindow(m_window);
	glfwTerminate();

}


bool Application::InitGL() {

	if (!glfwInit()) {
		return false;
	}

	m_window = glfwCreateWindow(WindowSize::windowWidth, WindowSize::windowHeight, "glCraft++", nullptr, nullptr);

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

	glViewport(0, 0, WindowSize::windowWidth, WindowSize::windowHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//Initialise shaders that require Init
	m_outlineRenderer.Init();

	
	m_wRenderer.InitBaseShader();

	m_wRenderer.InitSkyShaderAndVAO();
	m_wRenderer.InitBloom();
	m_wRenderer.InitShadownMap();
	m_wRenderer.InitHDRFrameBuffer();

	m_wRenderer.InitLightVolumeTexture();

	return true;

}



void Application::ProcessInput() {
	
	PlayerInput input;

	bool pressSpace = (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS);

	bool spacePressedThisFrame = pressSpace && !wasSpacePressed;

	wasSpacePressed = pressSpace;


	if (spacePressedThisFrame) {
		double now = glfwGetTime();

		if (now - lastSpacePressTime <= spectateDoubleTime) {
			input.toggleSpectator = true;
		
			lastSpacePressTime = -1000.0;
		}
		else {
			lastSpacePressTime = now;
		}

	}

	if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) {
	
	
		input.forward = true;
	}
	if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) {

	
		input.back = true;
	}
	if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) {
		
		
		input.left = true;
	}
	if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) {
	
		input.right = true;
	}
	if (pressSpace) {

		input.up = true;
	}
	if (glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		
		input.down = true;
	}


	if (glfwGetKey(m_window, GLFW_KEY_TAB) == GLFW_PRESS) {
		m_worldThread.Debug_CurStreamCenter();
	}




	m_worldThread.SetInput(std::move(input));


}


void Application::OnMouseButton(int button, int action) {
	if (action != GLFW_PRESS) {
		return;
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && lastHit.isHit) {

		m_worldThread.SubmitEditBlock(
			lastHit.hitX,
			lastHit.hitY,
			lastHit.hitZ,
			BlockType::AIR
		);

	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && lastHit.isHit) {

		m_worldThread.SubmitEditBlock(
			lastHit.previousX,
			lastHit.previousY,
			lastHit.previousZ,     
			BlockType::TORCH
		);
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

	m_worldThread.AddMouseDelta(xoffset, yoffset);
}



void Application::UpdateRayHit() {

	WorldPos origin = m_camera.position;
	glm::vec3 rayDir = glm::normalize(m_camera.front);

	float distance = 4.0f;

	lastHit = m_worldThread.RequestRaycast(origin, rayDir, distance);

}


void Application::RenderOutline() {
	if (lastHit.isHit) {

		m_outlineRenderer.RenderOutline(
			lastHit.hitX,
			lastHit.hitY,
			lastHit.hitZ,
			m_camera
		);

	}

}


void Application::ApplyCameraStatus() {

	PlayerSnapshot snap = m_worldThread.GetPlrSnapshot();

	m_camera.position = snap.pos;
	m_camera.front = snap.front;
	m_camera.right = snap.right;
	m_camera.up = snap.up;


}