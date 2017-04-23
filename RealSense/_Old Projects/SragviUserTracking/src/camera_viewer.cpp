//Sragvi Tirumala, Michael Esposito
//January, 2017

//The purpose of this program is to stream the camera output, identify a user, and extract desired parameters from the user.
#include <windows.h>
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
#include "PersonTrackingProcessor.h"
#include "ProfileSetMap.h"

int wmain(int argc, WCHAR* argv[]) {
    /* Creates an instance of the PXCSenseManager */
    PXCSenseManager *pp = PXCSenseManager::CreateInstance();
    if (!pp) {
        wprintf_s(L"Unable to create the SenseManager\n");
        return 3;
    }
	// get session of pp instance
	PXCSession *session;
	session = pp->QuerySession();
	pp->EnablePersonTracking();
	

    // Create stream renders
    UtilRender renderc(L"Color"), renderd(L"Depth"), renderi(L"IR"), renderr(L"Right"), renderl(L"Left");
    pxcStatus sts;

	//print var
	int p = 0;

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

        /* Stream Data */
		while (true) {
			/* Waits until new frame is available and locks it for application processing */
			sts = pp->AcquireFrame(false);

			/* Render streams*/
			const PXCCapture::Sample *sample = pp->QuerySample();
			if (sample) {
				if (sample->depth && !renderd.RenderFrame(sample->depth)) break;
				if (sample->color && !renderc.RenderFrame(sample->color)) break;
				if (sample->ir && !renderi.RenderFrame(sample->ir))    break;
				if (sample->right && !renderr.RenderFrame(sample->right)) break;
				if (sample->left && !renderl.RenderFrame(sample->left))  break;
			}
			
			//doin some shit with person tracking
			PXCPersonTrackingModule* personModule = pp->QueryPersonTracking();
			if (personModule == NULL)
			{
				continue;
			}
			if (personModule->QueryOutput()->QueryNumberOfPeople() > 1) {
				wprintf_s(L"Too many people in image. \n");
				continue;
			}
			if (personModule->QueryOutput()->QueryNumberOfPeople() == 1) {
				PXCPersonTrackingData::Person* personData = personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, 0);
				assert(personData != NULL);
				PXCPersonTrackingData::PersonTracking* personTracking = personData->QueryTracking();
				PXCPersonTrackingData::PersonTracking::PointCombined centerMass = personTracking->QueryCenterMass();

				if (p % 30 == 0) {
					wprintf_s(L"The user's x location: %f \n The user's y location: %f \n The user's z location: %f \n", centerMass.world.point.x, centerMass.world.point.y, centerMass.world.point.z);
				}
			}
			
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
