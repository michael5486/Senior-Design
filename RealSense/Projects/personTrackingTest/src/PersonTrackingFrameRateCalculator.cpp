#include "PersonTrackingFrameRateCalculator.h"

PersonTrackingFrameRateCalculator::PersonTrackingFrameRateCalculator() : frameRate(0), currentlyCalclatedFrameRate(0), currentTime(), isFrameRateReady(false)
{
	QueryPerformanceCounter(&previousTime);
	QueryPerformanceFrequency(&frequency);
}

bool PersonTrackingFrameRateCalculator::IsFrameRateReady()
{
	return isFrameRateReady;
}

int PersonTrackingFrameRateCalculator::GetFrameRate()
{
	isFrameRateReady = false;
	return frameRate;
}

void PersonTrackingFrameRateCalculator::Tick()
{
	QueryPerformanceCounter(&currentTime);
	++currentlyCalclatedFrameRate;
	if (currentTime.QuadPart - previousTime.QuadPart > frequency.QuadPart)
	{
		isFrameRateReady = true;
		previousTime = currentTime;
		frameRate = currentlyCalclatedFrameRate;
		currentlyCalclatedFrameRate = 0;
	}
}

