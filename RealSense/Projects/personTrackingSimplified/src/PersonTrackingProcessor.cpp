#include "PersonTrackingProcessor.h"
#include <assert.h>
#include <string>
#include <sstream>
#include "pxcpersontrackingconfiguration.h"
#include "pxcsensemanager.h"
#include "PersonTrackingUtilities.h"
#include "PersonTrackingRendererManager.h"
#include "ProfileSetMap.h"
#include "resource.h"

extern PXCSession* session;
extern PersonTrackingRendererManager* renderer;

extern volatile bool isStopped;
extern volatile bool isActiveApp;
extern pxcCHAR fileName[1024];
extern HANDLE ghMutex;
bool LightenBackground(PXCImage* segmented_image);


long int GetInputId(HWND dialogWindow)
{
	wchar_t idString[32];
	Edit_GetText(GetDlgItem(dialogWindow, IDC_IDINPUT), idString, sizeof(idString)/sizeof(wchar_t));
	int id = wcstol(idString, NULL, 10);
	return id;
}

PersonTrackingProcessor::PersonTrackingProcessor(HWND window) : m_window(window), m_startTrackingFlag(false), m_stopTrackingFlag(false), m_registerFlag(false), m_unregisterFlag(false) {
	//put invalid values to stream profile set
	m_streamProfileSet.color.imageInfo.width = -1;
	m_streamProfileSet.color.imageInfo.height = -1;
	m_streamProfileSet.color.frameRate.min = -1;
	m_streamProfileSet.color.frameRate.max = -1;

}

void PersonTrackingProcessor::Process(HWND dialogWindow)
{
	PXCSenseManager* senseManager = session->CreateSenseManager();
	if (senseManager == NULL) 
	{
		PersonTrackingUtilities::SetStatus(dialogWindow, L"Failed to create an SDK SenseManager", statusPart);
		return;
	}

	/* Set Mode & Source */
	PXCCaptureManager* captureManager = senseManager->QueryCaptureManager();
	if (!PersonTrackingUtilities::GetPlaybackState(dialogWindow) && m_streamProfileSet.color.imageInfo.width != -1 && m_streamProfileSet.color.imageInfo.height != -1)
	{
		if (PersonTrackingUtilities::IsModuleSelected(dialogWindow, IDC_RECOGNITION))
		{
			if (m_streamProfileSet.color.imageInfo.width != 1920 &&
				m_streamProfileSet.color.imageInfo.width != 1280)
			{
				m_streamProfileSet.color.imageInfo.width = 1280;
				m_streamProfileSet.color.imageInfo.height = 720;
				m_streamProfileSet.color.frameRate.min = 30;
				m_streamProfileSet.color.frameRate.max = 30;
			}
		}
		captureManager->FilterByStreamProfiles(&m_streamProfileSet);
	}

	pxcStatus status = PXC_STATUS_NO_ERROR;
	if (PersonTrackingUtilities::GetRecordState(dialogWindow)) 
	{
		status = captureManager->SetFileName(fileName, true);
	} 
	else if (PersonTrackingUtilities::GetPlaybackState(dialogWindow)) 
	{
		status = captureManager->SetFileName(fileName, false);
		senseManager->QueryCaptureManager()->SetRealtime(false);
	} 
	if (status < PXC_STATUS_NO_ERROR) 
	{
		PersonTrackingUtilities::SetStatus(dialogWindow, L"Failed to Set Record/Playback File", statusPart);
		return;
	}

	/* Set Module */
	senseManager->EnablePersonTracking();

	/* Initialize */
	PersonTrackingUtilities::SetStatus(dialogWindow, L"Init Started", statusPart);

	PXCPersonTrackingModule* personModule = senseManager->QueryPersonTracking();
	if (personModule == NULL)
	{
		assert(personModule);
		return;
	}
	PXCPersonTrackingConfiguration* config = personModule->QueryConfiguration();
	if (config == NULL)
	{
		assert(config);
		return;
	}

	UpdateConfiguration(dialogWindow, config);

	if (senseManager->Init() < PXC_STATUS_NO_ERROR)
	{
		captureManager->FilterByStreamProfiles(NULL);
		if (senseManager->Init() < PXC_STATUS_NO_ERROR)
		{
			PersonTrackingUtilities::SetStatus(dialogWindow, L"Init Failed", statusPart);
			return;
		}
	}

	if (PersonTrackingUtilities::GetPlaybackState(dialogWindow))
	{
		//set first frame for playback mode
		int firstFrameNumber = PersonTrackingUtilities::GetStartFromFrameNumber(dialogWindow);
		int numFrames = captureManager->QueryNumberOfFrames();
		if (firstFrameNumber > numFrames) {
			std::wstringstream stringStream;
			stringStream << L"frame number>frame count = " << std::to_wstring(numFrames) << L" starting from 0";
			MessageBox(dialogWindow, stringStream.str().c_str(), NULL, MB_ICONERROR);
			firstFrameNumber = 0;
		}
		captureManager->SetFrameByIndex(firstFrameNumber);
	}

	PXCCapture::DeviceInfo info;
	senseManager->QueryCaptureManager()->QueryDevice()->QueryDeviceInfo(&info);
	int hr;
	if (info.model == PXCCapture::DEVICE_MODEL_DS4 && !PersonTrackingUtilities::GetPlaybackState(dialogWindow))
	{
		//hr = senseManager->QueryCaptureManager()->QueryDevice()->SetDSLeftRightExposure(31);
		hr = senseManager->QueryCaptureManager()->QueryDevice()->SetDSLeftRightAutoExposure(true);
		senseManager->QueryCaptureManager()->QueryDevice()->SetDepthConfidenceThreshold(0);
	}

	PersonTrackingUtilities::SetStatus(dialogWindow, L"Streaming", statusPart);

	bool isNotFirstFrame = false;
	bool isFinishedPlaying = false;
	bool activeapp = true;
	ResetEvent(renderer->GetRenderingFinishedSignal());

	renderer->SetSenseManager(senseManager);
	renderer->SetCallback(renderer->SignalProcessor);

	m_output = personModule->QueryOutput();

	if (!isStopped)
	{
		int frameNum = captureManager->QueryFrameIndex();
		while (true)
		{
			UpdateConfiguration(dialogWindow, config);
			if (senseManager->AcquireFrame(true) < PXC_STATUS_NO_ERROR)
			{
				isFinishedPlaying = true;
			}


			if (isFinishedPlaying || isStopped)
			{
				if (isStopped)
				{
					senseManager->ReleaseFrame();
				}

				if (isFinishedPlaying)
				{
					PostMessage(dialogWindow, WM_COMMAND, ID_STOP, 0);
				}

				break;
			}

			PXCCapture::Sample* sample = senseManager->QueryPersonTrackingSample();
			isNotFirstFrame = true;



			if (sample != NULL)
			{
				DWORD dwWaitResult;
				dwWaitResult = WaitForSingleObject(ghMutex,	INFINITE);
				if(dwWaitResult == WAIT_OBJECT_0)
				{
					// draw segmeneted image
					if (personModule->QueryOutput()->QueryNumberOfPeople() > 0) {
						if ( (renderer->GetRendererType() == PersonTrackingRenderer::R2D && PersonTrackingUtilities::IsModuleSelected(dialogWindow, IDC_SEGMENT)) ||
							(renderer->GetRendererType() == PersonTrackingRenderer::R3D && PersonTrackingUtilities::IsModuleSelected(dialogWindow, IDC_BLOB)) )
							renderer->CreateMarking(personModule, sample);
					}

					renderer->DrawBitmap(sample);
					renderer->SetOutput(personModule->QueryOutput());
					renderer->DrawFrameNumber(frameNum++);
					
					if (personModule->QueryOutput()->QueryNumberOfPeople() > 0)
					{
						if (m_startTrackingFlag)
						{
							PerformTrack(GetInputId(dialogWindow));
						}
						if (m_stopTrackingFlag)
						{
							PerformUntrack(GetInputId(dialogWindow));
						}

						if (m_registerFlag)
						{
							PerformRegister(GetInputId(dialogWindow));
						}
						if (m_unregisterFlag)
						{
							PerformUnregister(GetInputId(dialogWindow));
						}
					}

					int minId = 1000;
					for (int i = 0; i < m_output->QueryNumberOfPeople(); i++)
					{
						int currentId = m_output->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, i)->QueryTracking()->QueryId();
						if (currentId < minId)
							minId = currentId;
					}
					if (minId < 1000 && GetInputId(dialogWindow) < minId)
					{
						wchar_t minIdString[32];
						swprintf_s(minIdString, sizeof(minIdString) / sizeof(wchar_t), L"%d", minId);
						Edit_SetText(GetDlgItem(dialogWindow, IDC_IDINPUT), minIdString);
					}

					renderer->SignalRenderer();

					if (!ReleaseMutex(ghMutex))
					{
						throw std::exception("Failed to release mutex");
						return;
					}
				}
			}

			if (isNotFirstFrame)
			{
				WaitForSingleObject(renderer->GetRenderingFinishedSignal(), INFINITE);
			}

			senseManager->ReleaseFrame();
		}

		PersonTrackingUtilities::SetStatus(dialogWindow, L"Stopped", statusPart);
	}

	senseManager->Close(); 
	senseManager->Release();
}



void PersonTrackingProcessor::StartTracking()
{
	m_startTrackingFlag = true;
}

void PersonTrackingProcessor::StopTracking()
{
	m_stopTrackingFlag = true;
}

void PersonTrackingProcessor::PerformTrack(int id)
{
	m_startTrackingFlag = false;
	m_output->StartTracking(id);
}

void PersonTrackingProcessor::PerformUntrack(int id)
{
	m_stopTrackingFlag = false;
	m_output->StopTracking(id);
}

void PersonTrackingProcessor::PerformRegister(int id)
{
	m_registerFlag = false;
	auto personData = m_output->QueryPersonDataById(id);
	if(personData != NULL)
		personData->QueryRecognition()->RegisterUser();
}

void PersonTrackingProcessor::PerformUnregister(int id)
{
	m_unregisterFlag = false;
	auto personData = m_output->QueryPersonDataById(id);
	if(personData != NULL)
		personData->QueryRecognition()->UnregisterUser();
}

bool PersonTrackingProcessor::IsTracking()
{
	return (m_output->GetTrackingState() == PXCPersonTrackingData::TRACKING_STATE_TRACKING ? true : false);
}

void PersonTrackingProcessor::RegisterUser()
{
		m_registerFlag = true;
}

void PersonTrackingProcessor::UnregisterUser()
{
		m_unregisterFlag = true;
}

template<typename T>
void SyncModuleStateWithGui(HWND dialogWindow, int idc, T module)
{
	if (PersonTrackingUtilities::IsModuleSelected(dialogWindow, idc))
	{
		if (!module->IsEnabled()) 
		{
			module->Enable();
		}
	}
	else
	{
		if (module->IsEnabled())
		{
			module->Disable();
		}
	}
}
void PersonTrackingProcessor::UpdateConfiguration(HWND dialogWindow, PXCPersonTrackingConfiguration* config)
{
	auto trackingModeGui = PersonTrackingUtilities::IsModuleSelected(dialogWindow, IDC_INTERACTIVE_MODE) ?
		PXCPersonTrackingConfiguration::TrackingConfiguration::TRACKING_MODE_INTERACTIVE :
		PXCPersonTrackingConfiguration::TrackingConfiguration::TRACKING_MODE_FOLLOWING;
	auto trackingModeActual = config->QueryTracking()->GetTrackingMode();
	if (trackingModeGui != trackingModeActual)
	{
		config->QueryTracking()->SetTrackingMode(trackingModeGui);
	}

	SyncModuleStateWithGui(dialogWindow, IDC_EXPRESSIONS, config->QueryExpressions());
	SyncModuleStateWithGui(dialogWindow, IDC_LOCATION, config->QueryTracking());
	SyncModuleStateWithGui(dialogWindow, IDC_RECOGNITION, config->QueryRecognition());
	SyncModuleStateWithGui(dialogWindow, IDC_SKELETON, config->QuerySkeletonJoints());
	
	if (PersonTrackingUtilities::IsModuleSelected(dialogWindow, IDC_GESTURE))
	{
		if (!config->QueryGestures()->IsEnabled())
		{
			config->QueryGestures()->Enable();
			config->QueryGestures()->EnableAllGestures();
		}
	}
	else
	{
		if (config->QueryGestures()->IsEnabled())
		{
			config->QueryGestures()->DisableAllGestures();
			config->QueryGestures()->Disable();
		}
	}

	if (PersonTrackingUtilities::IsModuleSelected(dialogWindow, IDC_SEGMENT) || PersonTrackingUtilities::IsModuleSelected(m_window, IDC_HEAD) 
		|| PersonTrackingUtilities::IsModuleSelected(m_window, IDC_BLOB))
	{
		if (!config->QueryTracking()->IsSegmentationEnabled()) 
		{
			config->QueryTracking()->EnableSegmentation();
		}
	}
	else
	{
		if (config->QueryTracking()->IsSegmentationEnabled()) 
		{
			config->QueryTracking()->DisableSegmentation();
		}
	}

	if (PersonTrackingUtilities::IsModuleSelected(dialogWindow, IDC_HEADPOSE))
	{
		config->QueryTracking()->EnableHeadPose();
	}
	else
	{
		config->QueryTracking()->DisableHeadPose();
	}
}

void PersonTrackingProcessor::SetStreamProfileSet(const PXCCapture::Device::StreamProfileSet & streamProfileSet)
{
	m_streamProfileSet = streamProfileSet;
}

//bool LightenBackground(PXCImage* segmented_image)
//{
//    // If the segmented_image contains a segmented region, 
//    // this return code is set to true.
//    bool bMaskIsSegmented = false;
//
//    // Iterate over the pixels in the image
//    PXCImage::ImageData segmented_image_data;
//    segmented_image->AcquireAccess(PXCImage::ACCESS_READ_WRITE,
//        PXCImage::PIXEL_FORMAT_RGB32, &segmented_image_data);
//    const pxcI32 height = segmented_image->QueryInfo().height;
//    const pxcI32 width = segmented_image->QueryInfo().width;
//    for (int y = 0; y < height; y++)
//    {
//        // Get the address of the row of pixels
//        pxcBYTE* p = segmented_image_data.planes[0]
//            + y * segmented_image_data.pitches[0];
//
//        // For each pixel in the row...
//        const char GREY = 0x7f;
//        for (int x = 0; x < width; x++)
//        {
//            // ...if the pixel is part of the segmented region...
//            if (p[3] > 0)
//            {
//                // set the return flag, if it's not already set
//                if (!bMaskIsSegmented) bMaskIsSegmented = true;
//
//                // When the user moves into the near/far extent, the alpha values will drop from 255 to 1.
//                // This can be used to fade the user in/out as a cue to move into the ideal operating range.
//                if (p[3] != 255)
//                {
//                    const float blend_factor = (float)p[3] / 255.0f;
//                    for (int ch = 0; ch < 3; ch++)
//                    {
//                        pxcBYTE not_visible = ((p[ch] >> 4) + GREY);
//                        p[ch] = (pxcBYTE)(p[ch] * blend_factor + not_visible * (1.0f - blend_factor));
//                    }
//                }
//            }
//            else for (int ch = 0; ch < 3; ch++) p[ch] = (p[ch] >> 4) + GREY;
//
//            // Move the pointer to the next pixel (RGBA)
//            p += 4;
//        }
//    }
//    segmented_image->ReleaseAccess(&segmented_image_data);
//
//    return bMaskIsSegmented;
//}

