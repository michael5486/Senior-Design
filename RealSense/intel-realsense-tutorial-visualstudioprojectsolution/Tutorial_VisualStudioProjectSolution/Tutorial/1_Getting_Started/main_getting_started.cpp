/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2013-2014 Intel Corporation. All Rights Reserved.

*******************************************************************************/
/* 
Description:
This is the Geting Started sample that demonstrates how to create an instance of the PXCSenseManager, which is the main interface 
to any of the predefined modules. This sample also shows how to retrieve the underlying session created by PXCSenseManager. 
Then, you will use the session to enumerate all available modules that are automatically loaded with the RSSDK.
*/

#include <windows.h>
#include <wchar.h>
#include "pxcsensemanager.h"

int wmain(int argc, WCHAR* argv[]) {
	
	// create the PXCSenseManager
	PXCSenseManager *psm=0;
	psm = PXCSenseManager::CreateInstance();
	if (!psm) {
        wprintf_s(L"Unable to create the PXCSenseManager\n");
        return 1;
    }

	// Retrieve the underlying session created by the PXCSenseManager.
	// The returned instance is an PXCSenseManager internally managed object.
	// Note:  Do not release it!
	PXCSession *session;
	session = psm->QuerySession();
	if (session == NULL) {
        wprintf_s(L"Session not created by PXCSenseManager\n");
        return 2;
    }
	
	// query the session version
	PXCSession::ImplVersion ver;
	ver = session->QueryVersion();

	// print version to console
	wprintf_s(L"Hello SDK Version %d.%d\n",ver.major, ver.minor); 

	// enumerate all available modules that are automatically loaded with the RSSDK
	for (int i=0;;i++) {
		PXCSession::ImplDesc desc;
		if ( session->QueryImpl(0,i,&desc) < PXC_STATUS_NO_ERROR ) break; 

		// print the module friendly name and iuid (interface unique ID)
		wprintf_s(L"Module[%d]: %s\n",i,desc.friendlyName);
		wprintf_s(L"           iuid=%x\n",desc.iuid);
	}

	// close the streams and release any session and processing module instances
	// internally release will call Close as well. 
	// - Release frees all instances. Need to call CreateInstance again.
	// - Close just close the device and module instances so that you can re-init. No need to call CreateInstance again. 
	psm->Release(); 
	
	system("pause");

	return 0;
}