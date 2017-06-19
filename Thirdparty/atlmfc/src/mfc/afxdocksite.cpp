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

#include "afxframewndex.h"
#include "afxmdiframewndex.h"
#include "afxoleipframewndex.h"
#include "afxoledocipframewndex.h"
#include "afxmdichildwndex.h"
#include "afxolecntrframewndex.h"

#include "afxpane.h"
#include "afxdockingpanesrow.h"
#include "afxrebar.h"

#include "afxglobalutils.h"
#include "afxdocksite.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CDockSite, CBasePane)

/////////////////////////////////////////////////////////////////////////////
// CDockSite

CDockSite::CDockSite() : m_nDockBarID(0)
{
}

CDockSite::~CDockSite()
{
	while (!m_lstDockBarRows.IsEmpty())
	{
		delete m_lstDockBarRows.RemoveHead();
	}
}

BEGIN_MESSAGE_MAP(CDockSite, CBasePane)
	//{{AFX_MSG_MAP(CDockSite)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_CONTEXTMENU()
	ON_WM_NCDESTROY()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDockSite message handlers

BOOL CDockSite::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, DWORD dwControlBarStyle, CCreateContext* pContext)
{
	ASSERT_VALID(this);
	return CDockSite::CreateEx(0, dwStyle, rect, pParentWnd, dwControlBarStyle, pContext);
}

BOOL CDockSite::CreateEx(DWORD dwStyleEx, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, DWORD dwControlBarStyle, CCreateContext* pContext)
{
	ASSERT_VALID(this);

	DWORD dwEnableAlignment = GetEnabledAlignment();
	EnableDocking(dwEnableAlignment | dwStyle);

	SetPaneAlignment(dwStyle);

	dwStyle |= WS_CHILDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPED;
	dwStyleEx = WS_EX_LEFT;

	// Align the bar along borders; initially, create the dock bar with zero height/width
	CRect rectDockBar = rect;

	CRect rectParent;
	pParentWnd->GetClientRect(&rectParent);

	rectDockBar = rectParent;

	switch (GetCurrentAlignment())
	{
	case CBRS_ALIGN_LEFT:
		rectDockBar.right = 0;
		m_nDockBarID = AFX_IDW_DOCKBAR_LEFT;
		break;

	case CBRS_ALIGN_RIGHT:
		rectDockBar.left = rectParent.right;
		m_nDockBarID = AFX_IDW_DOCKBAR_RIGHT;
		break;

	case CBRS_ALIGN_TOP:
		rectDockBar.bottom = rectParent.top;
		m_nDockBarID = AFX_IDW_DOCKBAR_TOP;
		break;

	case CBRS_ALIGN_BOTTOM:
		rectDockBar.top  = rectParent.bottom;
		m_nDockBarID = AFX_IDW_DOCKBAR_BOTTOM;
		break;
	}

	m_dwControlBarStyle = dwControlBarStyle;
	m_pDockSite = pParentWnd;

	return CWnd::CreateEx(dwStyleEx, afxGlobalData.RegisterWindowClass(_T("Afx:DockPane")), NULL, dwStyle, rectDockBar, pParentWnd, m_nDockBarID, pContext);
}

void CDockSite::AlignDockSite(const CRect& rectToAlignBy, CRect& rectResult, BOOL bMoveImmediately)
{
	ASSERT_VALID(this);
	if (rectResult.IsRectEmpty())
	{
		GetWindowRect(rectResult);
	}

	CRect rectOld;
	GetWindowRect(rectOld);

	int nCurrWidth = rectResult.Width();
	int nCurrHeight = rectResult.Height();

	switch (GetCurrentAlignment())
	{
	case CBRS_ALIGN_LEFT:
		rectResult.TopLeft() = rectToAlignBy.TopLeft();
		rectResult.bottom = rectResult.top + rectToAlignBy.Height();
		rectResult.right = rectResult.left + nCurrWidth;
		break;

	case CBRS_ALIGN_TOP:
		rectResult.TopLeft() = rectToAlignBy.TopLeft();
		rectResult.right = rectResult.left + rectToAlignBy.Width();
		rectResult.bottom = rectResult.top + nCurrHeight;
		break;

	case CBRS_ALIGN_RIGHT:
		rectResult.BottomRight() = rectToAlignBy.BottomRight();
		rectResult.top = rectResult.bottom - rectToAlignBy.Height();
		rectResult.left = rectResult.right - nCurrWidth;
		break;
	case CBRS_ALIGN_BOTTOM:
		rectResult.BottomRight() = rectToAlignBy.BottomRight();
		rectResult.left = rectResult.right - rectToAlignBy.Width();
		rectResult.top = rectResult.bottom - nCurrHeight;
		break;
	}

	if (rectResult != rectOld && bMoveImmediately)
	{
		CRect rectNew = rectResult;
		ASSERT_VALID(GetParent());
		GetParent()->ScreenToClient(rectNew);

		OnSetWindowPos(&wndBottom, rectNew, SWP_NOACTIVATE | SWP_NOZORDER);
	}
}

// Moves control bar within row; floats the bar or moves it to an adjustent row
// if the bar' virtual rectangle is being moved out of row beyond a limit
BOOL CDockSite::MovePane(CPane* pControlBar, UINT /*nFlags*/, CPoint ptOffset)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pControlBar);

	CDockingPanesRow* pRow = pControlBar->GetDockSiteRow();
	ASSERT_VALID(pRow);

	CRect rectVirtual;
	pControlBar->GetVirtualRect(rectVirtual);

	// where the virtual rectangle will be if it's moved according to ptOffset
	rectVirtual.OffsetRect(ptOffset);

	CPoint ptMouse;
	GetCursorPos(&ptMouse);

	CRect rectRow;
	pRow->GetWindowRect(rectRow);

	CPoint ptDelta(0, 0);

	// check whether the control bar should change its state from docked to floated
	CBasePane* pDockBar = NULL;

	if (pControlBar->IsChangeState(15, &pDockBar))
	{
		pControlBar->UpdateVirtualRect(ptOffset);
		pControlBar->GetVirtualRect(rectVirtual);
		pControlBar->FloatPane(rectVirtual, DM_MOUSE);
		return TRUE; // indicates that the bar was floated and shouldn't be moved anymore within the dock bar
	}

	bool bOuterRow = false;
	CDockingPanesRow* pNextRow = RowFromPoint(rectVirtual.CenterPoint(), bOuterRow);

	int nBaseLineOffset = 0;
	int nOffsetLimit = 0;

	if (IsHorizontal())
	{
		nBaseLineOffset = min(rectRow.bottom - rectVirtual.bottom, rectRow.top - rectVirtual.top);
		nOffsetLimit = rectVirtual.Height() * 2 / 3; // / 2;
	}
	else
	{
		nBaseLineOffset = min(rectRow.right - rectVirtual.right, rectRow.left - rectVirtual.left);
		nOffsetLimit = rectVirtual.Width() * 2 /3 ; // / 2;
	}

	if (abs(nBaseLineOffset) > nOffsetLimit)
	{
		if (pRow->GetPaneCount() > 1  && nBaseLineOffset < pRow->GetRowHeight())
		{
			// the bar should be put on the separate row, find a position to insert the row
			POSITION pos = m_lstDockBarRows.Find(pRow);
			ENSURE(pos != NULL);

			if (nBaseLineOffset < 0) // moving down - find the next visible row
			{
				// the new row should be inserted before next visible row
				FindNextVisibleRow(pos);
			}
			// otherwise the new row will be inserted before the current row
			//(that's visible for sure) by AddRow(it inserts a row before spec. pos).

			pRow->RemovePane(pControlBar);
			CDockingPanesRow* pNewRow = AddRow(pos, IsHorizontal() ? rectVirtual.Height() : rectVirtual.Width());
			pNewRow->AddPaneFromRow(pControlBar, DM_MOUSE);

			return FALSE;
		}
		else if (pRow != pNextRow && pNextRow != NULL)
		{
			ASSERT_VALID(pNextRow);
			//the bar is moved from the separate row to adjustent row(if exist)

			SetRedraw (FALSE);

			if (pRow->IsExclusiveRow())
			{
				SwapRows(pNextRow, pRow);
			}
			else
			{
				if (pNextRow->IsExclusiveRow())
				{
					SwapRows(pRow, pNextRow);
				}
				else
				{
					pRow->RemovePane(pControlBar);
					pNextRow->AddPaneFromRow(pControlBar, DM_MOUSE);
				}
			}

			pControlBar->m_bDisableMove = true;

			SetRedraw (TRUE);
			RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);

			return FALSE;
		}

	}
	// just move the bar within the row
	if (abs(nBaseLineOffset) < rectRow.Height())
	{
		HDWP hdwp = BeginDeferWindowPos(pRow->GetPaneCount());
		pRow->MovePane(pControlBar, ptOffset, TRUE, hdwp);
		EndDeferWindowPos(hdwp);
		return FALSE;
	}
	return FALSE;
}

CDockingPanesRow* CDockSite::FindNextVisibleRow(POSITION& pos, BOOL bForward)
{
	if (m_lstDockBarRows.IsEmpty())
	{
		pos = NULL;
		return NULL;
	}

	if (pos == NULL)
	{
		pos = bForward  ? m_lstDockBarRows.GetHeadPosition() : m_lstDockBarRows.GetTailPosition();
	}
	else
	{
		// we need to skip to the next / prev row from the current position
		bForward ? m_lstDockBarRows.GetNext(pos) : m_lstDockBarRows.GetPrev(pos);
	}

	while (pos != NULL)
	{
		POSITION posSave = pos;
		CDockingPanesRow* pRow = (CDockingPanesRow*) (bForward ? m_lstDockBarRows.GetNext(pos) : m_lstDockBarRows.GetPrev(pos));
		ASSERT_VALID(pRow);

		if (pRow->IsVisible())
		{
			pos = posSave;
			return pRow;
		}
	}

	return NULL;
}

void CDockSite::CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType)
{
	ASSERT_VALID(this);

	CWnd::CalcWindowRect(lpClientRect, nAdjustType);
}

void CDockSite::DockPane(CPane* pControlBar, AFX_DOCK_METHOD dockMethod, LPCRECT lpRect)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pControlBar);

	CRect rectDockArea; rectDockArea.SetRectEmpty();
	if (lpRect != NULL)
	{
		rectDockArea = lpRect;
	}

	BOOL bVertDock = !IsHorizontal();
	CSize szBarSize = pControlBar->CalcFixedLayout(FALSE, !bVertDock);

	if (!m_lstControlBars.Find(pControlBar))
	{
		CDockingPanesRow* pRowToDock = NULL;
		bool bOuterRow = false;

		if (dockMethod == DM_MOUSE)
		{
			// calculate from which side the control bar is coming, using mouse cursor position.
			// the default bar width(for side bars) and height(for top/bottom bars)
			// is 30 for this example

			CPoint ptMouse;
			GetCursorPos(&ptMouse);

			CRect rectDockBar;
			GetWindowRect(&rectDockBar);

			// get pointer to the row on which the bar should be placed
			pRowToDock = RowFromPoint(ptMouse, bOuterRow);
		}
		else if (dockMethod == DM_DBL_CLICK || dockMethod == DM_RECT)
		{
			if (dockMethod == DM_DBL_CLICK && m_lstDockBarRows.Find(pControlBar->m_recentDockInfo.m_pRecentDockBarRow) != NULL)
			{
				pRowToDock = pControlBar->m_recentDockInfo.m_pRecentDockBarRow;
			}
			else
			{
				int nRowCount = (int) m_lstDockBarRows.GetCount();

				if (CDockingManager::m_bRestoringDockState)
				{
					if (pControlBar->m_recentDockInfo.m_nRecentRowIndex > nRowCount - 1)
					{
						for (int i = 0;
							i < pControlBar->m_recentDockInfo.m_nRecentRowIndex - nRowCount + 1; i++)
						{
							AddRow(NULL, bVertDock ? szBarSize.cx : szBarSize.cy);
						}
					}

					POSITION posRow = m_lstDockBarRows.FindIndex(pControlBar->m_recentDockInfo.m_nRecentRowIndex);
					pRowToDock = (CDockingPanesRow*) m_lstDockBarRows.GetAt(posRow);
				}
				else
				{
					if (pControlBar->m_recentDockInfo.m_nRecentRowIndex < nRowCount && dockMethod == DM_DBL_CLICK)
					{
						POSITION pos = m_lstDockBarRows.FindIndex(pControlBar->m_recentDockInfo.m_nRecentRowIndex);
						pRowToDock = (CDockingPanesRow*) m_lstDockBarRows.GetAt(pos);
						bOuterRow = true;
					}
					else if (dockMethod == DM_DBL_CLICK && !pControlBar->m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect.IsRectEmpty())
					{
						pRowToDock = FindRowByRect(pControlBar->m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect);
					}
					else if (dockMethod == DM_RECT && lpRect != NULL)
					{
						pRowToDock = FindRowByRect(lpRect);
					}
				}

				if (pRowToDock == NULL)
				{
					AddRow(NULL, bVertDock ? szBarSize.cx : szBarSize.cy);
					pRowToDock = (CDockingPanesRow*) m_lstDockBarRows.GetTail();
				}
			}

			ASSERT_VALID(pRowToDock);
			rectDockArea = &pControlBar->m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect;
			ClientToScreen(rectDockArea);
		}

		// if the bar is being placed on the exclusive row
		//(with menu bar, for example) we should create a new row, put the
		// bar on new row and put this row after/before the exclusive row
		POSITION posSwapRow = NULL;
		if (pRowToDock != NULL && pRowToDock->IsExclusiveRow() || pRowToDock != NULL && !pControlBar->DoesAllowSiblingBars() && !pRowToDock->IsEmpty())
		{
			posSwapRow = m_lstDockBarRows.Find(pRowToDock);
			ENSURE(posSwapRow != NULL);
			pRowToDock = NULL;
		}

		if (pRowToDock == NULL)
		{
			POSITION posNewBar = NULL;

			if (posSwapRow != NULL)
			{
				// the bar is inserted before the specified position in AddRow
				posNewBar = posSwapRow;
				if (!bOuterRow)
				{
					m_lstDockBarRows.GetNext(posNewBar);
				}
			}
			else
			{
				posNewBar = bOuterRow ? m_lstDockBarRows.GetHeadPosition() : NULL;
			}

			pRowToDock = AddRow(posNewBar, bVertDock ? szBarSize.cx : szBarSize.cy);
		}

		ASSERT_VALID(pRowToDock);

		// the bar should be placed on the existing row or new row
		pRowToDock->AddPane(pControlBar, dockMethod, rectDockArea);
		// if the bar suudently changed its size we need to resize the row again
		CSize sizeBarNew = pControlBar->CalcFixedLayout(FALSE, !bVertDock);
		if (sizeBarNew != szBarSize)
		{
			ResizeRow(pRowToDock, bVertDock ? sizeBarNew.cx : sizeBarNew.cy);
		}

		m_lstControlBars.AddTail(pControlBar);
		AdjustDockingLayout();
		ShowWindow(SW_SHOW);
	}
}

CDockingPanesRow* CDockSite::FindRowByRect(CRect rectRow)
{
	bool b;
	CPoint pt = rectRow.TopLeft();
	ClientToScreen(&pt);
	return RowFromPoint(pt, b);
}

BOOL CDockSite::DockPaneLeftOf(CPane* pBarToDock, CPane* pTargetBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBarToDock);
	ASSERT_VALID(pTargetBar);

	CDockingPanesRow* pTargetRow = RowFromPane(pTargetBar);

	if (pTargetRow == NULL)
	{
		return FALSE;
	}

	CRect rectTargetBar;
	pTargetBar->GetWindowRect(rectTargetBar);
	ScreenToClient(rectTargetBar);

	BOOL bVertDock = !IsHorizontal();
	CSize szBarSize = pBarToDock->CalcFixedLayout(FALSE, !bVertDock);

	CRect rectFinal;

	if (IsHorizontal())
	{
		rectFinal.SetRect(rectTargetBar.left - szBarSize.cx - 10, rectTargetBar.top, rectTargetBar.left - 10, rectTargetBar.bottom);
	}
	else
	{
		rectFinal.SetRect(rectTargetBar.left, rectTargetBar.top - szBarSize.cy - 10, rectTargetBar.right, rectTargetBar.top - 10);
	}

	pBarToDock->PrepareToDock(this, DM_RECT);
	ClientToScreen(rectFinal);
	pTargetRow->m_bIgnoreBarVisibility = TRUE;
	pTargetRow->AddPane(pBarToDock, DM_RECT, &rectFinal);

	POSITION pos = m_lstControlBars.Find(pTargetBar);
	ENSURE(pos != NULL);

	m_lstControlBars.InsertBefore(pos, pBarToDock);

	AdjustDockingLayout();
	FixupVirtualRects();
	pTargetRow->m_bIgnoreBarVisibility = FALSE;

	return TRUE;
}

void CDockSite::RemovePane(CPane* pControlBar, AFX_DOCK_METHOD /*dockMethod*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pControlBar);

	if (!m_lstControlBars.IsEmpty())
	{
		POSITION pos = m_lstControlBars.Find(pControlBar);
		if (pos != NULL)
		{
			m_lstControlBars.RemoveAt(pos);
			// we need to reposition bars according to the new situation
			// 1. expand bars that were stretched due to presence of this bar
			// 2. remove empty rows

			CDockingPanesRow* pRow = pControlBar->GetDockSiteRow();
			if (pRow != NULL)
			{
				pRow->RemovePane(pControlBar);
			}
		}
	}
}

void CDockSite::FixupVirtualRects()
{
	ASSERT_VALID(this);

	for (POSITION pos = m_lstDockBarRows.GetHeadPosition(); pos != NULL;)
	{
		CDockingPanesRow* pNextRow = (CDockingPanesRow*) m_lstDockBarRows.GetNext(pos);
		ASSERT_VALID(pNextRow);

		pNextRow->FixupVirtualRects(false);
	}
}

void CDockSite::RepositionPanes(CRect& rectNewClientArea)
{
	ASSERT_VALID(this);

	CRect rectOldArea;
	GetClientRect(rectOldArea);
	CSize sizeNew = rectNewClientArea.Size();
	CSize sizeOld = rectOldArea.Size();
	if (sizeNew != sizeOld)
	{
		int nHorzOffset = sizeNew.cx - sizeOld.cx;
		int nVertOffset = sizeNew.cy - sizeOld.cy;

		for (POSITION pos = m_lstDockBarRows.GetHeadPosition(); pos != NULL;)
		{
			CDockingPanesRow* pNextRow = (CDockingPanesRow*) m_lstDockBarRows.GetNext(pos);
			ASSERT_VALID(pNextRow);
			if (nHorzOffset != 0)
			{
				pNextRow->RepositionPanes(rectNewClientArea, WMSZ_RIGHT, nHorzOffset > 0, abs(nHorzOffset));
			}

			if (nVertOffset != 0)
			{
				pNextRow->RepositionPanes(rectNewClientArea, WMSZ_BOTTOM, nVertOffset > 0, abs(nVertOffset));
			}
		}
	}
	else
	{
		// sanity check
		for (POSITION pos = m_lstDockBarRows.GetHeadPosition(); pos != NULL;)
		{
			CDockingPanesRow* pNextRow = (CDockingPanesRow*) m_lstDockBarRows.GetNext(pos);
			ASSERT_VALID(pNextRow);

			pNextRow->ExpandStretchedPanesRect();
		}
	}
}

CDockingPanesRow* CDockSite::CreateRow(CDockSite* /*pParentDocBar*/, int nOffset, int nRowHeight)
{
	ASSERT_VALID(this);
	CDockingPanesRow* pRow = new CDockingPanesRow(this, nOffset, nRowHeight);
	if (!pRow->Create())
	{
		delete pRow;
		return NULL;
	}
	return pRow;
}

CDockingPanesRow* CDockSite::AddRow(POSITION posRowBefore, int nRowHeight)
{
	ASSERT_VALID(this);
	// claculate the row offset
	int nOffset = 0;

	for (POSITION pos = m_lstDockBarRows.GetHeadPosition(); pos != posRowBefore;)
	{
		CDockingPanesRow* pNextRow = (CDockingPanesRow*) m_lstDockBarRows.GetNext(pos);
		ASSERT_VALID(pNextRow);
		if (pNextRow->IsVisible())
		{
			nOffset += pNextRow->GetRowHeight();
		}
	}

	ResizeDockSiteByOffset(nRowHeight);

	CDockingPanesRow* pNewRow = CreateRow(this, nOffset, nRowHeight);

	if (pNewRow == NULL)
	{
		ASSERT(FALSE);
		return NULL;
	}

	if (posRowBefore != NULL)
	{
		POSITION pos = m_lstDockBarRows.InsertBefore(posRowBefore, pNewRow);
		OnInsertRow(pos);
	}
	else
	{
		m_lstDockBarRows.AddTail(pNewRow);
	}

	return pNewRow;
}

void CDockSite::RemoveRow(CDockingPanesRow* pRow)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pRow);
	ASSERT(!m_lstDockBarRows.IsEmpty());

	int nRowHeight = pRow->GetRowHeight();
	if (pRow->IsVisible())
	{
		ResizeDockSiteByOffset(-nRowHeight);
	}

	POSITION pos = m_lstDockBarRows.Find(pRow);
	if (pos != NULL)
	{
		OnRemoveRow(pos);
		m_lstDockBarRows.RemoveAt(pos);
		delete pRow;
	}
}

int CDockSite::ResizeRow(CDockingPanesRow* pRow, int nNewSize, BOOL bAdjustLayout)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pRow);

	int nOffset = nNewSize - pRow->GetRowHeight();
	if (nOffset < 0 && !pRow->IsEmpty())
	{
		CSize size = pRow->CalcFixedLayout(TRUE, IsHorizontal());
		if (IsHorizontal() && nNewSize - size.cy < 0 || !IsHorizontal() && nNewSize - size.cx < 0)
		{
			return 0;
		}
	}
	int nActualOffset = OnResizeRow(pRow, nOffset);
	ResizeDockSiteByOffset(nActualOffset, bAdjustLayout);

	return nActualOffset;
}

void CDockSite::ShowRow(CDockingPanesRow* pRow, BOOL bShow, BOOL bAdjustLayout)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pRow);
	ASSERT(!m_lstDockBarRows.IsEmpty());

	POSITION pos = m_lstDockBarRows.Find(pRow);
	OnShowRow(pos, bShow);

	int nRowHeight = pRow->GetRowHeight();
	ResizeDockSiteByOffset(bShow ? nRowHeight : -nRowHeight, bAdjustLayout);

}

void CDockSite::OnInsertRow(POSITION pos)
{
	ASSERT_VALID(this);
	ENSURE(pos != NULL);

	CDockingPanesRow* pNewRow = (CDockingPanesRow*) m_lstDockBarRows.GetNext(pos);
	ASSERT_VALID(pNewRow);

	int nRowSize = pNewRow->GetRowHeight();

	// when the row is inserted, all control bars that belongs to the rows after new,
	// should be moved down
	while (pos != NULL)
	{
		CDockingPanesRow* pNextRow = (CDockingPanesRow*) m_lstDockBarRows.GetNext(pos);
		ASSERT_VALID(pNextRow);
		pNextRow->Move(nRowSize);
	}
}

void CDockSite::OnRemoveRow(POSITION pos, BOOL bByShow)
{
	ASSERT_VALID(this);
	ENSURE(pos != NULL);

	CDockingPanesRow* pRowToRemove = (CDockingPanesRow*) m_lstDockBarRows.GetNext(pos);
	ASSERT_VALID(pRowToRemove);

	if (!pRowToRemove->IsVisible() && !bByShow)
	{
		return;
	}

	int nRowSize = pRowToRemove->GetRowHeight();

	while (pos != NULL)
	{
		CDockingPanesRow* pNextRow = (CDockingPanesRow*) m_lstDockBarRows.GetNext(pos);
		ASSERT_VALID(pNextRow);
		pNextRow->Move(-nRowSize);
	}
}

void CDockSite::OnShowRow(POSITION pos, BOOL bShow)
{
	ASSERT_VALID(this);
	ENSURE(pos != NULL);

	bShow ? OnInsertRow(pos) : OnRemoveRow(pos, TRUE);
}

int CDockSite::OnResizeRow(CDockingPanesRow* pRowToResize, int nOffset)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pRowToResize);

	int nActualOffset = pRowToResize->Resize(nOffset);
	if (!pRowToResize->IsVisible())
	{
		return 0;
	}

	POSITION pos = m_lstDockBarRows.Find(pRowToResize);
	m_lstDockBarRows.GetNext(pos);
	// skip to next row
	while (pos != NULL)
	{
		CDockingPanesRow* pNextRow = (CDockingPanesRow*) m_lstDockBarRows.GetNext(pos);
		ASSERT_VALID(pNextRow);
		pNextRow->Move(nActualOffset);
	}

	return nActualOffset;
}

void CDockSite::SwapRows(CDockingPanesRow* pFirstRow, CDockingPanesRow* pSecondRow)
{
	POSITION posFirstRow = m_lstDockBarRows.Find(pFirstRow);
	POSITION posSecondRow = m_lstDockBarRows.Find(pSecondRow);

	ENSURE(posFirstRow != NULL);
	ENSURE(posSecondRow != NULL);

	POSITION posTmp = posFirstRow;

	FindNextVisibleRow(posTmp);

	bool bSwapDown = (posTmp == posSecondRow);

	if (!bSwapDown)
	{
		posTmp = posFirstRow;
		FindNextVisibleRow(posTmp, FALSE);
		if (posTmp != posSecondRow)
		{
			return;
		}
	}

	m_lstDockBarRows.InsertAfter(posFirstRow, pSecondRow);
	m_lstDockBarRows.InsertAfter(posSecondRow, pFirstRow);
	m_lstDockBarRows.RemoveAt(posFirstRow);
	m_lstDockBarRows.RemoveAt(posSecondRow);

	int nRowHeight = pFirstRow->GetRowHeight();
	pSecondRow->Move(bSwapDown ? -nRowHeight : nRowHeight);
	nRowHeight = pSecondRow->GetRowHeight();
	pFirstRow->Move(bSwapDown ? nRowHeight : -nRowHeight);
	FixupVirtualRects();
}

CDockingPanesRow* CDockSite::RowFromPoint(CPoint pt, bool& bOuterRow) const
{
	ASSERT_VALID(this);

	bOuterRow = false;
	CRect rectRow;
	for (POSITION pos = m_lstDockBarRows.GetHeadPosition(); pos != NULL;)
	{
		CDockingPanesRow* pRow = (CDockingPanesRow*) m_lstDockBarRows.GetNext(pos);
		ASSERT_VALID(pRow);

		if (!pRow->IsVisible())
		{
			continue;
		}

		pRow->GetWindowRect(rectRow);
		if (rectRow.PtInRect(pt))
		{
			return pRow;
		}
	}

	CRect rectWnd;
	GetWindowRect(&rectWnd);

	if (IsHorizontal() && pt.y < rectWnd.top || !IsHorizontal() && pt.x < rectWnd.left)
	{
		bOuterRow = true;
	}

	return NULL;
}

CDockingPanesRow* CDockSite::RowFromPane(CBasePane* pBar) const
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);

	for (POSITION pos = m_lstDockBarRows.GetHeadPosition(); pos != NULL;)
	{
		CDockingPanesRow* pRow = (CDockingPanesRow*) m_lstDockBarRows.GetNext(pos);
		ASSERT_VALID(pRow);

		if (pRow->HasPane(pBar) != NULL)
		{
			return pRow;
		}
	}

	return NULL;
}

BOOL CDockSite::ShowPane(CBasePane* pBar, BOOL bShow, BOOL bDelay, BOOL /*bActivate*/)
{
	CDockingPanesRow* pRow = RowFromPane(pBar);

	if (pRow != NULL)
	{
		CPane* pBarToShow = DYNAMIC_DOWNCAST(CPane, pBar);
		// allows to show/hide only CPane-derived bars(other bars
		// has no docking abilitty)
		if (pBarToShow != NULL)
		{
			return pRow->ShowPane(pBarToShow, bShow, bDelay);
		}
	}
	return FALSE;
}

void CDockSite::ResizeDockSiteByOffset(int nOffset, BOOL bAdjustLayout)
{
	ASSERT_VALID(this);

	CRect rect;
	GetWindowRect(&rect);
	GetParent()->ScreenToClient(&rect);

	switch (GetCurrentAlignment())
	{
	case CBRS_ALIGN_LEFT:
		rect.right += nOffset;
		break;

	case CBRS_ALIGN_RIGHT:
		rect.left -= nOffset;
		break;

	case CBRS_ALIGN_TOP:
		rect.bottom += nOffset;
		break;

	case CBRS_ALIGN_BOTTOM:
		rect.top  -= nOffset;
		break;
	}

	MoveWindow(rect);
	if (bAdjustLayout)
	{
		AdjustDockingLayout();
	}
}

bool CDockSite::IsLastRow(CDockingPanesRow* pRow) const
{
	ASSERT_VALID(this);
	return(!m_lstDockBarRows.IsEmpty() && (pRow == m_lstDockBarRows.GetHead() || pRow == m_lstDockBarRows.GetTail()));
}

CSize CDockSite::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	ASSERT_VALID(this);

	int nTotalHeightRequired = 0;

	BOOL bHorzBar = IsHorizontal();

	for (POSITION pos = m_lstDockBarRows.GetHeadPosition(); pos != NULL;)
	{
		CDockingPanesRow* pNextRow = (CDockingPanesRow*) m_lstDockBarRows.GetNext(pos);
		ASSERT_VALID(pNextRow);

		if (!pNextRow->IsVisible())
		{
			continue;
		}

		int nCurrHeight = pNextRow->GetRowHeight();
		CSize sizeRowRequired = pNextRow->CalcFixedLayout(bStretch, bHorz);

		int nHeightRequired =  bHorzBar ? sizeRowRequired.cy : sizeRowRequired.cx;

		if (nHeightRequired != nCurrHeight && nHeightRequired > 0)
		{
			ResizeRow(pNextRow, nHeightRequired, FALSE);
		}

		nTotalHeightRequired += nHeightRequired;
	}

	CRect rectWnd;
	GetWindowRect(rectWnd);

	return rectWnd.Size();
}

void CDockSite::ResizeDockSite(int nNewWidth, int nNewHeight) // not called from anywhere !!!
{
	ASSERT_VALID(this);
	CWnd* pParentWnd = GetParent();
	ASSERT_VALID(pParentWnd);

	CRect rectDockBar;
	GetClientRect(&rectDockBar);
	MapWindowPoints(pParentWnd, &rectDockBar);

	switch (GetCurrentAlignment())
	{
	case CBRS_ALIGN_LEFT:
		if (nNewHeight != -1)
		{
			rectDockBar.bottom = rectDockBar.top + nNewHeight;
		}
		break;

	case CBRS_ALIGN_RIGHT:
		if (nNewHeight != -1)
		{
			rectDockBar.bottom = rectDockBar.top + nNewHeight;
		}
		break;

	case CBRS_ALIGN_TOP:
		if (nNewWidth != -1)
		{
			rectDockBar.right = rectDockBar.left + nNewWidth;
		}
		break;

	case CBRS_ALIGN_BOTTOM:
		if (nNewWidth != -1)
		{
			rectDockBar.right = rectDockBar.left + nNewWidth;
		}
		break;
	}

	OnSetWindowPos(&wndBottom, rectDockBar, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CDockSite::OnPaint()
{
	CPaintDC dc(this); // device context for painting
}

BOOL CDockSite::IsRectWithinDockSite(CRect rect, CPoint& ptDelta)
{
	ASSERT_VALID(this);
	CRect rectWnd;
	GetWindowRect(&rectWnd);

	ptDelta.x = ptDelta.y = 0;

	if (IsHorizontal())
	{
		if (rect.left < rectWnd.left)
		{
			ptDelta.x = rectWnd.left - rect.left;
			return FALSE;
		}
		if (rect.right >  rectWnd.right)
		{
			ptDelta.x = rectWnd.right - rect.right;
			return FALSE;
		}
	}
	else
	{
		if (rect.top < rectWnd.top)
		{
			ptDelta.y = rectWnd.top - rect.top;
			return FALSE;
		}
		if (rect.bottom > rectWnd.bottom)
		{
			ptDelta.y = rect.bottom - rectWnd.bottom;
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CDockSite::CanAcceptPane(const CBasePane* pBar) const
{
	ASSERT_VALID(this);
	if (pBar == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return !IsResizable();
}

void CDockSite::OnSize(UINT nType, int cx, int cy)
{
	ASSERT_VALID(this);

	CWnd::OnSize(nType, cx, cy);
}

CPane* CDockSite::PaneFromPoint(CPoint pt)
{
	ASSERT_VALID(this);

	CRect rectBar;
	CPane* pBar = NULL;
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		pBar = (CPane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pBar);

		pBar->GetWindowRect(rectBar);
		if (rectBar.PtInRect(pt))
		{
			return pBar;
		}
	}

	return NULL;
}

int __stdcall CDockSite::RectSideFromPoint(const CRect& rect, const CPoint& point)
{
	int nDeltaLeft = point.x - rect.left;
	int nDeltaTop = point.y - rect.top;
	int nDeltaRight = rect.right - point.x;
	int nDeltaBottom = rect.bottom - point.y;

	// use hit test definition to describe the side
	UINT nHitTestLR = (nDeltaLeft <= nDeltaRight) ? HTLEFT : HTRIGHT;
	UINT nHitTetsTB = (nDeltaTop <= nDeltaBottom) ? HTTOP : HTBOTTOM;

	int nHitTest = HTERROR;
	if (nHitTestLR == HTLEFT && nHitTetsTB == HTTOP)
	{
		nHitTest = (nDeltaLeft <= nDeltaTop) ? HTLEFT : HTTOP;
	}
	else if (nHitTestLR == HTRIGHT && nHitTetsTB == HTTOP)
	{
		nHitTest = (nDeltaRight <= nDeltaTop) ? HTRIGHT : HTTOP;
	}
	else if (nHitTestLR == HTLEFT && nHitTetsTB == HTBOTTOM)
	{
		nHitTest = (nDeltaLeft <= nDeltaBottom) ? HTLEFT : HTBOTTOM;
	}
	else if (nHitTestLR == HTRIGHT && nHitTetsTB == HTBOTTOM)
	{
		nHitTest = (nDeltaRight <= nDeltaBottom) ? HTRIGHT : HTBOTTOM;
	}
	else
	{
		return HTERROR;
	}
	return nHitTest;
}

BOOL CDockSite::ReplacePane(CPane* pOldBar, CPane* pNewBar)
{
	ASSERT_VALID(this);
	POSITION pos = m_lstControlBars.Find(pOldBar);

	if (pos != NULL)
	{
		m_lstControlBars.InsertAfter(pos, pNewBar);
		m_lstControlBars.RemoveAt(pos);
		return TRUE;
	}

	return FALSE;
}

BOOL CDockSite::OnSetWindowPos(const CWnd* pWndInsertAfter, const CRect& rectWnd, UINT nFlags)
{
	ASSERT_VALID(this);
	return(BOOL)(SetWindowPos(pWndInsertAfter, rectWnd.left, rectWnd.top, rectWnd.Width(), rectWnd.Height(), nFlags | SWP_NOACTIVATE) != 0);
}

void CDockSite::OnNcDestroy()
{
	CWnd::OnNcDestroy();
	delete this;
}

BOOL CDockSite::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

	CMFCVisualManager::GetInstance()->OnFillBarBackground(pDC, this, rect, rect, FALSE);
	return TRUE;
}

void CDockSite::AdjustLayout()
{
	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CBasePane* pBar = (CBasePane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pBar);
		pBar->AdjustLayout();
	}
}

void CDockSite::AdjustDockingLayout()
{
	ASSERT_VALID(this);

	CWnd* pParent = GetParent();
	ASSERT_VALID(pParent);

	if (pParent->IsKindOf(RUNTIME_CLASS(CFrameWndEx)))
	{
		((CFrameWndEx*) pParent)->AdjustDockingLayout();
	}
	else if (pParent->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		((CMDIFrameWndEx*) pParent)->AdjustDockingLayout(NULL);
	}
	else if (pParent->IsKindOf(RUNTIME_CLASS(COleIPFrameWndEx)))
	{
		((COleIPFrameWndEx*) pParent)->AdjustDockingLayout();
	}
	else if (pParent->IsKindOf(RUNTIME_CLASS(COleDocIPFrameWndEx)))
	{
		((COleDocIPFrameWndEx*) pParent)->AdjustDockingLayout();
	}
	else if (pParent->IsKindOf(RUNTIME_CLASS(COleCntrFrameWndEx)))
	{
		((COleCntrFrameWndEx*) pParent)->AdjustDockingLayout();
	}
	else if (pParent->IsKindOf(RUNTIME_CLASS(CMDIChildWndEx)))
	{
		((CMDIChildWndEx*) pParent)->AdjustDockingLayout();
	}
	else if (pParent->IsKindOf(RUNTIME_CLASS(CDialog)))
	{
		afxGlobalUtils.m_bDialogApp = TRUE;
	}
}

int CDockSite::FindRowIndex(CDockingPanesRow* pRow)
{
	ASSERT_VALID(this);

	if (pRow == NULL)
	{
		return 0;
	}

	int nIndex = 0;
	for (POSITION pos = m_lstDockBarRows.GetHeadPosition(); pos != NULL; nIndex++)
	{
		CDockingPanesRow* pNextRow = (CDockingPanesRow*) m_lstDockBarRows.GetNext(pos);
		if (pNextRow == pRow)
		{
			return nIndex;
		}
	}

	return 0;
}

void CDockSite::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if (CMFCPopupMenu::GetActiveMenu() != NULL)
	{
		return;
	}

	if (!CMFCToolBar::IsCustomizeMode() && !IsDragMode())
	{
		CFrameWnd* pParentFrame = AFXGetTopLevelFrame(this);
		if (pParentFrame == NULL)
		{
			ASSERT(FALSE);
			return;
		}

		OnPaneContextMenu(pParentFrame, point);
	}
}

BOOL CDockSite::IsDragMode() const
{
	ASSERT_VALID(this);

	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CPane* pBar = DYNAMIC_DOWNCAST(CPane, m_lstControlBars.GetNext(pos));
		if (pBar == NULL)
		{
			continue;
		}

		if (pBar->IsDragMode())
		{
			return TRUE;
		}
	}

	return FALSE;
}

CPane* CDockSite::FindPaneByID(UINT nID)
{
	ASSERT_VALID(this);

	for (POSITION pos = m_lstControlBars.GetHeadPosition(); pos != NULL;)
	{
		CPane* pBar = (CPane*) m_lstControlBars.GetNext(pos);
		ASSERT_VALID(pBar);

		if (pBar->GetDlgCtrlID() == (int) nID)
		{
			return pBar;
		}

		// Check for rebar:
		CMFCReBar* pRebar = DYNAMIC_DOWNCAST(CMFCReBar, pBar);
		if (pRebar != NULL)
		{
			ASSERT_VALID(pRebar);

			CPane* pBarPane = DYNAMIC_DOWNCAST(CPane, pRebar->GetDlgItem(nID));
			if (pBarPane != NULL)
			{
				return pBarPane;
			}
		}
	}

	return NULL;
}

void CDockSite::OnDestroy()
{
	RemovePaneFromDockManager(this, FALSE);
	CBasePane::OnDestroy();
}



