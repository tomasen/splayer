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
#include "afxcolordialog.h"
#include "afxcolorpropertysheet.h"
#include "afxglobals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CScreenWnd window

class CScreenWnd : public CWnd
{
// Construction
public:
	CScreenWnd();

// Overrides
public:
	virtual BOOL Create(CMFCColorDialog* pColorDlg);

// Implementation
public:
	virtual ~CScreenWnd();

// Generated message map functions
protected:
	//{{AFX_MSG(CScreenWnd)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CMFCColorDialog* m_pColorDlg;
};

/////////////////////////////////////////////////////////////////////////////
// CMFCColorDialog dialog

CMFCColorDialog::CMFCColorDialog(COLORREF clrInit, DWORD /*dwFlags - reserved */, CWnd* pParentWnd, HPALETTE hPal)
	: CDialogEx(CMFCColorDialog::IDD, pParentWnd)
{
	//{{AFX_DATA_INIT(CMFCColorDialog)
	//}}AFX_DATA_INIT

	m_pColourSheetOne = NULL;
	m_pColourSheetTwo = NULL;

	m_CurrentColor = m_NewColor = clrInit;
	m_pPropSheet = NULL;
	m_bIsMyPalette = TRUE;
	m_pPalette = NULL;

	if (hPal != NULL)
	{
		m_pPalette = CPalette::FromHandle(hPal);
		m_bIsMyPalette = FALSE;
	}

	m_bPickerMode = FALSE;
}

CMFCColorDialog::~CMFCColorDialog()
{
	if (m_pColourSheetOne != NULL)
	{
		delete m_pColourSheetOne;
	}

	if (m_pColourSheetTwo != NULL)
	{
		delete m_pColourSheetTwo;
	}
}

void CMFCColorDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CMFCColorDialog)
	DDX_Control(pDX, IDC_AFXBARRES_COLOR_SELECT, m_btnColorSelect);
	DDX_Control(pDX, IDC_AFXBARRES_STATICPLACEHOLDER, m_wndStaticPlaceHolder);
	DDX_Control(pDX, IDC_AFXBARRES_COLOURPLACEHOLDER, m_wndColors);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMFCColorDialog, CDialogEx)
	//{{AFX_MSG_MAP(CMFCColorDialog)
	ON_WM_DESTROY()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_AFXBARRES_COLOR_SELECT, &CMFCColorDialog::OnColorSelect)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCColorDialog message handlers

BOOL CMFCColorDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	if (AfxGetMainWnd() != NULL && (AfxGetMainWnd()->GetExStyle() & WS_EX_LAYOUTRTL))
	{
		ModifyStyleEx(0, WS_EX_LAYOUTRTL);
	}

	if (afxGlobalData.m_nBitsPerPixel < 8) // 16 colors, call standard dialog
	{
		CColorDialog dlg(m_CurrentColor, CC_FULLOPEN | CC_ANYCOLOR);
		int nResult = (int) dlg.DoModal();
		m_NewColor = dlg.GetColor();
		EndDialog(nResult);

		return TRUE;
	}

	if (m_pPalette == NULL)
	{
		m_pPalette = new CPalette();
		RebuildPalette();
	}

	m_wndColors.SetType(CMFCColorPickerCtrl::CURRENT);
	m_wndColors.SetPalette(m_pPalette);

	m_wndColors.SetOriginalColor(m_CurrentColor);
	m_wndColors.SetColor(m_NewColor);

	// Create property sheet.
	m_pPropSheet = new CMFCColorPropertySheet(_T(""), this);
	ENSURE(m_pPropSheet);

	m_pColourSheetOne = new CMFCStandardColorsPropertyPage;
	m_pColourSheetTwo = new CMFCCustomColorsPropertyPage;

	// Set parent dialog.
	m_pColourSheetOne->m_pDialog = this;
	m_pColourSheetTwo->m_pDialog = this;

	m_pPropSheet->AddPage(m_pColourSheetOne);
	m_pPropSheet->AddPage(m_pColourSheetTwo);

	// Retrieve the location of the window
	CRect rectListWnd;
	m_wndStaticPlaceHolder.GetWindowRect(rectListWnd);
	ScreenToClient(rectListWnd);

	if (!m_pPropSheet->Create(this, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, 0))
	{
		TRACE0("CMFCColorDialog::OnInitDialog(): Can't create the property sheet.....\n");
	}

	m_pPropSheet->SetWindowPos(NULL, rectListWnd.left, rectListWnd.top, rectListWnd.Width(), rectListWnd.Height(), SWP_NOZORDER | SWP_NOACTIVATE);

	SetPageOne(GetRValue(m_CurrentColor), GetGValue(m_CurrentColor), GetBValue(m_CurrentColor));
	SetPageTwo(GetRValue(m_CurrentColor), GetGValue(m_CurrentColor), GetBValue(m_CurrentColor));

	m_btnColorSelect.SetImage(IDB_AFXBARRES_COLOR_PICKER);

	m_hcurPicker = AfxGetApp()->LoadCursor(IDC_AFXBARRES_COLOR);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMFCColorDialog::SetCurrentColor(COLORREF rgb)
{
	m_CurrentColor = rgb;
}

void CMFCColorDialog::SetPageOne(BYTE R, BYTE G, BYTE B)
{
	m_pColourSheetOne->m_hexpicker.SelectCellHexagon(R, G, B);
	m_pColourSheetOne->m_hexpicker_greyscale.SelectCellHexagon(R, G, B);
}

void CMFCColorDialog::SetPageTwo(BYTE R, BYTE G, BYTE B)
{
	m_pColourSheetTwo->Setup(R, G, B);
}

void CMFCColorDialog::OnDestroy()
{
	if (m_bIsMyPalette && m_pPalette != NULL)
	{
		m_pPalette->DeleteObject();
		delete m_pPalette;
		m_pPalette = NULL;
	}

	CDialogEx::OnDestroy();
}

void CMFCColorDialog::SetNewColor(COLORREF rgb)
{
	m_NewColor = rgb;

	if (afxGlobalData.m_nBitsPerPixel == 8) // 256 colors
	{
		ENSURE(m_pPalette != NULL);

		UINT uiPalIndex = m_pPalette->GetNearestPaletteIndex(rgb);
		m_wndColors.SetColor(PALETTEINDEX(uiPalIndex));
	}
	else
	{
		m_wndColors.SetColor(rgb);
	}

	m_wndColors.Invalidate();
	m_wndColors.UpdateWindow();
}

void CMFCColorDialog::OnSysColorChange()
{
	CDialogEx::OnSysColorChange();

	afxGlobalData.UpdateSysColors();

	if (m_bIsMyPalette)
	{
		if (afxGlobalData.m_nBitsPerPixel < 8) // 16 colors, call standard dialog
		{
			ShowWindow(SW_HIDE);

			CColorDialog dlg(m_CurrentColor, CC_FULLOPEN | CC_ANYCOLOR);
			int nResult = (int) dlg.DoModal();
			m_NewColor = dlg.GetColor();
			EndDialog(nResult);
		}
		else
		{
			::DeleteObject(m_pPalette->Detach());
			RebuildPalette();

			Invalidate();
			UpdateWindow();
		}
	}
}

void CMFCColorDialog::RebuildPalette()
{
	ENSURE(m_pPalette->GetSafeHandle() == NULL);

	// Create palette:
	CClientDC dc(this);

	int nColors = 256; // Use 256 first entries
	UINT nSize = sizeof(LOGPALETTE) +(sizeof(PALETTEENTRY) * nColors);
	LOGPALETTE *pLP = (LOGPALETTE *) new BYTE[nSize];

	pLP->palVersion = 0x300;
	pLP->palNumEntries = (USHORT) nColors;

	::GetSystemPaletteEntries(dc.GetSafeHdc(), 0, nColors, pLP->palPalEntry);

	m_pPalette->CreatePalette(pLP);

	delete[] pLP;
}

void CMFCColorDialog::OnColorSelect()
{
	if (m_bPickerMode)
	{
		return;
	}

	CWinThread* pCurrThread = ::AfxGetThread();
	if (pCurrThread == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	MSG msg;
	m_bPickerMode = TRUE;;
	::SetCursor(m_hcurPicker);

	CScreenWnd* pScreenWnd = new CScreenWnd;
	if (!pScreenWnd->Create(this))
	{
		return;
	}

	SetForegroundWindow();
	BringWindowToTop();

	SetCapture();

	COLORREF colorSaved = m_NewColor;

	while (m_bPickerMode)
	{
		while (m_bPickerMode && ::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_KEYDOWN)
			{
				switch (msg.wParam)
				{
				case VK_RETURN:
					m_bPickerMode = FALSE;
					break;

				case VK_ESCAPE:
					SetNewColor(colorSaved);
					m_bPickerMode = FALSE;
					break;
				}
			}
			else if (msg.message == WM_RBUTTONDOWN || msg.message == WM_MBUTTONDOWN)
			{
				m_bPickerMode = FALSE;
			}
			else
			{
				if (!pCurrThread->PreTranslateMessage(&msg))
				{
					::TranslateMessage(&msg);
					::DispatchMessage(&msg);
				}

				pCurrThread->OnIdle(0);
			}
		}

		WaitMessage();
	}

	ReleaseCapture();
	pScreenWnd->DestroyWindow();
	delete pScreenWnd;

	m_bPickerMode = FALSE;
}

BOOL CMFCColorDialog::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (m_bPickerMode)
	{
		::SetCursor(m_hcurPicker);
		return TRUE;
	}

	return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
}

void CMFCColorDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bPickerMode)
	{
		ClientToScreen(&point);

		CClientDC dc(NULL);
		SetNewColor(dc.GetPixel(point));
	}

	CDialogEx::OnMouseMove(nFlags, point);
}

void CMFCColorDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bPickerMode = FALSE;

	SetPageOne(GetRValue(m_NewColor), GetGValue(m_NewColor), GetBValue(m_NewColor));
	SetPageTwo(GetRValue(m_NewColor), GetGValue(m_NewColor), GetBValue(m_NewColor));

	CDialogEx::OnLButtonDown(nFlags, point);
}

/////////////////////////////////////////////////////////////////////////////
// CScreenWnd

CScreenWnd::CScreenWnd()
{
}

CScreenWnd::~CScreenWnd()
{
}

BEGIN_MESSAGE_MAP(CScreenWnd, CWnd)
	//{{AFX_MSG_MAP(CScreenWnd)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScreenWnd message handlers

BOOL CScreenWnd::Create(CMFCColorDialog* pColorDlg)
{
	CWnd* pWndDesktop = GetDesktopWindow();
	if (pWndDesktop == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	m_pColorDlg = pColorDlg;

	CRect rectScreen;
	pWndDesktop->GetWindowRect(rectScreen);

	CString strClassName = ::AfxRegisterWndClass(CS_SAVEBITS, AfxGetApp()->LoadCursor(IDC_AFXBARRES_COLOR), (HBRUSH)(COLOR_BTNFACE + 1), NULL);

	return CWnd::CreateEx(WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT, strClassName, _T(""), WS_VISIBLE | WS_POPUP, rectScreen, NULL, 0);
}

void CScreenWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	MapWindowPoints(m_pColorDlg, &point, 1);
	m_pColorDlg->SendMessage(WM_MOUSEMOVE, nFlags, MAKELPARAM(point.x, point.y));

	CWnd::OnMouseMove(nFlags, point);
}

void CScreenWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	MapWindowPoints(m_pColorDlg, &point, 1);
	m_pColorDlg->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELPARAM(point.x, point.y));
}

BOOL CScreenWnd::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

BOOL CMFCColorDialog::PreTranslateMessage(MSG* pMsg)
{
#ifdef _UNICODE
#define AFX_TCF_TEXT CF_UNICODETEXT
#else
#define AFX_TCF_TEXT CF_TEXT
#endif

	if (pMsg->message == WM_KEYDOWN)
	{
		UINT nChar = (UINT) pMsg->wParam;
		BOOL bIsCtrl = (::GetAsyncKeyState(VK_CONTROL) & 0x8000);

		if (bIsCtrl &&(nChar == _T('C') || nChar == VK_INSERT))
		{
			if (OpenClipboard())
			{
				EmptyClipboard();

				CString strText;
				strText.Format(_T("RGB(%d, %d, %d)"), GetRValue(m_NewColor), GetGValue(m_NewColor), GetBValue(m_NewColor));

				HGLOBAL hClipbuffer = ::GlobalAlloc(GMEM_DDESHARE, (strText.GetLength() + 1) * sizeof(TCHAR));
				LPTSTR lpszBuffer = (LPTSTR) GlobalLock(hClipbuffer);

				lstrcpy(lpszBuffer, (LPCTSTR) strText);

				::GlobalUnlock(hClipbuffer);
				::SetClipboardData(AFX_TCF_TEXT, hClipbuffer);

				CloseClipboard();
			}
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


