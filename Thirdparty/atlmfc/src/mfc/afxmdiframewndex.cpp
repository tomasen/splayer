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
#include "afxmdiframewndex.h"
#include "afxmdichildwndex.h"
#include "afxtoolbar.h"
#include "afxmenubar.h"
#include "afxpopupmenu.h"
#include "afxtoolbarmenubutton.h"
#include "afxpaneframewnd.h"
#include "afxribbonres.h"
#include "afxwindowsmanagerdialog.h"
#include "afxusertoolsmanager.h"
#include "afxcontextmenumanager.h"

#include "afxdockablepane.h"
#include "afxtabbedpane.h"
#include "afxpreviewviewex.h"
#include "afxribbonbar.h"
#include "afxribbonstatusbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "imm32.lib")

/////////////////////////////////////////////////////////////////////////////
// CMDIFrameWndEx

IMPLEMENT_DYNCREATE(CMDIFrameWndEx, CMDIFrameWnd)

BOOL CMDIFrameWndEx::m_bDisableSetRedraw = TRUE;

#pragma warning(disable : 4355)

CMDIFrameWndEx::CMDIFrameWndEx() :
	m_Impl(this), m_hmenuWindow(NULL), m_bContextHelp(FALSE), m_bDoSubclass(TRUE), m_uiWindowsDlgMenuId(0),
	m_bShowWindowsDlgAlways(FALSE), m_bShowWindowsDlgHelpButton(FALSE), m_bWasMaximized(FALSE), m_bIsMinimized(FALSE),
	m_bClosing(FALSE), m_nFrameID(0), m_pPrintPreviewFrame(NULL), m_bCanConvertControlBarToMDIChild(FALSE)
{
}

#pragma warning(default : 4355)

CMDIFrameWndEx::~CMDIFrameWndEx()
{
}

//{{AFX_MSG_MAP(CMDIFrameWndEx)
BEGIN_MESSAGE_MAP(CMDIFrameWndEx, CMDIFrameWnd)
	ON_WM_MENUCHAR()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_ACTIVATE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_CONTEXTMENU()
	ON_WM_NCPAINT()
	ON_WM_NCACTIVATE()
	ON_WM_NCMOUSEMOVE()
	ON_WM_NCHITTEST()
	ON_WM_NCCALCSIZE()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_ACTIVATEAPP()
	ON_WM_SYSCOLORCHANGE()
	ON_MESSAGE(WM_IDLEUPDATECMDUI, &CMDIFrameWndEx::OnIdleUpdateCmdUI)
	ON_MESSAGE(WM_SETTEXT, &CMDIFrameWndEx::OnSetText)
	ON_MESSAGE(WM_DWMCOMPOSITIONCHANGED, &CMDIFrameWndEx::OnDWMCompositionChanged)
	ON_MESSAGE(WM_EXITSIZEMOVE, &CMDIFrameWndEx::OnExitSizeMove)
	ON_COMMAND(ID_CONTEXT_HELP, &CMDIFrameWndEx::OnContextHelp)
	ON_UPDATE_COMMAND_UI(ID_VIEW_STATUS_BAR, &CMDIFrameWndEx::OnUpdatePaneMenu)
	ON_COMMAND_EX(ID_VIEW_STATUS_BAR, &CMDIFrameWndEx::OnPaneCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOOLBAR, &CMDIFrameWndEx::OnUpdatePaneMenu)
	ON_COMMAND_EX(ID_VIEW_TOOLBAR, &CMDIFrameWndEx::OnPaneCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_REBAR, &CMDIFrameWndEx::OnUpdatePaneMenu)
	ON_COMMAND_EX(ID_VIEW_REBAR, &CMDIFrameWndEx::OnPaneCheck)
	ON_COMMAND(ID_WINDOW_NEW, &CMDIFrameWndEx::OnWindowNew)
	ON_REGISTERED_MESSAGE(AFX_WM_TOOLBARMENU, &CMDIFrameWndEx::OnToolbarContextMenu)
	ON_REGISTERED_MESSAGE(AFX_WM_CHANGEVISUALMANAGER, &CMDIFrameWndEx::OnChangeVisualManager)
	ON_REGISTERED_MESSAGE(AFX_WM_POSTSETPREVIEWFRAME, &CMDIFrameWndEx::OnPostPreviewFrame)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMDIFrameWndEx::OnToolbarCreateNew)
	ON_REGISTERED_MESSAGE(AFX_WM_DELETETOOLBAR, &CMDIFrameWndEx::OnToolbarDelete)
	ON_MESSAGE(WM_POWERBROADCAST, &OnPowerBroadcast)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// CMDIFrameWndEx message handlers

BOOL CMDIFrameWndEx::OnSetMenu(HMENU hmenu)
{
	COleClientItem* pActiveItem = GetInPlaceActiveItem();
	if (pActiveItem != NULL && pActiveItem->GetInPlaceWindow() != NULL)
	{
		return FALSE;
	}

	if (m_Impl.m_pRibbonBar != NULL && (m_Impl.m_pRibbonBar->GetStyle() & WS_VISIBLE))
	{
		SetMenu(NULL);
		m_Impl.m_pRibbonBar->SetActiveMDIChild(MDIGetActive());
		return TRUE;
	}

	if (m_Impl.m_pMenuBar != NULL)
	{
		SetMenu(NULL);
		m_Impl.m_pMenuBar->CreateFromMenu(hmenu == NULL ? m_Impl.m_hDefaultMenu : hmenu);
		return TRUE;
	}

	return FALSE;
}

BOOL CMDIFrameWndEx::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	if (!CMDIFrameWnd::OnCreateClient(lpcs, pContext))
	{
		return FALSE;
	}

	if (m_bDoSubclass)
	{
		m_wndClientArea.SubclassWindow(m_hWndMDIClient);
	}

	return TRUE;
}

LRESULT CMDIFrameWndEx::OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu)
{
	if (m_Impl.OnMenuChar(nChar))
	{
		return MAKELPARAM(MNC_EXECUTE, -1);
	}

	return CMDIFrameWnd::OnMenuChar(nChar, nFlags, pMenu);
}

void CMDIFrameWndEx::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos)
{
	if ((lpwndpos->flags & SWP_FRAMECHANGED) == SWP_FRAMECHANGED)
	{
		m_Impl.OnWindowPosChanging(lpwndpos);
	}

	CMDIFrameWnd::OnWindowPosChanged(lpwndpos);

	if (m_Impl.m_pMenuBar != NULL)
	{
		BOOL bMaximized;
		CMDIChildWnd* pChild = MDIGetActive(&bMaximized);

		if (pChild == NULL || !bMaximized)
		{
			m_Impl.m_pMenuBar->SetMaximizeMode(FALSE);
		}
		else
		{
			m_Impl.m_pMenuBar->SetMaximizeMode(TRUE, pChild);
		}
	}

	if (m_Impl.m_pRibbonBar != NULL)
	{
		ASSERT_VALID(m_Impl.m_pRibbonBar);

		BOOL bMaximized;
		CMDIChildWnd* pChild = MDIGetActive(&bMaximized);

		if (pChild == NULL || !bMaximized)
		{
			m_Impl.m_pRibbonBar->SetMaximizeMode(FALSE);
		}
		else
		{
			m_Impl.m_pRibbonBar->SetMaximizeMode(TRUE, pChild);
		}
	}
}

BOOL CMDIFrameWndEx::PreTranslateMessage(MSG* pMsg)
{
	BOOL bProcessAccel = TRUE;

	switch(pMsg->message)
	{
	case WM_SYSKEYDOWN:
		if (m_Impl.m_pRibbonBar != NULL && m_Impl.m_pRibbonBar->OnSysKeyDown(this, pMsg->wParam, pMsg->lParam))
		{
			return TRUE;
		}

	case WM_CONTEXTMENU:
		if (CMFCPopupMenu::m_pActivePopupMenu != NULL && ::IsWindow(CMFCPopupMenu::m_pActivePopupMenu->m_hWnd) &&
			pMsg->wParam == VK_MENU)
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
		//-----------------------------------------
		// Pass keyboard action to the active menu:
		//-----------------------------------------
		if (!CFrameImpl::IsHelpKey(pMsg) && m_Impl.ProcessKeyboard((int) pMsg->wParam))
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

	return CMDIFrameWnd::PreTranslateMessage(pMsg);
}

BOOL CMDIFrameWndEx::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (HIWORD(wParam) == 1)
	{
		UINT uiCmd = LOWORD(wParam);
		CMFCToolBar::AddCommandUsage(uiCmd);

		//---------------------------
		// Simmulate ESC keystroke...
		//---------------------------
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
		return CMDIFrameWnd::OnCommand(wParam, lParam);
	}

	return FALSE;
}

HMENU CMDIFrameWndEx::GetWindowMenuPopup(HMENU hMenuBar)
{
	if (m_bClosing)
	{
		return NULL;
	}

	m_hmenuWindow = CMDIFrameWnd::GetWindowMenuPopup(hMenuBar);
	return m_hmenuWindow;
}

BOOL CMDIFrameWndEx::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext)
{
	m_Impl.m_nIDDefaultResource = nIDResource;
	m_Impl.LoadLargeIconsState();

	if (!CMDIFrameWnd::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}

	m_Impl.OnLoadFrame();

	if (GetMenuBar() != NULL)
	{
		m_hMenuDefault = m_Impl.m_hDefaultMenu;
	}

	return TRUE;
}

void CMDIFrameWndEx::OnClose()
{
	if (m_pPrintPreviewFrame != NULL)
	{
		m_pPrintPreviewFrame->SendMessage(WM_CLOSE);
		m_pPrintPreviewFrame = NULL;
		return;
	}

	if (!m_Impl.IsPrintPreview())
	{
		m_bClosing = TRUE;

		// Deactivate OLE container first:
		COleClientItem* pActiveItem = GetInPlaceActiveItem();
		if (pActiveItem != NULL)
		{
			pActiveItem->Deactivate();
		}

		m_Impl.OnCloseFrame();
	}

	CMDIFrameWnd::OnClose();
}

BOOL CMDIFrameWndEx::PreCreateWindow(CREATESTRUCT& cs)
{
	m_Impl.SetDockingManager(&m_dockManager);
	m_Impl.RestorePosition(cs);
	return CMDIFrameWnd::PreCreateWindow(cs);
}

BOOL CMDIFrameWndEx::ShowPopupMenu(CMFCPopupMenu* pMenuPopup)
{
	if (!m_Impl.OnShowPopupMenu(pMenuPopup, this))
	{
		return FALSE;
	}


	if (!CMFCToolBar::IsCustomizeMode() && m_hmenuWindow != NULL && pMenuPopup != NULL && pMenuPopup->GetHMenu() != NULL)
	{
		//-----------------------------------------------------------
		// Check the popup menu for the "Windows..." menu maching...:
		//-----------------------------------------------------------
		HMENU hMenuPop = pMenuPopup->GetHMenu();
		BOOL bIsWindowMenu = FALSE;

		int iItemMax = ::GetMenuItemCount(hMenuPop);
		for (int iItemPop = 0; !bIsWindowMenu && iItemPop < iItemMax; iItemPop ++)
		{
			UINT nID = ::GetMenuItemID( hMenuPop, iItemPop);
			bIsWindowMenu = (nID >= AFX_IDM_WINDOW_FIRST && nID <= AFX_IDM_WINDOW_LAST);
		}

		if (bIsWindowMenu)
		{
			CMenu* pMenu = CMenu::FromHandle(m_hmenuWindow);
			if (pMenu != NULL)
			{
				int iCount = (int) pMenu->GetMenuItemCount();
				BOOL bIsFirstWindowItem = TRUE;
				BOOL bIsStandradWindowsDlg = FALSE;

				for (int i = 0; i < iCount; i ++)
				{
					UINT uiCmd = pMenu->GetMenuItemID(i);
					if (uiCmd < AFX_IDM_FIRST_MDICHILD || uiCmd == (UINT) -1)
					{
						continue;
					}

					if (m_uiWindowsDlgMenuId != 0 && uiCmd == AFX_IDM_FIRST_MDICHILD + 9)
					{
						// Don't add standrd "Windows..." command
						bIsStandradWindowsDlg = TRUE;
						continue;
					}

					if (bIsFirstWindowItem)
					{
						pMenuPopup->InsertSeparator();
						bIsFirstWindowItem = FALSE;

						::SendMessage(m_hWndMDIClient, WM_MDIREFRESHMENU, 0, 0);
					}

					CString strText;
					pMenu->GetMenuString(i, strText, MF_BYPOSITION);

					CMFCToolBarMenuButton button(uiCmd, NULL, -1, strText);

					UINT uiState = pMenu->GetMenuState(i, MF_BYPOSITION);
					if (uiState & MF_CHECKED)
					{
						button.m_nStyle |= TBBS_CHECKED;
					}

					pMenuPopup->InsertItem(button);
				}

				if (m_uiWindowsDlgMenuId != 0 && (bIsStandradWindowsDlg || m_bShowWindowsDlgAlways))
				{
					if (!CMFCToolBar::GetBasicCommands().IsEmpty())
					{
						CMFCToolBar::AddBasicCommand(m_uiWindowsDlgMenuId);
					}

					//-----------------------------
					// Add our "Windows..." dialog:
					//-----------------------------
					pMenuPopup->InsertItem(CMFCToolBarMenuButton(m_uiWindowsDlgMenuId, NULL, -1, m_strWindowsDlgMenuText));
				}
			}
		}
	}

	if (pMenuPopup != NULL && pMenuPopup->m_bShown)
	{
		return TRUE;
	}

	return OnShowPopupMenu(pMenuPopup);
}

void CMDIFrameWndEx::OnClosePopupMenu(CMFCPopupMenu* pMenuPopup)
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

BOOL CMDIFrameWndEx::OnDrawMenuImage(CDC* pDC, const CMFCToolBarMenuButton* pMenuButton, const CRect& rectImage)
{
	ASSERT_VALID(this);

	if (m_Impl.m_pRibbonBar != NULL)
	{
		ASSERT_VALID(m_Impl.m_pRibbonBar);
		return m_Impl.m_pRibbonBar->DrawMenuImage(pDC, pMenuButton, rectImage);
	}

	return FALSE;
}

LRESULT CMDIFrameWndEx::OnToolbarCreateNew(WPARAM,LPARAM lp)
{
	ENSURE(lp != NULL);
	return(LRESULT) m_Impl.CreateNewToolBar((LPCTSTR) lp);
}

LRESULT CMDIFrameWndEx::OnToolbarDelete(WPARAM,LPARAM lp)
{
	CMFCToolBar* pToolbar = (CMFCToolBar*) lp;
	ASSERT_VALID(pToolbar);

	return(LRESULT) m_Impl.DeleteToolBar(pToolbar);
}

void CMDIFrameWndEx::HtmlHelp(DWORD_PTR dwData, UINT nCmd)
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

void CMDIFrameWndEx::WinHelp(DWORD dwData, UINT nCmd)
{
	if (dwData > 0 || !m_bContextHelp)
	{
		CMDIFrameWnd::WinHelp(dwData, nCmd);
	}
	else
	{
		OnContextHelp();
	}
}

void CMDIFrameWndEx::OnContextHelp()
{
	m_bContextHelp = TRUE;

	if (!m_bHelpMode && CanEnterHelpMode())
	{
		CMFCToolBar::SetHelpMode();
	}

	CMDIFrameWnd::OnContextHelp();

	if (!m_bHelpMode)
	{
		CMFCToolBar::SetHelpMode(FALSE);
	}

	m_bContextHelp = FALSE;
}

BOOL CMDIFrameWndEx::DockPaneLeftOf(CPane* pBar, CPane* pLeftOf)
{
	return m_dockManager.DockPaneLeftOf(pBar, pLeftOf);
}

void CMDIFrameWndEx::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CMDIFrameWnd::OnActivate(nState, pWndOther, bMinimized);
	switch(nState)
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

void CMDIFrameWndEx::OnActivateApp(BOOL bActive, DWORD /*dwThreadID*/)
{
	m_dockManager.OnActivateFrame(bActive);
	m_Impl.OnActivateApp(bActive);
}

void CMDIFrameWndEx::EnableWindowsDialog(UINT uiMenuId, LPCTSTR lpszMenuText, BOOL bShowAlways, BOOL bShowHelpButton)
{
	ENSURE(lpszMenuText != NULL);
	ENSURE(uiMenuId != 0);

	m_uiWindowsDlgMenuId = uiMenuId;
	m_strWindowsDlgMenuText = lpszMenuText;
	m_bShowWindowsDlgAlways = bShowAlways;
	m_bShowWindowsDlgHelpButton = bShowHelpButton;
}

void CMDIFrameWndEx::EnableWindowsDialog(UINT uiMenuId, UINT uiMenuTextResId, BOOL bShowAlways, BOOL bShowHelpButton)
{
	CString strMenuText;
	ENSURE(strMenuText.LoadString(uiMenuTextResId));

	EnableWindowsDialog(uiMenuId, strMenuText, bShowAlways, bShowHelpButton);
}

void CMDIFrameWndEx::ShowWindowsDialog()
{
	CMFCWindowsManagerDialog dlg(this, m_bShowWindowsDlgHelpButton);
	dlg.DoModal();
}

COleClientItem* CMDIFrameWndEx::GetInPlaceActiveItem()
{
	CFrameWnd* pActiveFrame = GetActiveFrame();
	if (pActiveFrame == NULL)
	{
		return NULL;
	}

	ASSERT_VALID(pActiveFrame);

	CView* pView = pActiveFrame->GetActiveView();
	if (pView == NULL || pView->IsKindOf (RUNTIME_CLASS (CPreviewViewEx)))
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

void CMDIFrameWndEx::OnUpdateFrameMenu(HMENU hMenuAlt)
{
	CMDIFrameWnd::OnUpdateFrameMenu(hMenuAlt);

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

void CMDIFrameWndEx::OnDestroy()
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
		if (m_wndClientArea.m_hWnd != pNextWnd->m_hWnd)
		{
			lstChildren.AddTail(pNextWnd->m_hWnd);
		}
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
	CMDIFrameWnd::OnDestroy();
}

void CMDIFrameWndEx::EnableMDITabbedGroups(BOOL bEnable, const CMDITabInfo& params)
{
	m_wndClientArea.EnableMDITabbedGroups(bEnable, params);
}

void CMDIFrameWndEx::EnableMDITabs(BOOL bEnable/* = TRUE*/, BOOL bIcons/* = TRUE*/, CMFCTabCtrl::Location tabLocation /* = CMFCTabCtrl::LOCATION_BOTTOM*/,
	BOOL bTabCloseButton/* = FALSE*/, CMFCTabCtrl::Style style/* = CMFCTabCtrl::STYLE_3D_SCROLLED*/, BOOL bTabCustomTooltips/* = FALSE*/, BOOL bActiveTabCloseButton/* = FALSE*/)
{
	ASSERT(style == CMFCTabCtrl::STYLE_3D_SCROLLED || style == CMFCTabCtrl::STYLE_3D_ONENOTE || style == CMFCTabCtrl::STYLE_3D_VS2005 ||
		style == CMFCTabCtrl::STYLE_3D_ROUNDED || style == CMFCTabCtrl::STYLE_3D_ROUNDED_SCROLL);

	CMDITabInfo params;
	params.m_style = style;
	params.m_tabLocation = tabLocation;
	params.m_bTabIcons = bIcons;
	params.m_bTabCloseButton = bTabCloseButton;
	params.m_bTabCustomTooltips = bTabCustomTooltips;
	params.m_bActiveTabCloseButton = bActiveTabCloseButton;

	m_wndClientArea.EnableMDITabs(bEnable, params);
}

void CMDIFrameWndEx::OnSetPreviewMode(BOOL bPreview, CPrintPreviewState* pState)
{
	ASSERT_VALID(this);

	if (m_wndClientArea.DoesMDITabExist())
	{
		m_wndClientArea.m_bTabIsVisible = !bPreview;
		((CWnd&)m_wndClientArea.GetMDITabs()).ShowWindow(bPreview ? SW_HIDE : SW_SHOWNOACTIVATE);
	}

	m_dockManager.SetPrintPreviewMode(bPreview, pState);

	DWORD dwSavedState = pState->dwStates;
	CMDIFrameWnd::OnSetPreviewMode(bPreview, pState);
	pState->dwStates = dwSavedState;

	AdjustDockingLayout();
	RecalcLayout();

	if (m_Impl.m_pRibbonBar != NULL && m_Impl.m_pRibbonBar->IsReplaceFrameCaption())
	{
		PostMessage(AFX_WM_POSTSETPREVIEWFRAME, bPreview);
	}
}

BOOL CMDIFrameWndEx::OnShowPanes(BOOL bShow)
{
	ASSERT_VALID(this);

	BOOL bResult = m_dockManager.ShowPanes(bShow);
	AdjustDockingLayout();

	return bResult;
}

void CMDIFrameWndEx::AddDockSite()
{
	ASSERT_VALID(this);
}

BOOL CMDIFrameWndEx::AddPane(CBasePane* pControlBar, BOOL bTail)
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

BOOL CMDIFrameWndEx::InsertPane(CBasePane* pControlBar, CBasePane* pTarget, BOOL bAfter)
{
	ASSERT_VALID(this);
	return m_dockManager.InsertPane(pControlBar, pTarget, bAfter);
}

void CMDIFrameWndEx::RemovePaneFromDockManager(CBasePane* pControlBar, BOOL bDestroy, BOOL bAdjustLayout, BOOL bAutoHide, CBasePane* pBarReplacement)
{
	ASSERT_VALID(this);
	m_dockManager.RemovePaneFromDockManager(pControlBar, bDestroy, bAdjustLayout, bAutoHide, pBarReplacement);
}

void CMDIFrameWndEx::DockPane(CBasePane* pBar, UINT nDockBarID, LPCRECT lpRect)
{
	ASSERT_VALID(this);
	m_dockManager.DockPane(pBar, nDockBarID, lpRect);
}

CBasePane* CMDIFrameWndEx::GetPane(UINT nID)
{
	ASSERT_VALID(this);

	CBasePane* pBar = m_dockManager.FindPaneByID(nID, TRUE);
	return pBar;
}

void CMDIFrameWndEx::ShowPane(CBasePane* pBar, BOOL bShow, BOOL bDelay, BOOL bActivate)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);

	pBar->ShowPane(bShow, bDelay, bActivate);
}

CBasePane* CMDIFrameWndEx::PaneFromPoint(CPoint point, int nSensitivity, bool bExactBar, CRuntimeClass* pRTCBarType) const
{
	ASSERT_VALID(this);
	return m_dockManager.PaneFromPoint(point, nSensitivity, bExactBar, pRTCBarType);
}

CBasePane* CMDIFrameWndEx::PaneFromPoint(CPoint point, int nSensitivity, DWORD& dwAlignment, CRuntimeClass* pRTCBarType) const
{
	ASSERT_VALID(this);
	return m_dockManager.PaneFromPoint(point, nSensitivity, dwAlignment, pRTCBarType);
}

BOOL CMDIFrameWndEx::IsPointNearDockSite(CPoint point, DWORD& dwBarAlignment, BOOL& bOuterEdge) const
{
	ASSERT_VALID(this);
	return m_dockManager.IsPointNearDockSite(point, dwBarAlignment, bOuterEdge);
}

void CMDIFrameWndEx::AdjustDockingLayout(HDWP hdwp)
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

void CMDIFrameWndEx::AdjustClientArea()
{
	CRect rectClientAreaBounds = m_dockManager.GetClientAreaBounds();

	rectClientAreaBounds.left += m_rectBorder.left;
	rectClientAreaBounds.top  += m_rectBorder.top;
	rectClientAreaBounds.right -= m_rectBorder.right;
	rectClientAreaBounds.bottom -= m_rectBorder.bottom;

	if (m_wndClientArea.GetSafeHwnd() != NULL)
	{
		m_wndClientArea.CalcWindowRect(rectClientAreaBounds, 0);
	}
}

BOOL CMDIFrameWndEx::OnMoveMiniFrame(CWnd* pFrame)
{
	ASSERT_VALID(this);
	return m_dockManager.OnMoveMiniFrame(pFrame);
}

BOOL CMDIFrameWndEx::EnableDocking(DWORD dwDockStyle)
{
	return m_dockManager.EnableDocking(dwDockStyle);
}

BOOL CMDIFrameWndEx::EnableAutoHidePanes(DWORD dwDockStyle)
{
	return m_dockManager.EnableAutoHidePanes(dwDockStyle);
}

void CMDIFrameWndEx::RecalcLayout(BOOL bNotify)
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

		if (m_dockManager.IsPrintPreviewValid() || m_dockManager.IsOLEContainerMode())
		{
			if (pView != NULL && pView->IsKindOf(RUNTIME_CLASS(CPreviewViewEx)))
			{

				m_dockManager.RecalcLayout(bNotify);
				CRect rectClient = m_dockManager.GetClientAreaBounds();
				pView->SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOZORDER  | SWP_NOACTIVATE);
			}
			else
			{
				if (bNotify && m_dockManager.IsOLEContainerMode())
				{
					ActiveItemRecalcLayout();
				}
				else
				{
					m_bInRecalcLayout = FALSE;
					CMDIFrameWnd::RecalcLayout(bNotify);

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

void CMDIFrameWndEx::ActiveItemRecalcLayout()
{
	COleClientItem* pActiveItem = GetInPlaceActiveItem();
	if (pActiveItem != NULL)
	{
		if (pActiveItem->m_pInPlaceFrame != NULL)
		{
			CRect rectBounds = m_dockManager.GetClientAreaBounds();
			pActiveItem->m_pInPlaceFrame->OnRecalcLayout();
		}

		CView* pActiveView = pActiveItem->GetActiveView();
		if (pActiveView != NULL)
		{
			CMDIChildWndEx* pFrame = (CMDIChildWndEx*) pActiveView->GetParentFrame();

			if (pFrame != NULL && pFrame->m_bActivating)
			{
				pActiveItem->m_pInPlaceFrame->OnRecalcLayout();
			}
		}
	}
	AdjustClientArea();
}

BOOL CMDIFrameWndEx::NegotiateBorderSpace( UINT nBorderCmd, LPRECT lpRectBorder )
{
	CRect border, request;

	switch(nBorderCmd)
	{
	case borderGet:
		{
			CFrameWnd::NegotiateBorderSpace(nBorderCmd, lpRectBorder);
			m_dockManager.AdjustDockingLayout();
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

int CMDIFrameWndEx::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_dockManager.Create(this);

	m_Impl.m_bHasBorder = (lpCreateStruct->style & WS_BORDER) != NULL;

	CFrameImpl::AddFrame(this);
	OnChangeVisualManager(0, 0);

	return 0;
}

LRESULT CMDIFrameWndEx::OnExitSizeMove(WPARAM, LPARAM)
{
	RecalcLayout ();
	m_dockManager.FixupVirtualRects();
	return 0;
}

void CMDIFrameWndEx::OnUpdatePaneMenu(CCmdUI* pCmdUI)
{
	CBasePane* pBar = GetPane(pCmdUI->m_nID);
	if (pBar != NULL)
	{
		pCmdUI->SetCheck(pBar->IsWindowVisible());
		return;
	}

	pCmdUI->ContinueRouting();
}

BOOL CMDIFrameWndEx::OnPaneCheck(UINT nID)
{
	ASSERT_VALID(this);

	CBasePane* pBar = GetPane(nID);
	if (pBar != NULL)
	{
		ShowPane(pBar, !pBar->IsWindowVisible(), FALSE, FALSE);
		return TRUE;
	}

	return FALSE;
}


LRESULT CMDIFrameWndEx::OnIdleUpdateCmdUI(WPARAM, LPARAM)
{
	m_dockManager.SendMessageToMiniFrames(WM_IDLEUPDATECMDUI);
	return 0L;
}

void CMDIFrameWndEx::OnSize(UINT nType, int cx, int cy)
{
	if (m_bClosing)
	{
		CMDIFrameWnd::OnSize(nType, cx, cy);
		return;
	}

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

	m_dockManager.OnActivateFrame(!m_bIsMinimized);

	if (!m_bIsMinimized && nType != SIZE_MAXIMIZED && !m_bWasMaximized)
	{
		m_dockManager.m_bSizeFrame = TRUE;
		AdjustDockingLayout();
		CMDIFrameWnd::OnSize(nType, cx, cy);
		m_dockManager.m_bSizeFrame = FALSE;
		BOOL bParam = FALSE;
		SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &bParam, 0);
		if (!bParam)
		{
			RecalcLayout();
		}

		m_Impl.UpdateCaption();
		return;
	}

	CMDIFrameWnd::OnSize(nType, cx, cy);

	if (nType == SIZE_MAXIMIZED ||(nType == SIZE_RESTORED && m_bWasMaximized))
	{
		RecalcLayout();
	}

	m_bWasMaximized = (nType == SIZE_MAXIMIZED);
	m_Impl.UpdateCaption();
}

void CMDIFrameWndEx::OnWindowNew()
{
	CMDIChildWnd* pActiveChild = MDIGetActive();
	if (pActiveChild == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	ASSERT_VALID(pActiveChild);

	BOOL bIsZoomed = FALSE;

	if (pActiveChild->IsZoomed())
	{
		pActiveChild->ShowWindow(SW_RESTORE);
		bIsZoomed = TRUE;
	}

	CMDIFrameWnd::OnWindowNew();

	pActiveChild->RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE);

	if (bIsZoomed)
	{
		pActiveChild = MDIGetActive();
		if (pActiveChild != NULL)
		{
			pActiveChild->ShowWindow(SW_MAXIMIZE);
		}
	}
}

void CMDIFrameWndEx::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
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

CMDIChildWndEx* CMDIFrameWndEx::CreateDocumentWindow(LPCTSTR /*lpcszDocName*/, CObject* /*pObj*/)
{
	ASSERT(FALSE);
	TRACE0("If you use save/load state for MDI tabs, you must override this method in a derived class!\n");
	return NULL;
}

CMDIChildWndEx* CMDIFrameWndEx::CreateNewWindow(LPCTSTR lpcszDocName, CObject* /*pObj*/)
{
	TRACE0("If you use save/load state for MDI tabs, you should override this method in a derived class!\n");

	if (AreMDITabs())
	{
		OnWindowNew();
		return DYNAMIC_DOWNCAST(CMDIChildWndEx, MDIGetActive());
	}

	CDocument* pDoc = AfxGetApp()->OpenDocumentFile(lpcszDocName);
	if (pDoc == NULL)
	{
		return NULL;
	}

	POSITION pos = pDoc->GetFirstViewPosition();
	if (pos == NULL)
	{
		ASSERT(FALSE);
		return NULL;
	}

	CView* pView = pDoc->GetNextView(pos);
	ASSERT_VALID(pView);

	return DYNAMIC_DOWNCAST(CMDIChildWndEx, pView->GetParentFrame());
}

BOOL CMDIFrameWndEx::LoadMDIState(LPCTSTR lpszProfileName)
{
	return m_wndClientArea.LoadState(lpszProfileName, m_nFrameID);
}

BOOL CMDIFrameWndEx::SaveMDIState(LPCTSTR lpszProfileName)
{
	return m_wndClientArea.SaveState(lpszProfileName, m_nFrameID);
}

void CMDIFrameWndEx::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (m_wndClientArea.GetMDITabs().GetSafeHwnd() == NULL)
	{
		return;
	}

	if (CMFCPopupMenu::GetActiveMenu() != NULL)
	{
		return;
	}

	if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0) // Left mouse button is pressed
	{
		return;
	}

	if (pWnd->GetSafeHwnd() == m_wndClientArea.GetSafeHwnd())
	{
		CWnd* pWndCur = WindowFromPoint(point);

		if (IsMemberOfMDITabGroup(pWndCur))
		{
			CMFCTabCtrl* pWndTab = DYNAMIC_DOWNCAST(CMFCTabCtrl, pWndCur);
			if (pWndTab != NULL)
			{
				CPoint ptTab = point;
				pWndTab->ScreenToClient(&ptTab);

				const int nTab = pWndTab->GetTabFromPoint(ptTab);

				if (nTab >= 0)
				{
					pWndTab->SetActiveTab(nTab);
					OnShowMDITabContextMenu(point, GetMDITabsContextMenuAllowedItems(), FALSE);
				}
			}
		}
		else if ((CMFCPopupMenu::GetActiveMenu() == NULL) && (pWndCur == pWnd))
		{
			if (SendMessage(AFX_WM_TOOLBARMENU, (WPARAM) GetSafeHwnd(), MAKELPARAM(point.x, point.y)))
			{
				m_dockManager.OnPaneContextMenu(point);
			}
		}
	}
	else if (pWnd->GetSafeHwnd() == m_wndClientArea.GetMDITabs().GetSafeHwnd())
	{
		CMFCTabCtrl& wndTab = (CMFCTabCtrl&)(*pWnd);

		CRect rectTabs;
		wndTab.GetTabsRect(rectTabs);

		CPoint ptTab = point;
		wndTab.ScreenToClient(&ptTab);

		const int nTab = wndTab.GetTabFromPoint(ptTab);

		if (nTab >= 0)
		{
			wndTab.SetActiveTab(nTab);
			OnShowMDITabContextMenu(point, GetMDITabsContextMenuAllowedItems(), FALSE);
		}
	}
}

BOOL CMDIFrameWndEx::OnShowPopupMenu(CMFCPopupMenu* pMenuPopup)
{
	if (afxGlobalData.IsAccessibilitySupport() && pMenuPopup != NULL)
	{
		::NotifyWinEvent(EVENT_SYSTEM_MENUPOPUPSTART, pMenuPopup->GetSafeHwnd(), OBJID_WINDOW , CHILDID_SELF);
	}

	return TRUE;
}

LRESULT CMDIFrameWndEx::OnToolbarContextMenu(WPARAM,LPARAM)
{
	return 1l;
}

BOOL CMDIFrameWndEx::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (CMDIFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
	{
		return TRUE;
	}

	return m_dockManager.ProcessPaneContextMenuCommand(nID, nCode, pExtra, pHandlerInfo);
}

void CMDIFrameWndEx::OnNcPaint()
{
	if (!m_Impl.OnNcPaint())
	{
		Default();
	}
}

LRESULT CMDIFrameWndEx::OnSetText(WPARAM, LPARAM lParam)
{
	LRESULT lRes = Default();

	m_Impl.OnSetText((LPCTSTR)lParam);
	return lRes;
}

BOOL CMDIFrameWndEx::OnNcActivate(BOOL bActive)
{
	if (m_Impl.OnNcActivate(bActive))
	{
		return TRUE;
	}

	return CMDIFrameWnd::OnNcActivate(bActive);
}

void CMDIFrameWndEx::OnNcMouseMove(UINT nHitTest, CPoint point)
{
	m_Impl.OnNcMouseMove(nHitTest, point);
	CMDIFrameWnd::OnNcMouseMove(nHitTest, point);
}

LRESULT CMDIFrameWndEx::OnNcHitTest(CPoint point)
{
	UINT nHit = m_Impl.OnNcHitTest(point);
	if (nHit != HTNOWHERE)
	{
		return nHit;
	}

	return CMDIFrameWnd::OnNcHitTest(point);
}

LRESULT CMDIFrameWndEx::OnChangeVisualManager(WPARAM, LPARAM)
{
	m_Impl.OnChangeVisualManager();
	return 0;
}

void CMDIFrameWndEx::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	if (!m_Impl.OnNcCalcSize(bCalcValidRects, lpncsp))
	{
		CMDIFrameWnd::OnNcCalcSize(bCalcValidRects, lpncsp);
	}
}

void CMDIFrameWndEx::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_Impl.OnLButtonUp(point);
	CMDIFrameWnd::OnLButtonUp(nFlags, point);
}

void CMDIFrameWndEx::OnMouseMove(UINT nFlags, CPoint point)
{
	m_Impl.OnMouseMove(point);
	CMDIFrameWnd::OnMouseMove(nFlags, point);
}

void CMDIFrameWndEx::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_Impl.OnLButtonDown(point);
	CMDIFrameWnd::OnLButtonDown(nFlags, point);
}

LRESULT CMDIFrameWndEx::OnPostPreviewFrame(WPARAM, LPARAM)
{
	return 0;
}

CMDIChildWndEx* CMDIFrameWndEx::ControlBarToTabbedDocument(CDockablePane* pBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);

	CMDIChildWndEx* pFrame = new CMDIChildWndEx;
	ASSERT_VALID(pFrame);

	pBar->m_bWasFloatingBeforeTabbed = pBar->IsFloating();

	CString strName;
	pBar->GetWindowText(strName);

	if (!pFrame->Create(
		AfxRegisterWndClass(CS_DBLCLKS, 0, (HBRUSH)(COLOR_BTNFACE + 1), pBar->GetIcon(FALSE)),
		strName, WS_CHILD | WS_VISIBLE | WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, rectDefault, this))
	{
		return NULL;
	}

	pFrame->SetTitle(strName);
	pFrame->SetWindowText(strName);
	pFrame->AddTabbedPane(pBar);

	return pFrame;
}

BOOL CMDIFrameWndEx::TabbedDocumentToControlBar(CMDIChildWndEx* pMDIChildWnd)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pMDIChildWnd);

	if (!pMDIChildWnd->IsTabbedPane())
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, pMDIChildWnd->GetTabbedPane());
	if (pBar != NULL)
	{
		if (pBar->m_bWasFloatingBeforeTabbed)
		{
			pBar->SetParent(this);
			pBar->SetMDITabbed(FALSE);
			pBar->FloatPane(pBar->m_recentDockInfo.m_rectRecentFloatingRect, DM_DBL_CLICK);
		}
		else
		{
			pBar->ShowWindow(SW_HIDE);
			pBar->SetParent(this);
			pBar->SetMDITabbed(FALSE);
			pBar->DockToRecentPos();
		}
	}

	pMDIChildWnd->SendMessage(WM_CLOSE);
	return TRUE;
}

void CMDIFrameWndEx::UpdateMDITabbedBarsIcons()
{
	ASSERT_VALID(this);

	//-----------------------------------
	// Set MDI tabbed control bars icons:
	//-----------------------------------
	HWND hwndMDIChild = ::GetWindow(m_hWndMDIClient, GW_CHILD);

	while (hwndMDIChild != NULL)
	{
		CMDIChildWndEx* pMDIChildFrame = DYNAMIC_DOWNCAST(CMDIChildWndEx, CWnd::FromHandle(hwndMDIChild));

		if (pMDIChildFrame != NULL && pMDIChildFrame->IsTabbedPane())
		{
			CDockablePane* pBar = pMDIChildFrame->GetTabbedPane();
			ASSERT_VALID(pBar);

#pragma warning(disable : 4311)
			SetClassLongPtr(hwndMDIChild, GCLP_HICONSM, (long) pBar->GetIcon(FALSE));
#pragma warning(default : 4311)
		}

		hwndMDIChild = ::GetWindow(hwndMDIChild, GW_HWNDNEXT);
	}
}

BOOL CMDIFrameWndEx::OnShowMDITabContextMenu(CPoint point, DWORD dwAllowedItems, BOOL /*bTabDrop*/)
{
	if ((dwAllowedItems & AFX_MDI_CAN_BE_DOCKED) == 0)
	{
		return FALSE;
	}

	if (afxContextMenuManager == NULL)
	{
		return FALSE;
	}

	const UINT idTabbed = (UINT) -106;

	CMenu menu;
	menu.CreatePopupMenu();

	CString strItem;
	ENSURE(strItem.LoadString(IDS_AFXBARRES_TABBED));

	menu.AppendMenu(MF_STRING, idTabbed, strItem);
	menu.CheckMenuItem(idTabbed, MF_CHECKED);

	HWND hwndThis = GetSafeHwnd();

	int nMenuResult = afxContextMenuManager->TrackPopupMenu(menu, point.x, point.y, this);

	if (::IsWindow(hwndThis))
	{
		switch(nMenuResult)
		{
		case idTabbed:
			{
				CMDIChildWndEx* pMDIChild = DYNAMIC_DOWNCAST(CMDIChildWndEx, MDIGetActive());
				if (pMDIChild != NULL)
				{
					TabbedDocumentToControlBar(pMDIChild);
				}
			}
		}
	}

	return TRUE;
}

LRESULT CMDIFrameWndEx::OnDWMCompositionChanged(WPARAM,LPARAM)
{
	m_Impl.OnDWMCompositionChanged();
	return 0;
}

LRESULT CMDIFrameWndEx::OnPowerBroadcast(WPARAM wp, LPARAM)
{
	LRESULT lres = Default();

	if (wp == PBT_APMRESUMESUSPEND)
	{
		afxGlobalData.Resume();
	}

	return lres;
}

void CMDIFrameWndEx::OnSysColorChange()
{
	CMDIFrameWnd::OnSysColorChange();
	m_Impl.OnChangeVisualManager();
	SetWindowRgn(NULL, TRUE);
}
