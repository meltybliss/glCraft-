#pragma once

#include <syncstream>
#include <iostream>
#include <stdint.h>
#include "World/ChunkCoord.h"


namespace ThreadSafe_Log {

	inline void logDirtyNeighbor(
		ChunkCoord coord,
		bool urgent
	) {
		
		if (coord.x != 0 || coord.z != 0) return;

		std::osyncstream(std::cout)
			<< "[DIRTY_NEIGHBOR]"
			<< coord.x << ", " << coord.z
			<< "Urgent: " << urgent
			<< "\n";
		
	}


	inline void logMeshJob(ChunkCoord coord) {
		if (coord.x != 0 || coord.z != 0) return;

		std::osyncstream(std::cout)
			<< "[MESH_JOB]"
			<< coord.x << ", " << coord.z
			<< "\n";

	}


	inline void logMeshDone(ChunkCoord coord) {

		if (coord.x != 0 || coord.z != 0) return;

		std::osyncstream(std::cout)
			<< "[MeshJobDone]"
			<< coord.x << ", " << coord.z
			<< "\n";


	}

}
