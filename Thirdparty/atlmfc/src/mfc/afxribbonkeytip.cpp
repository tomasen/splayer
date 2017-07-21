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
#include "afxcontrolbarutil.h"
#include "multimon.h"

#include "afxglobals.h"
#include "afxribbonkeytip.h"
#include "afxbaseribbonelement.h"
#include "afxvisualmanager.h"
#include "afxpopupmenu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonKeyTip

CString CMFCRibbonKeyTip::m_strClassName;

CMFCRibbonKeyTip::CMFCRibbonKeyTip(CMFCRibbonBaseElement* pElement, BOOL bIsMenu)
{
	ASSERT_VALID(pElement);
	m_pElement = pElement;
	m_bIsMenu = bIsMenu;

	m_rectScreen.SetRectEmpty();
}

CMFCRibbonKeyTip::~CMFCRibbonKeyTip()
{
}

BEGIN_MESSAGE_MAP(CMFCRibbonKeyTip, CWnd)
	//{{AFX_MSG_MAP(CMFCRibbonKeyTip)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_MOUSEACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonKeyTip message handlers

void CMFCRibbonKeyTip::OnPaint()
{
	ASSERT_VALID(m_pElement);

	CPaintDC dc(this); // device context for painting

	CMemDC memDC(dc, this);
	CDC* pDC = &memDC.GetDC();

	CFont* pOldFont = pDC->SelectObject(&afxGlobalData.fontRegular);
	ENSURE(pOldFont != NULL);

	pDC->SetBkMode(TRANSPARENT);

	CRect rect;
	GetClientRect(rect);

	m_pElement->OnDrawKeyTip(pDC, rect, m_bIsMenu);

	pDC->SelectObject(pOldFont);
}

BOOL CMFCRibbonKeyTip::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

BOOL CMFCRibbonKeyTip::Show()
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pElement);

	if (GetSafeHwnd() != NULL)
	{
		ShowWindow(SW_SHOWNOACTIVATE);
		return TRUE;
	}

	CWnd* pWndParent = m_pElement->GetParentWnd();

	if (pWndParent->GetSafeHwnd() == NULL)
	{
		return FALSE;
	}

	CClientDC dc(NULL);

	CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontRegular);
	ENSURE(pOldFont != NULL);

	CRect rect = m_pElement->GetKeyTipRect(&dc, m_bIsMenu);

	dc.SelectObject(pOldFont);

	if (rect.IsRectEmpty())
	{
		return FALSE;
	}

	pWndParent->ClientToScreen(&rect);

	// Normalize inside screen:
	CRect rectScreen;

	MONITORINFO mi;
	mi.cbSize = sizeof(MONITORINFO);
	if (GetMonitorInfo(MonitorFromPoint(rect.TopLeft(), MONITOR_DEFAULTTONEAREST), &mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		::SystemParametersInfo(SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	if (rect.right > rectScreen.right)
	{
		rect.OffsetRect(rectScreen.right - rect.right, 0);
	}
	else if (rect.left < rectScreen.left)
	{
		rect.OffsetRect(rectScreen.left - rect.left, 0);
	}

	if (rect.bottom > rectScreen.bottom)
	{
		rect.OffsetRect(0, rectScreen.bottom - rect.bottom);
	}
	else if (rect.top < rectScreen.top)
	{
		rect.OffsetRect(rectScreen.top - rect.top, 0);
	}

	if (m_strClassName.IsEmpty())
	{
		m_strClassName = ::AfxRegisterWndClass(CS_SAVEBITS, ::LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_BTNFACE + 1), NULL);
	}

	DWORD dwStyleEx = WS_EX_TOOLWINDOW | WS_EX_TOPMOST;

	if (m_pElement->IsDisabled() && CMFCVisualManager::GetInstance()->IsLayeredRibbonKeyTip())
	{
		dwStyleEx |= WS_EX_LAYERED;
	}

	if (!CreateEx(dwStyleEx, m_strClassName, _T(""), WS_POPUP, rect, NULL, 0))
	{
		return FALSE;
	}

	m_rectScreen = rect;

	if (dwStyleEx & WS_EX_LAYERED)
	{
		afxGlobalData.SetLayeredAttrib(GetSafeHwnd(), 0, 128, LWA_ALPHA);
	}

	ShowWindow(SW_SHOWNOACTIVATE);
	return TRUE;
}

void CMFCRibbonKeyTip::Hide()
{
	ASSERT_VALID(this);

	if (GetSafeHwnd() != NULL && IsWindowVisible())
	{
		ShowWindow(SW_HIDE);
		UpdateMenuShadow();
	}
}

void CMFCRibbonKeyTip::UpdateMenuShadow()
{
	CWnd* pMenu = CMFCPopupMenu::GetActiveMenu();

	if (pMenu != NULL && CWnd::FromHandlePermanent(pMenu->GetSafeHwnd()) != NULL && !m_rectScreen.IsRectEmpty())
	{
		CMFCPopupMenu::UpdateAllShadows(m_rectScreen);
	}
}

void CMFCRibbonKeyTip::OnDestroy()
{
	if (IsWindowVisible())
	{
		ShowWindow(SW_HIDE);
		UpdateMenuShadow();
	}

	CWnd::OnDestroy();
}

int CMFCRibbonKeyTip::OnMouseActivate(CWnd* /*pDesktopWnd*/, UINT /*nHitTest*/, UINT /*message*/)
{
	return MA_NOACTIVATE;
}


