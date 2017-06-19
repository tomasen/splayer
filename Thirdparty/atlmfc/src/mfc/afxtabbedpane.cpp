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

#include "afxpanedivider.h"
#include "afxtabbedpane.h"
#include "afxpaneframewnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMFCTabCtrl::Style CTabbedPane::m_StyleTabWnd = CMFCTabCtrl::STYLE_3D;
CArray<COLORREF, COLORREF> CTabbedPane::m_arTabsAutoColors;
BOOL CTabbedPane::m_bIsTabsAutoColor = FALSE;
CList<HWND,HWND> CTabbedPane::m_lstTabbedControlBars;

BOOL CTabbedPane::m_bTabsAlwaysTop = FALSE;
CRuntimeClass* CTabbedPane::m_pTabWndRTC = RUNTIME_CLASS(CMFCTabCtrl);

IMPLEMENT_SERIAL(CTabbedPane, CBaseTabbedPane, VERSIONABLE_SCHEMA | 2)

/////////////////////////////////////////////////////////////////////////////
// CTabbedPane

CTabbedPane::CTabbedPane(BOOL bAutoDestroy) : CBaseTabbedPane(bAutoDestroy)
{
}

CTabbedPane::~CTabbedPane()
{
}

BEGIN_MESSAGE_MAP(CTabbedPane, CBaseTabbedPane)
	//{{AFX_MSG_MAP(CTabbedPane)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabbedPane message handlers

int CTabbedPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	ASSERT_VALID(this);
	if (CBaseTabbedPane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectClient(0, 0, lpCreateStruct->cx, lpCreateStruct->cy);

	ENSURE(m_pTabWnd == NULL);
	ENSURE(m_pTabWndRTC != NULL);

	m_pTabWnd = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_pTabWndRTC->CreateObject());

	if (m_pTabWnd == NULL)
	{
		TRACE0("Failed to dynamically inatantiate a tab window object\n");
		return -1;      // fail to create
	}

	CMFCTabCtrl* pTabWnd = (CMFCTabCtrl*) m_pTabWnd;

	// Create tabs window:
	if (!pTabWnd->Create(m_StyleTabWnd, rectClient, this, 101, CTabbedPane::m_bTabsAlwaysTop ? CMFCTabCtrl::LOCATION_TOP : CMFCTabCtrl::LOCATION_BOTTOM))
	{
		TRACE0("Failed to create tab window\n");
		delete m_pTabWnd;
		m_pTabWnd = NULL;
		return -1;      // fail to create
	}

	m_pTabWnd->m_bActivateTabOnRightClick = TRUE;

	if (m_bIsTabsAutoColor)
	{
		pTabWnd->EnableAutoColor();
		pTabWnd->SetAutoColors(m_arTabsAutoColors);
	}

	pTabWnd->AutoDestroyWindow(FALSE);
	pTabWnd->HideSingleTab();

	pTabWnd->SetTabBorderSize(CMFCVisualManager::GetInstance()->GetDockingTabsBordersSize());
	pTabWnd->m_bEnableWrapping = TRUE;

	m_lstTabbedControlBars.AddTail(GetSafeHwnd());
	return 0;
}

BOOL CTabbedPane::FloatTab(CWnd* pBar, int nTabID, AFX_DOCK_METHOD dockMethod, BOOL bHide)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);

	if (GetTabWnd()->GetTabsNum() > 1)
	{
		return CBaseTabbedPane::FloatTab(pBar, nTabID, dockMethod, bHide);
	}

	return FALSE;
}

BOOL CTabbedPane::DetachPane(CWnd* pBar, BOOL bHide)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);

	if (GetTabWnd()->GetTabsNum() > 0)
	{
		return CBaseTabbedPane::DetachPane(pBar, bHide);
	}

	return FALSE;
}

BOOL CTabbedPane::CheckTabbedBarAlignment()
{
	return TRUE;
}

void CTabbedPane::GetTabArea(CRect& rectTabAreaTop, CRect& rectTabAreaBottom) const
{
	rectTabAreaTop.SetRectEmpty();
	rectTabAreaBottom.SetRectEmpty();

	if (IsTabLocationBottom())
	{
		GetTabWnd()->GetTabsRect(rectTabAreaBottom);
		GetTabWnd()->ClientToScreen(rectTabAreaBottom);
	}
	else
	{
		GetTabWnd()->GetTabsRect(rectTabAreaTop);
		GetTabWnd()->ClientToScreen(rectTabAreaTop);
	}
}

BOOL CTabbedPane::IsTabLocationBottom() const
{
	return(GetTabWnd()->GetLocation() == CMFCTabCtrl::LOCATION_BOTTOM);
}

void CTabbedPane::OnPressCloseButton()
{
	if (m_pTabWnd == NULL)
	{
		return;
	}

	CWnd* pWnd = m_pTabWnd->GetActiveWnd();

	CFrameWnd* pParentFrame = DYNAMIC_DOWNCAST(CFrameWnd, AFXGetParentFrame(this));
	ASSERT_VALID(pParentFrame);

	if (pParentFrame != NULL)
	{
		if (pParentFrame->SendMessage(AFX_WM_ON_PRESS_CLOSE_BUTTON, NULL, (LPARAM)(LPVOID) pWnd))
		{
			return;
		}
	}

	int nVisibleTabNum = m_pTabWnd->GetVisibleTabsNum();

	if (nVisibleTabNum == 1)
	{
		CDockablePane::OnPressCloseButton();
	}

	int nActiveTab = m_pTabWnd->GetActiveTab();
	m_pTabWnd->ShowTab(nActiveTab, FALSE);
}

void __stdcall CTabbedPane::EnableTabAutoColor(BOOL bEnable/* = TRUE*/)
{
	m_bIsTabsAutoColor = bEnable;
	ResetTabs();
}

void __stdcall CTabbedPane::SetTabAutoColors(const CArray<COLORREF, COLORREF>& arColors)
{
	m_arTabsAutoColors.RemoveAll();

	for (int i = 0; i < arColors.GetSize(); i++)
	{
		m_arTabsAutoColors.Add(arColors [i]);
	}

	ResetTabs();
}

void CTabbedPane::OnDestroy()
{
	POSITION pos = m_lstTabbedControlBars.Find(GetSafeHwnd());
	if (pos == NULL)
	{
		ASSERT(FALSE);
	}
	else
	{
		m_lstTabbedControlBars.RemoveAt(pos);
	}

	CBaseTabbedPane::OnDestroy();
}

void __stdcall CTabbedPane::ResetTabs()
{
	for (POSITION pos = m_lstTabbedControlBars.GetHeadPosition(); pos != NULL;)
	{
		HWND hWnd = m_lstTabbedControlBars.GetNext(pos);
		if (!::IsWindow(hWnd))
		{
			continue;
		}

		CTabbedPane* pBar = DYNAMIC_DOWNCAST(CTabbedPane, CWnd::FromHandlePermanent(hWnd));
		if (pBar == NULL)
		{
			continue;
		}

		ASSERT_VALID(pBar);

		CMFCTabCtrl* pTabWnd = pBar->GetTabWnd();
		ASSERT_VALID(pTabWnd);

		pTabWnd->SetTabBorderSize(CMFCVisualManager::GetInstance()->GetDockingTabsBordersSize());
		pTabWnd->SetDrawFrame(CMFCVisualManager::GetInstance()->IsDockingTabHasBorder());

		pTabWnd->ModifyTabStyle(m_StyleTabWnd);
		pTabWnd->RecalcLayout();

		if (m_bIsTabsAutoColor)
		{
			pTabWnd->EnableAutoColor();
			pTabWnd->SetAutoColors(m_arTabsAutoColors);
		}
		else
		{
			pTabWnd->EnableAutoColor(FALSE);

			CArray<COLORREF, COLORREF> arTabsAutoColors;
			pTabWnd->SetAutoColors(arTabsAutoColors);
		}
	}
}



