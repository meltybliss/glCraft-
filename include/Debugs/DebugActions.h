#pragma once
#include <optional>


struct DebugActions {


	std::optional<int> selectedBlockId;
	std::optional<double> timeOfDay;
	std::optional<double> dayLengthSeconds;
	std::optional<bool> timePaused;
	std::optional<double> timeScale;

};