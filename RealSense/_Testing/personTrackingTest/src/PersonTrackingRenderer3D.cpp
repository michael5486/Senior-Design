#include "PersonTrackingRenderer3D.h"
#include "PersonTrackingUtilities.h"
#include "pxcprojection.h"


PersonTrackingRenderer3D::~PersonTrackingRenderer3D()
{
}

PersonTrackingRenderer3D::PersonTrackingRenderer3D(HWND window, PXCSession* session) : PersonTrackingRenderer(window), m_session(session)
{
}

bool PersonTrackingRenderer3D::ProjectVertex(const PXCPoint3DF32 &v, int &x, int &y, int radius)
{
	x = int(m_outputImageInfo.width * (0.5f + 0.001f * v.x));
	y = int(m_outputImageInfo.height * (0.5f - 0.001f * v.y));

	return ((radius <= x) && (x < m_outputImageInfo.width - radius) && (radius <= y) && (y < m_outputImageInfo.height - radius));
}


void PersonTrackingRenderer3D::DrawGraphics(PXCPersonTrackingData* personOutput)
{
	assert(personOutput != NULL);
	if (!m_bitmap) return;

	const int numPersons = personOutput->QueryNumberOfPeople();
	for (int i = 0; i < numPersons; ++i)
	{
		PXCPersonTrackingData::Person* personData;
		personData = personOutput->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, i);
		assert(personData != NULL);
		PXCPersonTrackingData::PersonTracking* personTracking;
		personTracking = personData->QueryTracking();

		if (PersonTrackingUtilities::IsModuleSelected(m_window, IDC_SKELETON))
		{
			PXCPersonTrackingData::PersonJoints* personJoints = personData->QuerySkeletonJoints();
			DrawSkeleton(personJoints);
		}

		DrawCenterOfMass(personTracking, i);
	}
}



void PersonTrackingRenderer3D::DrawBitmap(PXCCapture::Sample* sample)
{
	if (m_bitmap)
	{
		DeleteObject(m_bitmap);
		m_bitmap = 0;
	}

	PXCImage* image = sample->depth;


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
}


/*

void PersonTrackingRenderer3D::DrawBitmap(PXCCapture::Sample* sample)
{
	PXCImage *image = sample->depth;
	assert(image);
	if (m_bitmap)
	{
		DeleteObject(m_bitmap);
		m_bitmap = 0;
	}

	PXCImage::ImageInfo info = image->QueryInfo();

	m_outputImageInfo.width = 1024;
	m_outputImageInfo.height = 1024;
	m_outputImageInfo.format = PXCImage::PIXEL_FORMAT_RGB32;
	m_outputImageInfo.reserved = 0;

	m_outputImage = m_session->CreateImage(&m_outputImageInfo);
	assert(m_outputImage);
	
	PXCImage::ImageData data;
	if(image->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB32, &data) >= PXC_STATUS_NO_ERROR)
	{
		memset(&m_outputImageData, 0, sizeof(m_outputImageData));
		pxcStatus status = m_outputImage->AcquireAccess(PXCImage::ACCESS_WRITE, PXCImage::PIXEL_FORMAT_RGB32, &m_outputImageData);
		if(status < PXC_STATUS_NO_ERROR) return;

		int stridePixels = m_outputImageData.pitches[0];
		pxcBYTE *pixels = reinterpret_cast<pxcBYTE*> (m_outputImageData.planes[0]);
		memset(pixels, 0, stridePixels * m_outputImageInfo.height);

		// get access to depth data
		PXCPoint3DF32* vertices = new PXCPoint3DF32[info.width * info.height];
		PXCProjection* projection(m_senseManager->QueryCaptureManager()->QueryDevice()->CreateProjection());
		if (!projection)
		{
			if (vertices) delete[] vertices;
			return;
		}

		projection->QueryVertices(image, vertices);
		projection->Release();
		int strideVertices = info.width;

		// render vertices
		int numVertices = 0;
		for (int y = 0; y < info.height; y++)
		{
			const PXCPoint3DF32 *verticesRow = vertices + y * strideVertices;
			for (int x = 0; x < info.width; x++)
			{
				const PXCPoint3DF32 &v = verticesRow[x];
				if (v.z <= 0.0f)
				{
					continue;
				}

				int ix = 0, iy = 0;
				if(ProjectVertex(v, ix, iy))
				{
					pxcBYTE *ptr = m_outputImageData.planes[0];
					ptr += iy * m_outputImageData.pitches[0];
					ptr += ix * 4;
					ptr[0] = pxcBYTE(255.0f * 0.5f);
					ptr[1] = pxcBYTE(255.0f * 0.5f);
					ptr[2] = pxcBYTE(255.0f * 0.5f);
					ptr[3] = pxcBYTE(255.0f);
				}

				numVertices++;
			}
		}
		if (vertices) delete[] vertices;

		if (m_bitmap) 
		{
			DeleteObject(m_bitmap);
			m_bitmap = 0;
		}

		HWND hwndPanel = GetDlgItem(m_window, IDC_PANEL);
		HDC dc = GetDC(hwndPanel);
		BITMAPINFO binfo;
		memset(&binfo, 0, sizeof(binfo));
		binfo.bmiHeader.biWidth = m_outputImageData.pitches[0]/4;
		binfo.bmiHeader.biHeight = - (int)m_outputImageInfo.height;
		binfo.bmiHeader.biBitCount = 32;
		binfo.bmiHeader.biPlanes = 1;
		binfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		binfo.bmiHeader.biCompression = BI_RGB;
		Sleep(1);
		m_bitmap = CreateDIBitmap(dc, &binfo.bmiHeader, CBM_INIT, m_outputImageData.planes[0], &binfo, DIB_RGB_COLORS);

		ReleaseDC(hwndPanel, dc);

		m_outputImage->ReleaseAccess(&m_outputImageData);
		image->ReleaseAccess(&data);
		m_outputImage->Release();
	}
}

*/


void PersonTrackingRenderer3D::DrawCenterOfMass(PXCPersonTrackingData::PersonTracking* trackedPerson, int index)
{

}

void PersonTrackingRenderer3D::DrawSkeleton(PXCPersonTrackingData::PersonJoints* personJoints)
{

}


void PersonTrackingRenderer3D::DrawPerson(PXCPersonTrackingData::Person* trackedPerson)
{
	/*HWND panelWindow = GetDlgItem(m_window, IDC_PANEL);
	HDC dc1 = GetDC(panelWindow);
	HDC dc2 = CreateCompatibleDC(dc1);

	if (!dc2)
	{
	ReleaseDC(panelWindow, dc1);
	return;
	}

	HFONT hFont = CreateFont(28, 18, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, L"MONOSPACE");

	if (!hFont)
	{
	DeleteDC(dc2);
	ReleaseDC(panelWindow, dc1);
	return;
	}

	SetBkMode(dc2, TRANSPARENT);

	SelectObject(dc2, m_bitmap);
	SelectObject(dc2, hFont);

	BITMAP bitmap;
	GetObject(m_bitmap, sizeof(bitmap), &bitmap);

	const PXCPersonTrackingData::PersonData *PersonData = trackedPerson->QueryPerson();

	if(PersonData == NULL)
	{
	DeleteObject(hFont);
	DeleteDC(dc2);
	ReleaseDC(panelWindow, dc1);
	return;
	}
	PXCPersonTrackingData::HeadPosition outPersonCenterPoint;
	PersonData->QueryHeadPosition(&outPersonCenterPoint);

	int headCenter_x = 0, headCenter_y = 0;
	if(ProjectVertex(outPersonCenterPoint.headCenter, headCenter_x, headCenter_y, 2))
	{
	if(PersonData->QueryConfidence() > 0 && outPersonCenterPoint.confidence > 0)
	{
	SetTextColor(dc2, RGB(0, 0, 255));
	TextOut(dc2, headCenter_x, headCenter_y, L"•", 1);
	}
	else
	{
	SetTextColor(dc2, RGB(255, 255, 255));
	TextOut(dc2, headCenter_x, headCenter_y, L"x", 1);
	}

	const PXCPersonTrackingData::LandmarksData *landmarkData = trackedPerson->QueryLandmarks();

	if (!landmarkData)
	{
	DeleteObject(hFont);
	DeleteDC(dc2);
	ReleaseDC(panelWindow, dc1);
	return;
	}

	PXCPersonTrackingData::LandmarkPoint* points = new PXCPersonTrackingData::LandmarkPoint[landmarkData->QueryNumPoints()];

	points[29].world.x = 0.0;
	points[29].world.y = 0.0;
	points[29].world.z = 0.0;

	landmarkData->QueryPoints(points); //data set for all landmarks in frame

	points[29].world.x *= 1000.0f;
	points[29].world.y *= 1000.0f;
	points[29].world.z *= 1000.0f;

	int noseTip_x = 0, noseTip_y = 0;

	if(ProjectVertex(points[29].world, noseTip_x, noseTip_y, 1))
	{
	PXCPoint3DF32 direction;
	direction.x = headCenter_x - noseTip_x;
	direction.y = headCenter_y - noseTip_y;

	HPEN lineColor;

	if (PersonData->QueryConfidence() > 0)
	{
	lineColor = CreatePen(PS_SOLID, 3, RGB(0 ,255 ,255));
	}
	else
	{
	lineColor = CreatePen(PS_SOLID, 3, RGB(255 ,0 , 0));
	}

	if (!lineColor)
	{
	DeleteObject(hFont);
	DeleteDC(dc2);
	ReleaseDC(panelWindow, dc1);
	return;
	}
	SelectObject(dc2, lineColor);

	MoveToEx(dc2, noseTip_x, noseTip_y, 0);
	LineTo(dc2, noseTip_x + 1.2 * direction.x, noseTip_y + 1.2 * direction.y);

	DeleteObject(lineColor);
	}
	}

	DeleteObject(hFont);
	DeleteDC(dc2);
	ReleaseDC(panelWindow, dc1);*/
}



PXCImage* PersonTrackingRenderer3D::GetRelevantMask(PXCPersonTrackingData::Person* person) {
	return person->QueryTracking()->QueryBlobMask();
}


PXCImage* PersonTrackingRenderer3D::GetRelevantImage(PXCCapture::Sample* sample) {
	return sample->depth;
}
