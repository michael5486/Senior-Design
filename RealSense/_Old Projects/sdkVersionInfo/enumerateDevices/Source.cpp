#include <pxcsession.h>
#include <stdio.h>

int main() {

	PXCSession *session = PXCSession::CreateInstance();
	PXCSession::ImplVersion ver = session->QueryVersion();
	wprintf_s(L"SDK Version %d.%d\n", ver.major, ver.minor);
	session->Release();
	printf("hello world");
}



/*#include <pxcsession.h>
#include <pxccapture.h>
#include <pxcstatus.h>
#include <stdio.h>


using namespace std;

void main() {
	// device is a PXCCapture::Device instance
	PXCCapture::DeviceInfo dinfo = {};
	device->QueryDeviceInfo(&dinfo);

	if (dinfo.streams&PXCCapture::STREAM_TYPE_COLOR) {
		wprintf_s(L"COLOR STREAM\n");
	}

	if (dinfo.streams&PXCCapture::STREAM_TYPE_DEPTH) {
		wprintf_s(L"DEPTH STREAM\n");
	}

	
}*/