#include <Windows.h>
#include <WinUser.h>
#include "pxccapturemanager.h"
#include "pxcsensemanager.h"
#include "pxcmetadata.h"
#include "util_cmdline.h"
#include "util_render.h"
#include <conio.h> //for _getch
#include "pxcpersontrackingmodule.h"
#include "pxcpersontrackingdata.h"
#include "pxcvideomodule.h"
#include "pxcpersontrackingconfiguration.h"
#include "pxcmetadata.h"
#include "service/pxcsessionservice.h"
#include <assert.h>
#include "myPerson.h"
//#include "myLogging.h"
#include "circleBuffer.h"

/* Required for ouputting data to file */
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <stdlib.h>
#include <sstream>

using namespace std;


#define MAX_INITIALIZE_COUNT 100
#define ATV_WIDTH 2 //FIGURE OUT ACTUAL VALUE OF ATV WIDTH (ft.)
#define WHEEL_RADIUS 0.5 //FIGURE OUT ACTUAL VALUE OF WHEEL RADIUS (ft.)
int initializeCount = 0;

PXCSession *session = NULL;

/* Variables needed to compile */
pxcCHAR fileName[1024] = { 0 };
HANDLE ghMutex;

/* Global variables used in target identification */
double frameRate = 30.0; //adjust as needed
myPerson targetUser;
int TU_uID = 0; //TU's uID always starts at 0, this value can only change after comparePeopleinFOV
int TU_pID;
bool isInitialized = false;
int numPeopleFound = -1;
double targetUserTorsoHeight;
double targetUserShoulderWidth;

/*ULV variables*/
double robVector[2];
circleBuffer circBuff;

/* Method declarations */ 
boolean initializeTargetUser(PXCPersonTrackingModule* personModule);
void comparePeopleInFOV(PXCPersonTrackingModule* personModule, int numPeople);
//myPerson convertPXCPersonToMyPerson(PXCPersonTrackingData::Person* person);
boolean containsTargetUser(PXCPersonTrackingModule* personModule);
double proximitytoLKL(myPoint currCM);
void targetUserFound(PXCPersonTrackingModule* personModule);
double* determineControls(myPoint destination);
double convertFeetToRSU(double feet);
double convertRSUToFeet(double rsu);

int main(int argc, WCHAR* argv[]) {
	
	/* Creates an instance of the PXCSenseManager */
	PXCSenseManager *pp = PXCSenseManager::CreateInstance();

	if (!pp) {
		wprintf_s(L"Unable to create the SenseManager\n");
		return 3;
	}

	/* Initiates session */
	session = pp->QuerySession();
	
	/* Configures coordinate sytem to work with OpenCV */
	session->SetCoordinateSystem(PXCSession::COORDINATE_SYSTEM_REAR_OPENCV);
	
	/* Enables person tracking module */
	pp->EnablePersonTracking();
	
	/* Checks if coodinate system successfull switched to OpenCV mode */
	PXCSession::CoordinateSystem cs = session->QueryCoordinateSystem();
	if (cs & PXCSession::COORDINATE_SYSTEM_REAR_OPENCV) {
		printf("OpenCV coordinate mode enabled...\n");
	}
	else {
		printf("OpenCV mode unsuccessful...\n");
	}


	/* Creates stream renders */
	UtilRender renderc(L"Color"), renderd(L"Depth");
	pxcStatus sts;

	/* Creates render windows */
	HWND colorWindow = renderc.m_hWnd;
	HWND depthWindow = renderc.m_hWnd;

	do {
		PXCVideoModule::DataDesc desc = {};
		desc.deviceInfo.streams = PXCCapture::STREAM_TYPE_COLOR | PXCCapture::STREAM_TYPE_DEPTH;
		pp->EnableStreams(&desc);

		/* Initializes the pipeline */
		sts = pp->Init();
		if (sts < PXC_STATUS_NO_ERROR) {
			/* Enable a single stream */
			pp->Close();
			pp->EnableStream(PXCCapture::STREAM_TYPE_DEPTH);
			sts = pp->Init();
			if (sts < PXC_STATUS_NO_ERROR) {
				pp->Close();
				pp->EnableStream(PXCCapture::STREAM_TYPE_COLOR);
				sts = pp->Init();
			}

			if (sts < PXC_STATUS_NO_ERROR) {
				wprintf_s(L"Failed to locate any video stream(s)\n");
				pp->Release();
				return sts;
			}
		}

		/* Enabling skeleton joint tracking */
		PXCPersonTrackingModule* personModule = pp->QueryPersonTracking();
		PXCPersonTrackingConfiguration* personTrackingConfig = personModule->QueryConfiguration();
		personTrackingConfig->QueryTracking()->SetTrackingMode(PXCPersonTrackingConfiguration::TrackingConfiguration::TRACKING_MODE_INTERACTIVE);

		PXCPersonTrackingConfiguration::SkeletonJointsConfiguration* skeletonJoints = personTrackingConfig->QuerySkeletonJoints();
		skeletonJoints->Enable();
		//printf("is jointTracking Enabled?: %d\n", skeletonJoints->IsEnabled());


		printf("Initializing stream...\n");



		while (!isInitialized) {
			/* Waits until new frame is available and locks it for application processing */
			sts = pp->AcquireFrame(false);

			/* Render streams */
			PXCCapture::Sample *sample = pp->QuerySample();

			if (sample) {
				PXCPersonTrackingModule* personModule = pp->QueryPersonTracking();

				/* If no persons are visible, renders and releases current frame */
				if (personModule == NULL) {
					if (sample->depth && !renderd.RenderFrame(sample->depth)) break;
					if (sample->color && !renderc.RenderFrame(sample->color)) break;
					pp->ReleaseFrame();
					printf("Person Module is NULL, not initialized\n");
					continue;
				}

				int numPeople = personModule->QueryOutput()->QueryNumberOfPeople();

				/* Found a person */
				if (numPeople == 1) {
					/* When this method is called enough times, it will return true and break initialization loop */
					if (initializeTargetUser(personModule)) {
						break;
					}

				}

				/* Releases lock so pipeline can process next frame */
				pp->ReleaseFrame();

			}
		}

		printf("Target initialized: \n");
		printf("torsoHeight = %f\n", targetUserTorsoHeight);
		printf("shoulderWidth = %f\n", targetUserShoulderWidth);



		/* Stream Data */
		while (true) {
			/* Waits until new frame is available and locks it for application processing */
			sts = pp->AcquireFrame(false);

			/* Render streams */
			PXCCapture::Sample *sample = pp->QuerySample();

			if (sample) {
				//printf("running");
				PXCPersonTrackingModule* personModule = pp->QueryPersonTracking();

				/* If no persons are visible, renders and releases current frame */
				if (personModule == NULL) {
					if (sample->depth && !renderd.RenderFrame(sample->depth)) break;
					if (sample->color && !renderc.RenderFrame(sample->color)) break;
					pp->ReleaseFrame();
					printf("Person Module is null, man\n");
					continue;
				}

				int numPeople = personModule->QueryOutput()->QueryNumberOfPeople();

				/* Found a person */

				if (numPeople == 0) {
					//CALL targetUserNotFound, SET LKL AS DESTINATION, DETERMINE CONTROLS, MAKE NOISE
				}
				else if (numPeople == 1) {
					printf("numPeople = %d\n", numPeople);
					if (containsTargetUser(personModule)) {
						//targetUserFound(personModule);
						printf("contains target user...   z = %f\n", personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, 0)->QueryTracking()->QueryCenterMass().world.point.z);
					}
					else {
						printf("contains secondary user... z = %f\n", personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, 0)->QueryTracking()->QueryCenterMass().world.point.z);
					}
				}
				else {
					printf("numPeople = %d\n", numPeople);
					printf("more than 1 person in FOV\n");
					//COMPARE PEOPLE IN FOV
				}
			}


			/* Releases lock so pipeline can process next frame */
			pp->ReleaseFrame();

			if (_kbhit()) { // Break loop
				int c = _getch() & 255;
				if (c == 27 || c == 'q' || c == 'Q') break; // ESC|q|Q for Exit
			}

		}

	} while (sts == PXC_STATUS_STREAM_CONFIG_CHANGED);

	wprintf_s(L"Exiting\n");

	// Clean Up
	pp->Release();
	return 0;
}

/* Finds median shoulderDistance and torsoHeight values over a definied number of frames to increase accuracy */
boolean initializeTargetUser(PXCPersonTrackingModule* personModule) {
	/* Accesses the only person in camera's FOV, our target user */
	PXCPersonTrackingData::Person* personData = personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, 0);
	assert(personData != NULL);

	/* Queries for skeleton joint data */
	PXCPersonTrackingData::PersonJoints* personJoints = personData->QuerySkeletonJoints();

	PXCPersonTrackingData::PersonJoints::SkeletonPoint* joints = new PXCPersonTrackingData::PersonJoints::SkeletonPoint[personJoints->QueryNumJoints()];
	personJoints->QueryJoints(joints);

	/* Joint info is valid, initialize target user */
	if (isJointInfoValid(joints)) {
		myPoint head          (joints[2].world.x, joints[2].world.y, joints[2].world.z, joints[2].image.x, joints[2].image.y);
		myPoint shoulderLeft  (joints[4].world.x, joints[4].world.y, joints[4].world.z, joints[4].image.x, joints[4].image.y);
		myPoint shoulderRight (joints[5].world.x, joints[5].world.y, joints[5].world.z, joints[5].image.x, joints[5].image.y);
		myPoint spineMid      (joints[3].world.x, joints[3].world.y, joints[3].world.z, joints[3].image.x, joints[3].image.y);
		
		PXCPersonTrackingData::PersonTracking::PointCombined centerMass = personData->QueryTracking()->QueryCenterMass();
		myPoint myCenterMass(centerMass.world.point.x, centerMass.world.point.y, centerMass.world.point.z, centerMass.image.point.x, centerMass.image.point.y);
		
		/* Calculates torsoHeight and shoulderDistance and adds them to respective vectors to keep track */
		targetUser.updatePerson(head, shoulderLeft, shoulderRight, spineMid, myCenterMass);

		/* Successful joint reading, increment counter */
		initializeCount++;
	}

	if (initializeCount >= MAX_INITIALIZE_COUNT) {
		/*After enough frames of initialization, begin location buffer, track target user's uID, and determine user proportions*/
		PXCPersonTrackingData::PersonTracking::PointCombined centerMass = personData->QueryTracking()->QueryCenterMass();
		myPoint myCenterMass(centerMass.world.point.x, centerMass.world.point.y, centerMass.world.point.z, centerMass.image.point.x, centerMass.image.point.y);
		circBuff.startBuffer(myCenterMass);
		targetUserTorsoHeight = targetUser.getMedianTorsoHeight();
		targetUserShoulderWidth = targetUser.getMedianShoulderDistance();
		TU_uID = personData->QueryTracking()->QueryId();
		return true;
	}
	
	/* Frees up space allocated for joints array, it is not needed anymore */
	delete[] joints;

	/* Not enough joint readings from target user, return false */
	return false;
}



/* Iterates across all people in FOV, compares against target user */
void comparePeopleInFOV(PXCPersonTrackingModule* personModule, int numPeople) {

	/* Iterates across all people in FOV */
	for (int perIter = 0; perIter < numPeople; perIter++) {

		PXCPersonTrackingData::Person* personData = personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, perIter);
		assert(personData != NULL);
		PXCPersonTrackingData::PersonJoints* personJoints = personData->QuerySkeletonJoints();

		PXCPersonTrackingData::PersonJoints::SkeletonPoint* joints = new PXCPersonTrackingData::PersonJoints::SkeletonPoint[personJoints->QueryNumJoints()];
		personJoints->QueryJoints(joints);

		PXCPersonTrackingData::PersonTracking::PointCombined centerMass = personData->QueryTracking()->QueryCenterMass();
		myPoint myCenterMass(centerMass.world.point.x, centerMass.world.point.y, centerMass.world.point.z, centerMass.image.point.x, centerMass.image.point.y);


		if (isJointInfoValid(joints) == false) {
			printf("Invalid joint data...no comparison\n");
		}
		else {
			//myPoint leftHand        (joints[0].world.x, joints[0].world.y, joints[0].world.z, joints[0].image.x, joints[0].image.y);
			//myPoint rightHand       (joints[1].world.x, joints[1].world.y, joints[1].world.z, joints[1].image.x, joints[1].image.y);
			myPoint head            (joints[2].world.x, joints[2].world.y, joints[2].world.z, joints[2].image.x, joints[2].image.y);
			myPoint shoulderLeft    (joints[4].world.x, joints[4].world.y, joints[4].world.z, joints[4].image.x, joints[4].image.y);
			myPoint shoulderRight   (joints[5].world.x, joints[5].world.y, joints[5].world.z, joints[5].image.x, joints[5].image.y);
			myPoint spineMid        (joints[3].world.x, joints[3].world.y, joints[3].world.z, joints[3].image.x, joints[3].image.y);


			myPerson curr = myPerson(head, shoulderLeft, shoulderRight, spineMid, myCenterMass);
			//curr.printPerson(); //can implement while testing
			double currConf = compareTorsoRatio(curr, targetUser); //confidence that current person is user
			printf("%d. Similarity = %.2f\n", perIter, currConf);

		}
	}
}

/*calculate person's proximity to TU's LKL given person's CM*/
double proximitytoLKL(myPoint currCM) { 
	myPoint LKL = circBuff.returnLKL();
	double xLKL = LKL.getImageX(), zLKL = LKL.getWorldZ();
	double xCM = currCM.getImageX(), zCM = currCM.getWorldZ();
	double prox2LKL = sqrt(pow((xLKL - xCM), 2.0) + pow((zLKL - zCM), 2.0));
	return prox2LKL;
}

void targetUserFound(PXCPersonTrackingModule* personModule) {
	/* Target User has been found, update ULV/LKL and figure out controls */
	PXCPersonTrackingData::Person* personData = personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, TU_pID);
	assert(personData != NULL);

	PXCPersonTrackingData::PersonTracking::PointCombined centerMass = personData->QueryTracking()->QueryCenterMass();
	myPoint myCenterMass(centerMass.world.point.x, centerMass.world.point.y, centerMass.world.point.z, centerMass.image.point.x, centerMass.image.point.y);
	double* controls = determineControls(myCenterMass);
	printf("Left motor rotational freq: %f, Right motor rotational freq: %f (rad/s)\n",controls[0],controls[1]);
	//THIS IS WHERE WE ACTUALLY SEND OUT CONTROLS
	circBuff.updateULV(myCenterMass, robVector);
}

double* determineControls(myPoint destination) { //uses x and z position of destination to determine rotational freq values for left and right motors
	double x = destination.getImageX()-160;
	double z = destination.getWorldZ();
	double theta = atan(z / x);
	double followingDistance = convertFeetToRSU(5.0);
	double wheelSeparation = convertFeetToRSU(ATV_WIDTH);
	double len = sqrt(pow(x,2.0)+pow(z,2.0)) - followingDistance; 
	//robVector[0] = len*cos(theta);
	//robVector[1] = len*sin(theta); //incorporate after ATV can actually move
	robVector[0, 1] = 0; //initially robot does not move
	double dispR = sqrt(pow(len*sin(theta) - (wheelSeparation/2)*cos(theta), 2) + pow(len*cos(theta) + (wheelSeparation/2)*(sin(theta)+1), 2)); //right motor displacement (rsu)
	double dispL = sqrt(pow(len*sin(theta) + (wheelSeparation/2)*cos(theta), 2) + pow(len*cos(theta) - (wheelSeparation/2)*(sin(theta)+1), 2)); //left motor displacement (rsu)
	//FIGURE OUT frameRate
	double wR = convertRSUToFeet(dispR) * frameRate / WHEEL_RADIUS;
	double wL = convertRSUToFeet(dispL) * frameRate / WHEEL_RADIUS;
	double controls[2] = { wR,wL };
	return controls;
	//BE WARY OF UNITS!!!
}

double convertFeetToRSU(double feet) {
	//FIGURE THIS OUT
	double rsu = feet*280 + 183.33;
	return rsu;
}

double convertRSUToFeet(double rsu) {
	//FIGURE THIS OUT
	double feet = rsu*0.0035 - 0.588;
	return feet;
}

/* Determines if targetUser is in the FOV */
boolean containsTargetUser(PXCPersonTrackingModule* personModule) {

	int numPersons = personModule->QueryOutput()->QueryNumberOfPeople();
	for (int i = 0; i < numPersons; i++) {

		PXCPersonTrackingData::Person* personData = personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, i);

		/* Finds the unique ID of each user */
		int uniqueID = personData->QueryTracking()->QueryId();

		if (uniqueID == TU_uID) {
			TU_pID = i;
			return true;
		}
	}

	return false;
}