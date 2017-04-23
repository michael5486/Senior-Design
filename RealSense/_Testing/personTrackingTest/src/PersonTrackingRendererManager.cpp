#include "PersonTrackingRendererManager.h"

extern HANDLE ghMutex;

PersonTrackingRendererManager::PersonTrackingRendererManager(PersonTrackingRenderer2D* renderer2D, PersonTrackingRenderer3D* renderer3D) :
	m_renderer2D(renderer2D), m_renderer3D(renderer3D), m_currentRenderer(NULL)
{
	m_rendererSignal = CreateEvent(NULL, FALSE, FALSE, NULL);
}

PersonTrackingRendererManager::~PersonTrackingRendererManager(void)
{
	CloseHandle(m_rendererSignal);

	if(m_renderer2D != NULL)
		delete m_renderer2D;

	if(m_renderer3D != NULL)
		delete m_renderer3D;
}

void PersonTrackingRendererManager::SetRendererType(PersonTrackingRenderer::RendererType type)
{
	//DWORD dwWaitResult;
	//dwWaitResult = WaitForSingleObject(ghMutex,	INFINITE);
	//if(dwWaitResult == WAIT_OBJECT_0)
	{
		if(type == PersonTrackingRenderer::R2D)
		{
			m_currentRenderer = m_renderer2D;
			m_currentType = PersonTrackingRenderer::R2D;
		}
		else
		{
			m_currentRenderer = m_renderer3D;
			m_currentType = PersonTrackingRenderer::R3D;
		}
		/*if(!ReleaseMutex(ghMutex))
		{
			throw std::exception("Failed to release mutex");
			return;
		}*/
	}
}

void PersonTrackingRendererManager::Render()
{
	WaitForSingleObject(m_rendererSignal, INFINITE);
	m_currentRenderer->Render();

	m_callback();
}

void PersonTrackingRendererManager::SetSenseManager(PXCSenseManager* senseManager)
{
	m_renderer2D->SetSenseManager(senseManager);
	m_renderer3D->SetSenseManager(senseManager);
}


void PersonTrackingRendererManager::SetCallback(OnFinishedRenderingCallback callback)
{
	m_callback = callback;
}

void PersonTrackingRendererManager::DrawBitmap(PXCCapture::Sample* sample)
{
	m_currentRenderer->DrawBitmap(sample);
}

void PersonTrackingRendererManager::DrawFrameNumber(int num)
{
	m_currentRenderer->DrawFrameNumber(num);
}

void PersonTrackingRendererManager::SetOutput(PXCPersonTrackingData* output)
{
	m_renderer2D->SetOutput(output);
	m_renderer3D->SetOutput(output);
}

void PersonTrackingRendererManager::SignalRenderer()
{
	SetEvent(m_rendererSignal);
}

void PersonTrackingRendererManager::Reset()
{
	m_renderer2D->Reset();
}

void PersonTrackingRendererManager::SignalProcessor()
{
	SetEvent(GetRenderingFinishedSignal());
}

HANDLE& PersonTrackingRendererManager::GetRenderingFinishedSignal()
{
	static HANDLE renderingFinishedSignal = CreateEvent(NULL, FALSE, TRUE, NULL);
	return renderingFinishedSignal;
}

void PersonTrackingRendererManager::CreateMarking(PXCPersonTrackingModule* personModule, PXCCapture::Sample* sample) {
	m_currentRenderer->CreateMarking(personModule, sample);
}


PersonTrackingRenderer::RendererType PersonTrackingRendererManager::GetRendererType() {
	return m_currentType;
}