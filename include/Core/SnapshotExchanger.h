#pragma once
#include "Render/DayNightSnapshot.h"
#include "Render/"

#include "Core/SnapshotChannel.h"
#include <mutex>
#include <atomic>

//currently its compatible only with daynightSnapshot 
class SnapshotExchanger {
public:

	


private:

	SnapshotChannel<DayNightSnapshot>
		m_daynightChannel;




};