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
#include "afxlistctrl.h"
#include "afxdrawmanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCListCtrl

IMPLEMENT_DYNAMIC(CMFCListCtrl, CListCtrl)

CMFCListCtrl::CMFCListCtrl()
{
	m_iSortedColumn = -1;
	m_bAscending = TRUE;
	m_bMarkSortedColumn = FALSE;
	m_clrSortedColumn = (COLORREF)-1;
	m_hOldFont = NULL;
}

CMFCListCtrl::~CMFCListCtrl()
{
}

//{{AFX_MSG_MAP(CMFCListCtrl)
BEGIN_MESSAGE_MAP(CMFCListCtrl, CListCtrl)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_SIZE()
	ON_MESSAGE(WM_STYLECHANGED, &CMFCListCtrl::OnStyleChanged)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CMFCListCtrl::OnCustomDraw)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, &CMFCListCtrl::OnColumnClick)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// CMFCListCtrl message handlers

BOOL CMFCListCtrl::InitList()
{
	InitHeader();
	InitColors();
	return TRUE;
}

void CMFCListCtrl::InitHeader()
{
	// Initialize header control:
	m_wndHeader.SubclassDlgItem(0, this);
}

void CMFCListCtrl::PreSubclassWindow()
{
	CListCtrl::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (pThreadState->m_pWndInit == NULL)
	{
		if (!InitList())
		{
			ASSERT(FALSE);
		}
	}
}

int CMFCListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!InitList())
	{
		return -1;
	}

	return 0;
}

void CMFCListCtrl::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	ENSURE(pNMListView != NULL);

	int iColumn = pNMListView->iSubItem;
	BOOL bShiftIsPressed = (::GetAsyncKeyState(VK_SHIFT) & 0x8000);
	int nColumnState = GetHeaderCtrl().GetColumnState(iColumn);
	BOOL bAscending = TRUE;

	if (nColumnState != 0)
	{
		bAscending = nColumnState <= 0;
	}

	Sort(iColumn, bAscending, bShiftIsPressed && IsMultipleSort());
	*pResult = 0;
}

void CMFCListCtrl::Sort(int iColumn, BOOL bAscending, BOOL bAdd)
{
	CWaitCursor wait;

	GetHeaderCtrl().SetSortColumn(iColumn, bAscending, bAdd);

	m_iSortedColumn = iColumn;
	m_bAscending = bAscending;

	SortItems(CompareProc, (LPARAM) this);
}

void CMFCListCtrl::SetSortColumn(int iColumn, BOOL bAscending, BOOL bAdd)
{
	GetHeaderCtrl().SetSortColumn(iColumn, bAscending, bAdd);
}

void CMFCListCtrl::RemoveSortColumn(int iColumn)
{
	GetHeaderCtrl().RemoveSortColumn(iColumn);
}

void CMFCListCtrl::EnableMultipleSort(BOOL bEnable)
{
	GetHeaderCtrl().EnableMultipleSort(bEnable);
}

BOOL CMFCListCtrl::IsMultipleSort() const
{
	return((CMFCListCtrl*) this)->GetHeaderCtrl().IsMultipleSort();
}

int CMFCListCtrl::OnCompareItems(LPARAM /*lParam1*/, LPARAM /*lParam2*/, int /*iColumn*/)
{
	return 0;
}

int CALLBACK CMFCListCtrl::CompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CMFCListCtrl* pList = (CMFCListCtrl*) lParamSort;
	ASSERT_VALID(pList);

	int nRes = pList->OnCompareItems(lParam1, lParam2, pList->m_iSortedColumn);
	nRes = pList->m_bAscending ? nRes : -nRes;

	return nRes;
}

void CMFCListCtrl::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	ENSURE(pNMHDR != NULL);
	LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)pNMHDR;

	switch (lplvcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;

	case CDDS_ITEMPREPAINT:
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
		break;

	case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
		{
			int iColumn = lplvcd->iSubItem;
			int iRow = (int) lplvcd->nmcd.dwItemSpec;

			lplvcd->clrTextBk = OnGetCellBkColor(iRow, iColumn);
			lplvcd->clrText = OnGetCellTextColor(iRow, iColumn);

			if (iColumn == m_iSortedColumn && m_bMarkSortedColumn && lplvcd->clrTextBk == GetBkColor())
			{
				lplvcd->clrTextBk = m_clrSortedColumn;
			}

			HFONT hFont = OnGetCellFont( iRow, iColumn, (DWORD) lplvcd->nmcd.lItemlParam);
			if (hFont != NULL)
			{
				m_hOldFont = (HFONT) SelectObject(lplvcd->nmcd.hdc, hFont);
				ENSURE(m_hOldFont != NULL);

				*pResult = CDRF_NEWFONT | CDRF_NOTIFYPOSTPAINT;
			}
			else
			{
				*pResult = CDRF_DODEFAULT;
			}
		}
		break;

	case CDDS_ITEMPOSTPAINT | CDDS_SUBITEM:
		if (m_hOldFont != NULL)
		{
			SelectObject(lplvcd->nmcd.hdc, m_hOldFont);
			m_hOldFont = NULL;
		}

		*pResult = CDRF_DODEFAULT;
		break;
	}
}

void CMFCListCtrl::EnableMarkSortedColumn(BOOL bMark/* = TRUE*/, BOOL bRedraw/* = TRUE */)
{
	m_bMarkSortedColumn = bMark;

	if (GetSafeHwnd() != NULL && bRedraw)
	{
		RedrawWindow();
	}
}

BOOL CMFCListCtrl::OnEraseBkgnd(CDC* pDC)
{
	BOOL bRes = CListCtrl::OnEraseBkgnd(pDC);

	if (m_iSortedColumn >= 0 && m_bMarkSortedColumn)
	{
		CRect rectClient;
		GetClientRect(&rectClient);

		CRect rectHeader;
		GetHeaderCtrl().GetItemRect(m_iSortedColumn, &rectHeader);
		GetHeaderCtrl().MapWindowPoints(this, rectHeader);

		CRect rectColumn = rectClient;
		rectColumn.left = rectHeader.left;
		rectColumn.right = rectHeader.right;

		CBrush br(m_clrSortedColumn);
		pDC->FillRect(rectColumn, &br);
	}

	return bRes;
}

void CMFCListCtrl::OnSysColorChange()
{
	CListCtrl::OnSysColorChange();

	InitColors();
	RedrawWindow();
}

void CMFCListCtrl::InitColors()
{
	m_clrSortedColumn = CDrawingManager::PixelAlpha(GetBkColor(), .97, .97, .97);
}

LRESULT CMFCListCtrl::OnStyleChanged(WPARAM wp, LPARAM lp)
{
	int nStyleType = (int) wp;
	LPSTYLESTRUCT lpStyleStruct = (LPSTYLESTRUCT) lp;

	CListCtrl::OnStyleChanged(nStyleType, lpStyleStruct);

	if ((lpStyleStruct->styleNew & LVS_REPORT) && (lpStyleStruct->styleOld & LVS_REPORT) == 0)
	{
		if (GetHeaderCtrl().GetSafeHwnd() == NULL)
		{
			InitHeader();
		}
	}

	return 0;
}

void CMFCListCtrl::OnSize(UINT nType, int cx, int cy)
{
	CListCtrl::OnSize(nType, cx, cy);

	if (m_wndHeader.GetSafeHwnd() != NULL)
	{
		m_wndHeader.RedrawWindow();
	}
}


