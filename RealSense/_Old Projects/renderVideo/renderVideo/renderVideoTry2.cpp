#include <pxccapture.h>
#include <pxcsensemanager.h>
#include <pxcsession.h>
#include <pxccapturemanager.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include "util_render.h"



#define WIDTH 640
#define HEIGHT 480

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


	//if this starts to return -3, (or anything other than 0) unplug R200 and plug back in
	psm->EnableStream(PXCCapture::STREAM_TYPE_COLOR, WIDTH, HEIGHT);
	//this doesn't work when you specify a pixel size for some reason...keep unset with width and height = 0
	psm->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, 0, 0);
	psm->EnableStream(PXCCapture::STREAM_TYPE_LEFT, 0, 0);
	psm->EnableStream(PXCCapture::STREAM_TYPE_RIGHT, 0, 0);
	//it breaks when you try to enable the IR stream...everything else works though
	//psm->EnableStream(PXCCapture::STREAM_TYPE_IR, 0, 0);



	printf("Init(): %d\n", psm->Init());

	UtilRender *renderColor = new UtilRender(L"COLOR_STREAM");
	UtilRender *renderDepth = new UtilRender(L"DEPTH_STREAM");
	UtilRender *renderLeft = new UtilRender(L"LEFT_STREAM");
	UtilRender *renderRight = new UtilRender(L"RIGHT_STREAM");
	UtilRender *renderIr = new UtilRender(L"IR_STREAM");




	PXCImage *colorIm, *depthIm, *leftIm, *rightIm, *irIm;
	while (true) {

		if (psm->AcquireFrame(true) < PXC_STATUS_NO_ERROR) {
			break;
		}

		PXCCapture::Sample *sample = psm->QuerySample();
		if (sample) {
			if (sample->color) {
				colorIm = sample->color;
				if (!renderColor->RenderFrame(colorIm))
					break;
			}
			if (sample->depth) {
				depthIm = sample->depth;
				if (!renderDepth->RenderFrame(depthIm))
					break;
			}
			if (sample->left) {
				leftIm = sample->left;
				if (!renderLeft->RenderFrame(leftIm))
					break;
			}
			if (sample->right) {
				rightIm = sample->right;
				if (!renderRight->RenderFrame(rightIm))
					break;
			}
			if (sample->ir) {
				irIm = sample->ir;
				if (!renderIr->RenderFrame(irIm))
					break;
			}
		}
		psm->ReleaseFrame();
	}

	capture->Release();

	std::cout << "Press any key to continue...";
	std::cin >> pause;

}