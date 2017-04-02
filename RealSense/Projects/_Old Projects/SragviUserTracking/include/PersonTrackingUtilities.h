#pragma once

#include <windows.h>
#include <map>
#include "pxcdefs.h"
#include "PXCPersonTrackingConfiguration.h"

enum StatusWindowPart { statusPart, alertPart };

class PersonTrackingUtilities
{
public:
	static int GetChecked(HMENU menu);
	static pxcCHAR* GetCheckedDevice(HWND dialogWindow);
	static pxcCHAR* GetCheckedModule(HWND dialogWindow);
	static void SetStatus(HWND dialogWindow, pxcCHAR *line, StatusWindowPart part);
	static bool IsModuleSelected(HWND hwndDlg, const int moduleID);
	static bool GetRecordState(HWND hwndDlg);
	static bool GetPlaybackState(HWND DialogWindow);
	static int GetStartFromFrameNumber(HWND DialogWindow);
	static int PersonTrackingUtilities::GetCheckedColorProfileIdx(HWND dialogWindow);
	static const int TextHeight = 16;

	//position of menu at GUI
	enum MenuPosition
	{
		DEVICE_MENU_POSITION = 0,
		COLOR_MENU_POSITION,
		MODULE_MENU_POSITION,
		MODE_MENU_POSITION,
	};
};
