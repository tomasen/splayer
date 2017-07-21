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
#include "afxcontrolbarutil.h"
#include "afxribbonminitoolbar.h"
#include "afxribbonbar.h"
#include "afxglobals.h"
#include "afxcontextmenumanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define AFX_ID_VISIBILITY_TIMER 1

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonMiniToolBar

IMPLEMENT_DYNCREATE(CMFCRibbonMiniToolBar, CMFCRibbonPanelMenu)

CMFCRibbonMiniToolBar* CMFCRibbonMiniToolBar::m_pCurrent = NULL;

CMFCRibbonMiniToolBar::CMFCRibbonMiniToolBar()
{
	if (m_pCurrent != NULL)
	{
		m_pCurrent->SendMessage(WM_CLOSE);
		m_pCurrent = NULL;
	}

	m_wndRibbonBar.m_bIsFloaty = TRUE;
	m_bContextMenuMode = FALSE;
	m_nTransparency = 0;
	m_bWasHovered = FALSE;
	m_bDisableAnimation = TRUE;
}

CMFCRibbonMiniToolBar::~CMFCRibbonMiniToolBar()
{
	ASSERT(m_pCurrent == this);
	m_pCurrent = NULL;

	if (m_bContextMenuMode)
	{
		afxContextMenuManager->SetDontCloseActiveMenu(FALSE);
	}
}

BEGIN_MESSAGE_MAP(CMFCRibbonMiniToolBar, CMFCRibbonPanelMenu)
	//{{AFX_MSG_MAP(CMFCRibbonMiniToolBar)
	ON_WM_TIMER()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CMFCRibbonMiniToolBar::SetCommands(CMFCRibbonBar* pRibbonBar, const CList<UINT,UINT>& lstCommands)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pRibbonBar);

	CArray<CMFCRibbonBaseElement*, CMFCRibbonBaseElement*> arButtons;

	for (POSITION pos = lstCommands.GetHeadPosition(); pos != NULL;)
	{
		UINT uiCmd = lstCommands.GetNext(pos);

		if (uiCmd == 0)
		{
			// Separator on the floaty is not supported
			continue;
		}

		CMFCRibbonBaseElement* pSrcElem = pRibbonBar->FindByID(uiCmd, FALSE);
		if (pSrcElem == NULL)
		{
			continue;
		}

		arButtons.Add(pSrcElem);
	}

	m_wndRibbonBar.AddButtons(pRibbonBar, arButtons, TRUE);
}

BOOL CMFCRibbonMiniToolBar::Show(int x, int y)
{
	ASSERT_VALID(this);

	CSize size = m_wndRibbonBar.CalcSize(FALSE);

	if (!Create(m_wndRibbonBar.m_pRibbonBar, x, y - size.cy - ::GetSystemMetrics (SM_CYCURSOR) / 2, (HMENU) NULL))
	{
		return FALSE;
	}

	m_pCurrent = this;

	ModifyStyleEx(0, WS_EX_LAYERED);

	UpdateTransparency();

	afxGlobalData.SetLayeredAttrib(GetSafeHwnd(), 0, m_nTransparency, LWA_ALPHA);
	return TRUE;
}

BOOL CMFCRibbonMiniToolBar::ShowWithContextMenu(int x, int y, UINT uiMenuResID, CWnd* pWndOwner)
{
	ASSERT_VALID(this);

	if (afxContextMenuManager == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (x == -1 || y == -1)
	{
		return afxContextMenuManager->ShowPopupMenu(uiMenuResID, x, y, pWndOwner);
	}

	CSize size = m_wndRibbonBar.CalcSize(FALSE);

	const int yOffset = 15;

	if (!Create(m_wndRibbonBar.m_pRibbonBar, x, y - size.cy - yOffset, (HMENU) NULL))
	{
		return FALSE;
	}

	m_pCurrent = this;

	m_bContextMenuMode = TRUE;

	ASSERT_VALID(afxContextMenuManager);

	afxContextMenuManager->SetDontCloseActiveMenu();

	m_nMinWidth = size.cx;

	afxContextMenuManager->ShowPopupMenu(uiMenuResID, x, y, pWndOwner);

	m_nMinWidth = 0;

	CMFCPopupMenu* pPopup = CMFCPopupMenu::GetActiveMenu();

	if (pPopup != NULL)
	{
		ASSERT_VALID(pPopup);
		pPopup->m_hwndConnectedFloaty = GetSafeHwnd();

		CRect rectMenu;
		pPopup->GetWindowRect(&rectMenu);

		if (rectMenu.top < y)
		{
			int cyScreen = GetSystemMetrics(SM_CYMAXIMIZED) - (GetSystemMetrics(SM_CYSCREEN) - GetSystemMetrics(SM_CYMAXIMIZED));
			if (rectMenu.bottom + size.cy + yOffset < cyScreen)
			{
				SetWindowPos (NULL, rectMenu.left, rectMenu.bottom + yOffset, -1, -1, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
			}
			else
			{
				SetWindowPos (NULL, rectMenu.left, rectMenu.top - size.cy - yOffset, -1, -1, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
			}
		}
	}

	return TRUE;
}

void CMFCRibbonMiniToolBar::OnTimer(UINT_PTR nIDEvent)
{
	CMFCRibbonPanelMenu::OnTimer(nIDEvent);

	if (nIDEvent != AFX_ID_VISIBILITY_TIMER)
	{
		return;
	}

	if (m_bContextMenuMode)
	{
		KillTimer(AFX_ID_VISIBILITY_TIMER);
		return;
	}

	if (UpdateTransparency())
	{
		afxGlobalData.SetLayeredAttrib(GetSafeHwnd(), 0, m_nTransparency, LWA_ALPHA);
	}
}

int CMFCRibbonMiniToolBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (!m_bContextMenuMode && (GetExStyle() & WS_EX_LAYOUTRTL))
	{
		m_iShadowSize = 0;
	}

	if (CMFCRibbonPanelMenu::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_bContextMenuMode)
	{
		SetTimer(AFX_ID_VISIBILITY_TIMER, 100, NULL);
	}

	return 0;
}

BOOL CMFCRibbonMiniToolBar::UpdateTransparency()
{
	BYTE nTransparency = 0;

	if (m_wndRibbonBar.GetPanel()->GetDroppedDown() != NULL || m_wndRibbonBar.GetPanel()->GetHighlighted() != NULL || m_wndRibbonBar.GetPanel()->GetPressed() != NULL)
	{
		nTransparency = 255;
	}
	else
	{
		CRect rect;
		GetWindowRect(rect);

		CPoint ptCursor;
		::GetCursorPos(&ptCursor);

		if (rect.PtInRect(ptCursor))
		{
			m_bWasHovered = TRUE;
			nTransparency = 255;
		}
		else
		{
			const int x = ptCursor.x;
			const int y = ptCursor.y;

			int dx = 0;
			int dy = 0;

			if (x < rect.left)
			{
				dx = rect.left - x;
			}
			else if (x > rect.right)
			{
				dx = x - rect.right;
			}

			if (y < rect.top)
			{
				dy = rect.top - y;
			}
			else if (y > rect.bottom)
			{
				dy = y - rect.bottom;
			}

			const int nDistance = max(dx, dy);
			const int nMaxShowDistance = m_bWasHovered ? 66 : 22;
			const int nDimissDistance = m_bWasHovered ? 176 : 44;

			if (nDistance > nDimissDistance)
			{
				PostMessage(WM_CLOSE);
				return FALSE;
			}

			if (nDistance < nMaxShowDistance)
			{
				float fMaxShowDistance = (float)nMaxShowDistance;
				float fDistance = (float)nDistance;
				float fTransparencyPct = (fMaxShowDistance - fDistance) / fMaxShowDistance;
				float fTransparency = (float)(255 * fTransparencyPct);
				nTransparency = (BYTE)fTransparency;
			}
		}
	}

	if (m_nTransparency == nTransparency)
	{
		return FALSE;
	}

	m_nTransparency = nTransparency;
	return TRUE;
}


