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

#include "afxframewndex.h"
#include "afxpaneframewnd.h"
#include "afxmultipaneframewnd.h"
#include "afxpanedivider.h"

#include "afxframewndex.h"
#include "afxmdiframewndex.h"
#include "afxoleipframewndex.h"
#include "afxoledocipframewndex.h"
#include "afxmdichildwndex.h"
#include "afxolecntrframewndex.h"

#include "afxdocksite.h"
#include "afxdockingpanesrow.h"

#include "afxbasetabctrl.h"

#include "afxbasepane.h"
#include "afxbasetabbedpane.h"
#include "afxdockablepaneadapter.h"

#include "afxregpath.h"
#include "afxsettingsstore.h"

#include "afxglobalutils.h"
#include "afxdockingmanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const CString strBaseControlBarProfile = _T("BasePanes");
BOOL CBasePane::m_bSetTooltipTopmost = TRUE;

#define AFX_REG_SECTION_FMT    _T("%sBasePane-%d")
#define AFX_REG_SECTION_FMT_EX _T("%sBasePane-%d%x")

BOOL CBasePane::m_bMultiThreaded = FALSE;
CCriticalSection CBasePane::m_CriticalSection;

IMPLEMENT_DYNAMIC(CBasePane, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CBasePane

CBasePane::CBasePane()
{
	m_dwEnabledAlignment = 0;
	m_dwStyle = 0;
	m_pParentDockBar = NULL;
	m_pDockBarRow = NULL;
	m_pDockSite = NULL;

	m_bRecentVisibleState = FALSE;
	m_bIsRestoredFromRegistry = FALSE;

	m_dwControlBarStyle = 0;

	m_bVisible = FALSE;
	m_dockMode = DT_UNDEFINED;
	m_bEnableIDChecking = TRUE;

	m_lpszBarTemplateName = NULL;
	m_sizeDialog = CSize(0, 0);

	m_rectBar.SetRectEmpty();

	m_bIsDlgControl = FALSE;
	m_bIsMDITabbed = FALSE;

	EnableActiveAccessibility();

}

CBasePane::~CBasePane()
{
}

//{{AFX_MSG_MAP(CBasePane)
BEGIN_MESSAGE_MAP(CBasePane, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_SETTINGCHANGE()
	ON_MESSAGE(WM_IDLEUPDATECMDUI, &CBasePane::OnIdleUpdateCmdUI)
	ON_MESSAGE(WM_HELPHITTEST, &CBasePane::OnHelpHitTest)
	ON_MESSAGE(WM_INITDIALOG, &CBasePane::HandleInitDialog)
	ON_MESSAGE(WM_SETICON, &CBasePane::OnSetIcon)
	ON_MESSAGE(WM_GETOBJECT, &CBasePane::OnGetObject)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// CBasePane message handlers

BOOL CBasePane::CreateEx(DWORD dwStyleEx, LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
	DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwControlBarStyle, CCreateContext* pContext)
{
	ASSERT_VALID(pParentWnd);

	m_bIsDlgControl = pParentWnd->IsKindOf(RUNTIME_CLASS(CDialog));

	if (m_bEnableIDChecking)
	{
		CDockingManager* pDockManager = afxGlobalUtils.GetDockingManager(pParentWnd);
		if (pDockManager == NULL)
		{
			pDockManager = afxGlobalUtils.GetDockingManager(AFXGetParentFrame(pParentWnd));
			if (pDockManager != NULL)
			{
				if (pDockManager->FindPaneByID(nID, TRUE) != NULL)
				{
					TRACE0("Control bar must be created with unique ID!\n");
				}
			}
		}
	}

	m_bVisible = m_bVisible & WS_VISIBLE;

	SetPaneStyle(dwStyle | GetPaneStyle());
	m_dwControlBarStyle = dwControlBarStyle;

	BOOL bResult = FALSE;

	if (m_lpszBarTemplateName != NULL)
	{
		CREATESTRUCT cs;
		memset(&cs, 0, sizeof(cs));
		cs.lpszClass = lpszClassName;//AFX_WNDCONTROLBAR;
		cs.lpszName = lpszWindowName;
		cs.style = dwStyle | WS_CHILD;
		cs.hMenu = (HMENU)(UINT_PTR) nID;
		cs.hInstance = AfxGetInstanceHandle();
		cs.hwndParent = pParentWnd->GetSafeHwnd();

		if (!PreCreateWindow(cs))
		{
			return FALSE;
		}

		//--------------------------
		// create a modeless dialog
		//--------------------------
		if (!CreateDlg(m_lpszBarTemplateName, pParentWnd))
		{
			TRACE(_T("Can't create dialog: %s\n"), m_lpszBarTemplateName);
			return FALSE;
		}

#pragma warning (disable : 4311)
		SetClassLongPtr(m_hWnd, GCLP_HBRBACKGROUND, (long) ::GetSysColorBrush(COLOR_BTNFACE));
#pragma warning (default : 4311)

		SetDlgCtrlID(nID);

		CRect rectWindow;
		GetWindowRect(&rectWindow);

		m_sizeDialog = rectWindow.Size();
		bResult = TRUE;
	}
	else
	{
		bResult = CWnd::CreateEx(dwStyleEx, lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
	}

	if (bResult)
	{
		if (pParentWnd->IsKindOf(RUNTIME_CLASS(CFrameWnd)))
		{
			m_pDockSite = DYNAMIC_DOWNCAST(CFrameWnd, pParentWnd);
		}
		else
		{
			// case of miniframe or smth. else
			m_pDockSite = DYNAMIC_DOWNCAST(CFrameWnd, AFXGetParentFrame(pParentWnd));
		}

		m_bIsDlgControl = pParentWnd->IsKindOf(RUNTIME_CLASS(CDialog));
	}

	return bResult;
}

BOOL CBasePane::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CBasePane::DoPaint(CDC* pDC)
{
	CRect rectClip;
	pDC->GetClipBox(rectClip);

	CRect rect;
	GetClientRect(rect);

	CMFCVisualManager::GetInstance()->OnFillBarBackground(pDC, this, rect, rectClip);
}

BOOL CBasePane::IsDocked() const
{
	// return TRUE if its parent is not miniframe or the bar is floating
	// in the miniframe with another control bar
	CPaneFrameWnd* pParentMiniFrame = GetParentMiniFrame();
	if (pParentMiniFrame != NULL)
	{
		ASSERT_VALID(pParentMiniFrame);
		if (pParentMiniFrame->GetPaneCount() == 1)
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CBasePane::IsTabbed() const
{
	ASSERT_VALID(this);
	CWnd* pParent = GetParent();
	ASSERT_VALID(pParent);
	return pParent->IsKindOf(RUNTIME_CLASS(CMFCBaseTabCtrl));
}

BOOL CBasePane::IsMDITabbed() const
{
	return m_bIsMDITabbed;
}

BOOL CBasePane::IsVisible() const
{
	ASSERT_VALID(this);

	if (!IsTabbed())
	{
		if (CDockingManager::m_bRestoringDockState)
		{
			return GetRecentVisibleState();
		}

		return ((GetStyle() & WS_VISIBLE) != 0);
	}

	HWND hWndTab = NULL;
	CMFCBaseTabCtrl* pParent = GetParentTabWnd(hWndTab);

	ASSERT_VALID(pParent);

	if (!pParent->IsWindowVisible())
	{
		return FALSE;
	}

	int iTabNum = pParent->GetTabFromHwnd(hWndTab);

	if (iTabNum >= 0 && iTabNum < pParent->GetTabsNum())
	{
		return pParent->IsTabVisible(iTabNum);
	}

	return FALSE;
}

CPaneFrameWnd* CBasePane::GetParentMiniFrame(BOOL bNoAssert) const
{
	ASSERT_VALID(this);
	CPaneFrameWnd* pMiniFrame = NULL;
	CWnd* pParent = GetParent();

	while (pParent != NULL)
	{
		if (!bNoAssert)
		{
			ASSERT_VALID(pParent);
		}

		if (pParent != NULL && pParent->IsKindOf(RUNTIME_CLASS(CPaneFrameWnd)))
		{
			pMiniFrame = DYNAMIC_DOWNCAST(CPaneFrameWnd, pParent);
			break;
		}
		pParent = pParent->GetParent();
	}

	return pMiniFrame;
}

void CBasePane::OnPaint()
{
	if (m_bMultiThreaded)
	{
		m_CriticalSection.Lock();
	}

	CPaintDC dc(this);

	// erase background now
	if (GetStyle() & WS_VISIBLE)
		DoPaint(&dc);

	if (m_bMultiThreaded)
	{
		m_CriticalSection.Unlock();
	}
}

HDWP CBasePane::MoveWindow(CRect& rect, BOOL bRepaint, HDWP hdwp)
{
	CRect rectOld;
	GetWindowRect(rectOld);

	if (IsFloating())
	{
		CPaneFrameWnd* pMiniFrame = GetParentMiniFrame();
		ASSERT_VALID(pMiniFrame);
		pMiniFrame->ScreenToClient(rectOld);
	}
	else if(m_pDockSite != NULL)
	{
		m_pDockSite->ScreenToClient(rectOld);
	}

	if (rectOld == rect)
	{
		return hdwp;
	}

	if (hdwp != NULL)
	{
		UINT uFlags = SWP_NOZORDER | SWP_NOACTIVATE;
		return DeferWindowPos(hdwp, GetSafeHwnd(), NULL, rect.left, rect.top, rect.Width(),
			rect.Height(), uFlags);

	}
	CWnd::MoveWindow(&rect, bRepaint);
	return NULL;
}

HDWP CBasePane::SetWindowPos(const CWnd* pWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags, HDWP hdwp)
{
	if (hdwp == NULL)
	{
		CWnd::SetWindowPos(pWndInsertAfter, x, y, cx, cy, nFlags);
		return NULL;
	}

	HDWP hdwpNew = DeferWindowPos(hdwp, GetSafeHwnd(), NULL, x, y, cx, cy, nFlags);
	if (hdwpNew == NULL)
	{
		DWORD dwLastError = GetLastError();
		TRACE1("DeferWindowPos failded, error code %d\n", dwLastError);
		SetWindowPos(NULL, x, y, cx, cy, nFlags);
		return hdwp;
	}

	return hdwpNew;
}

// frame mapping functions
void CBasePane::AddPane(CBasePane* pBar)
{
	CWnd* pParentFrame = GetDockSiteFrameWnd();
	if (pParentFrame == NULL || afxGlobalUtils.m_bDialogApp)
	{
		return;
	}
	ASSERT_VALID(pParentFrame);

	if (pParentFrame->IsKindOf(RUNTIME_CLASS(CFrameWndEx)))
	{
		((CFrameWndEx*) pParentFrame)->AddPane(pBar);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		((CMDIFrameWndEx*) pParentFrame)->AddPane(pBar);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(COleIPFrameWndEx)))
	{
		((COleIPFrameWndEx*) pParentFrame)->AddPane(pBar);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(COleDocIPFrameWndEx)))
	{
		((COleDocIPFrameWndEx*) pParentFrame)->AddPane(pBar);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(CMDIChildWndEx)))
	{
		((CMDIChildWndEx*) pParentFrame)->AddPane(pBar);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(COleCntrFrameWndEx)))
	{
		((COleCntrFrameWndEx*) pParentFrame)->AddPane(pBar);
	}
	else
	{
		ASSERT(FALSE);
	}
}

void CBasePane::RemovePaneFromDockManager(CBasePane* pBar, BOOL bDestroy,BOOL bAdjustLayout, BOOL bAutoHide, CBasePane* pBarReplacement)
{
	CWnd* pParentFrame = GetDockSiteFrameWnd();
	if (pParentFrame == NULL || afxGlobalUtils.m_bDialogApp)
	{
		return;
	}

	ASSERT_VALID(pParentFrame);

	if (pParentFrame->IsKindOf(RUNTIME_CLASS(CFrameWndEx)))
	{
		((CFrameWndEx*) pParentFrame)->RemovePaneFromDockManager(pBar, bDestroy, bAdjustLayout, bAutoHide, pBarReplacement);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		((CMDIFrameWndEx*) pParentFrame)->RemovePaneFromDockManager(pBar, bDestroy, bAdjustLayout, bAutoHide, pBarReplacement);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(COleIPFrameWndEx)))
	{
		((COleIPFrameWndEx*) pParentFrame)->RemovePaneFromDockManager(pBar, bDestroy, bAdjustLayout, bAutoHide, pBarReplacement);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(COleDocIPFrameWndEx)))
	{
		((COleDocIPFrameWndEx*) pParentFrame)->RemovePaneFromDockManager(pBar, bDestroy, bAdjustLayout, bAutoHide, pBarReplacement);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(CMDIChildWndEx)))
	{
		((CMDIChildWndEx*) pParentFrame)->RemovePaneFromDockManager(pBar, bDestroy, bAdjustLayout, bAutoHide, pBarReplacement);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(COleCntrFrameWndEx)))
	{
		((COleCntrFrameWndEx*) pParentFrame)->RemovePaneFromDockManager(pBar, bDestroy, bAdjustLayout, bAutoHide, pBarReplacement);
	}
	else
	{
		ASSERT(FALSE);
	}
}

BOOL CBasePane::IsPointNearDockSite(CPoint point, DWORD& dwBarAlignment, BOOL& bOuterEdge) const
{
	CWnd* pParentFrame = GetDockSiteFrameWnd();

	if (pParentFrame == NULL || afxGlobalUtils.m_bDialogApp)
	{
		return TRUE;
	}

	ASSERT_VALID(pParentFrame);

	if (pParentFrame->IsKindOf(RUNTIME_CLASS(CFrameWndEx)))
	{
		return ((CFrameWndEx*) pParentFrame)->IsPointNearDockSite(point, dwBarAlignment, bOuterEdge);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		return ((CMDIFrameWndEx*) pParentFrame)->IsPointNearDockSite(point, dwBarAlignment, bOuterEdge);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(COleIPFrameWndEx)))
	{
		return ((COleIPFrameWndEx*) pParentFrame)->IsPointNearDockSite(point, dwBarAlignment, bOuterEdge);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(COleDocIPFrameWndEx)))
	{
		return ((COleDocIPFrameWndEx*) pParentFrame)->IsPointNearDockSite(point, dwBarAlignment, bOuterEdge);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(CMDIChildWndEx)))
	{
		return ((CMDIChildWndEx*) pParentFrame)->IsPointNearDockSite(point, dwBarAlignment, bOuterEdge);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(COleCntrFrameWndEx)))
	{
		return ((COleCntrFrameWndEx*) pParentFrame)->IsPointNearDockSite(point, dwBarAlignment, bOuterEdge);
	}
	else
	{
		ASSERT(FALSE);
	}
	return FALSE;
}

CBasePane* CBasePane::PaneFromPoint(CPoint point, int nSensitivity, bool bExactBar, CRuntimeClass* pRTCBarType) const
{
	CWnd* pParentFrame = GetDockSiteFrameWnd();

	if (pParentFrame == NULL || afxGlobalUtils.m_bDialogApp)
	{
		return NULL;
	}

	ASSERT_VALID(pParentFrame);

	if (pParentFrame->IsKindOf(RUNTIME_CLASS(CFrameWndEx)))
	{
		return ((CFrameWndEx*) pParentFrame)->PaneFromPoint(point, nSensitivity, bExactBar, pRTCBarType);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		return ((CMDIFrameWndEx*) pParentFrame)->PaneFromPoint(point, nSensitivity, bExactBar, pRTCBarType);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(COleIPFrameWndEx)))
	{
		return ((COleIPFrameWndEx*) pParentFrame)->PaneFromPoint(point, nSensitivity, bExactBar, pRTCBarType);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(COleDocIPFrameWndEx)))
	{
		return ((COleDocIPFrameWndEx*) pParentFrame)->PaneFromPoint(point, nSensitivity, bExactBar, pRTCBarType);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(CMDIChildWndEx)))
	{
		return ((CMDIChildWndEx*) pParentFrame)->PaneFromPoint(point, nSensitivity, bExactBar, pRTCBarType);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(COleCntrFrameWndEx)))
	{
		return ((COleCntrFrameWndEx*) pParentFrame)->PaneFromPoint(point, nSensitivity, bExactBar, pRTCBarType);
	}
	else
	{
		ASSERT(FALSE);
	}
	return FALSE;
}

BOOL CBasePane::InsertPane(CBasePane* pControlBar, CBasePane* pTarget, BOOL bAfter)
{
	CMultiPaneFrameWnd* pParentMiniFrame = DYNAMIC_DOWNCAST(CMultiPaneFrameWnd, GetParentMiniFrame());
	if (pParentMiniFrame != NULL)
	{
		return pParentMiniFrame->InsertPane(pControlBar, pTarget, bAfter);
	}

	CWnd* pParentFrame = GetDockSiteFrameWnd();

	if (pParentFrame == NULL || afxGlobalUtils.m_bDialogApp)
	{
		return TRUE;
	}

	ASSERT_VALID(pParentFrame);

	if (pParentFrame->IsKindOf(RUNTIME_CLASS(CFrameWndEx)))
	{
		return ((CFrameWndEx*) pParentFrame)->InsertPane(pControlBar, pTarget, bAfter);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		return ((CMDIFrameWndEx*) pParentFrame)->InsertPane(pControlBar, pTarget, bAfter);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(COleIPFrameWndEx)))
	{
		return ((COleIPFrameWndEx*) pParentFrame)->InsertPane(pControlBar, pTarget, bAfter);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(COleDocIPFrameWndEx)))
	{
		return ((COleDocIPFrameWndEx*) pParentFrame)->InsertPane(pControlBar, pTarget, bAfter);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(CMDIChildWndEx)))
	{
		return ((CMDIChildWndEx*) pParentFrame)->InsertPane(pControlBar, pTarget, bAfter);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(COleCntrFrameWndEx)))
	{
		return ((COleCntrFrameWndEx*) pParentFrame)->InsertPane(pControlBar, pTarget, bAfter);
	}
	else
	{
		ASSERT(FALSE);
	}
	return FALSE;
}

void CBasePane::AdjustDockingLayout(HDWP hdwp)
{
	CPaneFrameWnd* pParentMiniFrame = GetParentMiniFrame();

	if (pParentMiniFrame != NULL)
	{
		pParentMiniFrame->OnPaneRecalcLayout();
		return;
	}

	CWnd* pParentFrame = GetDockSiteFrameWnd();

	if (afxGlobalUtils.m_bDialogApp && pParentFrame == NULL)
	{
		return;
	}

	ASSERT_VALID(pParentFrame);

	if (pParentFrame->IsKindOf(RUNTIME_CLASS(CFrameWndEx)))
	{
		((CFrameWndEx*) pParentFrame)->AdjustDockingLayout(hdwp);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		((CMDIFrameWndEx*) pParentFrame)->AdjustDockingLayout(hdwp);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(COleIPFrameWndEx)))
	{
		((COleIPFrameWndEx*) pParentFrame)->AdjustDockingLayout(hdwp);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(COleDocIPFrameWndEx)))
	{
		((COleDocIPFrameWndEx*) pParentFrame)->AdjustDockingLayout(hdwp);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(CMDIChildWndEx)))
	{
		((CMDIChildWndEx*) pParentFrame)->AdjustDockingLayout(hdwp);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(COleCntrFrameWndEx)))
	{
		((COleCntrFrameWndEx*) pParentFrame)->AdjustDockingLayout(hdwp);
	}
	else
	{
		ASSERT(FALSE);
	}
}

void CBasePane::DockPaneUsingRTTI(BOOL bUseDocSite)
{
	CWnd* pParentFrame = bUseDocSite ? m_pDockSite : (CWnd*) AFXGetParentFrame(this);

	if (pParentFrame == NULL || afxGlobalUtils.m_bDialogApp)
	{
		return;
	}

	ASSERT_VALID(pParentFrame);

	if (pParentFrame->IsKindOf(RUNTIME_CLASS(CFrameWndEx)))
	{
		((CFrameWndEx*) pParentFrame)->DockPane(this);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		((CMDIFrameWndEx*) pParentFrame)->DockPane(this);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(COleIPFrameWndEx)))
	{
		((COleIPFrameWndEx*) pParentFrame)->DockPane(this);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(COleDocIPFrameWndEx)))
	{
		((COleDocIPFrameWndEx*) pParentFrame)->DockPane(this);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(CMDIChildWndEx)))
	{
		((CMDIChildWndEx*) pParentFrame)->DockPane(this);
	}
	else if (pParentFrame->IsKindOf(RUNTIME_CLASS(COleCntrFrameWndEx)))
	{
		((COleCntrFrameWndEx*) pParentFrame)->DockPane(this);
	}
	else
	{
		ASSERT(FALSE);
	}
}

void CBasePane::ShowPane(BOOL bShow, BOOL bDelay, BOOL bActivate)
{
	int nShowCmd = bShow ? SW_SHOWNOACTIVATE : SW_HIDE;

	if (IsFloating() && !IsTabbed())
	{
		ShowWindow(nShowCmd);

		CWnd* pParent = GetParent();
		ASSERT_VALID(pParent);

		pParent->ShowWindow(nShowCmd);
		pParent->PostMessage(AFX_WM_CHECKEMPTYMINIFRAME);
	}
	else if (m_pParentDockBar != NULL)
	{
		m_pParentDockBar->ShowPane(this, bShow, bDelay, bActivate);
	}
	else if (IsTabbed())
	{
		HWND hWndTab = NULL;
		CMFCBaseTabCtrl* pTabParent = GetParentTabWnd(hWndTab);
		ASSERT_VALID(pTabParent);

		CBaseTabbedPane* pTabbedControlBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, pTabParent->GetParent());

		if (pTabbedControlBar != NULL && !pTabbedControlBar->IsPaneVisible() && pTabbedControlBar->GetTabsNum() > 1 && bShow)
		{
			pTabbedControlBar->ShowTab(this, TRUE, bDelay, bActivate);
			return;
		}

		if (pTabbedControlBar != NULL)
		{
			ASSERT_VALID(pTabbedControlBar);
			pTabbedControlBar->ShowTab(this, bShow, bDelay, bActivate);

			if (pTabParent->GetVisibleTabsNum() == 0)
			{
				pTabbedControlBar->ShowPane(bShow, bDelay, bActivate);
			}
		}
		else
		{
			int iTab = pTabParent->GetTabFromHwnd(GetSafeHwnd());
			pTabParent->ShowTab(iTab, bShow, !bDelay);
		}
	}
	else
	{
		ShowWindow(nShowCmd);
		if (!bDelay)
		{
			AdjustDockingLayout();
		}
	}

	if (GetPaneRow() != NULL)
	{
		GetPaneRow()->FixupVirtualRects(false);
	}
}

LRESULT CBasePane::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM)
{
	// the style must be visible and if it is docked
	// the dockbar style must also be visible
	if ((GetStyle() & WS_VISIBLE) &&
		(m_pParentDockBar == NULL || (m_pParentDockBar->GetStyle() & WS_VISIBLE)))
	{
		CFrameWnd* pTarget = (CFrameWnd*)GetOwner();
		if (pTarget == NULL || !pTarget->IsFrameWnd())
			pTarget = AFXGetParentFrame(this);
		if (pTarget != NULL)
			OnUpdateCmdUI(pTarget, (BOOL)wParam);
	}

	return 0L;
}

void CBasePane::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	if (m_pDockBarRow != NULL)
	{
		m_pDockBarRow->OnResizePane(this);
	}
}

void CBasePane::Serialize(CArchive& ar)
{
	CWnd::Serialize(ar);

	if (ar.IsLoading())
	{
		DWORD dwAlign = 0;
		ar >> dwAlign;
		m_dwStyle |= dwAlign;

		ar >> m_bRecentVisibleState;
	}
	else
	{
		ar << (m_dwStyle & CBRS_ALIGN_ANY);
		ar << IsVisible();
	}
}

BOOL CBasePane::LoadState(LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CString strProfileName = ::AFXGetRegPath(strBaseControlBarProfile, lpszProfileName);

	if (nIndex == -1)
	{
		nIndex = GetDlgCtrlID();
	}

	CString strSection;
	if (uiID == (UINT) -1)
	{
		strSection.Format(AFX_REG_SECTION_FMT, (LPCTSTR)strProfileName, nIndex);
	}
	else
	{
		strSection.Format(AFX_REG_SECTION_FMT_EX, (LPCTSTR)strProfileName, nIndex, uiID);
	}

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (!reg.Open(strSection))
	{
		return FALSE;
	}

	reg.Read(_T("IsVisible"), m_bRecentVisibleState);
	m_bIsRestoredFromRegistry = TRUE;

	return TRUE;
}

BOOL CBasePane::SaveState(LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{

	CString strProfileName = ::AFXGetRegPath(strBaseControlBarProfile, lpszProfileName);

	if (nIndex == -1)
	{
		nIndex = GetDlgCtrlID();
	}

	CString strSection;
	if (uiID == (UINT) -1)
	{
		strSection.Format(AFX_REG_SECTION_FMT, (LPCTSTR)strProfileName, nIndex);
	}
	else
	{
		strSection.Format(AFX_REG_SECTION_FMT_EX, (LPCTSTR)strProfileName, nIndex, uiID);
	}

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);

	if (reg.CreateKey(strSection))
	{
		BOOL bIsVisible = IsVisible();
		reg.Write(_T("IsVisible"), bIsVisible);
	}

	return TRUE;
}

CWnd* CBasePane::GetDockSiteFrameWnd() const
{
	if (m_pDockSite == NULL && GetParent()->IsKindOf(RUNTIME_CLASS(CDialog)))
	{
		afxGlobalUtils.m_bDialogApp = TRUE;
	}

	return m_pDockSite;
}

AFX_DOCK_TYPE CBasePane::GetDockingMode() const
{
	if (m_dockMode != DT_UNDEFINED)
	{
		return m_dockMode;
	}

	return CDockingManager::GetDockingMode();
}

BOOL CBasePane::CanFloat() const
{
	if (!IsTabbed())
	{
		return m_dwControlBarStyle & AFX_CBRS_FLOAT;
	}

	HWND hWndTab = NULL;

	CMFCBaseTabCtrl* pParentTabWnd = GetParentTabWnd(hWndTab);

	if (pParentTabWnd == NULL)
	{
		return m_dwControlBarStyle & AFX_CBRS_FLOAT;
	}

	int nTabNum = pParentTabWnd->GetTabFromHwnd(hWndTab);
	if (nTabNum == -1)
	{
		return m_dwControlBarStyle & AFX_CBRS_FLOAT;
	}

	return pParentTabWnd->IsTabDetachable(nTabNum);
}

CMFCBaseTabCtrl* CBasePane::GetParentTabWnd(HWND& hWndTab) const
{
	ASSERT_VALID(this);

	const CWnd* pWndToCheck = this;

	CDockablePaneAdapter* pWrapper = DYNAMIC_DOWNCAST(CDockablePaneAdapter, GetParent());
	if (pWrapper != NULL)
	{
		pWndToCheck = pWrapper;
		hWndTab = pWrapper->GetSafeHwnd();
	}
	else
	{
		hWndTab = GetSafeHwnd();
	}

	CMFCBaseTabCtrl* pParentTabWnd = DYNAMIC_DOWNCAST(CMFCBaseTabCtrl, pWndToCheck->GetParent());
	if (pParentTabWnd == NULL)
	{
		CBaseTabbedPane* pParentTabBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, pWndToCheck->GetParent());
		if (pParentTabBar != NULL)
		{
			return pParentTabBar->GetUnderlyingWindow();
		}
	}

	return pParentTabWnd;
}

CBaseTabbedPane* CBasePane::GetParentTabbedPane() const
{
	HWND hWndTab = NULL;

	if (!IsTabbed())
	{
		return NULL;
	}

	CMFCBaseTabCtrl* pTabWnd = GetParentTabWnd(hWndTab);
	if (hWndTab == NULL || pTabWnd == NULL)
	{
		return NULL;
	}

	ASSERT_VALID(pTabWnd);

	return DYNAMIC_DOWNCAST(CBaseTabbedPane, pTabWnd->GetParent());
}

LRESULT CBasePane::OnHelpHitTest(WPARAM, LPARAM lParam)
{
	ASSERT_VALID(this);

	INT_PTR nID = OnToolHitTest((DWORD_PTR)lParam, NULL);
	if (nID != -1)
		return HID_BASE_COMMAND+nID;

	nID = _AfxGetDlgCtrlID(m_hWnd);
	return nID != 0 ? HID_BASE_CONTROL+nID : 0;
}

LRESULT CBasePane::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	ASSERT_VALID(this);

	LRESULT lResult;
	switch (message)
	{
	case WM_NOTIFY:
	case WM_COMMAND:
	case WM_DRAWITEM:
	case WM_MEASUREITEM:
	case WM_DELETEITEM:
	case WM_COMPAREITEM:
	case WM_VKEYTOITEM:
	case WM_CHARTOITEM:
		// send these messages to the owner if not handled
		if (OnWndMsg(message, wParam, lParam, &lResult))
			return lResult;
		else
		{
			// try owner next
			lResult = GetOwner()->SendMessage(message, wParam, lParam);

			// special case for TTN_NEEDTEXTA and TTN_NEEDTEXTW
			if (message == WM_NOTIFY)
			{
				NMHDR* pNMHDR = (NMHDR*)lParam;
				if (pNMHDR->code == TTN_NEEDTEXTA || pNMHDR->code == TTN_NEEDTEXTW)
				{
					TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
					TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;

					if (pNMHDR->code == TTN_NEEDTEXTA)
					{
						if (pTTTA->hinst == 0 && (!pTTTA->lpszText || !*pTTTA->lpszText))
						{
							// not handled by owner, so let bar itself handle it
							lResult = CWnd::WindowProc(message, wParam, lParam);
						}
					}
					else if (pNMHDR->code == TTN_NEEDTEXTW)
					{
						if (pTTTW->hinst == 0 && (!pTTTW->lpszText || !*pTTTW->lpszText))
						{
							// not handled by owner, so let bar itself handle it
							lResult = CWnd::WindowProc(message, wParam, lParam);
						}
					}
				}
			}
			return lResult;
		}
	}

	// otherwise, just handle in default way
	lResult = CWnd::WindowProc(message, wParam, lParam);
	return lResult;
}

BOOL CBasePane::PreTranslateMessage(MSG* pMsg)
{
	ASSERT_VALID(this);
	ENSURE(m_hWnd != NULL);

	if (CWnd::PreTranslateMessage(pMsg))
		return TRUE;

	CWnd* pOwner = GetOwner();

	// don't translate dialog messages when in Shift+F1 help mode
	CFrameWnd* pFrameWnd = GetTopLevelFrame();
	if (pFrameWnd != NULL && pFrameWnd->m_bHelpMode)
		return FALSE;

	// since 'IsDialogMessage' will eat frame window accelerators,
	//   we call all frame windows' PreTranslateMessage first
	while (pOwner != NULL)
	{
		// allow owner & frames to translate before IsDialogMessage does
		if (pOwner->PreTranslateMessage(pMsg))
			return TRUE;

		// try parent frames until there are no parent frames
		if (IsWindow(pOwner->GetSafeHwnd()))
		{
			pOwner = pOwner->GetParentFrame();
		}
		else
		{
			break;
		}
	}

	// filter both messages to dialog and from children
	return PreTranslateInput(pMsg);
}

LRESULT CBasePane::HandleInitDialog(WPARAM, LPARAM)
{
	if (m_lpszBarTemplateName != NULL)
	{
		if (!ExecuteDlgInit(m_lpszBarTemplateName))
		{
			return FALSE;
		}
	}

	if (!UpdateData(FALSE))
	{
		return FALSE;
	}

	return TRUE;
}

LRESULT CBasePane::OnSetIcon(WPARAM,LPARAM)
{
	LRESULT lres = Default();

	if (IsTabbed())
	{
		HWND hWndTab = NULL;
		CMFCBaseTabCtrl* pParentTab = GetParentTabWnd(hWndTab);

		ASSERT_VALID(pParentTab);

		int iTabNum = pParentTab->GetTabFromHwnd(hWndTab);

		if (iTabNum >= 0 && iTabNum < pParentTab->GetTabsNum())
		{
			pParentTab->SetTabHicon(iTabNum, GetIcon(FALSE));
		}
	}

	return lres;
}

LRESULT CBasePane::OnGetObject(WPARAM wParam, LPARAM lParam)
{
	if (afxGlobalData.IsAccessibilitySupport() && IsAccessibilityCompatible())
	{
		return CWnd::OnGetObject(wParam, lParam);
	}

	return (LRESULT)0L;
}

BOOL CBasePane::OnSetAccData(long /*lVal*/)
{
	return TRUE;
}

DWORD CBasePane::GetCurrentAlignment() const
{
	return m_dwStyle & CBRS_ALIGN_ANY;
}

void CBasePane::CopyState(CBasePane* pOrgBar)
{
	ASSERT_VALID(pOrgBar);

	m_dwEnabledAlignment = pOrgBar->GetEnabledAlignment();
	m_bRecentVisibleState = pOrgBar->GetRecentVisibleState();
	m_bIsRestoredFromRegistry = pOrgBar->IsRestoredFromRegistry();
	m_pDockSite = pOrgBar->GetDockSiteFrameWnd();
	m_rectBar = pOrgBar->GetPaneRect();
	m_bIsDlgControl = pOrgBar->IsDialogControl();
	m_dwStyle = pOrgBar->GetPaneStyle();
	m_dwControlBarStyle = pOrgBar->GetControlBarStyle();
}

void CBasePane::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CWnd::OnSettingChange(uFlags, lpszSection);
	afxGlobalData.OnSettingChange();
}

void CBasePane::OnPaneContextMenu(CWnd* pParentFrame, CPoint point)
{
	ASSERT_VALID(pParentFrame);

	if (pParentFrame->SendMessage(AFX_WM_TOOLBARMENU, (WPARAM) GetSafeHwnd(), MAKELPARAM(point.x, point.y)) == 0)
	{
		return;
	}

	CDockingManager* pDockManager = afxGlobalUtils.GetDockingManager(GetParentFrame());
	if (pDockManager == NULL)
	{
		return;
	}

	ASSERT_VALID(pDockManager);
	pDockManager->OnPaneContextMenu(point);
}

HRESULT CBasePane::get_accChildCount(long *pcountChildren)
{
	if (!pcountChildren)
	{
		return E_INVALIDARG;
	}

	*pcountChildren = 0;
	return S_OK;
}

HRESULT CBasePane::get_accParent(IDispatch **ppdispParent)
{
	HRESULT hr = E_INVALIDARG;

	if (ppdispParent)
	{
		CWnd* pWnd = GetParent();
		if (pWnd)
		{
			AccessibleObjectFromWindow(pWnd->GetSafeHwnd () , (DWORD) OBJID_CLIENT, IID_IAccessible, (void**) ppdispParent);

			hr  = (*ppdispParent) ? S_OK : S_FALSE;
		}
	}

	return hr;
}

HRESULT CBasePane::get_accChild(VARIANT varChild, IDispatch **ppdispChild)
{
	if (!(*ppdispChild))
	{
		return E_INVALIDARG;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		return E_INVALIDARG;
	}

	if (m_pStdObject != NULL)
	{
		*ppdispChild = m_pStdObject;
	}
	else
	{
		*ppdispChild = NULL;
	}

	return S_OK;
}

HRESULT CBasePane::get_accName(VARIANT varChild, BSTR *pszName)
{
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		CString strText;
		GetWindowText(strText);
		*pszName = strText.AllocSysString();
		return S_OK;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal > 0))
	{
		OnSetAccData(varChild.lVal);
		if (m_AccData.m_strAccName.IsEmpty())
		{
			return S_FALSE;
		}
		*pszName = m_AccData.m_strAccName.AllocSysString();
	}

	return S_OK;
}

HRESULT CBasePane::get_accDescription(VARIANT varChild, BSTR *pszDescription)
{
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		CString strText;
		GetWindowText(strText);
		*pszDescription = strText.AllocSysString();
		return S_OK;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal > 0))
	{
		OnSetAccData(varChild.lVal);
		if (m_AccData.m_strDescription.IsEmpty())
		{
			return S_FALSE;
		}
		*pszDescription = m_AccData.m_strDescription.AllocSysString();
		return S_OK;
	}

	return S_FALSE;
}

HRESULT CBasePane::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		pvarRole->vt = VT_I4;
		pvarRole->lVal = ROLE_SYSTEM_TOOLBAR;
		return S_OK;
	}

	if (!pvarRole || ((varChild.vt != VT_I4) && (varChild.lVal != CHILDID_SELF)))
	{
		return E_INVALIDARG;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal > 0))
	{
		pvarRole->vt = VT_I4;
		OnSetAccData(varChild.lVal);
		pvarRole->lVal = m_AccData.m_nAccRole;
		return S_OK;
	}

	pvarRole->vt = VT_I4;
	pvarRole->lVal = ROLE_SYSTEM_PUSHBUTTON;

	return S_OK;
}

HRESULT CBasePane::get_accState(VARIANT varChild, VARIANT *pvarState)
{
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		pvarState->vt = VT_I4;
		pvarState->lVal = STATE_SYSTEM_DEFAULT;
		return S_OK;
	}

	if (!pvarState || ((varChild.vt != VT_I4) && (varChild.lVal != CHILDID_SELF)))
	{
		return E_INVALIDARG;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal > 0))
	{
		OnSetAccData(varChild.lVal);
		pvarState->vt = VT_I4;
		pvarState->lVal = m_AccData.m_bAccState;
		return S_OK; 
	}

	return E_INVALIDARG;
}

HRESULT CBasePane::get_accHelp(VARIANT varChild, BSTR *pszHelp)
{
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		*pszHelp = SysAllocString(L"ControlPane");
		return S_OK;
	}

	if (((varChild.vt != VT_I4) && (varChild.lVal != CHILDID_SELF)) || (NULL == pszHelp))
	{
		return E_INVALIDARG;
	}

	OnSetAccData(varChild.lVal);
	if (m_AccData.m_strAccHelp.IsEmpty())
	{
		return S_FALSE;
	}

	*pszHelp = m_AccData.m_strAccHelp.AllocSysString();
	return S_OK;
}

HRESULT CBasePane::get_accFocus(VARIANT *pvarChild)
{
	if (NULL == pvarChild)
	{
		return E_INVALIDARG;
	}

	return DISP_E_MEMBERNOTFOUND; 
}

HRESULT CBasePane::get_accSelection(VARIANT *pvarChildren)
{
	if (NULL == pvarChildren)
	{
		return E_INVALIDARG;
	}

	return DISP_E_MEMBERNOTFOUND; 
}

HRESULT CBasePane::get_accHelpTopic(BSTR* /*pszHelpFile*/, VARIANT /*varChild*/, long* /*pidTopic*/)
{
	return S_FALSE;
}

HRESULT CBasePane::get_accKeyboardShortcut(VARIANT varChild, BSTR *pszKeyboardShortcut)
{
	if ((varChild.vt != VT_I4) && (varChild.lVal != CHILDID_SELF))
	{
		return E_INVALIDARG;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		*pszKeyboardShortcut = SysAllocString(L"");
		return S_OK;
	}

	if (!pszKeyboardShortcut || ((varChild.vt != VT_I4) && (varChild.lVal != CHILDID_SELF)))
	{
		return E_INVALIDARG;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal > 0))
	{
		OnSetAccData(varChild.lVal);
		if (m_AccData.m_strAccKeys.IsEmpty())
		{
			return S_FALSE;
		}

		*pszKeyboardShortcut = m_AccData.m_strAccKeys.AllocSysString();
		return S_OK;
	}

	return S_FALSE;
}

HRESULT CBasePane::get_accDefaultAction(VARIANT varChild, BSTR *pszDefaultAction)
{
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		return S_FALSE;
	}

	if ((varChild.vt != VT_I4) && (varChild.lVal != CHILDID_SELF))
	{
		return E_INVALIDARG;
	}

	OnSetAccData(varChild.lVal);

	if (m_AccData.m_strAccDefAction.IsEmpty())
	{
		return S_FALSE;
	}

	*pszDefaultAction = m_AccData.m_strAccDefAction.AllocSysString();
	return S_OK;
}

HRESULT CBasePane::accSelect(long flagsSelect, VARIANT varChild)
{
	if (m_pStdObject != NULL)
	{
		return m_pStdObject->accSelect(flagsSelect, varChild); 
	}

	return E_INVALIDARG;
}

HRESULT CBasePane::accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild)
{
	if( !pxLeft || !pyTop || !pcxWidth || !pcyHeight )
	{
		return E_INVALIDARG;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		CRect rc;
		GetWindowRect(rc);

		*pxLeft = rc.left;
		*pyTop = rc.top;
		*pcxWidth = rc.Width();
		*pcyHeight = rc.Height();

		return S_OK;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal > 0))
	{
		OnSetAccData(varChild.lVal);

		*pxLeft = m_AccData.m_rectAccLocation.left;
		*pyTop = m_AccData.m_rectAccLocation.top;
		*pcxWidth = m_AccData.m_rectAccLocation.Width();
		*pcyHeight = m_AccData.m_rectAccLocation.Height();
		return S_OK;
	}

	return S_OK;
}

HRESULT CBasePane::accHitTest(long xLeft, long yTop, VARIANT *pvarChild)
{
	if (!pvarChild)
	{
		return E_INVALIDARG;
	}

	OnSetAccData((LONG)MAKELPARAM((WORD)xLeft, (WORD)yTop));

	if (m_AccData.m_nAccHit != 0)
	{
		pvarChild->vt = VT_I4;
		LPARAM lParam = MAKELPARAM((WORD)xLeft, (WORD)yTop);
		pvarChild->lVal = (LONG)lParam;
	}
	else
	{
		pvarChild->vt = VT_I4;
		pvarChild->lVal = CHILDID_SELF;
	}

	return S_OK;
}

HRESULT CBasePane::get_accValue(VARIANT varChild, BSTR *pszValue)
{
	if ((varChild.vt == VT_I4) && (varChild.lVal > 0))
	{
		OnSetAccData(varChild.lVal);
		if (m_AccData.m_strAccValue.IsEmpty())
		{
			return S_FALSE;
		}
		*pszValue = m_AccData.m_strAccValue.AllocSysString();
	}
	else
	{
		return S_FALSE;
	}

	return S_OK;
}
