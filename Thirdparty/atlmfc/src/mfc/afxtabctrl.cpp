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

#include "afxglobals.h"
#include "afxtabctrl.h"

// detachable bars support
#include "afxdockablepane.h"
#include "afxvisualmanager.h"
#include "afxtabbedpane.h"
#include "afxtoolbarbutton.h"
#include "afxpaneframewnd.h"

#include "afxribbonres.h"

#include "afxmdiframewndex.h"
#include "afxcontextmenumanager.h"

#include "afxmdiclientareawnd.h"
#include "afxtooltipmanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

UINT AFX_WM_ON_HSCROLL = ::RegisterWindowMessage(_T("AFX_WM_ON_HSCROLL"));
UINT AFX_WM_GETDRAGBOUNDS = ::RegisterWindowMessage(_T("AFX_WM_GETDRAGBOUNDS"));
UINT AFX_WM_ON_DRAGCOMPLETE = ::RegisterWindowMessage(_T("AFX_WM_ON_DRAGCOMPLETE"));
UINT AFX_WM_ON_TABGROUPMOUSEMOVE  = ::RegisterWindowMessage(_T("AFX_WM_ON_TABGROUPMOUSEMOVE"));
UINT AFX_WM_ON_CANCELTABMOVE = ::RegisterWindowMessage(_T("AFX_WM_ON_CANCELTABMOVE"));
UINT AFX_WM_ON_MOVETABCOMPLETE = ::RegisterWindowMessage(_T("AFX_WM_ON_MOVETABCOMPLETE"));

BOOL CMFCTabCtrl::m_bEnableActivate = TRUE;
CMap<UINT,UINT,HICON,HICON> CMFCTabCtrl::m_mapDocIcons;

/////////////////////////////////////////////////////////////////////////////
// CMFCTabCtrl

IMPLEMENT_DYNCREATE(CMFCTabCtrl, CMFCBaseTabCtrl)

#define AFX_MIN_SCROLL_WIDTH  (::GetSystemMetrics(SM_CXHSCROLL) * 2)
#define AFX_SPLITTER_WIDTH    5
#define AFX_RESIZEBAR_SIZE    6
#define AFX_TABS_FONT         _T("Arial")

CMFCTabCtrl::CMFCTabCtrl()
{
	m_iTabsNum = 0;
	m_iActiveTab = -1;

	m_bFlat = FALSE;
	m_bIsOneNoteStyle = FALSE;
	m_bIsVS2005Style = FALSE;
	m_bLeftRightRounded = FALSE;
	m_bScroll = FALSE;
	m_bCloseBtn = FALSE;
	m_bSharedScroll = FALSE;
	m_rectTabsArea.SetRectEmpty();
	m_rectWndArea.SetRectEmpty();
	m_nTabsHorzOffset = 0;
	m_nFirstVisibleTab = 0;
	m_nTabsHorzOffsetMax = 0;
	m_nTabsTotalWidth = 0;
	m_nHorzScrollWidth = 0;
	m_nScrollBarRight = 0;
	m_rectTabSplitter.SetRectEmpty();
	m_bTrackSplitter = FALSE;

	m_bFlatFrame = TRUE;

	m_bHideInactiveWnd = TRUE;
	m_bAutoSizeWindow = TRUE;
	m_bAutoDestroyWindow = TRUE;

	m_bTransparent = FALSE;
	m_bTopEdge = FALSE;
	m_bDrawFrame = TRUE;

	m_bHideNoTabs = FALSE;

	m_bIsActiveTabBold = FALSE;
	m_bTabDocumentsMenu = FALSE;
	m_bActiveTabCloseButton = FALSE;
	m_bHiddenDocuments = FALSE;
	m_nTabMaxWidth = 0;

	m_ResizeMode = RESIZE_NO;
	m_rectResize.SetRectEmpty();
	m_rectResizeDrag.SetRectEmpty();
	m_rectResizeBounds.SetRectEmpty();
	m_bResize = FALSE;
	m_bIsActiveInMDITabGroup = FALSE;
}

CMFCTabCtrl::~CMFCTabCtrl()
{
}

BEGIN_MESSAGE_MAP(CMFCTabCtrl, CMFCBaseTabCtrl)
	//{{AFX_MSG_MAP(CMFCTabCtrl)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_HSCROLL()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CANCELMODE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_SETTINGCHANGE()
	ON_WM_SETFOCUS()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_WINDOWPOSCHANGING()
	ON_REGISTERED_MESSAGE(AFX_WM_UPDATETOOLTIPS, &CMFCTabCtrl::OnUpdateToolTips)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CMFCTabCtrl::Create(Style style, const RECT& rect, CWnd* pParentWnd, UINT nID, Location location /* = LOCATION_BOTTOM*/, BOOL bCloseBtn /* = FALSE */)
{
	m_bFlat = (style == STYLE_FLAT) ||(style == STYLE_FLAT_SHARED_HORZ_SCROLL);
	m_bSharedScroll = style == STYLE_FLAT_SHARED_HORZ_SCROLL;
	m_bIsOneNoteStyle = (style == STYLE_3D_ONENOTE);
	m_bIsVS2005Style = (style == STYLE_3D_VS2005);
	m_bLeftRightRounded = (style == STYLE_3D_ROUNDED || style == STYLE_3D_ROUNDED_SCROLL);
	m_bHighLightTabs = m_bIsOneNoteStyle;
	m_location = location;
	m_bScroll = (m_bFlat || style == STYLE_3D_SCROLLED || style == STYLE_3D_ONENOTE || style == STYLE_3D_VS2005 || style == STYLE_3D_ROUNDED_SCROLL);
	m_bCloseBtn = bCloseBtn;

	if (!m_bFlat && m_bSharedScroll)
	{
		//--------------------------------------
		// Only flat tab has a shared scrollbar!
		//--------------------------------------
		ASSERT(FALSE);
		m_bSharedScroll = FALSE;
	}

	return CMFCBaseTabCtrl::Create(afxGlobalData.RegisterWindowClass(_T("Afx:TabWnd")), _T(""), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, rect, pParentWnd, nID);
}

/////////////////////////////////////////////////////////////////////////////
// CMFCTabCtrl message handlers

void CMFCTabCtrl::OnDestroy()
{
	if (m_brActiveTab.GetSafeHandle() != NULL)
	{
		m_brActiveTab.DeleteObject();
	}

	m_lstButtons.RemoveAll();

	CTooltipManager::DeleteToolTip(m_pToolTip);
	CTooltipManager::DeleteToolTip(m_pToolTipClose);

	CMFCBaseTabCtrl::OnDestroy();
}

void CMFCTabCtrl::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CMemDC memDC(dc, this);
	CDC* pDC = &memDC.GetDC();

	dc.GetClipBox(&m_rectCurrClip);

	COLORREF clrDark;
	COLORREF clrBlack;
	COLORREF clrHighlight;
	COLORREF clrFace;
	COLORREF clrDarkShadow;
	COLORREF clrLight;
	CBrush* pbrFace = NULL;
	CBrush* pbrBlack = NULL;

	CMFCVisualManager::GetInstance()->GetTabFrameColors(this, clrDark, clrBlack, clrHighlight, clrFace, clrDarkShadow, clrLight, pbrFace, pbrBlack);

	ASSERT_VALID(pbrFace);
	ASSERT_VALID(pbrBlack);

	CRect rectClient;
	GetClientRect(&rectClient);

	CBrush* pOldBrush = pDC->SelectObject(pbrFace);
	ENSURE(pOldBrush != NULL);

	CPen penDark(PS_SOLID, 1, clrDark);
	CPen penBlack(PS_SOLID, 1, clrBlack);
	CPen penHiLight(PS_SOLID, 1, clrHighlight);

	CPen* pOldPen = (CPen*) pDC->SelectObject(&penDark);
	ENSURE(pOldPen != NULL);

	const int nTabBorderSize = GetTabBorderSize();

	CRect rectTabs = rectClient;

	if (m_location == LOCATION_BOTTOM)
	{
		rectTabs.top = m_rectTabsArea.top;
	}
	else
	{
		rectTabs.bottom = m_rectTabsArea.bottom;
	}

	pDC->ExcludeClipRect(m_rectWndArea);

	BOOL bBackgroundIsReady = CMFCVisualManager::GetInstance()->OnEraseTabsFrame(pDC, rectClient, this);

	if (!m_bDrawFrame && !bBackgroundIsReady)
	{
		pDC->FillRect(rectClient, pbrFace);
	}

	CMFCVisualManager::GetInstance()->OnEraseTabsArea(pDC, rectTabs, this);

	CRect rectFrame = rectClient;

	if (nTabBorderSize == 0)
	{
		if (m_location == LOCATION_BOTTOM)
		{
			rectFrame.bottom = m_rectTabsArea.top + 1;
		}
		else
		{
			rectFrame.top = m_rectTabsArea.bottom - 1;
		}

		if (m_bFlat)
		{
			pDC->FrameRect(&rectFrame, pbrBlack);
		}
		else
		{
			pDC->FrameRect(&rectFrame, pbrFace);
		}
	}
	else
	{
		int yLine = m_location == LOCATION_BOTTOM ? m_rectTabsArea.top : m_rectTabsArea.bottom;

		if (!m_bFlat)
		{
			if (m_location == LOCATION_BOTTOM)
			{
				rectFrame.bottom = m_rectTabsArea.top;
			}
			else
			{
				rectFrame.top = m_rectTabsArea.bottom;
			}
		}

		//-----------------------------------------------------
		// Draw wide 3-dimensional frame around the Tabs area:
		//-----------------------------------------------------
		if (m_bFlatFrame)
		{
			CRect rectBorder(rectFrame);

			if (m_bFlat)
			{
				if (m_location == LOCATION_BOTTOM)
				{
					rectBorder.bottom = m_rectTabsArea.top + 1;
				}
				else
				{
					rectBorder.top = m_rectTabsArea.bottom - 1;
				}
			}

			rectFrame.DeflateRect(1, 1);

			if (m_bDrawFrame && !bBackgroundIsReady && rectFrame.Width() > 0 && rectFrame.Height() > 0)
			{
				pDC->PatBlt(rectFrame.left, rectFrame.top, nTabBorderSize, rectFrame.Height(), PATCOPY);
				pDC->PatBlt(rectFrame.left, rectFrame.top, rectFrame.Width(), nTabBorderSize, PATCOPY);
				pDC->PatBlt(rectFrame.right - nTabBorderSize - 1, rectFrame.top, nTabBorderSize + 1, rectFrame.Height(), PATCOPY);
				pDC->PatBlt(rectFrame.left, rectFrame.bottom - nTabBorderSize, rectFrame.Width(), nTabBorderSize, PATCOPY);

				if (m_location == LOCATION_BOTTOM)
				{
					pDC->PatBlt(rectFrame.left, m_rectWndArea.bottom, rectFrame.Width(), rectFrame.bottom - m_rectWndArea.bottom, PATCOPY);
				}
				else
				{
					pDC->PatBlt(rectFrame.left, rectFrame.top, rectFrame.Width(), m_rectWndArea.top - rectFrame.top, PATCOPY);
				}
			}

			if (m_bFlat)
			{
				//---------------------------
				// Draw line below the tabs:
				//---------------------------
				pDC->SelectObject(&penBlack);
				pDC->MoveTo(rectFrame.left + nTabBorderSize, yLine);
				pDC->LineTo(rectFrame.right - nTabBorderSize, yLine);
			}

			pDC->Draw3dRect(&rectBorder, clrFace, clrFace);

			if (GetTabsHeight() == 0)
			{
				pDC->Draw3dRect(&rectBorder, clrFace, clrFace);
			}
			else
			{
				if (m_bDrawFrame)
				{
					pDC->Draw3dRect(&rectBorder, clrDark, clrDark);
				}

				if (!m_bIsOneNoteStyle)
				{
					int xRight = rectBorder.right - 1;

					if (!m_bDrawFrame)
					{
						xRight -= nTabBorderSize;
					}

					if (m_location == LOCATION_BOTTOM)
					{
						pDC->SelectObject(&penBlack);

						pDC->MoveTo(rectBorder.left, rectBorder.bottom - 1);
						pDC->LineTo(xRight, rectBorder.bottom - 1);
					}
					else
					{
						pDC->SelectObject(&penHiLight);

						pDC->MoveTo(rectBorder.left, rectBorder.top);
						pDC->LineTo(xRight, rectBorder.top);
					}
				}
			}
		}
		else
		{
			if (m_bDrawFrame)
			{
				pDC->Draw3dRect(&rectFrame, clrHighlight, clrDarkShadow);

				rectFrame.DeflateRect(1, 1);
				pDC->Draw3dRect(&rectFrame, clrLight, clrDark);

				rectFrame.DeflateRect(1, 1);

				if (!bBackgroundIsReady && rectFrame.Width() > 0 && rectFrame.Height() > 0)
				{
					pDC->PatBlt(rectFrame.left, rectFrame.top, nTabBorderSize, rectFrame.Height(), PATCOPY);
					pDC->PatBlt(rectFrame.left, rectFrame.top, rectFrame.Width(), nTabBorderSize, PATCOPY);
					pDC->PatBlt(rectFrame.right - nTabBorderSize, rectFrame.top, nTabBorderSize, rectFrame.Height(), PATCOPY);
					pDC->PatBlt(rectFrame.left, rectFrame.bottom - nTabBorderSize, rectFrame.Width(), nTabBorderSize, PATCOPY);

					if (m_location == LOCATION_BOTTOM)
					{
						pDC->PatBlt(rectFrame.left, m_rectWndArea.bottom, rectFrame.Width(), rectFrame.bottom - m_rectWndArea.bottom, PATCOPY);
					}
					else
					{
						pDC->PatBlt(rectFrame.left, rectFrame.top, rectFrame.Width(), m_rectWndArea.top - rectFrame.top, PATCOPY);
					}

					if (m_bFlat)
					{
						//---------------------------
						// Draw line below the tabs:
						//---------------------------
						pDC->SelectObject(&penBlack);

						pDC->MoveTo(rectFrame.left + nTabBorderSize, yLine);
						pDC->LineTo(rectFrame.right - nTabBorderSize, yLine);
					}

					if (nTabBorderSize > 2)
					{
						rectFrame.DeflateRect(nTabBorderSize - 2, nTabBorderSize - 2);
					}

					if (rectFrame.Width() > 0 && rectFrame.Height() > 0)
					{
						pDC->Draw3dRect(&rectFrame, clrDarkShadow, clrHighlight);
					}
				}
				else
				{
					rectFrame.DeflateRect(2, 2);
				}
			}
		}
	}

	if (m_bTopEdge && m_location == LOCATION_TOP)
	{
		pDC->SelectObject(&penDark);

		pDC->MoveTo(rectClient.left, m_rectTabsArea.bottom);
		pDC->LineTo(rectClient.left, rectClient.top);
		pDC->LineTo(rectClient.right - 1, rectClient.top);
		pDC->LineTo(rectClient.right - 1, m_rectTabsArea.bottom);
	}

	CFont* pOldFont = pDC->SelectObject(m_bFlat ? &m_fntTabs : &afxGlobalData.fontRegular);
	ENSURE(pOldFont != NULL);

	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(afxGlobalData.clrBtnText);

	if (m_rectTabsArea.Width() > 5 && m_rectTabsArea.Height() > 5)
	{
		//-----------
		// Draw tabs:
		//-----------
		CRect rectClip = m_rectTabsArea;
		rectClip.InflateRect(1, nTabBorderSize);

		CRgn rgn;
		rgn.CreateRectRgnIndirect(rectClip);

		for (int i = m_iTabsNum - 1; i >= 0; i--)
		{
			CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [i];
			ASSERT_VALID(pTab);

			if (!pTab->m_bVisible)
				continue;

			m_iCurTab = i;

			if (i != m_iActiveTab) // Draw active tab last
			{
				pDC->SelectClipRgn(&rgn);

				if (m_bFlat)
				{
					pDC->SelectObject(&penBlack);
					DrawFlatTab(pDC, pTab, FALSE);
				}
				else
				{
					Draw3DTab(pDC, pTab, FALSE);
				}
			}
		}

		if (m_iActiveTab >= 0)
		{
			//-----------------
			// Draw active tab:
			//-----------------
			pDC->SetTextColor(afxGlobalData.clrWindowText);

			CMFCTabInfo* pTabActive = (CMFCTabInfo*) m_arTabs [m_iActiveTab];
			ASSERT_VALID(pTabActive);

			m_iCurTab = m_iActiveTab;

			pDC->SelectClipRgn(&rgn);

			if (m_bFlat)
			{
				pDC->SelectObject(&m_brActiveTab);
				pDC->SelectObject(&m_fntTabsBold);
				pDC->SetTextColor(GetActiveTabTextColor());
				pDC->SelectObject(&penBlack);

				DrawFlatTab(pDC, pTabActive, TRUE);

				//---------------------------------
				// Draw line bellow the active tab:
				//---------------------------------
				const int xLeft = max( m_rectTabsArea.left + 1, pTabActive->m_rect.left + 1);

				if (pTabActive->m_rect.right > m_rectTabsArea.left + 1)
				{
					CPen penLight(PS_SOLID, 1, GetActiveTabColor());
					pDC->SelectObject(&penLight);

					if (m_location == LOCATION_BOTTOM)
					{
						pDC->MoveTo(xLeft, pTabActive->m_rect.top);
						pDC->LineTo(pTabActive->m_rect.right, pTabActive->m_rect.top);
					}
					else
					{
						pDC->MoveTo(xLeft, pTabActive->m_rect.bottom);
						pDC->LineTo(pTabActive->m_rect.right, pTabActive->m_rect.bottom);
					}

					pDC->SelectObject(pOldPen);
				}
			}
			else
			{
				if (m_bIsActiveTabBold)
				{
					if (!IsMDITabGroup() || m_bIsActiveInMDITabGroup)
					{
						pDC->SelectObject(&afxGlobalData.fontBold);
					}
				}

				Draw3DTab(pDC, pTabActive, TRUE);
			}
		}

		pDC->SelectClipRgn(NULL);
	}

	if (!m_rectTabSplitter.IsRectEmpty())
	{
		pDC->FillRect(m_rectTabSplitter, pbrFace);

		CRect rectTabSplitter = m_rectTabSplitter;

		pDC->Draw3dRect(rectTabSplitter, clrDarkShadow, clrDark);
		rectTabSplitter.DeflateRect(1, 1);
		pDC->Draw3dRect(rectTabSplitter, clrHighlight, clrDark);
	}

	if (m_bFlat && m_nTabsHorzOffset > 0)
	{
		pDC->SelectObject(&penDark);

		const int xDivider = m_rectTabsArea.left - 1;

		if (m_location == LOCATION_BOTTOM)
		{
			pDC->MoveTo(xDivider, m_rectTabsArea.top + 1);
			pDC->LineTo(xDivider, m_rectTabsArea.bottom - 2);
		}
		else
		{
			pDC->MoveTo(xDivider, m_rectTabsArea.bottom);
			pDC->LineTo(xDivider, m_rectTabsArea.top + 2);
		}
	}

	if (!m_rectResize.IsRectEmpty())
	{
		pDC->FillRect(m_rectResize, pbrFace);

		pDC->SelectObject(&penDark);

		if (m_ResizeMode == RESIZE_VERT)
		{
			pDC->MoveTo(m_rectResize.left, m_rectResize.top);
			pDC->LineTo(m_rectResize.left, m_rectResize.bottom);
		}
		else
		{
			pDC->MoveTo(m_rectResize.left, m_rectResize.top);
			pDC->LineTo(m_rectResize.right, m_rectResize.top);
		}
	}

	pDC->SelectObject(pOldFont);
	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);

	if (memDC.IsMemDC())
	{
		dc.ExcludeClipRect(m_rectWndArea);
	}
}

void CMFCTabCtrl::OnSize(UINT nType, int cx, int cy)
{
	CMFCBaseTabCtrl::OnSize(nType, cx, cy);

	int nTabsAreaWidth = cx - 4 * ::GetSystemMetrics(SM_CXVSCROLL) - 2 * GetTabBorderSize();

	if (nTabsAreaWidth <= AFX_MIN_SCROLL_WIDTH)
	{
		m_nHorzScrollWidth = 0;
	}
	else if (nTabsAreaWidth / 2 > AFX_MIN_SCROLL_WIDTH)
	{
		m_nHorzScrollWidth = nTabsAreaWidth / 2;
	}
	else
	{
		m_nHorzScrollWidth = nTabsAreaWidth;
	}

	if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
	{
		m_nTabsHorzOffset = 0;
		m_nFirstVisibleTab = 0;

		SetRedraw(FALSE);

		RecalcLayout();

		if (m_iActiveTab >= 0)
		{
			EnsureVisible(m_iActiveTab);
		}

		SetRedraw(TRUE);
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
	}
	else
	{
		RecalcLayout();
	}

	SynchronizeScrollBar();
}

BOOL CMFCTabCtrl::SetActiveTab(int iTab)
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		TRACE(_T("SetActiveTab: illegal tab number %d\n"), iTab);
		return FALSE;
	}

	if (iTab >= m_arTabs.GetSize())
	{
		ASSERT(FALSE);
		return FALSE;
	}

	BOOL bIsFirstTime = (m_iActiveTab == -1);

	if (m_iActiveTab == iTab) // Already active, do nothing
	{
		if (IsMDITabGroup())
		{
			ActivateMDITab(m_iActiveTab);
		}

		return TRUE;
	}

	if (FireChangingActiveTab(iTab))
	{
		return FALSE;
	}

	CMDIFrameWndEx* pParentFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetParentFrame());
	BOOL bEnableSetRedraw = FALSE;

	if (pParentFrame != NULL && m_bIsMDITab)
	{
		bEnableSetRedraw = !pParentFrame->m_bClosing && !CMDIFrameWndEx::m_bDisableSetRedraw;
	}

	CWnd* pWndParent = GetParent();
	ASSERT_VALID(pWndParent);

	if (m_iTabsNum > 1 && bEnableSetRedraw)
	{
		pWndParent->SetRedraw(FALSE);
	}

	if (m_iActiveTab != -1 && m_bHideInactiveWnd)
	{
		//--------------------
		// Hide active window:
		//--------------------
		CWnd* pWndActive = GetActiveWnd();
		if (pWndActive != NULL)
		{
			pWndActive->ShowWindow(SW_HIDE);
		}
	}

	m_iActiveTab = iTab;

	//------------------------
	// Show new active window:
	//------------------------
	HideActiveWindowHorzScrollBar();

	CWnd* pWndActive = GetActiveWnd();
	if (pWndActive == NULL)
	{
		ASSERT(FALSE);
		pWndParent->SetRedraw(TRUE);
		return FALSE;
	}

	ASSERT_VALID(pWndActive);

	pWndActive->ShowWindow(SW_SHOW);
	if (!m_bHideInactiveWnd)
	{
		pWndActive->BringWindowToTop();
	}

	if (m_bAutoSizeWindow)
	{
		//----------------------------------------------------------------------
		// Small trick: to adjust active window scroll sizes, I should change an
		// active window size twice(+1 pixel and -1 pixel):
		//----------------------------------------------------------------------
		pWndActive->SetWindowPos(NULL, -1, -1, m_rectWndArea.Width() + 1, m_rectWndArea.Height(), SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
		pWndActive->SetWindowPos(NULL, -1, -1, m_rectWndArea.Width(), m_rectWndArea.Height(), SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
	}

	EnsureVisible(m_iActiveTab);

	if (m_bFlat)
	{
		SynchronizeScrollBar();
	}

	//--------------------------------------------------
	// Set text to the parent frame/docking control bar:
	//--------------------------------------------------
	CTabbedPane* pTabControlBar = DYNAMIC_DOWNCAST(CTabbedPane, GetParent());
	if (pTabControlBar != NULL && pTabControlBar->CanSetCaptionTextToTabName()) // tabbed dock bar - redraw caption only in this case
	{
		CString strCaption;
		GetTabLabel(m_iActiveTab, strCaption);

		// miniframe will take the text from the tab control bar
		pTabControlBar->SetWindowText(strCaption);

		CWnd* pWndToUpdate = pTabControlBar;
		if (!pTabControlBar->IsDocked())
		{
			pWndToUpdate = pTabControlBar->GetParent();
		}

		if (pWndToUpdate != NULL)
		{
			pWndToUpdate->RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
		}
	}

	if (m_bIsActiveTabBold || m_bActiveTabCloseButton)
	{
		RecalcLayout();
	}

	//-------------
	// Redraw tabs:
	//-------------
	Invalidate();
	UpdateWindow();

	if (!bIsFirstTime)
	{
		CView* pActiveView = DYNAMIC_DOWNCAST(CView, pWndActive);
		if (pActiveView != NULL)
		{
			CFrameWnd* pFrame = AFXGetParentFrame(pActiveView);
			ASSERT_VALID(pFrame);

			pFrame->SetActiveView(pActiveView);
		}
		else if (m_bEnableActivate)
		{
			pWndActive->SetFocus();
		}
	}

	if (m_btnClose.GetSafeHwnd() != NULL)
	{
		//----------------------------------------------------
		// Enable/disable "Close" button according to ability
		// to close an active window:
		//----------------------------------------------------
		BOOL bEnableClose = TRUE;

		HMENU hSysMenu = pWndActive->GetSystemMenu(FALSE)->GetSafeHmenu();
		if (hSysMenu != NULL)
		{
			MENUITEMINFO menuInfo;
			ZeroMemory(&menuInfo,sizeof(MENUITEMINFO));
			menuInfo.cbSize = sizeof(MENUITEMINFO);
			menuInfo.fMask = MIIM_STATE;

			if (!::GetMenuItemInfo(hSysMenu, SC_CLOSE, FALSE, &menuInfo) || (menuInfo.fState & MFS_GRAYED) || (menuInfo.fState & MFS_DISABLED))
			{
				bEnableClose = FALSE;
			}
		}

		m_btnClose.EnableWindow(bEnableClose);
	}

	FireChangeActiveTab(m_iActiveTab);

	if (m_iTabsNum > 1 && bEnableSetRedraw)
	{
		pWndParent->SetRedraw(TRUE);

		const UINT uiRedrawFlags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN;

		if (m_bSetActiveTabByMouseClick)
		{
			CRect rectWindow;
			GetWindowRect(rectWindow);
			GetParent()->ScreenToClient(rectWindow);

			pWndParent->RedrawWindow(rectWindow, NULL, uiRedrawFlags);
		}
		else
		{
			pWndParent->RedrawWindow(NULL, NULL, uiRedrawFlags);
		}
	}


	if (m_iActiveTab != -1 && pTabControlBar != NULL)
	{
		CBasePane* pBar = DYNAMIC_DOWNCAST(CBasePane, GetTabWnd(m_iActiveTab));
		if (pBar != NULL)
		{
			CPaneFrameWnd* pParentMiniFrame = pBar->GetParentMiniFrame();

			if (pBar->GetControlBarStyle() & AFX_CBRS_AUTO_ROLLUP)
			{
				pTabControlBar->m_dwControlBarStyle |= AFX_CBRS_AUTO_ROLLUP;
				if (pParentMiniFrame != NULL)
				{
					pParentMiniFrame->OnSetRollUpTimer();
				}
			}
			else
			{
				pTabControlBar->m_dwControlBarStyle &= ~AFX_CBRS_AUTO_ROLLUP;
				if (pParentMiniFrame != NULL)
				{
					pParentMiniFrame->OnKillRollUpTimer();
				}

			}
		}
	}

	return TRUE;
}

void CMFCTabCtrl::AdjustTabs()
{
	m_bHiddenDocuments = FALSE;

	m_rectCloseButton.SetRectEmpty();

	int nVisibleTabsNum = GetVisibleTabsNum();
	if (nVisibleTabsNum == 0 || GetTabsHeight() == 0)
	{
		return;
	}

	if (m_bHideSingleTab && nVisibleTabsNum <= 1)
	{
		for (int i = 0; i < m_iTabsNum; i ++)
		{
			CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [i];
			ASSERT_VALID(pTab);

			pTab->m_rect.SetRectEmpty();
		}

		return;
	}

	if (m_pToolTipClose->GetSafeHwnd() != NULL)
	{
		if (m_pToolTipClose->GetToolCount() == 0)
		{
			m_pToolTipClose->AddTool(this, LPSTR_TEXTCALLBACK, CRect(0, 0, 0, 0), 1);
		}
	}

	CRect rectActiveTabTT(0, 0, 0, 0);

	//-------------------------
	// Define tab's full width:
	//-------------------------
	CClientDC dc(this);

	CFont* pOldFont = dc.SelectObject(m_bFlat ? &m_fntTabsBold : &afxGlobalData.fontRegular);
	ENSURE(pOldFont != NULL);

	m_nTabsTotalWidth = 0;

	//----------------------------------------------
	// First, try set all tabs in its original size:
	//----------------------------------------------
	int x = m_rectTabsArea.left - m_nTabsHorzOffset;
	int i = 0;

	for (i = 0; i < m_iTabsNum; i++)
	{
		CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [i];
		ASSERT_VALID(pTab);

		CSize sizeImage(0, 0);
		if (pTab->m_hIcon != NULL || pTab->m_uiIcon != (UINT)-1)
		{
			sizeImage = m_sizeImage;
		}

		if (m_bIsActiveTabBold &&(m_bIsOneNoteStyle || m_bIsVS2005Style || i == m_iActiveTab))
		{
			dc.SelectObject(&afxGlobalData.fontBold);
		}

		int nExtraWidth = 0;

		if (pTab->m_bVisible)
		{
			pTab->m_nFullWidth = sizeImage.cx + AFX_TAB_IMAGE_MARGIN + (pTab->m_bIconOnly ? 0 : dc.GetTextExtent(pTab->m_strText).cx) + 2 * AFX_TAB_TEXT_MARGIN;

			if (m_bLeftRightRounded)
			{
				pTab->m_nFullWidth += m_rectTabsArea.Height() / 2;
				nExtraWidth = m_rectTabsArea.Height() / 2;
			}
			else if (m_bIsOneNoteStyle)
			{
				pTab->m_nFullWidth += m_rectTabsArea.Height() + 2 * AFX_TAB_IMAGE_MARGIN;
				nExtraWidth = m_rectTabsArea.Height() - AFX_TAB_IMAGE_MARGIN - 1;
			}
			else if (m_bIsVS2005Style)
			{
				pTab->m_nFullWidth += m_rectTabsArea.Height() - AFX_TAB_IMAGE_MARGIN;
				nExtraWidth = m_rectTabsArea.Height() - AFX_TAB_IMAGE_MARGIN - 1;
			}

			if (m_bActiveTabCloseButton && i == m_iActiveTab)
			{
				pTab->m_nFullWidth += m_rectTabsArea.Height() - 2;
			}
		}
		else
		{
			pTab->m_nFullWidth = 0;
		}

		if (m_bIsActiveTabBold && i == m_iActiveTab)
		{
			dc.SelectObject(&afxGlobalData.fontRegular); // Bold tab is available for 3d tabs only
		}

		int nTabWidth = pTab->m_nFullWidth;

		if (m_bScroll && m_nTabMaxWidth > 0)
		{
			nTabWidth = min(nTabWidth, m_nTabMaxWidth);
		}

		pTab->m_rect = CRect(CPoint(x, m_rectTabsArea.top), CSize(nTabWidth, m_rectTabsArea.Height() - 2));

		if (!pTab->m_bVisible)
		{
			if (m_pToolTip->GetSafeHwnd() != NULL)
			{
				m_pToolTip->SetToolRect(this, pTab->m_iTabID, CRect(0, 0, 0, 0));
			}
			continue;
		}

		if (m_location == LOCATION_TOP)
		{
			pTab->m_rect.OffsetRect(0, 2);
		}

		if (m_bTabDocumentsMenu && pTab->m_rect.right > m_rectTabsArea.right)
		{
			BOOL bHideTab = TRUE;

			if (i == m_iActiveTab && i == 0)
			{
				int nWidth = m_rectTabsArea.right - pTab->m_rect.left;

				if (nWidth >= nExtraWidth + 2 * AFX_TAB_TEXT_MARGIN)
				{
					pTab->m_rect.right = m_rectTabsArea.right;
					bHideTab = FALSE;
				}
			}

			if (bHideTab)
			{
				pTab->m_nFullWidth = 0;
				pTab->m_rect.SetRectEmpty();
				m_bHiddenDocuments = TRUE;
				continue;
			}
		}

		if (m_pToolTip->GetSafeHwnd() != NULL)
		{
			BOOL bShowTooltip = pTab->m_bAlwaysShowToolTip || m_bCustomToolTips;

			if (pTab->m_rect.left < m_rectTabsArea.left ||
				pTab->m_rect.right > m_rectTabsArea.right)
			{
				bShowTooltip = TRUE;
			}

			if (m_bScroll && m_nTabMaxWidth > 0 &&
				pTab->m_rect.Width() < pTab->m_nFullWidth)
			{
				bShowTooltip = TRUE;
			}

			m_pToolTip->SetToolRect(this, pTab->m_iTabID,
				bShowTooltip ? pTab->m_rect : CRect(0, 0, 0, 0));

			if (bShowTooltip && i == m_iActiveTab)
			{
				rectActiveTabTT = pTab->m_rect;
			}
		}

		x += pTab->m_rect.Width() + 1 - nExtraWidth;
		m_nTabsTotalWidth += pTab->m_rect.Width() + 1;

		if (i > 0)
		{
			m_nTabsTotalWidth -= nExtraWidth;
		}

		if (m_bFlat)
		{
			//--------------------------------------------
			// In the flat mode tab is overlapped by next:
			//--------------------------------------------
			pTab->m_rect.right += m_nTabsHeight / 2;
		}
	}

	if (m_bScroll || x < m_rectTabsArea.right)
	{
		m_nTabsTotalWidth += m_nTabsHeight / 2;
	}
	else
	{
		//-----------------------------------------
		// Not enouth space to show the whole text.
		//-----------------------------------------
		int nTabsWidth = m_rectTabsArea.Width();
		int nTabWidth = nTabsWidth / nVisibleTabsNum - 1;

		if (m_bLeftRightRounded)
		{
			nTabWidth = max( m_sizeImage.cx + m_rectTabsArea.Height() / 2, (nTabsWidth - m_rectTabsArea.Height() / 3) / nVisibleTabsNum);
		}

		//------------------------------------
		// May be it's too wide for some tabs?
		//------------------------------------
		int nRest = 0;
		int nCutTabsNum = nVisibleTabsNum;

		for (i = 0; i < m_iTabsNum; i ++)
		{
			CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [i];
			ASSERT_VALID(pTab);

			if (!pTab->m_bVisible)
			{
				continue;
			}

			if (pTab->m_nFullWidth < nTabWidth)
			{
				nRest += nTabWidth - pTab->m_nFullWidth;
				nCutTabsNum --;
			}
		}

		if (nCutTabsNum > 0)
		{
			nTabWidth += nRest / nCutTabsNum;

			//----------------------------------
			// Last pass: set actual rectangles:
			//----------------------------------
			x = m_rectTabsArea.left;
			for (i = 0; i < m_iTabsNum; i ++)
			{
				CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [i];
				ASSERT_VALID(pTab);

				if (!pTab->m_bVisible)
				{
					if (m_pToolTip->GetSafeHwnd() != NULL)
					{
						m_pToolTip->SetToolRect(this, pTab->m_iTabID, CRect(0, 0, 0, 0));
					}

					continue;
				}

				CSize sizeImage(0, 0);
				if (pTab->m_hIcon != NULL || pTab->m_uiIcon != (UINT)-1)
				{
					sizeImage = m_sizeImage;
				}

				BOOL bIsTrucncated = pTab->m_nFullWidth > nTabWidth;
				int nCurrTabWidth = (bIsTrucncated) ? nTabWidth : pTab->m_nFullWidth;

				if (nTabWidth < sizeImage.cx + AFX_TAB_IMAGE_MARGIN)
				{
					// Too narrow!
					nCurrTabWidth = (m_rectTabsArea.Width() + m_nTabBorderSize * 2) / nVisibleTabsNum;
				}
				else
				{
					if (pTab->m_strText.IsEmpty() || pTab->m_bIconOnly)
					{
						nCurrTabWidth = sizeImage.cx + 2 * CMFCBaseTabCtrl::AFX_TAB_TEXT_MARGIN;
					}
				}

				if (m_bLeftRightRounded)
				{
					nCurrTabWidth += m_rectTabsArea.Height() / 2 - 1;
				}

				pTab->m_rect = CRect(CPoint(x, m_rectTabsArea.top), CSize(nCurrTabWidth, m_rectTabsArea.Height() - 2));

				if (!m_bFlat)
				{
					if (m_location == LOCATION_TOP)
					{
						pTab->m_rect.OffsetRect(0, 2);
					}

					if (m_pToolTip->GetSafeHwnd() != NULL)
					{
						BOOL bShowTooltip = bIsTrucncated || pTab->m_bAlwaysShowToolTip || m_bCustomToolTips;

						m_pToolTip->SetToolRect(this, pTab->m_iTabID, bShowTooltip ? pTab->m_rect : CRect(0, 0, 0, 0));

						if (bShowTooltip && i == m_iActiveTab)
						{
							rectActiveTabTT = pTab->m_rect;
						}
					}
				}

				x += nCurrTabWidth;
				if (m_bLeftRightRounded)
				{
					x -= m_rectTabsArea.Height() / 2;
				}

				if (nRest > 0)
				{
					x ++;
				}
			}
		}
	}

	dc.SelectObject(pOldFont);

	if (m_bActiveTabCloseButton && m_iActiveTab >= 0)
	{
		GetTabRect(m_iActiveTab, m_rectCloseButton);

		m_rectCloseButton.left = m_rectCloseButton.right - m_rectCloseButton.Height();

		m_rectCloseButton.DeflateRect(2, 2);
		m_rectCloseButton.OffsetRect(-CMFCVisualManager::GetInstance()->GetTabHorzMargin(this), GetLocation() == CMFCBaseTabCtrl::LOCATION_TOP ? 1 : -1);

		if (m_pToolTipClose->GetSafeHwnd() != NULL)
		{
			m_pToolTipClose->SetToolRect(this, 1, m_rectCloseButton);

			CMFCTabInfo* pActiveTab = (CMFCTabInfo*) m_arTabs [m_iActiveTab];

			if (m_pToolTip->GetSafeHwnd() != NULL && pActiveTab != NULL)
			{
				if (!rectActiveTabTT.IsRectEmpty())
				{
					rectActiveTabTT.right = m_rectCloseButton.left - 1;
					m_pToolTip->SetToolRect(this, pActiveTab->m_iTabID, rectActiveTabTT);
				}
			}
		}
	}
}

void CMFCTabCtrl::Draw3DTab(CDC* pDC, CMFCTabInfo* pTab, BOOL bActive)
{
	ASSERT_VALID(pTab);
	ASSERT_VALID(pDC);

	ASSERT_VALID(pTab);
	ASSERT_VALID(pDC);

	if ((m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded) && pTab->m_rect.left < m_rectTabsArea.left)
	{
		return;
	}

	if (pTab->m_bVisible)
	{
		CRect rectInter;
		if (m_rectCurrClip.IsRectEmpty() || rectInter.IntersectRect(pTab->m_rect, m_rectCurrClip))
		{
			CMFCVisualManager::GetInstance()->OnDrawTab( pDC, pTab->m_rect, m_iCurTab, bActive, this);
		}
	}
}

void CMFCTabCtrl::DrawFlatTab(CDC* pDC, CMFCTabInfo* pTab, BOOL bActive)
{
	ASSERT_VALID(pTab);
	ASSERT_VALID(pDC);

	if (pTab->m_bVisible)
	{
		CMFCVisualManager::GetInstance()->OnDrawTab(pDC, pTab->m_rect, m_iCurTab, bActive, this);
	}
}

void CMFCTabCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_rectTabSplitter.PtInRect(point))
	{
		m_bTrackSplitter = TRUE;
		SetCapture();
		return;
	}

	if (m_ResizeMode != RESIZE_NO && m_rectResize.PtInRect(point))
	{
		RECT rectBounds;
		LRESULT lResult = GetParent()->SendMessage(AFX_WM_GETDRAGBOUNDS, (WPARAM)(LPVOID)this, (LPARAM)(LPVOID) &rectBounds);
		m_rectResizeBounds = rectBounds;

		if (lResult != 0 && !m_rectResizeBounds.IsRectEmpty())
		{
			m_bResize = TRUE;
			SetCapture();
			m_rectResizeDrag = m_rectResize;
			ClientToScreen(m_rectResizeDrag);

			CRect rectEmpty;
			rectEmpty.SetRectEmpty();

			DrawResizeDragRect(m_rectResizeDrag, rectEmpty);
			return;
		}
	}

	if (IsMDITabGroup())
	{
		int nTab = GetTabFromPoint(point);
		if (nTab == m_iActiveTab)
		{
			ActivateMDITab(nTab);
		}
	}

	CMFCBaseTabCtrl::OnLButtonDown(nFlags, point);

	if (!m_bReadyToDetach)
	{
		CWnd* pWndTarget = FindTargetWnd(point);
		if (pWndTarget != NULL)
		{
			ASSERT_VALID(pWndTarget);

			MapWindowPoints(pWndTarget, &point, 1);
			pWndTarget->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELPARAM(point.x, point.y));
		}
	}
}

void CMFCTabCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	for (POSITION pos = m_lstButtons.GetHeadPosition(); pos != NULL;)
	{
		CWnd* pWndBtn = CWnd::FromHandle(m_lstButtons.GetNext(pos));
		ASSERT_VALID(pWndBtn);

		CRect rectBtn;
		pWndBtn->GetClientRect(rectBtn);

		pWndBtn->MapWindowPoints(this, rectBtn);

		if (rectBtn.PtInRect(point))
		{
			return;
		}
	}

	CWnd* pWndTarget = FindTargetWnd(point);
	if (pWndTarget != NULL)
	{
		ASSERT_VALID(pWndTarget);

		MapWindowPoints(pWndTarget, &point, 1);
		pWndTarget->SendMessage(WM_LBUTTONDBLCLK, nFlags, MAKELPARAM(point.x, point.y));
	}
	else
	{
		CMFCBaseTabCtrl::OnLButtonDblClk(nFlags, point);
	}
}

int CMFCTabCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMFCBaseTabCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy(0, 0, 0, 0);

	if (m_bScroll)
	{
		//-----------------------
		// Create scroll buttons:
		//-----------------------
		if (m_bFlat)
		{
			m_btnScrollFirst.Create(_T(""), WS_CHILD | WS_VISIBLE, rectDummy, this, (UINT) -1);
			m_btnScrollFirst.SetStdImage(CMenuImages::IdArrowFirst);
			m_btnScrollFirst.m_bDrawFocus = FALSE;
			m_btnScrollFirst.m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;
			m_lstButtons.AddTail(m_btnScrollFirst.GetSafeHwnd());
		}

		m_btnScrollLeft.Create(_T(""), WS_CHILD | WS_VISIBLE, rectDummy, this, (UINT) -1);
		m_btnScrollLeft.SetStdImage(m_bFlat ? CMenuImages::IdArrowLeftLarge : CMenuImages::IdArrowLeftTab3d,
			m_bIsOneNoteStyle || m_bIsVS2005Style || m_bFlat ? CMenuImages::ImageBlack : CMenuImages::ImageDkGray,
			m_bFlat ?(CMenuImages::IMAGES_IDS) 0 : CMenuImages::IdArrowLeftDsbldTab3d);
		m_btnScrollLeft.m_bDrawFocus = FALSE;
		m_btnScrollLeft.m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;

		if (!m_bIsOneNoteStyle && !m_bIsVS2005Style)
		{
			m_btnScrollLeft.SetAutorepeatMode(50);
		}

		m_lstButtons.AddTail(m_btnScrollLeft.GetSafeHwnd());

		m_btnScrollRight.Create(_T(""), WS_CHILD | WS_VISIBLE, rectDummy, this, (UINT) -1);
		m_btnScrollRight.SetStdImage( m_bFlat ? CMenuImages::IdArrowRightLarge : CMenuImages::IdArrowRightTab3d,
			m_bIsOneNoteStyle || m_bIsVS2005Style || m_bFlat ? CMenuImages::ImageBlack : CMenuImages::ImageDkGray,
			m_bFlat ?(CMenuImages::IMAGES_IDS) 0 : CMenuImages::IdArrowRightDsbldTab3d);
		m_btnScrollRight.m_bDrawFocus = FALSE;
		m_btnScrollRight.m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;

		if (!m_bIsOneNoteStyle && !m_bIsVS2005Style)
		{
			m_btnScrollRight.SetAutorepeatMode(50);
		}

		m_lstButtons.AddTail(m_btnScrollRight.GetSafeHwnd());

		if (m_bFlat)
		{
			m_btnScrollLast.Create(_T(""), WS_CHILD | WS_VISIBLE, rectDummy, this, (UINT) -1);
			m_btnScrollLast.SetStdImage(CMenuImages::IdArrowLast);
			m_btnScrollLast.m_bDrawFocus = FALSE;
			m_btnScrollLast.m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;
			m_lstButtons.AddTail(m_btnScrollLast.GetSafeHwnd());
		}

		m_btnClose.Create(_T(""), WS_CHILD | WS_VISIBLE, rectDummy, this, (UINT) -1);
		m_btnClose.SetStdImage(CMenuImages::IdClose, m_bIsOneNoteStyle || m_bIsVS2005Style || m_bFlat ? CMenuImages::ImageBlack : CMenuImages::ImageDkGray);
		m_btnClose.m_bDrawFocus = FALSE;
		m_btnClose.m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;
		m_lstButtons.AddTail(m_btnClose.GetSafeHwnd());

		if (!m_bFlat && m_bScroll)
		{
			CString str;

			ENSURE(str.LoadString(IDS_AFXBARRES_CLOSEBAR));
			m_btnClose.SetTooltip(str);

			ENSURE(str.LoadString(IDP_AFXBARRES_SCROLL_LEFT));
			m_btnScrollLeft.SetTooltip(str);

			ENSURE(str.LoadString(IDP_AFXBARRES_SCROLL_RIGHT));
			m_btnScrollRight.SetTooltip(str);
		}
	}

	if (m_bSharedScroll)
	{
		m_wndScrollWnd.Create(WS_CHILD | WS_VISIBLE | SBS_HORZ, rectDummy, this, (UINT) -1);
	}

	if (m_bFlat)
	{
		//---------------------
		// Create active brush:
		//---------------------
		m_brActiveTab.CreateSolidBrush(GetActiveTabColor());
	}
	else
	{
		//---------------------------------------
		// Text may be truncated. Create tooltip.
		//---------------------------------------
		if (CTooltipManager::CreateToolTip(m_pToolTip, this, AFX_TOOLTIP_TYPE_TAB))
		{
			m_pToolTip->SetWindowPos(&wndTop, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);
		}
	}

	CTooltipManager::CreateToolTip(m_pToolTipClose, this, AFX_TOOLTIP_TYPE_TAB);

	if (afxGlobalData.m_hcurStretch == NULL)
	{
		afxGlobalData.m_hcurStretch = AfxGetApp()->LoadCursor(AFX_IDC_HSPLITBAR);
	}

	if (afxGlobalData.m_hcurStretchVert == NULL)
	{
		afxGlobalData.m_hcurStretchVert = AfxGetApp()->LoadCursor(AFX_IDC_VSPLITBAR);
	}

	SetTabsHeight();
	return 0;
}

BOOL CMFCTabCtrl::SetImageList(UINT uiID, int cx, COLORREF clrTransp)
{
	return CMFCBaseTabCtrl::SetImageList(uiID, cx, clrTransp);
}

BOOL CMFCTabCtrl::SetImageList(HIMAGELIST hImageList)
{
	return CMFCBaseTabCtrl::SetImageList(hImageList);
}

BOOL CMFCTabCtrl::OnEraseBkgnd(CDC* pDC)
{
	if (!m_bTransparent && GetVisibleTabsNum() == 0)
	{
		CRect rectClient;
		GetClientRect(rectClient);
		pDC->FillRect(rectClient, &afxGlobalData.brBtnFace);
	}

	return TRUE;
}

BOOL CMFCTabCtrl::PreTranslateMessage(MSG* pMsg)
{
	switch(pMsg->message)
	{
	case WM_KEYDOWN:
		if (m_iActiveTab != -1 && ::GetAsyncKeyState(VK_CONTROL) & 0x8000) // Ctrl is pressed
		{
			switch(pMsg->wParam)
			{
			case VK_NEXT:
				{
					for (int i = m_iActiveTab + 1; i < m_iActiveTab + m_iTabsNum; ++i)
					{
						int iTabIndex = i % m_iTabsNum;
						if (IsTabVisible(iTabIndex))
						{
							m_bUserSelectedTab = TRUE;
							SetActiveTab(iTabIndex);
							GetActiveWnd()->SetFocus();
							FireChangeActiveTab(m_iActiveTab);
							m_bUserSelectedTab = FALSE;
							break;
						}
					}
					return TRUE;
				}
			case VK_PRIOR:
				{
					for (int i = m_iActiveTab - 1 + m_iTabsNum; i > m_iActiveTab; --i)
					{
						int iTabIndex = i % m_iTabsNum;
						if (IsTabVisible(iTabIndex))
						{
							m_bUserSelectedTab = TRUE;
							SetActiveTab(iTabIndex);
							GetActiveWnd()->SetFocus();
							FireChangeActiveTab(m_iActiveTab);
							m_bUserSelectedTab = FALSE;
							break;
						}
					}
					return TRUE;
				}
			}
		}

		// Continue....

	case WM_SYSKEYDOWN:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_NCLBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
	case WM_NCMBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCRBUTTONUP:
	case WM_NCMBUTTONUP:
	case WM_MOUSEMOVE:
		if (m_pToolTipClose->GetSafeHwnd() != NULL)
		{
			m_pToolTipClose->RelayEvent(pMsg);
		}

		if (m_pToolTip->GetSafeHwnd() != NULL)
		{
			m_pToolTip->RelayEvent(pMsg);
		}
		break;
	}

	return CMFCBaseTabCtrl::PreTranslateMessage(pMsg);
}

void CMFCTabCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (!m_bFlat)
	{
		CMFCBaseTabCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	if (pScrollBar->GetSafeHwnd() == m_wndScrollWnd.GetSafeHwnd())
	{
		static BOOL bInsideScroll = FALSE;

		if (m_iActiveTab != -1 && !bInsideScroll)
		{
			CWnd* pWndActive = GetActiveWnd();
			ASSERT_VALID(pWndActive);

			CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [m_iActiveTab];
			ASSERT_VALID(pTab);

			WPARAM wParam = MAKEWPARAM(nSBCode, nPos);

			//----------------------------------
			// Pass scroll to the active window:
			//----------------------------------
			bInsideScroll = TRUE;

			if (pTab->m_bIsListView && (LOBYTE(nSBCode) == SB_THUMBPOSITION || LOBYTE(nSBCode) == SB_THUMBTRACK))
			{
				int dx = nPos - pWndActive->GetScrollPos(SB_HORZ);
				pWndActive->SendMessage(LVM_SCROLL, dx, 0);
			}

			pWndActive->SendMessage(WM_HSCROLL, wParam, 0);

			bInsideScroll = FALSE;

			m_wndScrollWnd.SetScrollPos(pWndActive->GetScrollPos(SB_HORZ));

			HideActiveWindowHorzScrollBar();
			GetParent()->SendMessage(AFX_WM_ON_HSCROLL, wParam);
		}

		return;
	}

	CMFCBaseTabCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

CWnd* CMFCTabCtrl::FindTargetWnd(const CPoint& point)
{
	if (point.y < m_nTabsHeight)
	{
		return NULL;
	}

	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [i];
		ASSERT_VALID(pTab);

		if (!pTab->m_bVisible)
			continue;

		if (pTab->m_rect.PtInRect(point))
		{
			return NULL;
		}
	}

	CWnd* pWndParent = GetParent();
	ASSERT_VALID(pWndParent);

	return pWndParent;
}

void CMFCTabCtrl::AdjustTabsScroll()
{
	ASSERT_VALID(this);

	if (!m_bScroll)
	{
		m_nTabsHorzOffset = 0;
		m_nFirstVisibleTab = 0;
		return;
	}

	if (m_iTabsNum == 0)
	{
		m_nTabsHorzOffsetMax = 0;
		m_nTabsHorzOffset = 0;
		m_nFirstVisibleTab = 0;
		return;
	}

	int nPrevHorzOffset = m_nTabsHorzOffset;

	m_nTabsHorzOffsetMax = max(0, m_nTabsTotalWidth - m_rectTabsArea.Width());

	if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
	{
		m_nTabsHorzOffset = max(0, m_nTabsHorzOffset);
	}
	else
	{
		m_nTabsHorzOffset = min(max(0, m_nTabsHorzOffset), m_nTabsHorzOffsetMax);
	}

	if (nPrevHorzOffset != m_nTabsHorzOffset)
	{
		AdjustTabs();
		InvalidateRect(m_rectTabsArea);
		UpdateWindow();
	}

	UpdateScrollButtonsState();
}

void CMFCTabCtrl::RecalcLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	ASSERT_VALID(this);

	int nTabsHeight = GetTabsHeight();
	const int nTabBorderSize = GetTabBorderSize();

	int nVisibleTabs = GetVisibleTabsNum();

	BOOL bHideTabs = (m_bHideSingleTab && nVisibleTabs <= 1) ||(m_bHideNoTabs && nVisibleTabs == 0);

	CRect rectClient;
	GetClientRect(rectClient);

	switch(m_ResizeMode)
	{
	case RESIZE_VERT:
		m_rectResize = rectClient;
		rectClient.right -= AFX_RESIZEBAR_SIZE;
		m_rectResize.left = rectClient.right + 1;
		break;

	case RESIZE_HORIZ:
		m_rectResize = rectClient;
		rectClient.bottom -= AFX_RESIZEBAR_SIZE;
		m_rectResize.top = rectClient.bottom + 1;
		break;

	default:
		m_rectResize.SetRectEmpty();
	}

	m_rectTabsArea = rectClient;
	m_rectTabsArea.DeflateRect(2, 0);

	int nScrollBtnWidth = 0;
	int nButtons = 0;
	int nButtonsWidth = 0;
	int nButtonsHeight = 0;
	int nButtonMargin = 0;

	if (m_bScroll)
	{
		nScrollBtnWidth = CMenuImages::Size().cx + 4 + CMFCVisualManager::GetInstance()->GetButtonExtraBorder().cx;

		if (!m_bFlat)
		{
			nScrollBtnWidth = min(nTabsHeight - 4 , nScrollBtnWidth + 2);
		}

		nButtons = (int) m_lstButtons.GetCount();
		if (!m_bCloseBtn || m_bActiveTabCloseButton)
		{
			nButtons--;
		}

		if (m_bTabDocumentsMenu)
		{
			nButtons--;
		}

		nButtonMargin = 3;
		nButtonsWidth = bHideTabs ? 0 :(nScrollBtnWidth + nButtonMargin) * nButtons;
	}

	if (m_bFlat)
	{
		if (m_location == LOCATION_BOTTOM)
		{
			if (nTabBorderSize > 1)
			{
				m_rectTabsArea.bottom -= nTabBorderSize - 1;
			}

			m_rectTabsArea.top = m_rectTabsArea.bottom - nTabsHeight;
		}
		else
		{
			if (nTabBorderSize > 1)
			{
				m_rectTabsArea.top += nTabBorderSize - 1;
			}

			m_rectTabsArea.bottom = m_rectTabsArea.top + nTabsHeight;
		}

		m_rectTabsArea.left += nButtonsWidth + 1;
		m_rectTabsArea.right--;

		if (m_rectTabsArea.right < m_rectTabsArea.left)
		{
			if (nTabBorderSize > 0)
			{
				m_rectTabsArea.left = rectClient.left + nTabBorderSize + 1;
				m_rectTabsArea.right = rectClient.right - nTabBorderSize - 1;
			}
			else
			{
				m_rectTabsArea.left = rectClient.left;
				m_rectTabsArea.right = rectClient.right;
			}
		}

		nButtonsHeight = m_rectTabsArea.Height();

		if (m_rectTabsArea.Height() + nTabBorderSize > rectClient.Height())
		{
			nButtonsHeight = 0;
			m_rectTabsArea.left = 0;
			m_rectTabsArea.right = 0;
		}

		int y = m_rectTabsArea.top;

		if (nButtonsHeight != 0)
		{
			y += max(0, (nButtonsHeight - nScrollBtnWidth) / 2);
			nButtonsHeight = nScrollBtnWidth;
		}

		// Reposition scroll butons:
		ReposButtons( CPoint(rectClient.left + nTabBorderSize + 1, y), CSize(nScrollBtnWidth, nButtonsHeight), bHideTabs, nButtonMargin);
	}
	else
	{
		if (m_location == LOCATION_BOTTOM)
		{
			m_rectTabsArea.top = m_rectTabsArea.bottom - nTabsHeight;
		}
		else
		{
			m_rectTabsArea.bottom = m_rectTabsArea.top + nTabsHeight;
		}

		if (m_bScroll)
		{
			m_rectTabsArea.right -= nButtonsWidth;

			if ((m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded) && !m_bTabDocumentsMenu)
			{
				m_rectTabsArea.OffsetRect(nScrollBtnWidth, 0);
			}

			// Reposition scroll butons:
			ReposButtons( CPoint(m_rectTabsArea.right + 1, m_rectTabsArea.CenterPoint().y - nScrollBtnWidth / 2), CSize(nScrollBtnWidth, nScrollBtnWidth), bHideTabs, nButtonMargin);
		}
	}

	m_rectWndArea = rectClient;
	m_nScrollBarRight = m_rectTabsArea.right - ::GetSystemMetrics(SM_CXVSCROLL);

	if (nTabBorderSize > 0)
	{
		m_rectWndArea.DeflateRect(nTabBorderSize + 1, nTabBorderSize + 1);

		switch(m_ResizeMode)
		{
		case RESIZE_VERT:
			m_rectWndArea.right += nTabBorderSize + 2;
			break;

		case RESIZE_HORIZ:
			m_rectWndArea.bottom += nTabBorderSize + 2;
			break;
		}
	}

	if (m_bFlat)
	{
		if (m_location == LOCATION_BOTTOM)
		{
			m_rectWndArea.bottom = m_rectTabsArea.top;

			if (nTabBorderSize == 0)
			{
				m_rectWndArea.top++;
				m_rectWndArea.left++;
			}
		}
		else
		{
			m_rectWndArea.top = m_rectTabsArea.bottom + nTabBorderSize;

			if (nTabBorderSize == 0)
			{
				m_rectWndArea.bottom--;
				m_rectWndArea.left++;
			}
		}
	}
	else
	{
		if (m_location == LOCATION_BOTTOM)
		{
			m_rectWndArea.bottom = m_rectTabsArea.top - nTabBorderSize;
		}
		else
		{
			m_rectWndArea.top = m_rectTabsArea.bottom + nTabBorderSize;
		}
	}

	if (m_bAutoSizeWindow)
	{
		for (int i = 0; i < m_iTabsNum; i ++)
		{
			CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [i];
			ASSERT_VALID(pTab);

			if (pTab->m_bVisible && pTab->m_pWnd->GetSafeHwnd() != NULL)
			{
				pTab->m_pWnd->SetWindowPos(NULL, m_rectWndArea.left, m_rectWndArea.top, m_rectWndArea.Width(), m_rectWndArea.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
			}
		}
	}

	AdjustWndScroll();
	AdjustTabs();
	AdjustTabsScroll();

	CRect rectFrame = rectClient;
	if (nTabBorderSize == 0)
	{
		if (m_location == LOCATION_BOTTOM)
		{
			rectFrame.bottom = m_rectTabsArea.top + 1;
		}
		else
		{
			rectFrame.top = m_rectTabsArea.bottom - 1;
		}
		InvalidateRect(rectFrame);
	}
	else
	{
		if (!m_bFlat)
		{
			if (m_location == LOCATION_BOTTOM)
			{
				rectFrame.bottom = m_rectTabsArea.top;
			}
			else
			{
				rectFrame.top = m_rectTabsArea.bottom;
			}
		}

		if (m_bFlatFrame)
		{
			CRect rectBorder(rectFrame);

			if (m_bFlat)
			{
				if (m_location == LOCATION_BOTTOM)
				{
					rectBorder.bottom = m_rectTabsArea.top + 1;
				}
				else
				{
					rectBorder.top = m_rectTabsArea.bottom - 1;
				}
			}
			InvalidateRect(rectBorder);
		}
		else
		{
			rectFrame.DeflateRect(1, 1);
			InvalidateRect(rectFrame);
		}
	}

	CRect rcUpdateArea;
	GetClientRect(&rcUpdateArea);

	if (m_location != LOCATION_BOTTOM)
	{
		rcUpdateArea.bottom = m_rectWndArea.bottom;
	}
	else
	{
		rcUpdateArea.top = m_rectWndArea.top;
	}

	InvalidateRect(rcUpdateArea);
	UpdateWindow();
}

void CMFCTabCtrl::AdjustWndScroll()
{
	if (!m_bSharedScroll)
	{
		return;
	}

	ASSERT_VALID(this);

	CRect rectScroll = m_rectTabsArea;

	int nVisibleTabs = GetVisibleTabsNum();

	BOOL bHideTabs = (m_bHideSingleTab && nVisibleTabs <= 1) ||(m_bHideNoTabs && nVisibleTabs == 0);

	if (!bHideTabs)
	{
		if (m_nHorzScrollWidth >= AFX_MIN_SCROLL_WIDTH)
		{
			rectScroll.top++;
			rectScroll.right = m_nScrollBarRight;
			rectScroll.left = rectScroll.right - m_nHorzScrollWidth;
			rectScroll.bottom -= 2;

			m_rectTabSplitter = rectScroll;
			m_rectTabSplitter.top ++;
			m_rectTabSplitter.right = rectScroll.left;
			m_rectTabSplitter.left = m_rectTabSplitter.right - AFX_SPLITTER_WIDTH;

			m_rectTabsArea.right = m_rectTabSplitter.left;

			ASSERT(!m_rectTabSplitter.IsRectEmpty());
		}
		else
		{
			rectScroll.SetRectEmpty();
			m_rectTabSplitter.SetRectEmpty();
		}
	}
	else
	{
		rectScroll.bottom -= 2;
		m_rectTabSplitter.SetRectEmpty();
	}

	m_wndScrollWnd.SetWindowPos(NULL, rectScroll.left, rectScroll.top, rectScroll.Width(), rectScroll.Height(), SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);
}

BOOL CMFCTabCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (m_bFlat && !m_rectTabSplitter.IsRectEmpty())
	{
		CPoint ptCursor;
		::GetCursorPos(&ptCursor);
		ScreenToClient(&ptCursor);

		if (m_rectTabSplitter.PtInRect(ptCursor))
		{
			::SetCursor(afxGlobalData.m_hcurStretch);
			return TRUE;
		}
	}

	if (!m_rectResize.IsRectEmpty())
	{
		CPoint ptCursor;
		::GetCursorPos(&ptCursor);
		ScreenToClient(&ptCursor);

		if (m_rectResize.PtInRect(ptCursor))
		{
			::SetCursor(m_ResizeMode == RESIZE_VERT ? afxGlobalData.m_hcurStretch : afxGlobalData.m_hcurStretchVert);
			return TRUE;
		}
	}

	return CMFCBaseTabCtrl::OnSetCursor(pWnd, nHitTest, message);
}

void CMFCTabCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bTrackSplitter || m_bResize)
	{
		StopResize(FALSE);
		m_bTrackSplitter = FALSE;
		m_bResize = FALSE;
		ReleaseCapture();
	}

	if (IsMDITabGroup())
	{
		CPoint pointDelta;
		GetCursorPos(&pointDelta);
		pointDelta = m_ptHot - pointDelta;
		int nDrag = GetSystemMetrics(SM_CXDRAG);
		if (GetCapture() == this && m_bReadyToDetach && (abs(pointDelta.x) > nDrag || abs(pointDelta.y) > nDrag))
		{
			ReleaseCapture();
			if (!IsPtInTabArea(point))
			{
				GetParent()->SendMessage(AFX_WM_ON_MOVETABCOMPLETE, (WPARAM) this, (LPARAM) MAKELPARAM(point.x, point.y));
			}
		}
		else
		{
			ActivateMDITab();
		}
	}

	CMFCBaseTabCtrl::OnLButtonUp(nFlags, point);
}

void CMFCTabCtrl::StopResize(BOOL bCancel)
{
	if (m_bResize)
	{
		CRect rectEmpty;
		rectEmpty.SetRectEmpty();

		DrawResizeDragRect(rectEmpty, m_rectResizeDrag);

		m_bResize = FALSE;
		ReleaseCapture();

		if (!bCancel)
		{
			CRect rectWnd;
			GetWindowRect(rectWnd);

			if (m_ResizeMode == RESIZE_VERT)
			{
				rectWnd.right = m_rectResizeDrag.right;
			}
			else if (m_ResizeMode == RESIZE_HORIZ)
			{
				rectWnd.bottom = m_rectResizeDrag.bottom;
			}

			RECT rect = rectWnd;
			GetParent()->SendMessage(AFX_WM_ON_DRAGCOMPLETE, (WPARAM) this, (LPARAM) &rect);
		}

		m_rectResizeDrag.SetRectEmpty();
		m_rectResizeBounds.SetRectEmpty();
	}
}

void CMFCTabCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bResize)
	{
		CRect rectNew = m_rectResizeDrag;
		ClientToScreen(&point);
		CSize size;
		if (m_ResizeMode == RESIZE_VERT)
		{
			int nWidth = rectNew.Width();
			size.cx = size.cy = nWidth;

			rectNew.left = point.x - nWidth / 2;
			rectNew.right = rectNew.left + nWidth;

			if (rectNew.left < m_rectResizeBounds.left)
			{
				rectNew.left = m_rectResizeBounds.left;
				rectNew.right = rectNew.left + nWidth;
			}
			else if (rectNew.right > m_rectResizeBounds.right)
			{
				rectNew.right = m_rectResizeBounds.right;
				rectNew.left = rectNew.right - nWidth;
			}
		}
		else if (m_ResizeMode == RESIZE_HORIZ)
		{
			int nHeight = rectNew.Height();
			size.cx = size.cy = nHeight;

			rectNew.top = point.y - nHeight / 2;
			rectNew.bottom = rectNew.top + nHeight;

			if (rectNew.top < m_rectResizeBounds.top)
			{
				rectNew.top = m_rectResizeBounds.top;
				rectNew.bottom = rectNew.top + nHeight;
			}
			else if (rectNew.bottom > m_rectResizeBounds.bottom)
			{
				rectNew.bottom = m_rectResizeBounds.bottom;
				rectNew.top = rectNew.bottom - nHeight;
			}
		}

		DrawResizeDragRect(rectNew, m_rectResizeDrag);
		m_rectResizeDrag = rectNew;
		return;
	}

	if (m_bTrackSplitter)
	{
		int nSplitterLeftPrev = m_rectTabSplitter.left;

		m_nHorzScrollWidth = min(m_nScrollBarRight - m_rectTabsArea.left - AFX_SPLITTER_WIDTH, m_nScrollBarRight - point.x);
		m_nHorzScrollWidth = max(AFX_MIN_SCROLL_WIDTH, m_nHorzScrollWidth);
		AdjustWndScroll();

		if (m_rectTabSplitter.left > nSplitterLeftPrev)
		{
			CRect rect = m_rectTabSplitter;
			rect.left = nSplitterLeftPrev - 20;
			rect.right = m_rectTabSplitter.left;
			rect.InflateRect(0, GetTabBorderSize() + 1);

			InvalidateRect(rect);
		}

		CRect rectTabSplitter = m_rectTabSplitter;
		rectTabSplitter.InflateRect(0, GetTabBorderSize());

		InvalidateRect(rectTabSplitter);
		UpdateWindow();
		AdjustTabsScroll();
	}
	else if (GetCapture() == this && IsMDITabGroup() && m_bReadyToDetach)
	{
		CPoint pointDelta;
		GetCursorPos(&pointDelta);
		pointDelta = m_ptHot - pointDelta;
		int nDrag = GetSystemMetrics(SM_CXDRAG);
		if (GetCapture() == this && m_bReadyToDetach && (abs(pointDelta.x) < nDrag && abs(pointDelta.y) < nDrag))
		{
			return;
		}

		if (GetParent()->SendMessage(AFX_WM_ON_TABGROUPMOUSEMOVE, nFlags, MAKELPARAM(point.x, point.y)))
		{
			return;
		}
	}

	if (!m_bFlat)
	{
		if (CMFCVisualManager::GetInstance()->AlwaysHighlight3DTabs())
		{
			m_bHighLightTabs = TRUE;
		}
		else if (m_bIsOneNoteStyle)
		{
			m_bHighLightTabs = CMFCVisualManager::GetInstance()->IsHighlightOneNoteTabs();
		}
	}

	CMFCBaseTabCtrl::OnMouseMove(nFlags, point);
}

void CMFCTabCtrl::OnCancelMode()
{
	BOOL bWasCaptured = (GetCapture() == this);

	if (IsMDITabGroup() && bWasCaptured)
	{
		GetParent()->SendMessage(AFX_WM_ON_CANCELTABMOVE);
	}

	CMFCBaseTabCtrl::OnCancelMode();
	StopResize(TRUE);

	if (m_bTrackSplitter)
	{
		m_bResize = FALSE;
		m_bTrackSplitter = FALSE;
		ReleaseCapture();
	}
}

void CMFCTabCtrl::OnSysColorChange()
{
	CMFCBaseTabCtrl::OnSysColorChange();

	if (m_bFlat && m_clrActiveTabFg == (COLORREF) -1)
	{
		if (m_brActiveTab.GetSafeHandle() != NULL)
		{
			m_brActiveTab.DeleteObject();
		}

		m_brActiveTab.CreateSolidBrush(GetActiveTabColor());

		Invalidate();
		UpdateWindow();
	}
}

BOOL CMFCTabCtrl::SynchronizeScrollBar(SCROLLINFO* pScrollInfo/* = NULL*/)
{
	if (!m_bSharedScroll)
	{
		return FALSE;
	}

	ASSERT_VALID(this);

	SCROLLINFO scrollInfo;
	memset(&scrollInfo, 0, sizeof(SCROLLINFO));

	scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_ALL;

	CWnd* pWndActive = GetActiveWnd();

	if (pScrollInfo != NULL)
	{
		scrollInfo = *pScrollInfo;
	}
	else if (pWndActive != NULL)
	{
		if (!pWndActive->GetScrollInfo(SB_HORZ, &scrollInfo) || scrollInfo.nMin +(int) scrollInfo.nPage >= scrollInfo.nMax)
		{
			m_wndScrollWnd.EnableScrollBar(ESB_DISABLE_BOTH);
			return TRUE;
		}
	}

	m_wndScrollWnd.EnableScrollBar(ESB_ENABLE_BOTH);
	m_wndScrollWnd.SetScrollInfo(&scrollInfo);

	HideActiveWindowHorzScrollBar();
	return TRUE;
}

void CMFCTabCtrl::HideActiveWindowHorzScrollBar()
{
	CWnd* pWnd = GetActiveWnd();
	if (pWnd == NULL || !m_bSharedScroll)
	{
		return;
	}

	ASSERT_VALID(pWnd);

	pWnd->ShowScrollBar(SB_HORZ, FALSE);
	pWnd->ModifyStyle(WS_HSCROLL, 0, SWP_DRAWFRAME);
}

void CMFCTabCtrl::SetTabsHeight()
{
	if (m_bFlat)
	{
		m_nTabsHeight = ::GetSystemMetrics(SM_CYHSCROLL) + CMFCBaseTabCtrl::AFX_TAB_TEXT_MARGIN / 2;

		LOGFONT lfDefault;
		afxGlobalData.fontRegular.GetLogFont(&lfDefault);

		LOGFONT lf;
		memset(&lf, 0, sizeof(LOGFONT));

		lf.lfCharSet = lfDefault.lfCharSet;
		lf.lfHeight = lfDefault.lfHeight;
		lstrcpy(lf.lfFaceName, AFX_TABS_FONT);

		CClientDC dc(this);

		TEXTMETRIC tm;

		do
		{
			m_fntTabs.DeleteObject();
			m_fntTabs.CreateFontIndirect(&lf);

			CFont* pFont = dc.SelectObject(&m_fntTabs);
			ENSURE(pFont != NULL);

			dc.GetTextMetrics(&tm);
			dc.SelectObject(pFont);

			if (tm.tmHeight + CMFCBaseTabCtrl::AFX_TAB_TEXT_MARGIN / 2 <= m_nTabsHeight)
			{
				break;
			}

			//------------------
			// Try smaller font:
			//------------------
			if (lf.lfHeight < 0)
			{
				lf.lfHeight ++;
			}
			else
			{
				lf.lfHeight --;
			}
		}
		while (lf.lfHeight != 0);

		//------------------
		// Create bold font:
		//------------------
		lf.lfWeight = FW_BOLD;
		m_fntTabsBold.DeleteObject();
		m_fntTabsBold.CreateFontIndirect(&lf);
	}
	else if (m_bIsVS2005Style)
	{
		const int nImageHeight = m_sizeImage.cy <= 0 ? 0 : m_sizeImage.cy + 7;
		m_nTabsHeight = (max(nImageHeight, afxGlobalData.GetTextHeight() + 4));
	}
	else
	{
		CMFCBaseTabCtrl::SetTabsHeight();
	}
}

void CMFCTabCtrl::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CMFCBaseTabCtrl::OnSettingChange(uFlags, lpszSection);

	//-----------------------------------------------------------------
	// In the flat modetabs height should be same as scroll bar height
	//-----------------------------------------------------------------
	if (m_bFlat)
	{
		SetTabsHeight();
		RecalcLayout();
		SynchronizeScrollBar();
	}
}

BOOL CMFCTabCtrl::EnsureVisible(int iTab)
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		TRACE(_T("EnsureVisible: illegal tab number %d\n"), iTab);
		return FALSE;
	}

	if (!m_bScroll || m_rectTabsArea.Width() <= 0)
	{
		return TRUE;
	}

	//---------------------------------------------------------
	// Be sure, that active tab is visible(not out of scroll):
	//---------------------------------------------------------
	CRect rectTab = ((CMFCTabInfo*) m_arTabs [iTab])->m_rect;

	if (m_bTabDocumentsMenu)
	{
		if (rectTab.left >= m_rectTabsArea.right || rectTab.IsRectEmpty())
		{
			CMFCBaseTabCtrl::MoveTab(iTab, 0);
		}

		return TRUE;
	}

	BOOL bAdjustTabs = FALSE;

	if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
	{
		if (rectTab.left < m_rectTabsArea.left || rectTab.right > m_rectTabsArea.right)
		{
			// Calculate total width of tabs located left from active + active:
			int nWidthLeft = 0;
			const int nExtraWidth = m_rectTabsArea.Height() - AFX_TAB_IMAGE_MARGIN - 1;

			for (int i = 0; i <= iTab; i++)
			{
				nWidthLeft += ((CMFCTabInfo*) m_arTabs [i])->m_rect.Width() - nExtraWidth;
			}

			m_nTabsHorzOffset = 0;
			m_nFirstVisibleTab = 0;

			while ( m_nFirstVisibleTab < iTab && nWidthLeft > m_rectTabsArea.Width())
			{
				const int nCurrTabWidth = ((CMFCTabInfo*) m_arTabs [m_nFirstVisibleTab])->m_rect.Width() - nExtraWidth;

				m_nTabsHorzOffset += nCurrTabWidth;
				nWidthLeft -= nCurrTabWidth;

				m_nFirstVisibleTab++;
			}

			bAdjustTabs = TRUE;
		}
	}
	else
	{
		if (rectTab.left < m_rectTabsArea.left)
		{
			m_nTabsHorzOffset -= (m_rectTabsArea.left - rectTab.left);
			bAdjustTabs = TRUE;
		}
		else if (rectTab.right > m_rectTabsArea.right && rectTab.Width() <= m_rectTabsArea.Width())
		{
			m_nTabsHorzOffset += (rectTab.right - m_rectTabsArea.right);
			bAdjustTabs = TRUE;
		}
	}

	if (bAdjustTabs)
	{
		AdjustTabs();
		AdjustTabsScroll();

		RedrawWindow();
	}

	return TRUE;
}

BOOL CMFCTabCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	BOOL bRes = CMFCBaseTabCtrl::OnNotify(wParam, lParam, pResult);

	NMHDR* pNMHDR = (NMHDR*)lParam;
	ENSURE(pNMHDR != NULL);

	if (pNMHDR->code == TTN_SHOW)
	{
		if (m_pToolTip->GetSafeHwnd() != NULL)
		{
			m_pToolTip->SetWindowPos(&wndTop, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);
		}

		if (m_pToolTipClose->GetSafeHwnd() != NULL && pNMHDR->hwndFrom == m_pToolTipClose->GetSafeHwnd())
		{
			m_pToolTipClose->SetWindowPos(&wndTop, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);
		}
	}

	if (pNMHDR->code == HDN_ITEMCHANGED)
	{
		SynchronizeScrollBar();
	}

	return bRes;
}

void CMFCTabCtrl::HideNoTabs(BOOL bHide)
{
	if (m_bHideNoTabs == bHide)
	{
		return;
	}

	m_bHideNoTabs = bHide;

	if (GetSafeHwnd() != NULL)
	{
		RecalcLayout();
		SynchronizeScrollBar();
	}
}

BOOL CMFCTabCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	const int nScrollOffset = 20;

	BOOL bScrollTabs = FALSE;
	const int nPrevOffset = m_nTabsHorzOffset;

	if ((HWND)lParam == m_btnScrollLeft.GetSafeHwnd())
	{
		bScrollTabs = TRUE;

		if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
		{
			if (m_nFirstVisibleTab > 0)
			{
				m_nTabsHorzOffset -= ((CMFCTabInfo*) m_arTabs [m_nFirstVisibleTab - 1])->m_rect.Width() - (m_rectTabsArea.Height() - AFX_TAB_IMAGE_MARGIN - 2);
				m_nFirstVisibleTab --;
			}
		}
		else
		{
			m_nTabsHorzOffset -= nScrollOffset;
		}
	}
	else if ((HWND)lParam == m_btnScrollRight.GetSafeHwnd())
	{
		if (m_bTabDocumentsMenu)
		{
			CRect rectButton;
			m_btnScrollRight.GetWindowRect(&rectButton);

			m_btnScrollRight.SetPressed(TRUE);

			CPoint ptMenu(rectButton.left, rectButton.bottom);

			if (GetExStyle() & WS_EX_LAYOUTRTL)
			{
				ptMenu.x += rectButton.Width();
			}

			m_btnScrollRight.SendMessage(WM_CANCELMODE);

			HWND hwndThis = GetSafeHwnd();
			m_btnScrollRight.SetPressed (TRUE); 

			OnShowTabDocumentsMenu(ptMenu);

			if (!::IsWindow(hwndThis))
			{
				return TRUE;
			}

			m_btnScrollRight.SetPressed(FALSE);
		}
		else
		{
			bScrollTabs = TRUE;

			if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
			{
				if (m_nFirstVisibleTab < m_iTabsNum)
				{
					m_nTabsHorzOffset += ((CMFCTabInfo*) m_arTabs [m_nFirstVisibleTab])->m_rect.Width() - (m_rectTabsArea.Height() - AFX_TAB_IMAGE_MARGIN - 1);
					m_nFirstVisibleTab ++;
				}
			}
			else
			{
				m_nTabsHorzOffset += nScrollOffset;
			}
		}
	}
	else if ((HWND)lParam == m_btnScrollFirst.GetSafeHwnd())
	{
		bScrollTabs = TRUE;
		m_nTabsHorzOffset = 0;
	}
	else if ((HWND)lParam == m_btnScrollLast.GetSafeHwnd())
	{
		bScrollTabs = TRUE;
		m_nTabsHorzOffset = m_nTabsHorzOffsetMax;
	}
	else if ((HWND)lParam == m_btnClose.GetSafeHwnd())
	{
		CWnd* pWndActive = GetActiveWnd();
		if (pWndActive != NULL)
		{
			pWndActive->SendMessage(WM_CLOSE);
		}

		return TRUE;
	}

	if (bScrollTabs)
	{
		if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
		{
			m_nTabsHorzOffset = max(0, m_nTabsHorzOffset);
		}
		else
		{
			m_nTabsHorzOffset = min(max(0, m_nTabsHorzOffset), m_nTabsHorzOffsetMax);
		}

		if (nPrevOffset != m_nTabsHorzOffset)
		{
			AdjustTabs();
			UpdateScrollButtonsState();
			Invalidate();
			UpdateWindow();
		}

		return TRUE;
	}

	return CMFCBaseTabCtrl::OnCommand(wParam, lParam);
}

void CMFCTabButton::OnFillBackground(CDC* pDC, const CRect& rectClient)
{
	CMFCVisualManager::GetInstance()->OnEraseTabsButton(pDC, rectClient, this, DYNAMIC_DOWNCAST(CMFCTabCtrl, GetParent()));
}

void CMFCTabButton::OnDrawBorder(CDC* pDC, CRect& rectClient, UINT uiState)
{
	CMFCVisualManager::GetInstance()->OnDrawTabsButtonBorder(pDC, rectClient, this, uiState, DYNAMIC_DOWNCAST(CMFCTabCtrl, GetParent()));
}

void CMFCTabCtrl::OnSetFocus(CWnd* pOldWnd)
{
	CMFCBaseTabCtrl::OnSetFocus(pOldWnd);

	CWnd* pWndActive = GetActiveWnd();
	if (pWndActive != NULL)
	{
		pWndActive->SetFocus();
	}
}

void CMFCTabCtrl::ReposButtons(CPoint pt, CSize sizeButton, BOOL bHide, int nButtonMargin)
{
	BOOL bIsFirst = TRUE;
	for (POSITION pos = m_lstButtons.GetHeadPosition(); pos != NULL;)
	{
		HWND hWndButton = m_lstButtons.GetNext(pos);
		ENSURE(hWndButton != NULL);

		BOOL bCloseBtn = m_bCloseBtn && !m_bActiveTabCloseButton;

		if (bHide ||(!bCloseBtn && hWndButton == m_btnClose.GetSafeHwnd()) || (m_bTabDocumentsMenu && bIsFirst))
		{
			::SetWindowPos(hWndButton, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER);
		}
		else
		{
			int x = pt.x;

			if ((m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded) && bIsFirst) // Scroll left is on left
			{
				x = m_rectTabsArea.left - sizeButton.cx - 1;
			}

			::SetWindowPos(hWndButton, NULL, x, pt.y, sizeButton.cx, sizeButton.cy, SWP_NOACTIVATE | SWP_NOZORDER);

			if ((!m_bIsOneNoteStyle && !m_bIsVS2005Style && !m_bLeftRightRounded) || !bIsFirst)
			{
				pt.x += sizeButton.cx + nButtonMargin;
			}
		}

		::InvalidateRect(hWndButton, NULL, TRUE);
		::UpdateWindow(hWndButton);

		bIsFirst = FALSE;
	}
}

BOOL CMFCTabCtrl::IsPtInTabArea(CPoint point) const
{
	return m_rectTabsArea.PtInRect(point);
}

void CMFCTabCtrl::EnableInPlaceEdit(BOOL bEnable)
{
	ASSERT_VALID(this);

	if (!m_bFlat)
	{
		// In-place editing is available for the flat tabs only!
		ASSERT(FALSE);
		return;
	}

	m_bIsInPlaceEdit = bEnable;
}

void CMFCTabCtrl::SetActiveTabBoldFont(BOOL bIsBold)
{
	ASSERT_VALID(this);

	m_bIsActiveTabBold = bIsBold;

	if (GetSafeHwnd() != NULL)
	{
		RecalcLayout();
		SynchronizeScrollBar();
	}
}

void CMFCTabCtrl::HideSingleTab(BOOL bHide)
{
	if (m_bHideSingleTab == bHide)
	{
		return;
	}

	m_bHideSingleTab = bHide;

	if (GetSafeHwnd() != NULL)
	{
		RecalcLayout();
		SynchronizeScrollBar();
	}
}

void CMFCTabCtrl::UpdateScrollButtonsState()
{
	ASSERT_VALID(this);

	if (GetSafeHwnd() == NULL || !m_bScroll || m_bFlat)
	{
		return;
	}

	if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
	{
		if (m_arTabs.GetSize() == 0)
		{
			m_btnScrollLeft.EnableWindow(FALSE);
			m_btnScrollRight.EnableWindow(FALSE);
		}
		else
		{
			m_btnScrollLeft.EnableWindow(m_nFirstVisibleTab > 0);

			CMFCTabInfo* pLastTab = (CMFCTabInfo*) m_arTabs [m_arTabs.GetSize() - 1];
			ASSERT_VALID(pLastTab);

			m_btnScrollRight.EnableWindow(m_bTabDocumentsMenu || (pLastTab->m_rect.right > m_rectTabsArea.right && m_nFirstVisibleTab < m_arTabs.GetSize() - 1));
		}
	}
	else
	{
		m_btnScrollLeft.EnableWindow(m_nTabsHorzOffset > 0);
		m_btnScrollRight.EnableWindow(m_bTabDocumentsMenu || m_nTabsHorzOffset < m_nTabsHorzOffsetMax);
	}

	if (m_bTabDocumentsMenu)
	{
		m_btnScrollRight.SetStdImage(m_bHiddenDocuments ? CMenuImages::IdCustomizeArrowDownBold : CMenuImages::IdArrowDownLarge,
			m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded ? CMenuImages::ImageBlack : CMenuImages::ImageDkGray, CMenuImages::IdArrowDownLarge);
	}

	for (POSITION pos = m_lstButtons.GetHeadPosition(); pos != NULL;)
	{
		HWND hWndButton = m_lstButtons.GetNext(pos);
		ENSURE(hWndButton != NULL);

		if (!::IsWindowEnabled(hWndButton))
		{
			::SendMessage(hWndButton, WM_CANCELMODE, 0, 0);
		}
	}
}

BOOL CMFCTabCtrl::ModifyTabStyle(Style style)
{
	ASSERT_VALID(this);

	m_bFlat = (style == STYLE_FLAT);
	m_bIsOneNoteStyle = (style == STYLE_3D_ONENOTE);
	m_bIsVS2005Style = (style == STYLE_3D_VS2005);
	m_bHighLightTabs = m_bIsOneNoteStyle;
	m_bLeftRightRounded = (style == STYLE_3D_ROUNDED || style == STYLE_3D_ROUNDED_SCROLL);

	SetScrollButtons();
	SetTabsHeight();

	return TRUE;
}

void CMFCTabCtrl::SetScrollButtons()
{
	const int nAutoRepeat = m_bIsOneNoteStyle || m_bTabDocumentsMenu ? 0 : 50;

	m_btnScrollLeft.SetAutorepeatMode(nAutoRepeat);
	m_btnScrollRight.SetAutorepeatMode(nAutoRepeat);

	m_btnScrollLeft.SetStdImage(CMenuImages::IdArrowLeftTab3d,
		m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded ? CMenuImages::ImageBlack : CMenuImages::ImageDkGray, CMenuImages::IdArrowLeftDsbldTab3d);

	if (m_bTabDocumentsMenu)
	{
		m_btnScrollRight.SetStdImage(CMenuImages::IdArrowDownLarge,
			m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded ? CMenuImages::ImageBlack : CMenuImages::ImageDkGray, CMenuImages::IdArrowDownLarge);
	}
	else
	{
		m_btnScrollRight.SetStdImage(CMenuImages::IdArrowRightTab3d,
			m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded ? CMenuImages::ImageBlack : CMenuImages::ImageDkGray, CMenuImages::IdArrowRightDsbldTab3d);
	}

	m_btnClose.SetStdImage(CMenuImages::IdClose, m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded ? CMenuImages::ImageBlack : CMenuImages::ImageDkGray);
}

void CMFCTabCtrl::SetDrawFrame(BOOL bDraw)
{
	m_bDrawFrame = bDraw;
}

DROPEFFECT CMFCTabCtrl::OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	return OnDragOver(pDataObject, dwKeyState, point);
}

DROPEFFECT CMFCTabCtrl::OnDragOver(COleDataObject* /*pDataObject*/, DWORD /*dwKeyState*/, CPoint /*point*/)
{
	return DROPEFFECT_NONE;
}

int CMFCTabCtrl::GetTabFromPoint(CPoint& pt) const
{
	ASSERT_VALID(this);

	if (!m_rectTabsArea.PtInRect(pt))
	{
		return -1;
	}

	if (!m_bIsOneNoteStyle && !m_bIsVS2005Style && !m_bLeftRightRounded)
	{
		return CMFCBaseTabCtrl::GetTabFromPoint(pt);
	}

	//------------------------
	// Check active tab first:
	//------------------------
	if (m_iActiveTab >= 0)
	{
		CMFCTabInfo* pActiveTab = (CMFCTabInfo*) m_arTabs [m_iActiveTab];
		ASSERT_VALID(pActiveTab);

		CRect rectTab = pActiveTab->m_rect;

		if (rectTab.PtInRect(pt))
		{
			if (m_iActiveTab > 0 && pt.x < rectTab.left + rectTab.Height())
			{
				const int x = pt.x - rectTab.left;
				const int y = pt.y - rectTab.top;

				if (x * x + y * y < rectTab.Height() * rectTab.Height() / 2)
				{
					for (int i = m_iActiveTab - 1; i >= 0; i--)
					{
						CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [i];
						ASSERT_VALID(pTab);

						if (pTab->m_bVisible)
						{
							return i;
						}
					}
				}
			}

			return m_iActiveTab;
		}
	}

	for (int i = 0; i < m_iTabsNum; i++)
	{
		CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [i];
		ASSERT_VALID(pTab);

		if (pTab->m_bVisible && pTab->m_rect.PtInRect(pt))
		{
			return i;
		}
	}

	return -1;
}

void CMFCTabCtrl::SwapTabs(int nFisrtTabID, int nSecondTabID)
{
	ASSERT_VALID(this);

	CMFCBaseTabCtrl::SwapTabs(nFisrtTabID, nSecondTabID);

	if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
	{
		m_nTabsHorzOffset = 0;
		m_nFirstVisibleTab = 0;
	}
}

void CMFCTabCtrl::MoveTab(int nSource, int nDest)
{
	ASSERT_VALID(this);

	CMFCBaseTabCtrl::MoveTab(nSource, nDest);

	if (m_bIsOneNoteStyle || m_bIsVS2005Style || m_bLeftRightRounded)
	{
		m_nTabsHorzOffset = 0;
		m_nFirstVisibleTab = 0;

		EnsureVisible(m_iActiveTab);
	}
}

void CMFCTabCtrl::EnableTabDocumentsMenu(BOOL bEnable/* = TRUE*/)
{
	ASSERT_VALID(this);

	if (m_bFlat && !m_bScroll)
	{
		ASSERT(FALSE);
		return;
	}

	m_bTabDocumentsMenu = bEnable;

	CString str;
	ENSURE(str.LoadString(m_bTabDocumentsMenu ? IDS_AFXBARRES_OPENED_DOCS : IDP_AFXBARRES_SCROLL_RIGHT));

	if (m_bScroll)
	{
		m_btnScrollRight.SetTooltip(str);
	}

	SetScrollButtons();

	RecalcLayout();

	m_nTabsHorzOffset = 0;
	m_nFirstVisibleTab = 0;

	if (m_iActiveTab >= 0)
	{
		EnsureVisible(m_iActiveTab);
	}
}

void CMFCTabCtrl::EnableActiveTabCloseButton(BOOL bEnable/* = TRUE*/)
{
	ASSERT_VALID(this);

	m_bActiveTabCloseButton = bEnable;

	RecalcLayout();

	if (m_iActiveTab >= 0)
	{
		EnsureVisible(m_iActiveTab);
	}
}

void CMFCTabCtrl::OnShowTabDocumentsMenu(CPoint point)
{
	if (afxContextMenuManager == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	const UINT idStart = (UINT) -100;

	CMenu menu;
	menu.CreatePopupMenu();

	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [i];
		ASSERT_VALID(pTab);

		if (!pTab->m_bVisible)
		{
			continue;
		}

		const UINT uiID = idStart - i;
		CString strTabName = pTab->m_strText;

		//--------------------------------
		// Replace all single '&' by '&&':
		//--------------------------------
		const CString strDummyAmpSeq = _T("\001\001");

		strTabName.Replace(_T("&&"), strDummyAmpSeq);
		strTabName.Replace(_T("&"), _T("&&"));
		strTabName.Replace(strDummyAmpSeq, _T("&&"));

		// Insert sorted:
		BOOL bInserted = FALSE;

		for (UINT iMenu = 0; iMenu < menu.GetMenuItemCount(); iMenu++)
		{
			CString strMenuItem;
			menu.GetMenuString(iMenu, strMenuItem, MF_BYPOSITION);

			if (strTabName.CompareNoCase(strMenuItem) < 0)
			{
				menu.InsertMenu(iMenu, MF_BYPOSITION, uiID, strTabName);
				bInserted = TRUE;
				break;
			}
		}

		if (!bInserted)
		{
			menu.AppendMenu(MF_STRING, uiID, strTabName);
		}

		if (pTab->m_pWnd->GetSafeHwnd() != NULL)
		{
			HICON hIcon = pTab->m_pWnd->GetIcon(FALSE);
			if (hIcon == NULL)
			{
				hIcon = (HICON)(LONG_PTR) GetClassLongPtr(pTab->m_pWnd->GetSafeHwnd(), GCLP_HICONSM);
			}

			m_mapDocIcons.SetAt(uiID, hIcon);
		}
	}

	HWND hwndThis = GetSafeHwnd();

	int nMenuResult = afxContextMenuManager->TrackPopupMenu(
		menu, point.x, point.y, this);

	if (!::IsWindow(hwndThis))
	{
		return;
	}

	int iTab = idStart - nMenuResult;
	if (iTab >= 0 && iTab < m_iTabsNum)
	{
		m_bUserSelectedTab = TRUE;
		SetActiveTab(iTab);
		m_bUserSelectedTab = FALSE;
	}

	m_mapDocIcons.RemoveAll();
}

HICON __stdcall CMFCTabCtrl::GetDocumentIcon(UINT nCmdID)
{
	HICON hIcon = NULL;
	m_mapDocIcons.Lookup(nCmdID, hIcon);

	return hIcon;
}

void CMFCTabCtrl::SetTabMaxWidth(int nTabMaxWidth)
{
	m_nTabMaxWidth = nTabMaxWidth;
	RecalcLayout();
}

void CMFCTabCtrl::SetResizeMode(ResizeMode resizeMode)
{
	m_ResizeMode = resizeMode;
	RecalcLayout();
}

void CMFCTabCtrl::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos)
{
	CMFCBaseTabCtrl::OnWindowPosChanged(lpwndpos);
	if (IsMDITabGroup())
	{
		lpwndpos->hwndInsertAfter = HWND_BOTTOM;
	}
}

void CMFCTabCtrl::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos)
{
	CMFCBaseTabCtrl::OnWindowPosChanging(lpwndpos);
	if (IsMDITabGroup())
	{
		lpwndpos->hwndInsertAfter = HWND_BOTTOM;
	}
}

void CMFCTabCtrl::DrawResizeDragRect(CRect& rectNew, CRect& rectOld)
{
	CWindowDC dc(GetDesktopWindow());
	CSize size;

	if (m_ResizeMode == RESIZE_VERT)
	{
		size.cx = size.cy = m_rectResizeDrag.Width() / 2 + 1;
	}
	else
	{
		size.cx = size.cy = m_rectResizeDrag.Height() / 2 + 1;
	}

	dc.DrawDragRect(rectNew, size, rectOld, size);
}

BOOL CMFCTabCtrl::IsMDITabGroup() const
{
	CWnd* pParent = GetParent();
	if (pParent != NULL)
	{
		ASSERT_VALID(pParent);
		return pParent->IsKindOf(RUNTIME_CLASS(CMDIClientAreaWnd));
	}
	return FALSE;
}

void CMFCTabCtrl::ActivateMDITab(int nTab)
{
	ASSERT(IsMDITabGroup());

	if (nTab == -1)
	{
		nTab = m_iActiveTab;
	}

	if (nTab == -1)
	{
		return;
	}

	CWnd* pActiveWnd = GetTabWnd(nTab);
	if (pActiveWnd != NULL)
	{
		ASSERT_VALID(pActiveWnd);

		if (nTab != m_iActiveTab)
		{
			if (!SetActiveTab(nTab))
			{
				return;
			}
		}

		GetParent()->SendMessage(WM_MDIACTIVATE, (WPARAM) pActiveWnd->GetSafeHwnd());
		pActiveWnd->SetFocus();
	}
}

LRESULT CMFCTabCtrl::OnUpdateToolTips(WPARAM wp, LPARAM)
{
	UINT nTypes = (UINT) wp;

	if ((nTypes & AFX_TOOLTIP_TYPE_TAB) == 0)
	{
		return 0;
	}

	CTooltipManager::CreateToolTip(m_pToolTip, this, AFX_TOOLTIP_TYPE_TAB);

	if (m_pToolTip->GetSafeHwnd() == NULL)
	{
		return 0;
	}

	CRect rectDummy(0, 0, 0, 0);

	CTooltipManager::CreateToolTip(m_pToolTipClose, this, AFX_TOOLTIP_TYPE_TAB);

	if (m_pToolTipClose->GetSafeHwnd() != NULL)
	{
		m_pToolTipClose->AddTool(this, LPSTR_TEXTCALLBACK, rectDummy, 1);
	}

	for (int i = 0; i < m_iTabsNum; i++)
	{
		CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [i];
		ASSERT_VALID(pTab);

		m_pToolTip->AddTool(this, m_bCustomToolTips ? LPSTR_TEXTCALLBACK :(LPCTSTR)(pTab->m_strText), &rectDummy, pTab->m_iTabID);
	}

	RecalcLayout();
	return 0;
}


