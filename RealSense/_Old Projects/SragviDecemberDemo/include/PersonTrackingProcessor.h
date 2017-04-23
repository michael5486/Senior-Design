#pragma once

#include <windows.h>

#include "pxccapture.h"

class PXCSenseManager;
class PXCPersonTrackingData;
class PXCPersonTrackingConfiguration;

class PersonTrackingProcessor
{
public:
	PersonTrackingProcessor(HWND window);
	void SetStreamProfileSet(const PXCCapture::Device::StreamProfileSet& streamProfileSet);
	void Process(HWND dialogWindow);
	void StartTracking();
	void StopTracking();
	bool IsTracking();
	void RegisterUser();
	void UnregisterUser();

private:
	void PerformTrack(int id);
	void PerformUntrack(int id);
	void PerformRegister(int id);
	void PerformUnregister(int id);
	void UpdateConfiguration(HWND dialogWindow, PXCPersonTrackingConfiguration* config);
	HWND m_window;
	PXCPersonTrackingData* m_output;
	bool m_startTrackingFlag;
	bool m_stopTrackingFlag;
	bool m_registerFlag;
	bool m_unregisterFlag;
	PXCCapture::Device::StreamProfileSet m_streamProfileSet;
};