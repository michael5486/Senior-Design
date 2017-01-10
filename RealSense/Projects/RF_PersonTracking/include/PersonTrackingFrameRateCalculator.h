#pragma once

#include "windows.h"

class PersonTrackingFrameRateCalculator
{
private:
	LARGE_INTEGER frequency;
	LARGE_INTEGER previousTime;
	LARGE_INTEGER currentTime;
	int currentlyCalclatedFrameRate;
	int frameRate;
	bool isFrameRateReady;

public:
	PersonTrackingFrameRateCalculator();
	bool IsFrameRateReady();
	int GetFrameRate();
	void Tick();
};
