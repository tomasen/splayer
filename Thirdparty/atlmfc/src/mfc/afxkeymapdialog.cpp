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
#include "afxkeymapdialog.h"
#include "afxtoolbarscustomizedialog.h"
#include "afxmultidoctemplateex.h"
#include "afxtoolbarbutton.h"
#include "afxwinappex.h"
#include "afxsettingsstore.h"
#include "afxglobals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef _UNICODE
#define AFX_TCF_TEXT CF_UNICODETEXT
#else
#define AFX_TCF_TEXT CF_TEXT
#endif

static const int nColumnCommand = 0;
static const int nColumnKeys = 1;
static const int nColumnDescr = 2;

static const CString strWindowPlacementRegSection = _T("KeyMapWindowPlacement");
static const CString strRectKey = _T("KeyMapWindowRect");

static int CALLBACK listCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	// lParamSort contains a pointer to the dialog.
	// The lParam of an item is just its index.

	CMFCKeyMapDialog* pDlg = (CMFCKeyMapDialog*) lParamSort;
	ASSERT_VALID(pDlg);

	LVFINDINFO info;
	info.flags = LVFI_PARAM;

	info.lParam = lParam1;
	int iIndex1 = pDlg->m_KeymapList.FindItem(&info);
	ASSERT(iIndex1 >= 0);

	info.lParam = lParam2;
	int iIndex2 = pDlg->m_KeymapList.FindItem(&info);
	ASSERT(iIndex2 >= 0);

	CString strItem1 = pDlg->m_KeymapList.GetItemText(iIndex1, pDlg->m_nSortedCol);
	CString strItem2 = pDlg->m_KeymapList.GetItemText(iIndex2, pDlg->m_nSortedCol);

	return pDlg->m_bSortAscending ? strItem1.Compare(strItem2) : strItem2.Compare(strItem1);
}

/////////////////////////////////////////////////////////////////////////////
// CMFCKeyMapDialog dialog

CMFCKeyMapDialog::CMFCKeyMapDialog(CFrameWnd* pWndParentFrame, BOOL bEnablePrint /* = FALSE */) :
	CDialogEx(CMFCKeyMapDialog::IDD, pWndParentFrame), m_bEnablePrint(bEnablePrint)
{
	m_pWndParentFrame = pWndParentFrame;

	m_hAccelTable = NULL;
	m_lpAccel = NULL;
	m_nAccelSize = 0;

	m_nSortedCol = 0;
	m_bSortAscending = TRUE;
}

CMFCKeyMapDialog::~CMFCKeyMapDialog()
{
	if (m_pDlgCust != NULL)
	{
		delete m_pDlgCust;
	}

	if (m_lpAccel != NULL)
	{
		delete [] m_lpAccel;
	}
}

void CMFCKeyMapDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMFCKeyMapDialog)
	DDX_Control(pDX, IDC_AFXBARRES_ACCEL_LABEL, m_wndAccelLabel);
	DDX_Control(pDX, IDC_AFXBARRES_KEYLIST, m_KeymapList);
	DDX_Control(pDX, IDC_AFXBARRES_CATEGORY, m_wndCategoryList);
	DDX_Control(pDX, IDC_AFXBARRES_VIEW_ICON, m_wndViewIcon);
	DDX_Control(pDX, IDC_AFXBARRES_VIEW_TYPE, m_wndViewTypeList);
	DDX_Control(pDX, IDC_AFXBARRES_PRINT_KEYMAP, m_ButtonPrint);
	DDX_Control(pDX, IDC_AFXBARRES_COPY_KEYMAP, m_ButtonCopy);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMFCKeyMapDialog, CDialogEx)
	//{{AFX_MSG_MAP(CMFCKeyMapDialog)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_AFXBARRES_VIEW_TYPE, &CMFCKeyMapDialog::OnSelchangeViewType)
	ON_CBN_SELCHANGE(IDC_AFXBARRES_CATEGORY, &CMFCKeyMapDialog::OnSelchangeCategory)
	ON_BN_CLICKED(IDC_AFXBARRES_COPY_KEYMAP, &CMFCKeyMapDialog::OnCopy)
	ON_BN_CLICKED(IDC_AFXBARRES_PRINT_KEYMAP, &CMFCKeyMapDialog::OnPrint)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCKeyMapDialog message handlers

BOOL CMFCKeyMapDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	if (AfxGetMainWnd() != NULL && (AfxGetMainWnd()->GetExStyle() & WS_EX_LAYOUTRTL))
	{
		ModifyStyleEx(0, WS_EX_LAYOUTRTL);
	}

	{
		// Set dialog icon:
		LPCTSTR lpszResourceName = MAKEINTRESOURCE(IDI_AFXBARRES_HELP);
		ENSURE(lpszResourceName != NULL);

		SetIcon((HICON) ::LoadImage (
			AfxFindResourceHandle(lpszResourceName, RT_ICON), 
			lpszResourceName,
			IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_SHARED), FALSE);

		// Setup buttons:
		m_ButtonPrint.m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;
		m_ButtonCopy.m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;

		CString strTT;

		if (m_bEnablePrint)
		{
			m_ButtonPrint.SetImage(afxGlobalData.Is32BitIcons() ? IDB_AFXBARRES_PRINT32 : IDB_AFXBARRES_PRINT, NULL);
			m_ButtonPrint.GetWindowText(strTT);
			m_ButtonPrint.SetWindowText(_T(""));
			m_ButtonPrint.SetTooltip(strTT);
			m_ButtonPrint.SizeToContent();
			m_ButtonPrint.m_bDrawFocus = FALSE;
		}
		else
		{
			m_ButtonPrint.ShowWindow(SW_HIDE);
		}

		m_ButtonCopy.SetImage(afxGlobalData.Is32BitIcons() ? IDB_AFXBARRES_COPY32 : IDB_AFXBARRES_COPY, NULL);
		m_ButtonCopy.GetWindowText(strTT);
		m_ButtonCopy.SetWindowText(_T(""));
		m_ButtonCopy.SetTooltip(strTT);
		m_ButtonCopy.SizeToContent();
		m_ButtonCopy.m_bDrawFocus = FALSE;

		// Add columns:
		OnSetColumns();
		SetColumnsWidth();
	}

	// Find all application document templates and fill accelerator tables  combobox by document template data:
	CDocManager* pDocManager = AfxGetApp()->m_pDocManager;
	if (pDocManager != NULL)
	{
		// Walk all templates in the application:
		for (POSITION pos = pDocManager->GetFirstDocTemplatePosition(); pos != NULL;)
		{
			CMultiDocTemplateEx* pTemplate = (CMultiDocTemplateEx*) pDocManager->GetNextDocTemplate(pos);
			ASSERT_VALID(pTemplate);
			ASSERT_KINDOF(CDocTemplate, pTemplate);

			// We are interested in CMultiDocTemplateEx objects with the shared menu only....
			if (!pTemplate->IsKindOf(RUNTIME_CLASS(CMultiDocTemplate)) || pTemplate->m_hAccelTable == NULL)
			{
				continue;
			}

			CString strName;
			pTemplate->GetDocString(strName, CDocTemplate::fileNewName);

			int iIndex = m_wndViewTypeList.AddString(strName);
			m_wndViewTypeList.SetItemData(iIndex, (DWORD_PTR) pTemplate);
		}
	}

	// Add a default application:
	CFrameWnd* pWndMain = DYNAMIC_DOWNCAST(CFrameWnd, m_pWndParentFrame);
	if (pWndMain != NULL && pWndMain->m_hAccelTable != NULL)
	{
		CString strName;
		ENSURE(strName.LoadString(IDS_AFXBARRES_DEFAULT_VIEW));

		int iIndex = m_wndViewTypeList.AddString(strName);
		m_wndViewTypeList.SetItemData(iIndex, (DWORD_PTR) NULL);

		m_wndViewTypeList.SetCurSel(iIndex);
		OnSelchangeViewType();
	}

	m_KeymapList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	OnSelchangeViewType();

	// Initialize commands by category:
	m_pDlgCust = new CMFCToolBarsCustomizeDialog(m_pWndParentFrame, TRUE);
	m_pDlgCust->EnableUserDefinedToolbars();
	m_pDlgCust->FillCategoriesComboBox(m_wndCategoryList);

	m_wndCategoryList.SetCurSel(0);
	OnSelchangeCategory();

	CWinAppEx* pApp = DYNAMIC_DOWNCAST(CWinAppEx, AfxGetApp());

	// Restore window position and size:
	if (pApp != NULL)
	{
		ASSERT_VALID(pApp);

		CSettingsStoreSP regSP;
		CSettingsStore& reg = regSP.Create(FALSE, TRUE);

		CRect rectPosition;

		if (reg.Open(pApp->GetRegSectionPath(strWindowPlacementRegSection)) && reg.Read(strRectKey, rectPosition))
		{
			MoveWindow(rectPosition);
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CMFCKeyMapDialog::OnSelchangeViewType()
{
	m_hAccelTable = NULL;

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

		hicon = AfxGetApp ()->LoadIcon (pTemplate->GetResId());
		m_hAccelTable = pTemplate->m_hAccelTable;
	}
	else
	{
		CFrameWnd* pWndMain = DYNAMIC_DOWNCAST(CFrameWnd, m_pWndParentFrame);
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
	OnSelchangeCategory();
}

void CMFCKeyMapDialog::OnSelchangeCategory()
{
	UpdateData();

	ENSURE(m_lpAccel != NULL);

	int iIndex = m_wndCategoryList.GetCurSel();
	if (iIndex == LB_ERR)
	{
		return;
	}

	CObList* pCategoryButtonsList = (CObList*) m_wndCategoryList.GetItemData(iIndex);
	ASSERT_VALID(pCategoryButtonsList);

	int nItem = 0;
	m_KeymapList.DeleteAllItems();

	for (POSITION pos = pCategoryButtonsList->GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarButton* pButton = (CMFCToolBarButton*) pCategoryButtonsList->GetNext(pos);
		ENSURE(pButton != NULL);

		if (pButton->m_nID > 0 && pButton->m_nID != (UINT) -1)
		{
			OnInsertItem(pButton, nItem ++);
		}
	}

	m_KeymapList.SortItems(listCompareFunc, (LPARAM) this);
}

void CMFCKeyMapDialog::OnCopy()
{
	m_KeymapList.SetFocus();
	CopyKeyMap();
}

void CMFCKeyMapDialog::OnPrint()
{
	m_KeymapList.SetFocus();
	PrintKeyMap();
}

void CMFCKeyMapDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (m_KeymapList.GetSafeHwnd() == NULL)
	{
		return;
	}

	// List of keys should cover the whole bottom part of the dialog:
	CRect rectList;
	m_KeymapList.GetClientRect(rectList);
	m_KeymapList.MapWindowPoints(this, &rectList);

	CRect rectClient;
	GetClientRect(rectClient);

	rectList.right = rectClient.right;
	rectList.bottom = rectClient.bottom;

	m_KeymapList.SetWindowPos(NULL, -1, -1, rectList.Width(), rectList.Height(), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);

	// Adjust the columns width:
	SetColumnsWidth();
}

BOOL CMFCKeyMapDialog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR* pNMHDR = (NMHDR*) lParam;
	if (pNMHDR != NULL && pNMHDR->code == HDN_ITEMCLICK)
	{
		HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;

		if (phdn->iButton == 0) // Left button
		{
			if (phdn->iItem == m_nSortedCol)
				m_bSortAscending = !m_bSortAscending;
			else
				m_bSortAscending = TRUE;

			m_nSortedCol = phdn->iItem;
			m_KeymapList.SortItems(listCompareFunc, (LPARAM) this);
			m_KeymapList.SetSortColumn(m_nSortedCol, m_bSortAscending);
		}
	}

	return CDialogEx::OnNotify(wParam, lParam, pResult);
}

void CMFCKeyMapDialog::CopyKeyMap()
{
	int i = m_KeymapList.GetSelectedCount();
	if (i <= 0)
	{
		MessageBeep((UINT)-1);
		return;
	}

	CString strText;
	int nItem = -1;
	int nFlag = (m_KeymapList.GetSelectedCount() > 0)  ?  LVNI_SELECTED : LVNI_ALL;

	while ((nItem = m_KeymapList.GetNextItem(nItem, nFlag)) >= 0)
	{
		strText += FormatItem(nItem) + _T("\r\n");
	}

	if (!strText.IsEmpty () && OpenClipboard())
	{
		EmptyClipboard();

		HGLOBAL hClipbuffer = ::GlobalAlloc(GHND, (strText.GetLength() + 1) * sizeof(TCHAR));
		ENSURE(hClipbuffer != NULL);

		LPTSTR lpszBuffer = (LPTSTR) GlobalLock(hClipbuffer);
		ENSURE(lpszBuffer != NULL);

#pragma warning(disable : 6383)
		_tcscpy_s (lpszBuffer, (strText.GetLength() + 1) * sizeof(TCHAR), (LPCTSTR) strText);
#pragma warning(default : 6383)

		::GlobalUnlock(hClipbuffer);
		::SetClipboardData(AFX_TCF_TEXT, hClipbuffer);

		CloseClipboard();
	}
}

void CMFCKeyMapDialog::PrintKeyMap()
{
	CWaitCursor WaitCursor;

	int nItem = -1;
	int nFlag = (m_KeymapList.GetSelectedCount() > 0)  ?  LVNI_SELECTED : LVNI_ALL;

	CPrintDialog dlgPrint(FALSE, PD_ALLPAGES | PD_RETURNDC | PD_NOSELECTION, NULL);
	if (dlgPrint.DoModal() != IDOK)
	{
		return;
	}

	// Obtain a handle to the device context.
	HDC hdcPrn = dlgPrint.GetPrinterDC();
	if (hdcPrn == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	CDC dc;
	dc.Attach(hdcPrn);

	CSize szPage(dc.GetDeviceCaps(HORZRES), dc.GetDeviceCaps(VERTRES));

	dc.StartDoc(_T("AfxKeyMapDlg"));  // begin a new print job
	dc.StartPage();

	int nPage = 1;
	int y = OnPrintHeader(dc, nPage, szPage.cx);

	while ((nItem = m_KeymapList.GetNextItem(nItem, nFlag)) >= 0)
	{
		int nItemHeight = OnPrintItem(dc, nItem, y, szPage.cx, TRUE /* Calc height */);
		if (y + nItemHeight > szPage.cy)
		{
			dc.EndPage();

			dc.StartPage();
			y = OnPrintHeader(dc, ++nPage, szPage.cx);
		}

		y += OnPrintItem(dc, nItem, y, szPage.cx, FALSE /* Draw */);
	}

	dc.EndPage();
	dc.EndDoc();
}

int CMFCKeyMapDialog::OnPrintHeader(CDC& dc, int nPage, int cx) const
{
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);

	int nYMargin = tm.tmHeight * 2;

	CString strAppName = (AfxGetApp()->m_pszAppName == NULL) ? _T("") : AfxGetApp()->m_pszAppName;

	CString strTitle;
	GetWindowText(strTitle);

	CString strCaption;
	strCaption.Format(_T("- %d -\r\n%s: %s"), nPage, (LPCTSTR)strAppName, (LPCTSTR)strTitle);

	CRect rectText(0, nYMargin, cx, 32767);
	return dc.DrawText(strCaption, rectText, DT_WORDBREAK | DT_CENTER) + 2 * nYMargin;
}

int CMFCKeyMapDialog::OnPrintItem(CDC& dc, int nItem, int y, int cx, BOOL bCalcHeight) const
{
	ASSERT_VALID(this);
	ASSERT(nItem >= 0);

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);

	int nXMargin = tm.tmMaxCharWidth * 2;

	CMFCToolBarButton* pButton = (CMFCToolBarButton*) m_KeymapList.GetItemData(nItem);
	ASSERT_VALID(pButton);

	CString strCommand = pButton->m_strText;
	CString strKeys = m_KeymapList.GetItemText(nItem, nColumnKeys);
	CString strDescr = m_KeymapList.GetItemText(nItem, nColumnDescr);

	// Define column width:
	int nKeyColumWidth = dc.GetTextExtent(CString(_T("Ctrl+Shift+W"))).cx + nXMargin;
	int nRestOfWidth = cx - nKeyColumWidth - 2 * nXMargin;

	int nHeight = 1;

	for (int iStep = 0; iStep <(bCalcHeight ? 1 : 2); iStep ++)
	{
		UINT uiFormat = iStep == 0 ? (DT_CALCRECT | DT_WORDBREAK) : (DT_WORDBREAK | DT_END_ELLIPSIS);

		CRect rectCmd(CPoint(nXMargin, y), CSize(nRestOfWidth / 3, 32676));
		int nCmdHeight = dc.DrawText(strCommand, rectCmd, uiFormat);

		CRect rectKey(CPoint(rectCmd.right + nXMargin, y), CSize(nKeyColumWidth, 32676));
		int nKeyHeight = dc.DrawText(strKeys, rectKey, uiFormat);

		CRect rectDescr(rectKey.right + nXMargin, y, cx, 32676);
		int nDescrHeight = dc.DrawText(strDescr, rectDescr, uiFormat);

		nHeight = max(nCmdHeight, max(nKeyHeight, nDescrHeight));
	}

	return nHeight;
}

void CMFCKeyMapDialog::SetColumnsWidth()
{
	CRect rectList;
	m_KeymapList.GetClientRect(rectList);

	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(m_KeymapList.GetFont());
	ASSERT_VALID(pOldFont);

	int nKeyColumWidth = dc.GetTextExtent(CString(_T("Ctrl+Shift+W"))).cx + 10;

	dc.SelectObject(pOldFont);

	int nRestOfWidth = rectList.Width() - nKeyColumWidth - ::GetSystemMetrics(SM_CXHSCROLL);

	m_KeymapList.SetColumnWidth(nColumnCommand, nRestOfWidth / 3);
	m_KeymapList.SetColumnWidth(nColumnKeys, nKeyColumWidth);
	m_KeymapList.SetColumnWidth(nColumnDescr, nRestOfWidth * 2 / 3);
}

void CMFCKeyMapDialog::OnDestroy()
{
	//----------------------------------
	// Save window position and size:
	//----------------------------------
	CWinAppEx* pApp = DYNAMIC_DOWNCAST(CWinAppEx, AfxGetApp());
	if (pApp != NULL)
	{
		ASSERT_VALID(pApp);

		CRect rectPosition;
		GetWindowRect(rectPosition);

		CSettingsStoreSP regSP;
		CSettingsStore& reg = regSP.Create(FALSE, FALSE);

		if (reg.CreateKey(pApp->GetRegSectionPath(strWindowPlacementRegSection)))
		{
			reg.Write(strRectKey, rectPosition);
		}
	}

	CDialogEx::OnDestroy();
}

CString CMFCKeyMapDialog::FormatItem(int nItem) const
{
	ASSERT_VALID(this);

	CString strKeys = m_KeymapList.GetItemText(nItem, nColumnKeys);
	if (strKeys.IsEmpty())
	{
		strKeys = _T("-");
	}

	CString strItem;
	strItem.Format(_T("%-30s\t%-20s\t%s"), (LPCTSTR)m_KeymapList.GetItemText(nItem, nColumnCommand), (LPCTSTR)strKeys, (LPCTSTR)m_KeymapList.GetItemText(nItem, nColumnDescr));
	return strItem;
}

void CMFCKeyMapDialog::OnSetColumns()
{
	CString strCaption;

	ENSURE(strCaption.LoadString(IDS_AFXBARRES_COMMAND));
	m_KeymapList.InsertColumn(nColumnCommand, strCaption);

	ENSURE(strCaption.LoadString(IDS_AFXBARRES_KEYS));
	m_KeymapList.InsertColumn(nColumnKeys, strCaption);

	ENSURE(strCaption.LoadString(IDS_AFXBARRES_DESCRIPTION));
	m_KeymapList.InsertColumn(nColumnDescr, strCaption);
}

void CMFCKeyMapDialog::OnInsertItem(CMFCToolBarButton* pButton, int nItem)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pButton);

	// Set command name:
	CString strText = pButton->m_strTextCustom.IsEmpty() ? pButton->m_strText : pButton->m_strTextCustom;

	int iIndex = m_KeymapList.InsertItem(nItem, strText, -1);
	m_KeymapList.SetItemData(iIndex, (DWORD_PTR) pButton);

	m_KeymapList.SetItemText(iIndex, nColumnKeys, GetCommandKeys(pButton->m_nID));

	// Set command description:
	CString strDescr;
	CFrameWnd* pParent = GetParentFrame();

	if (pParent != NULL && pParent->GetSafeHwnd() != NULL)
	{
		pParent->GetMessageString(pButton->m_nID, strDescr);
	}

	m_KeymapList.SetItemText(iIndex, nColumnDescr, strDescr);
}

CString CMFCKeyMapDialog::GetCommandKeys(UINT uiCmdID) const
{
	// Fill keys associated with selected command:
	CString strKey;

	for (int i = 0; i < m_nAccelSize; i ++)
	{
		if (uiCmdID == m_lpAccel [i].cmd)
		{
			ENSURE(&m_lpAccel [i] != NULL);

			CMFCAcceleratorKey helper(&m_lpAccel [i]);
			CString sNewKey;
			helper.Format(sNewKey);

			if (!strKey.IsEmpty())
			{
				strKey += _T("; ");
			}

			strKey += sNewKey;
		}
	}

	return strKey;
}


