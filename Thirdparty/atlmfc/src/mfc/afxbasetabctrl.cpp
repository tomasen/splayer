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
#include "afxdockablepane.h"
#include "afxdockablepaneadapter.h"
#include "afxbasetabbedpane.h"
#include "afxpaneframewnd.h"
#include "afxdockingmanager.h"
#include "afxglobalutils.h"
#include "afxmdiclientareawnd.h"
#include "afxtooltipmanager.h"
#include "afxribbonres.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCTabDropTarget message handlers

BOOL CMFCTabDropTarget::Register(CMFCBaseTabCtrl *pOwner)
{
	m_pOwner = pOwner;
	return COleDropTarget::Register(pOwner);
}

DROPEFFECT CMFCTabDropTarget::OnDragEnter(CWnd* /*pWnd*/, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	ENSURE(m_pOwner != NULL);

	if (!CMFCToolBar::IsCustomizeMode() || !pDataObject->IsDataAvailable(CMFCToolBarButton::m_cFormat))
	{
		return DROPEFFECT_NONE;
	}

	return m_pOwner->OnDragEnter(pDataObject, dwKeyState, point);
}

void CMFCTabDropTarget::OnDragLeave(CWnd* /*pWnd*/)
{
	ENSURE(m_pOwner != NULL);
	m_pOwner->OnDragLeave();
}

DROPEFFECT CMFCTabDropTarget::OnDragOver(CWnd* /*pWnd*/, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	ENSURE(m_pOwner != NULL);

	if (!CMFCToolBar::IsCustomizeMode() || !pDataObject->IsDataAvailable(CMFCToolBarButton::m_cFormat))
	{
		if (m_pOwner != NULL)
		{
			int nTabIndex = m_pOwner->GetTabFromPoint(point);

			if (nTabIndex != -1 && nTabIndex != m_pOwner->GetActiveTab())
			{
				m_pOwner->SetActiveTab(nTabIndex);
			}
		}

		return(DROPEFFECT_NONE);
	}


	return m_pOwner->OnDragOver(pDataObject, dwKeyState, point);
}

DROPEFFECT CMFCTabDropTarget::OnDropEx(CWnd* /*pWnd*/, COleDataObject* pDataObject, DROPEFFECT dropEffect, DROPEFFECT /*dropList*/, CPoint point)
{
	ENSURE(m_pOwner != NULL);

	if (!CMFCToolBar::IsCustomizeMode() || !pDataObject->IsDataAvailable(CMFCToolBarButton::m_cFormat))
	{
		return DROPEFFECT_NONE;
	}

	return m_pOwner->OnDrop(pDataObject, dropEffect, point) ? dropEffect : DROPEFFECT_NONE;
}

/////////////////////////////////////////////////////////////////////////////
// CMFCBaseTabCtrl

int CMFCBaseTabCtrl::AFX_TAB_TEXT_MARGIN = 4;
int CMFCBaseTabCtrl::AFX_TAB_IMAGE_MARGIN = 4;

#define AFX_DEFAULT_TAB_BORDER_SIZE 2

UINT AFX_WM_ON_RENAME_TAB = ::RegisterWindowMessage(_T("AFX_WM_ON_RENAME_TAB"));
UINT AFX_WM_CHANGE_ACTIVE_TAB = ::RegisterWindowMessage(_T("AFX_WM_ONCHANGE_ACTIVE_TAB"));
UINT AFX_WM_ON_MOVE_TAB = ::RegisterWindowMessage(_T("AFX_WM_ON_MOVE_TAB"));
UINT AFX_WM_CHANGING_ACTIVE_TAB = ::RegisterWindowMessage(_T("AFX_WM_ON_CHANGING_ACTIVE_TAB"));
UINT AFX_WM_ON_GET_TAB_TOOLTIP = ::RegisterWindowMessage(_T("AFX_WM_ON_GET_TAB_TOOLTIP"));

IMPLEMENT_DYNAMIC(CMFCBaseTabCtrl, CWnd)

CMFCBaseTabCtrl::CMFCBaseTabCtrl()
{
	m_bAutoDestroyWindow = FALSE;
	m_iActiveTab = -1;
	m_iTabBeforeDrag = -1;
	m_iTabsNum = 0;

	m_sizeImage = CSize(0, 0);
	m_hImageList = NULL;

	m_iCurTab = -1;
	m_nNextTabID = 1;

	m_bCustomToolTips = FALSE;

	m_bHideInactiveWnd = TRUE;

	m_location = LOCATION_BOTTOM;
	m_bReadyToDetach = FALSE;
	m_ptHot = CPoint(0, 0);
	m_nOffsetFromTabLeft = 0;

	m_iHighlighted = -1;
	m_iPressed = -1;

	m_bActivateOnBtnUp = FALSE;

	m_bEnableTabSwap = TRUE;
	m_nRestoredActiveTabID = 0;

	m_pDockingBarWrapperRTC = NULL;

	m_bEnableWrapping = FALSE;

	m_clrActiveTabBk = (COLORREF) -1;
	m_clrActiveTabFg = (COLORREF) -1;

	m_nTabBorderSize = AFX_DEFAULT_TAB_BORDER_SIZE;

	m_nTabsHeight = 0;

	// in-place editing
	m_pInPlaceEdit = NULL;
	m_iEditedTab = -1;

	m_bIsInPlaceEdit = FALSE;
	m_bHideSingleTab = FALSE;

	m_bLabelNoPrefix = FALSE;
	m_bActivateLastVisibleTab = FALSE;
	m_bHighLightTabs = FALSE;

	m_bIsAutoColor = FALSE;
	m_bIsDefaultAutoColor = TRUE;

	m_bIsDlgControl = FALSE;

	m_iLastActiveTab = -1;
	m_bActivateLastActiveTab = FALSE;

	m_bIsMDITab = FALSE;
	m_bSetActiveTabFired = FALSE;
	m_bUserSelectedTab = FALSE;

	m_bSetActiveTabByMouseClick = FALSE;
	m_bActivateTabOnRightClick = FALSE;

	m_bWindowPosChanged = FALSE;

	m_pToolTip = NULL;
	m_pToolTipClose = NULL;

	m_bTabCloseButtonHighlighted = FALSE;
	m_bTabCloseButtonPressed = FALSE;
	m_rectCloseButton.SetRectEmpty();
}

CMFCBaseTabCtrl::~CMFCBaseTabCtrl()
{
}

//{{AFX_MSG_MAP(CMFCBaseTabCtrl)
BEGIN_MESSAGE_MAP(CMFCBaseTabCtrl, CWnd)
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_CANCELMODE()
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_WINDOWPOSCHANGED()
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, &CMFCBaseTabCtrl::OnNeedTipText)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// CMFCBaseTabCtrl message handlers

void CMFCBaseTabCtrl::CleanUp()
{
	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [i];
		ASSERT_VALID(pTab);

		// we need to delete tab info only in the case the contained window is not
		// derived from CPane, because CPane detects that it's
		// tabbed while processing OnDestroy and removes itself from the parent tabbed
		// window.
		if (pTab->m_pWnd->m_hWnd == NULL || ::IsWindow(pTab->m_pWnd->m_hWnd))
		{
			BOOL bDeleteTabInfo = !pTab->m_pWnd->IsKindOf(RUNTIME_CLASS(CPane));

			if (m_bAutoDestroyWindow)
			{
				pTab->m_pWnd->DestroyWindow();
			}

			if (bDeleteTabInfo || !m_bAutoDestroyWindow)
			{
				delete pTab;
			}
		}
	}

	CTooltipManager::DeleteToolTip(m_pToolTip);
	CTooltipManager::DeleteToolTip(m_pToolTipClose);

	m_arTabs.RemoveAll();
	m_iTabsNum = 0;
	m_iActiveTab = -1;
}

void CMFCBaseTabCtrl::AddTab(CWnd* pNewWnd, LPCTSTR lpszName, UINT uiImageId, BOOL bDetachable)
{
	if (pNewWnd->GetSafeHwnd() != NULL && pNewWnd->GetDlgCtrlID() == -1)
	{
		ASSERT(FALSE);
		TRACE0("Unable to add a new tab with control bar ID -1. \n");
		return;
	}

	CWnd* pWndToAdd = CreateWrapper(pNewWnd, lpszName, bDetachable);
	ASSERT_VALID(pWndToAdd);

	InsertTab(pWndToAdd, lpszName, -1, uiImageId, bDetachable);
}

void CMFCBaseTabCtrl::AddTab(CWnd* pTabWnd, UINT uiResTabLabel, UINT uiImageId, BOOL bDetachable)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pTabWnd);

	if (pTabWnd->GetDlgCtrlID() == -1)
	{
		ASSERT(FALSE);
		TRACE0("Unable to add a new tab with control bar ID -1. \n");
		return;
	}

	CString strLabel;
	ENSURE(strLabel.LoadString(uiResTabLabel));

	CWnd* pWndToAdd = CreateWrapper(pTabWnd, strLabel, bDetachable);
	ASSERT_VALID(pWndToAdd);

	AddTab(pWndToAdd, strLabel, uiImageId, bDetachable);
}

void CMFCBaseTabCtrl::InsertTab(CWnd* pNewWnd, LPCTSTR lpszTabLabel, int nInsertAt, UINT uiImageId, BOOL bDetachable)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pNewWnd);
	ENSURE(lpszTabLabel != NULL);

	CWnd* pWndToAdd = CreateWrapper(pNewWnd, lpszTabLabel, bDetachable);
	ASSERT_VALID(pWndToAdd);

	if (!IsWindowVisible())
	{
		ShowWindow(SW_SHOW);
	}

	if (nInsertAt < 0 || nInsertAt > m_iTabsNum)
	{
		nInsertAt = m_iTabsNum;
	}

	CWnd* pActiveWnd = GetActiveWnd();

	// can't detach non-docking control bar derived objects
	if (!pWndToAdd->IsKindOf(RUNTIME_CLASS(CDockablePane)))
	{
		bDetachable = FALSE;
	}

	m_arTabs.InsertAt(nInsertAt, new CMFCTabInfo(lpszTabLabel, uiImageId, pWndToAdd, m_nNextTabID, bDetachable));

	m_iTabsNum++;

	if (m_pToolTip->GetSafeHwnd() != NULL)
	{
		CRect rectEmpty(0, 0, 0, 0);
		m_pToolTip->AddTool(this, m_bCustomToolTips ? LPSTR_TEXTCALLBACK : lpszTabLabel, &rectEmpty, m_nNextTabID);
	}

	m_nNextTabID ++;

	OnChangeTabs();
	RecalcLayout();

	if (m_iTabsNum == 1)
	{
		//----------------------------------------
		// First tab automatically becames active:
		//----------------------------------------
		SetActiveTab(0);
	}
	else
	{
		m_iLastActiveTab = m_iActiveTab;

		if (m_iActiveTab == nInsertAt)
		{
			m_iLastActiveTab++;

			if (m_bHideInactiveWnd && pActiveWnd != NULL)
			{
				pActiveWnd->ShowWindow(SW_HIDE);
			}

			pWndToAdd->ShowWindow(SW_SHOWNORMAL);
		}
		else if (m_bHideInactiveWnd)
		{
			if (pWndToAdd->GetSafeHwnd() != NULL)
			{
				pWndToAdd->ShowWindow(SW_HIDE);
			}
		}
	}

	if (!m_bHideInactiveWnd && pActiveWnd != NULL && pActiveWnd->GetSafeHwnd() != NULL)
	{
		pActiveWnd->BringWindowToTop();
	}
}

void CMFCBaseTabCtrl::InsertTab(CWnd* pNewWnd, UINT uiResTabLabel, int nInsertAt, UINT uiImageId, BOOL bDetachable)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pNewWnd);

	if (pNewWnd->GetDlgCtrlID() == -1)
	{
		ASSERT(FALSE);
		TRACE0("Unable to add a new tab with control bar ID -1. \n");
		return;
	}

	CString strLabel;
	ENSURE(strLabel.LoadString(uiResTabLabel));

	CWnd* pWndToAdd = CreateWrapper(pNewWnd, strLabel, bDetachable);
	ASSERT_VALID(pWndToAdd);

	InsertTab(pWndToAdd, strLabel, nInsertAt, uiImageId, bDetachable);
}

BOOL CMFCBaseTabCtrl::RemoveTab(int iTab, BOOL bRecalcLayout)
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		TRACE(_T("RemoveTab: illegal tab number %d\n"), iTab);
		return FALSE;
	}

	if (m_iTabsNum == 1)
	{
		RemoveAllTabs();
		return TRUE;
	}

	CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	if (m_pToolTip->GetSafeHwnd() != NULL)
	{
		m_pToolTip->DelTool(this, pTab->m_iTabID);
	}

	//----------------------------
	// Detach tab from collection:
	//----------------------------
	m_arTabs.RemoveAt(iTab);
	m_iTabsNum --;

	//-----------------------------------
	// Destroy tab window and delete tab:
	//-----------------------------------
	if (m_bAutoDestroyWindow)
	{
		ASSERT_VALID(pTab->m_pWnd);
		pTab->m_pWnd->DestroyWindow();
	}

	delete pTab;

	int iActiveTab = m_iActiveTab;
	if (m_iActiveTab >= iTab)
	{
		if (m_bActivateLastVisibleTab)
		{
			GetLastVisibleTab(iActiveTab);
		}
		else
		{
			// Find the next best tab to be activated
			for (int i = m_iTabsNum - 1; i >= 0; --i)
			{
				CMFCTabInfo* pNextActiveTab = (CMFCTabInfo*) m_arTabs [i];
				ASSERT_VALID(pNextActiveTab);

				if (i < iTab && iActiveTab >= 0 && iActiveTab < m_iTabsNum)
				{
					break;
				}

				if (pNextActiveTab->m_bVisible)
				{
					iActiveTab = i;
				}
			}
		}

		m_iActiveTab = -1;
	}

	OnChangeTabs();

	if (bRecalcLayout)
	{
		RecalcLayout();
		if (iActiveTab != -1)
		{
			if (m_bActivateLastActiveTab &&(m_iLastActiveTab != -1))
			{
				int iLastActiveTab = m_iLastActiveTab;
				if (iTab < m_iLastActiveTab)
				{
					iLastActiveTab = m_iLastActiveTab -1;
				}

				int iTabToActivate = -1;
				GetFirstVisibleTab(iLastActiveTab, iTabToActivate);

				SetActiveTab(iTabToActivate);
			}
			else
			{
				int iTabToActivate = -1;
				GetFirstVisibleTab(iActiveTab, iTabToActivate);
				SetActiveTab(iTabToActivate);
			}

			FireChangeActiveTab(m_iActiveTab);
		}
	}

	return TRUE;
}

void CMFCBaseTabCtrl::RemoveAllTabs()
{
	m_iActiveTab = -1;
	m_nNextTabID = 1;

	while (m_iTabsNum > 0)
	{
		CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [m_iTabsNum - 1];
		ASSERT_VALID(pTab);

		m_iTabsNum--;

		if (m_pToolTip->GetSafeHwnd() != NULL)
		{
			m_pToolTip->DelTool(this, pTab->m_iTabID);
		}

		if (m_bAutoDestroyWindow)
		{
			pTab->m_pWnd->DestroyWindow();
		}

		delete pTab;
	}

	if (m_pToolTip->GetSafeHwnd() != NULL)
	{
		ASSERT(m_pToolTip->GetToolCount() == 0);
	}

	m_arTabs.RemoveAll();

	OnChangeTabs();

	RecalcLayout();
	FireChangeActiveTab((UINT)-1);
}

int CMFCBaseTabCtrl::GetVisibleTabsNum() const
{
	int nCount = 0;

	for (int i = 0; i < m_iTabsNum; i++)
	{
		CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [i];
		ASSERT_VALID(pTab);

		if (pTab->m_bVisible)
		{
			nCount++;
		}
	}

	return nCount;
}

BOOL CMFCBaseTabCtrl::ShowTab(int iTab, BOOL bShow /*= TRUE*/, BOOL bRecalcLayout /*= TRUE*/, BOOL bActivate)
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		TRACE(_T("ShowTab: illegal tab number %d\n"), iTab);
		return FALSE;
	}

	CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	if (pTab->m_bVisible == bShow)
	{
		return TRUE;
	}

	int nVisibleCount = GetVisibleTabsNum();
	pTab->m_bVisible = bShow;

	int iActiveTab = (bShow ? m_iActiveTab : -1);

	if (!bShow)
	{
		//----------
		// Hide tab:
		//----------
		if (m_bHideInactiveWnd)
		{
			ASSERT_VALID(pTab->m_pWnd);
			pTab->m_pWnd->ShowWindow(SW_HIDE);
		}

		if (iTab == m_iActiveTab)
		{
			// Find the next best tab to be activated
			for (int i = m_iTabsNum - 1; i >= 0; --i)
			{
				CMFCTabInfo* pNextActiveTab = (CMFCTabInfo*) m_arTabs [i];
				ASSERT_VALID(pNextActiveTab);

				if (i < iTab && iActiveTab >= 0)
				{
					break;
				}

				if (pNextActiveTab->m_bVisible)
				{
					iActiveTab = i;
				}
			}

			m_iActiveTab = -1;
		}
	}

	// If there was no tab visible, activate this first one
	if (bShow && nVisibleCount == 0)
	{
		iActiveTab = iTab;
	}

	if (bRecalcLayout)
	{
		RecalcLayout();
	}

	if (iActiveTab >= 0 && !bShow && m_iActiveTab == -1 || bActivate || nVisibleCount == 0)
	{
		SetActiveTab(iActiveTab);
		FireChangeActiveTab(m_iActiveTab);
	}

	return TRUE;
}

CWnd* CMFCBaseTabCtrl::GetTabWnd(int iTab) const
{
	if (iTab >= 0 && iTab < m_iTabsNum)
	{
		CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [iTab];
		ASSERT_VALID(pTab);

		return pTab->m_pWnd;
	}
	else
	{
		return NULL;
	}
}

CWnd* CMFCBaseTabCtrl::GetTabWndNoWrapper(int iTab) const
{
	if (iTab >= 0 && iTab < m_iTabsNum)
	{
		CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [iTab];
		ASSERT_VALID(pTab);

		CDockablePaneAdapter* pWrapper = DYNAMIC_DOWNCAST(CDockablePaneAdapter, pTab->m_pWnd);
		if (pWrapper != NULL)
		{
			return pWrapper->GetWrappedWnd();
		}

		return pTab->m_pWnd;
	}
	else
	{
		return NULL;
	}
}

BOOL CMFCBaseTabCtrl::GetTabRect(int iTab, CRect& rect) const
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}

	CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	if (!pTab->m_bVisible)
	{
		rect.SetRectEmpty();
		return FALSE;
	}

	rect = pTab->m_rect;
	return TRUE;
}

int CMFCBaseTabCtrl::GetTabID(int iTab) const
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return -1;
	}

	CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	return pTab->m_iTabID;
}

BOOL CMFCBaseTabCtrl::GetTabLabel(int iTab, CString& strLabel) const
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}

	CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	strLabel = pTab->m_bIconOnly ? _T("") : pTab->m_strText;
	return TRUE;
}

BOOL CMFCBaseTabCtrl::SetTabLabel(int iTab, const CString& strLabel)
{
	if (iTab < 0 || iTab >= m_iTabsNum || strLabel.IsEmpty())
	{
		return FALSE;
	}

	CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	pTab->m_strText = strLabel;

	if (m_pToolTip->GetSafeHwnd() != NULL)
	{
		if (m_bCustomToolTips)
		{
			CToolInfo info;

			m_pToolTip->GetToolInfo(info, this, pTab->m_iTabID);
			m_pToolTip->DelTool(this, pTab->m_iTabID);

			m_pToolTip->AddTool(this, LPSTR_TEXTCALLBACK, &info.rect, pTab->m_iTabID);
		}
		else
		{
			m_pToolTip->UpdateTipText(strLabel, this, pTab->m_iTabID);
		}
	}

	if (pTab->m_pWnd->GetSafeHwnd() != NULL)
	{
		pTab->m_pWnd->SetWindowText(strLabel);
	}

	RecalcLayout();

	if (iTab == m_iActiveTab)
	{
		//--------------------------------------------------
		// Set text to the parent frame/docking control bar:
		//--------------------------------------------------
		CBaseTabbedPane* pTabControlBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, GetParent());
		if (pTabControlBar != NULL) // tabbed dock bar - redraw caption only in this case
		{
			// miniframe will take the text from the tab control bar
			if (pTabControlBar->CanSetCaptionTextToTabName())
			{
				pTabControlBar->SetWindowText(strLabel);
			}

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
	}

	return TRUE;
}
UINT CMFCBaseTabCtrl::GetTabIcon(int iTab) const
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return(UINT) -1;
	}

	CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	return pTab->m_uiIcon;
}

BOOL CMFCBaseTabCtrl::SetTabIcon(int iTab, UINT uiIcon)
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}

	CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	if (pTab->m_hIcon != NULL)
	{
		::DestroyIcon(pTab->m_hIcon);
	}

	pTab->m_uiIcon = uiIcon;
	pTab->m_hIcon = NULL;

	return TRUE;
}

HICON CMFCBaseTabCtrl::GetTabHicon(int iTab) const
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return NULL;
	}

	CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	return pTab->m_hIcon;
}

BOOL CMFCBaseTabCtrl::SetTabHicon(int iTab, HICON hIcon)
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}

	CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	if (pTab->m_hIcon != NULL)
	{
		::DestroyIcon(pTab->m_hIcon);
	}

	if (hIcon != NULL)
	{
		pTab->m_hIcon = ::CopyIcon(hIcon);
	}
	else
	{
		pTab->m_hIcon = NULL;
	}

	pTab->m_uiIcon = (UINT)-1;

	m_sizeImage.cx = max(m_sizeImage.cx, afxGlobalData.m_sizeSmallIcon.cx);
	m_sizeImage.cy = max(m_sizeImage.cy, afxGlobalData.m_sizeSmallIcon.cy);

	SetTabsHeight();

	return TRUE;
}

BOOL CMFCBaseTabCtrl::IsTabIconOnly(int iTab) const
{
	ASSERT_VALID(this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}

	CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	return pTab->m_bIconOnly;
};

BOOL CMFCBaseTabCtrl::SetTabIconOnly(int iTab, BOOL bIconOnly, BOOL bAlwaysShowToolTip)
{
	ASSERT_VALID(this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}

	CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	pTab->m_bIconOnly = bIconOnly;
	pTab->m_bAlwaysShowToolTip = bAlwaysShowToolTip;

	RecalcLayout();
	return TRUE;
}

BOOL CMFCBaseTabCtrl::IsTabDetachable(int iTab) const
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}

	CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	return pTab->m_bIsDetachable;
}

BOOL CMFCBaseTabCtrl::EnableTabDetach(int iTab, BOOL bEnable)
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}

	CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	pTab->m_bIsDetachable = bEnable;
	return TRUE;
}

CWnd* CMFCBaseTabCtrl::GetActiveWnd() const
{
	return m_iActiveTab == -1 ? NULL : ((CMFCTabInfo*) m_arTabs [m_iActiveTab])->m_pWnd;
}

int CMFCBaseTabCtrl::GetTabFromPoint(CPoint& pt) const
{
	for (int i = 0; i < m_iTabsNum; i ++)
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

int CMFCBaseTabCtrl::GetTabFromHwnd(HWND hwnd) const
{
	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [i];
		ASSERT_VALID(pTab);

		if (pTab->m_pWnd == NULL)
		{
			continue;
		}

		if (pTab->m_pWnd->GetSafeHwnd() == hwnd)
		{
			return i;
		}

		CDockablePaneAdapter* pWrapper = DYNAMIC_DOWNCAST(CDockablePaneAdapter, pTab->m_pWnd);
		if (pWrapper != NULL)
		{
			CWnd* pWrappedWnd = pWrapper->GetWrappedWnd();
			if (pWrappedWnd != NULL && pWrappedWnd->GetSafeHwnd() == hwnd)
			{
				return i;
			}
		}
	}

	return -1;
}

int CMFCBaseTabCtrl::GetTabByID(int id) const
{
	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [i];
		ASSERT_VALID(pTab);

		if (pTab->m_iTabID == id)
		{
			return i;
		}
	}

	return -1;
}

BOOL CMFCBaseTabCtrl::IsTabVisible(int iTab) const
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		TRACE(_T("IsTabVisible: illegal tab number %d\n"), iTab);
		return FALSE;
	}

	const CMFCTabInfo* pTab = (const CMFCTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	return pTab->m_bVisible;
}

int CMFCBaseTabCtrl::GetTabNumberToDetach(int nTabNum) const
{
	return(nTabNum == -1 ? m_iActiveTab : nTabNum);
}

BOOL CMFCBaseTabCtrl::DetachTab(AFX_DOCK_METHOD dockMethod, int nTabNum, BOOL bHide)
{
	int nTabToDetach = GetTabNumberToDetach(nTabNum);
	if (nTabToDetach < 0)
	{
		return FALSE;
	}

	ASSERT(nTabToDetach >= 0 && nTabToDetach < GetTabsNum());

	if (!IsTabDetachable(nTabToDetach))
	{
		return FALSE;
	}

	CPane* pDockingBar = DYNAMIC_DOWNCAST(CPane, GetTabWnd(nTabToDetach));
	if (pDockingBar == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(pDockingBar);

	if (!pDockingBar->CanFloat())
	{
		return FALSE;
	}

	CRect rectFloat; rectFloat.SetRectEmpty();

	if (!pDockingBar->OnBeforeFloat(rectFloat, dockMethod))
	{
		return FALSE;
	}

	if (dockMethod == DM_MOUSE)
	{
		CPoint ptMouse;
		GetCursorPos(&ptMouse);
		CPoint ptHotDelta = m_ptHot - ptMouse;
		CSize szSensitivity = CDockablePane::GetDragSensitivity();

		if (abs(ptHotDelta.x) < szSensitivity.cx && abs(ptHotDelta.y) < szSensitivity.cy)
		{
			return FALSE;
		}
	}

	if (pDockingBar != NULL && pDockingBar->CanBeAttached())
	{
		AFX_DOCK_TYPE dockType = pDockingBar->GetDockingMode();

		CBaseTabbedPane* pParent = DYNAMIC_DOWNCAST(CBaseTabbedPane, GetParent());
		CPaneFrameWnd* pMiniFrame = pParent == NULL ? NULL : pParent->GetParentMiniFrame();

		if (pParent != NULL)
		{
			if (!pParent->CanFloat() && GetVisibleTabsNum() == 1)
			{
				return FALSE;
			}

			if (pDockingBar->IsKindOf(RUNTIME_CLASS(CDockablePane)))
			{
				((CDockablePane*) pDockingBar)->EnableGripper(TRUE);
			}

			if (!bHide &&(dockType & DT_STANDARD) != 0 && dockMethod == DM_MOUSE)
			{
				bHide = TRUE;
			}

			if (/*!bHide && */nTabToDetach != m_iActiveTab)
			{
				pDockingBar->ShowWindow(SW_SHOW);
			}

			pParent->FloatTab(pDockingBar, nTabToDetach, dockMethod, bHide);

			if (GetTabsNum() == 0)
			{
				if (pMiniFrame != NULL)
				{
					pMiniFrame->RemovePane(pParent, TRUE);
				}
				else
				{
					if (pParent->AllowDestroyEmptyTabbedPane())
					{
						pParent->DestroyWindow();
					}
					else
					{
						ShowWindow(SW_HIDE);
					}
				}
			}
			else if (GetVisibleTabsNum() == 0)
			{
				pParent->ShowPane(FALSE, FALSE, FALSE);
			}
			RecalcLayout();
		}

		CPaneFrameWnd* pParentFrame = pDockingBar->GetParentMiniFrame();
		if ((dockType & DT_STANDARD) != 0 && dockMethod == DM_MOUSE)
		{
			if (pParentFrame != NULL)
			{
				ReleaseCapture();
				pParentFrame->SetFocus();
				GetParent()->SendMessage(WM_IDLEUPDATECMDUI);
			}
		}

		m_bReadyToDetach = FALSE;

		HWND hWndToDestroy = NULL;
		if (GetTabsNum() == 1 && m_bHideSingleTab)
		{
			CRect rectWnd;
			pParent->GetWindowRect(rectWnd);

			CDockablePane* pTabDockingBar = DYNAMIC_DOWNCAST(CDockablePane, GetTabWnd(0));

			pParent->InsertPane(pTabDockingBar, pParent);
			pParent->ReplacePane(pTabDockingBar, dockMethod);
			pTabDockingBar->SetPaneAlignment(pParent->GetCurrentAlignment());
			RemoveTab(0);
			CWnd* pNewParent = (pMiniFrame == NULL) ? pParent->GetDockSiteFrameWnd() : pMiniFrame;

			pTabDockingBar->EnableGripper(TRUE);
			pNewParent->ScreenToClient(rectWnd);

			pTabDockingBar->SetParent(pNewParent);
			pTabDockingBar->SetWindowPos(NULL, rectWnd.left, rectWnd.top, rectWnd.Width(), rectWnd.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_HIDEWINDOW);

			pTabDockingBar->ShowPane(TRUE, FALSE, FALSE);
			pParent->ShowWindow(SW_HIDE);
			pTabDockingBar->GetParent()->InvalidateRect(NULL);
			pTabDockingBar->GetParent()->UpdateWindow();
			hWndToDestroy = pParent->GetSafeHwnd();
		}

		if (dockMethod == DM_MOUSE && pParentFrame != NULL)
		{
			ASSERT_VALID(pParentFrame);

			if ((dockType & DT_STANDARD) != 0)
			{
				pParentFrame->EnterDragMode(hWndToDestroy);
				pParentFrame->MoveDragFrame();
			}
			else if ((dockType & DT_IMMEDIATE) != 0)
			{
				pParentFrame->SetFocus();
			}
		}

		return TRUE;
	}

	return FALSE;
}

void CMFCBaseTabCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bWindowPosChanged = FALSE;

	CWnd::OnLButtonDown(nFlags, point);

	if (m_rectCloseButton.PtInRect(point))
	{
		m_bTabCloseButtonPressed = TRUE;
		RedrawWindow(m_rectCloseButton);
		return;
	}

	m_iHighlighted = GetTabFromPoint(point);
	int iHighlighted = m_iHighlighted;

	if (iHighlighted >= 0)
	{
		BOOL bTabWasMoved = FALSE;

		if (!ActivateOnBtnUp())
		{
			int nTab = GetTabFromPoint(point);

			if (nTab >= 0 && nTab != m_iActiveTab)
			{
				m_iLastActiveTab = m_iActiveTab;

				m_bSetActiveTabFired = FALSE;
				m_bSetActiveTabByMouseClick = TRUE;
				m_bUserSelectedTab = TRUE;

				if (!SetActiveTab(nTab))
				{
					m_bSetActiveTabFired = TRUE;
					m_bSetActiveTabByMouseClick = FALSE;
					m_bUserSelectedTab = FALSE;
					m_bWindowPosChanged = FALSE;
					return;
				}

				m_bSetActiveTabByMouseClick = FALSE;
				m_bUserSelectedTab = FALSE;

				if (!m_bSetActiveTabFired)
				{
					FireChangeActiveTab(m_iActiveTab);
				}

				m_bSetActiveTabFired = FALSE;
			}
			else if (nTab == m_iActiveTab)
			{
				//-----------------------------
				// Set focus to the tab window:
				//-----------------------------
				CWnd* pWnd = GetTabWnd(nTab);
				if (pWnd->GetSafeHwnd() != NULL)
				{
					pWnd->SetFocus();
				}
			}

			bTabWasMoved = (nTab != GetTabFromPoint(point));

			m_iHighlighted = -1;
			ReleaseCapture();
		}
		else
		{
			m_iPressed = m_iHighlighted;
		}

		if (iHighlighted != m_iActiveTab)
		{
			InvalidateTab(iHighlighted);
		}

		if (!bTabWasMoved && !m_bWindowPosChanged)
		{
			EnterDragMode();
		}
	}
	m_bWindowPosChanged = FALSE;
}

void CMFCBaseTabCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	CWnd::OnRButtonDown(nFlags, point);

	if (m_bActivateTabOnRightClick)
	{
		int nTab = GetTabFromPoint(point);
		if (nTab >= 0 && nTab != m_iActiveTab)
		{
			m_bSetActiveTabFired = FALSE;
			m_bSetActiveTabByMouseClick = TRUE;
			m_bUserSelectedTab = FALSE;

			if (!SetActiveTab(nTab))
			{
				m_bSetActiveTabByMouseClick = FALSE;
				m_bUserSelectedTab = FALSE;
				return;
			}

			m_bSetActiveTabByMouseClick = FALSE;
			m_bUserSelectedTab = FALSE;

			if (!m_bSetActiveTabFired)
			{
				FireChangeActiveTab(m_iActiveTab);
			}

			m_bSetActiveTabFired = FALSE;
		}
		else if (nTab == m_iActiveTab)
		{
			//-----------------------------
			// Set focus to the tab window:
			//-----------------------------
			CWnd* pWnd = GetTabWnd(nTab);
			if (pWnd->GetSafeHwnd() != NULL)
			{
				pWnd->SetFocus();
			}
		}
	}
}

void CMFCBaseTabCtrl::EnterDragMode()
{
	m_bReadyToDetach = TRUE;
	SetCapture();
	GetCursorPos(&m_ptHot);

	CRect rectTab;
	GetTabRect(m_iActiveTab, rectTab);
	m_iTabBeforeDrag = m_iActiveTab;
	ClientToScreen(rectTab);
	m_nOffsetFromTabLeft = m_ptHot.x - rectTab.left;
}

void CMFCBaseTabCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CWnd::OnLButtonDblClk(nFlags, point);

	if (m_bIsInPlaceEdit)
	{
		int iTab = GetTabFromPoint(point);
		if (iTab  == GetActiveTab())
		{
			if (StartRenameTab(iTab))
				return;
		}
	}
	else
	{
		CWnd* pWndTarget = FindTargetWnd(point);
		if (pWndTarget == NULL)
		{
			DetachTab(DM_DBL_CLICK);
		}
	}
}

void CMFCBaseTabCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bTabCloseButtonPressed)
	{
		m_bTabCloseButtonPressed = FALSE;
		m_bTabCloseButtonHighlighted = FALSE;

		RedrawWindow(m_rectCloseButton);

		if (m_rectCloseButton.PtInRect(point))
		{
			CWnd* pWndActive = GetActiveWnd();
			if (pWndActive != NULL)
			{
				pWndActive->SendMessage(WM_CLOSE);
			}

			return;
		}
	}

	if (m_iTabBeforeDrag != m_iActiveTab)
	{
		CWnd* pWndParent = GetParent();
		ASSERT_VALID(pWndParent);

		pWndParent->SendMessage(AFX_WM_ON_MOVE_TAB, m_iTabBeforeDrag, m_iActiveTab);
		if (pWndParent->IsKindOf(RUNTIME_CLASS(CBaseTabbedPane)) || pWndParent->IsKindOf(RUNTIME_CLASS(CMDIClientAreaWnd)))
		{
			pWndParent = AFXGetParentFrame(pWndParent);
			if (pWndParent != NULL)
			{
				pWndParent->SendMessage(AFX_WM_ON_MOVE_TAB, m_iTabBeforeDrag, m_iActiveTab);
			}
		}
	}

	if (m_bReadyToDetach)
	{
		m_bReadyToDetach = FALSE;
		ReleaseCapture();

		if (!ActivateOnBtnUp())
		{
			m_iPressed = -1;
			m_iHighlighted = -1;
		}
	}

	if (ActivateOnBtnUp())
	{
		bool bNewActiveTab = m_iActiveTab != m_iHighlighted;

		if (m_iHighlighted == m_iPressed && m_iHighlighted >= 0 && m_iHighlighted != m_iActiveTab)
		{
			m_iLastActiveTab = m_iActiveTab;
			m_bSetActiveTabByMouseClick = TRUE;
			m_bUserSelectedTab = FALSE;

			if (!SetActiveTab(m_iHighlighted))
			{
				m_bSetActiveTabByMouseClick = FALSE;
				m_bUserSelectedTab = FALSE;
				m_iPressed = -1;

				if (!IsOneNoteStyle())
				{
					m_iHighlighted = -1;
				}

				ReleaseCapture();
				return;
			}

			FireChangeActiveTab(m_iActiveTab);
			m_bSetActiveTabByMouseClick = FALSE;
			m_bUserSelectedTab = FALSE;
		}

		int iHighlighted = m_iHighlighted;
		int iPressed = m_iPressed;

		m_iPressed = -1;

		if (!IsOneNoteStyle())
		{
			m_iHighlighted = -1;
		}

		ReleaseCapture();

		if (bNewActiveTab)
		{
			InvalidateTab(iHighlighted);

			if (iPressed != iHighlighted)
			{
				InvalidateTab(iPressed);
			}
		}
	}

	if (IsOneNoteStyle())
	{
		CRect rectTabAreaTop;
		CRect rectTabAreaBottom;

		GetTabArea(rectTabAreaTop, rectTabAreaBottom);

		if (!rectTabAreaTop.IsRectEmpty())
		{
			InvalidateRect(rectTabAreaTop, FALSE);
		}

		if (!rectTabAreaBottom.IsRectEmpty())
		{
			InvalidateRect(rectTabAreaBottom, FALSE);
		}

		UpdateWindow();
	}

	CWnd::OnLButtonUp(nFlags, point);
}

void CMFCBaseTabCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	CWnd::OnMouseMove(nFlags, point);

	int iPrevHighlighted = m_iHighlighted;

	BOOL bTabCloseButtonHighlighted = m_bTabCloseButtonHighlighted;
	m_bTabCloseButtonHighlighted = m_rectCloseButton.PtInRect(point);

	if (bTabCloseButtonHighlighted != m_bTabCloseButtonHighlighted)
	{
		if (m_bTabCloseButtonHighlighted)
		{
			m_bTabCloseButtonPressed = (nFlags & MK_LBUTTON);
		}

		RedrawWindow(m_rectCloseButton);
	}

	if (m_iHighlighted >= 0 && m_iPressed < 0 && !m_bReadyToDetach)
	{
		CPoint pt = point;
		ClientToScreen(&pt);
		CWnd* pWnd = CWnd::WindowFromPoint(pt);
		if (pWnd != NULL && pWnd->GetSafeHwnd() != GetSafeHwnd())
		{
			ReleaseCapture();
			m_iHighlighted = -1;
			InvalidateTab(iPrevHighlighted);
			return;
		}
	}

	m_iHighlighted = GetTabFromPoint(point);

	if (m_iPressed >= 0 && m_iHighlighted != m_iPressed)
	{
		m_iHighlighted = -1;
	}

	if (m_iHighlighted != iPrevHighlighted &&(m_bHighLightTabs || IsActiveTabCloseButton()))
	{
		if (iPrevHighlighted < 0)
		{
			if (m_iHighlighted >= 0)
			{
				SetCapture();
			}
		}
		else
		{
			if (m_iHighlighted < 0 && m_iPressed < 0)
			{
				m_bTabCloseButtonHighlighted = FALSE;
				m_bTabCloseButtonPressed = FALSE;

				if (!m_bReadyToDetach)
				{
					ReleaseCapture();
				}
			}
		}

		InvalidateTab(m_iHighlighted);
		InvalidateTab(iPrevHighlighted);
	}

	if (m_bReadyToDetach)
	{
		int nNumTabs = m_iTabsNum; // how many tabs before detch

		// try to rearrange tabs if their number > 1
		if (IsPtInTabArea(point) && nNumTabs > 1 && m_bEnableTabSwap)
		{
			CRect rectTab;
			int nTabNum = GetTabFromPoint(point);

			if (nTabNum != m_iActiveTab && nTabNum != -1)
			{
				int nSecondTab = m_iActiveTab;
				SwapTabs(nTabNum, nSecondTab);
				RecalcLayout();
				SetActiveTab(nTabNum);
				int nCurrTabNum = GetTabFromPoint(point);
				if (nCurrTabNum != nTabNum)
				{
					GetTabRect(nTabNum, rectTab);
					CPoint ptCursorNewPos = point;

					ptCursorNewPos.x = rectTab.left + m_nOffsetFromTabLeft;

					ClientToScreen(&ptCursorNewPos);
					SetCursorPos(ptCursorNewPos.x, ptCursorNewPos.y);
				}
			}
			return;
		}

		if (IsPtInTabArea(point))
		{
			return;
		}

		BOOL bDetachSucceeded = DetachTab(DM_MOUSE);

		if (bDetachSucceeded && nNumTabs <= 2)
		{
			// last tab was detached successfully - run out, because the control
			// has been destroyed
			return;
		}

		if (bDetachSucceeded)
		{
			m_bReadyToDetach = FALSE;
		}

		return;
	}
}

void CMFCBaseTabCtrl::OnCancelMode()
{
	CWnd::OnCancelMode();

	if (m_bTabCloseButtonPressed || m_bTabCloseButtonHighlighted)
	{
		m_bTabCloseButtonPressed = FALSE;
		m_bTabCloseButtonHighlighted = FALSE;

		RedrawWindow(m_rectCloseButton);
	}

	if (m_iHighlighted >= 0)
	{
		int iTab = m_iHighlighted;

		ReleaseCapture();
		m_iHighlighted = -1;
		m_iPressed = -1;

		InvalidateTab(iTab);
	}

	if (m_pInPlaceEdit != NULL)
	{
		m_pInPlaceEdit->DestroyWindow();
		delete m_pInPlaceEdit;
		m_pInPlaceEdit = NULL;
		m_iEditedTab = -1;
		ReleaseCapture();
	}

	m_bReadyToDetach = FALSE;
}

void CMFCBaseTabCtrl::InvalidateTab(int iTab)
{
	ASSERT_VALID(this);

	if (iTab < 0)
	{
		return;
	}

	CRect rectTab;

	if (GetTabRect(iTab, rectTab))
	{
		InvalidateRect(rectTab);
		UpdateWindow();
	}
}

void CMFCBaseTabCtrl::OnDestroy()
{
	CleanUp();
	CWnd::OnDestroy();
}

int CMFCBaseTabCtrl::FindTabInfo(int nBarID, CMFCTabInfo** ppTabInfo)
{
	*ppTabInfo = NULL;
	for (int i = 0; i < m_arTabs.GetSize(); i++)
	{
		CMFCTabInfo* pTabInfo = (CMFCTabInfo*) m_arTabs.GetAt(i);
		ASSERT_VALID(pTabInfo);

		if (pTabInfo->m_pWnd != NULL && pTabInfo->m_pWnd->GetDlgCtrlID() == nBarID)
		{
			*ppTabInfo = pTabInfo;
			return i;
		}
	}

	return NULL;
}

void CMFCBaseTabCtrl::ApplyRestoredTabInfo(BOOL bUseTabIndexes)
{
	CDockingManager* pDockManager = afxGlobalUtils.GetDockingManager(AFXGetParentFrame(this));
	ASSERT_VALID(pDockManager);

	POSITION pos = NULL;

	for (pos = m_lstRestoredTabInfo.GetHeadPosition(); pos != NULL;)
	{
		CMFCRestoredTabInfo tabInfo = m_lstRestoredTabInfo.GetNext(pos);

		CMFCTabInfo* pCurrTabInfo = NULL;
		FindTabInfo(tabInfo.m_nControlBarID, &pCurrTabInfo);
		if (pCurrTabInfo != NULL)
		{
			continue;
		}

		CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, pDockManager->FindPaneByID(tabInfo.m_nControlBarID, TRUE));

		if (pBar != NULL)
		{
			if (pBar->IsTabbed())
			{
				CMFCBaseTabCtrl* pTabWnd = (CMFCBaseTabCtrl*) pBar->GetParent();
				CBaseTabbedPane* pTabBar = (CBaseTabbedPane*) pTabWnd->GetParent();
				ASSERT_VALID(pTabBar);
				pBar->SetParent(GetParent());
				pTabBar->RemovePane(pBar);
				if (pBar->IsKindOf(RUNTIME_CLASS(CDockablePane)))
				{
					((CDockablePane*) pBar)->EnableGripper(TRUE);
				}

				pBar->ShowWindow(SW_SHOW);
			}

			if (pBar->IsAutoHideMode())
			{
				pBar->SetAutoHideMode(FALSE, CBRS_ALIGN_ANY);
			}

			CPaneFrameWnd* pMiniFrame = pBar->GetParentMiniFrame();
			if (pMiniFrame != NULL)
			{
				pMiniFrame->RemovePane(pBar);
			}

			pBar->SetParent(GetParent());

			CBaseTabbedPane* pTabbedBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, GetParent());
			pBar->AttachToTabWnd(pTabbedBar, DM_SHOW, FALSE);
		}
	}

	int nTabIdx = 0;
	int nVisibleTabNumber = 0;

	for (pos = m_lstRestoredTabInfo.GetHeadPosition(); pos != NULL; nTabIdx++)
	{
		CMFCRestoredTabInfo tabInfo = m_lstRestoredTabInfo.GetNext(pos);
		CMFCTabInfo* pCurrTabInfo = NULL;
		int nCurIdx = FindTabInfo(tabInfo.m_nControlBarID, &pCurrTabInfo);

		if (pCurrTabInfo == NULL)
		{
			continue;
		}

		pCurrTabInfo->m_strText  = tabInfo.m_strText;
		pCurrTabInfo->m_clrText  = tabInfo.m_clrText;
		pCurrTabInfo->m_clrBack  = tabInfo.m_clrBack;
		pCurrTabInfo->m_bIsDetachable = tabInfo.m_bDetachable;

		ShowTab(nCurIdx, tabInfo.m_bVisible, FALSE);

		if (tabInfo.m_bVisible)
		{
			nVisibleTabNumber++;
		}

		if (bUseTabIndexes && nCurIdx != nTabIdx)
		{
			SwapTabs(nCurIdx, nTabIdx);
			if (pCurrTabInfo->m_pWnd != NULL && nTabIdx == m_nRestoredActiveTabID)
			{
				pCurrTabInfo->m_pWnd->ShowWindow(SW_SHOW);
			}
		}
	}

	if (nVisibleTabNumber > 0 && !SetActiveTab(m_nRestoredActiveTabID))
	{
		SetActiveTab(0);
	}

	if (nVisibleTabNumber == 0)
	{
		CBaseTabbedPane* pTabBar = DYNAMIC_DOWNCAST(CBaseTabbedPane, GetParent());
		if (pTabBar != NULL)
		{
			pTabBar->ShowPane(FALSE, FALSE, FALSE);
		}
	}

	RecalcLayout();
}

void CMFCBaseTabCtrl::Serialize(CArchive& ar)
{
	int nTabCount = 0;
	if (ar.IsLoading())
	{
		m_lstRestoredTabInfo.RemoveAll();

		ar >> nTabCount;

		for (int i = 0; i < nTabCount; i++)
		{
			CMFCRestoredTabInfo tabInfo;

			ar >> tabInfo.m_strText;
			ar >> tabInfo.m_bVisible;
			ar >> tabInfo.m_nControlBarID;
			ar >> tabInfo.m_bDetachable;
			ar >> tabInfo.m_clrText;
			ar >> tabInfo.m_clrBack;

			m_lstRestoredTabInfo.AddTail(tabInfo);
		}
		ar >> m_nRestoredActiveTabID;

		int nLoc = 0;
		ar >> nLoc;
	}
	else
	{
		nTabCount = (int) m_arTabs.GetSize();
		ar << nTabCount;

		for (int i = 0; i < nTabCount; i++)
		{
			CMFCTabInfo* pTabInfo = (CMFCTabInfo*) m_arTabs.GetAt(i);
			ASSERT_VALID(pTabInfo);

			ar << pTabInfo->m_strText;
			ar << pTabInfo->m_bVisible;

			ASSERT_VALID(pTabInfo->m_pWnd);
			ar << pTabInfo->m_pWnd->GetDlgCtrlID();

			ar << pTabInfo->m_bIsDetachable;
			ar << pTabInfo->m_clrText;
			ar << pTabInfo->m_clrBack;
		}

		ar << m_iActiveTab;
		ar << m_location;
	}
}

void CMFCBaseTabCtrl::SwapTabs(int nFisrtTabID, int nSecondTabID)
{
	if (m_arTabs.GetSize() <= nFisrtTabID ||
		m_arTabs.GetSize() <= nSecondTabID)
	{
		return;
	}
	CMFCTabInfo* pTabInfoFirst  = (CMFCTabInfo*) m_arTabs.GetAt(nFisrtTabID);
	CMFCTabInfo* pTabInfoSecond = (CMFCTabInfo*) m_arTabs.GetAt(nSecondTabID);
	m_arTabs.SetAt(nFisrtTabID, pTabInfoSecond);
	m_arTabs.SetAt(nSecondTabID, pTabInfoFirst);
}

void CMFCBaseTabCtrl::MoveTab(int nSource, int nDest)
{
	ASSERT_VALID(this);

	if (nSource == nDest)
	{
		return;
	}

	CMFCTabInfo* pSource = (CMFCTabInfo*) m_arTabs [nSource];
	CMFCTabInfo* pActive = (CMFCTabInfo*) m_arTabs [m_iActiveTab];

	ASSERT(nDest < m_arTabs.GetSize());

	if (nDest == -1)
	{
		m_arTabs.Add(pSource);
		m_arTabs.RemoveAt(nSource);
	}
	else
	{
		m_arTabs.RemoveAt(nSource);
		m_arTabs.InsertAt(nDest, pSource);
	}

	for (int iTab = 0; iTab < m_arTabs.GetSize(); iTab++)
	{
		if (pActive == (CMFCTabInfo*) m_arTabs [iTab])
		{
			if (iTab != m_iActiveTab)
			{
				SetActiveTab(iTab);
				FireChangeActiveTab(m_iActiveTab);
			}
			break;
		}
	}
	RecalcLayout();
}

BOOL CMFCBaseTabCtrl::SetTabsOrder(const CArray<int,int>& arOrder)
{
	ASSERT_VALID(this);

	if (arOrder.GetSize() != m_iTabsNum)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CPtrArray arTabsSaved;
	int i = 0;

	for (i = 0; i < m_iTabsNum; i ++)
	{
		CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [i];
		ASSERT_VALID(pTab);

		arTabsSaved.Add(pTab);
	}

	m_arTabs.RemoveAll();

	BOOL bActiveTabIsChamged = FALSE;

	for (i = 0; i < m_iTabsNum; i ++)
	{
		int nIndex = arOrder [i];
		ASSERT(nIndex >= 0);
		ASSERT(nIndex < m_iTabsNum);

		CMFCTabInfo* pTab = (CMFCTabInfo*) arTabsSaved [nIndex];
		ASSERT_VALID(pTab);

		m_arTabs.Add(pTab);

		if (nIndex == m_iActiveTab && !bActiveTabIsChamged)
		{
			m_iActiveTab = i;
			bActiveTabIsChamged = TRUE;
		}
	}

	return TRUE;
}

CWnd* CMFCBaseTabCtrl::CreateWrapper(CWnd* pWndToWrap, LPCTSTR lpszTabLabel, BOOL bDetachable)
{
	ASSERT_VALID(pWndToWrap);

	if (pWndToWrap->IsKindOf(RUNTIME_CLASS(CDockablePane)) || !bDetachable || !m_bEnableWrapping)
	{
		return pWndToWrap;
	}


	CDockablePaneAdapter* pWrapper = NULL;

	if (m_pDockingBarWrapperRTC != NULL)
	{
		pWrapper = DYNAMIC_DOWNCAST(CDockablePaneAdapter, m_pDockingBarWrapperRTC->CreateObject());
	}
	else
	{
		pWrapper = new CDockablePaneAdapter;
	}

	ASSERT_VALID(pWrapper);

	CRect rectInit(pWrapper->m_rectInitial);

	// create will alter the runtime class, but here we have to save it
	// and set back after create
	CRuntimeClass* pSaveRTC = pWrapper->GetTabbedPaneRTC();

	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	DWORD dwControlBarStyle = AFX_DEFAULT_DOCKING_PANE_STYLE;
	if (pWndToWrap->IsKindOf(RUNTIME_CLASS(CBasePane)))
	{
		dwStyle |= ((CBasePane*) pWndToWrap)->GetPaneStyle();
		dwControlBarStyle = ((CBasePane*) pWndToWrap)->GetControlBarStyle();
	}

	if (!pWrapper->Create(lpszTabLabel, this, rectInit, TRUE, (UINT) pWndToWrap->GetDlgCtrlID(), dwStyle, AFX_CBRS_REGULAR_TABS, dwControlBarStyle))
	{
		delete pWrapper;
		TRACE0("Unable to create CDockablePaneAdapter. \n");
		return pWndToWrap;
	}
	pWrapper->EnableGripper(FALSE);
	pWrapper->SetWrappedWnd(pWndToWrap);

	pWrapper->m_recentDockInfo.m_rectRecentFloatingRect = rectInit;

	if (pSaveRTC != NULL)
	{
		pWrapper->SetTabbedPaneRTC(pSaveRTC);
	}

	return pWrapper;
}

BOOL CMFCBaseTabCtrl::StartRenameTab(int iTab)
{
	ASSERT_VALID(this);
	ENSURE(GetSafeHwnd() != NULL);

	if (!m_bIsInPlaceEdit)
	{
		return FALSE;
	}

	if (iTab < 0 || iTab >= m_arTabs.GetSize())
	{
		return FALSE;
	}

	CMFCTabInfo* pTabInfo = (CMFCTabInfo*) m_arTabs.GetAt(iTab);
	if (pTabInfo->m_rect.IsRectEmpty())
	{
		return FALSE;
	}

	ENSURE(m_pInPlaceEdit == NULL);

	m_pInPlaceEdit = new CEdit;
	ASSERT_VALID(m_pInPlaceEdit);

	CRect rectEdit = pTabInfo->m_rect;
	CalcRectEdit(rectEdit);

	if (!m_pInPlaceEdit->Create(WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, rectEdit, this, 1))
	{
		delete m_pInPlaceEdit;
		m_pInPlaceEdit = NULL;

		return FALSE;
	}

	m_pInPlaceEdit->SetWindowText(pTabInfo->m_strText);
	m_pInPlaceEdit->SetFont(&afxGlobalData.fontRegular);
	m_pInPlaceEdit->SetSel(0, -1, TRUE);

	m_pInPlaceEdit->SetFocus();
	m_iEditedTab = iTab;

	SetCapture();
	return TRUE;
}

BOOL CMFCBaseTabCtrl::PreTranslateMessage(MSG* pMsg)
{
	if (m_pInPlaceEdit != NULL)
	{
		if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
		{
			switch(pMsg->wParam)
			{
			case VK_RETURN:
				if (!RenameTab())
				{
					MessageBeep((UINT)-1);
					return TRUE;
				}
				// Slide down!

			case VK_ESCAPE:
				m_pInPlaceEdit->DestroyWindow();
				delete m_pInPlaceEdit;
				m_pInPlaceEdit = NULL;
				m_iEditedTab = -1;
				ReleaseCapture();
				break;

			default:
				return FALSE;
			}

			return TRUE;
		}
		else if (pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST)
		{
			CRect rectEdit;
			m_pInPlaceEdit->GetClientRect(rectEdit);
			m_pInPlaceEdit->MapWindowPoints(this, rectEdit);

			CPoint ptCursor;
			::GetCursorPos(&ptCursor);
			ScreenToClient(&ptCursor);

			if (rectEdit.PtInRect(ptCursor))
			{
				m_pInPlaceEdit->SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
			}
			else if (pMsg->message != WM_MOUSEMOVE)
			{
				m_pInPlaceEdit->DestroyWindow();
				delete m_pInPlaceEdit;
				m_pInPlaceEdit = NULL;
				m_iEditedTab = -1;
				ReleaseCapture();
			}

			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	return CWnd::PreTranslateMessage(pMsg);
}

BOOL CMFCBaseTabCtrl::RenameTab()
{
	ASSERT_VALID(this);
	ASSERT(m_bIsInPlaceEdit);

	if (m_pInPlaceEdit == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CString strName;
	m_pInPlaceEdit->GetWindowText(strName);

	if (!strName.IsEmpty() && OnRenameTab(m_iEditedTab, strName) &&
		GetParent()->SendMessage(AFX_WM_ON_RENAME_TAB, m_iEditedTab, (LPARAM)(LPCTSTR) strName) == 0)
	{
		return SetTabLabel(m_iEditedTab, strName);
	}

	return FALSE;
}

void CMFCBaseTabCtrl::SetActiveTabTextColor(COLORREF clr)
{
	m_clrActiveTabFg = clr;
}

void CMFCBaseTabCtrl::SetActiveTabColor(COLORREF clr)
{
	m_clrActiveTabBk = clr;

	if (m_brActiveTab.GetSafeHandle() != NULL)
	{
		m_brActiveTab.DeleteObject();
	}

	m_brActiveTab.CreateSolidBrush(GetActiveTabColor());
}

COLORREF CMFCBaseTabCtrl::GetTabBkColor(int iTab) const
{
	ASSERT_VALID(this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return(COLORREF)-1;
	}

	CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	COLORREF color = pTab->m_clrBack;

	if (color == (COLORREF)-1 && m_bIsAutoColor)
	{
		color = m_arAutoColors [iTab % m_arAutoColors.GetSize()];
		pTab->m_clrBack = color;
	}

	return color;
}

BOOL CMFCBaseTabCtrl::SetTabBkColor(int iTab, COLORREF color)
{
	ASSERT_VALID(this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}

	CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	pTab->m_clrBack = color;
	return TRUE;
}

COLORREF CMFCBaseTabCtrl::GetTabTextColor(int iTab) const
{
	ASSERT_VALID(this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return(COLORREF)-1;
	}

	CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	return pTab->m_clrText;
}

BOOL CMFCBaseTabCtrl::SetTabTextColor(int iTab, COLORREF color)
{
	ASSERT_VALID(this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}

	CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	pTab->m_clrText = color;
	return TRUE;
}

int CMFCBaseTabCtrl::GetTabFullWidth(int iTab) const
{
	ASSERT_VALID(this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return 0;
	}

	CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	return pTab->m_nFullWidth;
}

BOOL CMFCBaseTabCtrl::SetImageList(UINT uiID, int cx, COLORREF clrTransp)
{
	CBitmap bmp;
	if (!bmp.LoadBitmap(uiID))
	{
		TRACE(_T("CMFCTabCtrl::SetImageList Can't load bitmap: %x\n"), uiID);
		return FALSE;
	}

	if (m_Images.GetSafeHandle() != NULL)
	{
		m_Images.DeleteImageList();
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = (clrTransp == (COLORREF) -1) ? 0 : ILC_MASK;

	switch(bmpObj.bmBitsPixel)
	{
	case 4:
	default:
		nFlags |= ILC_COLOR4;
		break;

	case 8:
		nFlags |= ILC_COLOR8;
		break;

	case 16:
		nFlags |= ILC_COLOR16;
		break;

	case 24:
		nFlags |= ILC_COLOR24;
		break;

	case 32:
		nFlags |= ILC_COLOR32;
		break;
	}

	m_Images.Create(cx, bmpObj.bmHeight, nFlags, 0, 0);
	m_Images.Add(&bmp, clrTransp);

	m_sizeImage = CSize(cx, bmpObj.bmHeight);

	SetTabsHeight();
	return TRUE;
}

BOOL CMFCBaseTabCtrl::SetImageList(HIMAGELIST hImageList)
{
	ENSURE(hImageList != NULL);

	if (m_Images.GetSafeHandle() != NULL)
	{
		m_Images.DeleteImageList();
	}

	CImageList* pImageList = CImageList::FromHandle(hImageList);
	if (pImageList == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	IMAGEINFO info;
	pImageList->GetImageInfo(0, &info);

	CRect rectImage = info.rcImage;
	m_sizeImage = rectImage.Size();

	m_hImageList = hImageList;

	SetTabsHeight();
	return TRUE;
}

void CMFCBaseTabCtrl::ClearImageList()
{
	if (m_Images.GetSafeHandle() != NULL)
	{
		m_Images.DeleteImageList();
	}

	m_sizeImage = CSize(0, 0);
	SetTabsHeight();
}

void CMFCBaseTabCtrl::ResetImageList()
{
	ClearImageList();
	m_hImageList = NULL;
	m_mapAddedIcons.RemoveAll();
}

void CMFCBaseTabCtrl::SetTabsHeight()
{
	const int nImageHeight = m_sizeImage.cy <= 0 ? 0 : m_sizeImage.cy + 7;
	m_nTabsHeight = (max(nImageHeight, afxGlobalData.GetTextHeight() + 5));
}

void CMFCBaseTabCtrl::HideSingleTab(BOOL bHide)
{
	if (m_bHideSingleTab == bHide)
	{
		return;
	}

	m_bHideSingleTab = bHide;

	if (GetSafeHwnd() != NULL)
	{
		RecalcLayout();
	}
}

CWnd* CMFCBaseTabCtrl::GetFirstVisibleTab(int& iTabNum)
{
	return GetFirstVisibleTab(0, iTabNum);
}

CWnd* CMFCBaseTabCtrl::GetFirstVisibleTab(int iStartFrom, int& iTabNum)
{
	for (int i = iStartFrom; i < m_iTabsNum; i++)
	{
		CMFCTabInfo* pTabInfo = (CMFCTabInfo*) m_arTabs [i];
		ASSERT_VALID(pTabInfo);

		if (pTabInfo->m_bVisible)
		{
			iTabNum = i;
			return pTabInfo->m_pWnd;
		}
	}
	if (iStartFrom > 0)
	{
		return GetFirstVisibleTab(0, iTabNum);
	}
	iTabNum = -1;
	return NULL;
}

CWnd* CMFCBaseTabCtrl::GetLastVisibleTab(int& iTabNum)
{
	for (int i = m_iTabsNum - 1; i >= 0; i--)
	{
		CMFCTabInfo* pTabInfo = (CMFCTabInfo*) m_arTabs [i];
		ASSERT_VALID(pTabInfo);

		if (pTabInfo->m_bVisible)
		{
			iTabNum = i;
			return pTabInfo->m_pWnd;
		}
	}
	iTabNum = -1;
	return NULL;
}

void CMFCBaseTabCtrl::SetDrawNoPrefix(BOOL bNoPrefix, BOOL bRedraw)
{
	ASSERT_VALID(this);
	m_bLabelNoPrefix = bNoPrefix;

	if (bRedraw && GetSafeHwnd() != NULL)
	{
		RedrawWindow();
	}
}

int CMFCBaseTabCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (AFXGetParentFrame(this) == NULL)
	{
		m_bEnableWrapping = FALSE;
	}

	_AFX_THREAD_STATE* pState = AfxGetThreadState();
	if (pState->m_bNeedTerm) // AfxOleInit was called
	{
		m_DropTarget.Register(this);
	}

	return 0;
}

CSize CMFCBaseTabCtrl::GetMaxWindowSize() const
{
	CSize sizeMax(0, 0);
	int nTabCount = (int) m_arTabs.GetSize();

	for (int i = 0; i < nTabCount; i++)
	{
		CMFCTabInfo* pTabInfo = (CMFCTabInfo*) m_arTabs.GetAt(i);
		ASSERT_VALID(pTabInfo);

		if (pTabInfo->m_pWnd != NULL)
		{
			CSize sizeCurr(0, 0);
			if (pTabInfo->m_pWnd->IsKindOf(RUNTIME_CLASS(CBasePane)))
			{
				CBasePane* pBar = DYNAMIC_DOWNCAST(CBasePane, pTabInfo->m_pWnd);
				if (pBar != NULL)
				{
					sizeCurr = pBar->CalcFixedLayout(FALSE, TRUE);
				}
			}
			else
			{
				CRect rectWnd;
				pTabInfo->m_pWnd->GetWindowRect(rectWnd);
				sizeCurr = rectWnd.Size();
			}

			sizeMax.cx = max(sizeCurr.cx, sizeMax.cx);
			sizeMax.cy = max(sizeCurr.cy, sizeMax.cy);
		}
	}

	return sizeMax;
}

void CMFCBaseTabCtrl::EnableAutoColor(BOOL bEnable/* = TRUE*/)
{
	if (m_bIsAutoColor && !bEnable)
	{
		for (int i = 0; i < m_iTabsNum; i ++)
		{
			CMFCTabInfo* pTabInfo = (CMFCTabInfo*) m_arTabs.GetAt(i);
			ASSERT_VALID(pTabInfo);

			pTabInfo->m_clrBack = (COLORREF)-1;
		}
	}

	m_bIsAutoColor = bEnable;
	InitAutoColors();

	if (GetSafeHwnd() != NULL)
	{
		RedrawWindow();
	}
}

void CMFCBaseTabCtrl::InitAutoColors()
{
	if (!m_bIsDefaultAutoColor)
	{
		return;
	}

	m_arAutoColors.RemoveAll();

	if (afxGlobalData.m_nBitsPerPixel > 8)
	{
		m_arAutoColors.Add(RGB(197, 212, 242));
		m_arAutoColors.Add(RGB(255, 220, 120));
		m_arAutoColors.Add(RGB(190, 206, 161));
		m_arAutoColors.Add(RGB(240, 160, 161));
		m_arAutoColors.Add(RGB(188, 168, 225));
		m_arAutoColors.Add(RGB(156, 193, 182));
		m_arAutoColors.Add(RGB(247, 184, 134));
		m_arAutoColors.Add(RGB(217, 173, 194));
		m_arAutoColors.Add(RGB(165, 194, 215));
		m_arAutoColors.Add(RGB(179, 166, 190));
		m_arAutoColors.Add(RGB(234, 214, 163));
		m_arAutoColors.Add(RGB(246, 250, 125));
		m_arAutoColors.Add(RGB(181, 233, 157));
		m_arAutoColors.Add(RGB(95, 195, 207));
		m_arAutoColors.Add(RGB(193, 131, 131));
		m_arAutoColors.Add(RGB(202, 202, 213));
	}
	else
	{
		m_arAutoColors.Add(RGB(0, 255, 0));
		m_arAutoColors.Add(RGB(0, 255, 255));
		m_arAutoColors.Add(RGB(255, 0, 255));
		m_arAutoColors.Add(RGB(192, 192, 192));
		m_arAutoColors.Add(RGB(255, 255, 0));
	}
}

void CMFCBaseTabCtrl::OnSysColorChange()
{
	CWnd::OnSysColorChange();

	InitAutoColors();
}

void CMFCBaseTabCtrl::SetAutoColors(const CArray<COLORREF, COLORREF>& arColors)
{
	int i = 0;

	for (i = 0; i < m_iTabsNum; i ++)
	{
		CMFCTabInfo* pTabInfo = (CMFCTabInfo*) m_arTabs.GetAt(i);
		ASSERT_VALID(pTabInfo);

		pTabInfo->m_clrBack = (COLORREF)-1;
	}

	m_arAutoColors.RemoveAll();

	if (arColors.GetSize() == 0)
	{
		m_bIsDefaultAutoColor = TRUE;
		InitAutoColors();
	}
	else
	{
		m_bIsDefaultAutoColor = FALSE;

		for (i = 0; i < arColors.GetSize(); i++)
		{
			m_arAutoColors.Add(arColors [i]);
		}
	}

	if (GetSafeHwnd() != NULL)
	{
		RedrawWindow();
	}
}

void CMFCBaseTabCtrl::SetLocation(Location location)
{
	ASSERT_VALID(this);

	m_location = location;
	RecalcLayout();

	if (GetSafeHwnd() != NULL)
	{
		GetParent()->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
	}
}

BOOL CMFCBaseTabCtrl::IsColored() const
{
	for (int iTab = 0; iTab < m_iTabsNum; iTab++)
	{
		if (GetTabBkColor(iTab) != (COLORREF)-1)
		{
			return TRUE;
		}
	}

	return FALSE;
}

void CMFCBaseTabCtrl::FireChangeActiveTab(int nNewTab)
{
	CFrameWnd* pParentFrame = AFXGetTopLevelFrame(this);
	CWnd* pParent = GetParent();
	ASSERT_VALID(pParent);

	pParent->SendMessage(AFX_WM_CHANGE_ACTIVE_TAB, nNewTab, (LPARAM)this);
	if (pParent != pParentFrame && pParentFrame != NULL)
	{
		ASSERT_VALID(pParentFrame);
		pParentFrame->SendMessage(AFX_WM_CHANGE_ACTIVE_TAB, nNewTab, (LPARAM)this);
	}

	m_bSetActiveTabFired = TRUE;
}

BOOL CMFCBaseTabCtrl::FireChangingActiveTab(int nNewTab)
{
	CFrameWnd* pParentFrame = AFXGetTopLevelFrame(this);
	CWnd* pParent = GetParent();
	ASSERT_VALID(pParent);

	BOOL bRes = (BOOL) pParent->SendMessage(AFX_WM_CHANGING_ACTIVE_TAB, nNewTab, (LPARAM)this);
	if (bRes)
	{
		return TRUE;
	}

	if (pParent != pParentFrame && pParentFrame != NULL)
	{
		ASSERT_VALID(pParentFrame);
		bRes = (BOOL) pParentFrame->SendMessage(AFX_WM_CHANGING_ACTIVE_TAB, nNewTab, (LPARAM)this);
	}

	return bRes;
}

BOOL CMFCBaseTabCtrl::HasImage(int iTab) const
{
	ASSERT_VALID(this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}

	CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	if (pTab->m_hIcon != NULL)
	{
		return TRUE;
	}

	return GetImageList() != NULL && pTab->m_uiIcon != (UINT)-1;
}

void CMFCBaseTabCtrl::SetTabBorderSize(int nTabBorderSize, BOOL bRepaint/* = TRUE*/)
{
	if (nTabBorderSize == -1)
	{
		nTabBorderSize = AFX_DEFAULT_TAB_BORDER_SIZE;
	}

	if (m_nTabBorderSize != nTabBorderSize)
	{
		m_nTabBorderSize = nTabBorderSize;

		RecalcLayout();

		if (bRepaint && GetSafeHwnd() != NULL)
		{
			Invalidate();
			UpdateWindow();
		}
	}
}

BOOL CMFCBaseTabCtrl::EnableCustomToolTips(BOOL bEnable/* = TRUE*/)
{
	if (m_bCustomToolTips == bEnable)
	{
		return TRUE;
	}

	m_bCustomToolTips = bEnable;

	if (m_pToolTip->GetSafeHwnd() == NULL)
	{
		return TRUE;
	}

	for (int i = 0; i < m_iTabsNum; i++)
	{
		CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [i];
		ASSERT_VALID(pTab);

		if (m_bCustomToolTips)
		{
			CToolInfo info;

			m_pToolTip->GetToolInfo(info, this, pTab->m_iTabID);
			m_pToolTip->DelTool(this, pTab->m_iTabID);

			m_pToolTip->AddTool(this, LPSTR_TEXTCALLBACK, &info.rect, pTab->m_iTabID);
		}
		else
		{
			m_pToolTip->UpdateTipText(pTab->m_strText, this, pTab->m_iTabID);
		}
	}

	RecalcLayout();
	return TRUE;
}

BOOL CMFCBaseTabCtrl::OnNeedTipText(UINT /*id*/, NMHDR* pNMH, LRESULT* /*pResult*/)
{
	static CMFCTabToolTipInfo info;

	ENSURE(pNMH != NULL);

	LPNMTTDISPINFO pTTDispInfo = (LPNMTTDISPINFO) pNMH;
	ASSERT((pTTDispInfo->uFlags & TTF_IDISHWND) == 0);

	if (pNMH->hwndFrom == m_pToolTipClose->GetSafeHwnd())
	{
		ENSURE(info.m_strText.LoadString(IDS_AFXBARRES_CLOSEBAR));

		pTTDispInfo->lpszText = const_cast<LPTSTR>((LPCTSTR) info.m_strText);
		return TRUE;
	}

	if (m_pToolTip->GetSafeHwnd() == NULL || pNMH->hwndFrom != m_pToolTip->GetSafeHwnd())
	{
		return FALSE;
	}

	CFrameWnd* pParentFrame = AFXGetTopLevelFrame(this);
	CWnd* pParent = GetParent();
	ASSERT_VALID(pParent);

	info.m_pTabWnd = this;

	CPoint ptCursor;
	::GetCursorPos(&ptCursor);
	ScreenToClient(&ptCursor);

	if (m_rectCloseButton.PtInRect(ptCursor))
	{
		return FALSE;
	}

	info.m_nTabIndex = GetTabFromPoint(ptCursor);
	info.m_strText.Empty();

	pParent->SendMessage(AFX_WM_ON_GET_TAB_TOOLTIP, 0, (LPARAM) &info);
	if (pParent != pParentFrame && pParentFrame != NULL)
	{
		ASSERT_VALID(pParentFrame);
		pParentFrame->SendMessage(AFX_WM_ON_GET_TAB_TOOLTIP, 0, (LPARAM) &info);
	}

	if (info.m_strText.IsEmpty())
	{
		return FALSE;
	}

	pTTDispInfo->lpszText = const_cast<LPTSTR>((LPCTSTR) info.m_strText);
	return TRUE;
}

void CMFCBaseTabCtrl::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos)
{
	CWnd::OnWindowPosChanged(lpwndpos);

	m_bWindowPosChanged = TRUE;
}



