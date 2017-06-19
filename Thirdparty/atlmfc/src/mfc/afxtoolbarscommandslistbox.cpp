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

#include "afxtoolbarscommandslistbox.h"
#include "afxtoolbar.h"
#include "afxtoolbarbutton.h"
#include "afxribbonres.h"
#include "afxglobals.h"
#include "afxvisualmanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarsCommandsListBox

CMFCToolBarsCommandsListBox::CMFCToolBarsCommandsListBox() : m_sizeButton(0, 0)
{
}

CMFCToolBarsCommandsListBox::~CMFCToolBarsCommandsListBox()
{
}

BEGIN_MESSAGE_MAP(CMFCToolBarsCommandsListBox, CListBox)
	//{{AFX_MSG_MAP(CMFCToolBarsCommandsListBox)
	ON_WM_LBUTTONDOWN()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarsCommandsListBox message handlers

void CMFCToolBarsCommandsListBox::OnLButtonDown(UINT nFlags, CPoint point)
{
	CListBox::OnLButtonDown(nFlags, point);

	int iIndex = GetCurSel();
	if (iIndex == LB_ERR)
	{
		return;
	}

	// Be sure that we realy click into the item!
	CRect rect;
	GetItemRect(iIndex, &rect);

	if (!rect.PtInRect(point))
	{
		return;
	}

	// Trigger mouse up event(to change selection notification):
	SendMessage(WM_LBUTTONUP, nFlags, MAKELPARAM(point.x, point.y));

	// Get selected button:
	CMFCToolBarButton* pButton = (CMFCToolBarButton*) GetItemData(iIndex);
	ASSERT_VALID(pButton);

	// Prepare clipboard data and start drag:
	COleDataSource srcItem;

	pButton->m_bDragFromCollection = TRUE;
	pButton->PrepareDrag(srcItem);
	pButton->m_bDragFromCollection = FALSE;

	{
		::SetCursor(AfxGetApp()->LoadCursor(IDC_AFXBARRES_DELETE));
	}

	srcItem.DoDragDrop(DROPEFFECT_COPY|DROPEFFECT_MOVE, &rect, &CMFCToolBar::m_DropSource);
}

void CMFCToolBarsCommandsListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
	ENSURE(lpMIS != NULL);

	UINT uiRowHeight = (UINT) m_sizeButton.cy;
	if (lpMIS->itemHeight < uiRowHeight)
	{
		lpMIS->itemHeight = uiRowHeight;
	}
}

void CMFCToolBarsCommandsListBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	ASSERT_VALID(pDC);

	CRect rect = lpDIS->rcItem;

	if (lpDIS->itemID != (UINT)-1)
	{
		CMFCToolBarButton* pButton = (CMFCToolBarButton*) GetItemData(lpDIS->itemID);
		ASSERT_VALID(pButton);

		CString strText = pButton->m_strText;
		GetText(lpDIS->itemID, pButton->m_strText);

		CMFCVisualManager::GetInstance()->OnFillCommandsListBackground(pDC, rect);

		pButton->OnDrawOnCustomizeList(pDC, rect, (lpDIS->itemState & ODS_SELECTED) &&(lpDIS->itemState & ODS_FOCUS));
		pButton->m_strText = strText;
	}
}

void CMFCToolBarsCommandsListBox::PreSubclassWindow()
{
	CListBox::PreSubclassWindow();

	CSize sizeMenuImage = CMFCToolBar::GetMenuImageSize();

	m_sizeButton = CSize( sizeMenuImage.cx + 6, sizeMenuImage.cy + 6);
}

BOOL CMFCToolBarsCommandsListBox::OnEraseBkgnd(CDC* pDC)
{
	ASSERT_VALID(pDC);

	CRect rectClient;
	GetClientRect(rectClient);

	CMFCVisualManager::GetInstance()->OnFillCommandsListBackground(pDC, rectClient);
	return TRUE;
}


