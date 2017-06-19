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
#include "afxtoolbarscustomizedialog.h"
#include "afxmousepropertypage.h"
#include "afxmousemanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCMousePropertyPage property page

IMPLEMENT_DYNCREATE(CMFCMousePropertyPage, CPropertyPage)

CMFCMousePropertyPage::CMFCMousePropertyPage() : CPropertyPage(CMFCMousePropertyPage::IDD)
{
	m_strCommandDescription = _T("");
	m_iCurrViewId = -1;
}

CMFCMousePropertyPage::~CMFCMousePropertyPage()
{
}

void CMFCMousePropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMFCMousePropertyPage)
	DDX_Control(pDX, IDC_AFXBARRES_LIST_VIEWS, m_wndListOfViews);
	DDX_Control(pDX, IDC_AFXBARRES_LIST_OF_COMMANDS, m_wndListOfCommands);
	DDX_Control(pDX, IDC_AFXBARRES_COMMAND_DESCRIPTION, m_wndCommandDescription);
	DDX_Text(pDX, IDC_AFXBARRES_COMMAND_DESCRIPTION, m_strCommandDescription);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMFCMousePropertyPage, CPropertyPage)
	//{{AFX_MSG_MAP(CMFCMousePropertyPage)
	ON_BN_CLICKED(IDC_AFXBARRES_NO_DBLCLIICK, &CMFCMousePropertyPage::OnNoDblcliick)
	ON_BN_CLICKED(IDC_AFXBARRES_USE_DBLCLIICK, &CMFCMousePropertyPage::OnUseDblcliick)
	ON_LBN_SELCHANGE(IDC_AFXBARRES_LIST_OF_COMMANDS, &CMFCMousePropertyPage::OnSelchangeListOfCommands)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_AFXBARRES_LIST_VIEWS, &CMFCMousePropertyPage::OnItemchangedListViews)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCMousePropertyPage message handlers

void CMFCMousePropertyPage::OnNoDblcliick()
{
	afxMouseManager->SetCommandForDblClk(m_iCurrViewId, 0);
	EnableDblClkControls(FALSE);
}

void CMFCMousePropertyPage::OnUseDblcliick()
{
	EnableDblClkControls();

	if (m_iCurrViewId < 0)
	{
		MessageBeep((UINT) -1);
		return;
	}

	afxMouseManager->SetCommandForDblClk(m_iCurrViewId, 0);
}

void CMFCMousePropertyPage::OnSelchangeListOfCommands()
{
	ENSURE(afxMouseManager != NULL);

	if (m_iCurrViewId < 0)
	{
		MessageBeep((UINT) -1);
		return;
	}

	int iIndex = m_wndListOfCommands.GetCurSel();
	UINT uiCmdId = (UINT) m_wndListOfCommands.GetItemData(iIndex);

	// Get command description:
	CFrameWnd* pParent = GetParentFrame();
	if (pParent != NULL && pParent->GetSafeHwnd() != NULL)
	{
		pParent->GetMessageString(uiCmdId, m_strCommandDescription);
	}
	else
	{
		m_strCommandDescription.Empty();
	}

	// Update mouse manager:
	afxMouseManager->SetCommandForDblClk(m_iCurrViewId, uiCmdId);

	UpdateData(FALSE);
}

void CMFCMousePropertyPage::OnItemchangedListViews(NMHDR* pNMHDR, LRESULT* pResult)
{
	ENSURE(afxMouseManager != NULL);

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	ENSURE(pNMListView != NULL);

	*pResult = 0;

	if (pNMListView->uChanged != LVIF_STATE)
	{
		return;
	}

	for (int i = 0; i < m_wndListOfViews.GetItemCount(); i ++)
	{
		UINT uState = m_wndListOfViews.GetItemState(i, LVIF_STATE | LVIS_SELECTED);
		if (uState & LVIS_SELECTED)
		{
			m_iCurrViewId = (int) m_wndListOfViews.GetItemData(i);
			ASSERT(m_iCurrViewId >= 0);

			UINT uiCmd = afxMouseManager->GetViewDblClickCommand(m_iCurrViewId);
			if (uiCmd == 0)
			{
				CheckDlgButton(IDC_AFXBARRES_USE_DBLCLIICK, 0);
				CheckDlgButton(IDC_AFXBARRES_NO_DBLCLIICK, 1);
				EnableDblClkControls(FALSE);
			}
			else
			{
				CheckDlgButton(IDC_AFXBARRES_USE_DBLCLIICK, 1);
				CheckDlgButton(IDC_AFXBARRES_NO_DBLCLIICK, 0);
				EnableDblClkControls();
				SelectCommand(uiCmd);
			}

			break;
		}
	}
}

BOOL CMFCMousePropertyPage::OnInitDialog()
{
	ENSURE(afxMouseManager != NULL);

	CPropertyPage::OnInitDialog();

	CStringList listOfViewNames;
	afxMouseManager->GetViewNames(listOfViewNames);

	// Create image list:
	if (!m_ViewsImages.Create( afxGlobalData.m_sizeSmallIcon.cx, afxGlobalData.m_sizeSmallIcon.cy, ILC_COLOR | ILC_MASK, (int) listOfViewNames.GetCount(), 1))
	{
		ASSERT(FALSE);
	}

	m_wndListOfViews.SetImageList(&m_ViewsImages, LVSIL_SMALL);

	POSITION pos;

	// Fill views list:
	CRect rect;
	m_wndListOfViews.GetClientRect(&rect);
	m_wndListOfViews.InsertColumn(0, _T(""), LVCFMT_LEFT, rect.Width() - 1);

	ASSERT(!listOfViewNames.IsEmpty());

	int iMaxWidth = 0;

	for (pos = listOfViewNames.GetHeadPosition(); pos != NULL;)
	{
		CString strViewName = listOfViewNames.GetNext(pos);

		int iImageIndex = -1;

		// Add view icon:
		UINT uiViewIconId = afxMouseManager->GetViewIconId(afxMouseManager->GetViewIdByName(strViewName));

		HICON hViewIcon;
		if (uiViewIconId != 0 && (hViewIcon = AfxGetApp()->LoadIcon(uiViewIconId)) != NULL)
		{
			iImageIndex = m_ViewsImages.Add(hViewIcon);
			::DestroyIcon(hViewIcon);
		}

		int iIndex = m_wndListOfViews.GetItemCount();
		for (int i = 0; i < m_wndListOfViews.GetItemCount(); i ++)
		{
			CString strText = m_wndListOfViews.GetItemText(i, 0);
			if (strText > strViewName)
			{
				iIndex = i;
				break;
			}
		}

		m_wndListOfViews.InsertItem(iIndex, strViewName, iImageIndex);
		m_wndListOfViews.SetItemData(iIndex, (DWORD) afxMouseManager->GetViewIdByName(strViewName));

		int iStrWidth = m_wndListOfViews.GetStringWidth(strViewName);
		iMaxWidth = max(iStrWidth, iMaxWidth);
	}

	// Add icon width pluse some pixels:
	IMAGEINFO info;
	m_ViewsImages.GetImageInfo(0, &info);
	CRect rectImage = info.rcImage;

	iMaxWidth += rectImage.Width() + 10;
	m_wndListOfViews.SetColumnWidth(0, iMaxWidth);

	// Fill commands list:
	CMFCToolBarsCustomizeDialog* pWndParent = DYNAMIC_DOWNCAST(CMFCToolBarsCustomizeDialog, GetParent());
	ENSURE(pWndParent != NULL);

	pWndParent->FillAllCommandsList(m_wndListOfCommands);

	// Select the first view:
	m_wndListOfViews.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	m_wndListOfViews.EnsureVisible(0, FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CMFCMousePropertyPage::EnableDblClkControls(BOOL bEnable)
{
	m_wndListOfCommands.EnableWindow(bEnable);
	m_wndCommandDescription.EnableWindow(bEnable);

	if (!bEnable)
	{
		m_wndListOfCommands.SetCurSel(-1);

		m_strCommandDescription.Empty();
		UpdateData(FALSE);
	}
}

BOOL CMFCMousePropertyPage::SelectCommand(UINT uiCmd)
{
	// Get selected item description:
	CFrameWnd* pParent = GetParentFrame();
	if (pParent != NULL && pParent->GetSafeHwnd() != NULL)
	{
		pParent->GetMessageString(uiCmd, m_strCommandDescription);
	}
	else
	{
		m_strCommandDescription.Empty();
	}

	UpdateData(FALSE);

	// Select command in the commands listbox:
	for (int iCmdIndex = 0; iCmdIndex < m_wndListOfCommands.GetCount(); iCmdIndex ++)
	{
		if (uiCmd == (UINT) m_wndListOfCommands.GetItemData(iCmdIndex))
		{
			m_wndListOfCommands.SetCurSel(iCmdIndex);
			m_wndListOfCommands.SetTopIndex(iCmdIndex);

			return TRUE;
		}
	}

	return FALSE;
}



