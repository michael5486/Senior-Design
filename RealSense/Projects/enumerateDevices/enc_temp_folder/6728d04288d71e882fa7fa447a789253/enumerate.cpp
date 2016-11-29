#include <pxccapture.h>
#include <pxcsensemanager.h>
#include <pxcsession.h>
#include <pxccapturemanager.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

using namespace std;

int main() {

	// create the PXCSenseManager
	int pause;
	PXCSenseManager *psm = 0;
	psm = PXCSenseManager::CreateInstance();
	if (!psm) {
		wprintf_s(L"Unable to create the PXCSenseManager\n");
		return 1;
	}
	// Retrieve the underlying session created by the PXCSenseManager.
	// The returned instance is an PXCSenseManager internally managed object.
	// Note: Do not release the session!
	PXCSession *session;
	session = psm->QuerySession();
	if (session == NULL) {
		wprintf_s(L"Session not created by PXCSenseManager\n");
		return 2;
	}

	// session is a PXCSession instance
	PXCSession::ImplDesc desc1 = {};
	desc1.group = PXCSession::IMPL_GROUP_SENSOR;
	desc1.subgroup = PXCSession::IMPL_SUBGROUP_VIDEO_CAPTURE;

	//for (int m = 0;; m++) {
	PXCSession::ImplDesc desc2;
	if (session->QueryImpl(&desc1, 0, &desc2) < PXC_STATUS_NO_ERROR)
		wprintf_s(L"Module[%d]: %s\n", 0, desc2.friendlyName);

	PXCCapture *capture = 0;
	pxcStatus sts = session->CreateImpl<PXCCapture>(&desc2, &capture);

	//querying device info
	PXCCapture::DeviceInfo dinfo;
	sts = capture->QueryDeviceInfo(0, &dinfo);

	//printing device name
	printf("Device Name:\n");
	wprintf_s(L"    Device[%d]: %s\n", 0, dinfo.name);

	//printing device serial
	printf("Device Serial:\n");
	wprintf_s(L"    Device[%d]: %s\n", 0, dinfo.serial);

	//printing orientation
	printf("Device Orientation:\n");
	PXCCapture::DeviceOrientation orient = dinfo.orientation;
	std::cout << "    Device[" << 0 << "]:  " << orient << "\n";

	//printing device model
	printf("Device Model:\n");
	PXCCapture::DeviceModel model = dinfo.model;
	std::cout << "    Device[" << 0 << "]:  " << model << "\n";


	PXCCaptureManager *cm = psm->QueryCaptureManager();
	PXCCapture::Device *device = cm->QueryDevice();

	// Retrieve camera color and depth field of view
	PXCPointF32 color_fov = device->QueryColorFieldOfView();
	PXCPointF32 depth_fov = device->QueryDepthFieldOfView();

	//std::cout << "Color FOV: " << color_fov.x << ", " << &color_fov.y << "\n";
	//std::cout << "Depth FOV: " << &depth_fov.x << ", " << &depth_fov.y << "\n";

	printf("Color FOV: %d, %d\n", &color_fov.x, &color_fov.y);
	printf("Depth FOV: %d, %d\n", &depth_fov.x, &depth_fov.y);

	capture->Release();

	std::cout << "Press any key to continue...";
	std::cin >> pause;
	//}

	/*wstring ws(dinfo.orientation);    string str(ws.begin(), ws.end());
	std::cout << "Device[" << d << "]:  " << str.c_str() << std::endl;

	wstring ws(dinfo.model);    string str(ws.begin(), ws.end());
	std::cout << "Device[" << d << "]:  " << str.c_str() << std::endl;*/

}