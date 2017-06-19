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

#include "afxglobals.h"
#include "afxdocksite.h"
#include "afxdockingpanesrow.h"
#include "afxpaneframewnd.h"

#include "afxpane.h"
#include "afxtabctrl.h"
#include "afxtabbedpane.h"
#include "afxdockablepaneadapter.h"

#include "afxdockingmanager.h"
#include "afxglobalutils.h"

#include "afxolecntrframewndex.h"

#include "afxregpath.h"
#include "afxsettingsstore.h"
#include "afxrebar.h"

#include "afxcontextmenumanager.h"

#include "afxribbonres.h"
#include "afxpopupmenu.h"

#include "afxmdiframewndex.h"
#include "afxmdichildwndex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const CString strControlBarProfile = _T("Panes");

int CPane::m_bHandleMinSize = FALSE;

#define AFX_REG_SECTION_FMT _T("%sPane-%d")
#define AFX_REG_SECTION_FMT_EX _T("%sPane-%d%x")

IMPLEMENT_DYNCREATE(CPane, CBasePane)

/////////////////////////////////////////////////////////////////////////////
// CPane

#pragma warning(disable : 4355)

CPane::CPane() :
m_bCaptured(false), m_nID(0), m_bDisableMove(false), m_recentDockInfo(this)
{
	m_cxLeftBorder = m_cxRightBorder = 6;
	m_cxDefaultGap = 2;
	m_cyTopBorder = m_cyBottomBorder = 2;
	m_pData = NULL;
	m_nCount = 0;
	m_nMRUWidth = 32767;
	m_bDblClick = false;

	m_ptClientHotSpot.x = m_ptClientHotSpot.y = 0;

	m_rectSavedDockedRect.SetRectEmpty();

	m_bDragMode = FALSE;
	m_bWasFloatingBeforeMove = FALSE;
	m_bWasFloatingBeforeTabbed = FALSE;
	m_bRecentFloatingState = FALSE;

	m_pMiniFrameRTC = RUNTIME_CLASS(CPaneFrameWnd);
	m_rectDragImmediate.SetRectEmpty();

	m_hwndMiniFrameToBeClosed = NULL;

	m_bFirstInGroup = TRUE;
	m_bLastInGroup = TRUE;
	m_bActiveInGroup = TRUE;
	m_bExclusiveRow = FALSE;

	m_bPinState = FALSE;

	m_sizeMin.cx = m_sizeMin.cy = 1;
}

#pragma warning(default : 4355)

CPane::~CPane()
{
	// free array
	if (m_pData != NULL)
	{
		ASSERT(m_nCount != 0);
		free(m_pData);
	}
}

BEGIN_MESSAGE_MAP(CPane, CBasePane)
	//{{AFX_MSG_MAP(CPane)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_NCDESTROY()
	ON_WM_CONTEXTMENU()
	ON_WM_CANCELMODE()
	ON_WM_CHAR()
	ON_WM_DESTROY()
	ON_WM_STYLECHANGED()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPane message handlers

BOOL CPane::Create(LPCTSTR lpszClassName, DWORD dwStyle, const RECT& rect,
	CWnd* pParentWnd, UINT nID, DWORD dwControlBarStyle, CCreateContext* pContext)
{
	return CPane::CreateEx(0, lpszClassName, dwStyle, rect, pParentWnd, nID, dwControlBarStyle, pContext);
}
//
BOOL CPane::CreateEx(DWORD dwStyleEx, LPCTSTR lpszClassName, DWORD dwStyle, const RECT& rect,
	CWnd* pParentWnd, UINT nID, DWORD dwControlBarStyle, CCreateContext* pContext)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pParentWnd);

	CString strClassName;

	if (lpszClassName == NULL)
	{
		strClassName = afxGlobalData.RegisterWindowClass(_T("Afx:ControlBar"));
	}
	else
	{
		strClassName = lpszClassName;
	}

	m_nID = nID;

	dwStyle |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	if (!CBasePane::CreateEx(dwStyleEx, strClassName, NULL, dwStyle, rect, pParentWnd, nID, dwControlBarStyle, pContext))
	{
		return FALSE;
	}

	CRect rectInit = rect;
	pParentWnd->ClientToScreen(rectInit);

	if (m_recentDockInfo.m_recentMiniFrameInfo.m_rectDockedRect.IsRectEmpty())
	{
		m_recentDockInfo.m_recentMiniFrameInfo.m_rectDockedRect = rectInit;
	}

	if (m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect.IsRectEmpty())
	{
		m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect = rectInit;
	}

	if (!rectInit.IsRectEmpty())
	{
		m_recentDockInfo.m_rectRecentFloatingRect = rectInit;
	}

	SetOwner(pParentWnd);
	UpdateVirtualRect();

	if (m_dwControlBarStyle & CanFloat())
	{
		m_dragFrameImpl.Init(this);
	}

	return TRUE;
}

void CPane::SetBorders(int cxLeft, int cyTop, int cxRight, int cyBottom)
{
	m_cxLeftBorder = cxLeft;
	m_cxRightBorder = cxRight;
	m_cyTopBorder = cyTop;
	m_cyBottomBorder = cyBottom;
}

void CPane::SetBorders(LPCRECT lpRect)
{
	m_cxLeftBorder = lpRect->left;
	m_cxRightBorder = lpRect->right;
	m_cyTopBorder = lpRect->top;
	m_cyBottomBorder = lpRect->bottom;
}

CRect CPane::GetBorders() const
{
	CRect rect(m_cxLeftBorder, m_cyTopBorder, m_cxRightBorder, m_cyBottomBorder);
	return rect;
}

void CPane::OnLButtonDown(UINT nFlags, CPoint point)
{
	ASSERT_VALID(this);

	if (!m_bCaptured && CanFloat())
	{
		CPaneFrameWnd* pMiniFrame = GetParentMiniFrame();

		if ((GetDockingMode() & DT_IMMEDIATE) != 0 || pMiniFrame != NULL)
		{
			StoreRecentDockSiteInfo();
			if (pMiniFrame == NULL)
			{
				EnterDragMode(TRUE);
				m_bWasFloatingBeforeMove = FALSE;
			}
			else if (pMiniFrame != NULL)
			{
				ASSERT_VALID(pMiniFrame);
				// it's on the miniframe - reflect message to the miniframe if
				// this bar is alone on the miniframe
				if (pMiniFrame->GetPaneCount() == 1)
				{
					MapWindowPoints(pMiniFrame, &point, 1);
					pMiniFrame->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELPARAM(point.x, point.y));
					m_bWasFloatingBeforeMove = TRUE;
				}
				else
				{
					EnterDragMode(TRUE);
					m_bWasFloatingBeforeMove = FALSE;
				}
				return;
			}
		}
		else if ((GetDockingMode() & DT_STANDARD) != 0)
		{
			EnterDragMode(TRUE);
		}
	}

	CWnd::OnLButtonDown(nFlags, point);
}

// EnterDragMode is called from OnLButtonDown and OnContinueMoving - when docked
// in the last case we dont need to save recent floating info and client hot spot
void CPane::EnterDragMode(BOOL bChangeHotPoint)
{
	ASSERT_VALID(this);

	CPoint ptCursorPos;
	GetCursorPos(&ptCursorPos);

	// fixup the current virtual rectangle when mouse btn is down - to be
	// prepared to move THIS control bar
	UpdateVirtualRect();

	if (bChangeHotPoint)
	{
		m_ptClientHotSpot = ptCursorPos;
		ScreenToClient(&m_ptClientHotSpot);
	}

	if (!m_bCaptured && IsDocked())
	{
		SetCapture();

		m_bCaptured = true;
		m_dragFrameImpl.m_ptHot = ptCursorPos;

		SetDragMode(TRUE);

		GetWindowRect(m_rectDragImmediate);
	}
}

void CPane::StoreRecentDockSiteInfo()
{
	m_recentDockInfo.m_pRecentDockBarRow = m_pDockBarRow;
	m_recentDockInfo.m_pRecentDockBar = m_pParentDockBar;

	if (m_recentDockInfo.m_pRecentDockBar != NULL)
	{
		m_recentDockInfo.m_nRecentRowIndex =
			m_recentDockInfo.m_pRecentDockBar->FindRowIndex(m_pDockBarRow);
	}

	CalcRecentDockedRect();
}

void CPane::OnLButtonUp(UINT nFlags, CPoint point)
{
	ASSERT_VALID(this);

	CPaneFrameWnd* pMiniFrame = GetParentMiniFrame();

	if (m_bCaptured)
	{
		ReleaseCapture();
		m_bCaptured = false;

		if (nFlags != 0xFFFF)
		{
			if (m_hwndMiniFrameToBeClosed != NULL && ::IsWindow(m_hwndMiniFrameToBeClosed))
			{
				::DestroyWindow(m_hwndMiniFrameToBeClosed);
			}

			m_hwndMiniFrameToBeClosed = NULL;
		}

		SetDragMode(FALSE);

		CDockingManager* pDockManager = afxGlobalUtils.GetDockingManager(GetParent());
		if (pDockManager != NULL)
		{
			pDockManager->StopSDocking();
		}

		if ((GetDockingMode() & DT_STANDARD) != 0 &&(m_dragFrameImpl.m_bDragStarted || m_dragFrameImpl.m_nInsertedTabID >= 0))
		{
			CRect rectFinal = m_dragFrameImpl.m_rectDrag;
			if (m_dragFrameImpl.m_bDragStarted &&(GetDockingMode() & DT_STANDARD) != 0)
			{
				m_dragFrameImpl.EndDrawDragFrame();
			}
			BOOL bWasDocked = FALSE;
			StoreRecentDockSiteInfo();
			CPane* pTargetBar = DockPaneStandard(bWasDocked);

			if (!bWasDocked)
			{
				if (!rectFinal.IsRectEmpty() && pTargetBar != this)
				{
					FloatPane(rectFinal, DM_STANDARD);
				}
			}

			return;
		}
	}
	else if (pMiniFrame != NULL && !m_bDblClick && pMiniFrame->IsWindowVisible())
	{
		// it's attached to the miniframe - reflect message to the miniframe
		ASSERT_VALID(pMiniFrame);
		MapWindowPoints(pMiniFrame, &point, 1);
		pMiniFrame->SendMessage(WM_LBUTTONUP, nFlags, MAKELPARAM(point.x, point.y));
		return;
	}
	m_bDblClick = false;

	if (m_pDockBarRow != NULL)
	{
		m_pDockBarRow->FixupVirtualRects(false);
	}

	CWnd::OnLButtonUp(nFlags, point);
}

void CPane::OnMouseMove(UINT nFlags, CPoint point)
{
	ASSERT_VALID(this);
	// the control bar is moved when it resides on the dock bar(docked)
	if (m_bCaptured)
	{
		AFX_DOCK_TYPE docktype = GetDockingMode();

		if ((docktype & DT_IMMEDIATE) != 0)
		{
			CPoint ptMouse;
			GetCursorPos(&ptMouse);

			CPoint ptOffset = ptMouse - m_dragFrameImpl.m_ptHot;
			m_rectDragImmediate.OffsetRect(ptOffset);

			UpdateVirtualRect(ptOffset);

			if (m_pParentDockBar != NULL)
			{
				m_pParentDockBar->MovePane(this, nFlags, ptOffset);
				RedrawWindow();
			}

			m_dragFrameImpl.m_ptHot = ptMouse;
		}
		else if ((docktype & DT_STANDARD) != 0)
		{
			m_dragFrameImpl.MoveDragFrame();
		}
	}
	else
	{
		// it should be moved(if captured) along with the mini fraeme
		CWnd::OnMouseMove(nFlags, point);
	}
}

void CPane::RecalcLayout()
{
	ASSERT_VALID(this);

	if (m_pDockBarRow != NULL)
	{
		UpdateVirtualRect();
	}
	else
	{
		CPaneFrameWnd* pMiniFrame = GetParentMiniFrame();
		CWnd* pParent = GetParent();

		if (pMiniFrame != NULL && !pParent->IsKindOf(RUNTIME_CLASS(CMFCTabCtrl)))
		{
			pMiniFrame->OnPaneRecalcLayout();
		}
	}
}

BOOL CPane::DockByMouse(CBasePane* pDockBar)
{
	ASSERT_VALID(this);

	if (!OnBeforeDock(&pDockBar, NULL, DM_MOUSE))
	{
		return FALSE;
	}

	if (Dock(pDockBar, NULL, DM_MOUSE))
	{
		OnAfterDock(pDockBar, NULL, DM_MOUSE);
		return TRUE;
	}

	return FALSE;
}

BOOL CPane::DockPane(CBasePane* pDockBar, LPCRECT lpRect, AFX_DOCK_METHOD dockMethod)
{
	ASSERT_VALID(this);
	if (!OnBeforeDock(&pDockBar, lpRect, dockMethod))
	{
		return FALSE;
	}

	if (Dock(pDockBar, lpRect, dockMethod))
	{
		OnAfterDock(pDockBar, lpRect, dockMethod);
		return TRUE;
	}
	return FALSE;
}

BOOL CPane::Dock(CBasePane* pDockBar, LPCRECT lpRect, AFX_DOCK_METHOD dockMethod)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDockBar);

	if (dockMethod == DM_DBL_CLICK)
	{
		pDockBar = m_recentDockInfo.m_pRecentDockBar;

		if (pDockBar == NULL)
		{
			CDockingManager* pDockManager = afxGlobalUtils.GetDockingManager(GetDockSiteFrameWnd());
			ASSERT_VALID(pDockManager);
			pDockManager->DockPane(this);
			return TRUE;
		}
	}

	ASSERT_KINDOF(CDockSite, pDockBar);

	// check whether the control bar can be docked at the given DockPane
	if (!CanBeDocked(pDockBar) || !pDockBar->CanAcceptPane(this))
	{
		return FALSE;
	}

	// save the window rectandle of the control bar, because it will be adjusted in the
	// moment when the parent is changed
	CRect rect;
	rect.SetRectEmpty();
	GetWindowRect(&rect);

	BOOL bWasCaptured = TRUE;
	CPaneFrameWnd* pMiniFrame = GetParentMiniFrame();
	if (pMiniFrame != NULL)
	{
		bWasCaptured = pMiniFrame->IsCaptured();
	}

	if (pDockBar != NULL)
	{
		pDockBar->SetRedraw (FALSE);
	}

	PrepareToDock((CDockSite*)pDockBar, dockMethod);

	if (dockMethod == DM_MOUSE)
	{
		m_pParentDockBar->DockPane(this, dockMethod, &rect);
		if (bWasCaptured)
		{
			OnContinueMoving();
		}
	}
	else if (dockMethod == DM_RECT || dockMethod == DM_DBL_CLICK)
	{
		m_pParentDockBar->DockPane(this, dockMethod, lpRect);
	}

	if (pDockBar != NULL)
	{
		pDockBar->SetRedraw (TRUE);
	}

	return TRUE;
}

void CPane::PrepareToDock(CDockSite* pDockBar, AFX_DOCK_METHOD dockMethod)
{
	if (pDockBar != NULL)
	{
		m_pParentDockBar = DYNAMIC_DOWNCAST(CDockSite, pDockBar);
		ASSERT_VALID(m_pParentDockBar);
		// remove the control bar from its miniframe
		RemoveFromMiniframe(pDockBar, dockMethod);

		// align correctly and turn on all borders
		DWORD dwStyle = GetPaneStyle();
		dwStyle &= ~(CBRS_ALIGN_ANY);
		dwStyle |= (m_dwStyle & CBRS_ALIGN_ANY) | CBRS_BORDER_ANY;

		dwStyle &= ~CBRS_FLOATING;
		SetPaneStyle(dwStyle);

		SetPaneAlignment(pDockBar->GetCurrentAlignment());
	}
}

void CPane::RemoveFromMiniframe(CWnd* pNewParent, AFX_DOCK_METHOD dockMethod)
{
	ASSERT_VALID(this);

	CPaneFrameWnd* pMiniFrame = GetParentMiniFrame();

	CWnd* pOldParent = GetParent();
	// reassign the parentship to the new parent
	OnBeforeChangeParent(pNewParent);

	// miniframe will be NULL if the bar has never been floating
	if (pMiniFrame != NULL)
	{
		// remove the control bar from its miniframe
		// DO NOT destroy miniframe meanwhile

		// delay destroying of miniframes only in case if it's the first miniframe after
		// the control bar has been captured

		// dockMethod == DM_DBL_CLICK - would be required to prevent crash while canceling
		// drag from tab window to main frame edge - see support from 30/06/2004,
		// but currently can't be reproduced. Left here until testing has been finished
		BOOL bDelayDestroy = ((dockMethod == DM_MOUSE /*|| dockMethod == DM_DBL_CLICK*/) && m_hwndMiniFrameToBeClosed == NULL);

		pMiniFrame->RemovePane(this, FALSE, bDelayDestroy);
		if ((dockMethod == DM_MOUSE /*|| dockMethod == DM_DBL_CLICK*/) && m_hwndMiniFrameToBeClosed == NULL /*&& pMiniFrame->GetPaneCount() == 0*/)
		{
			m_hwndMiniFrameToBeClosed = pMiniFrame->GetSafeHwnd();
		}
		if (dockMethod == DM_MOUSE)
		{
			pMiniFrame->SendMessage(WM_LBUTTONUP, 0, 0);
		}
	}

	if (pNewParent != NULL)
	{
		SetParent(pNewParent);
	}
	OnAfterChangeParent(pOldParent);
}

BOOL CPane::OnBeforeDock(CBasePane** /*ppDockBar*/, LPCRECT /*lpRect*/, AFX_DOCK_METHOD /*dockMethod*/)
{
	ASSERT_VALID(this);
	CPaneFrameWnd* pParentMiniFrame = GetParentMiniFrame();
	if (pParentMiniFrame != NULL)
	{
		m_bPinState = pParentMiniFrame->GetPinState();
	}
	return TRUE;
}

BOOL CPane::OnBeforeFloat(CRect& /*rectFloat*/, AFX_DOCK_METHOD /*dockMethod*/)
{
	ASSERT_VALID(this);
	return TRUE;
}

void CPane::OnAfterFloat()
{
	ASSERT_VALID(this);
	SetPaneAlignment(CBRS_ALIGN_TOP);
	CPaneFrameWnd* pParentMiniFrame = GetParentMiniFrame();
	if (pParentMiniFrame != NULL)
	{
		pParentMiniFrame->Pin(m_bPinState);
		pParentMiniFrame->SetWindowPos(NULL, -1, -1, -1, -1, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}

	if (CPane::m_bHandleMinSize)
	{
		SetWindowRgn(NULL, TRUE);
	}
}

void CPane::OnContinueMoving()
{
	ASSERT_VALID(this);
	// continue moving
	EnterDragMode(FALSE);
}

BOOL CPane::FloatPane(CRect rectFloat, AFX_DOCK_METHOD dockMethod, bool bShow)
{
	ASSERT_VALID(this);
	if (!IsDocked() && !IsTabbed())
	{
		return TRUE;
	}

	if (!CanFloat())
	{
		return TRUE;
	}

	CRect rectBeforeFloat;
	GetWindowRect(rectBeforeFloat);

	CWnd* pDockSite = GetDockSiteFrameWnd();
	ASSERT_VALID(pDockSite);

	pDockSite->ScreenToClient(rectBeforeFloat);

	CPoint ptMouseScreen; //mouse coords
	GetCursorPos(&ptMouseScreen);

	CPoint ptScreen = m_ptClientHotSpot;
	ClientToScreen(&ptScreen);

	if (!OnBeforeFloat(rectFloat, dockMethod))
	{
		return TRUE;
	}

	CRect rectDelta(16, 16, 16, 16);
	afxGlobalUtils.AdjustRectToWorkArea(rectFloat, &rectDelta);

	// create miniframe if it does not exist and move it if it does exist
	CPaneFrameWnd* pParentMiniFrame = CreateDefaultMiniframe(rectFloat);
	if (pParentMiniFrame == NULL)
	{
		return FALSE;
	}

	CDockingManager* pDockManager = afxGlobalUtils.GetDockingManager(GetDockSiteFrameWnd());
	if (dockMethod != DM_MOUSE && pDockManager != NULL && !pDockManager->m_bRestoringDockState)
	{
		StoreRecentDockSiteInfo();
	}

	// this rectangle does not take into account miniframe caption height and borders
	pParentMiniFrame->m_rectRecentFloatingRect = m_recentDockInfo.m_rectRecentFloatingRect;

	OnBeforeChangeParent(pParentMiniFrame);

	CPoint ptMouseClient = ptMouseScreen;
	ScreenToClient(&ptMouseClient);

	if (dockMethod == DM_MOUSE)
	{
		SendMessage(WM_LBUTTONUP, 0xFFFF, MAKELPARAM(ptMouseClient.x, ptMouseClient.y));
		if (IsTabbed())
		{
			CPaneFrameWnd* pWnd = GetParentMiniFrame();
			if (pWnd != NULL)
			{
				pWnd->SendMessage(WM_LBUTTONUP, 0, MAKELPARAM(ptMouseClient.x, ptMouseClient.y));
			}
		}
	}

	CWnd* pOldParent = GetParent();
	SetParent(pParentMiniFrame);
	if (m_pParentDockBar != NULL)
	{
		OnAfterChangeParent(m_pParentDockBar);
		m_pParentDockBar = NULL;
	}
	else
	{
		OnAfterChangeParent(pOldParent);
	}

	pParentMiniFrame->AddPane(this);

	//move control bar to the top left corner of the miniframe
	pParentMiniFrame->CheckGripperVisibility();
	SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);

	if (dockMethod == DM_MOUSE &&(GetDockingMode() & DT_IMMEDIATE) != 0)
	{
		pParentMiniFrame->EnterDragMode();
		// block the first MouseMove to prevent immediate docking
		pParentMiniFrame->m_bBlockMove = true;
	}

	OnAfterFloat();

	DWORD dwStyle = GetPaneStyle();
	dwStyle |= CBRS_FLOATING;
	SetPaneStyle(dwStyle);

	RecalcLayout();

	if (bShow)
	{
		GetParentMiniFrame()->AdjustLayout();
	}

	// move the default miniframe so the client hot spot will be on place
	// move the default miniframe so the client hot spot will be on place
	if (dockMethod == DM_MOUSE)
	{
		CRect rectFinalMiniFrame;
		pParentMiniFrame->GetWindowRect(rectFinalMiniFrame);

		ptScreen = m_ptClientHotSpot;

		pParentMiniFrame->ClientToScreen(&ptScreen);

		if (ptScreen.x > rectFinalMiniFrame.left + rectFinalMiniFrame.Width() || ptScreen.x < rectFinalMiniFrame.left)
		{
			ptScreen.x = rectFinalMiniFrame.left + rectFinalMiniFrame.Width() / 2;
		}

		if (ptScreen.y > rectFinalMiniFrame.top + rectFinalMiniFrame.Height() || ptScreen.y < rectFinalMiniFrame.top)
		{
			ptScreen.y = rectFinalMiniFrame.top + pParentMiniFrame->GetCaptionHeight() / 2;
		}

		CPoint ptOffset = ptMouseScreen - ptScreen;

		rectFinalMiniFrame.OffsetRect(ptOffset);

		pParentMiniFrame->SetWindowPos(NULL, rectFinalMiniFrame.left, rectFinalMiniFrame.top,
			rectFinalMiniFrame.Width(), rectFinalMiniFrame.Height(), SWP_NOZORDER | SWP_NOACTIVATE);

		pParentMiniFrame->SetHotPoint(ptMouseScreen);
	}

	if (bShow)
	{
		pParentMiniFrame->ShowWindow(SW_SHOWNA);
		GetDockSiteFrameWnd()->RedrawWindow(&rectBeforeFloat, NULL, RDW_FRAME | RDW_INVALIDATE |
			RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_NOINTERNALPAINT | RDW_NOERASE);
		if (GetDockSiteFrameWnd()->IsKindOf(RUNTIME_CLASS(COleCntrFrameWndEx)))
		{
			((COleCntrFrameWndEx*) GetDockSiteFrameWnd())->AdjustDockingLayout();
		}

		if (CanFocus())
		{
			pParentMiniFrame->SetFocus();
		}
	}

	return TRUE;
}

void CPane::OnBeforeChangeParent(CWnd* pWndNewParent, BOOL bDelay)
{
	ASSERT_VALID(this);
	if (m_pParentDockBar != NULL)
	{
		m_pParentDockBar->RemovePane(this, DM_UNKNOWN);
	}

	CBasePane::OnBeforeChangeParent(pWndNewParent, bDelay);
}

void CPane::OnAfterChangeParent(CWnd* pWndOldParent)
{
	ASSERT_VALID(this);
	UpdateVirtualRect();
	if (!GetParent()->IsKindOf(RUNTIME_CLASS(CDockSite)))
	{
		m_pParentDockBar = NULL;
		m_pDockBarRow = NULL;
	}
	CBasePane::OnAfterChangeParent(pWndOldParent);
}

BOOL CPane::MoveByAlignment(DWORD dwAlignment, int nOffset)
{
	ASSERT_VALID(this);

	CRect rect;
	GetWindowRect(rect);

	CWnd* pParentWnd = GetParent();

	ASSERT_VALID(pParentWnd);
	pParentWnd->ScreenToClient(&rect);

	switch(dwAlignment & CBRS_ALIGN_ANY)
	{
	case CBRS_ALIGN_LEFT:
	case CBRS_ALIGN_RIGHT:
		rect.OffsetRect(nOffset, 0);
		UpdateVirtualRect(CPoint(nOffset, 0));
		break;

	case CBRS_ALIGN_TOP:
	case CBRS_ALIGN_BOTTOM:
		rect.OffsetRect(0, nOffset);
		UpdateVirtualRect(CPoint(0, nOffset));
		break;
	}

	return(BOOL)(SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE) != 0);
}

CSize CPane::MovePane(CRect rectNew, BOOL bForceMove, HDWP& /*hdwp*/)
{
	ASSERT_VALID(this);

	CSize sizeMin;
	CSize sizeNew = rectNew.Size();

	GetMinSize(sizeMin);

	CRect rectCurrent;
	GetWindowRect(rectCurrent);

	CSize sizeActual = rectNew.Size() - rectCurrent.Size();

	if (!bForceMove && abs(sizeNew.cx) < sizeMin.cx)
	{
		if (rectCurrent.left == rectNew.left || rectCurrent.left != rectNew.left && rectCurrent.right != rectNew.right)
		{
			rectNew.right = rectCurrent.left + sizeMin.cx;
		}
		else if (rectCurrent.right == rectNew.right)
		{
			rectNew.left = rectCurrent.right - sizeMin.cx;
		}
		sizeActual.cx = rectCurrent.Width() - rectNew.Width();
	}

	if (!bForceMove && abs(sizeNew.cy) < sizeMin.cy)
	{
		if (rectCurrent.top == rectNew.top || rectCurrent.top != rectNew.top && rectCurrent.bottom != rectNew.bottom)
		{
			rectNew.bottom = rectCurrent.top + sizeMin.cy;
		}
		else if (rectCurrent.bottom == rectNew.bottom)
		{
			rectNew.top = rectCurrent.bottom - sizeMin.cy;
		}

		sizeActual.cy = rectCurrent.Height() - rectNew.Height();
	}

	ASSERT_VALID(GetParent());
	GetParent()->ScreenToClient(rectNew);
	MoveWindow(rectNew);
	return sizeActual;
}

int CPane::StretchPaneDeferWndPos(int nStretchSize, HDWP& /*hdwp*/)
{
	ASSERT_VALID(this);

	// the bar is stretched - calculate how far it can be expanded and do not
	// exceed its original size
	int nAvailExpandSize = GetAvailableExpandSize();
	int nAvailStretchSize = GetAvailableStretchSize();

	int nActualStretchSize = 0;
	if (nStretchSize > 0)
	{
		if (nAvailExpandSize == 0)
		{
			return 0;
		}
		// the bar is expanded
		nActualStretchSize = nAvailExpandSize > nStretchSize ? nStretchSize : nAvailExpandSize;
	}
	else
	{
		nActualStretchSize = nAvailStretchSize < abs(nStretchSize) ? -nAvailStretchSize : nStretchSize;
	}


	CRect rect;
	GetWindowRect(rect);

	if (IsHorizontal())
	{
		rect.right += nActualStretchSize;
	}
	else
	{
		rect.bottom += nActualStretchSize;
	}

	OnBeforeStretch(nActualStretchSize);

	if (abs(nActualStretchSize) > 0)
	{
		ASSERT_VALID(GetParent());
		GetParent()->ScreenToClient(rect);
		MoveWindow(rect);
		OnAfterStretch(nActualStretchSize);
	}

	return nActualStretchSize;
}

int CPane::GetAvailableExpandSize() const
{
	ASSERT_VALID(this);

	CRect rect;
	GetWindowRect(rect);

	// can't expand beyond virtual rect
	if ((IsHorizontal() && rect.Width() >= m_rectVirtual.Width() || !IsHorizontal() && rect.Height() >= m_rectVirtual.Height()))
	{
		return 0;
	}

	return IsHorizontal() ? m_rectVirtual.Width() - rect.Width() : m_rectVirtual.Height() - rect.Height();
}

int CPane::GetAvailableStretchSize() const
{
	ASSERT_VALID(this);

	CRect rect;
	GetWindowRect(rect);

	CSize sizeMin;
	GetMinSize(sizeMin);

	return IsHorizontal() ? rect.Width() - sizeMin.cx: rect.Height() - sizeMin.cy;
}

CSize CPane::CalcAvailableSize(CRect rectRequired)
{
	ASSERT_VALID(this);

	CSize sizeMin;
	GetMinSize(sizeMin);

	CSize sizeAvailable(0, 0);

	if (rectRequired.Width() < sizeMin.cx)
	{
		rectRequired.right = rectRequired.left + sizeMin.cx;
	}

	if (rectRequired.Height() < sizeMin.cy)
	{
		rectRequired.bottom = rectRequired.top + sizeMin.cy;
	}

	CRect rectCurrent;
	GetWindowRect(rectCurrent);

	// available space is negative when stretching
	sizeAvailable.cx = rectRequired.Width() - rectCurrent.Width();
	sizeAvailable.cy = rectRequired.Height() - rectCurrent.Height();

	return sizeAvailable;
}

bool CPane::IsLeftOf(CRect rect, bool bWindowRect) const
{
	ASSERT_VALID(this);
	if (m_pParentDockBar == NULL)
	{
		return true;
	}

	CRect rectBar;
	GetWindowRect(&rectBar);

	if (!bWindowRect)
	{
		m_pParentDockBar->ScreenToClient(&rectBar);
	}

	if (m_pParentDockBar->IsHorizontal())
	{
		return(rect.left < rectBar.left);
	}
	else
	{
		return(rect.top < rectBar.top);
	}
}

bool CPane::IsLastPaneOnLastRow() const
{
	ASSERT_VALID(this);
	if (m_pParentDockBar->IsLastRow(m_pDockBarRow))
	{
		return(m_pDockBarRow->GetPaneCount() == 1);
	}
	return false;
}

AFX_CS_STATUS CPane::IsChangeState(int nOffset, CBasePane** ppTargetBar) const
{
	ASSERT_VALID(this);
	ENSURE(ppTargetBar != NULL);

	CPoint ptMousePos;
	CRect rectBarWnd;
	CRect rectDockBarWnd;
	CRect rectIntersect;

	CRect rectVirtual;

	CPoint  ptDelta;

	GetCursorPos(&ptMousePos);

	GetWindowRect(&rectBarWnd);
	GetVirtualRect(rectVirtual);

	// check whether the mouse is around a dock bar
	CBasePane* pBaseBar = PaneFromPoint(ptMousePos, nOffset, FALSE, RUNTIME_CLASS(CDockSite));

	*ppTargetBar = DYNAMIC_DOWNCAST(CDockSite, pBaseBar);

	if (m_pParentDockBar != NULL)
	{
		// the mouse is around the dock bar, check the virtual rect
		m_pParentDockBar->GetWindowRect(&rectDockBarWnd);
		if (!rectIntersect.IntersectRect(rectDockBarWnd, rectVirtual))
		{
			return CS_DOCK_IMMEDIATELY;
		}

		// there is some intersection of the virtual rectangle an the dock bar.
		// special processing when horizontal bar is about to float in horizontal direction
		bool bTreatMouse = false;
		if (m_pParentDockBar->IsHorizontal())
		{
			if (rectVirtual.left < rectDockBarWnd.left && rectDockBarWnd.left - rectVirtual.left > nOffset * 2 ||
				rectVirtual.right > rectDockBarWnd.right && rectVirtual.right - rectDockBarWnd.right > nOffset * 2)
			{
				bTreatMouse = true;
			}
		}
		else
		{
			if (rectVirtual.top < rectDockBarWnd.top && rectDockBarWnd.top - rectVirtual.top > nOffset * 2 ||
				rectVirtual.bottom > rectDockBarWnd.bottom && rectVirtual.bottom - rectDockBarWnd.bottom > nOffset * 2)
			{
				bTreatMouse = true;
			}
		}

		if (bTreatMouse && !rectDockBarWnd.PtInRect(ptMousePos))
		{
			return CS_DOCK_IMMEDIATELY;
		}
	}
	else
	{
		if (*ppTargetBar == NULL)
		{
			// the mouse is out of dock bar in either direction - keep the bar floating
			return CS_NOTHING;
		}

		if (!CanBeDocked(*ppTargetBar))
		{
			// bar's style does not allow to dock the bar to this dock bar
			return CS_NOTHING;
		}
		// the mouse is getting closer to a dock bar
		(*ppTargetBar)->GetWindowRect(&rectDockBarWnd);

		if (rectDockBarWnd.PtInRect(ptMousePos))
		{
			// the mouse is over the dock bar, the bar must be docked
			return CS_DOCK_IMMEDIATELY;
		}

		// check on which side the mouse is relatively to the dock bar
		bool bMouseLeft = ptMousePos.x < rectDockBarWnd.left;
		bool bMouseRight = ptMousePos.x > rectDockBarWnd.right;
		bool bMouseTop  = ptMousePos.y < rectDockBarWnd.top;
		bool bMouseBottom = ptMousePos.y > rectDockBarWnd.bottom;

		double dPixelsOnDock = nOffset;
		int nMouseOffset  = 0;
		if (bMouseLeft)
		{
			dPixelsOnDock = ((rectBarWnd.right - ptMousePos.x) * 100. / rectBarWnd.Width()) / 100. * nOffset;
			nMouseOffset = rectDockBarWnd.left - ptMousePos.x;

		}
		else if (bMouseRight)
		{
			dPixelsOnDock = ((ptMousePos.x - rectBarWnd.left) * 100. / rectBarWnd.Width()) / 100. * nOffset;
			nMouseOffset = ptMousePos.x - rectDockBarWnd.right;
		}
		else if (bMouseTop)
		{
			dPixelsOnDock = ((rectBarWnd.bottom - ptMousePos.y) * 100. / rectBarWnd.Height()) / 100. * nOffset;
			nMouseOffset = rectDockBarWnd.top - ptMousePos.y;
		}
		else if (bMouseBottom)
		{
			dPixelsOnDock = ((ptMousePos.y - rectBarWnd.top) * 100. / rectBarWnd.Height()) / 100. * nOffset;
			nMouseOffset = ptMousePos.y - rectDockBarWnd.bottom;
		}

		if (nMouseOffset <= dPixelsOnDock)
		{
			return CS_DOCK_IMMEDIATELY;
		}
	}

	return CS_NOTHING;
}

CPaneFrameWnd* CPane::CreateDefaultMiniframe(CRect rectInitial)
{
	ASSERT_VALID(this);

	CRect rectVirtual = rectInitial;

	CPaneFrameWnd* pMiniFrame =
		(CPaneFrameWnd*) m_pMiniFrameRTC->CreateObject();

	if (pMiniFrame != NULL)
	{
		// it must have valid CFrameEx window as parent
		CWnd* pParentFrame = AFXGetParentFrame(this);
		ASSERT_VALID(pParentFrame);

		pMiniFrame->SetDockingManager(afxGlobalUtils.GetDockingManager(GetDockSiteFrameWnd()));

		if (!pMiniFrame->Create(NULL, WS_POPUP, rectVirtual, pParentFrame))
		{
			TRACE0("Failed to create miniframe");
			delete pMiniFrame;
			return NULL;
		}
	}
	else
	{
		TRACE0("Failed to create miniframe using runtime class information \n");
		ASSERT(FALSE);
	}
	return pMiniFrame;
}

void CPane::UpdateVirtualRect()
{
	ASSERT_VALID(this);

	GetWindowRect(m_rectVirtual);

	CSize size = CalcFixedLayout(FALSE, IsHorizontal());

	m_rectVirtual.right = m_rectVirtual.left + size.cx;
	m_rectVirtual.bottom = m_rectVirtual.top + size.cy;

	if (GetParent() != NULL)
	{
		GetParent()->ScreenToClient(m_rectVirtual);
	}

}

void CPane::UpdateVirtualRect(CPoint ptOffset)
{
	ASSERT_VALID(this);
	if ((GetParent()->GetExStyle() & WS_EX_LAYOUTRTL) && IsHorizontal())
	{
		ptOffset.x = -ptOffset.x;
		m_rectVirtual.OffsetRect(ptOffset);
	}
	else
	{
		m_rectVirtual.OffsetRect(ptOffset);
	}
}
//
void CPane::UpdateVirtualRect(CSize sizeNew)
{
	ASSERT_VALID(this);

	GetWindowRect(m_rectVirtual);

	m_rectVirtual.right = m_rectVirtual.left + sizeNew.cx;
	m_rectVirtual.bottom = m_rectVirtual.top + sizeNew.cy;

	if (GetParent() != NULL)
	{
		GetParent()->ScreenToClient(m_rectVirtual);
	}
}

void CPane::GetVirtualRect(CRect& rectVirtual) const
{
	ASSERT_VALID(this);
	rectVirtual = m_rectVirtual;
	ASSERT_VALID(GetParent());
	GetParent()->ClientToScreen(rectVirtual);
}

void CPane::SetVirtualRect(const CRect& rect, BOOL bMapToParent)
{
	ASSERT_VALID(this);
	m_rectVirtual = rect;
	ASSERT_VALID(GetParent());
	if (bMapToParent)
	{
		MapWindowPoints(GetParent(), m_rectVirtual);
	}
}

void CPane::OnDestroy()
{
	if (IsTabbed())
	{
		CWnd* pParent = GetParent();
		ASSERT_VALID(pParent);

		if (pParent->IsKindOf(RUNTIME_CLASS(CMFCBaseTabCtrl)))
		{
			pParent = pParent->GetParent();
			ASSERT_VALID(pParent);
		}

		if (pParent->IsKindOf(RUNTIME_CLASS(CBaseTabbedPane)))
		{
			CBaseTabbedPane* pTabbedBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, pParent);
			ENSURE(pTabbedBar != NULL);

			HWND hwnd = m_hWnd;
			pTabbedBar->RemovePane(this);
			if (!IsWindow(hwnd))
			{
				// the control bar has been destroyed by RemovePane
				return;
			}
		}
	}

	CBasePane::OnDestroy();
}

void CPane::OnNcDestroy()
{
	ASSERT_VALID(this);
	CPaneFrameWnd::AddRemovePaneFromGlobalList(this, FALSE /* remove*/);
	ASSERT_VALID(this);

	CPaneFrameWnd* pMiniFrame = GetParentMiniFrame(TRUE);

	if (pMiniFrame != NULL)
		pMiniFrame->RemovePane(this, FALSE);

	CBasePane::OnNcDestroy();
}

// MFC's control bar compatibility

BOOL CPane::AllocElements(int nElements, int cbElement)
{
	ASSERT_VALID(this);
	ENSURE(nElements >= 0 && cbElement >= 0);
	ENSURE(m_pData != NULL || m_nCount == 0);

	// allocate new data if necessary
	void* pData = NULL;
	if (nElements > 0)
	{
		ENSURE(cbElement > 0);
		if ((pData = calloc(nElements, cbElement)) == NULL)
			return FALSE;
	}

	free(m_pData);      // free old data

	// set new data and elements
	m_pData = pData;
	m_nCount = nElements;

	return TRUE;
}

void CPane::CalcInsideRect(CRect& rect, BOOL bHorz) const
{
	ASSERT_VALID(this);
	DWORD dwStyle = GetPaneStyle();

	if (!IsFloating() && !IsTabbed())
	{
		if (dwStyle & CBRS_BORDER_LEFT)
			rect.left += AFX_CX_BORDER;
		if (dwStyle & CBRS_BORDER_TOP)
			rect.top += AFX_CY_BORDER;
		if (dwStyle & CBRS_BORDER_RIGHT)
			rect.right -= AFX_CX_BORDER;
		if (dwStyle & CBRS_BORDER_BOTTOM)
			rect.bottom -= AFX_CY_BORDER;
	}

	// inset the top and bottom.
	if (bHorz)
	{
		rect.left += m_cxLeftBorder;
		rect.top += m_cyTopBorder;
		rect.right -= m_cxRightBorder;
		rect.bottom -= m_cyBottomBorder;

		if ((dwStyle &(CBRS_GRIPPER|CBRS_FLOATING)) == CBRS_GRIPPER)
		{
			if (GetExStyle() & WS_EX_LAYOUTRTL)
			{
				rect.right -= AFX_CX_BORDER_GRIPPER+AFX_CX_GRIPPER+AFX_CX_BORDER_GRIPPER;
			}
			else
			{
				rect.left += AFX_CX_BORDER_GRIPPER+AFX_CX_GRIPPER+AFX_CX_BORDER_GRIPPER;
			}
		}
	}
	else
	{
		rect.left += m_cyTopBorder;
		rect.top += m_cxLeftBorder;
		rect.right -= m_cyBottomBorder;
		rect.bottom -= m_cxRightBorder;

		if ((dwStyle &(CBRS_GRIPPER|CBRS_FLOATING)) == CBRS_GRIPPER)
		{
			rect.top += AFX_CY_BORDER_GRIPPER+AFX_CY_GRIPPER+AFX_CY_BORDER_GRIPPER;
		}
	}
}

void CPane::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	OnProcessDblClk();

	if (CanFloat())
	{
		FloatPane(m_recentDockInfo.m_rectRecentFloatingRect, DM_DBL_CLICK);
		CBasePane::OnLButtonDblClk(nFlags, point);
	}
}

void CPane::OnProcessDblClk()
{
	m_bDblClick = true;

	StoreRecentDockSiteInfo();

	if (m_bCaptured)
	{
		ReleaseCapture();

		m_bCaptured = false;
		SetDragMode(FALSE);

		if (m_hwndMiniFrameToBeClosed != NULL && ::IsWindow(m_hwndMiniFrameToBeClosed))
		{
			::DestroyWindow(m_hwndMiniFrameToBeClosed);
		}

		m_hwndMiniFrameToBeClosed = NULL;
	}
}

BOOL CPane::IsTabbed() const
{
	CWnd* pImmediateParent = GetParent();
	if (pImmediateParent == NULL)
	{
		return FALSE;
	}

	CWnd* pNextParent = pImmediateParent->GetParent();
	return(pNextParent != NULL) &&((pImmediateParent->IsKindOf(RUNTIME_CLASS(CMFCBaseTabCtrl)) &&(pNextParent->IsKindOf(RUNTIME_CLASS(CBaseTabbedPane)))) ||
		(pImmediateParent->IsKindOf(RUNTIME_CLASS(CDockablePaneAdapter)) && pNextParent->IsKindOf(RUNTIME_CLASS(CMFCBaseTabCtrl))));
}

void CPane::SetDragMode(BOOL bOnOff)
{
	m_bDragMode = bOnOff;
}

void CPane::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if (CMFCPopupMenu::GetActiveMenu() != NULL)
	{
		return;
	}

	if (!CMFCToolBar::IsCustomizeMode())
	{
		if (OnShowControlBarMenu(point))
		{
			return;
		}

		CFrameWnd* pParentFrame = DYNAMIC_DOWNCAST(CFrameWnd, m_pDockSite);
		if (pParentFrame == NULL)
		{
			pParentFrame = AFXGetTopLevelFrame(this);
		}

		if (pParentFrame != NULL)
		{
			ASSERT_VALID(pParentFrame);

			OnPaneContextMenu(pParentFrame, point);
		}
	}
}

BOOL CPane::LoadState(LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CString strProfileName = ::AFXGetRegPath(strControlBarProfile, lpszProfileName);

	if (nIndex == -1)
	{
		nIndex = GetDlgCtrlID();
	}

	CString strSection;
	if (uiID == (UINT) -1)
	{
		strSection.Format(AFX_REG_SECTION_FMT, (LPCTSTR)strProfileName, nIndex);
	}
	else
	{
		strSection.Format(AFX_REG_SECTION_FMT_EX, (LPCTSTR)strProfileName, nIndex, uiID);
	}

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (!reg.Open(strSection))
	{
		return FALSE;
	}

	reg.Read(_T("ID"), (int&) m_nID);

	reg.Read(_T("RectRecentFloat"), m_recentDockInfo.m_rectRecentFloatingRect);
	reg.Read(_T("RectRecentDocked"), m_rectSavedDockedRect);

	// !!!!!! change to appropriate handling for slider/frame
	m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect = m_rectSavedDockedRect;

	reg.Read(_T("RecentFrameAlignment"), m_recentDockInfo.m_dwRecentAlignmentToFrame);
	reg.Read(_T("RecentRowIndex"), m_recentDockInfo.m_nRecentRowIndex);
	reg.Read(_T("IsFloating"), m_bRecentFloatingState);
	reg.Read(_T("MRUWidth"), m_nMRUWidth);
	reg.Read(_T("PinState"), m_bPinState);

	return CBasePane::LoadState(lpszProfileName, nIndex, uiID);
}

BOOL CPane::SaveState(LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CString strProfileName = ::AFXGetRegPath(strControlBarProfile, lpszProfileName);

	if (nIndex == -1)
	{
		nIndex = GetDlgCtrlID();
	}

	CString strSection;
	if (uiID == (UINT) -1)
	{
		strSection.Format(AFX_REG_SECTION_FMT, (LPCTSTR)strProfileName, nIndex);
	}
	else
	{
		strSection.Format(AFX_REG_SECTION_FMT_EX, (LPCTSTR)strProfileName, nIndex, uiID);
	}

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);

	if (reg.CreateKey(strSection))
	{
		BOOL bFloating = IsFloating();

		if (bFloating)
		{
			CPaneFrameWnd* pMiniFrame = GetParentMiniFrame();
			if (pMiniFrame != NULL)
			{
				pMiniFrame->GetWindowRect(m_recentDockInfo.m_rectRecentFloatingRect);
			}
		}
		else
		{
			CalcRecentDockedRect();
			if (m_pParentDockBar != NULL)
			{
				m_recentDockInfo.m_dwRecentAlignmentToFrame = m_pParentDockBar->GetCurrentAlignment();
				m_recentDockInfo.m_nRecentRowIndex = m_pParentDockBar->FindRowIndex(m_pDockBarRow);
			}
		}

		reg.Write(_T("ID"), (int&)m_nID);

		reg.Write(_T("RectRecentFloat"), m_recentDockInfo.m_rectRecentFloatingRect);
		reg.Write(_T("RectRecentDocked"), m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect);

		reg.Write(_T("RecentFrameAlignment"), m_recentDockInfo.m_dwRecentAlignmentToFrame);
		reg.Write(_T("RecentRowIndex"), m_recentDockInfo.m_nRecentRowIndex);
		reg.Write(_T("IsFloating"), bFloating);
		reg.Write(_T("MRUWidth"), m_nMRUWidth);
		reg.Write(_T("PinState"), m_bPinState);
	}
	return CBasePane::SaveState(lpszProfileName, nIndex, uiID);
}

void CPane::CalcRecentDockedRect()
{
	GetWindowRect(m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect);

	if (m_pParentDockBar != NULL)
	{
		m_pParentDockBar->ScreenToClient(m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect);
	}
	else if (GetDockSiteFrameWnd() != NULL)
	{
		GetDockSiteFrameWnd()->ScreenToClient(m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect);
	}
}

void CPane::SetDockState(CDockingManager* pDockManager)
{
	ASSERT_VALID(this);

	if (!m_bRecentFloatingState)
	{
		CDockSite* pDockBar = pDockManager->FindDockSite(m_recentDockInfo.m_dwRecentAlignmentToFrame, TRUE);

		if (pDockBar != NULL)
		{
			pDockManager->DockPane(this, pDockBar->GetDockSiteID(), m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect);
		}

		if (m_pParentDockBar != NULL)
		{
			m_pParentDockBar->ShowPane(this, GetRecentVisibleState(), TRUE, FALSE);
			if (m_pDockBarRow != NULL)
			{
				m_pDockBarRow->ExpandStretchedPanes();
			}
		}
	}
}

void CPane::OnCancelMode()
{
	CBasePane::OnCancelMode();
	if (m_bCaptured)
	{
		if ((GetDockingMode() & DT_STANDARD) != 0)
		{
			m_dragFrameImpl.EndDrawDragFrame();
		}

		ReleaseCapture();
		m_bCaptured = false;
		SetDragMode(FALSE);

		if (m_hwndMiniFrameToBeClosed != NULL && ::IsWindow(m_hwndMiniFrameToBeClosed))
		{
			::DestroyWindow(m_hwndMiniFrameToBeClosed);
		}

		m_hwndMiniFrameToBeClosed = NULL;
	}
}

void CPane::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_ESCAPE)
	{
		OnCancelMode();
	}
	CBasePane::OnChar(nChar, nRepCnt, nFlags);
}

void CPane::SetActiveInGroup(BOOL bActive)
{
	m_bActiveInGroup = bActive;
}

void CPane::UndockPane(BOOL bDelay)
{
	ASSERT_VALID(this);
	if (m_pParentDockBar != NULL)
	{
		m_pParentDockBar->RemovePane(this, DM_UNKNOWN);
	}

	if (!bDelay)
	{
		AdjustDockingLayout();
	}
}

void CPane::AdjustSizeImmediate(BOOL bRecalcLayout)
{
	CMFCReBar* pBar = DYNAMIC_DOWNCAST(CMFCReBar, GetParent());
	if (pBar != NULL)
	{
		return;
	}

	CSize sizeCurr = CalcFixedLayout(FALSE, IsHorizontal());
	CRect rect;
	GetWindowRect(rect);

	if (rect.Size() != sizeCurr)
	{
		SetWindowPos(NULL, 0, 0, sizeCurr.cx, sizeCurr.cy, SWP_NOMOVE  | SWP_NOACTIVATE | SWP_NOZORDER);
	}

	if (m_pParentDockBar != NULL)
	{
		UpdateVirtualRect();
		if (bRecalcLayout)
		{
			m_pDockBarRow->ArrangePanes(this);
			AFXGetParentFrame(this)->RecalcLayout();
		}
	}
}

void CPane::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	CBasePane::OnStyleChanged(nStyleType, lpStyleStruct);

	if (nStyleType == GWL_EXSTYLE)
	{
		if (((lpStyleStruct->styleOld & WS_EX_LAYOUTRTL) != 0 && (lpStyleStruct->styleNew & WS_EX_LAYOUTRTL) == 0 ||
			(lpStyleStruct->styleOld & WS_EX_LAYOUTRTL) == 0 && (lpStyleStruct->styleNew & WS_EX_LAYOUTRTL) != 0))
		{
			OnRTLChanged((lpStyleStruct->styleNew & WS_EX_LAYOUTRTL) != 0);
		}
	}
}

void CPane::OnRTLChanged(BOOL bIsRTL)
{
	afxGlobalData.m_bIsRTL = bIsRTL;

	if (GetParentDockSite() != NULL && IsHorizontal())
	{
		SetWindowPos(NULL, m_rectVirtual.left, m_rectVirtual.top, m_rectVirtual.Width(), m_rectVirtual.Height(), SWP_NOZORDER);
	}
}

BOOL CPane::OnShowControlBarMenu(CPoint point)
{
	if (afxContextMenuManager == NULL)
	{
		return FALSE;
	}

	if ((GetEnabledAlignment() & CBRS_ALIGN_ANY) == 0 && !CanFloat())
	{
		return FALSE;
	}

	const UINT idFloating = (UINT) -102;
	const UINT idDocking = (UINT) -103;
	const UINT idAutoHide = (UINT) -104;
	const UINT idHide = (UINT) -105;
	const UINT idTabbed = (UINT) -106;

	CMenu menu;
	menu.CreatePopupMenu();

	{
		CString strItem;

		ENSURE(strItem.LoadString(IDS_AFXBARRES_FLOATING));
		menu.AppendMenu(MF_STRING, idFloating, strItem);

		ENSURE(strItem.LoadString(IDS_AFXBARRES_DOCKING));
		menu.AppendMenu(MF_STRING, idDocking, strItem);

		ENSURE(strItem.LoadString(IDS_AFXBARRES_TABBED));
		menu.AppendMenu(MF_STRING, idTabbed, strItem);

		ENSURE(strItem.LoadString(IDS_AFXBARRES_AUTOHIDE));
		menu.AppendMenu(MF_STRING, idAutoHide, strItem);

		ENSURE(strItem.LoadString(IDS_AFXBARRES_HIDE));
		menu.AppendMenu(MF_STRING, idHide, strItem);
	}

	if (!CanFloat())
	{
		menu.EnableMenuItem(idFloating, MF_GRAYED);
	}

	if (!CanAutoHide() || GetParentMiniFrame() != NULL)
	{
		menu.EnableMenuItem(idAutoHide, MF_GRAYED);
	}

	if (IsAutoHideMode())
	{
		menu.EnableMenuItem(idFloating, MF_GRAYED);
		menu.EnableMenuItem(idDocking, MF_GRAYED);
		menu.CheckMenuItem(idAutoHide, MF_CHECKED);
		menu.EnableMenuItem(idHide, MF_GRAYED);
	}

	CMDIFrameWndEx* pFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetDockSiteFrameWnd());
	if (!CanBeTabbedDocument() || pFrame != NULL && pFrame->IsFullScreen())
	{
		menu.EnableMenuItem(idTabbed, MF_GRAYED);
	}

	if (IsMDITabbed())
	{
		menu.EnableMenuItem(idFloating, MF_GRAYED);
		menu.EnableMenuItem(idDocking, MF_GRAYED);
		menu.CheckMenuItem(idTabbed, MF_CHECKED);
	}

	if (IsFloating())
	{
		menu.CheckMenuItem(idFloating, MF_CHECKED);
	}
	else if (!IsAutoHideMode() && !IsMDITabbed())
	{
		menu.CheckMenuItem(idDocking, MF_CHECKED);
	}

	if ((GetEnabledAlignment() & CBRS_ALIGN_ANY) == 0)
	{
		menu.EnableMenuItem(idDocking, MF_GRAYED);
	}

	if (!CanBeClosed())
	{
		menu.EnableMenuItem(idHide, MF_GRAYED);
	}

	if (!OnBeforeShowPaneMenu(menu))
	{
		return FALSE;
	}

	HWND hwndThis = GetSafeHwnd();

	int nMenuResult = afxContextMenuManager->TrackPopupMenu(
		menu, point.x, point.y, this);

	if (!::IsWindow(hwndThis))
	{
		return TRUE;
	}

	if (!OnAfterShowPaneMenu(nMenuResult))
	{
		return TRUE;
	}

	switch(nMenuResult)
	{
	case idDocking:
		if (IsFloating())
		{
			CPaneFrameWnd* pMiniFrame = GetParentMiniFrame();
			if (pMiniFrame != NULL)
			{
				pMiniFrame->OnDockToRecentPos();
			}
		}
		break;

	case idFloating:
		{
			BOOL bWasFloated = FALSE;

			CBaseTabbedPane* pTabbedBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, IsTabbed() ? GetParentTabbedPane() : this);

			if (pTabbedBar != NULL)
			{
				ASSERT_VALID(pTabbedBar);

				CMFCBaseTabCtrl* pTabWnd = pTabbedBar->GetUnderlyingWindow();
				if (pTabWnd != NULL)
				{
					ASSERT_VALID(pTabWnd);

					const int nTabID = pTabWnd->GetActiveTab();
					CWnd* pWnd = pTabWnd->GetTabWnd(nTabID);

					if (pWnd != NULL && pTabWnd->IsTabDetachable(nTabID))
					{
						bWasFloated = pTabbedBar->DetachPane(pWnd, FALSE);
						if (bWasFloated)
						{
							if (pTabWnd->GetTabsNum() > 0 &&
								pTabWnd->GetVisibleTabsNum() == 0)
							{
								pTabbedBar->ShowPane(FALSE, FALSE, FALSE);
							}
						}
					}
				}
			}

			if (!bWasFloated)
			{
				FloatPane(m_recentDockInfo.m_rectRecentFloatingRect);
			}
		}
		break;

	case idAutoHide:
		ToggleAutoHide();
		break;

	case idHide:
		OnPressCloseButton();
		break;

	case idTabbed:
		if (IsMDITabbed())
		{
			CMDIChildWndEx* pMDIChild = DYNAMIC_DOWNCAST(CMDIChildWndEx, GetParent());
			if (pMDIChild == NULL)
			{
				ASSERT(FALSE);
				return FALSE;
			}

			CMDIFrameWndEx* pTabbedFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetDockSiteFrameWnd());
			if (pTabbedFrame == NULL)
			{
				ASSERT(FALSE);
				return FALSE;
			}

			pTabbedFrame->TabbedDocumentToControlBar(pMDIChild);
		}
		else
		{
			ConvertToTabbedDocument();
		}
	}

	return TRUE;
}

void CPane::OnPressCloseButton()
{
	CPaneFrameWnd* pMiniFrame = GetParentMiniFrame();
	if (pMiniFrame != NULL)
	{
		if (pMiniFrame->OnCloseMiniFrame())
		{
			pMiniFrame->CloseMiniFrame();
		}
	}
}

void CPane::CopyState(CPane* pOrgBar)
{
	ASSERT_VALID(pOrgBar);

	CBasePane::CopyState(pOrgBar);

	m_bFirstInGroup = pOrgBar->m_bFirstInGroup;
	m_bLastInGroup = pOrgBar->m_bLastInGroup;
	m_bActiveInGroup = pOrgBar->m_bActiveInGroup;

	pOrgBar->GetMinSize(m_sizeMin);

	m_recentDockInfo = pOrgBar->m_recentDockInfo;
	m_rectSavedDockedRect = pOrgBar->m_rectSavedDockedRect;
	m_bRecentFloatingState = pOrgBar->m_bRecentFloatingState;
}

void CPane::GetPaneName(CString& strName) const
{
	if (GetSafeHwnd() == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	GetWindowText(strName);
}

BOOL CPane::CanBeTabbedDocument() const
{
	ASSERT_VALID(this);

	if (IsAutoHideMode())
	{
		return FALSE;
	}

	CMDIFrameWndEx* pMDIFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetDockSiteFrameWnd());
	if (pMDIFrame == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(pMDIFrame);

	if (!pMDIFrame->CanConvertControlBarToMDIChild())
	{
		return FALSE;
	}

	return pMDIFrame->AreMDITabs();
}

void CPane::ConvertToTabbedDocument(BOOL /*bActiveTabOnly*/)
{
	ASSERT(FALSE);
	TRACE0("You need to derive a class from CDockablePane\n");
	return;
}



