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
#include "multimon.h"
#include "afxdesktopalertdialog.h"

#include "afxdesktopalertwnd.h"
#include "afxribbonres.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define AFX_MAX_CLASS_NAME 255
#define AFX_STATIC_CLASS _T("Static")
#define AFX_BUTTON_CLASS _T("Button")

#define AFX_MAX_TEXT_LEN 512

IMPLEMENT_DYNCREATE(CMFCDesktopAlertDialog, CDialogEx)

/////////////////////////////////////////////////////////////////////////////
// CMFCDesktopAlertDialog

CMFCDesktopAlertDialog::CMFCDesktopAlertDialog()
{
	m_pParentPopup = NULL;
	m_bDefault = FALSE;
	m_sizeDlg = CSize(0, 0);
	m_bDontSetFocus = FALSE;
	m_bMenuIsActive = FALSE;
}

CMFCDesktopAlertDialog::~CMFCDesktopAlertDialog()
{
}

//{{AFX_MSG_MAP(CMFCDesktopAlertDialog)
BEGIN_MESSAGE_MAP(CMFCDesktopAlertDialog, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_MESSAGE(WM_PRINTCLIENT, &CMFCDesktopAlertDialog::OnPrintClient)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// CMFCDesktopAlertDialog message handlers

HBRUSH CMFCDesktopAlertDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetBkMode(TRANSPARENT);

		if (afxGlobalData.IsHighContrastMode())
		{
			pDC->SetTextColor(afxGlobalData.clrWindowText);
		}

		return(HBRUSH) ::GetStockObject(HOLLOW_BRUSH);
	}

	return CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL CMFCDesktopAlertDialog::OnEraseBkgnd(CDC* pDC)
{
	if (!afxGlobalData.IsWindowsThemingDrawParentBackground())
	{
		CRect rectClient;
		GetClientRect(&rectClient);

		CMFCVisualManager::GetInstance()->OnFillPopupWindowBackground(pDC, rectClient);
	}

	return TRUE;
}

void CMFCDesktopAlertDialog::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CMemDC memDC(dc, this);
	CDC* pDC = &memDC.GetDC();

	OnDraw(pDC);
}

void CMFCDesktopAlertDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	CDialogEx::OnLButtonDown(nFlags, point);

	GetParent()->SendMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(point.x, point.y));
	SetFocus();
}

BOOL CMFCDesktopAlertDialog::HasFocus() const
{
	if (GetSafeHwnd() == NULL)
	{
		return FALSE;
	}

	if (m_bMenuIsActive)
	{
		return TRUE;
	}

	CWnd* pWndMain = AfxGetMainWnd();
	if (pWndMain->GetSafeHwnd() == NULL)
	{
		return FALSE;
	}

	if (pWndMain->IsIconic() || !pWndMain->IsWindowVisible() || pWndMain != GetForegroundWindow())
	{
		return FALSE;
	}

	CWnd* pFocus = GetFocus();

	BOOL bActive = (pFocus->GetSafeHwnd() != NULL && (IsChild(pFocus) || pFocus->GetSafeHwnd() == GetSafeHwnd()));

	return bActive;
}

BOOL CMFCDesktopAlertDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	ASSERT_VALID(m_pParentPopup);

	if (m_pParentPopup->ProcessCommand((HWND)lParam))
	{
		return TRUE;
	}

	if (m_btnURL.GetSafeHwnd() == (HWND) lParam && m_btnURL.GetDlgCtrlID() == LOWORD(wParam) && m_pParentPopup->OnClickLinkButton(m_btnURL.GetDlgCtrlID()))
	{
		return TRUE;
	}

	return CDialogEx::OnCommand(wParam, lParam);
}

int CMFCDesktopAlertDialog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_pParentPopup = DYNAMIC_DOWNCAST(CMFCDesktopAlertWnd, GetParent());
	ASSERT_VALID(m_pParentPopup);

	return 0;
}

BOOL CMFCDesktopAlertDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CWnd* pWndChild = GetWindow(GW_CHILD);
	while (pWndChild != NULL)
	{
		ASSERT_VALID(pWndChild);

		CMFCButton* pButton = DYNAMIC_DOWNCAST(CMFCButton, pWndChild);
		if (pButton != NULL)
		{
			pButton->m_bDrawFocus = FALSE;
		}
		else
		{
			TCHAR lpszClassName [AFX_MAX_CLASS_NAME + 1];

			::GetClassName(pWndChild->GetSafeHwnd(), lpszClassName, AFX_MAX_CLASS_NAME);
			CString strClass = lpszClassName;

			if (strClass == AFX_STATIC_CLASS &&(pWndChild->GetStyle() & SS_ICON))
			{
				pWndChild->ShowWindow(SW_HIDE);
			}
		}

		pWndChild = pWndChild->GetNextWindow();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CMFCDesktopAlertDialog::OnPrintClient(WPARAM wp, LPARAM lp)
{
	if (lp & PRF_CLIENT)
	{
		CDC* pDC = CDC::FromHandle((HDC) wp);
		ASSERT_VALID(pDC);

		OnDraw(pDC);
	}

	return 0;
}

void CMFCDesktopAlertDialog::OnDraw(CDC* pDC)
{
	ASSERT_VALID(pDC);

	CRect rectClient;
	GetClientRect(&rectClient);

	CMFCVisualManager::GetInstance()->OnFillPopupWindowBackground(pDC, rectClient);

	CWnd* pWndChild = GetWindow(GW_CHILD);

	while (pWndChild != NULL)
	{
		ASSERT_VALID(pWndChild);

		TCHAR lpszClassName [AFX_MAX_CLASS_NAME + 1];

		::GetClassName(pWndChild->GetSafeHwnd(), lpszClassName, AFX_MAX_CLASS_NAME);
		CString strClass = lpszClassName;

		if (strClass == AFX_STATIC_CLASS &&(pWndChild->GetStyle() & SS_ICON))
		{
			CRect rectIcon;
			pWndChild->GetWindowRect(rectIcon);
			ScreenToClient(rectIcon);

			HICON hIcon = ((CStatic*) pWndChild)->GetIcon();
			pDC->DrawIcon(rectIcon.TopLeft(), hIcon);
		}

		pWndChild = pWndChild->GetNextWindow();
	}
}

CSize CMFCDesktopAlertDialog::GetOptimalTextSize(CString str)
{
	if (str.IsEmpty())
	{
		return CSize(0, 0);
	}

	CRect rectScreen;
	CRect rectDlg;
	GetWindowRect(rectDlg);

	MONITORINFO mi;
	mi.cbSize = sizeof(MONITORINFO);
	if (GetMonitorInfo(MonitorFromPoint(rectDlg.TopLeft(), MONITOR_DEFAULTTONEAREST), &mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		::SystemParametersInfo(SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	CClientDC dc(this);

	CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontRegular);
	ASSERT_VALID(pOldFont);

	int nStepY = afxGlobalData.GetTextHeight();
	int nStepX = nStepY * 3;

	CRect rectText(0, 0, nStepX, nStepY);

	for (;;)
	{
		CRect rectTextSaved = rectText;

		int nHeight = dc.DrawText(str, rectText, DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX);
		int nWidth = rectText.Width();

		rectText = rectTextSaved;

		if (nHeight <= rectText.Height() || rectText.Width() > rectScreen.Width() || rectText.Height() > rectScreen.Height())
		{
			rectText.bottom = rectText.top + nHeight + 5;
			rectText.right = rectText.left + nWidth + 5;
			break;
		}

		rectText.right += nStepX;
		rectText.bottom += nStepY;
	}

	dc.SelectObject(pOldFont);
	return rectText.Size();
}

BOOL CMFCDesktopAlertDialog::CreateFromParams(CMFCDesktopAlertWndInfo& params, CMFCDesktopAlertWnd* pParent)
{
	if (!Create(IDD_AFXBARRES_POPUP_DLG, pParent))
	{
		return FALSE;
	}

	m_Params = params;

	int nXMargin = 10;
	int nYMargin = 10;

	int x = nXMargin;
	int y = nYMargin;

	int cxIcon = 0;

	CString strText = m_Params.m_strText;
	if (strText.GetLength() > AFX_MAX_TEXT_LEN)
	{
		strText = strText.Left(AFX_MAX_TEXT_LEN - 1);
	}

	CString strURL = m_Params.m_strURL;
	if (strURL.GetLength() > AFX_MAX_TEXT_LEN)
	{
		strURL = strURL.Left(AFX_MAX_TEXT_LEN - 1);
	}

	CSize sizeText = GetOptimalTextSize(strText);
	CSize sizeURL = GetOptimalTextSize(strURL);

	int cx = max(sizeText.cx, sizeURL.cx);

	if (m_Params.m_hIcon != NULL)
	{
		ICONINFO iconInfo;
		::GetIconInfo(m_Params.m_hIcon, &iconInfo);

		BITMAP bitmap;
		::GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bitmap);

		::DeleteObject(iconInfo.hbmColor);
		::DeleteObject(iconInfo.hbmMask);

		CRect rectIcon = CRect(nXMargin, nYMargin, bitmap.bmWidth + nXMargin, bitmap.bmHeight + nYMargin);

		m_wndIcon.Create(_T(""), WS_CHILD | SS_ICON | SS_NOPREFIX, rectIcon, this);
		m_wndIcon.SetIcon(m_Params.m_hIcon);

		cxIcon = rectIcon.Width() + nXMargin;
		x += cxIcon;
	}

	if (!strText.IsEmpty())
	{
		CRect rectText(CPoint(x, y), CSize(cx, sizeText.cy));

		m_wndText.Create(strText, WS_CHILD | WS_VISIBLE, rectText, this);
		m_wndText.SetFont(&afxGlobalData.fontRegular);

		y = rectText.bottom + nYMargin;
	}

	if (!strURL.IsEmpty())
	{
		CRect rectURL(CPoint(x, y), CSize(cx, sizeURL.cy));

		m_btnURL.Create(strURL, WS_VISIBLE | WS_CHILD, rectURL, this, m_Params.m_nURLCmdID);

		m_btnURL.m_bMultilineText = TRUE;
		m_btnURL.m_bAlwaysUnderlineText = FALSE;
		m_btnURL.m_bDefaultClickProcess = TRUE;
		m_btnURL.m_bDrawFocus = FALSE;

		y = rectURL.bottom + nYMargin;
	}

	m_sizeDlg = CSize(cxIcon + cx + 2 * nXMargin, y);
	return TRUE;
}

CSize CMFCDesktopAlertDialog::GetDlgSize()
{
	if (!m_bDefault)
	{
		ASSERT(FALSE);
		return CSize(0, 0);
	}

	return m_sizeDlg;
}

void CMFCDesktopAlertDialog::OnSetFocus(CWnd* pOldWnd)
{
	if (m_bDontSetFocus && pOldWnd->GetSafeHwnd() != NULL)
	{
		pOldWnd->SetFocus();
		return;
	}

	CDialogEx::OnSetFocus(pOldWnd);
}

BOOL CMFCDesktopAlertDialog::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_LBUTTONDOWN && m_pParentPopup->GetSafeHwnd() != NULL)
	{
		CWnd* pWnd = CWnd::FromHandle(pMsg->hwnd);
		if (pWnd != NULL)
		{
			TCHAR lpszClassName [AFX_MAX_CLASS_NAME + 1];

			::GetClassName(pWnd->GetSafeHwnd(), lpszClassName, AFX_MAX_CLASS_NAME);
			CString strClass = lpszClassName;

			if (strClass == AFX_STATIC_CLASS || pWnd->GetSafeHwnd() == GetSafeHwnd())
			{
				m_pParentPopup->StartWindowMove();
			}
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}



