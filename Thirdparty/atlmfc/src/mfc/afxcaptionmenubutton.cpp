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
#include "afxvisualmanager.h"
#include "afxcontextmenumanager.h"
#include "afxcaptionbutton.h"
#include "afxcaptionmenubutton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCCaptionMenuButton

CMFCCaptionMenuButton::CMFCCaptionMenuButton()
{
	m_nMenuResult = 0;
	m_bOSMenu = TRUE;
	m_bMenuIsActive = FALSE;
}

CMFCCaptionMenuButton::CMFCCaptionMenuButton(UINT nHit, BOOL bLeftAlign) :
	CMFCCaptionButton(nHit, bLeftAlign)
{
	m_nMenuResult = 0;
	m_bOSMenu = TRUE;
	m_bMenuIsActive = FALSE;
}

CMFCCaptionMenuButton::~CMFCCaptionMenuButton()
{
}

/////////////////////////////////////////////////////////////////////////////
// CMFCCaptionMenuButton message handlers

void CMFCCaptionMenuButton::OnDraw(CDC* pDC, BOOL bActive, BOOL bHorz, BOOL bMaximized, BOOL bDisabled)
{
	if (m_bHidden)
	{
		return;
	}

	CMFCVisualManager::GetInstance()->OnDrawCaptionButton(pDC, this, bActive, bHorz, bMaximized, bDisabled);
}

void CMFCCaptionMenuButton::ShowMenu(HMENU hMenu, CWnd* pWndOwner)
{
	ASSERT_VALID(pWndOwner);

	CRect rectWnd;
	pWndOwner->GetWindowRect(&rectWnd);
	CSize size = GetSize();
	CPoint point = m_ptOrg + rectWnd.TopLeft();
	point.x += size.cx;
	point.y += size.cy;

	m_bMenuIsActive = TRUE;
	m_bPushed = TRUE;
	pWndOwner->InvalidateRect(GetRect());
	pWndOwner->SendMessage(WM_NCPAINT);

	if (!m_bOSMenu && afxContextMenuManager != NULL)
	{
		m_nMenuResult = afxContextMenuManager->TrackPopupMenu(hMenu, point.x, point.y, pWndOwner, TRUE /* RightAlign */);
	}
	else
	{
		m_nMenuResult = ::TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, point.x, point.y, 0, pWndOwner->GetSafeHwnd(), NULL);
	}

	m_bMenuIsActive = FALSE;
	m_bPushed = FALSE;
	pWndOwner->InvalidateRect(GetRect());
	pWndOwner->SendMessage(WM_NCPAINT);
}


