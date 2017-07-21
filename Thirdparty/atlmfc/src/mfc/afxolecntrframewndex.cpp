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
#include "afxpreviewviewex.h"
#include "afxpaneframewnd.h"
#include "afxolecntrframewndex.h"
#include "afxoledocipframewndex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// COleCntrFrameWndEx

IMPLEMENT_DYNAMIC(COleCntrFrameWndEx, CFrameWnd)

COleCntrFrameWndEx::COleCntrFrameWndEx(COleIPFrameWnd* pInPlaceFrame) : COleCntrFrameWnd(pInPlaceFrame)
{
}

COleCntrFrameWndEx::~COleCntrFrameWndEx()
{
	POSITION pos = NULL;

	for (pos = m_dockManager.m_lstMiniFrames.GetHeadPosition(); pos != NULL;)
	{
		CPaneFrameWnd* pNextFrame = DYNAMIC_DOWNCAST(CPaneFrameWnd, m_dockManager.m_lstMiniFrames.GetNext(pos));
		if (pNextFrame != NULL)
		{
			pNextFrame->DestroyWindow();
		}
	}

	CList<HWND, HWND> lstChildren;
	CWnd* pNextWnd = GetTopWindow();
	while (pNextWnd != NULL)
	{
		lstChildren.AddTail(pNextWnd->m_hWnd);
		pNextWnd = pNextWnd->GetNextWindow();
	}

	for (pos = lstChildren.GetHeadPosition(); pos != NULL;)
	{
		HWND hwndNext = lstChildren.GetNext(pos);
		if (IsWindow(hwndNext) && ::GetParent(hwndNext) == m_hWnd)
		{
			::DestroyWindow(hwndNext);
		}
	}

	const CObList& list = CMFCToolBar::GetAllToolbars();
	CObList& afxAllToolBars = const_cast<CObList&>(list);

	for (pos = afxAllToolBars.GetHeadPosition(); pos != NULL;)
	{
		POSITION posSave = pos;

		CMFCToolBar* pToolBar = (CMFCToolBar*) afxAllToolBars.GetNext(pos);
		ENSURE(pToolBar != NULL);

		if (CWnd::FromHandlePermanent(pToolBar->m_hWnd) == NULL)
		{
			afxAllToolBars.RemoveAt(posSave);
		}
	}

}

//{{AFX_MSG_MAP(COleCntrFrameWndEx)
BEGIN_MESSAGE_MAP(COleCntrFrameWndEx, COleCntrFrameWnd)
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_MESSAGE_VOID(WM_IDLEUPDATECMDUI, COleCntrFrameWndEx::OnIdleUpdateCmdUI)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// COleCntrFrameWndEx message handlers

BOOL COleCntrFrameWndEx::DockPaneLeftOf(CPane* pBar, CPane* pLeftOf)
{
	m_dockManager.DockPaneLeftOf(pBar, pLeftOf);
	return TRUE;
}

void COleCntrFrameWndEx::OnSize(UINT nType, int cx, int cy)
{
	COleCntrFrameWnd::OnSize(nType, cx, cy);

	if (nType != SIZE_MINIMIZED)
	{
		AdjustDockingLayout();
	}
}

BOOL COleCntrFrameWndEx::PreCreateWindow(CREATESTRUCT& cs)
{
	m_dockManager.Create(this);

	return COleCntrFrameWnd::PreCreateWindow(cs);
}

void COleCntrFrameWndEx::AddDockSite()
{
	ASSERT_VALID(this);
}

BOOL COleCntrFrameWndEx::AddPane(CBasePane* pControlBar, BOOL bTail)
{
	ASSERT_VALID(this);
	return m_dockManager.AddPane(pControlBar, bTail);
}

BOOL COleCntrFrameWndEx::InsertPane(CBasePane* pControlBar, CBasePane* pTarget, BOOL bAfter)
{
	ASSERT_VALID(this);
	return m_dockManager.InsertPane(pControlBar, pTarget, bAfter);
}

void COleCntrFrameWndEx::RemovePaneFromDockManager(CBasePane* pControlBar, BOOL bDestroy, BOOL bAdjustLayout, BOOL bAutoHide, CBasePane* pBarReplacement)
{
	ASSERT_VALID(this);
	m_dockManager.RemovePaneFromDockManager(pControlBar, bDestroy, bAdjustLayout, bAutoHide, pBarReplacement);
	AdjustDockingLayout();
}

void COleCntrFrameWndEx::DockPane(CBasePane* pBar, UINT nDockBarID, LPCRECT lpRect)
{
	ASSERT_VALID(this);
	m_dockManager.DockPane(pBar, nDockBarID, lpRect);
	AdjustDockingLayout();
}

CBasePane* COleCntrFrameWndEx::GetPane(UINT nID)
{
	ASSERT_VALID(this);

	return m_dockManager.FindPaneByID(nID, TRUE);
}

void COleCntrFrameWndEx::ShowPane(CBasePane* pBar, BOOL bShow, BOOL bDelay, BOOL bActivate)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);

	pBar->ShowPane(bShow, bDelay, bActivate);
}

CBasePane* COleCntrFrameWndEx::PaneFromPoint(CPoint point, int nSensitivity, bool bExactBar, CRuntimeClass* pRTCBarType) const
{
	ASSERT_VALID(this);
	return m_dockManager.PaneFromPoint(point, nSensitivity, bExactBar, pRTCBarType);
}

CBasePane* COleCntrFrameWndEx::PaneFromPoint(CPoint point, int nSensitivity, DWORD& dwAlignment, CRuntimeClass* pRTCBarType) const
{
	ASSERT_VALID(this);
	return m_dockManager.PaneFromPoint(point, nSensitivity, dwAlignment, pRTCBarType);
}

BOOL COleCntrFrameWndEx::IsPointNearDockSite(CPoint point, DWORD& dwBarAlignment, BOOL& bOuterEdge) const
{
	ASSERT_VALID(this);
	return m_dockManager.IsPointNearDockSite(point, dwBarAlignment, bOuterEdge);
}

void COleCntrFrameWndEx::AdjustDockingLayout(HDWP /*hdwp*/)
{
	ASSERT_VALID(this);
	AdjustClientArea();
}

BOOL COleCntrFrameWndEx::OnMoveMiniFrame(CWnd* pFrame)
{
	ASSERT_VALID(this);
	return m_dockManager.OnMoveMiniFrame(pFrame);
}

BOOL COleCntrFrameWndEx::EnableDocking(DWORD dwDockStyle)
{
	return m_dockManager.EnableDocking(dwDockStyle);
}

BOOL COleCntrFrameWndEx::EnableAutoHidePanes(DWORD dwDockStyle)
{
	return m_dockManager.EnableAutoHidePanes(dwDockStyle);
}

void COleCntrFrameWndEx::RecalcLayout(BOOL bNotify)
{
	AdjustClientArea();
	m_dockManager.AdjustDockingLayout();
	m_dockManager.RecalcLayout(bNotify);

	CView* pView = GetActiveView();
	if (pView != NULL && pView->IsKindOf(RUNTIME_CLASS(CPreviewViewEx)) && m_dockManager.IsPrintPreviewValid())
	{
		CRect rectClient = m_dockManager.GetClientAreaBounds();
		pView->SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOZORDER  | SWP_NOACTIVATE);
	}

	m_pInPlaceFrame->RecalcLayout(bNotify);
}

void COleCntrFrameWndEx::OnSizing(UINT fwSide, LPRECT pRect)
{
	COleCntrFrameWnd::OnSizing(fwSide, pRect);

	CRect rect;
	GetWindowRect(rect);

	if (rect.Size() != CRect(pRect).Size())
	{
		AdjustDockingLayout();
	}
}

void COleCntrFrameWndEx::OnIdleUpdateCmdUI()
{
	COleCntrFrameWnd::OnIdleUpdateCmdUI();

	// update control bars
	m_dockManager.SendMessageToMiniFrames(WM_IDLEUPDATECMDUI);

	POSITION pos = m_dockManager.m_lstControlBars.GetHeadPosition();
	while (pos != NULL)
	{
		CBasePane* pBar = (CBasePane*)m_dockManager.m_lstControlBars.GetNext(pos);
		ENSURE(pBar != NULL);
		ASSERT_VALID(pBar);

		pBar->SendMessageToDescendants(WM_IDLEUPDATECMDUI, (WPARAM) TRUE);
	}
}

BOOL COleCntrFrameWndEx::OnShowPanes(BOOL bShow)
{
	ASSERT_VALID(this);
	BOOL bResult = m_dockManager.ShowPanes(bShow);
	AdjustDockingLayout();

	return bResult;
}

void COleCntrFrameWndEx::AdjustClientArea()
{
	COleServerDoc* pDoc = (COleServerDoc*)m_pInPlaceFrame->GetActiveDocument();

	if (pDoc != NULL )
	{
		ASSERT_VALID(pDoc);
		ASSERT_KINDOF(COleServerDoc, pDoc);
		COleDocIPFrameWndEx* pFrame = (COleDocIPFrameWndEx*)m_pInPlaceFrame;

		pDoc->OnResizeBorder(NULL, pFrame->m_lpFrame, TRUE);
	}
}



