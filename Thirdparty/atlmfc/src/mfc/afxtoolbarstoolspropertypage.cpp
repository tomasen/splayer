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
#include "afxribbonres.h"
#include "afxtoolbarstoolspropertypage.h"
#include "afxusertoolsmanager.h"
#include "afxtoolbarscustomizedialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const int nBtnNew = 0;
static const int nBtnDelete = 1;
static const int nBtnMoveUp = 2;
static const int nBtnMoveDn = 3;

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarsToolsPropertyPage property page

#pragma warning(disable : 4355)

CMFCToolBarsToolsPropertyPage::CMFCToolBarsToolsPropertyPage() : CPropertyPage(CMFCToolBarsToolsPropertyPage::IDD), m_wndToolsList(this)
{
	//{{AFX_DATA_INIT(CMFCToolBarsToolsPropertyPage)
	m_strCommand = _T("");
	m_strArguments = _T("");
	m_strInitialDirectory = _T("");
	//}}AFX_DATA_INIT

	m_pSelTool = NULL;
	m_pParentSheet = NULL;
}

#pragma warning(default : 4355)

CMFCToolBarsToolsPropertyPage::~CMFCToolBarsToolsPropertyPage()
{
}

void CMFCToolBarsToolsPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMFCToolBarsToolsPropertyPage)
	DDX_Control(pDX, IDD_AFXBARRES_MENU_INITIAL_DIRECTORY, m_wndInitialDirBtn);
	DDX_Control(pDX, IDD_AFXBARRES_MENU_ARGUMENTS, m_wndArgumentsBtn);
	DDX_Control(pDX, IDD_AFXBARRES_ARGUMENTS, m_wndArgumentsEdit);
	DDX_Control(pDX, IDD_AFXBARRES_INITIAL_DIRECTORY, m_wndInitialDirEdit);
	DDX_Control(pDX, IDD_AFXBARRES_COMMAND, m_wndCommandEdit);
	DDX_Control(pDX, IDD_AFXBARRES_BROWSE_COMMAND, m_wndBrowseBtn);
	DDX_Control(pDX, IDD_AFXBARRES_COMMANDS_LIST, m_wndToolsList);
	DDX_Text(pDX, IDD_AFXBARRES_COMMAND, m_strCommand);
	DDX_Text(pDX, IDD_AFXBARRES_ARGUMENTS, m_strArguments);
	DDX_Text(pDX, IDD_AFXBARRES_INITIAL_DIRECTORY, m_strInitialDirectory);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMFCToolBarsToolsPropertyPage, CPropertyPage)
	//{{AFX_MSG_MAP(CMFCToolBarsToolsPropertyPage)
	ON_BN_CLICKED(IDD_AFXBARRES_BROWSE_COMMAND, &CMFCToolBarsToolsPropertyPage::OnBrowseCommand)
	ON_BN_CLICKED(IDD_AFXBARRES_MENU_ARGUMENTS, &CMFCToolBarsToolsPropertyPage::OnArgumentsOptions)
	ON_BN_CLICKED(IDD_AFXBARRES_MENU_INITIAL_DIRECTORY, &CMFCToolBarsToolsPropertyPage::OnInitialDirectoryOptions)
	ON_EN_UPDATE(IDD_AFXBARRES_ARGUMENTS, &CMFCToolBarsToolsPropertyPage::OnUpdateTool)
	ON_EN_UPDATE(IDD_AFXBARRES_COMMAND, &CMFCToolBarsToolsPropertyPage::OnUpdateTool)
	ON_EN_UPDATE(IDD_AFXBARRES_INITIAL_DIRECTORY, &CMFCToolBarsToolsPropertyPage::OnUpdateTool)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarsToolsPropertyPage message handlers

BOOL CMFCToolBarsToolsPropertyPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	ENSURE(afxUserToolsManager != NULL);

	m_pParentSheet = DYNAMIC_DOWNCAST(CMFCToolBarsCustomizeDialog, GetParent());
	ENSURE(m_pParentSheet != NULL);

	//-------------
	// Add buttons:
	//-------------
	m_wndToolsList.SetStandardButtons();
	m_pParentSheet->OnInitToolsPage();

	//------------
	// Fill tools:
	//------------
	const CObList& lstTools = afxUserToolsManager->GetUserTools();
	for (POSITION pos = lstTools.GetHeadPosition(); pos != NULL;)
	{
		CUserTool* pTool = (CUserTool*) lstTools.GetNext(pos);
		ASSERT_VALID(pTool);

		m_wndToolsList.AddItem(pTool->m_strLabel, (DWORD_PTR) pTool);
	}

	UINT uMenuID = 0;
	uMenuID = afxUserToolsManager->GetInitialDirMenuID();
	if (uMenuID)
	{
		m_wndInitialDirBtn.ShowWindow(SW_SHOW);
		m_menuInitialDir.LoadMenu(uMenuID);
		m_wndInitialDirBtn.m_hMenu = m_menuInitialDir.GetSubMenu(0)->GetSafeHmenu();
	}

	uMenuID = afxUserToolsManager->GetArgumentsMenuID();
	if (uMenuID)
	{
		m_wndArgumentsBtn.ShowWindow(SW_SHOW);
		m_menuArguments.LoadMenu(uMenuID);
		m_wndArgumentsBtn.m_hMenu = m_menuArguments.GetSubMenu(0)->GetSafeHmenu();
	}

	m_wndInitialDirBtn.m_bRightArrow = TRUE;
	m_wndArgumentsBtn.m_bRightArrow  = TRUE;

	EnableControls();

	return TRUE;  // return TRUE unless you set the focus to a control
}

BOOL CVSToolsListBox::OnBeforeRemoveItem(int iItem)
{
	CUserTool* pTool = (CUserTool*) GetItemData(iItem);
	ASSERT_VALID(pTool);

	afxUserToolsManager->RemoveTool(pTool);
	m_pParent->m_pSelTool = NULL;

	return TRUE;
}

void CVSToolsListBox::OnAfterAddItem(int iItem)
{
	CUserTool* pTool = m_pParent->CreateNewTool();
	if (pTool == NULL)
	{
		RemoveItem(iItem);
		return;
	}

	ASSERT_VALID(pTool);

	pTool->m_strLabel = GetItemText(iItem);
	SetItemData(iItem, (DWORD_PTR) pTool);

	OnSelectionChanged();
}

void CVSToolsListBox::OnAfterRenameItem(int iItem)
{
	CUserTool* pTool = (CUserTool*) GetItemData(iItem);
	ASSERT_VALID(pTool);

	pTool->m_strLabel = GetItemText(iItem);
}

void CVSToolsListBox::OnAfterMoveItemUp(int iItem)
{
	CUserTool* pTool = (CUserTool*) GetItemData(iItem);
	ASSERT_VALID(pTool);

	afxUserToolsManager->MoveToolUp(pTool);
}

void CVSToolsListBox::OnAfterMoveItemDown(int iItem)
{
	CUserTool* pTool = (CUserTool*) GetItemData(iItem);
	ASSERT_VALID(pTool);

	afxUserToolsManager->MoveToolDown(pTool);
}

void CVSToolsListBox::OnSelectionChanged()
{
	int iSelItem = GetSelItem();
	CUserTool* pSelTool = (iSelItem < 0) ? NULL :(CUserTool*) GetItemData(iSelItem);

	if (pSelTool == NULL)
	{
		m_pParent->m_strCommand.Empty();
		m_pParent->m_strArguments.Empty();
		m_pParent->m_strInitialDirectory.Empty();
	}
	else
	{
		ASSERT_VALID(pSelTool);

		m_pParent->m_strCommand = pSelTool->GetCommand();
		m_pParent->m_strArguments = pSelTool->m_strArguments;
		m_pParent->m_strInitialDirectory = pSelTool->m_strInitialDirectory;
	}

	ASSERT_VALID(m_pParent->m_pParentSheet);
	m_pParent->m_pParentSheet->OnBeforeChangeTool(m_pParent->m_pSelTool);

	m_pParent->m_pSelTool = pSelTool;
	m_pParent->UpdateData(FALSE);

	m_pParent->EnableControls();

	m_pParent->m_pParentSheet->OnAfterChangeTool(m_pParent->m_pSelTool);
}

void CMFCToolBarsToolsPropertyPage::OnBrowseCommand()
{
	CFileDialog dlg(TRUE, afxUserToolsManager->GetDefExt(), NULL, 0, afxUserToolsManager->GetFilter(), this);
	if (dlg.DoModal() == IDOK)
	{
		m_strCommand = dlg.GetPathName();
		UpdateData(FALSE);
		OnUpdateTool();
	}
}

void CMFCToolBarsToolsPropertyPage::OnUpdateTool()
{
	UpdateData();

	int iSelItem = m_wndToolsList.GetSelItem();
	CUserTool* pSelTool = (iSelItem >= 0) ? (CUserTool*) m_wndToolsList.GetItemData(iSelItem) : NULL;

	if (pSelTool == NULL)
	{
		m_strCommand.Empty();
		m_strArguments.Empty();
		m_strInitialDirectory.Empty();

		UpdateData(FALSE);
	}
	else
	{
		ASSERT_VALID(pSelTool);

		pSelTool->SetCommand(m_strCommand);
		pSelTool->m_strArguments = m_strArguments;
		pSelTool->m_strInitialDirectory = m_strInitialDirectory;
	}

	EnableControls();
}

CUserTool* CMFCToolBarsToolsPropertyPage::CreateNewTool()
{
	ASSERT_VALID(m_pParentSheet);

	const int nMaxTools = afxUserToolsManager->GetMaxTools();

	if (afxUserToolsManager->GetUserTools().GetCount() == nMaxTools)
	{
		CString strError;
		strError.Format(IDS_AFXBARRES_TOO_MANY_TOOLS_FMT, nMaxTools);

		AfxMessageBox(strError);
		return NULL;
	}

	CUserTool* pTool = afxUserToolsManager->CreateNewTool();
	ASSERT_VALID(pTool);

	return pTool;
}

void CMFCToolBarsToolsPropertyPage::OnOK()
{
	OnUpdateTool();
	CPropertyPage::OnOK();
}

BOOL CMFCToolBarsToolsPropertyPage::OnKillActive()
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pParentSheet);

	if (!m_pParentSheet->CheckToolsValidity(afxUserToolsManager->GetUserTools()))
	{
		return FALSE;
	}

	return CPropertyPage::OnKillActive();
}

void CMFCToolBarsToolsPropertyPage::EnableControls()
{
	BOOL bEnableItemProps = (m_wndToolsList.GetSelItem() >= 0);

	m_wndCommandEdit.EnableWindow(bEnableItemProps);
	m_wndArgumentsEdit.EnableWindow(bEnableItemProps);
	m_wndInitialDirEdit.EnableWindow(bEnableItemProps);
	m_wndBrowseBtn.EnableWindow(bEnableItemProps);

	m_wndInitialDirBtn.EnableWindow(bEnableItemProps);
	m_wndArgumentsBtn.EnableWindow(bEnableItemProps);
}

void CMFCToolBarsToolsPropertyPage::OnArgumentsOptions()
{
	if (m_wndArgumentsBtn.m_nMenuResult != 0)
	{
		CString strItem;
		ENSURE(strItem.LoadString(m_wndArgumentsBtn.m_nMenuResult));

		//-----------------------------------------------
		// Insert text to the current arguments position:
		//-----------------------------------------------
		for (int i = 0; i < strItem.GetLength(); i++)
		{
			m_wndArgumentsEdit.SendMessage(WM_CHAR, (TCHAR) strItem [i]);
		}
	}
}

void CMFCToolBarsToolsPropertyPage::OnInitialDirectoryOptions()
{
	if (m_wndInitialDirBtn.m_nMenuResult != 0)
	{
		CString strItem;
		if (strItem.LoadString(m_wndInitialDirBtn.m_nMenuResult))
		{
			//-----------------------------------------------
			// Insert text to the current directory position:
			//-----------------------------------------------
			for (int i = 0; i < strItem.GetLength(); i++)
			{
				m_wndInitialDirEdit.SendMessage(WM_CHAR, (TCHAR) strItem [i]);
			}
		}
	}
}


