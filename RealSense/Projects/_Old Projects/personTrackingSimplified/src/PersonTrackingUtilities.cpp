#include <WindowsX.h>
#include "PersonTrackingUtilities.h"
#include "resource.h"
#include <commctrl.h>


bool PersonTrackingUtilities::GetPlaybackState(HWND DialogWindow)
{
	return (GetMenuState(GetMenu(DialogWindow), ID_MODE_PLAYBACK, MF_BYCOMMAND) & MF_CHECKED) != 0;
}

bool PersonTrackingUtilities::GetRecordState(HWND hwndDlg)
{
	return (GetMenuState(GetMenu(hwndDlg), ID_MODE_RECORD, MF_BYCOMMAND) & MF_CHECKED) != 0;
}

bool PersonTrackingUtilities::IsModuleSelected(HWND hwndDlg, const int moduleID)
{
	return (Button_GetState(GetDlgItem(hwndDlg, moduleID)) & BST_CHECKED) != 0;
}

void PersonTrackingUtilities::SetStatus(HWND dialogWindow, pxcCHAR *line, StatusWindowPart part)
{
	HWND hwndStatus = GetDlgItem(dialogWindow, IDC_STATUS);
	SendMessage(hwndStatus, SB_SETTEXT, (WPARAM)(INT) part, (LPARAM) (LPSTR) line);
	UpdateWindow(dialogWindow);
}

pxcCHAR* PersonTrackingUtilities::GetCheckedModule(HWND dialogWindow)
{
	HMENU menu = GetSubMenu(GetMenu(dialogWindow), MODULE_MENU_POSITION);
	static pxcCHAR line[256];

	GetMenuString(menu, GetChecked(menu), line, sizeof(line) / sizeof(pxcCHAR), MF_BYPOSITION);
	return line;
}

pxcCHAR* PersonTrackingUtilities::GetCheckedDevice(HWND dialogWindow)
{
	HMENU menu = GetSubMenu(GetMenu(dialogWindow), DEVICE_MENU_POSITION);
	static pxcCHAR line[256];
	GetMenuString(menu, GetChecked(menu), line, sizeof(line) / sizeof(pxcCHAR), MF_BYPOSITION);
	return line;
}

int PersonTrackingUtilities::GetCheckedColorProfileIdx(HWND dialogWindow)
{
	HMENU menu = GetSubMenu(GetMenu(dialogWindow), COLOR_MENU_POSITION);
	return GetChecked(menu);
}

int PersonTrackingUtilities::GetChecked(HMENU menu)
{
	for (int i = 0; i < GetMenuItemCount(menu); ++i)
	{
		if (GetMenuState(menu, i, MF_BYPOSITION) & MF_CHECKED) 
			return i;
	}

	return 0;
}

int PersonTrackingUtilities::GetStartFromFrameNumber(HWND DialogWindow)
{
	BOOL success;
	int startFromFrameNumber = (int)GetDlgItemInt(DialogWindow, IDC_STARTFRAMENUM_INPUT, &success, true);
	return (success && startFromFrameNumber >= 0) ? startFromFrameNumber : 0;
}