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
#include "afxbasetabctrl.h"
#include "afxbasetabbedpane.h"
#include "afxpaneframewnd.h"
#include "afxmultipaneframewnd.h"
#include "afxdockablepaneadapter.h"
#include "afxautohidebar.h"
#include "afxdocksite.h"
#include "afxglobalutils.h"
#include "afxmdiframewndex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CBaseTabbedPane, CDockablePane)

/////////////////////////////////////////////////////////////////////////////
// CBaseTabbedPane

CBaseTabbedPane::CBaseTabbedPane(BOOL bAutoDestroy)
{
	m_bAutoDestroy = bAutoDestroy;
	m_pTabWnd = NULL;
	m_bEnableIDChecking = FALSE;
	m_bSetCaptionTextToTabName = TRUE;

	EnableDocking(CBRS_ALIGN_ANY);
}

CBaseTabbedPane::~CBaseTabbedPane()
{
}

//{{AFX_MSG_MAP(CBaseTabbedPane)
BEGIN_MESSAGE_MAP(CBaseTabbedPane, CDockablePane)
	ON_WM_SIZE()
	ON_WM_NCDESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SETFOCUS()
	ON_REGISTERED_MESSAGE(AFX_WM_CHANGE_ACTIVE_TAB, &CBaseTabbedPane::OnChangeActiveTab)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// CBaseTabbedPane message handlers

void CBaseTabbedPane::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	CWnd* pUnderlinedWnd = GetUnderlyingWindow();

	if (pUnderlinedWnd != NULL && IsWindow(pUnderlinedWnd->GetSafeHwnd()))
	{
		CRect rectClient;
		GetClientRect(rectClient);

		pUnderlinedWnd->SetWindowPos(NULL, 0, 0, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW);
	}
}

void CBaseTabbedPane::OnNcDestroy()
{
	if (m_pTabWnd != NULL)
	{
		delete m_pTabWnd;
		m_pTabWnd = NULL;
	}

	CDockablePane::OnNcDestroy();

	if (m_bAutoDestroy)
	{
		delete this;
	}
}

BOOL CBaseTabbedPane::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

BOOL CBaseTabbedPane::AddTab(CWnd* pNewBar, BOOL bVisible, BOOL bSetActive, BOOL bDetachable)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pTabWnd);
	ASSERT_VALID(pNewBar);

	if (pNewBar->IsKindOf(RUNTIME_CLASS(CBaseTabbedPane)))
	{
		CBaseTabbedPane* pTabbedControlBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, pNewBar);

		// it's false when the tabbed bar is added from miniframe to docksite
		BOOL bSetInfoForSlider = (GetParentMiniFrame() != NULL);

		ASSERT_VALID(pTabbedControlBar);

		CMFCBaseTabCtrl* pWndTab = pTabbedControlBar->GetUnderlyingWindow();

		ASSERT_VALID(pWndTab);

		int nTabsNum = pWndTab->GetTabsNum();
		ASSERT(nTabsNum > 0);

		for (int i = 0; i < nTabsNum; i++)
		{
			CBasePane* pWnd = DYNAMIC_DOWNCAST(CBasePane, pWndTab->GetTabWnd(i));
			ASSERT_VALID(pWnd);

			bVisible = pWndTab->IsTabVisible(i);
			bDetachable = pWndTab->IsTabDetachable(i);

			pWnd->EnableGripper(FALSE);

			if (!AddTab(pWnd, bVisible, bVisible, bDetachable))
			{
				ASSERT(FALSE);
			}

			CDockablePane* pDockingBar = DYNAMIC_DOWNCAST(CDockablePane, pWnd);
			if (pDockingBar != NULL)
			{
				pDockingBar->m_recentDockInfo.SetInfo(bSetInfoForSlider, pTabbedControlBar->m_recentDockInfo);
			}
		}

		pWndTab->RemoveAllTabs();
		pNewBar->DestroyWindow();

		// stop processing - this function will be called
		// from AttachToTabWnd
		return FALSE;
	}
	else
	{
		if (pNewBar->IsKindOf(RUNTIME_CLASS(CPane)))
		{
			CPane* pNewControlBar = DYNAMIC_DOWNCAST(CPane, pNewBar);
			ASSERT_VALID(pNewControlBar);

			CWnd* pOldParent = pNewControlBar->GetParent();
			pNewControlBar->OnBeforeChangeParent(m_pTabWnd, TRUE);
			pNewControlBar->SetParent(m_pTabWnd);
			pNewControlBar->OnAfterChangeParent(pOldParent);

			if (pNewControlBar->IsKindOf(RUNTIME_CLASS(CDockablePane)))
			{
				((CDockablePane*) pNewControlBar)->EnableGripper(FALSE);
			}
		}

		CString strText;
		pNewBar->GetWindowText(strText);

		m_pTabWnd->AddTab(pNewBar, strText, bSetActive, bDetachable);

		int iTab = m_pTabWnd->GetTabsNum() - 1;
		m_pTabWnd->SetTabHicon(iTab, pNewBar->GetIcon(FALSE));
		m_pTabWnd->EnableTabDetach(iTab, bDetachable);

		if (bVisible)
		{
			if (bSetActive)
			{
				m_pTabWnd->SetActiveTab(iTab);
			}
		}
		else
		{
			ASSERT(!bSetActive);
			m_pTabWnd->ShowTab(iTab, FALSE);
		}
	}
	return TRUE;
}


CWnd* CBaseTabbedPane::FindPaneByID(UINT uBarID)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pTabWnd);

	for (int i = 0; i < m_pTabWnd->GetTabsNum(); i++)
	{
		CWnd* pBar = m_pTabWnd->GetTabWnd(i);
		ASSERT_VALID(pBar);

		if ((UINT) pBar->GetDlgCtrlID() == uBarID)
		{
			return pBar;
		}
	}

	return NULL;
}

CWnd* CBaseTabbedPane::FindBarByTabNumber(int nTabNum, BOOL bGetWrappedBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pTabWnd);

	if (nTabNum < 0 || nTabNum >= m_pTabWnd->GetTabsNum())
	{
		return NULL;
	}

	CWnd* pWnd = m_pTabWnd->GetTabWnd(nTabNum);
	ASSERT_VALID(pWnd);

	if (bGetWrappedBar && pWnd->IsKindOf(RUNTIME_CLASS(CDockablePaneAdapter)))
	{
		CDockablePaneAdapter* pWrapper = DYNAMIC_DOWNCAST(CDockablePaneAdapter, pWnd);
		pWnd = pWrapper->GetWrappedWnd();
		ASSERT_VALID(pWnd);
	}

	return pWnd;
}

BOOL CBaseTabbedPane::DetachPane(CWnd* pBar, BOOL bHide)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);
	ASSERT_VALID(m_pTabWnd);

	int nTabNumber = m_pTabWnd->GetTabFromHwnd(pBar->GetSafeHwnd());

	if (nTabNumber < 0)
	{
		return FALSE;
	}

	m_pTabWnd->DetachTab(DM_UNKNOWN, nTabNumber, bHide);
	return TRUE;
}

BOOL CBaseTabbedPane::RemovePane(CWnd* pBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);
	ASSERT_VALID(m_pTabWnd);

	int nTabNumber = m_pTabWnd->GetTabFromHwnd(pBar->GetSafeHwnd());

	if (nTabNumber < 0 || nTabNumber >= m_pTabWnd->GetTabsNum())
	{
		return FALSE;
	}

	m_pTabWnd->RemoveTab(nTabNumber);

	if (m_pTabWnd->GetTabsNum() == 0)
	{
		if (AllowDestroyEmptyTabbedPane())
		{
			if (IsDocked())
			{
				UndockPane();
			}
			else
			{
				CPaneFrameWnd* pMiniFrame = GetParentMiniFrame();
				pMiniFrame->RemovePane(this);
			}

			DestroyWindow();
			return FALSE;
		}
		else
		{
			m_pTabWnd->ShowWindow(SW_HIDE);
		}
	}

	return TRUE;
}

BOOL CBaseTabbedPane::ShowTab(CWnd* pBar, BOOL bShow, BOOL bDelay, BOOL bActivate)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);
	ASSERT_VALID(m_pTabWnd);

	int nTabNum = m_pTabWnd->GetTabFromHwnd(pBar->GetSafeHwnd());

	BOOL bResult = m_pTabWnd->ShowTab(nTabNum, bShow, !bDelay, bActivate);
	BOOL bNowVisible = m_pTabWnd->GetVisibleTabsNum() > 0;

	if (bNowVisible && !(m_pTabWnd->GetStyle() & WS_VISIBLE))
	{
		m_pTabWnd->ShowWindow(SW_SHOW);
	}

	CDockablePane::ShowPane(bNowVisible, bDelay, bActivate);
	return bResult;
}

BOOL CBaseTabbedPane::FloatTab(CWnd* pBar, int nTabID, AFX_DOCK_METHOD dockMethod, BOOL bHide)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pBar);
	ASSERT_VALID(m_pTabWnd);

	CString strWndText;
	pBar->GetWindowText(strWndText);

	if (strWndText.IsEmpty())
	{
		if (m_pTabWnd->GetTabLabel(nTabID, strWndText))
		{
			pBar->SetWindowText(strWndText);
		}
	}

	m_pTabWnd->RemoveTab(nTabID);

	if (dockMethod == DM_MOUSE)
	{
		m_pTabWnd->SendMessage(WM_LBUTTONUP, 0, 0);
	}

	CDockablePane* pDockingBar = DYNAMIC_DOWNCAST(CDockablePane, pBar);

	if (pDockingBar != NULL)
	{
		pDockingBar->StoreRecentTabRelatedInfo();
	}

	if (dockMethod == DM_DBL_CLICK && pDockingBar != NULL)
	{
		CMultiPaneFrameWnd* pParentMiniFrame = DYNAMIC_DOWNCAST(CMultiPaneFrameWnd, GetParentMiniFrame());

		if (pParentMiniFrame != NULL)
		{
			pParentMiniFrame->DockRecentPaneToMainFrame(pDockingBar);
			return TRUE;
		}
		else if (m_hDefaultSlider != NULL && IsWindow(m_hDefaultSlider))
		{
			CMultiPaneFrameWnd* pRecentMiniFrame = DYNAMIC_DOWNCAST(CMultiPaneFrameWnd, CWnd::FromHandlePermanent(pDockingBar->m_recentDockInfo.m_hRecentMiniFrame));
			if (pRecentMiniFrame != NULL && pRecentMiniFrame->AddRecentPane(pDockingBar))
			{
				return TRUE;
			}
		}
	}

	if (pBar->IsKindOf(RUNTIME_CLASS(CPane)))
	{
		CPane* pControlBar = DYNAMIC_DOWNCAST(CPane, pBar);
		ASSERT_VALID(pControlBar);
		pControlBar->FloatPane(pControlBar->m_recentDockInfo.m_rectRecentFloatingRect, dockMethod, !bHide);
		return TRUE;
	}
	return FALSE;
}

void CBaseTabbedPane::StoreRecentDockSiteInfo()
{
	int nTabsNum = m_pTabWnd->GetTabsNum();
	for (int i = 0; i < nTabsNum; i++)
	{
		CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, m_pTabWnd->GetTabWnd(i));
		if (pBar != NULL)
		{
			pBar->StoreRecentTabRelatedInfo();
		}
	}

	CDockablePane::StoreRecentDockSiteInfo();
}

BOOL CBaseTabbedPane::FloatPane(CRect rectFloat, AFX_DOCK_METHOD dockMethod, bool bShow)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pTabWnd);

	if (!CDockablePane::FloatPane(rectFloat, dockMethod, bShow))
	{
		return FALSE;
	}

	CPaneFrameWnd* pParentFrame = GetParentMiniFrame();
	if (pParentFrame != NULL)
	{
		pParentFrame->SetIcon(m_pTabWnd->GetTabHicon(m_pTabWnd->GetActiveTab()), FALSE);
	}

	return TRUE;
}

void CBaseTabbedPane::Serialize(CArchive& ar)
{
	CDockablePane::Serialize(ar);
}

void CBaseTabbedPane::SerializeTabWindow(CArchive& ar)
{
	if (m_pTabWnd != NULL)
	{
		m_pTabWnd->Serialize(ar);
	}
}

void __stdcall CBaseTabbedPane::LoadSiblingPaneIDs(CArchive& ar, CList<UINT, UINT>& lstBarIDs)
{
	ASSERT(ar.IsLoading());
	if (ar.IsLoading())
	{
		int nTabsNum = 0;
		ar >> nTabsNum;
		for (int i = 0; i < nTabsNum; i++)
		{
			int nBarID = -1;
			ar >> nBarID;
			ASSERT(nBarID != -1);
			lstBarIDs.AddTail(nBarID);
		}
	}
}

void CBaseTabbedPane::SaveSiblingBarIDs(CArchive& ar)
{
	ASSERT_VALID(this);
	ASSERT(ar.IsStoring());
	ASSERT_VALID(m_pTabWnd);

	if (ar.IsStoring() && m_pTabWnd != NULL)
	{
		int nTabsNum = m_pTabWnd->GetTabsNum();
		// DO NOT SAVE empty tabbed bars
		if (nTabsNum > 0)
		{
			ar << nTabsNum;
			for (int i = 0; i < nTabsNum; i++)
			{
				CBasePane* pWnd = DYNAMIC_DOWNCAST(CBasePane, m_pTabWnd->GetTabWnd(i));
				ASSERT_VALID(pWnd);

				ar << pWnd->GetDlgCtrlID();
			}
		}
	}
}

BOOL CBaseTabbedPane::LoadState(LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pTabWnd);

	FillDefaultTabsOrderArray();

	// if initially tabbed bars were detached by user and exist only as regular
	// docking control bars we need to give them a chance to load their state
	// from the registry

	CDockablePane::LoadState(lpszProfileName, nIndex, uiID);

	int nTabsNum = m_pTabWnd->GetTabsNum();
	for (int i = 0; i < nTabsNum; i++)
	{
		CBasePane* pWnd = DYNAMIC_DOWNCAST(CBasePane, m_pTabWnd->GetTabWnd(i));

		if (pWnd != NULL)
		{
			ASSERT_VALID(pWnd);
			pWnd->LoadState(lpszProfileName, nIndex, uiID);
		}
	}

	return TRUE;
}

BOOL CBaseTabbedPane::SaveState(LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pTabWnd);

	CDockablePane::SaveState(lpszProfileName, nIndex, uiID);

	int nTabsNum = m_pTabWnd->GetTabsNum();
	for (int i = 0; i < nTabsNum; i++)
	{
		CBasePane* pWnd = DYNAMIC_DOWNCAST(CBasePane, m_pTabWnd->GetTabWnd(i));

		if (pWnd != NULL)
		{
			ASSERT_VALID(pWnd);
			if (!pWnd->SaveState(lpszProfileName, nIndex, uiID))
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}


void CBaseTabbedPane::ApplyRestoredTabInfo(BOOL bUseTabIndexes)
{
	ASSERT_VALID(this);

	if (m_pTabWnd != NULL)
	{
		m_pTabWnd->ApplyRestoredTabInfo(bUseTabIndexes);
	}
}

void CBaseTabbedPane::RecalcLayout()
{
	ASSERT_VALID(this);

	CDockablePane::RecalcLayout();

	if (m_pTabWnd != NULL)
	{
		m_pTabWnd->RecalcLayout();
	}
}

BOOL CBaseTabbedPane::CanFloat() const
{
	ASSERT_VALID(this);

	return CDockablePane::CanFloat();
}

void CBaseTabbedPane::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	// Pass the focus to the tab window
	CWnd* pWndChild = GetUnderlyingWindow();
	if (pWndChild != NULL)
		pWndChild->SetFocus();
}

CMFCAutoHideBar* CBaseTabbedPane::SetAutoHideMode(BOOL bMode, DWORD dwAlignment, CMFCAutoHideBar* pCurrAutoHideBar, BOOL bUseTimer)
{
	BOOL bHandleMinSize = CPane::m_bHandleMinSize;
	if (bHandleMinSize)
	{
		CPane::m_bHandleMinSize = FALSE;
	}

	CMFCAutoHideBar* pAutoHideBar = pCurrAutoHideBar;
	CDockablePane* pActiveBar = NULL;

	int nActiveTab = m_pTabWnd->GetActiveTab();
	int nTabsNum = m_pTabWnd->GetTabsNum();

	CObList lstTmp;

	ShowPane(FALSE, TRUE, FALSE);

	int nNonDetachedCount = 0;
	for (int nNextTab = nTabsNum - 1; nNextTab >= 0; nNextTab--)
	{
		CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, m_pTabWnd->GetTabWnd(nNextTab));
		ASSERT_VALID(pBar);

		BOOL bIsVisible = m_pTabWnd->IsTabVisible(nNextTab);
		BOOL bDetachable = m_pTabWnd->IsTabDetachable(nNextTab);

		if (pBar != NULL && bIsVisible && bDetachable)
		{
			m_pTabWnd->RemoveTab(nNextTab, FALSE);
			pBar->EnableGripper(TRUE);

			pBar->StoreRecentTabRelatedInfo();

			CWnd* pOldParent = pBar->GetParent();
			pBar->OnBeforeChangeParent(m_pDockSite);
			pBar->SetParent(m_pDockSite);
			pBar->SetOwner(m_pDockSite);
			pBar->OnAfterChangeParent(pOldParent);

			lstTmp.AddHead(pBar);

			if (nNextTab == nActiveTab)
			{
				pActiveBar = pBar;
			}
		}
		else
		{
			nNonDetachedCount++;
		}
	}

	BOOL bActiveSet = FALSE;
	CPane* pNewAHBar = NULL;

	for (POSITION pos = lstTmp.GetHeadPosition(); pos != NULL;)
	{
		CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, lstTmp.GetNext(pos));
		ENSURE(pBar != NULL);
		ASSERT_VALID(pBar);

		BOOL bUseTimerForActiveBar = (pBar == pActiveBar) && bUseTimer;
		pNewAHBar = pBar->SetAutoHideMode(TRUE, dwAlignment, NULL, bUseTimerForActiveBar);

		if (pNewAHBar != NULL)
		{
			pNewAHBar->m_bFirstInGroup = (lstTmp.GetHead() == pBar);
			pNewAHBar->m_bLastInGroup = (lstTmp.GetTail() == pBar);
			pNewAHBar->m_bActiveInGroup = (pBar == pActiveBar);

			if (!bActiveSet && pNewAHBar->m_bActiveInGroup)
			{
				bActiveSet = TRUE;
			}
		}
	}

	if (pNewAHBar != NULL)
	{
		if (!bActiveSet)
		{
			pNewAHBar->m_bActiveInGroup = TRUE;
		}
		CRect rect(0, 0, 0, 0);
		pNewAHBar->GetParentDockSite()->RepositionPanes(rect);
	}

	if (nNonDetachedCount > 0)
	{
		if (m_pTabWnd->GetVisibleTabsNum() == 0)
		{
			ShowPane(FALSE, TRUE, FALSE);
		}
		else
		{
			if (m_pTabWnd->GetActiveTab() == -1)
			{
				int nVisibleTab = -1;
				GetFirstVisibleTab(nVisibleTab);
				m_pTabWnd->SetActiveTab(nVisibleTab);
			}
			m_pTabWnd->RecalcLayout();
			ShowPane(TRUE, TRUE, FALSE);
			pAutoHideBar = CDockablePane::SetAutoHideMode(bMode, dwAlignment, pCurrAutoHideBar, bUseTimer);
		}
	}

	if (pAutoHideBar != NULL)
	{
		pAutoHideBar->UpdateVisibleState();
	}

	CPane::m_bHandleMinSize = bHandleMinSize;

	return pAutoHideBar;
}

CWnd* CBaseTabbedPane::GetFirstVisibleTab(int& iTabNum)
{
	iTabNum = -1;
	if (m_pTabWnd == NULL)
	{
		return NULL;
	}

	return m_pTabWnd->GetFirstVisibleTab(iTabNum);
}

HICON CBaseTabbedPane::GetPaneIcon(BOOL bBigIcon)
{
	HICON hIcon = GetIcon(bBigIcon);

	if (hIcon == NULL && m_pTabWnd != NULL)
	{
		CWnd* pWnd = m_pTabWnd->GetActiveWnd();
		if (pWnd != NULL)
		{
			hIcon = pWnd->GetIcon(bBigIcon);
		}
	}

	return hIcon;
}

LRESULT CBaseTabbedPane::OnChangeActiveTab(WPARAM wp, LPARAM)
{
	int iTabNum = (int) wp;

	CString strLabel;
	if (m_pTabWnd != NULL && m_pTabWnd->GetTabLabel(iTabNum, strLabel) && m_bSetCaptionTextToTabName)
	{
		SetWindowText(strLabel);
	}

	OnActivateTab(iTabNum);
	if (CPane::m_bHandleMinSize)
	{
		CPaneFrameWnd* pWnd = GetParentMiniFrame();
		if (pWnd != NULL)
		{
			pWnd->OnPaneRecalcLayout();
		}
		else
		{
			afxGlobalUtils.ForceAdjustLayout(afxGlobalUtils.GetDockingManager(GetDockSiteFrameWnd()));
		}
	}
	return 0;
}

BOOL CBaseTabbedPane::Dock(CBasePane* pTargetBar, LPCRECT lpRect, AFX_DOCK_METHOD dockMethod)
{
	BOOL bFloating = (GetParentMiniFrame() != NULL);
	int nTabsNum = m_pTabWnd->GetTabsNum();
	BOOL bTabsHaveRecentInfo = TRUE;

	if (bFloating)
	{
		for (int i = 0; i < nTabsNum; i++)
		{
			if (m_pTabWnd->IsTabDetachable(i))
			{
				CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane,
					m_pTabWnd->GetTabWnd(i));
				if (pBar != NULL)
				{
					ASSERT_VALID(pBar);
					if (pBar->m_recentDockInfo.GetRecentPaneContainer(TRUE) == NULL && pBar->m_recentDockInfo.GetRecentTabContainer(TRUE) == NULL)
					{
						bTabsHaveRecentInfo = FALSE;
						break;
					}
				}
			}
		}
	}

	if (dockMethod != DM_DBL_CLICK || !bTabsHaveRecentInfo)
	{
		return CDockablePane::Dock(pTargetBar, lpRect, dockMethod);
	}

	if (bFloating && m_recentDockInfo.GetRecentPaneContainer(TRUE) != NULL || !bFloating && m_recentDockInfo.GetRecentPaneContainer(FALSE) != NULL)
	{
		return CDockablePane::Dock(pTargetBar, lpRect, dockMethod);
	}

	ShowPane(FALSE, TRUE, FALSE);

	int nNonDetachedCount = 0;
	for (int nNextTab = nTabsNum - 1; nNextTab >= 0; nNextTab--)
	{
		CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, m_pTabWnd->GetTabWnd(nNextTab));
		ASSERT_VALID(pBar);

		BOOL bIsVisible = m_pTabWnd->IsTabVisible(nNextTab);
		BOOL bDetachable = m_pTabWnd->IsTabDetachable(nNextTab);

		if (pBar != NULL && bIsVisible && bDetachable)
		{
			m_pTabWnd->RemoveTab(nNextTab, FALSE);
			pBar->EnableGripper(TRUE);

			pBar->StoreRecentTabRelatedInfo();

			pBar->DockPane(pBar, lpRect, dockMethod);
		}
		else
		{
			nNonDetachedCount++;
		}
	}

	if (nNonDetachedCount > 0)
	{
		if (m_pTabWnd->GetVisibleTabsNum() == 0)
		{
			ShowPane(FALSE, TRUE, FALSE);
		}
		else
		{
			if (m_pTabWnd->GetActiveTab() == -1)
			{
				int nVisibleTab = -1;
				GetFirstVisibleTab(nVisibleTab);
				m_pTabWnd->SetActiveTab(nVisibleTab);
			}
			m_pTabWnd->RecalcLayout();
			ShowPane(TRUE, TRUE, FALSE);
			return CDockablePane::Dock(pTargetBar, lpRect, dockMethod);

		}
	}
	else
	{
		DestroyWindow();
		return FALSE;
	}

	return TRUE;
}

void CBaseTabbedPane::FillDefaultTabsOrderArray()
{
	ASSERT_VALID(m_pTabWnd);
	m_arDefaultTabsOrder.RemoveAll();

	const int nTabsNum = m_pTabWnd->GetTabsNum();

	for (int i = 0; i < nTabsNum; i++)
	{
		int nID = m_pTabWnd->GetTabID(i);
		m_arDefaultTabsOrder.Add(nID);
	}
}

void CBaseTabbedPane::GetMinSize(CSize& size) const
{
	if (CPane::m_bHandleMinSize)
	{
		CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, m_pTabWnd->GetActiveWnd());
		if (pBar != NULL)
		{
			pBar->GetMinSize(size);
			return;

		}
	}
	CDockablePane::GetMinSize(size);
}

void CBaseTabbedPane::GetPaneList(CObList& lst, CRuntimeClass* pRTCFilter)
{
	CMFCBaseTabCtrl* pTabWnd = GetUnderlyingWindow();
	for (int i = 0; i < pTabWnd->GetTabsNum(); i++)
	{
		CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, pTabWnd->GetTabWnd(i));
		if (pBar != NULL)
		{
			ASSERT_VALID(pBar);
			if (pRTCFilter == NULL || pBar->GetRuntimeClass() == pRTCFilter)
			{
				lst.AddTail(pBar);
			}
		}
	}
}

void CBaseTabbedPane::ConvertToTabbedDocument(BOOL bActiveTabOnly)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pTabWnd);

	CMDIFrameWndEx* pMDIFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetDockSiteFrameWnd());
	if (pMDIFrame == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	ASSERT_VALID(pMDIFrame);

	HWND hwnd = GetSafeHwnd();

	if (bActiveTabOnly)
	{
		CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, m_pTabWnd->GetActiveWnd());
		if (pBar == NULL)
		{
			return;
		}

		pBar->StoreRecentTabRelatedInfo();
		pMDIFrame->ControlBarToTabbedDocument(pBar);
		RemovePane(pBar);
	}
	else
	{
		CObList lst;
		CMFCBaseTabCtrl* pTabWnd = GetUnderlyingWindow();

		for (int i = 0; i < pTabWnd->GetTabsNum(); i++)
		{
			if (pTabWnd->IsTabVisible(i))
			{
				CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, pTabWnd->GetTabWnd(i));
				if (pBar != NULL)
				{
					pBar->StoreRecentTabRelatedInfo();
					lst.AddTail(pBar);
				}
			}
		}

		for (POSITION pos = lst.GetHeadPosition(); pos != NULL;)
		{
			CDockablePane* pBar = (CDockablePane*) lst.GetNext(pos);
			pMDIFrame->ControlBarToTabbedDocument(pBar);
			RemovePane(pBar);
		}
	}

	if (IsWindow(hwnd) && GetVisibleTabsNum() == 0 && GetTabsNum() > 0)
	{
		ShowPane(FALSE, FALSE, FALSE);
	}
}



