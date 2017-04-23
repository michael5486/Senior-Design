#pragma once
#include <windows.h>
#include <WindowsX.h>
#include <wchar.h>
#include <string>
#include <assert.h>
#include <map>
#include "resource.h"
#include "PXCPersonTrackingData.h"
#include "PersonTrackingFrameRateCalculator.h"
#include "pxcsensemanager.h"

typedef void (*OnFinishedRenderingCallback)();

class PXCImage;

class PersonTrackingRenderer
{
public:
	enum RendererType { R2D, R3D };
	PersonTrackingRenderer(HWND window);
	virtual ~PersonTrackingRenderer();


	virtual void SetOutput(PXCPersonTrackingData* output);
	virtual void SetSenseManager(PXCSenseManager* senseManager);
	virtual PXCSenseManager* GetSenseManager();
	virtual void DrawBitmap(PXCCapture::Sample* sample) = 0;
	virtual void DrawFrameNumber(int num);
	virtual void Render();
	virtual void Reset();
	void CreateMarking(PXCPersonTrackingModule* personModule, PXCCapture::Sample* sample);

protected:
	static const int LANDMARK_ALIGNMENT = -3;
	int m_detectionCounter;
	HWND m_window;
	HBITMAP m_bitmap;
	PXCPersonTrackingData* m_currentFrameOutput;
	PXCSenseManager* m_senseManager;
	PersonTrackingFrameRateCalculator m_frameRateCalcuator;
	PXCPersonTrackingData::PersonJoints::SkeletonPoint* m_joints;

	virtual void DrawGraphics(PXCPersonTrackingData* personOutput) = 0;

	virtual void DrawFrameRate();
	virtual void RefreshUserInterPerson();
	virtual RECT GetResizeRect(RECT rectangle, BITMAP bitmap);
	virtual PXCImage* GetRelevantMask(PXCPersonTrackingData::Person* person) = 0;
	virtual PXCImage* GetRelevantImage(PXCCapture::Sample* sample) = 0;


};
