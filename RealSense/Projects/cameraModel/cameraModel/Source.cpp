#include <pxcsensemanager.h>
#include <stdio.h>

void main() {
	// Create a SenseManager instance
	PXCSenseManager *sm = PXCSenseManager::CreateInstance();
	// Other SenseManager configuration (say, enable streams or modules)
	// Initialize for starting streaming.
	sm->Init();
	// Get the camera info
	PXCCapture::DeviceInfo dinfo = {};
	sm->QueryCaptureManager()->QueryDevice()->QueryDeviceInfo(&dinfo);
	printf_s("camera model = %d\n", dinfo.model);
	// Clean up
	sm->Release();
}