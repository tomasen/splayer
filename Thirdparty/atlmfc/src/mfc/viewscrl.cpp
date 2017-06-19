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



#define AFX_CX_ANCHOR_BITMAP	32
#define AFX_CY_ANCHOR_BITMAP	32

#ifndef SM_MOUSEWHEELPRESENT
#define SM_MOUSEWHEELPRESENT 75
#endif

#ifndef SPI_GETWHEELSCROLLLINES
#define SPI_GETWHEELSCROLLLINES  104
#endif

/////////////////////////////////////////////////////////////////////////////
// CScrollView

BEGIN_MESSAGE_MAP(CScrollView, CView)
	//{{AFX_MSG_MAP(CScrollView)
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_MBUTTONDOWN, &CScrollView::HandleMButtonDown)
END_MESSAGE_MAP()

// Special mapping modes just for CScrollView implementation
#define MM_NONE             0
#define MM_SCALETOFIT       (-1)
	// standard GDI mapping modes are > 0

extern BOOL _afxGotScrollLines; // defined in wincore.cpp

UINT PASCAL _AfxGetMouseScrollLines()
{
	static UINT uCachedScrollLines;

	// if we've already got it and we're not refreshing,
	// return what we've already got

	if (_afxGotScrollLines)
		return uCachedScrollLines;

	// see if we can find the mouse window

	_afxGotScrollLines = TRUE;

	static UINT msgGetScrollLines;
	static WORD nRegisteredMessage;

	// couldn't use the window -- try system settings
	uCachedScrollLines = 3; // reasonable default
	::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uCachedScrollLines, 0);

	return uCachedScrollLines;
}

/////////////////////////////////////////////////////////////////////////////
//

#define ID_TIMER_TRACKING	0xE000

class _AFX_MOUSEANCHORWND : public CWnd
{
private:
   using CWnd::Create;

public:
	_AFX_MOUSEANCHORWND(CPoint& ptAnchor);
	~_AFX_MOUSEANCHORWND();

	BOOL Create(CScrollView* pParent);
	void SetBitmap(UINT nID);

	CRect m_rectDrag;
	CPoint m_ptAnchor;
	BOOL m_bQuitTracking;
	UINT m_nAnchorID;
	HCURSOR m_hAnchorCursor;

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(_AFX_MOUSEANCHORWND, CWnd)
	ON_WM_PAINT()
	ON_WM_TIMER()
END_MESSAGE_MAP()

_AFX_MOUSEANCHORWND::_AFX_MOUSEANCHORWND(CPoint& ptAnchor)
: m_ptAnchor(ptAnchor), m_bQuitTracking(FALSE)
{
}

_AFX_MOUSEANCHORWND::~_AFX_MOUSEANCHORWND()
{
}

BOOL _AFX_MOUSEANCHORWND::PreTranslateMessage(MSG* pMsg)
{
	ENSURE_ARG(pMsg != NULL);
	BOOL bRetVal = FALSE;

	switch (pMsg->message)
	{
	// any of these messages cause us to quit scrolling
	case WM_MOUSEWHEEL:
	case WM_KEYDOWN:
	case WM_CHAR:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
		m_bQuitTracking = TRUE;
		bRetVal = TRUE;
		break;

	// Button up message depend on the position of cursor
	// This enables the user to click and drag for a quick pan.
	case WM_MBUTTONUP:
		{
			CPoint pt(pMsg->lParam);
			ClientToScreen(&pt);
			if (!PtInRect(&m_rectDrag, pt))
				m_bQuitTracking = TRUE;
			bRetVal = TRUE;
		}
		break;
	}

	return bRetVal;
}

void _AFX_MOUSEANCHORWND::OnTimer(UINT_PTR nIDEvent)
{
	ASSERT(nIDEvent == ID_TIMER_TRACKING);
	UNUSED(nIDEvent);

	int nCursor = -1;

	CPoint ptNow;
	GetCursorPos(&ptNow);

	CRect rectClient;
	GetWindowRect(&rectClient);

	// decide where the relative position of the cursor is
	// pick a cursor that points where we're going
	nCursor = 0;

	// if vertical scrolling allowed, consider vertical
	// directions for the cursor we'll show
	if (m_nAnchorID == AFX_IDC_MOUSE_ORG_HV || m_nAnchorID == AFX_IDC_MOUSE_ORG_VERT)
	{
		if (ptNow.y < rectClient.top)
			nCursor = AFX_IDC_MOUSE_PAN_N;
		else if (ptNow.y > rectClient.bottom)
			nCursor = AFX_IDC_MOUSE_PAN_S;
	}

	// if horizontal scrolling allowed, cosider horizontal
	// directions for the cursor, too. Only consider diagonal
	// if we can scroll both ways.
	if (m_nAnchorID == AFX_IDC_MOUSE_ORG_HV || m_nAnchorID == AFX_IDC_MOUSE_ORG_HORZ)
	{
		if (ptNow.x < rectClient.left)
		{
			if (nCursor == 0)
				nCursor = AFX_IDC_MOUSE_PAN_W;
			else if (m_nAnchorID == AFX_IDC_MOUSE_ORG_HV)
				nCursor--;
		}
		else if (ptNow.x > rectClient.right)
		{
			if (nCursor == 0)
				nCursor = AFX_IDC_MOUSE_PAN_E;
			else if (m_nAnchorID == AFX_IDC_MOUSE_ORG_HV)
				nCursor++;
		}
	}

	if (m_bQuitTracking)
	{
		// Someone knows we should stop playing with the mouse
		// kill the timer, quit capturing the mouse, clear the cursor,
		// destroy the window, and make sure that CScrollView knows the
		// window isn't valid anymore.

		KillTimer(ID_TIMER_TRACKING);
		ReleaseCapture();
		SetCursor(NULL);
		CScrollView* pView = (CScrollView*) GetOwner();
		DestroyWindow();
		delete pView->m_pAnchorWindow;
		pView->m_pAnchorWindow = NULL;
	}
	else if (nCursor == 0)
	{
		// The cursor is over the anchor window; use a cursor that
		// looks like the anchor bitmap.
		SetCursor(m_hAnchorCursor);
	}
	else
	{
		// We're still actively tracking, so we need to find a cursor and
		// set it up.

		HINSTANCE hInst = AfxFindResourceHandle(ATL_MAKEINTRESOURCE(nCursor),
			ATL_RT_GROUP_CURSOR);
		HICON hCursor = ::LoadCursor(hInst, ATL_MAKEINTRESOURCE(nCursor));
		ASSERT(hCursor != NULL);
		SetCursor(hCursor);

		// ask the view how much to scroll this time
		CSize sizeDistance; // = ptNow - rectClient.CenterPoint();

		if (ptNow.x > rectClient.right)
			sizeDistance.cx = ptNow.x - rectClient.right;
		else if (ptNow.x < rectClient.left)
			sizeDistance.cx = ptNow.x - rectClient.left;
		else
			sizeDistance.cx = 0;

		if (ptNow.y > rectClient.bottom)
			sizeDistance.cy = ptNow.y - rectClient.bottom;
		else if (ptNow.y < rectClient.top)
			sizeDistance.cy = ptNow.y - rectClient.top;
		else
			sizeDistance.cy = 0;

		CScrollView* pView = (CScrollView*) GetOwner();

		CSize sizeToScroll = pView->GetWheelScrollDistance(sizeDistance,
			m_nAnchorID == AFX_IDC_MOUSE_ORG_HV || m_nAnchorID == AFX_IDC_MOUSE_ORG_HORZ,
			m_nAnchorID == AFX_IDC_MOUSE_ORG_HV || m_nAnchorID == AFX_IDC_MOUSE_ORG_VERT );

		// hide ourselves to minimize flicker
		ShowWindow(SW_HIDE);

		CWnd* pViewParent = pView->GetParent();
		CSplitterWnd* pSplitter = DYNAMIC_DOWNCAST(CSplitterWnd, pViewParent);

		// if it is in a splitter, then we need to handle it accordingly
		if (pSplitter == NULL)
		{
			// scroll the view
			pView->OnScrollBy(sizeToScroll, TRUE);
		}
		else
		{
			// ask the splitter to scroll
			pSplitter->DoScrollBy(pView, sizeToScroll, TRUE);
		}	

		// restore ourselves and repaint
		// SetFocus();
		UpdateWindow();

		// move ourselves back (since we're a child, we scroll too!)
		SetWindowPos(&CWnd::wndTop,
			m_ptAnchor.x - AFX_CX_ANCHOR_BITMAP/2,
			m_ptAnchor.y - AFX_CY_ANCHOR_BITMAP/2, 0, 0,
			SWP_NOACTIVATE | SWP_NOSIZE | SWP_SHOWWINDOW);
	}
}

void _AFX_MOUSEANCHORWND::SetBitmap(UINT nID)
{
	HINSTANCE hInst = AfxFindResourceHandle(ATL_MAKEINTRESOURCE(nID), ATL_RT_GROUP_CURSOR);
	ASSERT(hInst != NULL);
	m_hAnchorCursor = ::LoadCursor(hInst, ATL_MAKEINTRESOURCE(nID));
	m_nAnchorID = nID;
}

BOOL _AFX_MOUSEANCHORWND::Create(CScrollView* pParent)
{
	ENSURE_VALID(pParent);
	ASSERT_KINDOF(CScrollView, pParent);

	pParent->ClientToScreen(&m_ptAnchor);

	m_rectDrag.top = m_ptAnchor.y - GetSystemMetrics(SM_CYDOUBLECLK);
	m_rectDrag.bottom = m_ptAnchor.y + GetSystemMetrics(SM_CYDOUBLECLK);
	m_rectDrag.left = m_ptAnchor.x - GetSystemMetrics(SM_CXDOUBLECLK);
	m_rectDrag.right = m_ptAnchor.x + GetSystemMetrics(SM_CXDOUBLECLK);

	BOOL bRetVal = 
		CreateEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
			AfxRegisterWndClass(CS_SAVEBITS),
			NULL,
			WS_POPUP,
			m_ptAnchor.x - AFX_CX_ANCHOR_BITMAP/2,
			m_ptAnchor.y - AFX_CY_ANCHOR_BITMAP/2,
			AFX_CX_ANCHOR_BITMAP, AFX_CY_ANCHOR_BITMAP,
			NULL, NULL);
	SetOwner(pParent);

	if (bRetVal)
	{
		// set window's region for round effect
		CRgn rgn;
		rgn.CreateEllipticRgn(0, 0, AFX_CX_ANCHOR_BITMAP, AFX_CY_ANCHOR_BITMAP);
		SetWindowRgn(rgn, TRUE);

		// fire timer ever 50ms
		SetCapture();
		SetTimer(ID_TIMER_TRACKING, 50, NULL);
	}
#ifdef _DEBUG
	else
	{
		DWORD dwLastError = GetLastError();
		TRACE(traceAppMsg, 0, "Failed to create mouse anchor window! Last error is 0x%8.8X\n",
			dwLastError);
	}
#endif

	return bRetVal;
}

void _AFX_MOUSEANCHORWND::OnPaint()
{
	CPaintDC dc(this);
	CRect rect;
	GetClientRect(&rect);

	// shrink a pixel in every dimension for border
	rect.DeflateRect(1, 1, 1, 1);
	dc.Ellipse(rect);

	// draw anchor shape
	dc.DrawIcon(0, 0, m_hAnchorCursor);
}

/////////////////////////////////////////////////////////////////////////////
// CScrollView construction/destruction

CScrollView::CScrollView()
{
	// Init everything to zero
	AFX_ZERO_INIT_OBJECT(CView);

	m_pAnchorWindow = NULL;
	m_nMapMode = MM_NONE;
}

CScrollView::~CScrollView()
{
	if (m_pAnchorWindow != NULL)
		m_pAnchorWindow->DestroyWindow();
	delete m_pAnchorWindow;
}

/////////////////////////////////////////////////////////////////////////////
// CScrollView painting

void CScrollView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	ASSERT_VALID(pDC);

#ifdef _DEBUG
	if (m_nMapMode == MM_NONE)
	{
		TRACE(traceAppMsg, 0, "Error: must call SetScrollSizes() or SetScaleToFitSize()");
		TRACE(traceAppMsg, 0, "\tbefore painting scroll view.\n");
		ASSERT(FALSE);
		return;
	}
#endif //_DEBUG
	ASSERT(m_totalDev.cx >= 0 && m_totalDev.cy >= 0);
	switch (m_nMapMode)
	{
	case MM_SCALETOFIT:
		pDC->SetMapMode(MM_ANISOTROPIC);
		pDC->SetWindowExt(m_totalLog);  // window is in logical coordinates
		pDC->SetViewportExt(m_totalDev);
		if (m_totalDev.cx == 0 || m_totalDev.cy == 0)
			TRACE(traceAppMsg, 0, "Warning: CScrollView scaled to nothing.\n");
		break;

	default:
		ASSERT(m_nMapMode > 0);
		pDC->SetMapMode(m_nMapMode);
		break;
	}

	CPoint ptVpOrg(0, 0);       // assume no shift for printing
	if (!pDC->IsPrinting())
	{
		ASSERT(pDC->GetWindowOrg() == CPoint(0,0));

		// by default shift viewport origin in negative direction of scroll
		ptVpOrg = -GetDeviceScrollPosition();

		if (m_bCenter)
		{
			CRect rect;
			GetClientRect(&rect);

			// if client area is larger than total device size,
			// override scroll positions to place origin such that
			// output is centered in the window
			if (m_totalDev.cx < rect.Width())
				ptVpOrg.x = (rect.Width() - m_totalDev.cx) / 2;
			if (m_totalDev.cy < rect.Height())
				ptVpOrg.y = (rect.Height() - m_totalDev.cy) / 2;
		}
	}
	pDC->SetViewportOrg(ptVpOrg);

	CView::OnPrepareDC(pDC, pInfo);     // For default Printing behavior
}

/////////////////////////////////////////////////////////////////////////////
// Set mode and scaling/scrolling sizes

void CScrollView::SetScaleToFitSize(SIZE sizeTotal)
{
	// Note: It is possible to set sizeTotal members to negative values to
	//  effectively invert either the X or Y axis.

	ASSERT(m_hWnd != NULL);
	m_nMapMode = MM_SCALETOFIT;     // special internal value
	m_totalLog = sizeTotal;

	// reset and turn any scroll bars off
	if (m_hWnd != NULL && (GetStyle() & (WS_HSCROLL|WS_VSCROLL)))
	{
		SetScrollPos(SB_HORZ, 0);
		SetScrollPos(SB_VERT, 0);
		EnableScrollBarCtrl(SB_BOTH, FALSE);
		ASSERT((GetStyle() & (WS_HSCROLL|WS_VSCROLL)) == 0);
	}

	CRect rectT;
	GetClientRect(rectT);
	m_totalDev = rectT.Size();

	if (m_hWnd != NULL)
	{
		// window has been created, invalidate
		UpdateBars();
		Invalidate(TRUE);
	}
}

const AFX_DATADEF SIZE CScrollView::sizeDefault = {0,0};

void CScrollView::SetScrollSizes(int nMapMode, SIZE sizeTotal,
	const SIZE& sizePage, const SIZE& sizeLine)
{
	ASSERT(sizeTotal.cx >= 0 && sizeTotal.cy >= 0);
	ASSERT(nMapMode > 0);
	ASSERT(nMapMode != MM_ISOTROPIC && nMapMode != MM_ANISOTROPIC);

	int nOldMapMode = m_nMapMode;
	m_nMapMode = nMapMode;
	m_totalLog = sizeTotal;

	//BLOCK: convert logical coordinate space to device coordinates
	{
		CWindowDC dc(NULL);
		ASSERT(m_nMapMode > 0);
		dc.SetMapMode(m_nMapMode);

		// total size
		m_totalDev = m_totalLog;
		dc.LPtoDP((LPPOINT)&m_totalDev);
		m_pageDev = sizePage;
		dc.LPtoDP((LPPOINT)&m_pageDev);
		m_lineDev = sizeLine;
		dc.LPtoDP((LPPOINT)&m_lineDev);
		if (m_totalDev.cy < 0)
			m_totalDev.cy = -m_totalDev.cy;
		if (m_pageDev.cy < 0)
			m_pageDev.cy = -m_pageDev.cy;
		if (m_lineDev.cy < 0)
			m_lineDev.cy = -m_lineDev.cy;
	} // release DC here

	// now adjust device specific sizes
	ASSERT(m_totalDev.cx >= 0 && m_totalDev.cy >= 0);
	if (m_pageDev.cx == 0)
		m_pageDev.cx = m_totalDev.cx / 10;
	if (m_pageDev.cy == 0)
		m_pageDev.cy = m_totalDev.cy / 10;
	if (m_lineDev.cx == 0)
		m_lineDev.cx = m_pageDev.cx / 10;
	if (m_lineDev.cy == 0)
		m_lineDev.cy = m_pageDev.cy / 10;

	if (m_hWnd != NULL)
	{
		// window has been created, invalidate now
		UpdateBars();
		if (nOldMapMode != m_nMapMode)
			Invalidate(TRUE);
	}
}

/////////////////////////////////////////////////////////////////////////////
// Getting information

CPoint CScrollView::GetScrollPosition() const   // logical coordinates
{
	if (m_nMapMode == MM_SCALETOFIT)
	{
		return CPoint(0, 0);    // must be 0,0
	}

	CPoint pt = GetDeviceScrollPosition();

	if (m_bCenter)
	{
		CRect rect;
		GetClientRect(&rect);

		// if client area is larger than total device size,
		// the scroll positions are overridden to place origin such that
		// output is centered in the window
		// GetDeviceScrollPosition() must reflect this

		if (m_totalDev.cx < rect.Width())
			pt.x = -((rect.Width() - m_totalDev.cx) / 2);
		if (m_totalDev.cy < rect.Height())
			pt.y = -((rect.Height() - m_totalDev.cy) / 2);

	}
	// pt may be negative if m_bCenter is set

	if (m_nMapMode != MM_TEXT)
	{
		ASSERT(m_nMapMode > 0); // must be set
		CWindowDC dc(NULL);
		dc.SetMapMode(m_nMapMode);
		dc.DPtoLP((LPPOINT)&pt);
	}
	return pt;
}

void CScrollView::ScrollToPosition(POINT pt)    // logical coordinates
{
	ASSERT(m_nMapMode > 0);     // not allowed for shrink to fit
	if (m_nMapMode != MM_TEXT)
	{
		CWindowDC dc(NULL);
		dc.SetMapMode(m_nMapMode);
		dc.LPtoDP((LPPOINT)&pt);
	}

	// now in device coordinates - limit if out of range
	int xMax = GetScrollLimit(SB_HORZ);
	int yMax = GetScrollLimit(SB_VERT);
	if (pt.x < 0)
		pt.x = 0;
	else if (pt.x > xMax)
		pt.x = xMax;
	if (pt.y < 0)
		pt.y = 0;
	else if (pt.y > yMax)
		pt.y = yMax;

	ScrollToDevicePosition(pt);
}

CPoint CScrollView::GetDeviceScrollPosition() const
{
	CPoint pt(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));
	ASSERT(pt.x >= 0 && pt.y >= 0);
	return pt;
}

void CScrollView::GetDeviceScrollSizes(int& nMapMode, SIZE& sizeTotal,
			SIZE& sizePage, SIZE& sizeLine) const
{
	if (m_nMapMode <= 0)
		TRACE(traceAppMsg, 0, "Warning: CScrollView::GetDeviceScrollSizes returning invalid mapping mode.\n");
	nMapMode = m_nMapMode;
	sizeTotal = m_totalDev;
	sizePage = m_pageDev;
	sizeLine = m_lineDev;
}

void CScrollView::ScrollToDevicePosition(POINT ptDev)
{
	ASSERT(ptDev.x >= 0);
	ASSERT(ptDev.y >= 0);

	// Note: ScrollToDevicePosition can and is used to scroll out-of-range
	//  areas as far as CScrollView is concerned -- specifically in
	//  the print-preview code.  Since OnScrollBy makes sure the range is
	//  valid, ScrollToDevicePosition does not vector through OnScrollBy.

	int xOrig = GetScrollPos(SB_HORZ);
	SetScrollPos(SB_HORZ, ptDev.x);
	int yOrig = GetScrollPos(SB_VERT);
	SetScrollPos(SB_VERT, ptDev.y);
	ScrollWindow(xOrig - ptDev.x, yOrig - ptDev.y);
}

CSize CScrollView::GetWheelScrollDistance(CSize sizeDistance, BOOL bHorz, BOOL bVert)
{
	CSize sizeRet;

	if (bHorz)
		sizeRet.cx = sizeDistance.cx / 10;
	else
		sizeRet.cx = 0;

	if (bVert)
		sizeRet.cy = sizeDistance.cy / 10;
	else
		sizeRet.cy = 0;

	return sizeRet;
}

/////////////////////////////////////////////////////////////////////////////
// Other helpers

void CScrollView::FillOutsideRect(CDC* pDC, CBrush* pBrush)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pBrush);

	// fill rect outside the image
	CRect rect;
	GetClientRect(rect);
	ASSERT(rect.left == 0 && rect.top == 0);
	rect.left = m_totalDev.cx;
	if (!rect.IsRectEmpty())
		pDC->FillRect(rect, pBrush);    // vertical strip along the side
	rect.left = 0;
	rect.right = m_totalDev.cx;
	rect.top = m_totalDev.cy;
	if (!rect.IsRectEmpty())
		pDC->FillRect(rect, pBrush);    // horizontal strip along the bottom
}

void CScrollView::ResizeParentToFit(BOOL bShrinkOnly)
{
	// adjust parent rect so client rect is appropriate size
	ASSERT(m_nMapMode != MM_NONE);  // mapping mode must be known

	// determine current size of the client area as if no scrollbars present
	CRect rectClient;
	GetWindowRect(rectClient);
	CRect rect = rectClient;
	CalcWindowRect(rect);
	rectClient.left += rectClient.left - rect.left;
	rectClient.top += rectClient.top - rect.top;
	rectClient.right -= rect.right - rectClient.right;
	rectClient.bottom -= rect.bottom - rectClient.bottom;
	rectClient.OffsetRect(-rectClient.left, -rectClient.top);
	ASSERT(rectClient.left == 0 && rectClient.top == 0);

	// determine desired size of the view
	CRect rectView(0, 0, m_totalDev.cx, m_totalDev.cy);
	if (bShrinkOnly)
	{
		if (rectClient.right <= m_totalDev.cx)
			rectView.right = rectClient.right;
		if (rectClient.bottom <= m_totalDev.cy)
			rectView.bottom = rectClient.bottom;
	}
	CalcWindowRect(rectView, CWnd::adjustOutside);
	rectView.OffsetRect(-rectView.left, -rectView.top);
	ASSERT(rectView.left == 0 && rectView.top == 0);
	if (bShrinkOnly)
	{
		if (rectClient.right <= m_totalDev.cx)
			rectView.right = rectClient.right;
		if (rectClient.bottom <= m_totalDev.cy)
			rectView.bottom = rectClient.bottom;
	}

	// dermine and set size of frame based on desired size of view
	CRect rectFrame;
	CFrameWnd* pFrame = EnsureParentFrame();
	pFrame->GetWindowRect(rectFrame);
	CSize size = rectFrame.Size();
	size.cx += rectView.right - rectClient.right;
	size.cy += rectView.bottom - rectClient.bottom;
	pFrame->SetWindowPos(NULL, 0, 0, size.cx, size.cy,
		SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

/////////////////////////////////////////////////////////////////////////////

void CScrollView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	if (m_nMapMode == MM_SCALETOFIT)
	{
		// force recalculation of scale to fit parameters
		SetScaleToFitSize(m_totalLog);
	}
	else
	{
		// UpdateBars() handles locking out recursion
		UpdateBars();
	}
}

LRESULT CScrollView::HandleMButtonDown(WPARAM wParam, LPARAM lParam)
{
	UINT nFlags = static_cast<UINT>(wParam);
	CPoint point(lParam);

	// if the user has CTRL or SHIFT down, we certainly
	// do not handle the message

	if (nFlags & (MK_SHIFT | MK_CONTROL))
	{
		CView::OnMButtonDown(nFlags, point);
		return FALSE;
	}

	// Otherwise, we actually have scrolling work to do.
	// See if we have a wheel mouse...

	BOOL bSupport = ::GetSystemMetrics(SM_MOUSEWHEELPRESENT);

	// If not a wheel mouse, the middle button is really the
	// middle button and not the wheel; the application should've
	//	handled it.

	if (!bSupport)
		CView::OnMButtonDown(nFlags, point);
	else
	{
		if (m_pAnchorWindow == NULL)
		{
			BOOL bVertBar;
			BOOL bHorzBar;
			CheckScrollBars(bHorzBar, bVertBar);

			UINT nBitmapID = 0;

			// based on which scrollbars we have, figure out which
			// anchor cursor to use for the anchor window
			if (bVertBar)
			{
				if (bHorzBar)
					nBitmapID = AFX_IDC_MOUSE_ORG_HV;
				else
					nBitmapID = AFX_IDC_MOUSE_ORG_VERT;
			}
			else if (bHorzBar)
				nBitmapID = AFX_IDC_MOUSE_ORG_HORZ;

			// if there's no scrollbars, we don't do anything!
			if (nBitmapID == 0)
			{
				CView::OnMButtonDown(nFlags, point);
				return FALSE;
			}
			else
			{
				m_pAnchorWindow = new _AFX_MOUSEANCHORWND(point);
				m_pAnchorWindow->SetBitmap(nBitmapID);
				m_pAnchorWindow->Create(this);
				m_pAnchorWindow->ShowWindow(SW_SHOWNA);
			}
		}
		else
		{
			m_pAnchorWindow->DestroyWindow();
			delete m_pAnchorWindow;
			m_pAnchorWindow = NULL;
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Scrolling Helpers

void CScrollView::CenterOnPoint(CPoint ptCenter) // center in device coords
{
	CRect rect;
	GetClientRect(&rect);           // find size of client window

	int xDesired = ptCenter.x - rect.Width() / 2;
	int yDesired = ptCenter.y - rect.Height() / 2;

	DWORD dwStyle = GetStyle();

	if ((dwStyle & WS_HSCROLL) == 0 || xDesired < 0)
	{
		xDesired = 0;
	}
	else
	{
		int xMax = GetScrollLimit(SB_HORZ);
		if (xDesired > xMax)
			xDesired = xMax;
	}

	if ((dwStyle & WS_VSCROLL) == 0 || yDesired < 0)
	{
		yDesired = 0;
	}
	else
	{
		int yMax = GetScrollLimit(SB_VERT);
		if (yDesired > yMax)
			yDesired = yMax;
	}

	ASSERT(xDesired >= 0);
	ASSERT(yDesired >= 0);

	SetScrollPos(SB_HORZ, xDesired);
	SetScrollPos(SB_VERT, yDesired);
}

/////////////////////////////////////////////////////////////////////////////
// Tie to scrollbars and CWnd behaviour

void CScrollView::GetScrollBarSizes(CSize& sizeSb)
{
	sizeSb.cx = sizeSb.cy = 0;
	DWORD dwStyle = GetStyle();

	if (GetScrollBarCtrl(SB_VERT) == NULL)
	{
		// vert scrollbars will impact client area of this window
		sizeSb.cx = afxData.cxVScroll;
		if (dwStyle & WS_BORDER)
			sizeSb.cx -= AFX_CX_BORDER;
	}
	if (GetScrollBarCtrl(SB_HORZ) == NULL)
	{
		// horz scrollbars will impact client area of this window
		sizeSb.cy = afxData.cyHScroll;
		if (dwStyle & WS_BORDER)
			sizeSb.cy -= AFX_CY_BORDER;
	}
}

BOOL CScrollView::GetTrueClientSize(CSize& size, CSize& sizeSb)
	// return TRUE if enough room to add scrollbars if needed
{
	CRect rect;
	GetClientRect(&rect);
	ASSERT(rect.top == 0 && rect.left == 0);
	size.cx = rect.right;
	size.cy = rect.bottom;
	DWORD dwStyle = GetStyle();

	// first get the size of the scrollbars for this window
	GetScrollBarSizes(sizeSb);

	// first calculate the size of a potential scrollbar
	// (scroll bar controls do not get turned on/off)
	if (sizeSb.cx != 0 && (dwStyle & WS_VSCROLL))
	{
		// vert scrollbars will impact client area of this window
		size.cx += sizeSb.cx;   // currently on - adjust now
	}
	if (sizeSb.cy != 0 && (dwStyle & WS_HSCROLL))
	{
		// horz scrollbars will impact client area of this window
		size.cy += sizeSb.cy;   // currently on - adjust now
	}

	// return TRUE if enough room
	return (size.cx > sizeSb.cx && size.cy > sizeSb.cy);
}

// helper to return the state of the scrollbars without actually changing
//  the state of the scrollbars
void CScrollView::GetScrollBarState(CSize sizeClient, CSize& needSb,
	CSize& sizeRange, CPoint& ptMove, BOOL bInsideClient)
{
	// get scroll bar sizes (the part that is in the client area)
	CSize sizeSb;
	GetScrollBarSizes(sizeSb);

	// enough room to add scrollbars
	sizeRange = m_totalDev - sizeClient;
		// > 0 => need to scroll
	ptMove = GetDeviceScrollPosition();
		// point to move to (start at current scroll pos)

	BOOL bNeedH = sizeRange.cx > 0;
	if (!bNeedH)
		ptMove.x = 0;                       // jump back to origin
	else if (bInsideClient)
		sizeRange.cy += sizeSb.cy;          // need room for a scroll bar

	BOOL bNeedV = sizeRange.cy > 0;
	if (!bNeedV)
		ptMove.y = 0;                       // jump back to origin
	else if (bInsideClient)
		sizeRange.cx += sizeSb.cx;          // need room for a scroll bar

	if (bNeedV && !bNeedH && sizeRange.cx > 0)
	{
		ASSERT(bInsideClient);
		// need a horizontal scrollbar after all
		bNeedH = TRUE;
		sizeRange.cy += sizeSb.cy;
	}

	// if current scroll position will be past the limit, scroll to limit
	if (sizeRange.cx > 0 && ptMove.x >= sizeRange.cx)
		ptMove.x = sizeRange.cx;
	if (sizeRange.cy > 0 && ptMove.y >= sizeRange.cy)
		ptMove.y = sizeRange.cy;

	// now update the bars as appropriate
	needSb.cx = bNeedH;
	needSb.cy = bNeedV;

	// needSb, sizeRange, and ptMove area now all updated
}

void CScrollView::UpdateBars()
{
	// UpdateBars may cause window to be resized - ignore those resizings
	if (m_bInsideUpdate)
		return;         // Do not allow recursive calls

	// Lock out recursion
	m_bInsideUpdate = TRUE;

	// update the horizontal to reflect reality
	// NOTE: turning on/off the scrollbars will cause 'OnSize' callbacks
	ASSERT(m_totalDev.cx >= 0 && m_totalDev.cy >= 0);

	CRect rectClient;
	BOOL bCalcClient = TRUE;

	// allow parent to do inside-out layout first
	CWnd* pParentWnd = GetParent();
	if (pParentWnd != NULL)
	{
		// if parent window responds to this message, use just
		//  client area for scroll bar calc -- not "true" client area
		if ((BOOL)pParentWnd->SendMessage(WM_RECALCPARENT, 0,
			(LPARAM)(LPCRECT)&rectClient) != 0)
		{
			// use rectClient instead of GetTrueClientSize for
			//  client size calculation.
			bCalcClient = FALSE;
		}
	}

	CSize sizeClient;
	CSize sizeSb;

	if (bCalcClient)
	{
		// get client rect
		if (!GetTrueClientSize(sizeClient, sizeSb))
		{
			// no room for scroll bars (common for zero sized elements)
			CRect rect;
			GetClientRect(&rect);
			if (rect.right > 0 && rect.bottom > 0)
			{
				// if entire client area is not invisible, assume we have
				//  control over our scrollbars
				EnableScrollBarCtrl(SB_BOTH, FALSE);
			}
			m_bInsideUpdate = FALSE;
			return;
		}
	}
	else
	{
		// let parent window determine the "client" rect
		GetScrollBarSizes(sizeSb);
		sizeClient.cx = rectClient.right - rectClient.left;
		sizeClient.cy = rectClient.bottom - rectClient.top;
	}

	// enough room to add scrollbars
	CSize sizeRange;
	CPoint ptMove;
	CSize needSb;

	// get the current scroll bar state given the true client area
	GetScrollBarState(sizeClient, needSb, sizeRange, ptMove, bCalcClient);
	if (needSb.cx)
		sizeClient.cy -= sizeSb.cy;
	if (needSb.cy)
		sizeClient.cx -= sizeSb.cx;

	// first scroll the window as needed
	ScrollToDevicePosition(ptMove); // will set the scroll bar positions too

	// this structure needed to update the scrollbar page range
	SCROLLINFO info;
	info.fMask = SIF_PAGE|SIF_RANGE;
	info.nMin = 0;

	// now update the bars as appropriate
	EnableScrollBarCtrl(SB_HORZ, needSb.cx);
	if (needSb.cx)
	{
		info.nPage = sizeClient.cx;
		info.nMax = m_totalDev.cx-1;
		if (!SetScrollInfo(SB_HORZ, &info, TRUE))
			SetScrollRange(SB_HORZ, 0, sizeRange.cx, TRUE);
	}
	EnableScrollBarCtrl(SB_VERT, needSb.cy);
	if (needSb.cy)
	{
		info.nPage = sizeClient.cy;
		info.nMax = m_totalDev.cy-1;
		if (!SetScrollInfo(SB_VERT, &info, TRUE))
			SetScrollRange(SB_VERT, 0, sizeRange.cy, TRUE);
	}

	// remove recursion lockout
	m_bInsideUpdate = FALSE;
}

void CScrollView::CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType)
{
	if (nAdjustType == adjustOutside)
	{
		// allow for special client-edge style
		::AdjustWindowRectEx(lpClientRect, 0, FALSE, GetExStyle());

		if (m_nMapMode != MM_SCALETOFIT)
		{
			// if the view is being used in-place, add scrollbar sizes
			//  (scollbars should appear on the outside when in-place editing)
			CSize sizeClient(
				lpClientRect->right - lpClientRect->left,
				lpClientRect->bottom - lpClientRect->top);

			CSize sizeRange = m_totalDev - sizeClient;
				// > 0 => need to scroll

			// get scroll bar sizes (used to adjust the window)
			CSize sizeSb;
			GetScrollBarSizes(sizeSb);

			// adjust the window size based on the state
			if (sizeRange.cy > 0)
			{   // vertical scroll bars take up horizontal space
				lpClientRect->right += sizeSb.cx;
			}
			if (sizeRange.cx > 0)
			{   // horizontal scroll bars take up vertical space
				lpClientRect->bottom += sizeSb.cy;
			}
		}
	}
	else
	{
		// call default to handle other non-client areas
		::AdjustWindowRectEx(lpClientRect, GetStyle(), FALSE,
			GetExStyle() & ~(WS_EX_CLIENTEDGE));
	}
}

/////////////////////////////////////////////////////////////////////////////
// CScrollView scrolling

void CScrollView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar != NULL && pScrollBar->SendChildNotifyLastMsg())
		return;     // eat it

	// ignore scroll bar msgs from other controls
	if (pScrollBar != GetScrollBarCtrl(SB_HORZ))
		return;

	OnScroll(MAKEWORD(nSBCode, 0xff), nPos);
}

void CScrollView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar != NULL && pScrollBar->SendChildNotifyLastMsg())
		return;     // eat it

	// ignore scroll bar msgs from other controls
	if (pScrollBar != GetScrollBarCtrl(SB_VERT))
		return;

	OnScroll(MAKEWORD(0xff, nSBCode), nPos);
}

BOOL CScrollView::OnMouseWheel(UINT fFlags, short zDelta, CPoint point)
{
	// we don't handle anything but scrolling
	if (fFlags & (MK_SHIFT | MK_CONTROL))
		return FALSE;

	// if the parent is a splitter, it will handle the message
	if (GetParentSplitter(this, TRUE))
		return FALSE;

	// we can't get out of it--perform the scroll ourselves
	return DoMouseWheel(fFlags, zDelta, point);
}

void CScrollView::CheckScrollBars(BOOL& bHasHorzBar, BOOL& bHasVertBar) const
{
	DWORD dwStyle = GetStyle();
	CScrollBar* pBar = GetScrollBarCtrl(SB_VERT);
	bHasVertBar = ((pBar != NULL) && pBar->IsWindowEnabled()) ||
					(dwStyle & WS_VSCROLL);

	pBar = GetScrollBarCtrl(SB_HORZ);
	bHasHorzBar = ((pBar != NULL) && pBar->IsWindowEnabled()) ||
					(dwStyle & WS_HSCROLL);
}

// This function isn't virtual. If you need to override it,
// you really need to override OnMouseWheel() here or in
// CSplitterWnd.

BOOL CScrollView::DoMouseWheel(UINT fFlags, short zDelta, CPoint point)
{
	UNUSED_ALWAYS(point);
	UNUSED_ALWAYS(fFlags);

	// if we have a vertical scroll bar, the wheel scrolls that
	// if we have _only_ a horizontal scroll bar, the wheel scrolls that
	// otherwise, don't do any work at all

	BOOL bHasHorzBar, bHasVertBar;
	CheckScrollBars(bHasHorzBar, bHasVertBar);
	if (!bHasVertBar && !bHasHorzBar)
		return FALSE;

	BOOL bResult = FALSE;
	UINT uWheelScrollLines = _AfxGetMouseScrollLines();
	int nToScroll = ::MulDiv(-zDelta, uWheelScrollLines, WHEEL_DELTA);
	int nDisplacement;

	if (bHasVertBar)
	{
		if (uWheelScrollLines == WHEEL_PAGESCROLL)
		{
			nDisplacement = m_pageDev.cy;
			if (zDelta > 0)
				nDisplacement = -nDisplacement;
		}
		else
		{
			nDisplacement = nToScroll * m_lineDev.cy;
			nDisplacement = min(nDisplacement, m_pageDev.cy);
		}
		bResult = OnScrollBy(CSize(0, nDisplacement), TRUE);
	}
	else if (bHasHorzBar)
	{
		if (uWheelScrollLines == WHEEL_PAGESCROLL)
		{
			nDisplacement = m_pageDev.cx;
			if (zDelta > 0)
				nDisplacement = -nDisplacement;
		}
		else
		{
			nDisplacement = nToScroll * m_lineDev.cx;
			nDisplacement = min(nDisplacement, m_pageDev.cx);
		}
		bResult = OnScrollBy(CSize(nDisplacement, 0), TRUE);
	}

	if (bResult)
		UpdateWindow();

	return bResult;
}

BOOL CScrollView::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll)
{
	// calc new x position
	int x = GetScrollPos(SB_HORZ);
	int xOrig = x;

	switch (LOBYTE(nScrollCode))
	{
	case SB_TOP:
		x = 0;
		break;
	case SB_BOTTOM:
		x = INT_MAX;
		break;
	case SB_LINEUP:
		x -= m_lineDev.cx;
		break;
	case SB_LINEDOWN:
		x += m_lineDev.cx;
		break;
	case SB_PAGEUP:
		x -= m_pageDev.cx;
		break;
	case SB_PAGEDOWN:
		x += m_pageDev.cx;
		break;
	case SB_THUMBTRACK:
		x = nPos;
		break;
	}

	// calc new y position
	int y = GetScrollPos(SB_VERT);
	int yOrig = y;

	switch (HIBYTE(nScrollCode))
	{
	case SB_TOP:
		y = 0;
		break;
	case SB_BOTTOM:
		y = INT_MAX;
		break;
	case SB_LINEUP:
		y -= m_lineDev.cy;
		break;
	case SB_LINEDOWN:
		y += m_lineDev.cy;
		break;
	case SB_PAGEUP:
		y -= m_pageDev.cy;
		break;
	case SB_PAGEDOWN:
		y += m_pageDev.cy;
		break;
	case SB_THUMBTRACK:
		y = nPos;
		break;
	}

	BOOL bResult = OnScrollBy(CSize(x - xOrig, y - yOrig), bDoScroll);
	if (bResult && bDoScroll)
		UpdateWindow();

	return bResult;
}

BOOL CScrollView::OnScrollBy(CSize sizeScroll, BOOL bDoScroll)
{
	int xOrig, x;
	int yOrig, y;

	// don't scroll if there is no valid scroll range (ie. no scroll bar)
	CScrollBar* pBar;
	DWORD dwStyle = GetStyle();
	pBar = GetScrollBarCtrl(SB_VERT);
	if ((pBar != NULL && !pBar->IsWindowEnabled()) ||
		(pBar == NULL && !(dwStyle & WS_VSCROLL)))
	{
		// vertical scroll bar not enabled
		sizeScroll.cy = 0;
	}
	pBar = GetScrollBarCtrl(SB_HORZ);
	if ((pBar != NULL && !pBar->IsWindowEnabled()) ||
		(pBar == NULL && !(dwStyle & WS_HSCROLL)))
	{
		// horizontal scroll bar not enabled
		sizeScroll.cx = 0;
	}

	// adjust current x position
	xOrig = x = GetScrollPos(SB_HORZ);
	int xMax = GetScrollLimit(SB_HORZ);
	x += sizeScroll.cx;
	if (x < 0)
		x = 0;
	else if (x > xMax)
		x = xMax;

	// adjust current y position
	yOrig = y = GetScrollPos(SB_VERT);
	int yMax = GetScrollLimit(SB_VERT);
	y += sizeScroll.cy;
	if (y < 0)
		y = 0;
	else if (y > yMax)
		y = yMax;

	// did anything change?
	if (x == xOrig && y == yOrig)
		return FALSE;

	if (bDoScroll)
	{
		// do scroll and update scroll positions
		ScrollWindow(-(x-xOrig), -(y-yOrig));
		if (x != xOrig)
			SetScrollPos(SB_HORZ, x);
		if (y != yOrig)
			SetScrollPos(SB_VERT, y);
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CScrollView diagnostics

#ifdef _DEBUG
void CScrollView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);

	dc << "m_totalLog = " << m_totalLog;
	dc << "\nm_totalDev = " << m_totalDev;
	dc << "\nm_pageDev = " << m_pageDev;
	dc << "\nm_lineDev = " << m_lineDev;
	dc << "\nm_bCenter = " << m_bCenter;
	dc << "\nm_bInsideUpdate = " << m_bInsideUpdate;
	dc << "\nm_nMapMode = ";
	switch (m_nMapMode)
	{
	case MM_NONE:
		dc << "MM_NONE";
		break;
	case MM_SCALETOFIT:
		dc << "MM_SCALETOFIT";
		break;
	case MM_TEXT:
		dc << "MM_TEXT";
		break;
	case MM_LOMETRIC:
		dc << "MM_LOMETRIC";
		break;
	case MM_HIMETRIC:
		dc << "MM_HIMETRIC";
		break;
	case MM_LOENGLISH:
		dc << "MM_LOENGLISH";
		break;
	case MM_HIENGLISH:
		dc << "MM_HIENGLISH";
		break;
	case MM_TWIPS:
		dc << "MM_TWIPS";
		break;
	default:
		dc << "*unknown*";
		break;
	}

	dc << "\n";
}

void CScrollView::AssertValid() const
{
	CView::AssertValid();

	switch (m_nMapMode)
	{
	case MM_NONE:
	case MM_SCALETOFIT:
	case MM_TEXT:
	case MM_LOMETRIC:
	case MM_HIMETRIC:
	case MM_LOENGLISH:
	case MM_HIENGLISH:
	case MM_TWIPS:
		break;
	case MM_ISOTROPIC:
	case MM_ANISOTROPIC:
		ASSERT(FALSE); // illegal mapping mode
	default:
		ASSERT(FALSE); // unknown mapping mode
	}
}
#endif //_DEBUG


IMPLEMENT_DYNAMIC(CScrollView, CView)

/////////////////////////////////////////////////////////////////////////////
