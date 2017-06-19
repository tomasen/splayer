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

#include "afxribbonres.h"
#include "afxcontrolbarutil.h"
#include "afxtoolbarsoptionspropertypage.h"
#include "afxtoolbar.h"
#include "afxmenubar.h"
#include "afxmdiframewndex.h"
#include "afxframewndex.h"
#include "afxtoolbarscustomizedialog.h"
#include "afxvisualmanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarsOptionsPropertyPage property page

IMPLEMENT_DYNCREATE(CMFCToolBarsOptionsPropertyPage, CPropertyPage)

CMFCToolBarsOptionsPropertyPage::CMFCToolBarsOptionsPropertyPage(BOOL bIsMenuBarExist) :
	CPropertyPage(CMFCToolBarsOptionsPropertyPage::IDD), m_bIsMenuBarExist(bIsMenuBarExist)
{
	//{{AFX_DATA_INIT(CMFCToolBarsOptionsPropertyPage)
	m_bShowTooltips = CMFCToolBar::m_bShowTooltips;
	m_bShowShortcutKeys = CMFCToolBar::m_bShowShortcutKeys;
	m_bRecentlyUsedMenus = CMFCMenuBar::m_bRecentlyUsedMenus;
	m_bShowAllMenusDelay = CMFCMenuBar::m_bShowAllMenusDelay;
	m_bLargeIcons = CMFCToolBar::m_bLargeIcons;
	//}}AFX_DATA_INIT
}

CMFCToolBarsOptionsPropertyPage::~CMFCToolBarsOptionsPropertyPage()
{
}

void CMFCToolBarsOptionsPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMFCToolBarsOptionsPropertyPage)
	DDX_Control(pDX, IDC_AFXBARRES_LARGE_ICONS, m_wndLargeIcons);
	DDX_Control(pDX, IDC_AFXBARRES_SHOW_RECENTLY_USED_MENUS, m_wndRUMenus);
	DDX_Control(pDX, IDC_AFXBARRES_RESET_USAGE_DATA, m_wndResetUsageBtn);
	DDX_Control(pDX, IDC_AFX_RU_MENUS_TITLE, m_wndRuMenusLine);
	DDX_Control(pDX, IDC_AFX_RU_MENUS_LINE, m_wndRuMenusTitle);
	DDX_Control(pDX, IDC_AFXBARRES_SHOW_MENUS_DELAY, m_wndShowAllMenusDelay);
	DDX_Control(pDX, IDC_AFXBARRES_SHOW_TOOLTIPS_WITH_KEYS, m_wndShowShortcutKeys);
	DDX_Check(pDX, IDC_AFXBARRES_SHOW_TOOLTIPS, m_bShowTooltips);
	DDX_Check(pDX, IDC_AFXBARRES_SHOW_TOOLTIPS_WITH_KEYS, m_bShowShortcutKeys);
	DDX_Check(pDX, IDC_AFXBARRES_SHOW_RECENTLY_USED_MENUS, m_bRecentlyUsedMenus);
	DDX_Check(pDX, IDC_AFXBARRES_SHOW_MENUS_DELAY, m_bShowAllMenusDelay);
	DDX_Check(pDX, IDC_AFXBARRES_LARGE_ICONS, m_bLargeIcons);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMFCToolBarsOptionsPropertyPage, CPropertyPage)
	//{{AFX_MSG_MAP(CMFCToolBarsOptionsPropertyPage)
	ON_BN_CLICKED(IDC_AFXBARRES_SHOW_TOOLTIPS_WITH_KEYS, &CMFCToolBarsOptionsPropertyPage::OShowTooltipsWithKeys)
	ON_BN_CLICKED(IDC_AFXBARRES_SHOW_TOOLTIPS, &CMFCToolBarsOptionsPropertyPage::OnShowTooltips)
	ON_BN_CLICKED(IDC_AFXBARRES_RESET_USAGE_DATA, &CMFCToolBarsOptionsPropertyPage::OnResetUsageData)
	ON_BN_CLICKED(IDC_AFXBARRES_SHOW_RECENTLY_USED_MENUS, &CMFCToolBarsOptionsPropertyPage::OnShowRecentlyUsedMenus)
	ON_BN_CLICKED(IDC_AFXBARRES_SHOW_MENUS_DELAY, &CMFCToolBarsOptionsPropertyPage::OnShowMenusDelay)
	ON_BN_CLICKED(IDC_AFXBARRES_LARGE_ICONS, &CMFCToolBarsOptionsPropertyPage::OnLargeIcons)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarsOptionsPropertyPage message handlers

BOOL CMFCToolBarsOptionsPropertyPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_wndShowShortcutKeys.EnableWindow(m_bShowTooltips);
	m_wndShowAllMenusDelay.EnableWindow(m_bRecentlyUsedMenus);

	if (CMFCToolBar::m_lstBasicCommands.IsEmpty() || !m_bIsMenuBarExist)
	{
		m_wndRUMenus.ShowWindow(SW_HIDE);
		m_wndRUMenus.EnableWindow(FALSE);

		m_wndResetUsageBtn.ShowWindow(SW_HIDE);
		m_wndResetUsageBtn.EnableWindow(FALSE);

		m_wndRuMenusLine.ShowWindow(SW_HIDE);
		m_wndRuMenusLine.EnableWindow(FALSE);

		m_wndRuMenusTitle.ShowWindow(SW_HIDE);
		m_wndRuMenusTitle.EnableWindow(FALSE);

		m_wndShowAllMenusDelay.ShowWindow(SW_HIDE);
		m_wndShowAllMenusDelay.EnableWindow(FALSE);
	}

	CMFCToolBarsCustomizeDialog* pWndParent = DYNAMIC_DOWNCAST(CMFCToolBarsCustomizeDialog, GetParent());
	ENSURE(pWndParent != NULL);

	if (pWndParent->GetFlags() & AFX_CUSTOMIZE_NO_LARGE_ICONS)
	{
		m_wndLargeIcons.ShowWindow(SW_HIDE);
		m_wndLargeIcons.EnableWindow(FALSE);
		m_bLargeIcons = FALSE;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CMFCToolBarsOptionsPropertyPage::OShowTooltipsWithKeys()
{
	UpdateData();
	CMFCToolBar::m_bShowShortcutKeys = m_bShowShortcutKeys;
}

void CMFCToolBarsOptionsPropertyPage::OnShowTooltips()
{
	UpdateData();
	CMFCToolBar::m_bShowTooltips = m_bShowTooltips;
	m_wndShowShortcutKeys.EnableWindow(m_bShowTooltips);
}

void CMFCToolBarsOptionsPropertyPage::OnResetUsageData()
{
	if (AfxMessageBox(IDS_AFXBARRES_RESET_USAGE_WARNING, MB_YESNO) == IDYES)
	{
		CMFCToolBar::m_UsageCount.Reset();
	}
}

void CMFCToolBarsOptionsPropertyPage::OnShowRecentlyUsedMenus()
{
	UpdateData();
	m_wndShowAllMenusDelay.EnableWindow(m_bRecentlyUsedMenus);
	CMFCMenuBar::m_bRecentlyUsedMenus = m_bRecentlyUsedMenus;
}

void CMFCToolBarsOptionsPropertyPage::OnShowMenusDelay()
{
	UpdateData();
	CMFCMenuBar::m_bShowAllMenusDelay = m_bShowAllMenusDelay;
}

void CMFCToolBarsOptionsPropertyPage::OnLargeIcons()
{
	UpdateData();
	CMFCToolBar::SetLargeIcons(m_bLargeIcons);
}


