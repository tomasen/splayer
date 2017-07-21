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
#include "afxcontrolbarutil.h"
#include "afxglobals.h"
#include "afxpropertygridtooltipctrl.h"
#include "afxtooltipctrl.h"
#include "afxvisualmanager.h"
#include "afxdrawmanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCPropertyGridToolTipCtrl

CString CMFCPropertyGridToolTipCtrl::m_strClassName;

IMPLEMENT_DYNAMIC(CMFCPropertyGridToolTipCtrl, CWnd)

CMFCPropertyGridToolTipCtrl::CMFCPropertyGridToolTipCtrl()
{
	m_rectLast.SetRectEmpty();
	m_nTextMargin = 10;
	m_hFont = NULL;
	m_pWndParent = NULL;
}

CMFCPropertyGridToolTipCtrl::~CMFCPropertyGridToolTipCtrl()
{
}

//{{AFX_MSG_MAP(CMFCPropertyGridToolTipCtrl)
BEGIN_MESSAGE_MAP(CMFCPropertyGridToolTipCtrl, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_MESSAGE(WM_SETFONT, &CMFCPropertyGridToolTipCtrl::OnSetFont)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// CMFCPropertyGridToolTipCtrl message handlers

BOOL CMFCPropertyGridToolTipCtrl::Create(CWnd* pWndParent)
{
	ASSERT_VALID(pWndParent);
	m_pWndParent = pWndParent;

	if (m_strClassName.IsEmpty())
	{
		m_strClassName = ::AfxRegisterWndClass(CS_SAVEBITS, ::LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_BTNFACE + 1));
	}

	return CreateEx(0, m_strClassName, _T(""), WS_POPUP, 0, 0, 0, 0, pWndParent->GetSafeHwnd(), (HMENU) NULL);
}

BOOL CMFCPropertyGridToolTipCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CMFCPropertyGridToolTipCtrl::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rect;
	GetClientRect(rect);

	CMFCToolTipInfo params;
	CMFCVisualManager::GetInstance()->GetToolTipInfo(params);

	if (params.m_clrFill == (COLORREF)-1)
	{
		::FillRect(dc.GetSafeHdc(), rect, ::GetSysColorBrush(COLOR_INFOBK));
	}
	else
	{
		if (params.m_clrFillGradient == (COLORREF)-1)
		{
			CBrush br(params.m_clrFill);
			dc.FillRect(rect, &br);
		}
		else
		{
			CDrawingManager dm(dc);

			dm.FillGradient2(rect, params.m_clrFillGradient, params.m_clrFill, params.m_nGradientAngle == -1 ? 90 : params.m_nGradientAngle);
		}
	}

	COLORREF clrLine = params.m_clrBorder == (COLORREF)-1 ? ::GetSysColor(COLOR_INFOTEXT) : params.m_clrBorder;
	COLORREF clrText = params.m_clrText == (COLORREF)-1 ? ::GetSysColor(COLOR_INFOTEXT) : params.m_clrText;

	dc.Draw3dRect(rect, clrLine, clrLine);

	CFont* pPrevFont = m_hFont == NULL ? (CFont*) dc.SelectStockObject(DEFAULT_GUI_FONT) :
	dc.SelectObject(CFont::FromHandle(m_hFont));
	ENSURE(pPrevFont != NULL);

	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(clrText);

	if (m_strText.FindOneOf(_T("\n")) != -1) // multi-line tooltip
	{
		rect.DeflateRect(m_nTextMargin, m_nTextMargin);
		if (rect.Height() < m_rectLast.Height())
		{
			// center tooltip vertically
			rect.top += (m_rectLast.Height() - rect.Height()) / 2;
		}

		dc.DrawText(m_strText, rect, DT_LEFT | DT_WORDBREAK);
	}
	else // single line tooltip
	{
		rect.DeflateRect(m_nTextMargin, 0);
		dc.DrawText(m_strText, rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
	}

	dc.SelectObject(pPrevFont);
}

void CMFCPropertyGridToolTipCtrl::Track(CRect rect, const CString& strText)
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	if (m_rectLast == rect && m_strText == strText)
	{
		return;
	}

	ASSERT_VALID(m_pWndParent);

	m_rectLast = rect;
	m_strText = strText;

	CClientDC dc(this);

	CFont* pPrevFont = m_hFont == NULL ? (CFont*) dc.SelectStockObject(DEFAULT_GUI_FONT) :
	dc.SelectObject(CFont::FromHandle(m_hFont));
	ENSURE(pPrevFont != NULL);

	int nTextHeight = rect.Height();
	int nTextWidth = rect.Width();
	if (m_strText.FindOneOf(_T("\n")) != -1) // multi-line tooltip
	{
		const int nDefaultHeight = afxGlobalData.GetTextHeight();
		const int nDefaultWidth = 200;
		CRect rectText(0, 0, nDefaultWidth, nDefaultHeight);

		nTextHeight = dc.DrawText(m_strText, rectText, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);
		nTextWidth = rectText.Width();
		nTextHeight += 2 * m_nTextMargin;
		nTextWidth += 2 * m_nTextMargin;
	}
	else
	{
		nTextWidth = dc.GetTextExtent(m_strText).cx + 2 * m_nTextMargin;
	}

	dc.SelectObject(pPrevFont);

	if (m_pWndParent->GetExStyle() & WS_EX_LAYOUTRTL)
	{
		rect.left = rect.right - nTextWidth;
	}
	else
	{
		rect.right = rect.left + nTextWidth;
	}
	rect.bottom = rect.top + nTextHeight;
	if (rect.Height() < m_rectLast.Height())
	{
		rect.top = m_rectLast.top;
		rect.bottom = m_rectLast.bottom;
	}

	MONITORINFO mi;
	mi.cbSize = sizeof(MONITORINFO);

	CRect rectScreen;

	if (GetMonitorInfo(MonitorFromPoint(rect.TopLeft(), MONITOR_DEFAULTTONEAREST), &mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		::SystemParametersInfo(SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	if (rect.Width() > rectScreen.Width())
	{
		rect.left = rectScreen.left;
		rect.right = rectScreen.right;
	}
	else if (rect.right > rectScreen.right)
	{
		rect.right = rectScreen.right;
		rect.left = rect.right - nTextWidth;
	}
	else if (rect.left < rectScreen.left)
	{
		rect.left = rectScreen.left;
		rect.right = rect.left + nTextWidth;
	}

	if (rect.Height() > rectScreen.Height())
	{
		rect.top = rectScreen.top;
		rect.bottom = rectScreen.bottom;
	}
	else if (rect.bottom > rectScreen.bottom)
	{
		rect.bottom = rectScreen.bottom;
		rect.top = rect.bottom - nTextHeight;
	}
	else if (rect.top < rectScreen.top)
	{
		rect.top = rectScreen.top;
		rect.bottom = rect.bottom + nTextHeight;
	}

	SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOACTIVATE | SWP_NOOWNERZORDER);

	ShowWindow(SW_SHOWNOACTIVATE);
	Invalidate();
	UpdateWindow();

	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
}

void CMFCPropertyGridToolTipCtrl::Hide()
{
	if (GetSafeHwnd() != NULL)
	{
		ShowWindow(SW_HIDE);
	}
}

void CMFCPropertyGridToolTipCtrl::Deactivate()
{
	m_strText.Empty();
	m_rectLast.SetRectEmpty();

	Hide();
}

LRESULT CMFCPropertyGridToolTipCtrl::OnSetFont(WPARAM wParam, LPARAM lParam)
{
	BOOL bRedraw = (BOOL) LOWORD(lParam);

	m_hFont = (HFONT) wParam;

	if (bRedraw)
	{
		Invalidate();
		UpdateWindow();
	}

	return 0;
}

BOOL CMFCPropertyGridToolTipCtrl::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message >= WM_MOUSEFIRST &&
		pMsg->message <= WM_MOUSELAST)
	{
		if (pMsg->message != WM_MOUSEMOVE)
		{
			Hide();
		}

		ASSERT_VALID(m_pWndParent);

		// the parent should receive the mouse message in its client coordinates
		CPoint pt(LOWORD(pMsg->lParam), HIWORD(pMsg->lParam));
		MapWindowPoints(m_pWndParent, &pt, 1);
		LPARAM lParam = MAKELPARAM(pt.x, pt.y);

		m_pWndParent->SendMessage(pMsg->message, pMsg->wParam, lParam);
		return TRUE;
	}

	return CWnd::PreTranslateMessage(pMsg);
}



