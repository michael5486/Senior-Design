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

PXCSession *session = NULL;

/* Variables needed to compile */
pxcCHAR fileName[1024] = { 0 };
HANDLE ghMutex;
volatile bool isStopped = false;

/* Global variables used in target identification */
//std::map<const int, myPerson> peopleMap;
myPerson targetUser;
bool isInitialized = false;

int main(int argc, WCHAR* argv[]) {
	double compareUser(myPerson,myPerson); //user comparison function
	/* Creates an instance of the PXCSenseManager */
	PXCSenseManager *pp = PXCSenseManager::CreateInstance();

	if (!pp) {
		wprintf_s(L"Unable to create the SenseManager\n");
		return 3;
	}

	/* Enables person tracking module */
	session = pp->QuerySession();
	pp->EnablePersonTracking();

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
		if (sts<PXC_STATUS_NO_ERROR) {
			/* Enable a single stream */
			pp->Close();
			pp->EnableStream(PXCCapture::STREAM_TYPE_DEPTH);
			sts = pp->Init();
			if (sts<PXC_STATUS_NO_ERROR) {
				pp->Close();
				pp->EnableStream(PXCCapture::STREAM_TYPE_COLOR);
				sts = pp->Init();
			}

			if (sts<PXC_STATUS_NO_ERROR) {
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
				PXCPersonTrackingModule* personModule = pp->QueryPersonTracking(); //is this line still necessary? Isn't personModule already initialized and the associated functions enabled?

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
					//printf("Number of people: %d\n", numPeople);
					PXCPersonTrackingData::Person* personData = personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, 0);
					assert(personData != NULL);
					PXCPersonTrackingData::PersonJoints* personJoints = personData->QuerySkeletonJoints();

					PXCPersonTrackingData::PersonJoints::SkeletonPoint* joints = new PXCPersonTrackingData::PersonJoints::SkeletonPoint[personJoints->QueryNumJoints()];
					personJoints->QueryJoints(joints);
					if (joints[0].jointType != 6 || joints[1].jointType != 7 || joints[2].jointType != 10 || joints[3].jointType != 19 || joints[4].jointType != 16 || joints[5].jointType != 17) {
						printf("Invalid jointType data...\n");
					}
					else {
						/* Initializing target user */
						/* We should average our tracked joints over 5 seconds or something, removing outliers (median calculated values)
						, initializing shouldn't occur in one frame*/
						if (isInitialized == false) {
							if (joints[0].image.x == 0 && joints[0].image.y == 0) { //no image coordinates for left hand, skips initialization
								printf("Invalid left hand...\n");
								continue;
							}
							if (joints[1].image.x == 0 && joints[1].image.y == 0) { //no image coordinates for right hand, skips initialization
								
								printf("Invalid right hand...\n");
								continue;
							}
							if (joints[3].image.x == 0 && joints[3].image.y == 0) { //no image coordinates for left shoulder, skips initialization
								printf("Invalid left shoulder...\n");
								continue;
							}
							if (joints[4].image.x == 0 && joints[4].image.y == 0) { //no image coordinates for right shoulder, skips initialization
								printf("Invalid right shoulder...\n");
								continue;
							}
							printf("Initializing target user...\n");
							myPoint leftHand     (joints[0].world.x, joints[0].world.y, joints[0].world.z, joints[0].image.x, joints[0].image.y);
							myPoint rightHand    (joints[1].world.x, joints[1].world.y, joints[1].world.z, joints[1].image.x, joints[1].image.y);
							myPoint head         (joints[2].world.x, joints[2].world.y, joints[2].world.z, joints[2].image.x, joints[2].image.y);
							myPoint shoulderLeft (joints[3].world.x, joints[3].world.y, joints[3].world.z, joints[3].image.x, joints[3].image.y);
							myPoint shoulderRight(joints[4].world.x, joints[4].world.y, joints[4].world.z, joints[4].image.x, joints[4].image.y);
							myPoint spineMid     (joints[5].world.x, joints[5].world.y, joints[5].world.z, joints[5].image.x, joints[5].image.y);
							targetUser.updateJoints(head, shoulderLeft, shoulderRight, leftHand, rightHand, spineMid);
							targetUser.printPerson();
							isInitialized = true;
						}
						else {
							double userConf = 0.0; //calculate confidence value that person found is user
							myPerson found; //this person's tracked values will be closest to user's, location will be outputted to microcontroller (or some other function or some shit)
							for (int perIter = 0; perIter < numPeople; perIter++) { //iterating across these mofos
								myPerson curr;
								PXCPersonTrackingData::Person* personData = personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, perIter);
								assert(personData != NULL); 
								PXCPersonTrackingData::PersonJoints* personJoints = personData->QuerySkeletonJoints();

								PXCPersonTrackingData::PersonJoints::SkeletonPoint* joints = new PXCPersonTrackingData::PersonJoints::SkeletonPoint[personJoints->QueryNumJoints()];
								personJoints->QueryJoints(joints);
								if (joints[0].jointType != 6 || joints[1].jointType != 7 || joints[2].jointType != 10 || joints[3].jointType != 19 || joints[4].jointType != 16 || joints[5].jointType != 17) {
									printf("Invalid jointType data...\n");
								}
								else {
									printf("Initializing target user...\n");
									myPoint leftHand(joints[0].world.x, joints[0].world.y, joints[0].world.z, joints[0].image.x, joints[0].image.y);
									myPoint rightHand(joints[1].world.x, joints[1].world.y, joints[1].world.z, joints[1].image.x, joints[1].image.y);
									myPoint head(joints[2].world.x, joints[2].world.y, joints[2].world.z, joints[2].image.x, joints[2].image.y);
									myPoint shoulderLeft(joints[3].world.x, joints[3].world.y, joints[3].world.z, joints[3].image.x, joints[3].image.y);
									myPoint shoulderRight(joints[4].world.x, joints[4].world.y, joints[4].world.z, joints[4].image.x, joints[4].image.y);
									myPoint spineMid(joints[5].world.x, joints[5].world.y, joints[5].world.z, joints[5].image.x, joints[5].image.y);
									curr.updateJoints(head, shoulderLeft, shoulderRight, leftHand, rightHand, spineMid);
									//curr.printPerson(); //can implement while testing
									double currConf = compareUser(curr,targetUser); //confidence that current person is user
									if (currConf > userConf) {
										found = curr; //the user location values can be extracted using myPerson found
										userConf = currConf;
									}
								}
							}
						}
					}
					delete[] joints;
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

double compareUser(myPerson curr, myPerson user) {
	return 0.0;
}