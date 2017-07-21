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
#include "multimon.h"

#include <afxpriv.h>
#include "mmsystem.h"
#include "afxdropdowntoolbar.h"
#include "afxglobals.h"
#include "afxtoolbarmenubutton.h"
#include "afxmdiframewndex.h"
#include "afxframewndex.h"
#include "afxmenubar.h"
#include "afxsound.h"
#include "afxtoolbarmenubutton.h"
#include "afxtrackmouse.h"
#include "afxvisualmanager.h"
#include "afxdrawmanager.h"
#include "afxribbonres.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const UINT uiShowBarTimerId = 1;
static const int nArrowSize = 7;

UINT CMFCDropDownToolbarButton::m_uiShowBarDelay = 500; // ms

IMPLEMENT_SERIAL(CMFCDropDownToolBar, CMFCToolBar, 1)

extern CObList afxAllToolBars;

BOOL CMFCDropDownToolBar::OnSendCommand(const CMFCToolBarButton* pButton)
{
	ASSERT_VALID(pButton);

	if ((pButton->m_nStyle & TBBS_DISABLED) != 0 || pButton->m_nID == 0 || pButton->m_nID == (UINT)-1)
	{
		return FALSE;
	}

	CMFCDropDownFrame* pParent = (CMFCDropDownFrame*)GetParent();
	ASSERT_KINDOF(CMFCDropDownFrame, pParent);

	pParent->m_pParentBtn->SetDefaultCommand(pButton->m_nID);

	// Send command to the parent frame:
	CFrameWnd* pParentFrame = GetParentFrame();
	ASSERT_VALID(pParentFrame);

	GetOwner()->PostMessage(WM_COMMAND, pButton->m_nID);
	pParentFrame->DestroyWindow();
	return TRUE;
}

void CMFCDropDownToolBar::OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
{
	CMFCToolBar::OnUpdateCmdUI((CFrameWnd*)GetCommandTarget(), bDisableIfNoHndler);
}

BEGIN_MESSAGE_MAP(CMFCDropDownToolBar, CMFCToolBar)
	//{{AFX_MSG_MAP(CMFCDropDownToolBar)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CMFCDropDownToolBar::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	if (m_ptLastMouse != CPoint(-1, -1) && abs(m_ptLastMouse.x - point.x) < 1 && abs(m_ptLastMouse.y - point.y) < 1)
	{
		m_ptLastMouse = point;
		return;
	}

	m_ptLastMouse = point;

	int iPrevHighlighted = m_iHighlighted;
	m_iHighlighted = HitTest(point);

	CMFCToolBarButton* pButton = m_iHighlighted == -1 ? NULL : GetButton(m_iHighlighted);
	if (pButton != NULL && (pButton->m_nStyle & TBBS_SEPARATOR || (pButton->m_nStyle & TBBS_DISABLED && !AllowSelectDisabled())))
	{
		m_iHighlighted = -1;
	}

	if (!m_bTracked)
	{
		m_bTracked = TRUE;

		TRACKMOUSEEVENT trackmouseevent;
		trackmouseevent.cbSize = sizeof(trackmouseevent);
		trackmouseevent.dwFlags = TME_LEAVE;
		trackmouseevent.hwndTrack = GetSafeHwnd();
		trackmouseevent.dwHoverTime = HOVER_DEFAULT;
		AFXTrackMouse(&trackmouseevent);
	}

	if (iPrevHighlighted != m_iHighlighted)
	{
		BOOL bNeedUpdate = FALSE;

		m_iButtonCapture = m_iHighlighted;
		if (iPrevHighlighted != -1)
		{
			CMFCToolBarButton* pTBBCapt = GetButton(iPrevHighlighted);
			ENSURE(pTBBCapt != NULL);
			ASSERT(!(pTBBCapt->m_nStyle & TBBS_SEPARATOR));

			UINT nNewStyle = (pTBBCapt->m_nStyle & ~TBBS_PRESSED);

			if (nNewStyle != pTBBCapt->m_nStyle)
			{
				SetButtonStyle(iPrevHighlighted, nNewStyle);
			}

		}

		if (m_iButtonCapture != -1)
		{
			CMFCToolBarButton* pTBBCapt = GetButton(m_iButtonCapture);
			ENSURE(pTBBCapt != NULL);
			ASSERT(!(pTBBCapt->m_nStyle & TBBS_SEPARATOR));

			UINT nNewStyle = (pTBBCapt->m_nStyle & ~TBBS_PRESSED);
			if (m_iHighlighted == m_iButtonCapture)
			{
				nNewStyle |= TBBS_PRESSED;
			}

			if (nNewStyle != pTBBCapt->m_nStyle)
			{
				SetButtonStyle(m_iButtonCapture, nNewStyle);
				bNeedUpdate = TRUE;
			}
		}

		if ((m_iButtonCapture == -1 || iPrevHighlighted == m_iButtonCapture) && iPrevHighlighted != -1)
		{
			InvalidateButton(iPrevHighlighted);
			bNeedUpdate = TRUE;
		}

		if ((m_iButtonCapture == -1 || m_iHighlighted == m_iButtonCapture) && m_iHighlighted != -1)
		{
			InvalidateButton(m_iHighlighted);
			bNeedUpdate = TRUE;
		}

		if (bNeedUpdate)
		{
			UpdateWindow();
		}

		if (m_iHighlighted != -1 && (m_iHighlighted == m_iButtonCapture || m_iButtonCapture == -1))
		{
			ENSURE(pButton != NULL);
			ShowCommandMessageString(pButton->m_nID);
		}
		else if (m_iButtonCapture == -1 && m_hookMouseHelp == NULL)
		{
			GetOwner()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
		}

		OnChangeHot(m_iHighlighted);
	}
}

void CMFCDropDownToolBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	CRect rectClient;
	GetClientRect(&rectClient);

	if (!m_bCustomizeMode && !rectClient.PtInRect(point))
	{
		CFrameWnd* pParentFrame = GetParentFrame();
		ASSERT_VALID(pParentFrame);

		pParentFrame->DestroyWindow();
		return;
	}

	if (!m_bCustomizeMode && m_iHighlighted >= 0)
	{
		m_iButtonCapture = m_iHighlighted;

		CMFCToolBarButton* pButton = GetButton(m_iHighlighted);
		ASSERT_VALID(pButton);

		pButton->m_nStyle &= ~TBBS_PRESSED;
	}

	CMFCToolBar::OnLButtonUp(nFlags, point);
}

/////////////////////////////////////////////////////////////////////////////
// CMFCDropDownFrame

static const int nBorderSize = 2;

CString CMFCDropDownFrame::m_strClassName;

IMPLEMENT_SERIAL(CMFCDropDownFrame, CMiniFrameWnd, VERSIONABLE_SCHEMA | 1)

CMFCDropDownFrame::CMFCDropDownFrame()
{
	m_x = m_y = 0;
	m_pParentBtn = NULL;
	m_bAutoDestroyParent = TRUE;
	m_bAutoDestroy = TRUE;
	m_pWndOriginToolbar = NULL;
}

CMFCDropDownFrame::~CMFCDropDownFrame()
{
	m_wndToolBar.m_Buttons.RemoveAll(); // toolbar has references to original buttons!

	if (m_bAutoDestroy)
	{
		m_wndToolBar.DestroyWindow();
	}
}

BEGIN_MESSAGE_MAP(CMFCDropDownFrame, CMiniFrameWnd)
	//{{AFX_MSG_MAP(CMFCDropDownFrame)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_MOUSEACTIVATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
	ON_WM_ACTIVATEAPP()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCDropDownFrame message handlers

BOOL CMFCDropDownFrame::Create(CWnd* pWndParent, int x, int y, CMFCDropDownToolBar* pWndOriginToolbar)
{
	ASSERT_VALID(pWndOriginToolbar);
	ENSURE(pWndParent != NULL);

	AFXPlaySystemSound(AFX_SOUND_MENU_POPUP);

	if (m_strClassName.IsEmpty())
	{
		m_strClassName = ::AfxRegisterWndClass(CS_SAVEBITS, ::LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_BTNFACE + 1), NULL);
	}

	m_pWndOriginToolbar = pWndOriginToolbar;

	if (x == -1 && y == -1) // Undefined position
	{
		if (pWndParent != NULL)
		{
			CRect rectParent;
			pWndParent->GetClientRect(&rectParent);
			pWndParent->ClientToScreen(&rectParent);

			m_x = rectParent.left + 5;
			m_y = rectParent.top + 5;
		}
		else
		{
			m_x = 0;
			m_y = 0;
		}
	}
	else
	{
		m_x = x;
		m_y = y;
	}

	DWORD dwStyle = WS_POPUP;

	CRect rect(x, y, x, y);
	BOOL bCreated = CMiniFrameWnd::CreateEx(0, m_strClassName, m_strCaption, dwStyle, rect, pWndParent->GetOwner() == NULL ? pWndParent : pWndParent->GetOwner());
	if (!bCreated)
	{
		return FALSE;
	}

	ShowWindow(SW_SHOWNOACTIVATE);
	return TRUE;
}

int CMFCDropDownFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	ASSERT_VALID(m_pWndOriginToolbar);
	ASSERT(m_pWndOriginToolbar->m_bLocked);

	if (CMiniFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CMFCToolBar* pParentBar = m_pParentBtn == NULL ? NULL : DYNAMIC_DOWNCAST(CMFCToolBar, m_pParentBtn->m_pWndParent);

	BOOL bHorz = pParentBar == NULL ? TRUE : pParentBar->IsHorizontal();
	DWORD style = bHorz? CBRS_ORIENT_VERT : CBRS_ORIENT_HORZ;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | style, CRect(1, 1, 1, 1), AFX_IDW_TOOLBAR + 39))
	{
		TRACE(_T("Can't create toolbar bar\n"));
		return -1;
	}

	m_wndToolBar.m_bLocked = TRUE;

	// "Clone" the original toolbar:
	m_pWndOriginToolbar->m_ImagesLocked.CopyTemp(m_wndToolBar.m_ImagesLocked);
	m_pWndOriginToolbar->m_ColdImagesLocked.CopyTemp(m_wndToolBar.m_ColdImagesLocked);
	m_pWndOriginToolbar->m_DisabledImagesLocked.CopyTemp(m_wndToolBar.m_DisabledImagesLocked);

	m_wndToolBar.m_sizeButtonLocked = m_pWndOriginToolbar->m_sizeButtonLocked;
	m_wndToolBar.m_sizeImageLocked = m_pWndOriginToolbar->m_sizeImageLocked;
	m_wndToolBar.m_sizeCurButtonLocked = m_pWndOriginToolbar->m_sizeCurButtonLocked;
	m_wndToolBar.m_sizeCurImageLocked = m_pWndOriginToolbar->m_sizeCurImageLocked;

	m_wndToolBar.m_dwStyle &= ~CBRS_GRIPPER;

	m_wndToolBar.SetOwner(m_pWndOriginToolbar->GetOwner());
	m_wndToolBar.SetRouteCommandsViaFrame(m_pWndOriginToolbar->GetRouteCommandsViaFrame());

	m_wndToolBar.m_Buttons.AddTail(&m_pWndOriginToolbar->m_Buttons);

	RecalcLayout();
	::ReleaseCapture();
	m_wndToolBar.SetCapture();

	return 0;
}

void CMFCDropDownFrame::OnSize(UINT nType, int cx, int cy)
{
	CMiniFrameWnd::OnSize(nType, cx, cy);

	if (m_wndToolBar.GetSafeHwnd() != NULL)
	{
		m_wndToolBar.SetWindowPos(NULL, nBorderSize, nBorderSize, cx - nBorderSize * 2, cy - nBorderSize * 2, SWP_NOZORDER | SWP_NOACTIVATE);
	}
}

void CMFCDropDownFrame::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectClient; // Client area rectangle
	GetClientRect(&rectClient);

	dc.Draw3dRect(rectClient, afxGlobalData.clrBarLight, afxGlobalData.clrBarDkShadow);
	rectClient.DeflateRect(1, 1);
	dc.Draw3dRect(rectClient, afxGlobalData.clrBarHilite, afxGlobalData.clrBarShadow);
}

int CMFCDropDownFrame::OnMouseActivate(CWnd* /*pDesktopWnd*/, UINT /*nHitTest*/, UINT /*message*/)
{
	return MA_NOACTIVATE;
}

void CMFCDropDownFrame::RecalcLayout(BOOL /*bNotify*/)
{
#ifdef _DEBUG
	if (m_pParentBtn != NULL)
	{
		ASSERT_VALID(m_pParentBtn);
		ASSERT(m_pParentBtn->m_pPopupMenu == this);
	}
#endif // _DEBUG

	if (!::IsWindow(m_hWnd) || !::IsWindow(m_wndToolBar.m_hWnd))
	{
		return;
	}

	CMFCToolBar* pParentBar = m_pParentBtn == NULL ? NULL : DYNAMIC_DOWNCAST(CMFCToolBar, m_pParentBtn->m_pWndParent);

	BOOL bHorz = pParentBar == NULL ? TRUE : pParentBar->IsHorizontal();

	CSize size = m_wndToolBar.CalcSize(bHorz);
	size.cx += nBorderSize * 3;
	size.cy += nBorderSize * 3;

	// Adjust the menu position by the screen size:
	CRect rectScreen;

	MONITORINFO mi;
	mi.cbSize = sizeof(MONITORINFO);
	if (GetMonitorInfo(MonitorFromPoint(CPoint(m_x, m_y), MONITOR_DEFAULTTONEAREST), &mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		::SystemParametersInfo(SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	if (m_x + size.cx > rectScreen.right)
	{
		// Menu can't be overlapped with the parent popup menu!
		pParentBar = m_pParentBtn == NULL ? NULL : DYNAMIC_DOWNCAST(CMFCToolBar, m_pParentBtn->m_pWndParent);
		if (pParentBar != NULL && (pParentBar->IsHorizontal()) == 0)
		{
			// Parent menu bar is docked vertical, place menu
			// in the left or right side of the parent frame:
			CRect rectParent;
			pParentBar->GetWindowRect(rectParent);

			m_x = rectParent.left - size.cx;
		}
		else
		{
			m_x = rectScreen.Width() - size.cx - 1;
		}
	}

	if (m_y + size.cy > rectScreen.bottom)
	{
		m_y -= size.cy;

		if (m_pParentBtn != NULL)
		{
			m_y -= m_pParentBtn->m_rect.Height() + 4;
		}
		else if (m_y < 0)
		{
			m_y = 0;
		}
	}

	SetWindowPos(NULL, m_x, m_y, size.cx, size.cy, SWP_NOZORDER | SWP_NOACTIVATE);
}

void CMFCDropDownFrame::OnDestroy()
{
	if (m_pParentBtn != NULL)
	{
		ASSERT(m_pParentBtn->m_pPopupMenu == this);

		m_pParentBtn->m_pPopupMenu = NULL;
		m_pParentBtn->m_nStyle = m_pParentBtn->m_nStyle & ~TBBS_PRESSED;

		CMFCToolBar* pparentBar = DYNAMIC_DOWNCAST(CMFCToolBar, m_pParentBtn->m_pWndParent);
		if (pparentBar)
		{
			CPoint point;
			::GetCursorPos(&point);

			pparentBar->ScreenToClient(&point);
			pparentBar->SendMessage(WM_LBUTTONUP, NULL, MAKELONG(point.x, point.y));
		}
	}

	CMiniFrameWnd::OnDestroy();
}

void CMFCDropDownFrame::PostNcDestroy()
{
	if (m_pParentBtn != NULL)
	{
		m_pParentBtn->OnCancelMode();
	}

	CMiniFrameWnd::PostNcDestroy();
}

CMFCDropDownFrame* CMFCDropDownFrame::GetParentPopupMenu() const
{
	if (m_pParentBtn == NULL)
	{
		return NULL;
	}

	CMFCPopupMenuBar* pParentBar = DYNAMIC_DOWNCAST(CMFCPopupMenuBar, m_pParentBtn->m_pWndParent);
	if (pParentBar != NULL)
	{
		CMFCDropDownFrame* pParentMenu = DYNAMIC_DOWNCAST(CMFCDropDownFrame, pParentBar->GetParentFrame());
		ASSERT_VALID(pParentMenu);

		return pParentMenu;
	}
	else
	{
		return NULL;
	}
}

CMFCMenuBar* CMFCDropDownFrame::GetParentMenuBar() const
{
	if (m_pParentBtn == NULL)
	{
		return NULL;
	}

	CMFCMenuBar* pParentBar = DYNAMIC_DOWNCAST(CMFCMenuBar, m_pParentBtn->m_pWndParent);
	return pParentBar;
}

BOOL CMFCDropDownFrame::OnEraseBkgnd(CDC* pDC)
{
	CRect rectClient; // Client area rectangle
	GetClientRect(&rectClient);

	pDC->FillSolidRect(rectClient, afxGlobalData.clrBarFace);
	return TRUE;
}

void CMFCDropDownFrame::OnActivateApp(BOOL bActive, DWORD /*dwThreadID*/)
{
	if (!bActive && !CMFCToolBar::IsCustomizeMode())
	{
		SendMessage(WM_CLOSE);
	}
}

IMPLEMENT_SERIAL(CMFCDropDownToolbarButton, CMFCToolBarButton, VERSIONABLE_SCHEMA | 1)

// Construction/Destruction
CMFCDropDownToolbarButton::CMFCDropDownToolbarButton()
{
	m_pToolBar = NULL;
	m_pPopupMenu = NULL;
	m_pWndParent = NULL;
	m_uiTimer = 0;
	m_bLocked = TRUE;
	m_iSelectedImage = 0;
	m_bInternalDraw = FALSE;
	m_bLocalUserButton = FALSE;
}

CMFCDropDownToolbarButton::CMFCDropDownToolbarButton(LPCTSTR lpszName, CMFCDropDownToolBar* pToolBar)
{
	ENSURE(lpszName != NULL);
	m_strName = lpszName;

	m_uiTimer = 0;

	m_pPopupMenu = NULL;
	m_pWndParent = NULL;

	ASSERT_VALID(pToolBar);
	m_pToolBar = pToolBar;

	CMFCToolBarButton* pbutton = pToolBar->GetButton(0);
	if (pbutton == NULL) // Toolbar is empty!
	{
		ASSERT(FALSE);
	}
	else
	{
		CMFCToolBarButton::CopyFrom(*pbutton);
	}

	m_iSelectedImage = 0;

	m_bLocalUserButton = FALSE;
}

CMFCDropDownToolbarButton::~CMFCDropDownToolbarButton()
{
}

void CMFCDropDownToolbarButton::SetDefaultCommand(UINT uiCmd)
{
	ASSERT_VALID(m_pToolBar);

	m_nID = uiCmd;

	// Find image index:
	int iImage = 0;
	m_iSelectedImage = -1;

	for (int i = 0; i < m_pToolBar->GetCount(); i ++)
	{
		CMFCToolBarButton* pButton = m_pToolBar->GetButton(i);
		ASSERT_VALID(pButton);

		if (pButton->m_nStyle & TBBS_SEPARATOR)
		{
			continue;
		}

		if (pButton->m_nID == uiCmd)
		{
			m_bLocalUserButton = pButton->m_bUserButton;

			if (m_bLocalUserButton)
			{
				m_iSelectedImage = pButton->GetImage();
			}
			else
			{
				m_iSelectedImage = iImage;
			}
			break;
		}

		iImage ++;
	}

	if (m_iSelectedImage == -1)
	{
		ASSERT(FALSE);
		m_iSelectedImage = 0;
	}
}

//////////////////////////////////////////////////////////////////////
// Overrides:

void CMFCDropDownToolbarButton::CopyFrom(const CMFCToolBarButton& s)
{
	CMFCToolBarButton::CopyFrom(s);

	const CMFCDropDownToolbarButton& src = (const CMFCDropDownToolbarButton&) s;

	m_pToolBar = src.m_pToolBar;
	m_strName = src.m_strName;
	m_iSelectedImage = src.m_iSelectedImage;

	m_bDragFromCollection = FALSE;
}

void CMFCDropDownToolbarButton::Serialize(CArchive& ar)
{
	CMFCToolBarButton::Serialize(ar);

	UINT uiToolbarResID = 0;

	if (ar.IsLoading())
	{
		m_pToolBar = NULL;

		ar >> uiToolbarResID;
		ar >> m_strName;
		ar >> m_iSelectedImage;

		// Find toolbar with required resource ID:
		for (POSITION pos = afxAllToolBars.GetHeadPosition(); pos != NULL;)
		{
			CMFCDropDownToolBar* pToolBar = DYNAMIC_DOWNCAST(CMFCDropDownToolBar, afxAllToolBars.GetNext(pos));

			if (pToolBar != NULL && CWnd::FromHandlePermanent(pToolBar->m_hWnd) != NULL)
			{
				ASSERT_VALID(pToolBar);
				if (pToolBar->m_uiOriginalResID == uiToolbarResID)
				{
					m_pToolBar = pToolBar;
					break;
				}
			}
		}

		SetDefaultCommand(m_nID);
	}
	else
	{
		if (m_pToolBar == NULL)
		{
			ASSERT(FALSE);
		}
		else
		{
			ASSERT_VALID(m_pToolBar);
			uiToolbarResID = m_pToolBar->m_uiOriginalResID;
		}

		ar << uiToolbarResID;
		ar << m_strName;
		ar << m_iSelectedImage;
	}
}

void CMFCDropDownToolbarButton::OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* /*pImages*/, BOOL bHorz, BOOL bCustomizeMode, BOOL bHighlight, BOOL bDrawBorder, BOOL bGrayDisabledButtons)
{

	ASSERT_VALID(pDC);

	// Fill button interior:
	FillInterior(pDC, rect, bHighlight);

	int nActualArrowSize = CMFCToolBar::IsLargeIcons() ? nArrowSize * 2 : nArrowSize;
	int nHalfArrowSize = CMFCToolBar::IsLargeIcons() ? nArrowSize : nArrowSize / 2 + 1;

	CRect rectParent = rect;
	rectParent.right -= nActualArrowSize / 2 + 1;

	if (m_pToolBar != NULL)
	{
		CAfxDrawState ds;

		BOOL bImage = m_bImage;
		m_bInternalDraw = TRUE;

		if (!m_bLocalUserButton)
		{
			m_pToolBar->m_ImagesLocked.SetTransparentColor(afxGlobalData.clrBtnFace);
			m_pToolBar->m_ImagesLocked.PrepareDrawImage(ds, m_pToolBar->GetImageSize());
		}
		else
		{
			m_pToolBar->m_pUserImages->SetTransparentColor(afxGlobalData.clrBtnFace);
			m_pToolBar->m_pUserImages->PrepareDrawImage(ds, m_pToolBar->GetImageSize());
		}

		m_iImage = m_iSelectedImage;
		m_iUserImage = m_iSelectedImage;
		m_bImage = TRUE;

		BOOL bDisableFill = m_bDisableFill;
		m_bDisableFill = TRUE;

		if (m_bLocalUserButton)
		{
			m_bUserButton = m_bLocalUserButton;
			CMFCToolBarButton::OnDraw(pDC, rect, m_pToolBar->m_pUserImages, bHorz,  bCustomizeMode, bHighlight,  FALSE, bGrayDisabledButtons);
			m_bUserButton = FALSE;
		}
		else
		{
			CMFCToolBarButton::OnDraw(pDC, rectParent, &m_pToolBar->m_ImagesLocked, bHorz, bCustomizeMode, bHighlight, FALSE, bGrayDisabledButtons);
		}
		m_bDisableFill = bDisableFill;
		m_iImage = -1;
		m_iUserImage = -1;
		m_bImage = bImage;

		if (!m_bLocalUserButton)
		{
			m_pToolBar->m_ImagesLocked.EndDrawImage(ds);
		}
		else
		{
			m_pToolBar->m_pUserImages->EndDrawImage(ds);
		}
		m_bInternalDraw = FALSE;
	}

	int offset = (m_nStyle & TBBS_PRESSED) ? 1 : 0;

	CPoint triang [] =
	{
		CPoint(rect.right - nActualArrowSize + offset - 1, rect.bottom - nHalfArrowSize + offset + 1),
		CPoint(rect.right - nHalfArrowSize + offset + 1, rect.bottom - nHalfArrowSize + offset + 1),
		CPoint(rect.right - nHalfArrowSize + offset + 1, rect.bottom - nActualArrowSize + offset - 1)
	};

	CPen* pOldPen = (CPen*) pDC->SelectStockObject(NULL_PEN);
	ENSURE(pOldPen != NULL);

	CBrush* pOldBrush = (CBrush*) pDC->SelectObject(&afxGlobalData.brBlack);
	ENSURE(pOldBrush != NULL);

	pDC->Polygon(triang, 3);

	if (!bCustomizeMode && HaveHotBorder() && bDrawBorder)
	{
		if (m_pPopupMenu != NULL || (m_nStyle &(TBBS_PRESSED | TBBS_CHECKED)))
		{
			// Pressed in or checked:
			if (m_nID != 0 && m_nID != (UINT) -1)
			{
				CMFCVisualManager::GetInstance()->OnDrawButtonBorder(pDC, this, rect, CMFCVisualManager::ButtonsIsPressed);
			}
			else
			{
				CMFCVisualManager::GetInstance()->OnDrawButtonBorder(pDC, this, rect, CMFCVisualManager::ButtonsIsHighlighted);
			}
		}
		else if (bHighlight && !(m_nStyle & TBBS_DISABLED) && !(m_nStyle &(TBBS_CHECKED | TBBS_INDETERMINATE)))
		{
			if (m_nStyle & TBBS_PRESSED)
			{
				CMFCVisualManager::GetInstance()->OnDrawButtonBorder(pDC, this, rect, CMFCVisualManager::ButtonsIsPressed);
			}
			else
			{
				CMFCVisualManager::GetInstance()->OnDrawButtonBorder(pDC, this, rect, CMFCVisualManager::ButtonsIsHighlighted);
			}
		}
	}

	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);
}

static CMFCDropDownToolbarButton* g_pButtonDown = NULL;

static void CALLBACK EXPORT TimerProc(HWND hWnd, UINT, UINT_PTR, DWORD)
{
	CWnd* pwnd = CWnd::FromHandle(hWnd);
	if (g_pButtonDown != NULL)
	{
		g_pButtonDown->OnClick(pwnd, FALSE);
	}
}

BOOL CMFCDropDownToolbarButton::OnClick(CWnd* pWnd, BOOL bDelay)
{
	ASSERT_VALID(pWnd);
	if (m_uiTimer == 0)
	{
		if (m_pWndParent != NULL)
		{
			m_uiTimer = (UINT) m_pWndParent->SetTimer(uiShowBarTimerId, m_uiShowBarDelay, TimerProc);
		}

		g_pButtonDown = this;
		return CMFCToolBarButton::OnClick(pWnd, bDelay);
	}

	if (m_pWndParent != NULL)
	{
		m_pWndParent->KillTimer(m_uiTimer);
	}

	m_uiTimer = 0;
	g_pButtonDown = NULL;

	CMFCMenuBar* pMenuBar = DYNAMIC_DOWNCAST(CMFCMenuBar, m_pWndParent);

	if (m_pPopupMenu != NULL)
	{
		// Second click to the popup menu item closes the menu:
		ASSERT_VALID(m_pPopupMenu);

		m_pPopupMenu->m_bAutoDestroyParent = FALSE;
		m_pPopupMenu->DestroyWindow();
		m_pPopupMenu = NULL;

		if (pMenuBar != NULL)
		{
			pMenuBar->SetHot(NULL);
		}
	}
	else
	{
		CMFCPopupMenuBar* pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenuBar, m_pWndParent);
		if (bDelay && pParentMenu != NULL && !CMFCToolBar::IsCustomizeMode())
		{
		}
		else
		{
			DropDownToolbar(pWnd);
		}

		if (pMenuBar != NULL)
		{
			pMenuBar->SetHot(this);
		}
	}

	if (m_pWndParent != NULL)
	{
		m_pWndParent->InvalidateRect(m_rect);
	}

	return FALSE;
}

BOOL CMFCDropDownToolbarButton::OnClickUp()
{
	CMFCMenuBar* pMenuBar = DYNAMIC_DOWNCAST(CMFCMenuBar, m_pWndParent);

	if (m_uiTimer)
	{
		if (m_pWndParent != NULL)
		{
			m_pWndParent->KillTimer(m_uiTimer);
		}

		m_uiTimer = 0;
		g_pButtonDown = NULL;
		return FALSE;
	}

	if (m_pPopupMenu != NULL)
	{
		// Second click to the popup menu item closes the menu:
		ASSERT_VALID(m_pPopupMenu);

		m_pPopupMenu->m_bAutoDestroyParent = FALSE;
		m_pPopupMenu->DestroyWindow();
		m_pPopupMenu = NULL;

		if (pMenuBar != NULL)
		{
			pMenuBar->SetHot(NULL);
		}
	}

	return TRUE;
}

void CMFCDropDownToolbarButton::OnChangeParentWnd(CWnd* pWndParent)
{
	CMFCToolBarButton::OnChangeParentWnd(pWndParent);

	m_bText = FALSE;
	m_strText.Empty();
	m_bUserButton = FALSE;
}

void CMFCDropDownToolbarButton::OnCancelMode()
{
	if (m_pWndParent != NULL && ::IsWindow(m_pWndParent->m_hWnd))
	{
		m_pWndParent->InvalidateRect(m_rect);
		m_pWndParent->UpdateWindow();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMFCDropDownToolbarButton diagnostics

#ifdef _DEBUG
void CMFCDropDownToolbarButton::AssertValid() const
{
	CObject::AssertValid();
}

void CMFCDropDownToolbarButton::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}

#endif

BOOL CMFCDropDownToolbarButton::DropDownToolbar(CWnd* pWnd)
{
	if (m_pToolBar == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (m_pPopupMenu != NULL)
	{
		return FALSE;
	}

	if (pWnd == NULL)
	{
		pWnd = m_pWndParent;
		if (m_pWndParent == NULL)
		{
			return FALSE;
		}
	}

	// Define a new menu position. Place the menu in the right side
	// of the current menu in the poup menu case or under the current
	// item by default:
	CPoint point;

	CMFCToolBar* pParentBar = DYNAMIC_DOWNCAST(CMFCToolBar, m_pWndParent);

	if (pParentBar != NULL && !pParentBar->IsHorizontal())
	{
		// Parent menu bar is docked vertical, place menu
		// in the left or right side of the parent frame:
		point = CPoint(m_rect.right + 1, m_rect.top);
		pWnd->ClientToScreen(&point);
	}
	else
	{
		point = CPoint(m_rect.left - 1, m_rect.bottom);
		pWnd->ClientToScreen(&point);
	}

	m_pPopupMenu = new CMFCDropDownFrame;
	m_pPopupMenu->m_pParentBtn = this;

	return m_pPopupMenu->Create(pWnd, point.x, point.y, m_pToolBar);
}

SIZE CMFCDropDownToolbarButton::OnCalculateSize(CDC* pDC, const CSize& sizeDefault, BOOL bHorz)
{
	if (m_nID == 0 && m_pToolBar != NULL)
	{
		ASSERT_VALID(m_pToolBar);

		CMFCToolBarButton* pButton = m_pToolBar->GetButton(0);
		if (pButton == NULL) // Toolbar is empty!
		{
			ASSERT(FALSE);
		}
		else
		{
			SetDefaultCommand(pButton->m_nID);
		}
	}

	BOOL bImage = m_bImage;

	m_iImage = m_iSelectedImage;
	m_bImage = TRUE;

	CSize sizeBtn = CMFCToolBarButton::OnCalculateSize(pDC, sizeDefault, bHorz);

	m_iImage = -1;
	m_bImage = bImage;

	int nArrowWidth = CMFCToolBar::IsLargeIcons() ? nArrowSize + 2 : nArrowSize / 2 + 1;
	sizeBtn.cx += nArrowWidth;

	return sizeBtn;
}

BOOL CMFCDropDownToolbarButton::ExportToMenuButton(CMFCToolBarMenuButton& menuButton) const
{
	if (m_pToolBar == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (!CMFCToolBarButton::ExportToMenuButton(menuButton))
	{
		return FALSE;
	}

	// Create a popup menu with all items:
	CMenu menu;
	menu.CreatePopupMenu();

	for (POSITION pos = m_pToolBar->m_Buttons.GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarButton* pButton = (CMFCToolBarButton*) m_pToolBar->m_Buttons.GetNext(pos);
		ENSURE(pButton != NULL);

		if (pButton->m_nStyle & TBBS_SEPARATOR)
		{
			menu.AppendMenu(MF_SEPARATOR);
		}
		else if (pButton->m_nID != 0 && pButton->m_nID != (UINT) -1)// Ignore sub-menus
		{
			CString strItem = pButton->m_strText;
			if (strItem.IsEmpty())
			{
				CString strMessage;
				int iOffset;

				if (strMessage.LoadString(pButton->m_nID) && (iOffset = strMessage.Find(_T('\n'))) != -1)
				{
					strItem = strMessage.Mid(iOffset + 1);
				}
			}

			menu.AppendMenu(MF_STRING, pButton->m_nID, strItem);
		}
	}

	menuButton.m_nID = 0;
	menuButton.m_strText = m_strName;
	menuButton.SetImage(-1);
	menuButton.m_bImage = FALSE;
	menuButton.CreateFromMenu(menu);

	menu.DestroyMenu();
	return TRUE;
}

int CMFCDropDownToolbarButton::OnDrawOnCustomizeList(CDC* pDC, const CRect& rect, BOOL bSelected)
{
	CString strText = m_strText;
	m_strText = m_strName;

	int iResult = CMFCToolBarButton::OnDrawOnCustomizeList(pDC, rect, bSelected);

	m_strText = strText;
	return iResult;
}

BOOL CMFCDropDownToolbarButton::OnCustomizeMenu(CMenu* pPopup)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pPopup);

	pPopup->EnableMenuItem(ID_AFXBARRES_TOOLBAR_IMAGE, MF_GRAYED | MF_BYCOMMAND);
	pPopup->EnableMenuItem(ID_AFXBARRES_TOOLBAR_TEXT, MF_GRAYED | MF_BYCOMMAND);
	pPopup->EnableMenuItem(ID_AFXBARRES_TOOLBAR_IMAGE_AND_TEXT, MF_GRAYED | MF_BYCOMMAND);
	pPopup->EnableMenuItem(ID_AFXBARRES_TOOLBAR_APPEARANCE, MF_GRAYED | MF_BYCOMMAND);
	pPopup->EnableMenuItem(ID_AFXBARRES_COPY_IMAGE, MF_GRAYED | MF_BYCOMMAND);

	return TRUE;
}


