// This MFC Library source code supports the Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Classes Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// http://msdn.microsoft.com/officeui.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

#include "stdafx.h"
#include "afxpriv.h"
#include "afxframewndex.h"
#include "afxmenubar.h"
#include "afxpopupmenu.h"
#include "afxpaneframewnd.h"
#include "afxusertoolsmanager.h"
#include "afxpreviewviewex.h"
#include "afxpanedivider.h"
#include "afxribbonbar.h"
#include "afxribbonstatusbar.h"
#include "afxvisualmanager.h"
#include "afxglobalutils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "imm32.lib")

/////////////////////////////////////////////////////////////////////////////
// CFrameWndEx

IMPLEMENT_DYNCREATE(CFrameWndEx, CFrameWnd)

#pragma warning(disable : 4355)

CFrameWndEx::CFrameWndEx() :
	m_Impl(this), m_bContextHelp(FALSE), m_bWasMaximized(FALSE), m_bIsMinimized(FALSE), m_pPrintPreviewFrame(NULL)
{
}

#pragma warning(default : 4355)

CFrameWndEx::~CFrameWndEx()
{
}

//{{AFX_MSG_MAP(CFrameWndEx)
BEGIN_MESSAGE_MAP(CFrameWndEx, CFrameWnd)
	ON_WM_MENUCHAR()
	ON_WM_ACTIVATE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_SIZING()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_NCPAINT()
	ON_WM_NCACTIVATE()
	ON_WM_CREATE()
	ON_WM_NCMOUSEMOVE()
	ON_WM_NCHITTEST()
	ON_WM_NCCALCSIZE()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_ACTIVATEAPP()
	ON_WM_SYSCOLORCHANGE()
	ON_MESSAGE(WM_IDLEUPDATECMDUI, &CFrameWndEx::OnIdleUpdateCmdUI)
	ON_MESSAGE(WM_SETTEXT, &CFrameWndEx::OnSetText)
	ON_MESSAGE(WM_DWMCOMPOSITIONCHANGED, &CFrameWndEx::OnDWMCompositionChanged)
	ON_UPDATE_COMMAND_UI(ID_VIEW_STATUS_BAR, &CFrameWndEx::OnUpdatePaneMenu)
	ON_COMMAND_EX(ID_VIEW_STATUS_BAR, &CFrameWndEx::OnPaneCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOOLBAR, &CFrameWndEx::OnUpdatePaneMenu)
	ON_COMMAND_EX(ID_VIEW_TOOLBAR, &CFrameWndEx::OnPaneCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_REBAR, &CFrameWndEx::OnUpdatePaneMenu)
	ON_COMMAND_EX(ID_VIEW_REBAR, &CFrameWndEx::OnPaneCheck)
	ON_REGISTERED_MESSAGE(AFX_WM_TOOLBARMENU, &CFrameWndEx::OnToolbarContextMenu)
	ON_REGISTERED_MESSAGE(AFX_WM_CHANGEVISUALMANAGER, &CFrameWndEx::OnChangeVisualManager)
	ON_REGISTERED_MESSAGE(AFX_WM_POSTSETPREVIEWFRAME, &CFrameWndEx::OnPostPreviewFrame)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CFrameWndEx::OnToolbarCreateNew)
	ON_REGISTERED_MESSAGE(AFX_WM_DELETETOOLBAR, &CFrameWndEx::OnToolbarDelete)
	ON_MESSAGE(WM_POWERBROADCAST, &OnPowerBroadcast)
	ON_MESSAGE(WM_EXITSIZEMOVE, &CFrameWndEx::OnExitSizeMove)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// CFrameWndEx message handlers

LRESULT CFrameWndEx::OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu)
{
	if (m_Impl.OnMenuChar(nChar))
	{
		return MAKELPARAM(MNC_EXECUTE, -1);
	}

	return CFrameWnd::OnMenuChar(nChar, nFlags, pMenu);
}

afx_msg LRESULT CFrameWndEx::OnSetMenu(WPARAM wp, LPARAM lp)
{
	OnSetMenu((HMENU) wp);
	return DefWindowProc(WM_MDISETMENU, NULL, lp);
}

BOOL CFrameWndEx::OnSetMenu(HMENU hmenu)
{
	if (m_Impl.m_pMenuBar != NULL)
	{
		m_Impl.m_pMenuBar->CreateFromMenu(hmenu == NULL ? m_Impl.m_hDefaultMenu : hmenu);
		return TRUE;
	}

	return FALSE;
}

BOOL CFrameWndEx::PreTranslateMessage(MSG* pMsg)
{
	BOOL bProcessAccel = TRUE;

	switch (pMsg->message)
	{
	case WM_SYSKEYDOWN:

		if (m_Impl.m_pRibbonBar != NULL && m_Impl.m_pRibbonBar->OnSysKeyDown(this, pMsg->wParam, pMsg->lParam))
		{
			return TRUE;
		}

	case WM_CONTEXTMENU:
		if (CMFCPopupMenu::m_pActivePopupMenu != NULL && ::IsWindow(CMFCPopupMenu::m_pActivePopupMenu->m_hWnd) && pMsg->wParam == VK_MENU)
		{
			CMFCPopupMenu::m_pActivePopupMenu->SendMessage(WM_CLOSE);
			return TRUE;
		}
		else if (m_Impl.ProcessKeyboard((int) pMsg->wParam))
		{
			return TRUE;
		}
		break;

	case WM_SYSKEYUP:
		{
			if (m_Impl.m_pRibbonBar != NULL && m_Impl.m_pRibbonBar->OnSysKeyUp(this, pMsg->wParam, pMsg->lParam))
			{
				return TRUE;
			}

			BOOL  isCtrlPressed = (0x8000 & GetKeyState(VK_CONTROL)) != 0;
			BOOL  isShiftPressed = (0x8000 & GetKeyState(VK_SHIFT)) != 0;

			HIMC hContext = ImmGetContext(m_hWnd);
			BOOL bIMEActive = ((hContext != NULL) && ImmGetOpenStatus(hContext));
			if (hContext != NULL)
			{
				ImmReleaseContext(m_hWnd, hContext);
			}

			if (m_Impl.m_pMenuBar != NULL && (pMsg->wParam == VK_MENU || (pMsg->wParam == VK_F10 && !isCtrlPressed && !isShiftPressed && !bIMEActive)))
			{
				if (m_Impl.m_pMenuBar == GetFocus())
				{
					SetFocus();
				}
				else
				{
					if ((pMsg->lParam &(1 << 29)) == 0)
					{
						m_Impl.m_pMenuBar->SetFocus();
					}
				}
				return TRUE;
			}

			if (CMFCPopupMenu::m_pActivePopupMenu != NULL && ::IsWindow(CMFCPopupMenu::m_pActivePopupMenu->m_hWnd))
			{
				return TRUE; // To prevent system menu opening
			}
		}
		break;

	case WM_KEYDOWN:
		// Pass keyboard action to the active menu:
		if (!CFrameImpl::IsHelpKey(pMsg) && m_Impl.ProcessKeyboard((int) pMsg->wParam, &bProcessAccel))
		{
			return TRUE;
		}

		if (pMsg->wParam == VK_ESCAPE)
		{
			if (IsFullScreen())
			{
				m_Impl.m_FullScreenMgr.RestoreState(this);
			}

			CSmartDockingManager* pSDManager = NULL;
			if ((pSDManager = m_dockManager.GetSmartDockingManagerPermanent()) != NULL && pSDManager->IsStarted())
			{
				pSDManager->CauseCancelMode();
			}

			CPaneDivider* pSlider = DYNAMIC_DOWNCAST(CPaneDivider, GetCapture());
			if (pSlider != NULL)
			{
				pSlider->SendMessage(WM_CANCELMODE);
				return TRUE;
			}
		}

		if (!bProcessAccel)
		{
			return FALSE;
		}
		break;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
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

	case WM_MOUSEWHEEL:
		if (m_Impl.ProcessMouseWheel(pMsg->wParam, pMsg->lParam))
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

BOOL CFrameWndEx::ShowPopupMenu(CMFCPopupMenu* pMenuPopup)
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

void CFrameWndEx::OnClosePopupMenu(CMFCPopupMenu* pMenuPopup)
{
	if (afxGlobalData.IsAccessibilitySupport() && pMenuPopup != NULL)
	{
		CMFCPopupMenu* pPopupParent = pMenuPopup->GetParentPopupMenu();
		CMFCToolBarMenuButton* pParentButton  = pMenuPopup->GetParentButton();

		if (pMenuPopup->IsEscClose() || pPopupParent != NULL || pParentButton == NULL)
		{
			::NotifyWinEvent(EVENT_SYSTEM_MENUPOPUPEND, pMenuPopup->GetSafeHwnd(), OBJID_WINDOW, CHILDID_SELF);
		}
		else
		{
			::NotifyWinEvent(EVENT_SYSTEM_MENUEND, pMenuPopup->GetSafeHwnd(), OBJID_WINDOW, CHILDID_SELF);
		}
	}

	if (CMFCPopupMenu::m_pActivePopupMenu == pMenuPopup)
	{
		CMFCPopupMenu::m_pActivePopupMenu = NULL;
	}

	m_dockManager.OnClosePopupMenu();
}

BOOL CFrameWndEx::OnDrawMenuImage(CDC* pDC, const CMFCToolBarMenuButton* pMenuButton, const CRect& rectImage)
{
	ASSERT_VALID(this);

	if (m_Impl.m_pRibbonBar != NULL)
	{
		ASSERT_VALID(m_Impl.m_pRibbonBar);
		return m_Impl.m_pRibbonBar->DrawMenuImage(pDC, pMenuButton, rectImage);
	}

	return FALSE;
}

BOOL CFrameWndEx::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (HIWORD(wParam) == 1)
	{
		UINT uiCmd = LOWORD(wParam);

		CMFCToolBar::AddCommandUsage(uiCmd);

		// Simmulate ESC keystroke...
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

BOOL CFrameWndEx::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext)
{
	m_Impl.m_nIDDefaultResource = nIDResource;
	m_Impl.LoadLargeIconsState();

	if (!CFrameWnd::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}

	m_Impl.OnLoadFrame();
	return TRUE;
}

void CFrameWndEx::OnClose()
{
	if (m_pPrintPreviewFrame != NULL)
	{
		m_pPrintPreviewFrame->SendMessage(WM_COMMAND, AFX_ID_PREVIEW_CLOSE);
		m_pPrintPreviewFrame = NULL;
		return;
	}

	// Deactivate OLE container first:
	COleClientItem* pActiveItem = GetInPlaceActiveItem();
	if (pActiveItem != NULL)
	{
		pActiveItem->Deactivate();
	}

	m_Impl.OnCloseFrame();
	CFrameWnd::OnClose();
}

BOOL CFrameWndEx::PreCreateWindow(CREATESTRUCT& cs)
{
	m_dockManager.Create(this);
	m_Impl.SetDockingManager(&m_dockManager);

	m_Impl.RestorePosition(cs);
	return CFrameWnd::PreCreateWindow(cs);
}

void CFrameWndEx::HtmlHelp(DWORD_PTR dwData, UINT nCmd)
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

void CFrameWndEx::WinHelp(DWORD dwData, UINT nCmd)
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

void CFrameWndEx::OnContextHelp()
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

LRESULT CFrameWndEx::OnToolbarCreateNew(WPARAM,LPARAM lp)
{
	ENSURE(lp != 0);
	return(LRESULT) m_Impl.CreateNewToolBar((LPCTSTR) lp);
}

LRESULT CFrameWndEx::OnToolbarDelete(WPARAM,LPARAM lp)
{
	ENSURE(lp != 0);

	CMFCToolBar* pToolbar = (CMFCToolBar*) lp;
	ASSERT_VALID(pToolbar);

	return(LRESULT) m_Impl.DeleteToolBar(pToolbar);
}

BOOL CFrameWndEx::DockPaneLeftOf(CPane* pBar, CPane* pLeftOf)
{
	return m_dockManager.DockPaneLeftOf(pBar, pLeftOf);
}

void CFrameWndEx::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CFrameWnd::OnActivate(nState, pWndOther, bMinimized);

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
		break;
	}
}

void CFrameWndEx::OnActivateApp(BOOL bActive, DWORD /*dwThreadID*/)
{
	m_dockManager.OnActivateFrame(bActive);
	m_Impl.OnActivateApp(bActive);
}

void CFrameWndEx::DelayUpdateFrameMenu(HMENU hMenuAlt)
{
	OnUpdateFrameMenu(hMenuAlt);
	CFrameWnd::DelayUpdateFrameMenu(hMenuAlt);
}

COleClientItem* CFrameWndEx::GetInPlaceActiveItem()
{
	CFrameWnd* pActiveFrame = GetActiveFrame();
	if (pActiveFrame == NULL)
	{
		return NULL;
	}

	ASSERT_VALID(pActiveFrame);

	CView* pView = pActiveFrame->GetActiveView();
	if (pView == NULL || pView->IsKindOf(RUNTIME_CLASS(CPreviewViewEx)))
	{
		return NULL;
	}

	ASSERT_VALID(pView);

	COleDocument* pDoc = DYNAMIC_DOWNCAST(COleDocument, pView->GetDocument());
	if (pDoc == NULL)
	{
		return NULL;
	}

	ASSERT_VALID(pDoc);
	return pDoc->GetInPlaceActiveItem(pView);
}

void CFrameWndEx::OnUpdateFrameMenu(HMENU hMenuAlt)
{
	CFrameWnd::OnUpdateFrameMenu(hMenuAlt);

	BOOL bIsMenuBar = m_Impl.m_pMenuBar != NULL && (m_Impl.m_pMenuBar->GetStyle() & WS_VISIBLE);
	BOOL bIsRibbon = FALSE;

	if (m_Impl.m_pRibbonBar != NULL && (m_Impl.m_pRibbonBar->GetStyle() & WS_VISIBLE))
	{
		bIsRibbon = TRUE;
	}

	if (bIsMenuBar || bIsRibbon)
	{
		COleClientItem* pActiveItem = GetInPlaceActiveItem();
		if (pActiveItem == NULL || pActiveItem->GetInPlaceWindow() == NULL)
		{
			SetMenu(NULL);
		}
		else
		{
			SetMenu(CMenu::FromHandle(hMenuAlt));
		}
	}
}

void CFrameWndEx::OnDestroy()
{
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

	CFrameImpl::RemoveFrame(this);
	CFrameWnd::OnDestroy();
}

void CFrameWndEx::AddDockSite()
{
	ASSERT_VALID(this);
}

BOOL CFrameWndEx::AddPane(CBasePane* pControlBar, BOOL bTail)
{
	ASSERT_VALID(this);

	CMFCRibbonBar* pRibbonBar = DYNAMIC_DOWNCAST(CMFCRibbonBar, pControlBar);
	if (pRibbonBar != NULL)
	{
		ASSERT_VALID(pRibbonBar);

		if (pRibbonBar->IsMainRibbonBar())
		{
			m_Impl.m_pRibbonBar = pRibbonBar;
		}
	}

	CMFCRibbonStatusBar* pRibbonStatusBar = DYNAMIC_DOWNCAST(CMFCRibbonStatusBar, pControlBar);
	if (pRibbonStatusBar != NULL)
	{
		ASSERT_VALID(pRibbonStatusBar);
		m_Impl.m_pRibbonStatusBar = pRibbonStatusBar;
	}

	return m_dockManager.AddPane(pControlBar, bTail);
}

BOOL CFrameWndEx::InsertPane(CBasePane* pControlBar, CBasePane* pTarget, BOOL bAfter)
{
	ASSERT_VALID(this);
	return m_dockManager.InsertPane(pControlBar, pTarget, bAfter);
}

void CFrameWndEx::RemovePaneFromDockManager(CBasePane* pControlBar, BOOL bDestroy, BOOL bAdjustLayout, BOOL bAutoHide, CBasePane* pBarReplacement)
{
	ASSERT_VALID(this);
	m_dockManager.RemovePaneFromDockManager(pControlBar, bDestroy, bAdjustLayout, bAutoHide, pBarReplacement);
}

void CFrameWndEx::DockPane(CBasePane* pBar, UINT nDockBarID, LPCRECT lpRect)
{
	ASSERT_VALID(this);
	m_dockManager.DockPane(pBar, nDockBarID, lpRect);
}

CBasePane* CFrameWndEx::PaneFromPoint(CPoint point, int nSensitivity, bool bExactBar, CRuntimeClass* pRTCBarType) const
{
	ASSERT_VALID(this);
	return m_dockManager.PaneFromPoint(point, nSensitivity, bExactBar, pRTCBarType);
}

CBasePane* CFrameWndEx::PaneFromPoint(CPoint point, int nSensitivity, DWORD& dwAlignment, CRuntimeClass* pRTCBarType) const
{
	ASSERT_VALID(this);
	return m_dockManager.PaneFromPoint(point, nSensitivity, dwAlignment, pRTCBarType);
}

BOOL CFrameWndEx::IsPointNearDockSite(CPoint point, DWORD& dwBarAlignment, BOOL& bOuterEdge) const
{
	ASSERT_VALID(this);
	return m_dockManager.IsPointNearDockSite(point, dwBarAlignment, bOuterEdge);
}

void CFrameWndEx::AdjustDockingLayout(HDWP hdwp)
{
	ASSERT_VALID(this);

	if (m_dockManager.IsInAdjustLayout())
	{
		return;
	}

	m_dockManager.AdjustDockingLayout(hdwp);

	AdjustClientArea();
	if (m_dockManager.IsOLEContainerMode())
	{
		RecalcLayout();
	}
}

void CFrameWndEx::AdjustClientArea()
{
	CWnd* pChildWnd = GetDlgItem(AFX_IDW_PANE_FIRST);
	if (pChildWnd != NULL)
	{
		CRect rectClientAreaBounds = m_dockManager.GetClientAreaBounds();

		rectClientAreaBounds.left += m_rectBorder.left;
		rectClientAreaBounds.top  += m_rectBorder.top;
		rectClientAreaBounds.right -= m_rectBorder.right;
		rectClientAreaBounds.bottom -= m_rectBorder.bottom;

		pChildWnd->CalcWindowRect(rectClientAreaBounds);

		if (!pChildWnd->IsKindOf(RUNTIME_CLASS(CSplitterWnd)))
		{
			pChildWnd->ModifyStyle(0, WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
		}
		else
		{
			pChildWnd->ModifyStyle(0, WS_CLIPSIBLINGS);
		}

		pChildWnd->SetWindowPos(&wndBottom, rectClientAreaBounds.left, rectClientAreaBounds.top, rectClientAreaBounds.Width(), rectClientAreaBounds.Height(), SWP_NOACTIVATE);
	}
}

BOOL CFrameWndEx::OnMoveMiniFrame(CWnd* pFrame)
{
	ASSERT_VALID(this);
	return m_dockManager.OnMoveMiniFrame(pFrame);
}

BOOL CFrameWndEx::EnableDocking(DWORD dwDockStyle)
{
	return m_dockManager.EnableDocking(dwDockStyle);
}

BOOL CFrameWndEx::EnableAutoHidePanes(DWORD dwDockStyle)
{
	return m_dockManager.EnableAutoHidePanes(dwDockStyle);
}

CBasePane* CFrameWndEx::GetPane(UINT nID)
{
	ASSERT_VALID(this);

	CBasePane* pBar = m_dockManager.FindPaneByID(nID, TRUE);
	return pBar;
}

void CFrameWndEx::ShowPane(CBasePane* pBar, BOOL bShow, BOOL bDelay, BOOL bActivate)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);

	pBar->ShowPane(bShow, bDelay, bActivate);
}

void CFrameWndEx::OnUpdatePaneMenu(CCmdUI* pCmdUI)
{
	CBasePane* pBar = GetPane(pCmdUI->m_nID);
	if (pBar != NULL)
	{
		pCmdUI->SetCheck((pBar->GetStyle() & WS_VISIBLE) != 0);
		return;
	}

	pCmdUI->ContinueRouting();
}

BOOL CFrameWndEx::OnPaneCheck(UINT nID)
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

void CFrameWndEx::OnSizing(UINT fwSide, LPRECT pRect)
{
	CFrameWnd::OnSizing(fwSide, pRect);

	AdjustDockingLayout();
}

void CFrameWndEx::RecalcLayout(BOOL bNotify)
{
	if (m_bInRecalcLayout)
		return;

	m_bInRecalcLayout = TRUE;

	BOOL bWasOleInPlaceActive = m_Impl.m_bIsOleInPlaceActive;
	m_Impl.m_bIsOleInPlaceActive = FALSE;

	COleClientItem* pActiveItem = GetInPlaceActiveItem();

	if (pActiveItem != NULL && pActiveItem->m_pInPlaceFrame != NULL && pActiveItem->GetItemState() == COleClientItem::activeUIState)
	{
		m_Impl.m_bIsOleInPlaceActive = TRUE;
		m_Impl.m_bHadCaption = (GetStyle() & WS_CAPTION) != 0;
	}

	if (!m_bIsMinimized)
	{
		CView* pView = GetActiveView();

		if (m_dockManager.IsPrintPreviewValid() || m_pNotifyHook != NULL)
		{
			if (pView != NULL && pView->IsKindOf(RUNTIME_CLASS(CPreviewViewEx)))
			{
				m_dockManager.RecalcLayout(bNotify);
				CRect rectClient = m_dockManager.GetClientAreaBounds();
				pView->SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOZORDER  | SWP_NOACTIVATE);
			}
			else
			{
				if (bNotify && m_pNotifyHook != NULL)
				{
					ActiveItemRecalcLayout();
				}
				else
				{
					m_bInRecalcLayout = FALSE;
					CFrameWnd::RecalcLayout(bNotify);

					AdjustClientArea();
				}
			}
		}
		else
		{
			m_dockManager.RecalcLayout(bNotify);
			AdjustClientArea();
		}
	}

	m_bInRecalcLayout = FALSE;

	if (bWasOleInPlaceActive != m_Impl.m_bIsOleInPlaceActive)
	{
		if (!m_Impl.m_bHadCaption)
		{
			if (m_Impl.m_bIsOleInPlaceActive)
			{
				ModifyStyle(0, WS_CAPTION);
			}
			else
			{
				ModifyStyle(WS_CAPTION, 0);
			}
		}

		m_Impl.OnChangeVisualManager();
		SetWindowPos(NULL, -1, -1, -1, -1, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
}

void CFrameWndEx::ActiveItemRecalcLayout()
{
	COleClientItem* pActiveItem = GetInPlaceActiveItem();

	if (pActiveItem != NULL && pActiveItem->m_pInPlaceFrame != NULL)
	{
		CRect rectBounds = m_dockManager.GetClientAreaBounds();
		pActiveItem->m_pInPlaceFrame->OnRecalcLayout();
	}

	AdjustClientArea();
}

BOOL CFrameWndEx::NegotiateBorderSpace( UINT nBorderCmd, LPRECT lpRectBorder )
{
	CRect border, request;

	switch (nBorderCmd)
	{
	case borderGet:
		{
			CFrameWnd::NegotiateBorderSpace(nBorderCmd, lpRectBorder);
			CRect rectBounds = m_dockManager.GetClientAreaBounds();
			ENSURE(lpRectBorder != NULL);

			*lpRectBorder = rectBounds;
			break;
		}
	case borderRequest:
		return TRUE;

	case borderSet:
		return CFrameWnd::NegotiateBorderSpace(nBorderCmd, lpRectBorder);

	default:
		ASSERT(FALSE);  // invalid CFrameWnd::BorderCmd
	}

	return TRUE;
}

void CFrameWndEx::OnSetPreviewMode(BOOL bPreview, CPrintPreviewState* pState)
{
	ASSERT_VALID(this);

	CFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CFrameWndEx, AFXGetTopLevelFrame(this));
	if (pMainFrame != NULL)
	{
		pMainFrame->SetPrintPreviewFrame(bPreview ? this : NULL);
	}

	m_dockManager.SetPrintPreviewMode(bPreview, pState);
	DWORD dwSavedState = pState->dwStates;
	CFrameWnd::OnSetPreviewMode(bPreview, pState);
	pState->dwStates = dwSavedState;

	AdjustDockingLayout();
	RecalcLayout();

	if (m_Impl.m_pRibbonBar != NULL && m_Impl.m_pRibbonBar->IsReplaceFrameCaption())
	{
		PostMessage(AFX_WM_POSTSETPREVIEWFRAME, bPreview);
	}
}

BOOL CFrameWndEx::OnShowPanes(BOOL bShow)
{
	ASSERT_VALID(this);
	BOOL bResult = m_dockManager.ShowPanes(bShow);
	AdjustDockingLayout();

	return bResult;
}

LRESULT CFrameWndEx::OnIdleUpdateCmdUI(WPARAM, LPARAM)
{
	m_dockManager.SendMessageToMiniFrames(WM_IDLEUPDATECMDUI);
	return 0L;
}

void CFrameWndEx::OnSize(UINT nType, int cx, int cy)
{
	m_bIsMinimized = (nType == SIZE_MINIMIZED);

	if (m_Impl.m_pRibbonBar || m_Impl.IsOwnerDrawCaption())
	{
		CRect rectWindow;
		GetWindowRect(rectWindow);

		WINDOWPOS wndpos;
		wndpos.flags = SWP_FRAMECHANGED;
		wndpos.x     = rectWindow.left;
		wndpos.y     = rectWindow.top;
		wndpos.cx    = rectWindow.Width();
		wndpos.cy    = rectWindow.Height();

		m_Impl.OnWindowPosChanging(&wndpos);
	}

	m_Impl.UpdateCaption();
	m_dockManager.OnActivateFrame(!m_bIsMinimized);

	if (!m_bIsMinimized && nType != SIZE_MAXIMIZED && !m_bWasMaximized)
	{
		m_dockManager.m_bSizeFrame = TRUE;
		CFrameWnd::OnSize(nType, cx, cy);
		AdjustDockingLayout();
		m_dockManager.m_bSizeFrame = FALSE;
		return;
	}

	CFrameWnd::OnSize(nType, cx, cy);

	if (nType == SIZE_MAXIMIZED ||(nType == SIZE_RESTORED && m_bWasMaximized))
	{
		RecalcLayout();
	}

	m_bWasMaximized = (nType == SIZE_MAXIMIZED);
}

LRESULT CFrameWndEx::OnExitSizeMove(WPARAM, LPARAM)
{
	RecalcLayout ();
	m_dockManager.FixupVirtualRects();
	return 0;
}

void CFrameWndEx::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	if (IsFullScreen())
	{
		m_Impl.m_FullScreenMgr.OnGetMinMaxInfo(lpMMI);
	}
	else
	{
		m_Impl.OnGetMinMaxInfo(lpMMI);
		CFrameWnd::OnGetMinMaxInfo(lpMMI);
	}
}

BOOL CFrameWndEx::OnShowPopupMenu(CMFCPopupMenu* pMenuPopup)
{
	if (afxGlobalData.IsAccessibilitySupport() && pMenuPopup != NULL)
	{
		::NotifyWinEvent(EVENT_SYSTEM_MENUPOPUPSTART, pMenuPopup->GetSafeHwnd(), OBJID_WINDOW , CHILDID_SELF);
	}

	return TRUE;
}

LRESULT CFrameWndEx::OnToolbarContextMenu(WPARAM,LPARAM)
{
	return 1l;
}

BOOL CFrameWndEx::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
	{
		return TRUE;
	}

	return m_dockManager.ProcessPaneContextMenuCommand(nID, nCode, pExtra, pHandlerInfo);
}

void CFrameWndEx::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos)
{
	if ((lpwndpos->flags & SWP_FRAMECHANGED) == SWP_FRAMECHANGED)
	{
		m_Impl.OnWindowPosChanging(lpwndpos);
	}

	CFrameWnd::OnWindowPosChanged(lpwndpos);
}

void CFrameWndEx::OnNcPaint()
{
	if (!m_Impl.OnNcPaint())
	{
		Default();
	}
}

LRESULT CFrameWndEx::OnSetText(WPARAM, LPARAM lParam)
{
	LRESULT lRes = Default();

	m_Impl.OnSetText((LPCTSTR)lParam);
	return lRes;
}

BOOL CFrameWndEx::OnNcActivate(BOOL bActive)
{
	if (m_Impl.OnNcActivate(bActive))
	{
		return TRUE;
	}

	return CFrameWnd::OnNcActivate(bActive);
}

int CFrameWndEx::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_Impl.m_bHasBorder = (lpCreateStruct->style & WS_BORDER) != NULL;

	CFrameImpl::AddFrame(this);
	OnChangeVisualManager(0, 0);
	return 0;
}

LRESULT CFrameWndEx::OnChangeVisualManager(WPARAM, LPARAM)
{
	m_Impl.OnChangeVisualManager();
	return 0;
}

void CFrameWndEx::OnNcMouseMove(UINT nHitTest, CPoint point)
{
	m_Impl.OnNcMouseMove(nHitTest, point);

	if (nHitTest == HTCAPTION &&(GetStyle() & WS_MAXIMIZE) == WS_MAXIMIZE)
	{
		BOOL bIsRibbonCaption = FALSE;

		if (m_Impl.m_pRibbonBar != NULL && m_Impl.m_pRibbonBar->IsWindowVisible() && m_Impl.m_pRibbonBar->IsReplaceFrameCaption())
		{
			bIsRibbonCaption = TRUE;
		}
		if (!bIsRibbonCaption && CMFCVisualManager::GetInstance()->IsOwnerDrawCaption())
		{
			return;
		}
	}

	CFrameWnd::OnNcMouseMove(nHitTest, point);
}

LRESULT CFrameWndEx::OnNcHitTest(CPoint point)
{
	UINT nHit = m_Impl.OnNcHitTest(point);
	if (nHit != HTNOWHERE)
	{
		return nHit;
	}

	return CFrameWnd::OnNcHitTest(point);
}

void CFrameWndEx::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	if (!m_Impl.OnNcCalcSize(bCalcValidRects, lpncsp))
	{
		CFrameWnd::OnNcCalcSize(bCalcValidRects, lpncsp);
	}
}

void CFrameWndEx::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_Impl.OnLButtonUp(point);
	CFrameWnd::OnLButtonUp(nFlags, point);
}

void CFrameWndEx::OnMouseMove(UINT nFlags, CPoint point)
{
	m_Impl.OnMouseMove(point);
	CFrameWnd::OnMouseMove(nFlags, point);
}

void CFrameWndEx::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_Impl.OnLButtonDown(point);
	CFrameWnd::OnLButtonDown(nFlags, point);
}

LRESULT CFrameWndEx::OnPostPreviewFrame(WPARAM, LPARAM)
{
	return 0;
}

LRESULT CFrameWndEx::OnDWMCompositionChanged(WPARAM,LPARAM)
{
	m_Impl.OnDWMCompositionChanged();
	return 0;
}

void CFrameWndEx::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	BOOL bIsRibbonCaption = FALSE;

	if (m_Impl.m_pRibbonBar != NULL && (m_Impl.m_pRibbonBar->IsWindowVisible() || !IsWindowVisible()) && m_Impl.m_pRibbonBar->IsReplaceFrameCaption())
	{
		bIsRibbonCaption = TRUE;
	}

	if (!m_Impl.IsOwnerDrawCaption() || !IsWindowVisible() || bIsRibbonCaption)
	{
		CFrameWnd::OnUpdateFrameTitle(bAddToTitle);
		return;
	}

	CString strTitle1;
	GetWindowText(strTitle1);

	CFrameWnd::OnUpdateFrameTitle(bAddToTitle);

	CString strTitle2;
	GetWindowText(strTitle2);

	if (strTitle1 != strTitle2)
	{
		SendMessage(WM_NCPAINT, 0, 0);
	}
}

LRESULT CFrameWndEx::OnPowerBroadcast(WPARAM wp, LPARAM)
{
	LRESULT lres = Default();

	if (wp == PBT_APMRESUMESUSPEND)
	{
		afxGlobalData.Resume();
	}

	return lres;
}

void CFrameWndEx::OnSysColorChange()
{
	CFrameWnd::OnSysColorChange();
	m_Impl.OnChangeVisualManager();
	SetWindowRgn(NULL, TRUE);
}
