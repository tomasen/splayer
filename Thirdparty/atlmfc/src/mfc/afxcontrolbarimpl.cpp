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
#include "afxcontrolbarimpl.h"
#include "afxtoolbar.h"
#include "afxvisualmanager.h"
#include "afxglobals.h"
#include "afxpane.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CFrameWnd* g_pTopLevelFrame = NULL;

// Construction/Destruction
CMFCControlBarImpl::CMFCControlBarImpl(CPane* pBar) : m_pBar(pBar)
{
	ASSERT_VALID(m_pBar);
}

CMFCControlBarImpl::~CMFCControlBarImpl()
{
}

void CMFCControlBarImpl::DrawNcArea()
{
	CWindowDC dc(m_pBar);

	CRect rectClient;
	m_pBar->GetClientRect(rectClient);

	CRect rectWindow;
	m_pBar->GetWindowRect(rectWindow);

	m_pBar->ScreenToClient(rectWindow);
	rectClient.OffsetRect(-rectWindow.left, -rectWindow.top);
	dc.ExcludeClipRect(rectClient);

	BOOL bRTL = m_pBar->GetExStyle() & WS_EX_LAYOUTRTL;

	{
		MSG* pMsg = &AfxGetThreadState()->m_lastSentMsg;

		ASSERT(pMsg->hwnd == m_pBar->m_hWnd);
		ASSERT(pMsg->message == WM_NCPAINT);

		CRgn* pRgn = NULL;
		if (pMsg->wParam != 1 &&
			(pRgn = CRgn::FromHandle((HRGN) pMsg->wParam)) != NULL)
		{
			CRect rect;
			m_pBar->GetWindowRect(rect);

			if (bRTL)
			{
				CRect rect2;
				pRgn->GetRgnBox(&rect2);
				rect2.OffsetRect(rect.right - rect2.right - rect2.left, -rect.top);
				CRgn rgn;
				rgn.CreateRectRgnIndirect(&rect2);
				dc.SelectClipRgn(&rgn, RGN_AND);
			}
			else
			{
				pRgn->OffsetRgn(- rect.TopLeft());
				dc.SelectClipRgn(pRgn, RGN_AND);
			}
		}
	}

	// draw borders in non-client area
	rectWindow.OffsetRect(-rectWindow.left, -rectWindow.top);
	CMFCVisualManager::GetInstance()->OnDrawPaneBorder(&dc, m_pBar, rectWindow);

	// erase parts not drawn
	dc.IntersectClipRect(rectWindow);
	CMFCVisualManager::GetInstance()->OnFillBarBackground(&dc, m_pBar, rectWindow, CRect(0, 0, 0, 0), TRUE /* NC area */);

	// draw gripper in non-client area
	if ((m_pBar->GetPaneStyle() &(CBRS_GRIPPER|CBRS_FLOATING)) != CBRS_GRIPPER)
	{
		dc.SelectClipRgn(NULL);
		return;
	}

	CRect rectGripper;
	GetGripperRect(rectGripper);

	BOOL bHorz = (m_pBar->GetPaneStyle() & CBRS_ORIENT_HORZ) ? TRUE : FALSE;

	CMFCVisualManager::GetInstance()->OnDrawBarGripper(&dc, rectGripper, bHorz, m_pBar);

	dc.SelectClipRgn(NULL);
}

void CMFCControlBarImpl::CalcNcSize(NCCALCSIZE_PARAMS FAR* lpncsp)
{
	ASSERT_VALID(m_pBar);

	CRect rect; rect.SetRectEmpty();
	BOOL bHorz = m_pBar->IsHorizontal();

	m_pBar->CalcInsideRect(rect, bHorz);

	if (bHorz &&(m_pBar->GetExStyle() & WS_EX_LAYOUTRTL) &&((m_pBar->GetStyle() &(CBRS_GRIPPER|CBRS_FLOATING)) == CBRS_GRIPPER))
	{
		rect.OffsetRect(-(AFX_CX_BORDER_GRIPPER+AFX_CX_GRIPPER+AFX_CX_BORDER_GRIPPER), 0);
	}

	// adjust non-client area for border space
	lpncsp->rgrc[0].left += rect.left;
	lpncsp->rgrc[0].top += rect.top;

	lpncsp->rgrc[0].right += rect.right;
	lpncsp->rgrc[0].bottom += rect.bottom;
}

BOOL CMFCControlBarImpl::GetBackgroundFromParent(CDC* pDC)
{
	return afxGlobalData.DrawParentBackground(m_pBar, pDC);
}

void CMFCControlBarImpl::GetGripperRect(CRect& rectGripper, BOOL bClientCoord)
{
	ASSERT_VALID(m_pBar);

	if (m_pBar->GetParentDockSite() == NULL)
	{
		rectGripper.SetRectEmpty();
		return;
	}

	BOOL bRTL = m_pBar->GetExStyle() & WS_EX_LAYOUTRTL;
	BOOL bHorz = (m_pBar->GetPaneStyle() & CBRS_ORIENT_HORZ) ? TRUE : FALSE;

	m_pBar->GetWindowRect(&rectGripper);

	CRect rcClient;
	m_pBar->GetClientRect(&rcClient);
	m_pBar->ClientToScreen(&rcClient);

	if (bHorz)
	{
		if (bRTL)
		{
			rectGripper.left = rcClient.right - 1;
		}
		else
		{
			rectGripper.right = min(rectGripper.right, rcClient.left - 1);
		}
	}
	else
	{
		rectGripper.bottom = min(rectGripper.bottom, rcClient.top - 1);
	}

	if (bClientCoord)
	{
		m_pBar->ScreenToClient(&rectGripper);
	}
	else
	{
		rectGripper.OffsetRect(-rectGripper.left, -rectGripper.top);
	}
}



