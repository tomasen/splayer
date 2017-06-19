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
#include "afxoutlookbartabctrl.h"
#include "afxoutlookbarpane.h"
#include "afxoutlookbar.h"
#include "afxdockablepaneadapter.h"
#include "afxglobals.h"
#include "afxvisualmanager.h"
#include "afxoutlookbarpanebutton.h"
#include "afxdockingmanager.h"
#include "afxribbonres.h"
#include "afxcustomizebutton.h"
#include "afxmultipaneframewnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const int nSplitterHeight = 8;
static const int nToolbarMarginHeight = 4;
static const UINT idShowMoreButtons = 0xf200;
static const UINT idShowFewerButtons = 0xf201;
static const UINT idNavigationPaneOptions = 0xf202;
static const UINT idToolbarCommandID = 0xf203;
static const int nTopEdgeHeight = 4;

/////////////////////////////////////////////////////////////////////////////
// CMFCOutlookBarScrollButton

void CMFCOutlookBarScrollButton::OnFillBackground(CDC* pDC, const CRect& rectClient)
{
	CMFCVisualManager::GetInstance()->OnFillOutlookPageButton(pDC, rectClient, m_bHighlighted, m_bPushed, afxGlobalData.clrBarText);
}

void CMFCOutlookBarScrollButton::OnDrawBorder(CDC* pDC, CRect& rectClient, UINT /*uiState*/)
{
	CMFCVisualManager::GetInstance()->OnDrawOutlookPageButtonBorder(pDC, rectClient, m_bHighlighted, m_bPushed);
}

/////////////////////////////////////////////////////////////////////////////
// COutlookCustomizeButton

class COutlookCustomizeButton : public CMFCCustomizeButton
{
	DECLARE_DYNCREATE(COutlookCustomizeButton)

	friend class CMFCOutlookBarTabCtrl;

	virtual void OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages, BOOL bHorz = TRUE, BOOL bCustomizeMode = FALSE,
		BOOL bHighlight = FALSE, BOOL bDrawBorder = TRUE, BOOL bGrayDisabledButtons = TRUE);
	virtual CMFCPopupMenu* CreatePopupMenu();
};

IMPLEMENT_DYNCREATE(COutlookCustomizeButton, CMFCCustomizeButton)

CMFCPopupMenu* COutlookCustomizeButton::CreatePopupMenu()
{
	CMFCPopupMenu* pMenu = CMFCCustomizeButton::CreatePopupMenu();
	if (pMenu == NULL)
	{
		return NULL;
	}

	pMenu->RemoveItem(pMenu->GetMenuBar()->CommandToIndex(m_iCustomizeCmdId));

	if (pMenu->GetMenuItemCount() > 0)
	{
		pMenu->InsertSeparator();
	}

	CString strItem;

	ENSURE(strItem.LoadString(IDS_AFXBARRES_SHOW_MORE_BUTTONS));
	pMenu->InsertItem(CMFCToolBarMenuButton(idShowMoreButtons, NULL, -1, strItem));

	ENSURE(strItem.LoadString(IDS_AFXBARRES_SHOW_FEWER_BUTTONS));
	pMenu->InsertItem(CMFCToolBarMenuButton(idShowFewerButtons, NULL, -1, strItem));

	ENSURE(strItem.LoadString(IDS_AFXBARRES_NAV_PANE_OPTIONS));
	pMenu->InsertItem(CMFCToolBarMenuButton(idNavigationPaneOptions, NULL, -1, strItem));

	return pMenu;
}

void COutlookCustomizeButton::OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* /*pImages*/,
	BOOL /*bHorz*/, BOOL /*bCustomizeMode*/, BOOL bHighlight, BOOL /*bDrawBorder*/, BOOL /*bGrayDisabledButtons*/)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(this);

	m_bDefaultDraw = TRUE;

	FillInterior(pDC, rect, bHighlight || IsDroppedDown());

	CSize sizeImage = CMenuImages::Size();

	int x = rect.left + max(0, (rect.Width() - sizeImage.cx) / 2);
	int y = rect.top + max(0, (rect.Height() - 2 * sizeImage.cy) / 2);

	CMenuImages::Draw(pDC, CMenuImages::IdMoreButtons, CPoint(x, y));

	y += sizeImage.cy;

	CMenuImages::Draw(pDC, CMenuImages::IdArrowDown, CPoint(x, y));
}

/////////////////////////////////////////////////////////////////////////////
// CMFCOutlookBarToolBar

IMPLEMENT_DYNAMIC(CMFCOutlookBarToolBar, CMFCToolBar)

CMFCOutlookBarToolBar::CMFCOutlookBarToolBar(CMFCOutlookBarTabCtrl* pParentBar) :
	m_pParentBar(pParentBar)
{
	m_bLocked = TRUE;
}

BEGIN_MESSAGE_MAP(CMFCOutlookBarToolBar, CMFCToolBar)
	//{{AFX_MSG_MAP(CMFCOutlookBarToolBar)
	ON_WM_SETCURSOR()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CMFCOutlookBarToolBar::OnSendCommand(const CMFCToolBarButton* pButton)
{
	int nIndex = ButtonToIndex(pButton);
	if (nIndex >= 0)
	{
		int iTab = -1;

		if (m_TabButtons.Lookup(nIndex, iTab))
		{
			if (m_pParentBar->SetActiveTab(iTab) && m_pParentBar->GetParentFrame() != NULL)
			{
				m_pParentBar->GetParentFrame()->SendMessage(AFX_WM_CHANGE_ACTIVE_TAB, iTab, 0);
			}

			return TRUE;
		}
	}

	return FALSE;
}

void CMFCOutlookBarToolBar::OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL /*bDisableIfNoHndler*/)
{
	for (int i = 0; i < m_Buttons.GetCount(); i++)
	{
		UINT nNewStyle = GetButtonStyle(i) & ~(TBBS_CHECKED | TBBS_INDETERMINATE);

		int iTab = -1;
		if (m_TabButtons.Lookup(i, iTab))
		{
			if (m_pParentBar->GetActiveTab() == iTab)
			{
				nNewStyle |= TBBS_CHECKED;
			}

			SetButtonStyle(i, nNewStyle | TBBS_CHECKBOX);
		}
	}
}

BOOL CMFCOutlookBarToolBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint ptCursor;
	::GetCursorPos(&ptCursor);

	ScreenToClient(&ptCursor);

	if (HitTest(ptCursor) >= 0)
	{
		::SetCursor(afxGlobalData.GetHandCursor());
		return TRUE;
	}

	return CMFCToolBar::OnSetCursor(pWnd, nHitTest, message);
}

BOOL CMFCOutlookBarToolBar::OnUserToolTip(CMFCToolBarButton* pButton, CString& strTTText) const
{
	strTTText = pButton->m_strText;
	return TRUE;
}

void CMFCOutlookBarToolBar::AdjustLocations()
{
	CSize sizeImage = GetImageSize();
	if (sizeImage == CSize(0, 0))
	{
		sizeImage = CSize(16, 16);
	}

	CSize sizeButton = sizeImage + CSize(10, 6 + 2 * nToolbarMarginHeight);

	CSize sizeCustomizeButton(0, 0);
	if (m_pCustomizeBtn != NULL)
	{
		sizeCustomizeButton = sizeButton;
		sizeCustomizeButton.cx = max(sizeCustomizeButton.cx, CMenuImages::Size().cx + 10);
	}

	CRect rectToolbar;
	GetClientRect(rectToolbar);

	int nCount = sizeCustomizeButton == CSize(0, 0) ? (int) m_Buttons.GetCount() : (int) m_Buttons.GetCount() - 1;

	int x = rectToolbar.right -  sizeCustomizeButton.cx + 2;

	int nCountToHide = nCount -(rectToolbar.Width() - sizeCustomizeButton.cx + 2) / (sizeButton.cx - 2);

	for (POSITION pos = m_Buttons.GetTailPosition();  pos != NULL;)
	{
		CMFCToolBarButton* pButton = (CMFCToolBarButton*) m_Buttons.GetPrev(pos);
		ASSERT_VALID(pButton);
		CMFCCustomizeButton* pCustomizeBtn = DYNAMIC_DOWNCAST(CMFCCustomizeButton, pButton);

		if (nCountToHide > 0 && pCustomizeBtn == NULL)
		{
			if (m_pCustomizeBtn != NULL)
			{
				CObList& list = const_cast<CObList&>(m_pCustomizeBtn->GetInvisibleButtons());
				list.AddHead(pButton);
			}

			pButton->SetRect(CRect(0, 0, 0, 0));
			nCountToHide--;
		}
		else
		{
			CSize sizeCurrButton = sizeButton;

			if (pButton == m_pCustomizeBtn)
			{
				sizeCurrButton = sizeCustomizeButton;
			}

			sizeCurrButton.cy++;
			pButton->SetRect(CRect(CPoint(x, -1), sizeCurrButton));

			x -= sizeButton.cx - 2;
		}
	}

	UpdateTooltips();
}

void CMFCOutlookBarToolBar::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS FAR* /*lpncsp*/)
{
}

void CMFCOutlookBarToolBar::OnNcPaint()
{
}

void CMFCOutlookBarToolBar::OnCustomizeMode(BOOL bSet)
{
	CMFCToolBar::OnCustomizeMode(bSet);
	EnableWindow(!bSet);
}

/////////////////////////////////////////////////////////////////////////////
// CMFCOutlookBarTabCtrl

IMPLEMENT_DYNCREATE(CMFCOutlookBarTabCtrl, CMFCBaseTabCtrl)

BOOL CMFCOutlookBarTabCtrl::m_bEnableAnimation = FALSE;

#pragma warning(disable : 4355)

CMFCOutlookBarTabCtrl::CMFCOutlookBarTabCtrl() : m_wndToolBar(this)
{
	m_rectWndArea.SetRectEmpty();
	m_rectCaption.SetRectEmpty();
	m_nBorderSize = 0;
	m_bActivateOnBtnUp = TRUE;
	m_bEnableTabSwap = FALSE;
	m_bScrollButtons = FALSE;

	m_btnUp.m_nFlatStyle = CMFCButton::BUTTONSTYLE_SEMIFLAT;
	m_btnUp.m_bDrawFocus = FALSE;

	m_btnDown.m_nFlatStyle = CMFCButton::BUTTONSTYLE_SEMIFLAT;
	m_btnDown.m_bDrawFocus = FALSE;

	m_nPageButtonTextAlign = TA_CENTER;

	m_bIsTracking = FALSE;
	m_rectSplitter.SetRectEmpty();

	m_nVisiblePageButtons = -1;
	m_nMaxVisiblePageButtons = 0;
	m_bDontAdjustLayout = FALSE;

	m_sizeToolbarImage = CSize(0, 0);
}

#pragma warning(default : 4355)

CMFCOutlookBarTabCtrl::~CMFCOutlookBarTabCtrl()
{
}

BEGIN_MESSAGE_MAP(CMFCOutlookBarTabCtrl, CMFCBaseTabCtrl)
	//{{AFX_MSG_MAP(CMFCOutlookBarTabCtrl)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CANCELMODE()
	ON_COMMAND_RANGE(idShowMoreButtons, idShowMoreButtons + 10, &CMFCOutlookBarTabCtrl::OnToolbarCommand)
	ON_UPDATE_COMMAND_UI_RANGE(idShowMoreButtons, idShowMoreButtons + 10, &CMFCOutlookBarTabCtrl::OnUpdateToolbarCommand)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCOutlookBarTabCtrl message handlers

BOOL CMFCOutlookBarTabCtrl::Create(const CRect& rect, CWnd* pParentWnd, UINT nID)
{
	if (!CWnd::Create(NULL, _T(""),
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		rect, pParentWnd, nID))
	{
		return FALSE;
	}

	SetTabsHeight();
	m_bHighLightTabs = TRUE;

	return TRUE;
}

BOOL CMFCOutlookBarTabCtrl::IsPtInTabArea(CPoint point) const
{
	CRect rectTop; rectTop.SetRectEmpty();
	CRect rectBottom; rectBottom.SetRectEmpty();
	GetTabArea(rectTop, rectBottom);

	ScreenToClient(rectTop);
	ScreenToClient(rectBottom);

	return rectTop.PtInRect(point) || rectBottom.PtInRect(point);
}

void CMFCOutlookBarTabCtrl::AddControl(CWnd* pWndCtrl, LPCTSTR lpszName, int nImageID, BOOL bDetachable, DWORD dwControlBarStyle)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pWndCtrl);

	AddTab(pWndCtrl, lpszName, nImageID);

	if (bDetachable && !IsMode2003())
	{
		int nInsertedTab = GetTabFromHwnd(pWndCtrl->GetSafeHwnd());

		CDockablePaneAdapter* pWrapper = DYNAMIC_DOWNCAST(CDockablePaneAdapter, GetTabWnd(nInsertedTab));
		if (pWrapper != NULL)
		{
			ASSERT_VALID(pWrapper);

			pWrapper->SetTabbedPaneRTC(RUNTIME_CLASS(CMFCOutlookBar));
			pWrapper->SetMiniFrameRTC(RUNTIME_CLASS(CMultiPaneFrameWnd));

			// we need this flag for the runtime checking
			pWrapper->SetPaneStyle(pWrapper->GetPaneStyle() | CBRS_FLOAT_MULTI);
			pWrapper->SetControlBarStyle(dwControlBarStyle);
		}
	}
}

void CMFCOutlookBarTabCtrl::RecalcLayout()
{
	ASSERT_VALID(this);

	if (GetSafeHwnd() == NULL || m_nTabsHeight == 0)
	{
		return;
	}

	const BOOL bIsMode2003 = IsMode2003();
	int nToolBarHeight = 0;

	if (bIsMode2003)
	{
		CSize sizeImage(0, 0);

		if (m_imagesToolbar.GetSafeHandle() != NULL)
		{
			sizeImage = m_sizeToolbarImage;
		}
		else
		{
			sizeImage = GetImageSize();
		}

		if (sizeImage.cy == 0)
		{
			sizeImage.cy = 16;
		}

		nToolBarHeight = sizeImage.cy + 6 + 2 * nToolbarMarginHeight - 2;
	}

	m_btnUp.SendMessage(WM_CANCELMODE);
	m_btnDown.SendMessage(WM_CANCELMODE);

	CRect rectClient;
	GetClientRect(rectClient);

	rectClient.DeflateRect(m_nBorderSize + 1, m_nBorderSize + 1);

	m_rectWndArea = rectClient;

	int nVisibleTabsNum = GetVisibleTabsNum();

	if (bIsMode2003)
	{
		if (m_nVisiblePageButtons == -1)
		{
			m_nVisiblePageButtons = nVisibleTabsNum;
		}

		if (m_nVisiblePageButtons > nVisibleTabsNum)
		{
			// Maybe, pages were removed?
			m_nVisiblePageButtons = nVisibleTabsNum;
		}

		m_nMaxVisiblePageButtons = min(nVisibleTabsNum, (rectClient.Height() - m_nTabsHeight - nToolBarHeight) / (2 * m_nTabsHeight));
		int nVisiblePageButtons = min(m_nMaxVisiblePageButtons, m_nVisiblePageButtons);

		m_rectCaption = rectClient;
		m_rectCaption.bottom = m_rectCaption.top + afxGlobalData.GetTextHeight() + 2 * CMFCBaseTabCtrl::AFX_TAB_TEXT_MARGIN;
		m_rectCaption.top += nTopEdgeHeight - 1;

		m_rectSplitter = rectClient;
		m_rectSplitter.bottom -= nToolBarHeight + m_nTabsHeight * nVisiblePageButtons;
		m_rectSplitter.top = m_rectSplitter.bottom - nSplitterHeight;

		m_rectWndArea.top = m_rectCaption.bottom;
		m_rectWndArea.bottom = m_rectSplitter.top;
	}
	else
	{
		m_rectCaption.SetRectEmpty();
		m_rectSplitter.SetRectEmpty();

		if (nVisibleTabsNum > 1 || !IsHideSingleTab())
		{
			m_rectWndArea.DeflateRect(0, 1);
		}
	}

	int y = bIsMode2003 ? m_rectSplitter.bottom : rectClient.top;

	if (nVisibleTabsNum > 1 || !IsHideSingleTab())
	{
		for (int i = 0; i < m_iTabsNum; i ++)
		{
			CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [i];
			ASSERT_VALID(pTab);

			pTab->m_rect = rectClient;
			pTab->m_rect.top = y;
			pTab->m_rect.right++;
			pTab->m_rect.bottom = y + m_nTabsHeight;

			if (pTab->m_rect.top >= rectClient.bottom - nToolBarHeight && bIsMode2003)
			{
				pTab->m_rect.SetRectEmpty();
			}

			if (!pTab->m_bVisible)
			{
				pTab->m_rect.SetRectEmpty();
				continue;
			}

			if (m_bScrollButtons && !bIsMode2003 && (i == m_iActiveTab || i == m_iActiveTab + 1))
			{
				CRect rectScroll = pTab->m_rect;
				pTab->m_rect.right -= m_nTabsHeight;
				rectScroll.left = pTab->m_rect.right;

				if (i == m_iActiveTab)
				{
					m_btnUp.SetWindowPos(NULL, rectScroll.left, rectScroll.top, rectScroll.Width(), rectScroll.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
				}
				else
				{
					m_btnDown.SetWindowPos(NULL, rectScroll.left, rectScroll.top, rectScroll.Width(), rectScroll.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
				}
			}

			if (i == m_iActiveTab && !bIsMode2003)
			{
				m_rectWndArea.top = y + m_nTabsHeight;

				int nVisibleAfter = 0;
				for (int j = i + 1; j < m_iTabsNum; j ++)
				{
					CMFCTabInfo* pCurrTab = (CMFCTabInfo*) m_arTabs [j];
					ENSURE(pCurrTab != NULL);

					if (pCurrTab->m_bVisible)
					{
						nVisibleAfter++;
					}
				}

				y = rectClient.bottom - m_nTabsHeight * nVisibleAfter + 1;
				m_rectWndArea.bottom = y - 1;
			}
			else
			{
				y += m_nTabsHeight;
			}
		}
	}


	if (m_bScrollButtons && !bIsMode2003 && m_iActiveTab == nVisibleTabsNum - 1)
	{
		m_rectWndArea.bottom -= m_nTabsHeight;

		m_btnDown.SetWindowPos(NULL, rectClient.right - m_nTabsHeight + 1, rectClient.bottom - m_nTabsHeight + 1,
			m_nTabsHeight, m_nTabsHeight, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
	}

	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [i];
		ASSERT_VALID(pTab);

		CMFCOutlookBarPane* pOutlookPane = NULL;

		if (pTab->m_bVisible)
		{
			CDockablePaneAdapter* pWrapper = DYNAMIC_DOWNCAST(CDockablePaneAdapter, pTab->m_pWnd);
			if (pWrapper != NULL)
			{
				pOutlookPane = DYNAMIC_DOWNCAST(CMFCOutlookBarPane, pWrapper->GetWrappedWnd());
				if (pOutlookPane != NULL)
				{
					pOutlookPane->m_nSize = m_rectWndArea.Width();
					pOutlookPane->m_nMaxLen = m_rectWndArea.Height();

					if (m_bDontAdjustLayout)
					{
						pOutlookPane->m_bDontAdjustLayout = TRUE;
					}
				}
			}

			pTab->m_pWnd->SetWindowPos(NULL, m_rectWndArea.left, m_rectWndArea.top, m_rectWndArea.Width(), m_rectWndArea.Height(), SWP_NOACTIVATE | SWP_NOZORDER);

			if (pOutlookPane != NULL)
			{
				pOutlookPane->m_bDontAdjustLayout = FALSE;
			}
		}
	}

	if (nVisibleTabsNum != 0 || bIsMode2003)
	{
		RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
	}
	else
	{
		ShowWindow(SW_HIDE);
	}

	if (bIsMode2003)
	{
		m_wndToolBar.ShowWindow(SW_SHOWNOACTIVATE);
		m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.bottom - nToolBarHeight, rectClient.Width(), nToolBarHeight, SWP_NOZORDER | SWP_NOACTIVATE);
		RebuildToolBar();
	}
	else
	{
		m_wndToolBar.ShowWindow(SW_HIDE);

		m_btnUp.RedrawWindow();
		m_btnDown.RedrawWindow();

		GetParent()->RedrawWindow(NULL, NULL);
	}
}

BOOL CMFCOutlookBarTabCtrl::SetActiveTab(int iTab)
{
	ASSERT_VALID(this);

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
		return TRUE;
	}

	const BOOL bIsMode2003 = IsMode2003();

	//-------------------------------------------------------------------
	// Show active tab with animation only if tab was activated by mouse:
	//-------------------------------------------------------------------
	BOOL bAnimate = (m_iHighlighted == m_iPressed) &&(m_iHighlighted != -1) && m_bEnableAnimation && !bIsMode2003;

	CMFCOutlookBar* pOutlookBar = DYNAMIC_DOWNCAST(CMFCOutlookBar, GetParent());
	if (pOutlookBar != NULL && !pOutlookBar->OnBeforeAnimation(iTab))
	{
		bAnimate = FALSE;
	}

	if (afxGlobalData.bIsRemoteSession)
	{
		// Disable animation in Terminal Services Environment
		bAnimate = FALSE;
	}

	int iOldActiveTab = m_iActiveTab;
	CWnd* pWndOld = GetActiveWnd();

	m_iActiveTab = iTab;
	CWnd* pWndActive = GetActiveWnd();
	if (pWndActive == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	ASSERT_VALID(pWndActive);

	if (bAnimate)
	{
		CMFCTabInfo* pTabInfoNew = (CMFCTabInfo*) m_arTabs [m_iActiveTab];
		CMFCTabInfo* pTabInfoOld = (CMFCTabInfo*) m_arTabs [iOldActiveTab];

		BOOL bMoveDown = (m_iActiveTab < iOldActiveTab);

		ASSERT_VALID(pTabInfoNew);
		ASSERT_VALID(pTabInfoOld);

		CRect rectClient;
		GetClientRect(rectClient);

		CRect rectOldWnd;

		pWndOld->GetWindowRect(rectOldWnd);
		ScreenToClient(rectOldWnd);

		const int dy = bMoveDown ? 30 : -30;
		const int nSteps = abs(rectOldWnd.Height() / dy);

		//---------------------
		// Hide scroll buttons:
		//---------------------
		BOOL bScrollButtons = m_bScrollButtons && !bIsMode2003;
		BOOL bIsUp = m_btnUp.IsWindowEnabled();
		BOOL bIsDown = m_btnDown.IsWindowEnabled();

		if (bScrollButtons)
		{
			m_btnUp.ShowWindow(SW_HIDE);
			m_btnDown.ShowWindow(SW_HIDE);

			for (int i = 0; i < m_iTabsNum; i++)
			{
				CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [i];
				ASSERT_VALID(pTab);

				if (i == m_iActiveTab || i == m_iActiveTab + 1)
				{
					pTab->m_rect.right += m_nTabsHeight;
				}
			}
		}

		CRect rectOld;

		if (bMoveDown)
		{
			CMFCTabInfo* pTabInfo = (CMFCTabInfo*) m_arTabs [m_iActiveTab + 1];
			rectOld = pTabInfo->m_rect;
			rectOld.bottom = rectOld.top + dy;
		}
		else
		{
			CMFCTabInfo* pTabInfo = (CMFCTabInfo*) m_arTabs [m_iActiveTab];
			rectOld = pTabInfo->m_rect;
			rectOld.top = rectOld.bottom + dy;
		}

		ModifyStyle(WS_CLIPCHILDREN, 0, SWP_NOREDRAW);

		CClientDC dc(this);

		CFont* pOldFont = (CFont*) dc.SelectObject(&afxGlobalData.fontRegular);
		dc.SetBkMode(TRANSPARENT);

		int nStartBtnIdx = bMoveDown ? m_iActiveTab + 1 : iOldActiveTab + 1;
		int nEndBtnIdx = bMoveDown ? iOldActiveTab : m_iActiveTab;

		CRect rectRedraw = rectOldWnd;

		// we need to move all tabs between old active and new active tabs
		BOOL bPrevDisableRecalcLayout = CDockingManager::m_bDisableRecalcLayout;
		CDockingManager::m_bDisableRecalcLayout = TRUE;

		for (int i = 0; i < nSteps; i++)
		{
			bMoveDown ? rectOldWnd.top += dy : rectOldWnd.bottom += dy;

			pWndOld->SetWindowPos(NULL, rectOldWnd.left, rectOldWnd.top, rectOldWnd.Width(), rectOldWnd.Height(), SWP_NOZORDER  | SWP_NOACTIVATE);

			for (int j = nStartBtnIdx; j <= nEndBtnIdx; j++)
			{
				CMFCTabInfo* pTabInfo = (CMFCTabInfo*) m_arTabs [j];
				ENSURE(pTabInfo != NULL);

				pTabInfo->m_rect.OffsetRect(0, dy);
				DrawTabButton(dc, j, FALSE);
			}

			dc.FillRect(rectOld, &afxGlobalData.brBarFace);
			rectOld.OffsetRect(0, dy);

			Sleep(10);
		}

		if (bScrollButtons)
		{
			//------------------------
			// Restore scroll buttons:
			//------------------------
			EnableScrollButtons(TRUE, bIsUp, bIsDown);
		}

		CDockingManager::m_bDisableRecalcLayout = bPrevDisableRecalcLayout;
		dc.SelectObject(pOldFont);

		ModifyStyle(0, WS_CLIPCHILDREN, SWP_NOREDRAW);
		pWndOld->ShowWindow(SW_HIDE);

		RecalcLayout();

		if (pOutlookBar != NULL)
		{
			pOutlookBar->OnAfterAnimation(iTab);
		}

		pWndActive->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW | SWP_NOREDRAW | SWP_NOZORDER | SWP_NOACTIVATE);

		pWndActive->BringWindowToTop();
		pWndActive->RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
	}
	else
	{
		//--------------------
		// Hide active window:
		//--------------------
		if (pWndOld != NULL)
		{
			pWndOld->ShowWindow(SW_HIDE);
		}

		RecalcLayout();

		//------------------------
		// Show new active window:
		//------------------------
		pWndActive->ShowWindow(SW_SHOW);
		pWndActive->BringWindowToTop();

		//----------------------------------------------------------------------
		// Small trick: to adjust active window scroll sizes, I should change an
		// active window size twice(+1 pixel and -1 pixel):
		//----------------------------------------------------------------------
		BOOL bPrevDisableRecalcLayout = CDockingManager::m_bDisableRecalcLayout;
		CDockingManager::m_bDisableRecalcLayout = TRUE;

		pWndActive->SetWindowPos(NULL, -1, -1, m_rectWndArea.Width() + 1, m_rectWndArea.Height(), SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
		pWndActive->SetWindowPos(NULL, -1, -1, m_rectWndArea.Width(), m_rectWndArea.Height(), SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);

		CDockingManager::m_bDisableRecalcLayout = bPrevDisableRecalcLayout;
	}

	//--------------------------------------------------
	// Set text to the parent frame/docking control bar:
	//--------------------------------------------------
	if (pOutlookBar != NULL && pOutlookBar->CanSetCaptionTextToTabName()) // tabbed dock bar - redraw caption only in this case
	{
		CString strCaption;
		GetTabLabel(m_iActiveTab, strCaption);

		//-------------------------------------------------------
		// Miniframe will take the text from the tab control bar:
		//-------------------------------------------------------
		if (pOutlookBar->CanSetCaptionTextToTabName())
		{
			pOutlookBar->SetWindowText(strCaption);
		}

		CWnd* pWndToUpdate = pOutlookBar;
		if (!pOutlookBar->IsDocked())
		{
			pWndToUpdate = pOutlookBar->GetParent();
		}

		if (pWndToUpdate != NULL)
		{
			pWndToUpdate->RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
		}
	}

	//-------------
	// Redraw tabs:
	//-------------
	RedrawWindow();

	if (!bIsFirstTime)
	{
		CView* pActiveView = DYNAMIC_DOWNCAST(CView, pWndActive);
		if (pActiveView != NULL)
		{
			CFrameWnd* pFrame = AFXGetParentFrame(pActiveView);
			ASSERT_VALID(pFrame);

			pFrame->SetActiveView(pActiveView);
		}
		else
		{
			pWndActive->SetFocus();
		}
	}

	return TRUE;
}

CWnd* CMFCOutlookBarTabCtrl::FindTargetWnd(const CPoint& pt)
{
	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [i];
		ASSERT_VALID(pTab);

		if (!pTab->m_bVisible)
			continue;

		if (pTab->m_rect.PtInRect(pt))
		{
			return NULL;
		}
	}

	CWnd* pWndParent = GetParent();
	ASSERT_VALID(pWndParent);

	return pWndParent;
}

void CMFCOutlookBarTabCtrl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	RecalcLayout();
}

void CMFCOutlookBarTabCtrl::OnPaint()
{
	CPaintDC dcPaint(this); // device context for painting
	CMemDC memDC(dcPaint, this);
	CDC& dc = memDC.GetDC();

	int nVisibleTabsNum = GetVisibleTabsNum();

	CRect rectClient;
	GetClientRect(rectClient);

	dc.FillRect(rectClient, &afxGlobalData.brBarFace);

	//-------------
	// Draw border:
	//-------------
	if (m_nBorderSize > 0)
	{
		CBrush* pOldBrush = dc.SelectObject(&afxGlobalData.brBarFace);
		ENSURE(pOldBrush != NULL);

		dc.PatBlt(rectClient.left, rectClient.top, m_nBorderSize, rectClient.Height(), PATCOPY);
		dc.PatBlt(rectClient.left, rectClient.top, rectClient.Width(), m_nBorderSize, PATCOPY);
		dc.PatBlt(rectClient.right - m_nBorderSize - 1, rectClient.top, m_nBorderSize + 1, rectClient.Height(), PATCOPY);
		dc.PatBlt(rectClient.left, rectClient.bottom - m_nBorderSize, rectClient.Width(), m_nBorderSize, PATCOPY);

		dc.SelectObject(pOldBrush);

		rectClient.DeflateRect(m_nBorderSize, m_nBorderSize);
	}

	dc.Draw3dRect(rectClient, afxGlobalData.clrBarShadow, afxGlobalData.clrBarShadow);

	CPen penDrak(PS_SOLID, 1, afxGlobalData.clrBarShadow);
	CPen* pOldPen = (CPen*) dc.SelectObject(&penDrak);
	ENSURE(pOldPen != NULL);

	dc.MoveTo(m_rectWndArea.left - 1, m_rectWndArea.bottom);
	dc.LineTo(m_rectWndArea.right + 1, m_rectWndArea.bottom);

	CMFCOutlookBar* pOutlookBar = DYNAMIC_DOWNCAST(CMFCOutlookBar, GetParent());

	CFont* pOldFont = (CFont*) dc.SelectObject( pOutlookBar != NULL && pOutlookBar->GetButtonsFont() != NULL ? pOutlookBar->GetButtonsFont() : &afxGlobalData.fontRegular);
	dc.SetBkMode(TRANSPARENT);

	if (nVisibleTabsNum > 1 || !IsHideSingleTab())
	{
		for (int i = 0; i < m_iTabsNum; i ++)
		{
			DrawTabButton(dc, i);
		}
	}

	if (!m_rectCaption.IsRectEmpty())
	{
		// Draw caption:
		CRect rectTop = m_rectCaption;
		rectTop.right++;

		rectTop.top -= nTopEdgeHeight + 1;
		rectTop.bottom = rectTop.top + nTopEdgeHeight + 1;

		dc.FillRect(rectTop, &afxGlobalData.brBarFace);

		COLORREF clrText = afxGlobalData.clrBarText;
		CMFCVisualManager::GetInstance()->OnFillOutlookBarCaption(&dc, m_rectCaption, clrText);

		CString strActivePage;
		GetTabLabel(m_iActiveTab, strActivePage);

		CRect rcText = m_rectCaption;
		rcText.DeflateRect(CMFCBaseTabCtrl::AFX_TAB_TEXT_MARGIN, 0);

		UINT uiDTFlags = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS;

		dc.SetTextColor(clrText);
		dc.DrawText(strActivePage, rcText, uiDTFlags);
	}

	if (!m_rectSplitter.IsRectEmpty())
	{
		// Draw splitter:
		CMFCVisualManager::GetInstance()->OnDrawOutlookBarSplitter(&dc, m_rectSplitter);
	}

	// Draw scroll buttons:
	if (m_bScrollButtons && !IsMode2003())
	{
		if (m_iActiveTab == m_iTabsNum - 1)
		{
			CRect rectFill = rectClient;
			rectFill.top = rectFill.bottom - m_nTabsHeight;

			dc.FillRect(rectFill, &afxGlobalData.brBarFace);
		}
	}

	dc.SelectObject(pOldPen);
	dc.SelectObject(pOldFont);
}

void CMFCOutlookBarTabCtrl::DrawTabButton(CDC& dc, int iButtonIdx, BOOL bDrawPressedButton)
{
	CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [iButtonIdx];
	ASSERT_VALID(pTab);

	CRect rectBtn = pTab->m_rect;

	if (rectBtn.IsRectEmpty())
	{
		return;
	}

	BOOL bIsHighlighted = (iButtonIdx == m_iHighlighted);
	BOOL bIsPressed = (iButtonIdx == m_iPressed) && bDrawPressedButton;
	BOOL bIsActive = (iButtonIdx == m_iActiveTab);

	if (IsMode2003() && bIsActive)
	{
		bIsPressed = TRUE;
	}

	COLORREF clrBtnText = afxGlobalData.clrBarText;

	CMFCVisualManager::GetInstance()->OnFillOutlookPageButton(&dc, rectBtn, bIsHighlighted, bIsPressed, clrBtnText);

	CMFCVisualManager::GetInstance()->OnDrawOutlookPageButtonBorder(&dc, rectBtn, bIsHighlighted, bIsPressed);

	//---------------
	// Draw tab icon:
	//---------------
	CSize sizeImage = GetImageSize();
	UINT uiIcon = GetTabIcon(iButtonIdx);
	HICON hIcon = GetTabHicon(iButtonIdx);

	if (uiIcon == (UINT)-1 && hIcon == NULL)
	{
		sizeImage.cx = 0;
	}

	if (sizeImage.cx + CMFCBaseTabCtrl::AFX_TAB_IMAGE_MARGIN <= rectBtn.Width())
	{
		CRect rectImage = rectBtn;

		rectImage.top += (rectBtn.Height() - sizeImage.cy) / 2;
		rectImage.bottom = rectImage.top + sizeImage.cy;

		rectImage.left += AFX_IMAGE_MARGIN;
		rectImage.right = rectImage.left + sizeImage.cx;

		if (hIcon != NULL)
		{
			//---------------------
			// Draw the tab's icon:
			//---------------------
			dc.DrawState(rectImage.TopLeft(), rectImage.Size(), hIcon, DSS_NORMAL, (HBRUSH) NULL);
		}
		else
		{
			const CImageList* pImageList = GetImageList();
			if (pImageList != NULL && uiIcon != (UINT)-1)
			{
				ASSERT_VALID(pImageList);

				//----------------------
				// Draw the tab's image:
				//----------------------
				((CImageList*)pImageList)->Draw(&dc, uiIcon, rectImage.TopLeft(), ILD_TRANSPARENT);
			}
		}
	}

#define AFX_TEXT_MARGIN 4
#define AFX_GRIPPER_MARGIN 4

	//---------------
	// Draw tab text:
	//---------------
	dc.SetTextColor(clrBtnText);

	CRect rcText = pTab->m_rect;

	if (pTab->m_bIsDetachable && !IsMode2003())
	{
		rcText.right -= AFX_CX_GRIPPER + AFX_GRIPPER_MARGIN * 2;
	}

	rcText.left += sizeImage.cx + 2 * AFX_TEXT_MARGIN;

	UINT uiDTFlags = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS;

	if (IsMode2003())
	{
		uiDTFlags |= DT_LEFT;
	}
	else
	{
		switch(m_nPageButtonTextAlign)
		{
		case TA_LEFT:
			uiDTFlags |= DT_LEFT;
			break;

		case TA_CENTER:
		default:
			uiDTFlags |= DT_CENTER;
			break;

		case TA_RIGHT:
			uiDTFlags |= DT_RIGHT;
			break;
		}
	}

	dc.DrawText(pTab->m_strText, rcText, uiDTFlags);

	if (pTab->m_bIsDetachable && !IsMode2003())
	{
		//--------------
		// Draw gripper:
		//--------------
		CRect rectGripper = pTab->m_rect;
		rectGripper.left = rcText.right;
		rectGripper.DeflateRect(AFX_GRIPPER_MARGIN, 2);

		CBasePane bar;
		CMFCVisualManager::GetInstance()->OnDrawBarGripper(&dc, rectGripper, TRUE, &bar);
	}
}

void CMFCOutlookBarTabCtrl::GetTabArea(CRect& rectTabAreaTop, CRect& rectTabAreaBottom) const
{
	rectTabAreaTop.SetRectEmpty();
	rectTabAreaBottom.SetRectEmpty();

	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [i];
		ASSERT_VALID(pTab);

		CRect rectBtn = pTab->m_rect;

		//--------------------------------
		// The first tab is always at top:
		//--------------------------------
		if (i == 0)
		{
			rectTabAreaTop = pTab->m_rect;
			continue;
		}

		if (rectTabAreaTop.bottom == pTab->m_rect.top)
		{
			rectTabAreaTop.bottom += pTab->m_rect.Height();
		}
		else if (rectTabAreaBottom.IsRectEmpty())
		{
			rectTabAreaBottom = pTab->m_rect;
		}
		else
		{
			rectTabAreaBottom.bottom += pTab->m_rect.Height();
		}
	}
	ClientToScreen(rectTabAreaTop);
	ClientToScreen(rectTabAreaBottom);
}

void CMFCOutlookBarTabCtrl::SetBorderSize(int nBorderSize)
{
	ASSERT_VALID(this);

	m_nBorderSize = nBorderSize;
	RecalcLayout();
}

BOOL CMFCOutlookBarTabCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint ptCursor;
	::GetCursorPos(&ptCursor);
	ScreenToClient(&ptCursor);

	if (m_rectSplitter.PtInRect(ptCursor))
	{
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENS));
		return TRUE;
	}

	if (GetTabFromPoint(ptCursor) >= 0)
	{
		::SetCursor(afxGlobalData.GetHandCursor());
		return TRUE;
	}

	return CMFCBaseTabCtrl::OnSetCursor(pWnd, nHitTest, message);
}

int CMFCOutlookBarTabCtrl::GetTabNumberToDetach(int nTabNum) const
{
	return(nTabNum == -1 ? m_iPressed : nTabNum);
}

BOOL CMFCOutlookBarTabCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

DROPEFFECT CMFCOutlookBarTabCtrl::OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	return OnDragOver(pDataObject, dwKeyState, point);
}

DROPEFFECT CMFCOutlookBarTabCtrl::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	CMFCToolBarButton* pButton = CMFCToolBarButton::CreateFromOleData(pDataObject);
	if (pButton == NULL)
	{
		return DROPEFFECT_NONE;
	}

	if (!pButton->IsKindOf(RUNTIME_CLASS(CMFCOutlookBarPaneButton)))
	{
		delete pButton;
		return DROPEFFECT_NONE;
	}

	delete pButton;

	int nTab = GetTabFromPoint(point);
	if (nTab < 0)
	{
		return DROPEFFECT_NONE;
	}

	SetActiveTab(nTab);
	BOOL bCopy = (dwKeyState & MK_CONTROL);

	return(bCopy) ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
}

void __stdcall CMFCOutlookBarTabCtrl::EnableAnimation(BOOL bEnable)
{
	m_bEnableAnimation = bEnable;
}

void CMFCOutlookBarTabCtrl::EnableScrollButtons(BOOL bEnable/* = TRUE*/, BOOL bIsUp/* = TRUE*/, BOOL bIsDown/* = TRUE*/)
{
	ASSERT_VALID(this);

	if (IsMode2003())
	{
		bEnable = FALSE;
	}

	BOOL bRecalcLayout = m_bScrollButtons != bEnable;

	m_bScrollButtons = bEnable;

	if (m_bScrollButtons)
	{
		m_btnUp.ShowWindow(SW_SHOWNOACTIVATE);
		m_btnUp.EnableWindow(bIsUp);
		m_btnUp.SetStdImage(CMenuImages::IdArrowUpLarge, bIsUp ? CMenuImages::ImageBlack : CMenuImages::ImageGray);

		m_btnDown.ShowWindow(SW_SHOWNOACTIVATE);
		m_btnDown.EnableWindow(bIsDown);
		m_btnDown.SetStdImage(CMenuImages::IdArrowDownLarge, bIsDown ? CMenuImages::ImageBlack : CMenuImages::ImageGray);
	}
	else
	{
		m_btnUp.ShowWindow(SW_HIDE);
		m_btnDown.ShowWindow(SW_HIDE);
	}

	m_btnUp.RedrawWindow();
	m_btnDown.RedrawWindow();

	if (bRecalcLayout)
	{
		RecalcLayout();
	}
}

int CMFCOutlookBarTabCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMFCBaseTabCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	//-----------------------
	// Create scroll buttons:
	//-----------------------
	CRect rectDummy(0, 0, 0, 0);

	m_btnUp.Create(_T(""), WS_CHILD | BS_PUSHBUTTON, rectDummy, this, (UINT)-1);
	m_btnUp.SetStdImage(CMenuImages::IdArrowUpLarge);
	m_btnUp.SetAutorepeatMode(100);

	m_btnDown.Create(_T(""), WS_CHILD | BS_PUSHBUTTON, rectDummy, this, (UINT)-1);
	m_btnDown.SetStdImage(CMenuImages::IdArrowDownLarge);
	m_btnDown.SetAutorepeatMode(100);

	m_wndToolBar.m_bLargeIconsAreEnbaled = FALSE;

	m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, AFX_DEFAULT_TOOLBAR_STYLE, CRect(0, 0, 0, 0));
	m_wndToolBar.SetOwner(this);
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetHotBorder(FALSE);

	return 0;
}

BOOL CMFCOutlookBarTabCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	HWND hwnd = (HWND) lParam;

	CMFCOutlookBar* pOutlookBar = DYNAMIC_DOWNCAST(CMFCOutlookBar, GetParent());

	if (pOutlookBar != NULL)
	{
		if (m_btnUp.GetSafeHwnd() == hwnd)
		{
			pOutlookBar->OnScroll(FALSE);

			if (!m_btnUp.IsWindowEnabled())
			{
				SetFocus();
			}

			return TRUE;
		}

		if (m_btnDown.GetSafeHwnd() == hwnd)
		{
			pOutlookBar->OnScroll(TRUE);

			if (!m_btnDown.IsWindowEnabled())
			{
				SetFocus();
			}

			return TRUE;
		}
	}

	return CMFCBaseTabCtrl::OnCommand(wParam, lParam);
}

void CMFCOutlookBarTabCtrl::SetPageButtonTextAlign(UINT uiAlign, BOOL bRedraw/* = TRUE*/)
{
	m_nPageButtonTextAlign = uiAlign;

	if (bRedraw && GetSafeHwnd() != NULL)
	{
		RedrawWindow();
	}
}

BOOL CMFCOutlookBarTabCtrl::IsMode2003() const
{
	ASSERT_VALID(this);

	CMFCOutlookBar* pOutlookBar = DYNAMIC_DOWNCAST(CMFCOutlookBar, GetParent());
	return pOutlookBar != NULL && pOutlookBar->IsMode2003();
}

BOOL CMFCOutlookBarTabCtrl::IsTabDetachable(int iTab) const
{
	ASSERT_VALID(this);

	if (IsMode2003())
	{
		return FALSE;
	}

	return CMFCBaseTabCtrl::IsTabDetachable(iTab);
}

void CMFCOutlookBarTabCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_rectSplitter.PtInRect(point))
	{
		m_bIsTracking = TRUE;
		SetCapture();
		return;
	}

	CMFCBaseTabCtrl::OnLButtonDown(nFlags, point);
}

void CMFCOutlookBarTabCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bIsTracking)
	{
		ReleaseCapture();
		m_bIsTracking = FALSE;
	}

	CMFCBaseTabCtrl::OnLButtonUp(nFlags, point);
}

void CMFCOutlookBarTabCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_bIsTracking)
	{
		CMFCBaseTabCtrl::OnMouseMove(nFlags, point);
		return;
	}

	if (m_nTabsHeight == 0 || m_nVisiblePageButtons == -1)
	{
		return;
	}

	int nDelta = (m_rectSplitter.top - point.y) / m_nTabsHeight;
	if (nDelta == 0)
	{
		return;
	}

	int nVisiblePageButtonsPrev = m_nVisiblePageButtons;

	m_nVisiblePageButtons += nDelta;

	m_nVisiblePageButtons = min(GetVisibleTabsNum(), max(0, m_nVisiblePageButtons));

	if (nVisiblePageButtonsPrev != m_nVisiblePageButtons)
	{
		m_bDontAdjustLayout = TRUE;
		RecalcLayout();
		m_bDontAdjustLayout = FALSE;

		point.y = m_rectSplitter.CenterPoint().y;
		ClientToScreen(&point);

		::SetCursorPos(point.x, point.y);
	}
}

void CMFCOutlookBarTabCtrl::OnCancelMode()
{
	CMFCBaseTabCtrl::OnCancelMode();

	if (m_bIsTracking)
	{
		ReleaseCapture();
		m_bIsTracking = FALSE;
	}
}

void CMFCOutlookBarTabCtrl::RebuildToolBar()
{
	ASSERT_VALID(this);

	if (!IsMode2003())
	{
		return;
	}

	m_wndToolBar.RemoveAllButtons();
	m_wndToolBar.m_TabButtons.RemoveAll();

	m_wndToolBar.EnableCustomizeButton(TRUE, 0, _T(""), FALSE);

	CSize sizeImage(0, 0);

	if (m_imagesToolbar.GetSafeHandle() != NULL)
	{
		sizeImage = m_sizeToolbarImage;
	}
	else
	{
		sizeImage = GetImageSize();
	}

	if (sizeImage == CSize(0, 0))
	{
		sizeImage = CSize(16, 16);
	}

	CSize sizeButton = sizeImage + CSize(6, 6 + 2 * nToolbarMarginHeight);
	m_wndToolBar.SetLockedSizes(sizeButton, sizeImage);
	m_wndToolBar.m_ImagesLocked.Clear();
	m_wndToolBar.m_ImagesLocked.SetImageSize(sizeImage);

	if (m_wndToolBar.m_pCustomizeBtn != NULL)
	{
		COutlookCustomizeButton customizeButton;
		customizeButton.CopyFrom(*m_wndToolBar.m_pCustomizeBtn);

		customizeButton.SetPipeStyle(FALSE);
		customizeButton.SetMenuRightAlign(FALSE);
		customizeButton.m_bShowAtRightSide = TRUE;
		customizeButton.SetMessageWnd(this);

		m_wndToolBar.m_Buttons.RemoveHead();
		delete m_wndToolBar.m_pCustomizeBtn;
		m_wndToolBar.m_pCustomizeBtn = NULL;

		m_wndToolBar.InsertButton(customizeButton);

		m_wndToolBar.m_pCustomizeBtn = (CMFCCustomizeButton*) m_wndToolBar.m_Buttons.GetHead();
	}

	int nButtonNum = 0;
	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CMFCTabInfo* pTab = (CMFCTabInfo*) m_arTabs [i];
		ASSERT_VALID(pTab);

		if (pTab->m_bVisible && pTab->m_rect.IsRectEmpty())
		{
			CMFCToolBarButton button(idToolbarCommandID + nButtonNum, nButtonNum, pTab->m_strText);
			m_wndToolBar.InsertButton(button);

			m_wndToolBar.m_TabButtons.SetAt(nButtonNum, i);

			HICON hIcon = NULL;
			UINT uiIcon = GetTabIcon(i);

			if (m_imagesToolbar.GetSafeHandle() != NULL)
			{
				hIcon = m_imagesToolbar.ExtractIcon(uiIcon);
			}
			else
			{
				hIcon = GetTabHicon(i);

				if (hIcon == NULL)
				{
					CImageList* pImageList = (CImageList*) GetImageList();

					if (pImageList != NULL && uiIcon != (UINT)-1)
					{
						hIcon = pImageList->ExtractIcon(uiIcon);
					}
				}
			}

			m_wndToolBar.m_ImagesLocked.AddIcon(hIcon);
			nButtonNum++;
		}
	}

	m_wndToolBar.AdjustLocations();
	m_wndToolBar.RedrawWindow();
}

void CMFCOutlookBarTabCtrl::OnShowMorePageButtons()
{
	m_nVisiblePageButtons++;

	m_bDontAdjustLayout = TRUE;
	RecalcLayout();
	m_bDontAdjustLayout = FALSE;
}

void CMFCOutlookBarTabCtrl::OnShowFewerPageButtons()
{
	m_nVisiblePageButtons--;

	m_bDontAdjustLayout = TRUE;
	RecalcLayout();
	m_bDontAdjustLayout = FALSE;
}

BOOL CMFCOutlookBarTabCtrl::CanShowMorePageButtons() const
{
	return m_nVisiblePageButtons < m_nMaxVisiblePageButtons;
}

BOOL CMFCOutlookBarTabCtrl::CanShowFewerPageButtons() const
{
	return m_nVisiblePageButtons > 0;
}

void CMFCOutlookBarTabCtrl::OnChangeTabs()
{
	// Will be recalculated in the next RecalcLayout()
	m_nVisiblePageButtons = -1;
}

BOOL CMFCOutlookBarTabCtrl::SetToolbarImageList(UINT uiID, int cx, COLORREF clrTransp)
{
	if (!IsMode2003())
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiID))
	{
		TRACE(_T("CMFCOutlookBarTabCtrl::SetToolbarImageList Can't load bitmap: %x\n"), uiID);
		return FALSE;
	}

	if (m_imagesToolbar.GetSafeHandle() != NULL)
	{
		m_imagesToolbar.DeleteImageList();
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

	m_imagesToolbar.Create(cx, bmpObj.bmHeight, nFlags, 0, 0);
	m_imagesToolbar.Add(&bmp, clrTransp);

	m_sizeToolbarImage = CSize(cx, bmpObj.bmHeight);

	RecalcLayout();
	return TRUE;
}

void CMFCOutlookBarTabCtrl::OnToolbarCommand(UINT id)
{
	switch(id)
	{
	case idShowMoreButtons:
		OnShowMorePageButtons();
		break;

	case idShowFewerButtons:
		OnShowFewerPageButtons();
		break;

	case idNavigationPaneOptions:
		OnShowOptions();
		break;
	}
}

void CMFCOutlookBarTabCtrl::OnUpdateToolbarCommand(CCmdUI* pCmdUI)
{
	switch(pCmdUI->m_nID)
	{
	case idShowMoreButtons:
		pCmdUI->Enable(CanShowMorePageButtons());
		break;

	case idShowFewerButtons:
		pCmdUI->Enable(CanShowFewerPageButtons());
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
// COutlookOptionsDlg dialog

class COutlookOptionsDlg : public CDialog
{
	// Construction
public:
	COutlookOptionsDlg(CMFCOutlookBarTabCtrl& parentBar);   // standard constructor

	// Dialog Data
	//{{AFX_DATA(COutlookOptionsDlg)
	enum { IDD = IDD_AFXBARRES_OUTLOOKBAR_OPTIONS };
	CButton m_btnMoveUp;
	CButton m_wndMoveDown;
	CButton m_wndReset;
	CMFCToolBarsListCheckBox m_wndList;
	//}}AFX_DATA


	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COutlookOptionsDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COutlookOptionsDlg)
	afx_msg void OnSelchange();
	afx_msg void OnDblclkList();
	afx_msg void OnMoveDown();
	afx_msg void OnMoveUp();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnReset();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CMFCOutlookBarTabCtrl& m_parentBar;

	void MoveItem(BOOL bMoveUp);
};

COutlookOptionsDlg::COutlookOptionsDlg(CMFCOutlookBarTabCtrl& parentBar)
	: CDialog(COutlookOptionsDlg::IDD, &parentBar), m_parentBar(parentBar)
{
}

void COutlookOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COutlookOptionsDlg)
	DDX_Control(pDX, IDC_AFXBARRES_MOVEUP, m_btnMoveUp);
	DDX_Control(pDX, IDC_AFXBARRES_MOVEDOWN, m_wndMoveDown);
	DDX_Control(pDX, IDC_AFXBARRES_LIST, m_wndList);
	DDX_Control(pDX, IDC_AFXBARRES_RESET, m_wndReset);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COutlookOptionsDlg, CDialog)
	//{{AFX_MSG_MAP(COutlookOptionsDlg)
	ON_LBN_SELCHANGE(IDC_AFXBARRES_LIST, &COutlookOptionsDlg::OnSelchange)
	ON_LBN_DBLCLK(IDC_AFXBARRES_LIST, &COutlookOptionsDlg::OnDblclkList)
	ON_BN_CLICKED(IDC_AFXBARRES_MOVEDOWN, &COutlookOptionsDlg::OnMoveDown)
	ON_BN_CLICKED(IDC_AFXBARRES_MOVEUP, &COutlookOptionsDlg::OnMoveUp)
	ON_BN_CLICKED(IDC_AFXBARRES_RESET, &COutlookOptionsDlg::OnReset)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COutlookOptionsDlg message handlers

void COutlookOptionsDlg::OnSelchange()
{
	m_btnMoveUp.EnableWindow(m_wndList.GetCurSel() > 0);
	m_wndMoveDown.EnableWindow(m_wndList.GetCurSel() < m_wndList.GetCount() - 1);
}

void COutlookOptionsDlg::OnDblclkList()
{
	int nSel = m_wndList.GetCurSel();
	if (nSel >= 0)
	{
		m_wndList.SetCheck(nSel, !m_wndList.GetCheck(nSel));
	}
}

void COutlookOptionsDlg::OnMoveDown()
{
	MoveItem(FALSE);
}

void COutlookOptionsDlg::OnMoveUp()
{
	MoveItem(TRUE);
}

BOOL COutlookOptionsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (AfxGetMainWnd() != NULL && (AfxGetMainWnd()->GetExStyle() & WS_EX_LAYOUTRTL))
	{
		ModifyStyleEx(0, WS_EX_LAYOUTRTL);
	}

	for (int i = 0; i < m_parentBar.m_iTabsNum; i ++)
	{
		CString str;
		m_parentBar.GetTabLabel(i, str);

		int nIndex = m_wndList.AddString(str);

		m_wndList.SetItemData(nIndex, (DWORD_PTR) i);
		m_wndList.SetCheck(nIndex, m_parentBar.IsTabVisible(i));
	}

	m_wndList.SetCurSel(0);
	OnSelchange();

	CMFCOutlookBar* pOutlookBar = DYNAMIC_DOWNCAST(CMFCOutlookBar, m_parentBar.GetParent());
	if (pOutlookBar == NULL)
	{
		m_wndReset.EnableWindow(FALSE);
		m_wndReset.ShowWindow(SW_HIDE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
}

void COutlookOptionsDlg::OnOK()
{
	CArray<int, int> arTabsOrder;

	for (int nIndex = 0; nIndex < m_wndList.GetCount(); nIndex++)
	{
		int i = (int) m_wndList.GetItemData(nIndex);

		BOOL bVisible = m_wndList.GetCheck(nIndex);

		if (bVisible != m_parentBar.IsTabVisible(i))
		{
			m_parentBar.ShowTab(i, bVisible, FALSE);
		}

		arTabsOrder.Add(i);
	}

	m_parentBar.SetTabsOrder(arTabsOrder);

	CDialog::OnOK();
}

void COutlookOptionsDlg::OnReset()
{
	CMFCOutlookBar* pOutlookBar = DYNAMIC_DOWNCAST(CMFCOutlookBar, m_parentBar.GetParent());
	if (pOutlookBar == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	CArray<int, int> arTabsOrder;
	int i = 0;

	for (i = 0; i < pOutlookBar->GetDefaultTabsOrder().GetSize(); i++)
	{
		int iTabID = pOutlookBar->GetDefaultTabsOrder() [i];
		int iTab = m_parentBar.GetTabByID(iTabID);

		if (iTab < 0)
		{
			ASSERT(FALSE);
			return;
		}

		arTabsOrder.Add(iTab);
	}

	m_wndList.ResetContent();

	for (i = 0; i < arTabsOrder.GetSize(); i ++)
	{
		int iTabNum = arTabsOrder [i];

		CString str;
		m_parentBar.GetTabLabel(iTabNum, str);

		int nIndex = m_wndList.AddString(str);

		m_wndList.SetItemData(nIndex, (DWORD_PTR) iTabNum);
		m_wndList.SetCheck(nIndex, TRUE);
	}

	m_wndList.SetCurSel(0);
	OnSelchange();
}

void COutlookOptionsDlg::MoveItem(BOOL bMoveUp)
{
	int nSel = m_wndList.GetCurSel();

	CString str;
	m_wndList.GetText(nSel, str);
	DWORD_PTR dwData = m_wndList.GetItemData(nSel);
	BOOL bCheck = m_wndList.GetCheck(nSel);

	m_wndList.DeleteString(nSel);

	int nNewIndex = bMoveUp ? nSel - 1 : nSel + 1;

	int nIndex = m_wndList.InsertString(nNewIndex, str);

	m_wndList.SetItemData(nIndex, dwData);
	m_wndList.SetCheck(nIndex, bCheck);

	m_wndList.SetCurSel(nIndex);
	OnSelchange();
}

void CMFCOutlookBarTabCtrl::OnShowOptions()
{
	COutlookOptionsDlg dlg(*this);
	if (dlg.DoModal() == IDOK)
	{
		m_bDontAdjustLayout = TRUE;
		RecalcLayout();
		m_bDontAdjustLayout = FALSE;
	}
}



