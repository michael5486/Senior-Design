#pragma once

#include <memory>
#include "PersonTrackingRenderer.h"
#include "service/pxcsessionservice.h"

	class PersonTrackingRenderer3D : public PersonTrackingRenderer
{
public:
	PersonTrackingRenderer3D(HWND window, PXCSession* session);
	virtual ~PersonTrackingRenderer3D();

	void DrawBitmap(PXCCapture::Sample* sample);

private:
	void DrawGraphics(PXCPersonTrackingData* PersonOutput);
	void DrawCenterOfMass(PXCPersonTrackingData::PersonTracking* trackedPerson, int index);
	void DrawSkeleton(PXCPersonTrackingData::PersonJoints* personJoints);
	bool ProjectVertex(const PXCPoint3DF32 &v, int &x, int &y, int radius = 0);
	void DrawPerson(PXCPersonTrackingData::Person* trackedPerson);
	PXCImage* GetRelevantMask(PXCPersonTrackingData::Person* person);
	PXCImage* GetRelevantImage(PXCCapture::Sample* sample);


	PXCSession* m_session;
	PXCImage::ImageInfo m_outputImageInfo;
	PXCImage* m_outputImage;
	PXCImage::ImageData m_outputImageData;
};