#include "World/FogSystem.h"
#include "World/Chunk.h"
#include "World/WorldConfig.h"

using namespace WorldConfig;


FogSystem::FogSystem() {
	
	InitRenderableDistTexture();

}

void FogSystem::InitRenderableDistTexture() {

	m_renderableDistTexture = std::make_unique<Texture1D>(360);


}


void FogSystem::UpdateFog(const WorldPos& camera, const ChunkRenderabilitySnapshot& snap) {

	for (size_t i = 0; i < m_renderableDistances.size(); ++i) {

		double angle = glm::radians(static_cast<double>(i));

		glm::dvec2 dir{ std::cos(angle), std::sin(angle) };

		double renderableDist = FindRenderableDistance(camera, dir, snap);


		m_renderableDistances[i] = static_cast<float>(renderableDist);
	}

	m_renderableDistTexture->Bind();
	m_renderableDistTexture->UpdateSub(m_renderableDistances.data());
	
	m_renderableDistTexture->Unbind();
}



double FogSystem::FindRenderableDistance(
	const WorldPos& camera,
	const glm::dvec2& dir,
	const ChunkRenderabilitySnapshot& snap) {

	//DDA

	constexpr double w = Chunk::CHUNK_WIDTH;
	constexpr double d = Chunk::CHUNK_DEPTH;


	constexpr double MAX_RENDER_DIST =
		static_cast<double>(LOAD_CHUNKS_DISTANCE) *
		static_cast<double>(w);


	int cx = (int)floorDiv(camera.block.x, Chunk::CHUNK_WIDTH);
	int cz = (int)floorDiv(camera.block.z, Chunk::CHUNK_DEPTH);


	double localX =
		static_cast<double>(
			floorMod(camera.block.x, Chunk::CHUNK_WIDTH)
		) + camera.local.x;


	double localZ =
		static_cast<double>(
			floorMod(camera.block.z, Chunk::CHUNK_DEPTH)

		) + camera.local.z;


	int stepX = dir.x > 0.0 ? 1 : -1;
	int stepZ = dir.y > 0.0 ? 1 : -1;


	double nextBoundaryLocalX =
		stepX > 0
		? w
		: 0.0;

	double nextBoundaryLocalZ =
		stepZ > 0
		? d
		: 0.0;

	double tMaxX =
		std::abs(dir.x) > 1e-9
		? (nextBoundaryLocalX - localX) / dir.x
		: std::numeric_limits<double>::infinity();


	double tMaxZ =
		std::abs(dir.y) > 1e-9
		? (nextBoundaryLocalZ - localZ) / dir.y
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



void FogSystem::Bind(unsigned int unit) const {

	m_renderableDistTexture->Bind(unit);

}