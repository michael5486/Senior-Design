/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2013-2014 Intel Corporation. All Rights Reserved.

*******************************************************************************/
#include <windows.h>
#include <windowsX.h>
#include "faceanalysis_render.h"
#include <pxcfacedata.h>
#include <wchar.h>
#include <iostream>   // std::cout
#include <string>     // std::string, std::to_string

using namespace std;

#define COLOR_RED        RGB(255,0,0)
#define COLOR_GREEN      RGB(0,255,0)
#define COLOR_BLUE       RGB(0,0,255)
#define COLOR_YELLOW     RGB(255,255,0)
#define COLOR_WHITE		 RGB(255,255,255)


void FaceRender::SetMaxFaces(int _maxFaces)
{
	maxFaces = _maxFaces;

	for(int i = 0; i<maxFaces; i++)
	{
		faces[i].faceId = i;
		faces[i].alertEnabled = false;
		faces[i].angleEnabled = false;
		faces[i].detectionEnabled = false;
		faces[i].expressionEnabled = false;
		faces[i].landmarksEnabled = false;

		switch (i)
		{
		case 0: faces[i].color = COLOR_GREEN; break;
		case 1: faces[i].color = COLOR_YELLOW; break;
		case 2: faces[i].color = COLOR_RED; break;
		default:
			break;
		}
	}
}

void FaceRender::SetLandmark(pxcI32 _faceId,  PXCFaceData::LandmarkPoint *_points, pxcI32 _numofPoints)
{
	if(!faces[_faceId].landmarksEnabled)
	{
		faces[_faceId].landmarksEnabled = true;
		for(int i = 0 ; i<_numofPoints; i++)
		{
			Point _p = {(int)_points[i].image.x,(int)_points[i].image.y};
			faces[_faceId].points.push_back(_p);
		}
	}

}

void FaceRender::SetDetection(pxcI32 _faceId, PXCRectI32 _faceRectangle)
{
	if(!faces[_faceId].detectionEnabled)
	{
		faces[_faceId].detectionEnabled = true;
		faces[_faceId].faceRectangle = _faceRectangle;
	}
}

void FaceRender::SetExpression(pxcI32 _faceId, PXCFaceData::Face *_trackedFace, pxcI32 _intensity, PXCFaceData::ExpressionsData::FaceExpression _expressionType)
{
	if(!faces[_faceId].expressionEnabled)
	{
		PXCFaceData::DetectionData *detectionData = _trackedFace->QueryDetection();

		if(detectionData!=NULL)
		{
			detectionData->QueryBoundingRect(&faces[_faceId].faceRectangle);
			faces[_faceId].expressionEnabled = true;
			faces[_faceId].intensity = _intensity;
			switch(_expressionType)
			{
			case PXCFaceData::ExpressionsData::FaceExpression::EXPRESSION_BROW_RAISER_LEFT:
				faces[_faceId].expressionType = "BROW_RAISER_LEFT"; break;
			case PXCFaceData::ExpressionsData::FaceExpression::EXPRESSION_BROW_RAISER_RIGHT:
				faces[_faceId].expressionType = "BROW_RAISER_RIGHT"; break;
			case PXCFaceData::ExpressionsData::FaceExpression::EXPRESSION_BROW_LOWERER_LEFT:
				faces[_faceId].expressionType = "BROW_LOWERER_LEFT"; break;
			case PXCFaceData::ExpressionsData::FaceExpression::EXPRESSION_BROW_LOWERER_RIGHT:
				faces[_faceId].expressionType = "BROW_LOWERER_RIGHT"; break;
			case PXCFaceData::ExpressionsData::FaceExpression::EXPRESSION_SMILE:
				faces[_faceId].expressionType = "SMILE"; break;
			case PXCFaceData::ExpressionsData::FaceExpression::EXPRESSION_KISS:
				faces[_faceId].expressionType = "KISS"; break;
			case PXCFaceData::ExpressionsData::FaceExpression::EXPRESSION_MOUTH_OPEN:
				faces[_faceId].expressionType = "MOUTH_OPEN"; break;
			case PXCFaceData::ExpressionsData::FaceExpression::EXPRESSION_EYES_CLOSED_LEFT:
				faces[_faceId].expressionType = "EYES_CLOSED_LEFT"; break;
			case PXCFaceData::ExpressionsData::FaceExpression::EXPRESSION_EYES_CLOSED_RIGHT:
				faces[_faceId].expressionType = "EYES_CLOSED_RIGHT"; break;
			case PXCFaceData::ExpressionsData::FaceExpression::EXPRESSION_HEAD_TURN_LEFT:
				faces[_faceId].expressionType = "HEAD_TURN_LEFT"; break;
			case PXCFaceData::ExpressionsData::FaceExpression::EXPRESSION_HEAD_TURN_RIGHT:
				faces[_faceId].expressionType = "HEAD_TURN_RIGHT"; break;
			case PXCFaceData::ExpressionsData::FaceExpression::EXPRESSION_HEAD_UP:
				faces[_faceId].expressionType = "HEAD_UP"; break;
			case PXCFaceData::ExpressionsData::FaceExpression::EXPRESSION_HEAD_DOWN:
				faces[_faceId].expressionType = "HEAD_DOWN"; break;
			case PXCFaceData::ExpressionsData::FaceExpression::EXPRESSION_HEAD_TILT_LEFT:
				faces[_faceId].expressionType = "HEAD_TILT_LEFT"; break;
			case PXCFaceData::ExpressionsData::FaceExpression::EXPRESSION_HEAD_TILT_RIGHT:
				faces[_faceId].expressionType = "HEAD_TILT_RIGHT"; break;
			case PXCFaceData::ExpressionsData::FaceExpression::EXPRESSION_EYES_TURN_LEFT:
				faces[_faceId].expressionType = "EYES_TURN_LEFT"; break;
			case PXCFaceData::ExpressionsData::FaceExpression::EXPRESSION_EYES_TURN_RIGHT:
				faces[_faceId].expressionType = "EYES_TURN_RIGHT"; break;
			case PXCFaceData::ExpressionsData::FaceExpression::EXPRESSION_EYES_UP:
				faces[_faceId].expressionType = "EYES_UP"; break;
			case PXCFaceData::ExpressionsData::FaceExpression::EXPRESSION_EYES_DOWN:
				faces[_faceId].expressionType = "EYES_DOWN"; break;
			}
		}
	}
}

void FaceRender::SetPose(pxcI32 _faceId, PXCFaceData::Face *_trackedFace, PXCFaceData::PoseEulerAngles _angle)
{
	if(!faces[_faceId].angleEnabled)
	{
		PXCFaceData::DetectionData *detectionData = _trackedFace->QueryDetection();

		if(detectionData!=NULL)
		{
			detectionData->QueryBoundingRect(&faces[_faceId].faceRectangle);
			faces[_faceId].angle = _angle;
			faces[_faceId].angleEnabled = true;
		}
	}

}

void FaceRender::DrawMore(HDC hdc, double sx, double sy) 
{

	/* Draw Landmarks */
	int radius = 1;

	for(int i=0; i<maxFaces;i++)
	{	
		if(faces[i].landmarksEnabled)
		{
			HPEN lColor=CreatePen(PS_SOLID, 2, faces[i].color);
			SelectObject(hdc, lColor);
			for (std::list<Point>::iterator itrl=faces[i].points.begin(); itrl!=faces[i].points.end(); itrl++) 
			{
				int x=(int)(itrl->x*sx);
				int y=(int)(itrl->y*sy);
				int rx=(int)(radius*sx);
				int ry=(int)(radius*sy);
				MoveToEx(hdc, x, y, NULL);
				Arc(hdc,x-rx,y-ry,x+rx,y+ry,x+rx,y+ry,x+rx,y+ry);

			}
			faces[i].landmarksEnabled = false;
			DeleteObject(lColor);
		}
		faces[i].points.clear();


		if(faces[i].detectionEnabled)
		{
			HPEN dColor=CreatePen(PS_SOLID, 1, faces[i].color);
			SelectObject(hdc, dColor);
			MoveToEx(hdc, faces[i].faceRectangle.x*sx, faces[i].faceRectangle.y*sy, 0);
			LineTo(hdc, faces[i].faceRectangle.x*sx, faces[i].faceRectangle.y*sy + faces[i].faceRectangle.h*sy);
			LineTo(hdc, faces[i].faceRectangle.x*sx + faces[i].faceRectangle.w*sx, faces[i].faceRectangle.y*sy + faces[i].faceRectangle.h*sy);
			LineTo(hdc, faces[i].faceRectangle.x*sx + faces[i].faceRectangle.w*sx, faces[i].faceRectangle.y*sy);
			LineTo(hdc, faces[i].faceRectangle.x*sx, faces[i].faceRectangle.y*sy);
			DeleteObject(dColor);
			faces[i].detectionEnabled = false;
		}

		if(faces[i].expressionEnabled)
		{
			SetTextColor(hdc, faces[i].color);
			char dispLine[64];
			strcpy_s(dispLine, faces[i].expressionType.c_str());
			TextOut(hdc, faces[i].faceRectangle.x*sx, (faces[i].faceRectangle.y-30)*sy, dispLine, strlen(dispLine));
			if(faces[i].intensity <10)
				TextOut(hdc, faces[i].faceRectangle.x*sx, (faces[i].faceRectangle.y-20)*sy,LPCSTR(to_string(faces[i].intensity).c_str()),1);
			else
				TextOut(hdc, faces[i].faceRectangle.x*sx, (faces[i].faceRectangle.y-20)*sy,LPCSTR(to_string(faces[i].intensity).c_str()),2);

			faces[i].expressionEnabled = false;
		}

		if(faces[i].angleEnabled)
		{
			SetTextColor(hdc, faces[i].color);
			WCHAR tempLine[64];
			swprintf_s<sizeof(tempLine) / sizeof(WCHAR) > (tempLine, L"Yaw  : %.0f", faces[i].angle.yaw);
			TextOut(hdc, (faces[i].faceRectangle.x+90)*sx, (faces[i].faceRectangle.y-35)*sy, LPCSTR(tempLine), wcslen(tempLine)*2);
			swprintf_s<sizeof(tempLine) / sizeof(WCHAR) > (tempLine, L"Roll : %.0f", faces[i].angle.roll);
			TextOut(hdc, (faces[i].faceRectangle.x+90)*sx, (faces[i].faceRectangle.y-25)*sy, LPCSTR(tempLine), wcslen(tempLine)*2);
			swprintf_s<sizeof(tempLine) / sizeof(WCHAR) > (tempLine, L"Pitch: %.0f", faces[i].angle.pitch);
			TextOut(hdc, (faces[i].faceRectangle.x+90)*sx, (faces[i].faceRectangle.y-15)*sy, LPCSTR(tempLine), wcslen(tempLine)*2);
			faces[i].angleEnabled = false;
		}

	}
}