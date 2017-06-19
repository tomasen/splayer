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

#include <afxpriv.h>
#include "afxribbonres.h"
#include "afxtoolbarscustomizedialog.h"
#include "afxtoolbarslistpropertypage.h"
#include "afxtoolbar.h"
#include "afxtoolbarbutton.h"
#include "afxpopupmenubar.h"
#include "afxtoolbarnamedialog.h"
#include "afxcommandmanager.h"
#include "afxmdiframewndex.h"
#include "afxoleipframewndex.h"
#include "afxframewndex.h"
#include "afxdropdowntoolbar.h"
#include "afxpaneframewnd.h"

IMPLEMENT_DYNCREATE(CMFCToolBarsCommandsPropertyPage, CPropertyPage)
IMPLEMENT_DYNCREATE(CMFCToolBarsListPropertyPage, CPropertyPage)

extern CObList afxAllToolBars;

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarsCommandsPropertyPage property page

CMFCToolBarsCommandsPropertyPage::CMFCToolBarsCommandsPropertyPage() : CPropertyPage(CMFCToolBarsCommandsPropertyPage::IDD)
{
	//{{AFX_DATA_INIT(CMFCToolBarsCommandsPropertyPage)
	m_strButtonDescription = _T("");
	//}}AFX_DATA_INIT
}

CMFCToolBarsCommandsPropertyPage::~CMFCToolBarsCommandsPropertyPage()
{
}

void CMFCToolBarsCommandsPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMFCToolBarsCommandsPropertyPage)
	DDX_Control(pDX, IDC_AFXBARRES_CATEGORY, m_wndCategory);
	DDX_Control(pDX, IDC_AFXBARRES_USER_TOOLS, m_wndTools);
	DDX_Text(pDX, IDC_AFXBARRES_BUTTON_DESCR, m_strButtonDescription);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMFCToolBarsCommandsPropertyPage, CPropertyPage)
	//{{AFX_MSG_MAP(CMFCToolBarsCommandsPropertyPage)
	ON_LBN_SELCHANGE(IDC_AFXBARRES_USER_TOOLS, &CMFCToolBarsCommandsPropertyPage::OnSelchangeUserTools)
	ON_LBN_SELCHANGE(IDC_AFXBARRES_CATEGORY, &CMFCToolBarsCommandsPropertyPage::OnSelchangeCategory)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CMFCToolBarsCommandsPropertyPage::OnSelchangeCategory()
{
	UpdateData();

	int iSelIndex = m_wndCategory.GetCurSel();
	if (iSelIndex == LB_ERR)
	{
		ASSERT(FALSE);
		return;
	}

	CWaitCursor wait;
	m_wndTools.SetRedraw(FALSE);

	m_wndTools.ResetContent();

	// Only "All commands" list shoud be sorted!
	CString strCategory;
	m_wndCategory.GetText(iSelIndex, strCategory);

	BOOL bAllCommands = (strCategory == m_strAllCategory);

	OnChangeSelButton(NULL);

	CObList* pCategoryButtonsList =
		(CObList*) m_wndCategory.GetItemData(iSelIndex);
	ASSERT_VALID(pCategoryButtonsList);

	CMFCToolBarsCustomizeDialog* pWndParent = DYNAMIC_DOWNCAST(CMFCToolBarsCustomizeDialog, GetParent());
	ENSURE(pWndParent != NULL);

	for (POSITION pos = pCategoryButtonsList->GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarButton* pButton = (CMFCToolBarButton*) pCategoryButtonsList->GetNext(pos);
		ENSURE(pButton != NULL);
		ASSERT_VALID(pButton);

		pButton->m_bUserButton = pButton->m_nID != (UINT) -1 && afxCommandManager->GetCmdImage(pButton->m_nID, FALSE) == -1;

		CString strText = pButton->m_strText;

		if (!pButton->m_strTextCustom.IsEmpty() &&
			(bAllCommands || pWndParent->GetCountInCategory(strText, *pCategoryButtonsList) > 1))
		{
			strText = pButton->m_strTextCustom;
		}

		int iIndex = -1;

		if (bAllCommands)
		{
			// Insert sortable:
			for (int i = 0; iIndex == -1 && i < m_wndTools.GetCount(); i ++)
			{
				CString strCommand;
				m_wndTools.GetText(i, strCommand);

				if (strCommand > strText)
				{
					iIndex = m_wndTools.InsertString(i, strText);
				}
			}
		}

		if (iIndex == -1) // Not inserted yet
		{
			iIndex = m_wndTools.AddString(strText);
		}

		m_wndTools.SetItemData(iIndex, (DWORD_PTR) pButton);
	}

	m_wndTools.SetRedraw(TRUE);
}

void CMFCToolBarsCommandsPropertyPage::OnSelchangeUserTools()
{
	int iIndex = m_wndTools.GetCurSel();
	if (iIndex == LB_ERR)
	{
		OnChangeSelButton(NULL);
	}
	else
	{
		OnChangeSelButton((CMFCToolBarButton*) m_wndTools.GetItemData(iIndex));
	}
}

BOOL CMFCToolBarsCommandsPropertyPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	CMFCToolBarsCustomizeDialog* pWndParent = DYNAMIC_DOWNCAST(CMFCToolBarsCustomizeDialog, GetParent());
	ENSURE(pWndParent != NULL);

	pWndParent->FillCategoriesListBox(m_wndCategory);

	m_wndCategory.SetCurSel(0);
	OnSelchangeCategory();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMFCToolBarsCommandsPropertyPage::OnChangeSelButton(CMFCToolBarButton* pSelButton)
{
	m_strButtonDescription = _T("");

	if (pSelButton != NULL)
	{
		if (pSelButton->m_nID == 0)
		{
			m_strButtonDescription = pSelButton->m_strText;
		}
		else
		{
			CFrameWnd* pParent = GetParentFrame();
			if (pParent != NULL && pParent->GetSafeHwnd() != NULL)
			{
				pParent->GetMessageString(pSelButton->m_nID, m_strButtonDescription);
			}
		}
	}

	m_pSelButton = pSelButton;
	UpdateData(FALSE);
}

void CMFCToolBarsCommandsPropertyPage::SetUserCategory(LPCTSTR lpszCategory)
{
	ENSURE(lpszCategory != NULL);
	m_strUserCategory = lpszCategory;
}

void CMFCToolBarsCommandsPropertyPage::SetAllCategory(LPCTSTR lpszCategory)
{
	ENSURE(lpszCategory != NULL);
	m_strAllCategory = lpszCategory;
}

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarsListPropertyPage property page

CMFCToolBarsListPropertyPage::CMFCToolBarsListPropertyPage(CFrameWnd* pParentFrame) :
	CPropertyPage(CMFCToolBarsListPropertyPage::IDD), m_bUserDefinedToolbars(FALSE), m_pParentFrame(pParentFrame)
{
	//{{AFX_DATA_INIT(CMFCToolBarsListPropertyPage)
	m_bTextLabels = FALSE;
	//}}AFX_DATA_INIT

	m_pSelectedToolbar = NULL;
	ASSERT_VALID(m_pParentFrame);
}

CMFCToolBarsListPropertyPage::~CMFCToolBarsListPropertyPage()
{
}

void CMFCToolBarsListPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMFCToolBarsListPropertyPage)
	DDX_Control(pDX, IDC_AFXBARRES_TEXT_LABELS, m_wndTextLabels);
	DDX_Control(pDX, IDC_AFXBARRES_RENAME_TOOLBAR, m_bntRenameToolbar);
	DDX_Control(pDX, IDC_AFXBARRES_NEW_TOOLBAR, m_btnNewToolbar);
	DDX_Control(pDX, IDC_AFXBARRES_DELETE_TOOLBAR, m_btnDelete);
	DDX_Control(pDX, IDC_AFXBARRES_RESET, m_btnReset);
	DDX_Control(pDX, IDC_AFXBARRES_TOOLBAR_LIST, m_wndToolbarList);
	DDX_Check(pDX, IDC_AFXBARRES_TEXT_LABELS, m_bTextLabels);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMFCToolBarsListPropertyPage, CPropertyPage)
	//{{AFX_MSG_MAP(CMFCToolBarsListPropertyPage)
	ON_LBN_SELCHANGE(IDC_AFXBARRES_TOOLBAR_LIST, &CMFCToolBarsListPropertyPage::OnSelchangeToolbarList)
	ON_LBN_DBLCLK(IDC_AFXBARRES_TOOLBAR_LIST, &CMFCToolBarsListPropertyPage::OnDblClkToolBarList)
	ON_BN_CLICKED(IDC_AFXBARRES_RESET, &CMFCToolBarsListPropertyPage::OnResetToolbar)
	ON_BN_CLICKED(IDC_AFXBARRES_RESET_ALL, &CMFCToolBarsListPropertyPage::OnResetAllToolbars)
	ON_BN_CLICKED(IDC_AFXBARRES_DELETE_TOOLBAR, &CMFCToolBarsListPropertyPage::OnDeleteToolbar)
	ON_BN_CLICKED(IDC_AFXBARRES_NEW_TOOLBAR, &CMFCToolBarsListPropertyPage::OnNewToolbar)
	ON_BN_CLICKED(IDC_AFXBARRES_RENAME_TOOLBAR, &CMFCToolBarsListPropertyPage::OnRenameToolbar)
	ON_BN_CLICKED(IDC_AFXBARRES_TEXT_LABELS, &CMFCToolBarsListPropertyPage::OnTextLabels)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CMFCToolBarsListPropertyPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	if (!m_bUserDefinedToolbars)
	{
		m_btnNewToolbar.EnableWindow(FALSE);

		m_btnNewToolbar.ShowWindow(SW_HIDE);
		m_btnDelete.ShowWindow(SW_HIDE);
		m_bntRenameToolbar.ShowWindow(SW_HIDE);
	}

	for (POSITION pos = afxAllToolBars.GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBar* pToolBar = (CMFCToolBar*) afxAllToolBars.GetNext(pos);
		ENSURE(pToolBar != NULL);

		if (CWnd::FromHandlePermanent(pToolBar->m_hWnd) != NULL)
		{
			ASSERT_VALID(pToolBar);

			// Don't add dropdown toolbars!
			if (!pToolBar->IsKindOf(RUNTIME_CLASS(CMFCDropDownToolBar)))
			{
				// Check, if toolbar belongs to this dialog's parent main frame window
				if (m_pParentFrame->GetTopLevelFrame() == pToolBar->GetTopLevelFrame() && pToolBar->AllowShowOnList() && !pToolBar->m_bMasked)
				{
					CString strName;
					pToolBar->GetWindowText(strName);

					if (strName.IsEmpty())
					{
						ENSURE(strName.LoadString(IDS_AFXBARRES_UNTITLED_TOOLBAR));
					}

					int iIndex = m_wndToolbarList.AddString(strName);
					m_wndToolbarList.SetItemData(iIndex, (DWORD_PTR) pToolBar);

					if (pToolBar->GetStyle() & WS_VISIBLE)
					{
						m_wndToolbarList.SetCheck(iIndex, 1);
					}

					m_wndToolbarList.EnableCheck(iIndex, pToolBar->CanBeClosed());
				}
			}
		}
	}

	CMFCToolBarsCustomizeDialog* pWndParent = DYNAMIC_DOWNCAST(CMFCToolBarsCustomizeDialog, GetParent());
	ENSURE(pWndParent != NULL);

	if ((pWndParent->GetFlags() & AFX_CUSTOMIZE_TEXT_LABELS) == 0)
	{
		m_wndTextLabels.ShowWindow(SW_HIDE);
	}

	if (m_wndToolbarList.GetCount() > 0)
	{
		m_wndToolbarList.SetCurSel(0);
		OnSelchangeToolbarList();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMFCToolBarsListPropertyPage::OnSelchangeToolbarList()
{
	int iIndex = m_wndToolbarList.GetCurSel();
	if (iIndex == LB_ERR)
	{
		m_pSelectedToolbar = NULL;
		m_btnReset.EnableWindow(FALSE);
		m_btnDelete.EnableWindow(FALSE);
		m_bntRenameToolbar.EnableWindow(FALSE);
		m_wndTextLabels.EnableWindow(FALSE);
		return;
	}

	m_pSelectedToolbar = (CMFCToolBar*) m_wndToolbarList.GetItemData(iIndex);
	ASSERT_VALID(m_pSelectedToolbar);

	m_btnReset.EnableWindow(m_pSelectedToolbar->CanBeRestored());
	m_btnDelete.EnableWindow(m_pSelectedToolbar->IsUserDefined());
	m_bntRenameToolbar.EnableWindow(m_pSelectedToolbar->IsUserDefined());
	m_wndTextLabels.EnableWindow(m_pSelectedToolbar->AllowChangeTextLabels());

	m_bTextLabels = m_pSelectedToolbar->AreTextLabels();
	UpdateData(FALSE);
}

void CMFCToolBarsListPropertyPage::OnDblClkToolBarList()
{
	int iIndex = m_wndToolbarList.GetCurSel();
	if (iIndex != LB_ERR)
	{
		m_pSelectedToolbar = (CMFCToolBar*) m_wndToolbarList.GetItemData(iIndex);
		ASSERT_VALID(m_pSelectedToolbar);

		if (m_pSelectedToolbar->CanBeClosed())
		{
			m_wndToolbarList.SetCheck(iIndex, !m_wndToolbarList.GetCheck(iIndex));
		}
		else
		{
			MessageBeep((UINT) -1);
		}
	}

	OnSelchangeToolbarList();
}

void CMFCToolBarsListPropertyPage::ShowToolBar(CMFCToolBar* pToolBar, BOOL bShow)
{
	if (m_wndToolbarList.GetSafeHwnd() == NULL)
	{
		return;
	}

	for (int i = 0; i < m_wndToolbarList.GetCount(); i ++)
	{
		CMFCToolBar* pListToolBar = (CMFCToolBar*) m_wndToolbarList.GetItemData(i);
		ASSERT_VALID(pListToolBar);

		if (pListToolBar == pToolBar)
		{
			m_wndToolbarList.SetCheck(i, bShow);
			break;
		}
	}
}

void CMFCToolBarsListPropertyPage::OnResetToolbar()
{
	if (m_pSelectedToolbar == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	ASSERT_VALID(m_pSelectedToolbar);
	ASSERT(m_pSelectedToolbar->CanBeRestored());

	CString strName;
	m_pSelectedToolbar->GetWindowText(strName);

	CString strPrompt;
	strPrompt.Format(IDS_AFXBARRES_RESET_TOOLBAR_FMT, strName);

	if (AfxMessageBox(strPrompt, MB_YESNO | MB_ICONQUESTION) != IDYES)
	{
		return;
	}

	m_pSelectedToolbar->RestoreOriginalstate();
}

void CMFCToolBarsListPropertyPage::OnResetAllToolbars()
{
	CString strPrompt;
	ENSURE(strPrompt.LoadString(IDS_AFXBARRES_RESET_ALL_TOOLBARS));

	if (AfxMessageBox(strPrompt, MB_YESNO | MB_ICONQUESTION) != IDYES)
	{
		return;
	}

	afxCommandManager->ClearAllCmdImages();

	// Fill image hash by the default image ids:
	for (POSITION pos = CMFCToolBar::m_DefaultImages.GetStartPosition(); pos != NULL;)
	{
		UINT uiCmdId;
		int iImage;

		CMFCToolBar::m_DefaultImages.GetNextAssoc(pos, uiCmdId, iImage);
		afxCommandManager->SetCmdImage(uiCmdId, iImage, FALSE);
	}

	for (int i = 0; i < m_wndToolbarList.GetCount(); i ++)
	{
		CMFCToolBar* pListToolBar = (CMFCToolBar*) m_wndToolbarList.GetItemData(i);
		ASSERT_VALID(pListToolBar);

		if (pListToolBar->CanBeRestored())
		{
			pListToolBar->RestoreOriginalstate();
		}
	}
}

void CMFCToolBarsListPropertyPage::OnDeleteToolbar()
{
	if (m_pSelectedToolbar == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	ASSERT_VALID(m_pSelectedToolbar);
	ASSERT(m_pSelectedToolbar->IsUserDefined());

	CFrameWnd* pParentFrame = GetParentFrame();
	if (pParentFrame == NULL)
	{
		MessageBeep(MB_ICONASTERISK);
		return;
	}

	CString strName;
	m_pSelectedToolbar->GetWindowText(strName);

	CString strPrompt;
	strPrompt.Format(IDS_AFXBARRES_DELETE_TOOLBAR_FMT, strName);

	if (AfxMessageBox(strPrompt, MB_YESNO | MB_ICONQUESTION) != IDYES)
	{
		return;
	}

	if (pParentFrame->SendMessage(AFX_WM_DELETETOOLBAR, 0, (LPARAM) m_pSelectedToolbar) == 0)
	{
		MessageBeep(MB_ICONASTERISK);
		return;
	}

	m_wndToolbarList.DeleteString(m_wndToolbarList.GetCurSel());
	m_wndToolbarList.SetCurSel(0);
	OnSelchangeToolbarList();
}

void CMFCToolBarsListPropertyPage::OnNewToolbar()
{
	CMFCToolBarNameDialog dlg(this);
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	CString strToolbarName = dlg.m_strToolbarName;

	CFrameWnd* pParentFrame = GetParentFrame();
	if (pParentFrame == NULL)
	{
		MessageBeep(MB_ICONASTERISK);
		return;
	}

	CMFCToolBar* pNewToolbar = (CMFCToolBar*)pParentFrame->SendMessage(AFX_WM_CREATETOOLBAR, 0, (LPARAM)(LPCTSTR) strToolbarName);
	if (pNewToolbar == NULL)
	{
		return;
	}

	ASSERT_VALID(pNewToolbar);

	int iIndex = m_wndToolbarList.AddString(strToolbarName);
	m_wndToolbarList.SetItemData(iIndex, (DWORD_PTR) pNewToolbar);

	m_wndToolbarList.SetCheck(iIndex, 1);
	m_wndToolbarList.SetCurSel(iIndex);
	m_wndToolbarList.SetTopIndex(iIndex);

	OnSelchangeToolbarList();
}

void CMFCToolBarsListPropertyPage::OnRenameToolbar()
{
	if (m_pSelectedToolbar == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	ASSERT_VALID(m_pSelectedToolbar);
	ASSERT(m_pSelectedToolbar->IsUserDefined());

	CMFCToolBarNameDialog dlg(this);
	m_pSelectedToolbar->GetWindowText(dlg.m_strToolbarName);

	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	CString strToolbarName = dlg.m_strToolbarName;

	m_pSelectedToolbar->SetWindowText(strToolbarName);
	if (m_pSelectedToolbar->IsFloating())
	{
		// Change floating frame title:
		CPaneFrameWnd* pParentMiniFrame = m_pSelectedToolbar->GetParentMiniFrame();
		if (pParentMiniFrame != NULL)
		{
			pParentMiniFrame->SetWindowText(strToolbarName);
			pParentMiniFrame->RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
		}
	}

	m_wndToolbarList.DeleteString(m_wndToolbarList.GetCurSel());

	int iIndex = m_wndToolbarList.AddString(strToolbarName);
	m_wndToolbarList.SetItemData(iIndex, (DWORD_PTR) m_pSelectedToolbar);

	if (m_pSelectedToolbar->GetStyle() & WS_VISIBLE)
	{
		m_wndToolbarList.SetCheck(iIndex, 1);
	}

	m_wndToolbarList.SetCurSel(iIndex);
	m_wndToolbarList.SetTopIndex(iIndex);

	OnSelchangeToolbarList();
}

BOOL CMFCToolBarsListPropertyPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
	UINT uiCode = HIWORD(wParam);
	UINT uiID = LOWORD(wParam);

	if (uiCode == CLBN_CHKCHANGE && uiID == IDC_AFXBARRES_TOOLBAR_LIST)
	{
		int iIndex = m_wndToolbarList.GetCurSel();
		if (iIndex != LB_ERR)
		{
			CMFCToolBar* pToolbar = (CMFCToolBar*) m_wndToolbarList.GetItemData(iIndex);
			ASSERT_VALID(pToolbar);

			if (pToolbar->CanBeClosed())
			{
				// Show/hide toolbar:
				pToolbar->ShowPane(m_wndToolbarList.GetCheck(iIndex), FALSE, TRUE);
			}
			else if (m_wndToolbarList.GetCheck(iIndex) == 0)
			{
				// Toolbar should be visible always!
				m_wndToolbarList.SetCheck(iIndex, TRUE);
				MessageBeep((UINT) -1);
			}
		}
	}

	return CPropertyPage::OnCommand(wParam, lParam);
}

void CMFCToolBarsListPropertyPage::OnTextLabels()
{
	UpdateData();

	ASSERT_VALID(m_pSelectedToolbar);
	m_pSelectedToolbar->EnableTextLabels(m_bTextLabels);
}



