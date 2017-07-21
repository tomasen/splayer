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

#include "afxribbonres.h"
#include "afxtoolbarbuttonslistbutton.h"
#include "afxtoolbarbutton.h"
#include "afxtoolbarimages.h"
#include "afxglobals.h"
#include "afxvisualmanager.h"
#include "afxtoolbarcomboboxbutton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const int nXOffset = 4;
static const int nYOffset = 5;

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarButtonsListButton

CMFCToolBarButtonsListButton::CMFCToolBarButtonsListButton()
{
	m_pSelButton = NULL;
	m_pImages = NULL;

	m_iScrollOffset = 0;
	m_iScrollTotal = 0;
	m_iScrollPage = 0;

	m_bEnableDragFromList = FALSE;
	m_bInited = FALSE;
}

CMFCToolBarButtonsListButton::~CMFCToolBarButtonsListButton()
{
}

BEGIN_MESSAGE_MAP(CMFCToolBarButtonsListButton, CButton)
	//{{AFX_MSG_MAP(CMFCToolBarButtonsListButton)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_VSCROLL()
	ON_WM_ENABLE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_KEYDOWN()
	ON_WM_GETDLGCODE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarButtonsListButton message handlers

BOOL CMFCToolBarButtonsListButton::OnEraseBkgnd(CDC* pDC)
{
	CRect rectClient; // Client area rectangle
	GetClientRect(&rectClient);

	pDC->FillSolidRect(&rectClient, IsWindowEnabled() ? afxGlobalData.clrWindow : afxGlobalData.clrBtnFace);
	return TRUE;
}

void CMFCToolBarButtonsListButton::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	if (!m_bInited)
	{
		RebuildLocations();
	}

	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	CRect rectClient = lpDIS->rcItem;

	if (m_pImages != NULL)
	{
		m_pImages->SetTransparentColor(afxGlobalData.clrBtnFace);

		CAfxDrawState ds;
		if (!m_pImages->PrepareDrawImage(ds))
		{
			return;
		}

		for (POSITION pos = m_Buttons.GetHeadPosition(); pos != NULL;)
		{
			CMFCToolBarButton* pButton = (CMFCToolBarButton*) m_Buttons.GetNext(pos);
			ENSURE(pButton != NULL);

			CRect rect = pButton->Rect();
			rect.OffsetRect(0, -m_iScrollOffset);

			if (rect.top >= rectClient.bottom)
			{
				break;
			}

			if (rect.bottom > rectClient.top)
			{
				int nSaveStyle = pButton->m_nStyle;
				BOOL bLocked = pButton->m_bLocked;
				BOOL bIsHighlight = FALSE;

				if (!IsWindowEnabled())
				{
					pButton->m_nStyle |= TBBS_DISABLED;
				}
				else if (pButton == m_pSelButton)
				{
					bIsHighlight = TRUE;
				}

				pButton->m_bLocked = TRUE;
				pButton->OnDraw(pDC, rect, m_pImages, TRUE, FALSE, bIsHighlight);
				pButton->m_nStyle = nSaveStyle;
				pButton->m_bLocked = bLocked;
			}
		}

		m_pImages->EndDrawImage(ds);
	}

	CMFCToolBarComboBoxButton btnDummy;
	rectClient.InflateRect(1, 1);
	CMFCVisualManager::GetInstance()->OnDrawComboBorder(pDC, rectClient, !IsWindowEnabled(), FALSE, TRUE, &btnDummy);
}

void CMFCToolBarButtonsListButton::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	SetFocus();

	CMFCToolBarButton* pButton = HitTest(point);
	if (pButton == NULL)
	{
		return;
	}

	SelectButton(pButton);

	if (m_bEnableDragFromList)
	{
		COleDataSource srcItem;

		pButton->m_bDragFromCollection = TRUE;
		pButton->PrepareDrag(srcItem);
		pButton->m_bDragFromCollection = TRUE;

		srcItem.DoDragDrop();
	}
}

void CMFCToolBarButtonsListButton::SetImages(CMFCToolBarImages* pImages)
{
	ASSERT_VALID(pImages);
	m_pImages = pImages;

	m_sizeButton.cx = m_pImages->GetImageSize().cx + 6;
	m_sizeButton.cy = m_pImages->GetImageSize().cy + 7;

	RemoveButtons();
}

void CMFCToolBarButtonsListButton::AddButton(CMFCToolBarButton* pButton)
{
	ASSERT_VALID(pButton);
	ENSURE(m_pImages != NULL);

	m_Buttons.AddTail(pButton);
	pButton->OnChangeParentWnd(this);

	RebuildLocations();

	HWND hwnd = pButton->GetHwnd();
	if (hwnd != NULL)
	{
		::EnableWindow(hwnd, FALSE);
	}
}

void CMFCToolBarButtonsListButton::RemoveButtons()
{
	SelectButton((CMFCToolBarButton*) NULL);

	while (!m_Buttons.IsEmpty())
	{
		CMFCToolBarButton* pButton = (CMFCToolBarButton*) m_Buttons.RemoveHead();
		ASSERT_VALID(pButton);

		pButton->OnChangeParentWnd(NULL);
	}

	m_iScrollOffset = 0;
	m_iScrollTotal = 0;
	m_iScrollPage = 0;

	EnableScrollBarCtrl(SB_VERT, FALSE);
	SetScrollRange(SB_VERT, 0, 0);
}

CMFCToolBarButton* CMFCToolBarButtonsListButton::HitTest(POINT point) const
{
	CRect rectClient;
	GetClientRect(&rectClient);

	for (POSITION pos = m_Buttons.GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarButton* pButton = (CMFCToolBarButton*) m_Buttons.GetNext(pos);
		ENSURE(pButton != NULL);

		CRect rect = pButton->Rect();
		rect.OffsetRect(0, -m_iScrollOffset);

		if (rect.PtInRect(point))
		{
			return pButton;
		}
	}

	return NULL;
}

void CMFCToolBarButtonsListButton::SelectButton(CMFCToolBarButton* pButton)
{
	if (m_pSelButton == pButton)
	{
		RedrawSelection();
		return;
	}

	CMFCToolBarButton* pOldSel = m_pSelButton;
	m_pSelButton = pButton;

	CRect rectClient;
	GetClientRect(&rectClient);

	CRect rectSelected;
	rectSelected.SetRectEmpty();

	for (POSITION pos = m_Buttons.GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarButton* pListButton = (CMFCToolBarButton*) m_Buttons.GetNext(pos);
		ENSURE(pListButton != NULL);

		CRect rect = pListButton->Rect();
		rect.OffsetRect(0, -m_iScrollOffset);

		if (pListButton == m_pSelButton)
		{
			rectSelected = rect;
		}

		if (pListButton == m_pSelButton || pListButton == pOldSel)
		{
			rect.InflateRect(2, 2);

			CRect rectInter;
			if (rectInter.IntersectRect(rectClient, rect))
			{
				InvalidateRect(&rectInter);
			}
		}
	}

	if (!rectSelected.IsRectEmpty())
	{
		if (rectSelected.top >= rectClient.bottom || rectSelected.bottom <= rectClient.top)
		{
			int iNewOffset = max(0, min(rectSelected.bottom - m_iScrollOffset - rectClient.Height(), m_iScrollTotal));
			SetScrollPos(SB_VERT, iNewOffset);

			m_iScrollOffset = iNewOffset;
			Invalidate();
		}
	}

	UpdateWindow();

	// Trigger mouse up event(to button click notification):
	CWnd* pParent = GetParent();
	if (pParent != NULL)
	{
		pParent->SendMessage( WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), BN_CLICKED), (LPARAM) m_hWnd);
	}
}

BOOL CMFCToolBarButtonsListButton::SelectButton(int iImage)
{
	if (iImage < 0)
	{
		SelectButton((CMFCToolBarButton*) NULL);
		return TRUE;
	}

	for (POSITION pos = m_Buttons.GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarButton* pListButton = (CMFCToolBarButton*) m_Buttons.GetNext(pos);
		ENSURE(pListButton != NULL);

		if (pListButton->GetImage() == iImage)
		{
			SelectButton(pListButton);
			return TRUE;
		}
	}

	return FALSE;
}

void CMFCToolBarButtonsListButton::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/)
{
	int iScrollOffset = m_iScrollOffset;

	switch (nSBCode)
	{
	case SB_TOP:
		iScrollOffset = 0;
		break;

	case SB_BOTTOM:
		iScrollOffset = m_iScrollTotal;
		break;

	case SB_LINEUP:
		iScrollOffset -= m_sizeButton.cy + nYOffset;
		break;

	case SB_LINEDOWN:
		iScrollOffset += m_sizeButton.cy + nYOffset;
		break;

	case SB_PAGEUP:
		iScrollOffset -= m_iScrollPage *(m_sizeButton.cy + nYOffset);
		break;

	case SB_PAGEDOWN:
		iScrollOffset += m_iScrollPage *(m_sizeButton.cy + nYOffset);
		break;

	case SB_THUMBPOSITION:
		iScrollOffset = ((m_sizeButton.cy + nYOffset) / 2 + nPos) /
			(m_sizeButton.cy + nYOffset) *(m_sizeButton.cy + nYOffset);
		break;

	default:
		return;
	}

	iScrollOffset = min(m_iScrollTotal, max(iScrollOffset, 0));

	if (iScrollOffset != m_iScrollOffset)
	{
		m_iScrollOffset = iScrollOffset;
		SetScrollPos(SB_VERT, m_iScrollOffset);

		CRect rectClient; // Client area rectangle
		GetClientRect(&rectClient);

		rectClient.right -= ::GetSystemMetrics(SM_CXVSCROLL) + 2;
		rectClient.InflateRect(-1, -1);

		InvalidateRect(rectClient);
	}
}

CScrollBar* CMFCToolBarButtonsListButton::GetScrollBarCtrl(int nBar) const
{
	if (nBar == SB_HORZ || m_wndScrollBar.GetSafeHwnd() == NULL)
	{
		return NULL;
	}

	return(CScrollBar* ) &m_wndScrollBar;
}

void CMFCToolBarButtonsListButton::OnEnable(BOOL bEnable)
{
	CButton::OnEnable(bEnable);

	RedrawWindow();
}

void CMFCToolBarButtonsListButton::OnSysColorChange()
{
	if (m_pImages == NULL)
	{
		return;
	}

	m_pImages->OnSysColorChange();
	RedrawWindow();
}

void CMFCToolBarButtonsListButton::RebuildLocations()
{
	if (GetSafeHwnd() == NULL || m_Buttons.IsEmpty())
	{
		return;
	}

	CRect rectClient;
	GetClientRect(&rectClient);

	CRect rectButtons = rectClient;

	rectButtons.right -= ::GetSystemMetrics(SM_CXVSCROLL) + 1;
	rectButtons.InflateRect(-nXOffset, -nYOffset);

	int x = rectButtons.left;
	int y = rectButtons.top - m_iScrollOffset;

	CClientDC dc(this);

	for (POSITION pos = m_Buttons.GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarButton* pButton = (CMFCToolBarButton*) m_Buttons.GetNext(pos);
		ENSURE(pButton != NULL);

		CSize sizeButton = pButton->OnCalculateSize(&dc, m_sizeButton, TRUE);

		if (x + sizeButton.cx > rectButtons.right)
		{
			if (x == rectButtons.left)
			{
				sizeButton.cx = rectButtons.right - rectButtons.left;
			}
			else
			{
				x = rectButtons.left;
				y += sizeButton.cy + nYOffset;
			}
		}

		pButton->SetRect(CRect(CPoint(x, y), CSize(sizeButton.cx, m_sizeButton.cy)));

		x += sizeButton.cx + nXOffset;
	}

	CMFCToolBarButton* pLastButton = (CMFCToolBarButton*) m_Buttons.GetTail();
	ENSURE(pLastButton != NULL);

	int iVisibleRows = rectButtons.Height() /(m_sizeButton.cy + nYOffset);
	int iTotalRows = pLastButton->Rect().bottom /(m_sizeButton.cy + nYOffset);

	int iNonVisibleRows = iTotalRows - iVisibleRows;
	if (iNonVisibleRows > 0) // Not enouth space.
	{
		if (m_wndScrollBar.GetSafeHwnd() == NULL)
		{
			CRect rectSB;
			GetClientRect(&rectSB);

			rectSB.InflateRect(-1, -1);
			rectSB.left = rectSB.right - ::GetSystemMetrics(SM_CXVSCROLL) - 1;

			m_wndScrollBar.Create(WS_CHILD | WS_VISIBLE | SBS_VERT, rectSB, this, 1);
		}

		m_iScrollTotal = iNonVisibleRows *(m_sizeButton.cy + nYOffset);
		m_iScrollPage = iVisibleRows;

		SetScrollRange(SB_VERT, 0, m_iScrollTotal);
	}

	m_bInited = TRUE;
}

void CMFCToolBarButtonsListButton::OnSize(UINT nType, int cx, int cy)
{
	CButton::OnSize(nType, cx, cy);
	RebuildLocations();
}

HBRUSH CMFCToolBarButtonsListButton::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);

	for (POSITION pos = m_Buttons.GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarButton* pButton = (CMFCToolBarButton*) m_Buttons.GetNext(pos);
		ASSERT_VALID(pButton);

		HWND hwdList = pButton->GetHwnd();
		if (hwdList == NULL) // No control
		{
			continue;
		}

		if (hwdList == pWnd->GetSafeHwnd() || ::IsChild(hwdList, pWnd->GetSafeHwnd()))
		{
			HBRUSH hbrButton = pButton->OnCtlColor(pDC, nCtlColor);
			return(hbrButton == NULL) ? hbr : hbrButton;
		}
	}

	return hbr;
}

UINT CMFCToolBarButtonsListButton::OnGetDlgCode()
{
	return DLGC_WANTARROWS;
}

void CMFCToolBarButtonsListButton::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case VK_LEFT:
	case VK_UP:
		if (m_pSelButton != NULL)
		{
			POSITION pos = m_Buttons.Find(m_pSelButton);
			if (pos != NULL)
			{
				m_Buttons.GetPrev(pos);
				if (pos != NULL)
				{
					SelectButton((CMFCToolBarButton*) m_Buttons.GetAt(pos));
				}
			}
		}
		else if (!m_Buttons.IsEmpty())
		{
			SelectButton((CMFCToolBarButton*) m_Buttons.GetHead());
		}

		return;

	case VK_RIGHT:
	case VK_DOWN:
		if (m_pSelButton != NULL)
		{
			POSITION pos = m_Buttons.Find(m_pSelButton);
			if (pos != NULL)
			{
				m_Buttons.GetNext(pos);
				if (pos != NULL)
				{
					SelectButton((CMFCToolBarButton*) m_Buttons.GetAt(pos));
				}
			}
		}
		else if (!m_Buttons.IsEmpty())
		{
			SelectButton((CMFCToolBarButton*) m_Buttons.GetHead());
		}
		return;

	case VK_HOME:
		if (!m_Buttons.IsEmpty())
		{
			SelectButton((CMFCToolBarButton*) m_Buttons.GetHead());
		}
		return;

	case VK_END:
		if (m_Buttons.IsEmpty())
		{
			SelectButton((CMFCToolBarButton*) m_Buttons.GetTail());
		}
		return;
	}

	CButton::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CMFCToolBarButtonsListButton::RedrawSelection()
{
	if (m_pSelButton == NULL)
	{
		return;
	}

	CRect rect = m_pSelButton->Rect();
	rect.OffsetRect(0, -m_iScrollOffset);

	rect.InflateRect(2, 2);

	InvalidateRect(rect);
	UpdateWindow();
}


