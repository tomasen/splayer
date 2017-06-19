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

#include "afxglobals.h"
#include "afxtoolbarsmenupropertypage.h"
#include "afxmenubar.h"
#include "afxmenuhash.h"
#include "afxpopupmenu.h"
#include "afxcontextmenumanager.h"
#include "afxmultidoctemplateex.h"
#include "afxtoolbarscustomizedialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern CObList afxAllToolBars;

CPoint CMFCToolBarsMenuPropertyPage::m_ptMenuLastPos = CPoint(100, 100);

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarsMenuPropertyPage property page

IMPLEMENT_DYNCREATE(CMFCToolBarsMenuPropertyPage, CPropertyPage)

CMFCToolBarsMenuPropertyPage::CMFCToolBarsMenuPropertyPage(CFrameWnd* pParentFrame, BOOL bAutoSet) :
	CPropertyPage(CMFCToolBarsMenuPropertyPage::IDD), m_pParentFrame(pParentFrame), m_bAutoSet(bAutoSet)
{
	//{{AFX_DATA_INIT(CMFCToolBarsMenuPropertyPage)
	m_strMenuDescr = _T("");
	m_strContextMenuName = _T("");
	m_strMenuName = _T("");
	m_iMenuAnimationType = (int) CMFCPopupMenu::m_AnimationType;
	m_bMenuShadows = CMFCMenuBar::IsMenuShadows();
	//}}AFX_DATA_INIT

	m_pMenuBar = NULL;
	m_hmenuCurr = NULL;
	m_hmenuSelected = NULL;
	m_pContextMenu = NULL;
	m_bIsDefaultMDIMenu = FALSE;
	m_uiContextMenuResId = 0;
}

CMFCToolBarsMenuPropertyPage::~CMFCToolBarsMenuPropertyPage()
{
}

void CMFCToolBarsMenuPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMFCToolBarsMenuPropertyPage)
	DDX_Control(pDX, IDC_AFXBARRES_MENU_ANIMATION_LABEL, m_wndMenuAnimationsLabel);
	DDX_Control(pDX, IDC_AFXBARRES_MENU_ANIMATION, m_wndMenuAnimations);
	DDX_Control(pDX, IDC_AFXBARRES_MENU_SHADOWS, m_wndMenuShadows);
	DDX_Control(pDX, IDC_AFXBARRES_CONTEXT_FRAME, m_wndContextFrame);
	DDX_Control(pDX, IDC_AFXBARRES_CONTEXT_HINT, m_wndContextHint);
	DDX_Control(pDX, IDC_AFXBARRES_RESET_MENU, m_wndResetMenuButton);
	DDX_Control(pDX, IDC_AFXBARRES_CONTEXT_MENU_CAPTION, m_wndContextMenuCaption);
	DDX_Control(pDX, IDC_AFXBARRES_CONTEXT_MENU_LIST, m_wndContextMenus);
	DDX_Control(pDX, IDC_AFXBARRES_TEMPL_ICON, m_wndIcon);
	DDX_Control(pDX, IDC_AFXBARRES_MENU_LIST, m_wndMenuesList);
	DDX_Text(pDX, IDC_AFXBARRES_MENU_DESCRIPTION, m_strMenuDescr);
	DDX_CBString(pDX, IDC_AFXBARRES_CONTEXT_MENU_LIST, m_strContextMenuName);
	DDX_CBString(pDX, IDC_AFXBARRES_MENU_LIST, m_strMenuName);
	DDX_CBIndex(pDX, IDC_AFXBARRES_MENU_ANIMATION, m_iMenuAnimationType);
	DDX_Check(pDX, IDC_AFXBARRES_MENU_SHADOWS, m_bMenuShadows);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMFCToolBarsMenuPropertyPage, CPropertyPage)
	//{{AFX_MSG_MAP(CMFCToolBarsMenuPropertyPage)
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_AFXBARRES_MENU_LIST, &CMFCToolBarsMenuPropertyPage::OnSelchangeMenuList)
	ON_CBN_SELCHANGE(IDC_AFXBARRES_CONTEXT_MENU_LIST, &CMFCToolBarsMenuPropertyPage::OnSelchangeContextMenuList)
	ON_BN_CLICKED(IDC_AFXBARRES_RESET_MENU, &CMFCToolBarsMenuPropertyPage::OnResetMenu)
	ON_BN_CLICKED(IDC_AFXBARRES_RESET_FRAME_MENU, &CMFCToolBarsMenuPropertyPage::OnResetFrameMenu)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarsMenuPropertyPage message handlers

BOOL CMFCToolBarsMenuPropertyPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	if (m_iMenuAnimationType == (int) CMFCPopupMenu::SYSTEM_DEFAULT_ANIMATION)
	{
		m_iMenuAnimationType = m_wndMenuAnimations.GetCount() - 1;
		UpdateData(FALSE);
	}

	POSITION pos = NULL;

	// Find application Menu Bar object(assume that only one):
	for (pos = afxAllToolBars.GetHeadPosition(); m_pMenuBar == NULL && pos != NULL;)
	{
		CMFCToolBar* pToolBar = (CMFCToolBar*) afxAllToolBars.GetNext(pos);
		ENSURE(pToolBar != NULL);

		if (CWnd::FromHandlePermanent(pToolBar->m_hWnd) != NULL)
		{
			ASSERT_VALID(pToolBar);
			m_pMenuBar = DYNAMIC_DOWNCAST(CMFCMenuBar, pToolBar);
		}
	}

	if (m_pMenuBar != NULL)
	{
		m_pMenuBar->m_pMenuPage = this;

		int iCurrMenu = -1;

		// Save MenuBar current menu:
		m_hmenuCurr = m_pMenuBar->GetHMenu();

		m_pMenuBar->OnChangeHot(-1);
		afxMenuHash.SaveMenuBar(m_hmenuCurr, m_pMenuBar);

		// Find all application document templates and fill menues combobox
		// by document template data:
		CDocManager* pDocManager = AfxGetApp()->m_pDocManager;
		if (m_bAutoSet && pDocManager != NULL)
		{
			// Walk all templates in the application:
			for (pos = pDocManager->GetFirstDocTemplatePosition(); pos != NULL;)
			{
				CMultiDocTemplateEx* pTemplate = (CMultiDocTemplateEx*) pDocManager->GetNextDocTemplate(pos);
				ASSERT_VALID(pTemplate);
				ASSERT_KINDOF(CDocTemplate, pTemplate);

				// We are interessing CMultiDocTemplate objects with
				// the shared menu only....
				if (!pTemplate->IsKindOf(RUNTIME_CLASS(CMultiDocTemplate)) || pTemplate->m_hMenuShared == NULL)
				{
					continue;
				}

				// Maybe, the template with same ID is already exist?
				BOOL bIsAlreadyExist = FALSE;
				for (int i = 0; !bIsAlreadyExist && i < m_wndMenuesList.GetCount(); i++)
				{
					CMultiDocTemplateEx* pListTemplate = (CMultiDocTemplateEx*) m_wndMenuesList.GetItemData(i);
					bIsAlreadyExist = pListTemplate != NULL && pListTemplate->GetResId() == pTemplate->GetResId();
				}

				if (!bIsAlreadyExist)
				{
					CString strName;
					pTemplate->GetDocString(strName, CDocTemplate::fileNewName);

					int iIndex = m_wndMenuesList.AddString(strName);
					m_wndMenuesList.SetItemData(iIndex, (DWORD_PTR) pTemplate);

					if (pTemplate->m_hMenuShared == m_hmenuCurr)
					{
						iCurrMenu = iIndex;
					}
				}
			}
		}

		// Add a default frame menu:
		CString strName;
		ENSURE(strName.LoadString(IDS_AFXBARRES_DEFUALT_MENU));

		int iIndex = m_wndMenuesList.AddString(strName);
		m_wndMenuesList.SetItemData(iIndex, (DWORD_PTR) NULL);

		if (iCurrMenu == -1)
		{
			m_bIsDefaultMDIMenu = TRUE;
			iCurrMenu = iIndex;
		}

		m_hmenuSelected = m_hmenuCurr;
		m_wndMenuesList.SetCurSel(iCurrMenu);

		UpdateData(FALSE);
		OnSelchangeMenuList();
	}
	else
	{
		// No menubar found, disable menu selecting engine:
		m_wndMenuesList.EnableWindow(FALSE);
		GetDlgItem(IDC_AFXBARRES_RESET_FRAME_MENU)->EnableWindow(FALSE);

		ENSURE(m_strMenuDescr.LoadString(IDS_AFXBARRES_NO_MENUBAR));

		UpdateData(FALSE);
	}

	// Initialize context menus:
	{
		CString strNoContextMenu;
		ENSURE(strNoContextMenu.LoadString(IDS_AFXBARRES_NO_CONTEXT));

		m_wndContextMenus.AddString(strNoContextMenu);
		m_wndContextMenus.SetCurSel(0);
	}

	if (afxContextMenuManager != NULL)
	{
		CStringList listOfNames;
		afxContextMenuManager->GetMenuNames(listOfNames);

		for (pos = listOfNames.GetHeadPosition(); pos != NULL;)
		{
			CString strName = listOfNames.GetNext(pos);
			m_wndContextMenus.AddString(strName);
		}

		m_wndContextMenuCaption.EnableWindow(m_wndContextMenus.GetCount() > 1);
		m_wndContextMenus.EnableWindow(m_wndContextMenus.GetCount() > 1);
	}
	else
	{
		// Hide all context menus fields:
		m_wndContextMenuCaption.ShowWindow(SW_HIDE);
		m_wndContextMenus.ShowWindow(SW_HIDE);
		m_wndContextHint.ShowWindow(SW_HIDE);
		m_wndContextFrame.ShowWindow(SW_HIDE);
		m_wndResetMenuButton.ShowWindow(SW_HIDE);
	}

	CMFCToolBarsCustomizeDialog* pWndParent = DYNAMIC_DOWNCAST(CMFCToolBarsCustomizeDialog, GetParent());
	ENSURE(pWndParent != NULL);

	if ((pWndParent->GetFlags() & AFX_CUSTOMIZE_MENU_SHADOWS) == 0)
	{
		m_wndMenuShadows.ShowWindow(SW_HIDE);
	}

	if ((pWndParent->GetFlags() & AFX_CUSTOMIZE_MENU_ANIMATIONS) == 0)
	{
		m_wndMenuAnimationsLabel.ShowWindow(SW_HIDE);
		m_wndMenuAnimations.ShowWindow(SW_HIDE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CMFCToolBarsMenuPropertyPage::OnSelchangeMenuList()
{
	UpdateData();

	if (m_pMenuBar == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	if (m_hmenuSelected != NULL)
	{
		m_pMenuBar->OnChangeHot(-1);
		afxMenuHash.SaveMenuBar(m_hmenuSelected, m_pMenuBar);
	}

	int iIndex = m_wndMenuesList.GetCurSel();
	if (iIndex == CB_ERR)
	{
		m_strMenuDescr = _T("");
		m_wndIcon.SetIcon(NULL);
		UpdateData(FALSE);
		m_hmenuSelected = NULL;
		return;
	}

	HICON hicon = NULL;
	HMENU hmenu = NULL;

	CMultiDocTemplateEx* pTemplate = (CMultiDocTemplateEx*) m_wndMenuesList.GetItemData(iIndex);
	if (pTemplate != NULL)
	{
		ASSERT_VALID(pTemplate);

		pTemplate->GetDocString(m_strMenuDescr, CDocTemplate::regFileTypeName);

		hicon = AfxGetApp()->LoadIcon(pTemplate->GetResId());
		if (hicon == NULL)
		{
			hicon = ::LoadIcon(NULL, IDI_APPLICATION);
		}

		hmenu = pTemplate->m_hMenuShared;
	}
	else
	{
		ENSURE(m_strMenuDescr.LoadString(IDS_AFXBARRES_DEFAULT_MENU_DESCR));

		CWnd* pWndMain = AfxGetMainWnd();
		if (pWndMain != NULL)
		{
			hicon = (HICON)(LONG_PTR) GetClassLongPtr(*pWndMain, GCLP_HICON);
		}

		hmenu = m_pMenuBar->GetDefaultMenu();
	}

	ENSURE(hmenu != NULL);

	m_pMenuBar->CreateFromMenu(hmenu);
	m_wndIcon.SetIcon(hicon);

	m_hmenuSelected = hmenu;
	UpdateData(FALSE);
}

void CMFCToolBarsMenuPropertyPage::OnDestroy()
{
	UpdateData();

	CMFCPopupMenu::m_AnimationType = m_iMenuAnimationType == m_wndMenuAnimations.GetCount() - 1 ? CMFCPopupMenu::SYSTEM_DEFAULT_ANIMATION :
	(CMFCPopupMenu::ANIMATION_TYPE) m_iMenuAnimationType;

	if (m_pMenuBar != NULL)
	{
		m_pMenuBar->m_pMenuPage = NULL;

		// Save the selected menu state:
		if (m_hmenuSelected != NULL)
		{
			m_pMenuBar->OnChangeHot(-1); // To close and save all popups
			afxMenuHash.SaveMenuBar(m_hmenuSelected, m_pMenuBar);
		}

		// Restore the current menu:
		if (m_hmenuCurr != NULL)
		{
			m_pMenuBar->CreateFromMenu(m_hmenuCurr);
		}
	}

	// Release the context menu resources:
	if (m_pContextMenu != NULL)
	{
		SaveMenu();
		m_pContextMenu->SendMessage(WM_CLOSE);
	}

	// Update shdows appearance:
	CMFCMenuBar::EnableMenuShadows(m_bMenuShadows);
	CPropertyPage::OnDestroy();
}

void CMFCToolBarsMenuPropertyPage::OnSelchangeContextMenuList()
{
	m_wndResetMenuButton.EnableWindow(FALSE);

	if (afxContextMenuManager == NULL)
	{
		return;
	}

	m_uiContextMenuResId = 0;

	// First, save and close the current menu:
	if (m_pContextMenu != NULL)
	{
		SaveMenu();

		CMFCPopupMenu* pMenu = m_pContextMenu;
		m_pContextMenu = NULL;
		pMenu->SendMessage(WM_CLOSE);
	}

	if (m_wndContextMenus.GetCurSel() <= 0)
	{
		// No is menu selected, nothing to do...
		return;
	}

	UpdateData();

	HMENU hMenu = afxContextMenuManager->GetMenuByName(m_strContextMenuName, &m_uiContextMenuResId);

	if (hMenu == NULL)
	{
		MessageBeep((UINT) -1);
		return;
	}

	HMENU hmenuPopup = ::GetSubMenu(hMenu, 0);
	if (hmenuPopup == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	m_pContextMenu = new CMFCPopupMenu(this, m_strContextMenuName);
	ENSURE(m_pContextMenu != NULL);

	m_pContextMenu->SetAutoDestroy(FALSE);

	if (!m_pContextMenu->Create(GetTopLevelFrame(), m_ptMenuLastPos.x, m_ptMenuLastPos.y, hmenuPopup))
	{
		AfxMessageBox(_T("Can't create context menu!"));
	}
	else
	{
		m_wndResetMenuButton.EnableWindow();
	}
}

void CMFCToolBarsMenuPropertyPage::CloseContextMenu(CMFCPopupMenu* pMenu)
{
	UNUSED_ALWAYS(pMenu);

	if (m_pContextMenu == NULL)
	{
		return;
	}

	ENSURE(m_pContextMenu == pMenu);

	SaveMenu();

	if (m_pContextMenu != NULL)
	{
		m_pContextMenu = NULL;
		m_wndContextMenus.SetCurSel(0);

		m_wndResetMenuButton.EnableWindow(FALSE);
	}

	m_uiContextMenuResId = 0;
}

void CMFCToolBarsMenuPropertyPage::SaveMenu()
{
	if (m_pContextMenu == NULL)
	{
		return;
	}

	// Save current menu position:
	CRect rectMenu;
	m_pContextMenu->GetWindowRect(&rectMenu);
	m_ptMenuLastPos = rectMenu.TopLeft();

	// Save menu context:
	afxMenuHash.SaveMenuBar(m_pContextMenu->m_hMenu, m_pContextMenu->GetMenuBar());
}

void CMFCToolBarsMenuPropertyPage::OnResetMenu()
{
	if (afxContextMenuManager == NULL)
	{
		return;
	}

	ENSURE(m_pContextMenu != NULL);

	{
		CString strPrompt;
		strPrompt.Format(IDS_AFXBARRES_RESET_MENU_FMT, m_strContextMenuName);

		if (AfxMessageBox(strPrompt, MB_YESNO | MB_ICONQUESTION) != IDYES)
		{
			return;
		}
	}

	HMENU hMenu = afxContextMenuManager->GetMenuByName(m_strContextMenuName, &m_uiContextMenuResId);
	if (hMenu == NULL)
	{
		MessageBeep((UINT) -1);
		return;
	}

	m_pContextMenu->GetMenuBar()->ImportFromMenu(::GetSubMenu(hMenu, 0));

	// Send notification to application main frame:
	if (m_pParentFrame != NULL)
	{
		m_pParentFrame->SendMessage(AFX_WM_RESETCONTEXTMENU, (WPARAM) m_uiContextMenuResId, (LPARAM) m_pContextMenu);
	}

	OnSelchangeContextMenuList();

	m_pContextMenu->RecalcLayout();
	m_pContextMenu->GetMenuBar()->Invalidate();
}

void CMFCToolBarsMenuPropertyPage::OnResetFrameMenu()
{
	UpdateData();

	if (m_pMenuBar == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	int iIndex = m_wndMenuesList.GetCurSel();
	if (iIndex == CB_ERR)
	{
		ASSERT(FALSE);
		return;
	}

	{
		CString strPrompt;
		strPrompt.Format(IDS_AFXBARRES_RESET_MENU_FMT, m_strMenuName);

		if (AfxMessageBox(strPrompt, MB_YESNO | MB_ICONQUESTION) != IDYES)
		{
			return;
		}
	}

	HMENU hOldMenu = NULL;

	CMultiDocTemplateEx* pTemplate = (CMultiDocTemplateEx*) m_wndMenuesList.GetItemData(iIndex);
	if (pTemplate != NULL) // Document's menu
	{
		ASSERT_VALID(pTemplate);

		HINSTANCE hInst = AfxFindResourceHandle(MAKEINTRESOURCE(pTemplate->GetResId()), RT_MENU);

		BOOL bIsCurrent = (pTemplate->m_hMenuShared == m_hmenuCurr);

		hOldMenu = pTemplate->m_hMenuShared;

		pTemplate->m_hMenuShared = ::LoadMenu(hInst, MAKEINTRESOURCE(pTemplate->GetResId()));
		m_pMenuBar->CreateFromMenu(pTemplate->m_hMenuShared, FALSE);

		CMFCMenuBar::UpdateMDIChildrenMenus(pTemplate);

		if (m_pParentFrame != NULL)
		{
			if (m_pParentFrame->SendMessage(AFX_WM_RESETMENU, pTemplate->GetResId()))
			{
				m_pMenuBar->AdjustLayout();
			};
		}

		afxMenuHash.SaveMenuBar(pTemplate->m_hMenuShared, m_pMenuBar);

		if (bIsCurrent)
		{
			ASSERT(!m_bIsDefaultMDIMenu);
			m_hmenuCurr = pTemplate->m_hMenuShared;
		}
	}
	else // Frame's default menu
	{
		UINT uiDefMenuResId = m_pMenuBar->GetDefaultMenuResId();
		if (uiDefMenuResId != 0)
		{
			HINSTANCE hInst = AfxFindResourceHandle(MAKEINTRESOURCE(uiDefMenuResId), RT_MENU);

			hOldMenu = m_pMenuBar->m_hDefaultMenu;

			HMENU hDefaultMenu = ::LoadMenu(hInst, MAKEINTRESOURCE(uiDefMenuResId));
			m_pMenuBar->OnDefaultMenuLoaded(hDefaultMenu);

			m_pMenuBar->CreateFromMenu(hDefaultMenu, TRUE);

			if (m_pParentFrame != NULL)
			{
				if (m_pParentFrame->SendMessage(AFX_WM_RESETMENU, uiDefMenuResId))
				{
					m_pMenuBar->AdjustLayout();
				}

				m_pParentFrame->m_hMenuDefault = hDefaultMenu;
			}

			afxMenuHash.SaveMenuBar(hDefaultMenu, m_pMenuBar);

			if (m_bIsDefaultMDIMenu)
			{
				m_hmenuCurr = hDefaultMenu;
			}
		}
	}

	if (m_pParentFrame != NULL)
	{
		m_pParentFrame->OnUpdateFrameMenu(m_hmenuCurr);
	}

	if (hOldMenu != NULL)
	{
		ENSURE(::IsMenu(hOldMenu));

		afxMenuHash.RemoveMenu(hOldMenu);
		::DestroyMenu(hOldMenu);
	}

	m_pMenuBar->Invalidate();
	m_pMenuBar->UpdateWindow();
}

BOOL CMFCToolBarsMenuPropertyPage::SelectMenu(CDocTemplate* pTemplate, BOOL bSaveCurr)
{
	for (int i = 0; i < m_wndMenuesList.GetCount(); i++)
	{
		if ((CDocTemplate*) m_wndMenuesList.GetItemData(i) == pTemplate)
		{
			if (!bSaveCurr)
			{
				m_hmenuSelected = NULL; // To prevent saving
			}

			if (m_pMenuBar != NULL)
			{
				m_hmenuCurr = m_pMenuBar->GetHMenu();
			}

			m_wndMenuesList.SetCurSel(i);
			OnSelchangeMenuList();

			return TRUE;
		}
	}

	return FALSE;
}


