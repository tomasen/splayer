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
#include "afxmdiclientareawnd.h"
#include "afxmdiframewndex.h"
#include "afxmdichildwndex.h"
#include "afxmenubar.h"
#include "afxdockablepane.h"
#include "afxbasetabbedpane.h"
#include "afxvisualmanager.h"
#include "afxregpath.h"
#include "afxsettingsstore.h"
#include "afxribbonres.h"

#define AFX_REG_SECTION_FMT _T("%sMDIClientArea-%d")
#define AFX_REG_ENTRY_MDITABS_STATE _T("MDITabsState")

static const CString strMDIClientAreaProfile = _T("MDIClientArea");

UINT AFX_WM_ON_MOVETOTABGROUP = ::RegisterWindowMessage(_T("AFX_WM_ON_MOVETOTABGROUP"));

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CMDIClientAreaWnd, CWnd)

#define AFX_UM_UPDATE_TABS (WM_USER + 101)
#define AFX_RESIZE_MARGIN 40
#define AFX_NEW_GROUP_MARGIN 40

/////////////////////////////////////////////////////////////////////////////
// CMDITabInfo

CMDITabInfo::CMDITabInfo()
{
	m_tabLocation = CMFCTabCtrl::LOCATION_TOP;
	m_style = CMFCTabCtrl::STYLE_3D_SCROLLED;
	m_bTabCloseButton = TRUE;
	m_bTabCustomTooltips = FALSE;
	m_bTabIcons = FALSE;
	m_bAutoColor = FALSE;
	m_bDocumentMenu = FALSE;
	m_bEnableTabSwap = TRUE;
	m_nTabBorderSize = CMFCVisualManager::GetInstance()->GetMDITabsBordersSize();
	m_bFlatFrame = TRUE;
	m_bActiveTabCloseButton = FALSE;
}
void CMDITabInfo::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << m_tabLocation;
		ar << m_style;
		ar << m_bTabCloseButton;
		ar << m_bTabIcons;
		ar << m_bAutoColor;
		ar << m_bDocumentMenu;
		ar << m_bEnableTabSwap;
		ar << m_nTabBorderSize;
	}
	else
	{
		int nValue;
		ar >> nValue;
		m_tabLocation = (CMFCTabCtrl::Location) nValue;

		ar >> nValue;
		m_style = (CMFCTabCtrl::Style) nValue;

		ar >> m_bTabCloseButton;
		ar >> m_bTabIcons;
		ar >> m_bAutoColor;
		ar >> m_bDocumentMenu;
		ar >> m_bEnableTabSwap;
		ar >> m_nTabBorderSize;
	}
}
/////////////////////////////////////////////////////////////////////////////
// CMDIClientAreaWnd

CMDIClientAreaWnd::CMDIClientAreaWnd()
{
	m_bTabIsVisible = FALSE;
	m_bTabIsEnabled = FALSE;

	m_bIsMDITabbedGroup = FALSE;
	m_groupAlignment = GROUP_NO_ALIGN;
	m_nResizeMargin = AFX_RESIZE_MARGIN;
	m_nNewGroupMargin = AFX_NEW_GROUP_MARGIN;

	m_bDisableUpdateTabs = FALSE;

	m_rectNewTabGroup.SetRectEmpty();
	m_nTotalResizeRest = 0;
}

CMDIClientAreaWnd::~CMDIClientAreaWnd()
{
	while (!m_lstTabbedGroups.IsEmpty())
	{
		delete m_lstTabbedGroups.RemoveTail();
	}

	while (!m_lstRemovedTabbedGroups.IsEmpty())
	{
		CMFCTabCtrl* pWnd= DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstRemovedTabbedGroups.RemoveTail());
		if (pWnd != NULL)
		{
			delete pWnd;
		}
	}

	if (!m_mapTabIcons.IsEmpty())
	{
		for (POSITION pos = m_mapTabIcons.GetStartPosition(); pos != NULL;)
		{
			CWnd* pWnd = NULL;
			CImageList* pImageList = NULL;

			m_mapTabIcons.GetNextAssoc(pos, pWnd, pImageList);
			if (pImageList != NULL)
			{
				delete pImageList;
			}
		}

		m_mapTabIcons.RemoveAll();
	}
}

void CMDIClientAreaWnd::EnableMDITabs(BOOL bEnable, const CMDITabInfo& params)
{
	if (m_bIsMDITabbedGroup)
	{
		EnableMDITabbedGroups(FALSE, params);
	}

	m_bTabIsEnabled = bEnable;
	m_bTabIsVisible = bEnable;

	m_mdiTabParams = params;
	ApplyParams(&m_wndTab);

	if (bEnable)
	{
		UpdateTabs();
		if (!IsKeepClientEdge())
		{
			ModifyStyleEx(WS_EX_CLIENTEDGE, 0);
		}
	}
	else
	{
		if (!IsKeepClientEdge())
		{
			ModifyStyleEx(0, WS_EX_CLIENTEDGE);
		}
	}

	if (m_wndTab.GetSafeHwnd() != NULL)
	{
		m_wndTab.ShowWindow(SW_SHOW);
	}

	BringWindowToTop();

	if (GetSafeHwnd() != NULL && GetParentFrame() != NULL)
	{
		GetParentFrame()->RecalcLayout();

		UINT uiRedrawFlags = RDW_ALLCHILDREN | RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE;

		if (m_wndTab.GetSafeHwnd() != NULL)
		{
			m_wndTab.RedrawWindow(NULL, NULL, uiRedrawFlags);
		}

		RedrawWindow(NULL, NULL, uiRedrawFlags);
	}
}

void CMDIClientAreaWnd::EnableMDITabbedGroups(BOOL bEnable, const CMDITabInfo& mdiTabParams)
{
	if (m_bTabIsEnabled)
	{
		EnableMDITabs(FALSE, mdiTabParams);
	}

	m_wndTab.ShowWindow(SW_HIDE);

	HWND hwndActive = (HWND) SendMessage(WM_MDIGETACTIVE, 0, 0);

	if (m_bIsMDITabbedGroup != bEnable)
	{
		m_bIsMDITabbedGroup = bEnable;

		if (!bEnable)
		{
			for (POSITION pos = m_lstTabbedGroups.GetHeadPosition(); pos != 0;)
			{
				CMFCTabCtrl* pNextWnd = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetNext(pos));
				ASSERT_VALID(pNextWnd);

				pNextWnd->ShowWindow(SW_HIDE);

				for (int i = 0; i < pNextWnd->GetTabsNum(); i++)
				{
					CWnd* pNextChildWnd = pNextWnd->GetTabWnd(i);
					ASSERT_VALID(pNextChildWnd);
					pNextChildWnd->ModifyStyle(0, CMDIChildWndEx::m_dwExcludeStyle | WS_SYSMENU, SWP_NOZORDER | SWP_FRAMECHANGED);
				}
			}
		}
	}

	m_bTabIsVisible = bEnable;

	if (!m_bIsMDITabbedGroup)
	{
		if (!IsKeepClientEdge())
		{
			ModifyStyleEx(0, WS_EX_CLIENTEDGE);
		}

		if (afxGlobalData.bIsWindowsVista)
		{
			CWnd* pWndChild = GetWindow(GW_CHILD);
			CList<CMDIChildWndEx*, CMDIChildWndEx*> lst;

			while (pWndChild != NULL)
			{
				ASSERT_VALID(pWndChild);

				CMDIChildWndEx* pMDIChild = DYNAMIC_DOWNCAST(CMDIChildWndEx, pWndChild);
				if (pMDIChild != NULL && pMDIChild->CanShowOnMDITabs())
				{
					lst.AddTail(pMDIChild);
				}

				pWndChild = pWndChild->GetNextWindow();
			}

			m_bDisableUpdateTabs = TRUE;

			for (POSITION pos = lst.GetTailPosition(); pos != NULL;)
			{
				CMDIChildWndEx* pMDIChild = lst.GetPrev(pos);
				pMDIChild->SetWindowPos(NULL, -1, -1, -1, -1, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
			}

			m_bDisableUpdateTabs = FALSE;

			UpdateTabs();
		}

		return;
	}

	m_mdiTabParams = mdiTabParams;

	if (!IsKeepClientEdge())
	{
		ModifyStyleEx(WS_EX_CLIENTEDGE, 0);
	}

	POSITION pos = NULL;

	for (pos = m_lstTabbedGroups.GetHeadPosition(); pos != NULL;)
	{
		CMFCTabCtrl* pNextWnd = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetNext(pos));
		ASSERT_VALID(pNextWnd);
		pNextWnd->ShowWindow(SW_SHOWNA);
		ApplyParams(pNextWnd);
	}

	UpdateMDITabbedGroups(TRUE);

	for (pos = m_lstTabbedGroups.GetHeadPosition(); pos != NULL;)
	{
		CMFCTabCtrl* pNextWnd = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetNext(pos));
		ASSERT_VALID(pNextWnd);
		pNextWnd->RecalcLayout();
	}

	if (m_bIsMDITabbedGroup)
	{
		SetActiveTab(hwndActive);
	}
}

void CMDIClientAreaWnd::ApplyParams(CMFCTabCtrl* pTabWnd)
{
	ASSERT_VALID(pTabWnd);

	pTabWnd->ModifyTabStyle(m_mdiTabParams.m_style);
	pTabWnd->SetLocation(m_mdiTabParams.m_tabLocation);
	pTabWnd->m_bCloseBtn = m_mdiTabParams.m_bTabCloseButton;
	pTabWnd->m_bActiveTabCloseButton = m_mdiTabParams.m_bActiveTabCloseButton;
	pTabWnd->EnableTabDocumentsMenu(m_mdiTabParams.m_bDocumentMenu);
	pTabWnd->EnableAutoColor(m_mdiTabParams.m_bAutoColor);
	pTabWnd->EnableTabSwap(m_mdiTabParams.m_bEnableTabSwap);
	pTabWnd->SetTabBorderSize(m_mdiTabParams.m_nTabBorderSize);
	pTabWnd->EnableCustomToolTips(m_mdiTabParams.m_bTabCustomTooltips);

	pTabWnd->HideInactiveWindow(FALSE);
	pTabWnd->HideNoTabs();
	pTabWnd->AutoSizeWindow(FALSE);
	pTabWnd->AutoDestroyWindow(FALSE);
	pTabWnd->SetFlatFrame(m_mdiTabParams.m_bFlatFrame);
	pTabWnd->m_bTransparent = TRUE;
	pTabWnd->m_bTopEdge = TRUE;
	pTabWnd->SetDrawNoPrefix(TRUE, FALSE);
	pTabWnd->SetActiveTabBoldFont();
	pTabWnd->m_bActivateLastVisibleTab = TRUE;
	pTabWnd->m_bActivateTabOnRightClick = TRUE;

	pTabWnd->m_bIsMDITab = TRUE;
}

//{{AFX_MSG_MAP(CMDIClientAreaWnd)
BEGIN_MESSAGE_MAP(CMDIClientAreaWnd, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_STYLECHANGING()
	ON_MESSAGE(WM_MDISETMENU, &CMDIClientAreaWnd::OnSetMenu)
	ON_MESSAGE(WM_MDIREFRESHMENU, &CMDIClientAreaWnd::OnMDIRefreshMenu)
	ON_MESSAGE(WM_MDIDESTROY, &CMDIClientAreaWnd::OnMDIDestroy)
	ON_MESSAGE(WM_MDINEXT, &CMDIClientAreaWnd::OnMDINext)
	ON_MESSAGE(AFX_UM_UPDATE_TABS, &CMDIClientAreaWnd::OnUpdateTabs)
	ON_REGISTERED_MESSAGE(AFX_WM_GETDRAGBOUNDS, &CMDIClientAreaWnd::OnGetDragBounds)
	ON_REGISTERED_MESSAGE(AFX_WM_ON_DRAGCOMPLETE, &CMDIClientAreaWnd::OnDragComplete)
	ON_REGISTERED_MESSAGE(AFX_WM_ON_TABGROUPMOUSEMOVE, &CMDIClientAreaWnd::OnTabGroupMouseMove)
	ON_REGISTERED_MESSAGE(AFX_WM_ON_CANCELTABMOVE, &CMDIClientAreaWnd::OnCancelTabMove)
	ON_REGISTERED_MESSAGE(AFX_WM_ON_MOVETABCOMPLETE, &CMDIClientAreaWnd::OnMoveTabComplete)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// CMDIClientAreaWnd message handlers

afx_msg LRESULT CMDIClientAreaWnd::OnSetMenu(WPARAM wp, LPARAM lp)
{
	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetParentFrame());
	if (pMainFrame != NULL && ::IsWindow(pMainFrame->GetSafeHwnd()))
	{
		if (pMainFrame->OnSetMenu((HMENU) wp))
		{
			wp = NULL;
		}
	}
	else
	{
		wp = NULL;
	}

	return DefWindowProc(WM_MDISETMENU, wp, lp);
}

LRESULT CMDIClientAreaWnd::OnMDIRefreshMenu(WPARAM /*wp*/, LPARAM /*lp*/)
{
	LRESULT lRes = Default();

	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetParentFrame());
	if (pMainFrame != NULL && pMainFrame->GetMenuBar() != NULL)
	{
		pMainFrame->m_hmenuWindow = pMainFrame->GetWindowMenuPopup(pMainFrame->GetMenuBar()->GetHMenu());
	}

	return lRes;
}

BOOL CMDIClientAreaWnd::OnEraseBkgnd(CDC* pDC)
{
	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetParentFrame());
	if (pMainFrame != NULL && pMainFrame->OnEraseMDIClientBackground(pDC))
	{
		return TRUE;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	if (CMFCVisualManager::GetInstance()->OnEraseMDIClientArea(pDC, rectClient))
	{
		return TRUE;
	}

	return CWnd::OnEraseBkgnd(pDC);
}

LRESULT CMDIClientAreaWnd::OnMDIDestroy(WPARAM wParam, LPARAM)
{
	LRESULT lRes = 0;
	CMDIFrameWndEx* pParentFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetParentFrame());

	CMDIChildWndEx* pMDIChild = DYNAMIC_DOWNCAST(CMDIChildWndEx, CWnd::FromHandle((HWND)wParam));
	BOOL bTabHeightChanged = FALSE;

	if (!pParentFrame->m_bClosing && !CMDIFrameWndEx::m_bDisableSetRedraw)
	{
		SetRedraw(FALSE);
	}

	HWND hwndActive = NULL;
	if (pMDIChild != NULL)
	{
		CMFCTabCtrl* pTabWnd = pMDIChild->GetRelatedTabGroup();
		pMDIChild->SetRelatedTabGroup(NULL);
		if (pTabWnd != NULL)
		{
			int nTabsHeight = pTabWnd->GetTabsHeight();

			int iTab = pTabWnd->GetTabFromHwnd((HWND)wParam);
			if (iTab >= 0)
			{
				pMDIChild->m_bToBeDestroyed = TRUE;
			}
			pTabWnd->RemoveTab(iTab);

			if (pTabWnd->GetTabsNum() == 0)
			{
				POSITION pos = m_lstTabbedGroups.Find(pTabWnd);

				if (pos != NULL)
				{
					// find window to activate next group after the current group has been destroyed
					// we should find the window to activate only if the active group is being destroyed
					if (m_lstTabbedGroups.GetCount() > 1 && pTabWnd->IsActiveInMDITabGroup())
					{
						m_lstTabbedGroups.GetNext(pos);
						if (pos == NULL)
						{
							pos = m_lstTabbedGroups.GetHeadPosition();
						}

						if (pos != NULL)
						{
							CMFCTabCtrl* pNextTabWnd = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetAt(pos));
							if (pNextTabWnd != NULL)
							{
								int iActiveTab = pNextTabWnd->GetActiveTab();
								if (iActiveTab == -1)
								{
									iActiveTab = 0;
								}
								CWnd* pActiveWnd = pNextTabWnd->GetTabWnd(iActiveTab);
								if (pActiveWnd != NULL)
								{
									ASSERT_VALID(pActiveWnd);
									hwndActive = pActiveWnd->GetSafeHwnd();
								}
							}
						}
					}
					RemoveTabGroup(pTabWnd);
				}
			}
			else
			{
				bTabHeightChanged = (nTabsHeight != pTabWnd->GetTabsHeight());
			}
		}
	}

	if (m_wndTab.GetSafeHwnd() != NULL)
	{
		int nTabsHeight = m_wndTab.GetTabsHeight();
		int iTab = m_wndTab.GetTabFromHwnd((HWND)wParam);
		if (iTab >= 0)
		{
			CMDIChildWndEx* pMDIChildTab = DYNAMIC_DOWNCAST(CMDIChildWndEx, m_wndTab.GetTabWnd(iTab));
			if (pMDIChildTab != NULL)
			{
				pMDIChildTab->m_bToBeDestroyed = TRUE;
			}

			m_wndTab.RemoveTab(iTab);
		}
		bTabHeightChanged = (nTabsHeight != m_wndTab.GetTabsHeight());
	}

	lRes = Default();
	if (bTabHeightChanged && pParentFrame != NULL)
	{
		pParentFrame->RecalcLayout();
	}

	if (!pParentFrame->m_bClosing)
	{
		if (IsWindow(hwndActive))
		{
			SetActiveTab(hwndActive);
		}
	}
	if (!pParentFrame->m_bClosing && !CMDIFrameWndEx::m_bDisableSetRedraw)
	{
		SetRedraw(TRUE);
		GetParent()->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
	}

	return lRes;
}

void CMDIClientAreaWnd::CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType)
{
	if (m_bDisableUpdateTabs)
	{
		return;
	}
	if (m_bIsMDITabbedGroup)
	{
		CalcWindowRectForMDITabbedGroups(lpClientRect, nAdjustType);
		CWnd::CalcWindowRect(lpClientRect, nAdjustType);
		return;
	}

	if (m_wndTab.GetSafeHwnd() != NULL)
	{
		BOOL bRedraw = FALSE;

		if (m_bTabIsVisible)
		{
			CRect rectOld;
			m_wndTab.GetWindowRect(rectOld);

			m_wndTab.SetWindowPos(NULL, lpClientRect->left, lpClientRect->top, lpClientRect->right - lpClientRect->left, lpClientRect->bottom - lpClientRect->top, SWP_NOZORDER | SWP_NOACTIVATE);

			CRect rectTabClient;
			m_wndTab.GetClientRect(rectTabClient);

			CRect rectTabWnd;
			m_wndTab.GetWndArea(rectTabWnd);

			lpClientRect->top += (rectTabWnd.top - rectTabClient.top);
			lpClientRect->bottom += (rectTabWnd.bottom - rectTabClient.bottom);
			lpClientRect->left += (rectTabWnd.left - rectTabClient.left);
			lpClientRect->right += (rectTabWnd.right - rectTabClient.right);

			m_wndTab.ShowWindow(SW_SHOWNA);

			CRect rectNew;
			m_wndTab.GetWindowRect(rectNew);

			bRedraw = (rectOld != rectNew);
		}
		else
		{
			m_wndTab.ShowWindow(SW_HIDE);
		}

		CRect rectOld;
		GetWindowRect(rectOld);
		int nHeightDelta = lpClientRect->bottom - lpClientRect->top - rectOld.Height();

		SetWindowPos(NULL, lpClientRect->left, lpClientRect->top, lpClientRect->right - lpClientRect->left, lpClientRect->bottom - lpClientRect->top, SWP_NOZORDER | SWP_NOACTIVATE);

		CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetParentFrame());
		if (pMainFrame != NULL)
		{
			pMainFrame->OnSizeMDIClient(rectOld, lpClientRect);
		}

		if (!m_bTabIsVisible)
		{
			CRect rectClient;
			GetClientRect(&rectClient);
			CMDIFrameWndEx* pFrame = (CMDIFrameWndEx*) GetParentFrame();
			ASSERT_VALID(pFrame);
			HWND hwndT = ::GetWindow(pFrame->m_hWndMDIClient, GW_CHILD);

			while (hwndT != NULL)
			{
				DWORD dwStyle = ::GetWindowLong(hwndT, GWL_STYLE);
				if (dwStyle & WS_MAXIMIZE)
				{
					break; // nothing to move;
				}
				if (dwStyle & WS_MINIMIZE)
				{
					CRect rectWnd;
					::GetWindowRect(hwndT, rectWnd);
					ScreenToClient(&rectWnd);

					rectWnd.OffsetRect(0, nHeightDelta);

					if (rectWnd.top < rectClient.top)
					{
						rectWnd.top = rectClient.top;
					}

					::SetWindowPos(hwndT, NULL, rectWnd.left, rectWnd.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
				}

				hwndT=::GetWindow(hwndT,GW_HWNDNEXT);
			}
		}
	}

	CWnd::CalcWindowRect(lpClientRect, nAdjustType);

	int nActiveTab = m_wndTab.GetActiveTab();
	for (int i = 0; i < m_wndTab.GetTabsNum(); i++)
	{
		CWnd* pWnd = m_wndTab.GetTabWnd(i);
		if (pWnd->GetSafeHwnd() == 0)
		{
			continue;
		}

		// only applies to MDI children in "always maximize" mode
		if ((pWnd->GetStyle() & WS_MINIMIZE) != 0 &&
			((pWnd->GetStyle() & WS_SYSMENU) == 0))
		{
			pWnd->ShowWindow(SW_RESTORE);
		}

		DWORD dwFlags = SWP_NOACTIVATE;
		if (i != nActiveTab)
		{
			dwFlags |= SWP_NOZORDER | SWP_NOREDRAW;
		}

		CRect rect(0, 0, lpClientRect->right - lpClientRect->left, lpClientRect->bottom - lpClientRect->top);

		CRect rectClient;
		pWnd->GetClientRect(rectClient);
		pWnd->ClientToScreen(rectClient);

		CRect rectScreen;
		pWnd->GetWindowRect(rectScreen);

		rect.left -= rectClient.left - rectScreen.left;
		rect.top -= rectClient.top - rectScreen.top;
		rect.right += rectScreen.right - rectClient.right;
		rect.bottom += rectScreen.bottom - rectClient.bottom;

		if (rectClient == rect)
		{
			break;
		}

		if (pWnd != NULL &&((pWnd->GetStyle() & WS_SYSMENU) == 0))
		{
			pWnd->SetWindowPos(&wndTop, rect.left, rect.top, rect.Width(), rect.Height(), dwFlags);
		}
	}
}

void CMDIClientAreaWnd::CalcWindowRectForMDITabbedGroups(LPRECT lpClientRect, UINT /*nAdjustType*/)
{
	SetWindowPos(&wndBottom, lpClientRect->left, lpClientRect->top, lpClientRect->right - lpClientRect->left, lpClientRect->bottom - lpClientRect->top, SWP_NOACTIVATE);

	if (m_lstTabbedGroups.IsEmpty())
	{
		return;
	}
	// special processing for single tab

	if (m_lstTabbedGroups.GetCount() == 1)
	{
		CMFCTabCtrl* pNextTab = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetHead());
		ASSERT_VALID(pNextTab);
		pNextTab->ShowWindow(SW_SHOWNA);
		pNextTab->SetWindowPos(NULL, 0, 0, lpClientRect->right - lpClientRect->left, lpClientRect->bottom - lpClientRect->top, SWP_NOZORDER | SWP_NOACTIVATE);

		AdjustMDIChildren(pNextTab);
		return;
	}

	ASSERT(m_groupAlignment != GROUP_NO_ALIGN);

	int nTotalSize = 0;
	POSITION pos = NULL;

	for (pos = m_lstTabbedGroups.GetHeadPosition(); pos != NULL;)
	{
		CMFCTabCtrl* pNextTab = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetNext(pos));
		ASSERT_VALID(pNextTab);

		CRect rect;
		pNextTab->GetWindowRect(rect);

		nTotalSize += (m_groupAlignment == GROUP_VERT_ALIGN) ? rect.Width() : rect.Height();
	}

	int nClientAreaWndSize = (m_groupAlignment == GROUP_VERT_ALIGN) ? lpClientRect->right - lpClientRect->left : lpClientRect->bottom - lpClientRect->top;

	int nDelta = (nClientAreaWndSize - nTotalSize) /(int) m_lstTabbedGroups.GetCount();
	int nRest  = (nClientAreaWndSize - nTotalSize) %(int) m_lstTabbedGroups.GetCount();

	m_nTotalResizeRest += nRest;
	if (abs(m_nTotalResizeRest) >= m_lstTabbedGroups.GetCount())
	{
		m_nTotalResizeRest > 0 ? nDelta ++ : nDelta --;
		m_nTotalResizeRest = 0;
	}

	int nOffset = 0;
	for (pos = m_lstTabbedGroups.GetHeadPosition(); pos != NULL;)
	{
		CMFCTabCtrl* pNextTab = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetNext(pos));
		ASSERT_VALID(pNextTab);

		if (pNextTab->GetSafeHwnd() != NULL)
		{
			CRect rect;
			pNextTab->GetWindowRect(rect);
			ScreenToClient(rect);

			if (m_groupAlignment == GROUP_VERT_ALIGN)
			{
				int nFinalWidth = rect.Width() + nDelta;
				if (pos == NULL && nClientAreaWndSize - nOffset + nFinalWidth != 0)
				{
					nFinalWidth = nClientAreaWndSize - nOffset;
				}
				pNextTab->SetWindowPos(NULL, nOffset, 0, nFinalWidth, lpClientRect->bottom - lpClientRect->top, SWP_NOZORDER | SWP_NOACTIVATE);
				nOffset += rect.Width() + nDelta;
			}
			else
			{
				int nFinalHeight = rect.Height() + nDelta;
				if (pos == NULL && nClientAreaWndSize - nOffset + nFinalHeight != 0)
				{
					nFinalHeight = nClientAreaWndSize - nOffset;
				}
				pNextTab->SetWindowPos(NULL, 0, nOffset, lpClientRect->right - lpClientRect->left, nFinalHeight, SWP_NOZORDER | SWP_NOACTIVATE);
				nOffset += rect.Height() + nDelta;
			}

			AdjustMDIChildren(pNextTab);
		}
	}
}

void CMDIClientAreaWnd::AdjustMDIChildren(CMFCTabCtrl* pTabWnd)
{
	if (!pTabWnd->IsWindowVisible() && CMDIFrameWndEx::m_bDisableSetRedraw)
	{
		return;
	}

	CRect rectTabWnd;
	pTabWnd->GetWndArea(rectTabWnd);
	pTabWnd->MapWindowPoints(this, rectTabWnd);

	int nActiveTab = pTabWnd->GetActiveTab();

	for (int i = 0; i < pTabWnd->GetTabsNum(); i++)
	{
		CWnd* pWnd = pTabWnd->GetTabWnd(i);
		if (pWnd->GetSafeHwnd() == 0)
		{
			continue;
		}

		DWORD dwStyle = ::GetWindowLong(pWnd->GetSafeHwnd(), GWL_STYLE);
		if ((dwStyle & WS_MINIMIZE) != 0)
		{
			pWnd->ShowWindow(SW_RESTORE);
		}

		DWORD dwFlags = SWP_NOACTIVATE;
		if (i != nActiveTab)
		{
			dwFlags |= SWP_NOZORDER | SWP_NOREDRAW;
		}

		if (pWnd != NULL)
		{
			pWnd->SetWindowPos(&wndTop, rectTabWnd.left, rectTabWnd.top, rectTabWnd.Width(), rectTabWnd.Height(), dwFlags);
		}
	}
}

void CMDIClientAreaWnd::SetActiveTab(HWND hwnd)
{
	if (m_bDisableUpdateTabs)
	{
		return;
	}
	if (m_bIsMDITabbedGroup)
	{
		CMDIChildWndEx* pMDIChild = DYNAMIC_DOWNCAST(CMDIChildWndEx, CWnd::FromHandle(hwnd));
		if (pMDIChild != NULL)
		{
			ASSERT_VALID(pMDIChild);
			CMFCTabCtrl* pTabWnd = pMDIChild->GetRelatedTabGroup();
			if (pTabWnd != NULL)
			{
				ASSERT_VALID(pTabWnd);

				int iTab = pTabWnd->GetTabFromHwnd(hwnd);
				if (iTab >= 0)
				{

					CRect rectTabWnd;
					pTabWnd->GetClientRect(rectTabWnd);

					if (rectTabWnd.IsRectEmpty())
					{
						CFrameWnd* pMainFrame = pMDIChild->GetTopLevelFrame();

						if (pMainFrame != NULL)
						{
							ASSERT_VALID(pMainFrame);
							pMainFrame->RecalcLayout();
						}
					}

					CMFCTabCtrl* pPrevActiveWnd = FindActiveTabWnd();
					if (pPrevActiveWnd != NULL)
					{
						pPrevActiveWnd->SetActiveInMDITabGroup(FALSE);
						pPrevActiveWnd->InvalidateTab(pPrevActiveWnd->GetActiveTab());
					}

					pTabWnd->SetActiveInMDITabGroup(TRUE);
					pTabWnd->SetActiveTab(iTab);
					pTabWnd->InvalidateTab(pTabWnd->GetActiveTab());
				}
			}
		}
	}
	else
	{
		if (m_bTabIsVisible)
		{
			int iTab = m_wndTab.GetTabFromHwnd(hwnd);
			if (iTab >= 0)
			{
				m_wndTab.SetActiveTab(iTab);
			}
		}
	}
}

LRESULT CMDIClientAreaWnd::OnUpdateTabs(WPARAM, LPARAM)
{
	UpdateTabs();
	return 0;
}

void CMDIClientAreaWnd::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();
	CreateTabGroup(&m_wndTab);
}

CMFCTabCtrl* CMDIClientAreaWnd::CreateTabGroup(CMFCTabCtrl* pWndTab)
{
	if (pWndTab == NULL)
	{
		pWndTab = new CMFCTabCtrl;
	}

	if (m_mdiTabParams.m_bTabCustomTooltips)
	{
		pWndTab->EnableCustomToolTips();
	}

	CWnd* pParent = m_bIsMDITabbedGroup ? this :(CWnd*) GetParentFrame();

	// Create MDI tabs control:
	if (!pWndTab->Create(m_mdiTabParams.m_style, CRect(0, 0, 0, 0), pParent, (UINT)-1, m_mdiTabParams.m_tabLocation, m_mdiTabParams.m_bTabCloseButton))
	{
		TRACE(_T("CMDIClientAreaWnd::OnCreate: can't create tabs window\n"));
		delete pWndTab;
		return NULL;
	}

	ApplyParams(pWndTab);

	if (!m_bTabIsVisible)
	{
		pWndTab->ShowWindow(SW_HIDE);
	}

	// Create tab icons:

	if (!m_bIsMDITabbedGroup)
	{
		m_TabIcons.Create(afxGlobalData.m_sizeSmallIcon.cx, afxGlobalData.m_sizeSmallIcon.cy, ILC_COLOR32 | ILC_MASK, 0, 1);
	}
	else
	{
		CImageList* pImageList = NULL;
		if (m_mapTabIcons.Lookup(pWndTab, pImageList) && pImageList != NULL)
		{
			pImageList->DeleteImageList();
		}
		else
		{
			pImageList = new CImageList;
			m_mapTabIcons.SetAt(pWndTab, pImageList);
		}

		pImageList->Create(afxGlobalData.m_sizeSmallIcon.cx, afxGlobalData.m_sizeSmallIcon.cy, ILC_COLOR32 | ILC_MASK, 0, 1);
	}

	return pWndTab;
}

void CMDIClientAreaWnd::UpdateTabs(BOOL bSetActiveTabVisible/* = FALSE*/)
{
	if (m_bDisableUpdateTabs)
	{
		return;
	}

	if (m_bIsMDITabbedGroup)
	{
		UpdateMDITabbedGroups(bSetActiveTabVisible);
		return;
	}

	if (m_wndTab.GetSafeHwnd() == NULL || !m_bTabIsVisible)
	{
		return;
	}

	BOOL bRecalcLayout = FALSE;
	BOOL bTabWndEmpty = m_wndTab.GetTabsNum() == 0;

	CWnd* pWndChild = GetWindow(GW_CHILD);
	while (pWndChild != NULL)
	{
		ASSERT_VALID(pWndChild);

		CMDIChildWndEx* pMDIChild = DYNAMIC_DOWNCAST(CMDIChildWndEx, pWndChild);

		BOOL bIsShowTab = TRUE;
		if (pMDIChild != NULL)
		{
			bIsShowTab = pMDIChild->CanShowOnMDITabs();
		}
		else if (pWndChild->IsKindOf(RUNTIME_CLASS(CMFCTabCtrl)))
		{
			pWndChild = pWndChild->GetNextWindow();
			continue;
		}

		// Get tab icon:
		int iIcon = -1;
		if (m_mdiTabParams.m_bTabIcons)
		{
			HICON hIcon = NULL;
			if (pMDIChild != NULL)
			{
				hIcon = pMDIChild->GetFrameIcon();
			}
			else
			{
				if ((hIcon = pWndChild->GetIcon(FALSE)) == NULL)
				{
					hIcon = (HICON)(LONG_PTR) GetClassLongPtr(*pWndChild, GCLP_HICONSM);
				}
			}

			if (hIcon != NULL)
			{
				if (!m_mapIcons.Lookup(hIcon, iIcon))
				{
					iIcon = m_TabIcons.Add(hIcon);
					m_mapIcons.SetAt(hIcon, iIcon);

					if (m_TabIcons.GetImageCount() == 1)
					{
						m_wndTab.SetImageList(m_TabIcons.GetSafeHandle());
					}
				}
			}
		}
		else
		{
			m_wndTab.ClearImageList();
			m_mapIcons.RemoveAll();

			while (m_TabIcons.GetImageCount() > 0)
			{
				m_TabIcons.Remove(0);
			}
		}

		// Get tab label(window caption):
		CString strTabLabel;
		if (pMDIChild != NULL)
		{
			strTabLabel = pMDIChild->GetFrameText();
		}
		else
		{
			pWndChild->GetWindowText(strTabLabel);
		}

		int iTabIndex = m_wndTab.GetTabFromHwnd(pWndChild->GetSafeHwnd());
		if (iTabIndex >= 0)
		{
			// Tab is already exist, update it:
			if (pWndChild->GetStyle() & WS_VISIBLE)
			{
				CString strCurTabLabel;
				m_wndTab.GetTabLabel(iTabIndex, strCurTabLabel);

				if (strCurTabLabel != strTabLabel)
				{
					// Text was changed, update it:
					m_wndTab.SetTabLabel(iTabIndex, strTabLabel);
					bRecalcLayout = TRUE;
				}

				if (m_wndTab.GetTabIcon(iTabIndex) != (UINT) iIcon)
				{
					// Icon was changed, update it:
					m_wndTab.SetTabIcon(iTabIndex, iIcon);
					bRecalcLayout = TRUE;
				}
			}
			else
			{
				// Window is hidden now, remove tab:
				m_wndTab.RemoveTab(iTabIndex);
				bRecalcLayout = TRUE;
			}
		}
		else if ((pMDIChild == NULL || !pMDIChild->m_bToBeDestroyed) && bIsShowTab)
		{
			// New item, add it now:
			m_wndTab.AddTab(pWndChild, strTabLabel, iIcon);
			m_wndTab.SetActiveTab(m_wndTab.GetTabsNum() - 1);

			bRecalcLayout = TRUE;
		}

		pWndChild = pWndChild->GetNextWindow();
	}

	if (bRecalcLayout && GetParentFrame() != NULL)
	{
		GetParentFrame()->RecalcLayout();
	}

	if (bSetActiveTabVisible)
	{
		m_wndTab.EnsureVisible(m_wndTab.GetActiveTab());
	}

	if (bTabWndEmpty && m_wndTab.GetTabsNum() > 0 || m_wndTab.GetTabsNum() == 0)
	{
		GetParentFrame()->RecalcLayout();
		RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_INTERNALPAINT);
	}
}

void CMDIClientAreaWnd::UpdateMDITabbedGroups(BOOL bSetActiveTabVisible)
{
	if (m_bDisableUpdateTabs)
	{
		return;
	}

	BOOL bRecalcLayout = FALSE;
	CWnd* pWndChild = GetWindow(GW_CHILD);
	HWND hwndActive = NULL;

	while (pWndChild != NULL)
	{
		ASSERT_VALID(pWndChild);

		CMDIChildWndEx* pMDIChild = DYNAMIC_DOWNCAST(CMDIChildWndEx, pWndChild);

		if (pMDIChild == NULL)
		{
			pWndChild = pWndChild->GetNextWindow();
			continue;
		}

		// always modify style
		pMDIChild->ModifyStyle(CMDIChildWndEx::m_dwExcludeStyle | WS_MAXIMIZE | WS_SYSMENU, 0, SWP_NOZORDER);

		BOOL bIsShowTab = pMDIChild->CanShowOnMDITabs();
		CString strTabLabel = pMDIChild->GetFrameText();

		CMFCTabCtrl* pRelatedTabWnd = pMDIChild->GetRelatedTabGroup();

		BOOL bRemoved = FALSE;
		if (pRelatedTabWnd == NULL && !pMDIChild->m_bToBeDestroyed && bIsShowTab)
		{
			if (m_lstTabbedGroups.IsEmpty())
			{
				pRelatedTabWnd = CreateTabGroup(NULL);
				m_lstTabbedGroups.AddTail(pRelatedTabWnd);
			}
			else
			{
				// new window to be added
				pRelatedTabWnd = FindActiveTabWnd();

				if (pRelatedTabWnd == NULL)
				{
					pRelatedTabWnd = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetHead());
				}
			}

			ENSURE(pRelatedTabWnd != NULL);
			ASSERT_VALID(pRelatedTabWnd);

			pMDIChild->SetRelatedTabGroup(pRelatedTabWnd);
			pRelatedTabWnd->AddTab(pWndChild, strTabLabel);

			if (!pRelatedTabWnd->IsWindowVisible())
			{
				pRelatedTabWnd->ShowWindow(SW_SHOWNA);
			}

			hwndActive = pWndChild->GetSafeHwnd();

		}
		else if (pRelatedTabWnd != NULL)
		{
			int iTabIndex = pRelatedTabWnd->GetTabFromHwnd(pWndChild->GetSafeHwnd());
			if (iTabIndex >= 0)
			{
				// Tab is already exist, update it:
				if (pWndChild->GetStyle() & WS_VISIBLE)
				{
					CString strCurTabLabel;
					pRelatedTabWnd->GetTabLabel(iTabIndex, strCurTabLabel);

					if (strCurTabLabel != strTabLabel)
					{
						// Text was changed, update it:
						pRelatedTabWnd->SetTabLabel(iTabIndex, strTabLabel);
						bRecalcLayout = TRUE;
					}
				}
				else
				{
					// Window is hidden now, remove tab:
					pRelatedTabWnd->RemoveTab(iTabIndex);
					if (pRelatedTabWnd->GetTabsNum() == 0)
					{
						RemoveTabGroup(pRelatedTabWnd, FALSE);
					}
					bRecalcLayout = TRUE;
					bRemoved = TRUE;
				}
			}
		}

		CImageList* pImageList = NULL;
		m_mapTabIcons.Lookup(pRelatedTabWnd, pImageList);

		if (pImageList != NULL)
		{
			ASSERT_VALID(pImageList);

			int iIcon = -1;
			if (m_mdiTabParams.m_bTabIcons)
			{
				HICON hIcon = NULL;
				if (pMDIChild != NULL)
				{
					hIcon = pMDIChild->GetFrameIcon();
				}

				if (hIcon != NULL)
				{
					if (!pRelatedTabWnd->IsIconAdded(hIcon, iIcon))
					{
						iIcon = pImageList->Add(hIcon);
						pRelatedTabWnd->AddIcon(hIcon, iIcon);
					}

					if (pRelatedTabWnd->GetImageList() != pImageList)
					{
						pRelatedTabWnd->SetImageList(pImageList->GetSafeHandle());
					}
				}

				if (!bRemoved)
				{
					int iTabIndex = pRelatedTabWnd->GetTabFromHwnd(pMDIChild->GetSafeHwnd());
					if (pRelatedTabWnd->GetTabIcon(iTabIndex) != (UINT) iIcon)
					{
						// Icon was changed, update it:
						pRelatedTabWnd->SetTabIcon(iTabIndex, iIcon);
					}
				}
			}
			else
			{
				ENSURE(pRelatedTabWnd != NULL);

				pRelatedTabWnd->ResetImageList();
				m_mapIcons.RemoveAll();

				while (pImageList->GetImageCount() > 0)
				{
					pImageList->Remove(0);
				}

				bRecalcLayout = TRUE;
			}
		}

		pWndChild = pWndChild->GetNextWindow();
	}

	for (POSITION pos = m_lstTabbedGroups.GetHeadPosition(); pos != NULL;)
	{
		CMFCTabCtrl* pNextTab = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetNext(pos));
		ASSERT_VALID(pNextTab);
		AdjustMDIChildren(pNextTab);
	}

	if (bRecalcLayout && GetParentFrame() != NULL)
	{
		GetParentFrame()->RecalcLayout();
	}

	if (hwndActive != NULL)
	{
		SetActiveTab(hwndActive);
	}

	if (bSetActiveTabVisible)
	{
		CMFCTabCtrl* pActiveWnd = FindActiveTabWnd();
		if (pActiveWnd != NULL)
		{
			ASSERT_VALID(pActiveWnd);
			pActiveWnd->EnsureVisible(pActiveWnd->GetActiveTab());
		}
	}
}

void CMDIClientAreaWnd::OnStyleChanging(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	if (nStyleType == GWL_EXSTYLE && !IsKeepClientEdge())
	{
		lpStyleStruct->styleNew = lpStyleStruct->styleOld & ~WS_EX_CLIENTEDGE;
	}

	CWnd::OnStyleChanging(nStyleType, lpStyleStruct);
}

BOOL CMDIClientAreaWnd::IsKeepClientEdge()
{
	BOOL bKeepEdge = FALSE;
	HWND hwndActive = (HWND) SendMessage(WM_MDIGETACTIVE, 0, 0);
	if (hwndActive != NULL)
	{
		CWnd* pWnd = CWnd::FromHandle(hwndActive);
		if (pWnd != NULL && ::IsWindow(pWnd->GetSafeHwnd()))
		{
			bKeepEdge = (pWnd->GetStyle() & WS_SYSMENU) != 0;
		}
	}

	return !m_bIsMDITabbedGroup && bKeepEdge;
}

LRESULT CMDIClientAreaWnd::OnGetDragBounds(WPARAM wp, LPARAM lp)
{
	if (!m_bIsMDITabbedGroup || m_lstTabbedGroups.IsEmpty())
	{
		return 0;
	}

	CMFCTabCtrl* pTabWndToResize = (CMFCTabCtrl*)(wp);
	LPRECT lpRectBounds = (LPRECT)(lp);

	if (pTabWndToResize == NULL)
	{
		return 0;
	}

	ASSERT_VALID(pTabWndToResize);
	CMFCTabCtrl* pNextTabWnd = GetNextTabWnd(pTabWndToResize);

	if (pNextTabWnd == NULL)
	{
		return 0;
	}

	ASSERT(m_groupAlignment != GROUP_NO_ALIGN);

	CRect rectTabWndToResize;
	CRect rectNextTabWnd;

	pTabWndToResize->GetWindowRect(rectTabWndToResize);
	pNextTabWnd->GetWindowRect(rectNextTabWnd);

	rectTabWndToResize.UnionRect(rectTabWndToResize, rectNextTabWnd);

	if (m_groupAlignment == GROUP_VERT_ALIGN)
	{
		rectTabWndToResize.left += m_nResizeMargin;
		rectTabWndToResize.right -= m_nResizeMargin;
	}
	else
	{
		rectTabWndToResize.top += m_nResizeMargin;
		rectTabWndToResize.bottom -= m_nResizeMargin;
	}

	CopyRect(lpRectBounds, &rectTabWndToResize);
	return TRUE;
}

LRESULT CMDIClientAreaWnd::OnDragComplete(WPARAM wp, LPARAM lp)
{
	if (!m_bIsMDITabbedGroup || m_lstTabbedGroups.IsEmpty())
	{
		return 0;
	}

	CMFCTabCtrl* pTabWndToResize = (CMFCTabCtrl*)(wp);
	LPRECT lpRectResized = (LPRECT)(lp);

	ASSERT_VALID(pTabWndToResize);
	CMFCTabCtrl* pNextTabWnd = GetNextTabWnd(pTabWndToResize);

	if (pNextTabWnd == NULL)
	{
		return 0;
	}

	ASSERT(m_groupAlignment != GROUP_NO_ALIGN);

	ScreenToClient(lpRectResized);
	pTabWndToResize->SetWindowPos(NULL, -1, -1, lpRectResized->right - lpRectResized->left, lpRectResized->bottom - lpRectResized->top, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);

	CRect rectNextWnd;
	pNextTabWnd->GetWindowRect(rectNextWnd);
	ScreenToClient(rectNextWnd);

	m_groupAlignment == GROUP_VERT_ALIGN ?  rectNextWnd.left = lpRectResized->right : rectNextWnd.top = lpRectResized->bottom;

	pNextTabWnd->SetWindowPos(NULL, rectNextWnd.left, rectNextWnd.top, rectNextWnd.Width(), rectNextWnd.Height(), SWP_NOZORDER | SWP_NOACTIVATE);

	AdjustMDIChildren(pTabWndToResize);
	AdjustMDIChildren(pNextTabWnd);

	return TRUE;
}

LRESULT CMDIClientAreaWnd::OnTabGroupMouseMove(WPARAM /*wp*/, LPARAM lp)
{
	CMFCTabCtrl* pTabWnd = DYNAMIC_DOWNCAST(CMFCTabCtrl, GetCapture());
	if (pTabWnd == NULL)
	{
		return 0;
	}

	if (m_lstTabbedGroups.GetCount() == 1 && pTabWnd->GetTabsNum() == 1)
	{
		return 0;
	}

	POINTS pt = MAKEPOINTS(lp);
	CPoint point(pt.x, pt.y);

	if (pTabWnd->IsPtInTabArea(point))
	{
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
		DrawNewGroupRect(NULL, m_rectNewTabGroup);
		m_rectNewTabGroup.SetRectEmpty();
		return 0;
	}

	CPoint pointScreen = point;
	pTabWnd->ClientToScreen(&pointScreen);

	CRect rectWnd;
	GetClientRect(rectWnd);

	ClientToScreen(rectWnd);

	if (afxGlobalData.m_hcurMoveTab == NULL)
	{
		afxGlobalData.m_hcurMoveTab = AfxGetApp()->LoadCursor(IDC_AFXBARRES_MOVE_TAB);
		afxGlobalData.m_hcurNoMoveTab = AfxGetApp()->LoadCursor(IDC_AFXBARRES_NO_MOVE_TAB);
	}

	if (!rectWnd.PtInRect(pointScreen))
	{
		::SetCursor(afxGlobalData.m_hcurNoMoveTab);

		DrawNewGroupRect(NULL, m_rectNewTabGroup);
		m_rectNewTabGroup.SetRectEmpty();
		return TRUE;
	}

	::SetCursor(afxGlobalData.m_hcurMoveTab);

	CMFCTabCtrl* pHoveredTabWnd = TabWndFromPoint(pointScreen);

	if (pHoveredTabWnd == NULL)
	{
		DrawNewGroupRect(NULL, m_rectNewTabGroup);
		m_rectNewTabGroup.SetRectEmpty();
		return 0;
	}

	CRect rectScreenHoveredWnd;
	pHoveredTabWnd->GetWindowRect(rectScreenHoveredWnd);

	CRect rectMargin = rectScreenHoveredWnd;

	BOOL bCalcVertRect = TRUE;

	if (m_groupAlignment == GROUP_NO_ALIGN)
	{
		bCalcVertRect = rectScreenHoveredWnd.right - pointScreen.x < rectScreenHoveredWnd.bottom - pointScreen.y;
	}
	else
	{
		bCalcVertRect = m_groupAlignment == GROUP_VERT_ALIGN;
	}

	if (m_groupAlignment == GROUP_VERT_ALIGN || bCalcVertRect)
	{
		rectMargin.left = rectScreenHoveredWnd.right - m_nNewGroupMargin;
		bCalcVertRect = TRUE;
	}
	else if (m_groupAlignment == GROUP_HORZ_ALIGN || !bCalcVertRect)
	{
		rectMargin.top  = rectScreenHoveredWnd.bottom - m_nNewGroupMargin;
		bCalcVertRect = FALSE;
	}

	CRect rectNew = rectScreenHoveredWnd;

	bCalcVertRect ? rectNew.left = rectScreenHoveredWnd.right - rectScreenHoveredWnd.Width() / 2: rectNew.top = rectScreenHoveredWnd.bottom - rectScreenHoveredWnd.Height() / 2;

	if (!rectMargin.PtInRect(pointScreen))
	{
		if (pHoveredTabWnd == pTabWnd)
		{
			rectNew.SetRectEmpty();
		}
		else
		{
			CPoint pointClient = pointScreen;
			pHoveredTabWnd->ScreenToClient(&pointClient);
			if (pHoveredTabWnd->IsPtInTabArea(pointClient))
			{
				pHoveredTabWnd->GetWndArea(rectNew);
				pHoveredTabWnd->ClientToScreen(rectNew);
			}
			else
			{
				rectNew.SetRectEmpty();
			}
		}
	}
	else if (pHoveredTabWnd == pTabWnd && pTabWnd->GetTabsNum() == 1)
	{
		rectNew.SetRectEmpty();
	}

	DrawNewGroupRect(rectNew, m_rectNewTabGroup);
	m_rectNewTabGroup = rectNew;
	m_bNewVericalGroup = bCalcVertRect;

	return TRUE;
}

LRESULT CMDIClientAreaWnd::OnMoveTabComplete(WPARAM wp, LPARAM lp)
{
	CMFCTabCtrl* pTabWnd = (CMFCTabCtrl*) wp;

	CRect rectNewTabGroup = m_rectNewTabGroup;
	DrawNewGroupRect(NULL, m_rectNewTabGroup);
	m_rectNewTabGroup.SetRectEmpty();

	if (pTabWnd == NULL)
	{
		return 0;
	}

	ASSERT_VALID(pTabWnd);

	POINTS pt = MAKEPOINTS(lp);
	CPoint point(pt.x, pt.y);

	CPoint pointScreen = point;
	pTabWnd->ClientToScreen(&pointScreen);

	CMFCTabCtrl* pHoveredTabWnd = TabWndFromPoint(pointScreen);

	if (pHoveredTabWnd == NULL)
	{
		return 0;
	}

	ASSERT_VALID(pHoveredTabWnd);

	BOOL bMenuResult = TRUE;
	if (rectNewTabGroup.IsRectEmpty())
	{
		CMDIFrameWndEx* pMDIFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetParent());
		ASSERT_VALID(pMDIFrame);

		CPoint pointCur;
		GetCursorPos(&pointCur);

		DWORD dwAllowedItems = GetMDITabsContextMenuAllowedItems();

		if (dwAllowedItems != 0)
		{
			bMenuResult = pMDIFrame->OnShowMDITabContextMenu(pointCur, dwAllowedItems, TRUE);
		}
	}

	CRect rectHoveredWnd;
	pHoveredTabWnd->GetWndArea(rectHoveredWnd);
	pHoveredTabWnd->ClientToScreen(rectHoveredWnd);

	if (!bMenuResult)
	{
		if (pTabWnd != pHoveredTabWnd)
		{
			MoveWindowToTabGroup(pTabWnd, pHoveredTabWnd);
		}
		else if (pTabWnd->GetTabsNum() > 1)
		{
			BOOL bVertGroup = FALSE;
			if (m_groupAlignment == GROUP_HORZ_ALIGN)
			{
				rectHoveredWnd.top = rectHoveredWnd.bottom - rectHoveredWnd.Height() / 2;
			}
			else
			{
				rectHoveredWnd.left = rectHoveredWnd.right - rectHoveredWnd.Width() / 2;
				bVertGroup = TRUE;
			}

			CMFCTabCtrl* pNewTabWnd = CreateNewTabGroup(pHoveredTabWnd, rectHoveredWnd, bVertGroup);
			MoveWindowToTabGroup(pTabWnd, pNewTabWnd);
		}
	}
	else if (!rectNewTabGroup.IsRectEmpty())
	{
		if (rectNewTabGroup == rectHoveredWnd)
		{
			MoveWindowToTabGroup(pTabWnd, pHoveredTabWnd);
		}
		else
		{
			CMFCTabCtrl* pNewTabWnd = CreateNewTabGroup(pHoveredTabWnd, rectNewTabGroup, m_bNewVericalGroup);
			MoveWindowToTabGroup(pTabWnd, pNewTabWnd);
		}
	}

	return 0;
}

BOOL CMDIClientAreaWnd::MoveWindowToTabGroup(CMFCTabCtrl* pTabWndFrom, CMFCTabCtrl* pTabWndTo, int nIdxFrom)
{
	ASSERT_VALID(pTabWndFrom);
	ASSERT_VALID(pTabWndTo);

	HWND hwndFrom = pTabWndFrom->GetSafeHwnd();
	HWND hwndTo	  = pTabWndTo->GetSafeHwnd();

	int nIdx = nIdxFrom;
	if (nIdx == -1)
	{
		nIdx = pTabWndFrom->GetActiveTab();
	}

	if (nIdx == -1)
	{
		return FALSE;
	}

	CMDIChildWndEx* pWnd = DYNAMIC_DOWNCAST(CMDIChildWndEx, pTabWndFrom->GetTabWnd(nIdx));

	if (pWnd == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(pWnd);

	CString strTabLabel = pWnd->GetFrameText();

	pTabWndFrom->RemoveTab(nIdx, TRUE);
	pWnd->SetRelatedTabGroup(pTabWndTo);
	pTabWndTo->AddTab(pWnd, strTabLabel);

	if (pTabWndFrom->GetTabsNum() == 0)
	{
		RemoveTabGroup(pTabWndFrom);
		UpdateMDITabbedGroups(TRUE);
		pTabWndTo->RecalcLayout();
	}
	else
	{
		AdjustMDIChildren(pTabWndFrom);
	}

	AdjustMDIChildren(pTabWndTo);
	SetActiveTab(pWnd->GetSafeHwnd());

	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetParentFrame());
	ASSERT_VALID(pMainFrame);

	if (pMainFrame != NULL)
	{
		pMainFrame->SendMessage(AFX_WM_ON_MOVETOTABGROUP, (WPARAM) hwndFrom, (LPARAM) hwndTo);
	}

	return TRUE;
}

CMFCTabCtrl* CMDIClientAreaWnd::CreateNewTabGroup(CMFCTabCtrl* pTabWndAfter, CRect rectGroup, BOOL bVertical)
{
	ASSERT_VALID(pTabWndAfter);
	POSITION pos = m_lstTabbedGroups.Find(pTabWndAfter);

	if (pos == NULL)
	{
		return NULL;
	}

	CMFCTabCtrl* pNewTabWnd = CreateTabGroup(NULL);
	pTabWndAfter->SetResizeMode(bVertical ? CMFCTabCtrl::RESIZE_VERT : CMFCTabCtrl::RESIZE_HORIZ);

	m_lstTabbedGroups.InsertAfter(pos, pNewTabWnd);
	if (pNewTabWnd != m_lstTabbedGroups.GetTail())
	{
		pNewTabWnd->SetResizeMode(bVertical ? CMFCTabCtrl::RESIZE_VERT : CMFCTabCtrl::RESIZE_HORIZ);
	}

	m_groupAlignment = bVertical ? GROUP_VERT_ALIGN : GROUP_HORZ_ALIGN;

	CRect rectWndAfter;
	pTabWndAfter->GetWindowRect(rectWndAfter);

	ScreenToClient(rectGroup);
	ScreenToClient(rectWndAfter);

	if (bVertical)
	{
		rectWndAfter.right -= rectGroup.Width();
		rectGroup.top = rectWndAfter.top;
		rectGroup.bottom = rectWndAfter.bottom;
	}
	else
	{
		rectWndAfter.bottom -= rectGroup.Height();
		rectGroup.left = rectWndAfter.left;
		rectGroup.right = rectWndAfter.right;
	}

	pTabWndAfter->SetWindowPos(NULL, -1, -1, rectWndAfter.Width(), rectWndAfter.Height(), SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	pNewTabWnd->SetWindowPos(NULL, rectGroup.left, rectGroup.top, rectGroup.Width(), rectGroup.Height(), SWP_NOZORDER |  SWP_NOACTIVATE);

	AdjustMDIChildren(pTabWndAfter);

	return pNewTabWnd;
}

void CMDIClientAreaWnd::RemoveTabGroup(CMFCTabCtrl* pTabWnd, BOOL /*bRecalcLayout*/)
{
	ASSERT_VALID(pTabWnd);
	ASSERT(pTabWnd->GetTabsNum() == 0);

	POSITION pos = m_lstTabbedGroups.Find(pTabWnd);
	if (pos == NULL)
	{
		TRACE0("Attempt to remove non-existing tab group");
		return;
	}

	CMFCTabCtrl* pSiblingTabWndToResize = NULL;
	POSITION posNextPrev = pos;
	BOOL bNext = FALSE;

	if (m_lstTabbedGroups.GetHeadPosition() == pos)
	{
		m_lstTabbedGroups.GetNext(posNextPrev);
		bNext = TRUE;
	}
	else
	{
		m_lstTabbedGroups.GetPrev(posNextPrev);
	}

	if (posNextPrev != NULL)
	{
		pSiblingTabWndToResize = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetAt(posNextPrev));
	}

	m_lstTabbedGroups.RemoveAt(pos);
	pTabWnd->ShowWindow(SW_HIDE);

	if (m_lstTabbedGroups.GetCount() > 0)
	{
		CMFCTabCtrl* pLastTabWnd = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetTail());
		ASSERT_VALID(pLastTabWnd);
		pLastTabWnd->SetResizeMode(CMFCTabCtrl::RESIZE_NO);
	}

	if (m_lstTabbedGroups.GetCount() <= 1)
	{
		m_groupAlignment = GROUP_NO_ALIGN;
	}

	if (pSiblingTabWndToResize != NULL)
	{
		CRect rectTabWndToRemove;
		pTabWnd->GetWindowRect(rectTabWndToRemove);
		CRect rectSiblingWnd;
		pSiblingTabWndToResize->GetWindowRect(rectSiblingWnd);

		rectSiblingWnd.UnionRect(rectSiblingWnd, rectTabWndToRemove);
		ScreenToClient(rectSiblingWnd);
		pSiblingTabWndToResize->SetWindowPos(NULL, rectSiblingWnd.left, rectSiblingWnd.top, rectSiblingWnd.Width(), rectSiblingWnd.Height(), SWP_NOZORDER);

		AdjustMDIChildren(pSiblingTabWndToResize);
	}

	CImageList* pImageList = NULL;
	if (m_mapTabIcons.Lookup(pTabWnd, pImageList) && pImageList != NULL)
	{
		delete pImageList;
		m_mapTabIcons.RemoveKey(pTabWnd);
	}

	m_lstRemovedTabbedGroups.AddTail(pTabWnd);
	pTabWnd->ShowWindow(SW_HIDE);

	return;
}

LRESULT CMDIClientAreaWnd::OnCancelTabMove(WPARAM, LPARAM)
{
	DrawNewGroupRect(NULL, m_rectNewTabGroup);
	m_rectNewTabGroup.SetRectEmpty();
	return 0;
}

DWORD CMDIClientAreaWnd::GetMDITabsContextMenuAllowedItems()
{
	CMFCTabCtrl* pActiveTabWnd = FindActiveTabWndByActiveChild();
	if (pActiveTabWnd == NULL)
	{
		return 0;
	}

	DWORD dwAllowedItems = 0;
	int nTabCount = pActiveTabWnd->GetTabsNum();

	if (nTabCount > 1)
	{
		if (m_lstTabbedGroups.GetCount() > 1)
		{
			dwAllowedItems = (m_groupAlignment == GROUP_VERT_ALIGN) ? AFX_MDI_CREATE_VERT_GROUP : AFX_MDI_CREATE_HORZ_GROUP;
		}
		else
		{
			dwAllowedItems = AFX_MDI_CREATE_VERT_GROUP | AFX_MDI_CREATE_HORZ_GROUP;
		}
	}

	if (pActiveTabWnd != m_lstTabbedGroups.GetHead())
	{
		dwAllowedItems |= AFX_MDI_CAN_MOVE_PREV;
	}

	if (pActiveTabWnd != m_lstTabbedGroups.GetTail())
	{
		dwAllowedItems |= AFX_MDI_CAN_MOVE_NEXT;
	}

	CMDIChildWndEx* pMDIChildFrame = DYNAMIC_DOWNCAST(CMDIChildWndEx, pActiveTabWnd->GetActiveWnd());

	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetParentFrame());
	if (pMDIChildFrame != NULL && pMDIChildFrame->IsTabbedPane() && pMainFrame != NULL && !pMainFrame->IsFullScreen())
	{
		dwAllowedItems |= AFX_MDI_CAN_BE_DOCKED;
	}

	return dwAllowedItems;
}

CMFCTabCtrl* CMDIClientAreaWnd::FindActiveTabWndByActiveChild()
{
	HWND hwndActive = (HWND) SendMessage(WM_MDIGETACTIVE, 0, 0);
	if (hwndActive == NULL)
	{
		return NULL;
	}

	for (POSITION pos = m_lstTabbedGroups.GetHeadPosition(); pos != NULL;)
	{
		CMFCTabCtrl* pNextTabWnd = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetNext(pos));
		ASSERT_VALID(pNextTabWnd);
		if (pNextTabWnd->GetTabFromHwnd(hwndActive) >= 0)
		{
			return pNextTabWnd;
		}
	}
	return NULL;
}

CMFCTabCtrl* CMDIClientAreaWnd::FindActiveTabWnd()
{
	for (POSITION pos = m_lstTabbedGroups.GetHeadPosition(); pos != NULL;)
	{
		CMFCTabCtrl* pNextTabWnd = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetNext(pos));
		ASSERT_VALID(pNextTabWnd);
		if (pNextTabWnd->IsActiveInMDITabGroup())
		{
			return pNextTabWnd;
		}
	}
	return NULL;
}

CMFCTabCtrl* CMDIClientAreaWnd::GetFirstTabWnd()
{
	if (m_lstTabbedGroups.IsEmpty())
	{
		return NULL;
	}
	return DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetHead());
}

BOOL CMDIClientAreaWnd::IsMemberOfMDITabGroup(CWnd* pWnd)
{
	if (!IsMDITabbedGroup())
	{
		return FALSE;
	}

	return(m_lstTabbedGroups.Find(pWnd) != NULL);
}

CMFCTabCtrl* CMDIClientAreaWnd::GetNextTabWnd(CMFCTabCtrl* pOrgTabWnd, BOOL /*bWithoutAsserts*/)
{
	POSITION pos = m_lstTabbedGroups.Find(pOrgTabWnd);

	if (pos == NULL)
	{
		ASSERT(FALSE);
		TRACE0("Trying to resize a member of tabbed group which is not in the list of groups.\n");
		return NULL;
	}

	m_lstTabbedGroups.GetNext(pos);
	if (pos == NULL)
	{
		ASSERT(FALSE);
		TRACE0("Trying to resize a last member of tabbed group, which should not be resizable.\n");
		return NULL;
	}

	CMFCTabCtrl* pNextTabWnd = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetAt(pos));

	if (pNextTabWnd == NULL)
	{
		ASSERT(FALSE);
		TRACE0("Next member of tabbed group is NULL, or not a tab window.\n");
		return NULL;
	}

	return pNextTabWnd;
}

CMFCTabCtrl* CMDIClientAreaWnd::TabWndFromPoint(CPoint ptScreen)
{
	for (POSITION pos = m_lstTabbedGroups.GetHeadPosition(); pos != NULL;)
	{
		CMFCTabCtrl* pNextTab = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetNext(pos));
		ASSERT_VALID(pNextTab);
		CRect rectWnd;
		pNextTab->GetWindowRect(rectWnd);
		if (rectWnd.PtInRect(ptScreen))
		{
			return pNextTab;
		}
	}
	return NULL;
}

void CMDIClientAreaWnd::DrawNewGroupRect(LPCRECT lpRectNew, LPCRECT lpRectOld)
{
	CWindowDC dc(GetDesktopWindow());
	CSize size(4, 4);
	CRect rectNew; rectNew.SetRectEmpty();
	CRect rectOld; rectOld.SetRectEmpty();
	if (lpRectNew != NULL)
	{
		rectNew = lpRectNew;
	}
	if (lpRectOld != NULL)
	{
		rectOld = lpRectOld;
	}
	dc.DrawDragRect(rectNew, size, rectOld, size);
}

void CMDIClientAreaWnd::MDITabMoveToNextGroup(BOOL bNext)
{
	CMFCTabCtrl* pActiveWnd = FindActiveTabWndByActiveChild();
	if (pActiveWnd == NULL)
	{
		return;
	}
	ASSERT_VALID(pActiveWnd);

	POSITION pos = m_lstTabbedGroups.Find(pActiveWnd);
	bNext ? m_lstTabbedGroups.GetNext(pos) : m_lstTabbedGroups.GetPrev(pos);

	if (pos == NULL)
	{
		return;
	}

	CMFCTabCtrl* pNextTabWnd = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetAt(pos));
	ASSERT_VALID(pNextTabWnd);

	MoveWindowToTabGroup(pActiveWnd, pNextTabWnd);
}

void CMDIClientAreaWnd::MDITabNewGroup(BOOL bVert)
{
	CMFCTabCtrl* pActiveWnd = FindActiveTabWndByActiveChild();
	if (pActiveWnd == NULL)
	{
		return;
	}
	ASSERT_VALID(pActiveWnd);

	CRect rect;
	pActiveWnd->GetWindowRect(rect);

	if (bVert)
	{
		rect.left += rect.Width() / 2;
	}
	else
	{
		rect.top += rect.Height() / 2;
	}

	CMFCTabCtrl* pNewTabWnd = CreateNewTabGroup(pActiveWnd, rect, bVert);
	MoveWindowToTabGroup(pActiveWnd, pNewTabWnd);
}

void CMDIClientAreaWnd::CloseAllWindows(CMFCTabCtrl* pTabWnd)
{
	if (pTabWnd != NULL)
	{
		ASSERT_VALID(pTabWnd);

		for (int i = pTabWnd->GetTabsNum() - 1; i >= 0; i--)
		{
			CMDIChildWndEx* pNextWnd = DYNAMIC_DOWNCAST(CMDIChildWndEx, pTabWnd->GetTabWnd(i));
			if (pNextWnd != NULL)
			{
				ASSERT_VALID(pNextWnd);
				pNextWnd->SendMessage(WM_CLOSE, (WPARAM) 0, (LPARAM) 0);
			}
		}
	}
	else
	{
		CObList lstWindows;
		CWnd* pWndChild = GetWindow(GW_CHILD);

		while (pWndChild != NULL)
		{
			ASSERT_VALID(pWndChild);

			CMDIChildWndEx* pMDIChild = DYNAMIC_DOWNCAST(CMDIChildWndEx, pWndChild);
			if (pMDIChild != NULL)
			{
				ASSERT_VALID(pMDIChild);
				lstWindows.AddTail(pMDIChild);
			}

			pWndChild = pWndChild->GetNextWindow();
		}

		for (POSITION pos = lstWindows.GetHeadPosition(); pos != NULL;)
		{
			CMDIChildWndEx* pMDIChild = DYNAMIC_DOWNCAST(CMDIChildWndEx, lstWindows.GetNext(pos));
			ASSERT_VALID(pMDIChild);
			pMDIChild->SendMessage(WM_CLOSE, (WPARAM) 0, (LPARAM) 0);
		}

	}
}

void CMDIClientAreaWnd::SerializeTabGroup(CArchive& ar, CMFCTabCtrl* pTabWnd, BOOL bSetRelation)
{
	ASSERT_VALID(pTabWnd);
	if (ar.IsStoring())
	{
		int nTabsNum = pTabWnd->GetTabsNum();
		ar << nTabsNum;

		int nActiveTab = pTabWnd->GetActiveTab();
		ar << nActiveTab;
		ar << pTabWnd->IsActiveInMDITabGroup();

		int i = 0;

		for (i = 0; i < nTabsNum; i++)
		{
			CMDIChildWndEx* pNextWnd = DYNAMIC_DOWNCAST(CMDIChildWndEx, pTabWnd->GetTabWnd(i));
			ASSERT_VALID(pNextWnd);

			CObject* pObject = NULL;
			CString strDocumentName = pNextWnd->GetDocumentName(&pObject);
			ar << strDocumentName;

			BOOL bObjPresent = (pObject != NULL);
			ar << bObjPresent;
			if (bObjPresent)
			{
				ar << pObject;
				delete pObject;
			}

			CString strLabel;
			pTabWnd->GetTabLabel(i, strLabel);
			ar << strLabel;

			ar << pTabWnd->GetResizeMode();
			ar << pTabWnd->GetTabBkColor(i);

			int nBarID = -1;

			if (pNextWnd->IsTabbedPane())
			{
				CDockablePane* pBar = pNextWnd->GetTabbedPane();
				if (pBar != NULL && pBar->GetSafeHwnd() != NULL)
				{
					nBarID = pBar->GetDlgCtrlID();
				}
			}

			ar << nBarID;
		}

		ar << pTabWnd->IsAutoColor();

		const CArray<COLORREF, COLORREF>& arColors = pTabWnd->GetAutoColors();
		ar <<(int) arColors.GetSize();

		for (i = 0; i < arColors.GetSize(); i++)
		{
			ar << arColors [i];
		}

		ar << pTabWnd->IsTabDocumentsMenu();
		ar << pTabWnd->IsTabSwapEnabled();
		ar << pTabWnd->GetTabBorderSize();

		CRect rectWindow;
		pTabWnd->GetWindowRect(rectWindow);
		ar << rectWindow;
	}
	else
	{
		int nTabsNum = 0;
		ar >> nTabsNum;

		int nActiveTab = -1;
		ar >> nActiveTab;

		BOOL bIsActiveInMDITabGroup = FALSE;
		ar >> bIsActiveInMDITabGroup;

		if (bIsActiveInMDITabGroup)
		{
			CMFCTabCtrl* pPrevActiveWnd = FindActiveTabWnd();
			if (pPrevActiveWnd != NULL)
			{
				pPrevActiveWnd->SetActiveInMDITabGroup(FALSE);
				pPrevActiveWnd->InvalidateTab(pPrevActiveWnd->GetActiveTab());
			}
		}
		pTabWnd->SetActiveInMDITabGroup(bIsActiveInMDITabGroup);

		CMDIFrameWndEx* pFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetParent());
		ASSERT_VALID(pFrame);

		int i = 0;

		for (i = 0; i < nTabsNum; i++)
		{
			CString strDocumentName;
			ar >> strDocumentName;

			BOOL bObjectPresent = FALSE;
			CObject* pObj = NULL;

			ar >> bObjectPresent;
			if (bObjectPresent)
			{
				ar >> pObj;
			}

			CString strLabel;
			ar >> strLabel;

			int nValue;
			ar >> nValue;
			pTabWnd->SetResizeMode((CMFCTabCtrl::ResizeMode) nValue);

			COLORREF clrTab = (COLORREF) -1;
			ar >> clrTab;

			int nBarID = -1;
			ar >> nBarID;

			CMDIChildWndEx* pNextWnd = NULL;

			if (!strDocumentName.IsEmpty())
			{
				if (m_lstLoadedTabDocuments.Find(strDocumentName) == NULL)
				{
					pNextWnd = pFrame->CreateDocumentWindow(strDocumentName, pObj);
					if (pNextWnd)
					{
						m_lstLoadedTabDocuments.AddTail(strDocumentName);
					}
				}
				else
				{
					pNextWnd = pFrame->CreateNewWindow(strDocumentName, pObj);
				}
			}
			else if (nBarID != -1)
			{
				CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, pFrame->GetPane(nBarID));
				if (pBar != NULL)
				{
					CBaseTabbedPane* pTabbedBar = pBar->GetParentTabbedPane();
					if (pTabbedBar != NULL)
					{
						pBar->StoreRecentTabRelatedInfo();
					}
					pNextWnd = pFrame->ControlBarToTabbedDocument(pBar);
					if (pTabbedBar != NULL)
					{
						pTabbedBar->RemovePane(pBar);
					}
				}
			}

			if (pNextWnd != NULL)
			{
				ASSERT_VALID(pNextWnd);
				pTabWnd->AddTab(pNextWnd, strLabel);
				pTabWnd->SetTabBkColor(i, clrTab);
				if (bSetRelation)
				{
					pNextWnd->SetRelatedTabGroup(pTabWnd);
				}
			}

			if (pObj != NULL)
			{
				delete pObj;
			}
		}

		BOOL bIsAutoColor = FALSE;
		ar >> bIsAutoColor;

		int nAutoColorSize = 0;
		ar >> nAutoColorSize;

		CArray<COLORREF, COLORREF> arColors;

		for (i = 0; i < nAutoColorSize; i++)
		{
			COLORREF clr = (COLORREF) -1;
			ar >> clr;
			arColors.SetAtGrow(i, clr);
		}

		pTabWnd->EnableAutoColor(bIsAutoColor);
		pTabWnd->SetAutoColors(arColors);
		m_mdiTabParams.m_bAutoColor = bIsAutoColor;

		BOOL bValue = FALSE;

		ar >> bValue;
		pTabWnd->EnableTabDocumentsMenu(bValue);
		m_mdiTabParams.m_bDocumentMenu = bValue;

		ar >> bValue;
		pTabWnd->EnableTabSwap(bValue);
		m_mdiTabParams.m_bEnableTabSwap = bValue;

		int nTabBorderSize = 1;
		ar >> nTabBorderSize;
		pTabWnd->SetTabBorderSize(nTabBorderSize);
		m_mdiTabParams.m_nTabBorderSize = nTabBorderSize;

		CRect rectWindow;
		ar >> rectWindow;

		pTabWnd->GetParent()->ScreenToClient(rectWindow);

		pTabWnd->SetWindowPos(NULL, rectWindow.left, rectWindow.right, rectWindow.Width(), rectWindow.Height(), SWP_NOZORDER | SWP_NOACTIVATE);

		if (pTabWnd->GetTabsNum() > 0)
		{
			if (nActiveTab > pTabWnd->GetTabsNum() - 1)
			{
				nActiveTab = pTabWnd->GetTabsNum() - 1;
			}

			pTabWnd->SetActiveTab(nActiveTab);
		}
	}
}

void CMDIClientAreaWnd::SerializeOpenChildren(CArchive& ar)
{
	if (ar.IsStoring())
	{
		CObList lstWindows;

		CWnd* pWndChild = GetWindow(GW_CHILD);
		while (pWndChild != NULL)
		{
			ASSERT_VALID(pWndChild);

			CMDIChildWndEx* pMDIChild = DYNAMIC_DOWNCAST(CMDIChildWndEx, pWndChild);
			if (pMDIChild != NULL)
			{
				ASSERT_VALID(pWndChild);

				CObject* pObj = NULL;
				CString str = pMDIChild->GetDocumentName(&pObj);
				if (pObj != NULL)
				{
					delete pObj;
				}
				if (!str.IsEmpty())
				{
					lstWindows.AddHead(pMDIChild);
				}
			}

			pWndChild = pWndChild->GetNextWindow();
		}

		HWND hwndActive = (HWND) SendMessage(WM_MDIGETACTIVE);

		ar <<(int) lstWindows.GetCount();

		for (POSITION pos = lstWindows.GetHeadPosition(); pos != NULL;)
		{
			CMDIChildWndEx* pMDIChild = DYNAMIC_DOWNCAST(CMDIChildWndEx, lstWindows.GetNext(pos));

			ASSERT_VALID(pMDIChild);

			CObject* pObj = NULL;
			CString str = pMDIChild->GetDocumentName(&pObj);
			ar << str;

			BOOL bObjPresent = (pObj != NULL);
			ar << bObjPresent;
			if (bObjPresent)
			{
				ar << pObj;
				delete pObj;
			}

			WINDOWPLACEMENT wp;
			pMDIChild->GetWindowPlacement(&wp);

			ar << wp.flags;
			ar << wp.length;
			ar << wp.ptMaxPosition;
			ar << wp.ptMinPosition;
			ar << wp.rcNormalPosition;
			ar << wp.showCmd;

			BOOL bActive = (pMDIChild->GetSafeHwnd() == hwndActive);
			ar << bActive;

			int nBarID = -1;

			if (pMDIChild->IsTabbedPane())
			{
				CDockablePane* pBar = pMDIChild->GetTabbedPane();
				if (pBar != NULL && pBar->GetSafeHwnd() != NULL)
				{
					nBarID = pBar->GetDlgCtrlID();
				}
			}

			ar << nBarID;
		}

	}
	else
	{
		CMDIFrameWndEx* pFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetParent());
		ASSERT_VALID(pFrame);

		int nCount = 0;
		ar >> nCount;

		HWND hwndActive = NULL;
		BOOL bMaximize = FALSE;
		for (int i = 0; i < nCount; i++)
		{
			CString strDocName;
			ar >> strDocName;

			BOOL bObjPresent = FALSE;
			CObject* pObj = NULL;

			ar >> bObjPresent;
			if (bObjPresent)
			{
				ar >> pObj;
			}

			WINDOWPLACEMENT wp;
			ar >> wp.flags;
			ar >> wp.length;
			ar >> wp.ptMaxPosition;
			ar >> wp.ptMinPosition;
			ar >> wp.rcNormalPosition;
			ar >> wp.showCmd;

			BOOL bActive = FALSE;
			ar >> bActive;

			int nBarID = -1;
			ar >> nBarID;

			if (bMaximize)
			{
				wp.showCmd = SW_SHOWMAXIMIZED;
			}

			CMDIChildWndEx* pNextWnd = NULL;

			if (!strDocName.IsEmpty() && nBarID == -1)
			{
				pNextWnd = pFrame->CreateNewWindow(strDocName, pObj);
			}
			else if (nBarID != -1)
			{

				CDockablePane* pBar = DYNAMIC_DOWNCAST(CDockablePane, pFrame->GetPane(nBarID));
				if (pBar != NULL)
				{
					CBaseTabbedPane* pTabbedBar = pBar->GetParentTabbedPane();
					if (pTabbedBar != NULL)
					{
						pBar->StoreRecentTabRelatedInfo();
					}
					pNextWnd = pFrame->ControlBarToTabbedDocument(pBar);
					if (pTabbedBar != NULL)
					{
						pTabbedBar->RemovePane(pBar);
					}
				}
			}

			if (pObj != NULL)
			{
				delete pObj;
			}
			if (pNextWnd != NULL)
			{
				ASSERT_VALID(pNextWnd);

				pNextWnd->SetWindowPlacement(&wp);
				if (wp.showCmd == SW_SHOWMAXIMIZED)
				{
					ShowWindow(wp.showCmd);
					bMaximize = TRUE;
				}

				if (bActive)
				{
					hwndActive = pNextWnd->GetSafeHwnd();
				}
			}
		}

		if (hwndActive != NULL)
		{
			SendMessage(WM_MDIACTIVATE, (WPARAM) hwndActive);
		}
	}
}

BOOL CMDIClientAreaWnd::SaveState(LPCTSTR lpszProfileName, UINT nFrameID)
{
	BOOL bResult = FALSE;
	CString strProfileName = ::AFXGetRegPath(strMDIClientAreaProfile, lpszProfileName);

	CString strSection;
	strSection.Format(AFX_REG_SECTION_FMT, (LPCTSTR)strProfileName, nFrameID);

	try
	{
		CMemFile file;
		{
			CArchive ar(&file, CArchive::store);

			Serialize(ar);
			ar.Flush();
		}

		UINT uiDataSize = (UINT) file.GetLength();
		LPBYTE lpbData = file.Detach();

		if (lpbData != NULL)
		{
			CSettingsStoreSP regSP;
			CSettingsStore& reg = regSP.Create(FALSE, FALSE);

			if (reg.CreateKey(strSection))
			{
				bResult = reg.Write(AFX_REG_ENTRY_MDITABS_STATE, lpbData, uiDataSize);
			}

			free(lpbData);
		}
	}
	catch(CMemoryException* pEx)
	{
		pEx->Delete();
		TRACE(_T("Memory exception in CMDIClientAreaWnd::SaveState()!\n"));
	}
	catch(CArchiveException* pEx)
	{
		pEx->Delete();
		TRACE(_T("CArchiveException exception in CMDIClientAreaWnd::SaveState()!\n"));
	}
	catch(...)
	{
		TRACE(_T("Unknown exception in CMDIClientAreaWnd::SaveState()!\n"));
	}

	return bResult;
}

BOOL CMDIClientAreaWnd::LoadState(LPCTSTR lpszProfileName, UINT nFrameID)
{
	BOOL bResult = FALSE;

	CString strProfileName = ::AFXGetRegPath(strMDIClientAreaProfile, lpszProfileName);

	CString strSection;
	strSection.Format(AFX_REG_SECTION_FMT, (LPCTSTR)strProfileName, nFrameID);

	LPBYTE lpbData = NULL;
	UINT uiDataSize;

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (!reg.Open(strSection))
	{
		return FALSE;
	}

	if (!reg.Read(AFX_REG_ENTRY_MDITABS_STATE, &lpbData, &uiDataSize))
	{
		return FALSE;
	}

	try
	{
		CMemFile file(lpbData, uiDataSize);
		CArchive ar(&file, CArchive::load);

		Serialize(ar);
		bResult = TRUE;
	}
	catch(CMemoryException* pEx)
	{
		pEx->Delete();
		TRACE(_T("Memory exception in CMDIClientAreaWnd::LoadState!\n"));
	}
	catch(CArchiveException* pEx)
	{
		pEx->Delete();
		TRACE(_T("CArchiveException exception in CMDIClientAreaWnd::LoadState()!\n"));
	}
	catch(...)
	{
		TRACE(_T("Unknown exception in CMDIClientAreaWnd::LoadState()!\n"));
	}

	if (lpbData != NULL)
	{
		delete [] lpbData;
	}

	if (!bResult)
	{
		m_bDisableUpdateTabs = FALSE;
		CloseAllWindows(NULL);
	}

	return bResult;
}

void CMDIClientAreaWnd::Serialize(CArchive& ar)
{
	m_mdiTabParams.Serialize(ar);

	if (ar.IsStoring())
	{
		ar << m_bTabIsEnabled;
		ar << m_bIsMDITabbedGroup;

		ar << m_bTabIsVisible;
		ar << m_groupAlignment;
		ar << m_nResizeMargin;
		ar << m_nNewGroupMargin;

		if (m_bTabIsEnabled)
		{
			SerializeTabGroup(ar, &m_wndTab);
		}
		else if (m_bIsMDITabbedGroup)
		{
			int nCountTabbedGroups = (int) m_lstTabbedGroups.GetCount();
			ar << nCountTabbedGroups;

			if (nCountTabbedGroups > 0)
			{
				for (POSITION pos = m_lstTabbedGroups.GetHeadPosition(); pos != NULL;)
				{
					CMFCTabCtrl* pNextWnd = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetNext(pos));
					SerializeTabGroup(ar, pNextWnd);
				}
			}
		}
		else
		{
			SerializeOpenChildren(ar);
		}
	}
	else if (ar.IsLoading())
	{
		CloseAllWindows(NULL);
		m_lstLoadedTabDocuments.RemoveAll();

		m_bDisableUpdateTabs = TRUE;

		ar >> m_bTabIsEnabled;
		ar >> m_bIsMDITabbedGroup;
		ar >> m_bTabIsVisible;

		int nValue;
		ar >> nValue;
		m_groupAlignment = (GROUP_ALIGNMENT) nValue;

		ar >> m_nResizeMargin;
		ar >> m_nNewGroupMargin;

		if (m_bTabIsEnabled)
		{
			SerializeTabGroup(ar, &m_wndTab);
			EnableMDITabs(TRUE, m_mdiTabParams);
		}
		else if (m_bIsMDITabbedGroup)
		{

			int nCountTabbedGroups = 0;
			ar >> nCountTabbedGroups;

			for (int i = 0; i < nCountTabbedGroups; i++)
			{
				CMFCTabCtrl* pNewTabWnd = CreateTabGroup(NULL);
				ASSERT_VALID(pNewTabWnd);
				SerializeTabGroup(ar, pNewTabWnd, TRUE);

				if (pNewTabWnd->GetTabsNum() == 0)
				{
					pNewTabWnd->DestroyWindow();
					delete pNewTabWnd;
				}
				else
				{
					m_lstTabbedGroups.AddTail(pNewTabWnd);
				}
			}

			// sanity check for resize mode - the last group might have been removed
			// because the document could not be opened
			if (m_lstTabbedGroups.GetCount() > 0)
			{
				CMFCTabCtrl* pLastTabWnd = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetTail());
				ASSERT_VALID(pLastTabWnd);
				pLastTabWnd->SetResizeMode(CMFCTabCtrl::RESIZE_NO);
			}

			EnableMDITabbedGroups(TRUE, m_mdiTabParams);
		}
		else
		{
			SerializeOpenChildren(ar);
		}

		m_bDisableUpdateTabs = FALSE;

		if (m_bIsMDITabbedGroup)
		{
			UpdateMDITabbedGroups(TRUE);
			for (POSITION pos = m_lstTabbedGroups.GetHeadPosition(); pos != NULL;)
			{
				CMFCTabCtrl* pNextWnd = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetNext(pos));
				ASSERT_VALID(pNextWnd);
				pNextWnd->RecalcLayout();

				if (pNextWnd->IsActiveInMDITabGroup())
				{
					CWnd* pWnd = pNextWnd->GetTabWnd(pNextWnd->GetActiveTab());
					PostMessage(WM_MDIACTIVATE, (WPARAM) pWnd->GetSafeHwnd());
				}
			}
		}
		else if (m_bTabIsEnabled)
		{
			UpdateTabs(TRUE);
			m_wndTab.RecalcLayout();
		}

		((CFrameWnd*) GetParent())->RecalcLayout();
	}
}

LRESULT CMDIClientAreaWnd::OnMDINext(WPARAM wp, LPARAM lp)
{
	if (!m_bIsMDITabbedGroup && !m_bTabIsEnabled)
	{
		return Default();
	}

	BOOL bNext = (lp == 0);

	CMFCTabCtrl* pActiveTabWnd = NULL;
	int nActiveTab = -1;

	if (m_bIsMDITabbedGroup)
	{
		pActiveTabWnd = FindActiveTabWnd();
	}
	else
	{
		pActiveTabWnd = &m_wndTab;
	}

	ASSERT_VALID(pActiveTabWnd);

	POSITION posActive = m_bIsMDITabbedGroup ? m_lstTabbedGroups.Find(pActiveTabWnd) : NULL;
	int nGroupCount = m_bIsMDITabbedGroup ?(int) m_lstTabbedGroups.GetCount() : 0;

	if (m_bIsMDITabbedGroup)
	{
		ENSURE(posActive != NULL);
	}

	nActiveTab = pActiveTabWnd->GetActiveTab();

	bNext ? nActiveTab++ : nActiveTab--;

	if (nActiveTab < 0)
	{
		if (nGroupCount > 0)
		{
			m_lstTabbedGroups.GetPrev(posActive);
			if (posActive != NULL)
			{
				pActiveTabWnd = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetAt(posActive));
			}
			else
			{
				pActiveTabWnd = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetTail());
			}
		}

		ENSURE(pActiveTabWnd != NULL);

		nActiveTab = pActiveTabWnd->GetTabsNum() - 1;
	}

	if (nActiveTab >= pActiveTabWnd->GetTabsNum())
	{
		if (nGroupCount > 0)
		{
			pActiveTabWnd = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetNext(posActive));
			if (posActive != NULL)
			{
				pActiveTabWnd = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetAt(posActive));
			}
			else
			{
				pActiveTabWnd = DYNAMIC_DOWNCAST(CMFCTabCtrl, m_lstTabbedGroups.GetHead());
			}
		}

		ENSURE(pActiveTabWnd != NULL);

		nActiveTab = 0;
	}

	CWnd* pWnd = pActiveTabWnd->GetTabWnd(nActiveTab);
	ASSERT_VALID(pWnd);

	if (pWnd->GetSafeHwnd() != (HWND) wp)
	{
		SetActiveTab(pWnd->GetSafeHwnd());
	}
	return 0L;
}



