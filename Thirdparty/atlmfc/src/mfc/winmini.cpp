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



#pragma warning(disable: 4706) // assignment within conditional

/////////////////////////////////////////////////////////////////////////////
// CMiniFrameWnd

BEGIN_MESSAGE_MAP(CMiniFrameWnd, CFrameWnd)
	//{{AFX_MSG_MAP(CMiniFrameWnd)
	ON_WM_NCACTIVATE()
	ON_WM_NCHITTEST()
	ON_WM_SYSCOMMAND()
	ON_WM_GETMINMAXINFO()
	ON_WM_NCCREATE()
	ON_MESSAGE(WM_FLOATSTATUS, &CMiniFrameWnd::OnFloatStatus)
	ON_MESSAGE(WM_QUERYCENTERWND, &CMiniFrameWnd::OnQueryCenterWnd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// CMiniFrameWnd constructors

CMiniFrameWnd::CMiniFrameWnd()
{
	m_bActive = FALSE;
}

CMiniFrameWnd::~CMiniFrameWnd()
{
	DestroyWindow();
}

BOOL CMiniFrameWnd::Create(LPCTSTR lpClassName, LPCTSTR lpszWindowName,
	DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	return CMiniFrameWnd::CreateEx(0, lpClassName, lpszWindowName, dwStyle,
		rect, pParentWnd, nID);
}

BOOL CMiniFrameWnd::CreateEx(DWORD dwExStyle, LPCTSTR lpClassName,
	LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect,
	CWnd* pParentWnd, UINT nID)
{
	m_strCaption = lpszWindowName;
	return CWnd::CreateEx(dwExStyle, lpClassName ? lpClassName :
		AfxRegisterWndClass(CS_DBLCLKS, ::LoadCursor(NULL, IDC_ARROW)),
		lpszWindowName, dwStyle, rect.left, rect.top, rect.right - rect.left,
		rect.bottom - rect.top, pParentWnd->GetSafeHwnd(), (HMENU)(UINT_PTR)nID);
}

/////////////////////////////////////////////////////////////////////////////
// CMiniFrameWnd message handlers

BOOL CMiniFrameWnd::OnNcCreate(LPCREATESTRUCT lpcs)
{
	if (!CFrameWnd::OnNcCreate(lpcs))
		return FALSE;

	if (GetStyle() & MFS_SYNCACTIVE)
	{
		// syncronize activation state with top level parent
		CWnd* pParentWnd = EnsureTopLevelParent();
		CWnd* pActiveWnd = GetForegroundWindow();
		BOOL bActive = (pParentWnd == pActiveWnd) ||
			(pParentWnd->GetLastActivePopup() == pActiveWnd &&
			 pActiveWnd->SendMessage(WM_FLOATSTATUS, FS_SYNCACTIVE) != 0);

		// the WM_FLOATSTATUS does the actual work
		SendMessage(WM_FLOATSTATUS, bActive ? FS_ACTIVATE : FS_DEACTIVATE);
	}

	return TRUE;
}

BOOL CMiniFrameWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	// WS_4THICKFRAME and MFS_THICKFRAME imply WS_THICKFRAME
	if (cs.style & (MFS_4THICKFRAME | MFS_THICKFRAME))
		cs.style |= WS_THICKFRAME;

	// WS_CAPTION implies WS_EX_TOOLWINDOW
	if (cs.style & WS_CAPTION)
		cs.dwExStyle |= WS_EX_TOOLWINDOW;

	VERIFY(CFrameWnd::PreCreateWindow(cs));
	cs.dwExStyle &= ~(WS_EX_CLIENTEDGE);

	return TRUE;
}

void CMiniFrameWnd::OnGetMinMaxInfo(MINMAXINFO* pMMI)
{
	// allow Windows to fill in the defaults
	CFrameWnd::OnGetMinMaxInfo(pMMI);

	// don't allow sizing smaller than the non-client area
	CRect rectWindow, rectClient;
	GetWindowRect(rectWindow);
	GetClientRect(rectClient);
	pMMI->ptMinTrackSize.x = rectWindow.Width() - rectClient.right;
	pMMI->ptMinTrackSize.y = rectWindow.Height() - rectClient.bottom;
}

BOOL CMiniFrameWnd::OnNcActivate(BOOL /* bActive */)
{
	if ((GetStyle() & MFS_SYNCACTIVE) == 0)
		return Default() != 0;

	if(m_nFlags & WF_KEEPMINIACTIVE)
		return FALSE;

	return TRUE;
}

LRESULT CMiniFrameWnd::OnNcHitTest(CPoint point)
{
	DWORD dwStyle = GetStyle();
	CRect rectWindow;
	GetWindowRect(&rectWindow);

	CSize sizeFrame(GetSystemMetrics(SM_CXFRAME),
		GetSystemMetrics(SM_CYFRAME));

	LRESULT nHit = CFrameWnd::OnNcHitTest(point);

	// MFS_BLOCKSYSMENU translates system menu hit to caption hit
	if (dwStyle & MFS_BLOCKSYSMENU)
	{
		if (nHit == HTSYSMENU)
			nHit = HTCAPTION;
		if (GetKeyState(VK_RBUTTON) < 0)
			return HTNOWHERE;
	}

	if ((nHit < HTSIZEFIRST || nHit > HTSIZELAST) && nHit != HTGROWBOX)
		return nHit;

	// MFS_MOVEFRAME translates all size requests to move requests
	if (dwStyle & MFS_MOVEFRAME)
		return HTCAPTION;

	// MFS_4THICKFRAME does not allow diagonal sizing
	rectWindow.InflateRect(-sizeFrame.cx, -sizeFrame.cy);
	if (dwStyle & MFS_4THICKFRAME)
	{
		switch (nHit)
		{
		case HTTOPLEFT:
			return point.y < rectWindow.top ? HTTOP : HTLEFT;
		case HTTOPRIGHT:
			return point.y < rectWindow.top ? HTTOP : HTRIGHT;
		case HTBOTTOMLEFT:
			return point.y > rectWindow.bottom ? HTBOTTOM : HTLEFT;
		case HTGROWBOX:
		case HTBOTTOMRIGHT:
			return point.y > rectWindow.bottom ? HTBOTTOM : HTRIGHT;
		}
	}
	return nHit;    // no special translation
}

void CMiniFrameWnd::OnSysCommand(UINT nID, LPARAM lParam)
{
	DWORD dwStyle = GetStyle();
	if ((dwStyle & WS_POPUP) &&
		((nID & 0xFFF0) != SC_CLOSE ||
		(GetKeyState(VK_F4) < 0 && GetKeyState(VK_MENU) < 0 &&
		(dwStyle & MFS_SYNCACTIVE))))
	{
		if (HandleFloatingSysCommand(nID, lParam))
			return;
	}
	CFrameWnd::OnSysCommand(nID, lParam);
}

void PASCAL CMiniFrameWnd::CalcBorders(
	LPRECT lpClientRect, DWORD dwStyle, DWORD dwExStyle)
{
	UNUSED_ALWAYS(dwExStyle);

	AdjustWindowRectEx(lpClientRect, dwStyle, FALSE, WS_EX_PALETTEWINDOW);
}

LRESULT CMiniFrameWnd::OnFloatStatus(WPARAM wParam, LPARAM)
{
	// these asserts make sure no conflicting actions are requested
	ASSERT(!((wParam & FS_SHOW) && (wParam & FS_HIDE)));
	ASSERT(!((wParam & FS_ENABLE) && (wParam & FS_DISABLE)));
	ASSERT(!((wParam & FS_ACTIVATE) && (wParam & FS_DEACTIVATE)));

	// FS_SYNCACTIVE is used to detect MFS_SYNCACTIVE windows
	LRESULT lResult = 0;
	if ((GetStyle() & MFS_SYNCACTIVE) && (wParam & FS_SYNCACTIVE))
		lResult = 1;

	if (wParam & (FS_SHOW|FS_HIDE))
	{
		SetWindowPos(NULL, 0, 0, 0, 0,
			((wParam & FS_SHOW) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW) | SWP_NOZORDER |
			SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
	}
	if (wParam & (FS_ENABLE|FS_DISABLE))
		EnableWindow((wParam & FS_ENABLE) != 0);

	if ((wParam & (FS_ACTIVATE|FS_DEACTIVATE)) &&
		GetStyle() & MFS_SYNCACTIVE)
	{
		ModifyStyle(MFS_SYNCACTIVE, 0);
		SendMessage(WM_NCACTIVATE, (wParam & FS_ACTIVATE) != 0);
		ModifyStyle(0, MFS_SYNCACTIVE);
	}

	return lResult;
}

LRESULT CMiniFrameWnd::OnQueryCenterWnd(WPARAM, LPARAM)
{
	// forward WM_QUERYCENTERWND to parent window
	HWND hWndParent = ::GetParent(m_hWnd);
	LRESULT lResult = ::SendMessage(hWndParent, WM_QUERYCENTERWND, 0, 0);
	if (lResult == 0)
		lResult = (LRESULT)hWndParent;
	return lResult;
}


IMPLEMENT_DYNCREATE(CMiniFrameWnd, CFrameWnd)

////////////////////////////////////////////////////////////////////////////
