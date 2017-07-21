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
#include "afxoutlookbarpanebutton.h"
#include "afxoutlookbarpane.h"
#include "afxmenuimages.h"
#include "afxvisualmanager.h"
#include "afxdrawmanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CMFCOutlookBarPaneButton, CMFCToolBarButton, 1)

#define AFX_BUTTON_OFFSET 10
#define AFX_HIGHLIGHT_PERCENTAGE 85

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFCOutlookBarPaneButton::CMFCOutlookBarPaneButton()
{
	m_pWndParentBar = NULL;
	m_sizeImage = CSize(0, 0);
	m_bIsWholeText = TRUE;
}

CMFCOutlookBarPaneButton::~CMFCOutlookBarPaneButton()
{
}

void CMFCOutlookBarPaneButton::OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages, BOOL bHorz,
	BOOL bCustomizeMode, BOOL bHighlight, BOOL /*bDrawBorder*/, BOOL /*bGrayDisabledButtons*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pWndParentBar);

	CSize csOffset(0, 0);
	if (!bCustomizeMode &&
		(bHighlight &&(m_nStyle & TBBS_PRESSED)))
	{
		csOffset = CSize(1, 1);
	}

	CRect rectInternal = rect;
	CRect rectText = rect;

	if (m_bExtraSize)
	{
		CSize sizeExtra = CMFCVisualManager::GetInstance()->GetButtonExtraBorder();
		if (sizeExtra != CSize(0, 0))
		{
			rectInternal.DeflateRect(sizeExtra.cx / 2 + 1, sizeExtra.cy / 2 + 1);

			if (!bHorz)
			{
				rectText.OffsetRect(0, sizeExtra.cy);
			}
			else
			{
				rectText.OffsetRect(sizeExtra.cx, 0);
			}
		}
	}

	CRect rectBorder = rectInternal;
	rectText.top += AFX_BUTTON_OFFSET / 2;

	if (pImages != NULL && GetImage() >= 0)
	{
		int x, y;

		CSize csImage = pImages->GetImageSize();

		if (!bHorz)
		{
			int iImageHorzOffset = (rectInternal.Width() - csImage.cx) / 2;
			x = rectInternal.left + iImageHorzOffset;
			y = rectInternal.top + AFX_BUTTON_OFFSET / 2;

			rectText.top += csImage.cy + 2;
		}
		else
		{
			int iImageVertOffset = (rectInternal.Height() - csImage.cy) / 2;
			x = rectInternal.left + AFX_BUTTON_OFFSET / 2;
			y = rectInternal.top + iImageVertOffset;

			rectText.left += csImage.cx + AFX_BUTTON_OFFSET;

			CRect rectTextTemp = rectText;
			int iTextHeight = pDC->DrawText(m_strText, rectTextTemp, DT_CALCRECT | DT_WORDBREAK);

			rectText.top = rectInternal.top +(rectInternal.Height() - iTextHeight) / 2;
		}

		rectBorder = CRect(CPoint(x, y), csImage);
		rectBorder.InflateRect(2, 2);

		// Fill button interior:
		if (m_pWndParentBar->IsDrawShadedHighlight())
		{
			if (bHighlight && !bCustomizeMode)
			{
				CDrawingManager dm(*pDC);
				dm.HighlightRect(rectBorder, AFX_HIGHLIGHT_PERCENTAGE);
			}
		}
		else
		{
			if (m_bExtraSize)
			{
				CSize sizeExtra = CMFCVisualManager::GetInstance()->GetButtonExtraBorder();
				if (sizeExtra != CSize(0, 0))
				{
					rectBorder.InflateRect(sizeExtra.cx / 2 - 1, sizeExtra.cy / 2 - 1);
				}
			}

			FillInterior(pDC, rectBorder, bHighlight);
		}

		pImages->Draw(pDC, x + csOffset.cx, y + csOffset.cy, GetImage(), FALSE,
			(m_nStyle & TBBS_DISABLED));
	}
	else
	{
		if (bHighlight && m_pWndParentBar->IsDrawShadedHighlight() && !bCustomizeMode)
		{
			CDrawingManager dm(*pDC);
			dm.HighlightRect(rectBorder, AFX_HIGHLIGHT_PERCENTAGE);
		}
	}

	if (!bCustomizeMode && (bHighlight ||(m_nStyle & TBBS_PRESSED) ||(m_nStyle & TBBS_CHECKED)))
	{
		if (((m_nStyle & TBBS_PRESSED) && bHighlight) || (m_nStyle & TBBS_CHECKED))
		{
			CMFCVisualManager::GetInstance()->OnDrawButtonBorder(pDC, this, rectBorder, CMFCVisualManager::ButtonsIsPressed);
		}
		else
		{
			CMFCVisualManager::GetInstance()->OnDrawButtonBorder(pDC, this, rectBorder, CMFCVisualManager::ButtonsIsHighlighted);
		}
	}

	if (m_bTextBelow && !m_strText.IsEmpty())
	{
		COLORREF clrText = (COLORREF)-1;
		CMFCVisualManager::AFX_BUTTON_STATE state = CMFCVisualManager::ButtonsIsRegular;

		if (bHighlight)
		{
			state = CMFCVisualManager::ButtonsIsHighlighted;
		}
		else if (m_nStyle &(TBBS_PRESSED | TBBS_CHECKED))
		{
			// Pressed in or checked:
			state = CMFCVisualManager::ButtonsIsPressed;
		}

		if (m_nStyle & TBBS_DISABLED)
		{
			if (m_pWndParentBar->IsBackgroundTexture())
			{
				clrText = afxGlobalData.clrGrayedText;
			}
		}
		else
		{
			clrText = m_pWndParentBar->GetRegularColor();
		}

		if (clrText == (COLORREF)-1)
		{
			if (m_pWndParentBar->IsBackgroundTexture())
			{
				clrText = afxGlobalData.clrWindowText;
			}
			else
			{
				clrText = CMFCVisualManager::GetInstance()->GetToolbarButtonTextColor(this, state);
			}
		}

		pDC->SetTextColor(clrText);

		if (m_bIsWholeText)
		{
			pDC->DrawText(m_strText, rectText, DT_WORDBREAK | DT_CENTER);
		}
		else
		{
			CString strText = m_strText;
			pDC->DrawText(strText, rectText, DT_WORDBREAK | DT_END_ELLIPSIS);
		}
	}
}

SIZE CMFCOutlookBarPaneButton::OnCalculateSize(CDC* pDC, const CSize& sizeDefault, BOOL bHorz)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	CSize sizeResult = sizeDefault;

	if (!bHorz)
	{
		int nCXMargin = pDC->GetTextExtent(_T("   ")).cx;
		CRect rectText(0, 0, sizeDefault.cx - nCXMargin, 1);

		int iTextHeight = m_bTextBelow ? pDC->DrawText(m_strText, rectText, DT_CALCRECT | DT_WORDBREAK) : 0;

		sizeResult.cy = sizeDefault.cy + iTextHeight + AFX_BUTTON_OFFSET;
		sizeResult.cx = max(m_sizeImage.cx + 4, min(sizeDefault.cx, rectText.Width()));

		m_bIsWholeText = rectText.Width() <= sizeDefault.cx;
	}
	else
	{
		CRect rectText(0, 0, 0, sizeDefault.cy);
		int iTextHeight = 0;

		if (m_bTextBelow)
		{
			do
			{
				rectText.right ++;
				iTextHeight = pDC->DrawText(m_strText, rectText, DT_CALCRECT | DT_WORDBREAK);
			}
			while (iTextHeight < pDC->GetTextExtent(m_strText).cy && rectText.Height() > sizeDefault.cy);
		}

		sizeResult.cx = sizeDefault.cx + rectText.Width() + AFX_BUTTON_OFFSET;
		sizeResult.cy = max(m_sizeImage.cy, min(sizeDefault.cy, rectText.Height()));

		m_bIsWholeText = TRUE;
	}

	return sizeResult;
}

void CMFCOutlookBarPaneButton::OnChangeParentWnd(CWnd* pWndParent)
{
	CMFCToolBarButton::OnChangeParentWnd(pWndParent);

	m_pWndParentBar = DYNAMIC_DOWNCAST(CMFCOutlookBarPane, pWndParent);
	ASSERT_VALID(m_pWndParentBar);
}

BOOL CMFCOutlookBarPaneButton::CanBeDropped(CMFCToolBar* pToolbar)
{
	ASSERT_VALID(pToolbar);
	return pToolbar->IsKindOf(RUNTIME_CLASS(CMFCOutlookBarPane));
}

void CMFCOutlookBarPaneButton::SetImage(int iImage)
{
	// Don't add image to hash!
	m_iImage = iImage;
}


