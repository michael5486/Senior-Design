	/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2012-2013 Intel Corporation. All Rights Reserved.

*******************************************************************************/
#include <Windows.h>
#include <WindowsX.h>
#include <stdio.h>
#include <cstdlib>
#include <commctrl.h>
#include <map>
#include "math.h"
#include "resource.h"
#include "pxcpersontrackingmodule.h"
#include "pxcpersontrackingdata.h"
#include "pxcvideomodule.h"
#include "pxcpersontrackingconfiguration.h"
#include "pxcmetadata.h"
#include "service/pxcsessionservice.h"
#include "PersonTrackingFrameRateCalculator.h"
#include "PersonTrackingRendererManager.h"
#include "PersonTrackingRenderer2D.h"
#include "PersonTrackingRenderer3D.h"
#include "PersonTrackingUtilities.h"
#include "PersonTrackingProcessor.h"
#include "ProfileSetMap.h"


/* Needed to output to log */
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <stdlib.h>
#include <sstream>
using namespace std;

ofstream outputFile;



pxcCHAR fileName[1024] = { 0 };
PXCSession* session = NULL;
PersonTrackingRendererManager* renderer = NULL;
PersonTrackingProcessor* processor = NULL;
ProfileSetMap* profileSetMap = NULL;
HANDLE ghMutex;

volatile bool isRunning = false;
volatile bool isStopped = false;
volatile bool isActiveApp = true;

/* Needed in main method */
void createLogFile(string fileName);



static int controls[] = { IDC_SCALE, IDC_SEGMENT, /*IDC_MIRROR,*/ IDC_LOCATION, ID_START, ID_STOP, IDC_BLOB, IDC_RECOGNITION, ID_REGISTER, ID_UNREGISTER, 
						  ID_TRACK, ID_UNTRACK, /* IDC_HEAD  ,*/ IDC_SKELETON, IDC_GESTURE, /*IDC_EXPRESSIONS,*/ IDC_IDINPUT, IDC_INTERACTIVE_MODE, IDC_FRAMENUM_TEXT, IDC_FRAMENUM,
						  IDC_STARTFRAMENUM_INPUT, IDC_START_FROM_FRAME_NB, IDC_EXPRESSIONS, IDC_HEADPOSE };
static RECT layout[3 + sizeof(controls) / sizeof(controls[0])];



void GetPlaybackFile(void) 
{
	OPENFILENAME filename;
	memset(&filename, 0, sizeof(filename));
	filename.lStructSize = sizeof(filename);
	filename.lpstrFilter = L"RSSDK clip (*.rssdk)\0*.rssdk\0Old format clip (*.pcsdk)\0*.pcsdk\0All Files (*.*)\0*.*\0\0";
	filename.lpstrFile = fileName; 
	fileName[0] = 0;
	filename.nMaxFile = sizeof(fileName) / sizeof(pxcCHAR);
	filename.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER;
	if (!GetOpenFileName(&filename)) 
		fileName[0] = 0;
}

void GetRecordFile(void) 
{
	OPENFILENAME filename;
	memset(&filename, 0, sizeof(filename));
	filename.lStructSize = sizeof(filename);
	filename.lpstrFilter = L"RSSDK clip (*.rssdk)\0*.rssdk\0All Files (*.*)\0*.*\0\0";
	filename.lpstrFile = fileName; 
	fileName[0] = 0;
	filename.nMaxFile = sizeof(fileName) / sizeof(pxcCHAR);
	filename.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_EXPLORER;
	if (GetSaveFileName(&filename)) {
		if (filename.nFilterIndex==1 && filename.nFileExtension==0) {
			int len =  std::char_traits<wchar_t>::length(fileName);
			if (len>1 && len<sizeof(fileName)/sizeof(pxcCHAR)-7) {
				wcscpy_s(&fileName[len], rsize_t(7), L".rssdk\0");
			}
		}
	} else fileName[0] = 0;
}

void PopulateColor(HMENU menu, std::wstring deviceName)
{
	if (profileSetMap == NULL)
	{
		return;
	}

	DeleteMenu(menu, PersonTrackingUtilities::COLOR_MENU_POSITION, MF_BYPOSITION);
	HMENU colorMenu = CreatePopupMenu();
	auto resolutionList = profileSetMap->GetSupportedProfileSets(deviceName);
	int itemId = ID_COLORX;
	int defaultResolutionIdx = 0;
	for (auto resolutionParamIt = resolutionList.begin(); resolutionParamIt != resolutionList.end(); ++resolutionParamIt)
	{
		
		if (resolutionParamIt->color.imageInfo.width == 320 && resolutionParamIt->color.imageInfo.height == 240 && resolutionParamIt->color.frameRate.min == 30)
		{
			defaultResolutionIdx = itemId;
			AppendMenu(colorMenu, MF_ENABLED, itemId, ProfileSetMap::StreamProfileSetToString(*resolutionParamIt).c_str());
		}
	}
	CheckMenuRadioItem(colorMenu, 0, GetMenuItemCount(colorMenu), defaultResolutionIdx - ID_COLORX , MF_BYPOSITION);
	InsertMenu(menu, PersonTrackingUtilities::COLOR_MENU_POSITION, MF_BYPOSITION | MF_POPUP, (UINT_PTR)colorMenu, L"Color");
}

void PopulateDevice(HMENU menu)
{
	DeleteMenu(menu, PersonTrackingUtilities::DEVICE_MENU_POSITION, MF_BYPOSITION);

	PXCSession::ImplDesc desc;
	memset(&desc, 0, sizeof(desc)); 
	desc.group = PXCSession::IMPL_GROUP_SENSOR;
	desc.subgroup = PXCSession::IMPL_SUBGROUP_VIDEO_CAPTURE;
	HMENU menu1 = CreatePopupMenu();
	for (int i = 0, k = ID_DEVICEX; ; ++i)
	{
		PXCSession::ImplDesc desc1;
		if (session->QueryImpl(&desc, i, &desc1) < PXC_STATUS_NO_ERROR)
			break;

		PXCCapture *capture;
		if (session->CreateImpl<PXCCapture>(&desc1, &capture) < PXC_STATUS_NO_ERROR) 
			continue;

		for (int j = 0; ; ++j) {
			PXCCapture::DeviceInfo deviceInfo;
			if (capture->QueryDeviceInfo(j, &deviceInfo) < PXC_STATUS_NO_ERROR) 
				break;

			AppendMenu(menu1, MF_STRING, k++, deviceInfo.name);
		}

		capture->Release();
	}
	CheckMenuRadioItem(menu1, 0, GetMenuItemCount(menu1), 0, MF_BYPOSITION);
	InsertMenu(menu, PersonTrackingUtilities::DEVICE_MENU_POSITION, MF_BYPOSITION | MF_POPUP, (UINT_PTR)menu1, L"Device");
	wchar_t selectedDeviceName[256];
	GetMenuString(menu1, 0, selectedDeviceName, sizeof(selectedDeviceName), MF_BYPOSITION);
	PopulateColor(menu, selectedDeviceName);
}

void PopulateModule(HMENU menu) 
{
	DeleteMenu(menu, PersonTrackingUtilities::MODULE_MENU_POSITION, MF_BYPOSITION);

	PXCSession::ImplDesc desc, desc1;
	memset(&desc, 0, sizeof(desc));
	desc.cuids[0] = PXCPersonTrackingModule::CUID;
	HMENU menu1 = CreatePopupMenu();

	for (int i = 0; ; ++i)
	{
		if (session->QueryImpl(&desc, i, &desc1) < PXC_STATUS_NO_ERROR) 
			break;
		AppendMenu(menu1, MF_STRING, ID_MODULEX + i, desc1.friendlyName);
	}

	CheckMenuRadioItem(menu1, 0, GetMenuItemCount(menu1), 0, MF_BYPOSITION);
	InsertMenu(menu, PersonTrackingUtilities::MODULE_MENU_POSITION, MF_BYPOSITION | MF_POPUP, (UINT_PTR)menu1, L"Module");
}

void SaveLayout(HWND dialogWindow) 
{
	GetClientRect(dialogWindow, &layout[0]);
	ClientToScreen(dialogWindow, (LPPOINT)&layout[0].left);
	ClientToScreen(dialogWindow, (LPPOINT)&layout[0].right);
	GetWindowRect(GetDlgItem(dialogWindow, IDC_PANEL), &layout[1]);
	GetWindowRect(GetDlgItem(dialogWindow, IDC_STATUS), &layout[2]);
	for (int i = 0; i < sizeof(controls) / sizeof(controls[0]); ++i)
		GetWindowRect(GetDlgItem(dialogWindow, controls[i]), &layout[3 + i]);
}

void RedoLayout(HWND dialogWindow)
{
	RECT rectangle;
	GetClientRect(dialogWindow, &rectangle);

	/* Status */
	SetWindowPos(GetDlgItem(dialogWindow, IDC_STATUS), dialogWindow, 
		0,
		rectangle.bottom - (layout[2].bottom - layout[2].top),
		rectangle.right - rectangle.left,
		(layout[2].bottom - layout[2].top),
		SWP_NOZORDER);

	/* Panel */
	SetWindowPos(
		GetDlgItem(dialogWindow,IDC_PANEL), dialogWindow,
		(layout[1].left - layout[0].left),
		(layout[1].top - layout[0].top),
		rectangle.right - (layout[1].left-layout[0].left) - (layout[0].right - layout[1].right),
		rectangle.bottom - (layout[1].top - layout[0].top) - (layout[0].bottom - layout[1].bottom),
		SWP_NOZORDER);

	/* Buttons & CheckBoxes */
	for (int i = 0; i < sizeof(controls) / sizeof(controls[0]); ++i)
	{
		SetWindowPos(
			GetDlgItem(dialogWindow,controls[i]), dialogWindow,
			rectangle.right - (layout[0].right - layout[3 + i].left),
			(layout[3 + i].top - layout[0].top),
			(layout[3 + i].right - layout[3 + i].left),
			(layout[3 + i].bottom - layout[3 + i].top),
			SWP_NOZORDER);
	}
}

static DWORD WINAPI RenderingThread(LPVOID arg)
{
	while (true)
		renderer->Render();
}

static DWORD WINAPI ProcessingThread(LPVOID arg) 
{
	HWND window = (HWND)arg;
	processor->Process(window);

	isRunning = false;
	return 0;
}

INT_PTR CALLBACK MessageLoopThread(HWND dialogWindow, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
	HMENU menu1 = GetMenu(dialogWindow);
	HMENU menu2;
	pxcCHAR* deviceName;
	switch (message) 
	{ 
	case WM_INITDIALOG:
		PopulateDevice(menu1);
		CheckDlgButton(dialogWindow, IDC_SCALE, BST_CHECKED);
		CheckDlgButton(dialogWindow, IDC_LOCATION, BST_CHECKED);
		Button_Enable(GetDlgItem(dialogWindow, IDC_SEGMENT), true);
		Button_Enable(GetDlgItem(dialogWindow, IDC_SKELETON), true);
		Button_Enable(GetDlgItem(dialogWindow, IDC_GESTURE), true);
		Button_Enable(GetDlgItem(dialogWindow, IDC_EXPRESSIONS), true);
		Button_Enable(GetDlgItem(dialogWindow, IDC_INTERACTIVE_MODE), true);
		deviceName = PersonTrackingUtilities::GetCheckedDevice(dialogWindow);
		Edit_SetText(GetDlgItem(dialogWindow, IDC_IDINPUT), L"0");
		Edit_SetText(GetDlgItem(dialogWindow, IDC_STARTFRAMENUM_INPUT), L"0");

		PopulateModule(menu1);
		SaveLayout(dialogWindow);
		return TRUE; 

	case WM_COMMAND: 
		menu2 = GetSubMenu(menu1, PersonTrackingUtilities::DEVICE_MENU_POSITION);
		if (LOWORD(wParam) >= ID_DEVICEX && LOWORD(wParam) < ID_DEVICEX + GetMenuItemCount(menu2)) 
		{
			CheckMenuRadioItem(menu2, 0, GetMenuItemCount(menu2), LOWORD(wParam) - ID_DEVICEX, MF_BYPOSITION);	
			deviceName = PersonTrackingUtilities::GetCheckedDevice(dialogWindow);
			PopulateColor(menu1, deviceName);
			return TRUE;
		}

		menu2 = GetSubMenu(menu1, PersonTrackingUtilities::COLOR_MENU_POSITION);
		if (LOWORD(wParam) >= ID_COLORX && LOWORD(wParam) < ID_COLORX + GetMenuItemCount(menu2))
		{
			CheckMenuRadioItem(menu2, 0, GetMenuItemCount(menu2), LOWORD(wParam) - ID_COLORX, MF_BYPOSITION);
			return TRUE;
		}

		menu2 = GetSubMenu(menu1, PersonTrackingUtilities::MODULE_MENU_POSITION);
		if (LOWORD(wParam) >= ID_MODULEX && LOWORD(wParam) < ID_MODULEX + GetMenuItemCount(menu2)) 
		{
			CheckMenuRadioItem(menu2, 0, GetMenuItemCount(menu2), LOWORD(wParam) - ID_MODULEX,MF_BYPOSITION);
			return TRUE;
		}

		menu2 = GetSubMenu(menu1, PersonTrackingUtilities::MODE_MENU_POSITION);
		if (LOWORD(wParam) >= ID_PROFILEX && LOWORD(wParam) < ID_PROFILEX + GetMenuItemCount(menu2)) 
		{
			CheckMenuRadioItem(menu2, 0, GetMenuItemCount(menu2), LOWORD(wParam) - ID_PROFILEX,MF_BYPOSITION);
			HWND hwndTab = GetDlgItem(dialogWindow, IDC_TAB);
			if (TabCtrl_GetCurSel(hwndTab) == 0)
			{
				renderer->SetRendererType(PersonTrackingRenderer::R2D);
			}
			else if(TabCtrl_GetCurSel(hwndTab) == 1)
			{
				renderer->SetRendererType(PersonTrackingRenderer::R3D);
			}
			return TRUE;
		}

		switch (LOWORD(wParam)) 
		{
		case IDCANCEL:
			isStopped = true;
			if (isRunning) {
				PostMessage(dialogWindow, WM_COMMAND, IDCANCEL, 0);
			} else 
			{
				DestroyWindow(dialogWindow); 
				PostQuitMessage(0);
			}
			return TRUE;
		case ID_START:
			Button_Enable(GetDlgItem(dialogWindow, ID_START), false);
			Button_Enable(GetDlgItem(dialogWindow, ID_STOP), true);
			Button_Enable(GetDlgItem(dialogWindow, IDC_RECOGNITION), false);


			for (int i = 0;i < GetMenuItemCount(menu1); ++i)
				EnableMenuItem(menu1, i, MF_BYPOSITION | MF_GRAYED);

			DrawMenuBar(dialogWindow);
			isStopped = false;
			isRunning = true;

			if (processor) 
				delete processor;

			processor = new PersonTrackingProcessor(dialogWindow);
			PXCCapture::Device::StreamProfileSet profileSet;
			if (profileSetMap->GetSupportedProfileByIndex(PersonTrackingUtilities::GetCheckedDevice(dialogWindow),
				3, profileSet))
			{
				processor->SetStreamProfileSet(profileSet);
			}

			renderer->Reset();

			CreateThread(0, 0, ProcessingThread, dialogWindow, 0, 0);
			if (PersonTrackingUtilities::IsModuleSelected(dialogWindow, IDC_RECOGNITION))
			{
				Button_Enable(GetDlgItem(dialogWindow, ID_REGISTER), true);
				Button_Enable(GetDlgItem(dialogWindow, ID_UNREGISTER), true);
			}

			Sleep(0); //TODO: remove
			return TRUE;

		case ID_STOP:
			isStopped = true;
			if (isRunning) 
			{
				PostMessage(dialogWindow, WM_COMMAND, ID_STOP, 0);
			}
			else 
			{
				for (int i = 0; i < GetMenuItemCount(menu1); ++i)
					EnableMenuItem(menu1, i, MF_BYPOSITION | MF_ENABLED);

				DrawMenuBar(dialogWindow);
				Button_Enable(GetDlgItem(dialogWindow, ID_START), true);
				Button_Enable(GetDlgItem(dialogWindow, ID_STOP), false);
				Edit_SetText(GetDlgItem(dialogWindow, IDC_IDINPUT), L"0");
				Button_Enable(GetDlgItem(dialogWindow, IDC_RECOGNITION), true);


				deviceName = PersonTrackingUtilities::GetCheckedDevice(dialogWindow);

				if (PersonTrackingUtilities::IsModuleSelected(dialogWindow, IDC_RECOGNITION))
				{
					Button_Enable(GetDlgItem(dialogWindow, ID_REGISTER), false);
					Button_Enable(GetDlgItem(dialogWindow, ID_UNREGISTER), false);
				}
			}
			return TRUE;

		case ID_TRACK:
			processor->StartTracking();
			return TRUE;

		case ID_UNTRACK:
			processor->StopTracking();
			return TRUE;

		case ID_REGISTER:
			processor->RegisterUser();
			return TRUE;

		case ID_UNREGISTER:
			processor->UnregisterUser();
			return TRUE;

		case ID_MODE_LIVE:
			CheckMenuItem(menu1, ID_MODE_LIVE, MF_CHECKED);
			CheckMenuItem(menu1, ID_MODE_PLAYBACK, MF_UNCHECKED);
			CheckMenuItem(menu1, ID_MODE_RECORD, MF_UNCHECKED);
			return TRUE;

		case ID_MODE_PLAYBACK:
			CheckMenuItem(menu1, ID_MODE_LIVE, MF_UNCHECKED);
			CheckMenuItem(menu1, ID_MODE_PLAYBACK, MF_CHECKED);
			CheckMenuItem(menu1, ID_MODE_RECORD, MF_UNCHECKED);
			GetPlaybackFile();
			return TRUE;

		case ID_MODE_RECORD:
			CheckMenuItem(menu1, ID_MODE_LIVE, MF_UNCHECKED);
			CheckMenuItem(menu1, ID_MODE_PLAYBACK, MF_UNCHECKED);
			CheckMenuItem(menu1, ID_MODE_RECORD, MF_CHECKED);
			GetRecordFile();
			return TRUE;
		} 
		break;
	case WM_SIZE:
		RedoLayout(dialogWindow);
		return TRUE;
    case WM_ACTIVATEAPP:
        isActiveApp = wParam;
        break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case TCN_SELCHANGE:
			{ 
				HWND hwndTab = GetDlgItem(dialogWindow, IDC_TAB);
				int iPage = TabCtrl_GetCurSel(hwndTab);
				deviceName = PersonTrackingUtilities::GetCheckedDevice(dialogWindow);
				
				if(iPage == 0)
				{
					renderer->SetRendererType(PersonTrackingRenderer::R2D);
				}
				if(iPage == 1)
				{
					renderer->SetRendererType(PersonTrackingRenderer::R3D);
				}

				return TRUE;
			} 
		}
		break;
	} 
	return FALSE; 
} 

HWND CreateTabControl(HWND hWnd, HINSTANCE hInstance)
{
	if(hWnd != NULL && hInstance != NULL)
	{
		HWND hTab = GetDlgItem(hWnd, IDC_TAB);

		if(hTab != NULL)
		{
			TC_ITEM tc;
			tc.mask = TCIF_TEXT;
			tc.pszText = L"2D";
			tc.iImage = -1;
			tc.lParam = 0;
			TabCtrl_InsertItem(hTab, 0, &tc);
			
			tc.pszText = L"3D";
			TabCtrl_InsertItem(hTab, 1, &tc);
		}
		return hTab;
	}
	return NULL;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int) {

	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen("CON", "w", stdout);

	printf("HELLO!!! I AM THE CONSOLE!");
	printf("Hellllloooo Worlddddd!!!!");

	/* Creating a log file to output logs in */
	//createLogFile("pointLogs/pointLog5.txt");


	InitCommonControls();

	session = PXCSession_Create();
	if (session == NULL) 
	{
        MessageBoxW(0, L"Failed to create an SDK session", L"Person Viewer", MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }

	profileSetMap = new ProfileSetMap(session);
	if (profileSetMap == NULL)
	{
		MessageBoxW(0, L"Failed to create resolution map object", L"Person Viewer", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	if (profileSetMap->Init() != 0)
	{
		MessageBoxW(0, L"Failed to get supported resolution from camera", L"Person Viewer", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}
	

    HWND dialogWindow = CreateDialogW(hInstance, MAKEINTRESOURCE(IDD_MAINFRAME), 0, MessageLoopThread);
    if (!dialogWindow)  
	{
        MessageBoxW(0, L"Failed to create a window", L"Person Viewer", MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }

	HWND statusWindow = CreateStatusWindow(WS_CHILD | WS_VISIBLE, L"", dialogWindow, IDC_STATUS);	
	if (!statusWindow) 
	{
	   MessageBoxW(0, L"Failed to create a status bar", L"Person Viewer", MB_ICONEXCLAMATION | MB_OK);
        return 1;
	}
	
	int statusWindowParts[] = {230, -1};
	SendMessage(statusWindow, SB_SETPARTS, sizeof(statusWindowParts)/sizeof(int), (LPARAM) statusWindowParts);
	SendMessage(statusWindow, SB_SETTEXT, (WPARAM)(INT) 0, (LPARAM) (LPSTR) TEXT("OK"));
	UpdateWindow(dialogWindow);

	HWND hwndTab = CreateTabControl(dialogWindow, hInstance);
	if (!hwndTab) 
	{
		MessageBoxW(0, L"Failed to create tab control", L"Person Viewer", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	PersonTrackingRenderer2D* renderer2D = new PersonTrackingRenderer2D(dialogWindow);
	if(renderer2D == NULL)
	{
		MessageBoxW(0, L"Failed to create 2D renderer", L"Person Viewer", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}
	PersonTrackingRenderer3D* renderer3D = new PersonTrackingRenderer3D(dialogWindow, session);
	if(renderer3D == NULL)
	{
		MessageBoxW(0, L"Failed to create 3D renderer", L"Person Viewer", MB_ICONEXCLAMATION | MB_OK);
		delete renderer2D;
		return 1;
	}
	renderer = new PersonTrackingRendererManager(renderer2D, renderer3D);
	if (renderer == NULL)
	{
		MessageBoxW(0, L"Failed to create renderer manager", L"Person Viewer", MB_ICONEXCLAMATION | MB_OK);
		delete renderer2D;
		delete renderer3D;
		return 1;
	}

	ghMutex = CreateMutex(NULL, FALSE, NULL);
	if (ghMutex == NULL) 
	{
		MessageBoxW(0, L"Failed to create mutex", L"Person Viewer", MB_ICONEXCLAMATION | MB_OK);
		delete renderer;
		return 1;
	}

	int iPage = TabCtrl_GetCurSel(hwndTab);
	if (iPage == 0)
	{
		renderer->SetRendererType(PersonTrackingRenderer::R2D);
	}
	else if (iPage == 1)
	{
		renderer->SetRendererType(PersonTrackingRenderer::R3D);
	}

	CreateThread(NULL, NULL, RenderingThread, NULL, NULL, NULL);

    MSG msg;
	while (int status = GetMessageW(&msg, NULL, 0, 0))
	{
        if (status == -1)
			return status;

        TranslateMessage(&msg);
        DispatchMessage(&msg);

		Sleep(0); // let other threads breathe
    }

	CloseHandle(renderer->GetRenderingFinishedSignal());

	if (processor)
		delete processor;

	if (renderer)
		delete renderer;

	if (profileSetMap)
		delete profileSetMap;

	session->Release();
    return (int)msg.wParam;
}

void createLogFile(string string) {
	/*outputFile.open(string);

	outputFile << "\n";
	outputFile << "Beginning data output...";
	outputFile << "\n\n\n";

	outputFile << left << setw(COUNT_WIDTH) << setfill(separator) << "Time";
	outputFile << left << setw(COLUMN_WIDTH) << setfill(separator) << "JOINT_HEAD";
	outputFile << left << setw(COLUMN_WIDTH) << setfill(separator) << "JOINT_SHOULDER_LEFT";
	outputFile << left << setw(COLUMN_WIDTH) << setfill(separator) << "JOINT_SHOULDER_RIGHT";
	outputFile << left << setw(COLUMN_WIDTH) << setfill(separator) << "JOINT_HAND_LEFT";
	outputFile << left << setw(COLUMN_WIDTH) << setfill(separator) << "JOINT_HAND_RIGHT";
	outputFile << left << setw(COLUMN_WIDTH) << setfill(separator) << "JOINT_SPINE_MID";
	outputFile << "\n";*/
}