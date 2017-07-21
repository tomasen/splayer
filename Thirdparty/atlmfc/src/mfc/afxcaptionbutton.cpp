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
#include "afxglobals.h"
#include "afxcaptionbutton.h"
#include "afxvisualmanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int CMFCCaptionButton::m_nButtonMargin = 3;
int CMFCCaptionButton::m_nButtonMarginVert = 4;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFCCaptionButton::CMFCCaptionButton()
{
	m_bPushed = FALSE;
	m_bFocused = FALSE;
	m_bDroppedDown = FALSE;
	m_bHidden = FALSE;
	m_bEnabled = TRUE;
	m_nHit = HTNOWHERE;
	m_bLeftAlign = FALSE;
	m_clrForeground = (COLORREF)-1;
	m_bIsMiniFrameButton = FALSE;
}

CMFCCaptionButton::CMFCCaptionButton(UINT nHit, BOOL bLeftAlign)
{
	m_bPushed = FALSE;
	m_bFocused = FALSE;
	m_bDroppedDown = FALSE;
	m_bHidden = FALSE;
	m_bEnabled = TRUE;
	m_nHit = nHit;
	m_bLeftAlign = bLeftAlign;
	m_clrForeground = (COLORREF)-1;
	m_bIsMiniFrameButton = FALSE;
}

CMFCCaptionButton::~CMFCCaptionButton()
{
}

UINT CMFCCaptionButton::GetHit() const
{
	return m_nHit;
}

void CMFCCaptionButton::OnDraw(CDC* pDC, BOOL bActive, BOOL /*bHorz*/, BOOL bMaximized, BOOL bDisabled)
{
	if (m_bHidden)
	{
		return;
	}

	CMFCVisualManager::GetInstance()->OnDrawCaptionButton(pDC, this, bActive, FALSE, bMaximized, bDisabled || !m_bEnabled);
}

CMenuImages::IMAGES_IDS CMFCCaptionButton::GetIconID(BOOL bHorz, BOOL bMaximized) const
{
	switch(m_nHit)
	{
	case HTCLOSE:
	case AFX_HTCLOSE:
		return CMenuImages::IdClose;

	case HTMINBUTTON:
		return bHorz ?
			bMaximized ? CMenuImages::IdArrowLeft : CMenuImages::IdArrowRight :
			bMaximized ? CMenuImages::IdArrowDownLarge :  CMenuImages::IdArrowUp;

	case HTMAXBUTTON:
		return bMaximized ? CMenuImages::IdPinHorz : CMenuImages::IdPinVert;

	case AFX_HTLEFTBUTTON:
		return CMenuImages::IdArrowBack;

	case AFX_HTRIGHTBUTTON:
		return CMenuImages::IdArrowForward;

	case AFX_HTMENU:
		return CMenuImages::IdArrowDownLarge;
	}

	return(CMenuImages::IMAGES_IDS)-1;
}

CMFCCaptionButtonEx::CMFCCaptionButtonEx(UINT nHit)
{
	m_nHit = nHit;
	m_rect.SetRectEmpty();
}

CMFCCaptionButtonEx::~CMFCCaptionButtonEx()
{
}


