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
#include "afxtoolbarmenubuttonsbutton.h"
#include "afxvisualmanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMFCToolBarMenuButtonsButton, CMFCToolBarButton)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFCToolBarMenuButtonsButton::CMFCToolBarMenuButtonsButton()
{
	m_uiSystemCommand = 0;
}

CMFCToolBarMenuButtonsButton::CMFCToolBarMenuButtonsButton(UINT uiCmdId)
{
	if (uiCmdId != SC_CLOSE && uiCmdId != SC_MINIMIZE && uiCmdId != SC_RESTORE)
	{
		ASSERT(FALSE);
	}

	m_uiSystemCommand = uiCmdId;
}

CMFCToolBarMenuButtonsButton::~CMFCToolBarMenuButtonsButton()
{
}

void CMFCToolBarMenuButtonsButton::OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* /*pImages*/,
	BOOL /*bHorz*/, BOOL /*bCustomizeMode*/, BOOL bHighlight, BOOL /*bDrawBorder*/, BOOL /*bGrayDisabledButtons*/)
{
	CMFCVisualManager::GetInstance()->OnDrawMenuSystemButton(pDC, rect, m_uiSystemCommand, m_nStyle, bHighlight);
}

SIZE CMFCToolBarMenuButtonsButton::OnCalculateSize(CDC* /*pDC*/, const CSize& /*sizeDefault*/, BOOL /*bHorz*/)
{
	return CSize( ::GetSystemMetrics(SM_CXMENUSIZE), ::GetSystemMetrics(SM_CYMENUSIZE)); // Fixed by JX Chen
}

void CMFCToolBarMenuButtonsButton::CopyFrom(const CMFCToolBarButton& s)
{
	CMFCToolBarButton::CopyFrom(s);

	const CMFCToolBarMenuButtonsButton& src = (const CMFCToolBarMenuButtonsButton&) s;
	m_uiSystemCommand = src.m_uiSystemCommand;
}


