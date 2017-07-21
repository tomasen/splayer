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

#include "afxglobalutils.h"
#include "afxdocksite.h"
#include "afxdockablepane.h"
#include "afxpanedivider.h"

#include "afxautohidedocksite.h"
#include "afxautohidebar.h"
#include "afxautohidebutton.h"

#include "afxdockingmanager.h"
#include "afxpaneframewnd.h"
#include "afxmultipaneframewnd.h"
#include "afxtabbedpane.h"
#include "afxmdichildwndex.h"

#include "afxcaptionbar.h"

#include "afxregpath.h"
#include "afxsettingsstore.h"

#include "afxdockingpanesrow.h"

#include "afxrebar.h"
#include "afxpopupmenu.h"
#include "afxoutlookbar.h"

#include "afxmdiframewndex.h"

#pragma warning(disable : 4706)

#include "multimon.h"

#pragma warning(default : 4706)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define AFX_REG_SECTION_FMT _T("%sDockingManager-%d")
#define AFX_REG_ENTRY_DOCKING_PANE_AND_PANEDIVIDERS _T("DockingPaneAndPaneDividers")

static const CString strDockingManagerProfile = _T("DockingManagers");

const DWORD dwDockBarMap[4][2] =
{
	{ AFX_IDW_DOCKBAR_TOP,      CBRS_TOP    },
	{ AFX_IDW_DOCKBAR_BOTTOM,   CBRS_BOTTOM },
	{ AFX_IDW_DOCKBAR_LEFT,     CBRS_LEFT   },
	{ AFX_IDW_DOCKBAR_RIGHT,    CBRS_RIGHT  },
};

UINT CDockingManager::m_nTimeOutBeforeToolBarDock = 200;
UINT CDockingManager::m_nTimeOutBeforeDockingBarDock = 220;

AFX_DOCK_TYPE CDockingManager::m_dockModeGlobal = DT_STANDARD;
UINT CDockingManager::m_ahSlideModeGlobal = AFX_AHSM_MOVE;
int CDockingManager::m_nDockSensitivity = 15;
BOOL CDockingManager::m_bRestoringDockState = FALSE;
BOOL CDockingManager::m_bHideDockingBarsInContainerMode = TRUE;
BOOL CDockingManager::m_bDisableRecalcLayout = FALSE;
BOOL CDockingManager::m_bFullScreenMode = FALSE;

BOOL CDockingManager::m_bSavingState = FALSE;

CSmartDockingInfo CDockingManager::m_SDParams;
BOOL CDockingManager::m_bSDParamsModified = FALSE;

BOOL CDockingManager::m_bDockBarMenu = FALSE;
BOOL CDockingManager::m_bIgnoreEnabledAlignment = FALSE;

CRuntimeClass* CDockingManager::m_pAutoHideToolbarRTC = RUNTIME_CLASS(CMFCAutoHideBar);

// Construction/Destruction
CDockingManager::CDockingManager() : m_pParentWnd(NULL), m_pSDManager(NULL)
{
	m_dwEnabledDockBars = 0;
	m_dwEnabledSlideBars = 0;
	m_pActiveSlidingWnd = NULL;

	m_pLastTargetBar = NULL;
	m_pLastMultiMiniFrame = NULL;
	m_clkLastTime = 0;
	m_statusLast = CS_NOTHING;

	m_rectDockBarBounds.SetRectEmpty();
	m_rectClientAreaBounds.SetRectEmpty();
	m_rectOuterEdgeBounds.SetRectEmpty();

	m_rectInPlace.SetRectEmpty();

	m_bIsPrintPreviewMode = FALSE;
	m_bEnableAdjustLayout = TRUE;

	m_bLockUpdate = FALSE;
	m_bAdjustingBarLayout = FALSE;
	m_bRecalcLayout = FALSE;
	m_bSizeFrame = FALSE;

	m_bDisableSetDockState = FALSE;

	m_bDisableRestoreDockState = FALSE;

	m_bControlBarsMenuIsShown = FALSE;

	m_bControlBarsContextMenu = FALSE;
	m_bControlBarsContextMenuToolbarsOnly = FALSE;
	m_uiCustomizeCmd = 0;

	m_bHiddenForOLE = FALSE;
}

CDockingManager::~CDockingManager()
{
	if (m_pSDManager != NULL)
	{
		delete m_pSDManager;
		m_pSDManager = NULL;
	}
}

BOOL CDockingManager::Create(CFrameWnd* pParentWnd)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pParentWnd);
	m_pParentWnd = pParentWnd;

	return TRUE;
}

BOOL CDockingManager::EnableDocking(DWORD dwStyle)
{
	AFX_DOCKSITE_INFO info;
	if (dwStyle & CBRS_ALIGN_TOP &&(m_dwEnabledDockBars & CBRS_ALIGN_TOP) == 0)
	{
		info.m_dwBarAlignment = CBRS_ALIGN_TOP;
		info.pDockBarRTC = RUNTIME_CLASS(CDockSite);
		if (!AddDockSite(info))
		{
			return FALSE;
		}
		m_dwEnabledDockBars |= CBRS_ALIGN_TOP;
	}

	if (dwStyle & CBRS_ALIGN_BOTTOM &&(m_dwEnabledDockBars & CBRS_ALIGN_BOTTOM) == 0)
	{
		info.m_dwBarAlignment = CBRS_ALIGN_BOTTOM;
		info.pDockBarRTC = RUNTIME_CLASS(CDockSite);
		if (!AddDockSite(info))
		{
			return FALSE;
		}
		m_dwEnabledDockBars |= CBRS_ALIGN_BOTTOM;
	}

	if (dwStyle & CBRS_ALIGN_LEFT &&(m_dwEnabledDockBars & CBRS_ALIGN_LEFT) == 0)
	{
		info.m_dwBarAlignment = CBRS_ALIGN_LEFT;
		info.pDockBarRTC = RUNTIME_CLASS(CDockSite);
		if (!AddDockSite(info))
		{
			return FALSE;
		}
		m_dwEnabledDockBars |= CBRS_ALIGN_LEFT;
	}

	if (dwStyle & CBRS_ALIGN_RIGHT &&(m_dwEnabledDockBars & CBRS_ALIGN_RIGHT) == 0)
	{
		info.m_dwBarAlignment = CBRS_ALIGN_RIGHT;
		info.pDockBarRTC = RUNTIME_CLASS(CDockSite);
		if (!AddDockSite(info))
		{
			return FALSE;
		}
		m_dwEnabledDockBars |= CBRS_ALIGN_RIGHT;
	}
	AdjustDockingLayout();

	return TRUE;
}

BOOL CDockingManager::EnableAutoHidePanes(DWORD dwStyle)
{
	AFX_DOCKSITE_INFO info;

	if (dwStyle & CBRS_ALIGN_TOP &&(m_dwEnabledSlideBars & CBRS_ALIGN_TOP) == 0)
	{
		if ((m_dwEnabledDockBars & CBRS_ALIGN_TOP) == 0)
		{
			EnableDocking(CBRS_ALIGN_TOP);
		}
		info.m_dwBarAlignment = CBRS_ALIGN_TOP;
		info.pDockBarRTC = RUNTIME_CLASS(CAutoHideDockSite);
		if (!AddDockSite(info))
		{
			return FALSE;
		}
		m_dwEnabledSlideBars |= CBRS_ALIGN_TOP;
	}

	if (dwStyle & CBRS_ALIGN_BOTTOM  &&(m_dwEnabledSlideBars & CBRS_ALIGN_BOTTOM) == 0)
	{
		if ((m_dwEnabledDockBars & CBRS_ALIGN_BOTTOM) == 0)
		{
			EnableDocking(CBRS_ALIGN_BOTTOM);
		}

		info.m_dwBarAlignment = CBRS_ALIGN_BOTTOM;
		info.pDockBarRTC = RUNTIME_CLASS(CAutoHideDockSite);
		if (!AddDockSite(info))
		{
			return FALSE;
		}
		m_dwEnabledSlideBars |= CBRS_ALIGN_BOTTOM;
	}

	if (dwStyle & CBRS_ALIGN_LEFT &&(m_dwEnabledSlideBars & CBRS_ALIGN_LEFT) == 0)
	{
		if ((m_dwEnabledDockBars & CBRS_ALIGN_LEFT) == 0)
		{
			EnableDocking(CBRS_ALIGN_LEFT);
		}

		info.m_dwBarAlignment = CBRS_ALIGN_LEFT;
		info.pDockBarRTC = RUNTIME_CLASS(CAutoHideDockSite);
		if (!AddDockSite(info))
		{
			return FALSE;
		}
		m_dwEnabledSlideBars |= CBRS_ALIGN_LEFT;
	}

	if (dwStyle & CBRS_ALIGN_RIGHT &&(m_dwEnabledSlideBars & CBRS_ALIGN_RIGHT) == 0)
	{
		if ((m_dwEnabledDockBars & CBRS_ALIGN_RIGHT) == 0)
		{
			EnableDocking(CBRS_ALIGN_RIGHT);
		}

		info.m_dwBarAlignment = CBRS_ALIGN_RIGHT;
		info.pDockBarRTC = RUNTIME_CLASS(CAutoHideDockSite);
		if (!AddDockSite(info))
		{
			return FALSE;
		}
		m_dwEnabledSlideBars |= CBRS_ALIGN_RIGHT;
	}

	AdjustDockingLayout();
	return TRUE;
}

BOOL CDockingManager::AddDockSite(const AFX_DOCKSITE_INFO& info, CDockSite** ppDockBar)
{
	ASSERT_VALID(this);

	if (ppDockBar != NULL)
	{
		*ppDockBar = NULL;
	}

	CDockSite* pDockBar = (CDockSite*) info.pDockBarRTC->CreateObject();
	ASSERT_VALID(pDockBar);
	if (pDockBar->Create(info.m_dwBarAlignment, CRect(0, 0, 0, 0), m_pParentWnd, 0))
	{
		m_lstControlBars.AddTail(pDockBar);
	}
	else
	{
		TRACE0("Failed to create DockPane");
		delete pDockBar;
		return FALSE;
	}

	if (ppDockBar != NULL)
	{
		*ppDockBar = pDockBar;
	}

	return TRUE;
}

BOOL CDockingManager::InsertDockSite(const AFX_DOCKSITE_INFO& info, DWORD dwAlignToInsertAfter, CDockSite** ppDockBar)
{
	ASSERT_VALID(this);

	if (ppDockBar != NULL)
	{
		*ppDockBar = NULL;
	}

	CDockSite* pDockBar = (CDockSite*) info.pDockBarRTC->CreateObject();
	ASSERT_VALID(pDockBar);
	if (pDockBar->Create(info.m_dwBarAlignment, CRect(0, 0, 0, 0), m_pParentWnd, 0))
	{
		BOOL bInserted = FALSE;
		for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
		{
			CBasePane* pNextBar = (CBasePane*) m_lstControlBars.GetNext(pos);
			ASSERT_VALID(pNextBar);

			if (pNextBar->IsKindOf(RUNTIME_CLASS(CDockSite)) && pNextBar->GetCurrentAlignment() == (dwAlignToInsertAfter & CBRS_ALIGN_ANY) && pos != NULL)
			{
				m_lstControlBars.InsertBefore(pos, pDockBar);
				bInserted = TRUE;
				break;
			}
		}

		if (!bInserted)
		{
			m_lstControlBars.AddTail(pDockBar);
		}
	}
	else
	{
		TRACE0("Failed to create DockPane");
		delete pDockBar;
		return FALSE;
	}

	if (ppDockBar != NULL)
	{
		*ppDockBar = pDockBar;
	}

	return TRUE;
}

BOOL CDockingManager::AddPane(CBasePane* pWnd, BOOL bTail, BOOL bAutoHide, BOOL bInsertForOuterEdge)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pWnd);

	CObList& lstBars = bAutoHide ? m_lstAutoHideBars : m_lstControlBars;

	if (lstBars.Find(pWnd))
	{
		TRACE0("Control bar already added!!!\n");
		ASSERT(FALSE);
		return FALSE;
	}

	if (bTail)
	{
		lstBars.AddTail(pWnd);
	}
	else if (bInsertForOuterEdge)
	{
		// find first control bar with the same alignment and insert before it
		for (POSITION pos = lstBars.GetHeadPosition(); pos != NULL;)
		{
			POSITION posSave = pos;
			CBasePane* pNextBar = DYNAMIC_DOWNCAST(CBasePane, m_lstControlBars.GetNext(pos));
			ASSERT_VALID(pNextBar);

			if (pNextBar->DoesAllowDynInsertBefore())
			{
				lstBars.InsertBefore(posSave, pWnd);
				return TRUE;
			}
		}

		lstBars.AddTail(pWnd);
	}
	else
	{
		lstBars.AddHead(pWnd);
	}

	pWnd->m_pDockSite = m_pParentWnd;

	return TRUE;
}

BOOL CDockingManager::InsertPane(CBasePane* pControlBar, CBasePane* pTarget, BOOL bAfter)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pControlBar);

	if (m_lstControlBars.Find(pControlBar))
	{
		TRACE0("Control bar already added!!!\n");
		ASSERT(FALSE);
		return FALSE;
	}

	POSITION pos = m_lstControlBars.Find(pTarget);
	if (pos == NULL)
	{
		TRACE0("Control bar does not exist in the control container!!!\n");
		ASSERT(FALSE);
		return FALSE;
	}

	if (bAfter)
	{
		m_lstControlBars.InsertAfter(pos, pControlBar);
	}
	else
	{
		m_lstControlBars.InsertBefore(pos, pControlBar);
	}
	return TRUE;
}

void CDockingManager::RemovePaneFromDockManager(CBasePane* pWnd, BOOL bDestroy, BOOL bAdjustLayout, BOOL bAutoHide, CBasePane* pBarReplacement)
{
	CObList& lstBars = bAutoHide ? m_lstAutoHideBars : m_lstControlBars;

	POSITION pos = lstBars.Find(pWnd);
	if (pBarReplacement != NULL)
	{
		pos != NULL ? lstBars.InsertAfter(pos, pBarReplacement) : lstBars.AddTail(pBarReplacement);
	}
	if (pos != NULL)
	{
		lstBars.RemoveAt(pos);
		if (bDestroy)
		{
			pWnd->DestroyWindow();
		}

		if (bAdjustLayout)
		{
			AdjustDockingLayout();
		}
	}
}

BOOL CDockingManager::AddMiniFrame(CPaneFrameWnd* pWnd)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pWnd);

	POSITION pos = m_lstMiniFrames.Find(pWnd);

	if (pos != NULL)
	{
		return FALSE;
	}

	m_lstMiniFrames.AddTail(pWnd);
	return TRUE;
}

BOOL CDockingManager::RemoveMiniFrame(CPaneFrameWnd* pWnd)
{
	ASSERT_VALID(this);

	POSITION pos = m_lstMiniFrames.Find(pWnd);
	if (pos != NULL)
	{
		m_lstMiniFrames.RemoveAt(pos);
		return TRUE;
	}

	return FALSE;
}

CBasePane* CDockingManager::PaneFromPoint(CPoint point, int nSensitivity, bool bExactBar,
	CRuntimeClass* pRTCBarType, BOOL bCheckVisibility, const CBasePane* pBarToIgnore) const
{
	ASSERT_VALID(this);

	if (m_pSDManager != NULL)
	{
		CSmartDockingStandaloneGuide::SDMarkerPlace nHilitedSide = m_pSDManager->GetHighlightedGuideNo();
		if (nHilitedSide >= CSmartDockingStandaloneGuide::sdLEFT && nHilitedSide <= CSmartDockingStandaloneGuide::sdBOTTOM)
		{
			return NULL;
		}
	}

	CPaneFrameWnd* pMiniFrameToIgnore = NULL;
	if (pBarToIgnore != NULL)
	{
		pMiniFrameToIgnore = pBarToIgnore->GetParentMiniFrame(TRUE);
	}

	CPaneFrameWnd* pFrame = FrameFromPoint(point, pMiniFrameToIgnore, FALSE);
	if (pFrame != NULL)
	{
		CBasePane* pBar = pFrame->PaneFromPoint(point, nSensitivity, bCheckVisibility);
		if (pBar != NULL && pBar != pBarToIgnore && (pRTCBarType == NULL || pBar->IsKindOf(pRTCBarType)))
		{
			return pBar;
		}
	}

	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CBasePane* pNextBar = (CBasePane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pNextBar);

		if ((pRTCBarType == NULL || pNextBar->IsKindOf(pRTCBarType)))
		{
			if (bCheckVisibility && !pNextBar->IsPaneVisible() || pNextBar == pBarToIgnore)
			{
				continue;
			}

			CRect rectWnd;
			pNextBar->GetWindowRect(rectWnd);
			if (!bExactBar)
			{
				rectWnd.InflateRect(nSensitivity, nSensitivity);
			}

			if (rectWnd.PtInRect(point))
			{
				return pNextBar;
			}
		}
	}

	return NULL;
}

CBasePane* CDockingManager::PaneFromPoint(CPoint point, int nSensitivity, DWORD& dwAlignment, CRuntimeClass* pRTCBarType, const CBasePane* pBarToIgnore) const
{
	ASSERT_VALID(this);
	dwAlignment = 0;
	CBasePane* pBar = PaneFromPoint(point, nSensitivity, true, NULL, FALSE, pBarToIgnore);

	if (pBar != NULL)
	{
		if ((pRTCBarType == NULL || pBar->IsKindOf(pRTCBarType)))
		{
			if (!afxGlobalUtils.CheckAlignment(point, pBar, nSensitivity, this, FALSE, dwAlignment))
			{
				return NULL;
			}
		}
		else
		{
			pBar = NULL;
		}
	}

	return pBar;
}

AFX_CS_STATUS CDockingManager::DeterminePaneAndStatus(CPoint pt, int nSensitivity, DWORD dwEnabledAlignment, CBasePane** ppTargetBar, const CBasePane* pBarToIgnore, const CBasePane* pBarToDock)
{
	ASSERT_VALID(pBarToDock);

	// find the exact control bar first.
	*ppTargetBar = PaneFromPoint(pt, nSensitivity, true, RUNTIME_CLASS(CDockablePane), TRUE, pBarToIgnore);

	if (*ppTargetBar == NULL)
	{
		// find a miniframe from point and check it for a single bar
		CPaneFrameWnd* pMiniFrame = FrameFromPoint(pt, NULL, TRUE);
		if (pMiniFrame != NULL && pBarToDock->GetParentMiniFrame() != pMiniFrame)
		{
			// detect caption
			LRESULT uiHitTest = pMiniFrame->HitTest(pt, TRUE);
			if (uiHitTest == HTCAPTION && pMiniFrame->GetVisiblePaneCount() == 1)
			{
				*ppTargetBar = DYNAMIC_DOWNCAST(CBasePane, pMiniFrame->GetFirstVisiblePane());
				return CS_DELAY_DOCK_TO_TAB;
			}
		}
	}
	// check this bar for caption and tab area
	if (*ppTargetBar != NULL)
	{
		if ((*ppTargetBar)->GetParentMiniFrame() != NULL && (pBarToDock->GetPaneStyle() & CBRS_FLOAT_MULTI) &&
			((*ppTargetBar)->GetPaneStyle() & CBRS_FLOAT_MULTI) || (*ppTargetBar)->GetParentMiniFrame() == NULL)
		{

			CDockablePane* pDockingBar = DYNAMIC_DOWNCAST(CDockablePane, *ppTargetBar);

			if (!pDockingBar->IsFloating() && (pDockingBar->GetCurrentAlignment() & dwEnabledAlignment) == 0)
			{
				return CS_NOTHING;
			}

			if (pDockingBar != NULL)
			{
				return pDockingBar->GetDockingStatus(pt, nSensitivity);
			}
		}
	}

	*ppTargetBar = NULL;

	// check whether the mouse cursor is at the outer edge of the dock bar
	// or at the inner edge of the most inner control bar(on client area) and the
	// bar is allowed to be docked at this side
	BOOL bOuterEdge = FALSE;
	DWORD dwAlignment = 0;

	if (IsPointNearDockSite(pt, dwAlignment, bOuterEdge) && (dwAlignment & dwEnabledAlignment))
	{
		return CS_DELAY_DOCK;
	}

	return CS_NOTHING;
}

CBasePane* CDockingManager::FindPaneByID(UINT uBarID, BOOL bSearchMiniFrames)
{
	ASSERT_VALID(this);

	POSITION pos = NULL;

	for (pos = m_lstAutoHideBars.GetHeadPosition(); pos != NULL;)
	{
		CBasePane* pBar = (CBasePane*)m_lstAutoHideBars.GetNext(pos);

		ASSERT_VALID(pBar);
		if (pBar->IsKindOf(RUNTIME_CLASS(CPaneDivider)))
		{
			CPaneDivider* pSlider = DYNAMIC_DOWNCAST(CPaneDivider, pBar);
			// SLIDER CONTAINS ONLY ONE BAR IN AUTOHIDE MODE
			pBar = (CBasePane*) pSlider->GetFirstPane();
		}

		if (pBar == NULL)
		{
			continue;
		}

		UINT uID = pBar->GetDlgCtrlID();

		if (uID == uBarID)
		{
			return pBar;
		}
	}

	for (pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CBasePane* pBar = (CBasePane*)m_lstControlBars.GetNext(pos);

		UINT uID = pBar->GetDlgCtrlID();

		if (uID == uBarID)
		{
			return pBar;
		}

		if (pBar->IsKindOf(RUNTIME_CLASS(CBaseTabbedPane)))
		{
			CBaseTabbedPane* pTabbedBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, pBar);
			ASSERT_VALID(pTabbedBar);

			CBasePane* pControlBar = DYNAMIC_DOWNCAST(CBasePane, pTabbedBar->FindPaneByID(uBarID));
			if (pControlBar != NULL)
			{
				return pControlBar;
			}
		}
		else if (pBar->IsKindOf(RUNTIME_CLASS(CDockSite)))
		{
			CDockSite* pDockBar = (CDockSite*) pBar;
			ASSERT_VALID(pDockBar);

			CPane* pBarID = pDockBar->FindPaneByID(uBarID);
			if (pBarID != NULL)
			{
				return DYNAMIC_DOWNCAST(CBasePane, pBarID);
			}
		}
		else if (pBar->IsKindOf(RUNTIME_CLASS(CMFCReBar)))
		{
			CMFCReBar* pReBar = (CMFCReBar*) pBar;
			ASSERT_VALID(pReBar);

			CBasePane* pBarID = DYNAMIC_DOWNCAST(CBasePane, pReBar->GetDlgItem(uBarID));
			if (pBarID != NULL)
			{
				return pBarID;
			}
		}
	}

	if (bSearchMiniFrames)
	{
		for (pos = m_lstMiniFrames.GetHeadPosition(); pos != NULL;)
		{
			CPaneFrameWnd* pMiniFrame = DYNAMIC_DOWNCAST(CPaneFrameWnd, m_lstMiniFrames.GetNext(pos));
			if (pMiniFrame != NULL)
			{
				CBasePane* pWnd = DYNAMIC_DOWNCAST(CBasePane, pMiniFrame->GetPane());
				if (pWnd != NULL && pWnd->GetDlgCtrlID() == (int) uBarID)
				{
					return pWnd;
				}
			}
		}

		return CPaneFrameWnd::FindFloatingPaneByID(uBarID);
	}

	return NULL;
}

CDockSite* CDockingManager::FindDockSite(DWORD dwAlignment, BOOL bOuter)
{
	for (POSITION pos = bOuter ? m_lstControlBars.GetHeadPosition() : m_lstControlBars.GetTailPosition(); pos != NULL;)
	{
		CBasePane* pBar = (CBasePane*)(bOuter ? m_lstControlBars.GetNext(pos) : m_lstControlBars.GetPrev(pos));
		ASSERT_VALID(pBar);

		if (!pBar->IsKindOf(RUNTIME_CLASS(CDockSite)))
		{
			continue;
		}

		if (pBar->GetCurrentAlignment() == (dwAlignment & CBRS_ALIGN_ANY))
		{
			return DYNAMIC_DOWNCAST(CDockSite, pBar);
		}
	}

	return NULL;
}

void CDockingManager::FixupVirtualRects()
{
	ASSERT_VALID(this);
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CDockSite* pBar = DYNAMIC_DOWNCAST(CDockSite, m_lstControlBars.GetNext(pos));
		if (pBar != NULL)
		{
			pBar->FixupVirtualRects();
		}
	}
	AdjustDockingLayout();
}

BOOL CDockingManager::IsPointNearDockSite(CPoint point, DWORD& dwBarAlignment, BOOL& bOuterEdge) const
{
	ASSERT_VALID(this);
	dwBarAlignment = 0;
	// check the "outer" edge first - non resizable dock bars

	CRect rectBounds = m_rectOuterEdgeBounds;
	m_pParentWnd->ClientToScreen(rectBounds);

	bOuterEdge = TRUE;
	if (afxGlobalUtils.CheckAlignment(point, NULL, CDockingManager::m_nDockSensitivity, this, bOuterEdge, dwBarAlignment, m_dwEnabledDockBars, rectBounds))
	{
		return TRUE;
	}

	// check the innre edges - edges of the client area
	rectBounds = m_rectClientAreaBounds;
	m_pParentWnd->ClientToScreen(rectBounds);

	bOuterEdge = FALSE;
	return afxGlobalUtils.CheckAlignment(point, NULL, CDockingManager::m_nDockSensitivity, this, bOuterEdge, dwBarAlignment, m_dwEnabledDockBars, rectBounds);
}

BOOL CDockingManager::DockPaneLeftOf(CPane* pBarToDock, CPane* pTargetBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBarToDock);
	ASSERT_VALID(pTargetBar);

	if (pTargetBar->IsKindOf(RUNTIME_CLASS(CDockablePane)) && pBarToDock->IsKindOf(RUNTIME_CLASS(CDockablePane)))
	{
	}
	else if (pTargetBar->IsKindOf(RUNTIME_CLASS(CMFCToolBar)) && pBarToDock->IsKindOf(RUNTIME_CLASS(CMFCToolBar)))
	{
		CDockSite* pDockBar = FindDockSiteByPane(pTargetBar);
		if (pDockBar != NULL)
		{
			pBarToDock->UndockPane(TRUE);
			BOOL bResult = pDockBar->DockPaneLeftOf(pBarToDock, pTargetBar);
			return bResult;
		}
	}

	return FALSE;
}

CDockSite* CDockingManager::FindDockSiteByPane(CPane* pTargetBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pTargetBar);

	UINT uID = pTargetBar->GetDlgCtrlID();

	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CDockSite* pBar = DYNAMIC_DOWNCAST(CDockSite, m_lstControlBars.GetNext(pos));
		if (pBar != NULL)
		{
			if (pBar->FindPaneByID(uID) == pTargetBar)
			{
				return pBar;
			}
		}
	}
	return NULL;
}

// Should be used for toolbars or(resizable) control bars that can be docked
// on a resizable DockPane

void CDockingManager::DockPane(CBasePane* pBar, UINT nDockBarID, LPCRECT lpRect)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);

	if (m_lstControlBars.IsEmpty())
	{
		return;
	}

	// if the bar can be
	pBar->UndockPane(TRUE);

	if (!pBar->CanBeResized() && !pBar->CanFloat())
	{
		AddPane(pBar);
		return;
	}

	DWORD dwBarDockStyle = pBar->GetEnabledAlignment();

	if (pBar->IsResizable())
	{
		// resazable control bars are docked to frame window(their dock site)
		// directly
		if (nDockBarID == 0)
		{
			pBar->DockToFrameWindow(pBar->GetCurrentAlignment(), lpRect);
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				DWORD dwDockBarID = dwDockBarMap [i][0];
				DWORD dwDockAlign = dwDockBarMap [i][1];

				if ((nDockBarID == 0 || nDockBarID == dwDockBarID) && (dwDockAlign & dwBarDockStyle))
				{
					pBar->DockToFrameWindow(dwDockAlign, lpRect);
					break;
				}
			}
		}
	}
	else
	{
		for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
		{
			CBasePane* pNextBar = (CBasePane*)m_lstControlBars.GetNext(pos);
			ASSERT_VALID(pNextBar);

			if (pNextBar->IsKindOf(RUNTIME_CLASS(CDockSite)))
			{
				CDockSite* pNextDockBar = (CDockSite*) pNextBar;

				if ((nDockBarID == 0 || pNextDockBar->GetDockSiteID() == nDockBarID) && pBar->CanBeDocked(pNextDockBar) && pNextDockBar->CanAcceptPane(pBar))
				{
					if (pBar->DockPane(pNextDockBar, lpRect, DM_RECT))
					{
						pBar->InvalidateRect(NULL);
						break;
					}
				}
			}
		}
	}
}

CMFCAutoHideBar* CDockingManager::AutoHidePane(CDockablePane* pBar, CMFCAutoHideBar* pCurrAutoHideToolBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);

	DWORD dwAlignment = pBar->GetCurrentAlignment();

	// create autohide toolbar and button - it's always inner dockbar
	CAutoHideDockSite* pAutoHideDockBar = DYNAMIC_DOWNCAST(CAutoHideDockSite, FindDockSite(dwAlignment, FALSE));

	if (pAutoHideDockBar == NULL)
	{
		// no autohide allowed at this side
		return NULL;
	}

	CMFCAutoHideBar* pAutoHideToolBar = pCurrAutoHideToolBar;

	if (pAutoHideToolBar == NULL)
	{
		pAutoHideToolBar = DYNAMIC_DOWNCAST(CMFCAutoHideBar, m_pAutoHideToolbarRTC->CreateObject());

		ASSERT_VALID(pAutoHideToolBar);

		DWORD dwControlBarStyle = 0; // can't float...
		if (!pAutoHideToolBar->Create(NULL, WS_VISIBLE | WS_CHILD, CRect(0, 0, 0, 0), m_pParentWnd, 1, dwControlBarStyle))
		{
			TRACE0("Failde to create autohide toolbar");
			ASSERT(FALSE);
			delete pAutoHideToolBar;
			return NULL;
		}
	}
	pAutoHideToolBar->EnableDocking(CBRS_ALIGN_ANY);

	CPaneDivider* pDefaultSlider = pBar->GetDefaultPaneDivider();

	ASSERT_VALID(pDefaultSlider);

	CMFCAutoHideButton* pBtn = pAutoHideToolBar->AddAutoHideWindow(pBar, dwAlignment);

	ASSERT_VALID(pBtn);

	// NULL indicates that there was a new toolbar created here
	if (pCurrAutoHideToolBar == NULL)
	{
		if (!pAutoHideDockBar->IsPaneVisible())
		{
			pAutoHideDockBar->ShowWindow(SW_SHOW);
		}
		pAutoHideToolBar->DockPane(pAutoHideDockBar, NULL, DM_RECT);
	}

	// recalc. layout according to the newly added bar
	AdjustDockingLayout();

	// register the slider with the manager
	AddPane(pDefaultSlider, TRUE, TRUE);

	AlignAutoHidePane(pDefaultSlider);

	pBar->BringWindowToTop();
	pDefaultSlider->BringWindowToTop();

	return pAutoHideToolBar;
}

void CDockingManager::HideAutoHidePanes(CDockablePane* pBarToExclude, BOOL bImmediately)
{
	for (POSITION pos = m_lstAutoHideBars.GetHeadPosition(); pos != NULL;)
	{
		CPaneDivider* pSlider = (CPaneDivider*) m_lstAutoHideBars.GetNext(pos);
		ASSERT_VALID(pSlider);

		CDockablePane* pControlBar = (CDockablePane*)pSlider->GetFirstPane();
		ASSERT_VALID(pControlBar);

		if (pControlBar == pBarToExclude)
		{
			continue;
		}

		if (pControlBar->IsPaneVisible())
		{
			pControlBar->Slide(FALSE, !bImmediately);
		}
	}
}

void CDockingManager::AlignAutoHidePane(CPaneDivider* pDefaultSlider, BOOL bIsVisible)
{
	CRect rectSlider;

	pDefaultSlider->GetWindowRect(rectSlider);
	BOOL bHorz = pDefaultSlider->IsHorizontal();

	DWORD dwAlignment = pDefaultSlider->GetCurrentAlignment();
	BOOL bIsRTL = m_pParentWnd->GetExStyle() & WS_EX_LAYOUTRTL;

	if (bIsVisible)
	{
		CSize sizeRequered = pDefaultSlider->CalcFixedLayout(FALSE, bHorz);

		if (bHorz)
		{
			dwAlignment & CBRS_ALIGN_TOP ? rectSlider.bottom = rectSlider.top + sizeRequered.cy : rectSlider.top = rectSlider.bottom - sizeRequered.cy;
		}
		else
		{
			dwAlignment & CBRS_ALIGN_LEFT ? rectSlider.right = rectSlider.left + sizeRequered.cx : rectSlider.left = rectSlider.right - sizeRequered.cx;
		}

		// m_rectOuterEdgeBounds - the area surrounded by dock bars
		CRect rectBoundsScreen = m_rectOuterEdgeBounds;
		m_pParentWnd->ClientToScreen(rectBoundsScreen);
		AlignByRect(rectBoundsScreen, rectSlider, dwAlignment, bHorz, TRUE);

		HDWP hdwp = NULL;
		pDefaultSlider->RepositionPanes(rectSlider, hdwp);
	}
	else
	{
		// it can be nonvisible only when moved out of screen - adjust  only width/height
		CBasePane* pControlBar = (CBasePane*)pDefaultSlider->GetFirstPane();
		CRect rectControlBar;
		pControlBar->GetWindowRect(rectControlBar);

		pDefaultSlider->GetParent()->ScreenToClient(rectSlider);
		pDefaultSlider->GetParent()->ScreenToClient(rectControlBar);

		if (bHorz)
		{
			rectSlider.left = rectControlBar.left = m_rectOuterEdgeBounds.left;
			rectSlider.right = rectControlBar.right = m_rectOuterEdgeBounds.right;
		}
		else
		{
			rectSlider.top = rectControlBar.top = m_rectOuterEdgeBounds.top;
			rectSlider.bottom = rectControlBar.bottom = m_rectOuterEdgeBounds.bottom;
		}

		CPoint ptOffset(0, 0);

		// slider is not hidden completely - it is aligned by m_rectOuterEdgeBounds
		switch (dwAlignment)
		{
		case CBRS_ALIGN_LEFT:
			if (bIsRTL)
			{
				if (rectSlider.right != m_rectOuterEdgeBounds.right)
				{
					ptOffset.x = m_rectOuterEdgeBounds.right - rectSlider.right;
				}
			}
			else
			{
				if (rectSlider.left != m_rectOuterEdgeBounds.left)
				{
					ptOffset.x = m_rectOuterEdgeBounds.left - rectSlider.left;
				}
			}
			break;

		case CBRS_ALIGN_RIGHT:
			if (bIsRTL)
			{
				if (rectSlider.left != m_rectOuterEdgeBounds.left)
				{
					ptOffset.x = m_rectOuterEdgeBounds.left - rectSlider.left;
				}
			}
			else
			{
				if (rectSlider.right != m_rectOuterEdgeBounds.right)
				{
					ptOffset.x = m_rectOuterEdgeBounds.right - rectSlider.right;
				}
			}
			break;

		case CBRS_ALIGN_TOP:
			if (rectSlider.top != m_rectOuterEdgeBounds.top)
			{
				ptOffset.y = m_rectOuterEdgeBounds.top - rectSlider.top;
			}
			break;

		case CBRS_ALIGN_BOTTOM:
			if (rectSlider.bottom != m_rectOuterEdgeBounds.bottom)
			{
				ptOffset.y = m_rectOuterEdgeBounds.bottom - rectSlider.bottom;
			}
			break;
		}

		rectSlider.OffsetRect(ptOffset);
		rectControlBar.OffsetRect(ptOffset);

		pDefaultSlider->SetWindowPos(NULL, rectSlider.left, rectSlider.top, rectSlider.Width(), rectSlider.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
		pControlBar->SetWindowPos(NULL, rectControlBar.left, rectControlBar.top, rectControlBar.Width(), rectControlBar.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
		pControlBar->RecalcLayout();
	}
}

void CDockingManager::CalcExpectedDockedRect(CWnd* pWnd, CPoint ptMouse, CRect& rectResult, BOOL& bDrawTab, CDockablePane** ppTargetBar)
{
	ASSERT_VALID(this);

	rectResult.SetRectEmpty();

	if (GetKeyState(VK_CONTROL) < 0)
	{
		return;
	}

	BOOL bOuterEdge = FALSE;
	DWORD dwAlignment = 0;

	CPaneFrameWnd* pOtherMiniFrame = FrameFromPoint(ptMouse, DYNAMIC_DOWNCAST(CPaneFrameWnd, pWnd), TRUE);

	if (pOtherMiniFrame != NULL)
	{
		pOtherMiniFrame->CalcExpectedDockedRect(pWnd, ptMouse, rectResult, bDrawTab, ppTargetBar);
	}

	if (pOtherMiniFrame == NULL || rectResult.IsRectEmpty())
	{
		CBasePane* pThisControlBar =
			(m_pSDManager != NULL && m_pSDManager->IsStarted()) ? DYNAMIC_DOWNCAST(CBasePane, (DYNAMIC_DOWNCAST(CPaneFrameWnd, pWnd))->GetPane()) : NULL;

		CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, PaneFromPoint(ptMouse, CDockingManager::m_nDockSensitivity, true, NULL, TRUE, pThisControlBar));

		if (pBar != NULL && pBar->GetDefaultPaneDivider() != NULL)
		{
			if (pWnd->IsKindOf(RUNTIME_CLASS(CPaneFrameWnd)))
			{
				CPaneFrameWnd* pMiniFrame = DYNAMIC_DOWNCAST(CPaneFrameWnd, pWnd);
				ASSERT_VALID(pMiniFrame);
				if (!pBar->CanAcceptMiniFrame(pMiniFrame))
				{
					return;
				}
			}

			CPaneDivider* pDefaultSlider = pBar->GetDefaultPaneDivider();
			ASSERT_VALID(pDefaultSlider);

			pDefaultSlider->CalcExpectedDockedRect(pWnd, ptMouse, rectResult, bDrawTab, ppTargetBar);
		}
		else if (IsPointNearDockSite(ptMouse, dwAlignment, bOuterEdge))
		{
			*ppTargetBar = NULL;

			if (pWnd->IsKindOf(RUNTIME_CLASS(CPaneFrameWnd)))
			{
				CPaneFrameWnd* pMiniFrame = DYNAMIC_DOWNCAST(CPaneFrameWnd, pWnd);
				ASSERT_VALID(pMiniFrame);

				CPane* pBarFrame = DYNAMIC_DOWNCAST(CPane, pMiniFrame->GetPane());
				if (pBarFrame != NULL &&(pBarFrame->GetEnabledAlignment() & dwAlignment) == 0)
				{
					return;
				}
			}
			else if (pWnd->IsKindOf(RUNTIME_CLASS(CDockablePane)))
			{
				CDockablePane* pBarDock = DYNAMIC_DOWNCAST(CDockablePane, pWnd);
				if ((pBarDock->GetEnabledAlignment() & dwAlignment) == 0)
				{
					return;
				}
			}

			CRect rectWnd;
			pWnd->GetWindowRect(rectWnd);

			rectResult = bOuterEdge ? m_rectOuterEdgeBounds : m_rectClientAreaBounds;

			BOOL bIsRTL = m_pParentWnd->GetExStyle() & WS_EX_LAYOUTRTL;

			switch (dwAlignment)
			{
			case CBRS_ALIGN_LEFT:
				if (bIsRTL)
				{
					rectResult.left = rectResult.right - rectWnd.Width();
				}
				else
				{
					rectResult.right = rectResult.left + rectWnd.Width();
				}
				break;

			case CBRS_ALIGN_RIGHT:
				if (bIsRTL)
				{
					rectResult.right = rectResult.left + rectWnd.Width();
				}
				else
				{
					rectResult.left = rectResult.right - rectWnd.Width();
				}
				break;

			case CBRS_ALIGN_TOP:
				rectResult.bottom = rectResult.top + rectWnd.Height();
				break;

			case CBRS_ALIGN_BOTTOM:
				rectResult.top = rectResult.bottom - rectWnd.Height();
				break;
			}

			AdjustRectToClientArea(rectResult, dwAlignment);
			m_pParentWnd->ClientToScreen(rectResult);
		}
		else
		{
			*ppTargetBar = NULL;
		}
	}
}

BOOL CDockingManager::AdjustRectToClientArea(CRect& rectResult, DWORD dwAlignment)
{
	BOOL bAdjusted = FALSE;

	int nAllowedHeight = (int)(m_rectClientAreaBounds.Height() * afxGlobalData.m_nCoveredMainWndClientAreaPercent / 100);
	int nAllowedWidth = (int)(m_rectClientAreaBounds.Width() * afxGlobalData.m_nCoveredMainWndClientAreaPercent / 100);

	if (dwAlignment & CBRS_ORIENT_HORZ && rectResult.Height() >= nAllowedHeight)
	{
		if (dwAlignment & CBRS_ALIGN_TOP)
		{
			rectResult.bottom = rectResult.top + nAllowedHeight;
			bAdjusted = TRUE;
		}
		else if (dwAlignment & CBRS_ALIGN_BOTTOM)
		{
			rectResult.top = rectResult.bottom - nAllowedHeight;
			bAdjusted = TRUE;
		}
	}
	else if (dwAlignment & CBRS_ORIENT_VERT && rectResult.Width() >= nAllowedWidth)
	{
		BOOL bIsRTL = m_pParentWnd->GetExStyle() & WS_EX_LAYOUTRTL;

		if (dwAlignment & CBRS_ALIGN_LEFT)
		{
			if (bIsRTL)
			{
				rectResult.left = rectResult.right - nAllowedWidth;
			}
			else
			{
				rectResult.right = rectResult.left + nAllowedWidth;
			}
			bAdjusted = TRUE;
		}
		else if (dwAlignment & CBRS_ALIGN_RIGHT)
		{
			if (bIsRTL)
			{
				rectResult.right = rectResult.left + nAllowedWidth;
			}
			else
			{
				rectResult.left = rectResult.right - nAllowedWidth;
			}
			bAdjusted = TRUE;
		}
	}

	return bAdjusted;
}

BOOL CDockingManager::OnMoveMiniFrame(CWnd* pFrame)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pFrame);

	if (GetKeyState(VK_CONTROL) < 0)
	{
		return TRUE;
	}

	BOOL bResult = TRUE;

	CPaneFrameWnd* pMiniFrameEx = DYNAMIC_DOWNCAST(CPaneFrameWnd, pFrame);

	BOOL bSDockingIsOn = m_pSDManager != NULL && m_pSDManager->IsStarted();

	if (pMiniFrameEx != NULL)
	{
		CRect rect;
		pFrame->GetWindowRect(rect);
		int captionHeight = pMiniFrameEx->GetCaptionHeight();
		CRect rectDelta(captionHeight, captionHeight, captionHeight, captionHeight);
		afxGlobalUtils.AdjustRectToWorkArea(rect, &rectDelta);

		pMiniFrameEx->SetWindowPos(NULL, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	if (pMiniFrameEx != NULL)
	{
		CPoint ptMouse;
		GetCursorPos(&ptMouse);

		CPane* pBar = DYNAMIC_DOWNCAST(CPane, pMiniFrameEx->GetPane());

		// first, check if there is any other miniframe around and whether
		// it is possible to dock at this miniframe - but only if the
		// current bar is docking control bar has style cbrs_float_multi

		if ((pBar == NULL) || (!bSDockingIsOn &&(pBar->GetPaneStyle() & CBRS_FLOAT_MULTI) && pBar->IsKindOf(RUNTIME_CLASS(CDockablePane))))
		{
			CPaneFrameWnd* pOtherMiniFrame = FrameFromPoint(ptMouse, pMiniFrameEx, TRUE);

			// dock only bars from miniframes that have the same parent main frame,
			// otherwise it will create problems for dockmanagers
			if (pOtherMiniFrame != NULL && pOtherMiniFrame->GetParent() == pMiniFrameEx->GetParent())
			{
				CMultiPaneFrameWnd* pMultiMiniFrame = DYNAMIC_DOWNCAST(CMultiPaneFrameWnd, pOtherMiniFrame);

				if (pMultiMiniFrame != NULL && m_pLastMultiMiniFrame == NULL)
				{
					m_clkLastTime = clock();
					m_pLastMultiMiniFrame = pMultiMiniFrame;
				}

				if (pMultiMiniFrame != NULL && m_pLastMultiMiniFrame == pMultiMiniFrame && clock() - m_clkLastTime >(int) m_nTimeOutBeforeDockingBarDock)
				{
					bResult = pMultiMiniFrame->DockFrame(pMiniFrameEx, DM_MOUSE);
					m_clkLastTime = clock();
					m_pLastMultiMiniFrame = NULL;
					return bResult;
				}

				return TRUE;
			}
		}

		m_pLastMultiMiniFrame = NULL;

		if (pBar != NULL)
		{
			if ((pBar->GetEnabledAlignment() & CBRS_ALIGN_ANY) == 0)
			{
				// docking was not enabled for this control bar
				return TRUE;
			}

			// target control bar or dock bar
			CBasePane* pTargetBar = NULL;
			AFX_CS_STATUS status = pBar->IsChangeState(CDockingManager::m_nDockSensitivity, &pTargetBar);

			if (pBar == pTargetBar)
			{
				status = CS_NOTHING;
			}

			if ((pTargetBar != NULL || status == CS_DELAY_DOCK) && !bSDockingIsOn)
			{
				BOOL bDockBar =  pTargetBar != NULL ? pTargetBar->IsKindOf(RUNTIME_CLASS(CDockSite)) : FALSE;

				BOOL bDockingBar = pTargetBar != NULL ? pTargetBar->IsKindOf(RUNTIME_CLASS(CDockablePane)) : TRUE;

				if (bDockBar || bDockingBar)
				{
					UINT uTimeOut = bDockBar ? m_nTimeOutBeforeToolBarDock : m_nTimeOutBeforeDockingBarDock;
					if (m_pLastTargetBar != pTargetBar || status != m_statusLast)
					{
						m_clkLastTime = clock();
						m_pLastTargetBar = pTargetBar;
						m_statusLast = status;

						pMiniFrameEx->SetDockingTimer(uTimeOut);

					}

					if (clock() - m_clkLastTime <(int) uTimeOut)
					{
						return TRUE;
					}
				}
			}

			m_pLastTargetBar = NULL;
			m_clkLastTime = clock();
			m_statusLast = CS_NOTHING;
			pMiniFrameEx->KillDockingTimer();

			if (status == CS_DOCK_IMMEDIATELY && pTargetBar != NULL)
			{
				// in the case docking was delayed we need always turn off predock state
				//(usually it happens only for resizable control bars)
				pMiniFrameEx->SetPreDockState(PDS_NOTHING);
				if (pBar->DockByMouse(pTargetBar))
				{
					return FALSE;
				}
			}

			if (status == CS_DELAY_DOCK && !bSDockingIsOn) // status returned by resizable control bar
			{
				bResult = pMiniFrameEx->SetPreDockState(PDS_DOCK_REGULAR, pTargetBar);
			}
			else if (status == CS_DELAY_DOCK_TO_TAB && !bSDockingIsOn)
			{
				bResult = pMiniFrameEx->SetPreDockState(PDS_DOCK_TO_TAB, pTargetBar);
				AdjustDockingLayout();
			}
			else
			{
				bResult = pMiniFrameEx->SetPreDockState(PDS_NOTHING, pTargetBar);
			}
		}
	}
	return bResult;
}

// used for autohide to prevent wrong z-order when auto hide window is sliding
// in(collapsed)

void CDockingManager::BringBarsToTop(DWORD dwAlignment, BOOL bExcludeDockedBars)
{
	dwAlignment &= CBRS_ALIGN_ANY;

	for (POSITION pos = m_lstControlBars.GetTailPosition(); pos != NULL;)
	{
		CBasePane* pBar = (CBasePane*) m_lstControlBars.GetPrev(pos);
		ASSERT_VALID(pBar);

		if (bExcludeDockedBars && (pBar->IsKindOf(RUNTIME_CLASS(CPane)) || pBar->IsKindOf(RUNTIME_CLASS(CPaneDivider))))
		{
			continue;
		}

		// starting from first dockbar do not exclude anything(so, th stattus bar
		// and so on will be on top)
		bExcludeDockedBars = FALSE;

		DWORD dwCurrAlignment = pBar->GetCurrentAlignment();

		if (dwCurrAlignment == dwAlignment || dwAlignment == 0)
		{
			pBar->BringWindowToTop();
		}
	}
}

void CDockingManager::SetAutohideZOrder(CDockablePane* pAHDockingBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pAHDockingBar);

	DWORD dwAlignment = pAHDockingBar->GetCurrentAlignment();
	CPaneDivider* pAHSlider = pAHDockingBar->GetDefaultPaneDivider();

	for (POSITION pos = m_lstControlBars.GetTailPosition(); pos != NULL;)
	{
		CBasePane* pBar = (CBasePane*) m_lstControlBars.GetPrev(pos);
		ASSERT_VALID(pBar);

		if (pBar == pAHSlider || pBar == pAHDockingBar)
		{
			continue;
		}

		if (pBar->IsKindOf(RUNTIME_CLASS(CPane)) && (pBar->GetCurrentAlignment() == dwAlignment))
		{
			pBar->SetWindowPos(pAHDockingBar, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}
		else if (pBar->IsKindOf(RUNTIME_CLASS(CPaneDivider)))
		{
			pBar->SetWindowPos(&CWnd::wndBottom, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}
	}

	pAHDockingBar->SetWindowPos(pAHSlider, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

}

void CDockingManager::RecalcLayout(BOOL /*bNotify*/)
{
	if (m_bDisableRecalcLayout)
	{
		return;
	}

	if (m_bRecalcLayout || m_bSizeFrame)
	{
		return;
	}

	if (!m_bEnableAdjustLayout)
	{
		return;
	}

	m_bRecalcLayout = TRUE;

	if (!IsOLEContainerMode())
	{
		POSITION pos = NULL;

		for (pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
		{
			CBasePane* pNextControlBar = (CBasePane*) m_lstControlBars.GetNext(pos);
			ASSERT_VALID(pNextControlBar);
			pNextControlBar->AdjustLayout();
		}

		for (pos = m_lstMiniFrames.GetHeadPosition(); pos != NULL;)
		{
			CPaneFrameWnd* pNextMiniFrame = (CPaneFrameWnd*) m_lstMiniFrames.GetNext(pos);
			ASSERT_VALID(pNextMiniFrame);
			pNextMiniFrame->AdjustLayout();
		}
	}
	else
	{
		for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
		{
			CBasePane* pNextControlBar = (CBasePane*) m_lstControlBars.GetNext(pos);
			ASSERT_VALID(pNextControlBar);
			if (pNextControlBar->IsPaneVisible())
			{
				pNextControlBar->AdjustLayout();
			}
		}
	}

	AdjustDockingLayout();
	m_bRecalcLayout = FALSE;
}

void CDockingManager::AdjustPaneFrames()
{
	ASSERT_VALID(this);

	UINT uiSWPFlags = SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED;

	POSITION pos = NULL;

	for (pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CBasePane* pNextControlBar = (CBasePane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pNextControlBar);

		pNextControlBar->SetWindowPos(NULL, -1, -1, -1, -1, uiSWPFlags);
	}

	for (pos = m_lstMiniFrames.GetHeadPosition(); pos != NULL;)
	{
		CPaneFrameWnd* pNextMiniFrame = (CPaneFrameWnd*) m_lstMiniFrames.GetNext(pos);
		ASSERT_VALID(pNextMiniFrame);

		pNextMiniFrame->SetWindowPos(NULL, -1, -1, -1, -1, uiSWPFlags);
		pNextMiniFrame->AdjustPaneFrames();
	}
}

void CDockingManager::AdjustDockingLayout(HDWP hdwp)
{
	ASSERT_VALID(this);

	if (m_bDisableRecalcLayout)
	{
		return;
	}

	if (m_bAdjustingBarLayout)
	{
		return;
	}

	if (m_pParentWnd == NULL)
	{
		return;
	}

	m_pParentWnd->GetClientRect(m_rectClientAreaBounds);

	if (!m_rectInPlace.IsRectEmpty())
	{
		m_rectClientAreaBounds = m_rectInPlace;
	}

	if (!m_bEnableAdjustLayout)
	{
		return;
	}

	if (m_lstControlBars.IsEmpty())
	{
		return;
	}

	if (AFXGetTopLevelFrame(m_pParentWnd) != NULL && AFXGetTopLevelFrame(m_pParentWnd)->IsIconic())

	{
		return;
	}

	m_bAdjustingBarLayout = TRUE;

	CRect rectSaveOuterEdgeBounds = m_rectOuterEdgeBounds;

	BOOL bDeferWindowPosHere = FALSE;
	if (hdwp == NULL && !m_bIsPrintPreviewMode)
	{
		hdwp = BeginDeferWindowPos((int) m_lstControlBars.GetCount());
		bDeferWindowPosHere = TRUE;
	}

	CRect rectCurrBounds = m_rectDockBarBounds;

	m_pParentWnd->GetClientRect(rectCurrBounds);
	if (!m_rectInPlace.IsRectEmpty())
	{
		rectCurrBounds = m_rectInPlace;
	}
	m_pParentWnd->ClientToScreen(rectCurrBounds);

	CRect rectControlBar;
	POSITION posLastDockBar = NULL;

	// find position of the last dock bar in the list(actually, it will be position
	// of the next control bar right after the last dock bar in the list)

	for (posLastDockBar = m_lstControlBars.GetTailPosition(); posLastDockBar != NULL;)
	{
		CBasePane* pDockBar = (CBasePane*) m_lstControlBars.GetPrev(posLastDockBar);
		if (posLastDockBar == NULL)
		{
			break;
		}

		if (pDockBar->IsKindOf(RUNTIME_CLASS(CDockSite)) || pDockBar->IsKindOf(RUNTIME_CLASS(CAutoHideDockSite)))
		{
			m_lstControlBars.GetNext(posLastDockBar);
			if (posLastDockBar != NULL)
			{
				m_lstControlBars.GetNext(posLastDockBar);
			}
			break;
		}
	}

	POSITION posBar = NULL;
	POSITION pos = NULL;

	for (pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		posBar = pos;

		CBasePane* pNextControlBar = (CBasePane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pNextControlBar);

		if (!pNextControlBar->IsPaneVisible() && (pNextControlBar->IsKindOf(RUNTIME_CLASS(CPane)) || pNextControlBar->IsKindOf(RUNTIME_CLASS(CPaneDivider)) ||
			pNextControlBar->IsKindOf(RUNTIME_CLASS(CDockSite)) && (m_bIsPrintPreviewMode || IsOLEContainerMode() || m_bHiddenForOLE)))
		{
			continue;
		}

		// let's see whether this control bar has enough space to be displayed,
		// has to be aligned differntly and so on.

		pNextControlBar->GetWindowRect(rectControlBar);
		CRect rectSave = rectControlBar;

		DWORD dwAlignment = pNextControlBar->GetCurrentAlignment();
		BOOL  bHorizontal = pNextControlBar->IsHorizontal();
		BOOL  bResizable  = pNextControlBar->IsResizable();

		if (pNextControlBar->IsKindOf(RUNTIME_CLASS(CDockablePane)))
		{
			CDockablePane* pDockingControlBar = DYNAMIC_DOWNCAST(CDockablePane, pNextControlBar);
			if (pDockingControlBar->GetDefaultPaneDivider() != NULL)
			{
				// resizable control bars with sliders will be aligned by slider itself!!!
				continue;
			}
		}

		CSize sizeRequered = pNextControlBar->CalcFixedLayout(FALSE, bHorizontal);

		if (bHorizontal)
		{
			dwAlignment & CBRS_ALIGN_TOP ? rectControlBar.bottom = rectControlBar.top + sizeRequered.cy : rectControlBar.top = rectControlBar.bottom - sizeRequered.cy;
		}
		else
		{
			dwAlignment & CBRS_ALIGN_LEFT ? rectControlBar.right = rectControlBar.left + sizeRequered.cx : rectControlBar.left = rectControlBar.right - sizeRequered.cx;
		}

		AlignByRect(rectCurrBounds, rectControlBar, dwAlignment, bHorizontal, bResizable);

		CRect rectControlBarScreen = rectControlBar;

		ASSERT_VALID(pNextControlBar->GetParent());
		if (pNextControlBar->IsKindOf(RUNTIME_CLASS(CDockSite)))
		{
			pNextControlBar->ScreenToClient(rectControlBar);
			if (pNextControlBar->IsHorizontal() && rectControlBar.Width() > 0 || !pNextControlBar->IsHorizontal() && rectControlBar.Height() > 0)
			{
				((CDockSite*) pNextControlBar)->RepositionPanes(rectControlBar);
			}

			rectControlBar = rectControlBarScreen;
		}

		if (pNextControlBar->IsKindOf(RUNTIME_CLASS(CPaneDivider)))
		{
			// the slider will change its position, as well as position of
			// its resizable control bars(container)
			((CPaneDivider*) pNextControlBar)->RepositionPanes(rectControlBar, hdwp);
		}
		else
		{
			pNextControlBar->GetParent()->ScreenToClient(rectControlBar);
			hdwp = pNextControlBar->SetWindowPos(NULL, rectControlBar.left, rectControlBar.top, rectControlBar.Width(), rectControlBar.Height(), SWP_NOZORDER | SWP_NOACTIVATE, hdwp);
		}

		if (dwAlignment & CBRS_ALIGN_TOP)
		{
			rectCurrBounds.top += rectControlBarScreen.Height();
		}
		else if (dwAlignment & CBRS_ALIGN_BOTTOM)
		{
			rectCurrBounds.bottom -= rectControlBarScreen.Height();
		}
		else if (dwAlignment & CBRS_ALIGN_LEFT)
		{
			rectCurrBounds.left += rectControlBarScreen.Width();
		}
		else
		{
			rectCurrBounds.right -= rectControlBarScreen.Width();
		}

		if (posLastDockBar == pos)
		{
			m_rectOuterEdgeBounds = rectCurrBounds;
		}

	}

	m_rectClientAreaBounds = rectCurrBounds;

	if (m_rectOuterEdgeBounds.IsRectEmpty() || IsOLEContainerMode())
	{
		m_rectOuterEdgeBounds = rectCurrBounds;
	}

	m_pParentWnd->ScreenToClient(m_rectClientAreaBounds);
	m_pParentWnd->ScreenToClient(m_rectOuterEdgeBounds);

	if (m_rectOuterEdgeBounds != rectSaveOuterEdgeBounds)
	{
		HideAutoHidePanes(NULL, TRUE);
	}

	// special processing for autohide dock bars
	for (pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CBasePane* pBar = (CBasePane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pBar);
		if (pBar->IsKindOf(RUNTIME_CLASS(CAutoHideDockSite)))
		{
			CAutoHideDockSite* pSlidingBar = (CAutoHideDockSite*) pBar;
			pSlidingBar->SetOffsetLeft(0);
			pSlidingBar->SetOffsetRight(0);
			CalcPaneOffset((CAutoHideDockSite*) pBar);
		}
	}

	if (bDeferWindowPosHere)
	{
		EndDeferWindowPos(hdwp);
	}

	if (m_pParentWnd->m_pNotifyHook != NULL)
	{
		m_pParentWnd->RecalcLayout();
	}

	m_bAdjustingBarLayout = FALSE;
}

void CDockingManager::CalcPaneOffset(CAutoHideDockSite* pBar)
{
	ASSERT_VALID(pBar);
	DWORD dwBarAlignOrg = pBar->GetCurrentAlignment();
	CRect rectBarOrg;
	pBar->GetWindowRect(rectBarOrg);
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CBasePane* pBarNext = (CBasePane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pBarNext);
		if (pBarNext->IsKindOf(RUNTIME_CLASS(CAutoHideDockSite)))
		{
			CAutoHideDockSite* pNextSlidingDockBar = DYNAMIC_DOWNCAST(CAutoHideDockSite, pBarNext);

			if (pNextSlidingDockBar == pBar || pNextSlidingDockBar->IsHorizontal() && pBar->IsHorizontal() || !pNextSlidingDockBar->IsHorizontal() && !pBar->IsHorizontal())
			{
				continue;
			}

			CRect rectBarNext;
			pNextSlidingDockBar->GetWindowRect(rectBarNext);
			if (rectBarNext.IsRectEmpty())
			{
				continue;
			}

			DWORD dwBarAlignNext = pNextSlidingDockBar->GetCurrentAlignment();

			if (dwBarAlignOrg & CBRS_ALIGN_LEFT && dwBarAlignNext & CBRS_ALIGN_TOP)
			{
				if (rectBarOrg.top == rectBarNext.bottom)
				{
					pNextSlidingDockBar->SetOffsetLeft(rectBarOrg.Width());
				}

				if (rectBarOrg.right == rectBarNext.left)
				{
					pBar->SetOffsetLeft(rectBarNext.Height());
				}
			}
			else if (dwBarAlignOrg & CBRS_ALIGN_TOP && dwBarAlignNext & CBRS_ALIGN_RIGHT)
			{
				if (rectBarOrg.right == rectBarNext.left)
				{
					pNextSlidingDockBar->SetOffsetLeft(rectBarOrg.Height());
				}

				if (rectBarOrg.bottom == rectBarNext.top)
				{
					pBar->SetOffsetRight(rectBarNext.Width());
				}
			}
			else if (dwBarAlignOrg & CBRS_ALIGN_RIGHT && dwBarAlignNext & CBRS_ALIGN_BOTTOM)
			{
				if (rectBarOrg.bottom == rectBarNext.top)
				{
					pNextSlidingDockBar->SetOffsetRight(rectBarOrg.Width());
				}

				if (rectBarOrg.left == rectBarNext.right)
				{
					pBar->SetOffsetRight(rectBarOrg.Width());
				}
			}
			else if (dwBarAlignOrg & CBRS_ALIGN_LEFT && dwBarAlignNext & CBRS_ALIGN_BOTTOM)
			{
				if (rectBarOrg.bottom == rectBarNext.top)
				{
					pNextSlidingDockBar->SetOffsetLeft(rectBarOrg.Width());
				}

				if (rectBarOrg.right == rectBarNext.left)
				{
					pBar->SetOffsetRight(rectBarNext.Height());
				}
			}
		}
	}
}

void CDockingManager::AlignByRect(const CRect& rectToAlignBy, CRect& rectResult, DWORD dwAlignment, BOOL bHorizontal, BOOL bResizable)
{
	ASSERT_VALID(this);

	int nCurrWidth = rectResult.Width();
	int nCurrHeight = rectResult.Height();

	DWORD dwCurrAlignment = dwAlignment & CBRS_ALIGN_ANY;
	switch (dwCurrAlignment)
	{
	case CBRS_ALIGN_LEFT:
		rectResult.TopLeft() = rectToAlignBy.TopLeft();
		rectResult.bottom = rectResult.top + rectToAlignBy.Height();
		rectResult.right = rectResult.left + nCurrWidth;
		break;

	case CBRS_ALIGN_TOP:
		rectResult.TopLeft() = rectToAlignBy.TopLeft();
		rectResult.right = rectResult.left + rectToAlignBy.Width();
		rectResult.bottom = rectResult.top + nCurrHeight;
		break;

	case CBRS_ALIGN_RIGHT:
		rectResult.BottomRight() = rectToAlignBy.BottomRight();
		rectResult.top = rectResult.bottom - rectToAlignBy.Height();
		rectResult.left = rectResult.right - nCurrWidth;
		break;
	case CBRS_ALIGN_BOTTOM:
		rectResult.BottomRight() = rectToAlignBy.BottomRight();
		rectResult.left = rectResult.right - rectToAlignBy.Width();
		rectResult.top = rectResult.bottom - nCurrHeight;
		break;
	}

	if (bHorizontal)
	{
		int nDelta = rectResult.Width() - rectToAlignBy.Width();
		if (nDelta != 0)
		{
			rectResult.right += nDelta;
		}

		nDelta = rectResult.Height() - rectToAlignBy.Height();
		if (nDelta > 0 && bResizable)
		{
			if (dwCurrAlignment & CBRS_ALIGN_TOP)
			{
				rectResult.bottom -= nDelta;
			}
			else if (dwCurrAlignment & CBRS_ALIGN_BOTTOM)
			{
				rectResult.top += nDelta;
			}
		}
	}
	else
	{
		int nDelta = rectResult.Height() - rectToAlignBy.Height();
		if (nDelta != 0)
		{
			rectResult.bottom += nDelta;
		}

		nDelta = rectResult.Width() - rectToAlignBy.Width();
		if (rectResult.Width() > rectToAlignBy.Width() && bResizable)
		{
			if (dwCurrAlignment & CBRS_ALIGN_LEFT)
			{
				rectResult.right -= nDelta;
			}
			else if (dwCurrAlignment & CBRS_ALIGN_RIGHT)
			{
				rectResult.left += nDelta;
			}
		}
	}
}

BOOL CDockingManager::SaveState(LPCTSTR lpszProfileName, UINT uiID)
{
	ASSERT_VALID(this);

	m_bSavingState = TRUE;

	CString strProfileName = ::AFXGetRegPath(strDockingManagerProfile, lpszProfileName);

	BOOL bResult = FALSE;

	CString strSection;
	strSection.Format(AFX_REG_SECTION_FMT, (LPCTSTR)strProfileName, uiID);

	POSITION pos = NULL;

	for (pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CBasePane* pBarNext = (CBasePane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pBarNext);

		if (pBarNext->IsKindOf(RUNTIME_CLASS(CDockablePane)) || pBarNext->IsKindOf(RUNTIME_CLASS(CPane)) && !pBarNext->IsKindOf(RUNTIME_CLASS(CMFCToolBar)))
		{
			pBarNext->SaveState(lpszProfileName);
		}
	}

	for (pos = m_lstAutoHideBars.GetHeadPosition(); pos != NULL;)
	{
		CPaneDivider* pSlider = DYNAMIC_DOWNCAST(CPaneDivider, m_lstControlBars.GetNext(pos));

		if (pSlider != NULL && pSlider->IsDefault())
		{
			CObList lstBars;
			CDockablePane* pNextBar = (CDockablePane*) pSlider->GetFirstPane();
			if (pNextBar != NULL)
			{
				pNextBar->SaveState(lpszProfileName);
			}
		}
	}

	for (pos = m_lstMiniFrames.GetHeadPosition(); pos != NULL;)
	{
		CPaneFrameWnd* pWnd = DYNAMIC_DOWNCAST(CPaneFrameWnd, m_lstMiniFrames.GetNext(pos));
		ASSERT_VALID(pWnd);
		pWnd->SaveState(lpszProfileName);
	}

	try
	{
		CMemFile file;
		{
			CArchive ar(&file, CArchive::store);

			Serialize(ar);
			ar.Flush();
		}

		UINT uiDataSize = (UINT) file.GetLength();
		LPBYTE lpbData = file.Detach();

		if (lpbData != NULL)
		{
			CSettingsStoreSP regSP;
			CSettingsStore& reg = regSP.Create(FALSE, FALSE);

			if (reg.CreateKey(strSection))
			{
				bResult = reg.Write(AFX_REG_ENTRY_DOCKING_PANE_AND_PANEDIVIDERS, lpbData, uiDataSize);
			}

			free(lpbData);
		}
	}
	catch(CMemoryException* pEx)
	{
		pEx->Delete();
		TRACE(_T("Memory exception in CDockingManager::SaveState()!\n"));
	}

	m_bSavingState = FALSE;
	return bResult;
}

BOOL CDockingManager::LoadState(LPCTSTR lpszProfileName, UINT uiID)
{
	ASSERT_VALID(this);

	CString strProfileName = ::AFXGetRegPath(strDockingManagerProfile, lpszProfileName);

	BOOL bResult = FALSE;

	CString strSection;
	strSection.Format(AFX_REG_SECTION_FMT, (LPCTSTR)strProfileName, uiID);

	POSITION pos = NULL;

	for (pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CBasePane* pBarNext = (CBasePane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pBarNext);

		if (pBarNext->IsKindOf(RUNTIME_CLASS(CDockablePane)) || pBarNext->IsKindOf(RUNTIME_CLASS(CPane)) && !pBarNext->IsKindOf(RUNTIME_CLASS(CMFCToolBar)))
		{
			pBarNext->LoadState(lpszProfileName);
		}
	}

	for (pos = m_lstAutoHideBars.GetHeadPosition(); pos != NULL;)
	{
		CBasePane* pBar = (CBasePane*)m_lstAutoHideBars.GetNext(pos);

		ASSERT_VALID(pBar);
		if (pBar->IsKindOf(RUNTIME_CLASS(CPaneDivider)))
		{
			CPaneDivider* pSlider = DYNAMIC_DOWNCAST(CPaneDivider, pBar);
			// SLIDER CONTAINS ONLY ONE BAR IN AUTOHIDE MODE
			pBar = (CBasePane*) pSlider->GetFirstPane();
			if (pBar != NULL && pBar->IsKindOf(RUNTIME_CLASS(CDockablePane)))
			{
				pBar->LoadState(lpszProfileName);
			}
		}
	}

	for (pos = m_lstMiniFrames.GetHeadPosition(); pos != NULL;)
	{
		CPaneFrameWnd* pWnd = DYNAMIC_DOWNCAST(CPaneFrameWnd, m_lstMiniFrames.GetNext(pos));

		ASSERT_VALID(pWnd);

		pWnd->LoadState(lpszProfileName);
	}

	LPBYTE lpbData = NULL;
	UINT uiDataSize;

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (!reg.Open(strSection))
	{
		return FALSE;
	}

	if (!reg.Read(AFX_REG_ENTRY_DOCKING_PANE_AND_PANEDIVIDERS, &lpbData, &uiDataSize))
	{
		return FALSE;
	}

	try
	{
		CMemFile file(lpbData, uiDataSize);
		CArchive ar(&file, CArchive::load);

		Serialize(ar);
		bResult = TRUE;
		m_bDisableSetDockState = FALSE;
	}
	catch(CMemoryException* pEx)
	{
		pEx->Delete();
		TRACE(_T("Memory exception in CDockingManager::LoadState!\n"));
		m_bDisableSetDockState = TRUE;
	}
	catch(CArchiveException* pEx)
	{
		pEx->Delete();
		TRACE(_T("CArchiveException exception in CDockingManager::LoadState()!\n"));

		// destroy loaded sliders(docking control bars are loaded by ID's and have
		// been already created by application

		for (pos = m_lstLoadedBars.GetHeadPosition(); pos != NULL;)
		{
			CBasePane* pNextBar = (CBasePane*) m_lstLoadedBars.GetNext(pos);
			if (pNextBar->IsKindOf(RUNTIME_CLASS(CPaneDivider)))
			{
				pNextBar->DestroyWindow();
			}
			else
			{
				pNextBar->SetRestoredFromRegistry(FALSE);
			}
		}

		m_lstLoadedBars.RemoveAll();
		m_bDisableSetDockState = TRUE;
	}
	catch(...)
	{
		// destroy loaded sliders(docking control bars are loaded by ID's and have
		// been already created by application

		for (pos = m_lstLoadedBars.GetHeadPosition(); pos != NULL;)
		{
			CBasePane* pNextBar = (CBasePane*) m_lstLoadedBars.GetNext(pos);
			if (pNextBar->IsKindOf(RUNTIME_CLASS(CPaneDivider)))
			{
				pNextBar->DestroyWindow();
			}
			else
			{
				pNextBar->SetRestoredFromRegistry(FALSE);
			}
		}

		m_lstLoadedBars.RemoveAll();
		m_bDisableSetDockState = TRUE;
	}

	if (lpbData != NULL)
	{
		delete [] lpbData;
	}

	return bResult;

}

void CDockingManager::Serialize(CArchive& ar)
{
	// calculate or load the number of docking control bars and sliders
	int nCBCount = 0;
	int nNonFloatingBarCount = 0;

	if (ar.IsStoring())
	{
		POSITION pos = NULL;

		// get rid on non-valid empty tabbed bars
		for (pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
		{
			CBaseTabbedPane* pNextBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, m_lstControlBars.GetAt(pos));

			if (pNextBar != NULL && pNextBar->GetTabsNum() == 0 && pNextBar->CanFloat())
			{
				m_lstControlBars.GetPrev(pos);
				pNextBar->UndockPane(TRUE);
				if (pos == NULL)
				{
					pos = m_lstControlBars.GetHeadPosition();
				}
			}
			if (pos != NULL)
			{
				m_lstControlBars.GetNext(pos);
			}
		}

		for (pos = m_lstMiniFrames.GetHeadPosition(); pos != NULL;)
		{
			CPaneFrameWnd* pNextMiniFrame = DYNAMIC_DOWNCAST(CPaneFrameWnd, m_lstMiniFrames.GetNext(pos));

			if (pNextMiniFrame != NULL)
			{
				pNextMiniFrame->RemoveNonValidPanes();
			}
		}

		for (pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
		{
			CBasePane* pNextBar = (CBasePane*) m_lstControlBars.GetNext(pos);

			if (pNextBar->IsKindOf(RUNTIME_CLASS(CDockablePane)) && pNextBar->CanFloat() || pNextBar->IsKindOf(RUNTIME_CLASS(CPaneDivider)) &&
				((CPaneDivider*) pNextBar)->DoesContainFloatingPane())
			{
				nCBCount++;
			}
			else
			{
				// static bar that may contain detachable/floating tabs
				if (pNextBar->IsKindOf(RUNTIME_CLASS(CBaseTabbedPane)))
				{
					nNonFloatingBarCount++;
				}
			}
		}

		ar << nNonFloatingBarCount;

		for (pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
		{
			CBasePane* pNextBar = (CBasePane*) m_lstControlBars.GetNext(pos);
			if (!pNextBar->CanFloat() && pNextBar->IsKindOf(RUNTIME_CLASS(CBaseTabbedPane)))
			{
				CBaseTabbedPane* pTabbedBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, pNextBar);
				if (pTabbedBar != NULL)
				{
					ASSERT_VALID(pTabbedBar->GetUnderlyingWindow());
					ar << pTabbedBar->GetDlgCtrlID();
					pTabbedBar->GetUnderlyingWindow()->Serialize(ar);
				}
			}
		}

		ar << nCBCount;

		// START from the tail, so the sliders and embedded containers
		// will be stored bedore their control bars

		for (pos = m_lstControlBars.GetTailPosition(); pos != NULL;)
		{
			CBasePane* pNextBar = (CBasePane*) m_lstControlBars.GetPrev(pos);
			ASSERT_VALID(pNextBar);

			if (pNextBar->IsKindOf(RUNTIME_CLASS(CDockablePane)) && pNextBar->CanFloat())
			{
				int nBarID = pNextBar->GetDlgCtrlID();
				// write docking control bar tag and ID

				if (nBarID != -1)
				{
					ar << TRUE;
					ar << nBarID;
				}
				else
				{
					// this is tab control bar - write its tabbed bar ids
					CBaseTabbedPane* pTabBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, pNextBar);
					ASSERT_VALID(pTabBar);

					// use first child bar as identifier of the tab control bar
					CWnd* pWnd  = pTabBar->FindBarByTabNumber(0);

					// if pWnd is NULL - write nothing! because we do not allow empty tabbed
					// bars
					if (pWnd != NULL)
					{
						int nTabbedBarID = pWnd->GetDlgCtrlID();
						ASSERT(nTabbedBarID != -1);

						ar << TRUE;
						ar << nBarID;
						ar << nTabbedBarID;
					}
				}
				continue;
			}
			else if (pNextBar->IsKindOf(RUNTIME_CLASS(CPaneDivider)) &&((CPaneDivider*) pNextBar)->DoesContainFloatingPane())
			{
				// write slider tag and serialize the slider
				ar << FALSE;

				CPaneDivider* pSlider = DYNAMIC_DOWNCAST(CPaneDivider, pNextBar);
				ASSERT_VALID(pSlider);

				pSlider->Serialize(ar);
			}
		}

		int nCountMiniFrames = 0;

		for (pos = m_lstMiniFrames.GetHeadPosition(); pos != NULL;)
		{
			CPaneFrameWnd* pWnd = DYNAMIC_DOWNCAST(CPaneFrameWnd, m_lstMiniFrames.GetNext(pos));
			ASSERT_VALID(pWnd);

			if (pWnd->GetPaneCount() > 0)
			{
				nCountMiniFrames++;
			}
		}

		ar << nCountMiniFrames;

		for (pos = m_lstMiniFrames.GetHeadPosition(); pos != NULL;)
		{
			CPaneFrameWnd* pWnd = DYNAMIC_DOWNCAST(CPaneFrameWnd, m_lstMiniFrames.GetNext(pos));
			ASSERT_VALID(pWnd);
			if (pWnd->GetPaneCount() > 0)
			{
				ar << pWnd;
			}
		}

		// serialize autohide bars

		ar <<(int) m_lstAutoHideBars.GetCount();

		for (pos = m_lstAutoHideBars.GetHeadPosition(); pos != NULL;)
		{
			AFX_AUTOHIDE_DOCKSITE_SAVE_INFO ahSaveInfo;

			CPaneDivider* pSlider = DYNAMIC_DOWNCAST(CPaneDivider, m_lstAutoHideBars.GetNext(pos));

			if (pSlider == NULL)
			{
				ASSERT(FALSE);
				continue;
			}

			ahSaveInfo.m_pSavedBar = DYNAMIC_DOWNCAST(CDockablePane, pSlider->GetFirstPane());

			if (ahSaveInfo.m_pSavedBar != NULL)
			{
				ahSaveInfo.Serialize(ar);
			}
		}

		// serialize MDI Tabbed Bars
		ar <<(int) m_lstHiddenMDITabbedBars.GetCount();

		for (pos = m_lstHiddenMDITabbedBars.GetHeadPosition(); pos != NULL;)
		{
			HWND hwnd = m_lstHiddenMDITabbedBars.GetNext(pos);
			CDockablePane* pNextBar = DYNAMIC_DOWNCAST(CDockablePane, CWnd::FromHandlePermanent(hwnd));
			if (pNextBar != NULL)
			{
				ar <<(int) pNextBar->GetDlgCtrlID();
			}
			else
			{
				ar <<(int) -1;
			}
		}
	}
	else
	{
		m_lstLoadedBars.RemoveAll();
		m_lstNonFloatingBars.RemoveAll();
		m_lstLoadedAutoHideBarIDs.RemoveAll();
		m_lstLoadedMiniFrames.RemoveAll();

		UINT nBarID = (UINT) -1;

		CList<UINT, UINT&> lstNotFoundBars;

		ar >> nNonFloatingBarCount;

		int i = 0;

		for (i = 0; i < nNonFloatingBarCount; i++)
		{
			ar >> nBarID;
			CBaseTabbedPane* pBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, FindPaneByID(nBarID, TRUE));
			if (pBar != NULL)
			{
				pBar->GetUnderlyingWindow()->Serialize(ar);
				m_lstNonFloatingBars.AddTail(pBar);
			}
		}

		ar >> nCBCount;

		BOOL bIsDockingControlBar = FALSE;

		CPaneDivider* pCurrentDefaultSlider = NULL;

		// the list was stored from the tail(to store sliders first)
		// therefore we need to add head
		for (i = 0; i < nCBCount; i++)
		{
			ar >> bIsDockingControlBar;

			if (bIsDockingControlBar)
			{
				ar >> nBarID;

				CDockablePane* pBar = NULL;
				if (nBarID != -1)
				{
					pBar = DYNAMIC_DOWNCAST(CDockablePane, FindPaneByID(nBarID, TRUE));
				}
				else
				{
					// tab docking bar - load first child bar
					ar >> nBarID;

					if (pCurrentDefaultSlider != NULL)
					{
						pBar = pCurrentDefaultSlider->FindTabbedPane(nBarID);
					}
				}

				if (pBar != NULL)
				{
					ASSERT_VALID(pBar);

					if (pBar->IsAutoHideMode())
					{
						pBar->SetAutoHideMode(FALSE, CBRS_ALIGN_ANY);
					}

					pBar->SetRestoredDefaultPaneDivider(pCurrentDefaultSlider->m_hWnd);
					pBar->SetPaneAlignment(pCurrentDefaultSlider->GetCurrentAlignment());
					m_lstLoadedBars.AddHead(pBar);
				}
				else
				{
					lstNotFoundBars.AddTail(nBarID);
				}
			}
			else
			{
				pCurrentDefaultSlider = DYNAMIC_DOWNCAST(CPaneDivider, CPaneDivider::m_pSliderRTC->CreateObject());
				ASSERT_VALID(pCurrentDefaultSlider);

				pCurrentDefaultSlider->Init(TRUE, m_pParentWnd);
				pCurrentDefaultSlider->Serialize(ar);

				m_lstLoadedBars.AddHead(pCurrentDefaultSlider);

				POSITION posSave = NULL;
				for (POSITION posNotFound = lstNotFoundBars.GetHeadPosition(); posNotFound != NULL;)
				{
					posSave = posNotFound;
					UINT nNextBarID = lstNotFoundBars.GetNext(posNotFound);
					CDockablePane* pNextBar = pCurrentDefaultSlider->FindTabbedPane(nNextBarID);
					if (pNextBar != NULL)
					{
						ASSERT_VALID(pNextBar);

						if (pNextBar->IsAutoHideMode())
						{
							pNextBar->SetAutoHideMode(FALSE, CBRS_ALIGN_ANY);
						}

						pNextBar->SetRestoredDefaultPaneDivider(pCurrentDefaultSlider->m_hWnd);
						pNextBar->SetPaneAlignment(pCurrentDefaultSlider->GetCurrentAlignment());
						m_lstLoadedBars.AddHead(pNextBar);
						lstNotFoundBars.RemoveAt(posSave);
					}
				}
			}
		}

		int nMiniFrameCount = 0;

		ar >> nMiniFrameCount;

		for (i = 0; i < nMiniFrameCount; i++)
		{
			CPaneFrameWnd* pWnd = NULL;
			CPaneFrameWnd::m_pParentWndForSerialize = m_pParentWnd;
			ar >> pWnd;
			m_lstLoadedMiniFrames.AddTail(pWnd);
		}

		int nAHBarCount = 0;
		ar >> nAHBarCount;

		for (i = 0; i < nAHBarCount; i++)
		{
			AFX_AUTOHIDE_DOCKSITE_SAVE_INFO info;
			info.Serialize(ar);
			m_lstLoadedAutoHideBarIDs.AddTail(info);
		}

		int nMdiTabbedBarsCount = 0;
		ar >> nMdiTabbedBarsCount;
		
		for (i = 0; i < nMdiTabbedBarsCount; i++)
		{
			int nMDITabbedBarID = -1;
			ar >> nMDITabbedBarID;
			if (nMDITabbedBarID != -1)
			{
				CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, FindPaneByID(nMDITabbedBarID, TRUE));
				if (pBar != NULL)
				{
					if (pBar->IsFloating())
					{
						CPaneFrameWnd* pMiniFrame = pBar->GetParentMiniFrame();
						if (pMiniFrame != NULL)
						{
							pMiniFrame->RemovePane(pBar);
						}

						pBar->SetParent(m_pParentWnd);
					}
					else if (pBar->IsAutoHideMode())
					{
						pBar->SetAutoHideMode(FALSE, CBRS_ALIGN_ANY);
					}
					else if (pBar->IsTabbed())
					{
						CBaseTabbedPane* pTabBar = pBar->GetParentTabbedPane();
						ASSERT_VALID(pTabBar);
						pBar->SetParent(m_pParentWnd);
						pTabBar->RemovePane(pBar);
					}
					else
					{
						pBar->UndockPane();
					}
					pBar->ShowWindow(FALSE);
					AddHiddenMDITabbedBar(pBar);
					pBar->SetMDITabbed(TRUE);
				}
			}
		}
	}
}

void CDockingManager::SetDockState()
{
	if (m_bDisableSetDockState || m_bDisableRestoreDockState)
	{
		return;
	}

	if (m_lstLoadedBars.IsEmpty() && m_lstLoadedMiniFrames.IsEmpty() && m_lstNonFloatingBars.IsEmpty() && m_lstLoadedAutoHideBarIDs.IsEmpty()  && m_lstControlBars.IsEmpty())
	{
		return;
	}

	m_bRestoringDockState = TRUE;

	m_bDisableRecalcLayout = TRUE;

	POSITION pos = NULL;
	CObList lstAutoHideBars;

	// set all autohide bars to the regular mode
	for (pos = m_lstAutoHideBars.GetHeadPosition(); pos != NULL;)
	{
		CPaneDivider* pSlider = DYNAMIC_DOWNCAST(CPaneDivider, m_lstAutoHideBars.GetNext(pos));
		if (pSlider != NULL)
		{
			CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, pSlider->GetFirstPane());
			if (pBar != NULL && pBar->GetAutoHideToolBar() != NULL && pBar->GetAutoHideToolBar()->m_bFirstInGroup)
			{
				lstAutoHideBars.AddTail(pBar);
			}
		}
	}

	for (pos = lstAutoHideBars.GetHeadPosition(); pos != NULL;)
	{
		CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, lstAutoHideBars.GetNext(pos));
		pBar->SetAutoHideMode(FALSE, CBRS_ALIGN_ANY);
	}

	for (pos = m_lstLoadedBars.GetHeadPosition(); pos != NULL;)
	{
		CDockablePane* pNextBar = DYNAMIC_DOWNCAST(CDockablePane, m_lstLoadedBars.GetNext(pos));
		if (pNextBar != NULL)
		{
			pNextBar->RestoreDefaultPaneDivider();
		}
	}

	// set up miniframes - the original list may be modified by SetDockState
	CObList lstMiniFrames;
	lstMiniFrames.AddTail(&m_lstLoadedMiniFrames);

	for (pos = lstMiniFrames.GetHeadPosition(); pos != NULL;)
	{
		CPaneFrameWnd* pWnd = DYNAMIC_DOWNCAST(CPaneFrameWnd, lstMiniFrames.GetNext(pos));
		ASSERT_VALID(pWnd);

		pWnd->SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		pWnd->SetDockState(this);
	}

	CObList lstAllBars;

	for (pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CBasePane* pBarNext = (CBasePane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pBarNext);

		if (pBarNext->IsKindOf(RUNTIME_CLASS(CDockSite)))
		{
			CDockSite* pDockBar = (CDockSite*)DYNAMIC_DOWNCAST(CDockSite, pBarNext);

			ASSERT_VALID(pDockBar);
			lstAllBars.AddTail((CObList*)&pDockBar->GetPaneList());
		}
		else if (pBarNext->IsKindOf(RUNTIME_CLASS(CPane)))
		{
			if (pBarNext->CanFloat())
			{
				lstAllBars.AddTail(pBarNext);
			}
			else if (pBarNext->IsRestoredFromRegistry())
			{
				// set the size of non-floating bar right now
				CRect rect = ((CPane*) pBarNext)->m_rectSavedDockedRect;
				pBarNext->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE);
				pBarNext->ShowPane(pBarNext->GetRecentVisibleState(), TRUE, FALSE);
			}
		}
	}

	// take toolbars from all originally created miniframes
	for (pos = m_lstMiniFrames.GetHeadPosition(); pos != NULL;)
	{
		CPaneFrameWnd* pWnd = DYNAMIC_DOWNCAST(CPaneFrameWnd, m_lstMiniFrames.GetNext(pos));

		if (pWnd != NULL)
		{
			CMFCBaseToolBar* pToolbar = DYNAMIC_DOWNCAST(CMFCBaseToolBar, pWnd->GetPane());
			if (pToolbar != NULL)
			{
				lstAllBars.AddTail(pToolbar);
			}
		}
	}

	// we must float all bars first
	for (pos = lstAllBars.GetHeadPosition(); pos != NULL;)
	{
		CPane* pNextBar = (CPane*) lstAllBars.GetNext(pos);
		ASSERT_VALID(pNextBar);

		if (pNextBar->IsRestoredFromRegistry())
		{
			if (pNextBar->IsKindOf(RUNTIME_CLASS(CBaseTabbedPane)))
			{
				// contained bars will be redocked later
				// the bar itself should be destroyed
			}
			else
			{
				pNextBar->FloatPane(pNextBar->m_recentDockInfo.m_rectRecentFloatingRect, DM_SHOW, false);
			}
		}
	}

	// redock at recent rows regular control bars(toolbars and so on)
	for (pos = lstAllBars.GetHeadPosition(); pos != NULL;)
	{
		CPane* pNextBar = (CPane*) lstAllBars.GetNext(pos);
		ASSERT_VALID(pNextBar);

		if (pNextBar->IsRestoredFromRegistry() && !pNextBar->IsKindOf(RUNTIME_CLASS(CDockablePane)))
		{
			pNextBar->SetDockState(this);
			pNextBar->UpdateVirtualRect();
		}
	}

	// add docking control bars and sliders(remove from miniframe first)
	for (pos = m_lstLoadedBars.GetHeadPosition(); pos != NULL;)
	{
		CBasePane* pNextBar = (CBasePane*) m_lstLoadedBars.GetNext(pos);
		ASSERT_VALID(this);

		if (pNextBar->IsTabbed())
		{
			CMFCBaseTabCtrl* pTabWnd = (CMFCBaseTabCtrl*) pNextBar->GetParent();
			CBaseTabbedPane* pTabBar = (CBaseTabbedPane*) pTabWnd->GetParent();
			ASSERT_VALID(pTabBar);
			pNextBar->SetParent(m_pParentWnd);
			pTabBar->RemovePane(pNextBar);
			if (pNextBar->IsKindOf(RUNTIME_CLASS(CDockablePane)))
			{
				((CDockablePane*) pNextBar)->EnableGripper(TRUE);
			}

			pNextBar->ShowWindow(SW_SHOW);
		}

		CPaneFrameWnd* pMiniFrame = pNextBar->GetParentMiniFrame();
		if (pMiniFrame != NULL)
		{
			pMiniFrame->RemovePane(pNextBar);
		}

		pNextBar->SetParent(m_pParentWnd);

		if (pNextBar->IsKindOf(RUNTIME_CLASS(CDockablePane)))
		{
			CDockablePane* pDockingBar = DYNAMIC_DOWNCAST(CDockablePane, pNextBar);
			CRect rect = pDockingBar->m_rectSavedDockedRect;

			pDockingBar->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE);

			BOOL bShow = pDockingBar->GetRecentVisibleState();
			pDockingBar->ShowWindow(bShow ? SW_SHOWNORMAL : SW_HIDE);

			CPaneDivider* pDefaultSlider = pDockingBar->GetDefaultPaneDivider();

			if (pDefaultSlider != NULL)
			{
				pDockingBar->SetPaneAlignment(pDefaultSlider->GetCurrentAlignment());
				pDefaultSlider->OnShowPane(pDockingBar, bShow);
			}

			if (pDockingBar->IsKindOf(RUNTIME_CLASS(CBaseTabbedPane)))
			{
				BOOL bLeftBar = FALSE;

				if (pDefaultSlider != NULL)
				{
					CPaneContainer* pContainer = pDefaultSlider->FindPaneContainer(pDockingBar, bLeftBar);

					ASSERT(pContainer != NULL);

					CList<UINT, UINT>* pListBarIDs = pContainer->GetAssociatedSiblingPaneIDs(pDockingBar);

					if (pListBarIDs != NULL)
					{
						for (POSITION posList = pListBarIDs->GetHeadPosition(); posList != NULL;)
						{
							UINT nIDNext = pListBarIDs->GetNext(posList);
							CDockablePane* pBarToAttach = DYNAMIC_DOWNCAST(CDockablePane, FindPaneByID(nIDNext, TRUE));

							if (pBarToAttach != NULL)
							{
								if (pBarToAttach->IsTabbed())
								{
									CMFCBaseTabCtrl* pTabWnd = (CMFCBaseTabCtrl*) pBarToAttach->GetParent();
									CBaseTabbedPane* pTabBar = (CBaseTabbedPane*)pTabWnd->GetParent();
									ASSERT_VALID(pTabBar);

									pBarToAttach->SetParent(m_pParentWnd);
									pTabBar->RemovePane(pBarToAttach);
								}
								else if (pBarToAttach->IsAutoHideMode())
								{
									pBarToAttach->SetAutoHideMode(FALSE, CBRS_ALIGN_ANY);
								}

								((CDockablePane*) pBarToAttach)->AttachToTabWnd(pDockingBar, DM_UNKNOWN, FALSE);
							}
						}
					}
					if (((CBaseTabbedPane*)pDockingBar)->GetTabsNum() == 0)
					{
						continue;
					}

					((CBaseTabbedPane*)pDockingBar)->ApplyRestoredTabInfo();
					pDockingBar->RecalcLayout();
				}
			}
		}
	}

	m_lstControlBars.AddTail(&m_lstLoadedBars);

	m_bDisableRecalcLayout = FALSE;
	AdjustDockingLayout();

	for (pos = m_lstLoadedAutoHideBarIDs.GetHeadPosition(); pos != NULL;)
	{
		AFX_AUTOHIDE_DOCKSITE_SAVE_INFO& info = m_lstLoadedAutoHideBarIDs.GetNext(pos);
		CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, FindPaneByID(info.m_nBarID, TRUE));
		if (pBar != NULL)
		{
			if (pBar->IsFloating())
			{
				pBar->DockToFrameWindow(info.m_dwBarAlignment);
			}
			else if (pBar->IsAutoHideMode())
			{
				pBar->SetAutoHideMode(FALSE, CBRS_ALIGN_ANY);
			}
			else if (pBar->IsTabbed())
			{
				CMFCBaseTabCtrl* pTabWnd = (CMFCBaseTabCtrl*) pBar->GetParent();
				CBaseTabbedPane* pTabBar = (CBaseTabbedPane*)pTabWnd->GetParent();
				ASSERT_VALID(pTabBar);

				pBar->SetParent(m_pParentWnd);
				pTabBar->RemovePane(pBar);
				pBar->EnableGripper(TRUE);
				pBar->DockToFrameWindow(info.m_dwBarAlignment);
			}
			else
			{
				pBar->DockToFrameWindow(info.m_dwBarAlignment);
			}

			pBar->SetWindowPos(NULL, info.m_rectBar.left, info.m_rectBar.top, info.m_rectBar.Width(), info.m_rectBar.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
			CMFCAutoHideBar* pNewToolbar = pBar->SetAutoHideMode(TRUE, info.m_dwBarAlignment, NULL, FALSE);

			if (pNewToolbar != NULL)
			{
				pNewToolbar->m_bActiveInGroup = info.m_bActiveInGroup;
				pNewToolbar->m_bFirstInGroup = info.m_bFirstInGroup;
				pNewToolbar->m_bLastInGroup = info.m_bLastInGroup;

				info.m_pSavedBar = pBar;
				info.m_pSavedBar->m_pAutoHideBar->SetRecentVisibleState(info.m_bIsVisible); // used by dockbar when the frame is loaded
			}
		}
	}

	for (pos = m_lstLoadedAutoHideBarIDs.GetHeadPosition(); pos != NULL;)
	{
		AFX_AUTOHIDE_DOCKSITE_SAVE_INFO& info = m_lstLoadedAutoHideBarIDs.GetNext(pos);

		if (info.m_pSavedBar != NULL && info.m_pSavedBar->IsHideInAutoHideMode())
		{
			info.m_pSavedBar->ShowPane(info.m_bIsVisible, FALSE, FALSE);
		}
	}

	for (pos = m_lstNonFloatingBars.GetHeadPosition(); pos != NULL;)
	{
		CBaseTabbedPane* pBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, m_lstNonFloatingBars.GetNext(pos));
		if (pBar != NULL)
		{
			pBar->ApplyRestoredTabInfo(TRUE);
		}
	}

	CObList lstCopy;
	lstCopy.AddTail(&m_lstControlBars);

	// check for empty sliders and tabbed bars
	for (pos = lstCopy.GetTailPosition(); pos != NULL;)
	{
		CObject* pBar = lstCopy.GetPrev(pos);
		CPaneDivider* pSlider = DYNAMIC_DOWNCAST(CPaneDivider, pBar);

		if (pSlider != NULL)
		{
			pSlider->NotifyAboutRelease();
			continue;
		}

		CBaseTabbedPane* pTabbedBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, pBar);
		if (pTabbedBar != NULL && pTabbedBar->GetTabsNum() == 0 && pTabbedBar->CanFloat())
		{
			pTabbedBar->UndockPane(TRUE);
			pTabbedBar->DestroyWindow();
		}
	}

	RecalcLayout();

	m_lstLoadedBars.RemoveAll();
	m_lstLoadedMiniFrames.RemoveAll();
	m_lstNonFloatingBars.RemoveAll();

	m_bRestoringDockState = FALSE;
}

void AFX_AUTOHIDE_DOCKSITE_SAVE_INFO::Serialize(CArchive& ar)
{
	if (ar.IsLoading())
	{
		ar >> m_nBarID;
		ar >> m_dwBarAlignment;
		ar >> m_bIsVisible;

		int nSiblingCount = 0;
		ar >> nSiblingCount;

		for (int i = 0; i < nSiblingCount; i++)
		{
			UINT nSiblingBarID = (UINT)-1;
			ar >> nSiblingBarID;
			if (nSiblingBarID != -1) // can't be tabbed or so on
			{
				m_lstSiblingBars.AddHead(nSiblingBarID);
			}
		}
		ar >> m_rectBar;

		ar >> m_bFirstInGroup;
		ar >> m_bLastInGroup;
		ar >> m_bActiveInGroup;
	}
	else
	{
		ENSURE(m_pSavedBar != NULL);

		ar << m_pSavedBar->GetDlgCtrlID();
		ar << m_pSavedBar->GetCurrentAlignment();
		ar <<(m_pSavedBar->IsHideInAutoHideMode() ? m_pSavedBar->IsVisible() : TRUE);

		CList<UINT, UINT&> lstSiblings;
		m_pSavedBar->GetRecentSiblingPaneInfo(lstSiblings);

		int nSiblingCount = (int) lstSiblings.GetCount();
		ar << nSiblingCount;

		for (POSITION pos = lstSiblings.GetHeadPosition(); pos != NULL;)
		{
			UINT nSiblingBarID = lstSiblings.GetNext(pos);
			ar << nSiblingBarID;
		}
		m_pSavedBar->GetWindowRect(&m_rectBar);
		if (m_rectBar.IsRectEmpty())
		{
			ar << m_pSavedBar->GetAHRestoredRect();
		}
		else
		{
			ar << m_rectBar;
		}

		ar << m_pSavedBar->GetAutoHideToolBar()->m_bFirstInGroup;
		ar << m_pSavedBar->GetAutoHideToolBar()->m_bLastInGroup;
		ar << m_pSavedBar->GetAutoHideToolBar()->m_bActiveInGroup;
	}
}

void CDockingManager::HideForPrintPreview(const CObList& lstBars)
{
	for (POSITION pos = lstBars.GetHeadPosition(); pos != NULL;)
	{
		CBasePane* pBar = (CBasePane*) lstBars.GetNext(pos);
		ASSERT_VALID(pBar);

		ASSERT_VALID(pBar);

		if (m_bHideDockingBarsInContainerMode || !IsOLEContainerMode())
		{
			if (pBar->IsVisible() && pBar->HideInPrintPreviewMode())
			{
				pBar->ShowPane(FALSE, TRUE, FALSE);
				m_lstBarsHiddenInPreview.AddTail(pBar);
			}

			for (POSITION posList = m_lstMiniFrames.GetHeadPosition(); posList != NULL;)
			{
				CWnd* pWnd = (CWnd*) m_lstMiniFrames.GetNext(posList);
				ASSERT_VALID(pWnd);

				if (pWnd->IsWindowVisible())
				{
					pWnd->ShowWindow(SW_HIDE);
					m_lstBarsHiddenInPreview.AddTail(pWnd);
				}
			}
		}
		else
		{
			if (pBar->IsVisible() && pBar->HideInPrintPreviewMode() && !pBar->IsKindOf(RUNTIME_CLASS(CDockablePane)) && !pBar->IsKindOf(RUNTIME_CLASS(CAutoHideDockSite)) && !pBar->IsKindOf(RUNTIME_CLASS(CPaneDivider)))
			{
				pBar->ShowPane(FALSE, TRUE, FALSE);
				m_lstBarsHiddenInPreview.AddTail(pBar);
			}
		}
	}
}

void CDockingManager::SetPrintPreviewMode(BOOL bPreview, CPrintPreviewState* /*pState*/)
{
	ASSERT_VALID(this);

	if (bPreview)
	{
		if (m_bIsPrintPreviewMode || IsOLEContainerMode())
		{
			m_bIsPrintPreviewMode = TRUE;
			return;
		}
		m_lstBarsHiddenInPreview.RemoveAll();

		// Set visibility of standard ControlBars
		if (m_bHideDockingBarsInContainerMode || !IsOLEContainerMode())
		{
			HideForPrintPreview(m_lstAutoHideBars);
		}
		HideForPrintPreview(m_lstControlBars);
	}
	else
	{
		if (!m_bIsPrintPreviewMode || IsOLEContainerMode())
		{
			m_bIsPrintPreviewMode = FALSE;
			return;
		}
		for (POSITION pos = m_lstBarsHiddenInPreview.GetHeadPosition(); pos != NULL;)
		{
			CWnd* pWnd = DYNAMIC_DOWNCAST(CWnd, m_lstBarsHiddenInPreview.GetNext(pos));
			if (pWnd == NULL)
			{
				ASSERT(FALSE);
				continue;
			}
			ASSERT_VALID(pWnd);

			if (pWnd->IsKindOf(RUNTIME_CLASS(CBasePane)))
			{
				CBasePane* pBar = DYNAMIC_DOWNCAST(CBasePane, pWnd);
				ASSERT_VALID(pBar);
				pBar->ShowPane(TRUE, TRUE, FALSE);
			}
			else
			{
				pWnd->ShowWindow(SW_SHOWNOACTIVATE);
			}

		}
	}

	m_bIsPrintPreviewMode = bPreview;
}

CPaneFrameWnd* CDockingManager::FrameFromPoint(CPoint pt, CPaneFrameWnd* pFrameToExclude, BOOL bFloatMultiOnly) const
{
	for (POSITION pos = m_lstMiniFrames.GetHeadPosition(); pos != NULL;)
	{
		CPaneFrameWnd* pWnd = DYNAMIC_DOWNCAST(CPaneFrameWnd, m_lstMiniFrames.GetNext(pos));
		ASSERT_VALID(pWnd);

		if (!pWnd->IsWindowVisible() || pWnd == pFrameToExclude)
		{
			continue;
		}

		if (!pWnd->IsKindOf(RUNTIME_CLASS(CMultiPaneFrameWnd)) && bFloatMultiOnly)
		{
			continue;
		}

		CRect rect;
		pWnd->GetWindowRect(rect);

		if (rect.PtInRect(pt))
		{
			return pWnd;
		}
	}

	return NULL;
}

BOOL CDockingManager::SendMessageToMiniFrames(UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	for (POSITION pos = m_lstMiniFrames.GetHeadPosition(); pos != NULL;)
	{
		CPaneFrameWnd* pWnd = DYNAMIC_DOWNCAST(CPaneFrameWnd, m_lstMiniFrames.GetNext(pos));

		ASSERT_VALID(pWnd);
		pWnd->SendMessage(uMessage, wParam, lParam);
	}

	return TRUE;
}

void CDockingManager::LockUpdate(BOOL bLock)
{
	if (bLock && m_pSDManager != NULL && m_pSDManager->IsStarted())
	{
		return;
	}

	m_bLockUpdate = bLock;

	bLock ? m_pParentWnd->LockWindowUpdate() : m_pParentWnd->UnlockWindowUpdate();

	POSITION pos = NULL;

	for (pos = m_lstMiniFrames.GetHeadPosition(); pos != NULL;)
	{
		CPaneFrameWnd* pWnd = DYNAMIC_DOWNCAST(CPaneFrameWnd, m_lstMiniFrames.GetNext(pos));

		pWnd->ValidateRect(NULL);
		pWnd->UpdateWindow();

		ASSERT_VALID(pWnd);
		bLock ? pWnd->LockWindowUpdate() : pWnd->UnlockWindowUpdate();
	}

	for (pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CWnd* pWnd = (CWnd*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pWnd);

		pWnd->ValidateRect(NULL);
		pWnd->UpdateWindow();

		bLock ? pWnd->LockWindowUpdate() : pWnd->UnlockWindowUpdate();
	}
}

void CDockingManager::GetPaneList(CObList& lstBars, BOOL bIncludeAutohide, CRuntimeClass* pRTCFilter, BOOL bIncludeTabs)
{
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CBasePane* pBar = DYNAMIC_DOWNCAST(CBasePane, m_lstControlBars.GetNext(pos));
		ASSERT_VALID(pBar);

		if (pBar->IsKindOf(RUNTIME_CLASS(CDockSite)))
		{
			CDockSite* pDockBar = DYNAMIC_DOWNCAST(CDockSite, pBar);
			ASSERT_VALID(pDockBar);

			const CObList& lstDockedBars = pDockBar->GetPaneList();

			for (POSITION posList = lstDockedBars.GetHeadPosition(); posList != NULL;)
			{
				CBasePane* pDockedBar = DYNAMIC_DOWNCAST(CBasePane, lstDockedBars.GetNext(posList));
				ASSERT_VALID(pDockedBar);

				if (pRTCFilter == NULL || pDockedBar->GetRuntimeClass() == pRTCFilter)
				{
					lstBars.AddTail(pDockedBar);
				}
			}
		}
		else if (pBar->IsKindOf(RUNTIME_CLASS(CTabbedPane)) && bIncludeTabs)
		{
			CTabbedPane* pTabbedBar = DYNAMIC_DOWNCAST(CTabbedPane, pBar);
			ASSERT_VALID(pTabbedBar);
			pTabbedBar->GetPaneList(lstBars, pRTCFilter);
		}

		if (pRTCFilter == NULL || pBar->GetRuntimeClass() == pRTCFilter)
		{
			lstBars.AddTail(pBar);
		}
	}

	if (bIncludeAutohide)
	{
		for (POSITION pos = m_lstAutoHideBars.GetHeadPosition(); pos != NULL;)
		{
			CBasePane* pBar = DYNAMIC_DOWNCAST(CBasePane, m_lstAutoHideBars.GetNext(pos));
			ASSERT_VALID(pBar);

			if (pBar->IsKindOf(RUNTIME_CLASS(CPaneDivider)))
			{
				CPaneDivider* pSlider = DYNAMIC_DOWNCAST(CPaneDivider, pBar);
				// SLIDER CONTAINS ONLY ONE BAR IN AUTOHIDE MODE
				if (pSlider != NULL)
				{
					pBar = DYNAMIC_DOWNCAST(CBasePane, pSlider->GetFirstPane());
					if (pBar != NULL &&
						(pRTCFilter == NULL || pBar->GetRuntimeClass() == pRTCFilter))
					{
						lstBars.AddTail(pBar);
					}
				}
			}
		}
	}

	CPaneFrameWnd::GetPaneList(lstBars, pRTCFilter, bIncludeTabs);
}

BOOL CDockingManager::ShowPanes(BOOL bShow)
{
	if (!bShow)
	{
		if (m_bHiddenForOLE)
		{
			return FALSE;
		}

		m_lstBarsHiddenForOLE.RemoveAll();

		CObList lstBars;
		GetPaneList(lstBars, TRUE, NULL, TRUE);

		BOOL bHideInAutoHideMode = CDockablePane::m_bHideInAutoHideMode;
		CDockablePane::m_bHideInAutoHideMode = TRUE;

		m_bDisableRecalcLayout = TRUE;
		for (POSITION pos = lstBars.GetHeadPosition(); pos != NULL;)
		{
			CBasePane* pBar = DYNAMIC_DOWNCAST(CBasePane, lstBars.GetNext(pos));

			if (pBar != NULL)
			{
				DWORD dwStyle = pBar->GetPaneStyle();
				if ((dwStyle & CBRS_HIDE_INPLACE) != 0 && (pBar->IsVisible() || pBar->IsAutoHideMode()))
				{
					pBar->ShowPane(FALSE, TRUE, FALSE);
					HWND hwndNext = pBar->GetSafeHwnd();
					m_lstBarsHiddenForOLE.AddTail(hwndNext);
				}
			}
		}
		m_bDisableRecalcLayout = FALSE;

		CDockablePane::m_bHideInAutoHideMode = bHideInAutoHideMode;

		m_bHiddenForOLE = TRUE;
	}
	else
	{
		if (!m_bHiddenForOLE)
		{
			return FALSE;
		}

		BOOL bHideInAutoHideMode = CDockablePane::m_bHideInAutoHideMode;
		CDockablePane::m_bHideInAutoHideMode = TRUE;

		m_bDisableRecalcLayout = TRUE;
		for (POSITION pos = m_lstBarsHiddenForOLE.GetHeadPosition(); pos != NULL;)
		{
			CBasePane* pBar = DYNAMIC_DOWNCAST(CBasePane, CWnd::FromHandlePermanent(m_lstBarsHiddenForOLE.GetNext(pos)));
			if (pBar != NULL)
			{
				pBar->ShowPane(TRUE, TRUE, FALSE);
			}
		}
		m_bDisableRecalcLayout = FALSE;
		CDockablePane::m_bHideInAutoHideMode = bHideInAutoHideMode;

		m_bHiddenForOLE = FALSE;
	}

	AdjustDockingLayout();

	// significantly reduces flickering. If we return TRUE, MFC will perform
	// additional recalc layout
	return FALSE;
}

void CDockingManager::ShowDelayShowMiniFrames(BOOL bShow)
{
	for (POSITION pos = m_lstMiniFrames.GetHeadPosition(); pos != NULL;)
	{
		CWnd* pWndNext = (CWnd *)m_lstMiniFrames.GetNext(pos);
		if (pWndNext->IsKindOf(RUNTIME_CLASS(CPaneFrameWnd)))
		{
			CPaneFrameWnd *pFrameWnd = DYNAMIC_DOWNCAST(CPaneFrameWnd, pWndNext);
			if (pFrameWnd->IsDelayShow())
			{
				HWND hWndNext = pWndNext->GetSafeHwnd();
				if (::IsWindow(hWndNext))
				{
					ShowWindow(hWndNext, bShow ? SW_SHOWNOACTIVATE : SW_HIDE);
				}
			}
		}
	}
}

void CDockingManager::OnActivateFrame(BOOL bActivate)
{
	if (m_pParentWnd == NULL)
	{
		return;
	}

	BOOL bCheckForToolBarsOnly = !m_pParentWnd->IsKindOf(RUNTIME_CLASS(CMDIChildWndEx));

	if (bActivate)
	{
		for (POSITION pos = m_lstBarsHiddenOnDeactivate.GetHeadPosition(); pos != NULL;)
		{
			HWND hWndNext = m_lstBarsHiddenOnDeactivate.GetNext(pos);
			if (IsWindow(hWndNext))
			{
				CWnd* pWndNext = CWnd::FromHandle(hWndNext);
				CPaneFrameWnd* pMiniFrame = DYNAMIC_DOWNCAST(CPaneFrameWnd, pWndNext);
				if (pMiniFrame != NULL && pMiniFrame->GetPaneCount() > 0)
				{
					ShowWindow(hWndNext, SW_SHOWNOACTIVATE);
				}
			}
		}

		m_lstBarsHiddenOnDeactivate.RemoveAll();
	}
	else
	{
		for (POSITION pos = m_lstMiniFrames.GetHeadPosition(); pos != NULL;)
		{
			CWnd* pWndNext = (CWnd*) m_lstMiniFrames.GetNext(pos);

			HWND hWndNext = pWndNext->GetSafeHwnd();
			if (::IsWindow(hWndNext) && IsWindowVisible(hWndNext))
			{
				if (bCheckForToolBarsOnly)
				{
					CPaneFrameWnd* pMiniFrame = DYNAMIC_DOWNCAST(CPaneFrameWnd, pWndNext);
					ASSERT_VALID(pMiniFrame);
					CMFCBaseToolBar* pToolbar = DYNAMIC_DOWNCAST(CMFCBaseToolBar, pMiniFrame->GetPane());

					if (pToolbar == NULL)
					{
						continue;
					}
				}

				ShowWindow(hWndNext, SW_HIDE);
				if (m_lstBarsHiddenOnDeactivate.Find(hWndNext) == NULL)
				{
					m_lstBarsHiddenOnDeactivate.AddTail(hWndNext);
				}
			}
		}
	}
}

void CDockingManager::ResortMiniFramesForZOrder()
{
	int nCount = (int) m_lstMiniFrames.GetCount();

	if (nCount == 0)
	{
		return;
	}

	CWnd* pFirst = DYNAMIC_DOWNCAST(CWnd, m_lstMiniFrames.GetHead());

	if (pFirst == NULL)
	{
		return;
	}

	CWnd* pParent = pFirst->GetParent();

	if (pParent == NULL)
	{
		return;
	}

	CObList lstNewMiniFrames;

	CWnd* pNext = NULL;
	for (pNext = pParent->GetWindow(GW_HWNDFIRST); pNext != NULL; pNext = pNext->GetNextWindow() )
	{
		if (m_lstMiniFrames.Find(pNext) != NULL)
		{
			lstNewMiniFrames.AddTail(pNext);
		}
	}

	m_lstMiniFrames.RemoveAll();
	m_lstMiniFrames.AddTail(&lstNewMiniFrames);
}

void __stdcall CDockingManager::SetDockingMode(AFX_DOCK_TYPE dockMode)
{
	m_dockModeGlobal = dockMode;

	if (m_dockModeGlobal == DT_SMART)
	{
		// DT_SMART should only be used along with DT_IMMEDIATE
		m_dockModeGlobal = AFX_DOCK_TYPE(DT_SMART | DT_IMMEDIATE);
	}
}

void __stdcall CDockingManager::SetSmartDockingParams(CSmartDockingInfo& params)
{
	int nCount = 0;

	for (int i = 0; i < AFX_SD_MARKERS_NUM; i++)
	{
		if (params.m_uiMarkerBmpResID [i] != 0)
		{
			nCount++;
		}
	}

	if (nCount != 0 && nCount != AFX_SD_MARKERS_NUM)
	{
		// Unable to set part of bitmap markers!
		ASSERT(FALSE);
		return;
	}

	params.CopyTo(m_SDParams);
	m_bSDParamsModified = TRUE;
}

BOOL CDockingManager::ReplacePane(CDockablePane* pOriginalBar, CDockablePane* pNewBar)
{
	if (pOriginalBar == NULL || pNewBar == NULL)
	{
		return FALSE;
	}
	ASSERT_VALID(pNewBar);
	ASSERT_VALID(pOriginalBar);

	CRect rectOrgWnd;
	pOriginalBar->GetWindowRect(rectOrgWnd);

	CWnd* pOrgParentWnd = pOriginalBar->GetParent();
	ASSERT_VALID(pOrgParentWnd);

	pOrgParentWnd->ScreenToClient(rectOrgWnd);
	pOriginalBar->StoreRecentDockSiteInfo();
	pNewBar->CopyState(pOriginalBar);

	if (pOriginalBar->IsAutoHideMode())
	{
		// hide the original bar
		pOriginalBar->Slide(FALSE, FALSE);

		// set the same window pos for the new bar
		pNewBar->SetWindowPos(NULL, rectOrgWnd.left, rectOrgWnd.top, rectOrgWnd.Width(), rectOrgWnd.Height(), SWP_NOZORDER);

		pNewBar->ShowWindow(SW_HIDE);

		// replace bar in default slider and in the button
		CMFCAutoHideButton* pButton = pOriginalBar->GetAutoHideButton();
		ASSERT_VALID(pButton);

		CPaneDivider* pSlider = pOriginalBar->GetDefaultPaneDivider();
		if (pSlider != NULL)
		{
			ASSERT_VALID(pSlider);
			pSlider->ReplacePane(pOriginalBar, pNewBar);
		}

		if (pButton != NULL)
		{
			pButton->ReplacePane(pNewBar);
		}

		// reparent
		pNewBar->SetParent(pOrgParentWnd);

		// tell the new bar that it's in autohide mode
		pNewBar->m_bPinState = TRUE;
		pNewBar->m_nAutoHideConditionTimerID = pNewBar->SetTimer(AFX_ID_CHECK_AUTO_HIDE_CONDITION, pNewBar->m_nTimeOutBeforeAutoHide, NULL);
		AlignAutoHidePane(pSlider);

		// need to update caption buttons
		pNewBar->SetWindowPos(NULL, -1, -1, -1, -1, SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);

		return TRUE;
	}

	if (pOriginalBar->IsTabbed())
	{
		HWND hwndTab = NULL;
		CMFCBaseTabCtrl* pTabWnd = pOriginalBar->GetParentTabWnd(hwndTab);

		if (pTabWnd != NULL)
		{
			ASSERT_VALID(pTabWnd);

			int nTabNum = pTabWnd->GetTabFromHwnd(pOriginalBar->GetSafeHwnd());

			CString strText;
			pOriginalBar->GetWindowText(strText);

			pTabWnd->InsertTab(pNewBar, strText, nTabNum + 1);
			pTabWnd->SetTabHicon(nTabNum + 1, pNewBar->GetIcon(FALSE));
			pTabWnd->RemoveTab(nTabNum);

			pNewBar->EnableGripper(FALSE);
			pNewBar->SetParent(pTabWnd);

			return TRUE;
		}
	}

	BOOL bResult = pOriginalBar->ReplacePane(pNewBar, DM_UNKNOWN, TRUE);
	if (bResult)
	{
		pNewBar->SetParent(pOrgParentWnd);
		pNewBar->SetWindowPos(NULL, rectOrgWnd.left, rectOrgWnd.top, rectOrgWnd.Width(), rectOrgWnd.Height(), SWP_NOZORDER);
		AdjustDockingLayout();

	}

	return bResult;
}

void CDockingManager::ReleaseEmptyPaneContainers()
{
	POSITION pos = NULL;

	for (pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CPaneDivider* pSlider = DYNAMIC_DOWNCAST(CPaneDivider, m_lstControlBars.GetNext(pos));
		if (pSlider != NULL && pSlider->IsDefault())
		{
			pSlider->ReleaseEmptyPaneContainers();
		}
	}

	for (pos = m_lstMiniFrames.GetHeadPosition(); pos != NULL;)
	{
		CMultiPaneFrameWnd* pWndNext = DYNAMIC_DOWNCAST(CMultiPaneFrameWnd, m_lstMiniFrames.GetNext(pos));
		if (pWndNext != NULL)
		{
			CPaneContainerManager& manager = pWndNext->GetPaneContainerManager();
			manager.ReleaseEmptyPaneContainers();
		}
	}
}

void CDockingManager::BuildPanesMenu(CMenu& menu, BOOL bToolbarsOnly)
{
	m_mapControlBarsInMenu.RemoveAll();

	CObList lstBars;
	GetPaneList(lstBars, TRUE);

	for (int nStep = 0; nStep < 2; nStep++) // 2 steps: 1-st: show toolbars, 2-nd other control bars
	{
		if (nStep == 1 && bToolbarsOnly)
		{
			break;
		}

		BOOL bIsFirst = TRUE;

		for (POSITION pos = lstBars.GetHeadPosition(); pos != NULL;)
		{
			CPane* pBar = DYNAMIC_DOWNCAST(CPane, lstBars.GetNext(pos));

			if (pBar == NULL || !::IsWindow(pBar->m_hWnd))
			{
				continue;
			}

			ASSERT_VALID(pBar);

			if (!pBar->AllowShowOnPaneMenu() || !pBar->CanBeClosed())
			{
				continue;
			}

			const BOOL bIsToolbar = pBar->IsKindOf(RUNTIME_CLASS(CMFCToolBar));

			if ((bIsToolbar && nStep == 1) ||(!bIsToolbar && nStep == 0))
			{
				continue;
			}

			CString strBarName;
			pBar->GetPaneName(strBarName);

			if (pBar->IsKindOf(RUNTIME_CLASS(CBaseTabbedPane)) && !pBar->IsKindOf(RUNTIME_CLASS(CMFCOutlookBar)))
			{
				CMFCBaseTabCtrl* pTabWnd = ((CBaseTabbedPane*)pBar)->GetUnderlyingWindow();
				if (pTabWnd != NULL)
				{
					for (int iTab = 0; iTab < pTabWnd->GetTabsNum(); iTab++)
					{
						CPane* pTabbedBar = DYNAMIC_DOWNCAST(CPane, pTabWnd->GetTabWnd(iTab));
						if (pTabbedBar != NULL && pTabbedBar->AllowShowOnPaneMenu())
						{
							CPane* pBarInMenu = NULL;

							if (!m_mapControlBarsInMenu.Lookup(pTabbedBar->GetDlgCtrlID(), pBarInMenu))
							{
								pTabbedBar->GetPaneName(strBarName);

								if (bIsFirst && nStep == 1 && menu.GetMenuItemCount() > 0)
								{
									menu.AppendMenu(MF_SEPARATOR);
								}

								menu.AppendMenu(MF_STRING, pTabbedBar->GetDlgCtrlID(), strBarName);

								bIsFirst = FALSE;

								m_mapControlBarsInMenu.SetAt(pTabbedBar->GetDlgCtrlID(), pTabbedBar);
							}
						}
					}
				}
			}
			else if (pBar->IsKindOf(RUNTIME_CLASS(CMFCReBar)))
			{
				CMFCReBar* pRebar = DYNAMIC_DOWNCAST(CMFCReBar, pBar);
				ASSERT_VALID(pBar);

				CReBarCtrl& wndReBar = pRebar->GetReBarCtrl();
				UINT uiReBarsCount = wndReBar.GetBandCount();

				REBARBANDINFO bandInfo;
				bandInfo.cbSize = pRebar->GetReBarBandInfoSize ();
				bandInfo.fMask = (RBBIM_CHILDSIZE | RBBIM_CHILD | RBBIM_IDEALSIZE);

				for (UINT uiBand = 0; uiBand < uiReBarsCount; uiBand++)
				{
					wndReBar.GetBandInfo(uiBand, &bandInfo);

					CPane* pRebarBand = DYNAMIC_DOWNCAST(CPane, CWnd::FromHandlePermanent(bandInfo.hwndChild));

					if (pRebarBand != NULL && pRebarBand->AllowShowOnPaneMenu())
					{
						pRebarBand->GetPaneName(strBarName);

						if (bIsFirst && nStep == 1 && menu.GetMenuItemCount() > 0)
						{
							menu.AppendMenu(MF_SEPARATOR);
						}

						menu.AppendMenu(MF_STRING, pRebarBand->GetDlgCtrlID(), strBarName);

						bIsFirst = FALSE;

						m_mapControlBarsInMenu.SetAt(pRebarBand->GetDlgCtrlID(), pRebarBand);
					}
				}
			}
			else
			{
				CPane* pBarInMenu = NULL;

				if (!m_mapControlBarsInMenu.Lookup(pBar->GetDlgCtrlID(), pBarInMenu))
				{
					if (bIsFirst && nStep == 1 && menu.GetMenuItemCount() > 0)
					{
						menu.AppendMenu(MF_SEPARATOR);
					}

					menu.AppendMenu(MF_STRING, pBar->GetDlgCtrlID(), strBarName);
					bIsFirst = FALSE;
					m_mapControlBarsInMenu.SetAt(pBar->GetDlgCtrlID(), pBar);
				}
			}
		}
	}

	// Add MDI tabbed bars(if any):
	CMDIFrameWndEx* pMDIFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, m_pParentWnd);

	if (pMDIFrame != NULL && !bToolbarsOnly)
	{
		HWND hwndMDIChild = ::GetWindow(pMDIFrame->m_hWndMDIClient, GW_CHILD);

		while (hwndMDIChild != NULL)
		{
			CMDIChildWndEx* pMDIChildFrame = DYNAMIC_DOWNCAST(CMDIChildWndEx, CWnd::FromHandle(hwndMDIChild));

			if (pMDIChildFrame != NULL && pMDIChildFrame->IsTabbedPane())
			{
				CDockablePane* pBar = pMDIChildFrame->GetTabbedPane();
				ASSERT_VALID(pBar);

				CString strBarName;
				pBar->GetPaneName(strBarName);

				menu.AppendMenu(MF_STRING, pBar->GetDlgCtrlID(), strBarName);
				m_mapControlBarsInMenu.SetAt(pBar->GetDlgCtrlID(), pBar);
			}

			hwndMDIChild = ::GetWindow(hwndMDIChild, GW_HWNDNEXT);
		}

		for (POSITION pos = m_lstHiddenMDITabbedBars.GetHeadPosition(); pos != NULL;)
		{
			HWND hwnd = m_lstHiddenMDITabbedBars.GetNext(pos);
			CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, CWnd::FromHandlePermanent(hwnd));

			if (pBar != NULL)
			{
				ASSERT_VALID(pBar);

				CString strBarName;
				pBar->GetPaneName(strBarName);

				menu.AppendMenu(MF_STRING, pBar->GetDlgCtrlID(), strBarName);
				m_mapControlBarsInMenu.SetAt(pBar->GetDlgCtrlID(), pBar);
			}
		}
	}

	if (m_uiCustomizeCmd != 0)
	{
		if (menu.GetMenuItemCount() > 0)
		{
			menu.AppendMenu(MF_SEPARATOR);
		}

		menu.AppendMenu(MF_STRING, m_uiCustomizeCmd, m_strCustomizeText);
	}
}

void CDockingManager::OnPaneContextMenu(CPoint point)
{
	if (!m_bControlBarsContextMenu)
	{
		return;
	}

	CFrameWnd* pFrame = AFXGetTopLevelFrame(m_pParentWnd);
	if (pFrame == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	CMenu menu;
	menu.CreatePopupMenu();

	BuildPanesMenu(menu, m_bControlBarsContextMenuToolbarsOnly);

	CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;
	pPopupMenu->SetAutoDestroy(FALSE);

	m_bControlBarsMenuIsShown = TRUE;

	pPopupMenu->Create(pFrame, point.x, point.y, (HMENU) menu);
}

BOOL CDockingManager::ProcessPaneContextMenuCommand(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* /*pHandlerInfo*/)
{
	if (!m_bControlBarsContextMenu || m_mapControlBarsInMenu.IsEmpty())
	{
		return FALSE;
	}

	if (nCode == CN_UPDATE_COMMAND_UI && !m_bControlBarsMenuIsShown)
	{
		return FALSE;
	}

	CPane* pBar = NULL;
	if (!m_mapControlBarsInMenu.Lookup(nID, pBar) || pBar == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(pBar);

	if (nCode == CN_UPDATE_COMMAND_UI)
	{
		CCmdUI* pCmdUI = (CCmdUI*) pExtra;
		if (pCmdUI == NULL)
		{
			return FALSE;
		}

		pCmdUI->SetCheck(pBar->IsVisible());
		return TRUE;
	}

	UINT nMsg = HIWORD(nCode);
	nCode = LOWORD(nCode);

	if ((nMsg != WM_COMMAND && nMsg != 0) || pExtra != NULL)
	{
		return TRUE;
	}

	pBar->ShowPane(!pBar->IsVisible(), FALSE, TRUE);

	CFrameWnd* pFrame = AFXGetTopLevelFrame(pBar);
	if (pFrame == NULL)
	{
		RecalcLayout();
	}
	else
	{
		pFrame->RecalcLayout();
	}

	m_mapControlBarsInMenu.RemoveAll();
	return TRUE;
}

void CDockingManager::OnClosePopupMenu()
{
	m_bControlBarsMenuIsShown = FALSE;
}

void CDockingManager::EnablePaneContextMenu(BOOL bEnable, UINT uiCustomizeCmd, const CString& strCustomizeText, BOOL bToolbarsOnly)
{
	m_bControlBarsContextMenu = bEnable;
	m_bControlBarsContextMenuToolbarsOnly = bToolbarsOnly;
	m_uiCustomizeCmd = uiCustomizeCmd;
	m_strCustomizeText = strCustomizeText;
}

void CDockingManager::AddHiddenMDITabbedBar(CDockablePane* pBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);

	HWND hwnd = pBar->GetSafeHwnd();
	m_lstHiddenMDITabbedBars.AddTail(hwnd);
}

void CDockingManager::RemoveHiddenMDITabbedBar(CDockablePane* pBar)
{
	ASSERT_VALID(this);

	HWND hwnd = pBar->GetSafeHwnd();
	for (POSITION pos = m_lstHiddenMDITabbedBars.GetHeadPosition(); pos != NULL; m_lstHiddenMDITabbedBars.GetNext(pos))
	{
		HWND hNext = m_lstHiddenMDITabbedBars.GetAt(pos);
		if (hNext == hwnd)
		{
			m_lstHiddenMDITabbedBars.RemoveAt(pos);
			return;
		}
	}
}

CSmartDockingInfo::CSmartDockingInfo()
{
	m_sizeTotal = CSize(93, 93);
	m_nCentralGroupOffset = 5;
	m_clrTransparent = RGB(255, 0, 255);
	m_clrToneSrc = (COLORREF)-1;
	m_clrToneDest = (COLORREF)-1;

	for (int i = 0; i < AFX_SD_MARKERS_NUM; i++)
	{
		m_uiMarkerBmpResID [i] = 0;
		m_uiMarkerLightBmpResID [i] = 0;
	}

	m_clrBaseBackground = (COLORREF)-1;
	m_clrBaseBorder = (COLORREF)-1;
	m_bUseThemeColorInShading = FALSE;
}

void CSmartDockingInfo::CopyTo(CSmartDockingInfo& params)
{
	params.m_sizeTotal = m_sizeTotal;
	params.m_nCentralGroupOffset = m_nCentralGroupOffset;
	params.m_clrTransparent = m_clrTransparent;
	params.m_clrToneSrc = m_clrToneSrc;
	params.m_clrToneDest = m_clrToneDest;

	for (int i = 0; i < AFX_SD_MARKERS_NUM; i++)
	{
		params.m_uiMarkerBmpResID [i] = m_uiMarkerBmpResID [i];
		params.m_uiMarkerLightBmpResID [i] = m_uiMarkerLightBmpResID [i];
	}

	params.m_clrBaseBackground = m_clrBaseBackground;
	params.m_clrBaseBorder = m_clrBaseBorder;
	params.m_bUseThemeColorInShading = m_bUseThemeColorInShading;
}



