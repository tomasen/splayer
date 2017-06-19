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
#include "afxmultipaneframewnd.h"
#include "afxglobalutils.h"
#include "afxdockingmanager.h"
#include "afxdockablepane.h"
#include "afxpanedivider.h"

#include "afxbasetabbedpane.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CMultiPaneFrameWnd,CPaneFrameWnd,VERSIONABLE_SCHEMA | 2)

/////////////////////////////////////////////////////////////////////////////
// CMultiPaneFrameWnd

CMultiPaneFrameWnd::CMultiPaneFrameWnd()
{
	m_hWndLastFocused = NULL;
	m_bHostsToolbar = FALSE;
}

CMultiPaneFrameWnd::~CMultiPaneFrameWnd()
{
}

//{{AFX_MSG_MAP(CMultiPaneFrameWnd)
BEGIN_MESSAGE_MAP(CMultiPaneFrameWnd, CPaneFrameWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_GETMINMAXINFO()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SETFOCUS()
	ON_MESSAGE(WM_IDLEUPDATECMDUI, &CMultiPaneFrameWnd::OnIdleUpdateCmdUI)
	ON_REGISTERED_MESSAGE(AFX_WM_CHECKEMPTYMINIFRAME, &CMultiPaneFrameWnd::OnCheckEmptyState)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// CMultiPaneFrameWnd message handlers

int CMultiPaneFrameWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPaneFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_barContainerManager.Create(this, NULL);
	return 0;
}

// Should return TRUE if no docking occures!!!

BOOL CMultiPaneFrameWnd::DockFrame(CPaneFrameWnd* pDockedFrame, AFX_DOCK_METHOD dockMethod)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDockedFrame);

	CMultiPaneFrameWnd* pMultiDockedFrame = DYNAMIC_DOWNCAST(CMultiPaneFrameWnd, pDockedFrame);

	if (pMultiDockedFrame == NULL )
	{
		// we can dock only multi mini frame windows!
		//(their dock bars have CBRS_FLOAT_MULTI style)
		return TRUE;
	}

	CBasePane* pFirstBar = DYNAMIC_DOWNCAST(CBasePane, pMultiDockedFrame->GetFirstVisiblePane());
	if (pFirstBar == NULL || pFirstBar->GetEnabledAlignment() == 0)
	{
		return TRUE;
	}

	CDockablePane* pTargetControlBar = NULL;
	BOOL bTabArea = FALSE;
	BOOL bCaption = FALSE;
	DWORD dwAlignment = CBRS_ALIGN_LEFT;

	CPoint pt;
	GetCursorPos(&pt);

	if (dockMethod == DM_MOUSE || dockMethod == DM_STANDARD)
	{
		CGlobalUtils globalUtilsLocal;
		if (!globalUtilsLocal.GetPaneAndAlignFromPoint(m_barContainerManager, pt, &pTargetControlBar, dwAlignment, bTabArea, bCaption))
		{
			return TRUE;
		}
	}

	if (pTargetControlBar == NULL || dwAlignment == 0)
	{
		return TRUE;
	}

	CPaneContainerManager& barManager = pMultiDockedFrame->GetPaneContainerManager();
	CWnd* pFirstDockedBar = barManager.GetFirstVisiblePane();

	if ((bTabArea || bCaption) && pTargetControlBar != NULL)
	{
		// if the first bar is a tabbed bar it will be destroyed when adding to tab wnd
		// we need to take one of its tabs
		CBaseTabbedPane* pTabbedBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, pFirstDockedBar);

		if (pTabbedBar != NULL)
		{
			int iTabNum = -1;
			pFirstDockedBar = pTabbedBar->GetFirstVisibleTab(iTabNum);
		}

		if (!m_barContainerManager.AddPaneContainerManagerToDockablePane(pTargetControlBar, barManager))
		{
			return TRUE;
		}
	}
	else
	{
		if (!m_barContainerManager.AddPaneContainerManager(pTargetControlBar, dwAlignment, barManager, TRUE))
		{
			return TRUE;
		}
	}

	HWND hDockedFrame = pDockedFrame->m_hWnd;
	pMultiDockedFrame->SendMessage(AFX_WM_CHECKEMPTYMINIFRAME);
	if (IsWindow(hDockedFrame))
	{
		pMultiDockedFrame->MoveWindow(pMultiDockedFrame->GetRecentFloatingRect());
	}

	OnPaneRecalcLayout();

	if (dockMethod == DM_MOUSE && pFirstDockedBar != NULL)
	{
		pFirstDockedBar->ScreenToClient(&pt);
		if (pFirstDockedBar->IsKindOf(RUNTIME_CLASS(CPane)))
		{
			((CPane*) pFirstDockedBar)->EnterDragMode(TRUE);
		}
		else
		{
			pFirstDockedBar->SendMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(pt.x, pt.y));
		}

	}

	OnSetRollUpTimer();

	return FALSE;
}

BOOL CMultiPaneFrameWnd::DockPane(CDockablePane* pDockedBar)
{
	CPoint pt;
	GetCursorPos(&pt);

	BOOL bTabArea = FALSE;
	BOOL bCaption = FALSE;
	CDockablePane* pTargetControlBar = NULL;
	DWORD dwAlignment = 0;

	CGlobalUtils globalUtilsLocal;
	if (!globalUtilsLocal.GetPaneAndAlignFromPoint(m_barContainerManager, pt, &pTargetControlBar, dwAlignment, bTabArea, bCaption))
	{
		return TRUE;
	}

	if (pTargetControlBar == NULL || dwAlignment == 0)
	{
		return TRUE;
	}

	pDockedBar->UndockPane(FALSE);

	pDockedBar->SetParent(this);
	BOOL bResult = m_barContainerManager.InsertPane(pDockedBar, pTargetControlBar, dwAlignment);

	if (!bResult)
	{
		ASSERT(FALSE);
	}

	if (bResult)
	{
		AddRemovePaneFromGlobalList(pDockedBar, TRUE);
		CheckGripperVisibility();
		OnPaneRecalcLayout();
		SendMessage(WM_NCPAINT);
	}

	OnSetRollUpTimer();

	if (pDockedBar->CanFocus())
	{
		pDockedBar->SetFocus();
	}

	OnPaneRecalcLayout();

	return !bResult;
}

CDockablePane* CMultiPaneFrameWnd::DockPaneStandard(BOOL& bWasDocked)
{
	if (!OnBeforeDock())
	{
		return NULL;
	}

	CObList lstBars;
	m_barContainerManager.AddPanesToList(&lstBars, NULL);
	CList<HWND, HWND> lstBarsHwnd;

	for (POSITION pos = lstBars.GetHeadPosition(); pos != NULL;)
	{
		CWnd* pWnd = DYNAMIC_DOWNCAST(CWnd, lstBars.GetNext(pos));
		if (pWnd != NULL)
		{
			lstBarsHwnd.AddTail(pWnd->GetSafeHwnd());
		}
	}

	CBasePane* pTargetControlBar = m_dragFrameImpl.m_pFinalTargetBar;
	AFX_PREDOCK_STATE state = m_dragFrameImpl.m_bDockToTab ? PDS_DOCK_TO_TAB : PDS_DOCK_REGULAR;

	CPaneFrameWnd* pParentMiniFrame = NULL;

	if (pTargetControlBar != NULL)
	{
		pParentMiniFrame = pTargetControlBar->GetParentMiniFrame();
	}

	CWnd* pFocusWnd = GetFocus();

	if (pParentMiniFrame == NULL)
	{
		bWasDocked = !SetPreDockState(state, pTargetControlBar, DM_STANDARD);
	}
	else
	{
		CMultiPaneFrameWnd* pParentMultiMiniFrame = DYNAMIC_DOWNCAST(CMultiPaneFrameWnd, pParentMiniFrame);
		if (pParentMultiMiniFrame != NULL && pParentMultiMiniFrame != this)
		{
			bWasDocked = !pParentMultiMiniFrame->DockFrame(this, DM_STANDARD);
		}
	}

	if (pFocusWnd != NULL && ::IsWindow(pFocusWnd->GetSafeHwnd()))
	{
		pFocusWnd->SetFocus();
	}

	if (bWasDocked)
	{
		for (POSITION pos = lstBarsHwnd.GetHeadPosition(); pos != NULL;)
		{
			HWND hwnd = lstBarsHwnd.GetNext(pos);
			CDockablePane* pNextBar = DYNAMIC_DOWNCAST(CDockablePane, CWnd::FromHandle(hwnd));
			if (pNextBar != NULL)
			{
				pNextBar->OnAfterDockFromMiniFrame();
			}
		}
	}

	return NULL;
}

void CMultiPaneFrameWnd::OnSetRollUpTimer()
{
	CObList lstControlBars;
	m_barContainerManager.AddPanesToList(&lstControlBars, NULL);

	for (POSITION pos = lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CBasePane* pNextBar = DYNAMIC_DOWNCAST(CBasePane, lstControlBars.GetNext(pos));
		ASSERT_VALID(pNextBar);

		if (pNextBar != NULL && pNextBar->GetControlBarStyle() & AFX_CBRS_AUTO_ROLLUP)
		{
			SetRollUpTimer();
			break;
		}
	}
}

void CMultiPaneFrameWnd::OnKillRollUpTimer()
{
	CObList lstControlBars;
	m_barContainerManager.AddPanesToList(&lstControlBars, NULL);

	BOOL bThereIsRollupState = FALSE;
	for (POSITION pos = lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CBasePane* pNextBar = DYNAMIC_DOWNCAST(CBasePane, lstControlBars.GetNext(pos));
		ASSERT_VALID(pNextBar);

		if (pNextBar != NULL && pNextBar->GetControlBarStyle() & AFX_CBRS_AUTO_ROLLUP)
		{
			bThereIsRollupState = TRUE;
			break;
		}
	}

	if (!bThereIsRollupState)
	{
		KillRollupTimer();
	}
}

void CMultiPaneFrameWnd::AdjustPaneFrames()
{
	CObList& lstControlBars = m_barContainerManager.m_lstControlBars;
	UINT uiSWPFlags = SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED;
	for (POSITION pos = lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CDockablePane* pNextBar = DYNAMIC_DOWNCAST(CDockablePane, lstControlBars.GetNext(pos));
		ASSERT_VALID(pNextBar);

		pNextBar->SetWindowPos(NULL, -1, -1, -1, -1, uiSWPFlags);
	}
}

void CMultiPaneFrameWnd::AddPane(CBasePane* pWnd)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pWnd);
	ASSERT_KINDOF(CDockablePane, pWnd);

	CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, pWnd);

	if (m_barContainerManager.IsEmpty())
	{
		m_barContainerManager.AddPane(pBar);
		CPaneFrameWnd::AddPane(pWnd);
	}

	OnSetRollUpTimer();
}

BOOL CMultiPaneFrameWnd::AddRecentPane(CDockablePane* pBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);

	CPaneContainer* pRecentContainer = pBar->m_recentDockInfo.GetRecentPaneContainer(FALSE);
	CPaneContainer* pRecentTabContainer = pBar->m_recentDockInfo.GetRecentTabContainer(FALSE);
	if (pRecentContainer != NULL)
	{
		pBar->SetParent(this);
		AddRemovePaneFromGlobalList(pBar, TRUE);
		CDockablePane* pAddedBar = m_barContainerManager.AddPaneToRecentPaneContainer(pBar, pRecentContainer);

		CWnd* pEmbeddedWnd = CWnd::FromHandlePermanent(m_hEmbeddedBar);

		if (pAddedBar != NULL && pEmbeddedWnd == NULL)
		{
			m_hEmbeddedBar = pAddedBar->GetSafeHwnd();
		}
		if (m_barContainerManager.GetVisiblePaneCount() == 1 && pBar == pAddedBar)
		{
			MoveWindow(m_rectRecentFloatingRect);
		}
		if (pAddedBar != NULL)
		{
			OnShowPane(pAddedBar, TRUE);
		}
	}
	else if (pRecentTabContainer != NULL)
	{
		pBar->SetParent(this);
		AddRemovePaneFromGlobalList(pBar, TRUE);
		BOOL bRecentLeftBar = pBar->m_recentDockInfo.IsRecentLeftPane(FALSE);
		CDockablePane* pTabbedBar = (CDockablePane*)(bRecentLeftBar ? pRecentTabContainer->GetLeftPane() : pRecentTabContainer->GetRightPane());
		if (pTabbedBar != NULL)
		{
			CDockablePane* pCreatedTabbedBar = NULL;
			pBar->AttachToTabWnd(pTabbedBar, DM_DBL_CLICK, TRUE, &pCreatedTabbedBar);

			pTabbedBar->ShowPane(TRUE, FALSE, TRUE);
			OnPaneRecalcLayout();
		}
		else
		{
			CDockablePane* pAddedBar = m_barContainerManager.AddPaneToRecentPaneContainer(pBar, pRecentTabContainer);
			OnShowPane(pAddedBar, TRUE);
		}
	}
	else
	{
		ASSERT(FALSE);
		return FALSE;
	}
	OnSetRollUpTimer();
	return TRUE;
}

void CMultiPaneFrameWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	OnPaneRecalcLayout();

	ArrangeCaptionButtons();
	SendMessage(WM_NCPAINT);
}

void CMultiPaneFrameWnd::OnSizing(UINT fwSide, LPRECT pRect)
{
	CWnd::OnSizing(fwSide, pRect);
}

void CMultiPaneFrameWnd::OnPaneRecalcLayout()
{
	ASSERT_VALID(this);

	CRect rectClient;
	GetClientRect(rectClient);
	HDWP hdwp = ::BeginDeferWindowPos(20);
	m_barContainerManager.ResizePaneContainers(rectClient, hdwp);
	EndDeferWindowPos(hdwp);

	if (CPane::m_bHandleMinSize)
	{
		CRect rectContainer;
		m_barContainerManager.GetWindowRect(rectContainer);

		CRect rectWnd;
		GetWindowRect(rectWnd);

		int nDeltaWidth = rectContainer.Width() - rectClient.Width();
		int nDeltaHeight = rectContainer.Height() - rectClient.Height();

		if (nDeltaWidth < 0)
		{
			nDeltaWidth = 0;
		}

		if (nDeltaHeight < 0)
		{
			nDeltaHeight = 0;
		}

		if (nDeltaWidth != 0 || nDeltaHeight != 0)
		{
			SetWindowPos(NULL, -1, -1, rectWnd.Width() + nDeltaWidth, rectWnd.Height() + nDeltaHeight, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		}
	}
}

void CMultiPaneFrameWnd::RemovePane(CBasePane* pBar, BOOL bDestroy, BOOL /*bNoDelayedDestroy*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);

	if (pBar->IsKindOf(RUNTIME_CLASS(CDockablePane)))
	{
		m_barContainerManager.RemovePaneFromPaneContainer((CDockablePane*) pBar);
		if (!m_barContainerManager.IsEmpty())
		{
			CPaneFrameWnd::ReplacePane(pBar, m_barContainerManager.GetFirstPane());
		}
		else
		{
			// do not destroy the miniframe in the base class
			CPaneFrameWnd::RemovePane(pBar, FALSE);

			// if embedded bar has became NULL set it to the first bar in the container
			CWnd* pEmbeddedWnd = CWnd::FromHandlePermanent(m_hEmbeddedBar);
			if (pEmbeddedWnd == NULL)
			{
				m_hEmbeddedBar = m_barContainerManager.GetFirstPane()->GetSafeHwnd();
			}
		}
	}

	if (bDestroy && GetPaneCount() == 0)
	{
		PostMessage(AFX_WM_CHECKEMPTYMINIFRAME);
	}
	else
	{
		CheckGripperVisibility();
		OnPaneRecalcLayout();
		SendMessage(WM_NCPAINT);
	}

	OnKillRollUpTimer();
}

BOOL CMultiPaneFrameWnd::SetPreDockState(AFX_PREDOCK_STATE preDockState, CBasePane* pBarToDock, AFX_DOCK_METHOD dockMethod)
{
	ASSERT_VALID(this);

	if (preDockState == PDS_NOTHING || preDockState == PDS_DOCK_TO_TAB && pBarToDock != NULL && !pBarToDock->CanBeAttached())
	{
		return TRUE;
	}

	CDockablePane* pTargetDockBar = DYNAMIC_DOWNCAST(CDockablePane, pBarToDock);

	CDockingManager* pDockManager = m_pDockManager != NULL ? m_pDockManager : afxGlobalUtils.GetDockingManager(GetParent());
	if (pDockManager == NULL)
	{
		ASSERT(FALSE);
		return TRUE;
	}

	CWnd* pFirstDockedBar = m_barContainerManager.GetFirstVisiblePane();

	// determine dock alignment and edge
	CPoint pt;
	GetCursorPos(&pt);

	DWORD dwAlignment = 0;

	CObList lstControlBars;
	m_barContainerManager.AddPanesToList(&lstControlBars, NULL);

	CList<HWND, HWND> lstHandles;

	POSITION pos = NULL;

	for (pos = lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CDockablePane* pNextBar = DYNAMIC_DOWNCAST(CDockablePane, lstControlBars.GetNext(pos));
		ASSERT_VALID(pNextBar);

		pNextBar->StoreRecentDockSiteInfo();
		lstHandles.AddTail(pNextBar->GetSafeHwnd());
	}

	if (pTargetDockBar != NULL)
	{
		CBasePane* pHeadBar = m_barContainerManager.GetFirstPane();

		if (pHeadBar == NULL || !pTargetDockBar->CanAcceptPane(pHeadBar) || !pHeadBar->CanAcceptPane(pTargetDockBar))
		{
			return TRUE;
		}

		if (!pHeadBar->IsPaneVisible() && m_barContainerManager.GetPaneCount() == 1 && (pHeadBar->GetDockingMode() & DT_STANDARD) != 0)
		{
			// the head bar is unvisible and there is only one bar in container manager
			// means that this bar was torn off from the tab window and its parent miniframe
			// is hidden
			pHeadBar->ModifyStyle(0, WS_VISIBLE);
		}

		BOOL bOuterEdge = FALSE;
		if (preDockState == PDS_DOCK_REGULAR && !afxGlobalUtils.CheckAlignment(pt, pTargetDockBar, CDockingManager::m_nDockSensitivity, NULL, bOuterEdge, dwAlignment))
		{
			// unable for some reason to determine alignment
			return TRUE;
		}

		if (preDockState == PDS_DOCK_REGULAR)
		{
			if (!pTargetDockBar->DockPaneContainer(m_barContainerManager, dwAlignment, dockMethod))
			{
				return TRUE;
			}
		}
		else if (preDockState == PDS_DOCK_TO_TAB)
		{
			for (pos = lstControlBars.GetHeadPosition(); pos != NULL;)
			{
				CDockablePane* pNextBar = DYNAMIC_DOWNCAST(CDockablePane, lstControlBars.GetNext(pos));
				ASSERT_VALID(pNextBar);

				AddRemovePaneFromGlobalList(pNextBar, FALSE);
				pNextBar->AttachToTabWnd(pTargetDockBar, dockMethod);
			}

			ShowWindow(SW_HIDE);
			MoveWindow(GetRecentFloatingRect());
			CPaneFrameWnd::OnCancelMode();
			SendMessage(AFX_WM_CHECKEMPTYMINIFRAME);
			return TRUE;
		}
		else
		{
			return TRUE;
		}
	}
	else // dock to frame window - need to create a new default slider
	{
		BOOL bOuterEdge = FALSE;
		if (!pDockManager->IsPointNearDockSite(pt, dwAlignment, bOuterEdge))
		{
			return TRUE;
		}

		CPaneDivider* pSlider = CDockablePane::CreateDefaultPaneDivider(dwAlignment, GetParent());

		if (pSlider == NULL)
		{
			return TRUE;
		}

		pSlider->SetPaneAlignment(dwAlignment);

		CRect rectContainer;
		m_barContainerManager.GetWindowRect(rectContainer);
		pDockManager->AdjustRectToClientArea(rectContainer, dwAlignment);
		HDWP hdwp = NULL;
		m_barContainerManager.ResizePaneContainers(rectContainer, hdwp);

		if (bOuterEdge)
		{
			// register slider with the dock manager
			pDockManager->AddPane(pSlider, !bOuterEdge, FALSE, bOuterEdge);
			pSlider->AddPaneContainer(m_barContainerManager, bOuterEdge);
		}
		else
		{
			pSlider->AddPaneContainer(m_barContainerManager, FALSE);
			pDockManager->AddPane(pSlider);
		}
	}

	// FINALLY destroy the frame - // all its bars should have been docked(and therefore removed)
	HWND hwndSave = m_hWnd;
	SendMessage(AFX_WM_CHECKEMPTYMINIFRAME);

	if (IsWindow(hwndSave))
	{
		MoveWindow(m_rectRecentFloatingRect);
	}
	else
	{
		return FALSE;
	}

	if (pFirstDockedBar != NULL && dockMethod == DM_MOUSE)
	{
		pFirstDockedBar->ScreenToClient(&pt);
		if (pFirstDockedBar->IsKindOf(RUNTIME_CLASS(CDockablePane)))
		{
			((CDockablePane*)pFirstDockedBar)->EnterDragMode(FALSE);
		}
		else
		{
			pFirstDockedBar->SendMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(pt.x, pt.y));
		}
	}

	// adjust the docking layout
	if (pTargetDockBar != NULL)
	{
		pTargetDockBar->RecalcLayout();
	}
	else if (pFirstDockedBar != NULL)
	{
		CDockablePane* pDockingBar = DYNAMIC_DOWNCAST(CDockablePane, pFirstDockedBar);
		if (pDockingBar != NULL)
		{
			pDockingBar->AdjustDockingLayout();
		}
	}

	OnSetRollUpTimer();

	for (pos = lstHandles.GetHeadPosition(); pos != NULL;)
	{
		HWND hwndNext = lstHandles.GetNext(pos);
		if (::IsWindow(hwndNext))
		{
			CDockablePane* pNextBar = DYNAMIC_DOWNCAST(CDockablePane, CWnd::FromHandle(hwndNext));
			if (pNextBar != NULL)
			{
				pNextBar->OnAfterDockFromMiniFrame();
			}
		}
	}

	return FALSE;
}

BOOL CMultiPaneFrameWnd::SaveState(LPCTSTR lpszProfileName, UINT uiID)
{
	ASSERT_VALID(this);
	CObList& lstControlBars = m_barContainerManager.m_lstControlBars;
	for (POSITION pos = lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CDockablePane* pNextBar = DYNAMIC_DOWNCAST(CDockablePane, lstControlBars.GetNext(pos));
		ASSERT_VALID(pNextBar);

		pNextBar->SaveState(lpszProfileName, uiID);
	}
	return TRUE;
}

BOOL CMultiPaneFrameWnd::LoadState(LPCTSTR lpszProfileName, UINT uiID)
{
	ASSERT_VALID(this);
	CObList& lstControlBars = m_barContainerManager.m_lstControlBars;
	for (POSITION pos = lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CDockablePane* pNextBar = DYNAMIC_DOWNCAST(CDockablePane, lstControlBars.GetNext(pos));
		ASSERT_VALID(pNextBar);

		pNextBar->LoadState(lpszProfileName, uiID);
	}
	return TRUE;
}

void CMultiPaneFrameWnd::Serialize(CArchive& ar)
{
	ASSERT_VALID(this);
	CPaneFrameWnd::Serialize(ar);
	m_barContainerManager.Serialize(ar);
}

void CMultiPaneFrameWnd::SetDockState(CDockingManager* pDockManager)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDockManager);

	CObList lstBarsToRemove;
	if (!m_barContainerManager.IsEmpty())
	{
		// float each control bar, reparent it and set its window position
		CObList& lstControlBars = m_barContainerManager.m_lstControlBars;
		POSITION pos = NULL;

		for (pos = lstControlBars.GetHeadPosition(); pos != NULL;)
		{
			CDockablePane* pNextBar = DYNAMIC_DOWNCAST(CDockablePane, lstControlBars.GetNext(pos));
			ASSERT_VALID(pNextBar);

			if (pNextBar->IsKindOf(RUNTIME_CLASS(CBaseTabbedPane)))
			{
				BOOL bLeftBar = FALSE;
				CPaneContainer* pContainer = m_barContainerManager.FindPaneContainer(pNextBar, bLeftBar);

				ENSURE(pContainer != NULL);

				CList<UINT, UINT>* pListBarIDs = pContainer->GetAssociatedSiblingPaneIDs(pNextBar);

				if (pListBarIDs != NULL)
				{
					for (POSITION posNext = pListBarIDs->GetHeadPosition(); posNext != NULL;)
					{
						UINT nIDNext = pListBarIDs->GetNext(posNext);
						CBasePane* pBarToAttach = pDockManager->FindPaneByID(nIDNext, TRUE);

						if (pBarToAttach == NULL)
						{
							continue;
						}

						if (pBarToAttach->IsKindOf(RUNTIME_CLASS(CDockablePane)) && ((CDockablePane*)pBarToAttach)->IsAutoHideMode())
						{
							((CDockablePane*)pBarToAttach)->SetAutoHideMode(FALSE, CBRS_ALIGN_ANY);
						}

						if (pBarToAttach->IsTabbed())
						{
							CMFCBaseTabCtrl* pTabWnd = (CMFCBaseTabCtrl*) pBarToAttach->GetParent();
							CBaseTabbedPane* pTabBar = (CBaseTabbedPane*)pTabWnd->GetParent();
							ASSERT_VALID(pTabBar);

							pBarToAttach->SetParent(this);
							pTabBar->RemovePane(pBarToAttach);
						}
						else
						{
							// float this bar in case if was docked somewhere else
							pBarToAttach->FloatPane(CRect(0, 0, 10, 10), DM_SHOW, false);
						}
						CPaneFrameWnd* pParentMiniFrame = pBarToAttach->GetParentMiniFrame();

						if (pParentMiniFrame != NULL && pParentMiniFrame != this)
						{
							pParentMiniFrame->RemovePane(pBarToAttach);
						}

						((CDockablePane*) pBarToAttach)->AttachToTabWnd(pNextBar, DM_UNKNOWN, FALSE);

						if (pParentMiniFrame != NULL)
						{
							pParentMiniFrame->PostMessage(AFX_WM_CHECKEMPTYMINIFRAME);
						}
					}
				}

				if (((CBaseTabbedPane*)pNextBar)->GetTabsNum() == 0)
				{
					lstBarsToRemove.AddTail(pNextBar);
				}
				else
				{
					((CBaseTabbedPane*)pNextBar)->ApplyRestoredTabInfo();
					pNextBar->RecalcLayout();
				}
				continue;
			}

			if (pNextBar->IsTabbed())
			{
				CMFCBaseTabCtrl* pTabWnd = (CMFCBaseTabCtrl*) pNextBar->GetParent();
				CBaseTabbedPane* pTabBar = (CBaseTabbedPane*) pTabWnd->GetParent();
				ASSERT_VALID(pTabBar);
				// set belong to any parent
				pNextBar->SetParent(GetParent());
				pTabBar->RemovePane(pNextBar);
				if (pNextBar->IsKindOf(RUNTIME_CLASS(CDockablePane)))
				{
					((CDockablePane*) pNextBar)->EnableGripper(TRUE);
				}

				pNextBar->ShowWindow(SW_SHOW);
			}

			if (pNextBar->IsAutoHideMode())
			{
				pNextBar->SetAutoHideMode(FALSE, CBRS_ALIGN_ANY);
			}

			CRect rectDummy;
			pNextBar->GetWindowRect(rectDummy);
			pNextBar->FloatPane(rectDummy, DM_SHOW, false);

			CPaneFrameWnd* pParentMiniFrame = pNextBar->GetParentMiniFrame();
			if (pParentMiniFrame != NULL)
			{
				pNextBar->SetParent(this);
				pParentMiniFrame->RemovePane(pNextBar);

				CRect rect = pNextBar->m_rectSavedDockedRect;

				pNextBar->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_FRAMECHANGED  | SWP_NOACTIVATE);
			}
		}

		for (pos = lstBarsToRemove.GetHeadPosition(); pos != NULL;)
		{
			CDockablePane* pNextBar = DYNAMIC_DOWNCAST(CDockablePane, lstBarsToRemove.GetNext(pos));
			RemovePane(pNextBar);
			pNextBar->DestroyWindow();
		}

		// retake the list
		CObList& lstModifiedControlBars = m_barContainerManager.m_lstControlBars;

		if (lstModifiedControlBars.IsEmpty())
		{
			SendMessage(AFX_WM_CHECKEMPTYMINIFRAME);
			return;
		}

		for (pos = lstModifiedControlBars.GetHeadPosition(); pos != NULL;)
		{
			CDockablePane* pNextBar = DYNAMIC_DOWNCAST(CDockablePane, lstModifiedControlBars.GetNext(pos));
			ASSERT_VALID(pNextBar);

			BOOL bShow = TRUE;
			if (pNextBar->IsKindOf(RUNTIME_CLASS(CPane)))
			{
				bShow = !(((CPane *)pNextBar)->m_bRecentFloatingState);
			}

			if (bShow)
			{
				pNextBar->ShowPane(pNextBar->GetRecentVisibleState(), FALSE, FALSE);
			}
			else
			{
				SetDelayShow(TRUE);
			}

			AddRemovePaneFromGlobalList(pNextBar, TRUE);
		}

		// set embedded bar to the first bar in the container
		CBasePane* pEmbeddedBar = DYNAMIC_DOWNCAST(CBasePane, lstControlBars.GetHead());
		if (pEmbeddedBar != NULL)
		{
			if (lstControlBars.GetCount() > 1)
			{
				m_hEmbeddedBar = pEmbeddedBar->GetSafeHwnd();
			}
			else
			{
				CString strText;
				pEmbeddedBar->GetWindowText(strText);
				SetWindowText(strText);

				SetIcon(pEmbeddedBar->GetIcon(FALSE), FALSE);
				SetIcon(pEmbeddedBar->GetIcon(TRUE), TRUE);
			}
		}
		OnSetRollUpTimer();
		SetCaptionButtons(m_dwCaptionButtons);
		OnPaneRecalcLayout();
		return;
	}

	// if we're here the miniframe is empty and should be destroyed
	PostMessage(AFX_WM_CHECKEMPTYMINIFRAME);
}

void CMultiPaneFrameWnd::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	CSize sizeBase;

	m_barContainerManager.GetMinSize(sizeBase);
	CalcMinSize(sizeBase, lpMMI);

	CWnd::OnGetMinMaxInfo(lpMMI);
}

void CMultiPaneFrameWnd::OnShowPane(CDockablePane* pBar, BOOL bShow)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);

	m_barContainerManager.OnShowPane(pBar, bShow);

	if (bShow)
	{
		ShowWindow(SW_SHOWNOACTIVATE);
		OnSetRollUpTimer();
	}
	else if (!m_barContainerManager.IsRootPaneContainerVisible())
	{
		ShowWindow(SW_HIDE);
		OnKillRollUpTimer();
	}

	CheckGripperVisibility();

	OnPaneRecalcLayout();

	// redraw caption to reflect the number of visible bars and set the recent pos
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE);
}

void CMultiPaneFrameWnd::CloseMiniFrame()
{
	if (OnCloseMiniFrame())
	{
		ShowWindow(SW_HIDE);
		m_barContainerManager.HideAll();
	}
}

void CMultiPaneFrameWnd::CheckGripperVisibility()
{
	if (IsWindowVisible())
	{
		int nVisibleCount = m_barContainerManager.GetVisiblePaneCount();

		if (nVisibleCount == 1) // take off caption from this bar
		{
			CDockablePane* pVisibleBar = DYNAMIC_DOWNCAST(CDockablePane, m_barContainerManager.GetFirstVisiblePane());

			if (pVisibleBar != NULL)
			{
				pVisibleBar->EnableGripper(FALSE);
			}
		}
		else
		{
			m_barContainerManager.EnableGrippers(TRUE);
		}
	}
}

CString CMultiPaneFrameWnd::GetCaptionText()
{
	CString strCaptionText;
	if (m_barContainerManager.GetVisiblePaneCount() == 1)
	{
		CWnd* pVisibleBar = DYNAMIC_DOWNCAST(CWnd, m_barContainerManager.GetFirstVisiblePane());

		if (pVisibleBar != NULL)
		{
			pVisibleBar->GetWindowText(strCaptionText);
		}
	}

	return strCaptionText;
}

void CMultiPaneFrameWnd::OnLButtonDblClk(UINT /*nFlags*/, CPoint /*point*/)
{
	OnDockToRecentPos();
}

void CMultiPaneFrameWnd::OnDockToRecentPos()
{
	CDockingManager* pDockManager = m_pDockManager != NULL ? m_pDockManager : afxGlobalUtils.GetDockingManager(this);

	CObList lstControlBars;
	m_barContainerManager.AddPanesToList(&lstControlBars, NULL);

	POSITION pos = NULL;

	for (pos = lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CDockablePane* pNextBar = DYNAMIC_DOWNCAST(CDockablePane, lstControlBars.GetNext(pos));
		ASSERT_VALID(pNextBar);

		pNextBar->StoreRecentDockSiteInfo();
	}

	for (pos = lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CDockablePane* pNextBar = DYNAMIC_DOWNCAST(CDockablePane, lstControlBars.GetNext(pos));
		ASSERT_VALID(pNextBar);

		AddRemovePaneFromGlobalList(pNextBar, FALSE);
		pNextBar->DockPane(pNextBar, NULL, DM_DBL_CLICK);
	}

	afxGlobalUtils.ForceAdjustLayout(pDockManager);

	SendMessage(AFX_WM_CHECKEMPTYMINIFRAME);
}

void CMultiPaneFrameWnd::SaveRecentFloatingState()
{
	GetWindowRect(m_rectRecentFloatingRect);
	CObList lstControlBars;
	m_barContainerManager.AddPanesToList(&lstControlBars, NULL);
	for (POSITION pos = lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CDockablePane* pNextBar = DYNAMIC_DOWNCAST(CDockablePane, lstControlBars.GetNext(pos));
		ASSERT_VALID(pNextBar);

		pNextBar->m_recentDockInfo.m_rectRecentFloatingRect = m_rectRecentFloatingRect;
	}
}

void CMultiPaneFrameWnd::StoreRecentDockSiteInfo(CPane* pBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);
	ASSERT_KINDOF(CDockablePane, pBar);

	CDockablePane* pDockingBar = DYNAMIC_DOWNCAST(CDockablePane, pBar);

	if (pDockingBar == NULL)
	{
		return;
	}

	BOOL bLeftBar = TRUE;
	CPaneContainer* pRecentContainer = m_barContainerManager.FindPaneContainer(pDockingBar, bLeftBar);

	pDockingBar->m_recentDockInfo.StoreDockInfo(pRecentContainer);
}

void CMultiPaneFrameWnd::StoreRecentTabRelatedInfo(CDockablePane* pDockingBar, CDockablePane* pTabbedBar)
{
	BOOL bLeftBar = TRUE;
	CPaneContainer* pRecentContainer = m_barContainerManager.FindPaneContainer(pTabbedBar, bLeftBar);

	pDockingBar->m_recentDockInfo.StoreDockInfo(pRecentContainer, pTabbedBar);
}

void CMultiPaneFrameWnd::DockRecentPaneToMainFrame(CDockablePane* pBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);
	AddRemovePaneFromGlobalList(pBar, FALSE);
	pBar->DockPane(pBar, NULL, DM_DBL_CLICK);
}

LRESULT CMultiPaneFrameWnd::OnCheckEmptyState(WPARAM, LPARAM)
{
	if (m_barContainerManager.m_pRootContainer != NULL)
	{
		m_barContainerManager.m_pRootContainer->ReleaseEmptyPaneContainer();
	}
	if (m_barContainerManager.GetNodeCount() == 0 || m_barContainerManager.GetNodeCount() == 1 && m_barContainerManager.m_pRootContainer != NULL && m_barContainerManager.m_pRootContainer->GetRefCount() == 0 && m_barContainerManager.m_pRootContainer->IsEmpty())
	{
		CPaneFrameWnd::OnCancelMode();
		DestroyWindow();
	}
	else if (m_barContainerManager.GetVisiblePaneCount() == 0)
	{
		ShowWindow(SW_HIDE);
		CPaneFrameWnd::OnCancelMode();
	}
	return 0;
}

BOOL CMultiPaneFrameWnd::InsertPane(CBasePane* pControlBar, CBasePane* /*pTarget*/, BOOL /*bAfter*/)
{
	AddRemovePaneFromGlobalList(pControlBar, TRUE);
	return TRUE;
}

void CMultiPaneFrameWnd::CalcExpectedDockedRect(CWnd* pWndToDock, CPoint ptMouse, CRect& rectResult, BOOL& bDrawTab, CDockablePane** ppTargetBar)
{
	CGlobalUtils globalUtilsLocal;
	if (m_bRolledUp)
	{
		// can't dock on rolled up miniframe
		bDrawTab = FALSE;
		rectResult.SetRectEmpty();
		return;
	}
	globalUtilsLocal.CalcExpectedDockedRect(m_barContainerManager, pWndToDock, ptMouse, rectResult, bDrawTab, ppTargetBar);
}

CBasePane* CMultiPaneFrameWnd::PaneFromPoint(CPoint point, int nSensitivity, BOOL bCheckVisibility)
{
	if (bCheckVisibility && !IsWindowVisible())
	{
		return NULL;
	}
	BOOL bTabArea = FALSE;
	BOOL bCaption = FALSE;
	return m_barContainerManager.PaneFromPoint(point, nSensitivity, TRUE, bTabArea, bCaption);
}

BOOL CMultiPaneFrameWnd::CanBeDockedToPane(const CDockablePane* pDockingBar) const
{
	for (POSITION pos = m_barContainerManager.m_lstControlBars.GetHeadPosition();
		pos != NULL;)
	{
		CDockablePane* pNextBar = DYNAMIC_DOWNCAST(CDockablePane, m_barContainerManager.m_lstControlBars.GetNext(pos));
		ASSERT_VALID(pNextBar);

		if (pDockingBar->CanAcceptPane(pNextBar) && pNextBar->CanAcceptPane(pDockingBar))
		{
			return TRUE;
		}
	}

	return FALSE;
}

LRESULT CMultiPaneFrameWnd::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM)
{
	CPaneFrameWnd::OnIdleUpdateCmdUI(wParam, 0);
	return 0L;
}

void CMultiPaneFrameWnd::ReplacePane(CBasePane* pBarOrg, CBasePane* pBarReplaceWith)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBarOrg);
	ASSERT_VALID(pBarReplaceWith);
	ASSERT_KINDOF(CDockablePane, pBarReplaceWith);

	m_barContainerManager.ReplacePane((CDockablePane*) pBarOrg, (CDockablePane*) pBarReplaceWith);
	OnSetRollUpTimer();
}

CWnd* CMultiPaneFrameWnd::GetPane() const
{
	CWnd* pWnd = CPaneFrameWnd::GetPane();
	if (pWnd == NULL)
	{
		pWnd = GetFirstVisiblePane();
		if (pWnd == NULL)
		{
			pWnd = m_barContainerManager.GetFirstPane();
		}
	}

	return pWnd;
}

void CMultiPaneFrameWnd::RemoveNonValidPanes()
{
	m_barContainerManager.RemoveNonValidPanes();
}

void CMultiPaneFrameWnd::OnSetFocus(CWnd* /*pOldWnd*/)
{
	CBasePane* pFirstBar = DYNAMIC_DOWNCAST(CBasePane, GetFirstVisiblePane());
	if (m_hWndLastFocused == NULL)
	{
		if (pFirstBar != NULL && ::IsWindow(pFirstBar->GetSafeHwnd()) && pFirstBar->CanFocus())
		{
			pFirstBar->SetFocus();
		}
	}
	else
	{
		CDockablePane* pWnd = DYNAMIC_DOWNCAST(CDockablePane, CWnd::FromHandlePermanent(m_hWndLastFocused));
		CPaneContainer* pContainer = NULL;
		if (pWnd != NULL)
		{
			BOOL bLeftBar;
			pContainer = m_barContainerManager.FindPaneContainer(pWnd, bLeftBar);
		}

		if (pContainer != NULL && ::IsWindow(pWnd->GetSafeHwnd()))
		{
			pWnd->SetFocus();
		}
		else if (pFirstBar != NULL && ::IsWindow(pFirstBar->GetSafeHwnd()))
		{
			pFirstBar->SetFocus();
		}
	}

	if (GetParentFrame() != NULL)
	{
		GetParentFrame()->SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
}
void CMultiPaneFrameWnd::ConvertToTabbedDocument()
{
	CObList lstControlBars;
	m_barContainerManager.AddPanesToList(&lstControlBars, NULL);
	for (POSITION pos = lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CDockablePane* pNextBar = DYNAMIC_DOWNCAST(CDockablePane, lstControlBars.GetNext(pos));
		ASSERT_VALID(pNextBar);
		pNextBar->ConvertToTabbedDocument(FALSE);
	}
	PostMessage(AFX_WM_CHECKEMPTYMINIFRAME);
}



