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
#include "PersonTrackingFrameRateCalculator.h"
#include "PersonTrackingRendererManager.h"
#include "PersonTrackingRenderer2d.h"
#include "PersonTrackingRenderer3d.h"
#include "PersonTrackingProcessor.h"
#include "PersonTrackingUtilities.h"
#include "ProfileSetMap.h"
#include "myFunctions.h"
#include "resource.h"



#define STREAM_WIDTH 640
#define STREAM_HEIGHT 480

PXCSession *session = NULL;
PersonTrackingRendererManager *renderer = NULL;
PersonTrackingRenderer2D *renderer2D = NULL;


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
	//session handles IO from modules
	session = pp->QuerySession();
	//enabling personTracking module
	pp->EnablePersonTracking();
	//need the module for querying capabilities
	//QueryPersonTracking returns the module instance when the module is not processing any incoming data
	//it returns null when the module is busy

	//for future reference, make sure you enable modules before pp.init()...just took me like an hour to figure that out





    //Create stream renders
    UtilRender renderc(L"Color"), renderd(L"Depth");
    pxcStatus sts;

	//getHWND(renderc);
	//getHWND(renderd);

	//-----------creating render windows---------------
	HWND colorWindow = renderc.m_hWnd;
	HWND depthWindow = renderc.m_hWnd;

	//printing out the values to each render box's pointer value. Will need in order to draw on it
	printf("colorWindowPtr: %p\n", renderc.m_hWnd);
	printf("depthWindowPtr: %p\n", renderd.m_hWnd);

	PersonTrackingRenderer2D *renderer2D = new PersonTrackingRenderer2D(colorWindow);
	PersonTrackingRenderer3D *renderer3D = new PersonTrackingRenderer3D(colorWindow, session);

	renderer = new PersonTrackingRendererManager(renderer2D, renderer3D);

	renderer->SetRendererType(PersonTrackingRenderer::R2D);
	renderer->SetSenseManager(pp);



	//variable to be printed
	int p = 0, count = 0;

    do {
		PXCVideoModule::DataDesc desc={};
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
		
		//finding out the stream resolutions
		PXCCaptureManager *cm = pp->QueryCaptureManager();
		PXCSizeI32 sizeColor = cm->QueryImageSize(PXCCapture::STREAM_TYPE_COLOR);
		PXCSizeI32 sizeDepth = cm->QueryImageSize(PXCCapture::STREAM_TYPE_DEPTH);

		printf("Color stream size: (%d, %d)\n", sizeColor.width, sizeColor.height);
		printf("Depth stream size: (%d, %d)\n", sizeDepth.width, sizeDepth.height);

		//Module Configuration
		PXCPersonTrackingConfiguration* personTrackingConfig = pp->QueryPersonTracking()->QueryConfiguration();
		//Module will track al angles (frontal and profile)
		personTrackingConfig->SetTrackedAngles(PXCPersonTrackingConfiguration::TrackingAngles::TRACKING_ANGLES_ALL);
		//Enabling Joint Tracking
		PXCPersonTrackingConfiguration::SkeletonJointsConfiguration* skeletonJoints = personTrackingConfig->QuerySkeletonJoints();
		skeletonJoints->SetMaxTrackedPersons(1);
		skeletonJoints->SetTrackingArea(PXCPersonTrackingConfiguration::SkeletonJointsConfiguration::SkeletonMode::);
		skeletonJoints->Enable();
		printf("is jointTracking Enabled?: %d\n", skeletonJoints->IsEnabled());


		/*PXCCapture::Device *device = cm->QueryDevice();
		PXCCapture::Device::StreamProfileSet profileSet = device->QueryStreamProfileSet();

		printf("(%d, %d)", profileSet.color.imageInfo.width, profileSet.color.imageInfo.height)*/

		//finding out video properties
		/*PXCPersonTrackingModule *moduleTest = pp->QueryPersonTracking();
		PXCVideoModule *vidModule = moduleTest->QueryInstance<PXCVideoModule>();
		PXCVideoModule::StreamDesc streamDesc = vidMo*/



        /* Stream Data */
		while (true) {
			/* Waits until new frame is available and locks it for application processing */
			sts = pp->AcquireFrame(false);

			//printf("reaching rendering portion\n");

			/* Render streams*/
			//this was originally a constant, but does it need to be? I'm going to try it without const
			//const PXCCapture::Sample *sample = pp->QuerySample();
			PXCCapture::Sample *sample = pp->QuerySample();
			if (sample) {
				//createBlueSample(sample);


				//RenderFrame method format: 
				//bool RenderFrame(const unsigned char* pBuffer, const int bitCount, const int width, const int height);
				//            OR
				//bool RenderFrame(PXCImage *image);   sample->*** is a PXCImage...figure out how to draw onto this PXCImage

				//	printf("beginning person tracking\n");

				PXCPersonTrackingModule* personModule = pp->QueryPersonTacking();
				//PXCPersonTrackingConfiguration* personTrackingConfig = personModule->QueryConfiguration();
				//PXCPersonTrackingConfiguration::SkeletonJointsConfiguration* skeletonJoints = personTrackingConfig->QuerySkeletonJoints();
				//skeletonJoints->Enable();

				//if personModule is null, skip this iteration
				if (personModule == NULL) {
					//printf("personModule is null\n");

					//still needs to render and release the frame though
					if (sample->depth && !renderd.RenderFrame(sample->depth)) break;
					if (sample->color && !renderc.RenderFrame(sample->color)) break;
					pp->ReleaseFrame();
					continue;
				}
				//if more than 1 person in FOV, skip this iteration
				if (personModule->QueryOutput()->QueryNumberOfPeople() > 1) {
					wprintf_s(L"Too many people in image. \n");
					pp->ReleaseFrame();
					continue;
				}

				//if one person in loop
				if (personModule->QueryOutput()->QueryNumberOfPeople() == 1) {
					PXCPersonTrackingData::Person* personData = personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, 0);
					assert(personData != NULL);
					PXCPersonTrackingData::PersonTracking* personTracking = personData->QueryTracking();
					PXCPersonTrackingData::PersonTracking::PointCombined centerMass = personTracking->QueryCenterMass();

					PXCPersonTrackingData::PersonJoints * ptj = personData->QuerySkeletonJoints();
					int numJoints = ptj->QueryNumJoints();
					printf("numJoints: %d ", numJoints);
					printf("is jointTracking Enabled?: %d ", skeletonJoints->IsEnabled());
					//PXCPersonTrackingData::PersonJoints::SkeletonPoint* jointArray[numJoints];
					//PXCPersonTrackingData::PersonJoints::

					//void PersonTrackingRenderer::CreateMarking(PXCPersonTrackingModule* personModule, PXCCapture::Sample* sample) {
					//this method might draw a rectangle around a person
									//renderer->CreateMarking(personModule, sample);
									//renderer->CreateMarking(personModule, sample);
									//renderer->SetOutput(personModule->QueryOutput());
									//renderer->DrawBitmap(sample);
									//renderer2D->DrawGraphics(personModule->QueryOutput());
									//renderer2D->DrawLocation(personTracking);
					//myDrawLocation(personTracking, renderc);
					//colorBitmapBlue(sample);
					//if (sample->depth && !renderd.RenderFrame(sample->depth)) break;
					if (sample->color && !renderc.RenderFrame(sample->color)) break;
					wprintf_s(L"Center Mass (%f, %f, %f\n", centerMass.world.point.x, centerMass.world.point.y, centerMass.world.point.z);
				}
			}

			//printf("loop count: %d\n", count++);
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
