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
#include "afxcolormenubutton.h"
#include "afxpane.h"
#include "afxcolorpopupmenu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCColorPopupMenu

IMPLEMENT_DYNAMIC(CMFCColorPopupMenu, CMFCPopupMenu)

BEGIN_MESSAGE_MAP(CMFCColorPopupMenu, CMFCPopupMenu)
	//{{AFX_MSG_MAP(CMFCColorPopupMenu)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CMFCColorPopupMenu::~CMFCColorPopupMenu()
{
}

/////////////////////////////////////////////////////////////////////////////
// CMFCColorPopupMenu message handlers

int CMFCColorPopupMenu::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMFCToolBar::IsCustomizeMode() && !m_bEnabledInCustomizeMode)
	{
		// Don't show color popup in cistomization mode
		return -1;
	}

	if (CMiniFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	DWORD toolbarStyle = AFX_DEFAULT_TOOLBAR_STYLE;
	if (GetAnimationType() != NO_ANIMATION && !CMFCToolBar::IsCustomizeMode())
	{
		toolbarStyle &= ~WS_VISIBLE;
	}

	if (!m_wndColorBar.Create(this, toolbarStyle | CBRS_TOOLTIPS | CBRS_FLYBY, 1))
	{
		TRACE(_T("Can't create popup menu bar\n"));
		return -1;
	}

	CWnd* pWndParent = GetParent();
	ASSERT_VALID(pWndParent);

	m_wndColorBar.SetOwner(pWndParent);
	m_wndColorBar.SetPaneStyle(m_wndColorBar.GetPaneStyle() | CBRS_TOOLTIPS);

	ActivatePopupMenu(AFXGetTopLevelFrame(pWndParent), this);
	RecalcLayout();
	return 0;
}

CPane* CMFCColorPopupMenu::CreateTearOffBar(CFrameWnd* pWndMain, UINT uiID, LPCTSTR lpszName)
{
	ASSERT_VALID(pWndMain);
	ENSURE(lpszName != NULL);
	ENSURE(uiID != 0);

	CMFCColorMenuButton* pColorMenuButton = DYNAMIC_DOWNCAST(CMFCColorMenuButton, GetParentButton());
	if (pColorMenuButton == NULL)
	{
		ASSERT(FALSE);
		return NULL;
	}

	CMFCColorBar* pColorBar = new CMFCColorBar(m_wndColorBar, pColorMenuButton->m_nID);

	if (!pColorBar->Create(pWndMain, AFX_DEFAULT_TOOLBAR_STYLE, uiID))
	{
		TRACE0("Failed to create a new toolbar!\n");
		delete pColorBar;
		return NULL;
	}

	pColorBar->SetWindowText(lpszName);
	pColorBar->SetPaneStyle(pColorBar->GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	pColorBar->EnableDocking(CBRS_ALIGN_ANY);

	return pColorBar;
}



