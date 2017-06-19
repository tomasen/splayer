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

#include "afxribbonres.h"
#include "afxglobals.h"
#include "afxglobalutils.h"
#include "afxframewndex.h"
#include "afxmdiframewndex.h"
#include "afxoleipframewndex.h"
#include "afxoledocipframewndex.h"
#include "afxmdichildwndex.h"
#include "afxolecntrframewndex.h"

#include "afxpane.h"
#include "afxdockablepane.h"

#include "afxtoolbar.h"
#include "afxtoolbarbutton.h"
#include "afxcolorbar.h"
#include "afxpaneframewnd.h"

#include "afxdocksite.h"
#include "afxdockingmanager.h"

#include "afxtabbedpane.h"
#include "afxvisualmanager.h"

#include "afxtoolbarmenubutton.h"

#include "afxcaptionmenubutton.h"
#include "afxcustomizebutton.h"

#include "afxtooltipmanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int CPaneFrameWnd::m_nCaptionButtonMargin = 0;
int CPaneFrameWnd::m_nCaptionButtonSpacing = 4;
const int CPaneFrameWnd::m_nToolbarBorderSize = 3;
UINT CPaneFrameWnd::m_nRollTimeOut = 250;
BOOL CPaneFrameWnd::m_bUseSaveBits = TRUE;

UINT AFX_WM_CHECKEMPTYMINIFRAME = ::RegisterWindowMessage(_T("MSG_CHECKEMPTYMINIFRAME"));

IMPLEMENT_SERIAL(CPaneFrameWnd, CWnd, VERSIONABLE_SCHEMA | 2)

/////////////////////////////////////////////////////////////////////////////
// CPaneFrameWnd

CMap<UINT,UINT,HWND,HWND> CPaneFrameWnd::m_mapFloatingBars;
CList<HWND, HWND> CPaneFrameWnd::m_lstFrames;
CFrameWnd* CPaneFrameWnd::m_pParentWndForSerialize = NULL;

CPaneFrameWnd::CPaneFrameWnd()
{
	m_bActive = FALSE;
	m_bCaptured = false;
	m_bBlockMove = false;
	m_bNoDelayedDestroy = FALSE;
	m_bDelayShow = FALSE;

	m_preDockStateCurr = PDS_NOTHING;
	m_pPreDockBar = NULL;
	m_bTabDragRectDisplayed = false;

	RecalcCaptionHeight();

	m_nHit = HTNOWHERE;
	m_nHot = HTNOWHERE;
	m_rectRedraw.SetRectEmpty();
	m_nDockTimerID = 0;
	m_nRollTimerID = 0;
	m_bRolledUp = FALSE;
	m_nHeightBeforeRollUp = 0;
	m_bPinned = FALSE;

	m_hEmbeddedBar = NULL;

	m_nRestoredEmbeddedBarID = 0;

	m_dwCaptionButtons = 0;
	m_rectRecentFloatingRect.SetRect(0, 0, 100, 100);

	m_hParentWnd = NULL;
	m_hWndToDestroyOnRelease = NULL;
	m_hLastFocusWnd = NULL;

	m_bIsMoving = FALSE;
	m_ptHot.x = m_ptHot.y = -1;

	m_bHostsToolbar = TRUE;
	m_pDockManager = NULL;

	m_pToolTip = NULL;
}

CPaneFrameWnd::~CPaneFrameWnd()
{
	RemoveAllCaptionButtons();
}

//{{AFX_MSG_MAP(CPaneFrameWnd)
BEGIN_MESSAGE_MAP(CPaneFrameWnd, CWnd)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOVING()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_NCHITTEST()
	ON_WM_SETCURSOR()
	ON_WM_SIZING()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_NCACTIVATE()
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_NCCREATE()
	ON_WM_ERASEBKGND()
	ON_WM_NCDESTROY()
	ON_WM_CANCELMODE()
	ON_WM_NCMOUSEMOVE()
	ON_WM_GETMINMAXINFO()
	ON_WM_CONTEXTMENU()
	ON_WM_DESTROY()
	ON_WM_CHAR()
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_MOUSEACTIVATE()
	ON_WM_SETFOCUS()
	ON_WM_CANCELMODE()
	ON_WM_SETTINGCHANGE()
	ON_MESSAGE(WM_EXITSIZEMOVE, &CPaneFrameWnd::OnExitSizeMove)
	ON_MESSAGE(WM_IDLEUPDATECMDUI, &CPaneFrameWnd::OnIdleUpdateCmdUI)
	ON_MESSAGE(WM_FLOATSTATUS, &CPaneFrameWnd::OnFloatStatus)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, &CPaneFrameWnd::OnNeedTipText)
	ON_REGISTERED_MESSAGE(AFX_WM_CHECKEMPTYMINIFRAME, &CPaneFrameWnd::OnCheckEmptyState)
	ON_REGISTERED_MESSAGE(AFX_WM_UPDATETOOLTIPS, &CPaneFrameWnd::OnUpdateToolTips)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// CPaneFrameWnd message handlers

BOOL CPaneFrameWnd::Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, CCreateContext* pContext)
{
	return CPaneFrameWnd::CreateEx(0, lpszWindowName, dwStyle, rect, pParentWnd, pContext);
}

BOOL CPaneFrameWnd::CreateEx(DWORD dwStyleEx, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, CCreateContext* pContext)
{
	dwStyle |= WS_POPUP;

	if (pParentWnd != NULL && pParentWnd->GetExStyle() & WS_EX_LAYOUTRTL)
	{
		dwStyleEx |= WS_EX_LAYOUTRTL;
	}

	m_hParentWnd = pParentWnd != NULL ? pParentWnd->m_hWnd : NULL;

	if (!CWnd::CreateEx(dwStyleEx, afxGlobalData.RegisterWindowClass(_T("Afx:MiniFrame")), lpszWindowName, dwStyle, rect, pParentWnd, 0, pContext))
	{
		return FALSE;
	}

	if (pParentWnd != NULL)
	{
		if (DYNAMIC_DOWNCAST(CFrameWnd, pParentWnd) == NULL)
		{
			TRACE0("Minframe parent must be derived from CFrameWnd. Creation failed.\n");
			return FALSE;
		}

		// register with dock manager
		CDockingManager* pDockManager = m_pDockManager != NULL ? m_pDockManager : afxGlobalUtils.GetDockingManager(pParentWnd);
		ASSERT_VALID(pDockManager);

		if (pDockManager == NULL)
		{
			TRACE0("Minframe parent must be connected to dock manager. Creation failed.\n");
			return FALSE;
		}

		pDockManager->AddMiniFrame(this);
	}

	m_dragFrameImpl.Init(this);
	return TRUE;
}

void CPaneFrameWnd::SaveRecentFloatingState()
{
	GetWindowRect(m_rectRecentFloatingRect);
	// save the recent floating rect
	if (m_hEmbeddedBar != NULL)
	{
		CPane* pContainedBar = DYNAMIC_DOWNCAST(CPane, CWnd::FromHandlePermanent(m_hEmbeddedBar));
		if (pContainedBar != NULL)
		{
			pContainedBar->m_recentDockInfo.m_rectRecentFloatingRect = m_rectRecentFloatingRect;

			CPoint ptClientHotSpot;
			GetCursorPos(&ptClientHotSpot);

			pContainedBar->ScreenToClient(&ptClientHotSpot);
			pContainedBar->SetClientHotSpot(ptClientHotSpot);
		}
	}
}

void CPaneFrameWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	// Close active popup menu:
	if (CMFCPopupMenu::GetActiveMenu() != NULL && ::IsWindow(CMFCPopupMenu::GetActiveMenu()->m_hWnd))
	{
		CMFCPopupMenu::GetActiveMenu()->SendMessage(WM_CLOSE);
	}

	m_bIsMoving = FALSE;
	if (m_nHot != HTNOWHERE)
	{
		CMFCCaptionButton* pBtn = FindButton(m_nHot);
		if (pBtn != NULL)
		{
			m_nHit = m_nHot;
			pBtn->m_bPushed = TRUE;
			RedrawCaptionButton(pBtn);
		}

		CWnd::OnLButtonDown(nFlags, point);
		return;
	}

	EnterDragMode();

	SaveRecentFloatingState();

	CPane* pContainedBar = DYNAMIC_DOWNCAST(CPane, CWnd::FromHandlePermanent(m_hEmbeddedBar));
	if (pContainedBar != NULL)
	{
		pContainedBar->m_bWasFloatingBeforeMove = TRUE;
		if (!pContainedBar->IsKindOf(RUNTIME_CLASS(CMFCToolBar)))
		{
			SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		}
	}
	else
	{
		SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	}

	CWnd::OnLButtonDown(nFlags, point);
}

int CPaneFrameWnd::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	if (m_hEmbeddedBar != NULL && IsWindow(m_hEmbeddedBar))
	{
		CWnd* pWnd = CWnd::FromHandle(m_hEmbeddedBar);
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);
		LRESULT nHit = HitTest(pt, TRUE);
		if (pWnd->IsKindOf(RUNTIME_CLASS(CMFCToolBar)) && nHit != HTCLIENT)
		{
			SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
			return MA_NOACTIVATE;
		}
	}

	return CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
}

void CPaneFrameWnd::EnterDragMode(HWND hWndToDestroyOnRelease)
{
	if (!m_bCaptured)
	{
		SetCapture();
		if (m_hWndToDestroyOnRelease == NULL)
		{
			m_hWndToDestroyOnRelease = hWndToDestroyOnRelease;
		}

		m_bCaptured = true;

		OnCapture(TRUE);

		GetCursorPos(&m_dragFrameImpl.m_ptHot);

		if ((GetDockingMode() & DT_IMMEDIATE) != 0 &&
			(GetDockingMode() & DT_SMART) == 0)
		{
			SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEALL));
		}

		GetCursorPos(&m_ptHot);

		if (m_pDockManager != NULL)
		{
			m_pDockManager->ResortMiniFramesForZOrder();
		}
		else
		{
			afxGlobalUtils.GetDockingManager(this)->ResortMiniFramesForZOrder();
		}
	}
}

void CPaneFrameWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bIsMoving = FALSE;
	// m_bCaptured is true when the miniframe is being dragged
	if (m_nHit != HTNOWHERE && !m_bCaptured)
	{
		UINT nHot = m_nHot;
		UINT nHit = m_nHit;

		StopCaptionButtonsTracking();

		if (nHot == nHit)
		{
			switch (nHit)
			{
			case HTCLOSE:
				if (OnCloseMiniFrame())
				{
					CloseMiniFrame();
					return;
				}
				break;

			case HTMAXBUTTON:
				m_bPinned = !m_bPinned;
				break;

			default:
				if (!CMFCToolBar::IsCustomizeMode())
				{
					OnPressButtons(nHit);
				}
			}
		}

		CWnd::OnLButtonUp(nFlags, point);
		return;
	}

	if (m_bCaptured)
	{
		ReleaseCapture();
		m_bCaptured = false;
		OnCapture(FALSE);
		BOOL bWasDocked = FALSE;

		CDockingManager* pDockManager = m_pDockManager != NULL ? m_pDockManager : afxGlobalUtils.GetDockingManager(this);
		CSmartDockingManager* pSDManager = pDockManager->GetSmartDockingManagerPermanent();
		BOOL bCtrlHeld = (GetKeyState(VK_CONTROL) < 0);
		if (!bCtrlHeld && pSDManager != NULL && pSDManager->IsStarted())
		{
			CSmartDockingStandaloneGuide::SDMarkerPlace marker = pSDManager->GetHighlightedGuideNo();
			if (marker == CSmartDockingStandaloneGuide::sdCMIDDLE)
			{
				CMDIFrameWndEx* pFrameWnd = DYNAMIC_DOWNCAST(CMDIFrameWndEx, pDockManager->GetDockSiteFrameWnd());
				if (pFrameWnd != NULL && pFrameWnd->AreMDITabs())
				{
					CPoint ptScreen;
					GetCursorPos(&ptScreen);
					CPaneFrameWnd* pOtherMiniFrameWnd = pDockManager->FrameFromPoint(ptScreen, this, FALSE);
					if (pOtherMiniFrameWnd == NULL)
					{
						CDockablePane* pThisControlBar = DYNAMIC_DOWNCAST(CDockablePane, GetFirstVisiblePane());
						CDockablePane* pOtherBar = DYNAMIC_DOWNCAST(CDockablePane,
							pDockManager->PaneFromPoint(ptScreen, CDockingManager::m_nDockSensitivity, true, NULL, TRUE, pThisControlBar));
						if (pOtherBar == NULL && pThisControlBar != NULL)
						{
							ConvertToTabbedDocument();
							m_dragFrameImpl.ResetState();
							pDockManager->StopSDocking();
							return;
						}
					}
				}
			}
		}

		if (DockPane(bWasDocked) || bWasDocked)
		{
			if (bWasDocked)
			{
				afxGlobalUtils.ForceAdjustLayout(pDockManager);
			}
			return;
		}

		m_dragFrameImpl.ResetState();
	}

	CWnd::OnLButtonUp(nFlags, point);
}

void CPaneFrameWnd::ConvertToTabbedDocument()
{
	CDockablePane* pThisControlBar = DYNAMIC_DOWNCAST(CDockablePane, GetFirstVisiblePane());
	if (pThisControlBar != NULL)
	{
		pThisControlBar->ConvertToTabbedDocument(FALSE);
		PostMessage(AFX_WM_CHECKEMPTYMINIFRAME);
		return;
	}
}

CDockablePane* CPaneFrameWnd::DockPane(BOOL& bWasDocked)
{
	CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, GetPane());

	bWasDocked = FALSE;

	AFX_DOCK_TYPE docktype = GetDockingMode();
	CDockingManager* pDockManager = m_pDockManager != NULL ? m_pDockManager : afxGlobalUtils.GetDockingManager(this);
	if (pDockManager == NULL)
	{
		return NULL;
	}

	CSmartDockingManager* pSDManager = pDockManager->GetSmartDockingManagerPermanent();

	if ((docktype & DT_IMMEDIATE) != 0 &&((docktype & DT_SMART) == 0 || pSDManager == NULL || !pSDManager->IsStarted()))
	{
		if (m_preDockStateCurr == PDS_DOCK_REGULAR)
		{
			ASSERT_VALID(pBar);
			bWasDocked = pBar->DockByMouse(m_pPreDockBar);
		}
		else if (m_preDockStateCurr == PDS_DOCK_TO_TAB && m_pPreDockBar != NULL )
		{
			CDockablePane* pTabControlBarAttachTo = DYNAMIC_DOWNCAST(CDockablePane, m_pPreDockBar);
			if (pBar == NULL || pTabControlBarAttachTo == NULL)
			{
				ASSERT(FALSE);
				TRACE0("Attempt to attach a control bar that is not derived from CDockablePane to tab window. \r\n");
			}
			else
			{
				bWasDocked = TRUE;

				pDockManager = m_pDockManager != NULL ? m_pDockManager : afxGlobalUtils.GetDockingManager(GetParent());
				ENSURE(pDockManager != NULL);

				pDockManager->StopSDocking();

				return pBar->AttachToTabWnd(pTabControlBarAttachTo, DM_MOUSE);
			}
		}

		pDockManager->StopSDocking();

		m_preDockStateCurr = PDS_NOTHING;
		return pBar;
	}
	else if ((docktype & DT_STANDARD) != 0 ||(docktype & DT_SMART) != 0)
	{
		CRect rectFinal = m_dragFrameImpl.m_rectDrag;

		if (pBar != NULL)
		{
			// it was a docking control bar, for toolbars pBar will be NULL
			m_dragFrameImpl.EndDrawDragFrame();
		}

		CFrameWnd* pFrameWnd = AFXGetParentFrame(this);

		BOOL bPrevDisableRecalcLayout = CDockingManager::m_bDisableRecalcLayout;
		CDockingManager::m_bDisableRecalcLayout = TRUE;
		CDockablePane* pDockedBar = pBar;

		pDockedBar = DockPaneStandard(bWasDocked);

		CDockingManager::m_bDisableRecalcLayout = bPrevDisableRecalcLayout;

		if (pDockManager)
		{
			pDockManager->StopSDocking();
		}

		if (pFrameWnd != NULL)
		{
			pFrameWnd->RecalcLayout();
		}

		if (pDockManager)
		{
			pDockManager->AdjustDockingLayout();
		}

		if (!rectFinal.IsRectEmpty() && (pDockedBar == NULL || pDockedBar->GetParentMiniFrame() == this) && !bWasDocked)
		{
			SetWindowPos(NULL, rectFinal.left, rectFinal.top, rectFinal.Width(), rectFinal.Height(), SWP_NOZORDER | SWP_NOCOPYBITS);

			if (!IsWindowVisible() && GetVisiblePaneCount() > 0)
			{
				ShowWindow(SW_SHOW);
			}
		}
		return pDockedBar;
	}
	return NULL;
}

CDockablePane* CPaneFrameWnd::DockPaneStandard(BOOL& bWasDocked)
{
	CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, GetPane());

	if (pBar == NULL || !OnBeforeDock())
	{
		return NULL;
	}

	if (!pBar->IsWindowVisible() &&(pBar->GetDockingMode() & DT_STANDARD) != 0)
	{
		pBar->ShowWindow(SW_SHOW);
	}

	return DYNAMIC_DOWNCAST(CDockablePane, pBar->DockPaneStandard(bWasDocked));
}

void CPaneFrameWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bBlockMove)
	{
		m_bBlockMove = false;
		return;
	}

	if (m_bCaptured)
	{
		if (!m_bIsMoving)
		{
			CPoint ptCurrent;
			GetCursorPos(&ptCurrent);

			if (ptCurrent.x != m_ptHot.x || ptCurrent.y != m_ptHot.y)
			{
				m_bIsMoving = TRUE;

				CDockablePane* pFirstBar = DYNAMIC_DOWNCAST(CDockablePane, GetFirstVisiblePane());

				if ((GetDockingMode() & DT_SMART) != 0 && pFirstBar != NULL && (pFirstBar->GetEnabledAlignment() & CBRS_ALIGN_ANY) != 0)
				{
					CDockingManager* pDockManager = m_pDockManager != NULL ? m_pDockManager : afxGlobalUtils.GetDockingManager(GetParent());
					if (pDockManager != NULL)
					{
						pDockManager->StartSDocking(this);
					}
				}
			}
			else
			{
				return;
			}
		}

		afxGlobalUtils.m_bIsDragging = TRUE;

		AFX_DOCK_TYPE docktype = GetDockingMode();

		CSmartDockingManager* pSDManager = NULL;
		CDockingManager* pDockManager = NULL;
		if ((docktype & DT_SMART) != 0)
		{
			pDockManager = m_pDockManager != NULL ? m_pDockManager : afxGlobalUtils.GetDockingManager(GetParent());
			if (pDockManager != NULL)
			{
				CPoint pt(point);
				ClientToScreen(&pt);

				pSDManager = pDockManager->GetSmartDockingManager();
				if (pSDManager != NULL)
				{
					pSDManager->OnMouseMove(pt);
				}
			}
		}

		if ((docktype & DT_STANDARD) != 0)
		{
			m_dragFrameImpl.MoveDragFrame();
		}
		else if ((docktype & DT_IMMEDIATE) != 0)
		{
			CPoint ptScreen = point;
			ClientToScreen(&ptScreen);
			BOOL bSDockingIsOn = (docktype & DT_SMART) != 0 && pSDManager != NULL && pSDManager->IsStarted();
			CSmartDockingStandaloneGuide::SDMarkerPlace nHilitedSideNo = bSDockingIsOn ? pSDManager->GetHighlightedGuideNo() : CSmartDockingStandaloneGuide::sdNONE;
			BOOL bCtrlHeld = (GetKeyState(VK_CONTROL) < 0);
			BOOL bOnCaption = FALSE;
			CPaneFrameWnd* pOtherMiniFrameWnd = NULL;
			if (bSDockingIsOn)
			{
				CBasePane* pThisControlBar = NULL;
				CDockablePane* pBar = NULL;
				CWnd* pBarToPlaceTo = NULL;

				if (!bCtrlHeld)
				{
					pOtherMiniFrameWnd = pDockManager->FrameFromPoint(ptScreen, this, FALSE);
					pThisControlBar = DYNAMIC_DOWNCAST(CBasePane, GetPane());
					if (pOtherMiniFrameWnd != NULL)
					{
						CDockablePane* pOtherDockingControlBar = DYNAMIC_DOWNCAST(CDockablePane, pOtherMiniFrameWnd->GetFirstVisiblePane());
						if (pOtherDockingControlBar != NULL && pOtherDockingControlBar->CanBeAttached() && pThisControlBar->CanBeAttached() &&
							pOtherDockingControlBar->CanAcceptMiniFrame(this) && afxGlobalUtils.CanPaneBeInFloatingMultiPaneFrameWnd(pThisControlBar) &&
							pOtherDockingControlBar->GetEnabledAlignment() == pThisControlBar->GetEnabledAlignment())
						{
							CRect rcCaption;
							pOtherMiniFrameWnd->GetCaptionRect(rcCaption);
							CRect rcWnd;
							pOtherMiniFrameWnd->GetWindowRect(rcWnd);
							rcCaption.OffsetRect(rcWnd.TopLeft());
							if (rcCaption.PtInRect(ptScreen))
							{
								bOnCaption = TRUE;
								pBarToPlaceTo = pOtherDockingControlBar;
							}
						}
					}

					if (!bOnCaption)
					{
						pBar = DYNAMIC_DOWNCAST(CDockablePane, pDockManager->PaneFromPoint(ptScreen, CDockingManager::m_nDockSensitivity, true, NULL, TRUE, pThisControlBar));
						if (pBar != NULL)
						{
							BOOL bCanBeTabbed = (pBar->GetParentMiniFrame() != NULL && afxGlobalUtils.CanPaneBeInFloatingMultiPaneFrameWnd(pThisControlBar) || pBar->GetParentMiniFrame() == NULL) &&
								(pBar->GetEnabledAlignment() == pThisControlBar->GetEnabledAlignment() && pBar->CanAcceptPane(pThisControlBar) && pThisControlBar->CanBeAttached() && pBar->CanBeAttached());
							CRect rcWnd;
							pBar->GetWindowRect(&rcWnd);
							if (rcWnd.PtInRect(ptScreen) &&(ptScreen.y - rcWnd.top) < pBar->GetCaptionHeight())
							{
								bOnCaption = bCanBeTabbed;
								pBarToPlaceTo = pBar;
							}
							else
							{
								CRect rcTabsTop;
								CRect rcTabsBottom;
								pBar->GetTabArea(rcTabsTop, rcTabsBottom);
								if (rcTabsTop.PtInRect(ptScreen) || rcTabsBottom.PtInRect(ptScreen))
								{
									bOnCaption = bCanBeTabbed;
									pBarToPlaceTo = pBar;
								}
							}
						}
					}
				}

				if (bCtrlHeld)
				{
					pSDManager->Show(FALSE);
				}
				else if (bOnCaption)
				{
					pSDManager->Show(FALSE);
					m_dragFrameImpl.PlaceTabPreDocking(pBarToPlaceTo);
				}
				else
				{
					pSDManager->Show(TRUE);
					BOOL bShowCentralGroup = FALSE;
					if (pBar != NULL)
					{
						BOOL bCanFloatMulti = afxGlobalUtils.CanPaneBeInFloatingMultiPaneFrameWnd(pThisControlBar);
						BOOL bIsTargetBarFloatingMulti = pBar->IsInFloatingMultiPaneFrameWnd();

						if (bIsTargetBarFloatingMulti && !bCanFloatMulti)
						{
						}
						else
							if (pBar->CanAcceptPane(pThisControlBar) && ((pThisControlBar->GetEnabledAlignment() & pBar->GetCurrentAlignment()) != 0 &&
								pBar->GetDefaultPaneDivider() != NULL || pBar->GetParentMiniFrame() != NULL && pThisControlBar->GetEnabledAlignment() == pBar->GetEnabledAlignment()))
							{
								int nShowMiddleMarker = 0;
								if (CanBeAttached() && pBar->CanBeAttached() && pThisControlBar->GetEnabledAlignment() == pBar->GetEnabledAlignment())
								{
									nShowMiddleMarker = 1;
								}
								CRect rcBar;
								pBar->GetWindowRect(rcBar);

								DWORD dwEnabledAlignment = CDockingManager::m_bIgnoreEnabledAlignment ? CBRS_ALIGN_ANY : pThisControlBar->GetEnabledAlignment();

								pSDManager->MoveCentralGroup(rcBar, nShowMiddleMarker, dwEnabledAlignment);
								pSDManager->ShowCentralGroup(TRUE);
								bShowCentralGroup = TRUE;
							}
					}
					if (!bShowCentralGroup)
					{
						CRect rcClient;
						pDockManager->GetClientAreaBounds(rcClient);
						GetParent()->ClientToScreen(&rcClient);
						if (rcClient.PtInRect(ptScreen))
						{
							int nShowMiddleMarker = 0;

							CMDIFrameWndEx* pDockSite = DYNAMIC_DOWNCAST(CMDIFrameWndEx, pThisControlBar->GetDockSiteFrameWnd());
							if (pDockSite != NULL && pDockSite->AreMDITabs() && pDockSite->CanConvertControlBarToMDIChild() && pThisControlBar->CanBeTabbedDocument())
							{
								nShowMiddleMarker = 1;
							}

							pSDManager->MoveCentralGroup(rcClient, nShowMiddleMarker, pThisControlBar->GetEnabledAlignment());
							pSDManager->ShowCentralGroup(TRUE, pThisControlBar->GetEnabledAlignment());
							bShowCentralGroup = TRUE;
						}
					}
					if (!bShowCentralGroup && !(nHilitedSideNo >= CSmartDockingStandaloneGuide::sdLEFT && nHilitedSideNo <= CSmartDockingStandaloneGuide::sdBOTTOM))
					{
						pSDManager->ShowCentralGroup(FALSE, pThisControlBar->GetEnabledAlignment());
					}
				}
			}

			AFX_DOCK_TYPE dockType = GetDockingMode();

			if (bSDockingIsOn
				&& nHilitedSideNo != CSmartDockingStandaloneGuide::sdNONE
				&& !bCtrlHeld)
			{
				CPoint ptLastHot = m_dragFrameImpl.m_ptHot;   // save
				m_dragFrameImpl.MoveDragFrame();
				m_dragFrameImpl.m_ptHot = ptLastHot;  // restore
			}
			else
			{
				if (bSDockingIsOn && !bOnCaption)
				{
					m_dragFrameImpl.RemoveTabPreDocking();
					pSDManager->HidePlace();
				}

				if (!bOnCaption && MoveMiniFrame())
				{
					CPoint ptMouse;
					GetCursorPos(&ptMouse);

					CPoint ptOffset = ptMouse - m_dragFrameImpl.m_ptHot;

					CRect rect;
					GetWindowRect(&rect);
					rect.OffsetRect(ptOffset);
					//MoveWindow(rect);
					SetWindowPos(NULL, rect.left, rect.top, -1, -1, SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);

					m_dragFrameImpl.m_ptHot = ptMouse;
					m_dragFrameImpl.m_rectDrag = rect;

				}
			}

			if ((dockType & DT_SMART) == 0)
			{
				SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEALL));
			}
		}

		afxGlobalUtils.m_bIsDragging = FALSE;
		return;
	}

	CPoint ptScreen = point;
	ClientToScreen(&ptScreen);

	OnTrackCaptionButtons(ptScreen);
	CWnd::OnMouseMove(nFlags, point);
}

void CPaneFrameWnd::MoveDragFrame()
{
	m_dragFrameImpl.MoveDragFrame(TRUE);
}

void CPaneFrameWnd::OnMoving(UINT fwSide, LPRECT pRect)
{
	CWnd::OnMoving(fwSide, pRect);
	MoveMiniFrame();
}

void CPaneFrameWnd::OnCheckRollState()
{
	ASSERT_VALID(this);

	CDockingManager* pDockManager = m_pDockManager != NULL ? m_pDockManager : afxGlobalUtils.GetDockingManager(GetParent());
	ASSERT_VALID(pDockManager);

	if (pDockManager->m_bLockUpdate)
	{
		return;
	}

	CRect rectClient;
	CRect rectWnd;

	GetWindowRect(rectWnd);
	GetClientRect(rectClient);

	BOOL bRollDown = IsRollDown();
	BOOL bRollUp   = IsRollUp();

	if (m_bRolledUp && bRollDown)
	{
		rectWnd.bottom = rectWnd.top + m_nHeightBeforeRollUp;
		SetWindowPos(NULL, rectWnd.left,  rectWnd.top, rectWnd.Width(), rectWnd.Height(), SWP_NOZORDER  | SWP_NOACTIVATE);
		m_bRolledUp = FALSE;
	}

	if (!m_bRolledUp && bRollUp)
	{
		m_nHeightBeforeRollUp = rectWnd.Height();
		rectWnd.bottom -= rectClient.Height();
		SetWindowPos(NULL, rectWnd.left,  rectWnd.top, rectWnd.Width(), rectWnd.Height(), SWP_NOZORDER  | SWP_NOACTIVATE);
		m_bRolledUp = TRUE;
	}
}

BOOL CPaneFrameWnd::IsRollDown() const
{
	CPoint ptMouse;
	GetCursorPos(&ptMouse);

	CRect rectWnd;
	GetWindowRect(rectWnd);

	const CWnd* pWnd = WindowFromPoint(ptMouse);
	return rectWnd.PtInRect(ptMouse) &&(pWnd->GetSafeHwnd() == GetSafeHwnd());
}

BOOL CPaneFrameWnd::IsRollUp() const
{
	CPoint ptMouse;
	GetCursorPos(&ptMouse);

	CRect rectWnd;
	GetWindowRect(rectWnd);

	return !rectWnd.PtInRect(ptMouse) && !m_bPinned;
}

void CPaneFrameWnd::AddPane(CBasePane* pWnd)
{
	ASSERT_VALID(pWnd);

	m_bHostsToolbar = pWnd->IsKindOf(RUNTIME_CLASS(CMFCToolBar));
	if (m_hEmbeddedBar != pWnd->GetSafeHwnd())
	{
		m_hEmbeddedBar = pWnd->GetSafeHwnd();

		CString strText;
		pWnd->GetWindowText(strText);
		SetWindowText(strText);

		SetIcon(pWnd->GetIcon(FALSE), FALSE);
		SetIcon(pWnd->GetIcon(TRUE), TRUE);

		AddRemovePaneFromGlobalList(pWnd, TRUE);
		if (pWnd->CanBeClosed())
		{
			if (m_bHostsToolbar)
			{
				CMFCToolBar* pWndToolBar = (CMFCToolBar*)pWnd;
				if (pWndToolBar->IsExistCustomizeButton() && pWndToolBar->IsAddRemoveQuickCustomize())
				{
					SetCaptionButtons(AFX_CAPTION_BTN_CLOSE|AFX_CAPTION_BTN_CUSTOMIZE);
				}
				else
				{
					SetCaptionButtons(AFX_CAPTION_BTN_CLOSE);
				}

			}
			else
			{
				SetCaptionButtons(AFX_CAPTION_BTN_CLOSE);
			}
		}

		if (pWnd->IsKindOf(RUNTIME_CLASS(CMFCMenuBar)))
		{
			CMFCToolBar* pWndToolBar = (CMFCToolBar*)pWnd;
			if (pWndToolBar->IsExistCustomizeButton())
			{
				SetCaptionButtons(AFX_CAPTION_BTN_CUSTOMIZE);
			}

		}
		OnSetRollUpTimer();
	}
}

void CPaneFrameWnd::OnSetRollUpTimer()
{
	CBasePane* pBar = DYNAMIC_DOWNCAST(CBasePane, GetPane());
	if (pBar != NULL && pBar->GetControlBarStyle() & AFX_CBRS_AUTO_ROLLUP)
	{
		SetRollUpTimer();
	}
}

void CPaneFrameWnd::OnKillRollUpTimer()
{
	CBasePane* pBar = DYNAMIC_DOWNCAST(CBasePane, GetPane());

	if (pBar != NULL &&(pBar->GetControlBarStyle() & AFX_CBRS_AUTO_ROLLUP) == 0 || pBar == NULL)
	{
		KillRollupTimer();
	}
}

void CPaneFrameWnd::SetRollUpTimer()
{
	if (m_nRollTimerID == 0)
	{
		m_nRollTimerID = (UINT) SetTimer(AFX_CHECK_ROLL_STATE, m_nRollTimeOut, NULL);
		SetCaptionButtons(m_dwCaptionButtons | AFX_CAPTION_BTN_PIN);
	}
}

void CPaneFrameWnd::KillRollupTimer()
{
	if (m_nRollTimerID != 0)
	{
		KillTimer(m_nRollTimerID);
		m_nRollTimerID = 0;
		DWORD dwButtons = m_dwCaptionButtons &(~AFX_CAPTION_BTN_PIN);
		SetCaptionButtons(dwButtons);
	}
}

void CPaneFrameWnd::RemovePane(CBasePane* pWnd, BOOL bDestroy, BOOL bNoDelayedDestroy)
{
	ASSERT_VALID(this);

	m_bNoDelayedDestroy = bNoDelayedDestroy;

	AddRemovePaneFromGlobalList(pWnd, FALSE);

	pWnd->OnRemoveFromMiniFrame(this);

	if (m_hEmbeddedBar == pWnd->GetSafeHwnd())
	{
		m_hEmbeddedBar = NULL;
	}

	OnKillRollUpTimer();

	if (GetPaneCount() == 0)
	{
		if (bDestroy)
		{
			DestroyWindow();
		}
		else
		{
			PostMessage(AFX_WM_CHECKEMPTYMINIFRAME);
		}
	}
}

BOOL __stdcall CPaneFrameWnd::AddRemovePaneFromGlobalList(CBasePane* pWnd, BOOL bAdd)
{
	ASSERT_VALID(pWnd);

	int nID = pWnd->GetDlgCtrlID();

	if (nID != -1)
	{
		bAdd ? m_mapFloatingBars.SetAt(nID, pWnd->GetSafeHwnd()) : m_mapFloatingBars.RemoveKey(nID);
	}
	else if (pWnd->IsKindOf(RUNTIME_CLASS(CBaseTabbedPane)))
	{
		CBaseTabbedPane* pTabbedBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, pWnd);

		int nTabsNum = pTabbedBar->GetTabsNum();

		for (int i = 0; i < nTabsNum; i++)
		{
			CWnd* pNextWnd = pTabbedBar->FindBarByTabNumber(i, TRUE);
			ASSERT_VALID(pNextWnd);

			int nBarID = pNextWnd->GetDlgCtrlID();

			if (nBarID == -1)
			{
				TRACE0("Tabbed control bar contains a bar with ID = -1\n");
				ASSERT(FALSE);
			}
			else
			{
				bAdd ?  m_mapFloatingBars.SetAt(nBarID, pNextWnd->GetSafeHwnd()) : m_mapFloatingBars.RemoveKey(nBarID);
			}
		}
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}


void CPaneFrameWnd::ReplacePane(CBasePane* pBarOrg, CBasePane* pBarReplaceWith)
{
	ASSERT_VALID(this);
	
	ENSURE(pBarOrg != NULL);
	ENSURE(pBarReplaceWith != NULL);
	ENSURE(pBarOrg != pBarReplaceWith);

	ASSERT_VALID(pBarOrg);
	ASSERT_VALID(pBarReplaceWith);

	AddRemovePaneFromGlobalList(pBarOrg, FALSE);

	if (pBarOrg->GetSafeHwnd() == m_hEmbeddedBar)
	{
		m_hEmbeddedBar = pBarReplaceWith->GetSafeHwnd();
	}

	AddRemovePaneFromGlobalList(pBarReplaceWith, TRUE);

	OnSetRollUpTimer();
}

CWnd* CPaneFrameWnd::GetPane() const
{
	ASSERT_VALID(this);

	return CWnd::FromHandlePermanent(m_hEmbeddedBar);
}

CWnd* CPaneFrameWnd::GetFirstVisiblePane() const
{
	if (GetVisiblePaneCount() == 1)
	{
		return GetPane();
	}
	return NULL;
}

void CPaneFrameWnd::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	CRect rectBorderSize;
	CalcBorderSize(rectBorderSize);

	lpncsp->rgrc[0].top += m_nCaptionHeight + rectBorderSize.top;
	lpncsp->rgrc[0].bottom -= rectBorderSize.bottom;
	lpncsp->rgrc[0].left += rectBorderSize.left;
	lpncsp->rgrc[0].right -= rectBorderSize.right;

	CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);
}

void CPaneFrameWnd::OnNcPaint()
{
	CDockingManager* pDockManager = m_pDockManager != NULL ? m_pDockManager : afxGlobalUtils.GetDockingManager(GetParent());
	if (pDockManager == NULL)
	{
		return;
	}

	ASSERT_VALID(pDockManager);

	if (pDockManager->m_bLockUpdate)
	{
		return;
	}

	CWindowDC dc(this); // device context for painting

	CDC* pDC = &dc;
	BOOL m_bMemDC = FALSE;
	CDC dcMem;
	CBitmap bmp;
	CBitmap* pOldBmp = NULL;

	CRect rectWindow;
	GetWindowRect(rectWindow);
	CRect rect;
	rect.SetRect(0, 0, rectWindow.Width(), rectWindow.Height());

	if (dcMem.CreateCompatibleDC(&dc) && bmp.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height()))
	{
		// Off-screen DC successfully created. Better paint to it then!
		m_bMemDC = TRUE;
		pOldBmp = dcMem.SelectObject(&bmp);
		pDC = &dcMem;
	}

	// client area is not our bussiness :)
	CRect rcClient, rcBar;
	GetWindowRect(rcBar);

	GetClientRect(rcClient);
	ClientToScreen(rcClient);
	rcClient.OffsetRect(-rcBar.TopLeft());

	dc.ExcludeClipRect(rcClient);

	CRgn rgn;
	if (!m_rectRedraw.IsRectEmpty())
	{
		rgn.CreateRectRgnIndirect(m_rectRedraw);
		dc.SelectClipRgn(&rgn);
	}

	// draw border
	OnDrawBorder(pDC);

	CRect rectCaption;
	GetCaptionRect(rectCaption);

	pDockManager = m_pDockManager != NULL ? m_pDockManager : afxGlobalUtils.GetDockingManager(GetParent());
	ASSERT_VALID(pDockManager);
	if (pDockManager->m_bLockUpdate)
	{
		rectCaption.SetRectEmpty();
	}

	// draw caption:
	GetCaptionRect(rectCaption);

	COLORREF clrText = CMFCVisualManager::GetInstance()->OnFillMiniFrameCaption(pDC, rectCaption, this, m_bActive);

	int xBtnsLeft = -1;
	int xBtnsRight = -1;
	for (POSITION pos = m_lstCaptionButtons.GetHeadPosition(); pos != NULL;)
	{
		CMFCCaptionButton* pBtn = (CMFCCaptionButton*) m_lstCaptionButtons.GetNext(pos);
		ASSERT_VALID(pBtn);

		pBtn->m_clrForeground = clrText;

		if (!pBtn->m_bHidden)
		{
			if (pBtn->m_bLeftAlign)
			{
				if (xBtnsRight == -1)
				{
					xBtnsRight = pBtn->GetRect().right + 2;
				}
				else
				{
					xBtnsRight = max(xBtnsRight, pBtn->GetRect().right + 2);
				}
			}
			else
			{
				if (xBtnsLeft == -1)
				{
					xBtnsLeft = pBtn->GetRect().left;
				}
				else
				{
					xBtnsLeft = min(xBtnsLeft, pBtn->GetRect().left);
				}
			}
		}
	}

	// Paint caption text:
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(clrText);

	CFont* pOldFont = pDC->SelectObject(&afxGlobalData.fontBold);
	ASSERT_VALID(pOldFont);

	CString strCaption = GetCaptionText();

	CRect rectText = rectCaption;
	if (xBtnsLeft != -1)
	{
		rectText.right = xBtnsLeft;
	}
	if (xBtnsRight != -1)
	{
		rectText.left = xBtnsRight;
	}

	rectText.DeflateRect(2, 0);

	pDC->DrawText(strCaption, rectText, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

	pDC->SelectObject(pOldFont);
	pDC->SelectClipRgn(NULL);

	// Paint caption buttons:
	OnDrawCaptionButtons(pDC);

	if (m_bMemDC)
	{
		// Copy the results to the on-screen DC:
		CRect rectClip;
		int nClipType = dc.GetClipBox(rectClip);
		if (nClipType != NULLREGION)
		{
			if (nClipType != SIMPLEREGION)
			{
				rectClip = rect;
			}

			dc.BitBlt(rectClip.left, rectClip.top, rectClip.Width(), rectClip.Height(), &dcMem, rectClip.left, rectClip.top, SRCCOPY);
		}

		dcMem.SelectObject(pOldBmp);
	}

	CWnd::OnNcPaint();
}

void CPaneFrameWnd::OnDrawBorder(CDC* pDC)
{
	ASSERT_VALID(pDC);

	CDockingManager* pDockManager = m_pDockManager != NULL ? m_pDockManager : afxGlobalUtils.GetDockingManager(GetParent());
	if (pDockManager == NULL)
	{
		return;
	}

	ASSERT_VALID(pDockManager);

	if (pDockManager->m_bLockUpdate)
	{
		return;
	}

	CRect rectWnd;
	GetWindowRect(&rectWnd);
	ScreenToClient(&rectWnd);

	CRect rectBorderSize;
	CalcBorderSize(rectBorderSize);

	rectWnd.OffsetRect(rectBorderSize.left, m_nCaptionHeight + rectBorderSize.top);

	CRect rectBorder = rectWnd;

	CMFCToolBar* pToolBar = DYNAMIC_DOWNCAST(CMFCToolBar, GetPane());

	if (pToolBar != NULL)
	{
		pDC->FillRect(rectBorder, &afxGlobalData.brBtnFace);

		CMFCVisualManager::GetInstance()->OnDrawFloatingToolbarBorder(pDC, pToolBar, rectBorder, rectBorderSize);
		return;
	}

	CMFCVisualManager::GetInstance()->OnDrawMiniFrameBorder(pDC, this, rectBorder, rectBorderSize);

}

void CPaneFrameWnd::OnDrawCaptionButtons(CDC* pDC)
{
	ASSERT_VALID(pDC);

	// Paint caption buttons:
	for (POSITION pos = m_lstCaptionButtons.GetHeadPosition(); pos != NULL;)
	{
		CMFCCaptionButton* pBtn = (CMFCCaptionButton*) m_lstCaptionButtons.GetNext(pos);
		ASSERT_VALID(pBtn);

		BOOL bMaximized = TRUE;
		if (pBtn->GetHit() == HTMAXBUTTON && m_bPinned)
		{
			bMaximized = FALSE;
		}

		pBtn->m_bEnabled = (!CMFCToolBar::IsCustomizeMode() || (pBtn->GetHit() == HTCLOSE) || (pBtn->GetHit() == AFX_HTCLOSE));

		pBtn->OnDraw(pDC, TRUE, TRUE, bMaximized, !pBtn->m_bEnabled);
		pBtn->m_clrForeground = (COLORREF)-1;
	}
}

LRESULT CPaneFrameWnd::OnNcHitTest(CPoint point)
{
	return HitTest(point, FALSE);
}

LRESULT CPaneFrameWnd::HitTest(CPoint point, BOOL bDetectCaption)
{
	// in cust. mode allow mouse processing only for floating toolbar
	if (IsCustModeAndNotFloatingToolbar())
	{
		return HTNOWHERE;
	}

	CRect rectWnd;
	GetWindowRect(&rectWnd);

	if (!rectWnd.PtInRect(point))
	{
		return HTNOWHERE;
	}

	CRect rectClient;
	GetClientRect(&rectClient);
	ClientToScreen(&rectClient);

	if (rectClient.PtInRect(point))
	{
		return HTCLIENT;
	}

	CRect rectBorderSize;
	CalcBorderSize(rectBorderSize);

	int nCursorWidth  = GetSystemMetrics(SM_CXCURSOR) / 2;
	int nCursorHeight = GetSystemMetrics(SM_CYCURSOR) / 2;

	CRect rectCaption(rectWnd.left + rectBorderSize.left, rectWnd.top + rectBorderSize.top,
		rectWnd.right - rectBorderSize.right, rectWnd.top + rectBorderSize.top + m_nCaptionHeight);

	if (rectCaption.PtInRect(point))
	{
		if (bDetectCaption)
		{
			return HTCAPTION;
		}

		for (POSITION pos = m_lstCaptionButtons.GetHeadPosition(); pos != NULL;)
		{
			CMFCCaptionButton* pBtn = (CMFCCaptionButton*) m_lstCaptionButtons.GetNext(pos);
			ASSERT_VALID(pBtn);

			// the button's rectangle has offset relative to caption, we need to offset it
			// by caption's topleft corner to get client coordinates
			CRect rectBtn = pBtn->GetRect();
			rectBtn.OffsetRect(rectCaption.TopLeft());

			if (rectBtn.PtInRect(point))
			{
				return pBtn->GetHit();
			}
		}

		return HTCLIENT;
	}

	BOOL bEnableCornerArrows = TRUE;
	BOOL bEnableSizing = TRUE;

	CWnd* pWndEmbedded  = CWnd::FromHandlePermanent(m_hEmbeddedBar);
	if (pWndEmbedded != NULL && pWndEmbedded->IsKindOf(RUNTIME_CLASS(CMFCToolBar)))
	{
		bEnableCornerArrows = FALSE;
	}
	if (pWndEmbedded != NULL && pWndEmbedded->IsKindOf(RUNTIME_CLASS(CMFCColorBar)))
	{
		CMFCColorBar* pColorBar = DYNAMIC_DOWNCAST(CMFCColorBar, pWndEmbedded);
		if (pColorBar && pColorBar->IsTearOff())
		{
			bEnableSizing = FALSE;
		}
	}

	// no corner arrows in sliding mode
	bEnableCornerArrows = bEnableCornerArrows;

	CRect rectBorder;

	if (bEnableCornerArrows)
	{
		// top left corner - border
		rectBorder.SetRect(rectWnd.left, rectWnd.top, rectWnd.left + nCursorWidth, rectWnd.top + nCursorHeight);
		if (rectBorder.PtInRect(point))
		{
			return HTTOPLEFT;
		}

		// top border
		rectBorder.SetRect(rectWnd.left + nCursorWidth, rectWnd.top, rectWnd.right - nCursorWidth, rectWnd.top + rectBorderSize.top);
		if (rectBorder.PtInRect(point))
		{
			return HTTOP;
		}

		// top right border
		rectBorder.SetRect(rectWnd.right - nCursorWidth, rectWnd.top, rectWnd.right, rectWnd.top + nCursorHeight);
		if (rectBorder.PtInRect(point))
		{
			return HTTOPRIGHT;
		}

		// right border
		rectBorder.SetRect(rectWnd.right - rectBorderSize.right, rectWnd.top + nCursorHeight, rectWnd.right, rectWnd.bottom - nCursorHeight);
		if (rectBorder.PtInRect(point))
		{
			return HTRIGHT;
		}

		// bottom right
		rectBorder.SetRect(rectWnd.right - nCursorWidth, rectWnd.bottom - nCursorHeight, rectWnd.right, rectWnd.bottom);
		if (rectBorder.PtInRect(point))
		{
			return HTBOTTOMRIGHT;
		}

		// bottom
		rectBorder.SetRect(rectWnd.left + nCursorWidth, rectWnd.bottom - rectBorderSize.bottom, rectWnd.right - nCursorWidth, rectWnd.bottom);
		if (rectBorder.PtInRect(point))
		{
			return HTBOTTOM;
		}

		// bottom left
		rectBorder.SetRect(rectWnd.left, rectWnd.bottom - nCursorHeight, rectWnd.left + nCursorWidth, rectWnd.bottom);
		if (rectBorder.PtInRect(point))
		{
			return HTBOTTOMLEFT;
		}

		// left
		rectBorder.SetRect(rectWnd.left, rectWnd.top + nCursorHeight, rectWnd.left + rectBorderSize.left, rectWnd.bottom - nCursorHeight);

		if (rectBorder.PtInRect(point))
		{
			return HTLEFT;
		}
	}
	else
	{
		// top border
		rectBorder.SetRect(rectWnd.left, rectWnd.top, rectWnd.right, rectWnd.top + rectBorderSize.top);
		if (rectBorder.PtInRect(point))
		{
			if (!bEnableSizing)
			{
				return HTBORDER; // no sizing
			}
			return HTTOP;
		}

		// left
		rectBorder.SetRect(rectWnd.left, rectWnd.top, rectWnd.left + rectBorderSize.left, rectWnd.bottom);

		if (rectBorder.PtInRect(point))
		{
			if (!bEnableSizing)
			{
				return HTBORDER; // no sizing
			}
			return HTLEFT;
		}

		// bottom
		rectBorder.SetRect(rectWnd.left, rectWnd.bottom - rectBorderSize.bottom, rectWnd.right, rectWnd.bottom);
		if (rectBorder.PtInRect(point))
		{
			if (!bEnableSizing)
			{
				return HTBORDER; // no sizing
			}
			return HTBOTTOM;
		}

		// right border
		rectBorder.SetRect(rectWnd.right - rectBorderSize.right, rectWnd.top, rectWnd.right, rectWnd.bottom);
		if (rectBorder.PtInRect(point))
		{
			if (!bEnableSizing)
			{
				return HTBORDER; // no sizing
			}
			return HTRIGHT;
		}
	}
	return CWnd::OnNcHitTest(point);
}

BOOL CPaneFrameWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	switch (nHitTest)
	{
	case HTTOPLEFT:
	case HTBOTTOMRIGHT:
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENWSE));
		return TRUE;

	case HTTOP:
	case HTBOTTOM:
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENS));
		return TRUE;

	case HTRIGHT:
	case HTLEFT:
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
		return TRUE;

	case HTTOPRIGHT:
	case HTBOTTOMLEFT:
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENESW));
		return TRUE;

	}
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CPaneFrameWnd::OnSizing(UINT fwSide, LPRECT pRect)
{
	CWnd::OnSizing(fwSide, pRect);

	if (m_bRolledUp)
	{
		// can't be resized when rolled up
		return;
	}

	CRect rectOldWnd;
	GetWindowRect(rectOldWnd);

	CRect rect(pRect);

	CRect rectOldClient;
	GetClientRect(rectOldClient);

	BOOL bHorz = (fwSide == WMSZ_LEFT || fwSide == WMSZ_RIGHT);

	if (bHorz && rect.Width() == rectOldWnd.Width() || !bHorz && rect.Height() == rectOldWnd.Height())
	{
		return;
	}

	CMFCToolBar* pNextBar = DYNAMIC_DOWNCAST(CMFCToolBar, CWnd::FromHandlePermanent(m_hEmbeddedBar));
	if (pNextBar == NULL)
	{
		return;
	}

	CRect rectBorderSize;
	CalcBorderSize(rectBorderSize);

	int nNewHeight = rect.Height() - rectBorderSize.top - rectBorderSize.bottom - m_nCaptionHeight;
	int nNewWidth  = rect.Width() - rectBorderSize.left - rectBorderSize.right;

	CSize sizeBarOld = pNextBar->CalcSize(FALSE);
	CSize sizeBar = pNextBar->StretchPane(bHorz ? nNewWidth  : nNewHeight, !bHorz);

	int nXBarDelta = sizeBar.cx - sizeBarOld.cx;
	int nYBarDelta = sizeBar.cy - sizeBarOld.cy;

	if (nXBarDelta == 0 && nYBarDelta == 0 && pNextBar->IsKindOf(RUNTIME_CLASS(CMFCBaseToolBar)))
	{
		*pRect = rectOldWnd;
		return;
	}

	if (nXBarDelta != 0)
	{
		if (fwSide == WMSZ_RIGHT)
		{
			pRect->right = pRect->left + rectOldWnd.Width() + nXBarDelta;
		}
		else
		{
			pRect->left = pRect->right - rectOldWnd.Width() - nXBarDelta;
		}
	}
	else
	{
		fwSide == WMSZ_RIGHT ? pRect->right = rectOldWnd.right : pRect->left = rectOldWnd.left;
	}

	if (nYBarDelta != 0)
	{
		if (fwSide == WMSZ_BOTTOM || fwSide == WMSZ_RIGHT || fwSide == WMSZ_LEFT)
		{
			pRect->bottom = pRect->top + rectOldWnd.Height() + nYBarDelta;
		}
		else
		{
			pRect->top = pRect->bottom - rectOldWnd.Height() - nYBarDelta;
		}
	}
	else
	{
		fwSide == WMSZ_BOTTOM ? pRect->bottom = rectOldWnd.bottom : pRect->top = rectOldWnd.top;
	}

	pNextBar->RecalcLayout();

	BOOL bParam = FALSE;
	SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &bParam, 0);
	if (!bParam && pNextBar->IsKindOf(RUNTIME_CLASS(CMFCBaseToolBar)))
	{
		SetWindowPos(NULL, pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top, SWP_NOZORDER | SWP_NOACTIVATE);
	}
}

void CPaneFrameWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	CPane* pBar = DYNAMIC_DOWNCAST(CPane, CWnd::FromHandlePermanent(m_hEmbeddedBar));
	if (pBar != NULL)
	{
		pBar->RecalcLayout();
	}

	CRect rect;
	GetWindowRect(rect);

	afxGlobalUtils.AdjustRectToWorkArea(rect);
	SetWindowPos(NULL, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);

	ArrangeCaptionButtons();
	SendMessage(WM_NCPAINT);
}

void CPaneFrameWnd::SizeToContent()
{
	ASSERT_VALID(this);

	CPane* pBar = DYNAMIC_DOWNCAST(CPane, CWnd::FromHandlePermanent(m_hEmbeddedBar));
	if (pBar != NULL)
	{
		CSize sizeBar = pBar->CalcFixedLayout(FALSE, TRUE);

		CRect rectWnd;
		GetWindowRect(rectWnd);

		CRect rectClient;
		GetClientRect(rectClient);
		int nXDelta = rectClient.Width() - sizeBar.cx;
		int nYDelta = rectClient.Height() - sizeBar.cy;

		SetWindowPos(NULL, 0, 0, rectWnd.Width() - nXDelta, rectWnd.Height() - nYDelta, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}
}

void CPaneFrameWnd::OnPaneRecalcLayout()
{
	CRect rect;
	GetClientRect(&rect);
	CPane* pBar = DYNAMIC_DOWNCAST(CPane, CWnd::FromHandlePermanent(m_hEmbeddedBar));
	BOOL bSizeChanged = FALSE;
	if (pBar != NULL)
	{
		if (CPane::m_bHandleMinSize)
		{
			CSize size;
			pBar->GetMinSize(size);
			if (rect.Width() < size.cx)
			{
				rect.right = rect.left + size.cx;
				bSizeChanged = TRUE;
			}
			if (rect.Height() < size.cy)
			{
				rect.bottom = rect.top + size.cy;
				bSizeChanged = TRUE;
			}
		}

		pBar->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE);
		pBar->RedrawWindow();

		if (bSizeChanged)
		{
			SizeToContent();
		}
	}
}

BOOL CPaneFrameWnd::SetPreDockState(AFX_PREDOCK_STATE preDockState, CBasePane* pBarToDock, AFX_DOCK_METHOD /*dockMethod*/)
{
	if (preDockState == PDS_NOTHING || preDockState == PDS_DOCK_TO_TAB && pBarToDock != NULL && !pBarToDock->CanBeAttached())
	{
		return TRUE;
	}

	if (pBarToDock != NULL && pBarToDock->GetParentMiniFrame() != NULL)
	{
		return TRUE;
	}

	BOOL bWasCaptured = m_bCaptured;
	if (m_bCaptured)
	{
		ReleaseCapture();
		m_bCaptured = false;
		OnCapture(FALSE);
	}

	CPoint ptScreen;
	GetCursorPos(&ptScreen);

	CPoint ptClientBar = ptScreen;

	CDockablePane* pWnd = (CDockablePane*) GetPane();

	pWnd->ScreenToClient(&ptClientBar);

	m_preDockStateCurr = preDockState;
	m_pPreDockBar = DYNAMIC_DOWNCAST(CDockablePane, pBarToDock);

	// it will be different bar in case
	// of tab window
	BOOL bWasDocked = FALSE;
	CDockablePane* pDockedBar = DockPane(bWasDocked);

	if (pDockedBar != NULL)
	{
		pDockedBar->AdjustDockingLayout();
		if (bWasCaptured)
		{
			pDockedBar->EnterDragMode(FALSE);
		}
	}

	return FALSE;
}

void CPaneFrameWnd::OnMovePane(CPane* /*pBar*/, CPoint ptOffset)
{
	CRect rectMiniFrame;
	GetWindowRect(rectMiniFrame);
	rectMiniFrame.OffsetRect(ptOffset);
	MoveWindow(rectMiniFrame);
}

void CPaneFrameWnd::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos)
{
	lpwndpos->flags |= SWP_NOACTIVATE;
	if ((lpwndpos->flags & SWP_NOMOVE) == 0)
	{
		CRect rectWnd;
		GetWindowRect(&rectWnd);
		CRect rect;

		if (lpwndpos->flags & SWP_NOSIZE)
		{
			rect.SetRect(lpwndpos->x, lpwndpos->y, lpwndpos->x + rectWnd.Width(), lpwndpos->y + rectWnd.Height());
		}
		else
		{
			rect.SetRect(lpwndpos->x, lpwndpos->y, lpwndpos->x + lpwndpos->cx, lpwndpos->y + lpwndpos->cy);
		}

		int captionHeight = GetCaptionHeight();
		CRect rectDelta(captionHeight, captionHeight, captionHeight, captionHeight);
		afxGlobalUtils.AdjustRectToWorkArea(rect, &rectDelta);
		lpwndpos->x = rect.left;
		lpwndpos->y = rect.top;
	}

	CWnd::OnWindowPosChanging(lpwndpos);
}

BOOL CPaneFrameWnd::MoveMiniFrame()
{
	CWnd* pParent = GetParent();

	if (pParent == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (pParent->IsKindOf(RUNTIME_CLASS(CFrameWndEx)))
	{
		return((CFrameWndEx*) pParent)->OnMoveMiniFrame(this);
	}
	else if (pParent->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		return((CMDIFrameWndEx*) pParent)->OnMoveMiniFrame(this);
	}
	else if (pParent->IsKindOf(RUNTIME_CLASS(COleIPFrameWndEx)))
	{
		return((COleIPFrameWndEx*) pParent)->OnMoveMiniFrame(this);
	}
	else if (pParent->IsKindOf(RUNTIME_CLASS(COleDocIPFrameWndEx)))
	{
		return((COleDocIPFrameWndEx*) pParent)->OnMoveMiniFrame(this);
	}
	else if (pParent->IsKindOf(RUNTIME_CLASS(CMDIChildWndEx)))
	{
		return((CMDIChildWndEx*) pParent)->OnMoveMiniFrame(this);
	}
	else if (pParent->IsKindOf(RUNTIME_CLASS(COleCntrFrameWndEx)))
	{
		return((COleCntrFrameWndEx*) pParent)->OnMoveMiniFrame(this);
	}
	else
	{
		ASSERT(FALSE);
	}
	return FALSE;
}

CString CPaneFrameWnd::GetCaptionText()
{
	if (m_hEmbeddedBar == NULL)
	{
		return _T("");
	}

	CString strCaption;
	CWnd* pEmbeddedWnd = CWnd::FromHandlePermanent(m_hEmbeddedBar);
	if (pEmbeddedWnd != NULL)
	{
		pEmbeddedWnd->GetWindowText(strCaption);
	}

	return strCaption;
}

void CPaneFrameWnd::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case AFX_DOCK_EVENT:
		{
			CDockingManager* pDockManager = m_pDockManager != NULL ? m_pDockManager : afxGlobalUtils.GetDockingManager(GetParent());
			ASSERT_VALID(pDockManager);
			pDockManager->OnMoveMiniFrame(this);
			return;
		}
	case AFX_CHECK_ROLL_STATE:
		OnCheckRollState();
		break;
	}

	CWnd::OnTimer(nIDEvent);
}

void CPaneFrameWnd::CalcBorderSize(CRect& rectBorderSize) const
{
	if (GetPane() == NULL && m_bHostsToolbar || DYNAMIC_DOWNCAST(CMFCToolBar, GetPane()) != NULL)
	{
		rectBorderSize.SetRect(m_nToolbarBorderSize, m_nToolbarBorderSize, m_nToolbarBorderSize, m_nToolbarBorderSize);
	}
	else
	{
		const int nMiniFrameBorderSize = 4;

		rectBorderSize.SetRect(nMiniFrameBorderSize, nMiniFrameBorderSize, nMiniFrameBorderSize, nMiniFrameBorderSize);
	}
}

BOOL CPaneFrameWnd::OnNcActivate(BOOL bActive)
{
	if ((GetStyle() & MFS_SYNCACTIVE) == 0)
	{
		bActive = (GetFocus() == this);
		if (m_bActive != bActive)
		{
			m_bActive = bActive;

			CDockingManager* pDockManager = m_pDockManager != NULL ? m_pDockManager : afxGlobalUtils.GetDockingManager(GetParent());
			if (pDockManager != NULL)
			{
				SendMessage(WM_NCPAINT);
			}
		}
	}
	else if (m_nFlags & WF_KEEPMINIACTIVE)
	{
		return FALSE;
	}

	return TRUE;
}

void CPaneFrameWnd::SetCaptionButtons(DWORD dwButtons)
{
	ASSERT_VALID(this);
	RemoveAllCaptionButtons();

	if (dwButtons & AFX_CAPTION_BTN_CLOSE)
	{
		CBasePane* pBar = DYNAMIC_DOWNCAST(CBasePane, GetPane());
		if (pBar != NULL && pBar->CanBeClosed())
		{
			AddButton(HTCLOSE);
		}
	}

	if (dwButtons & AFX_CAPTION_BTN_PIN)
	{
		AddButton(HTMAXBUTTON);
	}

	if (dwButtons & AFX_CAPTION_BTN_MENU)
	{
		AddButton(HTMINBUTTON);
	}

	if (dwButtons & AFX_CAPTION_BTN_CUSTOMIZE)
	{
		AddButton(AFX_HTMENU);
	}

	m_dwCaptionButtons = dwButtons;
	SetCaptionButtonsToolTips();

	ArrangeCaptionButtons();
	SendMessage(WM_NCPAINT);
}

void CPaneFrameWnd::AddButton(UINT nHit)
{
	ASSERT_VALID(this);

	CMFCCaptionButton* pBtn = FindButton(nHit);

	if (pBtn == NULL)
	{
		switch (nHit)
		{
		case AFX_HTMENU:
			{
				CMFCCaptionMenuButton *pMenuBtn = new CMFCCaptionMenuButton;
				pMenuBtn->m_bOSMenu = FALSE;
				pMenuBtn->m_nHit = AFX_HTMENU;
				m_lstCaptionButtons.AddHead(pMenuBtn);
				pMenuBtn->SetMiniFrameButton();
				break;
			}

		default:
			pBtn = new CMFCCaptionButton;
			m_lstCaptionButtons.AddHead(pBtn);
			pBtn->m_nHit = nHit;
			pBtn->SetMiniFrameButton();
			break;
		}
	}
}

void CPaneFrameWnd::RemoveButton(UINT nHit)
{
	ASSERT_VALID(this);

	for (POSITION pos = m_lstCaptionButtons.GetHeadPosition(); pos != NULL;)
	{
		POSITION posSave = pos;

		CMFCCaptionButton* pBtn = (CMFCCaptionButton*) m_lstCaptionButtons.GetNext(pos);
		ASSERT_VALID(pBtn);

		if (pBtn->m_nHit == nHit)
		{
			m_lstCaptionButtons.RemoveAt(posSave);
			delete pBtn;
			break;
		}
	}

	ArrangeCaptionButtons();
}

void CPaneFrameWnd::ReplaceButton(UINT nHit, UINT nHitNew)
{
	ASSERT_VALID(this);

	CMFCCaptionButton* pBtn = FindButton(nHit);
	if (pBtn != NULL)
	{
		pBtn->m_nHit = nHitNew;
	}
}

void CPaneFrameWnd::ShowButton(UINT nHit, BOOL bShow)
{
	ASSERT_VALID(this);
	CMFCCaptionButton* pBtn = FindButton(nHit);
	if (pBtn != NULL)
	{
		pBtn->m_bHidden = bShow;
		ArrangeCaptionButtons();
	}
}

CMFCCaptionButton* CPaneFrameWnd::FindButton(UINT uiHit) const
{
	ASSERT_VALID(this);

	for (POSITION pos = m_lstCaptionButtons.GetHeadPosition(); pos != NULL;)
	{
		CMFCCaptionButton* pBtn = (CMFCCaptionButton*) m_lstCaptionButtons.GetNext(pos);
		ASSERT_VALID(pBtn);

		if (pBtn->GetHit() == uiHit)
		{
			return pBtn;
		}
	}

	return NULL;
}

CMFCCaptionButton* CPaneFrameWnd::FindButton(CPoint point) const
{
	ASSERT_VALID(this);

	CRect rectWnd;
	GetWindowRect(&rectWnd);

	if (GetExStyle() & WS_EX_LAYOUTRTL)
	{
		point.x = rectWnd.right - point.x + rectWnd.left;
	}

	for (POSITION pos = m_lstCaptionButtons.GetHeadPosition(); pos != NULL;)
	{
		CMFCCaptionButton* pBtn = (CMFCCaptionButton*) m_lstCaptionButtons.GetNext(pos);
		ASSERT_VALID(pBtn);

		CRect rectBtn = pBtn->GetRect();
		rectBtn.OffsetRect(rectWnd.TopLeft());

		if (rectBtn.PtInRect(point))
		{
			return pBtn;
		}
	}

	return NULL;
}

void CPaneFrameWnd::RemoveAllCaptionButtons()
{
	ASSERT_VALID(this);

	m_dwCaptionButtons = 0;

	while (!m_lstCaptionButtons.IsEmpty())
	{
		delete m_lstCaptionButtons.RemoveHead();
	}
}

void CPaneFrameWnd::UpdateTooltips()
{
	// Update tool area for the tooltips:
	if (m_pToolTip->GetSafeHwnd() != NULL)
	{
		CRect rcBar;
		GetWindowRect(rcBar);
		ScreenToClient(rcBar);
		int i = 0;
		for (POSITION pos = m_lstCaptionButtons.GetHeadPosition(); pos != NULL; i++)
		{
			CMFCCaptionButton* pBtn = (CMFCCaptionButton*) m_lstCaptionButtons.GetNext(pos);
			ASSERT_VALID(pBtn);

			if (i < m_pToolTip->GetToolCount())
			{
				CRect rectTT;
				rectTT.SetRectEmpty();
				if (!pBtn->m_bHidden)
				{
					rectTT = pBtn->GetRect();
					rectTT.OffsetRect(rcBar.TopLeft());
				}
				m_pToolTip->SetToolRect(this, i + 1, rectTT);
			}
		}
	}
}

void CPaneFrameWnd::ArrangeCaptionButtons()
{
	ASSERT_VALID(this);

	CRect rectCaption;
	GetCaptionRect(rectCaption);

	CSize btnSize = CMFCCaptionButton::GetSize();

	CPoint ptOrgRight(rectCaption.right - m_nCaptionButtonMargin, rectCaption.top +(rectCaption.Height() - btnSize.cy) / 2);
	CPoint ptOrgLeft(rectCaption.left + m_nCaptionButtonMargin, rectCaption.top +(rectCaption.Height() - btnSize.cy) / 2);

	int i = 0;
	for (POSITION pos = m_lstCaptionButtons.GetTailPosition(); pos != NULL; i++)
	{
		CMFCCaptionButton* pBtn = (CMFCCaptionButton*) m_lstCaptionButtons.GetPrev(pos);
		ASSERT_VALID(pBtn);

		if (!pBtn->m_bHidden)
		{
			if (pBtn->m_bLeftAlign)
			{
				pBtn->Move(ptOrgLeft);
				ptOrgLeft.x += btnSize.cx;
				ptOrgLeft.x += m_nCaptionButtonSpacing;
			}
			else
			{
				ptOrgRight.x -= btnSize.cx;
				ptOrgRight.x = max(ptOrgRight.x, rectCaption.left);

				pBtn->Move(ptOrgRight);
				ptOrgRight.x -= m_nCaptionButtonSpacing;
			}
		}
	}

	UpdateTooltips();
}

void CPaneFrameWnd::RedrawCaptionButton(CMFCCaptionButton* pBtn)
{
	ASSERT_VALID(this);

	if (pBtn == NULL)
	{
		return;
	}

	ASSERT_VALID(pBtn);

	m_rectRedraw = pBtn->GetRect();
	SendMessage(WM_NCPAINT);
	m_rectRedraw.SetRectEmpty();

	UpdateWindow();
}

void CPaneFrameWnd::CloseMiniFrame()
{
	ShowWindow(SW_HIDE);

	if (m_hEmbeddedBar != NULL)
	{
		CWnd* pEmbeddedWnd = CWnd::FromHandlePermanent(m_hEmbeddedBar);
		if (pEmbeddedWnd != NULL)
		{
			pEmbeddedWnd->ShowWindow(SW_HIDE);
		}
	}
}

void CPaneFrameWnd::OnNcLButtonDblClk(UINT nHitTest, CPoint point)
{
	CWnd::OnNcLButtonDblClk(nHitTest, point);
}

void CPaneFrameWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CPoint ptScreen = point;
	ClientToScreen(&ptScreen);

	CMFCCaptionButton* pBtn = FindButton(ptScreen);
	if (pBtn != NULL)
	{
		CWnd::OnLButtonDblClk(nFlags, point);
		return;
	}

	OnDockToRecentPos();
	CWnd::OnLButtonDblClk(nFlags, point);
}

void CPaneFrameWnd::OnDockToRecentPos()
{
	CDockingManager* pDockManager = m_pDockManager != NULL ? m_pDockManager : afxGlobalUtils.GetDockingManager(this);
	CPane* pBar = DYNAMIC_DOWNCAST(CPane, CWnd::FromHandlePermanent(m_hEmbeddedBar));

	if (pBar != NULL && pBar->GetEnabledAlignment() & CBRS_ALIGN_ANY)
	{
		SaveRecentFloatingState();

		if (pBar->DockPane(pBar, NULL, DM_DBL_CLICK))
		{
			// was destroyed if DockPane returned TRUE
			afxGlobalUtils.ForceAdjustLayout(pDockManager);
			return;
		}
	}

}

BOOL CPaneFrameWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= MFS_SYNCACTIVE;
	return CWnd::PreCreateWindow(cs);
}

BOOL CPaneFrameWnd::OnNcCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (!CWnd::OnNcCreate(lpCreateStruct))
		return FALSE;

	if (GetStyle() & MFS_SYNCACTIVE)
	{
		// syncronize activation state with top level parent
		CWnd* pParentWnd = GetTopLevelParent();
		ENSURE(pParentWnd != NULL);
		CWnd* pActiveWnd = GetForegroundWindow();
		BOOL bActive = (pParentWnd == pActiveWnd) || (pParentWnd->GetLastActivePopup() == pActiveWnd && pActiveWnd->SendMessage(WM_FLOATSTATUS, FS_SYNCACTIVE) != 0);

		// the WM_FLOATSTATUS does the actual work
		SendMessage(WM_FLOATSTATUS, bActive ? FS_ACTIVATE : FS_DEACTIVATE);
	}

	return TRUE;
}

LRESULT CPaneFrameWnd::OnFloatStatus(WPARAM wParam, LPARAM)
{
	// these asserts make sure no conflicting actions are requested
	ASSERT(!((wParam & FS_SHOW) &&(wParam & FS_HIDE)));
	ASSERT(!((wParam & FS_ENABLE) &&(wParam & FS_DISABLE)));
	ASSERT(!((wParam & FS_ACTIVATE) &&(wParam & FS_DEACTIVATE)));

	// FS_SYNCACTIVE is used to detect MFS_SYNCACTIVE windows
	LRESULT lResult = 0;
	if ((GetStyle() & MFS_SYNCACTIVE) &&(wParam & FS_SYNCACTIVE))
		lResult = 1;

	if (wParam &(FS_SHOW|FS_HIDE))
	{
		SetWindowPos(NULL, 0, 0, 0, 0, ((wParam & FS_SHOW) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW) | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
	}
	if (wParam &(FS_ENABLE|FS_DISABLE))
		EnableWindow((wParam & FS_ENABLE) != 0);

	if ((wParam &(FS_ACTIVATE|FS_DEACTIVATE)) && GetStyle() & MFS_SYNCACTIVE)
	{
		ModifyStyle(MFS_SYNCACTIVE, 0);
		SendMessage(WM_NCACTIVATE, (wParam & FS_ACTIVATE) != 0);
		ModifyStyle(0, MFS_SYNCACTIVE);
	}

	return lResult;
}

BOOL CPaneFrameWnd::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CPaneFrameWnd::OnNcDestroy()
{
	OnCancelMode();
	CWnd::OnNcDestroy();
	delete this;
}

void CPaneFrameWnd::OnCancelMode()
{
	StopCaptionButtonsTracking();

	if (m_bCaptured)
	{
		if ((GetDockingMode() & DT_STANDARD) != 0)
		{
			m_dragFrameImpl.EndDrawDragFrame();
			if (!IsWindowVisible())
			{
				// probably dragging control bar detached from tab window
				CPane* pBar = DYNAMIC_DOWNCAST(CPane, CWnd::FromHandlePermanent(m_hEmbeddedBar));
				if (pBar != NULL && pBar->GetParent() == this)
				{
					BOOL bResult = pBar->DockPane(pBar, NULL, DM_DBL_CLICK);
					pBar->ShowPane(TRUE, FALSE, TRUE);
					if (!bResult)
					{
						return;
					}
				}
			}
		}
		else if ((GetDockingMode() & DT_SMART) != 0)
		{
			CDockingManager* pDockManager = m_pDockManager != NULL ? m_pDockManager : afxGlobalUtils.GetDockingManager(GetParent());
			if (pDockManager != NULL)
			{
				CSmartDockingManager* pSDManager = pDockManager->GetSmartDockingManagerPermanent();
				if (pSDManager != NULL && pSDManager->IsStarted())
				{
					m_dragFrameImpl.RemoveTabPreDocking();
					pDockManager->StopSDocking();
				}
			}
		}
		else if ((GetDockingMode() & DT_IMMEDIATE) != 0)
		{
			//m_dragFrameImpl.EndDrawDragFrame();
		}

		ReleaseCapture();
		m_bCaptured = false;
		OnCapture(FALSE);
	}

	CWnd::OnCancelMode();
}

void CPaneFrameWnd::GetCaptionRect(CRect& rectCaption) const
{
	CRect rectBorderSize;
	rectBorderSize.SetRectEmpty();
	CalcBorderSize(rectBorderSize);

	CRect rectWnd;
	GetWindowRect(&rectWnd);
	ScreenToClient(&rectWnd);
	rectWnd.OffsetRect(rectBorderSize.left, m_nCaptionHeight + rectBorderSize.top);

	rectCaption = CRect(rectWnd.left + rectBorderSize.left, rectWnd.top + rectBorderSize.top,
		rectWnd.right - rectBorderSize.right, rectWnd.top + rectBorderSize.top + m_nCaptionHeight);
}

void CPaneFrameWnd::OnNcMouseMove(UINT nHitTest, CPoint point)
{
	if (!m_bBlockMove && !m_bCaptured)
	{
		OnTrackCaptionButtons(point);
	}

	CWnd::OnNcMouseMove(nHitTest, point);
}

void CPaneFrameWnd::OnTrackCaptionButtons(CPoint point)
{
	if (CMFCPopupMenu::GetActiveMenu() != NULL)
	{
		return;
	}

	UINT nHot = m_nHot;
	BOOL bEnableChanged = FALSE;

	CMFCCaptionButton* pBtn = FindButton(point);

	if (pBtn != NULL)
	{
		BOOL bEnabled = pBtn->m_bEnabled;
		pBtn->m_bEnabled = (!CMFCToolBar::IsCustomizeMode() || (pBtn->GetHit() == HTCLOSE) || (pBtn->GetHit() == AFX_HTCLOSE));
		bEnableChanged = pBtn->m_bEnabled != bEnabled;
	}

	if (pBtn != NULL && pBtn->m_bEnabled)
	{
		m_nHot = pBtn->GetHit();
		pBtn->m_bFocused = TRUE;
	}
	else
	{
		m_nHot = HTNOWHERE;
	}

	if (m_nHot != nHot || bEnableChanged)
	{
		RedrawCaptionButton(pBtn);

		CMFCCaptionButton* pBtnOld = FindButton(nHot);
		if (pBtnOld != NULL)
		{
			pBtnOld->m_bFocused = FALSE;
			RedrawCaptionButton(pBtnOld);
		}
	}

	if (m_nHit == HTNOWHERE)
	{
		if (nHot != HTNOWHERE && m_nHot == HTNOWHERE)
		{
			::ReleaseCapture();
		}
		else if (nHot == HTNOWHERE && m_nHot != HTNOWHERE)
		{
			SetCapture();
		}
	}
}

void CPaneFrameWnd::StopCaptionButtonsTracking()
{
	if (m_nHit != HTNOWHERE)
	{
		CMFCCaptionButton* pBtn = FindButton(m_nHit);
		m_nHit = HTNOWHERE;

		ReleaseCapture();
		if (pBtn != NULL)
		{
			pBtn->m_bPushed = FALSE;
			RedrawCaptionButton(pBtn);
		}
	}

	if (m_nHot != HTNOWHERE)
	{
		CMFCCaptionButton* pBtn = FindButton(m_nHot);
		m_nHot = HTNOWHERE;

		ReleaseCapture();
		if (pBtn != NULL)
		{
			pBtn->m_bFocused = FALSE;
			RedrawCaptionButton(pBtn);
		}
	}
}

BOOL CPaneFrameWnd::IsCustModeAndNotFloatingToolbar() const
{
	CWnd* pEmbeddedWnd = CWnd::FromHandlePermanent(m_hEmbeddedBar);
	if (CMFCToolBar::IsCustomizeMode() && pEmbeddedWnd != NULL && !pEmbeddedWnd->IsKindOf(RUNTIME_CLASS(CMFCToolBar)))
	{
		return TRUE;
	}
	return FALSE;
}

int  CPaneFrameWnd::GetCaptionButtonTotalWidth() const
{
	int nTotalWidth = 0;
	for (POSITION pos = m_lstCaptionButtons.GetHeadPosition(); pos != NULL;)
	{
		CMFCCaptionButton* pBtn = (CMFCCaptionButton*) m_lstCaptionButtons.GetNext(pos);
		ASSERT_VALID(pBtn);

		CRect rectBtn = pBtn->GetRect();
		nTotalWidth += rectBtn.Width();
	}
	return nTotalWidth;
}

void CPaneFrameWnd::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	CPane* pContainedBar = DYNAMIC_DOWNCAST(CPane, CWnd::FromHandlePermanent(m_hEmbeddedBar));

	if (pContainedBar != NULL)
	{
		CSize sizeBase;
		pContainedBar->GetMinSize(sizeBase);

		CalcMinSize(sizeBase, lpMMI);
	}

	CWnd::OnGetMinMaxInfo(lpMMI);
}

void CPaneFrameWnd::CalcMinSize(CSize& sizeBase, MINMAXINFO FAR* lpMMI)
{
	CRect rectBorderSize;
	CalcBorderSize(rectBorderSize);

	lpMMI->ptMinTrackSize.x = max(sizeBase.cx, GetCaptionButtonTotalWidth()) + rectBorderSize.left + rectBorderSize.right;
	lpMMI->ptMinTrackSize.x = max(lpMMI->ptMinTrackSize.x, m_sizeMinSize.cx);

	lpMMI->ptMinTrackSize.y = sizeBase.cy + m_nCaptionHeight + rectBorderSize.top + rectBorderSize.bottom;
	lpMMI->ptMinTrackSize.y = max(lpMMI->ptMinTrackSize.y, m_sizeMinSize.cy);
}

void CPaneFrameWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if (CMFCToolBar::IsCustomizeMode() || m_bCaptured)
	{
		return;
	}

	if (CMFCPopupMenu::GetActiveMenu() != NULL)
	{
		return;
	}

	StopCaptionButtonsTracking();

	CPane* pFirstBar = DYNAMIC_DOWNCAST(CPane, GetFirstVisiblePane());

	if (pFirstBar != NULL && pFirstBar->OnShowControlBarMenu(point))
	{
		return;
	}

	if (pFirstBar == NULL)
	{
		return;
	}

	CRect rectWnd;
	GetWindowRect(&rectWnd);

	CRect rectBorderSize;
	CalcBorderSize(rectBorderSize);

	CRect rectCaption(rectWnd.left + rectBorderSize.left, rectWnd.top + rectBorderSize.top,
		rectWnd.right - rectBorderSize.right, rectWnd.top + rectBorderSize.top + m_nCaptionHeight);

	if (!rectCaption.PtInRect(point))
	{
		return;
	}

	CWnd* pParent = GetParent();
	if (pParent != NULL)
	{
		ASSERT_VALID(pParent);
		pFirstBar->OnPaneContextMenu(this, point);
	}
}

void CPaneFrameWnd::OnCapture(BOOL bOnOff)
{
	CPane* pBar = DYNAMIC_DOWNCAST(CPane, CWnd::FromHandlePermanent(m_hEmbeddedBar));

	if (pBar != NULL)
	{
		pBar->SetDragMode(bOnOff);
	}

	if (IsWindow(m_hWndToDestroyOnRelease) && !bOnOff)
	{
		::DestroyWindow(m_hWndToDestroyOnRelease);
		m_hWndToDestroyOnRelease = NULL;
	}

	if (pBar != NULL && !bOnOff)
	{
		if (pBar->m_hwndMiniFrameToBeClosed != m_hWnd && IsWindow(pBar->m_hwndMiniFrameToBeClosed))
		{
			::DestroyWindow(pBar->m_hwndMiniFrameToBeClosed);
		}
		pBar->m_hwndMiniFrameToBeClosed = NULL;
	}
}

void CPaneFrameWnd::OnDestroy()
{
	if (m_bCaptured)
	{
		ReleaseCapture();
		m_bCaptured = false;
		OnCapture(FALSE);
	}

	KillDockingTimer();

	if (m_nRollTimerID != 0)
	{
		KillTimer(m_nRollTimerID);
	}

	// register with dock manager
	CDockingManager* pDockManager = m_pDockManager != NULL ? m_pDockManager : afxGlobalUtils.GetDockingManager(GetParent());
	if (pDockManager != NULL)
	{
		ASSERT_VALID(pDockManager);
		pDockManager->RemoveMiniFrame(this);
	}

	POSITION pos = m_lstFrames.Find(GetSafeHwnd());
	ENSURE(pos != NULL);
	m_lstFrames.RemoveAt(pos);

	CTooltipManager::DeleteToolTip(m_pToolTip);

	CWnd::OnDestroy();
}

void CPaneFrameWnd::SetDockingTimer(UINT nTimeOut)
{
	if (m_nDockTimerID != 0)
	{
		KillDockingTimer();
	}
	m_nDockTimerID = (UINT) SetTimer(AFX_DOCK_EVENT, nTimeOut, NULL);
}

void CPaneFrameWnd::KillDockingTimer()
{
	if (m_nDockTimerID != 0)
	{
		KillTimer(m_nDockTimerID);
		m_nDockTimerID = 0;
	}
}

CBasePane* __stdcall CPaneFrameWnd::FindFloatingPaneByID(UINT nID)
{
	HWND hWnd = NULL;
	if (!m_mapFloatingBars.Lookup(nID, hWnd))
	{
		return NULL;
	}

	CBasePane* pBar = DYNAMIC_DOWNCAST(CBasePane, CWnd::FromHandlePermanent(hWnd));

	return pBar;
}

CPaneFrameWnd* __stdcall CPaneFrameWnd::FrameFromPoint(CPoint pt, int nSensitivity, CPaneFrameWnd* pFrameToExclude, BOOL bFloatMultiOnly)
{
	for (POSITION pos = m_mapFloatingBars.GetStartPosition(); pos != NULL;)
	{
		UINT uID = 0;
		HWND hWnd = NULL;

		m_mapFloatingBars.GetNextAssoc(pos, uID, hWnd);

		CBasePane* pNextBar = DYNAMIC_DOWNCAST(CBasePane, CWnd::FromHandlePermanent(hWnd));

		if (pNextBar == NULL)
		{
			continue;
		}

		if (!pNextBar->IsWindowVisible())
		{
			continue;
		}

		if ((bFloatMultiOnly &&
			((pNextBar->GetPaneStyle() & CBRS_FLOAT_MULTI) || pNextBar->IsInFloatingMultiPaneFrameWnd()) || !bFloatMultiOnly) && pNextBar->GetParentMiniFrame() != pFrameToExclude)
		{
			CRect rectBar, rectBarInflated;
			pNextBar->GetWindowRect(rectBar);
			rectBarInflated = rectBar;

			if (rectBarInflated.PtInRect(pt))
			{
				// the point around bar
				rectBar.InflateRect(-nSensitivity, -nSensitivity);
				if (!rectBar.PtInRect(pt) || nSensitivity == 0)
				{
					// the point near  the floating bar edges - return its parent frame
					CPaneFrameWnd* pWnd = DYNAMIC_DOWNCAST(CPaneFrameWnd, pNextBar->GetParentMiniFrame());

					if (pWnd != NULL && pWnd->IsWindowVisible())
					{
						return pWnd;
					}
					return NULL;
				}
			}
		}
	}
	return NULL;
}

void CPaneFrameWnd::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_bCaptured && nChar == VK_ESCAPE)
	{
		if ((GetDockingMode() & DT_STANDARD) != 0)
		{
			OnCancelMode();
		}
		else
		{
			ReleaseCapture();
			CPane* pBar = DYNAMIC_DOWNCAST(CPane, CWnd::FromHandlePermanent(m_hEmbeddedBar));

			if (pBar != NULL && pBar->GetEnabledAlignment() & CBRS_ALIGN_ANY)
			{
				pBar->m_recentDockInfo.m_rectRecentFloatingRect = m_rectRecentFloatingRect;
				if (!pBar->DockPane(pBar, NULL, DM_DBL_CLICK))
				{
					// was destroyed
					return;
				}
			}
		}
	}
	CWnd::OnChar(nChar, nRepCnt, nFlags);
}

void CPaneFrameWnd::AdjustLayout()
{
	CBasePane* pBaseControlBar = DYNAMIC_DOWNCAST(CBasePane, CWnd::FromHandlePermanent(m_hEmbeddedBar));
	if (pBaseControlBar != NULL)
	{
		pBaseControlBar->RecalcLayout();
		SizeToContent();
	}
}

BOOL CPaneFrameWnd::SaveState(LPCTSTR lpszProfileName, UINT uiID)
{
	CBasePane* pBaseControlBar = DYNAMIC_DOWNCAST(CBasePane, CWnd::FromHandlePermanent(m_hEmbeddedBar));
	if (pBaseControlBar != NULL)
	{
		return pBaseControlBar->SaveState(lpszProfileName, uiID);
	}
	return TRUE;
}

BOOL CPaneFrameWnd::LoadState(LPCTSTR lpszProfileName, UINT uiID)
{
	CBasePane* pBaseControlBar = DYNAMIC_DOWNCAST(CBasePane, CWnd::FromHandlePermanent(m_hEmbeddedBar));
	if (pBaseControlBar != NULL)
	{
		return pBaseControlBar->LoadState(lpszProfileName, uiID);
	}
	return TRUE;
}

void CPaneFrameWnd::Serialize(CArchive& ar)
{
	CWnd::Serialize(ar);

	if (ar.IsLoading())
	{
		DWORD dwStyle = 0;
		CRect rect; rect.SetRectEmpty();
		BOOL bIsVisible = FALSE;

		ar >> dwStyle;
		ar >> rect;
		ar >> bIsVisible;
		ar >> m_nRestoredEmbeddedBarID;
		ar >> m_dwCaptionButtons;
		ar >> m_bPinned;

		if (!Create(_T(""), dwStyle & ~WS_VISIBLE, rect, CPaneFrameWnd::m_pParentWndForSerialize))
		{
			throw new CArchiveException;
			return;
		}
		m_hParentWnd = CPaneFrameWnd::m_pParentWndForSerialize->m_hWnd;
	}
	else
	{
		CRect rect;
		GetWindowRect(rect);

		if (m_bRolledUp)
		{
			rect.bottom = rect.top + m_nHeightBeforeRollUp;
		}

		BOOL bIsVisible = IsWindowVisible();

		ar << GetStyle();
		ar << rect;
		ar << bIsVisible;

		CWnd* pEmbeddedWnd = CWnd::FromHandlePermanent(m_hEmbeddedBar);
		if (pEmbeddedWnd != 0)
		{
			ar << pEmbeddedWnd->GetDlgCtrlID();
		}
		else
		{
			ar << 0;
		}
		ar << m_dwCaptionButtons;
		ar << m_bPinned;
	}
}

void CPaneFrameWnd::SetDockState(CDockingManager* pDockManager)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDockManager);

	if (m_nRestoredEmbeddedBarID != NULL)
	{
		CBasePane* pBar = pDockManager->FindPaneByID(m_nRestoredEmbeddedBarID, TRUE);

		if (pBar != NULL && pBar->CanFloat() && ::IsWindow(pBar->m_hWnd))
		{

			if (pBar->IsTabbed())
			{
				CMFCBaseTabCtrl* pTabWnd = (CMFCBaseTabCtrl*) pBar->GetParent();
				CBaseTabbedPane* pTabBar = (CBaseTabbedPane*) pTabWnd->GetParent();
				ASSERT_VALID(pTabBar);
				// set belong to any parent
				pBar->SetParent(GetParent());
				pTabBar->RemovePane(pBar);
				if (pBar->IsKindOf(RUNTIME_CLASS(CDockablePane)))
				{
					((CDockablePane*) pBar)->EnableGripper(TRUE);
				}

				pBar->ShowWindow(SW_SHOW);
			}

			if (pBar->IsKindOf(RUNTIME_CLASS(CDockablePane)) && ((CDockablePane*) pBar)->IsAutoHideMode())
			{
				((CDockablePane*) pBar)->SetAutoHideMode(FALSE, CBRS_ALIGN_ANY);
			}

			CRect rectDummy;
			pBar->GetWindowRect(rectDummy);

			if (pBar->GetParentMiniFrame() == NULL)
			{
				pBar->FloatPane(rectDummy, DM_SHOW, false);
			}

			CPaneFrameWnd* pParentMiniFrame = pBar->GetParentMiniFrame();

			if (pParentMiniFrame != NULL)
			{
				pParentMiniFrame->RemovePane(pBar, FALSE);
				pBar->SetParent(this);
				pBar->AdjustDockingLayout();

				CRect rect;
				GetClientRect(rect);

				AddPane(pBar);

				//move control bar to the top left corner of the miniframe
				pBar->SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOSIZE  | SWP_NOACTIVATE);

				pBar->StretchPane(rect.Height(), TRUE);
				pBar->RecalcLayout();
				SizeToContent();

				SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED  | SWP_NOACTIVATE);

				BOOL bShow = TRUE;
				if (pBar->IsKindOf(RUNTIME_CLASS(CPane)))
				{
					bShow = !(((CPane *)pBar)->m_bRecentFloatingState);
				}

				if (bShow)
				{
					pBar->ShowPane(pBar->GetRecentVisibleState(), FALSE, FALSE);
				}
				else
				{
					SetDelayShow(TRUE);
				}

				SetCaptionButtons(m_dwCaptionButtons);
				return;
			}
		}
	}

	// if we're here the miniframe is empty - destroy it
	DestroyWindow();
}

LRESULT CPaneFrameWnd::OnCheckEmptyState(WPARAM, LPARAM)
{
	if (GetPaneCount() == 0)
	{
		if (m_bNoDelayedDestroy)
		{
			ShowWindow(SW_HIDE);
		}
		else
		{
			DestroyWindow();
		}
	}

	return 0;
}

void __stdcall CPaneFrameWnd::RedrawAll()
{
	for (POSITION pos = m_lstFrames.GetHeadPosition(); pos != NULL;)
	{
		HWND hwndFrame = m_lstFrames.GetNext(pos);
		if (CWnd::FromHandlePermanent(hwndFrame) != NULL)
		{
			::RedrawWindow(hwndFrame, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
		}
	}
}

int CPaneFrameWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_lstFrames.AddTail(GetSafeHwnd());

	CTooltipManager::CreateToolTip(m_pToolTip, this, AFX_TOOLTIP_TYPE_MINIFRAME);
	return 0;
}

void CPaneFrameWnd::StoreRecentDockSiteInfo(CPane* /*pBar*/)
{
}

void CPaneFrameWnd::StoreRecentTabRelatedInfo(CDockablePane* /*pDockingBar*/, CDockablePane* /*pTabbedBar*/)
{
}

CBasePane* CPaneFrameWnd::PaneFromPoint(CPoint point, int /*nSensitivity*/, BOOL bCheckVisibility)
{
	CPane* pBar = DYNAMIC_DOWNCAST(CPane, CWnd::FromHandlePermanent(m_hEmbeddedBar));

	if (pBar != NULL)
	{
		CRect rect;
		pBar->GetWindowRect(rect);
		if (rect.PtInRect(point))
		{
			if (!pBar->IsWindowVisible() && bCheckVisibility)
			{
				return NULL;
			}
			return pBar;
		}
	}

	return NULL;
}

BOOL CPaneFrameWnd::CanBeDockedToPane(const CDockablePane* pDockingBar) const
{
	CPane* pBar = DYNAMIC_DOWNCAST(CPane, CWnd::FromHandlePermanent(m_hEmbeddedBar));

	return pDockingBar->CanAcceptPane(pBar);
}

LRESULT CPaneFrameWnd::OnIdleUpdateCmdUI(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	CWnd* pFocus = GetFocus();
	BOOL bActiveOld = m_bActive;

	m_bActive = (pFocus->GetSafeHwnd() != NULL && (IsChild(pFocus) || pFocus->GetSafeHwnd() == GetSafeHwnd()));

	if (m_bActive != bActiveOld)
	{
		SendMessage(WM_NCPAINT);
	}

	SendMessageToDescendants(WM_IDLEUPDATECMDUI, (WPARAM)TRUE, 0, TRUE, TRUE);

	return 0L;
}

BOOL CPaneFrameWnd::StartTearOff(CMFCPopupMenu* pMenu)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pMenu);

	HWND hwndMenu = pMenu->GetSafeHwnd();
	pMenu->ShowWindow(SW_HIDE);

	// Redraw parent button:
	CMFCToolBarMenuButton* pParentBtn = pMenu->GetParentButton();
	if (pParentBtn != NULL)
	{
		CWnd* pWndParent = pParentBtn->GetParentWnd();
		if (pWndParent != NULL)
		{
			CRect rectBtn = pParentBtn->Rect();
			rectBtn.InflateRect(4, 4);

			pWndParent->InvalidateRect(rectBtn);
			pWndParent->UpdateWindow();
		}
	}

	CBasePane* pBar = DYNAMIC_DOWNCAST(CBasePane, GetPane());
	if (pBar == NULL)
	{
		return FALSE;
	}

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
	pBar->SetCapture();
	ENSURE(pBar == CWnd::GetCapture());

	BOOL bSuccess = FALSE;
	BOOL bStop = FALSE;

	// Move cirsor to the middle of the caption
	CRect rectFrame;
	GetWindowRect(rectFrame);

	int x = (rectFrame.left + rectFrame.right) / 2;
	int xOffset = x - rectFrame.left;

	int y = rectFrame.top + 5;
	int yOffset = y - rectFrame.top;

	::SetCursorPos(x, y);

	// get messages until capture lost or cancelled/accepted
	while (!bStop && CWnd::GetCapture() == pBar)
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
			break;

		case WM_MOUSEMOVE:
			{
				SetWindowPos(NULL, msg.pt.x - xOffset, msg.pt.y - yOffset, -1, -1, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
			}
			break;

		case WM_KEYDOWN:
			if (msg.wParam == VK_ESCAPE)
			{
				bStop = TRUE;
			}
			break;

		case WM_RBUTTONDOWN:
			bStop = TRUE;
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
		if (bSuccess)
		{
			pMenu->SendMessage(WM_CLOSE);
			if (AFXGetTopLevelFrame(this) != NULL)
			{
				AFXGetTopLevelFrame(this)->SetFocus();
			}
		}
		else
		{
			pMenu->ShowWindow(SW_SHOWNOACTIVATE);
		}
	}

	if (!bSuccess)
	{
		CFrameWnd* pWndMain = AFXGetTopLevelFrame(this);
		if (pWndMain != NULL)
		{
			CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, pWndMain);
			if (pMainFrame != NULL)
			{
				pMainFrame->m_Impl.RemoveTearOffToolbar(pBar);
			}
			else // Maybe, SDI frame...
			{
				CFrameWndEx* pFrame = DYNAMIC_DOWNCAST(CFrameWndEx, pWndMain);
				if (pFrame != NULL)
				{
					pFrame->m_Impl.RemoveTearOffToolbar(pBar);
				}
				else // Maybe, OLE frame...
				{
					COleIPFrameWndEx* pOleFrame = DYNAMIC_DOWNCAST(COleIPFrameWndEx, pWndMain);
					if (pOleFrame != NULL)
					{
						pOleFrame->m_Impl.RemoveTearOffToolbar(pBar);
					}
					else
					{
						COleDocIPFrameWndEx* pOleDocFrame = DYNAMIC_DOWNCAST(COleDocIPFrameWndEx, pWndMain);
						if (pOleDocFrame != NULL)
						{
							pOleDocFrame->m_Impl.RemoveTearOffToolbar(pBar);
						}
					}
				}
			}
		}

		pBar->DestroyWindow();
		delete pBar;
	}

	return bSuccess;
}

AFX_DOCK_TYPE CPaneFrameWnd::GetDockingMode() const
{
	CBasePane* pBar = DYNAMIC_DOWNCAST(CBasePane, GetPane());
	if (pBar != NULL)
	{
		return pBar->GetDockingMode();
	}

	return CDockingManager::GetDockingMode();
}

void CPaneFrameWnd::RemoveNonValidPanes()
{
	CWnd* pWnd = DYNAMIC_DOWNCAST(CWnd, GetPane());
	if (pWnd == NULL || !IsWindow(pWnd->GetSafeHwnd()))
	{
		m_hEmbeddedBar = NULL;
		return;
	}

	if (pWnd->GetDlgCtrlID() != -1)
	{
		// the bar is ok
		return;
	}

	// check for empty tabbed bar with ID -1
	CBaseTabbedPane* pTabbedBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, GetPane());

	if (pTabbedBar == NULL || pTabbedBar->GetTabsNum() == 0)
	{
		// notabbed control bar with ID -1 or tabbed control bar
		// withot tabs - are not valid
		m_hEmbeddedBar = NULL;
		return;
	}
}

void CPaneFrameWnd::OnClose()
{
	if (OnCloseMiniFrame())
	{
		CloseMiniFrame();
	}
}

void __stdcall CPaneFrameWnd::GetPaneList(CObList& lstBars, CRuntimeClass* pRTCFilter, BOOL bIncludeTabs)
{
	for (POSITION pos = CPaneFrameWnd::m_mapFloatingBars.GetStartPosition(); pos != NULL;)
	{
		UINT nID = (UINT) -1;
		HWND hWndBar = NULL;
		CPaneFrameWnd::m_mapFloatingBars.GetNextAssoc(pos, nID, hWndBar);

		if (bIncludeTabs)
		{
			CBaseTabbedPane* pTabbedBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, CWnd::FromHandle(hWndBar));
			if (pTabbedBar != NULL)
			{
				ASSERT_VALID(pTabbedBar);

				pTabbedBar->GetPaneList(lstBars, pRTCFilter);
				continue;
			}
		}

		CBasePane* pBar = DYNAMIC_DOWNCAST(CBasePane, CWnd::FromHandle(hWndBar));

		if (pBar != NULL &&
			(pRTCFilter == NULL || pBar->GetRuntimeClass() == pRTCFilter))
		{
			ASSERT_VALID(pBar);
			lstBars.AddTail(pBar);
		}
	}
}

void CPaneFrameWnd::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);
	m_hLastFocusWnd = NULL;
	if (m_hEmbeddedBar != NULL && IsWindow(m_hEmbeddedBar))
	{
		CWnd* pBar = CWnd::FromHandle(m_hEmbeddedBar);
		if (pBar->IsKindOf(RUNTIME_CLASS(CMFCToolBar)))
		{
			m_hLastFocusWnd = pOldWnd->GetSafeHwnd();
		}
	}

	CBasePane* pFirstBar = DYNAMIC_DOWNCAST(CBasePane, GetFirstVisiblePane());
	if (pFirstBar != NULL && pFirstBar->CanFocus())
	{
		pFirstBar->SetFocus();
	}

	if (GetParentFrame() != NULL)
	{
		GetParentFrame()->SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
}

LRESULT CPaneFrameWnd::OnExitSizeMove(WPARAM, LPARAM)
{
	if (m_hLastFocusWnd != NULL && IsWindow(m_hLastFocusWnd))
	{
		::SetFocus(m_hLastFocusWnd);
		m_hLastFocusWnd = NULL;
	}
	return 0;
}

BOOL CPaneFrameWnd::OnBeforeDock()
{
	if (GetKeyState(VK_CONTROL) < 0)
	{
		return FALSE;
	}

	CDockingManager* pDockManager = m_pDockManager != NULL ? m_pDockManager : afxGlobalUtils.GetDockingManager(GetParent());
	if (pDockManager != NULL)
	{
		CPoint ptMouse;
		GetCursorPos(&ptMouse);
		CRect rectExpected; rectExpected.SetRectEmpty();
		BOOL bDrawTab = FALSE;
		CDockablePane* pTargetBar = NULL;
		pDockManager->CalcExpectedDockedRect(this, ptMouse, rectExpected, bDrawTab, &pTargetBar);
		if (rectExpected.IsRectEmpty() && !bDrawTab)
		{
			return FALSE;
		}
	}

	return TRUE;
}

void CPaneFrameWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CWnd::OnSettingChange(uFlags, lpszSection);
	RecalcCaptionHeight();
}

void CPaneFrameWnd::RecalcCaptionHeight()
{
	m_nCaptionHeight = GetSystemMetrics(SM_CYSMCAPTION) + CMFCVisualManager::GetInstance()->GetCaptionButtonExtraBorder().cy;
	m_sizeMinSize.cx = m_sizeMinSize.cy = m_nCaptionHeight + 15;
}

BOOL CPaneFrameWnd::OnNeedTipText(UINT /*id*/, NMHDR* pNMH, LRESULT* /*pResult*/)
{
	static CString strTipText;

	ENSURE(pNMH != NULL);

	if (m_pToolTip->GetSafeHwnd() == NULL || pNMH->hwndFrom != m_pToolTip->GetSafeHwnd())
	{
		return FALSE;
	}

	if (CMFCPopupMenu::GetActiveMenu() != NULL)
	{
		return FALSE;
	}

	LPNMTTDISPINFO pTTDispInfo = (LPNMTTDISPINFO) pNMH;
	ASSERT((pTTDispInfo->uFlags & TTF_IDISHWND) == 0);

	if (pNMH->idFrom > 0 &&(int)pNMH->idFrom <= m_lstCaptionButtons.GetCount())
	{
		POSITION pos = m_lstCaptionButtons.FindIndex(pNMH->idFrom - 1);
		if (pos != NULL)
		{
			CMFCCaptionButton* pBtn = (CMFCCaptionButton*)m_lstCaptionButtons.GetAt(pos);
			ASSERT_VALID(pBtn);

			switch (pBtn->GetHit())
			{
			case HTCLOSE:
				ENSURE(strTipText.LoadString(IDS_AFXBARRES_CLOSE));
				pTTDispInfo->lpszText = const_cast<LPTSTR>((LPCTSTR) strTipText);
				return TRUE;

			case AFX_HTMENU:
				ENSURE(strTipText.LoadString(IDS_AFXBARRES_TOOLBAR_OPTIONS));
				pTTDispInfo->lpszText = const_cast<LPTSTR>((LPCTSTR) strTipText);
				return TRUE;

			case HTMAXBUTTON:
			case HTMINBUTTON:
				ENSURE(strTipText.LoadString(IDS_AFXBARRES_AUTOHIDEBAR));
				pTTDispInfo->lpszText = const_cast<LPTSTR>((LPCTSTR) strTipText);
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CPaneFrameWnd::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
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
		if (m_pToolTip->GetSafeHwnd() != NULL)
		{
			m_pToolTip->RelayEvent(pMsg);
		}
		break;
	}

	return CWnd::PreTranslateMessage(pMsg);
}

void CPaneFrameWnd::SetCaptionButtonsToolTips()
{
	if (m_pToolTip->GetSafeHwnd() == NULL)
	{
		return;
	}

	if (m_lstCaptionButtons.GetCount() == m_pToolTip->GetToolCount())
	{
		return;
	}

	int i = 0;

	for (i = 0 ; i < m_pToolTip->GetToolCount( ); i++)
	{
		m_pToolTip->DelTool(this, i + 1);
	}

	// Register tool for caption button's tooltip:
	for (i = m_pToolTip->GetToolCount(); i >= 0 && i < m_lstCaptionButtons.GetCount(); i ++)
	{
		CRect rectDummy;
		rectDummy.SetRectEmpty();
		m_pToolTip->AddTool(this, LPSTR_TEXTCALLBACK, &rectDummy, i + 1);
	}
}

void CPaneFrameWnd::OnPressButtons(UINT nHit)
{
	if (CMFCToolBar::IsCustomizeMode() ||(nHit != AFX_HTMENU))
	{
		return;
	}

	//Prepare for Quick Customize Pane
	CMFCCaptionMenuButton* pBtn = (CMFCCaptionMenuButton *)FindButton(nHit);
	if (pBtn == NULL)
	{
		return;
	}

	CWnd* pWnd = GetPane();
	if (pWnd == NULL || pWnd->GetSafeHwnd() == NULL)
	{
		return;
	}

	if (!pWnd->IsKindOf(RUNTIME_CLASS(CMFCToolBar)))
	{
		return;
	}

	CMFCToolBar* pTolBar = DYNAMIC_DOWNCAST(CMFCToolBar, pWnd);

	CMFCCustomizeButton* pCustBtn = pTolBar->GetCustomizeButton();
	if (!pCustBtn) //Not Enabled Customize Button
	{
		return;
	}

	if (pTolBar->IsAddRemoveQuickCustomize())
	{
		pBtn->m_bPushed = TRUE;

		//Get ToolBar's caption
		CString strCaption;
		pTolBar->GetWindowText(strCaption);

		strCaption.TrimLeft();
		strCaption.TrimRight();

		if (strCaption.IsEmpty ())
		{
			ENSURE(strCaption.LoadString(IDS_AFXBARRES_UNTITLED_TOOLBAR));
		}

		CMFCPopupMenu* pMenu = new CMFCPopupMenu();

		// Insert Dummy Menu Item

		CMFCPopupMenu* pMenuDummy = new CMFCPopupMenu();
		pMenuDummy->InsertItem(CMFCToolBarMenuButton(1, NULL, -1, _T("DUMMY")));

		CMFCToolBarMenuButton btnToolCaption((UINT)-1, pMenuDummy->GetMenuBar()->ExportToMenu(), -1, strCaption);

		CMFCToolBarMenuButton btnStandard(pCustBtn->GetCustomizeCmdId(), NULL, -1, pCustBtn->GetCustomizeText());

		CMFCPopupMenu* pMenuCustomize = new CMFCPopupMenu();

		pMenuCustomize->InsertItem(btnToolCaption);
		pMenuCustomize->InsertItem(btnStandard);

		CString strLabel;
		ENSURE(strLabel.LoadString(IDS_AFXBARRES_ADD_REMOVE_BTNS));

		CMFCToolBarMenuButton btnAddRemove((UINT)-1, pMenuCustomize->GetMenuBar()->ExportToMenu(), -1, strLabel);

		btnAddRemove.EnableQuickCustomize();

		delete pMenuDummy;
		delete pMenuCustomize;

		pMenu->InsertItem(btnAddRemove);

		CRect rc = pBtn->GetRect();
		CPoint pt(rc.left, rc.top);
		ClientToScreen(&pt);
		CSize size = pBtn->GetSize();

		pMenu->Create(this, pt.x-2, pt.y-9, NULL);
		pMenu->SetAutoDestroy();
		pMenu->SetOwner(this);
		pMenu->SetQuickMode(); // for Recently used mode
		pMenu->SetQuickCustomizeType(CMFCPopupMenu::QUICK_CUSTOMIZE_ADDREMOVE);
	}
}

BOOL CPaneFrameWnd::OnCloseMiniFrame()
{
	BOOL bCanClose = TRUE;
	CFrameWnd* pWndMain = AFXGetTopLevelFrame(this);
	if (pWndMain != NULL)
	{
		CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, pWndMain);
		if (pMainFrame != NULL)
		{
			bCanClose = pMainFrame->OnCloseMiniFrame(this);
		}
		else // Maybe, SDI frame...
		{
			CFrameWndEx* pFrame = DYNAMIC_DOWNCAST(CFrameWndEx, pWndMain);
			if (pFrame != NULL)
			{
				bCanClose = pFrame->OnCloseMiniFrame(this);
			}
			else // Maybe, OLE frame...
			{
				COleIPFrameWndEx* pOleFrame = DYNAMIC_DOWNCAST(COleIPFrameWndEx, pWndMain);
				if (pOleFrame != NULL)
				{
					bCanClose = pOleFrame->OnCloseMiniFrame(this);
				}
				else
				{
					COleDocIPFrameWndEx* pOleDocFrame = DYNAMIC_DOWNCAST(COleDocIPFrameWndEx, pWndMain);
					if (pOleDocFrame != NULL)
					{
						bCanClose = pOleDocFrame->OnCloseMiniFrame(this);
					}
				}
			}
		}
	}
	return bCanClose;
}

LRESULT CPaneFrameWnd::OnUpdateToolTips(WPARAM wp, LPARAM)
{
	UINT nTypes = (UINT) wp;

	if (nTypes & AFX_TOOLTIP_TYPE_MINIFRAME)
	{
		CTooltipManager::CreateToolTip(m_pToolTip, this, AFX_TOOLTIP_TYPE_MINIFRAME);

		SetCaptionButtonsToolTips();
		UpdateTooltips();
	}

	return 0;
}



