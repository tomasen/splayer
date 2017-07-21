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
#include "afxglobals.h"
#include "afxribboncheckbox.h"
#include "afxvisualmanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMFCRibbonCheckBox, CMFCRibbonButton)

#define AFX_CHECK_BOX_DEFAULT_SIZE (afxGlobalData.GetRibbonImageScale() == 1. ? 16 : 20)

const int nTextMarginLeft = 4;
const int nTextMarginRight = 6;

// Construction/Destruction
CMFCRibbonCheckBox::CMFCRibbonCheckBox()
{
}

CMFCRibbonCheckBox::CMFCRibbonCheckBox(UINT nID, LPCTSTR lpszText) :
	CMFCRibbonButton(nID, lpszText)
{
}

CMFCRibbonCheckBox::~CMFCRibbonCheckBox()
{
}

// Overrides
CSize CMFCRibbonCheckBox::GetIntermediateSize(CDC* /*pDC*/)
{
	ASSERT_VALID(this);
	m_szMargin = CSize(2, 3);

	const CSize sizeCheckBox = CSize(AFX_CHECK_BOX_DEFAULT_SIZE, AFX_CHECK_BOX_DEFAULT_SIZE);

	int cx = sizeCheckBox.cx + m_sizeTextRight.cx + nTextMarginLeft + nTextMarginRight + m_szMargin.cx;
	int cy = max(sizeCheckBox.cy, m_sizeTextRight.cy) + 2 * m_szMargin.cy;

	return CSize(cx, cy);
}

void CMFCRibbonCheckBox::OnDraw(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if (m_rect.IsRectEmpty())
	{
		return;
	}

	const CSize sizeCheckBox = CSize(AFX_CHECK_BOX_DEFAULT_SIZE, AFX_CHECK_BOX_DEFAULT_SIZE);

	// Draw check box:
	CRect rectCheck = m_rect;
	rectCheck.DeflateRect(m_szMargin);
	rectCheck.left++;
	rectCheck.right = rectCheck.left + sizeCheckBox.cx;
	rectCheck.top = rectCheck.CenterPoint().y - sizeCheckBox.cx / 2;

	rectCheck.bottom = rectCheck.top + sizeCheckBox.cy;

	const BOOL bIsHighlighted = (IsHighlighted() || IsFocused()) && !IsDisabled();

	CMFCVisualManager::GetInstance()->OnDrawCheckBoxEx(pDC, rectCheck, IsChecked() ||(IsPressed() && bIsHighlighted) ? 1 : 0,
		bIsHighlighted, IsPressed() && bIsHighlighted, !IsDisabled());

	// Draw text:
	COLORREF clrTextOld = (COLORREF)-1;

	if (m_bIsDisabled)
	{
		if (m_bQuickAccessMode)
		{
			clrTextOld = pDC->SetTextColor(CMFCVisualManager::GetInstance()->GetRibbonQuickAccessToolBarTextColor(TRUE));
		}
		else
		{
			clrTextOld = pDC->SetTextColor(CMFCVisualManager::GetInstance()->GetToolbarDisabledTextColor());
		}
	}

	CRect rectText = m_rect;
	rectText.left = rectCheck.right + nTextMarginLeft;

	DrawRibbonText(pDC, m_strText, rectText, DT_SINGLELINE | DT_VCENTER);

	if (clrTextOld != (COLORREF)-1)
	{
		pDC->SetTextColor(clrTextOld);
	}
}

void CMFCRibbonCheckBox::OnDrawOnList(CDC* pDC, CString strText, int nTextOffset, CRect rect, BOOL bIsSelected, BOOL bHighlighted)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	BOOL bIsDisabled = m_bIsDisabled;
	m_bIsDisabled = FALSE;

	CRect rectCheck = rect;
	rectCheck.right = rect.left + nTextOffset;

	if (rectCheck.Width() > rectCheck.Height())
	{
		rectCheck.left = rectCheck.CenterPoint().x - rectCheck.Height() / 2;
		rectCheck.right = rectCheck.left + rectCheck.Height();
	}
	else
	{
		rectCheck.top = rectCheck.CenterPoint().y - rectCheck.Width() / 2;
		rectCheck.bottom = rectCheck.top + rectCheck.Width();
	}

	CMFCVisualManager::GetInstance()->OnDrawRibbonCheckBoxOnList(pDC, this, rectCheck, bIsSelected, bHighlighted);

	rect.left += nTextOffset;

	const int nXMargin = 3;
	rect.DeflateRect(nXMargin, 0);

	pDC->DrawText(strText, rect, DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	m_bIsDisabled = bIsDisabled;
}



