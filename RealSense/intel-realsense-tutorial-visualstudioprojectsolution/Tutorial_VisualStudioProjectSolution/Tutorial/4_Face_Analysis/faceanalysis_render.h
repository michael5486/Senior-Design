/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2013-2014 Intel Corporation. All Rights Reserved.

*******************************************************************************/
#pragma once
#include <list>
#include "pxcsensemanager.h"
#include <pxcfacedata.h>
#include "util_render.h"
#include <string.h>

#define MAX_FACES 4

struct Point{
	int x, y;
};

class Face{

public:
	Face(){};
	pxcI32 faceId;

	COLORREF color;

	bool landmarksEnabled;
	std::list<Point>  points;

	bool detectionEnabled;
	PXCRectI32 faceRectangle;

	bool expressionEnabled;
	std::string expressionType;
	pxcI32 intensity;

	bool angleEnabled;
	PXCFaceData::PoseEulerAngles angle;

	bool alertEnabled;
	PXCFaceData::AlertData::AlertType label;

	Face(COLORREF,int);
};

class FaceRender: public UtilRender
{
public:
	FaceRender(pxcCHAR *title=0):UtilRender(title) {};

	void SetMaxFaces(int maxFaces);
	void SetLandmark(pxcI32 faceId,  PXCFaceData::LandmarkPoint *points, pxcI32 numofPoints);
	void SetDetection(pxcI32 faceId, PXCRectI32 faceRectangle);
	void SetExpression(pxcI32 _faceId, PXCFaceData::Face *trackedFace, pxcI32 intensity, PXCFaceData::ExpressionsData::FaceExpression expressionType);
	void SetPose(pxcI32 _faceId, PXCFaceData::Face *trackedFace, PXCFaceData::PoseEulerAngles angle);
	void SetAlert(pxcUID faceIdOfAlert, PXCFaceData::AlertData::AlertType label);
	
protected:
	virtual void DrawMore(HDC hdc, double scale_x, double scale_y);
	int maxFaces;
	Face faces[3];

};

