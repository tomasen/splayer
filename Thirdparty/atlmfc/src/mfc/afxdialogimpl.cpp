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
#include "afxpopupmenu.h"
#include "afxtoolbarmenubutton.h"
#include "afxdialogex.h"
#include "afxpropertypage.h"
#include "afxdialogimpl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HHOOK CDialogImpl::m_hookMouse = NULL;
CDialogImpl* CDialogImpl::m_pMenuDlgImpl = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDialogImpl::CDialogImpl(CWnd& dlg) : m_Dlg(dlg)
{
}

CDialogImpl::~CDialogImpl()
{
}

BOOL CDialogImpl::ProcessMouseClick(POINT pt)
{
	if (!CMFCToolBar::IsCustomizeMode() && CMFCPopupMenu::m_pActivePopupMenu != NULL && ::IsWindow(CMFCPopupMenu::m_pActivePopupMenu->m_hWnd))
	{
		CMFCPopupMenu::MENUAREA_TYPE clickArea = CMFCPopupMenu::m_pActivePopupMenu->CheckArea(pt);

		if (clickArea == CMFCPopupMenu::OUTSIDE)
		{
			// Click outside of menu
			// Maybe secondary click on the parent button?
			CMFCToolBarMenuButton* pParentButton = CMFCPopupMenu::m_pActivePopupMenu->GetParentButton();
			if (pParentButton != NULL)
			{
				CWnd* pWndParent = pParentButton->GetParentWnd();
				if (pWndParent != NULL)
				{
					CMFCPopupMenuBar* pWndParentPopupMenuBar = DYNAMIC_DOWNCAST(CMFCPopupMenuBar, pWndParent);

					CPoint ptClient = pt;
					pWndParent->ScreenToClient(&ptClient);

					if (pParentButton->Rect().PtInRect(ptClient))
					{
						// If user clicks second time on the parent button,
						// we should close an active menu on the toolbar/menubar
						// and leave it on the popup menu:
						if (pWndParentPopupMenuBar == NULL && !CMFCPopupMenu::m_pActivePopupMenu->InCommand())
						{
							// Toolbar/menu bar: close an active menu!
							CMFCPopupMenu::m_pActivePopupMenu->SendMessage(WM_CLOSE);
						}

						return TRUE;
					}

					if (pWndParentPopupMenuBar != NULL)
					{
						pWndParentPopupMenuBar->CloseDelayedSubMenu();

						CMFCPopupMenu* pWndParentPopupMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, pWndParentPopupMenuBar->GetParent());

						if (pWndParentPopupMenu != NULL)
						{
							CMFCPopupMenu::MENUAREA_TYPE clickAreaParent = pWndParentPopupMenu->CheckArea(pt);

							switch (clickAreaParent)
							{
							case CMFCPopupMenu::MENU:
							case CMFCPopupMenu::TEAROFF_CAPTION:
							case CMFCPopupMenu::LOGO:
								return FALSE;

							case CMFCPopupMenu::SHADOW_RIGHT:
							case CMFCPopupMenu::SHADOW_BOTTOM:
								pWndParentPopupMenu->SendMessage(WM_CLOSE);
								m_Dlg.SetFocus();

								return TRUE;
							}
						}
					}
				}
			}

			if (!CMFCPopupMenu::m_pActivePopupMenu->InCommand())
			{
				CMFCPopupMenu::m_pActivePopupMenu->SendMessage(WM_CLOSE);

				CWnd* pWndFocus = CWnd::GetFocus();
				if (pWndFocus != NULL && pWndFocus->IsKindOf(RUNTIME_CLASS(CMFCToolBar)))
				{
					m_Dlg.SetFocus();
				}

				if (clickArea != CMFCPopupMenu::OUTSIDE) // Click on shadow
				{
					return TRUE;
				}
			}
		}
		else if (clickArea == CMFCPopupMenu::SHADOW_RIGHT || clickArea == CMFCPopupMenu::SHADOW_BOTTOM)
		{
			CMFCPopupMenu::m_pActivePopupMenu->SendMessage(WM_CLOSE);
			m_Dlg.SetFocus();

			return TRUE;
		}
	}

	return FALSE;
}

BOOL CDialogImpl::ProcessMouseMove(POINT pt)
{
	if (!CMFCToolBar::IsCustomizeMode() && CMFCPopupMenu::m_pActivePopupMenu != NULL)
	{
		CRect rectMenu;
		CMFCPopupMenu::m_pActivePopupMenu->GetWindowRect(rectMenu);

		if (rectMenu.PtInRect(pt) || CMFCPopupMenu::m_pActivePopupMenu->GetMenuBar()->FindDestintationToolBar(pt) != NULL)
		{
			return FALSE; // Default processing
		}

		return TRUE; // Active menu "capturing"
	}

	return FALSE; // Default processing
}

BOOL CDialogImpl::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_SYSKEYDOWN:
	case WM_CONTEXTMENU:
		if (CMFCPopupMenu::m_pActivePopupMenu != NULL && ::IsWindow(CMFCPopupMenu::m_pActivePopupMenu->m_hWnd) && pMsg->wParam == VK_MENU)
		{
			CMFCPopupMenu::m_pActivePopupMenu->SendMessage(WM_CLOSE);
			return TRUE;
		}
		break;

	case WM_SYSKEYUP:
		if (CMFCPopupMenu::m_pActivePopupMenu != NULL && ::IsWindow(CMFCPopupMenu::m_pActivePopupMenu->m_hWnd))
		{
			return TRUE; // To prevent system menu opening
		}
		break;

	case WM_KEYDOWN:
		// Pass keyboard action to the active menu:
		if (CMFCPopupMenu::m_pActivePopupMenu != NULL && ::IsWindow(CMFCPopupMenu::m_pActivePopupMenu->m_hWnd))
		{
			CMFCPopupMenu::m_pActivePopupMenu->SendMessage(WM_KEYDOWN, (int) pMsg->wParam);
			return TRUE;
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

			if (ProcessMouseClick(pt))
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
		if (ProcessMouseClick(CPoint(AFX_GET_X_LPARAM(pMsg->lParam), AFX_GET_Y_LPARAM(pMsg->lParam))))
		{
			return TRUE;
		}
		break;

	case WM_MOUSEWHEEL:
		if (CMFCPopupMenu::m_pActivePopupMenu != NULL && ::IsWindow(CMFCPopupMenu::m_pActivePopupMenu->m_hWnd) && CMFCPopupMenu::m_pActivePopupMenu->IsScrollable())
		{
			CMFCPopupMenu::m_pActivePopupMenu->SendMessage(WM_MOUSEWHEEL, pMsg->wParam, pMsg->lParam);

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

			if (ProcessMouseMove(pt))
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

LRESULT CALLBACK CDialogImpl::DialogMouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	MOUSEHOOKSTRUCT* lpMS = (MOUSEHOOKSTRUCT*) lParam;
	ASSERT(lpMS != NULL);

	if (m_pMenuDlgImpl != NULL)
	{
		switch (wParam)
		{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_NCLBUTTONDOWN:
		case WM_NCRBUTTONDOWN:
		case WM_NCMBUTTONDOWN:
			{
				CPoint ptCursor;
				::GetCursorPos(&ptCursor);

				CRect rectWindow;
				m_pMenuDlgImpl->m_Dlg.GetWindowRect(rectWindow);

				if (!rectWindow.PtInRect(ptCursor))
				{
					m_pMenuDlgImpl->ProcessMouseClick(ptCursor);
				}
			}
		}
	}

	return CallNextHookEx(m_hookMouse, nCode, wParam, lParam);
}

void CDialogImpl::SetActiveMenu(CMFCPopupMenu* pMenu)
{
	CMFCPopupMenu::m_pActivePopupMenu = pMenu;

	if (pMenu != NULL)
	{
		if (m_hookMouse == NULL)
		{
			m_hookMouse = ::SetWindowsHookEx(WH_MOUSE, DialogMouseHookProc, 0, GetCurrentThreadId());
		}

		m_pMenuDlgImpl = this;
	}
	else
	{
		if (m_hookMouse != NULL)
		{
			::UnhookWindowsHookEx(m_hookMouse);
			m_hookMouse = NULL;
		}

		m_pMenuDlgImpl = NULL;
	}

}

void CDialogImpl::OnDestroy()
{
	if (m_pMenuDlgImpl != NULL && m_pMenuDlgImpl->m_Dlg.GetSafeHwnd() == m_Dlg.GetSafeHwnd())
	{
		m_pMenuDlgImpl = NULL;
	}
}

BOOL CDialogImpl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	if (HIWORD(wParam) == 1)
	{
		UINT uiCmd = LOWORD(wParam);

		CMFCToolBar::AddCommandUsage(uiCmd);

		// Simmulate ESC keystroke...
		if (CMFCPopupMenu::m_pActivePopupMenu != NULL && ::IsWindow(CMFCPopupMenu::m_pActivePopupMenu->m_hWnd))
		{
			CMFCPopupMenu::m_pActivePopupMenu->SendMessage(WM_KEYDOWN, VK_ESCAPE);
			return TRUE;
		}

		if (afxUserToolsManager != NULL && afxUserToolsManager->InvokeTool(uiCmd))
		{
			return TRUE;
		}
	}

	return FALSE;
}

void CDialogImpl::OnNcActivate(BOOL& bActive)
{
	// Stay active if WF_STAYACTIVE bit is on:
	if (m_Dlg.m_nFlags & WF_STAYACTIVE)
	{
		bActive = TRUE;
	}

	// But do not stay active if the window is disabled:
	if (!m_Dlg.IsWindowEnabled())
	{
		bActive = FALSE;
	}
}

void CDialogImpl::OnActivate(UINT nState, CWnd* pWndOther)
{
	m_Dlg.m_nFlags &= ~WF_STAYACTIVE;

	// Determine if this window should be active or not:
	CWnd* pWndActive = (nState == WA_INACTIVE) ? pWndOther : &m_Dlg;
	if (pWndActive != NULL)
	{
		BOOL bStayActive = (pWndActive->GetSafeHwnd() == m_Dlg.GetSafeHwnd() || pWndActive->SendMessage(WM_FLOATSTATUS, FS_SYNCACTIVE));

		if (bStayActive)
		{
			m_Dlg.m_nFlags |= WF_STAYACTIVE;
		}
	}
	else
	{
		// Force painting on our non-client area....
		m_Dlg.SendMessage(WM_NCPAINT, 1);
	}
}



