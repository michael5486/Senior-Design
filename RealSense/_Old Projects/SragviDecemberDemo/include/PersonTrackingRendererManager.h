#pragma once

#include "PersonTrackingRenderer.h"
#include "PersonTrackingRenderer2D.h"
#include "PersonTrackingRenderer3D.h"

class PersonTrackingRendererManager
{
public:
	PersonTrackingRendererManager(PersonTrackingRenderer2D* renderer2D, PersonTrackingRenderer3D* renderer3D);
	~PersonTrackingRendererManager();

	void SetRendererType(PersonTrackingRenderer::RendererType type);
	PersonTrackingRenderer::RendererType GetRendererType();
	void Render();
	void SetSenseManager(PXCSenseManager* senseManager);
	void SetCallback(OnFinishedRenderingCallback callback);
	void DrawBitmap(PXCCapture::Sample* sample);
	void CreateMarking(PXCPersonTrackingModule* personModule, PXCCapture::Sample* sample);

	void DrawFrameNumber(int num);
	void SetOutput(PXCPersonTrackingData* output);
	void SignalRenderer();

	void Reset();

	static HANDLE& GetRenderingFinishedSignal();
	static void SignalProcessor();

private:
	PersonTrackingRenderer2D* m_renderer2D;
	PersonTrackingRenderer3D* m_renderer3D;
	PersonTrackingRenderer* m_currentRenderer;

	PersonTrackingRenderer::RendererType m_currentType;

	HANDLE m_rendererSignal;
	OnFinishedRenderingCallback m_callback;
};

