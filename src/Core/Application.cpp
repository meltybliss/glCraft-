#include "Core/Application.h"
#include "Math/WorldPosMath.h"
#include <algorithm>
#include <iostream>
#include <cmath>

Application::Application() : m_exchanger(), m_session(m_exchanger, m_persistenceIO), m_wRenderer(m_exchanger) {}




void Application::UpdateStreamCenter() {

	int64_t centerCx =
		floorDiv(m_camera.position.block.x, Chunk::CHUNK_WIDTH)
	;

	int64_t centerCz =
		floorDiv(m_camera.position.block.z, Chunk::CHUNK_DEPTH)
	;


	m_session.GetWorldThread().SetDesiredStreamCenter({centerCx, centerCz});

	
}


void Application::ApplyDebugActions(const DebugActions& actions) {

	if (actions.selectedBlockId) {
		selectedBlockId = actions.selectedBlockId.value();
	}

	
	m_session.GetWorldThread().SetDebugStateFromDebug(actions);

}


void Application::Run() {
	
	float lastTime = (float)glfwGetTime();

	float fpsTimer = 0.f;
	int frameCount = 0;


	m_persistenceIO.StartThread();

	while (!glfwWindowShouldClose(m_window)) {
		float curTime = (float)glfwGetTime();
		float dt = curTime - lastTime;
		lastTime = curTime;

		glfwPollEvents();

		Tick_main();

	
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		Render_main();
	

		glfwSwapBuffers(m_window);

		//display fps
		frameCount++;
		fpsTimer += dt;

		if (fpsTimer >= 0.5f) {
			float fps = static_cast<float>(frameCount) / fpsTimer;

			std::string title =
				"glCraft++ | FPS: " + std::to_string(static_cast<int>(fps));

			glfwSetWindowTitle(m_window, title.c_str());

			if (m_debugUI && m_state == AppState::PLAY) {
				static uint64_t lastWorkerCompletedTasks = 0;
				static uint64_t lastWorkerBusyTimeNs = 0;
				static uint64_t lastWorldBusyTimeNs = 0;
				static uint64_t lastWorldLoopIterations = 0;

				const WorldThreadDebugStats worldStats =
					m_session.GetWorldThread().GetDebugStats();
				const auto& pipelineStats = worldStats.pipeline;

				const uint64_t completedTaskDelta =
					pipelineStats.completedTasks - lastWorkerCompletedTasks;
				const uint64_t workerBusyDelta =
					pipelineStats.busyTimeNs - lastWorkerBusyTimeNs;
				const uint64_t worldBusyDelta =
					worldStats.busyTimeNs - lastWorldBusyTimeNs;
				const uint64_t worldLoopDelta =
					worldStats.loopIterations - lastWorldLoopIterations;

				DebugPerformanceStats performance;
				performance.mainFps = fps;
				performance.mainFrameTimeMs =
					fps > 0.0f ? 1000.0f / fps : 0.0f;

				const double sampleNs =
					static_cast<double>(fpsTimer) * 1'000'000'000.0;

				performance.worldThreadUtilization = static_cast<float>(
					std::clamp(
						static_cast<double>(worldBusyDelta) / sampleNs * 100.0,
						0.0,
						100.0
					)
				);
				performance.worldIterationsPerSecond = static_cast<uint64_t>(
					static_cast<double>(worldLoopDelta) / fpsTimer
				);

				performance.workerCount = pipelineStats.workerCount;
				performance.activeWorkers = pipelineStats.activeWorkers;

				if (pipelineStats.workerCount > 0) {
					const double workerCapacityNs =
						sampleNs * pipelineStats.workerCount;
					performance.workerUtilization = static_cast<float>(
						std::clamp(
							static_cast<double>(workerBusyDelta) /
								workerCapacityNs * 100.0,
							0.0,
							100.0
						)
					);
				}

				performance.workerTasksPerSecond =
					static_cast<float>(completedTaskDelta) / fpsTimer;
				performance.averageWorkerTaskMs = completedTaskDelta > 0 ?
					static_cast<float>(
						static_cast<double>(workerBusyDelta) /
						completedTaskDelta /
						1'000'000.0
					) : 0.0f;

				performance.queuedWorkerTasks = pipelineStats.queuedTasks;
				performance.queuedCreateTasks = pipelineStats.queuedCreateTasks;
				performance.queuedTerrainTasks = pipelineStats.queuedTerrainTasks;
				performance.queuedMeshTasks = pipelineStats.queuedMeshTasks;
				performance.readyGenerateResults =
					pipelineStats.readyGenerateResults;
				performance.readyMeshResults = pipelineStats.readyMeshResults;

				performance.loadedChunks = worldStats.loadedChunks;
				performance.pendingChunkLoads = worldStats.pendingChunkLoads;
				performance.normalLightTasks = worldStats.normalLightTasks;
				performance.urgentLightTasks = worldStats.urgentLightTasks;
				performance.dirtyMeshTasks = worldStats.dirtyMeshTasks;
				performance.pendingMeshUploads = worldStats.pendingMeshUploads;

				m_debugUI->ReceivePerformanceStats(performance);

				lastWorkerCompletedTasks = pipelineStats.completedTasks;
				lastWorkerBusyTimeNs = pipelineStats.busyTimeNs;
				lastWorldBusyTimeNs = worldStats.busyTimeNs;
				lastWorldLoopIterations = worldStats.loopIterations;
			}

			frameCount = 0;
			fpsTimer = 0.f;
		}

	}

	m_session.Stop();

	m_imguiRenderer.Shutdown();


	glfwDestroyWindow(m_window);
	glfwTerminate();

}


void Application::EnterWorld() {

	glfwSetInputMode(
		m_window,
		GLFW_CURSOR,
		GLFW_CURSOR_DISABLED
	);

	m_firstMouse = true;



	m_state = AppState::PLAY;
}


void Application::Tick_main() {

	switch (m_state) {
		case AppState::TITLE: break;

		case AppState::WORLD_SELECTION: {

			//receive the action from the ui lets the user choose between LOAD the exsisting world and CREATE new world
			auto worldInfo =
				m_selectionUI.Get_SelectionResult();


			if (worldInfo.action == WorldSelectionAction::LoadWorld) {

				std::optional<WorldSaveData> data = m_persistenceIO.LoadWorld();

				if (data) {


					m_session.LoadWorld(*data);


					CreateWorldDebugUI();


					EnterWorld();
					m_session.Start(worldInfo);//start worldThread

				}
				else {
					std::cerr << "failed to load the world\n";

					//create new world

					m_session.CreateNewWorld();
				
					CreateWorldDebugUI();
					
					EnterWorld();
					m_session.Start(worldInfo);

				}				

			}
			else if (worldInfo.action == WorldSelectionAction::CreateNew) {

				m_session.CreateNewWorld();

				CreateWorldDebugUI();
				

				EnterWorld();
				m_session.Start(worldInfo);

			}


			break;
		}

		case AppState::PLAY: {

			Tick_playing();

			break;
		}


	}


}


void Application::Render_main() {


	m_imguiRenderer.BeginFrame();


	switch (m_state) {
		case AppState::TITLE: break;

		case AppState::WORLD_SELECTION: {

			//render the ui lets the user choose between LOAD the exsisting world and CREATE new world

			m_selectionUI.Render();

			break;
		}

		case AppState::PLAY: {

			Render_playing();

			break;

		}
	}


	m_imguiRenderer.EndFrame();

}



void Application::Tick_playing() {

	auto& worldThread = m_session.GetWorldThread();

	ProcessInput();

	UpdateSnapshots();

	ApplyCameraStatus();

	UpdateRayHit();//raycast
	UpdateStreamCenter();

	

	worldThread.SetLightVolumeCenter(m_camera.position.block);//cameraの位置が以前のworldThread内で保存されてるlightVolumeOriginと違う値になったときだけoriginを更新してsnapshotつくる。

	//m_wRenderer.RebuildDrityChunkMesh(m_world);
	m_wRenderer.UploadPendingMeshData(m_session.GetWorldThread());


}



void Application::Render_playing() {


	m_wRenderer.UpdateSnapshots();

	m_wRenderer.DeleteMeshes(m_session.GetWorldThread());



	m_wRenderer.RenderShadowPass(m_camera);


	m_wRenderer.UpdateFogSys(m_camera);///////



	m_wRenderer.BeginHDRScene();

	m_wRenderer.RenderSky(m_camera);

	m_wRenderer.RenderWorld(
		m_camera,
		m_session.GetWorldThread().GetWorldPtr()
	);


	RenderOutline();//switch shader


	m_wRenderer.EndHDRScene();


	DebugActions actions = m_debugUI->Draw();
	ApplyDebugActions(actions);

}


void Application::CreateWorldDebugUI() {

	//なんかちょっと汚い構造
	DebugSettings debugSettings =
		m_debugBuilder.BuildDebugData(*this, *m_session.GetWorldThread().GetWorldPtr());

	//UI
	m_debugUI = std::make_unique<DebugUI>();

	m_debugUI->ReceiveSettings(std::move(debugSettings));


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

	glfwSetInputMode(
		m_window,
		GLFW_CURSOR,
		GLFW_CURSOR_NORMAL
	);

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

	m_wRenderer.InitFogSys();
	//UI
	m_imguiRenderer.Init(m_window);

	return true;

}



void Application::ProcessInput() {
	
	PlayerInput input;

	bool pressSpace = (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS);

	bool spacePressedThisFrame = pressSpace && !wasSpacePressed;

	wasSpacePressed = pressSpace;

	bool pressTab = glfwGetKey(m_window, GLFW_KEY_TAB) == GLFW_PRESS;
	bool tabPressedThisFrame = pressTab && !wasTabPressed;
	wasTabPressed = pressTab;

	bool pressAlt = glfwGetKey(m_window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS;
	bool altPressedThisFrame = pressAlt && !wasAltPressed;
	wasAltPressed = pressAlt;



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


	

	if (tabPressedThisFrame) {
		int cursorMode =
			glfwGetInputMode(m_window, GLFW_CURSOR);
		if (cursorMode == GLFW_CURSOR_NORMAL) {

			glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			m_debugUI->SetIsOpening(false);
		}
		else if (cursorMode == GLFW_CURSOR_DISABLED) {

			glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			m_debugUI->SetIsOpening(true);
		}
	}


	if (altPressedThisFrame) {

		//m_session.GetWorldThread().Rebuild_allChunks();

	}


	m_session.GetWorldThread().SetInput(std::move(input));


}


void Application::OnMouseButton(int button, int action) {
	if (m_state != AppState::PLAY) return;

	if (action != GLFW_PRESS) {
		return;
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && lastHit.isHit) {

		m_session.GetWorldThread().SubmitEditBlock(
			lastHit.hitX,
			lastHit.hitY,
			lastHit.hitZ,
			BlockType::AIR
		);

	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && lastHit.isHit) {

		m_session.GetWorldThread().SubmitEditBlock(
			lastHit.previousX,
			lastHit.previousY,
			lastHit.previousZ,     
			(BlockType)selectedBlockId
		);
	}
}


void Application::OnMouseMove(double xpos, double ypos) {
	if (m_state != AppState::PLAY) return;

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

	if (m_debugUI && m_debugUI->GetIsOpening()) return;
	m_session.GetWorldThread().AddMouseDelta(xoffset, yoffset);
}



void Application::UpdateRayHit() {

	WorldPos origin = m_camera.position;
	glm::vec3 rayDir = glm::normalize(m_camera.front);

	float distance = 4.0f;

	lastHit = m_session.GetWorldThread().RequestRaycast(origin, rayDir, distance);

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


void Application::UpdateSnapshots() {

	if (auto opt = m_exchanger.AcquirePlrRenderSnap()) {
		m_plrRenderSnap = std::move(opt);
	}


}


void Application::ApplyCameraStatus() {

	if (!m_plrRenderSnap) {
		return;
	}

	auto& previous = m_plrRenderSnap->previous;
	auto& cur = m_plrRenderSnap->current;

	float alpha = CalcInterpolationAlpha(previous, cur);


	

	m_camera.position = WorldPosLerp(previous.pos, cur.pos, static_cast<double>(alpha));
	/*m_camera.front = glm::mix(previous.front, cur.front, alpha);
	m_camera.right = glm::mix(previous.right, cur.right, alpha);
	m_camera.up = glm::mix(previous.up, cur.up, alpha);
	*/

	m_camera.front = cur.front;
	m_camera.right = cur.right;
	m_camera.up = cur.up;

}


float Application::CalcInterpolationAlpha(const PlayerSnapshot& previous, const PlayerSnapshot& current) const {

	using clock = std::chrono::steady_clock;

	constexpr double FIXED_DT = 1.0 / 60.0;
	
	const auto renderTime = 
		clock::now() -
		std::chrono::duration_cast<clock::duration>(
			std::chrono::duration<double>(FIXED_DT)
		);

	const double stateInterval =
		std::chrono::duration<double>(
			current.simTime - previous.simTime
		).count();

	if (stateInterval <= 0.0) return 0.0f;

	const double elapsed =
		std::chrono::duration<double>(
			renderTime - previous.simTime
		).count();

	return std::clamp(
		static_cast<float>(elapsed / stateInterval),
		0.0f,
		1.0f
	);

}
