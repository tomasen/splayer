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

#pragma warning(disable : 4706)

#include "multimon.h"

#pragma warning(default : 4706)

#include "afxwinappex.h"
#include "afxpopupmenu.h"
#include "afxmenubar.h"
#include "afxglobals.h"
#include "afxtoolbarmenubutton.h"
#include "afxmdiframewndex.h"
#include "afxframewndex.h"
#include "afxoleipframewndex.h"
#include "afxoledocipframewndex.h"
#include "afxmenubar.h"
#include "afxtoolbarsmenupropertypage.h"
#include "afxmenuhash.h"
#include "afxmenuimages.h"
#include "afxshowallbutton.h"
#include "afxusertoolsmanager.h"
#include "afxmenutearoffmanager.h"
#include "afxusertool.h"
#include "afxsound.h"
#include "afxribbonres.h"
#include "afxdialogex.h"
#include "afxpropertypage.h"
#include "afxvisualmanager.h"
#include "afxdrawmanager.h"
#include "afxcommandmanager.h"
#include "afxkeyboardmanager.h"
#include "afxpane.h"
#include "afxpaneframewnd.h"
#include "afxcustomizemenubutton.h"
#include "afxcaptionmenubutton.h"
#include "afxbaseribbonelement.h"
#include "afxribbonminitoolbar.h"
#include "afxcontrolrenderer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCShadowWnd

class CMFCShadowWnd : public CMiniFrameWnd
{
	friend class CMFCPopupMenu;

	CMFCShadowWnd(CMFCPopupMenu* pOwner, int nOffset)
	{
		m_pOwner = pOwner;
		m_nOffset = nOffset;
		m_bIsRTL = FALSE; // **** STAS 3-Dec-2007

	}

	~CMFCShadowWnd()
	{
	}

	void Repos();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMFCShadowWnd)
	public:
	virtual BOOL Create();
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CMFCShadowWnd)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CMFCPopupMenu* m_pOwner;
	int m_nOffset;
	CMFCShadowRenderer m_Shadow;
	BOOL m_bIsRTL; 
};

BEGIN_MESSAGE_MAP(CMFCShadowWnd, CMiniFrameWnd)
	//{{AFX_MSG_MAP(CMFCShadowWnd)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CMFCShadowWnd::Create ()
{
	ASSERT_VALID (m_pOwner);

	if (!afxGlobalData.IsWindowsLayerSupportAvailable () || afxGlobalData.m_nBitsPerPixel <= 8)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CString strClassName = ::AfxRegisterWndClass (
			CS_SAVEBITS,
			::LoadCursor(NULL, IDC_ARROW),
			(HBRUSH)(COLOR_BTNFACE + 1), NULL);

	CRect rectDummy (0, 0, 0, 0);
	DWORD dwStyleEx = WS_EX_TOOLWINDOW | WS_EX_LAYERED;

	if (m_pOwner->GetExStyle() & WS_EX_LAYOUTRTL)
	{
		m_bIsRTL = TRUE; // **** STAS 3-Dec-2007
	}

	if (!CMiniFrameWnd::CreateEx (dwStyleEx, strClassName, _T(""), WS_POPUP, rectDummy, m_pOwner->GetParent ()))
	{
		return FALSE;
	}

	m_Shadow.Create (m_nOffset, RGB (90, 90, 90), 0, 50);
	return TRUE;
}

BOOL CMFCShadowWnd::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CMFCShadowWnd::OnSize(UINT nType, int cx, int cy)
{
	CMiniFrameWnd::OnSize (nType, cx, cy);

	if (cx == 0 || cy == 0)
	{
		return;
	}

	CPoint point (0, 0);
	CSize size (cx, cy);

	LPBYTE pBits = NULL;
	HBITMAP hBitmap = CDrawingManager::CreateBitmap_32 (size, (void**)&pBits);
	if (hBitmap == NULL)
	{
		return;
	}

	CBitmap bitmap;
	bitmap.Attach (hBitmap);

	CClientDC clientDC(this);
	CDC dc;
	dc.CreateCompatibleDC (&clientDC);

	CBitmap* pBitmapOld = (CBitmap*)dc.SelectObject (&bitmap);

	m_Shadow.Draw (&dc, CRect (point, size));	// **** STAS 3-Dec-2007


	BLENDFUNCTION bf;
	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;
	bf.SourceConstantAlpha = 255;
	bf.AlphaFormat = LWA_COLORKEY;

	UpdateLayeredWindow (NULL, 0, &size, &dc, &point, 0, &bf, 0x02);

	dc.SelectObject (pBitmapOld);
}

void CMFCShadowWnd::Repos() 
{
	ASSERT_VALID (m_pOwner);

	//	const BOOL bRTL = GetExStyle() & WS_EX_LAYOUTRTL; **** Stas 3-Dec-2007


	CRect rectWindow;
	m_pOwner->GetWindowRect (rectWindow);

	rectWindow.OffsetRect (m_bIsRTL ? -m_nOffset : m_nOffset, m_nOffset);	// *** Stas 3-dec-2007


	SetRedraw (FALSE);

	if (!IsWindowVisible ())
	{
		ShowWindow (SW_SHOWNOACTIVATE);

		SetWindowPos (&CWnd::wndTop, rectWindow.left, rectWindow.top, rectWindow.Width (), rectWindow.Height (),
			SWP_NOACTIVATE);

		m_pOwner->SetWindowPos (&CWnd::wndTop, -1, -1, -1, -1,
			SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	}
	else
	{
		SetWindowPos (NULL, rectWindow.left, rectWindow.top, rectWindow.Width (), rectWindow.Height (),
			SWP_NOACTIVATE | SWP_NOZORDER);
	}

	SetRedraw ();
	RedrawWindow ();
}

/////////////////////////////////////////////////////////////////////////////
// CMFCPopupMenu

static const int nFadeStep = 10;
static const int nAnimTimerId = 1;
static const int nScrollTimerId = 2;
static const int nScrollTimerDuration = 80;
static const int nMenuBarId = 1;
static const int nTearOffBarHeight = 10;
static const int nResizeBarBarHeightRight = 12;
static const int nResizeBarBarHeight = 9;
static const int nScrollBarID = 1;

CMFCPopupMenu::ANIMATION_TYPE CMFCPopupMenu::m_AnimationType = NO_ANIMATION;
UINT CMFCPopupMenu::m_AnimationSpeed = 15;
CMFCPopupMenu* CMFCPopupMenu::m_pActivePopupMenu = NULL;
BOOL CMFCPopupMenu::m_bForceShadow = TRUE;
BOOL CMFCPopupMenu::m_bForceMenuFocus = TRUE;
BOOL CMFCPopupMenu::m_bMenuSound = TRUE;
BOOL CMFCPopupMenu::m_bAlwaysShowEmptyToolsEntry = FALSE;
BOOL CMFCPopupMenu::m_bSendMenuSelectMsg = FALSE;
int CMFCPopupMenu::m_nMinWidth = 0;

static clock_t nLastAnimTime = 0;

IMPLEMENT_SERIAL(CMFCPopupMenu, CMiniFrameWnd, VERSIONABLE_SCHEMA | 1)

CMFCPopupMenu::CMFCPopupMenu() : m_pMenuCustomizationPage(NULL)
{
	Initialize();
}

CMFCPopupMenu::CMFCPopupMenu(CMFCToolBarsMenuPropertyPage* pCustPage, LPCTSTR lpszTitle) : m_pMenuCustomizationPage(pCustPage), m_strCaption(lpszTitle)
{
	Initialize();
}

void CMFCPopupMenu::Initialize()
{
	if (afxGlobalData.bIsRemoteSession)
	{
		m_AnimationType = NO_ANIMATION;
	}

	m_hMenu = NULL;
	m_ptLocation = CPoint(0, 0);
	m_ptLocationInitial = CPoint(0, 0);
	m_pParentBtn = NULL;
	m_bAutoDestroyParent = TRUE;
	m_bAutoDestroy = TRUE;
	m_FinalSize = CSize(0, 0);
	m_AnimSize = CSize(0, 0);
	m_nMenuBarHeight = 0;
	m_bAnimationIsDone = (GetAnimationType() == NO_ANIMATION);
	m_bDisableAnimation = FALSE;
	m_bScrollable = FALSE;
	m_bShowScrollBar = FALSE;
	m_nMaxHeight = -1;
	m_bTobeDstroyed = FALSE;
	m_bShown = FALSE;

	m_iMaxWidth = -1;
	m_iLogoWidth = 0;
	m_nLogoLocation = MENU_LOGO_LEFT;

	m_rectScrollUp.SetRectEmpty();
	m_rectScrollDn.SetRectEmpty();

	m_iScrollBtnHeight = CMenuImages::Size().cy + 2 *
		CMFCVisualManager::GetInstance()->GetPopupMenuBorderSize();
	m_iScrollMode = 0;

	m_bIsAnimRight = TRUE;
	m_bIsAnimDown = TRUE;

	m_iShadowSize = CMFCMenuBar::IsMenuShadows() && !CMFCToolBar::IsCustomizeMode() && afxGlobalData.m_nBitsPerPixel > 8 ? // Don't draw shadows in 256 colors or less
		CMFCVisualManager::GetInstance()->GetMenuShadowDepth() : 0;

	m_iFadePercent = 0;
	if (GetAnimationType() == FADE && afxGlobalData.m_nBitsPerPixel <= 8)
	{
		m_AnimationType = NO_ANIMATION;
		m_bAnimationIsDone = TRUE;
	}

	m_bTearOffTracking = FALSE;
	m_bIsTearOffCaptionActive = FALSE;
	m_rectTearOffCaption.SetRectEmpty();

	m_DropDirection = DROP_DIRECTION_NONE;

	m_pMessageWnd = NULL;
	m_bTrackMode = FALSE;
	m_bRightAlign = FALSE;
	m_bQuickCusomize = FALSE;
	m_QuickType = QUICK_CUSTOMIZE_NONE;
	m_bEscClose   = FALSE;

	m_pParentRibbonElement = NULL;
	m_hwndConnectedFloaty = NULL;
	m_nLastCommandID = 0;

	m_bIsResizable = FALSE;
	m_sizeMinResize = CSize(0, 0);
	m_rectResize.SetRectEmpty();
	m_bResizeTracking = FALSE;
	m_bWasResized = FALSE;
	m_bIsResizeBarOnTop = FALSE;
	m_sizeCurrent = CSize(0, 0);
	m_bHasBeenResized = FALSE;
	m_pWndShadow = NULL;
}

CMFCPopupMenu::~CMFCPopupMenu()
{
	if (m_pParentRibbonElement != NULL)
	{
		ASSERT_VALID(m_pParentRibbonElement);
		m_pParentRibbonElement->SetDroppedDown(NULL);
	}

	if (m_bAutoDestroy && m_hMenu != NULL)
	{
		::DestroyMenu(m_hMenu);
	}

	if (m_hwndConnectedFloaty != NULL && ::IsWindow(m_hwndConnectedFloaty))
	{
		::SendMessage(m_hwndConnectedFloaty, WM_CLOSE, 0, 0);
	}
}

//{{AFX_MSG_MAP(CMFCPopupMenu)
BEGIN_MESSAGE_MAP(CMFCPopupMenu, CMiniFrameWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_MOUSEACTIVATE()
	ON_WM_DESTROY()
	ON_WM_KEYDOWN()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SHOWWINDOW()
	ON_WM_SETCURSOR()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_NCHITTEST()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_ACTIVATEAPP()
	ON_WM_WINDOWPOSCHANGED()
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// CMFCPopupMenu message handlers

BOOL CMFCPopupMenu::Create(CWnd* pWndParent, int x, int y, HMENU hMenu, BOOL bLocked, BOOL bOwnMessage)
{
	AFXPlaySystemSound(AFX_SOUND_MENU_POPUP);

	ENSURE(pWndParent != NULL);

	UINT nClassStyle = CS_SAVEBITS;

	CString strClassName = ::AfxRegisterWndClass(nClassStyle, ::LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_BTNFACE + 1), NULL);

	m_hMenu = hMenu;

	if (x == -1 && y == -1) // Undefined position
	{
		if (pWndParent != NULL)
		{
			CRect rectParent;
			pWndParent->GetClientRect(&rectParent);
			pWndParent->ClientToScreen(&rectParent);

			m_ptLocation = CPoint(rectParent.left + 5, rectParent.top + 5);
		}
		else
		{
			m_ptLocation = CPoint(0, 0);
		}
	}
	else
	{
		m_ptLocation = CPoint(x, y);
	}

	m_ptLocationInitial = m_ptLocation;

	DWORD dwStyle = WS_POPUP;
	if (m_pMenuCustomizationPage != NULL)
	{
		dwStyle |= (WS_CAPTION | WS_SYSMENU);
	}

	if (pWndParent->GetSafeHwnd() != NULL &&
		(pWndParent->GetExStyle() & WS_EX_LAYOUTRTL))
	{
		m_bDisableAnimation = TRUE;
	}

	if (m_bDisableAnimation)
	{
		m_bAnimationIsDone = TRUE;
	}

	BOOL bIsAnimate = (GetAnimationType() != NO_ANIMATION) && !CMFCToolBar::IsCustomizeMode() && !m_bDisableAnimation;

	CMFCPopupMenu* pParentMenu = GetParentPopupMenu();
	if (pParentMenu != NULL)
	{
		m_bTrackMode = pParentMenu->m_bTrackMode;
	}

	if (bOwnMessage)
	{
		m_pMessageWnd = pWndParent;
	}
	else if (pParentMenu != NULL)
	{
		m_pMessageWnd = pParentMenu->GetMessageWnd();
	}

	CRect rect(x, y, x, y);
	BOOL bCreated = CMiniFrameWnd::CreateEx(pWndParent->GetExStyle() & WS_EX_LAYOUTRTL, strClassName, m_strCaption,
		dwStyle, rect, pWndParent->GetOwner() == NULL ? pWndParent : pWndParent->GetOwner());
	if (!bCreated)
	{
		return FALSE;
	}

	if (m_bRightAlign)
	{
		m_ptLocation.x -= m_FinalSize.cx - 1;
		m_ptLocationInitial = m_ptLocation;

		RecalcLayout();
	}

	CMFCPopupMenuBar* pMenuBar = GetMenuBar();
	ASSERT_VALID(pMenuBar);

	pMenuBar->m_bLocked = bLocked;
	pMenuBar->m_bDropDownListMode = m_bShowScrollBar;

	if (bIsAnimate)
	{
		// Adjust initial menu size:
		m_AnimSize = m_FinalSize + CSize(m_iShadowSize, m_iShadowSize);

		switch (GetAnimationType())
		{
		case UNFOLD:
			m_AnimSize.cx = pMenuBar->GetColumnWidth();

		case SLIDE:
			m_AnimSize.cy = pMenuBar->GetRowHeight();
			break;
		}

		if (pMenuBar->IsWindowVisible())
		{
			pMenuBar->ShowWindow(SW_HIDE);
		}

		SetTimer(nAnimTimerId, m_AnimationSpeed, NULL);
		nLastAnimTime = clock();
	}

	// Update windows covered by menu:
	UpdateBottomWindows();
	if (m_iShadowSize == 0 && GetAnimationType() == FADE && bIsAnimate)
	{
		m_AnimSize = m_FinalSize;
	}

	SetWindowPos(&wndTop, -1, -1, -1, -1, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

	if (CMFCToolBar::IsCustomizeMode())
	{
		pMenuBar->Invalidate();
		pMenuBar->UpdateWindow();
	}

	return TRUE;
}

int CMFCPopupMenu::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMiniFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	DWORD toolbarStyle = AFX_DEFAULT_TOOLBAR_STYLE;
	if (GetAnimationType() != NO_ANIMATION && !CMFCToolBar::IsCustomizeMode() && !m_bDisableAnimation)
	{
		toolbarStyle &= ~WS_VISIBLE;
	}

	CMFCPopupMenuBar* pMenuBar = GetMenuBar();
	ASSERT_VALID(pMenuBar);

	pMenuBar->m_bTrackMode = m_bTrackMode;

	if (m_pParentBtn != NULL && m_pParentBtn->IsMenuPaletteMode())
	{
		pMenuBar->m_bPaletteMode = TRUE;
		pMenuBar->m_bDisableSideBarInXPMode = TRUE;
		pMenuBar->m_bPaletteRows = m_pParentBtn->GetPaletteRows();
	}

	if (!pMenuBar->Create(this, toolbarStyle | CBRS_TOOLTIPS | CBRS_FLYBY, nMenuBarId))
	{
		TRACE(_T("Can't create popup menu bar\n"));
		return FALSE;
	}

	CMFCPopupMenu* pParentPopupMenu = GetParentPopupMenu();
	if (pParentPopupMenu != NULL)
	{
		m_iMaxWidth = pParentPopupMenu->m_iMaxWidth;
	}

	pMenuBar->m_iMaxWidth = m_iMaxWidth;
	pMenuBar->m_iMinWidth = m_nMinWidth;
	pMenuBar->SetOwner(GetParent());

	if (m_iShadowSize > 0 && afxGlobalData.IsWindowsLayerSupportAvailable () &&
		m_pParentRibbonElement != NULL)
	{
		m_pWndShadow = new CMFCShadowWnd (this, m_iShadowSize);
		m_iShadowSize = 0;

		
		m_pWndShadow->Create ();
	}

	return InitMenuBar()? 0 : -1;
}

void CMFCPopupMenu::OnSize(UINT nType, int cx, int cy)
{
	CMiniFrameWnd::OnSize(nType, cx, cy);

	CMFCPopupMenuBar* pMenuBar = GetMenuBar();
	ASSERT_VALID(pMenuBar);

	if (pMenuBar->m_bInUpdateShadow)
	{
		return;
	}

	if (pMenuBar->GetSafeHwnd() != NULL)
	{
		AdjustScroll(TRUE /*bForceMenuBarResize*/);
		SetScrollBar();
	}

	const int nBorderSize = GetBorderSize();

	if (m_iLogoWidth > 0)
	{
		CRect rectLogo;
		GetClientRect(rectLogo);

		switch (m_nLogoLocation)
		{
		case MENU_LOGO_LEFT:
			rectLogo.right = rectLogo.left + nBorderSize + m_iLogoWidth;
			break;

		case MENU_LOGO_RIGHT:
			rectLogo.left = rectLogo.right - nBorderSize - m_iLogoWidth;
			break;

		case MENU_LOGO_TOP:
			rectLogo.bottom = rectLogo.top + nBorderSize + m_iLogoWidth;
			break;

		case MENU_LOGO_BOTTOM:
			rectLogo.top = rectLogo.bottom - nBorderSize - m_iLogoWidth;
			break;
		}

		InvalidateRect(rectLogo);
		UpdateWindow();
	}
}

void CMFCPopupMenu::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	if (!m_bAnimationIsDone && !CMFCToolBar::IsCustomizeMode())
	{
		DrawFade(&dc);
	}
	else
	{
		DoPaint(&dc);
	}
}

int CMFCPopupMenu::OnMouseActivate(CWnd* /*pDesktopWnd*/, UINT /*nHitTest*/, UINT /*message*/)
{
	return MA_NOACTIVATE;
}

void CMFCPopupMenu::RecalcLayout(BOOL /*bNotify*/)
{
#ifdef _DEBUG
	if (m_pParentBtn != NULL)
	{
		ASSERT_VALID(m_pParentBtn);
		ASSERT(m_pParentBtn->m_pPopupMenu == this);
	}

	if (m_pParentRibbonElement != NULL)
	{
		ASSERT_VALID(m_pParentRibbonElement);
		ASSERT(m_pParentRibbonElement->m_pPopupMenu == this);
	}
#endif // _DEBUG

	CMFCPopupMenuBar* pMenuBar = GetMenuBar();
	ASSERT_VALID(pMenuBar);

	if (!::IsWindow(m_hWnd) || pMenuBar == NULL || !::IsWindow(pMenuBar->m_hWnd))
	{
		return;
	}

	if (pMenuBar->m_bInUpdateShadow)
	{
		return;
	}

	// Set tear-off attributes:
	BOOL bIsTearOff = (m_pParentBtn != NULL && m_pParentBtn->IsTearOffMenu() && !CMFCToolBar::IsCustomizeMode());

	CRect rectScreen;

	MONITORINFO mi;
	mi.cbSize = sizeof(MONITORINFO);
	if (GetMonitorInfo(MonitorFromPoint(m_ptLocation, MONITOR_DEFAULTTONEAREST), &mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		::SystemParametersInfo(SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	const int nBorderSize = GetBorderSize();
	const BOOL bRTL = GetExStyle() & WS_EX_LAYOUTRTL;

	if (m_bResizeTracking)
	{
		if (bRTL)
		{
			m_sizeCurrent.cx = min(m_sizeCurrent.cx, m_ptLocation.x - rectScreen.left - 2 * nBorderSize);
		}
		else
		{
			m_sizeCurrent.cx = min(m_sizeCurrent.cx, rectScreen.right - m_ptLocation.x - 2 * nBorderSize);
		}

		m_sizeCurrent.cy = min(m_sizeCurrent.cy, rectScreen.bottom - m_ptLocation.y - 2 * nBorderSize - m_rectResize.Height());
	}

	// Normalize location in the screen area:
	m_ptLocation.x = max(rectScreen.left, min(m_ptLocation.x, rectScreen.right));

	if (m_pParentBtn == NULL)
	{
		m_ptLocation.y = max(rectScreen.top, min(m_ptLocation.y, rectScreen.bottom));
	}

	CSize size = m_sizeCurrent;

	if (!m_bResizeTracking && !m_bWasResized)
	{
		size = pMenuBar->CalcSize(TRUE);
	};

	if (!m_bResizeTracking && !m_bWasResized)
	{
		size.cx += nBorderSize * 2;
		size.cy += nBorderSize * 2;

		switch (m_nLogoLocation)
		{
		case MENU_LOGO_LEFT:
		case MENU_LOGO_RIGHT:
			size.cx += m_iLogoWidth;
			break;

		case MENU_LOGO_TOP:
		case MENU_LOGO_BOTTOM:
			size.cy += m_iLogoWidth;
			break;
		}
	}

	if (m_pMenuCustomizationPage != NULL)
	{
		size.cy += ::GetSystemMetrics(SM_CYSMCAPTION);
		size.cy += 2 * ::GetSystemMetrics(SM_CYBORDER) + 5;
	}
	else if (bIsTearOff)
	{
		m_rectTearOffCaption = CRect(CPoint(nBorderSize, nBorderSize), CSize(size.cx - 2 * nBorderSize, nTearOffBarHeight));
		size.cy += nTearOffBarHeight;

		if (!CMFCToolBar::IsCustomizeMode() && m_wndToolTip.GetSafeHwnd() == NULL)
		{
			m_wndToolTip.Create(this);
			m_wndToolTip.Activate(TRUE);
			if (afxGlobalData.m_nMaxToolTipWidth != -1)
			{
				m_wndToolTip.SetMaxTipWidth(afxGlobalData.m_nMaxToolTipWidth);
			}

			m_wndToolTip.AddTool(this, IDS_AFX_TEAR_OFF, m_rectTearOffCaption, 1);
		}
	}

	if (m_nMaxHeight != -1 && size.cy > m_nMaxHeight)
	{
		if (!m_bResizeTracking && !m_bWasResized)
		{
			int nMaxHeight = m_nMaxHeight - nBorderSize * 2;

			size.cy = nMaxHeight -(nMaxHeight % pMenuBar->GetRowHeight()) + nBorderSize * 2 + 2;
			m_bHasBeenResized = TRUE;
		}

		m_bScrollable = TRUE;
	}

	if (m_bIsResizable)
	{
		const int nResizeBarHeight = m_sizeMinResize.cx > 0 ? nResizeBarBarHeightRight : nResizeBarBarHeight;

		if (m_bIsResizeBarOnTop)
		{
			m_rectResize = CRect(CPoint(nBorderSize, nBorderSize), CSize(size.cx - 2 * nBorderSize, nResizeBarHeight));
		}
		else
		{
			m_rectResize = CRect(CPoint(nBorderSize, size.cy - nBorderSize), CSize(size.cx - 2 * nBorderSize, nResizeBarHeight));
		}

		size.cy += nResizeBarHeight;
	}

	BOOL bIsRightAlign = bRTL || m_bRightAlign;

	CMFCMenuBar* pParentMenuBar = m_pParentBtn == NULL ? NULL : DYNAMIC_DOWNCAST(CMFCMenuBar, m_pParentBtn->m_pWndParent);

	if (pParentMenuBar != NULL && pParentMenuBar->IsFloating())
	{
		// When the popup menu is dropped-down from the floating menu bar,
		// it should not cover other menu bar items. Ask parent menu bar about
		// right popup menu location:
		DROP_DIRECTION direction = (DROP_DIRECTION)pParentMenuBar->GetFloatPopupDirection(m_pParentBtn);

		switch (direction)
		{
		case DROP_DIRECTION_TOP:
			m_ptLocation.y = m_ptLocationInitial.y - size.cy - m_pParentBtn->Rect().Height() + 1;
			m_DropDirection = direction;
			break;

		case DROP_DIRECTION_LEFT:
			m_ptLocation.y = m_ptLocationInitial.y - m_pParentBtn->Rect().Height() + 1;
			m_DropDirection = direction;

			if (bRTL)
			{
				m_ptLocation.x = m_ptLocationInitial.x + size.cx;

				if (m_ptLocation.x > rectScreen.right)
				{
					m_ptLocation.x = rectScreen.right;
					m_DropDirection = DROP_DIRECTION_NONE;
				}
			}
			else
			{
				m_ptLocation.x = m_ptLocationInitial.x - size.cx;

				if (m_ptLocation.x < rectScreen.left)
				{
					m_ptLocation.x = rectScreen.left;
					m_DropDirection = DROP_DIRECTION_NONE;
				}
			}
			break;

		case DROP_DIRECTION_RIGHT:
			if (bRTL)
			{
				m_ptLocation.x = m_ptLocationInitial.x - m_pParentBtn->Rect().Width();
			}
			else
			{
				m_ptLocation.x = m_ptLocationInitial.x + m_pParentBtn->Rect().Width();
			}

			m_ptLocation.y = m_ptLocationInitial.y - m_pParentBtn->Rect().Height() + 1;
			m_DropDirection = direction;
			break;
		}
	}

	// Prepare Quick Customize Drawing
	CRect rectQCParent;
	rectQCParent.SetRectEmpty();

	BOOL bConnectQCToParent = FALSE;

	if (CMFCVisualManager::GetInstance()->IsOfficeXPStyleMenus())
	{
		CMFCPopupMenu* pParentPopup = GetParentPopupMenu();

		if (pParentPopup != NULL)
		{
			CMFCToolBarMenuButton* pParentBtn = GetParentButton();
			if ((pParentBtn != NULL) &&(pParentBtn->IsQuickMode()))
			{
				if (!bRTL)
				{
					pParentPopup->RedrawWindow();
				}

				if (pParentPopup->IsQuickCustomize())
				{
					if (!m_bQuickCusomize)
					{
						rectQCParent = pParentBtn->Rect();
						CWnd* pParentWnd = pParentBtn->GetParentWnd();

						if (pParentWnd->GetSafeHwnd() != NULL)
						{
							pParentWnd->ClientToScreen(&rectQCParent);

							m_ptLocation.y = rectQCParent.top;
							bConnectQCToParent = TRUE;

							if (m_DropDirection == DROP_DIRECTION_LEFT)
							{
								m_ptLocation.x = bRTL ? rectQCParent.left : rectQCParent.left - size.cx;
							}
							else
							{
								m_ptLocation.x = bRTL ? rectQCParent.right + size.cx : rectQCParent.right;
							}
						}
					}
				}
			}
		}
	}

	// Adjust the menu position by the screen size:
	if ((bRTL &&(m_ptLocation.x - size.cx < rectScreen.left)) ||
		(!bIsRightAlign &&(m_ptLocation.x + size.cx > rectScreen.right)))
	{
		// Menu can't be overlapped with the parent popup menu!
		CMFCPopupMenu* pParentMenu = GetParentPopupMenu();

		if (pParentMenu != NULL)
		{
			CRect rectParent;
			pParentMenu->GetWindowRect(rectParent);

			m_ptLocation.x = bRTL ? rectParent.right + size.cx : rectParent.left - size.cx;

			if (m_pParentRibbonElement != NULL)
			{
				ASSERT_VALID(m_pParentRibbonElement);

				if (!m_pParentRibbonElement->IsMenuMode())
				{
					rectParent = m_pParentRibbonElement->GetRect();
					pParentMenu->ClientToScreen(&rectParent);

					m_ptLocation.x = bRTL ? rectParent.left + size.cx : rectParent.right - size.cx;
				}
			}

			m_DropDirection = bRTL ? DROP_DIRECTION_RIGHT : DROP_DIRECTION_LEFT;
		}
		else if (pParentMenuBar != NULL && (pParentMenuBar->IsHorizontal()) == 0)
		{
			// Parent menu bar is docked vertical, place menu
			// in the left or right side of the parent frame:
			CRect rectParentBtn = m_pParentBtn->Rect();
			pParentMenuBar->ClientToScreen(&rectParentBtn);

			m_ptLocation.x = bRTL ? rectParentBtn.right + size.cx : rectParentBtn.left - size.cx;
			m_DropDirection = DROP_DIRECTION_LEFT;
		}
		else
		{
			m_ptLocation.x = bRTL ? rectScreen.left + size.cx + 1 : m_bRightAlign ? rectScreen.left + 1 : rectScreen.right - size.cx - 1;
			m_DropDirection = DROP_DIRECTION_NONE;
		}

		if (!bRTL &&(m_ptLocation.x < rectScreen.left))
		{
			m_ptLocation.x = rectScreen.left;
			m_DropDirection = DROP_DIRECTION_NONE;
		}

		if (bRTL &&(m_ptLocation.x > rectScreen.right))
		{
			m_ptLocation.x = rectScreen.right;
			m_DropDirection = DROP_DIRECTION_NONE;
		}

		if (!m_bDisableAnimation)
		{
			if (GetAnimationType() == UNFOLD)
			{
				m_bIsAnimRight = FALSE;
			}
			else if (GetAnimationType() == FADE)
			{
				m_bIsAnimRight = FALSE;
				m_bIsAnimDown = FALSE;
			}
		}
	}

	if (m_ptLocation.y + size.cy > rectScreen.bottom)
	{
		const int nResizeBarHeight = m_rectResize.Height();

		if (m_bIsResizable)
		{
			m_rectResize = CRect(CPoint(nBorderSize, nBorderSize), CSize(size.cx - 2 * nBorderSize, nResizeBarHeight));

			m_bIsResizeBarOnTop = TRUE;
		}

		m_bIsAnimDown = FALSE;

		CRect rectParentBtn;
		CWnd* pWndParent = GetParentArea(rectParentBtn);

		if (pWndParent != NULL && m_DropDirection != DROP_DIRECTION_LEFT && m_DropDirection != DROP_DIRECTION_RIGHT)
		{
			CPoint ptRight(rectParentBtn.right, 0);
			pWndParent->ClientToScreen(&ptRight);

			CPoint ptTop(0, rectParentBtn.top - size.cy);
			pWndParent->ClientToScreen(&ptTop);

			if (ptTop.y < 0)
			{
				int yParentButtonTop = ptTop.y + size.cy;

				// Where more space: on top or on bottom of the button?
				if (rectScreen.bottom - yParentButtonTop < yParentButtonTop - rectScreen.top)
				{
					m_ptLocation.y = rectScreen.top;
					m_DropDirection = DROP_DIRECTION_NONE;
					size.cy += ptTop.y;
				}
			else
			{
				size.cy = rectScreen.bottom - m_ptLocation.y;
				m_bIsAnimDown = TRUE;

				if (m_bIsResizable)
				{
					// Restore resize box back to bottom:
					m_bIsResizeBarOnTop = FALSE;
					m_rectResize = CRect(CPoint(nBorderSize, size.cy - nBorderSize - nResizeBarHeight), CSize(size.cx - 2 * nBorderSize, nResizeBarHeight));
				}
			}

			m_bHasBeenResized = TRUE;
			m_bScrollable = TRUE;
			}
			else
			{
				m_ptLocation.y = ptTop.y;

				m_DropDirection =
					(pParentMenuBar != NULL && pParentMenuBar->IsHorizontal()) ? DROP_DIRECTION_TOP : DROP_DIRECTION_NONE;
			}
		}
		else
		{
			if (bConnectQCToParent)
			{
				m_ptLocation.y = rectQCParent.bottom - size.cy - 1;
			}
			else
			{
				m_ptLocation.y -= size.cy;

				if (GetParentPopupMenu() != NULL)
				{
					m_ptLocation.y += pMenuBar->GetRowHeight() + nBorderSize * 2;
				}
			}
		}

		if (m_ptLocation.y < rectScreen.top)
		{
			m_ptLocation.y = rectScreen.top;
			m_DropDirection = DROP_DIRECTION_NONE;
		}

		if (m_ptLocation.y + size.cy > rectScreen.bottom)
		{
			size.cy = rectScreen.bottom - m_ptLocation.y;
			m_bHasBeenResized = TRUE;
			m_bScrollable = TRUE;
		}
	}

	if (m_ptLocation.y < rectScreen.top)
	{
		if (m_pParentBtn != NULL && m_pParentBtn->GetParentWnd() != NULL && GetParentPopupMenu() == NULL)
		{
			CPoint ptRight(m_pParentBtn->Rect().right, 0);
			m_pParentBtn->GetParentWnd()->ClientToScreen(&ptRight);

			CPoint ptBottom(0, m_pParentBtn->Rect().bottom);
			m_pParentBtn->GetParentWnd()->ClientToScreen(&ptBottom);

			m_ptLocation.y = ptBottom.y;

			m_DropDirection = (pParentMenuBar != NULL && pParentMenuBar->IsHorizontal()) ? DROP_DIRECTION_BOTTOM : DROP_DIRECTION_NONE;
		}
		else
		{
			m_ptLocation.y = rectScreen.top;
		}

		if (m_ptLocation.y + size.cy > rectScreen.bottom)
		{
			m_ptLocation.y = rectScreen.top;

			if (size.cy > rectScreen.Height())
			{
				size.cy = rectScreen.Height();
				m_bHasBeenResized = TRUE;
				m_bScrollable = TRUE;
			}

			m_DropDirection = DROP_DIRECTION_NONE;
		}
	}

	if (m_bScrollable && m_bShowScrollBar && !m_bResizeTracking && !m_bWasResized)
	{
		size.cx += ::GetSystemMetrics(SM_CXVSCROLL);

		if (!m_rectResize.IsRectEmpty())
		{
			m_rectResize.right += ::GetSystemMetrics(SM_CXVSCROLL);
		}
	}

	m_FinalSize = size;

	if (GetAnimationType() != NO_ANIMATION || m_bAnimationIsDone || CMFCToolBar::IsCustomizeMode())
	{
		if (!CMFCToolBar::IsCustomizeMode())
		{
			size.cx += m_iShadowSize;
			size.cy += m_iShadowSize;
		}

		if (m_pMenuCustomizationPage != NULL)
		{
			SetWindowPos(NULL, -1, -1, size.cx, size.cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
		else
		{
			SetWindowPos(NULL, m_ptLocation.x -(bRTL ? size.cx : 0), m_ptLocation.y, size.cx, size.cy, SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	if (CMFCToolBar::IsCustomizeMode())
	{
		pMenuBar->AdjustLocations();
	}

	// Parent button maybe covered by shadow, repaint it:
	if (m_iShadowSize != 0 && !CMFCToolBar::IsCustomizeMode() && m_pParentBtn != NULL && m_pParentBtn->GetParentWnd() != NULL)
	{
		BOOL bOldShown = m_bShown;
		m_bShown = TRUE;

		CWnd* pWndParent = m_pParentBtn->GetParentWnd();

		CRect rectInter;

		CRect rectMenu;
		GetWindowRect(rectMenu);

		CRect rectShadowRight(CPoint(bRTL ? rectMenu.left - 1 - m_iShadowSize : rectMenu.right + 1, rectMenu.top), CSize(m_iShadowSize, rectMenu.Height() + m_iShadowSize));
		pWndParent->ScreenToClient(&rectShadowRight);

		if (rectInter.IntersectRect(rectShadowRight, m_pParentBtn->m_rect))
		{
			pWndParent->InvalidateRect(m_pParentBtn->m_rect);
			pWndParent->UpdateWindow();
		}

		CRect rectShadowBottom(CPoint(rectMenu.left, rectMenu.bottom + 1), CSize(rectMenu.Width() + m_iShadowSize, m_iShadowSize));
		pWndParent->ScreenToClient(&rectShadowBottom);

		if (rectInter.IntersectRect(rectShadowBottom, m_pParentBtn->m_rect))
		{
			pWndParent->InvalidateRect(m_pParentBtn->m_rect);
			pWndParent->UpdateWindow();
		}

		m_bShown = bOldShown;
	}

	if (m_bScrollable && m_bShowScrollBar && !m_bResizeTracking && !m_bWasResized)
	{
		RedrawWindow();
	}
}

void CMFCPopupMenu::OnDestroy()
{
	if (m_bQuickCusomize)
	{
		// Restore recently used state
		CMFCMenuBar::SetRecentlyUsedMenus(CMFCCustomizeMenuButton::m_bRecentlyUsedOld);

		// Made caption button non-selected
		CWnd* pWnd = GetOwner();
		if (pWnd != NULL)
		{
			if (pWnd->IsKindOf(RUNTIME_CLASS(CPaneFrameWnd)))
			{
				CPaneFrameWnd* pMiniFrm  = DYNAMIC_DOWNCAST(CPaneFrameWnd, pWnd);
				CMFCCaptionMenuButton* pBtn = (CMFCCaptionMenuButton *)pMiniFrm->FindButton(AFX_HTMENU);
				if (pBtn != NULL)
				{
					pBtn->m_bPushed = FALSE;
					pMiniFrm->OnNcPaint();

				}
			}
		}
	}

	CMFCPopupMenuBar* pMenuBar = GetMenuBar();
	ASSERT_VALID(pMenuBar);

	// First, maybe we have a dragged menu item. Remove it now:
	if (pMenuBar->m_pDragButton != NULL && !pMenuBar->m_bIsDragCopy)
	{
		pMenuBar->RemoveButton(pMenuBar->ButtonToIndex(pMenuBar->m_pDragButton));
		pMenuBar->m_pDragButton = NULL;
	}

	if (m_pParentRibbonElement != NULL)
	{
		m_pParentRibbonElement->m_pPopupMenu = NULL;

		CMFCPopupMenu* pParentMenu = GetParentPopupMenu();
		if (pParentMenu != NULL)
		{
			ASSERT_VALID(pParentMenu);

			if (pParentMenu->IsRibbonMiniToolBar())
			{
				CMFCRibbonMiniToolBar* pFloaty = DYNAMIC_DOWNCAST(CMFCRibbonMiniToolBar, pParentMenu);

				if (pFloaty != NULL && !pFloaty->IsContextMenuMode())
				{
					m_bAutoDestroyParent = FALSE;
				}
			}

			if (m_bAutoDestroyParent && !CMFCToolBar::IsCustomizeMode())
			{
				CPoint ptCursor;
				::GetCursorPos(&ptCursor);

				CRect rectParent;
				pParentMenu->GetWindowRect(rectParent);

				// Automatically close the parent popup menu:
				if (pParentMenu->IsAlwaysClose() || !rectParent.PtInRect(ptCursor))
				{
					pParentMenu->SendMessage(WM_CLOSE);
					m_pParentRibbonElement = NULL;
				}
			}
		}
	}

	if (m_pParentBtn != NULL)
	{
		ASSERT(m_pParentBtn->m_pPopupMenu == this);

		SaveState();

		m_pParentBtn->m_pPopupMenu = NULL;
		m_pParentBtn->m_bClickedOnMenu = FALSE;

		CMFCPopupMenu* pParentMenu = GetParentPopupMenu();
		if (pParentMenu != NULL)
		{
			if (m_bAutoDestroyParent && !CMFCToolBar::IsCustomizeMode())
			{
				// Automatically close the parent popup menu:
				pParentMenu->SendMessage(WM_CLOSE);
				m_pParentBtn = NULL;
			}
		}
	}
	else
	{
		CMFCMenuBar::SetShowAllCommands(FALSE);
	}

	if (m_pMenuCustomizationPage != NULL)
	{
		m_pMenuCustomizationPage->CloseContextMenu(this);
	}

	NotifyParentDlg(FALSE);

	// Inform the main frame about the menu detsroyng:
	CFrameWnd* pWndMain = AFXGetTopLevelFrame(this);

	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, pWndMain);
	if (pMainFrame != NULL)
	{
		pMainFrame->OnClosePopupMenu(this);
	}
	else // Maybe, SDI frame...
	{
		CFrameWndEx* pFrame = DYNAMIC_DOWNCAST(CFrameWndEx, pWndMain);
		if (pFrame != NULL)
		{
			pFrame->OnClosePopupMenu(this);
		}
		else // Maybe, OLE frame...
		{
			COleIPFrameWndEx* pOleFrame = DYNAMIC_DOWNCAST(COleIPFrameWndEx, pWndMain);
			if (pOleFrame != NULL)
			{
				pOleFrame->OnClosePopupMenu(this);
			}
			else
			{
				COleDocIPFrameWndEx* pOleDocFrame = DYNAMIC_DOWNCAST(COleDocIPFrameWndEx, pWndMain);
				if (pOleDocFrame != NULL)
				{
					pOleDocFrame->OnClosePopupMenu(this);
				}
			}
		}
	}

	if (m_bTrackMode && CMFCPopupMenu::m_pActivePopupMenu == this)
	{
		CMFCPopupMenu::m_pActivePopupMenu = NULL;
	}

	if (!CMFCToolBar::IsCustomizeMode() && pWndMain != NULL && m_pActivePopupMenu == NULL && GetParentToolBar() != NULL && GetParentToolBar() != GetFocus())
	{
		GetParentToolBar()->Deactivate();
	}

	if (m_pWndShadow->GetSafeHwnd () != NULL)
	{
		m_pWndShadow->DestroyWindow ();
	}

	CMiniFrameWnd::OnDestroy();
}

void CMFCPopupMenu::PostNcDestroy()
{
	if (m_pParentBtn != NULL)
	{
		ASSERT_VALID(m_pParentBtn);
		m_pParentBtn->OnCancelMode();
	}

	if (m_pParentRibbonElement != NULL)
	{
		ASSERT_VALID(m_pParentRibbonElement);
		m_pParentRibbonElement->ClosePopupMenu();
	}

	CMiniFrameWnd::PostNcDestroy();
}

void CMFCPopupMenu::SaveState()
{
	if (!CMFCToolBar::IsCustomizeMode())
	{
		return;
	}

	if (m_pParentBtn == NULL || m_pParentBtn->IsMenuPaletteMode())
	{
		return;
	}

	ASSERT_VALID(m_pParentBtn);

	CMFCPopupMenuBar* pMenuBar = GetMenuBar();
	ASSERT_VALID(pMenuBar);

	HMENU hmenu = pMenuBar->ExportToMenu();
	ENSURE(hmenu != NULL);

	m_pParentBtn->CreateFromMenu(hmenu);
	::DestroyMenu(hmenu);

	CMFCPopupMenu* pParentMenu = GetParentPopupMenu();
	if (pParentMenu != NULL)
	{
		pParentMenu->SaveState();
	}
}

void CMFCPopupMenu::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	ASSERT_VALID(this);

	CMFCPopupMenuBar* pMenuBar = GetMenuBar();
	ASSERT_VALID(pMenuBar);

#ifdef _DEBUG
	if (m_pParentBtn != NULL)
	{
		ASSERT_VALID(m_pParentBtn);
		ASSERT(m_pParentBtn->m_pPopupMenu == this);
	}
#endif // _DEBUG

	BOOL bHightlightWasChanged = FALSE;
	BOOL bIsRTL = GetExStyle() & WS_EX_LAYOUTRTL;

	if (bIsRTL)
	{
		if (nChar == VK_LEFT)
		{
			nChar = VK_RIGHT;
		}
		else if (nChar == VK_RIGHT)
		{
			nChar = VK_LEFT;
		}
	}

	switch (nChar)
	{
	case VK_RIGHT:
		{
			// Save animation type and disable animation:
			ANIMATION_TYPE animType = GetAnimationType();
			m_AnimationType = NO_ANIMATION;

			// Try to open next cascade menu:
			CMFCToolBarMenuButton* pSelItem = GetSelItem();
			if (pSelItem != NULL &&
				(pSelItem->m_nID == (UINT) -1 || pSelItem->m_nID == 0 || pSelItem->IsEmptyMenuAllowed()) && pSelItem->OpenPopupMenu())
			{
				if (pSelItem->m_pPopupMenu != NULL)
				{
					// Select a first menu item:
					if (GetSelItem() == pSelItem)
					{
						pSelItem->m_pPopupMenu->OnKeyDown(VK_HOME, 0, 0);
					}
				}
			}
			else
			{
				// No next menu, first try to go to the parent menu bar:
				CMFCToolBar* pToolBar = GetParentToolBar();
				if (pToolBar != NULL && !pToolBar->IsKindOf(RUNTIME_CLASS(CMFCPopupMenuBar)))
				{
					pToolBar->NextMenu();
				}
				else
				{
					// Close the current menu and move control to the parent
					// popup menu:
					CMFCPopupMenu* pParenMenu = GetParentPopupMenu();
					if (pParenMenu != NULL)
					{
						pParenMenu->SendMessage(WM_KEYDOWN, bIsRTL ? VK_LEFT : VK_RIGHT);
					}
				}
			}

			// Restore animation type:
			m_AnimationType = animType;
		}
		return;

	case VK_LEFT:
		{
			CMFCToolBar* pToolBar = GetParentToolBar();
			if (pToolBar != NULL)
			{
				pToolBar->PrevMenu();
			}
			else if (m_pParentBtn != NULL && m_pParentBtn->IsDroppedDown())
			{
				CloseMenu();
			}
		}
		return;

	case VK_DOWN:
		if ((::GetAsyncKeyState(VK_CONTROL) & 0x8000) && !pMenuBar->m_bAreAllCommandsShown)
		{
			ShowAllCommands();
			break;
		}

	case VK_UP:
	case VK_HOME:
	case VK_END:
		bHightlightWasChanged = TRUE;

	case VK_RETURN:
		if (!CMFCToolBar::IsCustomizeMode())
		{
			pMenuBar->OnKey(nChar);
		}
		break;

	case VK_ESCAPE:
		{
			m_bEscClose = TRUE;
			CloseMenu(TRUE);
		}

		return;

	case VK_PRIOR:
	case VK_NEXT:
		if (m_bShowScrollBar)
		{
			bHightlightWasChanged = TRUE;
			pMenuBar->OnKey(nChar);
			break;
		}

	default:
		if (pMenuBar->OnKey(nChar))
		{
			return;
		}
		else
		{
			CMiniFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
		}
	}

	if (bHightlightWasChanged && m_bScrollable && pMenuBar->m_iHighlighted >= 0)
	{
		// Maybe, selected item is invisible now?
		CMFCToolBarButton* pItem = pMenuBar->GetButton(pMenuBar->m_iHighlighted);
		if (pItem == NULL && pMenuBar->GetRowHeight() == 0)
		{
			ASSERT(FALSE);
		}
		else
		{
			CRect rectBar;
			pMenuBar->GetClientRect(rectBar);

			int iOffset = pMenuBar->GetOffset();
			int iOffsetDelta = 0;

			if (pItem->Rect().top < rectBar.top)
			{
				// Scroll up is needed!
				iOffsetDelta = (pItem->Rect().top - rectBar.top) / pMenuBar->GetRowHeight() - 1;
			}
			else if (pItem->Rect().bottom > rectBar.bottom)
			{
				// Scroll down is needed!
				iOffsetDelta = (pItem->Rect().bottom - rectBar.bottom) / pMenuBar->GetRowHeight() + 1;
			}

			if (iOffsetDelta != 0)
			{
				int iTotalRows = m_FinalSize.cy / pMenuBar->GetRowHeight() - 2;

				iOffset += iOffsetDelta;
				iOffset = min(max(0, iOffset), (int) pMenuBar->m_Buttons.GetCount() - iTotalRows - 1);

				pMenuBar->SetOffset(iOffset);

				const BOOL bScrollButtonsChanged = AdjustScroll();

				if (m_bShowScrollBar && m_wndScrollBarVert.GetSafeHwnd() != NULL)
				{
					m_wndScrollBarVert.SetScrollPos(iOffset);
				}
				else if (bScrollButtonsChanged)
				{
					// Scroll buttons were changed, adjust again
					AdjustScroll();
				}
			}
		}
	}

	if (bHightlightWasChanged && pMenuBar->m_bDropDownListMode)
	{
		OnChangeHot(pMenuBar->m_iHighlighted);
	}
}

CMFCPopupMenu* CMFCPopupMenu::GetParentPopupMenu() const
{
	ASSERT_VALID(this);

	CMFCPopupMenuBar* pParentBar = NULL;

	if (m_pParentBtn != NULL)
	{
		ASSERT_VALID(m_pParentBtn);
		pParentBar = DYNAMIC_DOWNCAST(CMFCPopupMenuBar, m_pParentBtn->m_pWndParent);
	}
	else if (m_pParentRibbonElement != NULL)
	{
		ASSERT_VALID(m_pParentRibbonElement);
		pParentBar = DYNAMIC_DOWNCAST(CMFCPopupMenuBar, m_pParentRibbonElement->GetParentWnd());
	}

	if (pParentBar != NULL)
	{
		CMFCPopupMenu* pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, AFXGetParentFrame(pParentBar));
		return pParentMenu;
	}
	else
	{
		return NULL;
	}
}

CMFCToolBar* CMFCPopupMenu::GetParentToolBar() const
{
	if (m_pParentBtn == NULL)
	{
		return NULL;
	}

	CMFCToolBar* pParentBar = DYNAMIC_DOWNCAST(CMFCToolBar, m_pParentBtn->m_pWndParent);
	return pParentBar;
}

CMFCToolBarMenuButton* CMFCPopupMenu::GetSelItem()
{
	CMFCPopupMenuBar* pMenuBar = GetMenuBar();
	ASSERT_VALID(pMenuBar);

	return DYNAMIC_DOWNCAST(CMFCToolBarMenuButton, pMenuBar->GetHighlightedButton());
}

void CMFCPopupMenu::CloseMenu(BOOL bSetFocusToBar)
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	m_bTobeDstroyed = TRUE;

	SaveState();

	CMFCPopupMenu* pParentMenu = GetParentPopupMenu();
	CMFCToolBar* pParentToolBar = GetParentToolBar();

	CFrameWnd* pWndMain = AFXGetTopLevelFrame(this);
	if (pParentMenu != NULL)
	{
		m_bAutoDestroyParent = FALSE;
		ActivatePopupMenu(pWndMain, pParentMenu);
	}
	else if (pParentToolBar != NULL)
	{
		ActivatePopupMenu(pWndMain, NULL);
		NotifyParentDlg(FALSE);

		if (bSetFocusToBar)
		{
			pParentToolBar->SetFocus();
		}
	}
	else
	{
		ActivatePopupMenu(pWndMain, NULL);
		NotifyParentDlg(FALSE);
	}

	SendMessage(WM_CLOSE);
}

int CMFCPopupMenu::InsertItem(const CMFCToolBarMenuButton& button, int iInsertAt)
{
	CMFCPopupMenuBar* pMenuBar = GetMenuBar();
	ASSERT_VALID(pMenuBar);

	return pMenuBar->InsertButton(button, iInsertAt);
}

int CMFCPopupMenu::InsertSeparator(int iInsertAt)
{
	CMFCPopupMenuBar* pMenuBar = GetMenuBar();
	ASSERT_VALID(pMenuBar);

	return pMenuBar->InsertSeparator(iInsertAt);
}

int CMFCPopupMenu::GetMenuItemCount() const
{
	CMFCPopupMenuBar* pMenuBar = ((CMFCPopupMenu*) this)->GetMenuBar();
	ASSERT_VALID(pMenuBar);

	return(int) pMenuBar->m_Buttons.GetCount();
}

CMFCToolBarMenuButton* CMFCPopupMenu::GetMenuItem(int iIndex) const
{
	CMFCPopupMenuBar* pMenuBar = ((CMFCPopupMenu*) this)->GetMenuBar();
	ASSERT_VALID(pMenuBar);

	return(CMFCToolBarMenuButton*) pMenuBar->GetButton(iIndex);
}

CMFCToolBarMenuButton* CMFCPopupMenu::FindSubItemByCommand(UINT uiCmd) const
{
	CMFCPopupMenuBar* pMenuBar = ((CMFCPopupMenu*) this)->GetMenuBar();
	ASSERT_VALID(pMenuBar);

	for (POSITION pos = pMenuBar->m_Buttons.GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarMenuButton* pItem = DYNAMIC_DOWNCAST(CMFCToolBarMenuButton, pMenuBar->m_Buttons.GetNext(pos));

		if (pItem != NULL)
		{
			ASSERT_VALID(pItem);

			const CObList& listCommands = pItem->GetCommands();

			for (POSITION posList = listCommands.GetHeadPosition(); posList != NULL;)
			{
				CMFCToolBarMenuButton* pSubItem = (CMFCToolBarMenuButton*) listCommands.GetNext(posList);
				ASSERT_VALID(pSubItem);

				if (pSubItem->m_nID == uiCmd)
				{
					return pItem;
				}
			}
		}
	}

	return NULL;
}

BOOL CMFCPopupMenu::RemoveItem(int iIndex)
{
	CMFCPopupMenuBar* pMenuBar = GetMenuBar();
	ASSERT_VALID(pMenuBar);

	return pMenuBar->RemoveButton(iIndex);
}

void CMFCPopupMenu::RemoveAllItems()
{
	CMFCPopupMenuBar* pMenuBar = GetMenuBar();
	ASSERT_VALID(pMenuBar);

	pMenuBar->RemoveAllButtons();
}

BOOL CMFCPopupMenu::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

BOOL __stdcall CMFCPopupMenu::ActivatePopupMenu(CFrameWnd* pTopFrame, CMFCPopupMenu* pPopupMenu)
{
	if (pPopupMenu != NULL)
	{
		pPopupMenu->NotifyParentDlg(TRUE);
	}

	if (pTopFrame != NULL)
	{
		ASSERT_VALID(pTopFrame);

		BOOL bRes = TRUE;

		CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, pTopFrame);
		if (pMainFrame != NULL)
		{
			bRes = pMainFrame->ShowPopupMenu(pPopupMenu);
		}
		else // Maybe, SDI frame...
		{
			CFrameWndEx* pFrame = DYNAMIC_DOWNCAST(CFrameWndEx, pTopFrame);
			if (pFrame != NULL)
			{
				bRes = pFrame->ShowPopupMenu(pPopupMenu);
			}
			else // Maybe, OLE frame
			{
				COleIPFrameWndEx* pOleFrame = DYNAMIC_DOWNCAST(COleIPFrameWndEx, pTopFrame);
				if (pOleFrame != NULL)
				{
					bRes = pOleFrame->ShowPopupMenu(pPopupMenu);
				}
				else
				{
					COleDocIPFrameWndEx* pOleDocFrame = DYNAMIC_DOWNCAST(COleDocIPFrameWndEx, pTopFrame);
					if (pOleDocFrame != NULL)
					{
						bRes = pOleDocFrame->ShowPopupMenu(pPopupMenu);
					}
				}
			}
		}

		if (!bRes)
		{
			if (pPopupMenu != NULL && !pPopupMenu->m_bTobeDstroyed)
			{
				pPopupMenu->CloseMenu();
			}

			return FALSE;
		}
	}

	if (pPopupMenu != NULL)
	{
		CMFCPopupMenuBar* pMenuBar = pPopupMenu->GetMenuBar();
		ASSERT_VALID(pMenuBar);

		CMFCPopupMenu* pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, pMenuBar->GetParent());
		if (pParentMenu != NULL && pParentMenu->GetParentButton() != NULL && !pMenuBar->m_bAreAllCommandsShown)
		{
			// Check if "Show all" button is not exist yet:
			if (pMenuBar->m_Buttons.IsEmpty() || DYNAMIC_DOWNCAST(CMFCShowAllButton, pMenuBar->m_Buttons.GetTail()) == NULL)
			{
				pMenuBar->InsertButton(CMFCShowAllButton());
			}
		}

		if (pPopupMenu->m_bTrackMode)
		{
			CMFCPopupMenu::m_pActivePopupMenu = pPopupMenu;
		}
	}

	return TRUE;
}

void CMFCPopupMenu::OnActivateApp(BOOL bActive, DWORD /*dwThreadID*/)
{
	if (!bActive && !CMFCToolBar::IsCustomizeMode() && !InCommand())
	{
		if (m_bTrackMode)
		{
			m_bTobeDstroyed = TRUE;
		}

		PostMessage(WM_CLOSE);
	}
}

void CMFCPopupMenu::OnTimer(UINT_PTR nIDEvent)
{
	CMFCPopupMenuBar* pMenuBar = GetMenuBar();
	ASSERT_VALID(pMenuBar);

	switch (nIDEvent)
	{
	case nAnimTimerId:
		if (!m_bAnimationIsDone)
		{
			clock_t nCurrAnimTime = clock();

			int nDuration = nCurrAnimTime - nLastAnimTime;
			int nSteps = (int)(.5 +(float) nDuration / m_AnimationSpeed);

			switch (GetAnimationType())
			{
			case UNFOLD:
				m_AnimSize.cx += nSteps * pMenuBar->GetColumnWidth();
				// no break intentionally

			case SLIDE:
				m_AnimSize.cy += nSteps * pMenuBar->GetRowHeight();
				break;

			case FADE:
				m_iFadePercent += nFadeStep;
				if (m_iFadePercent > 100 + nSteps * nFadeStep)
				{
					m_iFadePercent = 101;
				}
				break;
			}

			if ((GetAnimationType() != FADE && m_AnimSize.cy - m_iShadowSize >= m_FinalSize.cy) ||
				(GetAnimationType() == UNFOLD && m_AnimSize.cx - m_iShadowSize >= m_FinalSize.cx) || (GetAnimationType() == FADE && m_iFadePercent > 100))
			{

				m_AnimSize.cx = m_FinalSize.cx + m_iShadowSize;
				m_AnimSize.cy = m_FinalSize.cy + m_iShadowSize;

				KillTimer(nAnimTimerId);

				pMenuBar->SetWindowPos(NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOREDRAW|SWP_NOZORDER|SWP_SHOWWINDOW | SWP_NOACTIVATE);
				pMenuBar->ValidateRect(NULL);

				m_bAnimationIsDone = TRUE;

				if (m_iShadowSize != 0 && GetAnimationType() != FADE && m_DropDirection == DROP_DIRECTION_TOP)
				{
					UpdateShadow();
				}
			}

			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);

			nLastAnimTime = nCurrAnimTime;
		}
		break;

	case nScrollTimerId:
		{
			CPoint point;
			::GetCursorPos(&point);
			ScreenToClient(&point);

			CMFCToolBarMenuButton* pSelItem = GetSelItem();
			if (pSelItem != NULL)
			{
				pSelItem->OnCancelMode();
			}

			int iOffset = pMenuBar->GetOffset();

			if (m_rectScrollUp.PtInRect(point) && m_iScrollMode < 0) // Scroll Up
			{
				pMenuBar->SetOffset(iOffset - 1);
				AdjustScroll();
			}
			else if (m_rectScrollDn.PtInRect(point) && m_iScrollMode > 0) // Scroll Down
			{
				pMenuBar->SetOffset(iOffset + 1);
				AdjustScroll();
			}
			else
			{
				KillTimer(nScrollTimerId);
				m_iScrollMode = 0;
				InvalidateRect(m_rectScrollDn);
				InvalidateRect(m_rectScrollUp);
			}
		}
		break;
	}

	CMiniFrameWnd::OnTimer(nIDEvent);
}

void CMFCPopupMenu::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bTearOffTracking)
	{
		if (!m_rectTearOffCaption.PtInRect(point))
		{
			ReleaseCapture();
			m_bTearOffTracking = FALSE;

			TearOff(point);
		}

		return;
	}

	CMiniFrameWnd::OnMouseMove(nFlags, point);

	if (!m_bScrollable || m_iScrollMode != 0)
	{
		return;
	}

	if (m_rectScrollUp.PtInRect(point) && IsScrollUpAvailable())
	{
		m_iScrollMode = -1;
		InvalidateRect(m_rectScrollUp);
	}
	else if (m_rectScrollDn.PtInRect(point) && IsScrollDnAvailable())
	{
		m_iScrollMode = 1;
		InvalidateRect(m_rectScrollDn);
	}
	else
	{
		m_iScrollMode = 0;
	}

	if (m_iScrollMode != 0)
	{
		SetTimer(nScrollTimerId, nScrollTimerDuration, NULL);
	}
}

BOOL CMFCPopupMenu::IsScrollUpAvailable()
{
	CMFCPopupMenuBar* pMenuBar = GetMenuBar();
	ASSERT_VALID(pMenuBar);

	return pMenuBar->GetOffset() > 0;
}

BOOL CMFCPopupMenu::IsScrollDnAvailable()
{
	CMFCPopupMenuBar* pMenuBar = GetMenuBar();
	ASSERT_VALID(pMenuBar);

	if (pMenuBar->GetCount() == 0)
	{
		return FALSE;
	}

	CRect rectLastItem;
	pMenuBar->GetItemRect(pMenuBar->GetCount() - 1, rectLastItem);

	return rectLastItem.bottom > m_nMenuBarHeight + pMenuBar->GetRowHeight();
}

void CMFCPopupMenu::CollapseSubmenus()
{
	CMFCPopupMenuBar* pMenuBar = GetMenuBar();
	ENSURE(pMenuBar != NULL);
	ASSERT_VALID(pMenuBar);

	for (POSITION pos = pMenuBar->m_Buttons.GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarButton* pButton = (CMFCToolBarButton*) pMenuBar->m_Buttons.GetNext(pos);
		ENSURE(pButton != NULL);

		pButton->OnCancelMode();
	}
}

void CMFCPopupMenu::ShowAllCommands()
{
	CMFCToolBarMenuButton* pParentMenuButton = DYNAMIC_DOWNCAST(CMFCToolBarMenuButton, m_pParentBtn);
	if (pParentMenuButton != NULL)
	{
		CMFCPopupMenuBar* pMenuBar = GetMenuBar();
		ASSERT_VALID(pMenuBar);

		pMenuBar->SetHot(NULL);

		CMFCMenuBar::SetShowAllCommands();

		// Play standard menu popup sound!
		AFXPlaySystemSound(AFX_SOUND_MENU_POPUP);

		ShowWindow(SW_HIDE);
		m_bShown = FALSE;

		if (m_bmpShadowRight.GetSafeHandle() != NULL)
		{
			m_bmpShadowRight.DeleteObject();
		}

		if (m_bmpShadowBottom.GetSafeHandle() != NULL)
		{
			m_bmpShadowBottom.DeleteObject();
		}

		m_ptLocation = m_ptLocationInitial;

		InitMenuBar();

		if (m_bScrollable)
		{
			AdjustScroll();
			SetScrollBar();
		}

		UpdateBottomWindows();

		ShowWindow(SW_SHOWNOACTIVATE);

		if (pParentMenuButton->m_pWndParent != NULL && ::IsWindow(pParentMenuButton->m_pWndParent->m_hWnd))
		{
			pParentMenuButton->m_pWndParent->InvalidateRect(pParentMenuButton->Rect());
			pParentMenuButton->m_pWndParent->UpdateWindow();
		}
	}
}

void CMFCPopupMenu::SetMaxWidth(int iMaxWidth)
{
	if (iMaxWidth == m_iMaxWidth)
	{
		return;
	}

	m_iMaxWidth = iMaxWidth;
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CMFCPopupMenuBar* pMenuBar = GetMenuBar();
	ASSERT_VALID(pMenuBar);

	if (!::IsWindow(m_hWnd) || !::IsWindow(pMenuBar->m_hWnd))
	{
		return;
	}

	pMenuBar->m_iMaxWidth = m_iMaxWidth;
	RecalcLayout();
}

BOOL CMFCPopupMenu::InitMenuBar()
{
	CMFCPopupMenuBar* pMenuBar = GetMenuBar();
	ASSERT_VALID(pMenuBar);

	if (m_hMenu != NULL)
	{
		ENSURE(::IsMenu(m_hMenu));

		if (m_pParentBtn != NULL || !afxMenuHash.LoadMenuBar(m_hMenu, pMenuBar))
		{
			// Failed to restore, load the default state:
			if (CMFCMenuBar::IsShowAllCommands())
			{
				if (!pMenuBar->ImportFromMenu(m_hMenu, TRUE))
				{
					TRACE(_T("Can't import menu\n"));
					return FALSE;
				}

			}
			else
			{

				if (!pMenuBar->ImportFromMenu(m_hMenu, !HideRarelyUsedCommands()))
				{
					TRACE(_T("Can't import menu\n"));
					return FALSE;
				}
			}
		}
	}

	POSITION pos;

	// Maybe, we should process the MRU files:
	CRecentFileList* pMRUFiles = ((CWinAppEx*)AfxGetApp())->m_pRecentFileList;

	if (pMRUFiles != NULL && !CMFCToolBar::IsCustomizeMode())
	{
		int iMRUItemIndex = 0;
		BOOL bIsPrevSeparator = FALSE;

		for (pos = pMenuBar->m_Buttons.GetHeadPosition(); pos != NULL; iMRUItemIndex ++)
		{
			POSITION posSave = pos;

			CMFCToolBarButton* pButton = (CMFCToolBarButton*) pMenuBar->m_Buttons.GetNext(pos);
			ENSURE(pButton != NULL);

			if (pButton->m_nID == ID_FILE_MRU_FILE1 && pButton->m_strText == _T("Recent File"))
			{
				// Remove dummy item("Recent"):
				pMenuBar->m_Buttons.RemoveAt(posSave);
				delete pButton;

				TCHAR szCurDir [_MAX_PATH];
				::GetCurrentDirectory(_MAX_PATH, szCurDir);

				int nCurDir = lstrlen(szCurDir);
				ASSERT(nCurDir >= 0);

				szCurDir [nCurDir] = _T('\\');
				szCurDir [++ nCurDir] = _T('\0');

				// Add MRU files:
				int iNumOfFiles = 0; // Actual added to menu
				for (int i = 0; i < pMRUFiles->GetSize(); i ++)
				{
					CString strName;

					if (pMRUFiles->GetDisplayName(strName, i, szCurDir, nCurDir))
					{
						// Add shortcut number:
						CString strItem;
						strItem.Format(_T("&%d %s"), ++iNumOfFiles, (LPCTSTR)strName);

						pMenuBar->InsertButton(CMFCToolBarMenuButton(ID_FILE_MRU_FILE1 + i, NULL, -1, strItem), iMRUItemIndex ++);
					}
				}

				// Usualy, the MRU group is "covered" by two seperators.
				// If MRU list is empty, remove redandant separator:
				if (iNumOfFiles == 0 && // No files were added
					bIsPrevSeparator && // Prev. button was separator
					pos != NULL) // Not a last button
				{
					posSave = pos;

					pButton = (CMFCToolBarButton*)
						pMenuBar->m_Buttons.GetNext(pos);
					ENSURE(pButton != NULL);

					if (pButton->m_nStyle & TBBS_SEPARATOR)
					{
						// Next button also separator, remove it:
						pMenuBar->m_Buttons.RemoveAt(posSave);
						delete pButton;
					}
				}

				break;
			}

			bIsPrevSeparator = (pButton->m_nStyle & TBBS_SEPARATOR);
		}
	}

	// Setup user-defined tools:
	if (afxUserToolsManager != NULL && !CMFCToolBar::IsCustomizeMode())
	{
		BOOL bToolsAreReady = FALSE;
		int iToolItemIndex = 0;

		BOOL bIsPrevSeparator = FALSE;

		for (pos = pMenuBar->m_Buttons.GetHeadPosition(); pos != NULL; iToolItemIndex ++)
		{
			POSITION posSave = pos;

			CMFCToolBarButton* pButton =
				(CMFCToolBarButton*) pMenuBar->m_Buttons.GetNext(pos);
			ENSURE(pButton != NULL);
			ASSERT_VALID(pButton);

			if (afxUserToolsManager->GetToolsEntryCmd() == pButton->m_nID)
			{
				const CObList& lstTools = afxUserToolsManager->GetUserTools();

				// Replace dummy tools command by the user tools list:
				if (!m_bAlwaysShowEmptyToolsEntry || !lstTools.IsEmpty())
				{
					pMenuBar->m_Buttons.RemoveAt(posSave);
					delete pButton;
				}

				if (!bToolsAreReady)
				{
					if (!bIsPrevSeparator && !lstTools.IsEmpty() && !pMenuBar->m_Buttons.IsEmpty())
					{
						// Add separator before the first tool:
						pMenuBar->InsertSeparator(iToolItemIndex++);
					}

					for (POSITION posTool = lstTools.GetHeadPosition(); posTool != NULL;)
					{
						CUserTool* pTool = (CUserTool*) lstTools.GetNext(posTool);
						ASSERT_VALID(pTool);

						// Is user tool associated with the user image?
						int iUserImage = afxCommandManager->GetCmdImage(pTool->GetCommandId(), TRUE);

						pMenuBar->InsertButton(CMFCToolBarMenuButton(pTool->GetCommandId(), NULL, iUserImage == -1 ? 0 : iUserImage, pTool->m_strLabel, iUserImage != -1), iToolItemIndex ++);
					}

					if (pos != NULL)
					{
						// Add separator after the last tool:
						bIsPrevSeparator = pMenuBar->InsertSeparator(iToolItemIndex++) >= 0;
					}

					bToolsAreReady = TRUE;
				}
			}
			else if (pButton->m_nStyle & TBBS_SEPARATOR)
			{
				if (bIsPrevSeparator)
				{
					pMenuBar->m_Buttons.RemoveAt(posSave);
					delete pButton;
				}

				bIsPrevSeparator = TRUE;
			}
			else
			{
				bIsPrevSeparator = FALSE;
			}
		}
	}

	CFrameWnd* pTarget = (CFrameWnd*) pMenuBar->GetCommandTarget();
	if (pTarget == NULL || !pTarget->IsFrameWnd())
	{
		pTarget = AFXGetParentFrame(this);
	}
	if (pTarget != NULL)
	{
		pMenuBar->OnUpdateCmdUI(pTarget, FALSE);
	}

	// Maybe, main application frame should update the popup menu context before it
	// displayed(example - windows list):
	if (!ActivatePopupMenu(AFXGetTopLevelFrame(this), this))
	{
		return FALSE;
	}

	RecalcLayout();
	return TRUE;
}

BOOL CMFCPopupMenu::HideRarelyUsedCommands() const
{
	return(m_pParentBtn != NULL);
}

void CMFCPopupMenu::UpdateBottomWindows(BOOL bCheckOnly)
{
	CWnd* pWndMain = GetTopLevelParent();
	if (pWndMain == NULL)
	{
		return;
	}

	if (m_iShadowSize == 0 || GetForceShadow())
	{
		pWndMain->UpdateWindow();
		return;
	}

	// If menu will be shown outside of the application window,
	// don't show shadows!
	CRect rectMain;
	pWndMain->GetWindowRect(rectMain);

	CRect rectMenu(m_ptLocation, CSize(m_FinalSize.cx + m_iShadowSize, m_FinalSize.cy + m_iShadowSize));

	if (GetExStyle() & WS_EX_LAYOUTRTL)
	{
		rectMenu.OffsetRect(-(m_FinalSize.cx + m_iShadowSize), 0);
	}

	CRect rectInter;
	rectInter.UnionRect(&rectMenu, &rectMain);

	if (rectInter != rectMain)
	{
		m_iShadowSize = 0;

		if (!bCheckOnly)
		{
			SetWindowPos(NULL, -1, -1, m_FinalSize.cx, m_FinalSize.cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}
	else
	{
		pWndMain->UpdateWindow();
	}
}

void CMFCPopupMenu::DoPaint(CDC* pPaintDC)
{
	CRect rectClient; // Client area rectangle
	GetClientRect(&rectClient);

	BOOL bDrawShadows = m_iShadowSize != 0 && !CMFCToolBar::IsCustomizeMode();

	if (bDrawShadows)
	{
		BOOL bRTL = GetExStyle() & WS_EX_LAYOUTRTL;

		if (bRTL)
		{
			rectClient.left += m_iShadowSize;
		}
		else
		{
			rectClient.right -= m_iShadowSize;
		}

		rectClient.bottom -= m_iShadowSize;

		const int iMinBrightness = 100;
		const int iMaxBrightness = 65;

		// Draw the shadow, exclude the parent button:
		CRect rectParentBtn;
		rectParentBtn.SetRectEmpty();

		if (m_pParentBtn != NULL && GetParentPopupMenu() == NULL)
		{
			CWnd* pWnd = m_pParentBtn->GetParentWnd();
			if (pWnd!= NULL && pWnd->GetSafeHwnd() != NULL)
			{
				rectParentBtn = m_pParentBtn->Rect();
				rectParentBtn.right--;
				rectParentBtn.bottom--;
				pWnd->MapWindowPoints(this, &rectParentBtn);
			}
		}

		// Prevent shadow drawing over Quick Customize Add-Remove button
		if (CMFCVisualManager::GetInstance()->IsOfficeXPStyleMenus())
		{
			CMFCPopupMenu* pParentPopup = GetParentPopupMenu();
			if (pParentPopup != NULL)
			{
				ASSERT_VALID(pParentPopup);

				CMFCToolBarMenuButton* pParentBtn = GetParentButton();
				if ((pParentBtn != NULL) &&(pParentBtn->IsQuickMode()))
				{
					if (pParentPopup->IsQuickCustomize())
					{
						if (!m_bQuickCusomize &&(m_DropDirection == DROP_DIRECTION_LEFT))
						{
							CWnd* pWnd = m_pParentBtn->GetParentWnd();
							if (pWnd != NULL && pWnd->GetSafeHwnd() != NULL)
							{
								rectParentBtn = m_pParentBtn->Rect();
								rectParentBtn.bottom += 2;
								pWnd->MapWindowPoints(this, &rectParentBtn);
							}
						}
					}
				}
			}
		}

		if (afxGlobalData.bIsWindowsVista)
		{
			CClientDC dcDesktop(NULL);

			CRect rectMenu;
			GetWindowRect(rectMenu);

			pPaintDC->BitBlt(0, 0, rectMenu.Width(), rectMenu.Height(), &dcDesktop, rectMenu.left, rectMenu.top, SRCCOPY);

			if (bRTL)
			{
				CDrawingManager dm(*pPaintDC);
				dm.MirrorRect(CRect(0, 0, rectMenu.Width(), rectMenu.Height()));
			}
		}

		CMFCVisualManager::GetInstance()->OnDrawMenuShadow(pPaintDC, rectClient, rectParentBtn, m_iShadowSize, iMinBrightness, iMaxBrightness, &m_bmpShadowBottom, &m_bmpShadowRight, bRTL);

		if (bRTL)
		{
			rectClient.OffsetRect(-m_iShadowSize,0);
		}
	}

	CMFCVisualManager::GetInstance()->OnDrawMenuBorder(pPaintDC, this, rectClient);
	const int nBorderSize = GetBorderSize();

	rectClient.DeflateRect(nBorderSize, nBorderSize);

	// Draw menu logo(if exist):
	if (m_iLogoWidth > 0)
	{
		CRect rectLogo = rectClient;

		switch (m_nLogoLocation)
		{
		case MENU_LOGO_LEFT:
			rectLogo.right = rectLogo.left + nBorderSize + m_iLogoWidth;
			break;

		case MENU_LOGO_RIGHT:
			rectLogo.left = rectLogo.right - nBorderSize - m_iLogoWidth;
			break;

		case MENU_LOGO_TOP:
			rectLogo.bottom = rectLogo.top + nBorderSize + m_iLogoWidth;
			break;

		case MENU_LOGO_BOTTOM:
			rectLogo.top = rectLogo.bottom - nBorderSize - m_iLogoWidth;
			break;
		}

		CFrameWnd* pWndMain = AFXGetTopLevelFrame(this);

		CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, pWndMain);
		if (pMainFrame != NULL)
		{
			pMainFrame->OnDrawMenuLogo(pPaintDC, this, rectLogo);
		}
		else // Maybe, SDI frame...
		{
			CFrameWndEx* pFrame = DYNAMIC_DOWNCAST(CFrameWndEx, pWndMain);
			if (pFrame != NULL)
			{
				pFrame->OnDrawMenuLogo(pPaintDC, this, rectLogo);
			}
			else // Maybe, OLE frame...
			{
				COleIPFrameWndEx* pOleFrame = DYNAMIC_DOWNCAST(COleIPFrameWndEx, pWndMain);
				if (pOleFrame != NULL)
				{
					pOleFrame->OnDrawMenuLogo(pPaintDC, this, rectLogo);
				}
				else
				{
					COleDocIPFrameWndEx* pOleDocFrame = DYNAMIC_DOWNCAST(COleDocIPFrameWndEx, pWndMain);
					if (pOleDocFrame != NULL)
					{
						pOleDocFrame->OnDrawMenuLogo(pPaintDC, this, rectLogo);
					}
				}

			}
		}
	}

	if (!m_rectTearOffCaption.IsRectEmpty())
	{
		CMFCVisualManager::GetInstance()->OnDrawTearOffCaption(pPaintDC, m_rectTearOffCaption, m_bIsTearOffCaptionActive);
	}

	if (m_bScrollable)
	{
		if (IsScrollUpAvailable())
		{
			CMFCVisualManager::GetInstance()->OnDrawMenuScrollButton(pPaintDC, m_rectScrollUp, FALSE, m_iScrollMode < 0, FALSE, FALSE);
		}

		if (IsScrollDnAvailable())
		{
			if (GetMenuBar() != NULL)
			{
				CRect rectFill = rectClient;
				rectFill.bottom = m_rectScrollDn.top;
				rectFill.top = rectFill.bottom - nBorderSize - 1;

				CMFCVisualManager::GetInstance()->OnFillBarBackground(pPaintDC, GetMenuBar(), rectFill, rectFill);
			}

			CMFCVisualManager::GetInstance()->OnDrawMenuScrollButton(pPaintDC, m_rectScrollDn, TRUE, m_iScrollMode > 0, FALSE, FALSE);
		}
	}

	if (!m_rectResize.IsRectEmpty())
	{
		MENU_RESIZE_TYPE resizeType;

		if (m_bIsResizeBarOnTop)
		{
			resizeType = m_sizeMinResize.cx > 0 ? MENU_RESIZE_TOP_RIGHT : MENU_RESIZE_TOP;
		}
		else
		{
			resizeType = m_sizeMinResize.cx > 0 ? MENU_RESIZE_BOTTOM_RIGHT : MENU_RESIZE_BOTTOM;
		}

		CMFCVisualManager::GetInstance()->OnDrawMenuResizeBar(pPaintDC, m_rectResize, resizeType);
	}

	m_bShown = TRUE;
}

void CMFCPopupMenu::DrawFade(CDC* pPaintDC)
{
	CRect rectClient;
	GetClientRect(&rectClient);

	int cx = m_FinalSize.cx + m_iShadowSize;
	int cy = m_FinalSize.cy + m_iShadowSize;

	CDC dcMem;
	if (!dcMem.CreateCompatibleDC(pPaintDC))
	{
		return;
	}

	// create the three bitmaps if not done yet
	if (m_bmpScreenDst.GetSafeHandle() == NULL)
	{
		CBitmap* pBmpOld = NULL;

		if (GetAnimationType() == FADE || afxGlobalData.m_nBitsPerPixel > 8)
		{
			// Fill in the BITMAPINFOHEADER
			BITMAPINFOHEADER bih;
			bih.biSize = sizeof(BITMAPINFOHEADER);
			bih.biWidth = cx;
			bih.biHeight = cy;
			bih.biPlanes = 1;
			bih.biBitCount = 32;
			bih.biCompression = BI_RGB;
			bih.biSizeImage = cx * cy;
			bih.biXPelsPerMeter = 0;
			bih.biYPelsPerMeter = 0;
			bih.biClrUsed = 0;
			bih.biClrImportant = 0;

			HBITMAP hmbpDib;
			// Create a DIB section and attach it to the source bitmap
			hmbpDib = CreateDIBSection(dcMem.m_hDC, (LPBITMAPINFO)&bih, DIB_RGB_COLORS, (void **)&m_cFadeSrcBits, NULL, NULL);
			if (hmbpDib == NULL || m_cFadeSrcBits == NULL)
			{
				return;
			}

			m_bmpScreenSrc.Attach( hmbpDib );

			// Create a DIB section and attach it to the destination bitmap
			hmbpDib = CreateDIBSection(dcMem.m_hDC, (LPBITMAPINFO)&bih, DIB_RGB_COLORS, (void **)&m_cFadeDstBits, NULL, NULL);
			if (hmbpDib == NULL || m_cFadeDstBits == NULL)
			{
				return;
			}
			m_bmpScreenDst.Attach( hmbpDib );

			// Create a DIB section and attach it to the temporary bitmap
			hmbpDib = CreateDIBSection(dcMem.m_hDC, (LPBITMAPINFO)&bih, DIB_RGB_COLORS, (void **)&m_cFadeTmpBits, NULL, NULL);
			if (hmbpDib == NULL || m_cFadeTmpBits == NULL)
			{
				return;
			}

			m_bmpScreenTmp.Attach( hmbpDib );

			// get source image, representing the window below the popup menu
			pBmpOld = dcMem.SelectObject(&m_bmpScreenSrc);
			dcMem.BitBlt(0, 0, cx, cy, pPaintDC, rectClient.left, rectClient.top, SRCCOPY);

			// copy it to the destination so that shadow will be ok
			memcpy(m_cFadeDstBits, m_cFadeSrcBits, sizeof(COLORREF)* cx*cy);
			dcMem.SelectObject(&m_bmpScreenDst);
		}
		else
		{
			m_bmpScreenDst.CreateCompatibleBitmap(pPaintDC, cx, cy);
			pBmpOld = dcMem.SelectObject(&m_bmpScreenDst);
		}

		// get final image
		CRect rect;

		DoPaint(&dcMem);

		CMFCPopupMenuBar* pMenuBar = GetMenuBar();
		ASSERT_VALID(pMenuBar);

		pMenuBar->GetWindowRect(&rect);
		ScreenToClient(&rect);

		dcMem.SetViewportOrg(rect.TopLeft());
		pMenuBar->DoPaint(&dcMem);
		dcMem.SetViewportOrg(CPoint(0,0));

		dcMem.SelectObject(pBmpOld);
	}

	COLORREF *src = m_cFadeSrcBits;
	COLORREF *dst = m_cFadeDstBits;
	COLORREF *tmp = m_cFadeTmpBits;

	CBitmap* pBmpOld = NULL;

	switch (GetAnimationType())
	{
	case UNFOLD:
	case SLIDE:
		pBmpOld = dcMem.SelectObject(&m_bmpScreenDst);

		pPaintDC->BitBlt(m_bIsAnimRight ? rectClient.left : rectClient.right - m_AnimSize.cx, m_bIsAnimDown ? rectClient.top : rectClient.bottom - m_AnimSize.cy, m_AnimSize.cx, m_AnimSize.cy, &dcMem, 0, 0, SRCCOPY);
		break;

	case FADE:
		pBmpOld = dcMem.SelectObject(&m_bmpScreenTmp);
		for (int pixel = 0; pixel < cx * cy; pixel++)
		{
			*tmp++ = CDrawingManager::PixelAlpha(*src++, *dst++, 100 - m_iFadePercent);
		}

		pPaintDC->BitBlt(rectClient.left, rectClient.top, cx, cy, &dcMem, 0, 0, SRCCOPY);
	}

	dcMem.SelectObject(pBmpOld);
}

BOOL CMFCPopupMenu::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (!CMiniFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
	{
		return m_pMessageWnd != NULL ? m_pMessageWnd->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo) : FALSE;
	}

	return TRUE;
}

BOOL CMFCPopupMenu::PostCommand(UINT commandID)
{
	if (m_pMessageWnd != NULL)
	{
		BOOL bIsSysCommand = (commandID >= 0xF000 && commandID < 0xF1F0);
		return m_pMessageWnd->PostMessage(bIsSysCommand ? WM_SYSCOMMAND : WM_COMMAND, commandID);
	}

	return FALSE;
}

void CMFCPopupMenu::EnableMenuLogo(int iLogoSize, LOGO_LOCATION nLogoLocation/* = MENU_LOGO_LEFT*/)
{
	m_iLogoWidth = iLogoSize;
	m_nLogoLocation = nLogoLocation;

	RecalcLayout();
}

BOOL CMFCPopupMenu::AdjustScroll(BOOL bForceMenuBarResize)
{
	CMFCPopupMenuBar* pMenuBar = GetMenuBar();
	ASSERT_VALID(pMenuBar);

	CRect rectClient;
	GetClientRect(rectClient);

	if (!CMFCToolBar::IsCustomizeMode())
	{
		rectClient.right -= m_iShadowSize;
		rectClient.bottom -= m_iShadowSize;
	}

	const int nBorderSize = GetBorderSize();
	rectClient.DeflateRect(nBorderSize, nBorderSize);

	switch (m_nLogoLocation)
	{
	case MENU_LOGO_LEFT:
		rectClient.left += m_iLogoWidth;
		break;

	case MENU_LOGO_RIGHT:
		rectClient.right -= m_iLogoWidth;
		break;

	case MENU_LOGO_TOP:
		rectClient.top += m_iLogoWidth;
		break;

	case MENU_LOGO_BOTTOM:
		rectClient.bottom -= m_iLogoWidth;
		break;
	}

	rectClient.top += m_rectTearOffCaption.Height();

	if (m_bIsResizeBarOnTop)
	{
		rectClient.top += m_rectResize.Height();
	}
	else
	{
		rectClient.bottom -= m_rectResize.Height();
	}

	CRect rectScrollUpOld = m_rectScrollUp;
	CRect rectScrollDnOld = m_rectScrollDn;

	m_rectScrollUp.SetRectEmpty();
	m_rectScrollDn.SetRectEmpty();

	UINT uiSWPFlags = SWP_NOZORDER | SWP_NOACTIVATE;

	if (m_bScrollable)
	{
		if (m_bShowScrollBar)
		{
			rectClient.right -= ::GetSystemMetrics(SM_CXVSCROLL);
		}
		else
		{
			if (IsScrollUpAvailable())
			{
				m_rectScrollUp = rectClient;
				m_rectScrollUp.top += nBorderSize;
				m_rectScrollUp.bottom = m_rectScrollUp.top + m_iScrollBtnHeight;

				rectClient.top += m_iScrollBtnHeight + nBorderSize;
			}

			if (IsScrollDnAvailable())
			{
				m_rectScrollDn = rectClient;
				m_rectScrollDn.top = m_rectScrollDn.bottom - m_iScrollBtnHeight;

				rectClient.bottom -= m_iScrollBtnHeight + nBorderSize;
			}
		}
	}
	else if (!m_bAnimationIsDone)
	{
		uiSWPFlags |= SWP_NOREDRAW;

		KillTimer(nScrollTimerId);
		m_iScrollMode = 0;
	}

	if (bForceMenuBarResize || rectScrollUpOld != m_rectScrollUp || rectScrollDnOld != m_rectScrollDn)
	{
		pMenuBar->SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), uiSWPFlags);
		m_nMenuBarHeight = rectClient.Height();
	}
	else
	{
		pMenuBar->AdjustLayout();
	}

	BOOL bScrollButtonsChanged = FALSE;

	if (rectScrollUpOld != m_rectScrollUp)
	{
		InvalidateRect(rectScrollUpOld);
		InvalidateRect(m_rectScrollUp);

		bScrollButtonsChanged = TRUE;
	}

	if (rectScrollDnOld != m_rectScrollDn)
	{
		InvalidateRect(rectScrollDnOld);
		InvalidateRect(m_rectScrollDn);

		bScrollButtonsChanged = TRUE;
	}

	if (bScrollButtonsChanged)
	{
		UpdateWindow();
	}

	return bScrollButtonsChanged;
}

CMFCPopupMenu::MENUAREA_TYPE CMFCPopupMenu::CheckArea(const CPoint& ptScreen) const
{
	ASSERT_VALID(this);

	CRect rectWindow;
	GetClientRect(rectWindow);
	ClientToScreen(rectWindow);

	if (!rectWindow.PtInRect(ptScreen))
	{
		return OUTSIDE;
	}

	CRect rectLogo = rectWindow;

	switch (m_nLogoLocation)
	{
	case MENU_LOGO_LEFT:
		rectLogo.right = rectLogo.left + m_iLogoWidth;
		break;

	case MENU_LOGO_RIGHT:
		rectLogo.left = rectLogo.right - m_iLogoWidth;
		break;

	case MENU_LOGO_TOP:
		rectLogo.bottom = rectLogo.top + m_iLogoWidth;
		break;

	case MENU_LOGO_BOTTOM:
		rectLogo.top = rectLogo.bottom - m_iLogoWidth;
		break;
	}

	if (rectLogo.PtInRect(ptScreen))
	{
		return LOGO;
	}

	if (ptScreen.x > rectWindow.right - m_iShadowSize)
	{
		return SHADOW_RIGHT;
	}

	if (ptScreen.y > rectWindow.bottom - m_iShadowSize)
	{
		return SHADOW_BOTTOM;
	}

	if (!m_rectTearOffCaption.IsRectEmpty())
	{
		CRect rectTearOffCaption = m_rectTearOffCaption;
		ClientToScreen(&rectTearOffCaption);

		if (rectTearOffCaption.PtInRect(ptScreen))
		{
			return TEAROFF_CAPTION;
		}
	}

	return MENU;
}

void CMFCPopupMenu::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!CMFCToolBar::IsCustomizeMode() && m_rectTearOffCaption.PtInRect(point))
	{
		m_bIsTearOffCaptionActive = TRUE;
		InvalidateRect(m_rectTearOffCaption);

		m_bTearOffTracking = TRUE;
		SetCapture();
		return;
	}

	CMiniFrameWnd::OnLButtonDown(nFlags, point);
}

void CMFCPopupMenu::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bTearOffTracking)
	{
		m_bTearOffTracking = FALSE;
		ReleaseCapture();

		m_bIsTearOffCaptionActive = FALSE;
		InvalidateRect(m_rectTearOffCaption);
	}

	CMiniFrameWnd::OnLButtonUp(nFlags, point);
}

BOOL CMFCPopupMenu::TearOff(CPoint point)
{
	if (m_pParentBtn == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	UINT uiID = m_pParentBtn->m_uiTearOffBarID;
	if (uiID == 0)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CPoint ptScreen = point;
	ClientToScreen(&ptScreen);

	CFrameWnd* pWndMain = AFXGetTopLevelFrame(this);
	if (pWndMain == NULL)
	{
		return FALSE;
	}
	CPane* pBar = NULL;
	CFrameImpl* pFrameImpl = NULL;

	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, pWndMain);
	if (pMainFrame != NULL)
	{
		pBar = (CPane*) pMainFrame->GetPane(uiID);
		pFrameImpl = &pMainFrame->m_Impl;
	}
	else // Maybe, SDI frame...
	{
		CFrameWndEx* pFrame = DYNAMIC_DOWNCAST(CFrameWndEx, pWndMain);
		if (pFrame != NULL)
		{
			pBar = (CPane*) pFrame->GetPane(uiID);
			pFrameImpl = &pFrame->m_Impl;
		}
		else // Maybe, OLE frame...
		{
			COleIPFrameWndEx* pOleFrame = DYNAMIC_DOWNCAST(COleIPFrameWndEx, pWndMain);
			if (pOleFrame != NULL)
			{
				pBar = (CPane*) pOleFrame->GetPane(uiID);
				pFrameImpl = &pOleFrame->m_Impl;
			}
			else
			{
				COleDocIPFrameWndEx* pOleDocFrame = DYNAMIC_DOWNCAST(COleDocIPFrameWndEx, pWndMain);
				if (pOleDocFrame != NULL)
				{
					pBar = (CPane*) pOleDocFrame->GetPane(uiID);
					pFrameImpl = &pOleDocFrame->m_Impl;
				}
			}
		}
	}

	if (pFrameImpl == NULL)
	{
		return FALSE;
	}

	if (pBar != NULL) // Already exist, just show it
	{
		pBar->ShowPane(TRUE, FALSE, TRUE);

		CRect rectBar;
		pBar->GetWindowRect(rectBar);

		int cx = rectBar.Width();
		rectBar.left = ptScreen.x;
		rectBar.right = rectBar.left + cx;

		int cy = rectBar.Height();
		rectBar.top = ptScreen.y;
		rectBar.bottom = rectBar.top + cy;

		if (pBar->IsDocked())
		{
			pBar->FloatPane(rectBar, DM_SHOW);
		}
		else
		{
			pBar->MoveWindow(rectBar);
		}
	}
	else
	{
		CString strCaption = m_pParentBtn->m_strText;
		strCaption.Remove(_T('&'));

		if ((pBar = CreateTearOffBar(pWndMain, uiID, strCaption)) == NULL)
		{
			return FALSE;
		}

		pFrameImpl->AddTearOffToolbar(pBar);
	}

	ASSERT_VALID(pBar);

	// Send trigger to the main frame:
	BOOL bTearOff = TRUE;

	if (pMainFrame != NULL)
	{
		bTearOff = pMainFrame->OnTearOffMenu(this, pBar);
	}
	else // Maybe, SDI frame...
	{
		CFrameWndEx* pFrame = DYNAMIC_DOWNCAST(CFrameWndEx, pWndMain);
		if (pFrame != NULL)
		{
			bTearOff = pFrame->OnTearOffMenu(this, pBar);
		}
		else // Maybe, OLE frame...
		{
			COleIPFrameWndEx* pOleFrame = DYNAMIC_DOWNCAST(COleIPFrameWndEx, pWndMain);
			if (pOleFrame != NULL)
			{
				bTearOff = pOleFrame->OnTearOffMenu(this, pBar);
			}
			else
			{
				COleDocIPFrameWndEx* pOleDocFrame = DYNAMIC_DOWNCAST(COleDocIPFrameWndEx, pWndMain);
				if (pOleDocFrame != NULL)
				{
					bTearOff = pOleDocFrame->OnTearOffMenu(this, pBar);
				}
			}
		}
	}

	if (!bTearOff)
	{
		pBar->DestroyWindow();
		delete pBar;

		return FALSE;
	}

	pBar->OnUpdateCmdUI(pWndMain, TRUE);

	CRect rectBar(ptScreen, pBar->CalcSize(FALSE));
	pBar->FloatPane(rectBar, DM_SHOW);
	pBar->RecalcLayout();
	pWndMain->RecalcLayout();

	CPaneFrameWnd* pFloatFrame = pBar->GetParentMiniFrame(TRUE);
	if (pFloatFrame != NULL)
	{
		return pFloatFrame->StartTearOff(this);
	}

	return FALSE;
}

CPane* CMFCPopupMenu::CreateTearOffBar(CFrameWnd* pWndMain, UINT uiID, LPCTSTR lpszName)
{
	ASSERT_VALID(pWndMain);
	ENSURE(lpszName != NULL);
	ASSERT(uiID != 0);

	if (m_hMenu == NULL)
	{
		return NULL;
	}

	CMenu* pMenu = CMenu::FromHandle(m_hMenu);
	if (pMenu == NULL)
	{
		return NULL;
	}

	CMFCToolBar* pNewToolbar = new CMFCToolBar;
	if (!pNewToolbar->Create(pWndMain, AFX_DEFAULT_TOOLBAR_STYLE, uiID))
	{
		TRACE0("Failed to create a new toolbar!\n");
		delete pNewToolbar;
		return NULL;
	}

	pNewToolbar->SetWindowText(lpszName);

	int iCount = (int) pMenu->GetMenuItemCount();
	for (int i = 0; i < iCount; i ++)
	{
		UINT uiCmd = pMenu->GetMenuItemID(i);

		CString strText;
		pMenu->GetMenuString(i, strText, MF_BYPOSITION);

		switch (uiCmd)
		{
		case 0:
			if (i != iCount - 1)
			{
				pNewToolbar->InsertSeparator();
			}
			break;

		case -1:
			{
				UINT uiTearOffId = 0;
				if (g_pTearOffMenuManager != NULL)
				{
					uiTearOffId = g_pTearOffMenuManager->Parse(strText);
				}

				// Remove hotkey:
				int iTabOffset = strText.Find(_T('\t'));
				if (iTabOffset >= 0)
				{
					strText = strText.Left(iTabOffset);
				}

				CMFCToolBarMenuButton menuButton((UINT) -1, pMenu->GetSubMenu(i)->GetSafeHmenu(), -1, strText);
				if (menuButton.GetImage() == -1)
				{
					menuButton.m_bImage = FALSE;
					menuButton.m_bText = TRUE;
				}

				menuButton.SetTearOff(uiTearOffId);
				pNewToolbar->InsertButton(menuButton);
			}
			break;

		default:
			if (!IsStandardCommand(uiCmd))
			{
				CMFCToolBarButton button(uiCmd, -1, strText);
				if (button.GetImage() == -1)
				{
					button.m_bImage = FALSE;
					button.m_bText = TRUE;
				}

				pNewToolbar->InsertButton(button);
			}
		}
	}

	pNewToolbar->SetPaneStyle(pNewToolbar->GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	pNewToolbar->EnableDocking(CBRS_ALIGN_ANY);

	if (m_pParentBtn != NULL && m_pParentBtn->IsMenuPaletteMode())
	{
		int nColumns = iCount / m_pParentBtn->GetPaletteRows() + 1;
		int cx = pNewToolbar->GetColumnWidth() * nColumns;

		pNewToolbar->StretchPane(cx, FALSE);
	}

	return pNewToolbar;
}

BOOL CMFCPopupMenu::PreTranslateMessage(MSG* pMsg)
{
	if (m_wndToolTip.GetSafeHwnd() != NULL)
	{
		m_wndToolTip.RelayEvent(pMsg);
	}

	if (pMsg->message == WM_MOUSEMOVE &&
		(!m_rectScrollUp.IsRectEmpty() || !m_rectScrollDn.IsRectEmpty()))
	{
		CPoint ptCursor;
		::GetCursorPos(&ptCursor);
		ScreenToClient(&ptCursor);

		if (m_rectScrollUp.PtInRect(ptCursor) || m_rectScrollDn.PtInRect(ptCursor))
		{
			OnMouseMove((UINT) pMsg->wParam, ptCursor);
			return TRUE;
		}
	}

	return CMiniFrameWnd::PreTranslateMessage(pMsg);
}

BOOL CMFCPopupMenu::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR* pNMHDR = (NMHDR*) lParam;
	ENSURE(pNMHDR != NULL);

	if (pNMHDR->code == TTN_SHOW)
	{
		m_bIsTearOffCaptionActive = TRUE;
		InvalidateRect(m_rectTearOffCaption);
		UpdateWindow();
	}
	else if (pNMHDR->code == TTN_POP)
	{
		m_bIsTearOffCaptionActive = FALSE;
		InvalidateRect(m_rectTearOffCaption);
		UpdateWindow();
	}

	return CMiniFrameWnd::OnNotify(wParam, lParam, pResult);
}

void __stdcall CMFCPopupMenu::SetAnimationSpeed(UINT nElapse)
{
	if (nElapse == 0 || nElapse > 200)
	{
		ASSERT(FALSE);
		return;
	}

	m_AnimationSpeed = nElapse;
}

BOOL CMFCPopupMenu::NotifyParentDlg(BOOL bActivate)
{
	CDialogEx* pDlg = DYNAMIC_DOWNCAST(CDialogEx, m_pMessageWnd);
	CMFCPropertyPage* pPropPage = DYNAMIC_DOWNCAST(CMFCPropertyPage, m_pMessageWnd);

	if (pDlg == NULL && pPropPage == NULL)
	{
		return FALSE;
	}

	if (!bActivate && m_pActivePopupMenu != this)
	{
		return FALSE;
	}

	if (pDlg != NULL)
	{
		pDlg->SetActiveMenu(bActivate ? this : NULL);
	}

	if (pPropPage != NULL)
	{
		pPropPage->SetActiveMenu(bActivate ? this : NULL);
	}

	return TRUE;
}

void CMFCPopupMenu::UpdateShadow(LPRECT lprectScreen)
{
	ASSERT_VALID(this);

	if (m_iShadowSize <= 0)
	{
		// No menu shadow, nothing to do.
		return;
	}

	CWnd* pWndMain = GetTopLevelParent();
	if (pWndMain->GetSafeHwnd() == NULL)
	{
		return;
	}

	const BOOL bIsRTL = GetExStyle() & WS_EX_LAYOUTRTL;

	ASSERT_VALID(pWndMain);

	CRect rectClient;
	GetClientRect(rectClient);

	CRect rectUpdate1 = rectClient;
	CRect rectUpdate2 = rectClient;

	if (lprectScreen != NULL)
	{
		CRect rectRedraw = lprectScreen;
		ScreenToClient(&rectRedraw);

		CRect rectShadowRight = rectClient;
		if (bIsRTL)
		{
			rectShadowRight.right = rectShadowRight.left + m_iShadowSize + 1;
		}
		else
		{
			rectShadowRight.left = rectShadowRight.right - m_iShadowSize - 1;
		}

		if (!rectUpdate1.IntersectRect(rectRedraw, rectShadowRight))
		{
			rectUpdate1.SetRectEmpty();
		}

		CRect rectShadowBottom = rectClient;
		rectShadowBottom.top = rectShadowBottom.bottom - m_iShadowSize - 1;

		if (!rectUpdate2.IntersectRect(rectRedraw, rectShadowBottom))
		{
			rectUpdate2.SetRectEmpty();
		}

		if (rectUpdate1.IsRectEmpty() && rectUpdate2.IsRectEmpty())
		{
			return;
		}
	}

	CRect rectMenu;
	GetWindowRect(rectMenu);

	if (!m_bForceShadow)
	{
		CRect rectMain;
		pWndMain->GetWindowRect(rectMain);

		CRect rectInter;
		rectInter.UnionRect(&rectMenu, &rectMain);

		if (rectInter != rectMain)
		{
			return;
		}
	}

	int iShadowSize = m_iShadowSize;
	m_iShadowSize = 0;

	// m_bmpShadowRight and m_bmpShadowBottom contain the previous
	// screen shots. Delete them now:
	if (m_bmpShadowRight.GetSafeHandle() != NULL)
	{
		m_bmpShadowRight.DeleteObject();
	}

	if (m_bmpShadowBottom.GetSafeHandle() != NULL)
	{
		m_bmpShadowBottom.DeleteObject();
	}

	UINT uiSWPFlags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOSENDCHANGING;

	CRect rectWindow(-1, -1, -1, -1);

	CMFCPopupMenuBar* pMenuBar = GetMenuBar();
	ASSERT_VALID(pMenuBar);

	pMenuBar->m_bInUpdateShadow = TRUE;

	if (bIsRTL)
	{
		GetWindowRect(rectWindow);

		SetWindowPos(NULL, rectWindow.left + iShadowSize, rectWindow.top, rectClient.Width() - iShadowSize, rectClient.Height() - iShadowSize, uiSWPFlags);
	}
	else
	{
		uiSWPFlags |= SWP_NOMOVE;

		// Reduce menu size("cut" left and bottom shadows):
		SetWindowPos(NULL, -1, -1, rectClient.Width() - iShadowSize, rectClient.Height() - iShadowSize, uiSWPFlags);
	}

	UINT uiRDWFlags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN;

	CRect rectParent = rectUpdate1;

	if (!rectUpdate1.IsRectEmpty())
	{
		MapWindowPoints(pWndMain, &rectParent);
		pWndMain->RedrawWindow(rectParent, NULL, uiRDWFlags);
	}

	rectParent = rectUpdate2;

	if (!rectUpdate2.IsRectEmpty() && rectUpdate1 != rectUpdate2)
	{
		MapWindowPoints(pWndMain, &rectParent);
		pWndMain->RedrawWindow(rectParent, NULL, uiRDWFlags);
	}

	pWndMain->UpdateWindow();

	// Restore original size and update windows under the menu shadows:
	m_iShadowSize = iShadowSize;

	if (bIsRTL)
	{
		SetWindowPos(NULL, rectWindow.left, rectWindow.top, rectClient.Width(), rectClient.Height(), uiSWPFlags);
	}
	else
	{
		SetWindowPos(NULL, -1, -1, rectClient.Width(), rectClient.Height(), uiSWPFlags);
	}

	if (!rectUpdate1.IsRectEmpty())
	{
		InvalidateRect(rectUpdate1);
	}

	if (!rectUpdate2.IsRectEmpty() && rectUpdate1 != rectUpdate2)
	{
		InvalidateRect(rectUpdate2);
	}

	UpdateWindow();

	pMenuBar->m_bInUpdateShadow = FALSE;
}

void __stdcall CMFCPopupMenu::UpdateAllShadows(LPRECT lprectScreen)
{
	for (CMFCPopupMenu* pMenu = m_pActivePopupMenu; pMenu != NULL;
		pMenu = pMenu->GetParentPopupMenu())
	{
		if (CWnd::FromHandlePermanent(pMenu->GetSafeHwnd()) != NULL)
		{
			ASSERT_VALID(pMenu);
			pMenu->UpdateShadow(lprectScreen);
		}
	}
}

void CMFCPopupMenu::SetQuickMode()
{
	// Store recently used mode state
	CMFCCustomizeMenuButton::m_bRecentlyUsedOld = CMFCMenuBar::IsRecentlyUsedMenus();
	CMFCMenuBar::SetRecentlyUsedMenus(FALSE);

	m_bQuickCusomize = TRUE;
}

void CMFCPopupMenu::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CMiniFrameWnd::OnShowWindow(bShow, nStatus);

	if (!bShow)
	{
		m_bShown = FALSE;
	}
}

BOOL CMFCPopupMenu::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint ptCursor;
	::GetCursorPos(&ptCursor);
	ScreenToClient(&ptCursor);

	if (m_rectTearOffCaption.PtInRect(ptCursor))
	{
		if (afxGlobalData.m_hcurSizeAll == NULL)
		{
			afxGlobalData.m_hcurSizeAll = AfxGetApp()->LoadStandardCursor(IDC_SIZEALL);
		}

		SetCursor(afxGlobalData.m_hcurSizeAll);
		return TRUE;
	}

	return CMiniFrameWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CMFCPopupMenu::SetParentRibbonElement(CMFCRibbonBaseElement* pElem)
{
	ASSERT_VALID(this);

	m_pParentRibbonElement = pElem;
	pElem->m_pPopupMenu = this;
}

void CMFCPopupMenu::SetScrollBar()
{
	if (!m_bShowScrollBar || !m_bScrollable)
	{
		return;
	}

	CMFCPopupMenuBar* pMenuBar = GetMenuBar();
	ASSERT_VALID(pMenuBar);

	CRect rectClient;
	GetClientRect(rectClient);

	if (!CMFCToolBar::IsCustomizeMode())
	{
		rectClient.right -= m_iShadowSize;
		rectClient.bottom -= m_iShadowSize;
	}

	const int nBorderSize = GetBorderSize();
	rectClient.DeflateRect(nBorderSize, nBorderSize);

	switch (m_nLogoLocation)
	{
	case MENU_LOGO_LEFT:
		rectClient.left += m_iLogoWidth;
		break;

	case MENU_LOGO_RIGHT:
		rectClient.right -= m_iLogoWidth;
		break;

	case MENU_LOGO_TOP:
		rectClient.top += m_iLogoWidth;
		break;

	case MENU_LOGO_BOTTOM:
		rectClient.bottom -= m_iLogoWidth;
		break;
	}

	rectClient.top += m_rectTearOffCaption.Height();

	if (m_bIsResizeBarOnTop)
	{
		rectClient.top += m_rectResize.Height();
	}
	else
	{
		rectClient.bottom -= m_rectResize.Height();
	}

	const int cxScroll = ::GetSystemMetrics(SM_CXVSCROLL);

	CRect rectScrollBar = rectClient;
	rectScrollBar.left = rectScrollBar.right - cxScroll;
	rectClient.right -= cxScroll;

	SCROLLINFO si;

	ZeroMemory(&si, sizeof(SCROLLINFO));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;

	if (m_wndScrollBarVert.GetSafeHwnd() == NULL)
	{
		m_wndScrollBarVert.Create(WS_CHILD | WS_VISIBLE | SBS_VERT, rectScrollBar, this, nScrollBarID);
	}
	else
	{
		m_wndScrollBarVert.SetWindowPos(NULL, rectScrollBar.left, rectScrollBar.top, rectScrollBar.Width(), rectScrollBar.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
		m_wndScrollBarVert.GetScrollInfo(&si);
	}

	si.nMin = 0;
	si.nMax = 0;
	si.nPage = 0;

	const int nMenuRowHeight = pMenuBar->GetRowHeight();
	const int nMenuTotalItems = pMenuBar->GetCount();

	if (nMenuTotalItems > 0 && nMenuRowHeight > 0)
	{
		si.nMax = (nMenuTotalItems * nMenuRowHeight - m_nMenuBarHeight) / nMenuRowHeight + 1;
		si.nPage = /*m_nMenuBarHeight / nMenuRowHeight*/1;

		pMenuBar->m_nDropDownPageSize = m_nMenuBarHeight / nMenuRowHeight;
	}
	else
	{
		pMenuBar->m_nDropDownPageSize = 0;
	}

	m_wndScrollBarVert.SetScrollInfo(&si, TRUE);
	m_wndScrollBarVert.EnableScrollBar(si.nMax > 0 ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);
	m_wndScrollBarVert.EnableWindow();
}

void CMFCPopupMenu::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar->GetSafeHwnd() != m_wndScrollBarVert.GetSafeHwnd())
	{
		CMiniFrameWnd::OnVScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	CMFCPopupMenuBar* pMenuBar = GetMenuBar();
	ASSERT_VALID(pMenuBar);

	SCROLLINFO scrollInfo;
	ZeroMemory(&scrollInfo, sizeof(SCROLLINFO));

	scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_ALL;

	m_wndScrollBarVert.GetScrollInfo(&scrollInfo);

	int iOffset = pMenuBar->GetOffset();
	int nMaxOffset = scrollInfo.nMax;
	int nPage = scrollInfo.nPage;

	switch (nSBCode)
	{
	case SB_LINEUP:
		iOffset--;
		break;

	case SB_LINEDOWN:
		iOffset++;
		break;

	case SB_TOP:
		iOffset = 0;
		break;

	case SB_BOTTOM:
		iOffset = nMaxOffset;
		break;

	case SB_PAGEUP:
		iOffset -= nPage;
		break;

	case SB_PAGEDOWN:
		iOffset += nPage;
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		iOffset = nPos;
		break;

	default:
		return;
	}

	iOffset = min(max(0, iOffset), nMaxOffset);

	if (iOffset == pMenuBar->GetOffset())
	{
		return;
	}

	pMenuBar->SetOffset(iOffset);

	if (m_wndScrollBarVert.GetSafeHwnd() != NULL)
	{
		m_wndScrollBarVert.SetScrollPos(iOffset);
	}

	AdjustScroll();
}

BOOL CMFCPopupMenu::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/)
{
	ASSERT_VALID(this);

	if (GetActiveMenu() != this || !m_bScrollable)
	{
		return TRUE;
	}

	const int nSteps = abs(zDelta) / WHEEL_DELTA;

	for (int i = 0; i < nSteps; i++)
	{
		OnVScroll(zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0, &m_wndScrollBarVert);
	}

	return TRUE;
}

CWnd* CMFCPopupMenu::GetParentArea(CRect& rectParentBtn)
{
	ASSERT_VALID(this);

	if (m_pParentBtn != NULL)
	{
		ASSERT_VALID(m_pParentBtn);

		CWnd* pWndParent = m_pParentBtn->GetParentWnd();

		if (pWndParent != NULL)
		{
			rectParentBtn = m_pParentBtn->Rect();
			return pWndParent;
		}
	}
	else if (m_pParentRibbonElement != NULL)
	{
		ASSERT_VALID(m_pParentRibbonElement);

		CWnd* pWndParent = m_pParentRibbonElement->GetParentWnd();

		if (pWndParent != NULL)
		{
			rectParentBtn = m_pParentRibbonElement->GetRect();
			return pWndParent;
		}
	}

	return NULL;
}

int CMFCPopupMenu::GetBorderSize() const
{
	return CMFCVisualManager::GetInstance()->GetPopupMenuBorderSize();
}

LRESULT CMFCPopupMenu::OnNcHitTest(CPoint point)
{
	BOOL bRTL = GetExStyle() & WS_EX_LAYOUTRTL;

	// hit test the size box - convert to HTCAPTION if so
	if (!m_rectResize.IsRectEmpty())
	{
		CRect rect = m_rectResize;

		if (m_sizeMinResize.cx <= 0)
		{
			// Vertical resizing:
			ClientToScreen(&rect);

			if (rect.PtInRect(point))
			{
				return m_bIsResizeBarOnTop ? HTTOP : HTBOTTOM;
			}
		}
		else
		{
			rect.left = rect.right - rect.Height();

			ClientToScreen(&rect);

			if (rect.PtInRect(point))
			{
				if (m_bIsResizeBarOnTop)
				{
					return bRTL ? HTTOPLEFT : HTTOPRIGHT;
				}
				else
				{
					return bRTL ? HTBOTTOMLEFT : HTBOTTOMRIGHT;
				}
			}
		}
	}

	return CMiniFrameWnd::OnNcHitTest(point);
}

void CMFCPopupMenu::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	if (m_bIsResizeBarOnTop)
	{
		if (nHitTest == HTTOPLEFT || nHitTest == HTTOPRIGHT || nHitTest == HTTOP)
		{
			StartResize();
			return;
		}
	}
	else
	{
		if (nHitTest == HTBOTTOMLEFT || nHitTest == HTBOTTOMRIGHT || nHitTest == HTBOTTOM)
		{
			StartResize();
			return;
		}
	}

	CMiniFrameWnd::OnNcLButtonDown(nHitTest, point);
}

void CMFCPopupMenu::EnableResize(CSize sizeMinResize)
{
	m_bIsResizable = (sizeMinResize != CSize(0, 0));

	if (m_bIsResizable)
	{
		sizeMinResize.cy += sizeMinResize.cx > 0 ? nResizeBarBarHeightRight : nResizeBarBarHeight;
	}

	m_sizeMinResize = sizeMinResize;
}

void CMFCPopupMenu::EnableVertResize(int nMinResize)
{
	m_bIsResizable = nMinResize > 0;
	m_sizeMinResize = CSize(0, nMinResize);
}

BOOL CMFCPopupMenu::StartResize()
{
	ASSERT_VALID(this);

	HWND hwndMenu = GetSafeHwnd();

	// handle pending WM_PAINT messages
	MSG msgPaint;
	while (::PeekMessage(&msgPaint, NULL, WM_PAINT, WM_PAINT, PM_NOREMOVE))
	{
		if (!GetMessage(&msgPaint, NULL, WM_PAINT, WM_PAINT))
			return FALSE;
		DispatchMessage(&msgPaint);
	}

	// don't handle if capture already set
	if (::GetCapture() != NULL)
		return FALSE;

	// set capture to the window which received this message
	SetCapture();
	ASSERT(this == CWnd::GetCapture());

	BOOL bSuccess = FALSE;
	BOOL bStop = FALSE;

	CRect rectWindow;
	GetWindowRect(rectWindow);

	m_bResizeTracking = TRUE;
	GetMenuBar()->m_bResizeTracking = TRUE;

	int iShadowSize = m_iShadowSize;
	m_iShadowSize = 0;

	if (m_pWndShadow->GetSafeHwnd () != NULL)
	{
		m_pWndShadow->ShowWindow (SW_HIDE);
	}

	BOOL bIsMoseMove = FALSE;

	const int nBorderSize = GetBorderSize();

	// get messages until capture lost or cancelled/accepted
	while (!bStop && CWnd::GetCapture() == this)
	{
		MSG msg;
		if (!::GetMessage(&msg, NULL, 0, 0))
		{
			AfxPostQuitMessage((int) msg.wParam);
			break;
		}

		switch (msg.message)
		{
		case WM_LBUTTONUP:
			bStop = TRUE;
			bSuccess = TRUE;

			m_iShadowSize = iShadowSize;

			if (bIsMoseMove)
			{
				if (m_bmpShadowRight.GetSafeHandle() != NULL)
				{
					m_bmpShadowRight.DeleteObject();
				}

				if (m_bmpShadowBottom.GetSafeHandle() != NULL)
				{
					m_bmpShadowBottom.DeleteObject();
				}

				RecalcLayout();
				UpdateBottomWindows();

				if (m_pWndShadow->GetSafeHwnd () != NULL)
				{
					m_pWndShadow->Repos ();
				}

				ShowWindow(SW_SHOWNOACTIVATE);
			}

			break;

		case WM_MOUSEMOVE:
			{
				bIsMoseMove = TRUE;

				const BOOL bRTL = GetExStyle() & WS_EX_LAYOUTRTL;

				const int dx = (bRTL ? rectWindow.right - msg.pt.x : msg.pt.x - rectWindow.left) + iShadowSize + 2 * nBorderSize;

				CSize sizeNew = CSize(m_sizeMinResize.cx > 0 ? max(m_sizeMinResize.cx, dx) : rectWindow.Width() - iShadowSize,
					max(m_sizeMinResize.cy, m_bIsResizeBarOnTop ? rectWindow.bottom - msg.pt.y: msg.pt.y - rectWindow.top));

				if (sizeNew != m_sizeCurrent)
				{
					m_sizeCurrent = sizeNew;

					if (m_bIsResizeBarOnTop && sizeNew.cy > m_sizeMinResize.cy)
					{
						m_ptLocation.y = msg.pt.y - m_rectResize.Height() - GetBorderSize() - 1;
					}

					RecalcLayout();
					GetMenuBar()->AdjustLocations();

					if (m_wndScrollBarVert.GetSafeHwnd() != NULL)
					{
						OnVScroll(SB_THUMBTRACK, m_wndScrollBarVert.GetScrollPos(), &m_wndScrollBarVert);
					}

					RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
					UpdateBottomWindows();

					m_bWasResized = TRUE;
				}
			}
			break;

			// just dispatch rest of the messages
		default:
			DispatchMessage(&msg);
			break;
		}
	}

	ReleaseCapture();

	if (::IsWindow(hwndMenu))
	{
		m_bResizeTracking = FALSE;
		GetMenuBar()->m_bResizeTracking = FALSE;
	}

	return bSuccess;
}

CMFCPopupMenu* __stdcall CMFCPopupMenu::FindMenuWithConnectedFloaty()
{
	if (CMFCRibbonMiniToolBar::m_pCurrent == NULL)
	{
		return NULL;
	}

	for (CMFCPopupMenu* pMenu = m_pActivePopupMenu; pMenu != NULL; pMenu = pMenu->GetParentPopupMenu())
	{
		if (CWnd::FromHandlePermanent(pMenu->GetSafeHwnd()) != NULL)
		{
			ASSERT_VALID(pMenu);

			if (pMenu->m_hwndConnectedFloaty != NULL)
			{
				return pMenu;
			}
		}
	}
	return NULL;
}

void CMFCPopupMenu::TriggerResize()
{
	ASSERT_VALID(this);

	m_bResizeTracking = TRUE;
	GetMenuBar()->m_bResizeTracking = TRUE;

	GetMenuBar()->AdjustLocations();

	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
	UpdateBottomWindows();

	m_bResizeTracking = FALSE;
	GetMenuBar()->m_bResizeTracking = FALSE;
}

void CMFCPopupMenu::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	CMiniFrameWnd::OnWindowPosChanged(lpwndpos);

	if (m_pWndShadow->GetSafeHwnd () != NULL && !m_bResizeTracking)
	{
		if (lpwndpos->flags & SWP_HIDEWINDOW)
		{
			m_pWndShadow->ShowWindow (SW_HIDE);
		}
		else if ((lpwndpos->flags & (SWP_NOSIZE | SWP_NOMOVE)) == 0 || (lpwndpos->flags & SWP_SHOWWINDOW))
		{
			m_pWndShadow->Repos ();
		}
	}
}
