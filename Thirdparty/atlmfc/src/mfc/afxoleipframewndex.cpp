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
#include "afxoleipframewndex.h"
#include "afxmenubar.h"
#include "afxpaneframewnd.h"
#include "afxpopupmenu.h"
#include "afxusertoolsmanager.h"
#include "afxpreviewviewex.h"
#include "afxolecntrframewndex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// COleIPFrameWndEx

IMPLEMENT_DYNCREATE(COleIPFrameWndEx, COleIPFrameWnd)

#pragma warning(disable : 4355)

COleIPFrameWndEx::COleIPFrameWndEx() : m_Impl(this), m_bContextHelp(FALSE), m_hwndLastTopLevelFrame(NULL)
{
	m_dockManager.Create(this);
}

#pragma warning(default : 4355)

COleIPFrameWndEx::~COleIPFrameWndEx()
{
}

//{{AFX_MSG_MAP(COleIPFrameWndEx)
BEGIN_MESSAGE_MAP(COleIPFrameWndEx, COleIPFrameWnd)
	ON_WM_MENUCHAR()
	ON_WM_ACTIVATE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_IDLEUPDATECMDUI, &COleIPFrameWndEx::OnIdleUpdateCmdUI)
	ON_UPDATE_COMMAND_UI(ID_VIEW_STATUS_BAR, &COleIPFrameWndEx::OnUpdatePaneMenu)
	ON_COMMAND_EX(ID_VIEW_STATUS_BAR, &COleIPFrameWndEx::OnPaneCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOOLBAR, &COleIPFrameWndEx::OnUpdatePaneMenu)
	ON_COMMAND_EX(ID_VIEW_TOOLBAR, &COleIPFrameWndEx::OnPaneCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_REBAR, &COleIPFrameWndEx::OnUpdatePaneMenu)
	ON_COMMAND_EX(ID_VIEW_REBAR, &COleIPFrameWndEx::OnPaneCheck)
	ON_REGISTERED_MESSAGE(AFX_WM_TOOLBARMENU, &COleIPFrameWndEx::OnToolbarContextMenu)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &COleIPFrameWndEx::OnToolbarCreateNew)
	ON_REGISTERED_MESSAGE(AFX_WM_DELETETOOLBAR, &COleIPFrameWndEx::OnToolbarDelete)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// COleIPFrameWndEx message handlers

LRESULT COleIPFrameWndEx::OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu)
{
	if (m_Impl.OnMenuChar(nChar))
	{
		return MAKELPARAM(MNC_EXECUTE, -1);
	}

	return CFrameWnd::OnMenuChar(nChar, nFlags, pMenu);
}

afx_msg LRESULT COleIPFrameWndEx::OnSetMenu(WPARAM wp, LPARAM lp)
{
	OnSetMenu((HMENU) wp);
	return DefWindowProc(WM_MDISETMENU, NULL, lp);
}

BOOL COleIPFrameWndEx::OnSetMenu(HMENU hmenu)
{
	if (m_Impl.m_pMenuBar != NULL)
	{
		m_Impl.m_pMenuBar->CreateFromMenu
			(hmenu == NULL ? m_Impl.m_hDefaultMenu : hmenu);
		return TRUE;
	}

	return FALSE;
}

BOOL COleIPFrameWndEx::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		if (!CFrameImpl::IsHelpKey(pMsg) && m_Impl.ProcessKeyboard((int) pMsg->wParam))
		{
			return TRUE;
		}
		break;

	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
		{
			CPoint pt(AFX_GET_X_LPARAM(pMsg->lParam), AFX_GET_Y_LPARAM(pMsg->lParam));
			CWnd* pWnd = CWnd::FromHandle(pMsg->hwnd);

			if (pWnd != NULL && ::IsWindow(pMsg->hwnd))
			{
				pWnd->ClientToScreen(&pt);
			}

			if (m_Impl.ProcessMouseClick(pMsg->message, pt, pMsg->hwnd))
			{
				return TRUE;
			}

			if (!::IsWindow(pMsg->hwnd))
			{
				return TRUE;
			}
		}
		break;

	case WM_NCLBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCRBUTTONDOWN:
	case WM_NCRBUTTONUP:
	case WM_NCMBUTTONDOWN:
	case WM_NCMBUTTONUP:
		if (m_Impl.ProcessMouseClick(pMsg->message, CPoint(AFX_GET_X_LPARAM(pMsg->lParam), AFX_GET_Y_LPARAM(pMsg->lParam)), pMsg->hwnd))
		{
			return TRUE;
		}
		break;

	case WM_MOUSEMOVE:
		{
			CPoint pt(AFX_GET_X_LPARAM(pMsg->lParam), AFX_GET_Y_LPARAM(pMsg->lParam));
			CWnd* pWnd = CWnd::FromHandle(pMsg->hwnd);

			if (pWnd != NULL)
			{
				pWnd->ClientToScreen(&pt);
			}

			if (m_Impl.ProcessMouseMove(pt))
			{
				return TRUE;
			}
		}
	}

	return CFrameWnd::PreTranslateMessage(pMsg);
}

BOOL COleIPFrameWndEx::ShowPopupMenu(CMFCPopupMenu* pMenuPopup)
{
	if (!m_Impl.OnShowPopupMenu(pMenuPopup, this))
	{
		return FALSE;
	}

	if (pMenuPopup != NULL && pMenuPopup->m_bShown)
	{
		return TRUE;
	}

	return OnShowPopupMenu(pMenuPopup);
}

void COleIPFrameWndEx::OnClosePopupMenu(CMFCPopupMenu* pMenuPopup)
{
	if (CMFCPopupMenu::m_pActivePopupMenu == pMenuPopup)
	{
		CMFCPopupMenu::m_pActivePopupMenu = NULL;
	}

	m_dockManager.OnClosePopupMenu();
}

BOOL COleIPFrameWndEx::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (HIWORD(wParam) == 1)
	{
		UINT uiCmd = LOWORD(wParam);

		CMFCToolBar::AddCommandUsage(uiCmd);

		// Simulate ESC keystroke...
		if (m_Impl.ProcessKeyboard(VK_ESCAPE))
		{
			return TRUE;
		}

		if (afxUserToolsManager != NULL && afxUserToolsManager->InvokeTool(uiCmd))
		{
			return TRUE;
		}
	}

	if (!CMFCToolBar::IsCustomizeMode())
	{
		return CFrameWnd::OnCommand(wParam, lParam);
	}

	return FALSE;
}

BOOL COleIPFrameWndEx::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext)
{
	m_Impl.m_nIDDefaultResource = nIDResource;
	m_Impl.LoadLargeIconsState();

	return CFrameWnd::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext);
}

void COleIPFrameWndEx::HtmlHelp(DWORD_PTR dwData, UINT nCmd)
{
	if (dwData > 0 || !m_bContextHelp)
	{
		CFrameWnd::HtmlHelp(dwData, nCmd);
	}
	else
	{
		OnContextHelp();
	}
}

void COleIPFrameWndEx::WinHelp(DWORD dwData, UINT nCmd)
{
	if (dwData > 0 || !m_bContextHelp)
	{
		CFrameWnd::WinHelp(dwData, nCmd);
	}
	else
	{
		OnContextHelp();
	}
}

void COleIPFrameWndEx::OnContextHelp()
{
	m_bContextHelp = TRUE;

	if (!m_bHelpMode && CanEnterHelpMode())
	{
		CMFCToolBar::SetHelpMode();
	}

	CFrameWnd::OnContextHelp();

	if (!m_bHelpMode)
	{
		CMFCToolBar::SetHelpMode(FALSE);
	}

	m_bContextHelp = FALSE;
}

LRESULT COleIPFrameWndEx::OnToolbarCreateNew(WPARAM,LPARAM lp)
{
	ENSURE(lp != NULL);
	return(LRESULT) m_Impl.CreateNewToolBar((LPCTSTR) lp);
}

LRESULT COleIPFrameWndEx::OnToolbarDelete(WPARAM,LPARAM lp)
{
	CMFCToolBar* pToolbar = (CMFCToolBar*) lp;
	ASSERT_VALID(pToolbar);

	return(LRESULT) m_Impl.DeleteToolBar(pToolbar);
}

void COleIPFrameWndEx::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	COleIPFrameWnd::OnActivate(nState, pWndOther, bMinimized);

	switch (nState)
	{
	case WA_CLICKACTIVE:
		UpdateWindow();
		break;

	case WA_INACTIVE:
		if (!CMFCToolBar::IsCustomizeMode())
		{
			m_Impl.DeactivateMenu();
		}

		if (CMFCPopupMenu::GetActiveMenu() != NULL)
		{
			CMFCPopupMenu::GetActiveMenu()->SendMessage(WM_CLOSE);
		}

		break;
	}

	if (nState == WA_INACTIVE)
	{
		if (g_pTopLevelFrame == this)
		{
			CFrameWnd* pTopLevelFrame = DYNAMIC_DOWNCAST(CFrameWnd, CWnd::FromHandlePermanent(m_hwndLastTopLevelFrame));

			AFXSetTopLevelFrame(pTopLevelFrame);
		}
	}
	else
	{
		m_hwndLastTopLevelFrame = g_pTopLevelFrame->GetSafeHwnd();
		g_pTopLevelFrame = this;
	}
}

void COleIPFrameWndEx::OnClose()
{
	m_Impl.OnCloseFrame();
	COleIPFrameWnd::OnClose();
}

void COleIPFrameWndEx::OnDestroy()
{
	if (CMFCPopupMenu::GetActiveMenu() != NULL)
	{
		CMFCPopupMenu::GetActiveMenu()->SendMessage(WM_CLOSE);
	}

	if (g_pTopLevelFrame == this)
	{
		CFrameWnd* pTopLevelFrame = DYNAMIC_DOWNCAST(CFrameWnd, CWnd::FromHandlePermanent(m_hwndLastTopLevelFrame));
		g_pTopLevelFrame = pTopLevelFrame;
	}

	m_Impl.DeactivateMenu();

	if (m_hAccelTable != NULL)
	{
		::DestroyAcceleratorTable(m_hAccelTable);
		m_hAccelTable = NULL;
	}

	m_dockManager.m_bEnableAdjustLayout = FALSE;

	CList<HWND, HWND> lstChildren;
	CWnd* pNextWnd = GetTopWindow();

	while (pNextWnd != NULL)
	{
		lstChildren.AddTail(pNextWnd->m_hWnd);
		pNextWnd = pNextWnd->GetNextWindow();
	}

	for (POSITION pos = lstChildren.GetHeadPosition(); pos != NULL;)
	{
		HWND hwndNext = lstChildren.GetNext(pos);
		if (IsWindow(hwndNext) && ::GetParent(hwndNext) == m_hWnd)
		{
			::DestroyWindow(hwndNext);
		}
	}
	COleIPFrameWnd::OnDestroy();
}

void COleIPFrameWndEx::AddDockSite()
{
	ASSERT_VALID(this);
}

BOOL COleIPFrameWndEx::AddPane(CBasePane* pControlBar, BOOL bTail)
{
	ASSERT_VALID(this);
	return m_dockManager.AddPane(pControlBar, bTail);
}

BOOL COleIPFrameWndEx::InsertPane(CBasePane* pControlBar, CBasePane* pTarget, BOOL bAfter)
{
	ASSERT_VALID(this);
	return m_dockManager.InsertPane(pControlBar, pTarget, bAfter);
}

void COleIPFrameWndEx::RemovePaneFromDockManager(CBasePane* pControlBar, BOOL bDestroy, BOOL bAdjustLayout, BOOL bAutoHide, CBasePane* pBarReplacement)
{
	ASSERT_VALID(this);
	m_dockManager.RemovePaneFromDockManager(pControlBar, bDestroy, bAdjustLayout, bAutoHide, pBarReplacement);
}

void COleIPFrameWndEx::DockPane(CBasePane* pBar, UINT nDockBarID, LPCRECT lpRect)
{
	ASSERT_VALID(this);
	m_dockManager.DockPane(pBar, nDockBarID, lpRect);
}

CBasePane* COleIPFrameWndEx::PaneFromPoint(CPoint point, int nSensitivity, bool bExactBar, CRuntimeClass* pRTCBarType) const
{
	ASSERT_VALID(this);
	return m_dockManager.PaneFromPoint(point, nSensitivity, bExactBar, pRTCBarType);
}

CBasePane* COleIPFrameWndEx::PaneFromPoint(CPoint point, int nSensitivity, DWORD& dwAlignment, CRuntimeClass* pRTCBarType) const
{
	ASSERT_VALID(this);
	return m_dockManager.PaneFromPoint(point, nSensitivity, dwAlignment, pRTCBarType);
}

BOOL COleIPFrameWndEx::IsPointNearDockSite(CPoint point, DWORD& dwBarAlignment, BOOL& bOuterEdge) const
{
	ASSERT_VALID(this);
	return m_dockManager.IsPointNearDockSite(point, dwBarAlignment, bOuterEdge);
}

void COleIPFrameWndEx::AdjustDockingLayout(HDWP hdwp)
{
	ASSERT_VALID(this);
	CWnd* pChildWnd = GetWindow(GW_CHILD);
	while (pChildWnd != NULL)
	{
		ASSERT_VALID(pChildWnd);
		if (!pChildWnd->IsKindOf(RUNTIME_CLASS(CBasePane)))
		{
			break;
		}
		pChildWnd = GetWindow(GW_HWNDNEXT);
	}

	m_dockManager.AdjustDockingLayout(hdwp);
}

BOOL COleIPFrameWndEx::OnMoveMiniFrame(CWnd* pFrame)
{
	ASSERT_VALID(this);
	return m_dockManager.OnMoveMiniFrame(pFrame);
}

BOOL COleIPFrameWndEx::EnableDocking(DWORD dwDockStyle)
{
	return m_dockManager.EnableDocking(dwDockStyle);
}

BOOL COleIPFrameWndEx::EnableAutoHidePanes(DWORD dwDockStyle)
{
	return m_dockManager.EnableAutoHidePanes(dwDockStyle);
}

BOOL COleIPFrameWndEx::PreCreateWindow(CREATESTRUCT& cs)
{
	m_Impl.SetDockingManager(&m_dockManager);
	return COleIPFrameWnd::PreCreateWindow(cs);
}

CBasePane* COleIPFrameWndEx::GetPane(UINT nID)
{
	ASSERT_VALID(this);

	CBasePane* pBar = m_dockManager.FindPaneByID(nID, TRUE);
	return pBar;
}

void COleIPFrameWndEx::ShowPane(CBasePane* pBar, BOOL bShow, BOOL bDelay, BOOL bActivate)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);

	pBar->ShowPane(bShow, bDelay, bActivate);
}

void COleIPFrameWndEx::OnUpdatePaneMenu(CCmdUI* pCmdUI)
{
	CBasePane* pBar = GetPane(pCmdUI->m_nID);
	if (pBar != NULL)
	{
		pCmdUI->SetCheck((pBar->GetStyle() & WS_VISIBLE) != 0);
		return;
	}

	pCmdUI->ContinueRouting();
}

BOOL COleIPFrameWndEx::OnPaneCheck(UINT nID)
{
	ASSERT_VALID(this);

	CBasePane* pBar = GetPane(nID);
	if (pBar != NULL)
	{
		ShowPane(pBar, (pBar->GetStyle() & WS_VISIBLE) == 0, FALSE, FALSE);
		return TRUE;
	}

	return FALSE;
}

BOOL COleIPFrameWndEx::DockPaneLeftOf(CPane* pBar, CPane* pLeftOf)
{
	return m_dockManager.DockPaneLeftOf(pBar, pLeftOf);
}

void COleIPFrameWndEx::OnSetPreviewMode(BOOL bPreview, CPrintPreviewState* pState)
{
	ASSERT_VALID(this);

	m_dockManager.SetPrintPreviewMode(bPreview, pState);
	DWORD dwSavedState = pState->dwStates;
	COleIPFrameWnd::OnSetPreviewMode(bPreview, pState);
	pState->dwStates = dwSavedState;
	RecalcLayout();
}

void COleIPFrameWndEx::RecalcLayout(BOOL bNotify)
{
	COleIPFrameWnd::RecalcLayout(bNotify);

	if (m_bInRecalcLayout)
		return;

	m_bInRecalcLayout = TRUE;

	m_dockManager.AdjustDockingLayout();
	m_dockManager.RecalcLayout(bNotify);

	CView* pView = GetActiveView();
	if (pView != NULL && pView->IsKindOf(RUNTIME_CLASS(CPreviewViewEx)) && m_dockManager.IsPrintPreviewValid())
	{
		CRect rectClient = m_dockManager.GetClientAreaBounds();
		pView->SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOZORDER  | SWP_NOACTIVATE);
	}

	m_bInRecalcLayout = FALSE;
}

LRESULT COleIPFrameWndEx::OnIdleUpdateCmdUI(WPARAM, LPARAM)
{
	COleIPFrameWnd::OnIdleUpdateCmdUI();
	m_dockManager.SendMessageToMiniFrames(WM_IDLEUPDATECMDUI);

	COleCntrFrameWndEx* pFrame = DYNAMIC_DOWNCAST(COleCntrFrameWndEx, m_pMainFrame);
	if (pFrame!= NULL)
	{
		pFrame->OnIdleUpdateCmdUI();
	}

	return 0L;
}

BOOL COleIPFrameWndEx::OnCreateControlBars(CFrameWnd* pWndFrame, CFrameWnd* pWndDoc)
{
	// Remove this if you use pWndDoc
	UNREFERENCED_PARAMETER(pWndDoc);

	COleCntrFrameWndEx* pNewFrame = DYNAMIC_DOWNCAST(COleCntrFrameWndEx, pWndFrame);

	if (pNewFrame == NULL)
	{
		ASSERT(m_pMainFrame == pWndFrame);
		pNewFrame = new COleCntrFrameWndEx(this);
		ASSERT_VALID(pNewFrame);

		HWND hwndFrame = m_pMainFrame->Detach();
		delete m_pMainFrame;
		m_pMainFrame = pNewFrame;

		m_pMainFrame->Attach(hwndFrame);
		pNewFrame->CreateDockingManager();
	}

	return TRUE;
}

BOOL COleIPFrameWndEx::OnShowPanes(BOOL bShow)
{
	ASSERT_VALID(this);
	BOOL bResult = m_dockManager.ShowPanes(bShow);
	AdjustDockingLayout();

	return bResult;
}

COleCntrFrameWndEx* COleIPFrameWndEx::GetContainerFrameWindow()
{

	COleCntrFrameWndEx* pNewFrame = DYNAMIC_DOWNCAST(COleCntrFrameWndEx, m_pMainFrame);
	ASSERT_VALID(pNewFrame);
	return pNewFrame;
}

LRESULT COleIPFrameWndEx::OnToolbarContextMenu(WPARAM,LPARAM)
{
	return 1l;
}

BOOL COleIPFrameWndEx::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (COleIPFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
	{
		return TRUE;
	}

	return m_dockManager.ProcessPaneContextMenuCommand(nID, nCode, pExtra, pHandlerInfo);
}



