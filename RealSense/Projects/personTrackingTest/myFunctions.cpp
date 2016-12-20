#include <Windows.h>
#include <util_render.h>
#include <stdio.h>
#include <PersonTrackingRenderer.h>
#include <stdint.h>

PXCCapture::Sample *blueSample = NULL;

HWND getHWND(UtilRender utilrender) {

	//prints out the hWnd (window handle) pointer value
	char szBuff[64];
	printf(szBuff, "HWND: %p\n", utilrender.m_hWnd);
	//returns pointer value
	return utilrender.m_hWnd;

}

void createBlueSample(PXCCapture::Sample *sample) {
	//create a new image
	PXCImage* image = sample->color;
	PXCImage::ImageData data;

	if (image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB32, &data) >= PXC_STATUS_NO_ERROR) {
		const pxcI32 height = image->QueryInfo().height;
		const pxcI32 width = image->QueryInfo().width;

		for (int i = 0; i < height; i++) {
			// Get the address of the row of pixels
			pxcBYTE* p = data.planes[0] + i * data.pitches[0];

			for (int j = 0; j < width; j++) {
				//might make it blue
				p[1] = 255;
				p[2] = 255;

			}
		}
		image->ReleaseAccess(&data);

		//if (sample->color && !renderc.RenderFrame(sample->color)) break;

		UtilRender temp(L"temp");
		temp.RenderFrame(sample->color);


	}
	else {
		printf("Couldn't convert image...\n");
	}

}

void colorBitmapBlue(PXCCapture::Sample *sample) {

	printf("trying to color bitmap blue....\n");

	//create a new image
	PXCImage* image = sample->color;
	PXCImage::ImageData data;
	if (image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB32, &data) >= PXC_STATUS_NO_ERROR) {
		//pxcBYTE *imageData = data.planes[0];
		//for (int i = 0; i++; )
		const pxcI32 height = image->QueryInfo().height;
		const pxcI32 width = image->QueryInfo().width;

		for (int i = 0; i < height; i++) {
			// Get the address of the row of pixels
			pxcBYTE *p = data.planes[0] + i * data.pitches[0];
			//uint8_t temp = 255;
			////printf("%hhu    ", temp);
			//printf("%u  ", temp);

				for (int j = 0; j < width; j+=3) {
					//might make it blue
					
					*p = 255;
					//p[2] = 255;
					//printf("(%d, %d)", p[1], p[2]);
					//*p = temp;
					//printf("%u  ", &p);
					p++;
					*p = 0;
					p++;
					*p = 0;
					p++;

			}
	
		}
		printf("\n");
		image->ReleaseAccess(&data);
	}
	else {
		printf("Couldn't convert image...\n");
	}

}

/*void PersonTrackingRenderer2D::DrawBitmap(PXCCapture::Sample* sample)
{
	if (m_bitmap)
	{
		DeleteObject(m_bitmap);
		m_bitmap = 0;
	}

	PXCImage* image = sample->color;


	PXCImage::ImageInfo info = image->QueryInfo();
	PXCImage::ImageData data;
	if (image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB32, &data) >= PXC_STATUS_NO_ERROR)
	{
		HWND hwndPanel = GetDlgItem(m_window, IDC_PANEL);
		HDC dc = GetDC(hwndPanel);
		BITMAPINFO binfo;
		memset(&binfo, 0, sizeof(binfo));
		binfo.bmiHeader.biWidth = data.pitches[0] / 4;
		binfo.bmiHeader.biHeight = -(int)info.height;
		binfo.bmiHeader.biBitCount = 32;
		binfo.bmiHeader.biPlanes = 1;
		binfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		binfo.bmiHeader.biCompression = BI_RGB;
		Sleep(1);
		m_bitmap = CreateDIBitmap(dc, &binfo.bmiHeader, CBM_INIT, data.planes[0], &binfo, DIB_RGB_COLORS);

		ReleaseDC(hwndPanel, dc);
		image->ReleaseAccess(&data);
	}
}*/


void myDrawLocation(PXCPersonTrackingData::PersonTracking* trackedPerson, UtilRender utilrender)
{
	PXCPersonTrackingData::BoundingBox2D box = trackedPerson->Query2DBoundingBox();
	//++m_detectionCounter;
	//HWND panelWindow = GetDlgItem(m_window, IDC_PANEL);

	HWND renderBox = utilrender.m_hWnd;




	HDC dc1 = GetDC(renderBox);
//	HDC dc2 = CreateCompatibleDC(dc1);

	LineTo(dc1, 0, 0);
	LineTo(dc1, 0, 50);
	LineTo(dc1, 50, 50);
	LineTo(dc1, 50, 0);

	/*if (!dc2)
	{
		ReleaseDC(renderBox, dc1);
		return;
	}

	HBITMAP bitmap = utilrender.m_bitmap;

	SelectObject(dc2, utilrender.m_bitmap);

	BITMAP bitmap;
	GetObject(utilrender.m_bitmap, sizeof(bitmap), &bitmap);
	HPEN cyan;
	if (box.confidence)
	{
		cyan = CreatePen(PS_SOLID, 3, RGB(255, 255, 0));
	}
	else
	{
		cyan = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
	}

	if (!cyan)
	{
		DeleteDC(dc2);
		ReleaseDC(renderBox, dc1);
		return;
	}
	SelectObject(dc2, cyan);

	MoveToEx(dc2, box.rect.x, box.rect.y, 0);
	LineTo(dc2, box.rect.x, box.rect.y + box.rect.h);
	LineTo(dc2, box.rect.x + box.rect.w, box.rect.y + box.rect.h);
	LineTo(dc2, box.rect.x + box.rect.w, box.rect.y);
	LineTo(dc2, box.rect.x, box.rect.y);

	WCHAR line[64];
	//SelectObject(dc2, m_hFont);
	if (trackedPerson->QueryId() >= 0)
	{
		swprintf_s<sizeof(line) / sizeof(pxcCHAR)>(line, L"%d", trackedPerson->QueryId());
		TextOut(dc2, box.rect.x, box.rect.y, line, std::char_traits<wchar_t>::length(line));
	}

	DeleteObject(cyan);

	DeleteDC(dc2);*/
	ReleaseDC(renderBox, dc1);
}