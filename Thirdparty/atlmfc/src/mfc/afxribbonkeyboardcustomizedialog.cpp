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
#include "afxribbonkeyboardcustomizedialog.h"
#include "afxacceleratorkey.h"
#include "afxtoolbar.h"
#include "afxkeyboardmanager.h"
#include "afxmultidoctemplateex.h"
#include "afxbaseribbonelement.h"
#include "afxribbonbar.h"
#include "afxribboncategory.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonKeyboardCustomizeDialog dialog

CMFCRibbonKeyboardCustomizeDialog::CMFCRibbonKeyboardCustomizeDialog(CMFCRibbonBar* pRibbonBar, CWnd* pParent /*=NULL*/)
	: CDialogEx(CMFCRibbonKeyboardCustomizeDialog::IDD, pParent), m_wndCommandsList(pRibbonBar, FALSE /* Don't include separator */)
{
	ASSERT_VALID(pRibbonBar);

	m_strDescription = _T("");
	m_strAssignedTo = _T("");
	m_pRibbonBar = pRibbonBar;
	m_pParentFrame = m_pRibbonBar->GetTopLevelFrame();

	m_hAccelTable = NULL;
	m_lpAccel = NULL;
	m_nAccelSize = 0;
	m_pSelTemplate = NULL;
	m_pSelButton = NULL;
	m_pSelEntry = NULL;
}

CMFCRibbonKeyboardCustomizeDialog::~CMFCRibbonKeyboardCustomizeDialog()
{
	if (m_lpAccel != NULL)
	{
		delete [] m_lpAccel;
	}
}

void CMFCRibbonKeyboardCustomizeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMFCRibbonKeyboardCustomizeDialog)
	DDX_Control(pDX, IDC_AFXBARRES_ASSIGNED_TO_TITLE, m_wndAssignedToTitle);
	DDX_Control(pDX, IDC_AFXBARRES_NEW_SHORTCUT_KEY, m_wndNewKey);
	DDX_Control(pDX, IDC_AFXBARRES_VIEW_TYPE, m_wndViewTypeList);
	DDX_Control(pDX, IDC_AFXBARRES_VIEW_ICON, m_wndViewIcon);
	DDX_Control(pDX, IDC_AFXBARRES_REMOVE, m_wndRemoveButton);
	DDX_Control(pDX, IDC_AFXBARRES_CURRENT_KEYS_LIST, m_wndCurrentKeysList);
	DDX_Control(pDX, IDC_AFXBARRES_COMMANDS_LIST, m_wndCommandsList);
	DDX_Control(pDX, IDC_AFXBARRES_CATEGORY, m_wndCategoryList);
	DDX_Control(pDX, IDC_AFXBARRES_ASSIGN, m_wndAssignButton);
	DDX_Text(pDX, IDC_AFXBARRES_COMMAND_DESCRIPTION, m_strDescription);
	DDX_Text(pDX, IDC_AFXBARRES_ASSIGNED_TO, m_strAssignedTo);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMFCRibbonKeyboardCustomizeDialog, CDialogEx)
	//{{AFX_MSG_MAP(CMFCRibbonKeyboardCustomizeDialog)
	ON_BN_CLICKED(IDC_AFXBARRES_ASSIGN, &CMFCRibbonKeyboardCustomizeDialog::OnAssign)
	ON_BN_CLICKED(IDC_AFXBARRES_REMOVE, &CMFCRibbonKeyboardCustomizeDialog::OnRemove)
	ON_BN_CLICKED(IDC_AFXBARRES_RESET_SHORTCUTS, &CMFCRibbonKeyboardCustomizeDialog::OnResetAll)
	ON_LBN_SELCHANGE(IDC_AFXBARRES_CATEGORY, &CMFCRibbonKeyboardCustomizeDialog::OnSelchangeCategory)
	ON_LBN_SELCHANGE(IDC_AFXBARRES_COMMANDS_LIST, &CMFCRibbonKeyboardCustomizeDialog::OnSelchangeCommandsList)
	ON_LBN_SELCHANGE(IDC_AFXBARRES_CURRENT_KEYS_LIST, &CMFCRibbonKeyboardCustomizeDialog::OnSelchangeCurrentKeysList)
	ON_CBN_SELCHANGE(IDC_AFXBARRES_VIEW_TYPE, &CMFCRibbonKeyboardCustomizeDialog::OnSelchangeViewType)
	ON_EN_UPDATE(IDC_AFXBARRES_NEW_SHORTCUT_KEY, &CMFCRibbonKeyboardCustomizeDialog::OnUpdateNewShortcutKey)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonKeyboardCustomizeDialog message handlers

BOOL CMFCRibbonKeyboardCustomizeDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	ENSURE(afxKeyboardManager != NULL);
	ASSERT_VALID(m_pRibbonBar);

	// Initialize commands by category:
	CMFCRibbonCategory* pMainCategory = m_pRibbonBar->GetMainCategory();
	if (pMainCategory != NULL)
	{
		ASSERT_VALID(pMainCategory);
		m_wndCategoryList.AddString(pMainCategory->GetName());
	}

	for (int i = 0; i < m_pRibbonBar->GetCategoryCount(); i++)
	{
		m_wndCategoryList.AddString(
			m_pRibbonBar->GetCategory(i)->GetName());
	}

	if (m_wndCategoryList.GetCount() > 0)
	{
		m_wndCategoryList.SetCurSel(0);
		OnSelchangeCategory();
	}

	// Find all application document templates and fill menues combobox
	// by document template data:
	CDocManager* pDocManager = AfxGetApp()->m_pDocManager;
	if (m_bAutoSet && pDocManager != NULL)
	{
		// Walk all templates in the application:
		for (POSITION pos = pDocManager->GetFirstDocTemplatePosition(); pos != NULL;)
		{
			CMultiDocTemplateEx* pTemplate = (CMultiDocTemplateEx*) pDocManager->GetNextDocTemplate(pos);
			ASSERT_VALID(pTemplate);
			ASSERT_KINDOF(CDocTemplate, pTemplate);

			// We are interessing CMultiDocTemplateEx objects with
			// the shared menu only....
			if (!pTemplate->IsKindOf(RUNTIME_CLASS(CMultiDocTemplate)) || pTemplate->m_hAccelTable == NULL)
			{
				continue;
			}

			// Maybe, the template with same ID is already exist?
			BOOL bIsAlreadyExist = FALSE;
			for (int i = 0; !bIsAlreadyExist && i < m_wndViewTypeList.GetCount(); i++)
			{
				CMultiDocTemplateEx* pListTemplate = (CMultiDocTemplateEx*) m_wndViewTypeList.GetItemData(i);
				bIsAlreadyExist = pListTemplate != NULL && pListTemplate->GetResId() == pTemplate->GetResId();
			}

			if (!bIsAlreadyExist)
			{
				CString strName;
				pTemplate->GetDocString(strName, CDocTemplate::fileNewName);

				int iIndex = m_wndViewTypeList.AddString(strName);
				m_wndViewTypeList.SetItemData(iIndex, (DWORD_PTR) pTemplate);
			}
		}
	}

	// Add a default application:
	CFrameWnd* pWndMain = DYNAMIC_DOWNCAST(CFrameWnd, m_pParentFrame);
	if (pWndMain != NULL && pWndMain->m_hAccelTable != NULL)
	{
		CString strName;
		ENSURE(strName.LoadString(IDS_AFXBARRES_DEFAULT_VIEW));

		int iIndex = m_wndViewTypeList.AddString(strName);
		m_wndViewTypeList.SetItemData(iIndex, (DWORD_PTR) NULL);

		m_wndViewTypeList.SetCurSel(iIndex);
		OnSelchangeViewType();
	}

	if (m_wndViewTypeList.GetCurSel() == CB_ERR)
	{
		m_wndViewTypeList.SetCurSel(0);
		OnSelchangeViewType();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CMFCRibbonKeyboardCustomizeDialog::OnAssign()
{
	ENSURE(m_lpAccel != NULL);
	ENSURE(m_pSelButton != NULL);

	// Obtain a new acceleration entry from the keyboard control:
	ENSURE(m_wndNewKey.IsKeyDefined());

	ACCEL* pAccel = (ACCEL*) m_wndNewKey.GetAccel();
	ENSURE(pAccel != NULL);

	pAccel->cmd = (USHORT) m_pSelButton->GetID();

	// Create a new entries array:
	LPACCEL lpAccelOld = m_lpAccel;

	m_lpAccel = new ACCEL [m_nAccelSize + 1];
	ENSURE(m_lpAccel != NULL);

	memcpy(m_lpAccel, lpAccelOld, sizeof(ACCEL) * m_nAccelSize);

	int listcount = m_wndCurrentKeysList.GetCount();
	for (int i = 0; i < m_nAccelSize; i ++)
	{
		for (int idx=0; idx<listcount; idx++)
		{
			if ( m_wndCurrentKeysList.GetItemData(idx) == (DWORD_PTR) &lpAccelOld [i] )
			{
				m_wndCurrentKeysList.SetItemData(idx, (DWORD_PTR) &m_lpAccel [i]);
				break;
			}
		}
	}

	m_lpAccel [m_nAccelSize ++] = *pAccel;

	delete [] lpAccelOld;

	afxKeyboardManager->UpdateAccelTable(m_pSelTemplate, m_lpAccel, m_nAccelSize);

	AddKeyEntry(&m_lpAccel [m_nAccelSize - 1]);

	m_wndNewKey.ResetKey();
	OnUpdateNewShortcutKey();

	m_wndCommandsList.SetFocus();
}

void CMFCRibbonKeyboardCustomizeDialog::OnSelchangeCategory()
{
	UpdateData();

	int nIndex = m_wndCategoryList.GetCurSel();

	CMFCRibbonCategory* pCategory = NULL;

	if (m_pRibbonBar->GetMainCategory() != NULL)
	{
		nIndex--;

		if (nIndex < 0)
		{
			pCategory = m_pRibbonBar->GetMainCategory();
		}
	}

	if (pCategory == NULL)
	{
		pCategory = m_pRibbonBar->GetCategory(nIndex);
	}

	m_wndCommandsList.FillFromCategory(pCategory);

	m_wndNewKey.EnableWindow(FALSE);

	if (m_wndCommandsList.GetCount() > 0)
	{
		m_wndCommandsList.SetCurSel(0);
		OnSelchangeCommandsList();
	}
}

void CMFCRibbonKeyboardCustomizeDialog::OnSelchangeCommandsList()
{
	m_strDescription.Empty();
	m_wndCurrentKeysList.ResetContent();
	OnSelchangeCurrentKeysList();

	int iIndex = m_wndCommandsList.GetCurSel();
	if (iIndex == LB_ERR)
	{
		m_pSelButton = NULL;
		m_wndNewKey.EnableWindow(FALSE);

		UpdateData(FALSE);
		return;
	}

	// Set command description:
	m_pSelButton = (CMFCRibbonBaseElement*) m_wndCommandsList.GetItemData(iIndex);
	ASSERT_VALID(m_pSelButton);

	CFrameWnd* pParent = GetParentFrame();
	if (pParent != NULL && pParent->GetSafeHwnd() != NULL)
	{
		pParent->GetMessageString(m_pSelButton->GetID(), m_strDescription);
	}

	// Fill keys associated with selected command:
	if (m_lpAccel != NULL)
	{
		for (int i = 0; i < m_nAccelSize; i ++)
		{
			if (m_pSelButton->GetID() == m_lpAccel [i].cmd)
			{
				AddKeyEntry(&m_lpAccel [i]);
			}
		}
	}

	m_wndNewKey.EnableWindow();
	UpdateData(FALSE);
}

void CMFCRibbonKeyboardCustomizeDialog::OnSelchangeCurrentKeysList()
{
	int iIndex = m_wndCurrentKeysList.GetCurSel();
	if (iIndex == LB_ERR)
	{
		m_pSelEntry = NULL;
		m_wndRemoveButton.EnableWindow(FALSE);

		return;
	}

	m_pSelEntry = (LPACCEL) m_wndCurrentKeysList.GetItemData(iIndex);
	ENSURE(m_pSelEntry != NULL);

	m_wndRemoveButton.EnableWindow();
}

void CMFCRibbonKeyboardCustomizeDialog::OnRemove()
{
	ENSURE(m_pSelEntry != NULL);
	ENSURE(m_lpAccel != NULL);

	// Create a new entries array:
	LPACCEL lpAccelOld = m_lpAccel;

	m_lpAccel = new ACCEL [m_nAccelSize - 1];
	ENSURE(m_lpAccel != NULL);

	int iNewIndex = 0;
	for (int i = 0; i < m_nAccelSize; i ++)
	{
		if (m_pSelEntry != &lpAccelOld [i])
		{
			m_lpAccel [iNewIndex ++] = lpAccelOld [i];

			int listcount = m_wndCurrentKeysList.GetCount();
			for (int idx=0; idx<listcount; idx++)
			{
				if ( m_wndCurrentKeysList.GetItemData(idx) == (DWORD_PTR) &lpAccelOld [i] )
				{
					m_wndCurrentKeysList.SetItemData(idx, (DWORD_PTR) &m_lpAccel [iNewIndex-1]);
					break;
				}
			}
		}
	}

	delete [] lpAccelOld;
	m_nAccelSize --;

	afxKeyboardManager->UpdateAccelTable(m_pSelTemplate, m_lpAccel, m_nAccelSize);

	OnSelchangeCommandsList();
	m_wndCommandsList.SetFocus();
}

void CMFCRibbonKeyboardCustomizeDialog::OnResetAll()
{
	CString str;
	ENSURE(str.LoadString(IDS_AFXBARRES_RESET_KEYBOARD));

	if (AfxMessageBox(str, MB_YESNO | MB_ICONQUESTION) != IDYES)
	{
		return;
	}

	afxKeyboardManager->ResetAll();

	// Send notification to application main frame:
	if (m_pParentFrame != NULL)
	{
		m_pParentFrame->SendMessage(AFX_WM_RESETKEYBOARD);
	}

	OnSelchangeViewType();
	OnSelchangeCommandsList();
}

void CMFCRibbonKeyboardCustomizeDialog::OnSelchangeViewType()
{
	m_hAccelTable = NULL;
	m_pSelTemplate = NULL;

	if (m_lpAccel != NULL)
	{
		delete [] m_lpAccel;
		m_lpAccel = NULL;
	}

	int iIndex = m_wndViewTypeList.GetCurSel();
	if (iIndex == CB_ERR)
	{
		m_wndViewIcon.SetIcon(NULL);
		return;
	}

	HICON hicon = NULL;

	CMultiDocTemplateEx* pTemplate = (CMultiDocTemplateEx*) m_wndViewTypeList.GetItemData(iIndex);
	if (pTemplate != NULL)
	{
		ASSERT_VALID(pTemplate);

		hicon = AfxGetApp()->LoadIcon(pTemplate->GetResId());
		m_hAccelTable = pTemplate->m_hAccelTable;
	}
	else
	{
		CFrameWnd* pWndMain = DYNAMIC_DOWNCAST(CFrameWnd, m_pParentFrame);
		if (pWndMain != NULL)
		{
			hicon = (HICON)(LONG_PTR) GetClassLongPtr(*pWndMain, GCLP_HICON);
			m_hAccelTable = pWndMain->m_hAccelTable;
		}
	}

	if (hicon == NULL)
	{
		hicon = ::LoadIcon(NULL, IDI_APPLICATION);
	}

	m_wndViewIcon.SetIcon(hicon);

	ENSURE(m_hAccelTable != NULL);

	m_nAccelSize = ::CopyAcceleratorTable(m_hAccelTable, NULL, 0);

	m_lpAccel = new ACCEL [m_nAccelSize];
	ENSURE(m_lpAccel != NULL);

	::CopyAcceleratorTable(m_hAccelTable, m_lpAccel, m_nAccelSize);
	m_pSelTemplate = pTemplate;

	OnSelchangeCommandsList();
}

void CMFCRibbonKeyboardCustomizeDialog::AddKeyEntry(LPACCEL pEntry)
{
	ENSURE(pEntry != NULL);

	CMFCAcceleratorKey helper(pEntry);

	CString str;
	helper.Format(str);

	int iIndex = m_wndCurrentKeysList.AddString(str);
	m_wndCurrentKeysList.SetItemData(iIndex, (DWORD_PTR) pEntry);
}

void CMFCRibbonKeyboardCustomizeDialog::OnUpdateNewShortcutKey()
{
	ASSERT_VALID(m_pRibbonBar);

	ACCEL* pAccel = (ACCEL*) m_wndNewKey.GetAccel();
	ENSURE(pAccel != NULL);

	m_strAssignedTo.Empty();
	m_wndAssignedToTitle.ShowWindow(SW_HIDE);
	m_wndAssignButton.EnableWindow(FALSE);

	if (m_wndNewKey.IsKeyDefined())
	{
		ENSURE(m_lpAccel != NULL);

		BOOL bIsAlreadyDefined = FALSE;

		for (int i = 0; !bIsAlreadyDefined && i < m_nAccelSize; i ++)
		{
			const BYTE fRelFlags = FCONTROL | FALT | FSHIFT | FVIRTKEY;

			if (pAccel->key == m_lpAccel [i].key && (pAccel->fVirt & fRelFlags) == (m_lpAccel [i].fVirt & fRelFlags))
			{
				CMFCRibbonBaseElement* pCmd = m_pRibbonBar->FindByID(m_lpAccel [i].cmd, FALSE);
				if (pCmd != NULL)
				{
					m_strAssignedTo = pCmd->GetText();

					if (m_strAssignedTo.IsEmpty())
					{
						pCmd->UpdateTooltipInfo();
						m_strAssignedTo = pCmd->GetToolTipText();
					}
				}
				else
				{
					m_strAssignedTo = _T("????");

					CString strText;
					if (strText.LoadString(m_lpAccel [i].cmd) && !strText.IsEmpty())
					{
						AfxExtractSubString(m_strAssignedTo, strText, 1, '\n');
					}
				}

				bIsAlreadyDefined = TRUE;
			}
		}

		if (!bIsAlreadyDefined)
		{
			ENSURE(m_strAssignedTo.LoadString(IDP_AFXBARRES_UNASSIGNED));
			m_wndAssignButton.EnableWindow();
		}

		m_wndAssignedToTitle.ShowWindow(SW_SHOW);
	}

	UpdateData(FALSE);
}


