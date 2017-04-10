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
#include "SerialClass.h"
#include "myArduinoCommunication.h"
#include <vector>

/* Required for ouputting data to file */
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <stdlib.h>
#include <sstream>

using namespace std;


#define MAX_INITIALIZE_COUNT 100
#define ATV_WIDTH 1.5 //18 inches
#define WHEEL_RADIUS 0.25 //3 inches


/* Scales 0 to 100 */
#define NO_UID_THRESHOLD 25 //0.3*PROXIMITY + 0.2*MEASUREMENTS, starting with 35
#define MAX_POSSIBLE_DISPLACEMENT 30 //75% of .4 * proximity corresponds to .244 feet
#define ALL_PARAMS_THRESHOLD 70 
#define NO_MEAS_THRESHOLD 75 //PROX AND UID

#define PROXIMITY_EXP_DECAY 0.288 //Decay variable calculated from average frame rate expected user movement

/* Make sure to check the Arduino port before usage */
#define ARDUINO_PORT "COM3"


/* Definitions for the console */
#define CONSOLE_WIDTH 60
#define CONSOLE_HEIGHT 20
HANDLE wHnd;    // Handle to write to the console.
HANDLE rHnd;    // Handle to read from the console.

int initializeCount = 0;
PXCSession *session = NULL;

/* Variables needed to compile */
pxcCHAR fileName[1024] = { 0 };
HANDLE ghMutex;

/* Global variables used in target identification */
double frameRate = 30.0; //adjust as needed
myPerson targetUser;
int TU_uID = 0; //TU's uID always starts at 0, this value can only change after comparePeopleinFOV
bool isInitialized = false;
int numPeopleFound = -1;   //make this non global
int totalPeopleFound = 0;
double targetUserTorsoHeight;
double targetUserShoulderWidth;


/* Global variable declarations for serial communications to Arduino */
//Serial* SP = new Serial(ARDUINO_PORT);
//char *toWrite;
//boolean success = true; //tracks if info successfully sent
//int nbChar = 1; //all messages sent is 1 char long
unsigned int leftMotor = 0;
unsigned int rightMotor = 0;


/*ULV variables*/
double robVector[2];
circleBuffer circBuff;

/*pID is the ID of each user in the frame beginning from 0
uID is a uniquely generated ID*/

/* Method declarations */ 
boolean initializeTargetUser(PXCPersonTrackingModule* personModule);
int comparePeopleInFOV(PXCPersonTrackingModule* personModule, int numPeople, int pID); //returns updated pID of TU after comparison
//myPerson convertPXCPersonToMyPerson(PXCPersonTrackingData::Person* person);
int targetUserpID(PXCPersonTrackingModule* personModule); //returns -1 if TU uID not located
double proximitytoLKL(myPoint currCM); //TODO
double convertProximityToConfidenceScore(double);
void targetUserFound(PXCPersonTrackingModule* personModule, int pID);
void targetUserNotFound();
void determineControls(myPoint destination);
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

	/* Initializing Arduino serial communication */
	toWrite = new char[1]; //allocates memory for out character buffer
	*toWrite = 0; //sets it equal to zero

	if (SP->IsConnected())
		printf("Connected to %s\n", ARDUINO_PORT);
	else {
		printf("Cannot find Arduino, please try again.\n");
	}


	/* Creates stream renders */
	UtilRender renderc(L"Color"), renderd(L"Depth");
	pxcStatus sts;

	/* Creates render windows */
	HWND colorWindow = renderc.m_hWnd;
	HWND depthWindow = renderc.m_hWnd;


	/* Creates console handles */
	wHnd = GetStdHandle(STD_OUTPUT_HANDLE);
	rHnd = GetStdHandle(STD_INPUT_HANDLE);

	/* Change the window title */
	SetConsoleTitle(TEXT("userID"));

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
					//printf("Person Module is NULL, not initialized\n");
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

		Sleep(100000);

		//printf("torsoHeight-----------\n");
		//targetUser.printVector(targetUser.getTorsoVector());

		//printf("shoulderWidth--------\n");
		//targetUser.printVector(targetUser.getShoulderDistanceVector());



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
					//printf("Person Module is null, man\n");
					continue;
				}

				int numPeople = personModule->QueryOutput()->QueryNumberOfPeople();

				/* Found a person */

				if (numPeople == 0) {
					targetUserNotFound();
					//CALL targetUserNotFound, SET LKL AS DESTINATION, DETERMINE CONTROLS, MAKE NOISE
				}
				else if (numPeople == 1) {
					int TU_pID = targetUserpID(personModule);
					//printf("numPeople = %d\n", numPeople);
					if (TU_pID == -1) { //if pID is -1, TU's uID not found
						int updatedpID = comparePeopleInFOV(personModule, numPeople, TU_pID); //only when threshold is reached
						if (updatedpID == -1) {
							targetUserNotFound();
						}
						else {
							targetUserFound(personModule, updatedpID);




						}
						//printf("contains secondary user... z = %f\n", personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, 0)->QueryTracking()->QueryCenterMass().world.point.z);
					}
					else { //assume if only one person in frame and targetUserpID shows that uID is maintained, TU is person in frame
						targetUserFound(personModule, TU_pID);
						//printf("contains target user...   z = %f\n", personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, 0)->QueryTracking()->QueryCenterMass().world.point.z);
					}
				}
				/*Multiple persons in frame*/
				else {
					printf("numPeople = %d\n", numPeople);
					int TU_pID = targetUserpID(personModule);
					printf("numPeople = %d\n", numPeople);
					int updatedpID = comparePeopleInFOV(personModule, numPeople, TU_pID); //pID updates only when threshold is reached
					if (updatedpID == -1) {
						targetUserNotFound();
					}
					else {
						targetUserFound(personModule, updatedpID);
					}
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

		printf("torsoHeight         = %.2f\n", targetUser.getTorsoHeight());
		printf("torsoShoulder Ratio = %.2f\n", targetUser.getTorsoShoulderRatio());

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



/* Iterates across all people in FOV, compares against target user
Returns pID of person most likely to be TU given the associated confidence value surpasses
a set threshold */
int comparePeopleInFOV(PXCPersonTrackingModule* personModule, int numPeople, int pID) {
	/*user parameters*/
	double proximity; 
	double sameuID;
	double meas;
	int pID_of_TU=-1; //default updated pID of target user, remains -1 if TU not in frame

	vector<double> userConf (numPeople,0); //creates vector of confidence values for each person in frame
	vector<double> userConfMeas(numPeople, 0); //creates vector of confidence values for each person in frame
	bool measIncluded = TRUE; //if joint information is unavailable, a different confidence calculation is utilized
	
	/* Iterates across all people in FOV */
	for(int perIter = 0; perIter < numPeople; perIter++) {

		/*Access Joint Info*/
		PXCPersonTrackingData::Person* personData = personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, perIter);
		assert(personData != NULL);
		PXCPersonTrackingData::PersonJoints* personJoints = personData->QuerySkeletonJoints();
		PXCPersonTrackingData::PersonJoints::SkeletonPoint* joints = new PXCPersonTrackingData::PersonJoints::SkeletonPoint[personJoints->QueryNumJoints()];
		personJoints->QueryJoints(joints);
		if (perIter == pID) { sameuID = 100.0; }
		else { sameuID = 0.0; }
		
		/*Include Measurement Stuff if available*/
		if (isJointInfoValid(joints) == false) {
			//printf("Invalid joint data...no comparison\n");
			PXCPersonTrackingData::PersonTracking::PointCombined centerMass = personData->QueryTracking()->QueryCenterMass();
			myPoint myCenterMass(centerMass.world.point.x, centerMass.world.point.y, centerMass.world.point.z, centerMass.image.point.x, centerMass.image.point.y);
			measIncluded = FALSE; //invalid joint info for at least one person

			/*LKL Proximity*/
			proximity = proximitytoLKL(myCenterMass);
		}
		else {
			myPoint head(joints[2].world.x, joints[2].world.y, joints[2].world.z, joints[2].image.x, joints[2].image.y);
			myPoint shoulderLeft(joints[4].world.x, joints[4].world.y, joints[4].world.z, joints[4].image.x, joints[4].image.y);
			myPoint shoulderRight(joints[5].world.x, joints[5].world.y, joints[5].world.z, joints[5].image.x, joints[5].image.y);
			myPoint spineMid(joints[3].world.x, joints[3].world.y, joints[3].world.z, joints[3].image.x, joints[3].image.y);
			
			PXCPersonTrackingData::PersonTracking::PointCombined centerMass = personData->QueryTracking()->QueryCenterMass();
			myPoint myCenterMass(centerMass.world.point.x, centerMass.world.point.y, centerMass.world.point.z, centerMass.image.point.x, centerMass.image.point.y);
			myPerson curr = myPerson(head, shoulderLeft, shoulderRight, spineMid, myCenterMass);

			/*LKL Proximity*/
			proximity = proximitytoLKL(myCenterMass);
			
			/*Torso Ratio Comparison*/
			meas = compareTorsoRatio(curr, targetUser);
		}

		/*Adding Confidence Values to Vectors*/
		if (measIncluded) {
			double confM = (0.3*proximity + 0.2*meas + 0.5*sameuID); //confidence incorporating joint measurements
			double conf = (0.4*proximity + 0.6*sameuID); //confidence excluding joints
			userConfMeas[perIter] = confM;
			userConf[perIter] = conf; //add confidence values to vector
		}
		else {
			double conf = (0.4*proximity + 0.6*sameuID); //confidence excluding joints
			userConf[perIter] = conf; //add confidence value to vector
		}
	}

	/*determine if one of the new people in frame is the TU and return pID*/
	if (pID == -1) {
		if (measIncluded) { //no uID param available, only LKL and measurement
			double confidence = *max_element(userConfMeas.begin(), userConfMeas.end()); //0.3 prox, 0.2meas
			if (confidence >= NO_UID_THRESHOLD) {
				int pID_of_TU = distance(userConfMeas.begin(), max_element(userConfMeas.begin(), userConfMeas.end()));
			}
		}
		else { //no uID or measurement params available, only LKL
			double confidence = *max_element(userConf.begin(), userConf.end()); //utilize max possible distance traveled for threshold max 0.4
			if (confidence >= MAX_POSSIBLE_DISPLACEMENT) {
				int pID_of_TU = distance(userConf.begin(), max_element(userConf.begin(), userConf.end()));
			}
		}
	}
	else {
		if (measIncluded) {
			double confidence = *max_element(userConfMeas.begin(), userConfMeas.end()); //0.3 prox, 0.2meas, 0.5 uID
			if (confidence >= ALL_PARAMS_THRESHOLD) {
				int pID_of_TU = distance(userConfMeas.begin(), max_element(userConfMeas.begin(), userConfMeas.end()));
			}
		}
		else {
			double confidence = *max_element(userConfMeas.begin(), userConfMeas.end()); //0.4 prox, 0.6 uID
			if (confidence >= NO_MEAS_THRESHOLD) {
				int pID_of_TU = distance(userConf.begin(), max_element(userConf.begin(), userConf.end()));
			}
		}
	}
	return pID_of_TU;
}

/*calculate person's proximity to TU's LKL given person's CM*/
double proximitytoLKL(myPoint currCM) { 
	myPoint LKL = circBuff.returnLKL();
	double xLKL = LKL.getWorldX(), zLKL = LKL.getWorldZ()*1000;
	double xCM = currCM.getWorldX(), zCM = currCM.getWorldZ()*1000;
	xCM += robVector[0], zCM += robVector[1];
	double prox2LKL = sqrt(pow((xLKL - xCM), 2.0) + pow((zLKL - zCM), 2.0)); //returns RSU
	prox2LKL = convertRSUToFeet(prox2LKL);
	prox2LKL = convertProximityToConfidenceScore(prox2LKL);
	return prox2LKL;
}

/* Convert person's proximity to TU's LKL into a score from 0 (Low score) to 100 (High score).
   A higher score indicates that we are more confident selected person is the TU */
double convertProximityToConfidenceScore(double proximityToLKL) {  //requires input in feet
	double neg = -1 * proximityToLKL;
	double eToPower = pow(E, neg);
	return eToPower;
}

void targetUserFound(PXCPersonTrackingModule* personModule,int pID) {
	/* Target User has been found, update ULV/LKL and figure out controls */
	PXCPersonTrackingData::Person* personData = personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, pID);
	assert(personData != NULL);

	PXCPersonTrackingData::PersonTracking::PointCombined centerMass = personData->QueryTracking()->QueryCenterMass();
	myPoint myCenterMass(centerMass.world.point.x, centerMass.world.point.y, centerMass.world.point.z, centerMass.image.point.x, centerMass.image.point.y);
	double* controls = new double[2];
	determineControls(myCenterMass);

	/* Reset position in console */
	COORD cursorPos = { 2, 10 };
	SetConsoleCursorPosition(wHnd, cursorPos);
	//printf("userID = %d     totalUsersFound = %d\n", TU_uID, totalPeopleFound);
	printf("leftMotor = %d    rightMotor = %d", leftMotor, rightMotor);
	//printf("  Rotational freq (rot/s)    leftMotor = %.2f Right motor = %.2f\n", controls[1] / (2* 3.14) , controls[0] / (2 * 3.14));
	//printf("  Rotational freq (rad/s)    leftMotor = %.2f Right motor = %.2f\n", controls[0], controls[1]);
	//printf("  centerMass (x, z) = (%.2f, %.2f)", centerMass.world.point.x, centerMass.world.point.z);
	

	cursorPos = { 2, 15 };
	SetConsoleCursorPosition(wHnd, cursorPos);
	printf("                ");

	//THIS IS WHERE WE ACTUALLY SEND OUT CONTROLS
	circBuff.updateULV(myCenterMass, robVector);
}

void targetUserNotFound() {
	/*Target User has not been found in the frame, set LKL as destination and sound alarm*/
	myPoint LKL = circBuff.returnLKL();
	determineControls(LKL);
	//START BEEPING MOTHAFUCKAAAA
	COORD cursorPos = { 2, 15 };
	SetConsoleCursorPosition(wHnd, cursorPos);
	printf("TU not found\n");
}


/* Uses x and z distance from TU to determine ATV's directional movement */
void determineControls(myPoint centerMass) { 
	int x = centerMass.getImageX();
	if (x < 120) { //turn left
		turnLeft();
		sendControls();
	}
	else if (x >= 120 && x <= 200) { //go forward
		goForward();
		sendControls();
	}
	else { //x > 200...turn right
		turnRight();
		sendControls();
	}
}

/*linear relationship between Feet and RealSense Units*/
double convertFeetToRSU(double feet) { 
	double rsu = feet*280 + 183.33;
	return rsu;
}
double convertRSUToFeet(double rsu) {
	double feet = rsu*0.0035 - 0.588;
	return feet;
}

/* Determines if targetUser is in the FOV and returns pID of TU*/
int targetUserpID(PXCPersonTrackingModule* personModule) {

	int numPersons = personModule->QueryOutput()->QueryNumberOfPeople();
	for (int i = 0; i < numPersons; i++) {

		PXCPersonTrackingData::Person* personData = personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, i);

		/* Finds the unique ID(uID) of each user */
		int uniqueID = personData->QueryTracking()->QueryId();
		
		/* Tracks how many people found during program operation */
		if (uniqueID > totalPeopleFound) {
			totalPeopleFound++;
		}

		/* Returns the pID of TU */
		if (uniqueID == TU_uID) {
			return i;
		}
	}

	return -1;
}