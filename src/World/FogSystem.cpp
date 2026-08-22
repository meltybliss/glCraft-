#include "World/FogSystem.h"
#include "World/Chunk.h"
#include "World/WorldConfig.h"

using namespace WorldConfig;



void FogSystem::InitRenderableDistTexture() {

	m_renderableDistTexture = std::make_unique<Texture1D>(360);


}


void FogSystem::UpdateFog(const glm::dvec3& camera, const ChunkRenderabilitySnapshot& snap) {

	for (int i = 0; i < 360; ++i) {

		double angle = glm::radians(static_cast<double>(i));

		glm::dvec2 dir{ std::cos(angle), std::sin(angle) };

		double renderableDist = FindRenderableDistance(glm::dvec2(camera.x, camera.z), dir, snap);


		m_renderableDistances[i] = static_cast<float>(renderableDist);
	}

	m_renderableDistTexture->Bind();
	m_renderableDistTexture->UpdateSub(m_renderableDistances.data());
	
	m_renderableDistTexture->Unbind();
}



double FogSystem::FindRenderableDistance(
	const glm::dvec2& cameraXZ, 
	const glm::dvec2& dir,
	const ChunkRenderabilitySnapshot& snap) {

	//DDA

	constexpr double w = Chunk::CHUNK_WIDTH;
	constexpr double d = Chunk::CHUNK_DEPTH;


	constexpr double MAX_RENDER_DIST =
		static_cast<double>(LOAD_CHUNKS_DISTANCE) *
		static_cast<double>(w);


	int cx = (int)std::floor(cameraXZ.x / w);
	int cz = (int)std::floor(cameraXZ.y / d);


	int stepX = dir.x > 0.0 ? 1 : -1;
	int stepZ = dir.y > 0.0 ? 1 : -1;


	double nextBoundaryX =
		stepX > 0
		? (cx + 1) * w
		: cx * w;

	double nextBoundaryZ =
		stepZ > 0
		? (cz + 1) * d
		: cz * d;


	double tMaxX =
		std::abs(dir.x) > 1e-9
		? (nextBoundaryX - cameraXZ.x) / dir.x
		: std::numeric_limits<double>::infinity();

	double tMaxZ =
		std::abs(dir.y) > 1e-9
		? (nextBoundaryZ - cameraXZ.y) / dir.y
		: std::numeric_limits<double>::infinity();


	double deltaX =
		std::abs(dir.x) > 1e-9
		? w / std::abs(dir.x)
		: std::numeric_limits<double>::infinity();


	double deltaZ =
		std::abs(dir.y) > 1e-9
		? d / std::abs(dir.y)
		: std::numeric_limits<double>::infinity();


	double dist = 0.0;

	while (dist < MAX_RENDER_DIST) {


		if (tMaxX < tMaxZ) {

			dist = tMaxX;
			tMaxX += deltaX;
			cx += stepX;

		}
		else {
			dist = tMaxZ;
			tMaxZ += deltaZ;
			cz += stepZ;
		}

		if (!snap.IsRenderableChunk(cx, cz)) {
			return dist;
		}

	}


	return MAX_RENDER_DIST;
}