#include "Debugs/DebugDataBuilder.h"
#include "Core/Application.h"
#include "World/World.h"

DebugSettings DebugDataBuilder::BuildDebugData(const Application& app, World& world) {

	DebugSettings settings;
	DayNightState state =
		world.GetDayNightStateForDebug();

	settings.selectedBlockId = app.GetSelectedBlock();

	settings.timeOfDay = state.timeOfDay;
	settings.dayLengthSeconds = state.dayLengthSeconds;
	settings.timePaused = state.paused;
	settings.timeScale = state.timeScale;


	return settings;

}