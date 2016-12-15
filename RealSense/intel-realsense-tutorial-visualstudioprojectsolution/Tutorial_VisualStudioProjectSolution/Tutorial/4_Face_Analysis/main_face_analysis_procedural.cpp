/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2013-2014 Intel Corporation. All Rights Reserved.

*******************************************************************************/
/* 
Description:
This is the face analysis procedural sample that shows how to enable and get face
tracking->landmarks->pose features by procedural calls. 

1- Enable tracking, landmarks, and pose in the pipeline.
2- Initialize the pipeline camera-> face blocks by the Init function. 
3- In the loop, use the AcquireFrame function to wait for all streams and face detection data to be ready,
and then retrieve it through the QuerySample function. 
4- Loop through, querying all detected faces and querying the data for tracking, landmarks, and pose.
5- Render the frame or stream using the UtilRender class provided in the DSSDK libpxcutils.lib
6- Release the frame for reading next sample through the ReleaseFrame function. 
7- Finally, use the release function to clean up.
*/

#include <windows.h>
#include <wchar.h>
#include <pxcsensemanager.h>
#include <pxcfaceconfiguration.h>
#include "faceanalysis_render.h"

#define MAX_FACES 2

int wmain(int argc, WCHAR* argv[]) {

	// error checking Status
	pxcStatus sts;

	// initialize the util renderer 
	FaceRender *renderer = new FaceRender(L"PROCEDURAL FACE TRACKING");
	renderer->SetMaxFaces(MAX_FACES);

	// create the PXCSenseManager
	PXCSenseManager *psm=0;
	psm = PXCSenseManager::CreateInstance();
	if (!psm) {
		wprintf_s(L"Unable to create the PXCSenseManager\n");
		return 1;
	}

	// select the color stream of size 640x480
	psm->EnableStream(PXCCapture::STREAM_TYPE_COLOR, 640, 480);
	
	// Enable face analysis in the multimodal pipeline:
	sts = psm->EnableFace();
	if (sts < PXC_STATUS_NO_ERROR) {
		wprintf_s(L"Unable to enable Face Analysis\n");
		return 2;
	}
	//retrieve face module if ready
	PXCFaceModule* faceAnalyzer = psm->QueryFace();
	if (!psm) {
		wprintf_s(L"Unable to retrieve face results\n");
		return 3;
	}

	// initialize the PXCSenseManager pipeline
	if(psm->Init() < PXC_STATUS_NO_ERROR) return 4;

	// retrieves an instance of the PXCFaceData interface
	PXCFaceData* outputData = faceAnalyzer->CreateOutput();

	// retrieves an instance of the PXCFaceConfiguration interface
	PXCFaceConfiguration* config = faceAnalyzer->CreateActiveConfiguration();
	
	// configure the face module features
	// set the 3D face tracker. if caamera cannot support depth it will automatically roll back to 2D treacking.
	// if depth stream is not enabled it will be enabled if the camera supports it. 
	config->SetTrackingMode(PXCFaceConfiguration::TrackingModeType::FACE_MODE_COLOR_PLUS_DEPTH);
	config->EnableAllAlerts();
	config->landmarks.isEnabled = true;
	config->landmarks.maxTrackedFaces = MAX_FACES;
	config->detection.isEnabled = true;
	config->detection.maxTrackedFaces = MAX_FACES;
	config->pose.isEnabled = true;
	config->pose.maxTrackedFaces = MAX_FACES;
	config->QueryExpressions()->Enable();
	config->QueryExpressions()->EnableAllExpressions();
	config->QueryExpressions()->properties.maxTrackedFaces = MAX_FACES;
	config->ApplyChanges();


	// stream data
	int fnum = 0; //frame counter
	PXCImage *image = NULL;
	while (psm->AcquireFrame(true)>=PXC_STATUS_NO_ERROR) {

		// increment frame counter since a frame is acquired
		fnum++;

		//update the output data to the latest availible
		outputData->Update();	

		/* Detection Structs */
		PXCFaceData::DetectionData *detectionData;
		PXCRectI32 rectangle;

		/* Landmark Structs */
		PXCFaceData::LandmarksData* landmarkData;
		PXCFaceData::LandmarkPoint* landmarkPoints;
		pxcI32 numPoints;

		/* Expression Structs */
		PXCFaceData::ExpressionsData *expressionData;
		PXCFaceData::ExpressionsData::FaceExpressionResult expressionResult;

		/* Pose Structs */
		PXCFaceData::PoseData *poseData;
		PXCFaceData::PoseEulerAngles angles;

		// get number of detected faces
		pxcU16 numOfFaces = outputData->QueryNumberOfDetectedFaces();
		
		//loop through all detected faces
		for(pxcU16 i = 0 ; i < numOfFaces; i++)
		{
			// get face data by time of appearence
			PXCFaceData::Face *trackedFace = outputData->QueryFaceByIndex(i);
			if(trackedFace != NULL)
			{
				/* Query Detection Data */
				detectionData = trackedFace->QueryDetection();
				if(detectionData!=NULL)
				{
					/* Get rectangle of the detected face and render */
					if(detectionData->QueryBoundingRect(&rectangle))
						renderer->SetDetection(i,rectangle);
				}


				/* Query Landmark Data */
				landmarkData = trackedFace->QueryLandmarks();
				if(landmarkData!=NULL)
				{
					/* Get number of points from Landmark data*/
					numPoints = landmarkData->QueryNumPoints();

					/* Create an Array with the number of points */
					landmarkPoints = new PXCFaceData::LandmarkPoint[numPoints];

					/* Query Points from Landmark Data and render */
					if(landmarkData->QueryPoints(landmarkPoints))
						renderer->SetLandmark(i,landmarkPoints,numPoints);
				}


				/* Query Expression Data */
				expressionData = trackedFace->QueryExpressions();
				if(expressionData!=NULL)
				{
					/* Get expression of the detected face and render */
					if(expressionData->QueryExpression(PXCFaceData::ExpressionsData::EXPRESSION_MOUTH_OPEN, &expressionResult));
						renderer->SetExpression(i,trackedFace, expressionResult.intensity, PXCFaceData::ExpressionsData::EXPRESSION_MOUTH_OPEN);
				}

				/* Query Pose Data */
				poseData = trackedFace->QueryPose();
				if(poseData!=NULL)
				{
					/* Get angle of the detected face and render */
					if(poseData->QueryPoseAngles(&angles))
						renderer->SetPose(i,trackedFace,angles);
				}

			}

		}

		// retrieve all available image samples
		PXCCapture::Sample *sample = psm->QuerySample();

		// get the color data
		image = sample->color;

		// render the frame
		if (!renderer->RenderFrame(image)) break;

		// iterate through Alerts 
		PXCFaceData::AlertData alertData;
		for(int i = 0 ; i <outputData->QueryFiredAlertsNumber(); i++)
		{
			if(outputData->QueryFiredAlertData(i, &alertData)==PXC_STATUS_NO_ERROR)
			{
				// Display last alert - see AlertData::Label for all available alerts
				switch(alertData.label)
				{
				case PXCFaceData::AlertData::ALERT_NEW_FACE_DETECTED:
					{
						wprintf_s(L"Last Alert: Face %d Detected\n", alertData.faceId);
						break;
					}
				case PXCFaceData::AlertData::ALERT_FACE_LOST:
					{ 
						wprintf_s(L"Last Alert: Face %d Lost\n", alertData.faceId);
						break;
					}
				}

			}
		}

		// release or unlock the current frame to fetch the next frame
		psm->ReleaseFrame();
	}

	// delete the UtilRender instance
	renderer->Release();

	//delete the configuration
	config->Release();

	// close the last opened streams and release any session and processing module instances
	psm->Release();

	return 0;
}