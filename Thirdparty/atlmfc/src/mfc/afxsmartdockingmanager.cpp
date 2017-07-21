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

#include "afxcontrolbarutil.h"
#include "afxsmartdockingmanager.h"
#include "afxpaneframewnd.h"
#include "afxdockablepane.h"
#include "afxglobals.h"
#include "afxtabbedpane.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSmartDockingManager::CSmartDockingManager() :
m_bStarted(FALSE), m_bCreated(FALSE), m_bShown(FALSE), m_bCentralGroupShown(FALSE), m_nHiliteSideNo(CSmartDockingStandaloneGuide::sdNONE), m_pwndOwner(NULL), m_pDockingWnd(NULL)
{
	ZeroMemory(m_arMarkers, sizeof(m_arMarkers));
}

CSmartDockingManager::~CSmartDockingManager()
{
	Destroy();
}

void CSmartDockingManager::Create(CWnd * pwndOwner, CRuntimeClass* prtMarker, CRuntimeClass* prtCentralGroup)
{
	ASSERT_VALID(pwndOwner);

	if (prtMarker == NULL)
	{
		prtMarker = RUNTIME_CLASS(CSmartDockingStandaloneGuide);
	}

	if (prtCentralGroup == NULL)
	{
		prtCentralGroup = RUNTIME_CLASS(CSmartDockingGroupGuidesManager);
	}

	ENSURE(prtMarker != NULL && prtMarker->IsDerivedFrom(RUNTIME_CLASS(CSmartDockingStandaloneGuide)));
	ENSURE(prtCentralGroup != NULL && prtCentralGroup->IsDerivedFrom(RUNTIME_CLASS(CSmartDockingGroupGuidesManager)));

	Destroy();

	m_pCentralGroup = (CSmartDockingGroupGuidesManager*)(prtCentralGroup->m_pfnCreateObject());
	ASSERT_VALID(m_pCentralGroup);

	CSmartDockingStandaloneGuide::SDMarkerPlace i;
	for (i = CSmartDockingStandaloneGuide::sdLEFT; i <= CSmartDockingStandaloneGuide::sdBOTTOM; ++reinterpret_cast<int&>(i))
	{
		m_arMarkers[i] = (CSmartDockingStandaloneGuide*)(prtMarker->m_pfnCreateObject());
		ASSERT_VALID(m_arMarkers [i]);

		m_arMarkers[i]->Create(i, pwndOwner);
	}

	m_pCentralGroup->Create(pwndOwner);

	for (i = CSmartDockingStandaloneGuide::sdCLEFT; i <= CSmartDockingStandaloneGuide::sdCMIDDLE; ++reinterpret_cast<int&>(i))
	{
		m_arMarkers[i] = m_pCentralGroup->GetGuide(i);
	}

	m_pwndOwner = pwndOwner;

	m_wndPlaceMarker.Create(m_pwndOwner);

	m_bCreated = TRUE;
}

void CSmartDockingManager::Destroy()
{
	if (!m_bCreated)
	{
		return;
	}

	Stop();

	CSmartDockingStandaloneGuide::SDMarkerPlace i;
	for (i = CSmartDockingStandaloneGuide::sdLEFT; i <= CSmartDockingStandaloneGuide::sdBOTTOM; ++reinterpret_cast<int&>(i))
	{
		delete m_arMarkers[i];
		m_arMarkers[i] = NULL;
	}

	m_pCentralGroup->Destroy();
	delete m_pCentralGroup;
	m_pCentralGroup = NULL;

	m_bCreated = FALSE;
}

void CSmartDockingManager::Start(CWnd* pDockingWnd)
{
	if (!m_bCreated)
	{
		return;
	}

	if (m_bStarted)
	{
		return;
	}

	ASSERT_VALID(pDockingWnd);

	m_pDockingWnd = pDockingWnd;

	m_wndPlaceMarker.SetDockingWnd(m_pDockingWnd);

	m_nHiliteSideNo = CSmartDockingStandaloneGuide::sdNONE;

	m_dwEnabledAlignment = CBRS_ALIGN_ANY;
	if (m_pDockingWnd != NULL)
	{
		CPaneFrameWnd* pMiniFrame = DYNAMIC_DOWNCAST(CPaneFrameWnd, m_pDockingWnd);
		if (pMiniFrame != NULL)
		{
			CDockablePane* pFisrtBar = DYNAMIC_DOWNCAST(CDockablePane, pMiniFrame->GetFirstVisiblePane());
			if (pFisrtBar != NULL)
			{
				m_dwEnabledAlignment = pFisrtBar->GetEnabledAlignment();
			}
		}
	}

	CSmartDockingStandaloneGuide::SDMarkerPlace i;
	for (i = CSmartDockingStandaloneGuide::sdLEFT; i <= CSmartDockingStandaloneGuide::sdBOTTOM; ++reinterpret_cast<int&>(i))
	{
		m_arMarkers[i]->AdjustPos(&m_rcOuter);
		if (((m_dwEnabledAlignment & CBRS_ALIGN_LEFT) != 0) && i == CSmartDockingStandaloneGuide::sdLEFT ||
			((m_dwEnabledAlignment & CBRS_ALIGN_RIGHT) != 0) && i == CSmartDockingStandaloneGuide::sdRIGHT ||
			((m_dwEnabledAlignment & CBRS_ALIGN_TOP) != 0) && i == CSmartDockingStandaloneGuide::sdTOP ||
			((m_dwEnabledAlignment & CBRS_ALIGN_BOTTOM) != 0) && i == CSmartDockingStandaloneGuide::sdBOTTOM)
		{
			m_arMarkers[i]->Show(TRUE);
		}
	}

	m_bShown = TRUE;
	m_bCentralGroupShown = FALSE;

	m_bStarted = TRUE;
}

void CSmartDockingManager::Stop()
{
	if (!m_bStarted)
	{
		return;
	}

	m_nHiliteSideNo = CSmartDockingStandaloneGuide::sdNONE;

	m_wndPlaceMarker.Hide();

	CSmartDockingStandaloneGuide::SDMarkerPlace i;
	for (i = CSmartDockingStandaloneGuide::sdLEFT; i <= CSmartDockingStandaloneGuide::sdBOTTOM; ++reinterpret_cast<int&>(i))
	{
		m_arMarkers[i]->Show(FALSE);
	}

	m_pCentralGroup->Show(FALSE);

	m_bStarted = FALSE;
}

void CSmartDockingManager::Show(BOOL bShow)
{
	if (m_bStarted && m_bShown != bShow)
	{
		m_bShown = bShow;

		if (m_bCentralGroupShown)
		{
			m_pCentralGroup->Show(bShow);
		}
		CSmartDockingStandaloneGuide::SDMarkerPlace i;
		for (i = CSmartDockingStandaloneGuide::sdLEFT; i <= CSmartDockingStandaloneGuide::sdBOTTOM; ++reinterpret_cast<int&>(i))
		{
			if (((m_dwEnabledAlignment & CBRS_ALIGN_LEFT) != 0) && i == CSmartDockingStandaloneGuide::sdLEFT ||
				((m_dwEnabledAlignment & CBRS_ALIGN_RIGHT) != 0) && i == CSmartDockingStandaloneGuide::sdRIGHT ||
				((m_dwEnabledAlignment & CBRS_ALIGN_TOP) != 0) && i == CSmartDockingStandaloneGuide::sdTOP ||
				((m_dwEnabledAlignment & CBRS_ALIGN_BOTTOM) != 0) && i == CSmartDockingStandaloneGuide::sdBOTTOM)
			{
				m_arMarkers[i]->Show(bShow);
			}
		}

		if (!bShow && !m_wndPlaceMarker.m_bTabbed)
		{
			m_wndPlaceMarker.Hide();
		}
	}
}

void CSmartDockingManager::OnMouseMove(CPoint point)
{
	if (m_bStarted)
	{
		m_nHiliteSideNo = CSmartDockingStandaloneGuide::sdNONE;

		BOOL bFound = FALSE;
		CSmartDockingStandaloneGuide::SDMarkerPlace i;
		CSmartDockingStandaloneGuide::SDMarkerPlace first = m_pCentralGroup->m_bMiddleIsOn ? CSmartDockingStandaloneGuide::sdCMIDDLE : CSmartDockingStandaloneGuide::sdCBOTTOM;

		for (i = first; i >= CSmartDockingStandaloneGuide::sdLEFT; --reinterpret_cast<int&>(i)) // from top z-position
		{
			if (!bFound &&(m_arMarkers[i] != NULL) &&(m_arMarkers[i]->IsPtIn(point)))
			{
				bFound = TRUE;
				m_arMarkers[i]->Highlight();
				m_nHiliteSideNo = i;
			}
			else
			{
				if (m_arMarkers[i] != NULL)
				{
					m_arMarkers[i]->Highlight(FALSE);
				}
			}
		}
	}
}

void CSmartDockingManager::OnPosChange()
{
	if (m_bStarted)
	{
		RECT rcOwner;

		m_pwndOwner->GetClientRect(&rcOwner);
		m_pwndOwner->ClientToScreen(&rcOwner);

		CSmartDockingStandaloneGuide::SDMarkerPlace i;
		for (i = CSmartDockingStandaloneGuide::sdLEFT; i <= CSmartDockingStandaloneGuide::sdBOTTOM; ++reinterpret_cast<int&>(i))
		{
			m_arMarkers[i]->AdjustPos(&rcOwner);
		}

		m_pCentralGroup->AdjustPos(&rcOwner, -1);
	}
}

void CSmartDockingManager::SetOuterRect(CRect rcOuter)
{
	m_rcOuter = rcOuter;

	m_pwndOwner->ClientToScreen(&m_rcOuter);

	if (m_bStarted)
	{
		CSmartDockingStandaloneGuide::SDMarkerPlace i;
		for (i = CSmartDockingStandaloneGuide::sdLEFT; i <= CSmartDockingStandaloneGuide::sdBOTTOM; ++reinterpret_cast<int&>(i))
		{
			m_arMarkers[i]->AdjustPos(&m_rcOuter);
			m_arMarkers[i]->Show(TRUE);
		}

		m_pCentralGroup->AdjustPos(&m_rcOuter, -1);
	}
}

void CSmartDockingManager::ShowPlaceAt(CRect rect)
{
	if (!m_bStarted || !m_bShown)
	{
		return;
	}

	if (m_nHiliteSideNo != CSmartDockingStandaloneGuide::sdNONE)
	{
		m_wndPlaceMarker.ShowAt(rect);
	}
}

void CSmartDockingManager::HidePlace()
{
	if (m_bStarted)
	{
		m_wndPlaceMarker.Hide();
	}
}

void CSmartDockingManager::ShowTabbedPlaceAt(CRect rect, int nTabXOffset, int nTabWidth, int nTabHeight)
{
	if (m_bStarted)
	{
		CRect rectTab;
		if (CTabbedPane::m_bTabsAlwaysTop)
		{
			rectTab.SetRect(CPoint(nTabXOffset, rect.top - nTabHeight), CPoint(nTabXOffset + nTabWidth, rect.top));

		}
		else
		{
			rectTab.SetRect(CPoint(nTabXOffset, rect.Height()), CPoint(nTabXOffset + nTabWidth, rect.Height() + nTabHeight));
		}

		m_wndPlaceMarker.ShowTabbedAt(rect, rectTab);
	}
}

void CSmartDockingManager::MoveCentralGroup(CRect rect, int nMiddleIsOn, DWORD dwEnabledAlignment)
{
	if (m_bStarted && m_pCentralGroup != NULL)
	{
		CRect rectGroup;
		m_pCentralGroup->GetWindowRect(rectGroup);
		if (rectGroup == rect)
		{
			return;
		}

		m_pCentralGroup->ShowGuide(CSmartDockingStandaloneGuide::sdCLEFT, (dwEnabledAlignment & CBRS_ALIGN_LEFT) != 0, TRUE);
		m_pCentralGroup->ShowGuide(CSmartDockingStandaloneGuide::sdCTOP, (dwEnabledAlignment & CBRS_ALIGN_TOP) != 0, TRUE);
		m_pCentralGroup->ShowGuide(CSmartDockingStandaloneGuide::sdCRIGHT, (dwEnabledAlignment & CBRS_ALIGN_RIGHT) != 0, TRUE);
		m_pCentralGroup->ShowGuide(CSmartDockingStandaloneGuide::sdCBOTTOM, (dwEnabledAlignment & CBRS_ALIGN_BOTTOM) != 0, TRUE);

		if (m_pCentralGroup->AdjustPos(rect, nMiddleIsOn))
		{
			m_nHiliteSideNo = CSmartDockingStandaloneGuide::sdNONE;
		}
	}
}

void CSmartDockingManager::ShowCentralGroup(BOOL bShow, DWORD dwEnabledAlignment)
{
	if (m_bStarted && m_pCentralGroup != NULL && m_bShown && m_bCentralGroupShown != bShow)
	{
		m_pCentralGroup->ShowGuide(CSmartDockingStandaloneGuide::sdCLEFT, (dwEnabledAlignment & CBRS_ALIGN_LEFT) != 0, TRUE);
		m_pCentralGroup->ShowGuide(CSmartDockingStandaloneGuide::sdCTOP, (dwEnabledAlignment & CBRS_ALIGN_TOP) != 0, TRUE);
		m_pCentralGroup->ShowGuide(CSmartDockingStandaloneGuide::sdCRIGHT, (dwEnabledAlignment & CBRS_ALIGN_RIGHT) != 0, TRUE);
		m_pCentralGroup->ShowGuide(CSmartDockingStandaloneGuide::sdCBOTTOM, (dwEnabledAlignment & CBRS_ALIGN_BOTTOM) != 0, TRUE);

		m_pCentralGroup->Show(bShow);
	}
	m_bCentralGroupShown = bShow;
}

void CSmartDockingManager::CauseCancelMode()
{
	if (!m_bStarted)
	{
		return;
	}

	ASSERT_VALID(m_pDockingWnd);

	m_pDockingWnd->SendMessage(WM_CANCELMODE);
}



