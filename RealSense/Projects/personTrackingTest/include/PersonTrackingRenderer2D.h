#pragma once

#include "PersonTrackingRenderer.h"

class PersonTrackingRenderer2D : public PersonTrackingRenderer
{
public:
	PersonTrackingRenderer2D(HWND window);
	virtual ~PersonTrackingRenderer2D();

	void DrawBitmap(PXCCapture::Sample* sample);
	void Reset();

	//-----------I put these here-----------
	void DrawGraphics(PXCPersonTrackingData* PersonOutput);
	//void DrawLocation(PXCPersonTrackingData::PersonTracking* trackedPerson);

private:
	//original spots
	//void DrawGraphics(PXCPersonTrackingData* PersonOutput);
	void DrawLocation(PXCPersonTrackingData::PersonTracking* trackedPerson);
	void DrawLocation(PXCRectI32 rectangle, int red = 255, int green = 0, int blue = 0);
	void DrawHeadLocation(PXCPersonTrackingData::PersonTracking* trackedPerson);
	void DrawCenterOfMass(PXCPersonTrackingData::PersonTracking* trackedPerson, int index);
	void DrawRecognition(PXCPersonTrackingData::Person* personData, int index);
	void DrawHeadPose(PXCPersonTrackingData::PersonTracking* trackedPerson);
	void DrawSkeleton(PXCPersonTrackingData::PersonJoints* personJoints);
	void DrawGestures(PXCPersonTrackingData::PersonGestures* personGestures);
	void DrawExpressions(PXCPersonTrackingData::PersonExpressions* personExpressions, int personIndex);
	void DrawArrow(POINT start, POINT end, int red, int green, int blue);
	PXCImage* GetRelevantMask(PXCPersonTrackingData::Person* person);
	PXCImage* GetRelevantImage(PXCCapture::Sample* sample);

	int m_detectionCounter;
	PXCPersonTrackingData::PersonJoints::SkeletonPoint* joints;

	//assumes that m_bitmap object is not NULL 
	void UpdateFont();
	HFONT m_hFont;
	int m_fontWidth;
	int m_fontHeight;
};

