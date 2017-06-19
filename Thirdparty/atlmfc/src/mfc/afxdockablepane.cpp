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
#include "afxglobalutils.h"
#include "afxframewndex.h"
#include "afxmdiframewndex.h"
#include "afxoleipframewndex.h"
#include "afxoledocipframewndex.h"
#include "afxpaneframewnd.h"
#include "afxmultipaneframewnd.h"
#include "afxbasepane.h"
#include "afxdockingpanesrow.h"
#include "afxtabbedpane.h"
#include "afxdrawmanager.h"
#include "afxautohidebutton.h"
#include "afxautohidebar.h"
#include "afxautohidedocksite.h"
#include "afxpanedivider.h"
#include "afxribbonres.h"
#include "afxdockablepane.h"
#include "afxpanecontainermanager.h"
#include "afxoutlookbar.h"
#include "afxmultipaneframewnd.h"
#include "afxpropertygridtooltipctrl.h"
#include "afxtooltipmanager.h"
#include "afxmdichildwndex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int  CDockablePane::m_nTimeOutBeforeAutoHide = 700;
int  CDockablePane::m_nSlideDefaultTimeOut   = 1;
BOOL CDockablePane::m_bHideInAutoHideMode    = FALSE;
int  CDockablePane::m_nSlideSteps            = 12;

static int g_nCloseButtonMargin = 1;
static int g_nCaptionVertMargin = 2;
static int g_nCaptionHorzMargin = 2;

CSize CDockablePane::m_sizeDragSensitivity = CSize(GetSystemMetrics(SM_CXDRAG), GetSystemMetrics(SM_CYDRAG));

BOOL CDockablePane::m_bCaptionText = FALSE;
BOOL CDockablePane::m_bHideDisabledButtons = TRUE;
BOOL CDockablePane::m_bDisableAnimation = FALSE;

IMPLEMENT_SERIAL(CDockablePane, CPane, VERSIONABLE_SCHEMA | 2)

UINT AFX_WM_ON_PRESS_CLOSE_BUTTON = ::RegisterWindowMessage(_T("AFX_WM_ON_PRESS_CLOSE_BUTTON"));

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDockablePane::CDockablePane()
{
	m_bPrepareToFloat = false;
	m_bReadyToFloat = false;

	m_pTabbedControlBarRTC = RUNTIME_CLASS(CTabbedPane);

	m_hDefaultSlider = NULL;
	m_cyGripper = 0;
	m_bHasGripper = FALSE;
	m_nBorderSize = 0;
	m_dwSCBStyle = 0;
	m_bActive = FALSE;

	m_bEnableAutoHideAll = TRUE;

	m_bPinState = FALSE;
	m_nAutoHideConditionTimerID = 0;
	m_nSlideTimer = 0;
	m_nSlideStep = 0;
	m_nSlideDelta = 0;
	m_pAutoHideButton = NULL;
	m_pAutoHideBar = NULL;

	m_ahSlideMode = CDockingManager::m_ahSlideModeGlobal;

	m_bIsSliding = FALSE;
	m_bIsHiding = FALSE;
	m_bIsResizing = FALSE;

	m_nLastPercent = 100;

	m_rectRedraw.SetRectEmpty();
	m_rectRestored.SetRectEmpty();

	m_nHot = HTNOWHERE;
	m_nHit = HTNOWHERE;
	m_bCaptionButtonsCaptured = FALSE;

	m_hRestoredDefaultSlider = NULL;
	m_pToolTip = NULL;
}

CDockablePane::~CDockablePane()
{
}

//{{AFX_MSG_MAP(CDockablePane)
BEGIN_MESSAGE_MAP(CDockablePane, CPane)
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_NCHITTEST()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_DESTROY()
	ON_WM_NCMOUSEMOVE()
	ON_WM_CANCELMODE()
	ON_WM_TIMER()
	ON_WM_RBUTTONDOWN()
	ON_WM_SETTINGCHANGE()
	ON_WM_CONTEXTMENU()
	ON_WM_SETFOCUS()
	ON_MESSAGE(WM_SETTEXT, &CDockablePane::OnSetText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, &CDockablePane::OnNeedTipText)
	ON_REGISTERED_MESSAGE(AFX_WM_UPDATETOOLTIPS, &CDockablePane::OnUpdateToolTips)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

BOOL CDockablePane::Create(LPCTSTR lpszCaption, CWnd* pParentWnd, const RECT& rect, BOOL bHasGripper,
	UINT nID, DWORD dwStyle, DWORD dwTabbedStyle, DWORD dwControlBarStyle, CCreateContext* pContext)
{
	ASSERT_VALID(this);
	return CDockablePane::CreateEx(0, lpszCaption, pParentWnd, rect, bHasGripper, nID, dwStyle, dwTabbedStyle, dwControlBarStyle, pContext);
}

BOOL CDockablePane::Create(LPCTSTR lpszWindowName, CWnd* pParentWnd, CSize sizeDefault, BOOL bHasGripper,
	UINT nID, DWORD dwStyle, DWORD dwTabbedStyle, DWORD dwControlBarStyle)
{
	ASSERT_VALID(this);
	CRect rect(0, 0, sizeDefault.cx, sizeDefault.cy);
	return CDockablePane::CreateEx(0, lpszWindowName, pParentWnd, rect, bHasGripper, nID, dwStyle, dwTabbedStyle, dwControlBarStyle, NULL);
}

BOOL CDockablePane::CreateEx(DWORD dwStyleEx, LPCTSTR lpszCaption, CWnd* pParentWnd, const RECT& rect, BOOL bHasGripper,
	UINT nID, DWORD dwStyle, DWORD dwTabbedStyle, DWORD dwControlBarStyle, CCreateContext* pContext)
{
	ASSERT_VALID(this);

	if (dwStyle & CBRS_FLOAT_MULTI)
	{
		m_pMiniFrameRTC = RUNTIME_CLASS(CMultiPaneFrameWnd);
	}

	if (dwTabbedStyle & AFX_CBRS_OUTLOOK_TABS)
	{
		m_pTabbedControlBarRTC = RUNTIME_CLASS(CMFCOutlookBar);
	}
	else if (dwTabbedStyle & AFX_CBRS_REGULAR_TABS)
	{
		m_pTabbedControlBarRTC = RUNTIME_CLASS(CTabbedPane);
	}

	if (dwStyle & WS_CAPTION || bHasGripper)
	{
		m_bHasGripper = bHasGripper = TRUE;
		dwStyle &= ~WS_CAPTION;
	}

	if (!CPane::CreateEx(dwStyleEx, NULL, dwStyle, rect, pParentWnd, nID, dwControlBarStyle, pContext))
	{
		return FALSE;
	}

	m_rectRestored = rect;

	if (m_sizeDialog != CSize(0, 0))
	{
		m_rectRestored.right = m_rectRestored.left + m_sizeDialog.cx;
		m_rectRestored.bottom = m_rectRestored.top + m_sizeDialog.cy;
	}

	SetPaneAlignment(dwStyle & CBRS_ALIGN_ANY);
	EnableGripper(bHasGripper);

	if (lpszCaption != NULL)
	{
		SetWindowText(lpszCaption);
	}

	return TRUE;
}

void CDockablePane::EnableGripper(BOOL bEnable)
{
	if (bEnable && m_bHasGripper)
	{
		m_cyGripper = afxGlobalData.GetTextHeight() + g_nCaptionVertMargin * 2 + 1;
	}
	else
	{
		m_cyGripper = 0;
	}

	SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

int CDockablePane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPane::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetCaptionButtons();

	if (CTooltipManager::CreateToolTip(m_pToolTip, this, AFX_TOOLTIP_TYPE_DOCKBAR))
	{
		for (int i = 0; i < AFX_CONTROLBAR_BUTTONS_NUM; i ++)
		{
			CRect rectDummy;
			rectDummy.SetRectEmpty();

			m_pToolTip->AddTool(this, LPSTR_TEXTCALLBACK, &rectDummy, i + 1);
		}
	}

	return 0;
}

BOOL CDockablePane::IsDocked() const
{
	ASSERT_VALID(this);
	CPaneFrameWnd* pParent = GetParentMiniFrame();

	if (pParent != NULL && pParent->GetPaneCount() == 1)
	{
		return FALSE;
	}

	return TRUE;
}

void CDockablePane::OnAfterDock(CBasePane* /*pBar*/, LPCRECT /*lpRect*/, AFX_DOCK_METHOD /*dockMethod*/)
{
	if (!CDockingManager::m_bRestoringDockState)
	{
		SetFocus();
	}

	if (GetDockingMode() == DT_IMMEDIATE)
	{
		GetCursorPos(&m_ptClientHotSpot);
		ScreenToClient(&m_ptClientHotSpot);
	}

	if (GetDlgCtrlID() != -1 && GetParentMiniFrame() == NULL)
	{
		CPaneFrameWnd::AddRemovePaneFromGlobalList(this, FALSE /* remove*/);
	}

}

void CDockablePane::OnBeforeChangeParent(CWnd* pWndNewParent, BOOL bDelay)
{
	ASSERT_VALID(this);

	if (pWndNewParent != NULL)
	{
		BOOL bIsMDIChild = pWndNewParent->IsKindOf(RUNTIME_CLASS(CMDIChildWndEx));

		if (bIsMDIChild)
		{
			StoreRecentDockSiteInfo();
		}

		// is being floated or tabbed
		if (pWndNewParent->IsKindOf(RUNTIME_CLASS(CPaneFrameWnd)) || pWndNewParent->IsKindOf(RUNTIME_CLASS(CMFCTabCtrl)) || bIsMDIChild)
		{
			UndockPane(bDelay);
		}

		CPane::OnBeforeChangeParent(pWndNewParent);
	}
}

void CDockablePane::RemoveFromDefaultPaneDividier()
{
	ASSERT_VALID(this);

	if (m_hDefaultSlider != NULL)
	{
		// slider will be deleted here(by delete this) if it was a last
		// control bar registered with the slider
		SetDefaultPaneDivider(NULL);
	}
}

void CDockablePane::OnAfterChangeParent(CWnd* pWndOldParent)
{
	ASSERT_VALID(this);
	CPane::OnAfterChangeParent(pWndOldParent);

	CPaneFrameWnd* pMiniFrameParent = GetParentMiniFrame();
	if (pMiniFrameParent != NULL)
	{
		pMiniFrameParent->AddRemovePaneFromGlobalList(this, TRUE);
	}
}

void CDockablePane::UpdateTooltips()
{
	if (m_pToolTip->GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rcBar;
	GetWindowRect(rcBar);
	ScreenToClient(rcBar);

	for (int i = 0; i < m_arrButtons.GetSize() && i < m_pToolTip->GetToolCount( ); i ++)
	{
		CMFCCaptionButton* pbtn = m_arrButtons [i];
		ASSERT_VALID(pbtn);

		CRect rectTT = pbtn->GetRect();
		rectTT.OffsetRect(rcBar.TopLeft());
		m_pToolTip->SetToolRect(this, i + 1, rectTT);
	}
}

void CDockablePane::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	ASSERT_VALID(this);
	CPane::OnNcCalcSize(bCalcValidRects, lpncsp);

	if (IsFloating())
	{
		for (int i = 0; i < m_arrButtons.GetSize(); i ++)
		{
			CMFCCaptionButton* pbtn = m_arrButtons [i];
			ASSERT_VALID(pbtn);

			pbtn->m_bHidden = TRUE;
		}

		return;
	}

	int cyGripper = GetCaptionHeight();

	CRect rcClient = lpncsp->rgrc[0];
	rcClient.DeflateRect(0, cyGripper, 0, 0);

	// "hide" and "expand" buttons positioning:
	CSize sizeButton = CMFCCaptionButton::GetSize();
	CPoint ptOrgBtnRight = CPoint(rcClient.right - sizeButton.cx - g_nCaptionHorzMargin, rcClient.top - cyGripper - m_nBorderSize +(cyGripper - sizeButton.cy) / 2);
	CPoint ptOrgBtnLeft = CPoint(rcClient.left + g_nCaptionHorzMargin, ptOrgBtnRight.y);

	CRect rcBar;
	GetWindowRect(rcBar);
	ScreenToClient(rcBar);

	BOOL bHidePinBtn = !CanAutoHide();

	if (cyGripper > 0)
	{
		int i = 0;

		for (i = 0; i < m_arrButtons.GetSize(); i ++)
		{
			CMFCCaptionButton* pbtn = m_arrButtons [i];
			ASSERT_VALID(pbtn);

			UINT unHit = pbtn->GetHit();

			BOOL bHide = FALSE;
			if (m_bHideDisabledButtons)
			{
				bHide = bHidePinBtn && unHit == HTMAXBUTTON || !CanBeClosed() && unHit == AFX_HTCLOSE;
			}

			if (!CDockingManager::IsDockSiteMenu() && unHit == HTMINBUTTON)
			{
				bHide = TRUE;
			}

			pbtn->m_bFocused = pbtn->m_bPushed = FALSE;

			if (pbtn->m_bLeftAlign)
			{
				pbtn->Move(ptOrgBtnLeft - CRect(lpncsp->rgrc[0]).TopLeft(), bHide);

				if (!bHide)
				{
					ptOrgBtnLeft.Offset(sizeButton.cx + 2, 0);
				}
			}
			else
			{
				pbtn->Move(ptOrgBtnRight - CRect(lpncsp->rgrc[0]).TopLeft(), bHide);

				if (!bHide)
				{
					ptOrgBtnRight.Offset(- sizeButton.cx - 2, 0);
				}
			}
		}

		// Hide left aligned buttons if there is no room for them:
		for (i = 0; i < m_arrButtons.GetSize(); i ++)
		{
			CMFCCaptionButton* pbtn = m_arrButtons [i];
			ASSERT_VALID(pbtn);

			if (pbtn->m_bLeftAlign)
			{
				pbtn->m_bHidden = CRect(lpncsp->rgrc[0]).left + pbtn->GetRect().left >= ptOrgBtnRight.x;
			}
		}
	}
	else
	{
		for (int i = 0; i < m_arrButtons.GetSize(); i ++)
		{
			CMFCCaptionButton* pbtn = m_arrButtons [i];
			ASSERT_VALID(pbtn);

			pbtn->m_bHidden = TRUE;
		}
	}

	rcClient.right = max(rcClient.right, rcClient.left);
	rcClient.bottom = max(rcClient.bottom, rcClient.top);

	lpncsp->rgrc[0] = rcClient;

	UpdateTooltips();
}

void CDockablePane::OnNcPaint()
{
	if (m_bMultiThreaded)
	{
		m_CriticalSection.Lock();
	}

	ASSERT_VALID(this);

	// get window DC that is clipped to the non-client area
	CWindowDC dcPaint(this);

	CRect rectUpd;
	GetUpdateRect(rectUpd);

	CRect rcClient, rcBar;
	GetClientRect(rcClient);
	ClientToScreen(rcClient);
	GetWindowRect(rcBar);

	rcClient.OffsetRect(-rcBar.TopLeft());
	rcBar.OffsetRect(-rcBar.TopLeft());

	CDC* pDC = &dcPaint;
	BOOL m_bMemDC = FALSE;
	CDC dcMem;
	CBitmap bmp;
	CBitmap* pOldBmp = NULL;

	if (dcMem.CreateCompatibleDC(&dcPaint) && bmp.CreateCompatibleBitmap(&dcPaint, rcBar.Width(), rcBar.Height()))
	{
		// Off-screen DC successfully created. Better paint to it then!
		m_bMemDC = TRUE;
		pOldBmp = dcMem.SelectObject(&bmp);
		pDC = &dcMem;
	}

	// client area is not our bussiness :)
	dcPaint.ExcludeClipRect(rcClient);

	CRgn rgn;
	if (!m_rectRedraw.IsRectEmpty())
	{
		rgn.CreateRectRgnIndirect(m_rectRedraw);
		dcPaint.SelectClipRgn(&rgn);
	}

	// erase parts not drawn
	dcPaint.IntersectClipRect(rcBar);

	// erase NC background the hard way
	CMFCVisualManager::GetInstance()->OnFillBarBackground(pDC, this, rcBar, rcBar, TRUE /* NC area */);

	int cyGripper = GetCaptionHeight();

	if (cyGripper > 0)
	{
		// Paint caption and buttons:
		CRect rectCaption;

		GetWindowRect(&rectCaption);
		ScreenToClient(&rectCaption);

		rectCaption.OffsetRect(-rectCaption.left, -rectCaption.top);
		rectCaption.DeflateRect(0, 1);

		rectCaption.left = rcClient.left;
		rectCaption.top --;
		rectCaption.bottom = rectCaption.top + cyGripper - 2;

		DrawCaption(pDC, rectCaption);

		for (int i = 0; i < m_arrButtons.GetSize(); i ++)
		{
			CMFCCaptionButton* pbtn = m_arrButtons [i];
			ASSERT_VALID(pbtn);

			BOOL bIsMax = FALSE;

			switch (pbtn->GetHit())
			{
			case HTMAXBUTTON:
				bIsMax = m_bPinState;
				break;

			case HTMINBUTTON:
				bIsMax = TRUE;
				break;
			}

			pbtn->OnDraw(pDC, m_bActive, IsHorizontal(), bIsMax);
			pbtn->m_clrForeground = (COLORREF)-1;
		}
	}

	if (m_bMemDC)
	{
		// Copy the results to the on-screen DC:
		dcPaint.BitBlt(rcBar.left, rcBar.top, rcBar.Width(), rcBar.Height(), &dcMem, rcBar.left, rcBar.top, SRCCOPY);

		dcMem.SelectObject(pOldBmp);
	}

	dcPaint.SelectClipRgn(NULL);

	if (m_bMultiThreaded)
	{
		m_CriticalSection.Unlock();
	}
}

void CDockablePane::OnDrawDragRect(LPCRECT lprectNew, LPCRECT lprectOld)
{
	ASSERT_VALID(this);
	CWindowDC dcWnd(GetDesktopWindow());
	dcWnd.DrawDragRect(lprectNew, CSize(1, 1), lprectOld, CSize(1, 1));
}

LRESULT CDockablePane::OnNcHitTest(CPoint point)
{
	ASSERT_VALID(this);
	UINT nHitTest = HitTest(point);
	if (nHitTest != HTERROR)
	{
		return nHitTest;
	}
	return CPane::OnNcHitTest(point);
}

int CDockablePane::HitTest(CPoint point, BOOL bDetectCaption)
{
	ASSERT_VALID(this);
	CRect rectWnd;
	GetWindowRect(&rectWnd);

	if (!rectWnd.PtInRect(point))
	{
		return HTNOWHERE;
	}

	CDockingManager* pDockManager = afxGlobalUtils.GetDockingManager(GetDockSiteFrameWnd());
	ENSURE(pDockManager != NULL || afxGlobalUtils.m_bDialogApp);

	// should return hite test of client or caption only in the lock update mode
	if (pDockManager != NULL && !pDockManager->m_bLockUpdate)
	{
		for (int i = 0; i < m_arrButtons.GetSize(); i ++)
		{
			CMFCCaptionButton* pbtn = m_arrButtons [i];
			ASSERT_VALID(pbtn);

			CRect rc = pbtn->GetRect();
			rc.OffsetRect(rectWnd.TopLeft());
			if (rc.PtInRect(point))
			{
				return pbtn->GetHit();
			}
		}
	}

	CRect rectClient;
	GetClientRect(&rectClient);
	ClientToScreen(&rectClient);

	if (rectClient.PtInRect(point))
	{
		return HTCLIENT;
	}

	if (IsDocked())
	{
		CRect rect;
		int nBorderWidth  = 0;
		int nBorderHeight = 1;
		// caption
		rect.SetRect(rectWnd.left + nBorderWidth, rectWnd.top + nBorderHeight, rectWnd.right - nBorderWidth, rectWnd.top + nBorderHeight + GetCaptionHeight());
		if (rect.PtInRect(point))
		{
			return bDetectCaption ? HTCAPTION : HTCLIENT;
		}
	}

	return HTERROR;
}

CSize CDockablePane::CalcFixedLayout(BOOL /*bStretch*/, BOOL /*bHorz*/)
{
	ASSERT_VALID(this);
	CRect rectWnd;
	GetWindowRect(&rectWnd);
	CSize size = rectWnd.Size();
	return size;
}

void CDockablePane::OnPaint()
{
	ASSERT_VALID(this);
	CPaintDC dc(this); // device context for painting
}

AFX_CS_STATUS CDockablePane::IsChangeState(int nOffset, CBasePane** ppTargetBar) const
{
	ASSERT_VALID(this);
	ENSURE(ppTargetBar != NULL);

	CPoint ptMouse;
	GetCursorPos(&ptMouse);

	CWnd* pParentWnd = GetParent();

	if (pParentWnd->IsKindOf(RUNTIME_CLASS(CPaneFrameWnd)))
	{
		CPaneFrameWnd* pMiniFrame = DYNAMIC_DOWNCAST(CPaneFrameWnd, pParentWnd);
		pParentWnd = pMiniFrame->GetParent();
	}

	CDockingManager* pDockManager = afxGlobalUtils.GetDockingManager(pParentWnd);

	if (pDockManager == NULL)
	{
		return CS_NOTHING;
	}

	return pDockManager->DeterminePaneAndStatus(ptMouse, nOffset, GetEnabledAlignment(), ppTargetBar, this, this);
}

void CDockablePane::OnLButtonDown(UINT nFlags, CPoint point)
{
	ASSERT_VALID(this);

	if (m_nHot != HTNOWHERE)
	{
		CMFCCaptionButton* pBtn = FindButtonByHit(m_nHot);
		if (pBtn != NULL)
		{
			SetFocus();

			m_nHit = m_nHot;
			pBtn->m_bPushed = TRUE;
			RedrawButton(pBtn);
			return;
		}
	}
	else
	{
		CWnd* pWndChild = GetWindow(GW_CHILD);
		CWnd* pWndFirstChild = NULL;
		int nCount = 0;

		while (pWndChild != NULL)
		{
			pWndFirstChild = pWndChild;
			pWndChild = pWndChild->GetNextWindow();
			nCount++;
		}

		if (nCount == 1)
		{
			pWndFirstChild->SetFocus();
		}
	}

	if (!IsAutoHideMode() && !IsTabbed())
	{
		if (CanFloat())
		{
			m_bPrepareToFloat = true;
		}

		CPane::OnLButtonDown(nFlags, point);
	}

	SetFocus();
}

void CDockablePane::StoreRecentDockSiteInfo()
{
	CPaneFrameWnd* pParentMiniFrame = GetParentMiniFrame();

	CDockablePane* pBarToSave = this;

	if (IsTabbed())
	{
		CMFCBaseTabCtrl* pTabWnd = DYNAMIC_DOWNCAST(CMFCBaseTabCtrl, GetParent());
		if (pTabWnd != NULL)
		{
			pBarToSave = DYNAMIC_DOWNCAST(CDockablePane, pTabWnd->GetParent());
		}
	}

	CPaneDivider* pDefaultSlider = pBarToSave->GetDefaultPaneDivider();

	if (pParentMiniFrame != NULL)
	{
		pParentMiniFrame->StoreRecentDockSiteInfo(pBarToSave);
	}
	else if (pDefaultSlider != NULL)
	{
		pDefaultSlider->StoreRecentDockSiteInfo(pBarToSave);
	}
}

void CDockablePane::StoreRecentTabRelatedInfo()
{
	if (!IsTabbed())
	{
		return;
	}

	CDockablePane* pParentTabbedBar = NULL;

	CMFCBaseTabCtrl* pTabWnd = DYNAMIC_DOWNCAST(CMFCBaseTabCtrl, GetParent());
	if (pTabWnd != NULL)
	{
		pParentTabbedBar = DYNAMIC_DOWNCAST(CDockablePane, pTabWnd->GetParent());
	}

	if (pParentTabbedBar == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	CPaneFrameWnd* pParentMiniFrame = GetParentMiniFrame();
	CPaneDivider* pDefaultSlider = pParentTabbedBar->GetDefaultPaneDivider();

	if (pParentMiniFrame != NULL)
	{
		pParentMiniFrame->StoreRecentTabRelatedInfo(this, pParentTabbedBar);
	}
	else if (pDefaultSlider != NULL)
	{
		pDefaultSlider->StoreRecentTabRelatedInfo(this, pParentTabbedBar);
	}
}

void CDockablePane::OnRButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();

	CWnd* pMenu = CMFCPopupMenu::GetActiveMenu();

	if (pMenu != NULL && CWnd::FromHandlePermanent(pMenu->GetSafeHwnd()) != NULL)
	{
		CMFCPopupMenu::UpdateAllShadows();
	}

	CPane::OnRButtonDown(nFlags, point);
}

void CDockablePane::OnMouseMove(UINT nFlags, CPoint point)
{
	ASSERT_VALID(this);
	CPoint ptMouse;
	GetCursorPos(&ptMouse);

	if ((GetDockingMode() & DT_IMMEDIATE) != 0)
	{
		if ((!m_bCaptured && GetCapture() == this || m_bCaptured && GetCapture() != this || (GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0) && !m_bCaptionButtonsCaptured)
		{
			ReleaseCapture();
			m_bCaptured = false;
			m_bPrepareToFloat = false;
		}
		if (m_bPrepareToFloat)
		{
			CRect rectBar;
			GetWindowRect(rectBar);

			if (!m_bReadyToFloat)
			{
				m_bReadyToFloat = rectBar.PtInRect(ptMouse) == TRUE;
			}

			CRect rectLast = m_rectDragImmediate;

			CPoint ptOffset = ptMouse - m_dragFrameImpl.m_ptHot;
			m_dragFrameImpl.m_ptHot = ptMouse;

			CPoint ptClientHot = m_ptClientHotSpot;
			ClientToScreen(&ptClientHot);
			CPoint ptDragOffset = ptMouse - ptClientHot;

			UpdateVirtualRect(ptOffset);

			if ((abs(ptDragOffset.x) > m_sizeDragSensitivity.cx || abs(ptDragOffset.y) > m_sizeDragSensitivity.cy) && m_bReadyToFloat)
			{
				if (IsTabbed())
				{
					CMFCBaseTabCtrl* pParentTab = DYNAMIC_DOWNCAST(CMFCBaseTabCtrl, GetParent());
					if (pParentTab != NULL)
					{
						pParentTab->DetachTab(DM_MOUSE);
					}
				}
				else
				{
					FloatPane(m_recentDockInfo.m_rectRecentFloatingRect, DM_MOUSE);
					CDockingManager* pDockManager = afxGlobalUtils.GetDockingManager(GetDockSiteFrameWnd());
					afxGlobalUtils.ForceAdjustLayout(pDockManager);
				}
				m_bPrepareToFloat = false;
				m_bReadyToFloat = false;
			}
			return;
		}
	}
	else if ((GetDockingMode() & DT_STANDARD) != 0 && m_bPrepareToFloat)
	{
		CPane::OnMouseMove(nFlags, point);
		return;
	}

	CPoint ptScreen = point;
	ClientToScreen(&ptScreen);

	OnTrackCaptionButtons(ptScreen);
}

void CDockablePane::OnLButtonUp(UINT nFlags, CPoint point)
{
	ASSERT_VALID(this);

	if (m_bPrepareToFloat)
	{
		m_bPrepareToFloat = false;
	}

	if (m_nHit != HTNOWHERE)
	{
		CDockingManager* pDockManager = afxGlobalUtils.GetDockingManager(GetDockSiteFrameWnd());
		ENSURE(pDockManager != NULL || afxGlobalUtils.m_bDialogApp);

		UINT nHot = m_nHot;
		UINT nHit = m_nHit;

		StopCaptionButtonsTracking();

		CPaneDivider* pDefaultSlider = GetDefaultPaneDivider();

		if (nHot == nHit)
		{
			switch (nHit)
			{
			case AFX_HTCLOSE:
				{
					BOOL bCanClose = TRUE;
					CFrameWnd* pWndMain = AFXGetTopLevelFrame(this);
					if (pWndMain != NULL)
					{
						CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, pWndMain);
						if (pMainFrame != NULL)
						{
							bCanClose = pMainFrame->OnCloseDockingPane(this);
						}
						else // Maybe, SDI frame...
						{
							CFrameWndEx* pFrame = DYNAMIC_DOWNCAST(CFrameWndEx, pWndMain);
							if (pFrame != NULL)
							{
								bCanClose = pFrame->OnCloseDockingPane(this);
							}
							else // Maybe, OLE frame...
							{
								COleIPFrameWndEx* pOleFrame = DYNAMIC_DOWNCAST(COleIPFrameWndEx, pWndMain);
								if (pOleFrame != NULL)
								{
									bCanClose = pOleFrame->OnCloseDockingPane(this);
								}
								else
								{
									COleDocIPFrameWndEx* pOleDocFrame = DYNAMIC_DOWNCAST(COleDocIPFrameWndEx, pWndMain);
									if (pOleDocFrame != NULL)
									{
										bCanClose = pOleDocFrame->OnCloseDockingPane(this);
									}
								}
							}
						}
					}

					if (bCanClose)
					{
						OnPressCloseButton();
					}
					break;
				}

			case HTMAXBUTTON:
				if (GetAsyncKeyState(VK_CONTROL) && IsAutohideAllEnabled())
				{
					m_pDockSite->SetRedraw(FALSE);
					if (!m_bPinState)
					{
						CObList lstBars;
						pDefaultSlider->GetPanes(lstBars);

						for (POSITION pos = lstBars.GetHeadPosition(); pos != NULL;)
						{
							CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, lstBars.GetNext(pos));
							if (pBar->IsAutohideAllEnabled())
							{
								pBar->SetAutoHideMode(TRUE, pDefaultSlider->GetCurrentAlignment(), NULL, FALSE);
							}
						}
					}
					else
					{
						CAutoHideDockSite* pParentDockBar = DYNAMIC_DOWNCAST(CAutoHideDockSite, m_pAutoHideBar->GetParentDockSite());

						if (pParentDockBar != NULL)
						{
							pParentDockBar->UnSetAutoHideMode(NULL);
						}
					}

					m_pDockSite->SetRedraw(TRUE);

					CFrameWnd* pFrame = DYNAMIC_DOWNCAST(CFrameWnd, m_pDockSite);
					if (pFrame != NULL)
					{
						pFrame->RecalcLayout();
					}

					m_pDockSite->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
				}
				else
				{
					if (pDockManager != NULL && pDefaultSlider != NULL &&
						(pDefaultSlider->GetCurrentAlignment() & pDockManager->GetEnabledAutoHideAlignment()))
					{
						SetAutoHideMode(!m_bPinState, pDefaultSlider->GetCurrentAlignment());
					}
				}
				return;

			case HTMINBUTTON:
				if (CDockingManager::IsDockSiteMenu())
				{
					CMFCCaptionButton* pMenuButton = FindButtonByHit(HTMINBUTTON);
					if (pMenuButton == NULL)
					{
						ASSERT(FALSE);
						return;
					}

					CRect rectButton = pMenuButton->GetRect();

					CRect rcBar;
					GetWindowRect(rcBar);
					ScreenToClient(rcBar);

					rectButton.OffsetRect(rcBar.TopLeft());

					ClientToScreen(&rectButton);

					pMenuButton->m_bDroppedDown = TRUE;

					CPoint ptMenu(rectButton.left, rectButton.bottom + 1);

					if (GetExStyle() & WS_EX_LAYOUTRTL)
					{
						ptMenu.x += rectButton.Width();
					}

					HWND hwndThis = GetSafeHwnd();

					OnShowControlBarMenu(ptMenu);

					if (::IsWindow(hwndThis))
					{
						pMenuButton->m_bDroppedDown = FALSE;
						RedrawButton(pMenuButton);
					}
				}
				return;

			default:
				OnPressButtons(nHit);
			}
		}

		CWnd::OnLButtonUp(nFlags, point);
		return;
	}

	CPane::OnLButtonUp(nFlags, point);
}

void CDockablePane::OnPressCloseButton()
{
	CFrameWnd* pParentFrame = DYNAMIC_DOWNCAST(CFrameWnd, AFXGetParentFrame(this));
	ASSERT_VALID(pParentFrame);

	if (pParentFrame != NULL)
	{
		if (pParentFrame->SendMessage(AFX_WM_ON_PRESS_CLOSE_BUTTON, NULL, (LPARAM)(LPVOID) this))
		{
			return;
		}
	}

	if (IsAutoHideMode())
	{
		SetAutoHideMode(FALSE, GetCurrentAlignment());
	}
	ShowPane(FALSE, FALSE, FALSE);
	AdjustDockingLayout();
}

void CDockablePane::EnterDragMode(BOOL bChangeHotPoint)
{
	m_bPrepareToFloat = true;
	CPane::EnterDragMode(bChangeHotPoint);
}

CMFCAutoHideBar* CDockablePane::SetAutoHideMode(BOOL bMode, DWORD dwAlignment, CMFCAutoHideBar* pCurrAutoHideBar, BOOL bUseTimer)
{
	ASSERT_VALID(this);
	ASSERT(dwAlignment & CBRS_ALIGN_ANY);

	if (bMode == IsAutoHideMode())
	{
		return pCurrAutoHideBar;
	}

	CDockingManager* pDockManager = afxGlobalUtils.GetDockingManager(GetDockSiteFrameWnd());
	ASSERT_VALID(pDockManager);

	if (bMode)
	{
		m_bPinState = TRUE;

		CRect rectBeforeUndock;
		GetWindowRect(rectBeforeUndock);
		GetDockSiteFrameWnd()->ScreenToClient(rectBeforeUndock);

		StoreRecentDockSiteInfo();

		// set autohide mode
		UndockPane(FALSE);

		CPaneDivider* pDefaultSlider = GetDefaultPaneDivider();
		ENSURE(pDefaultSlider == NULL);
		pDefaultSlider = CreateDefaultPaneDivider(dwAlignment, GetDockSiteFrameWnd());

		if (pDefaultSlider == NULL)
		{
			TRACE0("Failed to create default slider\n");
			DockPane(this, NULL, DM_DBL_CLICK);
			return NULL;
		}

		m_hDefaultSlider = pDefaultSlider->m_hWnd;

		pDefaultSlider->SetAutoHideMode(TRUE);
		pDefaultSlider->AddPane(this);

		SetPaneAlignment(dwAlignment);
		pDefaultSlider->SetPaneAlignment(dwAlignment);

		pCurrAutoHideBar = pDockManager->AutoHidePane(this, pCurrAutoHideBar);

		if (IsPaneVisible())
		{
			pDefaultSlider->RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
			RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);

			GetDockSiteFrameWnd()->RedrawWindow(rectBeforeUndock,  NULL, RDW_INVALIDATE | RDW_UPDATENOW |  RDW_ALLCHILDREN);
		}
		else
		{
			ShowWindow(SW_SHOW);
		}

		if (bUseTimer)
		{
			m_nAutoHideConditionTimerID = SetTimer(AFX_ID_CHECK_AUTO_HIDE_CONDITION, m_nTimeOutBeforeAutoHide, NULL);
			Slide(FALSE, TRUE);
			GetDockSiteFrameWnd()->SetFocus();
		}
		else
		{
			Slide(FALSE, FALSE);
		}

		SetWindowPos(NULL, -1, -1, -1, -1, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW | SWP_FRAMECHANGED);
	}
	else if (m_pAutoHideBar != NULL)
	{
		CAutoHideDockSite* pParentDockBar = DYNAMIC_DOWNCAST(CAutoHideDockSite, m_pAutoHideBar->GetParentDockSite());

		if (pParentDockBar != NULL)
		{
			pParentDockBar->UnSetAutoHideMode(m_pAutoHideBar);
		}
	}

	return pCurrAutoHideBar;
}

void CDockablePane::UnSetAutoHideMode(CDockablePane* pFirstBarInGroup)
{
	m_bPinState = FALSE;

	if (m_nAutoHideConditionTimerID != 0)
	{
		KillTimer(m_nAutoHideConditionTimerID);
	}

	if (m_nSlideTimer != 0)
	{
		KillTimer(m_nSlideTimer);
	}

	BOOL bWasActive = m_pAutoHideBar->m_bActiveInGroup;

	m_pAutoHideBar->RemoveAutoHideWindow(this);

	RemoveFromDefaultPaneDividier();
	// unset autohide mode - make it docked back
	if (pFirstBarInGroup == NULL)
	{
		if (!DockPane(this, NULL, DM_DBL_CLICK))
		{
			return;
		}
	}
	else
	{
		AttachToTabWnd(pFirstBarInGroup, DM_SHOW, bWasActive);
	}
	ShowPane(TRUE, FALSE, bWasActive);
	AdjustDockingLayout();
}

void CDockablePane::OnTimer(UINT_PTR nIDEvent)
{
	BOOL bSlideDirection = FALSE;

	switch (nIDEvent)
	{
	case AFX_ID_CHECK_AUTO_HIDE_CONDITION:
		if (CheckAutoHideCondition())
		{
			KillTimer(m_nAutoHideConditionTimerID);
			m_nAutoHideConditionTimerID = 0;
		}
		return;

	case AFX_AUTO_HIDE_SLIDE_OUT_EVENT:
		bSlideDirection = TRUE;
		m_bIsHiding = FALSE;
		break;

	case AFX_AUTO_HIDE_SLIDE_IN_EVENT:
		bSlideDirection = FALSE;
		m_bIsHiding = TRUE;
		break;

	default:
		CPane::OnTimer(nIDEvent);
		return;
	}

	OnSlide(bSlideDirection);

	if (CheckStopSlideCondition(bSlideDirection))
	{
		KillTimer(m_nSlideTimer);

		m_bIsSliding = FALSE;
		m_nSlideTimer = 0;
		m_nSlideStep = 0;

		if (bSlideDirection) // slide out - show
		{

			RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);

			::RedrawWindow(m_hDefaultSlider, NULL, NULL, RDW_INVALIDATE);
			// one second time out to give the user ability to move the mouse over
			// miniframe
			if (m_nAutoHideConditionTimerID != 0)
			{
				KillTimer(m_nAutoHideConditionTimerID);
			}

			m_nAutoHideConditionTimerID = SetTimer(AFX_ID_CHECK_AUTO_HIDE_CONDITION, m_nTimeOutBeforeAutoHide, NULL);
		}
		else
		{
			ShowWindow(SW_HIDE);
			CPaneDivider* pDefaultSlider = GetDefaultPaneDivider();
			ASSERT_VALID(pDefaultSlider);
			pDefaultSlider->ShowWindow(SW_HIDE);
		}
	}

	CPane::OnTimer(nIDEvent);
}

// Returns TRUE when the dock bar should be hidden(strats slide in)

BOOL CDockablePane::CheckAutoHideCondition()
{
	if (m_bActive || m_bIsResizing || !IsAutoHideMode() || CMFCPopupMenu::GetActiveMenu() != NULL)
	{
		return FALSE;
	}

	if (m_pToolTip->GetSafeHwnd() != NULL && m_pToolTip->IsWindowVisible())
	{
		return FALSE;
	}

	ASSERT_VALID(m_pAutoHideButton);
	ASSERT_VALID(m_pAutoHideBar);

	CRect rectAutoHideBtn = m_pAutoHideButton->GetRect();
	m_pAutoHideBar->ClientToScreen(&rectAutoHideBtn);

	CPoint ptCursor;
	GetCursorPos(&ptCursor);

	CWnd* pWndFromPoint = WindowFromPoint(ptCursor);
	BOOL bCursorOverThisWindow = FALSE; // and this is topmost window
	while (pWndFromPoint != NULL)
	{
		if (pWndFromPoint == this || pWndFromPoint->m_hWnd == m_hDefaultSlider || pWndFromPoint->IsKindOf(RUNTIME_CLASS(CMFCPropertyGridToolTipCtrl)))
		{
			bCursorOverThisWindow = TRUE;
			break;
		}
		pWndFromPoint = pWndFromPoint->GetParent();
	}

	CRect rectWnd;
	GetWindowRect(rectWnd);
	CRect rectSlider;
	::GetWindowRect(m_hDefaultSlider, &rectSlider);

	rectWnd.UnionRect(rectWnd, rectSlider);

	if (rectWnd.PtInRect(ptCursor) ||  bCursorOverThisWindow || rectAutoHideBtn.PtInRect(ptCursor))
	{
		return FALSE;
	}

	Slide(FALSE);

	return TRUE;
}

BOOL CDockablePane::CheckStopSlideCondition(BOOL bDirection)
{
	if (!IsAutoHideMode())
	{
		return TRUE;
	}

	CRect rectWnd;
	GetWindowRect(rectWnd);

	GetDockSiteFrameWnd()->ScreenToClient(rectWnd);
	BOOL bIsRTL = GetDockSiteFrameWnd()->GetExStyle() & WS_EX_LAYOUTRTL;

	CRect rectAutoHideDockBar;
	m_pAutoHideBar->GetParentDockSite()->GetWindowRect(rectAutoHideDockBar);
	GetDockSiteFrameWnd()->ScreenToClient(rectAutoHideDockBar);

	BOOL bStop = FALSE;
	switch (GetCurrentAlignment())
	{

	case CBRS_ALIGN_RIGHT:
		if (m_ahSlideMode == AFX_AHSM_MOVE)
		{
			if (bIsRTL)
			{
				bStop = bDirection ? rectWnd.left >= rectAutoHideDockBar.right : rectWnd.right <= rectAutoHideDockBar.right;
			}
			else
			{
				bStop = bDirection ? rectWnd.right <= rectAutoHideDockBar.left : rectWnd.left >= rectAutoHideDockBar.left;
			}
		}
		else
		{
			bStop = bDirection ? rectWnd.Width() >= m_rectRestored.Width() : rectWnd.Width() <= 0;
		}
		break;

	case CBRS_ALIGN_LEFT:
		if (m_ahSlideMode == AFX_AHSM_MOVE)
		{
			if (bIsRTL)
			{
				bStop = bDirection ? rectWnd.right <= rectAutoHideDockBar.left : rectWnd.left >= rectAutoHideDockBar.left;
			}
			else
			{
				bStop = bDirection ? rectWnd.left >= rectAutoHideDockBar.right : rectWnd.right <= rectAutoHideDockBar.right;
			}
		}
		else
		{
			bStop = bDirection ? rectWnd.Width() >= m_rectRestored.Width() : rectWnd.Width() <= 0;
		}
		break;

	case CBRS_ALIGN_TOP:
		if (m_ahSlideMode == AFX_AHSM_MOVE)
		{
			bStop = bDirection ? rectWnd.top >= rectAutoHideDockBar.bottom : rectWnd.bottom <= rectAutoHideDockBar.bottom;
		}
		else
		{
		}
		break;

	case CBRS_ALIGN_BOTTOM:
		if (m_ahSlideMode == AFX_AHSM_MOVE)
		{
			bStop = bDirection ? rectWnd.bottom <= rectAutoHideDockBar.top : rectWnd.top >= rectAutoHideDockBar.top;
		}
		else
		{
			bStop = bDirection ? rectWnd.Height() >= m_rectRestored.Height() : rectWnd.Height() <= 0;
		}
		break;
	}

	return bStop;
}

void CDockablePane::OnSlide(BOOL bSlideDirection)
{
	if (!IsAutoHideMode() && !IsWindow(m_hDefaultSlider))
	{
		return;
	}

	BOOL bIsRTL = GetDockSiteFrameWnd()->GetExStyle() & WS_EX_LAYOUTRTL;

	m_nSlideStep++;

	CRect rect;
	GetWindowRect(&rect);
	GetDockSiteFrameWnd()->ScreenToClient(&rect);

	CRect rectSlider;
	::GetWindowRect(m_hDefaultSlider, &rectSlider);
	GetDockSiteFrameWnd()->ScreenToClient(&rectSlider);

	if (m_ahSlideMode == AFX_AHSM_MOVE)
	{
		OffsetRectForSliding(rect, bSlideDirection, bIsRTL);
		OffsetRectForSliding(rectSlider, bSlideDirection, bIsRTL);
		if (bSlideDirection)
		{
			CPoint pt = CalcCorrectOffset(rect, bIsRTL);
			rect.OffsetRect(pt);
			rectSlider.OffsetRect(pt);
		}
	}
	else
	{
		CalcRectForSliding(rect, rectSlider, bSlideDirection);
	}

	SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	::SetWindowPos(m_hDefaultSlider, NULL, rectSlider.left, rectSlider.top, rectSlider.Width(), rectSlider.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
}

void CDockablePane::OffsetRectForSliding(CRect& rect, BOOL bSlideDirection, BOOL bIsRTL)
{
	if (!IsAutoHideMode())
	{
		return;
	}

	switch (GetCurrentAlignment())
	{
	case CBRS_ALIGN_LEFT:
		if (bIsRTL)
		{
			bSlideDirection ? rect.OffsetRect(-m_nSlideDelta, 0) : rect.OffsetRect(m_nSlideDelta, 0);
		}
		else
		{
			bSlideDirection ? rect.OffsetRect(m_nSlideDelta, 0) : rect.OffsetRect(-m_nSlideDelta, 0);
		}
		break;

	case CBRS_ALIGN_RIGHT:
		if (bIsRTL)
		{
			bSlideDirection  ? rect.OffsetRect(m_nSlideDelta, 0) : rect.OffsetRect(-m_nSlideDelta, 0);
		}
		else
		{
			bSlideDirection  ? rect.OffsetRect(-m_nSlideDelta, 0) : rect.OffsetRect(m_nSlideDelta, 0);
		}
		break;

	case CBRS_ALIGN_TOP:
		bSlideDirection ? rect.OffsetRect(0, m_nSlideDelta) : rect.OffsetRect(0, -m_nSlideDelta);
		break;

	case CBRS_ALIGN_BOTTOM:
		bSlideDirection ? rect.OffsetRect(0, -m_nSlideDelta) : rect.OffsetRect(0, m_nSlideDelta);
		break;
	}
}

void CDockablePane::CalcRectForSliding(CRect& rect, CRect& rectSlider, BOOL bSlideDirection)
{
	if (!IsAutoHideMode())
	{
		return;
	}

	switch (GetCurrentAlignment())
	{
	case CBRS_ALIGN_LEFT:
		if (bSlideDirection)
		{
			rect.right += m_nSlideDelta;
			if (rect.Width() > m_rectRestored.Width())
			{
				rect.right = rect.left + m_rectRestored.Width();
			}
		}
		else
		{
			rect.right -= m_nSlideDelta;
			if (rect.right < rect.left)
			{
				rect.right = rect.left;
			}
		}
		{
			int nSliderWidth = rectSlider.Width();
			rectSlider.left = rect.right;
			rectSlider.right = rectSlider.left + nSliderWidth;
		}
		break;

	case CBRS_ALIGN_RIGHT:
		if (bSlideDirection)
		{
			rect.left -= m_nSlideDelta;
			if (rect.Width() > m_rectRestored.Width())
			{
				rect.left = rect.right - m_rectRestored.Width();
			}
		}
		else
		{
			rect.left += m_nSlideDelta;
			if (rect.left > rect.right)
			{
				rect.left = rect.right;
			}
		}
		{
			int nSliderWidth = rectSlider.Width();
			rectSlider.right = rect.left;
			rectSlider.left = rectSlider.right - nSliderWidth;
		}
		break;

	case CBRS_ALIGN_TOP:
		if (bSlideDirection)
		{
			rect.bottom += m_nSlideDelta;
			if (rect.Height() > m_rectRestored.Height())
			{
				rect.bottom = rect.top + m_rectRestored.Height();
			}
		}
		else
		{
			rect.bottom -= m_nSlideDelta;
			if (rect.bottom < rect.top)
			{
				rect.bottom = rect.top;
			}
		}
		{
			int nSliderHeight = rectSlider.Height();
			rectSlider.top = rect.bottom;
			rectSlider.bottom = rectSlider.top + nSliderHeight;
		}
		break;

	case CBRS_ALIGN_BOTTOM:
		if (bSlideDirection)
		{
			rect.top -= m_nSlideDelta;
			if (rect.Height() > m_rectRestored.Height())
			{
				rect.top = rect.bottom - m_rectRestored.Height();
			}
		}
		else
		{
			rect.top += m_nSlideDelta;
			if (rect.top > rect.bottom)
			{
				rect.top = rect.bottom;
			}
		}
		{
			int nSliderHeight = rectSlider.Height();
			rectSlider.bottom = rect.top;
			rectSlider.top = rectSlider.bottom - nSliderHeight;
		}
		break;
	}
}

CPoint CDockablePane::CalcCorrectOffset(CRect rect, BOOL bIsRTL)
{
	CRect rectAutoHideDockBar;
	m_pAutoHideBar->GetParentDockSite()->GetWindowRect(rectAutoHideDockBar);
	GetDockSiteFrameWnd()->ScreenToClient(rectAutoHideDockBar);

	switch (GetCurrentAlignment())
	{
	case CBRS_ALIGN_LEFT:
		if (bIsRTL)
		{
			if (rect.right < rectAutoHideDockBar.left)
			{
				return CPoint(rectAutoHideDockBar.left - rect.right, 0);
			}
		}
		else
		{
			if (rect.left > rectAutoHideDockBar.right)
			{
				return CPoint(rectAutoHideDockBar.right - rect.left, 0);
			}
		}
		break;

	case CBRS_ALIGN_RIGHT:
		if (bIsRTL)
		{
			if (rect.left > rectAutoHideDockBar.right)
			{
				return CPoint(rectAutoHideDockBar.right - rect.left, 0);
			}
		}
		else
		{
			if (rect.right < rectAutoHideDockBar.left)
			{
				return CPoint(rectAutoHideDockBar.left - rect.right, 0);
			}
		}
		break;

	case CBRS_ALIGN_TOP:
		if (rect.top > rectAutoHideDockBar.bottom)
		{
			return CPoint(0, rectAutoHideDockBar.bottom - rect.top);
		}
		break;

	case CBRS_ALIGN_BOTTOM:
		if (rect.bottom < rectAutoHideDockBar.top)
		{
			return CPoint(0, rectAutoHideDockBar.top - rect.bottom);
		}
		break;
	}

	return CPoint(0, 0);
}

void CDockablePane::Slide(BOOL bSlideOut, BOOL bUseTimer)
{
	ASSERT_VALID(this);

	if (!IsAutoHideMode())
	{
		return;
	}

	if (m_nSlideTimer != 0)
	{
		KillTimer(m_nSlideTimer);
	}

	if (m_nAutoHideConditionTimerID != 0)
	{
		KillTimer(m_nAutoHideConditionTimerID);
		m_nAutoHideConditionTimerID = 0;
	}

	CRect rectWnd;
	GetWindowRect(rectWnd);

	if (!bUseTimer || m_bDisableAnimation || afxGlobalData.bIsRemoteSession)
	{
		m_nSlideDelta = IsHorizontal() ? rectWnd.Height() : rectWnd.Width();
	}

	if (!bUseTimer)
	{
		m_rectRestored = rectWnd;
		// just move out from the screen

		OnSlide(FALSE);
		ShowWindow(SW_HIDE);
		::ShowWindow(m_hDefaultSlider, SW_HIDE);
		return;
	}

	CDockingManager* pDockManager = afxGlobalUtils.GetDockingManager(GetDockSiteFrameWnd());
	ASSERT_VALID(pDockManager);

	if (bSlideOut)
	{
		pDockManager->HideAutoHidePanes(this);
		pDockManager->AlignAutoHidePane(GetDefaultPaneDivider(), FALSE);
		ShowWindow(SW_SHOW);
		::ShowWindow(m_hDefaultSlider, SW_SHOW);
	}

	BringWindowToTop();
	::BringWindowToTop(m_hDefaultSlider);

	if (m_ahSlideMode == AFX_AHSM_MOVE)
	{
		pDockManager->BringBarsToTop();
	}

	m_nSlideTimer = SetTimer(bSlideOut ? AFX_AUTO_HIDE_SLIDE_OUT_EVENT : AFX_AUTO_HIDE_SLIDE_IN_EVENT, m_nSlideDefaultTimeOut, NULL);

	if (!m_bDisableAnimation && !afxGlobalData.bIsRemoteSession)
	{
		if (m_ahSlideMode == AFX_AHSM_MOVE)
		{
			GetDockSiteFrameWnd()->ScreenToClient(rectWnd);
			m_nSlideDelta = max(1, ((GetCurrentAlignment() & CBRS_ORIENT_HORZ) ? rectWnd.Height() : rectWnd.Width()) / m_nSlideSteps);

		}
		else if (m_ahSlideMode == AFX_AHSM_STRETCH)
		{
			if (!bSlideOut && !m_bIsSliding)
			{
				m_rectRestored = rectWnd;
				GetDockSiteFrameWnd()->ScreenToClient(m_rectRestored);
			}
			m_nSlideDelta = max(1, ((GetCurrentAlignment() & CBRS_ORIENT_HORZ) ? m_rectRestored.Height() : m_rectRestored.Width()) / m_nSlideSteps);
		}
	}

	m_nSlideStep = 0;
	m_bIsSliding = TRUE;
}

void CDockablePane::SetAutoHideParents(CMFCAutoHideBar* pToolBar, CMFCAutoHideButton* pBtn)
{
	ASSERT_VALID(pToolBar);
	ASSERT_VALID(pBtn);

	m_pAutoHideBar = pToolBar;
	m_pAutoHideButton = pBtn;
}

void CDockablePane::SetResizeMode(BOOL bResize)
{
	m_bIsResizing = bResize;
}

CPaneDivider* __stdcall CDockablePane::CreateDefaultPaneDivider(DWORD dwAlignment, CWnd* pParent, CRuntimeClass* pSliderRTC)
{
	CRect rectSlider(0, 0, CPaneDivider::GetDefaultWidth(), CPaneDivider::GetDefaultWidth());
	WORD dwSliderStyle = CPaneDivider::SS_HORZ;

	if (dwAlignment & CBRS_ALIGN_LEFT || dwAlignment & CBRS_ALIGN_RIGHT)
	{
		dwSliderStyle = CPaneDivider::SS_VERT;
	}

	// create a slider with a control bar container
	CPaneDivider* pSlider = NULL;
	if (pSliderRTC != NULL)
	{
		pSlider = DYNAMIC_DOWNCAST(CPaneDivider, pSliderRTC->CreateObject());
		ASSERT_VALID(pSlider);

		pSlider->SetDefaultMode(TRUE);
	}
	else
	{
		pSlider = DYNAMIC_DOWNCAST(CPaneDivider, CPaneDivider::m_pSliderRTC->CreateObject());
		ASSERT_VALID(pSlider);

		pSlider->Init(TRUE);
	}

	if (!pSlider->CreateEx(0, dwSliderStyle | WS_VISIBLE, rectSlider, pParent, (UINT) -1, NULL))
	{
		TRACE0("Can't create default slider while docking\n");
		delete pSlider;
		return NULL;
	}

	pSlider->SetPaneAlignment(dwAlignment);

	return pSlider;
}

void CDockablePane::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CPoint ptScreen = point;
	ClientToScreen(&ptScreen);

	CMFCCaptionButton* pBtn = FindButton(ptScreen);
	if (pBtn != NULL)
	{
		CWnd::OnLButtonDblClk(nFlags, point);
		return;
	}

	if (!IsAutoHideMode())
	{
		CDockablePane* pBarToDock = this;
		if (IsTabbed())
		{
			CMFCBaseTabCtrl* pTabWnd = DYNAMIC_DOWNCAST(CMFCBaseTabCtrl, GetParent());
			if (pTabWnd != NULL)
			{
				pBarToDock = DYNAMIC_DOWNCAST(CDockablePane, pTabWnd->GetParent());
			}
		}

		CMultiPaneFrameWnd* pParentMiniFrame = DYNAMIC_DOWNCAST(CMultiPaneFrameWnd, GetParentMiniFrame());

		if (pParentMiniFrame != NULL)
		{
			OnProcessDblClk();
			pParentMiniFrame->DockRecentPaneToMainFrame(pBarToDock);
		}
		else if (IsWindow(m_hDefaultSlider))
		{
			// currently docked at main frame
			CMultiPaneFrameWnd* pRecentMiniFrame = DYNAMIC_DOWNCAST(CMultiPaneFrameWnd, CWnd::FromHandlePermanent(m_recentDockInfo.m_hRecentMiniFrame));

			if (pRecentMiniFrame != NULL &&
				(m_recentDockInfo.GetRecentPaneContainer(FALSE) != NULL || m_recentDockInfo.GetRecentTabContainer(FALSE) != NULL))
			{
				OnBeforeFloat(m_recentDockInfo.m_rectRecentFloatingRect, DM_DBL_CLICK);

				OnProcessDblClk();
				UndockPane();

				HWND hwndThis = GetSafeHwnd();
				BOOL bCanFocus = CanFocus();

				pRecentMiniFrame->AddRecentPane(pBarToDock);

				if (IsWindow(hwndThis))
				{
					OnAfterFloat();
				}

				if (bCanFocus)
				{
					pRecentMiniFrame->SetFocus();
				}
			}
			else
			{
				CPane::OnLButtonDblClk(nFlags, point);
			}
		}
		else
		{
			OnProcessDblClk();
		}
	}
	else
	{
		CWnd::OnLButtonDblClk(nFlags, point);
	}
}

BOOL CDockablePane::OnBeforeFloat(CRect& rectFloat, AFX_DOCK_METHOD dockMethod)
{
	ASSERT_VALID(this);
	BOOL bResult = CPane::OnBeforeFloat(rectFloat, dockMethod);

	if (dockMethod == DM_MOUSE)
	{
		// prevent drawing of the drag rectangle on mouse up
		m_bPrepareToFloat = false;
	}

	return bResult;
}

void CDockablePane::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	ASSERT_VALID(this);

	if (!IsDocked())
	{
		CPane::OnNcLButtonDown(nHitTest, point);
	}
}

void CDockablePane::OnClose()
{
	ASSERT_VALID(this);
	DestroyWindow();
}

CDockablePane* CDockablePane::AttachToTabWnd(CDockablePane* pTabControlBarAttachTo,
	AFX_DOCK_METHOD dockMethod, BOOL bSetActive, CDockablePane** ppTabbedControlBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pTabControlBarAttachTo);

	if (ppTabbedControlBar != NULL)
	{
		*ppTabbedControlBar = NULL;
	}

	if (!pTabControlBarAttachTo->CanBeAttached() || !CanBeAttached())
	{
		return NULL; // invalid attempt to attach non-attachable control bar
	}

	// check whether pTabBar is derived from CTabbedPane. If so, we
	// can attach this bar to it immediately. Otherwise, we need to create a
	// new tabbed control bar and replace pTabControlBarAttachTo with it.
	CBaseTabbedPane* pTabbedBarAttachTo = DYNAMIC_DOWNCAST(CBaseTabbedPane, pTabControlBarAttachTo);

	BOOL bBarAttachToIsFloating = (pTabControlBarAttachTo->GetParentMiniFrame() != NULL);

	CWnd* pOldParent = GetParent();
	CRect rectWndTab; rectWndTab.SetRectEmpty();
	if (pTabbedBarAttachTo == NULL)
	{
		CWnd* pTabParent = pTabControlBarAttachTo->GetParent();
		if (DYNAMIC_DOWNCAST(CMFCBaseTabCtrl, pTabParent) != NULL)
		{
			pTabParent = pTabParent->GetParent();
		}

		pTabbedBarAttachTo = DYNAMIC_DOWNCAST(CBaseTabbedPane, pTabParent);

		if (pTabbedBarAttachTo == NULL)
		{
			pTabControlBarAttachTo->StoreRecentDockSiteInfo();

			pTabControlBarAttachTo->GetWindowRect(rectWndTab);
			pTabControlBarAttachTo->GetParent()->ScreenToClient(&rectWndTab);

			pTabbedBarAttachTo = pTabControlBarAttachTo->CreateTabbedPane();
			ASSERT_VALID(pTabbedBarAttachTo);

			pTabControlBarAttachTo->InsertPane(pTabbedBarAttachTo, pTabControlBarAttachTo);

			if (!pTabControlBarAttachTo->ReplacePane(pTabbedBarAttachTo, dockMethod))
			{
				if (!bBarAttachToIsFloating)
				{
					RemovePaneFromDockManager(pTabbedBarAttachTo);
				}
				ASSERT(FALSE);
				TRACE0("Failed to replace resizable control bar by tabbed control bar. \n");
				delete pTabbedBarAttachTo;
				return NULL;
			}

			pTabbedBarAttachTo->EnableDocking(pTabControlBarAttachTo->GetEnabledAlignment());
			pTabbedBarAttachTo->SetPaneAlignment(pTabControlBarAttachTo->GetCurrentAlignment());

			pTabControlBarAttachTo->UndockPane(TRUE);
			pTabbedBarAttachTo->AddTab(pTabControlBarAttachTo, TRUE, bSetActive);
			pTabControlBarAttachTo->EnableGripper(FALSE);
		}
	}

	if (ppTabbedControlBar != NULL)
	{
		*ppTabbedControlBar = pTabbedBarAttachTo;
	}

	EnableGripper(FALSE);

	// send before dock notification without guarantee that the bar will
	// be attached to another dock bar
	OnBeforeDock((CBasePane**)&pTabbedBarAttachTo, NULL, dockMethod);
	// reassign the parentship to the tab bar
	OnBeforeChangeParent(pTabbedBarAttachTo, TRUE);

	// remove from miniframe
	RemoveFromMiniframe(pTabbedBarAttachTo, dockMethod);

	// AddTab returns TRUE only if this pointer is not tabbed control bar
	//(tabbed control bar is destroyed by AddTab and its tab windows are copied
	// to pTabbedBarAttachTo tabbed window)
	BOOL bResult = pTabbedBarAttachTo->AddTab(this, TRUE, bSetActive);
	if (bResult)
	{
		OnAfterChangeParent(pOldParent);
		OnAfterDock(pTabbedBarAttachTo, NULL, dockMethod);
	}

	if (!rectWndTab.IsRectEmpty())
	{
		pTabbedBarAttachTo->SetWindowPos(NULL, rectWndTab.left, rectWndTab.top, rectWndTab.Width(), rectWndTab.Height(), SWP_NOZORDER | SWP_NOACTIVATE);

		if (bResult)
		{
			AdjustDockingLayout();
		}
	}

	pTabbedBarAttachTo->RecalcLayout();

	return bResult ? this : pTabbedBarAttachTo;
}

BOOL CDockablePane::ReplacePane(CDockablePane* pBarToReplaceWith, AFX_DOCK_METHOD /*dockMethod*/, BOOL bRegisterWithFrame)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBarToReplaceWith);

	CPaneFrameWnd* pParentMiniFrame = GetParentMiniFrame();

	if (pParentMiniFrame != NULL)
	{
		// this is tabbed control bar that should be replaced by docking control bar
		// within miniframe

		ASSERT_VALID(pParentMiniFrame);
		pParentMiniFrame->ReplacePane(this, pBarToReplaceWith);
		return TRUE;
	}
	else if (m_hDefaultSlider != NULL)
	{
		CPaneDivider* pDefaultSlider = GetDefaultPaneDivider();

		if (pDefaultSlider != NULL && pDefaultSlider->ReplacePane(this, pBarToReplaceWith))
		{
			// unregister from parent frame/dock manager the bar that is being replaced(this)
			if (bRegisterWithFrame)
			{
				RemovePaneFromDockManager(this, FALSE, FALSE, FALSE, pBarToReplaceWith);
			}
			else
			{
				RemovePaneFromDockManager(this, FALSE);
			}

			return TRUE;
		}
	}
	return FALSE;
}

CTabbedPane* CDockablePane::CreateTabbedPane()
{
	ASSERT_VALID(this);
	CRect rectTabBar;
	GetWindowRect(&rectTabBar);
	ASSERT_VALID(GetParent());
	GetParent()->ScreenToClient(&rectTabBar);

	CTabbedPane* pTabbedBar = (CTabbedPane*) m_pTabbedControlBarRTC->CreateObject();
	ASSERT_VALID(pTabbedBar);

	pTabbedBar->SetAutoDestroy(TRUE);

	if (!pTabbedBar->Create(_T(""), GetParent(), rectTabBar, TRUE, (UINT) -1, GetStyle() | CBRS_FLOAT_MULTI))
	{
		TRACE0("Failed to create tabbed control bar\n");
		return NULL;
	}

	// override recent floating/docking info

	pTabbedBar->m_recentDockInfo.m_recentMiniFrameInfo.m_rectDockedRect = m_recentDockInfo.m_recentMiniFrameInfo.m_rectDockedRect;
	pTabbedBar->m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect = m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect;
	pTabbedBar->m_recentDockInfo.m_rectRecentFloatingRect = m_recentDockInfo.m_rectRecentFloatingRect;

	return pTabbedBar;
}

BOOL CDockablePane::Dock(CBasePane* pTargetBar, LPCRECT lpRect, AFX_DOCK_METHOD dockMethod)
{
	CFrameWnd* pParentFrame = DYNAMIC_DOWNCAST(CFrameWnd, AFXGetParentFrame(this));
	ASSERT_VALID(pParentFrame);

	if (pTargetBar != NULL && !pTargetBar->CanAcceptPane(this) && pTargetBar != this)
	{
		return FALSE;
	}

	if (dockMethod == DM_RECT && lpRect == NULL)
	{
		TRACE0("Docking control bar must be docked by rect or by mouse!");
		ASSERT(FALSE);
		return FALSE;
	}

	m_bPrepareToFloat = false;

	if (dockMethod == DM_DBL_CLICK || dockMethod == DM_SHOW)
	{
		CPaneContainer* pRecentTabContainer = m_recentDockInfo.GetRecentTabContainer(TRUE);

		ShowWindow(SW_HIDE);

		RemoveFromMiniframe(AFXGetParentFrame(this), dockMethod);
		SetPaneAlignment(m_recentDockInfo.m_dwRecentAlignmentToFrame);

		CPaneDivider* pRecentDefaultSlider = m_recentDockInfo.GetRecentDefaultPaneDivider();
		if (pRecentDefaultSlider != NULL)
		{
			SetDefaultPaneDivider(pRecentDefaultSlider->m_hWnd);
		}

		if (pRecentTabContainer != NULL)
		{
			BOOL bRecentLeftBar = m_recentDockInfo.IsRecentLeftPane(TRUE);
			CDockablePane* pTabbedBar = (CDockablePane*)(bRecentLeftBar ? pRecentTabContainer->GetLeftPane() : pRecentTabContainer->GetRightPane());
			if (pTabbedBar != NULL)
			{
				BOOL bResult = (AttachToTabWnd(pTabbedBar, DM_DBL_CLICK) != NULL);
				if (bResult)
				{
					ShowPane(TRUE, FALSE, TRUE);
				}
				AdjustDockingLayout();
				return bResult;
			}
		}

		if (pRecentDefaultSlider != NULL)
		{
			EnableGripper(TRUE);
			/*
			SetWindowPos(NULL, m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect.left, m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect.top, m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect.Width(), m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect.Height(), SWP_NOZORDER | SWP_NOREDRAW);*/

			AdjustPaneToPaneContainer(pRecentDefaultSlider);

			InsertPane(this, pRecentDefaultSlider, FALSE);

			ShowWindow(SW_SHOW);

			CDockablePane* pAddedControlBar = pRecentDefaultSlider->AddRecentPane(this);
			if (pAddedControlBar == this)
			{
				AdjustDockingLayout();
				return TRUE;
			}
			else if (pAddedControlBar != NULL)
			{
				pAddedControlBar->AdjustDockingLayout();
				return FALSE;
			}
		}
		else
		{
			ShowWindow(SW_SHOW);
			return DockToFrameWindow(m_recentDockInfo.m_dwRecentAlignmentToFrame, (lpRect == NULL) ? &m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect : lpRect);
		}
		return FALSE;
	}

	CPoint ptMouse(0, 0);
	if (dockMethod == DM_MOUSE || dockMethod == DM_STANDARD)
	{
		GetCursorPos(&ptMouse);
	}

	DWORD dwAlignment = 0;
	if (pTargetBar == NULL)
	{
		// insert the resizable bar as first resizable if it crosses the outer edge
		// IsPointNearDockSite will return this information
		BOOL bOuterEdge = FALSE;

		if (dockMethod == DM_MOUSE  || dockMethod == DM_STANDARD)
		{
			CPoint ptMouseCur;
			GetCursorPos(&ptMouseCur);
			if (!IsPointNearDockSite(ptMouseCur, dwAlignment, bOuterEdge))
			{
				return FALSE;
			}
			return DockToFrameWindow(dwAlignment, NULL, DT_DOCK_LAST, NULL, -1, bOuterEdge);
		}
		else if (lpRect != NULL)
		{
		}
	}
	else
	{
		ASSERT_VALID(pTargetBar);

		if (dockMethod == DM_MOUSE || dockMethod == DM_STANDARD)
		{
			if (!afxGlobalUtils.CheckAlignment(ptMouse, pTargetBar, CDockingManager::m_nDockSensitivity, NULL, FALSE, dwAlignment))
			{
				return FALSE;
			}

			return DockToWindow((CDockablePane*) pTargetBar, dwAlignment, NULL);
		}
		else if (lpRect != NULL)
		{
			return DockToWindow((CDockablePane*) pTargetBar, 0, lpRect);
		}
	}

	return FALSE;
}

BOOL CDockablePane::DockToFrameWindow(DWORD dwAlignment, LPCRECT lpRect, DWORD /*dwDockFlags*/, CBasePane* /*pRelativeBar*/, int /*nRelativeIndex*/, BOOL bOuterEdge)
{
	ASSERT_VALID(this);
	ASSERT(dwAlignment & CBRS_ALIGN_ANY);

	LockWindowUpdate();

	RemoveFromMiniframe(AFXGetParentFrame(this), DM_UNKNOWN);

	if (m_hDefaultSlider != NULL && IsWindow(m_hDefaultSlider))
	{
		UndockPane(FALSE);
	}

	CPaneDivider* pDefaultSlider = NULL;
	// create a slider with a control bar container
	if ((pDefaultSlider = CreateDefaultPaneDivider(dwAlignment, GetDockSiteFrameWnd())) == NULL)
	{
		TRACE0("Failde to create default slider");
		ShowWindow(SW_SHOW);
		return FALSE;
	}

	m_hDefaultSlider = pDefaultSlider->m_hWnd;

	CRect rectBar;
	GetWindowRect(rectBar);

	CDockingManager* pDockManager = afxGlobalUtils.GetDockingManager(GetDockSiteFrameWnd());
	ASSERT_VALID(pDockManager);

	CSize minSize;
	GetMinSize(minSize);
	BOOL bSetMinSize = FALSE;
	if (rectBar.Width() < minSize.cx)
	{
		rectBar.right = rectBar.left + minSize.cx;
		bSetMinSize = TRUE;
	}
	if (rectBar.Height() < minSize.cy)
	{
		rectBar.bottom = rectBar.top + minSize.cy;
		bSetMinSize = TRUE;
	}

	if (pDockManager->AdjustRectToClientArea(rectBar, dwAlignment) || bSetMinSize)
	{
		SetWindowPos(NULL, 0, 0, rectBar.Width(), rectBar.Height(), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	}

	pDefaultSlider->AddPane(this);

	// register this docking bar and slider with the frame's window dock manager
	if (!bOuterEdge)
	{
		AddPane(this);
		AddPane(pDefaultSlider);
	}
	else
	{
		pDockManager->AddPane(pDefaultSlider, !bOuterEdge, FALSE, bOuterEdge);
		pDockManager->AddPane(this, !bOuterEdge, FALSE, bOuterEdge);
	}

	SetPaneAlignment(dwAlignment);
	pDefaultSlider->SetPaneAlignment(GetCurrentAlignment());
	m_recentDockInfo.m_dwRecentAlignmentToFrame = GetCurrentAlignment();

	EnableGripper(TRUE);

	if (lpRect != NULL)
	{
		CRect rect(lpRect);
		SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}

	UnlockWindowUpdate();
	AdjustDockingLayout();
	OnAfterDock(this, NULL, DM_UNKNOWN);
	return TRUE;
}

BOOL CDockablePane::DockToWindow(CDockablePane* pTargetWindow, DWORD dwAlignment, LPCRECT lpRect)
{

	ASSERT_VALID(this);
	ASSERT_VALID(pTargetWindow);
	ASSERT(dwAlignment & CBRS_ALIGN_ANY || lpRect != NULL);
	ASSERT_KINDOF(CDockablePane, pTargetWindow);

	CPaneDivider* pSlider = pTargetWindow->GetDefaultPaneDivider();

	if (pSlider == NULL)
	{
		ShowWindow(SW_SHOW);
		return FALSE;
	}

	if (m_hDefaultSlider != NULL && IsWindow(m_hDefaultSlider))
	{
		UndockPane(FALSE);
	}

	RemoveFromMiniframe(AFXGetParentFrame(this), DM_UNKNOWN);

	if (pSlider->InsertPane(this, pTargetWindow, dwAlignment, lpRect))
	{
		// the bar was successfully inserted into slider's container. Now, we need
		// to register it with the frame
		InsertPane(this, pTargetWindow, TRUE);
		m_hDefaultSlider = pSlider->m_hWnd;

		EnableGripper(TRUE);
		// force NcCalcSize to recalculate and draw the caption(gripper)
		SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW | SWP_FRAMECHANGED | SWP_SHOWWINDOW);

		AdjustDockingLayout();
		OnAfterDock(this, NULL, DM_UNKNOWN);
		return TRUE;
	}

	return FALSE;
}

BOOL CDockablePane::DockPaneContainer(CPaneContainerManager& barContainerManager, DWORD dwAlignment, AFX_DOCK_METHOD /*dockMethod*/)
{
	if (m_hDefaultSlider != NULL && IsWindow(m_hDefaultSlider))
	{
		CObList lstControlBars;
		barContainerManager.AddPanesToList(&lstControlBars, NULL);

		for (POSITION pos = lstControlBars.GetHeadPosition(); pos != NULL;)
		{
			CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, lstControlBars.GetNext(pos));

			InsertPane(pBar, this, TRUE);
			pBar->SetDefaultPaneDivider(m_hDefaultSlider);
			pBar->SetPaneAlignment(GetCurrentAlignment());
		}

		CPaneDivider* pDefaultSlider = GetDefaultPaneDivider();
		if (pDefaultSlider != NULL)
		{
			return pDefaultSlider->AddPaneContainer(this, barContainerManager, dwAlignment);
		}
	}

	return FALSE;
}

void CDockablePane::DrawCaption(CDC* pDC, CRect rectCaption)
{
	ASSERT_VALID(pDC);

	CRect rcbtnRight = CRect(rectCaption.BottomRight(), CSize(0, 0));
	int i = 0;

	for (i = (int) m_arrButtons.GetUpperBound(); i >= 0; i --)
	{
		if (!m_arrButtons [i]->m_bLeftAlign && !m_arrButtons [i]->m_bHidden)
		{
			rcbtnRight = m_arrButtons [i]->GetRect();
			break;
		}
	}

	CRect rcbtnLeft = CRect(rectCaption.TopLeft(), CSize(0, 0));
	for (i = (int) m_arrButtons.GetUpperBound(); i >= 0; i --)
	{
		if (m_arrButtons [i]->m_bLeftAlign && !m_arrButtons [i]->m_bHidden)
		{
			rcbtnLeft = m_arrButtons [i]->GetRect();
			break;
		}
	}

	COLORREF clrCptnText = CMFCVisualManager::GetInstance()->OnDrawPaneCaption(pDC, this, m_bActive, rectCaption, rcbtnRight);

	for (i = 0; i < m_arrButtons.GetSize(); i ++)
	{
		CMFCCaptionButton* pbtn = m_arrButtons [i];
		ASSERT_VALID(pbtn);

		pbtn->m_clrForeground = clrCptnText;
	}

	int nOldBkMode = pDC->SetBkMode(TRANSPARENT);
	COLORREF clrOldText = pDC->SetTextColor(clrCptnText);

	CFont* pOldFont = pDC->SelectObject(&afxGlobalData.fontRegular);
	ENSURE(pOldFont != NULL);

	CString strTitle;
	GetWindowText(strTitle);

	rectCaption.right = rcbtnRight.left;
	rectCaption.left = rcbtnLeft.right;
	rectCaption.top++;
	rectCaption.DeflateRect(g_nCaptionHorzMargin * 2, 0);

	pDC->DrawText(strTitle, rectCaption, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

	pDC->SelectObject(pOldFont);
	pDC->SetBkMode(nOldBkMode);
	pDC->SetTextColor(clrOldText);
}

void CDockablePane::RedrawButton(const CMFCCaptionButton* pButton)
{
	if (pButton == NULL /*|| GetParentMiniFrame(TRUE) != NULL*/)
	{
		return;
	}

	if (!pButton->m_bEnabled)
	{
		return;
	}

	m_rectRedraw = pButton->GetRect();
	SendMessage(WM_NCPAINT);
	m_rectRedraw.SetRectEmpty();

	UpdateWindow();
}

void __stdcall CDockablePane::SetCaptionStyle(BOOL bDrawText, BOOL /*bForceGradient*/, BOOL bHideDisabledButtons)
{
	m_bCaptionText = bDrawText;
	m_bHideDisabledButtons = bHideDisabledButtons;
}

void CDockablePane::AdjustPaneToPaneContainer(CPaneDivider* pSlider)
{
	CRect rectContainer = pSlider->GetRootContainerRect();
	if (!rectContainer.IsRectEmpty())
	{
		CFrameWnd* pFrame = GetParentFrame();
		if (pFrame != NULL)
		{
			ASSERT_VALID(pFrame);
			pFrame->ScreenToClient(rectContainer);
			CRect rectWnd;
			GetWindowRect(rectWnd);
			pFrame->ScreenToClient(rectWnd);

			CRect rectUnion;
			rectUnion.UnionRect(rectWnd, rectContainer);

			if (rectUnion != rectContainer)
			{
				rectWnd.OffsetRect(rectContainer.left - rectWnd.left, rectContainer.top - rectWnd.top);
				if (rectWnd.Width() > rectContainer.Width())
				{
					rectWnd.right = rectWnd.left + rectContainer.Width();
				}
				if (rectWnd.Height() > rectContainer.Height())
				{
					rectWnd.bottom = rectWnd.top + rectContainer.Height();
				}

				SetWindowPos(NULL, rectWnd.left, rectWnd.top, rectContainer.Width(), rectContainer.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
			}
		}
	}
}

void CDockablePane::ShowPane(BOOL bShow, BOOL bDelay, BOOL bActivate)
{
	if (IsAutoHideMode())
	{
		if (IsHideInAutoHideMode())
		{
			if (IsPaneVisible() && !bShow)
			{
				m_pAutoHideButton->ShowAttachedWindow(FALSE);
			}
			m_pAutoHideBar->ShowAutoHideWindow(this, bShow, bDelay);
		}
		else
		{
			m_pAutoHideButton->ShowAttachedWindow(TRUE);
			if (bShow && bActivate)
			{
				SetFocus();
				m_bActive = TRUE;
			}
		}
	}
	else if (IsFloating() || IsTabbed())
	{
		// standard procedure - show/hide bar and its miniframe
		CPane::ShowPane(bShow, bDelay, bActivate);
		CPaneFrameWnd* pMiniFrame = GetParentMiniFrame();
		if (pMiniFrame != NULL)
		{
			pMiniFrame->OnShowPane(this, bShow);
		}
		if (IsTabbed() && bDelay)
		{
			GetParentTabbedPane()->RecalcLayout();
		}
	}
	else if (IsMDITabbed())
	{
		CWnd* pParent = GetParent();
		if (bShow)
		{
			ConvertToTabbedDocument();
			ShowWindow(SW_SHOW);
		}
		else
		{
			pParent->SendMessage(WM_CLOSE);
		}
	}
	else
	{
		CPaneDivider* pDefaultSlider = GetDefaultPaneDivider();
		ShowWindow(bShow ? SW_SHOW : SW_HIDE);
		if (bShow && pDefaultSlider != NULL)
		{
			// adjust rect to fit the container, otherwise it will break the size
			// of container;
			AdjustPaneToPaneContainer(pDefaultSlider);
		}

		CPaneFrameWnd* pMiniFrame = GetParentMiniFrame();

		if (pMiniFrame != NULL)
		{
			pMiniFrame->OnShowPane(this, bShow);
		}
		else if (pDefaultSlider != NULL)
		{
			if (bShow)
			{
				int nLastPercent = GetLastPercentInPaneContainer();
				if (nLastPercent >= 50)
				{
					SetLastPercentInPaneContainer(50);
				}
				else
				{
					SetLastPercentInPaneContainer(nLastPercent + 1);
				}
			}

			// docked at main frame - notify to adjust container
			pDefaultSlider->OnShowPane(this, bShow);
			if (!bDelay)
			{
				AdjustDockingLayout();
			}
		}
		else
		{
			// floating with other bars on miniframe  - notify to adjust container
		}
	}

	if (IsTabbed() && bShow && bActivate)
	{
		CMFCBaseTabCtrl* pParentTab = DYNAMIC_DOWNCAST(CMFCBaseTabCtrl, GetParent());
		if (pParentTab == NULL)
		{
			ASSERT(FALSE);
			return;
		}

		ASSERT_VALID(pParentTab);
		pParentTab->SetActiveTab(pParentTab->GetTabFromHwnd(GetSafeHwnd()));
	}
}

void CDockablePane::UndockPane(BOOL bDelay)
{
	CPaneFrameWnd* pMiniFrame = DYNAMIC_DOWNCAST(CPaneFrameWnd, GetParentMiniFrame());

	if (pMiniFrame == NULL)
	{
		RemoveFromDefaultPaneDividier();
		// remove from dock site
		RemovePaneFromDockManager(this, FALSE, !bDelay);

		if (!bDelay && !IsFloating())
		{
			AdjustDockingLayout();
		}
	}
	else
	{
		pMiniFrame->RemovePane(this);
	}
}

void CDockablePane::OnDestroy()
{
	RemoveCaptionButtons();

	if (GetParentMiniFrame() != NULL)
	{
		RemoveFromMiniframe(NULL, DM_UNKNOWN);
	}
	else
	{
		UndockPane(TRUE);
	}

	if (IsMDITabbed())
	{
		CDockingManager* pDockManager = afxGlobalUtils.GetDockingManager(GetDockSiteFrameWnd());
		pDockManager->RemoveHiddenMDITabbedBar(this);

		CMDIChildWndEx* pWnd = DYNAMIC_DOWNCAST(CMDIChildWndEx, GetParent());
		if (pWnd != NULL)
		{
			pWnd->PostMessage(WM_CLOSE);
		}
	}

	CTooltipManager::DeleteToolTip(m_pToolTip);

	CPane::OnDestroy();
}

void CDockablePane::OnTrackCaptionButtons(CPoint point)
{
	if (CMFCPopupMenu::GetActiveMenu() != NULL)
	{
		return;
	}

	UINT nHot = m_nHot;

	CMFCCaptionButton* pBtn = FindButton(point);
	if (pBtn != NULL)
	{
		m_nHot = pBtn->GetHit();

		if (m_nHit == HTNOWHERE || m_nHit == m_nHot)
		{
			pBtn->m_bFocused = TRUE;
		}
	}
	else
	{
		m_nHot = HTNOWHERE;
	}

	if (m_nHot != nHot)
	{
		RedrawButton(pBtn);

		CMFCCaptionButton* pBtnOld = FindButtonByHit(nHot);
		if (pBtnOld != NULL)
		{
			pBtnOld->m_bFocused = FALSE;
			RedrawButton(pBtnOld);
		}
	}

	if (m_nHit == HTNOWHERE)
	{
		if (nHot != HTNOWHERE && m_nHot == HTNOWHERE)
		{
			::ReleaseCapture();
			m_bCaptionButtonsCaptured = FALSE;
		}
		else if (nHot == HTNOWHERE && m_nHot != HTNOWHERE)
		{
			SetCapture();
			m_bCaptionButtonsCaptured = TRUE;
		}
	}
}

void CDockablePane::OnNcMouseMove(UINT nHitTest, CPoint point)
{
	if (!m_bPrepareToFloat /*&& GetParentMiniFrame(TRUE) == NULL*/)
	{
		OnTrackCaptionButtons(point);
	}

	CPane::OnNcMouseMove(nHitTest, point);
}

void CDockablePane::StopCaptionButtonsTracking()
{
	if (m_nHit != HTNOWHERE)
	{
		CMFCCaptionButton* pBtn = FindButtonByHit(m_nHit);
		m_nHit = HTNOWHERE;

		ReleaseCapture();
		if (pBtn != NULL)
		{
			pBtn->m_bPushed = FALSE;
			RedrawButton(pBtn);
		}
	}

	if (m_nHot != HTNOWHERE)
	{
		CMFCCaptionButton* pBtn = FindButtonByHit(m_nHot);
		m_nHot = HTNOWHERE;

		ReleaseCapture();
		if (pBtn != NULL)
		{
			pBtn->m_bFocused = FALSE;
			RedrawButton(pBtn);
		}
	}
	m_bCaptionButtonsCaptured = FALSE;
}

void CDockablePane::OnCancelMode()
{
	StopCaptionButtonsTracking();
	if (m_bPrepareToFloat)
	{
		m_bPrepareToFloat = false;
	}
	CPane::OnCancelMode();
}

CMFCCaptionButton* CDockablePane::FindButton(CPoint point) const
{
	ASSERT_VALID(this);

	CRect rcBar;
	GetWindowRect(rcBar);
	ScreenToClient(rcBar);

	for (int i = 0; i < m_arrButtons.GetSize(); i ++)
	{
		CMFCCaptionButton* pbtn = m_arrButtons [i];
		ASSERT_VALID(pbtn);

		CRect rectBtn = pbtn->GetRect();
		rectBtn.OffsetRect(rcBar.TopLeft());

		ClientToScreen(rectBtn);

		if (rectBtn.PtInRect(point))
		{
			return pbtn;
		}
	}

	return NULL;
}

CMFCCaptionButton* CDockablePane::FindButtonByHit(UINT nHit) const
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arrButtons.GetSize(); i ++)
	{
		CMFCCaptionButton* pbtn = m_arrButtons [i];
		ASSERT_VALID(pbtn);

		if (pbtn->GetHit() == nHit)
		{
			return pbtn;
		}
	}

	return NULL;
}

void CDockablePane::EnableButton(UINT nHit, BOOL bEnable/* = TRUE*/)
{
	ASSERT_VALID(this);

	CMFCCaptionButton* pButton = FindButtonByHit(nHit);
	if (pButton == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	pButton->m_bEnabled = bEnable;
}

BOOL CDockablePane::IsButtonEnabled(UINT nHit) const
{
	ASSERT_VALID(this);

	CMFCCaptionButton* pButton = FindButtonByHit(nHit);
	if (pButton == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return pButton->m_bEnabled;
}

void CDockablePane::OnUpdateCmdUI(class CFrameWnd *pTarget, int bDisableIfNoHndler)
{
	UpdateDialogControls(pTarget, bDisableIfNoHndler);

	CWnd* pFocus = GetFocus();
	BOOL bActiveOld = m_bActive;

	m_bActive = (pFocus->GetSafeHwnd() != NULL && (IsChild(pFocus) || pFocus->GetSafeHwnd() == GetSafeHwnd()));

	if (m_bActive != bActiveOld)
	{
		SendMessage(WM_NCPAINT);
	}
}

BOOL CDockablePane::IsVisible() const
{
	if (IsAutoHideMode())
	{
		if (!IsHideInAutoHideMode())
		{
			return FALSE;
		}
		return m_pAutoHideBar->IsVisible();
	}
	return CPane::IsVisible();
}

BOOL CDockablePane::PreTranslateMessage(MSG* pMsg)
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

	if (pMsg->message == WM_KEYDOWN &&(GetDockingMode() & DT_STANDARD) != 0 && m_bPrepareToFloat && pMsg->wParam == VK_ESCAPE)
	{
		if (m_bPrepareToFloat)
		{
			PostMessage(WM_CANCELMODE);
			return TRUE;
		}
		else if (IsFloating())
		{
			CPaneFrameWnd* pParentWnd = DYNAMIC_DOWNCAST(CPaneFrameWnd, GetParent());
			if (pParentWnd != NULL && GetCapture() == pParentWnd)
			{
				pParentWnd->PostMessage(WM_CANCELMODE);
				return TRUE;
			}
		}
	}

	if (pMsg->message == WM_KEYDOWN && IsTabbed() && pMsg->wParam == VK_ESCAPE)
	{
		CBaseTabbedPane* pParentBar = GetParentTabbedPane();
		CPaneFrameWnd* pParentMiniFrame = pParentBar->GetParentMiniFrame();
		if (pParentBar != NULL && (pParentBar->IsTracked() || pParentMiniFrame != NULL && pParentMiniFrame->IsCaptured()))

		{
			if (pParentMiniFrame != NULL)
			{
				pParentMiniFrame->PostMessage(WM_CANCELMODE);
			}
			else
			{
				pParentBar->PostMessage(WM_CANCELMODE);
			}
			return TRUE;
		}
	}

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE &&
		(GetDockingMode() & DT_SMART) != 0)
	{
		CDockingManager* pDockManager = afxGlobalUtils.GetDockingManager(GetParent());
		if (pDockManager != NULL)
		{
			CSmartDockingManager* pSDManager = pDockManager->GetSmartDockingManagerPermanent();
			if (pSDManager != NULL && pSDManager->IsStarted())
			{
				CPaneFrameWnd* pParentWnd = DYNAMIC_DOWNCAST(CPaneFrameWnd, GetParent());
				if (pParentWnd != NULL && GetCapture() == pParentWnd)
				{
					pParentWnd->PostMessage(WM_CANCELMODE);
					return TRUE;
				}
			}
		}
	}

	return CPane::PreTranslateMessage(pMsg);
}

void CDockablePane::SetDefaultPaneDivider(HWND hSliderWnd)
{
	if (m_hDefaultSlider != hSliderWnd)
	{
		CPaneDivider* pDefaultSlider = GetDefaultPaneDivider();
		if (pDefaultSlider != NULL)
		{
			pDefaultSlider->RemovePane(this);
		}
	}
	m_hDefaultSlider = hSliderWnd;
}

BOOL CDockablePane::LoadState(LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	return CPane::LoadState(lpszProfileName, nIndex, uiID);
}

BOOL CDockablePane::SaveState(LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	return CPane::SaveState(lpszProfileName, nIndex, uiID);
}

void CDockablePane::Serialize(CArchive& ar)
{
	CPane::Serialize(ar);
	if (ar.IsLoading())
	{
		ar >> m_recentDockInfo.m_rectRecentFloatingRect;
		ar >> m_rectSavedDockedRect;
		m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect = m_rectSavedDockedRect;
		ar >> m_bRecentFloatingState;
	}
	else
	{
		BOOL bFloating = IsFloating();

		if (bFloating)
		{
			CPaneFrameWnd* pMiniFrame = GetParentMiniFrame();
			if (pMiniFrame != NULL)
			{
				pMiniFrame->GetWindowRect(m_recentDockInfo.m_rectRecentFloatingRect);
			}
		}
		else
		{
			CalcRecentDockedRect();
		}

		ar << m_recentDockInfo.m_rectRecentFloatingRect;
		ar << m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect;
		ar << m_bRecentFloatingState;
	}
}

void CDockablePane::GetRecentSiblingPaneInfo(CList<UINT, UINT&>& /*lstBarIDs*/)
{
}

LRESULT CDockablePane::OnSetText(WPARAM, LPARAM lParam)
{
	LRESULT lRes = Default();

	if (!lRes)
	{
		return lRes;
	}

	CPaneFrameWnd* pParentMiniFrame = NULL;

	if (IsTabbed())
	{
		// If we are docked on a tabbed control bar, we have to update the tab label
		CMFCBaseTabCtrl* pParentTabWnd = DYNAMIC_DOWNCAST(CMFCBaseTabCtrl, GetParent());

		ASSERT_VALID(pParentTabWnd);

		CWnd* pWndTabbedControlBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, pParentTabWnd->GetParent());

		if (pWndTabbedControlBar != NULL)
		{
			LPCTSTR lpcszTitle = reinterpret_cast<LPCTSTR>(lParam);
			int iTab = pParentTabWnd->GetTabFromHwnd(GetSafeHwnd());
			CString strLabel;
			if (iTab >= 0 && iTab < pParentTabWnd->GetTabsNum())
			{
				VERIFY(pParentTabWnd->GetTabLabel(iTab, strLabel));
				if (strLabel != lpcszTitle)
				{
					VERIFY(pParentTabWnd->SetTabLabel(iTab, lpcszTitle));
				}
			}
		}
	}
	else if ((pParentMiniFrame = GetParentMiniFrame()) != NULL)
	{
		pParentMiniFrame->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
	else if (IsAutoHideMode())
	{
		ASSERT_VALID(m_pAutoHideBar);
		m_pAutoHideBar->RedrawWindow();
		SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
		AdjustDockingLayout();
	}
	else
	{
		SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}

	return lRes;
}

CPane* CDockablePane::DockPaneStandard(BOOL& bWasDocked)
{
	CBasePane* pTargetBar = NULL;
	int nSensitivity = ((GetDockingMode() & DT_SMART) != 0) ? -1 : CDockingManager::m_nDockSensitivity;

	AFX_CS_STATUS status = IsChangeState(nSensitivity, &pTargetBar);

	CDockablePane* pTargetDockingBar = DYNAMIC_DOWNCAST(CDockablePane, pTargetBar);

	if (pTargetDockingBar == this || GetAsyncKeyState(VK_CONTROL) < 0)
	{
		return NULL;
	}

	CMultiPaneFrameWnd* pTargetMiniFrame = pTargetDockingBar != NULL ? DYNAMIC_DOWNCAST(CMultiPaneFrameWnd, pTargetDockingBar->GetParentMiniFrame()) : NULL;

	if (status == CS_DELAY_DOCK) // status returned by resizable control bar
	{
		if (pTargetMiniFrame != NULL)
		{
			if ((GetPaneStyle() & CBRS_FLOAT_MULTI) == 0)
			{
				return NULL;
			}
			else if (pTargetBar != NULL)
			{
				bWasDocked = !pTargetMiniFrame->DockPane(this);
				return this;
			}
		}

		bWasDocked = DockPane(pTargetDockingBar, NULL, DM_STANDARD);
	}
	else if (status == CS_DELAY_DOCK_TO_TAB && pTargetDockingBar != NULL && pTargetDockingBar->CanAcceptPane(this) && CanBeAttached())
	{
		UndockPane(FALSE);
		CDockablePane* pBar = AttachToTabWnd(pTargetDockingBar, DM_STANDARD);
		bWasDocked = (pBar != NULL);
		return pBar;
	}

	return NULL;
}

CPaneDivider* CDockablePane::GetDefaultPaneDivider() const
{
	return DYNAMIC_DOWNCAST(CPaneDivider, CWnd::FromHandlePermanent(m_hDefaultSlider));
}

AFX_CS_STATUS CDockablePane::GetDockingStatus(CPoint pt, int nSensitivity)
{
	ASSERT_VALID(this);

	AFX_DOCK_TYPE docktype = GetDockingMode();

	CDockingManager* pDockManager = afxGlobalUtils.GetDockingManager(GetDockSiteFrameWnd());
	CSmartDockingStandaloneGuide::SDMarkerPlace nHilitedSideNo = CSmartDockingStandaloneGuide::sdNONE;

	if ((docktype & DT_SMART) != 0 && pDockManager != NULL)
	{
		CSmartDockingManager* pSDManager = pDockManager->GetSmartDockingManager();
		if (pSDManager != NULL && pSDManager->IsStarted())
		{
			nHilitedSideNo = pSDManager->GetHighlightedGuideNo();
		}
	}

	// detect caption
	UINT nHitTest = HitTest(pt, TRUE);

	CRect rectTabAreaTop;
	CRect rectTabAreaBottom;
	GetTabArea(rectTabAreaTop, rectTabAreaBottom);

	if (nHitTest == HTCAPTION || rectTabAreaTop.PtInRect(pt) || rectTabAreaBottom.PtInRect(pt) || nHilitedSideNo == CSmartDockingStandaloneGuide::sdCMIDDLE)
	{
		// need to display "ready to create detachable tab" status
		return CS_DELAY_DOCK_TO_TAB;
	}
	else
	{
		CRect rectBar;
		GetWindowRect(&rectBar);

		rectBar.top += GetCaptionHeight();
		rectBar.top += rectTabAreaTop.Height();
		rectBar.bottom -= rectTabAreaBottom.Height();

		if (nSensitivity == -1)
		{
			// is it demanded?
			if (rectBar.PtInRect(pt))
			{
				// mouse over an edge
				return CS_DELAY_DOCK;
			}
		}
		else
		{
			rectBar.DeflateRect(nSensitivity, nSensitivity);
			if (!rectBar.PtInRect(pt))
			{
				// mouse over an edge
				return CS_DELAY_DOCK;
			}
		}
	}

	return CS_NOTHING;
}

BOOL CDockablePane::CanAcceptMiniFrame(CPaneFrameWnd* pMiniFrame) const
{
	return pMiniFrame->CanBeDockedToPane(this);
}

BOOL CDockablePane::IsInFloatingMultiPaneFrameWnd() const
{
	CPaneFrameWnd* pMiniFrame = GetParentMiniFrame();
	if (pMiniFrame != NULL)
	{
		return pMiniFrame->IsKindOf(RUNTIME_CLASS(CMultiPaneFrameWnd));
	}
	return FALSE;
}

void CDockablePane::SetCaptionButtons()
{
	RemoveCaptionButtons();

	m_arrButtons.Add(new CMFCCaptionButton(AFX_HTCLOSE));
	m_arrButtons.Add(new CMFCCaptionButton(HTMAXBUTTON));
	m_arrButtons.Add(new CMFCCaptionButton(HTMINBUTTON));
}

void CDockablePane::RemoveCaptionButtons()
{
	for (int i = 0; i < m_arrButtons.GetSize(); i++)
	{
		delete m_arrButtons[i];
	}
	m_arrButtons.RemoveAll();
}

void CDockablePane::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CPane::OnSettingChange(uFlags, lpszSection);

	if (m_cyGripper > 0)
	{
		m_cyGripper = 0;
		EnableGripper(TRUE);
	}
}

BOOL CDockablePane::OnNeedTipText(UINT /*id*/, NMHDR* pNMH, LRESULT* /*pResult*/)
{
	static CString strTipText;

	ENSURE(pNMH != NULL);

	if (m_pToolTip->GetSafeHwnd() == NULL || pNMH->hwndFrom != m_pToolTip->GetSafeHwnd())
	{
		return FALSE;
	}

	LPNMTTDISPINFO pTTDispInfo = (LPNMTTDISPINFO) pNMH;
	ASSERT((pTTDispInfo->uFlags & TTF_IDISHWND) == 0);

	UINT nTooltipResID = 0;

	switch (pNMH->idFrom)
	{
	case 1:
		nTooltipResID = IDS_AFXBARRES_CLOSEBAR;
		break;

	case 2:
		{
			SHORT state = GetAsyncKeyState(VK_CONTROL);
			nTooltipResID = IDS_AFXBARRES_AUTOHIDEBAR;

			if ((state & 0x8000) && IsAutohideAllEnabled())
			{
				nTooltipResID = IDS_AFXBARRES_AUTOHIDE_ALL;
			}
		}
		break;

	case 3:
		nTooltipResID = IDS_AFXBARRES_MENU;
		break;
	}

	if (nTooltipResID == 0)
	{
		return FALSE;
	}

	ENSURE(strTipText.LoadString(nTooltipResID));

	pTTDispInfo->lpszText = const_cast<LPTSTR>((LPCTSTR) strTipText);
	return TRUE;
}

void CDockablePane::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (IsTracked())
	{
		return;
	}

	if (m_bCaptionButtonsCaptured)
	{
		StopCaptionButtonsTracking();
	}

	CPane::OnContextMenu(pWnd, point);
}

void CDockablePane::OnSetFocus(CWnd* pOldWnd)
{
	CPane::OnSetFocus(pOldWnd);

	CMultiPaneFrameWnd* pParentMiniFrame = DYNAMIC_DOWNCAST(CMultiPaneFrameWnd, GetParentMiniFrame());
	if (pParentMiniFrame != NULL)
	{
		pParentMiniFrame->SetLastFocusedPane(GetSafeHwnd());
	}
}

void CDockablePane::ToggleAutoHide()
{
	ASSERT_VALID(this);

	CPaneDivider* pDefaultSlider = GetDefaultPaneDivider();

	if (CanAutoHide() && pDefaultSlider != NULL)
	{
		SetAutoHideMode(!m_bPinState, pDefaultSlider->GetCurrentAlignment());
	}
}

BOOL CDockablePane::CanAutoHide() const
{
	ASSERT_VALID(this);

	if (!CPane::CanAutoHide())
	{
		return FALSE;
	}

	CWnd* pParentWnd = GetParent();
	if (pParentWnd == NULL)
	{
		return FALSE;
	}

	if (pParentWnd->IsKindOf(RUNTIME_CLASS(CPaneFrameWnd)))
	{
		pParentWnd = pParentWnd->GetParent();
	}

	if (pParentWnd == NULL)
	{
		return FALSE;
	}

	CPaneDivider* pDefaultSlider = GetDefaultPaneDivider();
	CDockingManager* pDockManager = afxGlobalUtils.GetDockingManager(pParentWnd);

	return pDockManager != NULL && pDefaultSlider != NULL && (pDefaultSlider->GetCurrentAlignment() & pDockManager->GetEnabledAutoHideAlignment());
}

void CDockablePane::CopyState(CDockablePane* pOrgBar)
{
	ASSERT_VALID(pOrgBar);
	CPane::CopyState(pOrgBar);

	m_rectRestored = pOrgBar->GetAHRestoredRect();
	m_ahSlideMode = pOrgBar->GetAHSlideMode();
	m_nLastPercent = pOrgBar->GetLastPercentInPaneContainer();
	m_bEnableAutoHideAll = pOrgBar->IsAutohideAllEnabled();

}

LRESULT CDockablePane::OnUpdateToolTips(WPARAM wp, LPARAM)
{
	UINT nTypes = (UINT) wp;

	if (nTypes & AFX_TOOLTIP_TYPE_DOCKBAR)
	{
		CTooltipManager::CreateToolTip(m_pToolTip, this, AFX_TOOLTIP_TYPE_DOCKBAR);

		for (int i = 0; i < AFX_CONTROLBAR_BUTTONS_NUM; i ++)
		{
			CRect rectDummy;
			rectDummy.SetRectEmpty();

			m_pToolTip->AddTool(this, LPSTR_TEXTCALLBACK, &rectDummy, i + 1);
		}

		UpdateTooltips();
	}

	return 0;
}

int CDockablePane::GetCaptionHeight() const
{
	if (IsFloating() || IsMDITabbed() || m_cyGripper == 0)
	{
		return 0;
	}

	return m_cyGripper + CMFCVisualManager::GetInstance()->GetDockingPaneCaptionExtraHeight();
}

void CDockablePane::ConvertToTabbedDocument(BOOL bActiveTabOnly)
{
	ASSERT_VALID(this);

	if (IsAutoHideMode())
	{
		return;
	}

	CMDIFrameWndEx* pMDIFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetDockSiteFrameWnd());
	if (pMDIFrame == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	ASSERT_VALID(pMDIFrame);
	if (IsTabbed())
	{
		CTabbedPane* pBar = DYNAMIC_DOWNCAST(CTabbedPane, GetParentTabbedPane());
		if (pBar != NULL)
		{
			pBar->ConvertToTabbedDocument(bActiveTabOnly);
		}
	}
	else
	{
		pMDIFrame->ControlBarToTabbedDocument(this);
	}
}



