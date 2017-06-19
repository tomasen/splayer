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
#include "afxrebar.h"

HWND ChWindowFromPoint(HWND hWnd, POINT pt);

#pragma warning(disable : 4355)

/////////////////////////////////////////////////////////////////////////////
// CReBar

//{{AFX_MSG_MAP(CMFCReBar)
BEGIN_MESSAGE_MAP(CMFCReBar, CPane)
	ON_WM_NCCREATE()
	ON_WM_PAINT()
	ON_WM_NCCALCSIZE()
	ON_WM_ERASEBKGND()
	ON_WM_NCPAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(RB_SHOWBAND, &CMFCReBar::OnShowBand)
	ON_MESSAGE_VOID(WM_RECALCPARENT, CMFCReBar::OnRecalcParent)
	ON_NOTIFY_REFLECT(RBN_HEIGHTCHANGE, &CMFCReBar::OnHeightChange)
	ON_NOTIFY_REFLECT(RBN_ENDDRAG, &CMFCReBar::OnHeightChange)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

CMFCReBar::CMFCReBar() : m_Impl(this)
{
	SetBorders();

	if (_AfxGetComCtlVersion() < MAKELONG(1, 6))
	{
		// For 6.0 common controls, the call to RB_INSERTBAND will fail
		// if the 6.1 size is passed in. So, the old size must be used 
		// instead.
		m_nReBarBandInfoSize = sizeof(AFX_OLDREBARBANDINFO);
	}
	else
	{
		m_nReBarBandInfoSize = sizeof(REBARBANDINFO);
	}
}

void CMFCReBar::OnRecalcParent()
{
	CFrameWnd* pFrameWnd = AFXGetParentFrame(this);
	ENSURE(pFrameWnd != NULL);
	pFrameWnd->RecalcLayout();
}

void CMFCReBar::OnHeightChange(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	AdjustDockingLayout();
	*pResult = 0;
}

LRESULT CMFCReBar::OnShowBand(WPARAM wParam, LPARAM)
{
	LRESULT lResult = Default();
	if (lResult)
	{
		// keep window visible state in sync with band visible state
		REBARBANDINFO rbBand;
		rbBand.cbSize = m_nReBarBandInfoSize;
		rbBand.fMask = RBBIM_CHILD|RBBIM_STYLE;
		VERIFY(DefWindowProc(RB_GETBANDINFO, wParam, (LPARAM)&rbBand));
		CPane* pBar = DYNAMIC_DOWNCAST(CPane, CWnd::FromHandlePermanent(rbBand.hwndChild));
		BOOL bWindowVisible;
		if (pBar != NULL)
			bWindowVisible = pBar->IsVisible();
		else
			bWindowVisible = (::GetWindowLong(rbBand.hwndChild, GWL_STYLE) & WS_VISIBLE) != 0;
		BOOL bBandVisible = (rbBand.fStyle & RBBS_HIDDEN) == 0;
		if (bWindowVisible != bBandVisible)
			VERIFY(::ShowWindow(rbBand.hwndChild, bBandVisible ? SW_SHOW : SW_HIDE));
	}
	return lResult;
}

BOOL CMFCReBar::_AddMFCToolBar(CWnd* pBar, REBARBANDINFO* pRBBI)
{
	ASSERT_VALID(this);
	ENSURE(::IsWindow(m_hWnd));
	ENSURE(pBar != NULL);
	ENSURE(::IsWindow(pBar->m_hWnd));

	pRBBI->cbSize = m_nReBarBandInfoSize;
	pRBBI->fMask |= RBBIM_CHILD | RBBIM_CHILDSIZE;
	pRBBI->hwndChild = pBar->m_hWnd;

	CSize size;
	CPane* pTemp = DYNAMIC_DOWNCAST(CPane, pBar);
	if (pTemp != NULL)
	{
		size = pTemp->CalcFixedLayout(FALSE, m_dwStyle & CBRS_ORIENT_HORZ);
	}
	else
	{
		CRect rect;
		pBar->GetWindowRect(&rect);
		size = rect.Size();
	}
	//WINBUG: COMCTL32.DLL is off by 4 pixels in its sizing logic.  Whatever
	//  is specified as the minimum size, the system rebar will allow that band
	//  to be 4 actual pixels smaller!  That's why we add 4 to the size here.

	pRBBI->cxMinChild = size.cx;
	pRBBI->cyMinChild = size.cy;
	BOOL bResult = (BOOL)DefWindowProc(RB_INSERTBAND, (WPARAM)-1, (LPARAM)pRBBI);

	CFrameWnd* pFrameWnd = AFXGetParentFrame(this);
	if (pFrameWnd != NULL)
		pFrameWnd->RecalcLayout();

	return bResult;
}

BOOL CMFCReBar::AddBar(CWnd* pBar, LPCTSTR pszText, CBitmap* pbmp, DWORD dwStyle)
{
	REBARBANDINFO rbBand;
	rbBand.fMask = RBBIM_STYLE;
	rbBand.fStyle = dwStyle;
	if (pszText != NULL)
	{
		rbBand.fMask |= RBBIM_TEXT;
		rbBand.lpText = const_cast<LPTSTR>(pszText);
	}
	if (pbmp != NULL)
	{
		rbBand.fMask |= RBBIM_BACKGROUND;
		rbBand.hbmBack = (HBITMAP)*pbmp;
	}
	return _AddMFCToolBar(pBar, &rbBand);
}

BOOL CMFCReBar::AddBar(CWnd* pBar, COLORREF clrFore, COLORREF clrBack, LPCTSTR pszText, DWORD dwStyle)
{
	REBARBANDINFO rbBand;
	rbBand.fMask = RBBIM_STYLE | RBBIM_COLORS;
	rbBand.fStyle = dwStyle;
	rbBand.clrFore = clrFore;
	rbBand.clrBack = clrBack;
	if (pszText != NULL)
	{
		rbBand.fMask |= RBBIM_TEXT;
		rbBand.lpText = const_cast<LPTSTR>(pszText);
	}
	return _AddMFCToolBar(pBar, &rbBand);
}

CSize CMFCReBar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	ASSERT_VALID(this);
	ENSURE(::IsWindow(m_hWnd));

	// the union of the band rectangles is the total bounding rect
	int nCount = (int) DefWindowProc(RB_GETBANDCOUNT, 0, 0);
	REBARBANDINFO rbBand;
	rbBand.cbSize = m_nReBarBandInfoSize;
	int nTemp;

	// sync up hidden state of the bands
	for (nTemp = nCount; nTemp--; )
	{
		rbBand.fMask = RBBIM_CHILD|RBBIM_STYLE;
		VERIFY(DefWindowProc(RB_GETBANDINFO, nTemp, (LPARAM)&rbBand));
		CPane* pBar = DYNAMIC_DOWNCAST(CPane, CWnd::FromHandlePermanent(rbBand.hwndChild));
		BOOL bWindowVisible;
		if (pBar != NULL)
			bWindowVisible = pBar->IsVisible();
		else
			bWindowVisible = (::GetWindowLong(rbBand.hwndChild, GWL_STYLE) & WS_VISIBLE) != 0;
		BOOL bBandVisible = (rbBand.fStyle & RBBS_HIDDEN) == 0;
		if (bWindowVisible != bBandVisible)
			VERIFY(DefWindowProc(RB_SHOWBAND, nTemp, bWindowVisible));
	}

	// determine bounding rect of all visible bands
	CRect rectBound; rectBound.SetRectEmpty();
	for (nTemp = nCount; nTemp--; )
	{
		rbBand.fMask = RBBIM_STYLE;
		VERIFY(DefWindowProc(RB_GETBANDINFO, nTemp, (LPARAM)&rbBand));
		if ((rbBand.fStyle & RBBS_HIDDEN) == 0)
		{
			CRect rect;
			VERIFY(DefWindowProc(RB_GETRECT, nTemp, (LPARAM)&rect));
			rectBound |= rect;
		}
	}

	// add borders as part of bounding rect
	if (!rectBound.IsRectEmpty())
	{
		CRect rect; rect.SetRectEmpty();
		CalcInsideRect(rect, bHorz);
		rectBound.right -= rect.Width();
		rectBound.bottom -= rect.Height();
	}
	bStretch = 1;
	return CSize((bHorz && bStretch) ? 32767 : rectBound.Width(),
		(!bHorz && bStretch) ? 32767 : rectBound.Height());
}

BOOL CMFCReBar::Create(CWnd* pParentWnd, DWORD dwCtrlStyle, DWORD dwStyle, UINT nID)
{
	ENSURE( AfxIsExtendedFrameClass(pParentWnd) );

	ASSERT(!((dwStyle & CBRS_SIZE_FIXED) &&(dwStyle & CBRS_SIZE_DYNAMIC)));

	// save the style
	m_dwStyle = (dwStyle & CBRS_ALL);
	if (nID == AFX_IDW_REBAR)
		m_dwStyle |= CBRS_HIDE_INPLACE;

	dwStyle &= ~CBRS_ALL;
	dwStyle |= CCS_NOPARENTALIGN|CCS_NOMOVEY|CCS_NODIVIDER|CCS_NORESIZE|RBS_VARHEIGHT;
	dwStyle |= dwCtrlStyle | WS_CLIPCHILDREN;

	m_pDockSite = pParentWnd;

	// initialize common controls
	VERIFY(AfxDeferRegisterClass(AFX_WNDCOMMCTL_COOL_REG));

	// create the HWND
	CRect rect; rect.SetRectEmpty();
	if (!CWnd::Create(REBARCLASSNAME, NULL, dwStyle, rect, pParentWnd, nID))
		return FALSE;

	// Note: Parent must resize itself for control bar to be resized

	return TRUE;
}

void CMFCReBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHandler)
{
	UpdateDialogControls(pTarget, bDisableIfNoHandler);
}

BOOL CMFCReBar::OnNcCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (!CPane::OnNcCreate(lpCreateStruct))
		return FALSE;

	// if the owner was set before the rebar was created, set it now
	if (m_hWndOwner != NULL)
		DefWindowProc(RB_SETPARENT, (WPARAM)m_hWndOwner, 0);

	return TRUE;
}

BOOL CMFCReBar::OnEraseBkgnd(CDC*)
{
	return(BOOL)Default();
}

void CMFCReBar::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS* lpncsp)
{
	// calculate border space(will add to top/bottom, subtract from right/bottom)
	CRect rect; rect.SetRectEmpty();
	BOOL bHorz = (m_dwStyle & CBRS_ORIENT_HORZ) != 0;
	CPane::CalcInsideRect(rect, bHorz);

	// adjust non-client area for border space
	lpncsp->rgrc[0].left += rect.left;
	lpncsp->rgrc[0].top += rect.top;
	lpncsp->rgrc[0].right += rect.right;
	lpncsp->rgrc[0].bottom += rect.bottom;
}

void CMFCReBar::OnNcPaint()
{
	m_Impl.DrawNcArea();
}

void CMFCReBar::OnPaint()
{
	Default();
}

INT_PTR CMFCReBar::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	ASSERT_VALID(this);
	ENSURE(::IsWindow(m_hWnd));

	HWND hWndChild = ChWindowFromPoint(m_hWnd, point);
	CWnd* pWnd = CWnd::FromHandlePermanent(hWndChild);
	if (pWnd == NULL)
		return(INT_PTR) CPane::OnToolHitTest(point, pTI);

	ENSURE(pWnd->m_hWnd == hWndChild);
	return(INT_PTR) pWnd->OnToolHitTest(point, pTI);
}

LRESULT CMFCReBar::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// special handling for certain messages(forwarding to owner/parent)
	switch (message)
	{
	case WM_POPMESSAGESTRING:
	case WM_SETMESSAGESTRING:
		return GetOwner()->SendMessage(message, wParam, lParam);
	}
	return CPane::WindowProc(message, wParam, lParam);
}

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

#ifdef _DEBUG

// rebars do not support docking
void CMFCReBar::EnableDocking(DWORD dwAlignment)
{
	CPane::EnableDocking(dwAlignment);
}

#endif

CReBarCtrl& CMFCReBar::GetReBarCtrl() const
{
	return *(CReBarCtrl*)this;
}

IMPLEMENT_DYNAMIC(CMFCReBar, CPane)

/////////////////////////////////////////////////////////////////////////////
HWND ChWindowFromPoint(HWND hWnd, POINT pt)
{
	ENSURE(hWnd != NULL);

	// check child windows
	::ClientToScreen(hWnd, &pt);
	HWND hWndChild = ::GetWindow(hWnd, GW_CHILD);
	for (; hWndChild != NULL; hWndChild = ::GetWindow(hWndChild, GW_HWNDNEXT))
	{
		if (_AfxGetDlgCtrlID(hWndChild) != (WORD)-1 && (::GetWindowLong(hWndChild, GWL_STYLE) & WS_VISIBLE))
		{
			// see if point hits the child window
			CRect rect;
			::GetWindowRect(hWndChild, rect);
			if (rect.PtInRect(pt))
				return hWndChild;
		}
	}

	return NULL;    // not found
}

void CMFCReBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	CWnd::OnLButtonDown(nFlags, point);
}

void CMFCReBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	CWnd::OnLButtonUp(nFlags, point);
}

void CMFCReBar::OnMouseMove(UINT nFlags, CPoint point)
{
	CWnd::OnMouseMove(nFlags, point);
}

void CMFCReBar::SetPaneAlignment(DWORD dwAlignment)
{
	CReBarCtrl& wndReBar = GetReBarCtrl();
	UINT uiReBarsCount = wndReBar.GetBandCount();

	REBARBANDINFO bandInfo;
	bandInfo.cbSize = m_nReBarBandInfoSize;
	bandInfo.fMask = (RBBIM_CHILDSIZE | RBBIM_CHILD | RBBIM_IDEALSIZE);

	for (UINT uiBand = 0; uiBand < uiReBarsCount; uiBand ++)
	{
		wndReBar.GetBandInfo(uiBand, &bandInfo);
		if (bandInfo.hwndChild != NULL)
		{
			CBasePane* pBar = DYNAMIC_DOWNCAST(CBasePane, CWnd::FromHandlePermanent(bandInfo.hwndChild));

			if (pBar != NULL)
			{
				pBar->SetPaneAlignment(dwAlignment);
			}
		}
	}
}



