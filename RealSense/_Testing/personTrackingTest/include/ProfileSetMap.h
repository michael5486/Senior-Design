/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2012-2013 Intel Corporation. All Rights Reserved.

*******************************************************************************/
#pragma once
#include <unordered_map>
#include <string>

#include "pxcimage.h"
#include "pxcsession.h"
#include "pxccapture.h"


class ProfileSetMap
{
public:
	ProfileSetMap(PXCSession* session);
	int Init();
	virtual ~ProfileSetMap();
	virtual const std::list<PXCCapture::Device::StreamProfileSet>&  GetSupportedProfileSets(std::wstring deviceName);
	//index is position at list obtained at call to GetSupportedProfileSets
	virtual bool GetSupportedProfileByIndex(std::wstring deviceName, int idx, PXCCapture::Device::StreamProfileSet& profileSet);
	static std::wstring StreamProfileSetToString(const PXCCapture::Device::StreamProfileSet& profileSet);

protected:
	//filters profiles resolutions for resolution map
	virtual bool IsProfileSetSupported(const PXCCapture::Device::StreamProfileSet& profileSet, const PXCCapture::DeviceInfo& dinfo);
private:
	std::unordered_map<std::wstring, std::list<PXCCapture::Device::StreamProfileSet>> m_resolutionParametersMap;
	std::list<PXCCapture::Device::StreamProfileSet> m_emptyList;
	PXCSession* m_session;
};
