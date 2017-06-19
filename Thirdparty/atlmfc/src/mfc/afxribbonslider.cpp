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
#include "afxribbonslider.h"
#include "afxvisualmanager.h"
#include "afxribbonbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMFCRibbonSlider, CMFCRibbonBaseElement)

static const int cySliderHeight = 18;
static const int cxThumbWidth = 10;

static const int nThumbIndex = 0;
static const int nSliderIndex = 1;
static const int nZoomInIndex = 2;
static const int nZoomOutIndex = 3;

// Construction/Destruction
CMFCRibbonSlider::CMFCRibbonSlider()
{
	CommonInit();
}

CMFCRibbonSlider::CMFCRibbonSlider(UINT nID, int nWidth)
{
	CommonInit();

	m_nID = nID;
	m_nWidth = nWidth;
}

CMFCRibbonSlider::~CMFCRibbonSlider()
{
}

void CMFCRibbonSlider::CommonInit()
{
	m_nMin = 0;
	m_nMax = 100;
	m_nPos = 0;
	m_nZoomIncrement = 10;
	m_bZoomButtons = FALSE;
	m_nWidth = 100;

	m_rectZoomOut.SetRectEmpty();
	m_rectZoomIn.SetRectEmpty();
	m_rectSlider.SetRectEmpty();
	m_rectThumb.SetRectEmpty();

	m_nHighlighted = -1;
	m_nPressed = -1;
}

void CMFCRibbonSlider::OnDraw(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if (m_rect.IsRectEmpty())
	{
		return;
	}

	if (m_bZoomButtons)
	{
		// Draw zoom buttons:
		CMFCVisualManager::GetInstance()->OnDrawRibbonSliderZoomButton(pDC, this, m_rectZoomOut, TRUE,
			m_bIsHighlighted && m_nHighlighted == nZoomOutIndex, m_bIsPressed && m_nPressed == nZoomOutIndex, IsDisabled());
		CMFCVisualManager::GetInstance()->OnDrawRibbonSliderZoomButton(pDC, this, m_rectZoomIn, FALSE,
			m_bIsHighlighted && m_nHighlighted == nZoomInIndex, m_bIsPressed && m_nPressed == nZoomInIndex, IsDisabled());
	}

	// Draw channel:
	CRect rectChannel = m_rectSlider;
	rectChannel.top = rectChannel.CenterPoint().y - 1;
	rectChannel.bottom = rectChannel.top + 2;

	CMFCVisualManager::GetInstance()->OnDrawRibbonSliderChannel(pDC, this, rectChannel);

	// Draw thumb:
	CMFCVisualManager::GetInstance()->OnDrawRibbonSliderThumb(pDC, this, m_rectThumb,
		m_bIsHighlighted && (m_nHighlighted == nThumbIndex || m_nHighlighted == nSliderIndex), m_bIsPressed && m_nPressed == nThumbIndex, IsDisabled());
}

void CMFCRibbonSlider::CopyFrom(const CMFCRibbonBaseElement& s)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::CopyFrom(s);
	CMFCRibbonSlider& src = (CMFCRibbonSlider&) s;

	m_nMin = src.m_nMin;
	m_nMax = src.m_nMax;
	m_nPos = src.m_nPos;
	m_bZoomButtons = src.m_bZoomButtons;
	m_nWidth = src.m_nWidth;
}

CSize CMFCRibbonSlider::GetRegularSize(CDC* /*pDC*/)
{
	ASSERT_VALID(this);

	CSize size(m_nWidth, cySliderHeight);

	double dblScale = afxGlobalData.GetRibbonImageScale();

	if (dblScale > 1.)
	{
		dblScale = 1. +(dblScale - 1.) / 2;
		size.cy = (int)(.5 + dblScale * size.cy);
	}

	if (m_bZoomButtons)
	{
		size.cx += 2 * size.cy;
	}

	return size;
}

void CMFCRibbonSlider::SetRange(int nMin, int nMax)
{
	ASSERT_VALID(this);

	m_nMin = nMin;
	m_nMax = nMax;
}

void CMFCRibbonSlider::SetPos(int nPos, BOOL bRedraw)
{
	ASSERT_VALID(this);

	m_nPos = min(max(m_nMin, nPos), m_nMax);
	SetThumbRect();

	if (bRedraw)
	{
		Redraw();
	}
}

void CMFCRibbonSlider::SetZoomIncrement(int nZoomIncrement)
{
	ASSERT_VALID(this);
	m_nZoomIncrement = nZoomIncrement;
}

void CMFCRibbonSlider::SetZoomButtons(BOOL bSet)
{
	ASSERT_VALID(this);
	m_bZoomButtons = bSet;
}

void CMFCRibbonSlider::OnAfterChangeRect(CDC* pDC)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::OnAfterChangeRect(pDC);

	m_rectZoomOut.SetRectEmpty();
	m_rectZoomIn.SetRectEmpty();
	m_rectThumb.SetRectEmpty();

	m_rectSlider = m_rect;

	if (m_rect.IsRectEmpty())
	{
		return;
	}

	if (m_bZoomButtons)
	{
		m_rectZoomOut = m_rectSlider;
		m_rectZoomOut.right = m_rectZoomOut.left + m_rectZoomOut.Height();

		m_rectZoomIn = m_rectSlider;
		m_rectZoomIn.left = m_rectZoomIn.right - m_rectZoomIn.Height();

		m_rectSlider.left = m_rectZoomOut.right;
		m_rectSlider.right = m_rectZoomIn.left;
	}

	int nThumbWidth = cxThumbWidth;

	double dblScale = afxGlobalData.GetRibbonImageScale();

	if (dblScale > 1.)
	{
		dblScale = 1. +(dblScale - 1.) / 2;
		nThumbWidth = (int)(.5 + dblScale * nThumbWidth);
	}

	m_rectSlider.DeflateRect(nThumbWidth / 2, 0);

	SetThumbRect();
}

void CMFCRibbonSlider::SetThumbRect()
{
	ASSERT_VALID(this);

	if (m_nMax <= m_nMin || m_rect.IsRectEmpty())
	{
		m_rectThumb.SetRectEmpty();
		return;
	}

	m_rectThumb = m_rectSlider;

	double dx = ((double) m_rectSlider.Width()) /(m_nMax - m_nMin);
	int xOffset = (int)(.5 + dx *(m_nPos - m_nMin));

	int nThumbWidth = cxThumbWidth;

	double dblScale = afxGlobalData.GetRibbonImageScale();

	if (dblScale > 1.)
	{
		dblScale = 1. +(dblScale - 1.) / 2;

		nThumbWidth = (int)(.5 + dblScale * nThumbWidth);
		m_rectThumb.DeflateRect(0, 4);
	}

	m_rectThumb.left += xOffset - nThumbWidth / 2;
	m_rectThumb.right = m_rectThumb.left + nThumbWidth;
}

void CMFCRibbonSlider::OnLButtonDown(CPoint point)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::OnLButtonDown(point);

	if (!IsDisabled())
	{
		m_nPressed = GetHitTest(point);
		Redraw();
	}
}

void CMFCRibbonSlider::OnLButtonUp(CPoint point)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::OnLButtonUp(point);

	if (m_nPressed == m_nHighlighted && !IsDisabled())
	{
		switch(m_nPressed)
		{
		case nZoomInIndex:
			SetPos(m_nPos + m_nZoomIncrement);
			break;

		case nZoomOutIndex:
			SetPos(m_nPos - m_nZoomIncrement);
			break;

		default:
			SetPos(GetPosFromPoint(point.x));
			break;
		}

		NotifyCommand();
	}
}

void CMFCRibbonSlider::OnMouseMove(CPoint point)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::OnMouseMove(point);

	if (IsDisabled())
	{
		return;
	}

	int nHighlightedOld = m_nHighlighted;

	m_nHighlighted = GetHitTest(point);

	if (nHighlightedOld != m_nHighlighted)
	{
		Redraw();
	}

	if (m_bIsPressed)
	{
		if (m_nPressed == nThumbIndex)
		{
			int nPos = GetPosFromPoint(point.x);
			if (nPos != m_nPos)
			{
				SetPos(nPos);
				NotifyCommand();
			}
		}
	}
}

int CMFCRibbonSlider::GetHitTest(CPoint point) const
{
	ASSERT_VALID(this);

	if (m_rectThumb.PtInRect(point))
	{
		return nThumbIndex;
	}

	if (m_rectSlider.PtInRect(point))
	{
		return nSliderIndex;
	}

	if (m_rectZoomOut.PtInRect(point))
	{
		return nZoomOutIndex;
	}

	if (m_rectZoomIn.PtInRect(point))
	{
		return nZoomInIndex;
	}

	return -1;
}

int CMFCRibbonSlider::GetPosFromPoint(int x)
{
	ASSERT_VALID(this);

	if (m_nMax <= m_nMin || m_rect.IsRectEmpty())
	{
		return m_nMin;
	}

	double dx = ((double) m_rectSlider.Width()) /(m_nMax - m_nMin);
	int xOffset = x - m_rectSlider.left;

	return m_nMin +(int)((double) xOffset / dx);
}

BOOL CMFCRibbonSlider::IsAutoRepeatMode(int& /*nDelay*/) const
{
	ASSERT_VALID(this);
	return m_nPressed == nZoomInIndex || m_nPressed == nZoomOutIndex;
}

BOOL CMFCRibbonSlider::OnAutoRepeat()
{
	ASSERT_VALID(this);

	if (m_bIsDisabled)
	{
		return FALSE;
	}

	if (m_nPressed == nZoomInIndex)
	{
		SetPos(m_nPos + m_nZoomIncrement);
		NotifyCommand();
	}
	else if (m_nPressed == nZoomOutIndex)
	{
		SetPos(m_nPos - m_nZoomIncrement);
		NotifyCommand();
	}

	return TRUE;
}

void CMFCRibbonSlider::NotifyCommand()
{
	ASSERT_VALID(this);

	if (m_nID == 0 || m_nID == (UINT)-1)
	{
		return;
	}

	CMFCRibbonBar* pRibbonBar = GetTopLevelRibbonBar();
	if (pRibbonBar == NULL)
	{
		return;
	}

	ASSERT_VALID(pRibbonBar);

	CWnd* pWndParent = pRibbonBar->GetParent();
	if (pWndParent == NULL)
	{
		return;
	}

	pWndParent->SendMessage(WM_COMMAND, m_nID);
}

void CMFCRibbonSlider::OnDrawOnList(CDC* pDC, CString strText, int nTextOffset, CRect rect, BOOL /*bIsSelected*/, BOOL /*bHighlighted*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	const int nSliderWidth = rect.Height() * 2;

	BOOL bIsDisabled = m_bIsDisabled;
	m_bIsDisabled = FALSE;

	CRect rectText = rect;

	rectText.left += nTextOffset;
	rectText.right -= nSliderWidth;

	const int nXMargin = 3;
	rectText.DeflateRect(nXMargin, 0);

	pDC->DrawText(strText, rectText, DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

	// Draw channel:
	CRect rectChannel = rect;
	rectChannel.left = rectChannel.right - nSliderWidth;

	rectChannel.top = rectChannel.CenterPoint().y - 1;
	rectChannel.bottom = rectChannel.top + 2;

	CMFCVisualManager::GetInstance()->OnDrawRibbonSliderChannel(pDC, this, rectChannel);

	// Draw thumb:
	CRect rectThumb = rect;
	rectThumb.left = rectThumb.right - nSliderWidth;

	rectThumb.left = rectThumb.CenterPoint().x - 2;
	rectThumb.right = rectThumb.CenterPoint().x + 2;
	rectThumb.DeflateRect(0, 1);

	CMFCVisualManager::GetInstance()->OnDrawRibbonSliderThumb(pDC, this, rectThumb, FALSE, FALSE, FALSE);

	m_bIsDisabled = bIsDisabled;
}


