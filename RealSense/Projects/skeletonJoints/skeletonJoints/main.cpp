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
//#include "PersonTrackingFrameRateCalculator.h"
//#include "PersonTrackingRendererManager.h"
//#include "PersonTrackingRenderer2d.h"
//#include "PersonTrackingRenderer3d.h"
//#include "PersonTrackingProcessor.h"
//#include "PersonTrackingUtilities.h"
//#include "ProfileSetMap.h"
//#include "resource.h"

PXCSession *session = NULL;

//need this crap so it will compile...comes from extern variables in PersonTrackingProcessor.cpp
pxcCHAR fileName[1024] = { 0 };
HANDLE ghMutex;
volatile bool isStopped = false;

int main(int argc, WCHAR* argv[]) {
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
	//	personTrackingConfig->SetTrackedAngles(PXCPersonTrackingConfiguration::TrackingAngles::TRACKING_ANGLES_ALL);
		personTrackingConfig->QueryTracking()->SetTrackingMode(PXCPersonTrackingConfiguration::TrackingConfiguration::TRACKING_MODE_INTERACTIVE);
		
		PXCPersonTrackingConfiguration::SkeletonJointsConfiguration* skeletonJoints = personTrackingConfig->QuerySkeletonJoints();
		skeletonJoints->Enable();
		printf("is jointTracking Enabled?: %d\n", skeletonJoints->IsEnabled());


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
					printf("Number of people: %d\n", numPeople);
					PXCPersonTrackingData::Person* personData = personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, 0);
					assert(personData != NULL);
					PXCPersonTrackingData::PersonJoints* personJoints = personData->QuerySkeletonJoints();
					
					// Michael modifications

					PXCPersonTrackingData::PersonJoints::SkeletonPoint* joints = new PXCPersonTrackingData::PersonJoints::SkeletonPoint[personJoints->QueryNumJoints()];
					personJoints->QueryJoints(joints);

					if (joints[0].jointType != 6 || joints[1].jointType != 7 || joints[2].jointType != 10 || joints[3].jointType != 19 || joints[4].jointType != 16 || joints[5].jointType != 17) {
						printf("Invalid jointType data...");
					}
					else {
						printf("-------Joint Set--------\n");
						printf("  Type: %d  confidenceImage:%d x: %.2f y: %.2f z:%.2f\n", joints[0].jointType, joints[0].confidenceImage, joints[0].image.x, joints[0].image.y, joints[0].world.z);
						printf("  Type: %d  confidenceImage:%d x: %.2f y: %.2f z:%.2f\n", joints[1].jointType, joints[1].confidenceImage, joints[1].image.x, joints[1].image.y, joints[1].world.z);
						printf("  Type: %d confidenceImage:%d x: %.2f y: %.2f z:%.2f\n", joints[2].jointType, joints[2].confidenceImage, joints[2].image.x, joints[2].image.y, joints[2].world.z);
						printf("  Type: %d confidenceImage:%d x: %.2f y: %.2f z:%.2f\n", joints[3].jointType, joints[3].confidenceImage, joints[3].image.x, joints[3].image.y, joints[3].world.z);
						printf("  Type: %d confidenceImage:%d x: %.2f y: %.2f z:%.2f\n", joints[4].jointType, joints[4].confidenceImage, joints[4].image.x, joints[4].image.y, joints[4].world.z);
						printf("  Type: %d confidenceImage:%d x: %.2f y: %.2f z:%.2f\n", joints[5].jointType, joints[5].confidenceImage, joints[5].image.x, joints[5].image.y, joints[5].world.z);
					}
					//Michael modifications end
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