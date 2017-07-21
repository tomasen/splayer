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
#include "afxcolorpropertysheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCColorPropertySheet

IMPLEMENT_DYNAMIC(CMFCColorPropertySheet, CPropertySheet)

CMFCColorPropertySheet::CMFCColorPropertySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	: CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	m_hAccel = NULL;
}

CMFCColorPropertySheet::CMFCColorPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	: CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	m_hAccel = NULL;
}

CMFCColorPropertySheet::~CMFCColorPropertySheet()
{
}

BEGIN_MESSAGE_MAP(CMFCColorPropertySheet, CPropertySheet)
	//{{AFX_MSG_MAP(CMFCColorPropertySheet)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCColorPropertySheet message handlers

void CMFCColorPropertySheet::PostNcDestroy()
{
	// Call the base class routine first
	CPropertySheet::PostNcDestroy();

	if (m_bModeless)
	{
		delete this;
	}
}

BOOL CMFCColorPropertySheet::OnInitDialog()
{
	ASSERT_VALID(this);

	// Call the base class routine
	BOOL bRtnValue = CPropertySheet::OnInitDialog();

	ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	ResizeControl();
	return bRtnValue;
}

BOOL CMFCColorPropertySheet::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR* pNMHDR = (NMHDR*) lParam;
	ENSURE(pNMHDR != NULL);

	if (pNMHDR->code == TCN_SELCHANGE)
	{
		ResizeControl();
	}

	return CPropertySheet::OnNotify(wParam, lParam, pResult);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
void CMFCColorPropertySheet::LoadAcceleratorTable(UINT nAccelTableID /*=0*/)
{
	if (nAccelTableID)
	{
		m_hAccel = ::LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(nAccelTableID));
		ASSERT(m_hAccel);
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
BOOL CMFCColorPropertySheet::PreTranslateMessage(MSG* pMsg)
{
	//TRACE("[%s - %d] - CMFCColorPropertySheet::PreTranslateMessage().....\n", __FILE__,__LINE__);
	////////
	// Check to see if the property sheet has an accelerator table
	// attached to it. If there is one call it. Return TRUE if it has
	// been processed. Otherwise, past it to the base class function.
	////////
	if (m_hAccel && ::TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
	{
		return TRUE;
	}

	return CPropertySheet::PreTranslateMessage(pMsg);
}

void CMFCColorPropertySheet::OnSize(UINT nType, int cx, int cy)
{
	CPropertySheet::OnSize(nType, cx, cy);
	ResizeControl();
}

void CMFCColorPropertySheet::ResizeControl()
{
	CTabCtrl* pTabCtrl = GetTabControl();
	if (pTabCtrl == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	pTabCtrl->SetWindowPos(NULL, 0, 0, rectClient.Width(), rectClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE);

	int nPageCount = CPropertySheet::GetPageCount();

	int nXBorder = ::GetSystemMetrics(SM_CXEDGE);
	int nYBorder = ::GetSystemMetrics(SM_CYEDGE);

	for (int nPage = 0; nPage <= nPageCount - 1; nPage++)
	{
		CPropertyPage* pPage = GetPage(nPage);

		if ((pPage != NULL) &&(pPage->m_hWnd != NULL))
		{
			CRect rcTabCtrl;
			pPage->GetWindowRect(&rcTabCtrl);
			pTabCtrl->ScreenToClient(rcTabCtrl);

			pPage->SetWindowPos(NULL, rcTabCtrl.left, rcTabCtrl.top, rectClient.Width() -(nXBorder * 3),
				rectClient.Height() -(rcTabCtrl.top + nYBorder), SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}
}


