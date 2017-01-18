/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2012-2013 Intel Corporation. All Rights Reserved.

*******************************************************************************/

#include <sstream>
#include "ProfileSetMap.h"

#include "pxccapture.h"
#include "pxcsession.h"

ProfileSetMap::ProfileSetMap(PXCSession * session) :
	m_session(session)
{
}

ProfileSetMap::~ProfileSetMap()
{
}

int ProfileSetMap::Init()
{
	if (m_session == NULL)
	{
		return -1;
	}
	PXCSession::ImplDesc desc;
	memset(&desc, 0, sizeof(desc));
	//scan all connected cameras
	for (int i = 0; ; ++i)
	{
		PXCSession::ImplDesc desc1;
		if (m_session->QueryImpl(&desc, i, &desc1) < PXC_STATUS_NO_ERROR)
			break;

		PXCCapture* capture;
		if (m_session->CreateImpl<PXCCapture>(&desc1, &capture) < PXC_STATUS_NO_ERROR)
			continue;

		for (int j = 0; ; j++)
		{
			//scan all "subdevices"
			PXCCapture::Device* device = capture->CreateDevice(j);
			PXCCapture::DeviceInfo deviceInfo;
			if (capture->QueryDeviceInfo(j, &deviceInfo) < PXC_STATUS_NO_ERROR)
			{
				break;
			}
			std::list<PXCCapture::Device::StreamProfileSet> supportedResolutions;
			// scan supported color resolutions for current device
			for (int k = 0; k < device->QueryStreamProfileSetNum(PXCCapture::STREAM_TYPE_COLOR); ++k)
			{
				PXCCapture::Device::StreamProfileSet profileSet;
				if (device->QueryStreamProfileSet(PXCCapture::STREAM_TYPE_COLOR, k, &profileSet) < PXC_STATUS_NO_ERROR)
				{
					device->Release();
					continue;
				}
				if (IsProfileSetSupported(profileSet, deviceInfo))
				{

					supportedResolutions.push_back(profileSet);
				}
			}
			if (deviceInfo.name != NULL)
			{
				m_resolutionParametersMap.insert(std::pair <std::wstring, std::list<PXCCapture::Device::StreamProfileSet>>(std::wstring(deviceInfo.name), supportedResolutions));
			}
			device->Release();
		}
		capture->Release();
	}
	return 0;
}

const std::list<PXCCapture::Device::StreamProfileSet>& ProfileSetMap::GetSupportedProfileSets(std::wstring deviceName)
{
	using namespace std;
	auto it = m_resolutionParametersMap.find(deviceName);
	return (it != m_resolutionParametersMap.end()) ? it->second : m_emptyList;
}


bool ProfileSetMap::IsProfileSetSupported(const PXCCapture::Device::StreamProfileSet& profileSet, const PXCCapture::DeviceInfo& dinfo)
{
	if (profileSet.color.imageInfo.format != PXCImage::PIXEL_FORMAT_RGB32 || profileSet.color.options == PXCCapture::Device::STREAM_OPTION_UNRECTIFIED)
	{
		return false;
	}

	if (profileSet.color.frameRate.min != 30)
	{
		return false;
	}

	if (profileSet.color.imageInfo.width == 1920 && profileSet.color.imageInfo.height == 1080)
	{
		return true;
	}
	if (profileSet.color.imageInfo.width == 1280 && profileSet.color.imageInfo.height == 720)
	{
		return true;
	}
	if (profileSet.color.imageInfo.width == 960 && profileSet.color.imageInfo.height == 540)
	{
		return true;
	}
	if (profileSet.color.imageInfo.width == 640 && profileSet.color.imageInfo.height == 480)
	{
		return true;
	}
	if (profileSet.color.imageInfo.width == 320 && profileSet.color.imageInfo.height == 240)
	{
		return true;
	}
	return false;
}

static const std::wstring PIXEL_FORMAT_YUY2_STR   = L"YUY2";
static const std::wstring PIXEL_FORMAT_NV12_STR    = L"NV12";
static const std::wstring PIXEL_FORMAT_RGB32_STR  = L"RGB32";
static const std::wstring PIXEL_FORMAT_RGB24_STR  = L"RGB24";
static const std::wstring PIXEL_FORMAT_Y8_STR         = L"Y8";
static const std::wstring PIXEL_FORMAT_UNKNOWN_STR = L"UNKNOWN ";

static std::wstring PixelFornatToString(PXCImage::PixelFormat pixelFormat)
{
	switch (pixelFormat)
	{
	case PXCImage::PIXEL_FORMAT_YUY2:
		return PIXEL_FORMAT_YUY2_STR;
	case PXCImage::PIXEL_FORMAT_NV12:
		return PIXEL_FORMAT_NV12_STR;
	case PXCImage::PIXEL_FORMAT_RGB32:
		return PIXEL_FORMAT_RGB32_STR;
	case PXCImage::PIXEL_FORMAT_RGB24:
		return PIXEL_FORMAT_RGB24_STR;
	case PXCImage::PIXEL_FORMAT_Y8:
		return PIXEL_FORMAT_Y8_STR;
	default:
		return PIXEL_FORMAT_UNKNOWN_STR;
	}
}

static std::wostream& operator<<(std::wostream& os, const PXCImage::PixelFormat& obj)
{
	os << PixelFornatToString(obj);
	return os;
}

std::wstring ProfileSetMap::StreamProfileSetToString(const PXCCapture::Device::StreamProfileSet& profileSet)
{
	std::wstringstream wsStream;
	wsStream << profileSet.color.imageInfo.format << " " <<
		profileSet.color.imageInfo.width << L"x" << profileSet.color.imageInfo.height <<
		L" " << profileSet.color.frameRate.min << L"-" << profileSet.color.frameRate.max;
	return wsStream.str();
}

bool ProfileSetMap::GetSupportedProfileByIndex(std::wstring deviceName, int idx, PXCCapture::Device::StreamProfileSet& profileSet)
{
	auto supportedProfiles = GetSupportedProfileSets(deviceName);
	int elementIndex = 0;
	for (auto profileIt = supportedProfiles.begin(); profileIt != supportedProfiles.end(); ++profileIt, ++elementIndex)
	{
		if (elementIndex == idx)
		{
			profileSet = *profileIt;
			return true;
		}
	}
	return false;
}