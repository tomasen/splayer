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
#include "afxpopupmenu.h"
#include "afxpropertypage.h"
#include "afxtoolbarmenubutton.h"
#include "afxpropertysheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCPropertyPage

IMPLEMENT_DYNCREATE(CMFCPropertyPage, CPropertyPage)

#pragma warning(disable : 4355)

CMFCPropertyPage::CMFCPropertyPage() : m_Impl(*this)
{
	CommonInit();
}

CMFCPropertyPage::CMFCPropertyPage(UINT nIDTemplate, UINT nIDCaption) :
	CPropertyPage(nIDTemplate, nIDCaption), m_Impl(*this)
{
	CommonInit();
}

CMFCPropertyPage::CMFCPropertyPage(LPCTSTR lpszTemplateName, UINT nIDCaption) :
	CPropertyPage(lpszTemplateName, nIDCaption), m_Impl(*this)
{
	CommonInit();
}

#pragma warning(default : 4355)

void CMFCPropertyPage::CommonInit()
{
	m_pCategory = NULL;
	m_nIcon = -1;
	m_nSelIconNum = -1;
	m_hTreeNode = NULL;
}

CMFCPropertyPage::~CMFCPropertyPage()
{
}

BEGIN_MESSAGE_MAP(CMFCPropertyPage, CPropertyPage)
	//{{AFX_MSG_MAP(CMFCPropertyPage)
	ON_WM_ACTIVATE()
	ON_WM_NCACTIVATE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCPropertyPage message handlers

void CMFCPropertyPage::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	m_Impl.OnActivate(nState, pWndOther);
	CPropertyPage::OnActivate(nState, pWndOther, bMinimized);
}

BOOL CMFCPropertyPage::OnNcActivate(BOOL bActive)
{
	m_Impl.OnNcActivate(bActive);

	// Do not call the base class because it will call Default()
	// and we may have changed bActive.
	return(BOOL) DefWindowProc(WM_NCACTIVATE, bActive, 0L);
}

void CMFCPropertyPage::SetActiveMenu(CMFCPopupMenu* pMenu)
{
	m_Impl.SetActiveMenu(pMenu);
}

BOOL CMFCPropertyPage::PreTranslateMessage(MSG* pMsg)
{
	if (m_Impl.PreTranslateMessage(pMsg))
	{
		return TRUE;
	}

	return CPropertyPage::PreTranslateMessage(pMsg);
}

BOOL CMFCPropertyPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (m_Impl.OnCommand(wParam, lParam))
	{
		return TRUE;
	}

	return CPropertyPage::OnCommand(wParam, lParam);
}

BOOL CMFCPropertyPage::OnSetActive()
{
	CMFCPropertySheet* pParent = DYNAMIC_DOWNCAST(CMFCPropertySheet, GetParent());
	if (pParent != NULL)
	{
		pParent->OnActivatePage(this);
	}

	return CPropertyPage::OnSetActive();
}

BOOL CMFCPropertyPage::OnInitDialog()
{
	BOOL bRes = CPropertyPage::OnInitDialog();

	CMFCPropertySheet* pParent = DYNAMIC_DOWNCAST(CMFCPropertySheet, GetParent());
	if (pParent == NULL || pParent->GetHeaderHeight() == 0)
	{
		return bRes;
	}

	const int nHeaderHeight = pParent->GetHeaderHeight();

	CWnd* pWndChild = GetWindow(GW_CHILD);
	while (pWndChild != NULL)
	{
		CRect rectChild;
		pWndChild->GetWindowRect(rectChild);
		ScreenToClient(rectChild);

		rectChild.OffsetRect(0, nHeaderHeight);

		pWndChild->SetWindowPos(NULL, rectChild.left, rectChild.top, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		pWndChild = pWndChild->GetNextWindow();
	}

	return bRes;
}

BOOL CMFCPropertyPage::OnEraseBkgnd(CDC* pDC)
{
	BOOL bRes = CPropertyPage::OnEraseBkgnd(pDC);

	CMFCPropertySheet* pParent = DYNAMIC_DOWNCAST(CMFCPropertySheet, GetParent());
	if (pParent != NULL && pParent->GetHeaderHeight() > 0)
	{
		CRect rectClient;
		GetClientRect(rectClient);

		CRect rectHeader = rectClient;
		rectHeader.bottom = rectHeader.top + pParent->GetHeaderHeight();

		if (pParent->GetLook() == CMFCPropertySheet::PropSheetLook_OutlookBar ||
			pParent->GetLook() == CMFCPropertySheet::PropSheetLook_Tree || pParent->GetLook() == CMFCPropertySheet::PropSheetLook_List)
		{
			CRect rectParent;
			pParent->GetWindowRect(rectParent);

			ScreenToClient(rectParent);

			rectHeader.right = rectParent.right - ::GetSystemMetrics(SM_CXDLGFRAME);
		}

		pParent->OnDrawPageHeader(pDC, pParent->GetPageIndex(this), rectHeader);
	}

	return bRes;
}


