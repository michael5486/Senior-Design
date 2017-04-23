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
#include "circleBuffer.h"
#include <Gdiplus.h>
#include <atlimage.h>
#include <math.h>
#pragma comment(lib,"gdiplus.lib")

/* Required for ouputting data to file */
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <iomanip>
#include <sstream>

using namespace std;

/*For initialization*/
#define MAX_INITIALIZE_COUNT 100
int initializeCount = 0;

PXCSession *session = NULL;

/* Variables needed to compile */
pxcCHAR fileName[1024] = { 0 };
HANDLE ghMutex;
volatile bool isStopped = false;

/* Global variables used in target identification */
myPerson targetUser;
bool isInitialized = false;
int numPeopleFound = -1;
int targetUserCurrID = 0;  //target user is always the first user found, aka targetID 0
double targetUserTorsoHeight;
double targetUserShoulderWidth;

/* Global variables for logging joint data
//char separator = ' ';
int timeCounter = 0;
ofstream jointLog;
ofstream torsoLog;
ofstream leftArmLog;
ofstream rightArmLog;

// Global variables for the table
HANDLE wHnd;    // Handle to write to the console.
HANDLE rHnd;    // Handle to read from the console.
HANDLE threadSuccess; //Handle to control output table thread
int consoleWidth = 60;
int consoleHeight = 20;
*/

double robVector[2];
circleBuffer circBuff;
boolean imgFlag = false; //only want to extract one frame for now


/* Method declarations */
boolean initializeTargetUser(PXCPersonTrackingModule* personModule);
void comparePeopleInFOV(PXCPersonTrackingModule* personModule, int numPeople);
myPerson convertPXCPersonToMyPerson(PXCPersonTrackingData::Person* person);
void updateTargetUser(PXCPersonTrackingModule* personModule);
boolean isNewUser(PXCPersonTrackingModule *personModule);
boolean isJointInfoValid(PXCPersonTrackingData::PersonJoints::SkeletonPoint *joints);
double proximitytoLKL(myPoint currCM);
//void createOutputTable();
//void printTable(bool, double);
//void ErrorHandler(LPTSTR lpszFunction);
//DWORD WINAPI updateTable(LPVOID);




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


		//printf("Initializing stream...\n");
		/* Stream Data */
		
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

		while (true) {
			/* Waits until new frame is available and locks it for application processing */
			sts = pp->AcquireFrame(false);

			/* Render streams */
			PXCCapture::Sample *sample = pp->QuerySample();

			if (sample) {
				//printf("running");
				PXCPersonTrackingModule* personModule = pp->QueryPersonTracking();

				/* If no persons are visible, renders and releases current frame */
				//IF personModule IS NULL, WE SHOULD BE CALLING userNotFound
				if (personModule == NULL) {
					if (sample->depth && !renderd.RenderFrame(sample->depth)) break;
					if (sample->color && !renderc.RenderFrame(sample->color)) break;
					pp->ReleaseFrame();
					
					//printf("no Person");

					continue;
				}

				int numPeople = personModule->QueryOutput()->QueryNumberOfPeople();

				/* Found a person */
				if (numPeople != 0) {
					if (!imgFlag) {
						PXCImage *colorIm;
						colorIm = sample->color;
						PXCImage::ImageData colorData;
						if (colorIm->AcquireAccess(PXCImage::ACCESS_READ, colorData.format, &colorData) >= PXC_STATUS_NO_ERROR) {
							PXCImage::ImageInfo colorInfo = colorIm->QueryInfo();
							auto colorPitch = colorData.pitches[0] / sizeof(pxcBYTE);
							ofstream frameBuf;
							frameBuf.open("frame3.jpg");
							Gdiplus::Bitmap img(colorInfo.width, colorInfo.height, colorInfo.width*3, colorData.format, colorData.planes[0]);
							CLSID jpgClsid;
							//CLSIDFromString(L"{557cf401-1a04-11d3-9a73-0000f81ef32e}", &jpgClsid);
							//Gdiplus::Image image(L"frame1.tif");
							img.Save(L"frame1.jpg", &jpgClsid);
							frameBuf.close();
							printf("wabba wabba wabba... wobbuffet\npitch: %z\ncolor info width times 3: %d\nnumber of bytes in colordata.planes[0]: %z\n", colorPitch,colorInfo.width*3,sizeof(colorData.planes[0]));
							imgFlag = TRUE;
							colorIm->ReleaseAccess(&colorData);
						}
					}
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
	/*jointLog.close();
	torsoLog.close();
	leftArmLog.close();
	rightArmLog.close();
	CloseHandle(threadSuccess); //closes table output thread
	*/
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
		myPoint head(joints[2].world.x, joints[2].world.y, joints[2].world.z, joints[2].image.x, joints[2].image.y);
		myPoint shoulderLeft(joints[4].world.x, joints[4].world.y, joints[4].world.z, joints[4].image.x, joints[4].image.y);
		myPoint shoulderRight(joints[5].world.x, joints[5].world.y, joints[5].world.z, joints[5].image.x, joints[5].image.y);
		myPoint spineMid(joints[3].world.x, joints[3].world.y, joints[3].world.z, joints[3].image.x, joints[3].image.y);

		PXCPersonTrackingData::PersonTracking::PointCombined centerMass = personData->QueryTracking()->QueryCenterMass();
		myPoint myCenterMass(centerMass.world.point.x, centerMass.world.point.y, centerMass.world.point.z, centerMass.image.point.x, centerMass.image.point.y);

		/* Calculates torsoHeight and shoulderDistance and adds them to respective vectors to keep track */
		targetUser.updatePerson(head, shoulderLeft, shoulderRight, spineMid, myCenterMass);
		//targetUser.printPerson();

		/* Successful joint reading, increment counter */
		initializeCount++;
	}

	if (initializeCount >= MAX_INITIALIZE_COUNT) {
		targetUserTorsoHeight = targetUser.getMedianTorsoHeight();
		targetUserShoulderWidth = targetUser.getMedianShoulderDistance();
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
			myPoint head            (joints[2].world.x, joints[2].world.y, joints[2].world.z, joints[2].image.x, joints[2].image.y);
			myPoint shoulderLeft    (joints[4].world.x, joints[4].world.y, joints[4].world.z, joints[4].image.x, joints[4].image.y);
			myPoint shoulderRight   (joints[5].world.x, joints[5].world.y, joints[5].world.z, joints[5].image.x, joints[5].image.y);
			myPoint spineMid        (joints[3].world.x, joints[3].world.y, joints[3].world.z, joints[3].image.x, joints[3].image.y);


			myPerson curr = myPerson(head, shoulderLeft, shoulderRight, spineMid, myCenterMass);
			//curr.printPerson(); //can implement while testing
			double currConf = compareTorsoRatio(curr, targetUser); //confidence that current person is user
			double proximity = proximitytoLKL(curr.getCenterMass);
			printf("%d. Joint Similarity = %.2f , Proximity to LKL = %.2f\n", perIter, currConf,proximity);

		}
	}
}

double proximitytoLKL(myPoint currCM) {
	myPoint LKL = circBuff.returnLKL();
	double xLKL = LKL.getImageX, zLKL = LKL.getWorldZ;
	double xCM = currCM.getImageX, zCM = currCM.getWorldZ;
	double prox2LKL = sqrt(pow((xLKL - xCM), 2.0) + pow((zLKL - zCM),2.0));
	return prox2LKL;
}

/*This function needs to be revamped and transformed into TU Found*/
void updateTargetUser(PXCPersonTrackingModule* personModule) {
	//printf("Updating the target user...\n");
	/* Accesses the only person in camera's FOV, our target user */
	PXCPersonTrackingData::Person* personData = personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, 0);
	assert(personData != NULL);


	/* Queries for skeleton joint data */
	PXCPersonTrackingData::PersonJoints* personJoints = personData->QuerySkeletonJoints();

	PXCPersonTrackingData::PersonJoints::SkeletonPoint* joints = new PXCPersonTrackingData::PersonJoints::SkeletonPoint[personJoints->QueryNumJoints()];
	personJoints->QueryJoints(joints);

	PXCPersonTrackingData::PersonTracking::PointCombined centerMass = personData->QueryTracking()->QueryCenterMass();
	myPoint myCenterMass(centerMass.world.point.x, centerMass.world.point.y, centerMass.world.point.z, centerMass.image.point.x, centerMass.image.point.y);

	circBuff.updateULV(myCenterMass,robVector);

	if (isJointInfoValid(joints) == false) {
		//printf("Invalid jointType data...\n");
	}
	/* Joint info is valid, initialize target user */
	else {
		myPoint head           (joints[2].world.x, joints[2].world.y, joints[2].world.z, joints[2].image.x, joints[2].image.y);
		myPoint shoulderLeft   (joints[4].world.x, joints[4].world.y, joints[4].world.z, joints[4].image.x, joints[4].image.y);
		myPoint shoulderRight  (joints[5].world.x, joints[5].world.y, joints[5].world.z, joints[5].image.x, joints[5].image.y);
		myPoint spineMid       (joints[3].world.x, joints[3].world.y, joints[3].world.z, joints[3].image.x, joints[3].image.y);
		
		targetUser.updatePerson(head, shoulderLeft, shoulderRight, spineMid, myCenterMass);
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
		int numPersons = personModule->QueryOutput()->QueryNumberOfPeople();

		/* Finds the unique ID of each user */
		int uniqueID = personData->QueryTracking()->QueryId();
		//printf("uniqueID = %d numPeople=%d", uniqueID, numPeopleFound);

		/* If assigned ID greater than the size of personFound, person hasn't been seen before */
		if (uniqueID > numPeopleFound) {
			printf("New user found ID = %d\n", uniqueID);
			//numPeopleFound++;
			/* Adding the new personData to our history of persons found */
			numPeopleFound++;
			return true;
		}
	}
	return false;
}
	
/*void createOutputTable() {
	// Set up the handles for reading/writing
	wHnd = GetStdHandle(STD_OUTPUT_HANDLE);
	rHnd = GetStdHandle(STD_INPUT_HANDLE);

	// Change the window title
	SetConsoleTitle(TEXT("userID"));

	// Set up the required window size
	SMALL_RECT windowSize = { 0, 0, consoleHeight - 1, consoleWidth - 1 };

	// Change the console window size
	SetConsoleWindowInfo(wHnd, TRUE, &windowSize);

	// Create a COORD to hold the buffer size
	COORD bufferSize = { consoleHeight, consoleWidth };

	// Change the internal buffer size
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
*/
/* Parameter is void, can be any data type or no data at all. This function is executed by the thread */
/*DWORD WINAPI updateTable(LPVOID lpParam)
{
	//printf("thread started...");
	while (personsFound.size() > 0) {

		Sleep(100); //runs every tenth second
		printTable(true, 0.0); //prints all the persons in the table

	}
	return 0;
}*/

/*void printTable(bool targetFound, double targetUserVal) {

	//need to set cursor position before writing to screen 
	if (personsFound[0] == NULL) {
		//target user disappeared, mark that on output table
		COORD cursorPos = { 2, 1 };
		SetConsoleCursorPosition(wHnd, cursorPos);
		printf("TargetUser LKL  "); printLocation(targetUserLocBuff.getLastLocation());
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
}*/

boolean isJointInfoValid(PXCPersonTrackingData::PersonJoints::SkeletonPoint* joints) {
	/* Not valid data, return false */
	if (joints[0].jointType != 6 || joints[1].jointType != 7 || joints[2].jointType != 10 || joints[3].jointType != 19 || joints[4].jointType != 16 || joints[5].jointType != 17) {
		return false;
		printf("Garbage data...\n");
	}
	/*Hands not crucial for current comparison methodology */
	if (joints[0].image.x == 0 && joints[0].image.y == 0) { //no image coordinates for left hand, skips initialization
															//printf("Invalid left hand...\n");
															//return false;
	}
	if (joints[1].image.x == 0 && joints[1].image.y == 0) { //no image coordinates for right hand, skips initialization
															//printf("Invalid right hand...\n");
															//return false;
	}
	if (joints[3].image.x == 0 && joints[3].image.y == 0) { //no image coordinates for left shoulder, skips initialization
		printf("Invalid left shoulder...\n");
		return false;
	}
	if (joints[4].image.x == 0 && joints[4].image.y == 0) { //no image coordinates for right shoulder, skips initialization
		printf("Invalid right shoulder...\n");
		return false;
	}

	/* All tested issues passed, joint info is valid */
	return true;
}