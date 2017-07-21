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
#include "afxmdiframewndex.h"
#include "afxwindowsmanagerdialog.h"
#include "afxmdichildwndex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCWindowsManagerDialog dialog

UINT AFX_WM_WINDOW_HELP = ::RegisterWindowMessage(_T("WINDOW_HELP"));

CMFCWindowsManagerDialog::CMFCWindowsManagerDialog(CMDIFrameWndEx* pMDIFrame, BOOL bHelpButton)
	: CDialog(CMFCWindowsManagerDialog::IDD, pMDIFrame), m_pMDIFrame(pMDIFrame), m_bHelpButton(bHelpButton)
{
	ASSERT_VALID(m_pMDIFrame);

	m_bMDIActions = TRUE;
}

void CMFCWindowsManagerDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMFCWindowsManagerDialog)
	DDX_Control(pDX, IDC_AFXBARRES_LIST, m_wndList);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMFCWindowsManagerDialog, CDialog)
	//{{AFX_MSG_MAP(CMFCWindowsManagerDialog)
	ON_WM_DRAWITEM()
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDC_AFXBARRES_ACTIVATE, &CMFCWindowsManagerDialog::OnActivate)
	ON_BN_CLICKED(IDC_AFXBARRES_SAVE, &CMFCWindowsManagerDialog::OnSave)
	ON_BN_CLICKED(IDC_AFXBARRES_CLOSE, &CMFCWindowsManagerDialog::OnClose)
	ON_BN_CLICKED(IDC_AFXBARRES_CASCADE, &CMFCWindowsManagerDialog::OnCascade)
	ON_BN_CLICKED(IDC_AFXBARRES_TILEHORZ, &CMFCWindowsManagerDialog::OnTilehorz)
	ON_BN_CLICKED(IDC_AFXBARRES_TILEVERT, &CMFCWindowsManagerDialog::OnTilevert)
	ON_BN_CLICKED(IDC_AFXBARRES_MINIMIZE, &CMFCWindowsManagerDialog::OnMinimize)
	ON_BN_CLICKED(ID_HELP, &CMFCWindowsManagerDialog::OnWindowHelp)
	ON_LBN_SELCHANGE(IDC_AFXBARRES_LIST, &CMFCWindowsManagerDialog::OnSelchangeList)
	ON_LBN_DBLCLK(IDC_AFXBARRES_LIST, &CMFCWindowsManagerDialog::OnActivate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCWindowsManagerDialog message handlers

void CMFCWindowsManagerDialog::MDIMessage(UINT uMsg, WPARAM flag)
{
	ASSERT_VALID(m_pMDIFrame);

	CWaitCursor wait;

	int nItems = m_wndList.GetCount();
	if (nItems <= 0)
	{
		return;
	}

	HWND hMDIClient = m_pMDIFrame->m_hWndMDIClient;

	::LockWindowUpdate(hMDIClient);

	for (int i = nItems - 1; i >= 0; i--)
	{
		HWND hWnd= (HWND) m_wndList.GetItemData(i);
		::ShowWindow(hWnd, m_wndList.GetSel(i) > 0 ? SW_RESTORE : SW_MINIMIZE);
	}

	::SendMessage(hMDIClient, uMsg, flag, 0);
	::LockWindowUpdate(NULL);
}

void CMFCWindowsManagerDialog::OnActivate()
{
	if (!CMDIFrameWndEx::m_bDisableSetRedraw)
	{
		GetParent()->SetRedraw(FALSE);
	}

	if (m_wndList.GetSelCount() == 1)
	{
		int index;
		if (m_wndList.GetSelItems(1, &index) == 1)
		{
			DWORD_PTR dw = m_wndList.GetItemData(index);
			if (dw != (DWORD_PTR) LB_ERR)
			{
				WINDOWPLACEMENT wndpl;
				wndpl.length = sizeof(WINDOWPLACEMENT);
				::GetWindowPlacement((HWND) dw,&wndpl);

				if (wndpl.showCmd == SW_SHOWMINIMIZED)
				{
					::ShowWindow((HWND) dw,SW_RESTORE);
				}

				::SendMessage(m_pMDIFrame->m_hWndMDIClient,WM_MDIACTIVATE, (WPARAM) dw,0);
				EndDialog(IDOK);
			}
		}
	}

	if (!CMDIFrameWndEx::m_bDisableSetRedraw)
	{
		GetParent()->SetRedraw(TRUE);
		GetParent()->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
	}
}

void CMFCWindowsManagerDialog::OnSave()
{
	int nItems = m_wndList.GetCount();
	if (nItems > 0)
	{
		for (int i = 0; i < nItems; i++)
		{
			if (m_wndList.GetSel(i) > 0)
			{
				HWND hWnd= (HWND) m_wndList.GetItemData(i);

				if (m_lstSaveDisabled.Find(hWnd) == NULL)
				{
					CWnd* pWnd = CWnd::FromHandle(hWnd);
					CFrameWnd* pFrame = DYNAMIC_DOWNCAST(CFrameWnd, pWnd);

					if (pFrame != NULL)
					{
						CDocument *pDoc = pFrame->GetActiveDocument();
						if (pDoc != NULL)
						{
							ASSERT_VALID(pDoc);
							pDoc->DoFileSave();
						}
					}
				}
			}
		}
	}

	FillWindowList();
	SelActive();
	UpdateButtons();
}

void CMFCWindowsManagerDialog::OnClose()
{
	int nItems = m_wndList.GetCount();
	if (nItems > 0)
	{
		HWND hMDIClient = m_pMDIFrame->m_hWndMDIClient;

		m_wndList.SetRedraw(FALSE);

		for (int i = nItems - 1; i>=0; i--)
		{
			if (m_wndList.GetSel(i) > 0)
			{
				HWND hWnd= (HWND) m_wndList.GetItemData(i);

				::SendMessage(hWnd,WM_CLOSE, (WPARAM) 0, (LPARAM) 0);

				if (::GetParent(hWnd) == hMDIClient)
				{
					break;
				}
			}
		}

		m_wndList.SetRedraw(TRUE);
	}

	FillWindowList();
	SelActive();
	UpdateButtons();
}

void CMFCWindowsManagerDialog::OnCascade()
{
	MDIMessage(WM_MDICASCADE,0);
}

void CMFCWindowsManagerDialog::OnTilehorz()
{
	MDIMessage(WM_MDITILE, MDITILE_HORIZONTAL);
}

void CMFCWindowsManagerDialog::OnTilevert()
{
	MDIMessage(WM_MDITILE, MDITILE_VERTICAL);
}

void CMFCWindowsManagerDialog::OnMinimize()
{
	int nItems = m_wndList.GetCount();
	if (nItems > 0)
	{
		m_wndList.SetRedraw(FALSE);

		for (int i = nItems - 1; i >= 0; i--)
		{
			if (m_wndList.GetSel(i) > 0)
			{
				HWND hWnd= (HWND)m_wndList.GetItemData(i);
				::ShowWindow(hWnd,SW_MINIMIZE);
			}
		}

		m_wndList.SetRedraw(TRUE);
	}

	FillWindowList();
	SelActive();
	UpdateButtons();
}

BOOL CMFCWindowsManagerDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (AfxGetMainWnd() != NULL &&
		(AfxGetMainWnd()->GetExStyle() & WS_EX_LAYOUTRTL))
	{
		ModifyStyleEx(0, WS_EX_LAYOUTRTL);
	}

	FillWindowList();

	// If no MDI actions are available, hide all MDI-related buttons:

	if (!m_bMDIActions)
	{
		GetDlgItem(IDC_AFXBARRES_TILEHORZ)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_AFXBARRES_TILEVERT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_AFXBARRES_CASCADE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_AFXBARRES_MINIMIZE)->ShowWindow(SW_HIDE);
	}

	SelActive();
	UpdateButtons();

	CWnd* pBtnHelp = GetDlgItem(ID_HELP);
	if (pBtnHelp != NULL)
	{
		pBtnHelp->ShowWindow(m_bHelpButton ? SW_SHOW : SW_HIDE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CMFCWindowsManagerDialog::OnSelchangeList()
{
	UpdateButtons();
}

// Enables/Disables states of buttons
void CMFCWindowsManagerDialog::UpdateButtons()
{
	int nSel = m_wndList.GetSelCount();

	BOOL bClose = nSel > 0;
	BOOL bSave = FALSE;

	for (int i = 0; bClose && i < m_wndList.GetCount(); i++)
	{
		if (m_wndList.GetSel(i) > 0)
		{
			HWND hWnd= (HWND) m_wndList.GetItemData(i);

			if (m_lstCloseDisabled.Find(hWnd))
			{
				bClose = FALSE;
			}

			if (m_lstSaveDisabled.Find(hWnd) == NULL)
			{
				bSave = TRUE;
			}
		}
	}

	GetDlgItem(IDC_AFXBARRES_CLOSE)->EnableWindow(bClose);

	GetDlgItem(IDC_AFXBARRES_SAVE)->EnableWindow(bSave);
	GetDlgItem(IDC_AFXBARRES_TILEHORZ)->EnableWindow(m_bMDIActions && nSel>=2);
	GetDlgItem(IDC_AFXBARRES_TILEVERT)->EnableWindow(m_bMDIActions && nSel>=2);
	GetDlgItem(IDC_AFXBARRES_CASCADE)->EnableWindow(m_bMDIActions && nSel>=2);
	GetDlgItem(IDC_AFXBARRES_MINIMIZE)->EnableWindow(m_bMDIActions && nSel>0);

	GetDlgItem(IDC_AFXBARRES_ACTIVATE)->EnableWindow(nSel==1);
}

// Selects currently active window in listbox
void CMFCWindowsManagerDialog::SelActive()
{
	int nItems = m_wndList.GetCount();
	if (nItems > 0)
	{
		m_wndList.SetRedraw(FALSE);
		m_wndList.SelItemRange(FALSE, 0, nItems - 1);

		HWND hwndActive = (HWND) ::SendMessage(m_pMDIFrame->m_hWndMDIClient,WM_MDIGETACTIVE,0,0);

		for (int i = 0; i <nItems; i++)
		{
			if ((HWND) m_wndList.GetItemData(i)==hwndActive)
			{
				m_wndList.SetSel(i);
				break;
			}
		}

		m_wndList.SetRedraw(TRUE);
	}
}

// Refresh windows list
void CMFCWindowsManagerDialog::FillWindowList(void)
{
	m_wndList.SetRedraw(FALSE);
	m_wndList.ResetContent();

	int cxExtent = 0;

	CClientDC dcList(&m_wndList);
	CFont* pOldFont = dcList.SelectObject(GetFont());
	ASSERT_VALID(pOldFont);

	m_bMDIActions = TRUE;
	m_lstCloseDisabled.RemoveAll();
	m_lstSaveDisabled.RemoveAll();

	HWND hwndT = ::GetWindow(m_pMDIFrame->m_hWndMDIClient, GW_CHILD);
	while (hwndT != NULL)
	{
		CMDIChildWndEx* pFrame = DYNAMIC_DOWNCAST(CMDIChildWndEx, CWnd::FromHandle(hwndT));
		if (pFrame == NULL)
		{
			hwndT = ::GetWindow(hwndT,GW_HWNDNEXT);
			continue;
		}

		if (!pFrame->CanShowOnWindowsList())
		{
			hwndT = ::GetWindow(hwndT,GW_HWNDNEXT);
			continue;
		}

		TCHAR szWndTitle[256];
		::GetWindowText(hwndT,szWndTitle,sizeof(szWndTitle)/sizeof(szWndTitle[0]));

		int index = m_wndList.AddString(szWndTitle);

		int cxCurr = dcList.GetTextExtent(szWndTitle).cx;
		cxExtent = max(cxExtent, cxCurr);

		m_wndList.SetItemData(index, (DWORD_PTR) hwndT);

		if (pFrame != NULL && pFrame->IsReadOnly())
		{
			m_lstSaveDisabled.AddTail(hwndT);
		}

		DWORD dwStyle = ::GetWindowLong(hwndT, GWL_STYLE);
		if ((dwStyle & WS_SYSMENU) == 0)
		{
			m_bMDIActions = FALSE;
		}
		else
		{
			HMENU hSysMenu = ::GetSystemMenu(hwndT, FALSE);
			if (hSysMenu == NULL)
			{
				m_bMDIActions = FALSE;
			}
			else
			{
				MENUITEMINFO menuInfo;
				ZeroMemory(&menuInfo,sizeof(MENUITEMINFO));
				menuInfo.cbSize = sizeof(MENUITEMINFO);
				menuInfo.fMask = MIIM_STATE;

				if (!::GetMenuItemInfo(hSysMenu, SC_CLOSE, FALSE, &menuInfo) || (menuInfo.fState & MFS_GRAYED) || (menuInfo.fState & MFS_DISABLED))
				{
					m_lstCloseDisabled.AddTail(hwndT);
				}
			}
		}

		hwndT=::GetWindow(hwndT,GW_HWNDNEXT);
	}

	m_wndList.SetHorizontalExtent(cxExtent + ::GetSystemMetrics(SM_CXHSCROLL) + 30);
	dcList.SelectObject(pOldFont);

	m_wndList.SetRedraw(TRUE);
}

void CMFCWindowsManagerDialog::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDIS)
{
	if (nIDCtl != IDC_AFXBARRES_LIST)
	{
		CDialog::OnDrawItem(nIDCtl, lpDIS);
	}

	if (lpDIS->itemID == LB_ERR)
	{
		return;
	}

	CBrush& brFill = (lpDIS->itemState & ODS_SELECTED) ? afxGlobalData.brHilite : afxGlobalData.brWindow;
	COLORREF clText = (lpDIS->itemState & ODS_SELECTED) ? afxGlobalData.clrTextHilite : afxGlobalData.clrWindowText;
	CRect rect = lpDIS->rcItem;
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);

	if (lpDIS->itemAction &(ODA_DRAWENTIRE | ODA_SELECT))
	{
		pDC->FillRect(rect, &brFill);
	}

	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(clText);

	//-----------
	// Draw text:
	//-----------
	CString str;
	m_wndList.GetText(lpDIS->itemID, str);

	CRect rectText = rect;
	rectText.left += rectText.Height() + 4;

	pDC->DrawText(str, rectText, DT_LEFT | DT_VCENTER | DT_NOPREFIX| DT_SINGLELINE);

	//-----------
	// Draw icon:
	//-----------
	HICON hIcon = (HICON)(LONG_PTR) GetClassLongPtr((HWND) lpDIS->itemData, GCLP_HICONSM);
	if (hIcon != NULL)
	{
		CRect rectIcon = rect;
		rectIcon.right = rectIcon.left + rectIcon.Height();
		rectIcon.DeflateRect(2, 0);

		::DrawIconEx(pDC->GetSafeHdc(), rectIcon.left, rectIcon.top, hIcon, rectIcon.Height(), rectIcon.Height(), 0, NULL, DI_NORMAL);
	}

	if (lpDIS->itemAction & ODA_FOCUS)
	{
		pDC->DrawFocusRect(rect);
	}
}

void CMFCWindowsManagerDialog::OnWindowHelp()
{
	CWnd* pParentFrame = AfxGetMainWnd();
	pParentFrame->SendMessage(AFX_WM_WINDOW_HELP, 0, (LPARAM) this);
}

BOOL CMFCWindowsManagerDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
	pHelpInfo->iCtrlId;
	CWnd* pParentFrame = AfxGetMainWnd();
	pParentFrame->SendMessage(AFX_WM_WINDOW_HELP, 0, (LPARAM) this);
	return FALSE;
}


