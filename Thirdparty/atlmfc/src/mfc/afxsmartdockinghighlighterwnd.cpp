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
#include "afxsmartdockinghighlighterwnd.h"
#include "afxsmartdockingmanager.h"
#include "afxdrawmanager.h"
#include "afxglobals.h"
#include "afxtabbedpane.h"
#include "afxdockingmanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CSmartDockingHighlighterWnd

CSmartDockingHighlighterWnd::CSmartDockingHighlighterWnd() : m_bTabbed(FALSE), m_bShown(FALSE), m_pWndOwner(NULL), m_bUseThemeColorInShading(FALSE)
{
	m_rectLast.SetRectEmpty();
	m_rectTab.SetRectEmpty();
}

CSmartDockingHighlighterWnd::~CSmartDockingHighlighterWnd()
{
}

void CSmartDockingHighlighterWnd::Create(CWnd* pwndOwner)
{
	ASSERT_VALID(pwndOwner);

	m_pWndOwner = pwndOwner;

	CRect rect;
	rect.SetRectEmpty();

	DWORD dwExStyle = (afxGlobalData.IsWindowsLayerSupportAvailable() && afxGlobalData.m_nBitsPerPixel > 8) ? WS_EX_LAYERED : 0;

	CreateEx(dwExStyle, GetSmartDockingWndClassName<0>(), _T(""), WS_POPUP, rect, pwndOwner, NULL);

	if (dwExStyle == WS_EX_LAYERED)
	{
		afxGlobalData.SetLayeredAttrib(GetSafeHwnd(), 0, 100, LWA_ALPHA);
	}

	CSmartDockingInfo& params = CDockingManager::GetSmartDockingParams();
	m_bUseThemeColorInShading = params.m_bUseThemeColorInShading;
}

void CSmartDockingHighlighterWnd::ShowAt(CRect rect)
{
	if (m_bTabbed || m_rectLast != rect)
	{
		Hide();

		if (m_bTabbed)
		{
			SetWindowRgn(NULL, FALSE);
			m_bTabbed = FALSE;
		}

		SetWindowPos(&CWnd::wndTop, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOREDRAW);

		m_rectLast = rect;

		m_bShown = TRUE;
		RedrawWindow();
	}
}

void CSmartDockingHighlighterWnd::Hide()
{
	if (m_bShown)
	{
		ShowWindow(SW_HIDE);
		m_bShown = FALSE;

		if (m_pWndOwner != NULL)
		{
			m_pWndOwner->UpdateWindow();
		}

		if (m_pDockingWnd != NULL)
		{
			m_pDockingWnd->UpdateWindow();
		}

		m_rectLast.SetRectEmpty();
		m_rectTab.SetRectEmpty();
	}
}

void CSmartDockingHighlighterWnd::ShowTabbedAt(CRect rect, CRect rectTab)
{
	if (!m_bTabbed || m_rectLast != rect || m_rectTab != rectTab)
	{
		Hide();

		CRgn rgnMain;
		CTabbedPane::m_bTabsAlwaysTop ? rgnMain.CreateRectRgn(0, rectTab.Height(), rect.Width(),
			rect.Height() + rectTab.Height()) : rgnMain.CreateRectRgn(0, 0, rect.Width(), rect.Height());

		CRgn rgnTab;
		if (CTabbedPane::m_bTabsAlwaysTop)
		{
			rgnTab.CreateRectRgn(rectTab.left, 0, rectTab.Width(), rectTab.Height());
		}
		else
		{
			rgnTab.CreateRectRgnIndirect(rectTab);
		}

		rgnMain.CombineRgn(&rgnMain, &rgnTab, RGN_OR);
		SetWindowRgn(rgnMain, FALSE);

		m_bTabbed = TRUE;

		m_rectLast = rect;
		m_rectTab = rectTab;

		if (CTabbedPane::m_bTabsAlwaysTop)
		{
			SetWindowPos(&CWnd::wndTop, rect.left, rectTab.top, rect.Width(), rect.Height() + m_rectTab.Height(), SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOREDRAW);
		}
		else
		{
			SetWindowPos(&CWnd::wndTop, rect.left, rect.top, rect.Width(), rect.Height() + m_rectTab.Height(), SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOREDRAW);
		}
		m_bShown = TRUE;
		RedrawWindow();
	}
}

BEGIN_MESSAGE_MAP(CSmartDockingHighlighterWnd, CWnd)
	//{{AFX_MSG_MAP(CSmartDockingHighlighterWnd)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSmartDockingHighlighterWnd message handlers

void CSmartDockingHighlighterWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	if (!m_bShown)
	{
		return;
	}

	CRect rect;
	GetClientRect(rect);

	COLORREF colorFill = m_bUseThemeColorInShading ? afxGlobalData.clrActiveCaption : RGB(47, 103, 190);

	if (afxGlobalData.IsWindowsLayerSupportAvailable() && afxGlobalData.m_nBitsPerPixel > 8)
	{
		CBrush brFill(CDrawingManager::PixelAlpha(colorFill, 105));
		dc.FillRect(rect, &brFill);
	}
	else
	{
		CBrush brFill(CDrawingManager::PixelAlpha(RGB(255 - GetRValue(colorFill), 255 - GetGValue(colorFill), 255 - GetBValue(colorFill)), 50));

		CBrush* pBrushOld = dc.SelectObject(&brFill);
		dc.PatBlt(0, 0, rect.Width(), rect.Height(), PATINVERT);
		dc.SelectObject(pBrushOld);
	}
}

void CSmartDockingHighlighterWnd::OnClose()
{
	// do nothing
}

BOOL CSmartDockingHighlighterWnd::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}



