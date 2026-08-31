#pragma once


struct DayNightState {

	double timeOfDay = 0.5;//0~1
	double dayLengthSeconds = 160.0;
	double timeScale = 1.0;

	bool paused = false;


};