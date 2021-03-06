#pragma once
#include <Windows.h>
#include <stdio.h>
#include <util_render.h>
#include <PersonTrackingRenderer2D.h>
#include <stdint.h>

void createBlueSample(PXCCapture::Sample *sample);
HWND getHWND(UtilRender utilrender);
void myDrawLocation(PXCPersonTrackingData::PersonTracking* trackedPerson, UtilRender utilrender);
void colorBitmapBlue(PXCCapture::Sample* sample);
