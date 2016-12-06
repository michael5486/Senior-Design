#include "PersonTrackingRenderer2D.h"
#include "PersonTrackingUtilities.h"
#include "pxccapture.h"

PersonTrackingRenderer2D::~PersonTrackingRenderer2D()
{
	if (joints) {
		delete[] joints;
		joints = NULL;
	}
	if (m_hFont)
	{
		DeleteObject(m_hFont);
	}
}

PersonTrackingRenderer2D::PersonTrackingRenderer2D(HWND window) : PersonTrackingRenderer(window), joints(NULL), m_hFont(NULL)
{
}

void PersonTrackingRenderer2D::DrawGraphics(PXCPersonTrackingData* personOutput)
{
	assert(personOutput != NULL);
	if (!m_bitmap) return;

	UpdateFont();

	const int numPersons = personOutput->QueryNumberOfPeople();
	for (int i = 0; i < numPersons; ++i) 
	{
 		PXCPersonTrackingData::Person* personData = personOutput->QueryPersonData(PXCPersonTrackingData::ACCESS_ORDER_BY_ID, i);

		assert(personData != NULL);
		PXCPersonTrackingData::PersonTracking* personTracking;
		personTracking = personData->QueryTracking();

		if (!PersonTrackingUtilities::GetRecordState(m_window))
		{
			DrawLocation(personTracking);
			PXCPersonTrackingData::BoundingBox2D headBoundingBox = personTracking->QueryHeadBoundingBox();

			/*
			if (PersonTrackingUtilities::IsModuleSelected(m_window, IDC_HEAD))
			{
				if (headBoundingBox.confidence)
				{
					DrawLocation(headBoundingBox.rect);
				}
			}
			*/
		}

		if (PersonTrackingUtilities::IsModuleSelected(m_window, IDC_RECOGNITION) && personData->QueryRecognition() != NULL)
			DrawRecognition(personData, i);

		DrawCenterOfMass(personTracking, i);	//TODO: katya - do on demand

		if (PersonTrackingUtilities::IsModuleSelected(m_window, IDC_SKELETON)) 
		{
			PXCPersonTrackingData::PersonJoints* personJoints = personData->QuerySkeletonJoints();
			PersonTrackingRenderer2D::DrawSkeleton(personJoints);
		}
		if (PersonTrackingUtilities::IsModuleSelected(m_window, IDC_GESTURE))
		{
			PersonTrackingRenderer2D::DrawGestures(personData->QueryGestures());
		}
		if (PersonTrackingUtilities::IsModuleSelected(m_window, IDC_EXPRESSIONS))
		{
			PersonTrackingRenderer2D::DrawExpressions(personData->QueryExpressions(), personData->QueryTracking()->QueryId());
		}
		if (PersonTrackingUtilities::IsModuleSelected(m_window, IDC_HEADPOSE))
		{
			PersonTrackingRenderer2D::DrawHeadPose(personTracking);
		}
	}
}

void PersonTrackingRenderer2D::DrawSkeleton(PXCPersonTrackingData::PersonJoints* personJoints)
{
	if (personJoints == NULL || personJoints->QueryNumJoints() < 1)
		return;
	if (!m_bitmap) return;

	pxcI32 numPoints = personJoints->QueryNumJoints();

	if (!joints) 
	{
		joints = new PXCPersonTrackingData::PersonJoints::SkeletonPoint[personJoints->QueryNumJoints()];
	}
	personJoints->QueryJoints(joints);

	PXCRectI32 rect;
	rect.w = 4;
	rect.h = 4;

	for (int i = 0; i < numPoints; ++i) 
	{
		rect.x = max(0, joints[i].image.x - 2 - 3);
		rect.y = max(0, joints[i].image.y - 2);

		if (joints[i].confidenceImage > 0)
		{
			if(joints[i].jointType == PXCPersonTrackingData::PersonJoints::JOINT_HAND_LEFT)
				DrawLocation(rect, 0, 0, 255);
			else if(joints[i].jointType == PXCPersonTrackingData::PersonJoints::JOINT_HAND_RIGHT)
				DrawLocation(rect, 255, 0, 0);
			else if(joints[i].jointType == PXCPersonTrackingData::PersonJoints::JOINT_HEAD)
				DrawLocation(rect, 255, 255, 0);
			else if(joints[i].jointType == PXCPersonTrackingData::PersonJoints::JOINT_SPINE_MID)
				DrawLocation(rect, 0, 255, 0);
			else
				DrawLocation(rect, 255, 255, 255);
		}
	}

}

void PersonTrackingRenderer2D::DrawGestures(PXCPersonTrackingData::PersonGestures* personGestures)
{
	if (personGestures == nullptr || !personGestures->IsPointing() || !personGestures->QueryPointingInfo().confidence)
		return;
	PXCPersonTrackingData::PersonGestures::PointingInfo poitingInfo = personGestures->QueryPointingInfo();
	POINT start;
	POINT end;
	start.x = poitingInfo.colorPointingData.origin.x;
	start.y = poitingInfo.colorPointingData.origin.y;
	end.x = start.x + 500 * poitingInfo.colorPointingData.direction.x;
	end.y = start.y + 500 * poitingInfo.colorPointingData.direction.y;
	DrawArrow(start, end, 0, 255, 0);
}

void PersonTrackingRenderer2D::DrawExpressions(PXCPersonTrackingData::PersonExpressions* personExpressions, int personIndex)
{
	HWND panelWindow = GetDlgItem(m_window, IDC_PANEL);
	HDC dc1 = GetDC(panelWindow);
	HDC dc2 = CreateCompatibleDC(dc1);
	if (!dc2)
	{
		ReleaseDC(panelWindow, dc1);
		return;
	}

	SelectObject(dc2, m_bitmap);
	BITMAP bitmap;
	GetObject(m_bitmap, sizeof(bitmap), &bitmap);
	HPEN cyan = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
	if (!cyan)
	{
		DeleteDC(dc2);
		ReleaseDC(panelWindow, dc1);
		return;
	}

	SelectObject(dc2, m_hFont);
	SelectObject(dc2, cyan);

	const int maxColumnDisplayedFaces = 5;
	const int widthColumnMargin = 570;
	const int rowMargin = m_fontHeight;
	const int yStartingPosition = (personIndex % maxColumnDisplayedFaces) * 2 * m_fontHeight;
	const int xStartingPosition = 0;

	SetTextColor(dc2, RGB(0, 0, 0));
	WCHAR tempLine[64];
	int yPosition = yStartingPosition;

	PXCPersonTrackingData::PersonExpressions::PersonExpressionsResult neutralResult;
	bool isNeutralExpressionValid = personExpressions->QueryExpression(PXCPersonTrackingData::PersonExpressions::NEUTRAL, &neutralResult);
	if (isNeutralExpressionValid)
	{
		swprintf_s<sizeof(tempLine) / sizeof(WCHAR) >(tempLine, L"Neutral: %d", neutralResult.confidence);
		TextOut(dc2, xStartingPosition, yPosition, tempLine, (int)std::char_traits<wchar_t>::length(tempLine));
		yPosition += rowMargin;
	}

	PXCPersonTrackingData::PersonExpressions::PersonExpressionsResult happinessResult;
	bool isHappinessExpressionValid = personExpressions->QueryExpression(PXCPersonTrackingData::PersonExpressions::HAPPINESS, &happinessResult);
	if (isHappinessExpressionValid)
	{
		swprintf_s<sizeof(tempLine) / sizeof(WCHAR) >(tempLine, L"Happiness: %d", happinessResult.confidence);
		TextOut(dc2, xStartingPosition, yPosition, tempLine, (int)std::char_traits<wchar_t>::length(tempLine));
		yPosition += rowMargin;
	}
	
	PXCPersonTrackingData::PersonExpressions::PersonExpressionsResult sadnessResult;
	bool isSadnessExpressionValid = personExpressions->QueryExpression(PXCPersonTrackingData::PersonExpressions::SADNESS, &sadnessResult);
	if (isSadnessExpressionValid)
	{
		swprintf_s<sizeof(tempLine) / sizeof(WCHAR) >(tempLine, L"Sadness: %d", sadnessResult.confidence);
		TextOut(dc2, xStartingPosition, yPosition, tempLine, (int)std::char_traits<wchar_t>::length(tempLine));
		yPosition += rowMargin;
	}

	PXCPersonTrackingData::PersonExpressions::PersonExpressionsResult surpriseResult;
	bool isSurpriseExpressionValid = personExpressions->QueryExpression(PXCPersonTrackingData::PersonExpressions::SURPRISE, &surpriseResult);
	if (isSurpriseExpressionValid)
	{
		swprintf_s<sizeof(tempLine) / sizeof(WCHAR) >(tempLine, L"Surprised: %d", surpriseResult.confidence);
		TextOut(dc2, xStartingPosition, yPosition, tempLine, (int)std::char_traits<wchar_t>::length(tempLine));
		yPosition += rowMargin;
	}

	PXCPersonTrackingData::PersonExpressions::PersonExpressionsResult disgustResult;
	bool isDisgustExpressionValid = personExpressions->QueryExpression(PXCPersonTrackingData::PersonExpressions::DISGUST, &disgustResult);
	if (isDisgustExpressionValid)
	{
		swprintf_s<sizeof(tempLine) / sizeof(WCHAR) >(tempLine, L"Disgusted: %d", disgustResult.confidence);
		TextOut(dc2, xStartingPosition, yPosition, tempLine, (int)std::char_traits<wchar_t>::length(tempLine));
		yPosition += rowMargin;
	}

	PXCPersonTrackingData::PersonExpressions::PersonExpressionsResult contemptResult;
	bool isContemptExpressionValid = personExpressions->QueryExpression(PXCPersonTrackingData::PersonExpressions::CONTEMPT, &contemptResult);
	if (isContemptExpressionValid)
	{
		swprintf_s<sizeof(tempLine) / sizeof(WCHAR) >(tempLine, L"Contemptful: %d", contemptResult.confidence);
		TextOut(dc2, xStartingPosition, yPosition, tempLine, (int)std::char_traits<wchar_t>::length(tempLine));
		yPosition += rowMargin;
	}

	PXCPersonTrackingData::PersonExpressions::PersonExpressionsResult fearResult;
	bool isFearExpressionValid = personExpressions->QueryExpression(PXCPersonTrackingData::PersonExpressions::FEAR, &fearResult);
	if (isFearExpressionValid)
	{
		swprintf_s<sizeof(tempLine) / sizeof(WCHAR) >(tempLine, L"Fearful: %d", fearResult.confidence);
		TextOut(dc2, xStartingPosition, yPosition, tempLine, (int)std::char_traits<wchar_t>::length(tempLine));
		yPosition += rowMargin;
	}

	PXCPersonTrackingData::PersonExpressions::PersonExpressionsResult angerResult;
	bool isAngerExpressionValid = personExpressions->QueryExpression(PXCPersonTrackingData::PersonExpressions::ANGER, &angerResult);
	if (isAngerExpressionValid)
	{
		swprintf_s<sizeof(tempLine) / sizeof(WCHAR) >(tempLine, L"Angry: %d", angerResult.confidence);
		TextOut(dc2, xStartingPosition, yPosition, tempLine, (int)std::char_traits<wchar_t>::length(tempLine));
		yPosition += rowMargin;
	}
	
	DeleteObject(cyan);
	DeleteDC(dc2);
	ReleaseDC(panelWindow, dc1);
}

void PersonTrackingRenderer2D::DrawArrow(POINT start, POINT end, int red, int green, int blue)
{
	HWND panelWindow = GetDlgItem(m_window, IDC_PANEL);
	HDC dc1 = GetDC(panelWindow);
	HDC dc2 = CreateCompatibleDC(dc1);

	if (!dc2)
	{
		ReleaseDC(panelWindow, dc1);
		return;
	}

	SelectObject(dc2, m_bitmap);

	BITMAP bitmap;
	GetObject(m_bitmap, sizeof(bitmap), &bitmap);

	HPEN cyan = CreatePen(PS_SOLID, 2, RGB(0, 255, 0));

	if (!cyan)
	{
		DeleteDC(dc2);
		ReleaseDC(panelWindow, dc1);
		return;
	}
	SelectObject(dc2, cyan);

	MoveToEx(dc2, start.x, start.y, nullptr);
	LineTo(dc2, end.x, end.y);

	POINT points[3];
	points[0].x = end.x + 0.1 * (end.x - start.x);
	points[0].y = end.y + 0.1 * -(end.y - start.y);
	points[1].x = end.x + 0.1 * -(end.x - start.x);
	points[1].y = end.y + 0.1 * (end.y - start.y);
	points[2].x = end.x + 0.2 * (end.x - start.x);
	points[2].y = end.y + 0.2 * (end.y - start.y);
	Polygon(dc2, points, 3);

	DeleteObject(cyan);

	DeleteDC(dc2);
	ReleaseDC(panelWindow, dc1);
	DeleteObject(cyan);
}

// red, geeen, blue parameters define the color of rectangle
void PersonTrackingRenderer2D::DrawLocation(PXCRectI32 rectangle, int red, int green, int blue)
{
	HWND panelWindow = GetDlgItem(m_window, IDC_PANEL);
	HDC dc1 = GetDC(panelWindow);
	HDC dc2 = CreateCompatibleDC(dc1);

	if (!dc2) 
	{
		ReleaseDC(panelWindow, dc1);
		return;
	}

	SelectObject(dc2, m_bitmap);

	BITMAP bitmap;
	GetObject(m_bitmap, sizeof(bitmap), &bitmap);

	HPEN cyan = CreatePen(PS_SOLID, 3, RGB(red, green, blue));

	if (!cyan)
	{
		DeleteDC(dc2);
		ReleaseDC(panelWindow, dc1);
		return;
	}
	SelectObject(dc2, cyan);	

	MoveToEx(dc2, rectangle.x, rectangle.y, 0);
	LineTo(dc2, rectangle.x, rectangle.y + rectangle.h);
	LineTo(dc2, rectangle.x + rectangle.w, rectangle.y + rectangle.h);
	LineTo(dc2, rectangle.x + rectangle.w, rectangle.y);
	LineTo(dc2, rectangle.x, rectangle.y);

	DeleteObject(cyan);

	DeleteDC(dc2);
	ReleaseDC(panelWindow, dc1);
	DeleteObject(cyan);
}

void PersonTrackingRenderer2D::DrawBitmap(PXCCapture::Sample* sample)
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
}

void PersonTrackingRenderer2D::Reset()
{
	m_detectionCounter = 0;
}

void PersonTrackingRenderer2D::DrawLocation(PXCPersonTrackingData::PersonTracking* trackedPerson)
{
	PXCPersonTrackingData::BoundingBox2D box = trackedPerson->Query2DBoundingBox();
	++m_detectionCounter;
	HWND panelWindow = GetDlgItem(m_window, IDC_PANEL);
	HDC dc1 = GetDC(panelWindow);
	HDC dc2 = CreateCompatibleDC(dc1);

	if (!dc2)
	{
		ReleaseDC(panelWindow, dc1);
		return;
	}

	SelectObject(dc2, m_bitmap);

	BITMAP bitmap;
	GetObject(m_bitmap, sizeof(bitmap), &bitmap);
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
		ReleaseDC(panelWindow, dc1);
		return;
	}
	SelectObject(dc2, cyan);

	MoveToEx(dc2, box.rect.x, box.rect.y, 0);
	LineTo(dc2, box.rect.x, box.rect.y + box.rect.h);
	LineTo(dc2, box.rect.x + box.rect.w, box.rect.y + box.rect.h);
	LineTo(dc2, box.rect.x + box.rect.w, box.rect.y);
	LineTo(dc2, box.rect.x, box.rect.y);

	WCHAR line[64];
	SelectObject(dc2, m_hFont);
	if (trackedPerson->QueryId() >= 0)
	{
		swprintf_s<sizeof(line) / sizeof(pxcCHAR)>(line, L"%d", trackedPerson->QueryId());
		TextOut(dc2, box.rect.x, box.rect.y, line,  std::char_traits<wchar_t>::length(line));
	}

	DeleteObject(cyan);

	DeleteDC(dc2);
	ReleaseDC(panelWindow, dc1);
}

void PersonTrackingRenderer2D::DrawHeadLocation(PXCPersonTrackingData::PersonTracking* trackedPerson)
{
	PXCPersonTrackingData::BoundingBox2D headRect = trackedPerson->QueryHeadBoundingBox();
	if (!headRect.confidence)
		return;

	HWND panelWindow = GetDlgItem(m_window, IDC_PANEL);
	HDC dc1 = GetDC(panelWindow);
	HDC dc2 = CreateCompatibleDC(dc1);

	if (!dc2) 
	{
		ReleaseDC(panelWindow, dc1);
		return;
	}

	SelectObject(dc2, m_bitmap);

	BITMAP bitmap;
	GetObject(m_bitmap, sizeof(bitmap), &bitmap);

	HPEN cyan = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));

	if (!cyan)
	{
		DeleteDC(dc2);
		ReleaseDC(panelWindow, dc1);
		return;
	}
	SelectObject(dc2, cyan);	

	PXCRectI32 rectangle = headRect.rect;
	MoveToEx(dc2, rectangle.x, rectangle.y, 0);
	LineTo(dc2, rectangle.x, rectangle.y + rectangle.h);
	LineTo(dc2, rectangle.x + rectangle.w, rectangle.y + rectangle.h);
	LineTo(dc2, rectangle.x + rectangle.w, rectangle.y);
	LineTo(dc2, rectangle.x, rectangle.y);

	DeleteObject(cyan);

	DeleteDC(dc2);
	ReleaseDC(panelWindow, dc1);
}

void PersonTrackingRenderer2D::DrawCenterOfMass(PXCPersonTrackingData::PersonTracking* trackedPerson, int index)
{
	HWND panelWindow = GetDlgItem(m_window, IDC_PANEL);
	HDC dc1 = GetDC(panelWindow);
	HDC dc2 = CreateCompatibleDC(dc1);

	if (!dc2) 
	{
		ReleaseDC(panelWindow, dc1);
		return;
	}

	SelectObject(dc2, m_bitmap);

	BITMAP bitmap;
	GetObject(m_bitmap, sizeof(bitmap), &bitmap);

	HPEN cyan = CreatePen(PS_SOLID, 3, RGB(255 ,255 , 0));

	if (!cyan)
	{
		DeleteDC(dc2);
		ReleaseDC(panelWindow, dc1);
		return;
	}
	SelectObject(dc2, cyan);

	WCHAR line[200];

	PXCPersonTrackingData::PersonTracking::PointCombined centerMass = trackedPerson->QueryCenterMass();
	int personId = trackedPerson->QueryId();
	const int maxColumnDisplayedPersons = 5;
	const int widthColumnMargin = 500;
	const int numRows = 2;
	const int rowMargin = PersonTrackingUtilities::TextHeight;
	//const int yStartingPosition = rowMargin + index % maxColumnDisplayedPersons * numRows * PersonTrackingUtilities::TextHeight;
	//const int xStartingPosition = widthColumnMargin / maxColumnDisplayedPersons;
	const int yStartingPosition = trackedPerson->Query2DBoundingBox().rect.y;
	const int xStartingPosition = trackedPerson->Query2DBoundingBox().rect.x;
	SelectObject(dc2, m_hFont);
	if (trackedPerson->QueryId() >= 0)
	{
		if ( !(PersonTrackingUtilities::IsModuleSelected(m_window,IDC_SKELETON)) &&	!(PersonTrackingUtilities::IsModuleSelected(m_window,IDC_GESTURE ))){
			swprintf_s<sizeof(line) / sizeof(WCHAR) >(line, L"ID: %d\nDistance: %.2f\nx = %.2f\ny = %.2f ", trackedPerson->QueryId(), centerMass.world.point.z, centerMass.world.point.x, centerMass.world.point.y);
			RECT rect;
			rect.top = trackedPerson->Query2DBoundingBox().rect.y + trackedPerson->Query2DBoundingBox().rect.h * 0.66 ;
			rect.bottom = rect.top +  trackedPerson->Query2DBoundingBox().rect.h;
			rect.left = trackedPerson->Query2DBoundingBox().rect.x;
			rect.right = rect.left + trackedPerson->Query2DBoundingBox().rect.w * 4;
			DrawText(dc2, line, std::char_traits<wchar_t>::length(line), &rect, DT_BOTTOM);
			DeleteObject(cyan);

			SetBkMode(dc2, TRANSPARENT);

			SetTextColor(dc2, RGB(255, 0, 0));
			TextOut(dc2, centerMass.image.point.x, centerMass.image.point.y, L"•", 1);
		}

	}

	if (cyan != NULL)
	{
		DeleteObject(cyan);
	}

	DeleteDC(dc2);
	ReleaseDC(panelWindow, dc1);
}

void PersonTrackingRenderer2D::DrawRecognition(PXCPersonTrackingData::Person* personData, int index)
{
	PXCPersonTrackingData::PersonRecognition* recognitionData = personData->QueryRecognition();

	HWND panelWindow = GetDlgItem(m_window, IDC_PANEL);
	HDC dc1 = GetDC(panelWindow);

	if (!dc1)
	{
		return;
	}
	HDC dc2 = CreateCompatibleDC(dc1);
	if (!dc2)
	{
		ReleaseDC(panelWindow, dc1);
		return;
	}

	SelectObject(dc2, m_bitmap);

	BITMAP bitmap;
	GetObject(m_bitmap, sizeof(bitmap), &bitmap);

	SelectObject(dc2, m_hFont);

	WCHAR line1[64];
	int recognitionID = recognitionData->QueryRecognitionID();
	if (recognitionID != -1)
	{
		swprintf_s<sizeof(line1) / sizeof(pxcCHAR)>(line1, L"Registered ID: %d", recognitionID);
	}
	else
	{
		swprintf_s<sizeof(line1) / sizeof(pxcCHAR)>(line1, L"Not Registered");
	}

	PXCRectI32 rect = personData->QueryTracking()->Query2DBoundingBox().rect;
	int yStartingPosition;
	if (personData->QueryTracking()->Query2DBoundingBox().confidence)
	{
		SetBkMode(dc2, TRANSPARENT);
		yStartingPosition = rect.y - PersonTrackingUtilities::TextHeight;
	}
	else
	{
		const int yBasePosition = bitmap.bmHeight - PersonTrackingUtilities::TextHeight;
		yStartingPosition = yBasePosition - index * PersonTrackingUtilities::TextHeight;
		WCHAR userLine[64];
		swprintf_s<sizeof(userLine) / sizeof(pxcCHAR)>(userLine, L" User: %d", personData->QueryTracking()->QueryId());
		wcscat_s(line1, userLine);
	}
	SIZE textSize;
	GetTextExtentPoint32(dc2, line1, std::char_traits<wchar_t>::length(line1), &textSize);
	int x = rect.x + rect.w + 1;
	if (x + textSize.cx > bitmap.bmWidth)
		x = rect.x - 1 - textSize.cx;

	TextOut(dc2, x, yStartingPosition, line1, std::char_traits<wchar_t>::length(line1));

	DeleteDC(dc2);
	ReleaseDC(panelWindow, dc1);
}

void PersonTrackingRenderer2D::DrawHeadPose(PXCPersonTrackingData::PersonTracking* trackedPerson)
{
	HWND panelWindow = GetDlgItem(m_window, IDC_PANEL);
	HDC dc1 = GetDC(panelWindow);
	HDC dc2 = CreateCompatibleDC(dc1);

	if (!dc2)
	{
		ReleaseDC(panelWindow, dc1);
		return;
	}

	SelectObject(dc2, m_bitmap);

	BITMAP bitmap;
	GetObject(m_bitmap, sizeof(bitmap), &bitmap);

	WCHAR line[64];
	SelectObject(dc2, m_hFont);
	PXCPersonTrackingData::BoundingBox2D box = trackedPerson->Query2DBoundingBox();
	if (trackedPerson->QueryId() >= 0)
	{
		PXCPersonTrackingData::PoseEulerAngles angles;
		if (trackedPerson->QueryHeadPose(angles))
		{
			RECT rect;
			rect.top = box.rect.y + box.rect.h * 0.3;
			rect.bottom = rect.top + box.rect.h;
			rect.left = box.rect.x;
			rect.right = box.rect.w * 4;
			swprintf_s<sizeof(line) / sizeof(pxcCHAR)>(line, L"Head Pose:\nYaw %.2f\nPitch %.2f\nRoll %.2f", angles.yaw, angles.pitch, angles.roll);
			DrawText(dc2, line, std::char_traits<wchar_t>::length(line), &rect, DT_BOTTOM);
		}
	}

	DeleteDC(dc2);
	ReleaseDC(panelWindow, dc1);
}

void PersonTrackingRenderer2D::UpdateFont()
{
	if (m_bitmap == NULL)
	{
		return;
	}
	if (m_hFont != NULL)
	{
		DeleteObject(m_hFont);
	}
	BITMAP bm;
	GetObject(m_bitmap, sizeof(BITMAP), &bm);
	m_fontWidth = 24.0 / 1920.0 * bm.bmWidth;
	m_fontHeight = 48.0 / 1080.0 * bm.bmHeight;
	m_hFont = CreateFont(m_fontHeight, m_fontWidth, 0, 0, FW_LIGHT, 0, 0, 0, 0, 0, 0, 2, 0, L"MONOSPACE");
}


PXCImage* PersonTrackingRenderer2D::GetRelevantMask(PXCPersonTrackingData::Person* person) {
	return person->QueryTracking()->QuerySegmentationImage();
}


PXCImage* PersonTrackingRenderer2D::GetRelevantImage(PXCCapture::Sample* sample) {
	return sample->color;
}
