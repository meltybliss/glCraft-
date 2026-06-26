#pragma once

#include "Chunk.h"
#include "WorldCommand.h"
#include <thread>
#include <atomic>
#include <deque>
class WorldThread {
public:

	void SubmitEditBlock(
		int32_t worldX,
		int32_t worldY,
		int32_t worldZ,
		BlockType b

	);


private:

	std::thread worldThread;
	

	std::deque<WorldCommand> m_commands;


};