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
#include "afxribbonprogressbar.h"
#include "afxvisualmanager.h"
#include "afxpopupmenu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMFCRibbonProgressBar, CMFCRibbonBaseElement)

// Construction/Destruction
CMFCRibbonProgressBar::CMFCRibbonProgressBar()
{
	CommonInit();
}

CMFCRibbonProgressBar::CMFCRibbonProgressBar( UINT nID, int nWidth, int nHeight)
{
	CommonInit();

	m_nID = nID;
	m_nWidth = nWidth;
	m_nHeight = nHeight;
}

CMFCRibbonProgressBar::~CMFCRibbonProgressBar()
{
}

void CMFCRibbonProgressBar::CommonInit()
{
	m_nMin = 0;
	m_nMax = 100;
	m_nPos = 0;
	m_nWidth = 100;
	m_nHeight = 22;
	m_bInfiniteMode = FALSE;
}

void CMFCRibbonProgressBar::OnDraw(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if (m_rect.IsRectEmpty() || m_nMax - m_nMin <= 0)
	{
		return;
	}

	CRect rectProgress = m_rect;
	rectProgress.DeflateRect(5, 5);

	CRect rectChunk = rectProgress;
	rectChunk.right = rectChunk.left + m_nPos * rectChunk.Width() /(m_nMax - m_nMin);
	rectChunk.DeflateRect(1, 1);

	CMFCVisualManager::GetInstance()->OnDrawRibbonProgressBar(pDC, this, rectProgress, rectChunk, m_bInfiniteMode);
}

void CMFCRibbonProgressBar::CopyFrom(const CMFCRibbonBaseElement& s)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::CopyFrom(s);
	CMFCRibbonProgressBar& src = (CMFCRibbonProgressBar&) s;

	m_nMin = src.m_nMin;
	m_nMax = src.m_nMax;
	m_nPos = src.m_nPos;
	m_nWidth = src.m_nWidth;
	m_nHeight = src.m_nHeight;
	m_bInfiniteMode = src.m_bInfiniteMode;
}

CSize CMFCRibbonProgressBar::GetRegularSize(CDC* /*pDC*/)
{
	ASSERT_VALID(this);

	int nHeight = m_nHeight;

	if (afxGlobalData.GetRibbonImageScale() != 1.)
	{
		nHeight = (int)(.5 + afxGlobalData.GetRibbonImageScale() * nHeight);
		nHeight -= (nHeight - m_nHeight) / 2;
	}

	return CSize(m_nWidth, nHeight);
}

void CMFCRibbonProgressBar::SetRange(int nMin, int nMax)
{
	ASSERT_VALID(this);

	m_nMin = nMin;
	m_nMax = nMax;
}

void CMFCRibbonProgressBar::SetPos(int nPos, BOOL bRedraw)
{
	ASSERT_VALID(this);

	m_nPos = min(max(m_nMin, nPos), m_nMax);

	if (bRedraw)
	{
		Redraw();

		CWnd* pMenu = CMFCPopupMenu::GetActiveMenu();

		if (pMenu != NULL && CWnd::FromHandlePermanent(pMenu->GetSafeHwnd()) != NULL && GetParentWnd() != NULL)
		{
			CRect rectScreen = m_rect;
			GetParentWnd()->ClientToScreen(&rectScreen);

			CMFCPopupMenu::UpdateAllShadows(rectScreen);
		}
	}
}

void CMFCRibbonProgressBar::SetInfiniteMode(BOOL bSet)
{
	ASSERT_VALID(this);
	m_bInfiniteMode = bSet;
}

void CMFCRibbonProgressBar::OnDrawOnList(CDC* pDC, CString strText, int nTextOffset, CRect rect, BOOL /*bIsSelected*/, BOOL /*bHighlighted*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	const int nProgressWidth = rect.Height() * 2;

	BOOL bIsDisabled = m_bIsDisabled;
	m_bIsDisabled = FALSE;

	CRect rectText = rect;

	rectText.left += nTextOffset;
	rectText.right -= nProgressWidth;

	const int nXMargin = 3;
	rectText.DeflateRect(nXMargin, 0);

	pDC->DrawText(strText, rectText, DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

	// Draw progress:
	CRect rectProgress = rect;
	rectProgress.left = rectProgress.right - nProgressWidth;

	rectProgress.DeflateRect(1, rectProgress.Height() / 4);

	CRect rectChunk(0, 0, 0, 0);

	int nPos = m_nPos;
	m_nPos = m_nMin;

	CMFCVisualManager::GetInstance()->OnDrawRibbonProgressBar(pDC, this, rectProgress, rectChunk, FALSE);

	m_bIsDisabled = bIsDisabled;
	m_nPos = nPos;
}


