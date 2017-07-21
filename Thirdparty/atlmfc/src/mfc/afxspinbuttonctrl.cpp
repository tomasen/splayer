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
#include "afxtrackmouse.h"

#include "afxvisualmanager.h"
#include "afxglobals.h"
#include "afxspinbuttonctrl.h"
#include "afxtoolbarimages.h"
#include "afxdrawmanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCSpinButtonCtrl

CMFCSpinButtonCtrl::CMFCSpinButtonCtrl()
{
	m_bTracked = FALSE;

	m_bIsButtonPressedUp = FALSE;
	m_bIsButtonPressedDown = FALSE;

	m_bIsButtonHighligtedUp = FALSE;
	m_bIsButtonHighligtedDown = FALSE;
}

CMFCSpinButtonCtrl::~CMFCSpinButtonCtrl()
{
}

//{{AFX_MSG_MAP(CMFCSpinButtonCtrl)
BEGIN_MESSAGE_MAP(CMFCSpinButtonCtrl, CSpinButtonCtrl)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_CANCELMODE()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_MOUSELEAVE, &CMFCSpinButtonCtrl::OnMouseLeave)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// CMFCSpinButtonCtrl message handlers

void CMFCSpinButtonCtrl::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CMemDC memDC(dc, this);

	OnDraw(&memDC.GetDC());
}

void CMFCSpinButtonCtrl::OnDraw(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	CRect rectClient;
	GetClientRect(rectClient);

	if (CMFCToolBarImages::m_bIsDrawOnGlass)
	{
		CDrawingManager dm(*pDC);
		dm.DrawRect(rectClient, afxGlobalData.clrWindow, (COLORREF)-1);
	}
	else
	{
		pDC->FillRect(rectClient, &afxGlobalData.brWindow);
	}

	int nState = 0;

	if (m_bIsButtonPressedUp)
	{
		nState |= AFX_SPIN_PRESSEDUP;
	}

	if (m_bIsButtonPressedDown)
	{
		nState |= AFX_SPIN_PRESSEDDOWN;
	}

	if (m_bIsButtonHighligtedUp)
	{
		nState |= AFX_SPIN_HIGHLIGHTEDUP;
	}

	if (m_bIsButtonHighligtedDown)
	{
		nState |= AFX_SPIN_HIGHLIGHTEDDOWN;
	}

	if (!IsWindowEnabled())
	{
		nState |= AFX_SPIN_DISABLED;
	}

	CMFCVisualManager::GetInstance()->OnDrawSpinButtons(pDC, rectClient, nState, FALSE, this);
}

void CMFCSpinButtonCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rect;
	GetClientRect(rect);

	CRect rectUp = rect;
	rectUp.bottom = rect.CenterPoint().y;

	CRect rectDown = rect;
	rectDown.top = rectUp.bottom;

	m_bIsButtonPressedUp = rectUp.PtInRect(point);
	m_bIsButtonPressedDown = rectDown.PtInRect(point);

	CSpinButtonCtrl::OnLButtonDown(nFlags, point);
}

void CMFCSpinButtonCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bIsButtonPressedUp = FALSE;
	m_bIsButtonPressedDown = FALSE;

	m_bIsButtonHighligtedUp = FALSE;
	m_bIsButtonHighligtedDown = FALSE;

	CSpinButtonCtrl::OnLButtonUp(nFlags, point);
}

void CMFCSpinButtonCtrl::OnCancelMode()
{
	CSpinButtonCtrl::OnCancelMode();

	m_bIsButtonPressedUp = FALSE;
	m_bIsButtonPressedDown = FALSE;

	m_bIsButtonHighligtedUp = FALSE;
	m_bIsButtonHighligtedDown = FALSE;
}

void CMFCSpinButtonCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	BOOL bIsButtonHighligtedUp = m_bIsButtonHighligtedUp;
	BOOL bIsButtonHighligtedDown = m_bIsButtonHighligtedDown;

	CRect rect;
	GetClientRect(rect);

	CRect rectUp = rect;
	rectUp.bottom = rect.CenterPoint().y;

	CRect rectDown = rect;
	rectDown.top = rectUp.bottom;

	m_bIsButtonHighligtedUp = rectUp.PtInRect(point);
	m_bIsButtonHighligtedDown = rectDown.PtInRect(point);

	if (nFlags & MK_LBUTTON)
	{
		m_bIsButtonPressedUp = m_bIsButtonHighligtedUp;
		m_bIsButtonPressedDown = m_bIsButtonHighligtedDown;
	}

	CSpinButtonCtrl::OnMouseMove(nFlags, point);

	if (bIsButtonHighligtedUp != m_bIsButtonHighligtedUp || bIsButtonHighligtedDown != m_bIsButtonHighligtedDown)
	{
		RedrawWindow();
	}

	if (!m_bTracked)
	{
		m_bTracked = TRUE;

		TRACKMOUSEEVENT trackmouseevent;
		trackmouseevent.cbSize = sizeof(trackmouseevent);
		trackmouseevent.dwFlags = TME_LEAVE;
		trackmouseevent.hwndTrack = GetSafeHwnd();
		trackmouseevent.dwHoverTime = HOVER_DEFAULT;
		::AFXTrackMouse(&trackmouseevent);
	}
}

LRESULT CMFCSpinButtonCtrl::OnMouseLeave(WPARAM,LPARAM)
{
	m_bTracked = FALSE;

	if (m_bIsButtonPressedUp || m_bIsButtonPressedDown || m_bIsButtonHighligtedUp || m_bIsButtonHighligtedDown)
	{
		m_bIsButtonHighligtedUp = FALSE;
		m_bIsButtonHighligtedDown = FALSE;

		RedrawWindow();
	}

	return 0;
}

BOOL CMFCSpinButtonCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}


