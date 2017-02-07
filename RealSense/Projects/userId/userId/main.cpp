#include <Windows.h>
#include <WinUser.h>
#include "pxccapturemanager.h"
#include "pxcsensemanager.h"
#include "pxcmetadata.h"
#include "util_cmdline.h"
#include "util_render.h"
#include <conio.h>
#include "pxcpersontrackingmodule.h"
#include "pxcpersontrackingdata.h"
#include "pxcvideomodule.h"
#include "pxcpersontrackingconfiguration.h"
#include "pxcmetadata.h"
#include "service/pxcsessionservice.h"
#include <assert.h>
#include "myPerson.h"

/* Required for ouputting data to file */
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <stdlib.h>
#include <sstream>

using namespace std;

#define COUNT_WIDTH 15
#define COLUMN_WIDTH 35
#define VECTOR_WIDTH 6



PXCSession *session = NULL;

/* Variables needed to compile */
pxcCHAR fileName[1024] = { 0 };
HANDLE ghMutex;
volatile bool isStopped = false;

/* Global variables used in target identification */
myPerson targetUser;
bool isInitialized = false;

/* Global variables for logging joint data */
char separator = ' ';
int timeCounter = 0;
ofstream jointLog;
ofstream torsoLog;


/* Method declarations */
void initializeTargetUser(PXCPersonTrackingModule* personModule);
boolean isJointInfoValid(PXCPersonTrackingData::PersonJoints::SkeletonPoint* joints);
void comparePeopleInFOV(PXCPersonTrackingModule* personModule, int numPeople);
myPerson convertPXCPersonToMyPerson(PXCPersonTrackingData::Person* person);
void updateTargetUser(PXCPersonTrackingModule* personModule);

void createJointLogFile(string fileName);
void createVectorLogFile(string fileName, string feature);
string pointToString(myPoint point);
void printToJointLog(myPerson person);
void printToVectorLog(vector<double> vect);


int main(int argc, WCHAR* argv[]) {
	/* Setting up log file */
	//createJointLogFile("pointLogs/pointLog8.txt");
	createVectorLogFile("torsoLogs/torsoLog1.txt", "torso");
	
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


		printf("Initializing stream...");
		/* Stream Data */
		while (true) {
			/* Waits until new frame is available and locks it for application processing */
			sts = pp->AcquireFrame(false);

			/* Render streams*/
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
				if (numPeople != 0) {

					/* Initializing target user */
					if (isInitialized == false) {
						initializeTargetUser(personModule);
						//printf("Still initializing user...\n");

					}
					/* Once target user initialized, update the torso height */
					else {
						updateTargetUser(personModule);
						printToVectorLog(targetUser.getTorsoVector());

					}
					/* Comparing people in FOV against target user */
					/*else {
						//comparePeopleInFOV(personModule, numPeople);
						/* Person initialized, time to gather data 
						if (timeCounter < 500) {
							printf("timeCounter = %d", timeCounter);
							PXCPersonTrackingData::Person* personData = personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, 0);
							printToLog(convertPXCPersonToMyPerson(personData));
							//timeCounter++; it increases inside a convertPXCPersonToMyPerson
						}
					}*/
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
	jointLog.close();
	torsoLog.close();
	return 0;
}







/* We should average our tracked joints over 5 seconds or something, removing outliers (median calculated values)
, initializing shouldn't occur in one frame*/
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

/* Checks to see if gathered joint info is valid data */
boolean isJointInfoValid(PXCPersonTrackingData::PersonJoints::SkeletonPoint* joints) {
	/* Not valid data, return false */
	if (joints[0].jointType != 6 || joints[1].jointType != 7 || joints[2].jointType != 10 || joints[3].jointType != 19 || joints[4].jointType != 16 || joints[5].jointType != 17) {
		return false;
		printf("Garbage data...\n");
	}
	if (joints[0].image.x == 0 && joints[0].image.y == 0) { //no image coordinates for left hand, skips initialization
		printf("Invalid left hand...\n");
		return false;
	}
	if (joints[1].image.x == 0 && joints[1].image.y == 0) { //no image coordinates for right hand, skips initialization
		printf("Invalid right hand...\n");
		return false;
	}
	if (joints[3].image.x == 0 && joints[3].image.y == 0) { //no image coordinates for left shoulder, skips initialization
		printf("Invalid left shoulder...\n");
		return false;
	}
	if (joints[4].image.x == 0 && joints[4].image.y == 0) { //no image coordinates for right shoulder, skips initialization
		printf("Invalid right shoulder...\n");
		return false;
	}

	/* All testsed issues passed, joint info is valid */
	return true;
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
			//printf("Invalid joint data...\n");
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
			double currConf = compareTorsoAndArmLengths(curr, targetUser); //confidence that current person is user
			printf("%d. Similarity = %.2f\n", perIter, currConf);

		}
	}
}

void createJointLogFile(string string) {
	jointLog.open(string);

	jointLog << "\n";
	jointLog << "Beginning joint data output...";
	jointLog << "\n\n\n";

	jointLog << left << setw(COUNT_WIDTH) << setfill(separator) << "Time";
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << "JOINT_HEAD";
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << "JOINT_SHOULDER_LEFT";
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << "JOINT_SHOULDER_RIGHT";
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << "JOINT_HAND_LEFT";
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << "JOINT_HAND_RIGHT";
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << "JOINT_SPINE_MID";
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << "JOINT_CENTER_MASS";

	jointLog << "\n";
}

/* Creates the file to store our vector data */
void createVectorLogFile(string fileName, string featureType) {
	torsoLog.open(fileName);

	torsoLog << "\n";
	torsoLog << "Beginning " << featureType << " data output...";
	torsoLog << "\n\n\n";
}

/* Takes a myPerson as input, prints it to the joint log file */
void printToJointLog(myPerson newPerson) {
	jointLog << left << setw(COUNT_WIDTH) << setfill(separator) << timeCounter;
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << pointToString(newPerson.getHead());
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << pointToString(newPerson.getLeftShoulder());
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << pointToString(newPerson.getRightShoulder());
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << pointToString(newPerson.getLeftHand());
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << pointToString(newPerson.getRightHand());
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << pointToString(newPerson.getSpineMid());
	jointLog << left << setw(COLUMN_WIDTH) << setfill(separator) << pointToString(newPerson.getCenterMass());
	jointLog << "\n";
}

/* Prints the entire vector onto one line in the vector log file */
void printToVectorLog(vector<double> vect) {
	
	torsoLog << "Unsorted: ";
	for (vector<double>::iterator it = vect.begin(); it != vect.end(); it++) {
		torsoLog << left << setprecision(4) << setw(VECTOR_WIDTH) << setfill(separator) << *it;
	}
	torsoLog << "\n";
	torsoLog << "Sorted:   ";
	sort(vect.begin(), vect.end());
	for (vector<double>::iterator it = vect.begin(); it != vect.end(); it++) {
		torsoLog << left << setprecision(4) << setw(VECTOR_WIDTH) << setfill(separator) << *it;
	}
	torsoLog << "\n median: ";
	torsoLog << findMedian(vect);
	torsoLog << "\n";


}



/* Returns x and y coordinates of images, z coordinate from world */
string pointToString(myPoint point) {
	point.printPoint();
	stringstream ss;

	ss << left << setprecision(5) << setw(5) << point.getImageX();
	ss << left << ", ";
	ss << left << setprecision(5) << setw(5) << point.getImageY();
	ss << left << ", ";
	ss << left << setprecision(5) << setw(5) << point.getWorldZ();
	return ss.str(); //converts stringStream to a string
}

myPerson convertPXCPersonToMyPerson(PXCPersonTrackingData::Person* personData) {
	assert(personData != NULL);
	PXCPersonTrackingData::PersonJoints* personJoints = personData->QuerySkeletonJoints();
	/* Initializes null person */
	myPerson newPerson;

	PXCPersonTrackingData::PersonJoints::SkeletonPoint* joints = new PXCPersonTrackingData::PersonJoints::SkeletonPoint[personJoints->QueryNumJoints()];
	personJoints->QueryJoints(joints);

	PXCPersonTrackingData::PersonTracking::PointCombined centerMass = personData->QueryTracking()->QueryCenterMass();
	myPoint myCenterMass(centerMass.world.point.x, centerMass.world.point.y, centerMass.world.point.z, centerMass.image.point.x, centerMass.image.point.y);
	
	/* Joint info invalid, returns null user */
	if (isJointInfoValid(joints) == false) {
		printf("Converstion unsuccessful, outputting null person...\n");
	}
	/* Joint info is valid,c changes newPerson to have valid joint data */
	else {
		printf("Conversion successful, outputting to log...\n");
		myPoint leftHand      (joints[0].world.x, joints[0].world.y, joints[0].world.z, joints[0].image.x, joints[0].image.y);
		myPoint rightHand     (joints[1].world.x, joints[1].world.y, joints[1].world.z, joints[1].image.x, joints[1].image.y);
		myPoint head          (joints[2].world.x, joints[2].world.y, joints[2].world.z, joints[2].image.x, joints[2].image.y);
		myPoint shoulderLeft  (joints[4].world.x, joints[4].world.y, joints[4].world.z, joints[4].image.x, joints[4].image.y);
		myPoint shoulderRight (joints[5].world.x, joints[5].world.y, joints[5].world.z, joints[5].image.x, joints[5].image.y);
		myPoint spineMid      (joints[3].world.x, joints[3].world.y, joints[3].world.z, joints[3].image.x, joints[3].image.y);
		newPerson.changeJoints(head, shoulderLeft, shoulderRight, leftHand, rightHand, spineMid,  myCenterMass);
	}
	timeCounter++;
	delete[] joints;
	return newPerson;

}

void updateTargetUser(PXCPersonTrackingModule* personModule) {
	printf("Updating the target user...\n");
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
		printf("median torsoHeight: %f\n", targetUser.getMedianTorsoHeight());
	}


}