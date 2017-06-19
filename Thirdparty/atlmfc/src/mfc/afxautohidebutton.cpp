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
#include "afxautohidebutton.h"
#include "afxdockablepane.h"
#include "afxglobals.h"
#include "afxglobalutils.h"
#include "afxvisualmanager.h"
#include "afxdockingpanesrow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMFCAutoHideButton, CObject)

int CMFCAutoHideButton::m_nBorderSize = 1;
int CMFCAutoHideButton::m_nMarginSize = 2; // from border to icon
int CMFCAutoHideButton::m_nTextMargin = 10;
int CMFCAutoHideButton::m_nTextSizeNoIcon = 20;
BOOL CMFCAutoHideButton::m_bOverlappingTabs = TRUE;

// Construction/Destruction
CMFCAutoHideButton::CMFCAutoHideButton()
{
	m_bTop = FALSE;
	m_pParentBar = NULL;
	m_pAutoHideWindow = NULL;
	m_dwAlignment = 0;
	m_bVisible = FALSE;
	m_rect.SetRectEmpty();
}

CMFCAutoHideButton::~CMFCAutoHideButton()
{
}

BOOL CMFCAutoHideButton::Create(CMFCAutoHideBar* pParentBar, CDockablePane* pAutoHideWnd, DWORD dwAlignment)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pAutoHideWnd);
	ASSERT(dwAlignment & CBRS_ALIGN_ANY);

	m_pParentBar = pParentBar;
	m_pAutoHideWindow = pAutoHideWnd;
	m_dwAlignment = dwAlignment;

	pAutoHideWnd->SetAutoHideParents(pParentBar, this);

	CSize size = GetSize();
	m_rect.SetRect(0, 0, size.cx, size.cy);

	m_bVisible = TRUE;

	return TRUE;
}

void CMFCAutoHideButton::Move(int nOffset)
{
	ASSERT_VALID(this);
	// nOffset in pixels
	IsHorizontal() ? m_rect.OffsetRect(nOffset, 0) : m_rect.OffsetRect(0, nOffset);
}

CSize CMFCAutoHideButton::GetSize() const
{
	ASSERT_VALID(this);

	const BOOL bIsOverlapped = (CMFCVisualManager::GetInstance()->HasOverlappedAutoHideButtons());

	CSize size(m_nMarginSize + 2 * m_nBorderSize, m_nMarginSize + 2 * m_nBorderSize);

	if (m_pAutoHideWindow != NULL)
	{
		BOOL bHorz = IsHorizontal();
		HICON hIcon = m_pAutoHideWindow->GetPaneIcon(FALSE);
		CSize sizeText = GetTextSize();

		CSize sizeIcon(0, 0);
		if (hIcon != NULL)
		{
			sizeIcon = afxGlobalData.m_sizeSmallIcon;
		}

		int nSpacing = 0;

		if (bIsOverlapped)
		{
			if (bHorz)
			{
				int cy = max(sizeIcon.cy, sizeText.cy) + size.cy;
				nSpacing = cy * 2 / 3 + afxGlobalData.GetTextHeight();
			}
			else
			{
				int cx = max(sizeIcon.cx, sizeText.cx) + size.cx;
				nSpacing = cx * 2 / 3 + afxGlobalData.GetTextHeight();
			}
		}
		else
		{
			if (hIcon != NULL && (sizeText.cx > 0 && bHorz || sizeText.cy > 0 && !bHorz))
			{
				nSpacing = m_nTextMargin;
				nSpacing += IsHorizontal() ? sizeIcon.cx : sizeIcon.cy;
			}
			else if (hIcon == NULL)
			{
				nSpacing += m_nMarginSize + m_nTextSizeNoIcon;
			}
			else
			{
				nSpacing += m_nMarginSize + 1;
			}
		}

		if (bHorz)
		{
			size.cx += sizeIcon.cx + sizeText.cx + nSpacing;
			size.cy += max(sizeIcon.cy, sizeText.cy);
		}
		else
		{
			size.cx += max(sizeIcon.cx, sizeText.cx);
			size.cy += sizeIcon.cy + sizeText.cy + nSpacing;
		}

		CDockingPanesRow* pParentRow = m_pParentBar->GetDockSiteRow();
		if (pParentRow != NULL)
		{
			int nExtraSpace = pParentRow->GetExtraSpace();
			int nMaxBarSize = pParentRow->GetMaxPaneSize() - nExtraSpace;

			bHorz ? size.cy = max(size.cy, nMaxBarSize) : size.cx = max(size.cx, nMaxBarSize);
		}
	}

	return size;
}

void CMFCAutoHideButton::OnDraw(CDC* pDC)
{
	ASSERT_VALID(this);

	const BOOL bIsOverlapped = (CMFCVisualManager::GetInstance()->HasOverlappedAutoHideButtons());
	const int nTextMargin = bIsOverlapped ? (IsHorizontal() ? 5 : 7) : m_nTextMargin;

	CSize size = GetSize();
	m_rect.SetRect(0, 0, size.cx, size.cy);

	// calculate border size and draw the border
	CRect rectBorderSize(m_nBorderSize, 0, m_nBorderSize, m_nBorderSize);

	switch (m_dwAlignment & CBRS_ALIGN_ANY)
	{
	case CBRS_ALIGN_RIGHT:
		afxGlobalUtils.FlipRect(rectBorderSize, 90);
		break;
	case CBRS_ALIGN_BOTTOM:
		afxGlobalUtils.FlipRect(rectBorderSize, 180);
		break;
	case CBRS_ALIGN_LEFT:
		afxGlobalUtils.FlipRect(rectBorderSize, -90);
		break;
	}

	if (bIsOverlapped && !m_pParentBar->m_bFirstInGroup)
	{
		CRect rectPrev = m_rect;

		switch (m_dwAlignment & CBRS_ALIGN_ANY)
		{
		case CBRS_ALIGN_RIGHT:
		case CBRS_ALIGN_LEFT:
			rectPrev.OffsetRect(0, -m_rect.Height() + size.cx / 2);
			break;

		case CBRS_ALIGN_TOP:
		case CBRS_ALIGN_BOTTOM:
			rectPrev.OffsetRect(-m_rect.Width() + size.cy / 2, 0);
			break;
		}

		OnFillBackground(pDC, rectPrev);
		OnDrawBorder(pDC, rectPrev, rectBorderSize);
	}

	OnFillBackground(pDC, m_rect);
	OnDrawBorder(pDC, m_rect, rectBorderSize);

	if (m_pAutoHideWindow == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pAutoHideWindow);

	CRect rectDraw = m_rect;

	if (!bIsOverlapped)
	{
		rectDraw.DeflateRect(rectBorderSize);
	}

	rectDraw.DeflateRect(m_nMarginSize, m_nMarginSize);

	if (bIsOverlapped)
	{
		if (IsHorizontal())
		{
			const int nExtra = rectDraw.Height() * 2 / 3;
			rectDraw.DeflateRect(nExtra, 0);
		}
		else
		{
			const int nExtra = rectDraw.Width() * 2 / 3;
			rectDraw.DeflateRect(0, nExtra);
		}
	}

	if (m_pAutoHideWindow != NULL)
	{
		// draw the icon (if any)
		HICON hIcon = m_pAutoHideWindow->GetPaneIcon(FALSE);
		if (hIcon != NULL)
		{
			CSize sizeIcon(afxGlobalData.m_sizeSmallIcon);

			int dx = IsHorizontal() ? 0 : (rectDraw.Width() - sizeIcon.cx) / 2;
			int dy = IsHorizontal() ? (rectDraw.Height() - sizeIcon.cy) / 2 : 0;

			::DrawIconEx(pDC->GetSafeHdc(), rectDraw.left + dx, rectDraw.top + dy, hIcon, sizeIcon.cx, sizeIcon.cy, 0, NULL, DI_NORMAL);

			if (IsHorizontal())
			{
				rectDraw.left += nTextMargin + sizeIcon.cx;
			}
			else
			{
				rectDraw.top += nTextMargin + sizeIcon.cy;
			}
		}
		else
		{
			if (IsHorizontal())
			{
				rectDraw.left += m_nMarginSize;
			}
			else
			{
				rectDraw.top += m_nMarginSize;
			}
		}

		// Draw text:
		CString strText;
		m_pAutoHideWindow->GetWindowText(strText);

		if (!strText.IsEmpty() && m_pParentBar->m_bActiveInGroup || hIcon == NULL || !m_bOverlappingTabs)
		{
			int nOldMode = pDC->SetBkMode(TRANSPARENT);

			CFont* pFontOld = (CFont*) pDC->SelectObject(IsHorizontal() ? &afxGlobalData.fontRegular : &afxGlobalData.fontVert);
			ENSURE(pFontOld != NULL);

			pDC->SetTextColor(CMFCVisualManager::GetInstance()->GetAutoHideButtonTextColor(this));

			if (IsHorizontal())
			{
				pDC->DrawText(strText, &rectDraw, DT_SINGLELINE | DT_VCENTER);
			}
			else
			{
				TEXTMETRIC tm;
				pDC->GetTextMetrics(&tm);

				CRect rectText = rectDraw;

				rectText.left = rectText.right - (rectDraw.Width() - tm.tmHeight + 1) / 2;
				rectText.bottom = rectDraw.top + nTextMargin;

				pDC->DrawText(strText, &rectText, DT_SINGLELINE | DT_VCENTER | DT_NOCLIP);
			}

			pDC->SelectObject(pFontOld);
			pDC->SetBkMode(nOldMode);
		}
	}
}

void CMFCAutoHideButton::OnFillBackground(CDC* pDC, CRect rect)
{
	CMFCVisualManager::GetInstance()->OnFillAutoHideButtonBackground(pDC, rect, this);
}

void CMFCAutoHideButton::OnDrawBorder(CDC* pDC, CRect rectBounds, CRect rectBorderSize)
{
	CMFCVisualManager::GetInstance()->OnDrawAutoHideButtonBorder(pDC, rectBounds, rectBorderSize, this);
}

CSize CMFCAutoHideButton::GetTextSize() const
{
	CSize size(0, 0);

	if (m_pAutoHideWindow != NULL && m_pParentBar != NULL)
	{
		CString strText;
		m_pAutoHideWindow->GetWindowText(strText);

		if (!strText.IsEmpty())
		{
			CWindowDC dc(m_pParentBar);

			CFont* pFontOld = (CFont*) dc.SelectObject(IsHorizontal() ? &afxGlobalData.fontRegular : &afxGlobalData.fontVert);
			ENSURE(pFontOld != NULL);
			size = dc.GetTextExtent(strText);
			size.cx += m_nMarginSize;
			size.cy += m_nMarginSize;

			dc.SelectObject(pFontOld);

			if (!IsHorizontal())
			{
				int n = size.cy;
				size.cy = size.cx;
				size.cx = n;
			}

		}

		if (!m_pParentBar->m_bActiveInGroup && m_bOverlappingTabs)
		{
			IsHorizontal() ? size.cx = 0 : size.cy = 0;
		}
	}
	return size;
}

BOOL CMFCAutoHideButton::IsHorizontal() const
{
	return (m_dwAlignment & CBRS_ALIGN_TOP || m_dwAlignment & CBRS_ALIGN_BOTTOM);
}

void CMFCAutoHideButton::ShowAttachedWindow(BOOL bShow)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pAutoHideWindow);

	// collapse window out only if it is visible
	// expand window only if it is not visible

	if (bShow && !m_pAutoHideWindow->IsWindowVisible() || !bShow && m_pAutoHideWindow->IsWindowVisible())
	{
		m_pAutoHideWindow->Slide(bShow);
	}

	m_pParentBar->SetActiveInGroup(bShow);

}

void CMFCAutoHideButton::ShowButton(BOOL bShow)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pParentBar);

	m_bVisible = bShow;
}

void CMFCAutoHideButton::UnSetAutoHideMode(CDockablePane* pFirstBarInGroup)
{
	if (m_pAutoHideWindow != NULL)
	{
		m_pAutoHideWindow->UnSetAutoHideMode(pFirstBarInGroup);
	}
}

void CMFCAutoHideButton::ReplacePane(CDockablePane* pNewBar)
{
	ASSERT_VALID(pNewBar);
	m_pAutoHideWindow = pNewBar;
	pNewBar->SetAutoHideParents(m_pParentBar, this);
}



