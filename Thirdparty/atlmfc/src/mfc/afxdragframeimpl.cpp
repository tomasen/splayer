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

#include "afxdragframeimpl.h"

#include "afxtabbedpane.h"
#include "afxpaneframewnd.h"
#include "afxdockingmanager.h"
#include "afxglobals.h"
#include "afxglobalutils.h"
#include "afxmultipaneframewnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CDummyDockablePane : public CDockablePane
{
	virtual void DoPaint(CDC* /*pDC*/) {}

protected:
	afx_msg BOOL OnEraseBkgnd(CDC* /*pDC*/) {return FALSE;}
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CDummyDockablePane, CDockablePane)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

static UINT AFX_DUMMY_WND_ID = (UINT) -2;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFCDragFrameImpl::CMFCDragFrameImpl()
{
	m_rectDrag.SetRectEmpty();
	m_rectExpectedDocked.SetRectEmpty();
	m_ptHot.x = m_ptHot.y = 0;
	m_nOldThickness = afxGlobalData.m_nDragFrameThicknessFloat;
	m_pDraggedWnd = NULL;
	m_pDockManager = NULL;
	m_pTargetBar = NULL;
	m_nInsertedTabID = -1;
	m_bDockToTab = FALSE;
	m_pFinalTargetBar = NULL;
	m_bDragStarted = FALSE;
	m_bFrameTabDrawn = FALSE;
	m_pOldTargetBar = NULL;
	m_pWndDummy = NULL;
}

CMFCDragFrameImpl::~CMFCDragFrameImpl()
{
	if (m_pWndDummy != NULL)
	{
		m_pWndDummy->DestroyWindow();
		delete m_pWndDummy;
	}
}

void CMFCDragFrameImpl::Init(CWnd* pDraggedWnd)
{
	ASSERT_VALID(pDraggedWnd);
	m_pDraggedWnd = pDraggedWnd;

	CWnd* pDockSite = NULL;
	if (m_pDraggedWnd->IsKindOf(RUNTIME_CLASS(CPaneFrameWnd)))
	{
		CPaneFrameWnd* pMiniFrame = DYNAMIC_DOWNCAST(CPaneFrameWnd, m_pDraggedWnd);
		pDockSite = pMiniFrame->GetParent();
	}
	else if (m_pDraggedWnd->IsKindOf(RUNTIME_CLASS(CPane)))
	{
		CPane* pBar = DYNAMIC_DOWNCAST(CPane, m_pDraggedWnd);
		ASSERT_VALID(pBar);

		CPaneFrameWnd* pParentMiniFrame = pBar->GetParentMiniFrame();
		if (pParentMiniFrame != NULL)
		{
			pDockSite = pParentMiniFrame->GetParent();
		}
		else
		{
			pDockSite = pBar->GetDockSiteFrameWnd();
		}
	}

	m_pDockManager = afxGlobalUtils.GetDockingManager(pDockSite);
	if (afxGlobalUtils.m_bDialogApp)
	{
		return;
	}

	ENSURE(m_pDockManager != NULL);
}

void CMFCDragFrameImpl::MoveDragFrame(BOOL bForceMove)
{
	ASSERT_VALID(m_pDraggedWnd);

	m_pFinalTargetBar = NULL;

	if (m_pDraggedWnd == NULL || m_pDockManager == NULL)
	{
		return;
	}

	if (m_pWndDummy == NULL)
	{
		m_pWndDummy = new CDummyDockablePane;
		m_pWndDummy->CreateEx(0, _T(""), AFXGetTopLevelFrame(m_pDraggedWnd), CRect(0, 0, 0, 0), FALSE, AFX_DUMMY_WND_ID, WS_CHILD);
	}

	CSize szSensitivity = CDockablePane::GetDragSensitivity();

	CPoint ptMouse;
	GetCursorPos(&ptMouse);

	CPoint ptOffset = ptMouse - m_ptHot;

	if (abs(ptOffset.x) < szSensitivity.cx && abs(ptOffset.y) < szSensitivity.cy && m_rectDrag.IsRectEmpty() && !bForceMove)
	{
		return;
	}

	m_bDragStarted = TRUE;

	m_pDockManager->LockUpdate(TRUE);

	CRect rectOld = m_rectExpectedDocked.IsRectEmpty() ? m_rectDrag : m_rectExpectedDocked;
	BOOL bFirstTime = FALSE;

	if (m_rectDrag.IsRectEmpty())
	{
		if (m_pDraggedWnd->IsKindOf(RUNTIME_CLASS(CPaneFrameWnd)))
		{
			m_pDraggedWnd->GetWindowRect(m_rectDrag);
		}
		else if (m_pDraggedWnd->IsKindOf(RUNTIME_CLASS(CPane)))
		{
			CPane* pBar = DYNAMIC_DOWNCAST(CPane, m_pDraggedWnd);
			ASSERT_VALID(pBar);
			m_pDraggedWnd->GetWindowRect(m_rectDrag);

			// if the bar is docked then the floating rect has to be set to recent floating rect
			if (pBar->GetParentMiniFrame() == NULL)
			{
				m_rectDrag.right = m_rectDrag.left + pBar->m_recentDockInfo.m_rectRecentFloatingRect.Width();
				m_rectDrag.bottom = m_rectDrag.top + pBar->m_recentDockInfo.m_rectRecentFloatingRect.Height();
			}

			if (!m_rectDrag.PtInRect(m_ptHot))
			{
				int nOffset = m_rectDrag.left - m_ptHot.x;
				m_rectDrag.OffsetRect(-nOffset - 5, 0); // offset of mouse pointer
				// from the drag rect bound
			}
		}
		bFirstTime = TRUE;
	}

	BOOL bDrawTab = FALSE;
	CDockablePane* pOldTargetBar = m_pTargetBar;
	CRect rectExpected; rectExpected.SetRectEmpty();

	CSmartDockingManager* pSDManager = NULL;
	BOOL bSDockingIsOn = FALSE;

	if (m_pDockManager != NULL &&(pSDManager = m_pDockManager->GetSmartDockingManagerPermanent()) != NULL && pSDManager->IsStarted())
	{
		bSDockingIsOn = TRUE;
	}

	m_pDockManager->CalcExpectedDockedRect(m_pDraggedWnd, ptMouse, rectExpected, bDrawTab, &m_pTargetBar);

	if (pOldTargetBar != NULL && m_nInsertedTabID != -1 && (pOldTargetBar != m_pTargetBar || !bDrawTab))
	{
		RemoveTabPreDocking(pOldTargetBar);
		bFirstTime = TRUE;
	}

	BOOL bCanBeAttached = TRUE;
	if (m_pDraggedWnd->IsKindOf(RUNTIME_CLASS(CPaneFrameWnd)))
	{
	}
	else if (m_pDraggedWnd->IsKindOf(RUNTIME_CLASS(CPane)))
	{
		CPane* pBar = DYNAMIC_DOWNCAST(CPane, m_pDraggedWnd);
		bCanBeAttached = pBar->CanBeAttached();
	}

	if (m_pTargetBar != NULL && bCanBeAttached)
	{
		CBaseTabbedPane* pTabbedBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, m_pTargetBar);
		if (pTabbedBar != NULL && bDrawTab &&
			(pTabbedBar->GetVisibleTabsNum() > 1 && pTabbedBar->IsHideSingleTab() || pTabbedBar->GetVisibleTabsNum() > 0 && !pTabbedBar->IsHideSingleTab()))
		{
			PlaceTabPreDocking(pTabbedBar, bFirstTime);
			return;
		}
		else if (bDrawTab)
		{
			if (m_nInsertedTabID != -1)
			{
				return;
			}
			if (!bFirstTime)
			{
				EndDrawDragFrame(FALSE);
			}
			DrawFrameTab(m_pTargetBar, FALSE);
			m_nInsertedTabID = 1;
			return;
		}
	}

	m_rectDrag.OffsetRect(ptOffset);
	m_ptHot = ptMouse;

	m_rectExpectedDocked = rectExpected;

	int nNewThickness = m_rectExpectedDocked.IsRectEmpty()? afxGlobalData.m_nDragFrameThicknessFloat : afxGlobalData.m_nDragFrameThicknessDock;

	CRect rectDocked;
	if (m_rectExpectedDocked.IsRectEmpty())
	{
		if (!m_rectDrag.PtInRect(ptMouse))
		{
			CPoint ptMiddleRect(m_rectDrag.TopLeft().x + m_rectDrag.Width() / 2, m_rectDrag.top + 5);

			CPoint ptOffsetMid = ptMouse - ptMiddleRect;
			m_rectDrag.OffsetRect(ptOffsetMid);
		}
		rectDocked = m_rectDrag;
	}
	else
	{
		rectDocked = m_rectExpectedDocked;
	}
	if (!bSDockingIsOn || !m_rectExpectedDocked.IsRectEmpty())
	{
		DrawDragFrame(rectOld, rectDocked, bFirstTime, nNewThickness, m_nOldThickness);
		m_nOldThickness = nNewThickness;
	}
}

void CMFCDragFrameImpl::DrawFrameTab(CDockablePane* pTargetBar, BOOL bErase)
{
	CRect rectWnd;
	pTargetBar->GetWindowRect(rectWnd);

	CSmartDockingManager* pSDManager = NULL;
	BOOL bSDockingIsOn = FALSE;

	if (m_pDockManager != NULL
		&&(pSDManager = m_pDockManager->GetSmartDockingManagerPermanent()) != NULL
		&& pSDManager->IsStarted())
	{
		bSDockingIsOn = TRUE;
	}

	int nThikness = afxGlobalData.m_nDragFrameThicknessDock;
	CRect rectSmallTab = rectWnd;
	// to be changed to tab height

	if (CTabbedPane::m_bTabsAlwaysTop)
	{
		rectWnd.top += afxGlobalData.GetTextHeight();
		rectSmallTab.bottom = rectSmallTab.top + afxGlobalData.GetTextHeight();
		rectSmallTab.left += 10;
		rectSmallTab.right = rectSmallTab.left + 40;
	}
	else
	{
		rectWnd.bottom -= afxGlobalData.GetTextHeight();
		rectSmallTab.top = rectSmallTab.bottom - afxGlobalData.GetTextHeight();
		rectSmallTab.left += 10;
		rectSmallTab.right = rectSmallTab.left + 40;
	}

	if (rectSmallTab.right >= rectWnd.right)
	{
		rectSmallTab.right = rectWnd.right - nThikness - 4;
	}

	CRect rectEmpty; rectEmpty.SetRectEmpty();

	CRect rectLine;
	if (CTabbedPane::m_bTabsAlwaysTop)
	{
		rectLine.SetRect(rectSmallTab.left + nThikness, rectSmallTab.bottom - nThikness, rectSmallTab.right - nThikness, rectSmallTab.bottom + nThikness);
	}
	else
	{
		rectLine.SetRect(rectSmallTab.left + nThikness, rectSmallTab.top  - nThikness, rectSmallTab.right - nThikness, rectSmallTab.top + nThikness);
	}

	if (bErase)
	{
		if (bSDockingIsOn)
		{
			pSDManager->HidePlace();
		}
		else
		{
			DrawDragFrame(rectEmpty, rectSmallTab, FALSE, nThikness, nThikness);
			DrawDragFrame(rectEmpty, rectWnd, FALSE, nThikness, nThikness);
			DrawDragFrame(rectEmpty, rectLine, FALSE, nThikness, nThikness);
			m_bFrameTabDrawn = FALSE;
		}
	}
	else
	{
		if (bSDockingIsOn)
		{
			pSDManager->ShowTabbedPlaceAt(&rectWnd, 10, rectSmallTab.Width(), rectSmallTab.Height());
		}
		else
		{
			DrawDragFrame(rectEmpty, rectSmallTab, TRUE, nThikness, nThikness);
			DrawDragFrame(rectEmpty, rectWnd, TRUE, nThikness, nThikness);
			DrawDragFrame(rectEmpty, rectLine, TRUE, nThikness, nThikness);
			m_bFrameTabDrawn = TRUE;
		}
	}
}

void CMFCDragFrameImpl::EndDrawDragFrame(BOOL bClearInternalRects)
{
	if (m_pDockManager == NULL)
	{
		return;
	}

	BOOL bSDockingIsOn = FALSE;
	CSmartDockingManager* pSDManager = NULL;

	if ((pSDManager = m_pDockManager->GetSmartDockingManagerPermanent()) != NULL && pSDManager->IsStarted())
	{
		bSDockingIsOn = TRUE;
		pSDManager->HidePlace();
	}

	CRect rectEmpty; rectEmpty.SetRectEmpty();
	CRect rectDocked = m_rectExpectedDocked.IsRectEmpty() ? m_rectDrag : m_rectExpectedDocked;

	// do not draw the final frame(meaning - clear) if it was not drawn because of tab
	if (m_nInsertedTabID == -1)
	{
		if (!bSDockingIsOn)
		{
			DrawDragFrame(rectEmpty, rectDocked, 0, m_nOldThickness);
		}
	}
	else
	{
		m_bDockToTab = TRUE;
	}

	if (bClearInternalRects)
	{
		RemoveTabPreDocking();

		m_rectExpectedDocked.SetRectEmpty();
		m_rectDrag.SetRectEmpty();

		m_pFinalTargetBar = m_pTargetBar;
		m_pTargetBar = NULL;
	}

	m_bDragStarted = FALSE;

	ENSURE(m_pDockManager != NULL);
	if (!bSDockingIsOn)
	{
		m_pDockManager->LockUpdate(FALSE);
	}
}

void CMFCDragFrameImpl::DrawDragFrame(LPCRECT lpRectOld, LPCRECT lpRectNew, BOOL bFirstTime, int nNewThickness, int nOldThikness)
{
	CWindowDC dc(m_pDraggedWnd->GetDesktopWindow());

	CSize szNewThickness(nNewThickness, nNewThickness);
	CSize szOldThickness(nOldThikness, nOldThikness);

	CSmartDockingManager* pSDManager = NULL;

	if (m_pDockManager != NULL &&(pSDManager = m_pDockManager->GetSmartDockingManagerPermanent()) != NULL && pSDManager->IsStarted())
	{
		pSDManager->ShowPlaceAt(lpRectNew);
	}
	else
	{
		if (bFirstTime)
		{
			dc.DrawDragRect(lpRectNew, szNewThickness, NULL, szOldThickness);
		}
		else
		{
			dc.DrawDragRect(lpRectNew, szNewThickness, lpRectOld, szOldThickness);
		}
	}
}

void CMFCDragFrameImpl::PlaceTabPreDocking(CBaseTabbedPane* pTabbedBar, BOOL bFirstTime)
{
	if (m_nInsertedTabID != -1)
	{
		return;
	}
	if (!bFirstTime)
	{
		EndDrawDragFrame(FALSE);
	}
	CString strLabel;
	if (m_pDraggedWnd->IsKindOf(RUNTIME_CLASS(CMultiPaneFrameWnd)))
	{
		CMultiPaneFrameWnd* pMultiMiniFrame = DYNAMIC_DOWNCAST(CMultiPaneFrameWnd, m_pDraggedWnd);
		if (pMultiMiniFrame != NULL)
		{
			CWnd* pBar = pMultiMiniFrame->GetFirstVisiblePane();
			ASSERT_VALID(pBar);

			if (pBar != NULL)
			{
				pBar->GetWindowText(strLabel);
			}
		}
	}
	else
	{
		m_pDraggedWnd->GetWindowText(strLabel);
	}

	if (m_pWndDummy == NULL)
	{
		m_pWndDummy = new CDummyDockablePane;
		m_pWndDummy->CreateEx(0, _T(""), AFXGetTopLevelFrame(m_pDraggedWnd), CRect(0, 0, 0, 0), FALSE, AFX_DUMMY_WND_ID, WS_CHILD);
	}

	pTabbedBar->GetUnderlyingWindow()->AddTab(m_pWndDummy, strLabel);

	CSmartDockingManager* pSDManager = NULL;
	if ((pSDManager = m_pDockManager->GetSmartDockingManagerPermanent()) != NULL && pSDManager->IsStarted())
	{
		m_pDraggedWnd->ShowWindow(SW_HIDE);
	}

	m_nInsertedTabID = pTabbedBar->GetUnderlyingWindow()->GetTabFromHwnd(*m_pWndDummy);
	m_pOldTargetBar = pTabbedBar;
}

void CMFCDragFrameImpl::PlaceTabPreDocking(CWnd* pCBarToPlaceOn)
{
	CBaseTabbedPane* pTabbedBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, pCBarToPlaceOn);
	if (pTabbedBar != NULL &&
		(pTabbedBar->GetVisibleTabsNum() > 1 && pTabbedBar->IsHideSingleTab() || pTabbedBar->GetVisibleTabsNum() > 0 && !pTabbedBar->IsHideSingleTab()))
	{
		m_pTargetBar = pTabbedBar;
		PlaceTabPreDocking(pTabbedBar, TRUE);
		return;
	}
	else if (m_nInsertedTabID == -1)
	{
		CDockablePane* pDockingControlBar = DYNAMIC_DOWNCAST(CDockablePane, pCBarToPlaceOn);
		if (pDockingControlBar != NULL)
		{
			DrawFrameTab(pDockingControlBar, FALSE);
			m_pTargetBar = pDockingControlBar;
			m_pOldTargetBar = pDockingControlBar;
			m_nInsertedTabID = 1;
		}
	}
}

void CMFCDragFrameImpl::RemoveTabPreDocking(CDockablePane* pOldTargetBar)
{
	if (pOldTargetBar == NULL)
	{
		pOldTargetBar = m_pOldTargetBar;
	}

	if (pOldTargetBar != NULL && m_nInsertedTabID != -1)
	{
		CBaseTabbedPane* pOldTabbedBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, pOldTargetBar);
		if (pOldTabbedBar != NULL && !m_bFrameTabDrawn && m_pWndDummy != NULL && m_pWndDummy->GetSafeHwnd() != NULL)
		{
			CSmartDockingManager* pSDManager = NULL;
			BOOL bSDockingIsOn = FALSE;

			if (m_pDockManager != NULL &&(pSDManager = m_pDockManager->GetSmartDockingManagerPermanent()) != NULL && pSDManager->IsStarted())
			{
				bSDockingIsOn = TRUE;
			}

			m_pWndDummy->ShowWindow(SW_HIDE);
			if (!bSDockingIsOn)
			{
				m_pDockManager->LockUpdate(FALSE);
			}
			CWnd* pWnd = pOldTabbedBar->GetUnderlyingWindow()->GetTabWnd(m_nInsertedTabID);
			if (pWnd == m_pWndDummy)
			{
				pOldTabbedBar->GetUnderlyingWindow()->RemoveTab(m_nInsertedTabID);
			}
			if (!bSDockingIsOn)
			{
				m_pDockManager->LockUpdate(TRUE);
			}
		}
		else
		{
			DrawFrameTab(pOldTargetBar, TRUE);
		}

		CSmartDockingManager* pSDManager = NULL;

		if ((pSDManager = m_pDockManager->GetSmartDockingManagerPermanent()) != NULL && pSDManager->IsStarted())
		{
			m_pDraggedWnd->ShowWindow(SW_SHOW);
		}
	}

	m_nInsertedTabID = -1;
	m_pOldTargetBar = NULL;
}

void CMFCDragFrameImpl::ResetState()
{
	m_ptHot = CPoint(-1, -1);
	m_rectDrag.SetRectEmpty();
	m_rectExpectedDocked.SetRectEmpty();

	m_pFinalTargetBar = NULL;
	m_pOldTargetBar   = NULL;
	m_bDockToTab	  = FALSE;
	m_bDragStarted    = FALSE;

	m_nInsertedTabID  = -1;
}



