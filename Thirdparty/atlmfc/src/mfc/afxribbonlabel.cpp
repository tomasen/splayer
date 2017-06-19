// This MFC Library source code supports the Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Classes Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// http://msdn.microsoft.com/officeui.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

#include "stdafx.h"
#include "afxvisualmanager.h"
#include "afxglobals.h"
#include "afxribbonlabel.h"
#include "afxribboncategory.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMFCRibbonLabel, CMFCRibbonButton)

// Construction/Destruction
CMFCRibbonLabel::CMFCRibbonLabel()
{
}

CMFCRibbonLabel::CMFCRibbonLabel(LPCTSTR lpszText, BOOL bIsMultiLine)
{
	ENSURE(lpszText != NULL);
	m_strText = lpszText;

	m_bIsAlwaysLarge = bIsMultiLine;
}

CMFCRibbonLabel::~CMFCRibbonLabel()
{
}

BOOL CMFCRibbonLabel::SetACCData(CWnd* pParent, CAccessibilityData& data)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::SetACCData(pParent, data);

	data.m_nAccRole = ROLE_SYSTEM_GROUPING;
	data.m_bAccState = 0;
	return TRUE;
}

void CMFCRibbonLabel::OnDraw(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if (m_rect.IsRectEmpty())
	{
		return;
	}

	CRect rectText = m_rect;
	rectText.DeflateRect(m_szMargin.cx, 0);

	COLORREF cltTextOld = (COLORREF)-1;

	if (IsMenuMode())
	{
		rectText.bottom -= 2;

		COLORREF clrText = CMFCVisualManager::GetInstance()->OnDrawMenuLabel(pDC, m_rect);

		if (clrText != (COLORREF)-1)
		{
			cltTextOld = pDC->SetTextColor(clrText);
		}
	}
	else
	{
		CMFCVisualManager::GetInstance()->OnDrawRibbonLabel(pDC, this, m_rect);
	}

	CFont* pOldFont = NULL;

	if (IsMenuMode())
	{
		pOldFont = pDC->SelectObject(&afxGlobalData.fontBold);
		ASSERT_VALID(pOldFont);
	}

	UINT uiDTFlags = IsMenuMode() || !m_bIsAlwaysLarge ? DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX : DT_WORDBREAK | DT_NOPREFIX;

	if (!IsMenuMode() && m_bIsAlwaysLarge)
	{
		int dy = max(0, (rectText.Height() - m_sizeTextRight.cy) / 2);
		rectText.DeflateRect(0, dy);
	}

	DrawRibbonText(pDC, m_strText, rectText, uiDTFlags);

	if (pOldFont != NULL)
	{
		pDC->SelectObject(pOldFont);
	}

	if (cltTextOld != (COLORREF)-1)
	{
		cltTextOld = pDC->SetTextColor(cltTextOld);
	}
}

void CMFCRibbonLabel::OnAfterChangeRect(CDC* /*pDC*/)
{
	ASSERT_VALID(this);

	if (m_strToolTip.IsEmpty())
	{
		UpdateTooltipInfo();
	}
}

void CMFCRibbonLabel::OnCalcTextSize(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if (IsMenuMode() || !m_bIsAlwaysLarge)
	{
		CFont* pOldFont = NULL;

		if (IsMenuMode())
		{
			pOldFont = pDC->SelectObject(&afxGlobalData.fontBold);
			ASSERT_VALID(pOldFont);
		}

		CMFCRibbonButton::OnCalcTextSize(pDC);

		if (pOldFont != NULL)
		{
			pDC->SelectObject(pOldFont);
		}

		return;
	}

	// Multi-line label:

	ASSERT_VALID(m_pParent);

	const CSize sizeImageLarge = m_pParent->GetImageSize(TRUE);
	if (sizeImageLarge == CSize(0, 0))
	{
		ASSERT(FALSE);
		return;
	}

	const int nMaxHeight = 2 * sizeImageLarge.cy;

	int nTextHeight = 0;
	int nTextWidth = 0;

	CString strText = m_strText;

	for (int dx = 10; dx < 200; dx += 10)
	{
		CRect rectText(0, 0, dx, 10000);

		nTextHeight = pDC->DrawText(strText, rectText, DT_WORDBREAK | DT_CALCRECT);

		nTextWidth = rectText.Width();

		if (nTextHeight <= nMaxHeight && nTextWidth >= nTextHeight)
		{
			break;
		}
	}

	m_sizeTextRight.cx = nTextWidth;
	m_sizeTextRight.cy = nTextHeight;
}

CSize CMFCRibbonLabel::GetIntermediateSize(CDC* pDC)
{
	ASSERT_VALID(this);

	if (IsMenuMode())
	{
		m_szMargin = CSize(3, 3);
	}
	else
	{
		m_szMargin = CSize(2, 4);
	}

	OnCalcTextSize(pDC);
	return CSize(m_sizeTextRight.cx + 2 * m_szMargin.cx, m_sizeTextRight.cy + 2 * m_szMargin.cy);
}


