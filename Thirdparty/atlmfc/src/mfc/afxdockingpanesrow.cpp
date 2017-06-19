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

#include "afxpane.h"
#include "afxdocksite.h"
#include "afxdockingpanesrow.h"
#include "afxautohidebutton.h"
#include "afxvisualmanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CDockingPanesRow, CObject)

// Construction/Destruction
CDockingPanesRow::CDockingPanesRow(CDockSite* pParentDockBar, int nOffset, int nHeight) : m_pParentDockBar(pParentDockBar), m_nRowOffset(nOffset), m_nRowHeight(nHeight)
{
	ASSERT_VALID(pParentDockBar);
	m_dwRowAlignment = pParentDockBar->GetCurrentAlignment();
	m_nMinHeight = 0;
	m_nExtraSpace = 0;
	m_nExtraAlignment = AFX_ROW_ALIGN_TOP;
	m_bVisible = TRUE;
	m_nRowSavedHeight = 0;
	m_bIgnoreBarVisibility = FALSE;
}

CDockingPanesRow::~CDockingPanesRow()
{
}

void CDockingPanesRow::AddPane(CPane* pControlBar, AFX_DOCK_METHOD /*dockMethod*/, LPCRECT lpRect, BOOL bAddLast)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pControlBar);

	CRect rectRow;

	CPoint ptOffset(0, 0);
	if (lpRect != NULL && !IsRectEmpty(lpRect))
	{
		CRect rectOrg = lpRect;
		CRect rectDockBar;
		m_pParentDockBar->GetClientRect(&rectDockBar);
		GetWindowRect(rectRow);

		m_pParentDockBar->ScreenToClient(rectOrg);
		m_pParentDockBar->ScreenToClient(rectRow);

		if (IsHorizontal())
		{
			ptOffset.x = rectOrg.left - rectRow.left;
			ptOffset.y = rectDockBar.top + m_nRowOffset;
		}
		else
		{
			ptOffset.x = rectDockBar.left + m_nRowOffset;
			ptOffset.y = rectOrg.top - rectRow.top;
		}
	}
	else
	{
		int nAdditionalBarOffset = 0;
		if (bAddLast)
		{
			nAdditionalBarOffset = CalcLastPaneOffset();
			if (nAdditionalBarOffset > 0)
			{
				nAdditionalBarOffset += afxGlobalData.m_nAutoHideToolBarSpacing;
			}
		}

		GetClientRect(rectRow);
		if (IsHorizontal())
		{
			ptOffset.x = rectRow.left + nAdditionalBarOffset;
			ptOffset.y = m_nRowOffset;
			// align the bar to the bottom of the row
			if (m_nExtraAlignment == AFX_ROW_ALIGN_BOTTOM)
			{
				ptOffset.y += m_nExtraSpace;
			}
		}
		else
		{
			ptOffset.x = m_nRowOffset;
			ptOffset.y = rectRow.top + nAdditionalBarOffset;

			// align the bar to the right side of the row
			if (m_nExtraAlignment == AFX_ROW_ALIGN_BOTTOM)
			{
				ptOffset.x += m_nExtraSpace;
			}
		}
	}

	CSize szBarSize = pControlBar->CalcFixedLayout(FALSE, IsHorizontal());

	pControlBar->SetWindowPos(NULL, ptOffset.x, ptOffset.y, szBarSize.cx, szBarSize.cy, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

	OnInsertPane(pControlBar);
	pControlBar->UpdateVirtualRect();
}

void CDockingPanesRow::AddPaneFromRow(CPane* pControlBar, AFX_DOCK_METHOD dockMethod)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pControlBar);

	CRect rectWnd;
	pControlBar->GetWindowRect(&rectWnd);

	int nOffsetOnRow = 0;
	if (dockMethod == DM_MOUSE)
	{
		// position the bar at the mouse cursor
		CPoint ptMouse;
		GetCursorPos(&ptMouse);
		m_pParentDockBar->ScreenToClient(&ptMouse);

		CPoint ptHot = pControlBar->GetClientHotSpot();

		// take the grippers and borders int account
		CRect rectClient;
		pControlBar->GetClientRect(rectClient);
		pControlBar->ClientToScreen(rectClient);

		int nNCOffset = rectClient.left - rectWnd.left;

		nOffsetOnRow = IsHorizontal() ? ptMouse.x - ptHot.x - nNCOffset : ptMouse.y - ptHot.y - nNCOffset;

	}
	else
	{
		m_pParentDockBar->ScreenToClient(&rectWnd);
		nOffsetOnRow = IsHorizontal()  ? rectWnd.left : rectWnd.top;
	}

	CRect rectPos;
	if (IsHorizontal())
	{
		rectPos.SetRect(nOffsetOnRow, m_nRowOffset, nOffsetOnRow + rectWnd.Width(), m_nRowOffset + rectWnd.Height());
	}
	else
	{
		rectPos.SetRect(m_nRowOffset, nOffsetOnRow, m_nRowOffset + rectWnd.Width(), nOffsetOnRow  + rectWnd.Height());
	}

	pControlBar->SetWindowPos(NULL, rectPos.left, rectPos.top, rectPos.right, rectPos.bottom, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	OnInsertPane(pControlBar);
	pControlBar->UpdateVirtualRect();
}

void CDockingPanesRow::OnInsertPane(CPane* pControlBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pControlBar);

	CRect rectBarWnd;
	pControlBar->GetWindowRect(&rectBarWnd);

	CPane* pPrevControlBar = NULL;

	bool bWasInserted = false;
	POSITION posSave = NULL;
	// the existing bars are ordered from the left to right or from top to bottom
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		posSave = pos;
		CPane* pBarWnd = DYNAMIC_DOWNCAST(CPane, m_lstControlBars.GetNext(pos));
		pPrevControlBar = pBarWnd;

		ASSERT_VALID(pBarWnd);

		// the new bar should be inserted before the next bar it is left of
		bWasInserted = pBarWnd->IsLeftOf(rectBarWnd);

		if (bWasInserted)
		{
			m_lstControlBars.InsertBefore(posSave, pControlBar);
			break;
		}
	}

	// if the bar wasn't inserted
	if (!bWasInserted)
	{
		m_lstControlBars.AddTail(pControlBar);
	}

	UpdateVisibleState(TRUE);

	int nNewRowHeight = IsHorizontal() ? rectBarWnd.Height() : rectBarWnd.Width();

	if (nNewRowHeight > GetRowHeight())
	{
		m_pParentDockBar->ResizeRow(this, nNewRowHeight + m_nExtraSpace);
	}

	pControlBar->GetDockSiteRow(this);
	ArrangePanes(pControlBar);
}

void CDockingPanesRow::RemovePane(CPane* pControlBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pControlBar);

	POSITION pos = m_lstControlBars.Find(pControlBar);
	if (pos != NULL)
	{
		// expand the bar before it leaves the row
		HDWP hdwp = NULL;
		pControlBar->StretchPaneDeferWndPos(0xFFFF, hdwp);

		m_lstControlBars.RemoveAt(pos);

		pControlBar->GetDockSiteRow(NULL);

		if (!m_lstControlBars.IsEmpty())
		{
			FixupVirtualRects(true, pControlBar);
			// expand stretched bars(if any)
			ExpandStretchedPanes();
			UpdateVisibleState(FALSE);

			// find the biggest control bar and resize the row according to its size
			int nMaxBarSize = GetMaxPaneSize(FALSE);
			if (nMaxBarSize < GetRowHeight())
			{
				m_pParentDockBar->ResizeRow(this, nMaxBarSize);
				m_nRowHeight = nMaxBarSize;
			}

		}
		else
		{
			m_pParentDockBar->RemoveRow(this);
		}

	}
}

CSize CDockingPanesRow::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	if (!m_bVisible)
	{
		if (IsHorizontal())
		{
			return CSize(32767, 0);
		}
		return CSize(0, 32767);
	}

	BOOL bHorzBar = IsHorizontal();

	CSize sizeRequired(0, 0);
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CPane* pBar = DYNAMIC_DOWNCAST(CPane, m_lstControlBars.GetNext(pos));
		ASSERT_VALID(pBar);

		if (!pBar->IsVisible() && !m_bIgnoreBarVisibility)
		{
			continue;
		}

		CSize sizeBar = pBar->CalcFixedLayout(bStretch, bHorz);

		if (bHorzBar)
		{
			sizeRequired.cx += sizeBar.cx;
			sizeRequired.cy = max(sizeRequired.cy, sizeBar.cy);
		}
		else
		{
			sizeRequired.cx = max(sizeRequired.cx, sizeBar.cx);
			sizeRequired.cy += sizeBar.cy;
		}
	}

	if (bHorzBar && sizeRequired.cy > 0)
	{
		sizeRequired.cy += m_nExtraSpace;
	}

	if (!bHorzBar && sizeRequired.cx > 0)
	{
		sizeRequired.cx += m_nExtraSpace;
	}

	return sizeRequired;
}

void CDockingPanesRow::OnResizePane(CBasePane* /*pControlBar*/)
{
}

int CDockingPanesRow::Resize(int nOffset)
{
	int nNewHeight = m_nRowHeight + nOffset;

	int nActualOffset = nNewHeight - m_nRowHeight;
	m_nRowHeight = nNewHeight;

	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CPane* pBar = DYNAMIC_DOWNCAST(CPane, m_lstControlBars.GetNext(pos));
		if (pBar != NULL)
		{
			pBar->RecalcLayout();
		}
	}

	// return the actual resize offset
	return nActualOffset;
}

void CDockingPanesRow::Move(int nOffset)
{
	ASSERT_VALID(this);

	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CPane* pBar = DYNAMIC_DOWNCAST(CPane, m_lstControlBars.GetNext(pos));
		if (pBar != NULL)
		{
			pBar->MoveByAlignment(m_dwRowAlignment, nOffset);
		}
	}
	m_nRowOffset += nOffset;
}

void CDockingPanesRow::GetWindowRect(CRect& rect) const
{
	ASSERT_VALID(this);
	rect.SetRectEmpty();

	if (m_pParentDockBar == NULL)
	{
		return;
	}

	m_pParentDockBar->GetWindowRect(&rect);

	if (IsHorizontal())
	{
		rect.top += m_nRowOffset;
		rect.bottom = rect.top + GetRowHeight();
	}
	else
	{
		rect.left += m_nRowOffset;
		rect.right = rect.left + GetRowHeight();
	}
}

void CDockingPanesRow::GetClientRect(CRect& rect) const
{
	ASSERT_VALID(this);

	GetWindowRect(rect);
	m_pParentDockBar->ScreenToClient(&rect);

	if (IsHorizontal())
	{
		rect.top -= m_nRowOffset;
		rect.bottom = rect.top + GetRowHeight();
	}
	else
	{
		rect.left -= m_nRowOffset;
		rect.right = rect.left + GetRowHeight();
	}
}

void CDockingPanesRow::ScreenToClient(CRect& rect) const
{
	ASSERT_VALID(this);
	m_pParentDockBar->ScreenToClient(&rect);
}

BOOL CDockingPanesRow::ShowPane(CPane* pControlBar, BOOL bShow, BOOL bDelay)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pControlBar);
	if (!HasPane(pControlBar))
	{
		return FALSE;
	}

	pControlBar->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
	UpdateVisibleState(bDelay);

	if (!bShow)
	{
		ExpandStretchedPanes();
	}
	else if (!bDelay)
	{
		pControlBar->AdjustLayout();
		ArrangePanes(pControlBar);
	}

	if (!bDelay)
	{
		CRect rect;
		GetClientRect(rect);
		m_pParentDockBar->RepositionPanes(rect);
		RepositionPanes(rect);
	}

	return TRUE;
}

void CDockingPanesRow::UpdateVisibleState(BOOL bDelay)
{
	BOOL bUseRecentVisibleState = !m_pParentDockBar->GetParent()->IsWindowVisible();
	BOOL bOldVisibleState = m_bVisible;
	BOOL bNewVisibleState  = FALSE;
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CBasePane* pNextWnd = (CBasePane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pNextWnd);

		if (bUseRecentVisibleState && pNextWnd->IsRestoredFromRegistry())
		{
			bNewVisibleState = pNextWnd->GetRecentVisibleState();
			if (bNewVisibleState)
			{
				break;
			}
		}
		else if (pNextWnd->GetStyle() & WS_VISIBLE)
		{
			bNewVisibleState = TRUE;
			break;
		}
	}

	if (bOldVisibleState != bNewVisibleState)
	{
		ShowDockSiteRow(bNewVisibleState, bDelay);
	}
	m_bVisible = bNewVisibleState;
}

void CDockingPanesRow::ShowDockSiteRow(BOOL bShow, BOOL bDelay)
{
	m_bVisible = bShow;
	m_pParentDockBar->ShowRow(this, bShow, !bDelay);
}

void CDockingPanesRow::ExpandStretchedPanes()
{
	ASSERT_VALID(this);

	if (m_lstControlBars.IsEmpty())
	{
		return;
	}

	// do not use virt. rects - we need real row space
	int nAvailableLen = GetAvailableLength(FALSE);

	HDWP hdwp = NULL;
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CPane* pNextBar = (CPane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pNextBar);

		if (!pNextBar->IsVisible() && !m_bIgnoreBarVisibility)
		{
			continue;
		}

		int nRetSize = pNextBar->StretchPaneDeferWndPos(nAvailableLen, hdwp);

		nAvailableLen -= nRetSize;

		if (nAvailableLen <= 0)
		{
			break;
		}
	}

	ArrangePanes(NULL);

}

void CDockingPanesRow::ExpandStretchedPanesRect()
{
	ASSERT_VALID(this);

	if (m_lstControlBars.IsEmpty())
	{
		return;
	}

	BeginTrans();

	// do not use virt. rects - we need real row space
	int nAvailableLen = GetAvailableLengthRect();

	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CPane* pNextBar = (CPane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pNextBar);

		if (!pNextBar->IsVisible() && !m_bIgnoreBarVisibility)
		{
			continue;
		}

		int nRetSize = StretchPaneRect(pNextBar, nAvailableLen);

		nAvailableLen -= nRetSize;

		if (nAvailableLen <= 0)
		{
			break;
		}
	}

	ArrangePanesRect(NULL);
	CommitTrans();

}

void CDockingPanesRow::ShiftPanes(CPane* pControlBar, int nOffset, BOOL bForward)
{
	ASSERT_VALID(this);
	ASSERT(!m_lstControlBars.IsEmpty());

	if (nOffset == 0)
	{
		return;
	}

	POSITION pos = NULL;

	if (pControlBar != NULL)
	{
		pos = m_lstControlBars.Find(pControlBar);
	}
	else
	{
		pos = bForward ? m_lstControlBars.GetHeadPosition() : m_lstControlBars.GetTailPosition();
		pControlBar = (CPane*) m_lstControlBars.GetAt(pos);
	}

	int nMoveOffset = nOffset;

	CRect rectBar; rectBar.SetRectEmpty();

	while (pos != NULL)
	{
		CPane* pNextBar = (CPane*)(bForward ? m_lstControlBars.GetNext(pos) : m_lstControlBars.GetPrev(pos));

		if (!pNextBar->IsVisible() && !m_bIgnoreBarVisibility)
		{
			continue;
		}

		ASSERT_VALID(pNextBar);
		CRect rectNextBar;
		pNextBar->GetWindowRect(rectNextBar);

		if (pNextBar != pControlBar && !rectBar.IsRectEmpty())
		{
			if (IsHorizontal())
			{
				nMoveOffset -= bForward ? rectNextBar.left - rectBar.right : rectNextBar.right - rectBar.left;
			}
			else
			{
				nMoveOffset -= bForward ? rectNextBar.top - rectBar.bottom : rectNextBar.bottom - rectBar.top;
			}
		}

		if (nMoveOffset <= 0 && bForward || nMoveOffset >= 0 && !bForward)
		{
			break;
		}

		rectBar = rectNextBar;
		IsHorizontal() ? rectNextBar.OffsetRect(nMoveOffset, 0) : rectNextBar.OffsetRect(0, nMoveOffset);
		m_pParentDockBar->ScreenToClient(rectNextBar);

		pNextBar->SetWindowPos(NULL, rectNextBar.left, rectNextBar.top, rectNextBar.Width(), rectNextBar.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	}
}

void CDockingPanesRow::MovePane(CPane* pControlBar, CPoint ptOffset, BOOL bSwapControlBars, HDWP& hdwp)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pControlBar);

	CRect rectVirtual;
	pControlBar->GetVirtualRect(rectVirtual);

	CRect rectBarWnd;
	pControlBar->GetWindowRect(&rectBarWnd);

	CRect rectVirualNew = rectVirtual;
	rectVirualNew.OffsetRect(ptOffset);

	// the bar is being moved within the row, just move it horizontally or vertically
	bool bForward = true;
	CPoint ptMove(0, 0);
	int nAllowedOffset = 0;

	if (IsHorizontal())
	{
		nAllowedOffset = ptMove.x = ptOffset.x;
		bForward = (ptMove.x >= 0);
	}
	else
	{
		nAllowedOffset = ptMove.y = ptOffset.y;
		bForward = (ptMove.y >= 0);
	}

	if (!IsEnoughSpaceToMove(pControlBar, bForward, nAllowedOffset))
	{
		return;
	}

	if (IsHorizontal() && abs(nAllowedOffset) < abs(ptMove.x))
	{
		ptMove.x = nAllowedOffset;
	}
	else if (!IsHorizontal() && abs(nAllowedOffset) < abs(ptMove.y))
	{
		ptMove.y = nAllowedOffset;
	}

	rectBarWnd.OffsetRect(ptMove);

	if (CheckPanes(rectBarWnd, pControlBar, bForward, ptMove, bSwapControlBars, hdwp))
	{
		m_pParentDockBar->ScreenToClient(&rectBarWnd);

		pControlBar->SetWindowPos(NULL, rectBarWnd.left, rectBarWnd.top, rectBarWnd.Width(), rectBarWnd.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	}

	ArrangePanes(pControlBar);
}

void CDockingPanesRow::MovePane(CPane* pControlBar, CRect rectTarget, HDWP& /*hdwp*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pControlBar);

	CRect rectBarWnd;
	pControlBar->GetWindowRect(&rectBarWnd);

	if (IsHorizontal())
	{
		rectBarWnd.left = rectTarget.left;
		rectBarWnd.right = rectTarget.right;
	}
	else
	{
		rectBarWnd.top = rectTarget.top;
		rectBarWnd.bottom = rectTarget.bottom;
	}
	m_pParentDockBar->ScreenToClient(&rectBarWnd);

	pControlBar->SetWindowPos(NULL, rectBarWnd.left, rectBarWnd.top, rectBarWnd.Width(), rectBarWnd.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
}

void CDockingPanesRow::MovePane(CPane* pControlBar, int nOffset, bool bForward, HDWP& /*hdwp*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pControlBar);

	if (nOffset == 0)
	{
		return;
	}

	CRect rectBarWnd;
	pControlBar->GetWindowRect(&rectBarWnd);

	if (IsHorizontal())
	{
		rectBarWnd.OffsetRect(bForward ? nOffset : -nOffset, 0);
	}
	else
	{
		rectBarWnd.OffsetRect(0, bForward ? nOffset : -nOffset);
	}
	m_pParentDockBar->ScreenToClient(&rectBarWnd);

	pControlBar->SetWindowPos(NULL, rectBarWnd.left, rectBarWnd.top, rectBarWnd.Width(), rectBarWnd.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
}

void CDockingPanesRow::MovePane(CPane* pControlBar, int nAbsolutOffset, HDWP& /*hdwp*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pControlBar);

	CRect rectBarWnd;
	pControlBar->GetWindowRect(&rectBarWnd);

	CRect rectRow;
	GetWindowRect(rectRow);

	int nBarLength = 0;

	if (IsHorizontal())
	{
		nBarLength = rectBarWnd.Width();
		rectBarWnd.left = rectRow.left + nAbsolutOffset;
		rectBarWnd.right = rectBarWnd.left + nBarLength;
	}
	else
	{
		nBarLength = rectBarWnd.Height();
		rectBarWnd.top = rectRow.top + nAbsolutOffset;
		rectBarWnd.bottom = rectBarWnd.top + nBarLength;
	}

	m_pParentDockBar->ScreenToClient(&rectBarWnd);
	pControlBar->SetWindowPos(NULL, rectBarWnd.left, rectBarWnd.top, rectBarWnd.Width(), rectBarWnd.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
}

int CDockingPanesRow::GetVisibleCount()
{
	int nVisibleCount = 0;
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CPane* pNextBar = (CPane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pNextBar);

		if (!m_bIgnoreBarVisibility)
		{
			if (pNextBar->IsVisible())
			{
				nVisibleCount++;
			}
		}
		else
		{
			nVisibleCount++;
		}
	}

	return nVisibleCount;
}

BOOL CDockingPanesRow::CheckPanes(CRect& rectCurrentBar, CPane* pCurrentBar, bool bForward, CPoint ptOffset, BOOL bSwapControlBars, HDWP& hdwp)
{
	ASSERT_VALID(this);

	if (m_lstControlBars.GetCount() < 2 || GetVisibleCount() < 2)
	{
		// nothing to check - there is only one control bar on the dock bar
		return TRUE;
	}

	CRect rectNextControlBar;
	CRect rectNextControlBarVirt;

	CPane* pNextBar = NULL;
	CRect rectIntersect;

	BOOL bIntersect = FALSE;

	POSITION posCurrentBar = m_lstControlBars.Find(pCurrentBar);
	// position of the bar which intersects with the current bar
	POSITION posIntersect = NULL;
	// find a control bar which whill intersect with the current bar
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		posIntersect = pos;
		pNextBar = (CPane*) m_lstControlBars.GetNext(pos);

		if (!pNextBar->IsVisible() && !m_bIgnoreBarVisibility)
		{
			continue;
		}

		if (pNextBar != pCurrentBar)
		{
			ASSERT_VALID(pNextBar);

			pNextBar->GetWindowRect(&rectNextControlBar);
			bIntersect = rectIntersect.IntersectRect(rectNextControlBar, rectCurrentBar);
			if (bIntersect)
			{
				break;
			}
		}
	}

	if (!bIntersect || pNextBar == NULL)
	{
		// the current bar does not intersect with any bar on the row, check whether
		// we need to move "trailing bars" - whose virtual rectangle is out of place

		MoveTrailingPanes(posCurrentBar, ptOffset, bForward, pCurrentBar, hdwp);
		return TRUE;
	}

	// get the virtual rectangle of the bar that intersects with the curr. bar
	pNextBar->GetVirtualRect(rectNextControlBarVirt);

	// if the current bar has just crossed a point when it is no more on the left side
	// from the virtual rect(when moving forward), or it is no more on the right side
	// when moving backward;
	// the next control bar should be transferred to the opposite side
	CRect rectIntersectVirt;

	if (rectIntersectVirt.IntersectRect(rectNextControlBarVirt, rectCurrentBar) && bSwapControlBars)
	{
		if (bForward && pCurrentBar->IsLeftOf(rectNextControlBarVirt) || !bForward && !pCurrentBar->IsLeftOf(rectNextControlBarVirt))
		{
			CRect rectNew = rectNextControlBar;

			if (IsHorizontal())
			{
				bForward ? rectNew.right = rectCurrentBar.left : rectNew.left = rectCurrentBar.right ;
				bForward ? rectNew.left = rectNew.right - rectNextControlBar.Width() : rectNew.right = rectNew.left + rectNextControlBar.Width();
			}
			else
			{
				bForward ? rectNew.bottom = rectCurrentBar.top : rectNew.top = rectCurrentBar.bottom;
				bForward ? rectNew.top = rectNew.bottom - rectNextControlBar.Height() : rectNew.bottom = rectNew.top + rectNextControlBar.Height();
			}
			MovePane(pNextBar, rectNew, hdwp);

			m_lstControlBars.RemoveAt(posIntersect);

			bForward ? m_lstControlBars.InsertBefore(posCurrentBar, pNextBar) : m_lstControlBars.InsertAfter(posCurrentBar, pNextBar);

			ResolveIntersection(pNextBar, !bForward, hdwp);

			// now we need to shift all control bars behind pNextBar to make them closer
			// to the current bar

			CRect rectWnd;
			CRect rectVirt;
			bForward ? m_lstControlBars.GetNext(posCurrentBar) : m_lstControlBars.GetPrev(posCurrentBar);
			for (POSITION pos = posCurrentBar; pos != NULL;)
			{
				CPane* pMovedBar = (CPane*)(bForward ? m_lstControlBars.GetNext(pos) : m_lstControlBars.GetPrev(pos));
				ASSERT_VALID(pMovedBar);

				if (!pMovedBar->IsVisible() && !m_bIgnoreBarVisibility)
				{
					continue;
				}

				pMovedBar->GetWindowRect(&rectWnd);
				pMovedBar->GetVirtualRect(rectVirt);

				if (rectWnd != rectVirt)
				{
					int nOffset = IsHorizontal() ? rectNew.Width() : rectNew.Height();
					nOffset -= IsHorizontal() ? abs(ptOffset.x) : abs(ptOffset.y);
					MovePane(pMovedBar, nOffset, !bForward, hdwp); // move in opposite direction
				}

			}
			return TRUE;
		}
	}

	int nMoveOffset = IsHorizontal() ? rectIntersect.Width() : rectIntersect.Height();
	MovePane(pNextBar, nMoveOffset, bForward, hdwp);
	ResolveIntersection(pNextBar, bForward, hdwp);

	MoveTrailingPanes(posCurrentBar, ptOffset, bForward, pCurrentBar, hdwp);

	return TRUE;
}

void CDockingPanesRow::MoveTrailingPanes(POSITION posStart, CPoint ptOffset, bool bForward, CPane* pBarToSkip, HDWP& hdwp)
{
	ASSERT_VALID(this);

	CRect rectNextControlBar;
	CRect rectNextControlBarVirt;

	for (POSITION pos = posStart; pos != NULL;)
	{
		CPane* pNextBar = (CPane*)(bForward ? m_lstControlBars.GetPrev(pos) : m_lstControlBars.GetNext(pos));
		ASSERT_VALID(pNextBar);

		if (pNextBar == pBarToSkip)
		{
			continue;
		}

		if (!pNextBar->IsVisible() && !m_bIgnoreBarVisibility)
		{
			continue;
		}

		pNextBar->GetWindowRect(&rectNextControlBar);
		pNextBar->GetVirtualRect(rectNextControlBarVirt);

		if (rectNextControlBar != rectNextControlBarVirt)
		{
			int nOffsetToMove = 0;

			if (bForward && pNextBar->IsLeftOf(rectNextControlBarVirt) || !bForward && !pNextBar->IsLeftOf(rectNextControlBarVirt))
			{
				nOffsetToMove = IsHorizontal() ? abs(ptOffset.x) : abs(ptOffset.y);
			}
			else if (bForward && !pNextBar->IsLeftOf(rectNextControlBarVirt) || !bForward && pNextBar->IsLeftOf(rectNextControlBarVirt))
			{
				if (IsHorizontal())
				{
					nOffsetToMove = min(abs(ptOffset.x), abs(rectNextControlBarVirt.left - rectNextControlBar.left));
				}
				else
				{
					nOffsetToMove = min(abs(ptOffset.y), abs(rectNextControlBarVirt.top - rectNextControlBar.top));
				}
			}
			MovePane(pNextBar, nOffsetToMove, bForward, hdwp);
			ResolveIntersection(pNextBar, !bForward, hdwp);
		}
	}
}

void CDockingPanesRow::ResolveIntersection(CPane* pBar, bool bForward, HDWP& hdwp)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);

	POSITION posStart = m_lstControlBars.Find(pBar);

	CRect rectBarWnd;
	rectBarWnd.SetRectEmpty();

	CRect rectRowWnd;
	GetWindowRect(rectRowWnd);

	CRect rectIntersect;
	CRect rectMovedBar;

	for (POSITION pos = posStart; pos != NULL;)
	{
		CPane* pNextBar = (CPane*)(bForward ? m_lstControlBars.GetNext(pos) : m_lstControlBars.GetPrev(pos));

		ASSERT_VALID(pNextBar);

		if (!pNextBar->IsVisible() && !m_bIgnoreBarVisibility)
		{
			continue;
		}

		pNextBar->GetWindowRect(&rectBarWnd);

		CPane* pMovedBar = NULL;
		POSITION posSave = NULL;
		for (POSITION posMoved = pos; posMoved != NULL;)
		{
			posSave = posMoved;
			pMovedBar = (CPane*)(bForward ? m_lstControlBars.GetNext(posMoved) : m_lstControlBars.GetPrev(posMoved));

			if (pMovedBar->IsVisible() || m_bIgnoreBarVisibility)
			{
				break;
			}
			else
			{
				pMovedBar = NULL;
			}
		}

		if (pMovedBar == NULL)
		{
			break;
		}

		pMovedBar->GetWindowRect(&rectMovedBar);

		if (bForward &&(IsHorizontal() && rectMovedBar.left > rectBarWnd.right || !IsHorizontal() && rectMovedBar.top > rectBarWnd.bottom) || !bForward &&
			(IsHorizontal() && rectMovedBar.right < rectBarWnd.left || !IsHorizontal() && rectMovedBar.bottom < rectBarWnd.top))
		{
			pos = posSave;
			continue;
		}

		int nMoveOffset = 0;
		if (bForward)
		{
			nMoveOffset = IsHorizontal() ? rectBarWnd.right - rectMovedBar.left : rectBarWnd.bottom - rectMovedBar.top;
		}
		else
		{
			nMoveOffset = IsHorizontal() ? rectMovedBar.right - rectBarWnd.left : rectMovedBar.bottom - rectBarWnd.top;
		}

		MovePane(pMovedBar, nMoveOffset, bForward, hdwp);
		pos = posSave;
	}
}

BOOL CDockingPanesRow::IsEnoughSpaceToMove(CPane* pControlBar, bool bForward, int& nAllowedOffset)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pControlBar);

	int nLen = 0;
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CRect rectBar;
		CPane* pBar = (CPane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pBar);

		if (!pBar->IsVisible() && !m_bIgnoreBarVisibility)
		{
			continue;
		}

		pBar->GetWindowRect(&rectBar);

		if (pBar == pControlBar)
		{
			continue;
		}

		if (bForward && !pControlBar->IsLeftOf(rectBar) || !bForward && pControlBar->IsLeftOf(rectBar))
		{
			IsHorizontal() ? nLen += rectBar.Width() : nLen += rectBar.Height();
		}
	}

	CRect rectControlBar;
	pControlBar->GetWindowRect(&rectControlBar);

	CRect rectRow;
	GetWindowRect(rectRow);

	nAllowedOffset = 0;
	if (IsHorizontal())
	{
		nAllowedOffset = bForward ? rectRow.right - rectControlBar.right : rectRow.left - rectControlBar.left;
	}
	else
	{
		nAllowedOffset = bForward ? rectRow.bottom - rectControlBar.bottom : rectRow.top - rectControlBar.top;
	}

	nAllowedOffset = bForward ? nAllowedOffset - nLen : nAllowedOffset + nLen;

	if (bForward && nAllowedOffset <= 0 || !bForward && nAllowedOffset >= 0)
	{
		return FALSE;
	}

	return TRUE;
}

void CDockingPanesRow::OffsetFromRect(const CRect& rect, CPoint& pt, bool bForward)
{
	switch (m_dwRowAlignment & CBRS_ALIGN_ANY)
	{
	case CBRS_ALIGN_TOP:
	case CBRS_ALIGN_BOTTOM:
		bForward ? pt.x = rect.Width() : pt.x = -rect.Width();
		break;

	case CBRS_ALIGN_LEFT:
	case CBRS_ALIGN_RIGHT:
		bForward ? pt.y = rect.Height() : pt.y = -rect.Height();
		break;
	}
}

void CDockingPanesRow::FixupVirtualRects(bool bMoveBackToVirtualRect, CPane* pBarToExclude)
{
	ASSERT_VALID(this);

	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CPane* pNextBar = (CPane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pNextBar);

		if (!pNextBar->IsVisible() && !m_bIgnoreBarVisibility)
		{
			continue;
		}

		if (pNextBar == pBarToExclude)
		{
			continue;
		}

		CRect rectBarWnd;
		pNextBar->GetWindowRect(&rectBarWnd);
		if (bMoveBackToVirtualRect)
		{
			CRect rectVirtual;
			pNextBar->GetVirtualRect(rectVirtual);
			if (rectVirtual != rectBarWnd)
			{
				HDWP hdwp = BeginDeferWindowPos((int) m_lstControlBars.GetCount());
				MovePane(pNextBar, rectVirtual, hdwp);
				EndDeferWindowPos(hdwp);
			}
		}
		else
		{
			pNextBar->UpdateVirtualRect();
		}
	}
}

int CDockingPanesRow::GetAvailableLength(BOOL bUseVirtualRect) const
{
	ASSERT_VALID(this);

	CRect rectRow;
	GetClientRect(rectRow);

	int nTotalBarLength = 0;
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CPane* pNextBar = (CPane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pNextBar);

		if (!pNextBar->IsVisible() && !m_bIgnoreBarVisibility)
		{
			continue;
		}

		CRect rectWnd;
		bUseVirtualRect ? pNextBar->GetVirtualRect(rectWnd) : pNextBar->GetWindowRect(&rectWnd);

		nTotalBarLength += IsHorizontal() ? rectWnd.Width() : rectWnd.Height();
	}

	// debug variable
	int nAvailableLength = IsHorizontal() ? rectRow.Width() - nTotalBarLength : rectRow.Height() - nTotalBarLength;

	return nAvailableLength;
}

int CDockingPanesRow::GetMaxPaneSize(BOOL bSkipHiddenBars) const
{
	ASSERT_VALID(this);

	int nMaxSize = 0;
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CPane* pNextBar = (CPane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pNextBar);

		if (!pNextBar->IsVisible() && bSkipHiddenBars && !m_bIgnoreBarVisibility)
		{
			continue;
		}

		CRect rectWnd;
		pNextBar->GetWindowRect(&rectWnd);

		nMaxSize = max(nMaxSize, IsHorizontal() ? rectWnd.Height() : rectWnd.Width());
	}

	// don't use extra space if there are no visible bars
	if (nMaxSize != 0)
	{
		nMaxSize += m_nExtraSpace;
	}

	return nMaxSize;
}

void CDockingPanesRow::GetAvailableSpace(CRect& rect)
{
	ASSERT_VALID(this);

	GetWindowRect(rect);
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CPane* pNextBar = (CPane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pNextBar);

		if (!pNextBar->IsVisible() && !m_bIgnoreBarVisibility)
		{
			continue;
		}

		CRect rectWnd;
		pNextBar->GetWindowRect(&rectWnd);

		IsHorizontal() ? rect.DeflateRect(rectWnd.Width(), 0) : rect.DeflateRect(0, rectWnd.Height());
	}
}

void CDockingPanesRow::ArrangePanes(int nMargin, int nSpacing)
{
	CRect rectRow;
	CRect rectBar;
	CPoint ptOffset(0, 0);

	GetWindowRect(rectRow);

	bool bFistBar = true;
	const BOOL bIsOverlapped = (CMFCVisualManager::GetInstance()->HasOverlappedAutoHideButtons());

	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CPane* pBar = (CPane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pBar);

		if (!pBar->IsVisible() && !m_bIgnoreBarVisibility)
		{
			continue;
		}

		pBar->GetWindowRect(rectBar);

		if (bFistBar)
		{
			IsHorizontal() ? ptOffset.x = rectRow.left + nMargin : ptOffset.y = rectRow.top + nMargin;
		}

		if (!pBar->m_bFirstInGroup && !bFistBar)
		{
			const int nSize = nSpacing + CMFCAutoHideButton::m_nBorderSize;

			if (IsHorizontal())
			{
				ptOffset.x -= nSize;

				if (bIsOverlapped)
				{
					ptOffset.x -= rectBar.Height() / 2;
				}
			}
			else
			{
				ptOffset.y -= nSize;

				if (bIsOverlapped)
				{
					ptOffset.y -= rectBar.Width() / 2;
				}
			}
		}

		if (bFistBar)
		{
			bFistBar = false;
		}

		int nLen = 0;
		if (IsHorizontal())
		{
			int nWidth = rectBar.Width();
			rectBar.left = ptOffset.x;
			rectBar.right = rectBar.left + nWidth;
			nLen = nWidth;
		}
		else
		{
			int nHeight = rectBar.Height();
			rectBar.top = ptOffset.y;
			rectBar.bottom = rectBar.top + nHeight;
			nLen = nHeight;
		}

		ScreenToClient(rectBar);

		pBar->SetWindowPos(NULL, rectBar.left, rectBar.top, rectBar.Width(), rectBar.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
		pBar->StretchPane(nLen, !IsHorizontal());

		// get the rect after stretch
		pBar->GetWindowRect(rectBar);

		IsHorizontal() ? ptOffset.x += rectBar.Width() + nSpacing : ptOffset.y += rectBar.Height() + nSpacing;
	}
}

void CDockingPanesRow::ArrangePanes(CPane* pInitialBar)
{
	ASSERT_VALID(this);

	if (m_lstControlBars.IsEmpty())
	{
		return;
	}

	CRect rectRow;
	GetClientRect(rectRow);

	if (rectRow.IsRectEmpty())
	{
		// the row still is not initialized
		return;
	}

	HDWP hdwp = NULL;
	int nAvailLength = GetAvailableLength();

	// handle single bar
	if (m_lstControlBars.GetCount() == 1)
	{
		if (pInitialBar == NULL)
		{
			pInitialBar = (CPane*) m_lstControlBars.GetHead();
		}
		ASSERT_VALID(pInitialBar);

		if (nAvailLength < 0)
		{
			pInitialBar->StretchPaneDeferWndPos(nAvailLength, hdwp);

			CRect rectBar;
			pInitialBar->GetWindowRect(rectBar);
			m_pParentDockBar->ScreenToClient(rectBar);
			if (IsHorizontal())
			{
				rectBar.OffsetRect(-rectBar.TopLeft().x, -rectBar.TopLeft().y + m_nRowOffset);
			}
			else
			{
				rectBar.OffsetRect(-rectBar.TopLeft().x + m_nRowOffset, -rectBar.TopLeft().y);
			}
			pInitialBar->SetWindowPos(NULL, rectBar.left, rectBar.top, rectBar.Width(), rectBar.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
			return;
		}
	}

	if (pInitialBar != NULL)
	{
		ResolveIntersection(pInitialBar, false, hdwp);
	}

	if (pInitialBar == NULL)
	{
		pInitialBar = (CPane*) m_lstControlBars.GetHead();
	}

	ResolveIntersection(pInitialBar, true, hdwp);

	CPane* pFirstBar = FindFirstVisiblePane(TRUE);
	// how far the first bar is out of row bounds
	int nLeftOuterOffset = GetOutOfBoundsOffset(pFirstBar, TRUE);

	if (nLeftOuterOffset > 0)
	{
		ShiftPanes(pFirstBar, nLeftOuterOffset, TRUE);
	}

	CPane* pLastBar = FindFirstVisiblePane(FALSE);
	int nRightOuterOffset = GetOutOfBoundsOffset(pLastBar, FALSE);

	int nStretchSize = 0;
	if (nRightOuterOffset > 0 && nAvailLength > 0)
	{
		ShiftPanes(pLastBar, -nRightOuterOffset, FALSE);
	}
	else if (nRightOuterOffset > 0 && nAvailLength <= 0)
	{
		nStretchSize = nAvailLength;
		ShiftPanes(pLastBar, -(nRightOuterOffset - abs(nAvailLength)), FALSE);
	}
	else if (nRightOuterOffset < 0)
	{
		// nothing to do
	}

	if (nStretchSize < 0)
	{
		for (POSITION pos = m_lstControlBars.GetTailPosition(); pos != NULL;)
		{
			CPane* pPrevBar = (CPane*) m_lstControlBars.GetPrev(pos);
			ASSERT_VALID(pPrevBar);

			if (!pPrevBar->IsVisible() && !m_bIgnoreBarVisibility)
			{
				continue;
			}

			int nRetSize = pPrevBar->StretchPaneDeferWndPos(nStretchSize, hdwp);

			MovePane(pPrevBar, abs(nStretchSize) - abs(nRetSize), false, hdwp);

			if (nRetSize == nStretchSize)
			{
				break;
			}
			else
			{
				nStretchSize -= nRetSize;
			}
		}
	}
}

CPane* CDockingPanesRow::FindFirstVisiblePane(BOOL bForward)
{
	ASSERT_VALID(this);
	if (m_lstControlBars.IsEmpty())
	{
		return NULL;
	}

	for (POSITION pos = bForward ? m_lstControlBars.GetHeadPosition() : m_lstControlBars.GetTailPosition(); pos != NULL;)
	{
		CPane* pNextBar = (CPane*)(bForward ? m_lstControlBars.GetNext(pos) : m_lstControlBars.GetPrev(pos));

		ASSERT_VALID(pNextBar);
		if (!m_bIgnoreBarVisibility)
		{
			if (pNextBar->IsVisible())
			{
				return pNextBar;
			}
		}
		else
		{
			return pNextBar;
		}
	}

	return NULL;
}

int CDockingPanesRow::GetOutOfBoundsOffset(CPane* pBar, BOOL bLeftTopBound)
{
	ASSERT_VALID(this);

	CRect rectBar;
	CRect rectRow;

	if (pBar == NULL)
	{
		pBar = (CPane* )(bLeftTopBound ? m_lstControlBars.GetHead() : m_lstControlBars.GetTail());
	}

	ASSERT_VALID(pBar);

	pBar->GetWindowRect(rectBar);
	GetWindowRect(rectRow);

	int nBoundOffset = 0;

	// the offset is greater than zero if the bar is out of bounds
	if (IsHorizontal())
	{
		nBoundOffset = bLeftTopBound ?  rectRow.left - rectBar.left : rectBar.right - rectRow.right;
	}
	else
	{
		nBoundOffset = bLeftTopBound ? rectRow.top - rectBar.top : rectBar.bottom - rectRow.bottom;
	}

	return nBoundOffset;
}

void CDockingPanesRow::RepositionPanes(CRect& rectNewParentBarArea, UINT nSide, BOOL bExpand, int nOffset)
{
	ASSERT_VALID(this);

	if (m_lstControlBars.IsEmpty() || GetVisibleCount() == 0)
	{
		return;
	}

	CRect rectNewParentWnd = rectNewParentBarArea;

	ASSERT_VALID(m_pParentDockBar);
	m_pParentDockBar->ClientToScreen(rectNewParentWnd);

	CRect rectRowWnd;
	GetWindowRect(rectRowWnd);

	if (rectRowWnd.IsRectEmpty())
	{
		return;
	}

	int nStretchSize = IsHorizontal() ? rectNewParentWnd.Width() - rectRowWnd.Width() : rectNewParentWnd.Height() - rectRowWnd.Height();

	HDWP hdwp = NULL; //BeginDeferWindowPos(m_lstControlBars.GetCount());

	// handle exclusive bars first
	if (IsExclusiveRow())
	{
		CPane* pBar = (CPane*) m_lstControlBars.GetHead();

		ASSERT_VALID(pBar);
		ASSERT(!pBar->DoesAllowSiblingBars());

		if (IsHorizontal())
		{
			pBar->SetWindowPos(NULL, rectRowWnd.left, rectRowWnd.top, rectNewParentWnd.Width(), rectRowWnd.Height(), SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		}
		else
		{
			pBar->SetWindowPos(NULL, rectRowWnd.left, rectRowWnd.top, rectRowWnd.Width(), rectNewParentWnd.Height(), SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		}

		pBar->RedrawWindow();
		return;
	}

	int nAvailSpace = GetAvailableLength(TRUE);
	bool bResizeBars = nAvailSpace < 0 || !bExpand && nAvailSpace >= 0 && nAvailSpace < abs(nStretchSize);

	if (bResizeBars)
	{
		if (!bExpand)
		{
			// there is some available space on the row
			// we need to move control bars first and then stretch

			bool bTopLeftSide = (nSide == WMSZ_TOP || nSide == WMSZ_LEFT);

			int nActualStretchSize = nStretchSize;

			if (nAvailSpace >= 0)
			{
				int nSign = nStretchSize < 0 ?(-1) : 1;
				int nOutOfBoundOffset = GetOutOfBoundsOffset(NULL, bTopLeftSide);

				int nOffsetToShift = nAvailSpace - abs(nOutOfBoundOffset);

				ShiftPanes(NULL, nSign * nOffsetToShift, bTopLeftSide);
				nActualStretchSize = (abs(nStretchSize) - nAvailSpace) * nSign;
			}

			for (POSITION pos = m_lstControlBars.GetTailPosition(); pos != NULL;)
			{
				CPane* pPrevBar = (CPane*) m_lstControlBars.GetPrev(pos);
				ASSERT_VALID(pPrevBar);

				if (!pPrevBar->IsVisible() && !m_bIgnoreBarVisibility)
				{
					continue;
				}

				int nRetSize = pPrevBar->StretchPaneDeferWndPos(nActualStretchSize, hdwp);

				MovePane(pPrevBar, abs(nActualStretchSize) - abs(nRetSize), bTopLeftSide, hdwp);

				if (nRetSize == nActualStretchSize)
				{
					break;
				}
				else
				{
					nActualStretchSize -= nRetSize;
				}
			}

			return;
		}
		else
		{
			int nActualStretchSize = nStretchSize;
			for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
			{
				CPane* pNextBar = (CPane*) m_lstControlBars.GetNext(pos);
				ASSERT_VALID(pNextBar);

				if (!pNextBar->IsVisible() && !m_bIgnoreBarVisibility)
				{
					continue;
				}

				int nRetSize = pNextBar->StretchPaneDeferWndPos(nActualStretchSize, hdwp);

				if (nRetSize != 0 && pos != NULL)
				{
					for (POSITION posMovedBars = pos; posMovedBars != NULL;)
					{
						CPane* pBarToMove = (CPane*) m_lstControlBars.GetNext(posMovedBars);
						ASSERT_VALID(pBarToMove);

						if (!pBarToMove->IsVisible() && !m_bIgnoreBarVisibility)
						{
							continue;
						}

						MovePane(pBarToMove, nRetSize, true, hdwp);
					}
				}

				nActualStretchSize -= nRetSize;

				if (nActualStretchSize <= 0)
				{
					break;
				}
			}
		}
	}

	// adlust control bars(first and last) to the row area and try to move
	// them as close as it possible to their virtual rectangle

	if (IsHorizontal())
	{
		rectRowWnd.left = rectNewParentWnd.left;
		rectRowWnd.right = rectNewParentWnd.right;
	}
	else
	{
		rectRowWnd.top = rectNewParentWnd.top;
		rectRowWnd.bottom = rectNewParentWnd.bottom;
	}

	// check the first and last control bars
	CPane* pBarFirst = FindFirstVisiblePane(TRUE);
	ASSERT_VALID(pBarFirst);

	AdjustPaneToRowArea(pBarFirst, rectRowWnd, hdwp);

	CPane* pBarLast = FindFirstVisiblePane(FALSE);
	ASSERT_VALID(pBarLast);

	if (pBarFirst != pBarLast)
	{
		AdjustPaneToRowArea(pBarLast, rectRowWnd, hdwp);
	}

	if (nSide != (UINT)-1 && bExpand && GetAvailableLength(TRUE) + nStretchSize > 0)
	{
		CRect rectNextControlBar;
		CRect rectNextControlBarVirt;

		for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
		{
			CPane* pNextBar = (CPane*) m_lstControlBars.GetNext(pos);
			ASSERT_VALID(pNextBar);

			if (!pNextBar->IsVisible() && !m_bIgnoreBarVisibility)
			{
				continue;
			}

			pNextBar->GetWindowRect(&rectNextControlBar);
			pNextBar->GetVirtualRect(rectNextControlBarVirt);

			if (rectNextControlBar != rectNextControlBarVirt)
			{
				// always try to move to the virtual rect's direction
				// if the bar is currently on the left side of its virtual rect
				// move it forward, otherwise - backward
				bool bMoveBackward = (IsHorizontal() && rectNextControlBar.left > rectNextControlBarVirt.left) ||
					(!IsHorizontal() && rectNextControlBar.top > rectNextControlBarVirt.top);

				int nOffsetToMove = 0;
				if (IsHorizontal() &&(nSide == WMSZ_LEFT || nSide == WMSZ_RIGHT))
				{
					nOffsetToMove = min(abs(nOffset), abs(rectNextControlBarVirt.left - rectNextControlBar.left));
				}
				else if (!IsHorizontal() &&(nSide == WMSZ_TOP || nSide == WMSZ_BOTTOM))
				{
					nOffsetToMove = min(abs(nOffset), abs(rectNextControlBarVirt.top - rectNextControlBar.top));
				}
				int nSaveOffset = nOffsetToMove;
				if (IsEnoughSpaceToMove(pNextBar, !bMoveBackward, nSaveOffset))
				{
					MovePane(pNextBar, nOffsetToMove, !bMoveBackward, hdwp);
				}
			}
		}
	}
}

void CDockingPanesRow::AdjustPaneToRowArea(CPane* pBar, const CRect& rectRow, HDWP& hdwp)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);

	CRect rectBarWnd;
	pBar->GetWindowRect(rectBarWnd);

	CPoint ptOffset(0, 0);
	if (IsHorizontal())
	{
		if (rectBarWnd.left < rectRow.left)
		{
			ptOffset = CPoint(rectRow.left - rectBarWnd.left, 0);
			MovePane(pBar, ptOffset, FALSE, hdwp);
		}

		if (rectBarWnd.right > rectRow.right)
		{
			ptOffset = CPoint(rectRow.right - rectBarWnd.right, 0);
			MovePane(pBar, ptOffset, FALSE, hdwp);
		}
	}
	else
	{
		if (rectBarWnd.top < rectRow.top)
		{
			ptOffset = CPoint(0, rectRow.top - rectBarWnd.top);
			MovePane(pBar, ptOffset, FALSE, hdwp);
		}

		if (rectBarWnd.bottom > rectRow.bottom)
		{
			ptOffset = CPoint(0, rectRow.bottom - rectBarWnd.bottom);
			MovePane(pBar, ptOffset, FALSE, hdwp);
		}
	}
}

BOOL CDockingPanesRow::ReplacePane(CPane* pBarOld, CPane* pBarNew)
{
	ASSERT_VALID(this);

	POSITION pos = m_lstControlBars.Find(pBarOld);
	if (pos != NULL)
	{
		m_lstControlBars.InsertAfter(pos, pBarNew);
		m_lstControlBars.RemoveAt(pos);
		return TRUE;
	}
	return FALSE;
}

BOOL CDockingPanesRow::IsExclusiveRow() const
{
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CPane* pNextBar = (CPane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pNextBar);
		if (!pNextBar->DoesAllowSiblingBars())
		{
			return TRUE;
		}
	}

	return FALSE;
}

int CDockingPanesRow::CalcLastPaneOffset()
{
	if (m_lstControlBars.IsEmpty())
	{
		return 0;
	}

	CPane* pLastBar = (CPane*) m_lstControlBars.GetTail();
	ASSERT_VALID(pLastBar);

	CRect rect;
	pLastBar->GetWindowRect(rect);

	m_pParentDockBar->ScreenToClient(rect);

	return IsHorizontal() ? rect.right : rect.bottom;
}

void CDockingPanesRow::RedrawAll()
{
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CPane* pNextBar = (CPane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pNextBar);
		pNextBar->RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
	}
}

void CDockingPanesRow::GetGroupFromPane(CPane* pBar, CObList& lst)
{
	POSITION pos = m_lstControlBars.Find(pBar);
	if (pos == NULL)
	{
		return;
	}

	POSITION posGrp = NULL;

	// find first control bar in group
	for (posGrp = pos; posGrp != NULL;)
	{
		CPane* pPrevBar = (CPane*) m_lstControlBars.GetPrev(posGrp);
		ASSERT_VALID(pPrevBar);
		if (pPrevBar->m_bFirstInGroup)
		{
			if (posGrp == NULL)
			{
				posGrp =m_lstControlBars.GetHeadPosition();
			}
			else
			{
				m_lstControlBars.GetNext(posGrp);
			}
			break;
		}
	}

	// collect all bars in the group
	while (posGrp != NULL)
	{
		CPane* pNextBar = (CPane*) m_lstControlBars.GetNext(posGrp);
		ASSERT_VALID(pNextBar);
		lst.AddTail(pNextBar);

		if (pNextBar->m_bLastInGroup)
		{
			break;
		}
	}
}

void CDockingPanesRow::BeginTrans()
{
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CPane* pNextBar = (CPane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pNextBar);
		if (!pNextBar->IsVisible() && !m_bIgnoreBarVisibility)
		{
			continue;
		}
		pNextBar->FillWindowRect();
	}
}

void CDockingPanesRow::CommitTrans()
{
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CPane* pNextBar = (CPane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pNextBar);
		if (!pNextBar->IsVisible() && !m_bIgnoreBarVisibility)
		{
			continue;
		}
		CRect rect = pNextBar->GetPaneRect();
		CRect rectWnd;
		pNextBar->GetWindowRect(rectWnd);

		if (rect == rectWnd)
		{
			continue;
		}

		pNextBar->GetParent()->ScreenToClient(rect);
		pNextBar->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	}
}

int CDockingPanesRow::GetAvailableLengthRect()
{
	CRect rectRow;
	GetClientRect(rectRow);

	int nTotalBarLength = 0;
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CPane* pNextBar = (CPane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pNextBar);
		if (!pNextBar->IsVisible() && !m_bIgnoreBarVisibility)
		{
			continue;
		}

		CRect rectWnd = pNextBar->GetPaneRect();
		nTotalBarLength += IsHorizontal() ? rectWnd.Width() : rectWnd.Height();
	}

	// debug variable
	int nAvailableLength = IsHorizontal() ? rectRow.Width() - nTotalBarLength : rectRow.Height() - nTotalBarLength;

	return nAvailableLength;
}

int CDockingPanesRow::StretchPaneRect(CPane* pBar, int nStretchSize)
{
	// the bar is stretched - calculate how far it can be expanded and do not
	// exceed its original size
	int nAvailExpandSize = pBar->GetAvailableExpandSize();
	int nAvailStretchSize = pBar->GetAvailableStretchSize();

	int nActualStretchSize = 0;
	if (nStretchSize > 0)
	{
		if (nAvailExpandSize == 0)
		{
			return 0;
		}
		// the bar is expanded
		nActualStretchSize = nAvailExpandSize > nStretchSize ? nStretchSize : nAvailExpandSize;
	}
	else
	{
		nActualStretchSize = nAvailStretchSize < abs(nStretchSize) ? -nAvailStretchSize : nStretchSize;
	}

	CRect rect = pBar->GetPaneRect();

	if (IsHorizontal())
	{
		rect.right += nActualStretchSize;
	}
	else
	{
		rect.bottom += nActualStretchSize;
	}

	if (abs(nActualStretchSize) > 0)
	{
		pBar->SetPaneRect(rect);
	}

	return nActualStretchSize;
}

void CDockingPanesRow::ArrangePanesRect(CPane* pInitialBar)
{
	ASSERT_VALID(this);

	if (m_lstControlBars.IsEmpty())
	{
		return;
	}

	CRect rectRow;
	GetClientRect(rectRow);

	if (rectRow.IsRectEmpty())
	{
		// the row still is not initialized
		return;
	}

	HDWP hdwp = NULL;
	int nAvailLength = GetAvailableLengthRect();

	// handle single bar
	if (m_lstControlBars.GetCount() == 1)
	{
		if (pInitialBar == NULL)
		{
			pInitialBar = (CPane*) m_lstControlBars.GetHead();
		}
		ASSERT_VALID(pInitialBar);

		if (nAvailLength < 0)
		{
			StretchPaneRect(pInitialBar, nAvailLength);
			CRect rectBar = pInitialBar->GetPaneRect();
			pInitialBar->SetPaneRect(rectBar);
			return;
		}
	}

	if (pInitialBar != NULL)
	{
		ResolveIntersectionRect(pInitialBar, false);
	}

	if (pInitialBar == NULL)
	{
		pInitialBar = (CPane*) m_lstControlBars.GetHead();
	}

	ResolveIntersectionRect(pInitialBar, true);

	CPane* pFirstBar = FindFirstVisiblePane(TRUE);
	// how far the first bar is out of row bounds
	int nLeftOuterOffset = GetOutOfBoundsOffsetRect(pFirstBar, TRUE);

	if (nLeftOuterOffset > 0)
	{
		ShiftPanesRect(pFirstBar, nLeftOuterOffset, TRUE);
	}

	CPane* pLastBar = FindFirstVisiblePane(FALSE);
	int nRightOuterOffset = GetOutOfBoundsOffsetRect(pLastBar, FALSE);

	int nStretchSize = 0;
	if (nRightOuterOffset > 0 && nAvailLength > 0)
	{
		ShiftPanesRect(pLastBar, -nRightOuterOffset, FALSE);
	}
	else if (nRightOuterOffset > 0 && nAvailLength <= 0)
	{
		nStretchSize = nAvailLength;
		ShiftPanesRect(pLastBar, -(nRightOuterOffset - abs(nAvailLength)), FALSE);
	}
	else if (nRightOuterOffset < 0)
	{
		// nothing to do
	}

	if (nStretchSize < 0)
	{
		for (POSITION pos = m_lstControlBars.GetTailPosition(); pos != NULL;)
		{
			CPane* pPrevBar = (CPane*) m_lstControlBars.GetPrev(pos);
			ASSERT_VALID(pPrevBar);

			if (!pPrevBar->IsVisible() && !m_bIgnoreBarVisibility)
			{
				continue;
			}

			int nRetSize = pPrevBar->StretchPaneDeferWndPos(nStretchSize, hdwp);

			MovePaneRect(pPrevBar, abs(nStretchSize) - abs(nRetSize), false);

			if (nRetSize == nStretchSize)
			{
				break;
			}
			else
			{
				nStretchSize -= nRetSize;
			}
		}
	}
}

void CDockingPanesRow::ResolveIntersectionRect(CPane* pBar, bool bForward)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);

	POSITION posStart = m_lstControlBars.Find(pBar);

	CRect rectBarWnd;
	rectBarWnd.SetRectEmpty();

	CRect rectRowWnd;
	GetWindowRect(rectRowWnd);

	CRect rectIntersect;
	CRect rectMovedBar;

	for (POSITION pos = posStart; pos != NULL;)
	{
		CPane* pNextBar = (CPane*)(bForward ? m_lstControlBars.GetNext(pos) : m_lstControlBars.GetPrev(pos));

		ASSERT_VALID(pNextBar);

		if (!pNextBar->IsVisible() && !m_bIgnoreBarVisibility)
		{
			continue;
		}

		rectBarWnd = pNextBar->GetPaneRect();

		CPane* pMovedBar = NULL;
		POSITION posSave = NULL;
		for (POSITION posMoved = pos; posMoved != NULL;)
		{
			posSave = posMoved;
			pMovedBar = (CPane*)(bForward ? m_lstControlBars.GetNext(posMoved) : m_lstControlBars.GetPrev(posMoved));

			if (pMovedBar->IsVisible() || m_bIgnoreBarVisibility)
			{
				break;
			}
		}

		if (pMovedBar == NULL)
		{
			break;
		}

		rectMovedBar = pMovedBar->GetPaneRect();

		if (bForward &&(IsHorizontal() && rectMovedBar.left > rectBarWnd.right || !IsHorizontal() && rectMovedBar.top > rectBarWnd.bottom) || !bForward &&
			(IsHorizontal() && rectMovedBar.right < rectBarWnd.left || !IsHorizontal() && rectMovedBar.bottom < rectBarWnd.top))
		{
			pos = posSave;
			continue;
		}

		int nMoveOffset = 0;
		if (bForward)
		{
			nMoveOffset = IsHorizontal() ? rectBarWnd.right - rectMovedBar.left : rectBarWnd.bottom - rectMovedBar.top;
		}
		else
		{
			nMoveOffset = IsHorizontal() ? rectMovedBar.right - rectBarWnd.left : rectMovedBar.bottom - rectBarWnd.top;
		}

		MovePaneRect(pMovedBar, nMoveOffset, bForward);
		pos = posSave;
	}
}

int CDockingPanesRow::GetOutOfBoundsOffsetRect(CPane* pBar, BOOL bLeftTopBound)
{
	ASSERT_VALID(this);

	CRect rectBar;
	CRect rectRow;

	if (pBar == NULL)
	{
		pBar = (CPane* )(bLeftTopBound ? m_lstControlBars.GetHead() : m_lstControlBars.GetTail());
	}

	ASSERT_VALID(pBar);

	rectBar = pBar->GetPaneRect();
	GetWindowRect(rectRow);

	int nBoundOffset = 0;

	// the offset is greater than zero if the bar is out of bounds
	if (IsHorizontal())
	{
		nBoundOffset = bLeftTopBound ?  rectRow.left - rectBar.left : rectBar.right - rectRow.right;
	}
	else
	{
		nBoundOffset = bLeftTopBound ? rectRow.top - rectBar.top : rectBar.bottom - rectRow.bottom;
	}

	return nBoundOffset;
}

void CDockingPanesRow::ShiftPanesRect(CPane* pControlBar, int nOffset, BOOL bForward)
{
	ASSERT_VALID(this);
	ASSERT(!m_lstControlBars.IsEmpty());

	if (nOffset == 0)
	{
		return;
	}

	POSITION pos = NULL;

	if (pControlBar != NULL)
	{
		pos = m_lstControlBars.Find(pControlBar);
	}
	else
	{
		pos = bForward ? m_lstControlBars.GetHeadPosition() : m_lstControlBars.GetTailPosition();
		pControlBar = (CPane*) m_lstControlBars.GetAt(pos);
	}

	int nMoveOffset = nOffset;

	CRect rectBar; rectBar.SetRectEmpty();

	while (pos != NULL)
	{
		CPane* pNextBar = (CPane*)(bForward ? m_lstControlBars.GetNext(pos) : m_lstControlBars.GetPrev(pos));

		if (!pNextBar->IsVisible() && !m_bIgnoreBarVisibility)
		{
			continue;
		}

		ASSERT_VALID(pNextBar);
		CRect rectNextBar = pNextBar->GetPaneRect();

		if (pNextBar != pControlBar && !rectBar.IsRectEmpty())
		{
			if (IsHorizontal())
			{
				nMoveOffset -= bForward ? rectNextBar.left - rectBar.right : rectNextBar.right - rectBar.left;
			}
			else
			{
				nMoveOffset -= bForward ? rectNextBar.top - rectBar.bottom : rectNextBar.bottom - rectBar.top;
			}
		}

		if (nMoveOffset <= 0 && bForward || nMoveOffset >= 0 && !bForward)
		{
			break;
		}

		rectBar = rectNextBar;
		IsHorizontal() ? rectNextBar.OffsetRect(nMoveOffset, 0) : rectNextBar.OffsetRect(0, nMoveOffset);

		pNextBar->SetPaneRect(rectNextBar);
	}
}

void CDockingPanesRow::MovePaneRect(CPane* pControlBar, int nOffset, bool bForward)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pControlBar);

	if (nOffset == 0)
	{
		return;
	}

	CRect rectBarWnd = pControlBar->GetPaneRect();

	if (IsHorizontal())
	{
		rectBarWnd.OffsetRect(bForward ? nOffset : -nOffset, 0);
	}
	else
	{
		rectBarWnd.OffsetRect(0, bForward ? nOffset : -nOffset);
	}

	pControlBar->SetPaneRect(rectBarWnd);
}



