#include "PersonTrackingRenderer.h"
#include "PersonTrackingUtilities.h"
#include "pxccapture.h"
#include <map>


void BlendColorPop(PXCImage *base, PXCImage *mask);


PersonTrackingRenderer::PersonTrackingRenderer(HWND window) : m_window(window), m_currentFrameOutput(NULL), m_joints(NULL)
{
	m_senseManager = NULL;
}

PersonTrackingRenderer::~PersonTrackingRenderer()
{
	if (m_joints)
	{
		delete[] m_joints;
		m_joints = NULL;
	}
}

void PersonTrackingRenderer::SetOutput(PXCPersonTrackingData* output)
{
	m_currentFrameOutput = output;
}

void PersonTrackingRenderer::SetSenseManager(PXCSenseManager* senseManager)
{
	m_senseManager = senseManager;
}

PXCSenseManager* PersonTrackingRenderer::GetSenseManager()
{
	return m_senseManager;
}


void PersonTrackingRenderer::CreateMarking(PXCPersonTrackingModule* personModule, PXCCapture::Sample* sample) {
	int num_people = personModule->QueryOutput()->QueryNumberOfPeople();

	PXCPersonTrackingData::Person * person = personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, 0);
	PXCImage* segmented_image = GetRelevantMask(person);
	//PXCImage* segmented_image = person->QueryTracking()->QuerySegmentationImage();
	//PXCImage* segmented_image = person->QueryTracking()->QueryBlobMask(); vitaly

	if (segmented_image)
	{

		//ilan - combine segmented images

		for (int n = 1; n < num_people; n++)
		{

			PXCPersonTrackingData::Person * person2 = personModule->QueryOutput()->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, n);
			PXCImage* segmented_image2 = GetRelevantMask(person2);
			//PXCImage* segmented_image2 = person2->QueryTracking()->QuerySegmentationImage();
			//PXCImage* segmented_image = person->QueryTracking()->QueryBlobMask(); vitaly

			// lock the segmented images

			if (segmented_image2) {

				PXCImage::ImageData ddepth;
				segmented_image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_Y8, &ddepth);
				BYTE* bdepth = (BYTE*)ddepth.planes[0];

				PXCImage::ImageData ddepth2;
				segmented_image2->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_Y8, &ddepth2);
				BYTE* bdepth2 = (BYTE*)ddepth2.planes[0];

				PXCImage::ImageInfo info = segmented_image->QueryInfo();

				long width = info.width;
				long height = info.height;

				int dwidth2 = ddepth.pitches[0]; // aligned depth width

				for (long i = 0; i < height; i++)
				{

					long index2 = i*dwidth2;

					for (long j = 0; j < width; j++)
					{
						bdepth[index2] = bdepth[index2] | bdepth2[index2];
						index2++;
					}

				}

				segmented_image->ReleaseAccess(&ddepth);
				segmented_image2->ReleaseAccess(&ddepth2);
				segmented_image2->Release();


			}

		}

		// blend color and segmented image
		BlendColorPop(GetRelevantImage(sample), segmented_image);
		segmented_image->Release();
	}
}


void BlendColorPop(PXCImage *base, PXCImage *mask)
{
	if (base == NULL || mask == NULL)
		return;

	PXCImage::ImageInfo maskInfo = mask->QueryInfo();
	PXCImage::ImageData maskData;
	PXCImage::ImageInfo outInfo = base->QueryInfo();
	PXCImage::ImageData outData;

	if (maskInfo.width != outInfo.width || maskInfo.height != outInfo.height)
		return;

	pxcStatus photoSts = PXC_STATUS_NO_ERROR;
	if (outInfo.format == PXCImage::PIXEL_FORMAT_DEPTH) {
		photoSts = base->AcquireAccess(PXCImage::ACCESS_READ_WRITE, PXCImage::PIXEL_FORMAT_DEPTH, &outData);
	}
	else {
		photoSts = base->AcquireAccess(PXCImage::ACCESS_READ_WRITE, PXCImage::PIXEL_FORMAT_RGB24, &outData);
	}
	
	pxcStatus maskSts = mask->AcquireAccess(PXCImage::ACCESS_READ, maskInfo.format, &maskData);

	int maskBytesPerPixel = 0;
	switch (maskInfo.format) {
	case PXCImage::PixelFormat::PIXEL_FORMAT_Y8:		maskBytesPerPixel = 1; break;
	case PXCImage::PixelFormat::PIXEL_FORMAT_DEPTH_F32: maskBytesPerPixel = 4; break;
	case PXCImage::PixelFormat::PIXEL_FORMAT_Y16:		maskBytesPerPixel = 2; break;
	case PXCImage::PixelFormat::PIXEL_FORMAT_DEPTH:		maskBytesPerPixel = 2; break;
	}

	const char GREY = 0xaf;

	if (photoSts == PXC_STATUS_NO_ERROR)
	{
		if (maskSts != PXC_STATUS_NO_ERROR)
		{
			base->ReleaseAccess(&outData);
			return;
		}

		for (int x = 0; x < outInfo.width; x++)
		{
			for (int y = 0; y < outInfo.height; y++)
			{
				int alpha = *(maskData.planes[0] + (y * maskData.pitches[0]) + x * maskBytesPerPixel);
				alpha = alpha / 255;

				//depth image
				if (outInfo.format == PXCImage::PIXEL_FORMAT_DEPTH) {
					pxcBYTE *pixel = outData.planes[0] + (y * outData.pitches[0]) + x * 2;
					if (alpha == 1) {
						pixel[0] = 153;
						pixel[1] = 0;
					}
				}
				//color image
				else {
					pxcBYTE *pixel = outData.planes[0] + (y * outData.pitches[0]) + x * 3;
					for (int ch = 0; ch < 3; ch++)
					{
						pixel[ch] = (pxcBYTE)((alpha*pixel[ch]) + (1 - alpha)*((pixel[ch] >> 4) + GREY));
					}
				}

			}

		}

		base->ReleaseAccess(&outData);
		mask->ReleaseAccess(&maskData);
	}
	else
	{
		if (maskSts == PXC_STATUS_NO_ERROR)
		{
			mask->ReleaseAccess(&maskData);
		}
	}
}


void PersonTrackingRenderer::Render()
{
	DrawFrameRate();
	DrawGraphics(m_currentFrameOutput);
	RefreshUserInterPerson();
}

void PersonTrackingRenderer::Reset()
{
	m_detectionCounter = 0;
}

void PersonTrackingRenderer::DrawFrameRate()
{
	m_frameRateCalcuator.Tick();
	if (m_frameRateCalcuator.IsFrameRateReady())
	{
		int fps = m_frameRateCalcuator.GetFrameRate();

		pxcCHAR line[1024];
		swprintf_s<1024>(line, L"Rate (%d fps)", fps);
		PersonTrackingUtilities::SetStatus(m_window, line, statusPart);
	}
}

void PersonTrackingRenderer::DrawFrameNumber(int num)
{
	pxcCHAR line[1024];
	swprintf_s<1024>(line, L"%d", num);
	SetDlgItemText(m_window, IDC_FRAMENUM, line);
}

void PersonTrackingRenderer::RefreshUserInterPerson()
{
	if (!m_bitmap) return;

	HWND panel = GetDlgItem(m_window, IDC_PANEL);
	RECT rc;
	GetClientRect(panel, &rc);

	HDC dc = GetDC(panel);
	if (!dc)
	{
		return;
	}

	HBITMAP bitmap = CreateCompatibleBitmap(dc, rc.right, rc.bottom);

	if (!bitmap)
	{

		ReleaseDC(panel, dc);
		return;
	}
	HDC dc2 = CreateCompatibleDC(dc);
	if (!dc2)
	{
		DeleteObject(bitmap);
		ReleaseDC(m_window, dc);
		return;
	}
	SelectObject(dc2, bitmap);
	SetStretchBltMode(dc2, COLORONCOLOR);

	/* Draw the main window */
	HDC dc3 = CreateCompatibleDC(dc);

	if (!dc3)
	{
		DeleteDC(dc2);
		DeleteObject(bitmap);
		ReleaseDC(m_window, dc);
		return;
	}

	SelectObject(dc3, m_bitmap);
	BITMAP bm;
	GetObject(m_bitmap, sizeof(BITMAP), &bm);

	bool scale = Button_GetState(GetDlgItem(m_window, IDC_SCALE)) & BST_CHECKED;
	if (scale)
	{
		RECT rc1 = GetResizeRect(rc, bm);
		StretchBlt(dc2, rc1.left, rc1.top, rc1.right, rc1.bottom, dc3, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
	}
	else
	{
		BitBlt(dc2, 0, 0, rc.right, rc.bottom, dc3, 0, 0, SRCCOPY);
	}

	DeleteDC(dc3);
	DeleteDC(dc2);
	ReleaseDC(m_window, dc);

	HBITMAP bitmap2 = (HBITMAP)SendMessage(panel, STM_GETIMAGE, 0, 0);
	if (bitmap2) DeleteObject(bitmap2);
	SendMessage(panel, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bitmap);
	InvalidateRect(panel, 0, TRUE);
	DeleteObject(bitmap);
}

RECT PersonTrackingRenderer::GetResizeRect(RECT rectangle, BITMAP bitmap)
{
	RECT resizedRectangle;
	float sx = (float)rectangle.right / (float)bitmap.bmWidth;
	float sy = (float)rectangle.bottom / (float)bitmap.bmHeight;
	float sxy = sx < sy ? sx : sy;
	resizedRectangle.right = (int)(bitmap.bmWidth * sxy);
	resizedRectangle.left = (rectangle.right - resizedRectangle.right) / 2 + rectangle.left;
	resizedRectangle.bottom = (int)(bitmap.bmHeight * sxy);
	resizedRectangle.top = (rectangle.bottom - resizedRectangle.bottom) / 2 + rectangle.top;
	return resizedRectangle;
}


