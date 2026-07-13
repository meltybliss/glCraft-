#pragma once

#include <syncstream>
#include <iostream>
#include <stdint.h>


namespace ThreadSafe_Log {

	inline void logDirtyNeighbor(
		int32_t cx,
		int32_t cz,
		bool urgent
	) {
		
		if (cx != 0 || cz != 0) return;

		std::osyncstream(std::cout)
			<< "[DIRTY_NEIGHBOR]"
			<< cx << ", " << cz
			<< "Urgent: " << urgent
			<< "\n";
		
	}


	inline void logMeshJob(
		int32_t cx,
		int32_t cz
	) {
		if (cx != 0 || cz != 0) return;

		std::osyncstream(std::cout)
			<< "[MESH_JOB]"
			<< cx << ", " << cz
			<< "\n";

	}


	inline void logMeshDone(
		int32_t cx,
		int32_t cz
	) {

		if (cx != 0 || cz != 0) return;

		std::osyncstream(std::cout)
			<< "[MeshJobDone]"
			<< cx << ", " << cz
			<< "\n";


	}

}