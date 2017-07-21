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
#include "afxheaderctrl.h"
#include "afxglobals.h"
#include "afxvisualmanager.h"
#include "afxtrackmouse.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CMFCHeaderCtrl, CHeaderCtrl)

/////////////////////////////////////////////////////////////////////////////
// CMFCHeaderCtrl

CMFCHeaderCtrl::CMFCHeaderCtrl()
{
	m_bIsMousePressed = FALSE;
	m_bMultipleSort = FALSE;
	m_bAscending = TRUE;
	m_nHighlightedItem = -1;
	m_bTracked = FALSE;
	m_bIsDlgControl = FALSE;
	m_hFont = NULL;
}

CMFCHeaderCtrl::~CMFCHeaderCtrl()
{
}

//{{AFX_MSG_MAP(CMFCHeaderCtrl)
BEGIN_MESSAGE_MAP(CMFCHeaderCtrl, CHeaderCtrl)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CANCELMODE()
	ON_WM_CREATE()
	ON_MESSAGE(WM_MOUSELEAVE, &CMFCHeaderCtrl::OnMouseLeave)
	ON_MESSAGE(WM_SETFONT, &CMFCHeaderCtrl::OnSetFont)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// CMFCHeaderCtrl message handlers

void CMFCHeaderCtrl::OnDrawItem(CDC* pDC, int iItem, CRect rect, BOOL bIsPressed, BOOL bIsHighlighted)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	const int nTextMargin = 5;

	// Draw border:
	CMFCVisualManager::GetInstance()->OnDrawHeaderCtrlBorder(this, pDC, rect, bIsPressed, bIsHighlighted);

	if (iItem < 0)
	{
		return;
	}

	int nSortVal = 0;
	if (m_mapColumnsStatus.Lookup(iItem, nSortVal) && nSortVal != 0)
	{
		// Draw sort arrow:
		CRect rectArrow = rect;
		rectArrow.DeflateRect(5, 5);
		rectArrow.left = rectArrow.right - rectArrow.Height();

		if (bIsPressed)
		{
			rectArrow.right++;
			rectArrow.bottom++;
		}

		rect.right = rectArrow.left - 1;

		int dy2 = (int)(.134 * rectArrow.Width());
		rectArrow.DeflateRect(0, dy2);

		m_bAscending = nSortVal > 0;
		OnDrawSortArrow(pDC, rectArrow);
	}

	HD_ITEM hdItem;
	memset(&hdItem, 0, sizeof(hdItem));
	hdItem.mask = HDI_FORMAT | HDI_BITMAP | HDI_TEXT | HDI_IMAGE;

	TCHAR szText [256];
	hdItem.pszText = szText;
	hdItem.cchTextMax = 255;

	if (!GetItem(iItem, &hdItem))
	{
		return;
	}

	// Draw bitmap and image:
	if ((hdItem.fmt & HDF_IMAGE) && hdItem.iImage >= 0)
	{
		// By Max Khiszinsky:

		// The column has a image from imagelist:
		CImageList* pImageList = GetImageList();
		if (pImageList != NULL)
		{
			int cx = 0;
			int cy = 0;

			VERIFY(::ImageList_GetIconSize(*pImageList, &cx, &cy));

			CPoint pt = rect.TopLeft();
			pt.x ++;
			pt.y = (rect.top + rect.bottom - cy) / 2;

			VERIFY(pImageList->Draw(pDC, hdItem.iImage, pt, ILD_NORMAL));

			rect.left += cx;
		}
	}

	if ((hdItem.fmt &(HDF_BITMAP | HDF_BITMAP_ON_RIGHT)) && hdItem.hbm != NULL)
	{
		CBitmap* pBmp = CBitmap::FromHandle(hdItem.hbm);
		ASSERT_VALID(pBmp);

		BITMAP bmp;
		pBmp->GetBitmap(&bmp);

		CRect rectBitmap = rect;
		if (hdItem.fmt & HDF_BITMAP_ON_RIGHT)
		{
			rectBitmap.right--;
			rect.right = rectBitmap.left = rectBitmap.right - bmp.bmWidth;
		}
		else
		{
			rectBitmap.left++;
			rect.left = rectBitmap.right = rectBitmap.left + bmp.bmWidth;
		}

		rectBitmap.top += max(0, (rectBitmap.Height() - bmp.bmHeight) / 2);
		rectBitmap.bottom = rectBitmap.top + bmp.bmHeight;

		pDC->DrawState(rectBitmap.TopLeft(), rectBitmap.Size(), pBmp, DSS_NORMAL);
	}

	// Draw text:
	if ((hdItem.fmt & HDF_STRING) && hdItem.pszText != NULL)
	{
		CRect rectLabel = rect;
		rectLabel.DeflateRect(nTextMargin, 0);

		CString strLabel = hdItem.pszText;

		UINT uiTextFlags = DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX;
		if (hdItem.fmt & HDF_CENTER)
		{
			uiTextFlags |= DT_CENTER;
		}
		else if (hdItem.fmt & HDF_RIGHT)
		{
			uiTextFlags |= DT_RIGHT;
		}

		pDC->DrawText(strLabel, rectLabel, uiTextFlags);
	}
}

void CMFCHeaderCtrl::SetSortColumn(int iColumn, BOOL bAscending, BOOL bAdd)
{
	ASSERT_VALID(this);

	if (iColumn < 0)
	{
		m_mapColumnsStatus.RemoveAll();
		return;
	}

	if (bAdd)
	{
		if (!m_bMultipleSort)
		{
			ASSERT(FALSE);
			bAdd = FALSE;
		}
	}

	if (!bAdd)
	{
		m_mapColumnsStatus.RemoveAll();
	}

	m_mapColumnsStatus.SetAt(iColumn, bAscending ? 1 : -1);
	RedrawWindow();
}

void CMFCHeaderCtrl::RemoveSortColumn(int iColumn)
{
	ASSERT_VALID(this);
	m_mapColumnsStatus.RemoveKey(iColumn);
	RedrawWindow();
}

BOOL CMFCHeaderCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CMFCHeaderCtrl::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CMemDC memDC(dc, this);
	CDC* pDC = &memDC.GetDC();

	CRect rectClip;
	dc.GetClipBox(rectClip);

	CRect rectClient;
	GetClientRect(rectClient);

	OnFillBackground(pDC);

	CFont* pOldFont = SelectFont(pDC);
	ASSERT_VALID(pOldFont);

	pDC->SetTextColor(afxGlobalData.clrBtnText);
	pDC->SetBkMode(TRANSPARENT);

	CRect rect;
	GetClientRect(rect);

	CRect rectItem;
	int nCount = GetItemCount();

	int xMax = 0;

	for (int i = 0; i < nCount; i++)
	{
		// Is item pressed?
		CPoint ptCursor;
		::GetCursorPos(&ptCursor);
		ScreenToClient(&ptCursor);

		HDHITTESTINFO hdHitTestInfo;
		hdHitTestInfo.pt = ptCursor;

		int iHit = (int) SendMessage(HDM_HITTEST, 0, (LPARAM) &hdHitTestInfo);

		BOOL bIsHighlighted = iHit == i &&(hdHitTestInfo.flags & HHT_ONHEADER);
		BOOL bIsPressed = m_bIsMousePressed && bIsHighlighted;

		GetItemRect(i, rectItem);

		CRgn rgnClip;
		rgnClip.CreateRectRgnIndirect(&rectItem);
		pDC->SelectClipRgn(&rgnClip);

		// Draw item:
		OnDrawItem(pDC, i, rectItem, bIsPressed, m_nHighlightedItem == i);

		pDC->SelectClipRgn(NULL);

		xMax = max(xMax, rectItem.right);
	}

	// Draw "tail border":
	if (nCount == 0)
	{
		rectItem = rect;
		rectItem.right++;
	}
	else
	{
		rectItem.left = xMax;
		rectItem.right = rect.right + 1;
	}

	OnDrawItem(pDC, -1, rectItem, FALSE, FALSE);

	pDC->SelectObject(pOldFont);
}

void CMFCHeaderCtrl::OnFillBackground(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	CRect rectClient;
	GetClientRect(rectClient);

	CMFCVisualManager::GetInstance()->OnFillHeaderCtrlBackground(this, pDC, rectClient);
}

CFont* CMFCHeaderCtrl::SelectFont(CDC *pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	CFont* pOldFont = NULL;

	if (m_hFont != NULL)
	{
		pOldFont = pDC->SelectObject(CFont::FromHandle(m_hFont));
	}
	else
	{
		pOldFont = m_bIsDlgControl ? (CFont*) pDC->SelectStockObject(DEFAULT_GUI_FONT) : pDC->SelectObject(&afxGlobalData.fontRegular);
	}

	return pOldFont;
}

void CMFCHeaderCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bIsMousePressed = TRUE;
	CHeaderCtrl::OnLButtonDown(nFlags, point);
}

void CMFCHeaderCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bIsMousePressed = FALSE;
	CHeaderCtrl::OnLButtonUp(nFlags, point);
}

void CMFCHeaderCtrl::OnDrawSortArrow(CDC* pDC, CRect rectArrow)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(this);

	CMFCVisualManager::GetInstance()->OnDrawHeaderCtrlSortArrow(this, pDC, rectArrow, m_bAscending);
}

void CMFCHeaderCtrl::EnableMultipleSort(BOOL bEnable)
{
	ASSERT_VALID(this);

	if (m_bMultipleSort == bEnable)
	{
		return;
	}

	m_bMultipleSort = bEnable;

	if (!m_bMultipleSort)
	{
		m_mapColumnsStatus.RemoveAll();

		if (GetSafeHwnd() != NULL)
		{
			RedrawWindow();
		}
	}
}

int CMFCHeaderCtrl::GetSortColumn() const
{
	ASSERT_VALID(this);

	if (m_bMultipleSort)
	{
		TRACE0("Call CMFCHeaderCtrl::GetColumnState for muliple sort\n");
		ASSERT(FALSE);
		return -1;
	}

	int nCount = GetItemCount();
	for (int i = 0; i < nCount; i++)
	{
		int nSortVal = 0;
		if (m_mapColumnsStatus.Lookup(i, nSortVal) && nSortVal != 0)
		{
			return i;
		}
	}

	return -1;
}

BOOL CMFCHeaderCtrl::IsAscending() const
{
	ASSERT_VALID(this);

	if (m_bMultipleSort)
	{
		TRACE0("Call CMFCHeaderCtrl::GetColumnState for muliple sort\n");
		ASSERT(FALSE);
		return FALSE;
	}

	int nCount = GetItemCount();

	for (int i = 0; i < nCount; i++)
	{
		int nSortVal = 0;
		if (m_mapColumnsStatus.Lookup(i, nSortVal) && nSortVal != 0)
		{
			return nSortVal > 0;
		}
	}

	return FALSE;
}

int CMFCHeaderCtrl::GetColumnState(int iColumn) const
{
	int nSortVal = 0;
	m_mapColumnsStatus.Lookup(iColumn, nSortVal);

	return nSortVal;
}

void CMFCHeaderCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	if ((nFlags & MK_LBUTTON) == 0)
	{
		HDHITTESTINFO hdHitTestInfo;
		hdHitTestInfo.pt = point;

		int nPrevHighlightedItem = m_nHighlightedItem;
		m_nHighlightedItem = (int) SendMessage(HDM_HITTEST, 0, (LPARAM) &hdHitTestInfo);

		if ((hdHitTestInfo.flags & HHT_ONHEADER) == 0)
		{
			m_nHighlightedItem = -1;
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

		if (nPrevHighlightedItem != m_nHighlightedItem)
		{
			RedrawWindow();
		}
	}

	CHeaderCtrl::OnMouseMove(nFlags, point);
}

LRESULT CMFCHeaderCtrl::OnMouseLeave(WPARAM,LPARAM)
{
	m_bTracked = FALSE;

	if (m_nHighlightedItem >= 0)
	{
		m_nHighlightedItem = -1;
		RedrawWindow();
	}

	return 0;
}

void CMFCHeaderCtrl::OnCancelMode()
{
	CHeaderCtrl::OnCancelMode();

	if (m_nHighlightedItem >= 0)
	{
		m_nHighlightedItem = -1;
		RedrawWindow();
	}
}

int CMFCHeaderCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CHeaderCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	CommonInit();

	return 0;
}

void CMFCHeaderCtrl::PreSubclassWindow()
{
	CommonInit();
	CHeaderCtrl::PreSubclassWindow();
}

void CMFCHeaderCtrl::CommonInit()
{
	ASSERT_VALID(this);

	for (CWnd* pParentWnd = GetParent(); pParentWnd != NULL;
		pParentWnd = pParentWnd->GetParent())
	{
		if (pParentWnd->IsKindOf(RUNTIME_CLASS(CDialog)))
		{
			m_bIsDlgControl = TRUE;
			break;
		}
	}
}

LRESULT CMFCHeaderCtrl::OnSetFont(WPARAM wParam, LPARAM lParam)
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


