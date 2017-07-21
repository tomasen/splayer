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
#include "afxmdiframewndex.h"
#include "afxmdichildwndex.h"
#include "afxpaneframewnd.h"
#include "afxpreviewviewex.h"
#include "afxmdiclientareawnd.h"
#include "afxglobalutils.h"
#include "afxdockablepane.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BOOL CMDIChildWndEx::m_bEnableFloatingBars = FALSE;
DWORD CMDIChildWndEx::m_dwExcludeStyle = WS_CAPTION | WS_BORDER | WS_THICKFRAME;

static BOOL bInOnCreate = FALSE;

/////////////////////////////////////////////////////////////////////////////
// CMDIChildWndEx

IMPLEMENT_DYNCREATE(CMDIChildWndEx, CMDIChildWnd)

#pragma warning(disable : 4355)

CMDIChildWndEx::CMDIChildWndEx() : m_Impl(this)
{
	m_pMDIFrame = NULL;
	m_bToBeDestroyed = FALSE;
	m_bWasMaximized = FALSE;
	m_bIsMinimized = FALSE;
	m_rectOriginal.SetRectEmpty();
	m_bActivating = FALSE;

	// ---- MDITabGroup+
	m_pRelatedTabGroup = NULL;
	// ---- MDITabGroup-

	m_pTabbedControlBar = NULL;
}

#pragma warning(default : 4355)

CMDIChildWndEx::~CMDIChildWndEx()
{
}

//{{AFX_MSG_MAP(CMDIChildWndEx)
BEGIN_MESSAGE_MAP(CMDIChildWndEx, CMDIChildWnd)
	ON_WM_CREATE()
	ON_WM_MDIACTIVATE()
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_DESTROY()
	ON_WM_GETMINMAXINFO()
	ON_WM_NCPAINT()
	ON_WM_NCACTIVATE()
	ON_WM_NCMOUSEMOVE()
	ON_WM_NCHITTEST()
	ON_WM_NCCALCSIZE()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_ERASEBKGND()
	ON_WM_STYLECHANGED()
	ON_MESSAGE(WM_SETTEXT, &CMDIChildWndEx::OnSetText)
	ON_MESSAGE(WM_SETICON, &CMDIChildWndEx::OnSetIcon)
	ON_MESSAGE(WM_IDLEUPDATECMDUI, &CMDIChildWndEx::OnIdleUpdateCmdUI)
	ON_REGISTERED_MESSAGE(AFX_WM_CHANGEVISUALMANAGER, &CMDIChildWndEx::OnChangeVisualManager)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// CMDIChildWndEx message handlers

BOOL CMDIChildWndEx::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST && m_pMDIFrame != NULL && m_pMDIFrame->GetActivePopup() != NULL)
	{
		// Don't process accelerators if popup window is active
		return FALSE;
	}

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE && m_pRelatedTabGroup != NULL && GetCapture() == m_pRelatedTabGroup)
	{
		m_pRelatedTabGroup->PostMessage(WM_CANCELMODE);
		return CMDIChildWnd::PreTranslateMessage(pMsg);
	}

	return CMDIChildWnd::PreTranslateMessage(pMsg);
}

int CMDIChildWndEx::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	m_pMDIFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetMDIFrame());
	ASSERT_VALID(m_pMDIFrame);

	m_Impl.m_bHasBorder = (lpCreateStruct->style & WS_BORDER) != NULL;
	bInOnCreate = TRUE;

	if ((GetStyle() & WS_SYSMENU) == 0)
	{
		GetParent()->SetRedraw(FALSE);

		m_rectOriginal = CRect(CPoint(lpCreateStruct->x, lpCreateStruct->y), CSize(lpCreateStruct->cx, lpCreateStruct->cy));

		if (m_pMDIFrame != NULL && !m_pMDIFrame->IsMDITabbedGroup())
		{
			CRect rect;
			m_pMDIFrame->m_wndClientArea.GetClientRect(rect);

			CRect rectClient;
			GetClientRect(rectClient);
			ClientToScreen(rectClient);

			CRect rectScreen;
			GetWindowRect(rectScreen);

			rect.left -= rectClient.left - rectScreen.left;
			rect.top -= rectClient.top - rectScreen.top;
			rect.right += rectScreen.right - rectClient.right;
			rect.bottom += rectScreen.bottom - rectClient.bottom;

			SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
		}

		GetParent()->SetRedraw(TRUE);
	}

	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
	{
		bInOnCreate = FALSE;
		return -1;
	}

	if (m_pMDIFrame->IsPrintPreview())
	{
		m_pMDIFrame->SendMessage(WM_CLOSE);
	}

	CFrameImpl::AddFrame(this);
	PostMessage(AFX_WM_CHANGEVISUALMANAGER);

	bInOnCreate = FALSE;
	return 0;
}

BOOL CMDIChildWndEx::DockPaneLeftOf(CPane* pBar, CPane* pLeftOf)
{
	m_dockManager.DockPaneLeftOf(pBar, pLeftOf);
	return TRUE;
}

void CMDIChildWndEx::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	static BOOL bActivating = FALSE;

	m_dockManager.OnActivateFrame(bActivate);

	m_bActivating = bActivate;

	if (!bActivating)
	{
		bActivating = TRUE;

		CMDIChildWnd::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);

		if (bActivate && m_pMDIFrame != NULL)
		{
			ASSERT_VALID(m_pMDIFrame);
			m_pMDIFrame->m_wndClientArea.SetActiveTab(pActivateWnd->GetSafeHwnd());
		}

		bActivating = FALSE;

		if (bActivate && m_pMDIFrame != NULL)
		{
			CDockingManager* parentDockManager = m_pMDIFrame->GetDockingManager();
			ASSERT_VALID(parentDockManager);

			if (parentDockManager != NULL && parentDockManager->IsOLEContainerMode() || m_dockManager.IsOLEContainerMode())
			{
				//parentDockManager->AdjustDockingLayout();
				afxGlobalUtils.ForceAdjustLayout(parentDockManager, TRUE);
			}
		}
	}
}

void CMDIChildWndEx::ActivateFrame(int nCmdShow)
{
	CWnd* pWndParent = GetParent();
	ASSERT_VALID(pWndParent);

	if (CMDIFrameWndEx::m_bDisableSetRedraw || pWndParent->GetSafeHwnd() == NULL)
	{
		if ((GetStyle() & WS_SYSMENU) == 0)
		{
			nCmdShow = SW_SHOWMAXIMIZED;
		}

		if (m_pMDIFrame != 0 && m_pMDIFrame->IsMDITabbedGroup())
		{
			nCmdShow = SW_SHOWNORMAL;
		}

		CMDIChildWnd::ActivateFrame(nCmdShow);
		return;
	}

	pWndParent->SetRedraw(FALSE);

	CMDIChildWnd::ActivateFrame(nCmdShow);

	pWndParent->SetRedraw(TRUE);
	pWndParent->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);

}

LRESULT CMDIChildWndEx::OnSetText(WPARAM, LPARAM lParam)
{
	LRESULT lRes = Default();

	if (m_pMDIFrame != NULL)
	{
		ASSERT_VALID(m_pMDIFrame);
		m_pMDIFrame->m_wndClientArea.UpdateTabs(TRUE);
	}

	m_Impl.OnSetText((LPCTSTR)lParam);
	return lRes;
}

LRESULT CMDIChildWndEx::OnSetIcon(WPARAM,LPARAM)
{
	LRESULT lRes = Default();

	if (m_pMDIFrame != NULL)
	{
		ASSERT_VALID(m_pMDIFrame);
		m_pMDIFrame->m_wndClientArea.UpdateTabs();
	}

	return lRes;
}

CString CMDIChildWndEx::GetFrameText() const
{
	ASSERT_VALID(this);

	CString strText;
	GetWindowText(strText);

	return strText;
}

HICON CMDIChildWndEx::GetFrameIcon() const
{
	ASSERT_VALID(this);

	HICON hIcon = GetIcon(FALSE);
	if (hIcon == NULL)
	{
		hIcon = (HICON)(LONG_PTR) GetClassLongPtr(GetSafeHwnd(), GCLP_HICONSM);
	}

	return hIcon;
}

void CMDIChildWndEx::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	BOOL bRedraw = m_Impl.IsOwnerDrawCaption() && IsWindowVisible() &&(GetStyle() & WS_MAXIMIZE) == 0;

	CString strTitle1;

	if (bRedraw)
	{
		GetWindowText(strTitle1);
	}

	CMDIChildWnd::OnUpdateFrameTitle(bAddToTitle);

	if (bRedraw)
	{
		CString strTitle2;
		GetWindowText(strTitle2);

		if (strTitle1 != strTitle2)
		{
			SendMessage(WM_NCPAINT, 0, 0);
		}
	}

	if (m_pMDIFrame != NULL)
	{
		ASSERT_VALID(m_pMDIFrame);
		m_pMDIFrame->m_wndClientArea.UpdateTabs();
	}
}

void CMDIChildWndEx::OnSize(UINT nType, int cx, int cy)
{
	if (m_bToBeDestroyed)
	{
		// prevents main menu flickering when the last dockument is being closed
		return;
	}

	m_bIsMinimized = (nType == SIZE_MINIMIZED);

	if (CMFCVisualManager::GetInstance()->IsOwnerDrawCaption())
	{
		if (m_pMDIFrame != NULL && !m_pMDIFrame->IsMDITabbedGroup())
		{
			CRect rectWindow;
			GetWindowRect(rectWindow);

			WINDOWPOS wndpos;
			wndpos.flags = SWP_FRAMECHANGED;
			wndpos.x     = rectWindow.left;
			wndpos.y     = rectWindow.top;
			wndpos.cx    = rectWindow.Width();
			wndpos.cy    = rectWindow.Height();

			m_Impl.OnWindowPosChanging(&wndpos);
		}
		else if (m_Impl.m_bIsWindowRgn)
		{
			m_Impl.m_bIsWindowRgn = FALSE;
			SetWindowRgn(NULL, TRUE);
		}
	}

	if (!m_bIsMinimized && nType != SIZE_MAXIMIZED && !m_bWasMaximized)
	{
		m_dockManager.m_bSizeFrame = TRUE;
		CMDIChildWnd::OnSize(nType, cx, cy);
		AdjustDockingLayout();
		m_dockManager.m_bSizeFrame = FALSE;

		m_Impl.UpdateCaption();
		return;
	}

	CMDIChildWnd::OnSize(nType, cx, cy);

	if ((nType == SIZE_MAXIMIZED ||(nType == SIZE_RESTORED && m_bWasMaximized)))
	{
		RecalcLayout();

		if (m_pNotifyHook != NULL && nType == SIZE_RESTORED)
		{
			CMDIFrameWndEx* pTopLevelFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetParentFrame());
			if (pTopLevelFrame == NULL || !pTopLevelFrame->AreMDITabs())
			{
				ModifyStyle(0, WS_OVERLAPPEDWINDOW);
			}
		}
	}

	m_bWasMaximized = (nType == SIZE_MAXIMIZED);
	m_Impl.UpdateCaption();
}

BOOL CMDIChildWndEx::PreCreateWindow(CREATESTRUCT& cs)
{
	m_dockManager.Create(this);
	return CMDIChildWnd::PreCreateWindow(cs);
}

void CMDIChildWndEx::AddDockSite()
{
	ASSERT_VALID(this);
}

BOOL CMDIChildWndEx::AddPane(CBasePane* pControlBar, BOOL bTail)
{
	ASSERT_VALID(this);
	return m_dockManager.AddPane(pControlBar, bTail);
}

BOOL CMDIChildWndEx::InsertPane(CBasePane* pControlBar, CBasePane* pTarget, BOOL bAfter)
{
	ASSERT_VALID(this);
	return m_dockManager.InsertPane(pControlBar, pTarget, bAfter);
}

void CMDIChildWndEx::RemovePaneFromDockManager(CBasePane* pControlBar, BOOL bDestroy, BOOL bAdjustLayout, BOOL bAutoHide, CBasePane* pBarReplacement)
{
	ASSERT_VALID(this);
	m_dockManager.RemovePaneFromDockManager(pControlBar, bDestroy, bAdjustLayout, bAutoHide, pBarReplacement);
}

void CMDIChildWndEx::DockPane(CBasePane* pBar, UINT nDockBarID, LPCRECT /*lpRect*/)
{
	ASSERT_VALID(this);

	if (pBar->CanFloat() && !CMDIChildWndEx::m_bEnableFloatingBars)
	{
		// bar can't be floating
		pBar->m_dwControlBarStyle &= ~AFX_CBRS_FLOAT;
	}

	if (pBar->CanBeResized() || pBar->CanFloat())
	{
		pBar->EnableDocking(CBRS_ALIGN_ANY);
		m_dockManager.DockPane(pBar, nDockBarID);
	}
	else
	{
		AddPane(pBar, TRUE);
	}
}

void CMDIChildWndEx::OnSetPreviewMode(BOOL bPreview, CPrintPreviewState* pState)
{
	ASSERT_VALID(this);

	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, AfxGetMainWnd());
	if (pMainFrame != NULL)
	{
		pMainFrame->SetPrintPreviewFrame(bPreview ? this : NULL);
	}

	m_dockManager.SetPrintPreviewMode(bPreview, pState);
	DWORD dwSavedState = pState->dwStates;
	CMDIChildWnd::OnSetPreviewMode(bPreview, pState);
	pState->dwStates = dwSavedState;

	AdjustDockingLayout();
	RecalcLayout();
}

CBasePane* CMDIChildWndEx::GetPane(UINT nID)
{
	ASSERT_VALID(this);

	CBasePane* pBar = m_dockManager.FindPaneByID(nID, TRUE);
	return pBar;
}

void CMDIChildWndEx::ShowPane(CBasePane* pBar, BOOL bShow, BOOL bDelay, BOOL bActivate)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);

	pBar->ShowPane(bShow, bDelay, bActivate);
}

CBasePane* CMDIChildWndEx::PaneFromPoint(CPoint point, int nSensitivity, bool bExactBar, CRuntimeClass* pRTCBarType) const
{
	ASSERT_VALID(this);
	return m_dockManager.PaneFromPoint(point, nSensitivity, bExactBar, pRTCBarType);
}

CBasePane* CMDIChildWndEx::PaneFromPoint(CPoint point, int nSensitivity, DWORD& dwAlignment, CRuntimeClass* pRTCBarType) const
{
	ASSERT_VALID(this);
	return m_dockManager.PaneFromPoint(point, nSensitivity, dwAlignment, pRTCBarType);
}

BOOL CMDIChildWndEx::IsPointNearDockSite(CPoint point, DWORD& dwBarAlignment, BOOL& bOuterEdge) const
{
	ASSERT_VALID(this);
	return m_dockManager.IsPointNearDockSite(point, dwBarAlignment, bOuterEdge);
}

void CMDIChildWndEx::AdjustDockingLayout(HDWP hdwp)
{
	ASSERT_VALID(this);

	if (m_dockManager.IsInAdjustLayout())
	{
		return;
	}

	m_dockManager.AdjustDockingLayout(hdwp);
	AdjustClientArea();
}

void CMDIChildWndEx::AdjustClientArea()
{
	CWnd* pChildWnd = (m_pTabbedControlBar != NULL && m_pTabbedControlBar->IsMDITabbed() &&
		m_pTabbedControlBar->GetParent() == this) ? m_pTabbedControlBar : GetDlgItem(AFX_IDW_PANE_FIRST);
	if (pChildWnd != NULL)
	{
		if (!pChildWnd->IsKindOf(RUNTIME_CLASS(CSplitterWnd)))
		{
			pChildWnd->ModifyStyle(0, WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
		}
		else
		{
			pChildWnd->ModifyStyle(0, WS_CLIPSIBLINGS);
		}

		if (!bInOnCreate && !CDockingManager::m_bFullScreenMode)
		{
			CRect rectClientAreaBounds = m_dockManager.GetClientAreaBounds();

			rectClientAreaBounds.left += m_rectBorder.left;
			rectClientAreaBounds.top  += m_rectBorder.top;
			rectClientAreaBounds.right -= m_rectBorder.right;
			rectClientAreaBounds.bottom -= m_rectBorder.bottom;

			pChildWnd->SetWindowPos(&wndBottom, rectClientAreaBounds.left, rectClientAreaBounds.top,
				rectClientAreaBounds.Width(), rectClientAreaBounds.Height(), SWP_NOACTIVATE);
		}
	}
}

BOOL CMDIChildWndEx::OnMoveMiniFrame(CWnd* pFrame)
{
	ASSERT_VALID(this);
	return m_dockManager.OnMoveMiniFrame(pFrame);
}

BOOL CMDIChildWndEx::EnableDocking(DWORD dwDockStyle)
{
	return m_dockManager.EnableDocking(dwDockStyle);
}

BOOL CMDIChildWndEx::EnableAutoHidePanes(DWORD dwDockStyle)
{
	return m_dockManager.EnableAutoHidePanes(dwDockStyle);
}

void CMDIChildWndEx::RecalcLayout(BOOL bNotify)
{
	if (m_bInRecalcLayout)
		return;

	m_bInRecalcLayout = TRUE;

	if (!m_bIsMinimized)
	{
		CView* pView = GetActiveView();

		if (m_dockManager.IsPrintPreviewValid() || m_dockManager.IsOLEContainerMode())
		{
			if (pView != NULL && pView->IsKindOf(RUNTIME_CLASS(CPreviewViewEx)))
			{

				m_dockManager.RecalcLayout(bNotify);
				CRect rectClient = m_dockManager.GetClientAreaBounds();
				pView->SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOZORDER  | SWP_NOACTIVATE);
			}
			else
			{
				COleClientItem* pActiveItem = NULL;
				pView = GetActiveView();

				if (pView != NULL && GetParentFrame()->GetActiveFrame() == this && m_bActivating)
				{
					ASSERT_VALID(pView);

					COleDocument* pDoc = DYNAMIC_DOWNCAST(COleDocument, pView->GetDocument());
					if (pDoc != NULL)
					{
						ASSERT_VALID(pDoc);
						pActiveItem = pDoc->GetInPlaceActiveItem(pView);

						if (bNotify && pActiveItem != NULL && pActiveItem->m_pInPlaceFrame != NULL)
						{
							pActiveItem->m_pInPlaceFrame->OnRecalcLayout();
						}
					}
					CRect rectClient;

					CFrameWnd* pFrame = pView->GetParentFrame();
					if (pFrame != NULL)
					{
						pFrame->GetClientRect(rectClient);
						pView->SetWindowPos(NULL, 0, 0, rectClient.Width(), rectClient.Height(), SWP_NOZORDER  | SWP_NOACTIVATE);
					}
				}
				else
				{
					AdjustClientArea();
				}
			}
		}
		else
		{
			m_dockManager.RecalcLayout(bNotify);
			AdjustClientArea();
		}
	}

	m_bInRecalcLayout = FALSE;
}

void CMDIChildWndEx::OnSizing(UINT fwSide, LPRECT pRect)
{
	CMDIChildWnd::OnSizing(fwSide, pRect);

	CRect rect;
	GetWindowRect(rect);

	if (rect.Size() != CRect(pRect).Size())
	{
		AdjustDockingLayout();
	}

}

void CMDIChildWndEx::OnDestroy()
{
	if (m_pMDIFrame != NULL && m_pMDIFrame->IsPrintPreview())
	{
		m_pMDIFrame->SendMessage(WM_CLOSE);
	}

	if (m_pTabbedControlBar != NULL && CWnd::FromHandlePermanent(m_pTabbedControlBar->GetSafeHwnd()) != NULL)
	{
		CWnd* pParent = m_pTabbedControlBar->GetParent();

		if (pParent == this && m_pMDIFrame != NULL && !m_pMDIFrame->m_bClosing)
		{
			// tabbed MDI is being closed. We need to reassign parent of embedded control bar
			m_pTabbedControlBar->ShowWindow(SW_HIDE);
			m_pTabbedControlBar->SetParent(m_pTabbedControlBar->GetDockSiteFrameWnd());
			m_pMDIFrame->GetDockingManager()->AddHiddenMDITabbedBar(m_pTabbedControlBar);
		}
		m_pTabbedControlBar = NULL;
	}

	CFrameImpl::RemoveFrame(this);

	POSITION pos = NULL;

	for (pos = m_dockManager.m_lstMiniFrames.GetHeadPosition(); pos != NULL;)
	{
		CPaneFrameWnd* pNextFrame = DYNAMIC_DOWNCAST(CPaneFrameWnd, m_dockManager.m_lstMiniFrames.GetNext(pos));
		if (pNextFrame != NULL)
		{
			pNextFrame->DestroyWindow();
		}
	}

	CList<HWND, HWND> lstChildren;
	CWnd* pNextWnd = GetTopWindow();
	while (pNextWnd != NULL)
	{
		lstChildren.AddTail(pNextWnd->m_hWnd);
		pNextWnd = pNextWnd->GetNextWindow();
	}

	for (pos = lstChildren.GetHeadPosition(); pos != NULL;)
	{
		HWND hwndNext = lstChildren.GetNext(pos);
		if (IsWindow(hwndNext) && ::GetParent(hwndNext) == m_hWnd)
		{
			::DestroyWindow(hwndNext);
		}
	}

	// CMDIClientAreaWnd::OnMDIDestroy will take care about removing from the tabs.
	m_pRelatedTabGroup = NULL;

	CMDIChildWnd::OnDestroy();
}

void CMDIChildWndEx::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	if (m_pMDIFrame != NULL && m_pMDIFrame->IsFullScreen() && !m_pMDIFrame->AreMDITabs())
	{
		m_pMDIFrame->m_Impl.GetFullScreenMinMaxInfo(lpMMI);
	}
	else
	{
		CMDIChildWnd::OnGetMinMaxInfo(lpMMI);
	}
}

void CMDIChildWndEx::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	CMDIChildWnd::OnStyleChanged(nStyleType, lpStyleStruct);

	BOOL bWasSysMenu = (lpStyleStruct->styleOld & WS_SYSMENU);
	BOOL bIsSysMenu = (lpStyleStruct->styleNew & WS_SYSMENU);

	if (bWasSysMenu == bIsSysMenu)
	{
		return;
	}

	BOOL bIsInMDITabbedGroup = m_pMDIFrame != NULL && m_pMDIFrame->IsMDITabbedGroup();

	if (bWasSysMenu)
	{
		if ((lpStyleStruct->styleOld & WS_MAXIMIZE) == 0 && (lpStyleStruct->styleOld & WS_MINIMIZE) == 0)
		{
			CRect rectWindow;
			GetWindowRect(rectWindow);

			GetParent()->ScreenToClient(&rectWindow);
			m_rectOriginal = rectWindow;
		}

		if (m_pMDIFrame != NULL &&(m_pMDIFrame->m_wndClientArea.GetExStyle() & WS_EX_CLIENTEDGE) != 0)
		{
			m_pMDIFrame->m_wndClientArea.ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_FRAMECHANGED);
		}

		if (!bIsInMDITabbedGroup)
		{
			if (!IsZoomed() && bIsSysMenu)
			{
				if (CMFCVisualManager::GetInstance()->IsOwnerDrawCaption())
				{
					m_Impl.m_bIsWindowRgn = CMFCVisualManager::GetInstance()->OnSetWindowRegion(this, m_rectOriginal.Size());
				}
				else
				{
					m_Impl.m_bIsWindowRgn = FALSE;
					SetWindowRgn(NULL, TRUE);
				}
			}

			CRect rect;
			m_pMDIFrame->m_wndClientArea.GetClientRect(rect);


			CRect rectClient;
			GetClientRect(rectClient);
			ClientToScreen(rectClient);

			CRect rectScreen;
			GetWindowRect(rectScreen);

			rect.left -= rectClient.left - rectScreen.left;
			rect.top -= rectClient.top - rectScreen.top;
			rect.right += rectScreen.right - rectClient.right;
			rect.bottom += rectScreen.bottom - rectClient.bottom;

			if (!rect.IsRectNull())
			{
				SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
			}
		}
	}
	else if (!bIsInMDITabbedGroup)
	{
		if (m_pMDIFrame != NULL &&(m_pMDIFrame->m_wndClientArea.GetExStyle() & WS_EX_CLIENTEDGE) == 0)
		{
			m_pMDIFrame->m_wndClientArea.ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_FRAMECHANGED);
		}

		if (!IsZoomed() && bIsSysMenu)
		{
			if (CMFCVisualManager::GetInstance()->IsOwnerDrawCaption())
			{
				CRect rectWindow(m_rectOriginal);

				if (rectWindow.IsRectNull())
				{
					GetWindowRect(rectWindow);
					GetParent()->ScreenToClient(&rectWindow);
				}

				if (!rectWindow.IsRectNull())
				{
					m_Impl.m_bIsWindowRgn = CMFCVisualManager::GetInstance()->OnSetWindowRegion(this, rectWindow.Size());
				}
			}
			else
			{
				m_Impl.m_bIsWindowRgn = FALSE;
				SetWindowRgn(NULL, TRUE);
			}
		}

		if (!m_rectOriginal.IsRectNull())
		{
			SetWindowPos(NULL, m_rectOriginal.left, m_rectOriginal.top, m_rectOriginal.Width(), m_rectOriginal.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
		}
	}
}

LRESULT CMDIChildWndEx::OnIdleUpdateCmdUI(WPARAM, LPARAM)
{
	m_dockManager.SendMessageToMiniFrames(WM_IDLEUPDATECMDUI);
	return 0L;
}

LPCTSTR CMDIChildWndEx::GetDocumentName(CObject** /*pObj*/)
{
	CDocument* pDoc = GetActiveDocument();
	if (pDoc != NULL)
	{
		return pDoc->GetPathName();
	}
	return NULL;
}

void CMDIChildWndEx::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos)
{
	if ((lpwndpos->flags & SWP_FRAMECHANGED) == SWP_FRAMECHANGED)
	{
		if (m_pMDIFrame != NULL && !m_pMDIFrame->IsMDITabbedGroup())
		{
			m_Impl.OnWindowPosChanging(lpwndpos);
		}
		else if (m_Impl.m_bIsWindowRgn)
		{
			m_Impl.m_bIsWindowRgn = FALSE;
			SetWindowRgn(NULL, TRUE);
		}
	}

	CMDIChildWnd::OnWindowPosChanged(lpwndpos);
}

void CMDIChildWndEx::OnNcPaint()
{
	BOOL bIsInMDITabbedGroup = m_pMDIFrame != NULL && m_pMDIFrame->IsMDITabbedGroup();

	if (bIsInMDITabbedGroup || IsZoomed() || !CMFCVisualManager::GetInstance()->IsOwnerDrawCaption())
	{
		Default();
		return;
	}

	if (!m_Impl.OnNcPaint())
	{
		Default();
	}
}

BOOL CMDIChildWndEx::OnNcActivate(BOOL bActive)
{
	BOOL bIsOwnerDraw = m_Impl.OnNcActivate(bActive);

	if (bIsOwnerDraw)
	{
		SetRedraw(FALSE);
	}

	BOOL bRes = CMDIChildWnd::OnNcActivate(bActive);

	if (bIsOwnerDraw)
	{
		SetRedraw(TRUE);
		RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
	}

	return bRes;
}

void CMDIChildWndEx::OnNcMouseMove(UINT nHitTest, CPoint point)
{
	m_Impl.OnNcMouseMove(nHitTest, point);
	CMDIChildWnd::OnNcMouseMove(nHitTest, point);
}

LRESULT CMDIChildWndEx::OnNcHitTest(CPoint point)
{
	UINT nHit = m_Impl.OnNcHitTest(point);
	if (nHit != HTNOWHERE)
	{
		return nHit;
	}

	return CMDIChildWnd::OnNcHitTest(point);
}

LRESULT CMDIChildWndEx::OnChangeVisualManager(WPARAM, LPARAM)
{
	if (m_pMDIFrame != NULL && !m_pMDIFrame->IsMDITabbedGroup())
	{
		m_Impl.OnChangeVisualManager();
	}

	return 0;
}

void CMDIChildWndEx::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	BOOL bIsInMDITabbedGroup = m_pMDIFrame != NULL && m_pMDIFrame->IsMDITabbedGroup();

	if (!bIsInMDITabbedGroup && !IsZoomed() && CMFCVisualManager::GetInstance()->IsOwnerDrawCaption() && (GetStyle() & WS_BORDER) == 0)
	{
		lpncsp->rgrc[0].top += ::GetSystemMetrics(SM_CYCAPTION);
	}

	CMDIChildWnd::OnNcCalcSize(bCalcValidRects, lpncsp);
}

void CMDIChildWndEx::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_Impl.OnLButtonUp(point);
	CMDIChildWnd::OnLButtonUp(nFlags, point);
}

void CMDIChildWndEx::OnMouseMove(UINT nFlags, CPoint point)
{
	m_Impl.OnMouseMove(point);
	CMDIChildWnd::OnMouseMove(nFlags, point);
}

void CMDIChildWndEx::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_Impl.OnLButtonDown(point);
	CMDIChildWnd::OnLButtonDown(nFlags, point);
}

void CMDIChildWndEx::AddTabbedPane(CDockablePane* pControlBar)
{
	ASSERT_VALID(pControlBar);
	m_pTabbedControlBar = pControlBar;

	m_pTabbedControlBar->OnBeforeChangeParent(this);
	m_pTabbedControlBar->EnableGripper(FALSE);
	m_pTabbedControlBar->SetParent(this);
	m_pTabbedControlBar->ShowWindow(SW_SHOW);
	m_pTabbedControlBar->SetMDITabbed(TRUE);

	m_pMDIFrame->GetDockingManager()->RemoveHiddenMDITabbedBar(m_pTabbedControlBar);

	AdjustClientArea();
}

BOOL CMDIChildWndEx::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}



