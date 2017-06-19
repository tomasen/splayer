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
#include "afxshowallbutton.h"
#include "afxmenubar.h"
#include "afxpopupmenubar.h"
#include "afxpopupmenu.h"
#include "afxglobals.h"
#include "afxacceleratorkey.h"
#include "afxribbonres.h"
#include "afxvisualmanager.h"
#include "afxdrawmanager.h"

IMPLEMENT_DYNCREATE(CMFCShowAllButton, CMFCToolBarMenuButton)

const int nMinMenuWidth = 50;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFCShowAllButton::CMFCShowAllButton()
{
}

CMFCShowAllButton::~CMFCShowAllButton()
{
}

void CMFCShowAllButton::OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* /*pImages*/, BOOL /*bHorz*/, BOOL /*bCustomizeMode*/, BOOL bHighlight, BOOL /*bDrawBorder*/, BOOL /*bGrayDisabledButtons*/)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(this);

	CRect rectBar = rect;
	rectBar.DeflateRect(1, 1);

	// Fill button interior:
	FillInterior(pDC, rect, bHighlight);

	// Draw "show all" image:
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

	CMFCVisualManager::GetInstance()->OnDrawShowAllMenuItems(pDC, rectBar, state);

	// Draw button border:
	if (m_nStyle &(TBBS_PRESSED | TBBS_CHECKED))
	{
		// Pressed in or checked:
		CMFCVisualManager::GetInstance()->OnDrawButtonBorder(pDC, this, rectBar, CMFCVisualManager::ButtonsIsPressed);
	}
	else if (bHighlight)
	{
		CMFCVisualManager::GetInstance()->OnDrawButtonBorder(pDC, this, rectBar, CMFCVisualManager::ButtonsIsHighlighted);
	}
}

SIZE CMFCShowAllButton::OnCalculateSize(CDC* pDC, const CSize& sizeDefault, BOOL /*bHorz*/)
{
	return CSize(nMinMenuWidth, CMFCVisualManager::GetInstance()->GetShowAllMenuItemsHeight(pDC, sizeDefault));
}

BOOL CMFCShowAllButton::OnClick(CWnd* /*pWnd*/, BOOL bDelay)
{
	CMFCPopupMenuBar* pParentMenuBar = DYNAMIC_DOWNCAST(CMFCPopupMenuBar, m_pWndParent);
	if (pParentMenuBar == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (bDelay)
	{
		if (CMFCMenuBar::IsShowAllCommandsDelay())
		{
			pParentMenuBar->StartPopupMenuTimer(this, 2);
		}

		return TRUE;
	}

	CMFCPopupMenu* pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, pParentMenuBar->GetParent());
	if (pParentMenu == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	pParentMenu->ShowAllCommands();
	return TRUE;
}

BOOL CMFCShowAllButton::OpenPopupMenu(CWnd* pWnd)
{
	return OnClick(pWnd, FALSE);
}

BOOL CMFCShowAllButton::OnToolHitTest(const CWnd* /*pWnd*/, TOOLINFO* pTI)
{
	if (pTI == NULL)
	{
		return FALSE;
	}

	CString strKey;

	ACCEL accel;
	accel.fVirt = FVIRTKEY | FCONTROL;
	accel.key = VK_DOWN;

	CMFCAcceleratorKey helper(&accel);
	helper.Format(strKey);

	CString strText;
	strText.Format(IDS_AFXBARRES_EXPAND_FMT, strKey);

	pTI->lpszText = (LPTSTR) ::calloc((strText.GetLength() + 1), sizeof(TCHAR));
	if (pTI->lpszText == NULL)
	{
		return FALSE;
	}

	lstrcpy(pTI->lpszText, strText);

	pTI->uId = 0;
	return TRUE;
}


