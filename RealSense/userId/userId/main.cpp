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
#include "myLogging.h"

/* Required for ouputting data to file */
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <stdlib.h>
#include <sstream>

using namespace std;



PXCSession *session = NULL;

/* Variables needed to compile */
pxcCHAR fileName[1024] = { 0 };
HANDLE ghMutex;
volatile bool isStopped = false;

/* Global variables used in target identification */
myPerson targetUser;
bool isInitialized = false;
//vector<int> personList;
int numPeopleFound = -1;
int targetUserCurrID = 0;  //target user is always the first user found, aka targetID 0

/* Global variables for logging joint data */
//char separator = ' ';
int timeCounter = 0;
ofstream jointLog;
ofstream torsoLog;
ofstream leftArmLog;
ofstream rightArmLog;


/* Method declarations */
void initializeTargetUser(PXCPersonTrackingModule* personModule);
void comparePeopleInFOV(PXCPersonTrackingModule* personModule, int numPeople);
myPerson convertPXCPersonToMyPerson(PXCPersonTrackingData::Person* person);
void updateTargetUser(PXCPersonTrackingModule* personModule);
boolean isNewUser(PXCPersonTrackingModule *personModule);
boolean containsTargetUser(PXCPersonTrackingModule* personModule);


int main(int argc, WCHAR* argv[]) {
	/* Setting up log file */
	//createJointLogFile("pointLogs/pointLog14.txt");
	//createVectorLogFile("torsoLogs/torsoLog14.txt", "torso",torsoLog);
	//createVectorLogFile("leftArmLogs/leftArmLog14.txt", "leftArm",leftArmLog);
	//createVectorLogFile("rightArmLogs/rightArmLog14.txt", "rightArm",rightArmLog);
	//createVectorLogFile("zAxisLogs/zAxisLog3.txt", "zAxis", zAxisLog);
	
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
					continue;
				}

				int numPeople = personModule->QueryOutput()->QueryNumberOfPeople();

				/* Found a person */
				if (numPeople == 0) {
					//do nothing, no persons found. Here we want to make a noise and try to find the targetUser
				}
				else if (numPeople == 1) {

					/* Initializing target user */
					if (isInitialized == false) {
						initializeTargetUser(personModule);
						//printf("Still initializing user...\n");

					}

					if (containsTargetUser(personModule)) {
						printf("contains target user...\n");
					}
					else {
						printf("contains secondary user \n");
					}

				}

				else {
					printf("more than 1 person in FOV\n");
				}
					/* Once target user initialized, update the torso height */


				


				//	else {
					//	updateTargetUser(personModule);

						//if (isNewUser(personModule)) {
							/* Comparing people in FOV against target user */
						//	comparePeopleInFOV(personModule, numPeople);
						//}

						
						/* printing information to log files */
						//printToVectorLog(targetUser.getTorsoVector(),torsoLog);
						//printToVectorLog(targetUser.getLeftArmVector(),leftArmLog);
						//printToVectorLog(targetUser.getRightArmVector(),rightArmLog);
					//	PXCPersonTrackingData::Person* personData = personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, 0);
						//printToJointLog(convertPXCPersonToMyPerson(personData));


					//}
				//}
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
	jointLog.close();
	torsoLog.close();
	leftArmLog.close();
	rightArmLog.close();
	return 0;
}







/* We should average our tracked joints over 5 seconds or something, removing outliers (median calculated values),
initializing shouldn't occur in one frame*/
void initializeTargetUser(PXCPersonTrackingModule* personModule) {
	/* Accesses the only person in camera's FOV, our target user */
	PXCPersonTrackingData::Person* personData = personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, 0);
	assert(personData != NULL);

	/* Queries for skeleton joint data */
	PXCPersonTrackingData::PersonJoints* personJoints = personData->QuerySkeletonJoints();

	PXCPersonTrackingData::PersonJoints::SkeletonPoint* joints = new PXCPersonTrackingData::PersonJoints::SkeletonPoint[personJoints->QueryNumJoints()];
	personJoints->QueryJoints(joints);

	if (isJointInfoValid(joints) == false) {
		//printf("Invalid jointType data...\n");
	}
	/* Joint info is valid, initialize target user */
	else {
		printf("Initializing target user...\n");
		myPoint leftHand      (joints[0].world.x, joints[0].world.y, joints[0].world.z, joints[0].image.x, joints[0].image.y);
		myPoint rightHand     (joints[1].world.x, joints[1].world.y, joints[1].world.z, joints[1].image.x, joints[1].image.y);
		myPoint head          (joints[2].world.x, joints[2].world.y, joints[2].world.z, joints[2].image.x, joints[2].image.y);
		myPoint shoulderLeft  (joints[4].world.x, joints[4].world.y, joints[4].world.z, joints[4].image.x, joints[4].image.y);
		myPoint shoulderRight (joints[5].world.x, joints[5].world.y, joints[5].world.z, joints[5].image.x, joints[5].image.y);
		myPoint spineMid      (joints[3].world.x, joints[3].world.y, joints[3].world.z, joints[3].image.x, joints[3].image.y);
		
		PXCPersonTrackingData::PersonTracking::PointCombined centerMass = personData->QueryTracking()->QueryCenterMass();
		myPoint myCenterMass(centerMass.world.point.x, centerMass.world.point.y, centerMass.world.point.z, centerMass.image.point.x, centerMass.image.point.y);
		
		targetUser.changeJoints(head, shoulderLeft, shoulderRight, leftHand, rightHand, spineMid, myCenterMass);
		targetUser.printPerson();
		isInitialized = true;
	}

	/* Frees up space allocated for joints array, it is not needed anymore */
	delete[] joints;
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
			myPoint leftHand        (joints[0].world.x, joints[0].world.y, joints[0].world.z, joints[0].image.x, joints[0].image.y);
			myPoint rightHand       (joints[1].world.x, joints[1].world.y, joints[1].world.z, joints[1].image.x, joints[1].image.y);
			myPoint head            (joints[2].world.x, joints[2].world.y, joints[2].world.z, joints[2].image.x, joints[2].image.y);
			myPoint shoulderLeft    (joints[4].world.x, joints[4].world.y, joints[4].world.z, joints[4].image.x, joints[4].image.y);
			myPoint shoulderRight   (joints[5].world.x, joints[5].world.y, joints[5].world.z, joints[5].image.x, joints[5].image.y);
			myPoint spineMid        (joints[3].world.x, joints[3].world.y, joints[3].world.z, joints[3].image.x, joints[3].image.y);


			myPerson curr = myPerson(head, shoulderLeft, shoulderRight, leftHand, rightHand, spineMid, myCenterMass);
			//curr.printPerson(); //can implement while testing
			double currConf = compareTorsoRatio(curr, targetUser); //confidence that current person is user
			printf("%d. Similarity = %.2f\n", perIter, currConf);

		}
	}
}


void updateTargetUser(PXCPersonTrackingModule* personModule) {
	//printf("Updating the target user...\n");
	/* Accesses the only person in camera's FOV, our target user */
	PXCPersonTrackingData::Person* personData = personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, 0);
	assert(personData != NULL);

	/* Queries for skeleton joint data */
	PXCPersonTrackingData::PersonJoints* personJoints = personData->QuerySkeletonJoints();

	PXCPersonTrackingData::PersonJoints::SkeletonPoint* joints = new PXCPersonTrackingData::PersonJoints::SkeletonPoint[personJoints->QueryNumJoints()];
	personJoints->QueryJoints(joints);
	
	if (isJointInfoValid(joints) == false) {
		//printf("Invalid jointType data...\n");
	}
	/* Joint info is valid, initialize target user */
	else {
		myPoint leftHand       (joints[0].world.x, joints[0].world.y, joints[0].world.z, joints[0].image.x, joints[0].image.y);
		myPoint rightHand      (joints[1].world.x, joints[1].world.y, joints[1].world.z, joints[1].image.x, joints[1].image.y);
		myPoint head           (joints[2].world.x, joints[2].world.y, joints[2].world.z, joints[2].image.x, joints[2].image.y);
		myPoint shoulderLeft   (joints[4].world.x, joints[4].world.y, joints[4].world.z, joints[4].image.x, joints[4].image.y);
		myPoint shoulderRight  (joints[5].world.x, joints[5].world.y, joints[5].world.z, joints[5].image.x, joints[5].image.y);
		myPoint spineMid       (joints[3].world.x, joints[3].world.y, joints[3].world.z, joints[3].image.x, joints[3].image.y);

		PXCPersonTrackingData::PersonTracking::PointCombined centerMass = personData->QueryTracking()->QueryCenterMass();
		myPoint myCenterMass(centerMass.world.point.x, centerMass.world.point.y, centerMass.world.point.z, centerMass.image.point.x, centerMass.image.point.y);

		targetUser.updatePerson(head, shoulderLeft, shoulderRight, leftHand, rightHand, spineMid, myCenterMass);
		//printf("median torsoHeight: %f\n", targetUser.getMedianTorsoHeight());
		//printf("median leftArmLength: %f\n", targetUser.getMedianLeftArmLength());
		//printf("median rightArmLength: %f\n", targetUser.getMedianRightArmLength());
	}


}


/* Determines if persons seen in the camera have been seen before */
boolean isNewUser(PXCPersonTrackingModule *personModule) {

	//PXCPersonTrackingData::Person* personData = personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, 0);

	int numPersons = personModule->QueryOutput()->QueryNumberOfPeople();
	//int numPersons = personModulepersonData->QueryNumberOfPeople(); //returns number of people

	for (int i = 0; i < numPersons; i++) {
		PXCPersonTrackingData::Person* personData = personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, i);

		/* Finds the unique ID of each user */
		int uniqueID = personData->QueryTracking()->QueryId();
		//printf("uniqueID = %d numPeople=%d", uniqueID, numPeopleFound);
		/* If assigned ID greater than numPeopleFound, person hasn't been seen before */
		if (uniqueID > numPeopleFound) {
			printf("New user found ID = %d\n", uniqueID);
			numPeopleFound++;
			return true;
		}
	}
	return false;
}
	
/* Determines if targetUser is in the FOV */
boolean containsTargetUser(PXCPersonTrackingModule* personModule) {
	
	int numPersons = personModule->QueryOutput()->QueryNumberOfPeople();
	for (int i = 0; i < numPersons; i++) {

		PXCPersonTrackingData::Person* personData = personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, i);
		
		/* Finds the unique ID of each user */
		int uniqueID = personData->QueryTracking()->QueryId();
		
		if (uniqueID == targetUserCurrID) {
			return true;
		}
	}

	return false;
}