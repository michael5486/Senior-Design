#include <pxcsensemanager.h>
#include <pxcsession.h>
#include "util_render.h"
#include <iostream>
#include <stdlib.h>

#define WIDTH 640
#define HEIGHT 480


/*void main()
{
	int pause;

	UtilRender *renderColor = new UtilRender(L"COLOR_STREAM");
	UtilRender *renderDepth = new UtilRender(L"DEPTH_STREAM");

	//create sense manager
	PXCSenseManager *psm = 0;
	psm = PXCSenseManager::CreateInstance();
	
	if (!psm) {
		wprintf_s(L"Unable to create the PXCSenseManager\n");
		system("PAUSE");
		return;
	}

	//create a session
	PXCSession::ImplVersion version = psm->QuerySession()->QueryVersion();
	std::cout << "SDK Version:" << version.major << version.minor << std::endl;

	//enable the stream from realsense
	//psm->EnableStream(PXCCapture::STREAM_TYPE_COLOR, WIDTH, HEIGHT);
	psm->EnableStream(PXCCapture::STREAM_TYPE_IR, WIDTH, HEIGHT);

	
	//initialize stream
	psm->Init();
	
	//create an image for the color frames
	PXCImage *colorIm, *depthIm;
	
	while (true) {
		printf("Acquire frame: %d", psm->AcquireFrame(true));
		if (psm->AcquireFrame(true) < PXC_STATUS_NO_ERROR) {
			system("PAUSE");
			break;
		}

		PXCCapture::Sample *sample = psm->QuerySample();
		if (sample) {
			if (sample->color) {
				colorIm = sample->ir;
				renderColor->RenderFrame(colorIm);
			}
			printf("sample works\n");
			if (sample->depth) {
				depthIm = sample->depth;
				renderDepth->RenderFrame(depthIm);
				printf("rendered depth\n");
			}
		}
		psm->ReleaseFrame();
	}
	psm->Release();

	std::cout << "Press any key to continue...";
	std::cin >> pause;
}

		if (sample) {
			if (sample->color) {
				colorIm = sample->color;
				if (!renderColor->RenderFrame(colorIm))
					break;
			}
		}
*/