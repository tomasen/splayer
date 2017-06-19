// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"



#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// _AFXCTL_AMBIENT_CACHE implementation

THREAD_LOCAL(_AFXCTL_AMBIENT_CACHE, _afxAmbientCache)

_AFXCTL_AMBIENT_CACHE::_AFXCTL_AMBIENT_CACHE()
{
}

void _AFXCTL_AMBIENT_CACHE::Cache(QACONTAINER* pQAContainer)
{
	m_bValid = (pQAContainer != NULL);
	if (m_bValid)
	{
		m_dwAmbientFlags = pQAContainer->dwAmbientFlags;
		m_colorFore = pQAContainer->colorFore;
		m_colorBack = pQAContainer->colorBack;
		m_pFont = pQAContainer->pFont;
		m_pReserved = pQAContainer->pUndoMgr;
		m_dwAppearance = pQAContainer->dwAppearance;
		if (m_pFont != NULL)
			m_pFont->AddRef();
	}
	else
	{
		if (m_pFont != NULL)
			m_pFont->Release();
		m_dwAmbientFlags = 0;
		m_colorFore = 0;
		m_colorBack = 0;
		m_pFont = NULL;
		m_pReserved = NULL;
		m_dwAppearance = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////
// COleControl::XQuickActivate

STDMETHODIMP_(ULONG) COleControl::XQuickActivate::AddRef()
{
	// Delegate to our exported AddRef.
	METHOD_PROLOGUE_EX_(COleControl, QuickActivate)
	return (ULONG)pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) COleControl::XQuickActivate::Release()
{
	// Delegate to our exported Release.
	METHOD_PROLOGUE_EX_(COleControl, QuickActivate)
	return (ULONG)pThis->ExternalRelease();
}

STDMETHODIMP COleControl::XQuickActivate::QueryInterface(
	REFIID iid, LPVOID* ppvObj)
{
	// Delegate to our exported QueryInterface.
	METHOD_PROLOGUE_EX_(COleControl, QuickActivate)
	return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP COleControl::XQuickActivate::QuickActivate(
	QACONTAINER *pQAContainer, QACONTROL *pQAControl)
{
	METHOD_PROLOGUE_EX_(COleControl, QuickActivate)

	// Get the IOleObject interface
	HRESULT hr = S_OK;
	IOleObject* pOleObject = NULL;
	if (FAILED(hr = pThis->ExternalQueryInterface(&IID_IOleObject,
		reinterpret_cast<void**>(&pOleObject))))
	{
		return hr;
	}

	// Keep copy of ambient properties
	_afxAmbientCache->Cache(pQAContainer);

	// Set client site
	ASSERT(pOleObject != NULL);
	pOleObject->SetClientSite(pQAContainer->pClientSite);

	// Establish connections
	DWORD dwDummy;
	if (pQAContainer->pAdviseSink != NULL)
		pOleObject->Advise(pQAContainer->pAdviseSink, &dwDummy);

	if (pQAContainer->pPropertyNotifySink != NULL)
		pThis->m_xPropConnPt.m_xConnPt.Advise(pQAContainer->pPropertyNotifySink,
			&pQAControl->dwPropNotifyCookie);
	if (pQAContainer->pUnkEventSink != NULL)
		pThis->m_xEventConnPt.m_xConnPt.Advise(pQAContainer->pUnkEventSink,
			&pQAControl->dwEventCookie);

	// Fill in return values
	IViewObjectEx* pViewObject;
	if (SUCCEEDED(pThis->ExternalQueryInterface(&IID_IViewObjectEx,
		reinterpret_cast<void**>(&pViewObject))))
	{
		pViewObject->GetViewStatus(&pQAControl->dwViewStatus);

		// Set advise sink on IViewObject, while we're here.
		if (pQAContainer->pAdviseSink != NULL)
			pViewObject->SetAdvise(DVASPECT_CONTENT, 0, pQAContainer->pAdviseSink);

		pViewObject->Release();
	}
	else
	{
		pQAControl->dwViewStatus = 0;
	}

	pOleObject->GetMiscStatus(DVASPECT_CONTENT, &pQAControl->dwMiscStatus);

	pOleObject->Release();

	return S_OK;
}

STDMETHODIMP COleControl::XQuickActivate::SetContentExtent(LPSIZEL lpsizel)
{
	METHOD_PROLOGUE_EX_(COleControl, QuickActivate)
	return pThis->m_xOleObject.SetExtent(DVASPECT_CONTENT, lpsizel);
}

STDMETHODIMP COleControl::XQuickActivate::GetContentExtent(LPSIZEL lpsizel)
{
	METHOD_PROLOGUE_EX_(COleControl, QuickActivate)
	return pThis->m_xOleObject.GetExtent(DVASPECT_CONTENT, lpsizel);
}
