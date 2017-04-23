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
#include "myBuffer.h"

/* Required for ouputting data to file */
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <stdlib.h>
#include <sstream>

/* For threading */
#include <strsafe.h>


using namespace std;



PXCSession *session = NULL;

/* Variables needed to compile */
pxcCHAR fileName[1024] = { 0 };
HANDLE ghMutex;
volatile bool isStopped = false;

/* Global variables used in target identification */
myPerson targetUser;
bool isInitialized = false;
//int numPeopleFound = -1;

/* Global variables for logging joint data */
//char separator = ' ';
int timeCounter = 0;
ofstream jointLog;
ofstream torsoLog;
ofstream leftArmLog;
ofstream rightArmLog;

/* Global variables for the table */
HANDLE wHnd;    // Handle to write to the console.
HANDLE rHnd;    // Handle to read from the console.
HANDLE threadSuccess; //Handle to control output table thread
int consoleWidth = 60;
int consoleHeight = 20;
vector<PXCPersonTrackingData::Person*> personsFound;
myBuffer targetUserLocBuff; 
int targetUserCurrID = 0;
int targetUserMissingCount = 0;
#define MAX_TARGET_MISSING_COUNT 100



/* Method declarations */
void initializeTargetUser(PXCPersonTrackingModule* personModule);
void comparePeopleInFOV(PXCPersonTrackingModule* personModule, int numPeople);
myPerson convertPXCPersonToMyPerson(PXCPersonTrackingData::Person* person);
void updateTargetUser(PXCPersonTrackingModule* personModule);
boolean isNewUser(PXCPersonTrackingModule *personModule);
void createOutputTable();
void printTable(bool, double);
void ErrorHandler(LPTSTR lpszFunction);
DWORD WINAPI updateTable(LPVOID);
boolean containsTargetUser(PXCPersonTrackingModule* personModule);
boolean isTargetUserGone(PXCPersonTrackingModule* personModule);




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
		//printf("OpenCV coordinate mode enabled...\n");
	}
	else {
		//printf("OpenCV mode unsuccessful...\n");
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



		/* Creates the formatting and handles for our output table */
		createOutputTable();

		//printf("Initializing stream...\n");
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
					
					//printf("no Person");

					/* Target user already found, disappeared from FOV */
					if (personsFound.size() > 0) {
						isTargetUserGone(personModule); //increments personGoneCounter if necessary
						(personsFound[0]) = NULL;
					}
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
						if (isNewUser(personModule)) {
							/* Comparing people in FOV against target user */
							comparePeopleInFOV(personModule, numPeople);
						}
					}


						
						/* printing information to log files */
						//printToVectorLog(targetUser.getTorsoVector(),torsoLog);
						//printToVectorLog(targetUser.getLeftArmVector(),leftArmLog);
						//printToVectorLog(targetUser.getRightArmVector(),rightArmLog);
						//PXCPersonTrackingData::Person* personData = personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, 0);
						//printToJointLog(convertPXCPersonToMyPerson(personData));


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
	leftArmLog.close();
	rightArmLog.close();
	CloseHandle(threadSuccess); //closes table output thread
	return 0;
}







/* We should average our tracked joints over 5 seconds or something, removing outliers (median calculated values),
initializing shouldn't occur in one frame */
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
		//targetUser.printPerson();
		isInitialized = true;

		personsFound.push_back(personData); //puts first data into personsFound vector
		targetUserLocBuff.add(centerMass);

		/* Create thread to write values to the table */
		DWORD   dwThreadId;
		threadSuccess = CreateThread(NULL, 0, updateTable, NULL, 0, &dwThreadId);

		if (threadSuccess == NULL)
		{
			ErrorHandler(TEXT("CreateThread"));
			ExitProcess(3);
		}
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



	personsFound[0] = personData;

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
		
		targetUserLocBuff.add(centerMass);
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

		/* If assigned ID greater than the size of personFound, person hasn't been seen before */
		if (uniqueID > personsFound.size()) {
			printf("New user found ID = %d\n", uniqueID);
			//numPeopleFound++;
			/* Adding the new personData to our history of persons found */
			personsFound.push_back(personData);
			return true;
		}
	}
	return false;
}
	
void createOutputTable() {
	/* Set up the handles for reading/writing */
	wHnd = GetStdHandle(STD_OUTPUT_HANDLE);
	rHnd = GetStdHandle(STD_INPUT_HANDLE);

	/* Change the window title */
	SetConsoleTitle(TEXT("userID"));

	/* Set up the required window size */
	SMALL_RECT windowSize = { 0, 0, consoleHeight - 1, consoleWidth - 1 };

	/* Change the console window size */
	SetConsoleWindowInfo(wHnd, TRUE, &windowSize);

	/* Create a COORD to hold the buffer size */
	COORD bufferSize = { consoleHeight, consoleWidth };

	/* Change the internal buffer size */
	SetConsoleScreenBufferSize(wHnd, bufferSize);

	// Set up the character:
	CHAR_INFO border;
	border.Char.AsciiChar = '*';
	border.Attributes = FOREGROUND_GREEN | FOREGROUND_INTENSITY;

	// Set up the positions:

	for (int i = 0; i < consoleWidth; i++) {  //makes top border
		COORD charBufSize = { 1,1 };
		COORD characterPos = { 0 ,0 }; //draws in top left of buffer
		SMALL_RECT writeArea = { i,0,i,0 }; //top left coordinates, bottom right coordinates of drawing area
											// Write the character:
		WriteConsoleOutputA(wHnd, &border, charBufSize, characterPos, &writeArea);
	}

	for (int i = 0; i < consoleWidth; i++) { //makes bottom border
		COORD charBufSize = { 1,1 };
		COORD characterPos = { 0 ,0 }; //draws in top left of buffer
		SMALL_RECT writeArea = { i,consoleHeight,i,consoleHeight }; //top left coordinates, bottom right coordinates of drawing area
																	// Write the character:
		WriteConsoleOutputA(wHnd, &border, charBufSize, characterPos, &writeArea);
	}

	for (int i = 0; i < consoleHeight; i++) { //makes left border
		COORD charBufSize = { 1,1 };
		COORD characterPos = { 0 ,0 }; //draws in top left of buffer
		SMALL_RECT writeArea = { 0,i,0,i }; //top left coordinates, bottom right coordinates of drawing area
											// Write the character:
		WriteConsoleOutputA(wHnd, &border, charBufSize, characterPos, &writeArea);
	}

	for (int i = 0; i <= consoleHeight; i++) { //makes right border
		COORD charBufSize = { 1,1 };
		COORD characterPos = { 0 ,0 }; //draws in top left of buffer
		SMALL_RECT writeArea = { consoleWidth,i,consoleWidth,i }; //top left coordinates, bottom right coordinates of drawing area
																  // Write the character:
		WriteConsoleOutputA(wHnd, &border, charBufSize, characterPos, &writeArea);
	}

}

/* Parameter is void, can be any data type or no data at all. This function is executed by the thread */
DWORD WINAPI updateTable(LPVOID lpParam)
{
	//printf("thread started...");
	while (personsFound.size() > 0) {

		Sleep(100); //runs every tenth second
		printTable(true, 0.0); //prints all the persons in the table

	}
	return 0;
}

void printTable(bool targetFound, double targetUserVal) {

	//need to set cursor position before writing to screen 
	if (personsFound[0] == NULL) {
		//target user disappeared, mark that on output table
		COORD cursorPos = { 2, 1 };
		SetConsoleCursorPosition(wHnd, cursorPos);
		if (targetUserMissingCount >= MAX_TARGET_MISSING_COUNT) {
			printf("TargetUser LKL  "); printLocation(targetUserLocBuff.getLastLocation());
		}
	}
	else {
		COORD cursorPos = { 2, 1 };
		SetConsoleCursorPosition(wHnd, cursorPos);
		printf("TargetUser:    %f", personsFound[0]->QueryTracking()->QueryCenterMass().world.point.z);
		for (int personCount = 1; personCount < personsFound.size(); personCount++) {
			COORD cursorPos = { 2, personCount + 3 };
			SetConsoleCursorPosition(wHnd, cursorPos);
			printf("person %d:     %.1f", personCount, personsFound[personCount]->QueryTracking()->QueryCenterMass().world.point.z);
		}
	}
}

void ErrorHandler(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code.

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message.

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	// Free error-handling buffer allocations.

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}

/* Determines if targetUser is in the FOV */
boolean containsTargetUser(PXCPersonTrackingModule* personModule) {

	if (personModule == NULL) { //no users in FOV, therefore return false
		return false;
	}
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

/* Returns true if target user has been absent from FOV for a specified time */
boolean isTargetUserGone(PXCPersonTrackingModule* personModule) {
	
	if (targetUserMissingCount >= 100) { //user has been gone for a while, return true
		return true;
	}
	if (!containsTargetUser(personModule)) { //user is not in the current frame, increment counter
		targetUserMissingCount++;
	}
	else {  //user is in the frame, reset counter
		targetUserMissingCount = 0;
	}

}