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
#include "afxribboncustomizedialog.h"
#include "afxribbonbar.h"
#include "afxribboncategory.h"
#include "afxribbonkeyboardcustomizedialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CMFCRibbonCustomizeCategory : public CObject
{
	DECLARE_DYNAMIC(CMFCRibbonCustomizeCategory)

	CString m_strName;
	CList<UINT,UINT> m_lstIDs;
};

IMPLEMENT_DYNAMIC(CMFCRibbonCustomizeCategory, CObject)

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonCustomizeDialog

IMPLEMENT_DYNAMIC(CMFCRibbonCustomizeDialog, CMFCPropertySheet)

CMFCRibbonCustomizeDialog::CMFCRibbonCustomizeDialog(CWnd* pWndParent, CMFCRibbonBar* pRibbon) :
	CMFCPropertySheet(_T(""), pWndParent)
{
	m_psh.dwFlags |= PSH_NOAPPLYNOW;

	m_pPage = new CMFCRibbonCustomizePropertyPage(pRibbon);
	AddPage(m_pPage);
}

CMFCRibbonCustomizeDialog::~CMFCRibbonCustomizeDialog()
{
	delete m_pPage;
}

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonCustomizePropertyPage property page

IMPLEMENT_DYNCREATE(CMFCRibbonCustomizePropertyPage, CMFCPropertyPage)

CMFCRibbonCustomizePropertyPage::CMFCRibbonCustomizePropertyPage( CMFCRibbonBar* pRibbonBar) :
	CMFCPropertyPage(CMFCRibbonCustomizePropertyPage::IDD), m_wndCommandsList(pRibbonBar), m_wndQATList(pRibbonBar, TRUE, TRUE)
{
	ASSERT_VALID(pRibbonBar);

	//{{AFX_DATA_INIT(CMFCRibbonCustomizePropertyPage)
	m_nCategory = -1;
	m_bQAToolbarOnBottom = FALSE;
	//}}AFX_DATA_INIT

	m_pRibbonBar = pRibbonBar;
	m_bQAToolbarOnBottom = !m_pRibbonBar->IsQuickAccessToolbarOnTop();
}

CMFCRibbonCustomizePropertyPage::~CMFCRibbonCustomizePropertyPage()
{
	while (!m_lstCustomCategories.IsEmpty())
	{
		delete m_lstCustomCategories.RemoveHead();
	}
}

void CMFCRibbonCustomizePropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CMFCPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMFCRibbonCustomizePropertyPage)
	DDX_Control(pDX, IDS_AFXBARRES_ADD, m_wndAdd);
	DDX_Control(pDX, IDC_AFXBARRES_REMOVE, m_wndRemove);
	DDX_Control(pDX, IDC_AFXBARRES_COMMANDS_LIST, m_wndCommandsList);
	DDX_Control(pDX, IDC_AFXBARRES_CATEGORY, m_wndCategoryCombo);
	DDX_Control(pDX, IDC_AFXBARRES_QAT_COMMANDS_LIST, m_wndQATList);
	DDX_Control(pDX, IDC_AFXBARRES_MOVEUP, m_wndUp);
	DDX_Control(pDX, IDC_AFXBARRES_MOVEDOWN, m_wndDown);
	DDX_CBIndex(pDX, IDC_AFXBARRES_CATEGORY, m_nCategory);
	DDX_Check(pDX, IDC_AFXBARRES_QAT_ON_BOTTOM, m_bQAToolbarOnBottom);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMFCRibbonCustomizePropertyPage, CMFCPropertyPage)
	//{{AFX_MSG_MAP(CMFCRibbonCustomizePropertyPage)
	ON_BN_CLICKED(IDS_AFXBARRES_ADD, &CMFCRibbonCustomizePropertyPage::OnAdd)
	ON_BN_CLICKED(IDC_AFXBARRES_REMOVE, &CMFCRibbonCustomizePropertyPage::OnRemove)
	ON_BN_CLICKED(IDC_AFXBARRES_MOVEUP, &CMFCRibbonCustomizePropertyPage::OnUp)
	ON_BN_CLICKED(IDC_AFXBARRES_MOVEDOWN, &CMFCRibbonCustomizePropertyPage::OnDown)
	ON_BN_CLICKED(IDC_AFXBARRES_RESET, &CMFCRibbonCustomizePropertyPage::OnToolbarReset)
	ON_BN_CLICKED(IDC_AFXBARRES_KEYBOARD, &CMFCRibbonCustomizePropertyPage::OnCustomizeKeyboard)
	ON_CBN_SELENDOK(IDC_AFXBARRES_CATEGORY, &CMFCRibbonCustomizePropertyPage::OnSelendokCategoryCombo)
	ON_LBN_SELCHANGE(IDC_AFXBARRES_QAT_COMMANDS_LIST, &CMFCRibbonCustomizePropertyPage::OnSelchangeQATCommands)
	ON_LBN_SELCHANGE(IDC_AFXBARRES_COMMANDS_LIST, &CMFCRibbonCustomizePropertyPage::OnSelchangeCommandsList)
	ON_LBN_DBLCLK(IDC_AFXBARRES_COMMANDS_LIST, &CMFCRibbonCustomizePropertyPage::OnAdd)
	ON_LBN_DBLCLK(IDC_AFXBARRES_QAT_COMMANDS_LIST, &CMFCRibbonCustomizePropertyPage::OnRemove)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonCustomizePropertyPage message handlers

void CMFCRibbonCustomizePropertyPage::OnSelendokCategoryCombo()
{
	ASSERT_VALID(m_pRibbonBar);

	UpdateData();

	DWORD_PTR dwData = m_wndCategoryCombo.GetItemData(m_nCategory);

	if (dwData == 0) // Separator, get next
	{
		if (m_nCategory == m_wndCategoryCombo.GetCount() - 1)
		{
			return;
		}

		m_nCategory++;
		UpdateData(FALSE);
	}

	CMFCRibbonCustomizeCategory* pCustCategory = DYNAMIC_DOWNCAST(CMFCRibbonCustomizeCategory, (CObject*)dwData);
	if (pCustCategory != NULL)
	{
		ASSERT_VALID(pCustCategory);

		m_wndCommandsList.FillFromIDs(pCustCategory->m_lstIDs, FALSE);
		OnSelchangeCommandsList();
	}
	else
	{
		CMFCRibbonCategory* pCategory = DYNAMIC_DOWNCAST(CMFCRibbonCategory, (CObject*)dwData);
		if (pCategory != NULL)
		{
			ASSERT_VALID(pCategory);

			m_wndCommandsList.FillFromCategory(pCategory);
			OnSelchangeCommandsList();
		}
	}
}

void CMFCRibbonCustomizePropertyPage::OnAdd()
{
	CMFCRibbonBaseElement* pCmd = m_wndCommandsList.GetSelected();
	if (pCmd == NULL)
	{
		return;
	}

	ASSERT_VALID(pCmd);

	if (!m_wndQATList.AddCommand(pCmd, TRUE, FALSE))
	{
		return;
	}

	int nSel = m_wndCommandsList.GetCurSel();
	if (nSel < m_wndCommandsList.GetCount() - 1)
	{
		m_wndCommandsList.SetCurSel(nSel + 1);
	}

	OnSelchangeQATCommands();
	OnSelchangeCommandsList();
}

void CMFCRibbonCustomizePropertyPage::OnRemove()
{
	int nIndex = m_wndQATList.GetCurSel();
	if (nIndex >= 0)
	{
		m_wndQATList.DeleteString(nIndex);

		nIndex = min(nIndex, m_wndQATList.GetCount() - 1);

		if (nIndex >= 0)
		{
			m_wndQATList.SetCurSel(nIndex);
		}
	}

	OnSelchangeQATCommands();
	OnSelchangeCommandsList();
}

void CMFCRibbonCustomizePropertyPage::OnUp()
{
	MoveItem(TRUE);
}

void CMFCRibbonCustomizePropertyPage::OnDown()
{
	MoveItem(FALSE);
}

void CMFCRibbonCustomizePropertyPage::OnToolbarReset()
{
	CString strPrompt;

	CString strCaption;
	strPrompt.Format(IDS_AFXBARRES_RESET_TOOLBAR_FMT, strCaption);

	strPrompt.Remove(_T('\''));
	strPrompt.Remove(_T('\''));

	if (AfxMessageBox(strPrompt, MB_OKCANCEL | MB_ICONWARNING) != IDOK)
	{
		return;
	}

	CList<UINT,UINT> lstCmds;
	m_pRibbonBar->m_QAToolbar.GetDefaultCommands(lstCmds);

	m_wndQATList.FillFromIDs(lstCmds, FALSE);
}

void CMFCRibbonCustomizePropertyPage::OnSelchangeQATCommands()
{
	m_wndUp.EnableWindow(m_wndQATList.GetCurSel() > 0);
	m_wndDown.EnableWindow(m_wndQATList.GetCurSel() < m_wndQATList.GetCount() - 1);
	m_wndRemove.EnableWindow(m_wndQATList.GetCurSel() >= 0);
}

BOOL CMFCRibbonCustomizePropertyPage::OnInitDialog()
{
	CMFCPropertyPage::OnInitDialog();

	ASSERT_VALID(m_pRibbonBar);

	const CString strSeparator = _T("----------");

	if (DYNAMIC_DOWNCAST(CMFCRibbonCustomizeDialog, GetParent()) != NULL)
	{
		CString strTitle;
		GetWindowText(strTitle);

		GetParent()->SetWindowText(strTitle);
	}

	m_wndUp.SetStdImage(CMenuImages::IdArrowUpLarge);
	m_wndDown.SetStdImage(CMenuImages::IdArrowDownLarge);

	//-----------------------
	// Add custom categories:
	//-----------------------
	for (POSITION pos = m_lstCustomCategories.GetHeadPosition(); pos != NULL;)
	{
		CMFCRibbonCustomizeCategory* pCustCategory = m_lstCustomCategories.GetNext(pos);
		ASSERT_VALID(pCustCategory);

		int nIndex = m_wndCategoryCombo.AddString(pCustCategory->m_strName);
		m_wndCategoryCombo.SetItemData(nIndex, (DWORD_PTR) pCustCategory);
	}

	if (m_wndCategoryCombo.GetCount() > 0)
	{
		m_wndCategoryCombo.AddString(strSeparator);
	}

	//-------------------
	// Add main category:
	//-------------------
	CMFCRibbonCategory* pMainCategory = m_pRibbonBar->GetMainCategory();
	if (pMainCategory != NULL)
	{
		ASSERT_VALID(pMainCategory);

		int nIndex = m_wndCategoryCombo.AddString(pMainCategory->GetName());
		m_wndCategoryCombo.SetItemData(nIndex, (DWORD_PTR) pMainCategory);
		m_wndCategoryCombo.AddString(strSeparator);
	}

	int i = 0;
	BOOL bHasContextCategories = FALSE;

	//----------------------------
	// Add non-context categories:
	//----------------------------
	for (i = 0; i < m_pRibbonBar->GetCategoryCount(); i++)
	{
		CMFCRibbonCategory* pCategory = m_pRibbonBar->GetCategory(i);
		ASSERT_VALID(pCategory);

		if (pCategory->GetContextID() == 0)
		{
			int nIndex = m_wndCategoryCombo.AddString(pCategory->GetName());
			m_wndCategoryCombo.SetItemData(nIndex, (DWORD_PTR) pCategory);
		}
		else
		{
			bHasContextCategories = TRUE;
		}
	}

	if (bHasContextCategories)
	{
		//------------------------
		// Add context categories:
		//------------------------
		m_wndCategoryCombo.AddString(strSeparator);

		for (i = 0; i < m_pRibbonBar->GetCategoryCount(); i++)
		{
			CMFCRibbonCategory* pCategory = m_pRibbonBar->GetCategory(i);
			ASSERT_VALID(pCategory);

			const UINT uiContextID = pCategory->GetContextID();

			if (uiContextID != 0)
			{
				CString strName;
				CString strContext;

				if (m_pRibbonBar->GetContextName(uiContextID, strContext))
				{
					strName = strContext + _T(" | ") + pCategory->GetName();
				}
				else
				{
					strName = pCategory->GetName();
				}

				int nIndex = m_wndCategoryCombo.AddString(strName);
				m_wndCategoryCombo.SetItemData(nIndex, (DWORD_PTR) pCategory);
			}
		}
	}

	if (m_wndCategoryCombo.GetCount() > 0)
	{
		m_nCategory = 0;
		UpdateData(FALSE);

		OnSelendokCategoryCombo();
	}

	CList<UINT,UINT> lstQACommands;
	m_pRibbonBar->GetQuickAccessCommands(lstQACommands);

	m_wndQATList.FillFromIDs(lstQACommands, FALSE);

	OnSelchangeQATCommands();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMFCRibbonCustomizePropertyPage::MoveItem(BOOL bMoveUp)
{
	int nSel = m_wndQATList.GetCurSel();

	CString str;
	m_wndQATList.GetText(nSel, str);

	DWORD_PTR dwData = m_wndQATList.GetItemData(nSel);

	m_wndQATList.DeleteString(nSel);

	int nNewIndex = bMoveUp ? nSel - 1 : nSel + 1;

	int nIndex = m_wndQATList.InsertString(nNewIndex, str);

	m_wndQATList.SetItemData(nIndex, dwData);
	m_wndQATList.SetCurSel(nIndex);
	OnSelchangeQATCommands();
}

void CMFCRibbonCustomizePropertyPage::OnOK()
{
	UpdateData();

	ASSERT_VALID(m_pRibbonBar);

	CList<UINT,UINT> lstQACommands;

	for (int i = 0; i < m_wndQATList.GetCount(); i++)
	{
		lstQACommands.AddTail(m_wndQATList.GetCommand(i)->GetID());
	}

	m_pRibbonBar->m_QAToolbar.ReplaceCommands(lstQACommands);
	m_pRibbonBar->SetQuickAccessToolbarOnTop(!m_bQAToolbarOnBottom);

	m_pRibbonBar->RecalcLayout();

	CFrameWnd* pParentFrame = m_pRibbonBar->GetParentFrame();

	if (pParentFrame->GetSafeHwnd() != NULL)
	{
		pParentFrame->RecalcLayout();
		pParentFrame->RedrawWindow();
	}

	CMFCPropertyPage::OnOK();
}

void CMFCRibbonCustomizePropertyPage::OnCustomizeKeyboard()
{
	ASSERT_VALID(m_pRibbonBar);

	CMFCRibbonKeyboardCustomizeDialog dlg(m_pRibbonBar, this);
	dlg.DoModal();
}

void CMFCRibbonCustomizePropertyPage::OnSelchangeCommandsList()
{
	BOOL bEnableAddButton = TRUE;

	CMFCRibbonBaseElement* pCmd = m_wndCommandsList.GetSelected();
	if (pCmd == NULL)
	{
		bEnableAddButton = FALSE;
	}
	else
	{
		ASSERT_VALID(pCmd);
		bEnableAddButton = pCmd->GetID() == 0 || m_wndQATList.GetCommandIndex(pCmd->GetID()) < 0;
	}

	m_wndAdd.EnableWindow(bEnableAddButton);
}

void CMFCRibbonCustomizePropertyPage::AddCustomCategory(LPCTSTR lpszName, const CList<UINT, UINT>& lstIDS)
{
	ENSURE(lpszName != NULL);
	ENSURE(GetSafeHwnd() == NULL);

	CMFCRibbonCustomizeCategory* pCategory = new CMFCRibbonCustomizeCategory;
	pCategory->m_strName = lpszName;

	pCategory->m_lstIDs.AddHead((UINT)0); // Separator
	pCategory->m_lstIDs.AddTail((CList<UINT,UINT>*)&lstIDS);

	m_lstCustomCategories.AddTail(pCategory);
}


