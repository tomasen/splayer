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

#include "afxwinappex.h"
#include "afxribbonres.h"
#include "afxvisualmanager.h"
#include "afxcontextmenumanager.h"
#include "afxtaskspane.h"
#include "afxtaskspaneframewnd.h"
#include "afxmultipaneframewnd.h"
#include "afxregpath.h"
#include "afxsettingsstore.h"
#include "afxtoolbarmenubutton.h"
#include "afxpopupmenu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const CString strTasksPaneProfile = _T("MFCTasksPanes");
#define AFX_REG_SECTION_FMT _T("%sMFCTasksPane-%d")
#define AFX_REG_SECTION_FMT_EX _T("%sMFCTasksPane-%d%x")
#define AFX_REG_ENTRY_SETTINGS _T("Settings")

#define AFX_ID_SCROLL_VERT 1

static const int nBorderSize = 1;
static const int nNavToolbarId = 1;
static const int nAnimTimerId = AFX_ID_CHECK_AUTO_HIDE_CONDITION + 1;
static const int nScrollTimerId = AFX_ID_CHECK_AUTO_HIDE_CONDITION + 2;

static inline BOOL __stdcall IsSystemCommand(UINT uiCmd)
{
	return(uiCmd >= 0xF000 && uiCmd < 0xF1F0);
}

BOOL CMFCTasksPanePropertyPage::SetACCData(CWnd* /*pParent*/, CAccessibilityData& data)
{
	ASSERT_VALID(this);

	data.Clear();
	data.m_strAccName = m_strName;
	data.m_nAccRole = ROLE_SYSTEM_PAGETAB ;
	data.m_bAccState = STATE_SYSTEM_NORMAL;
	data.m_nAccHit = 1;
	data.m_bAccState = STATE_SYSTEM_DEFAULT;

	return TRUE;
}

BOOL CMFCTasksPaneTaskGroup::SetACCData(CWnd* pParent, CAccessibilityData& data)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pParent);

	data.Clear();
	data.m_strAccName = m_strName;
	data.m_nAccRole = ROLE_SYSTEM_GROUPING;
	data.m_bAccState = STATE_SYSTEM_NORMAL;
	data.m_nAccHit = 1;
	data.m_rectAccLocation = m_rect;
	pParent->ClientToScreen(&data.m_rectAccLocation);

	return TRUE;
}

BOOL CMFCTasksPaneTask::SetACCData(CWnd* pParent, CAccessibilityData& data)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pParent);

	data.Clear();
	data.m_strAccName = m_strName;
	data.m_strAccValue = m_strName;
	data.m_nAccRole = ROLE_SYSTEM_LINK;
	data.m_bAccState = STATE_SYSTEM_FOCUSABLE;

	if (!m_bEnabled)
	{
		data.m_bAccState |= STATE_SYSTEM_UNAVAILABLE;
	}

	data.m_nAccHit = 1;
	data.m_strAccDefAction = _T("Press");
	data.m_rectAccLocation = m_rect;
	pParent->ClientToScreen(&data.m_rectAccLocation);
	data.m_ptAccHit;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMFCTasksPane additional classes: CTasksPaneNavigateButton

class CTasksPaneNavigateButton : public CMFCToolBarButton
{
	friend class CMFCTasksPane;
	DECLARE_SERIAL(CTasksPaneNavigateButton)

protected:
	CTasksPaneNavigateButton(int iImage = -1) : CMFCToolBarButton()
	{
		m_iImage = iImage;
		m_bLocked = TRUE;
	}

	CTasksPaneNavigateButton(UINT uiID, int iImage, LPCTSTR lpszText = NULL) : CMFCToolBarButton(uiID, iImage, lpszText, FALSE, TRUE)
	{
	}
};

IMPLEMENT_SERIAL(CTasksPaneNavigateButton, CMFCToolBarButton, 1)

/////////////////////////////////////////////////////////////////////////////
// CTasksPaneHistoryButton

class CTasksPaneHistoryButton : public CMFCToolBarMenuButton
{
	friend class CMFCTasksPane;
	DECLARE_SERIAL(CTasksPaneHistoryButton)

public:
	CTasksPaneHistoryButton(int iImage = -1) : CMFCToolBarMenuButton()
	{
		m_iImage = iImage;
		m_bLocked = TRUE;

		m_pParentBar = NULL;
	}

	CTasksPaneHistoryButton(UINT uiID, int iImage, LPCTSTR lpszText = NULL, BOOL bUserButton = FALSE) : CMFCToolBarMenuButton()
	{
		m_nID = uiID;
		m_bUserButton = bUserButton;

		SetImage(iImage);
		m_strText = (lpszText == NULL) ? _T("") : lpszText;

		CMenu menu;
		menu.CreatePopupMenu();
		CreateFromMenu(menu.GetSafeHmenu());

		m_pParentBar = NULL;
	}

	virtual void OnChangeParentWnd(CWnd* pWndParent)
	{
		CMFCToolBarMenuButton::OnChangeParentWnd(pWndParent);
		m_pParentBar = DYNAMIC_DOWNCAST(CMFCTasksPane, pWndParent);
	}

	void UpdateMenu()
	{
		if (m_pParentBar == NULL)
		{
			return;
		}

		if (m_nID == ID_AFXBARRES_TASKPANE_BACK)
		{
			m_pParentBar->GetPreviousPages(m_lstPages);
		}
		else if (m_nID == ID_AFXBARRES_TASKPANE_FORWARD)
		{
			m_pParentBar->GetNextPages(m_lstPages);
		}

		CMenu menu;
		menu.CreatePopupMenu();

		for (POSITION pos = m_lstPages.GetHeadPosition(); pos != NULL; )
		{
			CString& strPageName = m_lstPages.GetNext(pos);
			menu.AppendMenu(MF_STRING, m_nID, strPageName);
		}

		CreateFromMenu(menu.GetSafeHmenu());
	}

	// data:
	CMFCTasksPane* m_pParentBar;
	CStringList m_lstPages; // pages history
};

IMPLEMENT_SERIAL(CTasksPaneHistoryButton, CMFCToolBarMenuButton, 1)

/////////////////////////////////////////////////////////////////////////////
// CTasksPaneMenuButton

class CTasksPaneMenuButton : public CMFCToolBarMenuButton
{
	friend class CMFCTasksPane;
	DECLARE_SERIAL(CTasksPaneMenuButton)

public:
	CTasksPaneMenuButton(HMENU hMenu = NULL) : CMFCToolBarMenuButton((UINT)-1, hMenu, -1)
	{
		m_pParentBar = NULL;
	}

	virtual HMENU CreateMenu() const
	{
		if (m_pParentBar == NULL)
		{
			return NULL;
		}

		ASSERT_VALID(m_pParentBar);

		return m_pParentBar->CreateMenu();
	}

	virtual CMFCPopupMenu* CreatePopupMenu()
	{
		CMFCPopupMenu* pMenu = CMFCToolBarMenuButton::CreatePopupMenu();
		if (pMenu == NULL)
		{
			ASSERT(FALSE);
			return NULL;
		}

		pMenu->SetRightAlign(TRUE);
		return pMenu;
	}

	virtual void OnChangeParentWnd(CWnd* pWndParent)
	{
		CMFCToolBarMenuButton::OnChangeParentWnd(pWndParent);
		m_pParentBar = DYNAMIC_DOWNCAST(CMFCTasksPane, pWndParent);
	}

	// data:
	CMFCTasksPane* m_pParentBar;
};

IMPLEMENT_SERIAL(CTasksPaneMenuButton, CMFCToolBarMenuButton, 1)


/////////////////////////////////////////////////////////////////////////////
// CMFCTasksPaneToolBar

IMPLEMENT_SERIAL(CMFCTasksPaneToolBar, CMFCToolBar, 1)

//{{AFX_MSG_MAP(CMFCTasksPaneToolBar)
BEGIN_MESSAGE_MAP(CMFCTasksPaneToolBar, CMFCToolBar)
	ON_MESSAGE(WM_IDLEUPDATECMDUI, &CMFCTasksPaneToolBar::OnIdleUpdateCmdUI)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

LRESULT CMFCTasksPaneToolBar::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM)
{
	// the style must be visible and if it is docked
	// the dockbar style must also be visible
	if (GetStyle() & WS_VISIBLE)
	{
		OnUpdateCmdUI((CFrameWnd*) GetOwner(), (BOOL)wParam);
	}

	return 0L;
}

void CMFCTasksPaneToolBar::AdjustLocations()
{
	ASSERT_VALID(this);

	if (GetSafeHwnd() == NULL || !::IsWindow(m_hWnd))
	{
		return;
	}

	CMFCToolBar::AdjustLocations();

	//----------------------------------
	// Get menu button and close button:
	//----------------------------------
	CTasksPaneNavigateButton* pCloseBtn = NULL;
	CTasksPaneMenuButton* pMenuBtn = NULL;

	for (POSITION pos = m_Buttons.GetHeadPosition(); pos != NULL; )
	{
		CMFCToolBarButton* pButton = (CMFCToolBarButton*) m_Buttons.GetNext(pos);
		ASSERT_VALID(pButton);

		if (pButton->m_nStyle & TBBS_SEPARATOR)
		{
		}
		else
		{
			if (pButton->IsKindOf(RUNTIME_CLASS(CTasksPaneNavigateButton)))
			{
				if (pButton->GetImage() == 3)
				{
					pCloseBtn = DYNAMIC_DOWNCAST(CTasksPaneNavigateButton, pButton);
					ASSERT_VALID(pCloseBtn);
				}

			}
			else if (pButton->IsKindOf(RUNTIME_CLASS(CTasksPaneMenuButton)))
			{
				pMenuBtn = DYNAMIC_DOWNCAST(CTasksPaneMenuButton, pButton);
				ASSERT_VALID(pMenuBtn);
			}
		}
	}

	CRect rectClient;
	GetClientRect(&rectClient);

	BOOL bShowCloseButton = FALSE;
	BOOL bStrechMenuButton = TRUE;

	if (pMenuBtn != NULL)
	{
		CRect rectMenuBtn = pMenuBtn->Rect();
		int nMin = rectMenuBtn.left + rectMenuBtn.Height() * 3;
		int nMax = rectClient.right - 1;
		if (pCloseBtn != NULL && bShowCloseButton)
		{
			nMax = rectClient.right - 1 - rectMenuBtn.Height();
		}

		// -------------------
		// Adjust menu button:
		// -------------------
		if (bStrechMenuButton)
		{
			rectMenuBtn.right = max(nMin, nMax);
			pMenuBtn->SetRect(rectMenuBtn);
		}

		// --------------------
		// Adjust close button:
		// --------------------
		if (pCloseBtn != NULL && bShowCloseButton)
		{
			CRect rectCloseBtn = pMenuBtn->Rect();
			rectCloseBtn.left = rectMenuBtn.right;
			rectCloseBtn.right = rectMenuBtn.right + rectCloseBtn.Height();

			if (rectCloseBtn.right < rectClient.right - 1)
			{
				rectCloseBtn.OffsetRect(rectClient.right - 1 - rectCloseBtn.right, 0);
			}

			pCloseBtn->SetRect(rectCloseBtn);
			pCloseBtn->Show(TRUE);
		}
		else if (pCloseBtn != NULL)
		{
			pCloseBtn->Show(FALSE);
		}
	}
	UpdateTooltips();
}

void CMFCTasksPaneToolBar::UpdateMenuButtonText(const CString& str)
{
	for (POSITION pos = m_Buttons.GetHeadPosition(); pos != NULL; )
	{
		CMFCToolBarButton* pButton = (CMFCToolBarButton*) m_Buttons.GetNext(pos);
		ASSERT_VALID(pButton);

		CTasksPaneMenuButton* pMenuBtn = DYNAMIC_DOWNCAST(CTasksPaneMenuButton, pButton);
		if (pMenuBtn != NULL)
		{
			ASSERT_VALID(pMenuBtn);

			pMenuBtn->m_strText = str;
		}
	}
}

void CMFCTasksPaneToolBar::UpdateButtons()
{
	for (POSITION pos = m_Buttons.GetHeadPosition(); pos != NULL; )
	{
		CMFCToolBarButton* pButton = (CMFCToolBarButton*) m_Buttons.GetNext(pos);
		ASSERT_VALID(pButton);

		CTasksPaneHistoryButton* pHistoryBtn = DYNAMIC_DOWNCAST(CTasksPaneHistoryButton, pButton);
		if (pHistoryBtn != NULL)
		{
			pHistoryBtn->UpdateMenu();
		}
	}
}

BOOL CMFCTasksPaneToolBar::OnUserToolTip(CMFCToolBarButton* pButton, CString& strTTText) const
{
	ASSERT_VALID(pButton);

	if (pButton->IsKindOf(RUNTIME_CLASS(CTasksPaneMenuButton)))
	{
		ENSURE(strTTText.LoadString(ID_AFXBARRES_TASKPANE_OTHER));
		return TRUE;
	}

	CTasksPaneNavigateButton* pNavButton = DYNAMIC_DOWNCAST(CTasksPaneNavigateButton, pButton);
	if (pNavButton != NULL)
	{
		ASSERT_VALID(pNavButton);
		strTTText = pNavButton->m_strText;
		return TRUE;
	}

	CTasksPaneHistoryButton* pHisButton = DYNAMIC_DOWNCAST(CTasksPaneHistoryButton, pButton);
	if (pHisButton != NULL)
	{
		ASSERT_VALID(pHisButton);
		strTTText = pHisButton->m_strText;
		return TRUE;
	}

	return CMFCToolBar::OnUserToolTip(pButton, strTTText);
}

void CMFCTasksPaneToolBar::AdjustLayout()
{
	CMFCToolBar::AdjustLayout();

	CMFCTasksPane* pTaskPane = DYNAMIC_DOWNCAST(CMFCTasksPane, GetParent());
	if (pTaskPane != NULL)
	{
		pTaskPane->RecalcLayout(TRUE);
	}
}


/////////////////////////////////////////////////////////////////////////////
// CMFCTasksPane

clock_t CMFCTasksPane::m_nLastAnimTime = 0;
const int CMFCTasksPane::m_nAnimTimerDuration = 30;
const int CMFCTasksPane::m_nScrollTimerDuration = 80;

IMPLEMENT_SERIAL(CMFCTasksPane, CDockablePane, VERSIONABLE_SCHEMA | 1)

CMFCTasksPane::CMFCTasksPane(): CDockablePane(), m_nMaxHistory(10)
{
	m_hFont = NULL;
	m_sizeIcon = CSize(0, 0);

	m_arrHistoryStack.Add(0);
	m_iActivePage = 0;
	m_pHotTask = NULL;
	m_pClickedTask = NULL;
	m_pHotGroupCaption = NULL;
	m_pClickedGroupCaption = NULL;
	m_bCanCollapse = TRUE;
	m_nVertScrollOffset = 0;
	m_nVertScrollTotal = 0;
	m_nVertScrollPage = 0;
	m_nRowHeight = 0;

	m_nVertMargin = -1; // default, use Visual Manager's settings
	m_nHorzMargin = -1;
	m_nGroupVertOffset = -1;
	m_nGroupCaptionHeight = -1;
	m_nGroupCaptionHorzOffset = -1;
	m_nGroupCaptionVertOffset = -1;
	m_nTasksHorzOffset = -1;
	m_nTasksIconHorzOffset = -1;
	m_nTasksIconVertOffset = -1;

	m_bOffsetCustomControls = TRUE;

	m_rectTasks.SetRectEmpty();

	m_bUseNavigationToolbar = FALSE;
	m_bHistoryMenuButtons = FALSE;
	m_uiToolbarBmpRes = 0;
	m_sizeToolbarImage = CSize(0, 0);
	m_sizeToolbarButton = CSize(0, 0);
	m_rectToolbar.SetRectEmpty();

	m_bUseScrollButtons = TRUE;
	m_rectScrollUp.SetRectEmpty();
	m_rectScrollDn.SetRectEmpty();
	m_iScrollBtnHeight = CMenuImages::Size().cy + 2 * nBorderSize;
	m_iScrollMode = 0;

	m_bAnimationEnabled = !afxGlobalData.bIsRemoteSession;

	m_pAnimatedGroup = NULL;
	m_sizeAnim = CSize(0, 0);

	m_bMenuBtnPressed = FALSE;

	m_bWrapTasks = FALSE;
	m_bWrapLabels = FALSE;

	EnableActiveAccessibility();
}

CMFCTasksPane::~CMFCTasksPane()
{
	while (!m_lstTasksPanes.IsEmpty())
	{
		delete m_lstTasksPanes.RemoveHead();
	}
}

BEGIN_MESSAGE_MAP(CMFCTasksPane, CDockablePane)
	//{{AFX_MSG_MAP(CMFCTasksPane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETTINGCHANGE()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_CANCELMODE()
	ON_WM_LBUTTONDOWN()
	ON_WM_VSCROLL()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_MOUSEWHEEL()
	ON_WM_TIMER()
	ON_COMMAND(IDOK, &CMFCTasksPane::OnOK)
	ON_COMMAND(IDCANCEL, &CMFCTasksPane::OnCancel)
	ON_MESSAGE(WM_SETFONT, &CMFCTasksPane::OnSetFont)
	ON_MESSAGE(WM_GETFONT, &CMFCTasksPane::OnGetFont)
	ON_MESSAGE(WM_SETTEXT, &CMFCTasksPane::OnSetText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, &CMFCTasksPane::OnNeedTipText)
	ON_COMMAND(ID_AFXBARRES_TASKPANE_BACK, &CMFCTasksPane::OnBack)
	ON_COMMAND(ID_AFXBARRES_TASKPANE_FORWARD, &CMFCTasksPane::OnForward)
	ON_COMMAND(ID_AFXBARRES_TASKPANE_HOME, &CMFCTasksPane::OnHome)
	ON_COMMAND(ID_AFXBARRES_TASKPANE_CLOSE, &CMFCTasksPane::OnClose)
	ON_COMMAND(ID_AFXBARRES_TASKPANE_OTHER, &CMFCTasksPane::OnOther)
	ON_UPDATE_COMMAND_UI(ID_AFXBARRES_TASKPANE_BACK, &CMFCTasksPane::OnUpdateBack)
	ON_UPDATE_COMMAND_UI(ID_AFXBARRES_TASKPANE_FORWARD, &CMFCTasksPane::OnUpdateForward)
	ON_UPDATE_COMMAND_UI(ID_AFXBARRES_TASKPANE_CLOSE, &CMFCTasksPane::OnUpdateClose)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCTasksPane message handlers

int CMFCTasksPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	//-----------------------------
	// Load Task Pane text strings:
	//-----------------------------
	CString strOther;
	ENSURE(strOther.LoadString(ID_AFXBARRES_TASKPANE_OTHER));

	CString strForward;
	ENSURE(strForward.LoadString(ID_AFXBARRES_TASKPANE_FORWARD));

	CString strBack;
	ENSURE(strBack.LoadString(ID_AFXBARRES_TASKPANE_BACK));

	GetWindowText(m_strCaption);
	if (m_strCaption.IsEmpty())
	{
		ENSURE(m_strCaption.LoadString(IDS_AFXBARRES_TASKPANE));
	}

	// --------------------------------------------
	// Register tools for caption buttons tooltips:
	// --------------------------------------------
	if (m_pToolTip->GetSafeHwnd() != NULL)
	{
		for (int i = 1; i <= 3; i ++)
		{
			CRect rectDummy;
			rectDummy.SetRectEmpty();

			m_pToolTip->AddTool(this, LPSTR_TEXTCALLBACK, &rectDummy, AFX_CONTROLBAR_BUTTONS_NUM + i);
		}
	}

	// ------------------------------------
	// Add default page to m_lstTasksPanes:
	// ------------------------------------
	AddPage(m_strCaption);
	SetCaption(m_strCaption);

	// ---------------
	// Create toolbar:
	// ---------------
	if (!CreateNavigationToolbar())
	{
		TRACE(_T("Can't create taskspane toolbar bar\n"));
		return FALSE;
	}

	CreateFonts();

	CRect rectDummy;
	rectDummy.SetRectEmpty();
	m_wndScrollVert.Create(WS_CHILD | WS_VISIBLE | SBS_VERT, rectDummy, this, AFX_ID_SCROLL_VERT);

	return 0;
}

void CMFCTasksPane::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	AdjustScroll();
	ReposTasks();

	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}

int CMFCTasksPane::ReposTasks(BOOL bCalcHeightOnly/* = FALSE*/)
{
	if (afxGlobalData.bIsRemoteSession)
	{
		m_bAnimationEnabled = FALSE;
	}

	if (GetSafeHwnd() == NULL || m_lstTaskGroups.IsEmpty())
	{
		return 0;
	}

	if ((m_rectTasks.top < 0) ||(m_rectTasks.bottom <= m_rectTasks.top) || (m_rectTasks.left < 0) ||(m_rectTasks.right <= m_rectTasks.left))
	{
		return 0; // m_rectTasks is not set yet
	}

	CRect rectTasks = m_rectTasks;
	rectTasks.DeflateRect((GetHorzMargin() != -1 ? GetHorzMargin() : CMFCVisualManager::GetInstance()->GetTasksPaneHorzMargin()),
		(GetVertMargin() != -1 ? GetVertMargin() : CMFCVisualManager::GetInstance()->GetTasksPaneVertMargin()));

	CClientDC dc(this);
	CFont* pFontOld = dc.SelectObject(&m_fontBold);

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);

	m_nRowHeight = max(tm.tmHeight, m_sizeIcon.cy);
	m_nAnimGroupExtraHeight = 0;

	int y = rectTasks.top - m_nVertScrollOffset * m_nRowHeight;

	// ---------------
	// Get active page
	// ---------------
	CMFCTasksPanePropertyPage* pActivePage = NULL;
	POSITION posPage = m_lstTasksPanes.FindIndex(m_arrHistoryStack[m_iActivePage]);
	ENSURE(posPage != NULL);

	pActivePage = (CMFCTasksPanePropertyPage*) m_lstTasksPanes.GetAt(posPage);
	ASSERT_VALID(pActivePage);

	POSITION pos = NULL;

	// -------------
	// Recalc groups
	// -------------
	for (pos = m_lstTaskGroups.GetHeadPosition(); pos != NULL;)
	{
		CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetNext(pos);
		ASSERT_VALID(pGroup);

		if (pGroup->m_pPage == pActivePage)
		{
			dc.SelectObject(&m_fontBold);

			// -----------------
			// Calc caption size
			// -----------------
			int nCaptionHeight = 0;
			if (!pGroup->m_strName.IsEmpty())
			{
				CFont* pFontOldBold = dc.SelectObject(&afxGlobalData.fontBold);
				CSize sizeText = dc.GetTextExtent(pGroup->m_strName);
				dc.SelectObject(pFontOldBold);
				int nVOffset = (GetGroupCaptionVertOffset() != -1 ? GetGroupCaptionVertOffset() : CMFCVisualManager::GetInstance()->GetTasksPaneGroupCaptionVertOffset());
				int nHeight = (GetGroupCaptionHeight() != -1 ? GetGroupCaptionHeight() : CMFCVisualManager::GetInstance()->GetTasksPaneGroupCaptionHeight());

				if (IsToolBox())
				{
					nVOffset = max(5, nVOffset);
					if (GetGroupCaptionHeight() == -1)
					{
						nHeight = 18;
					}
				}

				nCaptionHeight = max( sizeText.cy + nVOffset, nHeight );
			}
			else
			{
				nCaptionHeight = 0;
			}

			if (pGroup->m_hIcon != NULL && (pGroup->m_sizeIcon.cx < rectTasks.Width() - nCaptionHeight))
			{
				if (nCaptionHeight < pGroup->m_sizeIcon.cy)
				{
					y += pGroup->m_sizeIcon.cy - nCaptionHeight;
				}
			}

			if (!bCalcHeightOnly)
			{
				pGroup->m_rect = CRect(rectTasks.left, y, rectTasks.right, y + nCaptionHeight);
			}

			y += nCaptionHeight;
			int yGroup = y;

			SetFont(&dc);

			if (m_bCanCollapse && pGroup->m_bIsCollapsed && !pGroup->m_strName.IsEmpty() &&
				!(m_bAnimationEnabled && pGroup == m_pAnimatedGroup && m_sizeAnim.cy > 0 && !bCalcHeightOnly))
			{
				if (!bCalcHeightOnly)
				{
					// ---------------------
					// Recalc tasks in group
					// ---------------------
					pGroup->m_rectGroup = CRect(rectTasks.left, yGroup - 1, rectTasks.right, yGroup - 1);

					for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition(); posTask != NULL;)
					{
						CMFCTasksPaneTask* pTask = (CMFCTasksPaneTask*) pGroup->m_lstTasks.GetNext(posTask);
						ASSERT_VALID(pTask);

						if (pTask->m_hwndTask == NULL)
						{
							pTask->m_rect.SetRectEmpty();
						}
					}
				}
			}
			else // not collapsed
			{
				// ---------------------
				// Recalc tasks in group
				// ---------------------
				BOOL bNeedHeaderOffset = TRUE;
				BOOL bNeedFooterOffset = TRUE;
				CSize sizeGroupBorders = GetTasksGroupBorders();

				for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition(); posTask != NULL;)
				{
					CMFCTasksPaneTask* pTask = (CMFCTasksPaneTask*) pGroup->m_lstTasks.GetNext(posTask);
					ASSERT_VALID(pTask);

					if (pTask->m_hwndTask == NULL)
					{
						if (pTask->m_bVisible)
						{
							if (bNeedHeaderOffset)
							{
								y += (GetTasksIconVertOffset() != -1 ? GetTasksIconVertOffset() : CMFCVisualManager::GetInstance()->GetTasksPaneIconVertOffset());
							}

							int nTaskHOffset = (GetTasksHorzOffset() != -1 ? GetTasksHorzOffset() : CMFCVisualManager::GetInstance()->GetTasksPaneTaskHorzOffset());
							int nIconHOffset = (GetTasksIconHorzOffset() != -1 ? GetTasksIconHorzOffset() : CMFCVisualManager::GetInstance()->GetTasksPaneIconHorzOffset());

							// -----------------
							// if multiline text
							// -----------------
							if ((pTask->m_uiCommandID == 0) ? m_bWrapLabels : m_bWrapTasks)
							{
								CRect rectTask = rectTasks;
								rectTask.DeflateRect(nTaskHOffset, 0);
								rectTask.top = y;
								rectTask.bottom = y + m_sizeIcon.cy;

								// Determines the width of the text rectangle
								CRect rectText = rectTask;
								rectText.left += m_sizeIcon.cx + nIconHOffset;

								// Determines the height of the text rectangle
								CFont* pFontOldUnd = dc.SelectObject(&afxGlobalData.fontUnderline);
								int cy = dc.DrawText(pTask->m_strName, rectText, DT_CALCRECT | DT_WORDBREAK);
								dc.SelectObject(pFontOldUnd);

								if (pTask->m_bIsSeparator)
								{
									cy = max(cy, 10);
								}
								cy = max(cy, m_sizeIcon.cy);
								rectTask.bottom = rectTask.top + cy;

								if (!bCalcHeightOnly)
								{
									pTask->m_rect = rectTask;
								}

								y += cy +(GetTasksIconVertOffset() != -1 ? GetTasksIconVertOffset() : CMFCVisualManager::GetInstance()->GetTasksPaneIconVertOffset());
								bNeedHeaderOffset = FALSE;
								bNeedFooterOffset = FALSE;
							}
							// ----------------
							// single-line text
							// ----------------
							else
							{
								CFont* pFontOldUnd = dc.SelectObject(&afxGlobalData.fontUnderline);
								CSize sizeText = dc.GetTextExtent(pTask->m_strName);
								dc.SelectObject(pFontOldUnd);
								int cy = max(sizeText.cy, m_sizeIcon.cy);

								if (pTask->m_bIsSeparator)
								{
									cy = max(cy, 10);
								}

								if (!bCalcHeightOnly)
								{
									pTask->m_rect = CRect( rectTasks.left + nTaskHOffset, y,
										rectTasks.left + sizeText.cx + m_sizeIcon.cx + nTaskHOffset + nIconHOffset, y + cy);
									pTask->m_rect.right = max(pTask->m_rect.left, rectTasks.right - nTaskHOffset);
								}

								y += cy +(GetTasksIconVertOffset() != -1 ? GetTasksIconVertOffset() : CMFCVisualManager::GetInstance()->GetTasksPaneIconVertOffset());
								bNeedHeaderOffset = FALSE;
								bNeedFooterOffset = FALSE;
							}
						}
						else
						{
							if (!bCalcHeightOnly)
							{
								pTask->m_rect.SetRectEmpty();
							}
						}
					}

					else // Use child window
					{
						if (bNeedHeaderOffset && pTask->m_bVisible)
						{
							if (m_bOffsetCustomControls)
							{
								y += (GetTasksIconVertOffset() != -1 ? GetTasksIconVertOffset() : CMFCVisualManager::GetInstance()->GetTasksPaneIconVertOffset());
							}
							else
							{
								y += sizeGroupBorders.cy;
							}
						}

						CWnd* pChildWnd = CWnd::FromHandle(pTask->m_hwndTask);
						ASSERT_VALID(pChildWnd);

						if (!bCalcHeightOnly)
						{
							CRect rectChildWnd = rectTasks;
							rectChildWnd.bottom = y +(pTask->m_bVisible ? pTask->m_nWindowHeight : 0);
							rectChildWnd.top = max(m_rectTasks.top + 1, y);
							int nChildScrollValue = pTask->m_nWindowHeight - rectChildWnd.Height();
							rectChildWnd.bottom = min(m_rectTasks.bottom, rectChildWnd.bottom);

							if (m_bOffsetCustomControls)
							{
								rectChildWnd.DeflateRect((GetTasksHorzOffset() != -1 ? GetTasksHorzOffset(): CMFCVisualManager::GetInstance()->GetTasksPaneTaskHorzOffset()), 0);
							}
							else
							{
								rectChildWnd.DeflateRect(sizeGroupBorders.cx, 0);
							}

							pTask->m_rect = rectChildWnd;

							// Scroll child windows:
							if (IsToolBox())
							{
								if (pTask->m_rect.IsRectEmpty())
								{
									ScrollChild(pTask->m_hwndTask, 0);
								}
								else
								{
									ScrollChild(pTask->m_hwndTask, nChildScrollValue);
								}
							}
						}

						if (pTask->m_bVisible)
						{
							y += pTask->m_nWindowHeight;
							bNeedHeaderOffset = TRUE;
							bNeedFooterOffset = TRUE;
						}
					}

					if (bNeedFooterOffset && pTask->m_bVisible)
					{
						if (m_bOffsetCustomControls)
						{
							y += (GetTasksIconVertOffset() != -1 ? GetTasksIconVertOffset() : CMFCVisualManager::GetInstance()->GetTasksPaneIconVertOffset());
						}
						else
						{
							y += sizeGroupBorders.cy;
						}
					}

					// constrain task's height during the animation:
					if (!bCalcHeightOnly)
					{
						if (m_bAnimationEnabled && pGroup == m_pAnimatedGroup)
						{
							if (y > yGroup + m_sizeAnim.cy)
							{
								int nSave = y;
								y = yGroup + max(0, m_sizeAnim.cy);
								pTask->m_rect.bottom = max(pTask->m_rect.top, min(pTask->m_rect.bottom, y - 1));
								m_nAnimGroupExtraHeight += nSave - y;
							}
						}
					}
				}

				if (!bCalcHeightOnly)
				{
					pGroup->m_rectGroup = CRect(rectTasks.left, yGroup, rectTasks.right, y);
				}
			}

			y += (GetGroupVertOffset() != -1 ? GetGroupVertOffset() : CMFCVisualManager::GetInstance()->GetTasksPaneGroupVertOffset());
		}
	}

	if (!bCalcHeightOnly)
	{
		// ---------------------------------------------
		// Find the last task group for the active page:
		// ---------------------------------------------
		CMFCTasksPaneTaskGroup* pLastGroup = NULL;
		for (POSITION posGroup = m_lstTaskGroups.GetTailPosition(); posGroup != NULL; )
		{
			CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetPrev(posGroup);
			ASSERT_VALID(pGroup);

			if (pGroup->m_pPage == pActivePage)
			{
				pLastGroup = pGroup;
				break;
			}
		}

		if (pLastGroup != NULL)
		{
			// ---------------------------------------------
			// Offset the last group if it's bottom aligned:
			// ---------------------------------------------
			if (pLastGroup->m_bIsBottom && !pLastGroup->m_lstTasks.IsEmpty() && m_nVertScrollTotal == 0)
			{
				int nOffset = 0;
				for (POSITION posTask = pLastGroup->m_lstTasks.GetTailPosition(); posTask != NULL;)
				{
					CMFCTasksPaneTask* pTask = (CMFCTasksPaneTask*) pLastGroup->m_lstTasks.GetPrev(posTask);
					ASSERT_VALID(pTask);

					if (pTask->m_bVisible)
					{
						nOffset = rectTasks.bottom - pLastGroup->m_rectGroup.bottom;
						break;
					}
				}

				if (nOffset > 0)
				{
					for (POSITION posTask = pLastGroup->m_lstTasks.GetHeadPosition(); posTask != NULL;)
					{
						CMFCTasksPaneTask* pTask = (CMFCTasksPaneTask*) pLastGroup->m_lstTasks.GetNext(posTask);
						ASSERT_VALID(pTask);

						if (pTask->m_bVisible)
						{
							pTask->m_rect.OffsetRect(0, nOffset);
						}
					}

					pLastGroup->m_rect.OffsetRect(0, nOffset);
					pLastGroup->m_rectGroup.OffsetRect(0, nOffset);
				}
			}
		}

		// --------------------------------------------
		// Reposition or hide child windows for active page:
		// --------------------------------------------
		for (pos = m_lstTaskGroups.GetHeadPosition(); pos != NULL;)
		{
			CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetNext(pos);
			ASSERT_VALID(pGroup);

			if (pGroup->m_pPage == pActivePage)
			{
				BOOL bCollapsed = m_bCanCollapse && pGroup->m_bIsCollapsed && !pGroup->m_strName.IsEmpty();
				BOOL bAnimating = m_bAnimationEnabled && pGroup == m_pAnimatedGroup && m_sizeAnim.cy > 0;
				for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition(); posTask != NULL;)
				{
					CMFCTasksPaneTask* pTask = (CMFCTasksPaneTask*) pGroup->m_lstTasks.GetNext(posTask);
					ASSERT_VALID(pTask);

					if (pTask->m_hwndTask != NULL) // Use child window
					{
						CWnd* pChildWnd = CWnd::FromHandle(pTask->m_hwndTask);
						ASSERT_VALID(pChildWnd);

						if (bCollapsed && !bAnimating || !pTask->m_bVisible || pTask->m_rect.IsRectEmpty())
						{
							pChildWnd->ShowWindow(SW_HIDE);
						}
						else
						{
							pChildWnd->SetWindowPos(NULL, pTask->m_rect.left, pTask->m_rect.top, pTask->m_rect.Width(), pTask->m_rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
							pChildWnd->ShowWindow(SW_SHOWNOACTIVATE);
						}
					}

				}
			}
		}
	}

	dc.SelectObject(pFontOld);
	return y -(GetGroupVertOffset() != -1 ? GetGroupVertOffset() : CMFCVisualManager::GetInstance()->GetTasksPaneGroupVertOffset()) +
		m_nVertScrollOffset * m_nRowHeight + (GetVertMargin() != -1 ? GetVertMargin() : CMFCVisualManager::GetInstance()->GetTasksPaneVertMargin());
}

void CMFCTasksPane::OnDrawTasks(CDC* pDC, CRect /*rectWorkArea*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	CRect rectFill = m_rectTasks;
	rectFill.InflateRect(0, m_nVertScrollOffset * m_nRowHeight, 0, 0);

	OnFillBackground(pDC, rectFill);

	// ---------------
	// Get active page
	// ---------------
	CMFCTasksPanePropertyPage* pActivePage = NULL;
	POSITION posPage = m_lstTasksPanes.FindIndex(m_arrHistoryStack[m_iActivePage]);
	ENSURE(posPage != NULL);

	pActivePage = (CMFCTasksPanePropertyPage*) m_lstTasksPanes.GetAt(posPage);
	ASSERT_VALID(pActivePage);

	// ---------------------
	// Draw all tasks groups
	// ---------------------
	CRgn rgnClipTask;
	rgnClipTask.CreateRectRgnIndirect(CRect(0, 0, 0, 0));
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition(); pos != NULL; )
	{
		CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetNext(pos);
		ASSERT_VALID(pGroup);

		if (pGroup->m_pPage == pActivePage)
		{
			if (!pGroup->m_bIsCollapsed || pGroup->m_strName.IsEmpty() || (m_bAnimationEnabled && pGroup == m_pAnimatedGroup && m_sizeAnim.cy > 0))
			{
				if (!pGroup->m_rectGroup.IsRectEmpty())
				{
					CMFCVisualManager::GetInstance()->OnFillTasksGroupInterior(pDC, pGroup->m_rectGroup);
				}
				if (!pGroup->m_rect.IsRectEmpty())
				{
					CMFCVisualManager::GetInstance()->OnDrawTasksGroupCaption(pDC, pGroup, m_pHotGroupCaption == pGroup, FALSE, m_bCanCollapse);
				}
				if (!pGroup->m_rectGroup.IsRectEmpty())
				{
					CSize sizeGroupBorders = GetTasksGroupBorders();
					if (sizeGroupBorders.cx > 0 || sizeGroupBorders.cy > 0)
					{
						CMFCVisualManager::GetInstance()->OnDrawTasksGroupAreaBorder(pDC, pGroup->m_rectGroup, pGroup->m_bIsSpecial, pGroup->m_strName.IsEmpty());
					}

					// --------------
					// Draw all tasks
					// --------------
					for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition(); posTask != NULL;)
					{
						CMFCTasksPaneTask* pTask = (CMFCTasksPaneTask*) pGroup->m_lstTasks.GetNext(posTask);
						ASSERT_VALID(pTask);

						if (pTask->m_bVisible && pTask->m_hwndTask == NULL) // the task is not child window
						{
							rgnClipTask.SetRectRgn(&pTask->m_rect);
							pDC->SelectClipRgn(&rgnClipTask);

							CMFCVisualManager::GetInstance()->OnDrawTask(pDC, pTask, &m_lstIcons, (pTask == m_pHotTask));

							pDC->SelectClipRgn(NULL);
						}
					}
				}
			}
			else // Group is collapsed
			{
				if (!pGroup->m_rect.IsRectEmpty())
				{
					CMFCVisualManager::GetInstance()->OnDrawTasksGroupCaption(pDC, pGroup, m_pHotGroupCaption == pGroup, FALSE, m_bCanCollapse);
				}
			}

		}
	}
	rgnClipTask.DeleteObject();

	// ------------------------
	// Draw navigation toolbar:
	// ------------------------
	CRect rectToolbarOld = m_rectToolbar;
	if (m_bUseNavigationToolbar)
	{
		m_wndToolBar.Invalidate();
		m_wndToolBar.UpdateWindow();
	}

	// --------------------
	// Draw scroll buttons:
	// --------------------
	if (m_bUseScrollButtons)
	{
		if (IsScrollUpAvailable())
		{
			CMFCVisualManager::GetInstance()->OnDrawScrollButtons(pDC, m_rectScrollUp, nBorderSize, CMenuImages::IdArrowUp, m_iScrollMode < 0);
		}

		if (IsScrollDnAvailable())
		{
			CMFCVisualManager::GetInstance()->OnDrawScrollButtons(pDC, m_rectScrollDn, nBorderSize, CMenuImages::IdArrowDown, m_iScrollMode > 0);
		}
	}
}

void CMFCTasksPane::OnFillBackground(CDC* pDC, CRect rectFill)
{
	CMFCVisualManager::GetInstance()->OnFillTasksPaneBackground(pDC, rectFill);
}

void CMFCTasksPane::SetIconsList(HIMAGELIST hIcons)
{
	ASSERT_VALID(this);

	if (m_lstIcons.GetSafeHandle() != NULL)
	{
		m_lstIcons.DeleteImageList();
	}

	if (hIcons == NULL)
	{
		m_sizeIcon = CSize(0, 0);
	}
	else
	{
		m_lstIcons.Create(CImageList::FromHandle(hIcons));
		::ImageList_GetIconSize(hIcons, (int*) &m_sizeIcon.cx, (int*) &m_sizeIcon.cy);
	}

	AdjustScroll();
	ReposTasks();
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}

BOOL CMFCTasksPane::SetIconsList(UINT uiImageListResID, int cx, COLORREF clrTransparent)
{
	ASSERT_VALID(this);

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiImageListResID))
	{
		TRACE(_T("Can't load bitmap: %x\n"), uiImageListResID);
		return FALSE;
	}

	CImageList icons;

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = (clrTransparent == (COLORREF) -1) ? 0 : ILC_MASK;

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
		if (clrTransparent == (COLORREF)-1)
		{
			nFlags |= ILC_COLOR32 | ILC_MASK;
		}
		else
		{
			nFlags |= ILC_COLOR32;
		}
		break;
	}

	icons.Create(cx, bmpObj.bmHeight, nFlags, 0, 0);

	if (bmpObj.bmBitsPixel == 32 && clrTransparent == (COLORREF)-1)
	{
		icons.Add(&bmp, (CBitmap*) NULL);
	}
	else
	{
		icons.Add(&bmp, clrTransparent);
	}

	SetIconsList(icons);
	return TRUE;
}

int CMFCTasksPane::AddPage(LPCTSTR lpszPageLabel)
{
	ENSURE(lpszPageLabel != NULL);

	CMFCTasksPanePropertyPage* pPage = new CMFCTasksPanePropertyPage(lpszPageLabel, this);
	ASSERT_VALID(pPage);

	m_lstTasksPanes.AddTail(pPage);

	RebuildMenu();
	return(int) m_lstTasksPanes.GetCount() - 1;
}

void CMFCTasksPane::RemovePage(int nPageIdx)
{
	ASSERT(nPageIdx <= m_lstTasksPanes.GetCount()-1);
	if (nPageIdx <= 0)
	{
		ASSERT(FALSE);
		return;
	}

	POSITION posPage = m_lstTasksPanes.FindIndex(nPageIdx);
	ENSURE(posPage != NULL);
	CMFCTasksPanePropertyPage* pPage = (CMFCTasksPanePropertyPage*) m_lstTasksPanes.GetAt(posPage);
	ASSERT_VALID(pPage);

	//------------------------
	// Change the active page:
	//------------------------
	ASSERT(m_iActivePage >= 0);
	ASSERT(m_iActivePage <= m_arrHistoryStack.GetUpperBound());
	int nOldPageIdx = m_arrHistoryStack[m_iActivePage];

	if (m_arrHistoryStack.GetSize() == 1)
	{
		// history is empty - select the default page
		SaveHistory(0);
		int nOldActivePage = m_iActivePage;
		m_iActivePage = (int) m_arrHistoryStack.GetUpperBound();

		ChangeActivePage(m_iActivePage, nOldActivePage);
	}
	else if (nOldPageIdx == nPageIdx)
	{
		int nOldActivePage = m_iActivePage;
		if (m_iActivePage < m_arrHistoryStack.GetUpperBound())
		{
			m_iActivePage = (int) m_arrHistoryStack.GetUpperBound();
		}
		else
		{
			m_iActivePage--;
		}

		ChangeActivePage(m_iActivePage, nOldActivePage);
	}
	else if (GetSafeHwnd() != NULL)
	{
		RebuildMenu();

		AdjustScroll();
		ReposTasks();
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	}

	POSITION pos = NULL;

	//-----------------------------------------------
	// First, remove all tasks groups from this page:
	//-----------------------------------------------
	for (pos = m_lstTaskGroups.GetHeadPosition(); pos != NULL;)
	{
		POSITION posSave = pos;

		CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetNext(pos);
		ASSERT_VALID(pGroup);

		if (pGroup->m_pPage == pPage)
		{
			m_lstTaskGroups.RemoveAt(posSave);
			delete pGroup;
		}
	}

	//-------------
	// Remove page:
	//-------------
	pos = m_lstTasksPanes.FindIndex(nPageIdx);
	ENSURE(pos != NULL);

	m_lstTasksPanes.RemoveAt(pos);
	delete pPage;

	// --------------------------------------------------------
	// Refresh history - remove references to the deleted page:
	// --------------------------------------------------------
	CArray <int, int> arrCopy;
	arrCopy.Copy(m_arrHistoryStack);
	m_arrHistoryStack.RemoveAll();

	int nPrevIdx = nPageIdx;
	int iResult = -1;
	int iResultActivePage = 0;
	for (int i=0; i < arrCopy.GetSize(); i++)
	{
		if (arrCopy[i] != nPrevIdx)
		{
			if (arrCopy[i] < nPageIdx)
			{
				m_arrHistoryStack.Add(arrCopy[i]);
				nPrevIdx = arrCopy[i];
				iResult++;
			}
			else if (arrCopy[i] > nPageIdx)
			{
				m_arrHistoryStack.Add(arrCopy[i]-1);
				nPrevIdx = arrCopy[i];
				iResult++;
			}
		}
		if (i == m_iActivePage)
		{
			iResultActivePage = iResult;
		}
	}
	m_iActivePage = iResultActivePage;
	ASSERT(m_arrHistoryStack.GetSize() > 0);
	ASSERT(m_iActivePage >= 0);
	ASSERT(m_iActivePage <= m_arrHistoryStack.GetUpperBound());

	RebuildMenu();
}

void CMFCTasksPane::RemoveAllPages()
{
	//----------------------
	// Reset an active page:
	//----------------------
	int nOldActivePage = m_iActivePage;
	m_iActivePage = 0;
	ChangeActivePage(0, nOldActivePage); // Default page
	m_arrHistoryStack.RemoveAll();
	m_arrHistoryStack.Add(0);

	//--------------------------------------------------------
	// First, remove all tasks group except from default page:
	//--------------------------------------------------------
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition(); pos != NULL;)
	{
		POSITION posSave = pos;

		CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetNext(pos);
		ASSERT_VALID(pGroup);

		if (pGroup->m_pPage != NULL && pGroup->m_pPage != m_lstTasksPanes.GetHead()) // except default page
		{
			m_lstTaskGroups.RemoveAt(posSave);
			delete pGroup;
		}
	}

	//----------------------------------
	// Remove pages except default page:
	//----------------------------------
	while (m_lstTasksPanes.GetCount() > 1)
	{
		delete m_lstTasksPanes.RemoveTail();
	}

	RebuildMenu();
}

int CMFCTasksPane::AddGroup(int nPageIdx, LPCTSTR lpszGroupName, BOOL bBottomLocation/* = FALSE*/, BOOL bSpecial/* = FALSE*/, HICON hIcon/* = NULL*/)
{
	ASSERT(nPageIdx >= 0);
	ASSERT(nPageIdx <= m_lstTasksPanes.GetCount()-1);

	// ---------------
	// Get active page
	// ---------------
	CMFCTasksPanePropertyPage* pActivePage = NULL;
	POSITION posPage = m_lstTasksPanes.FindIndex(nPageIdx);
	ENSURE(posPage != NULL);

	pActivePage = (CMFCTasksPanePropertyPage*) m_lstTasksPanes.GetAt(posPage);
	ASSERT_VALID(pActivePage);

	// -------------
	// Add new group
	// -------------
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition(); pos != NULL;)
	{
		CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetNext(pos);
		ASSERT_VALID(pGroup);

		if (pGroup->m_pPage == pActivePage)
		{
			if (pGroup->m_bIsBottom)
			{
				pGroup->m_bIsBottom = FALSE;
			}
		}
	}

	m_lstTaskGroups.AddTail(new CMFCTasksPaneTaskGroup(lpszGroupName, bBottomLocation, bSpecial, FALSE, pActivePage, hIcon));

	AdjustScroll();
	ReposTasks();

	return(int) m_lstTaskGroups.GetCount() - 1;
}

void CMFCTasksPane::RemoveGroup(int nGroup)
{
	ASSERT(nGroup >= 0);
	ASSERT(nGroup < m_lstTaskGroups.GetCount());

	POSITION pos = m_lstTaskGroups.FindIndex(nGroup);
	if (pos == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetAt(pos);
	ASSERT_VALID(pGroup);

	m_lstTaskGroups.RemoveAt(pos);
	delete pGroup;

	AdjustScroll();
	ReposTasks();
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}

void CMFCTasksPane::RemoveAllGroups(int nPageIdx/* = 0*/)
{
	ASSERT(nPageIdx >= 0);
	ASSERT(nPageIdx < m_lstTasksPanes.GetCount());

	POSITION posPage = m_lstTasksPanes.FindIndex(nPageIdx);
	if (posPage == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	CMFCTasksPanePropertyPage* pPage = (CMFCTasksPanePropertyPage*) m_lstTasksPanes.GetAt(posPage);
	ASSERT_VALID(pPage);

	//----------------------------------------
	// Remove all tasks groups from this page:
	//----------------------------------------
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition(); pos != NULL;)
	{
		POSITION posSave = pos;

		CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetNext(pos);
		ASSERT_VALID(pGroup);

		if (pGroup->m_pPage == pPage)
		{
			m_lstTaskGroups.RemoveAt(posSave);
			delete pGroup;
		}
	}

	AdjustScroll();
	ReposTasks();
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}

BOOL CMFCTasksPane::SetGroupName(int nGroup, LPCTSTR lpszGroupName)
{
	POSITION pos = m_lstTaskGroups.FindIndex(nGroup);
	if (pos == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetAt(pos);
	ASSERT_VALID(pGroup);

	BOOL bCaptionWasEmpty = pGroup->m_strName.IsEmpty();

	pGroup->m_strName = lpszGroupName;

	if ((!bCaptionWasEmpty && pGroup->m_strName.IsEmpty()) || (bCaptionWasEmpty && !pGroup->m_strName.IsEmpty()))
	{
		AdjustScroll();
		ReposTasks();
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	}
	else
	{
		InvalidateRect(&pGroup->m_rect);
		UpdateWindow();
	}

	return TRUE;
}

BOOL CMFCTasksPane::SetGroupTextColor(int nGroup, COLORREF color, COLORREF colorHot)
{
	POSITION pos = m_lstTaskGroups.FindIndex(nGroup);
	if (pos == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetAt(pos);
	ASSERT_VALID(pGroup);

	pGroup->m_clrText = (COLORREF) color;
	pGroup->m_clrTextHot = (COLORREF) colorHot;

	InvalidateRect(&pGroup->m_rect);
	UpdateWindow();

	return TRUE;
}

BOOL CMFCTasksPane::CollapseGroup(CMFCTasksPaneTaskGroup* pGroup, BOOL bCollapse)
{
	ASSERT_VALID(pGroup);

	if ((!bCollapse && pGroup->m_bIsCollapsed) ||
		(bCollapse && !pGroup->m_bIsCollapsed))
	{
		pGroup->m_bIsCollapsed = bCollapse;

		AdjustScroll();
		ReposTasks();
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	}

	return TRUE;
}

void CMFCTasksPane::CollapseAllGroups(BOOL bCollapse)
{
	// -------------------
	// Collapse all groups
	// -------------------
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition(); pos != NULL;)
	{
		CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetNext(pos);
		ASSERT_VALID(pGroup);

		if ((!bCollapse && pGroup->m_bIsCollapsed) || (bCollapse && !pGroup->m_bIsCollapsed))
		{
			pGroup->m_bIsCollapsed = bCollapse;
		}
	}

	AdjustScroll();
	ReposTasks();
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}

void CMFCTasksPane::CollapseAllGroups(int nPageIdx, BOOL bCollapse)
{
	ASSERT(nPageIdx >= 0);
	ASSERT(nPageIdx < m_lstTasksPanes.GetCount());

	POSITION posPage = m_lstTasksPanes.FindIndex(nPageIdx);
	if (posPage == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	CMFCTasksPanePropertyPage* pPage = (CMFCTasksPanePropertyPage*) m_lstTasksPanes.GetAt(posPage);
	ASSERT_VALID(pPage);

	// -----------------------------------------
	// Collapse all groups at the specified page
	// -----------------------------------------
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition(); pos != NULL;)
	{
		CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetNext(pos);
		ASSERT_VALID(pGroup);

		if (pGroup->m_pPage == pPage)
		{
			if ((!bCollapse && pGroup->m_bIsCollapsed) ||
				(bCollapse && !pGroup->m_bIsCollapsed))
			{
				pGroup->m_bIsCollapsed = bCollapse;
			}
		}
	}

	AdjustScroll();
	ReposTasks();
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}

CMFCTasksPaneTaskGroup* CMFCTasksPane::GetTaskGroup(int nGroup) const
{
	ASSERT(nGroup >= 0);
	ASSERT(nGroup < m_lstTaskGroups.GetCount());

	POSITION pos = m_lstTaskGroups.FindIndex(nGroup);
	if (pos == NULL)
	{
		return NULL;
	}

	CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetAt(pos);
	ASSERT_VALID(pGroup);

	return pGroup;
}

BOOL CMFCTasksPane::GetGroupLocation(CMFCTasksPaneTaskGroup* pGroup, int &nGroup) const
{
	ASSERT_VALID(pGroup);

	int nGroupCount = 0;
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition(); pos != NULL; nGroupCount++)
	{
		CMFCTasksPaneTaskGroup* pTaskGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetNext(pos);
		ASSERT_VALID(pTaskGroup);

		if (pTaskGroup == pGroup)
		{
			nGroup = nGroupCount;
			return TRUE;
		}
	}

	return FALSE; // not found
}

int CMFCTasksPane::AddTask(int nGroup, LPCTSTR lpszTaskName, int nTaskIcon/* = -1*/, UINT uiCommandID/* = 0*/, DWORD dwUserData/* = 0*/)
{
	POSITION pos = m_lstTaskGroups.FindIndex(nGroup);
	if (pos == NULL)
	{
		ASSERT(FALSE);
		return -1;
	}

	CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetAt(pos);
	ASSERT_VALID(pGroup);

	pGroup->m_lstTasks.AddTail(new CMFCTasksPaneTask(pGroup, lpszTaskName, nTaskIcon, uiCommandID, dwUserData));

	AdjustScroll();
	ReposTasks();

	return(int) pGroup->m_lstTasks.GetCount() - 1;
}

BOOL CMFCTasksPane::SetTaskName(int nGroup, int nTask, LPCTSTR lpszTaskName)
{
	POSITION pos = m_lstTaskGroups.FindIndex(nGroup);
	if (pos == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetAt(pos);
	ASSERT_VALID(pGroup);

	pos = pGroup->m_lstTasks.FindIndex(nTask);
	if (pos == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CMFCTasksPaneTask* pTask = (CMFCTasksPaneTask*) pGroup->m_lstTasks.GetAt(pos);
	pTask->m_strName = lpszTaskName;

	if (pTask->m_bVisible)
		InvalidateRect(pTask->m_rect);

	return TRUE;
}

BOOL CMFCTasksPane::SetTaskTextColor(int nGroup, int nTask, COLORREF color, COLORREF colorHot)
{
	POSITION pos = m_lstTaskGroups.FindIndex(nGroup);
	if (pos == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetAt(pos);
	ASSERT_VALID(pGroup);

	pos = pGroup->m_lstTasks.FindIndex(nTask);
	if (pos == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CMFCTasksPaneTask* pTask = (CMFCTasksPaneTask*) pGroup->m_lstTasks.GetAt(pos);
	pTask->m_clrText = color;
	pTask->m_clrTextHot = colorHot;

	if (pTask->m_bVisible)
		InvalidateRect(pTask->m_rect);

	return TRUE;
}

BOOL CMFCTasksPane::ShowTask(int nGroup, int nTask, BOOL bShow, BOOL bRedraw)
{
	POSITION pos = m_lstTaskGroups.FindIndex(nGroup);
	if (pos == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetAt(pos);
	ASSERT_VALID(pGroup);

	pos = pGroup->m_lstTasks.FindIndex(nTask);
	if (pos == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CMFCTasksPaneTask* pTask = (CMFCTasksPaneTask*) pGroup->m_lstTasks.GetAt(pos);
	if ((!bShow && pTask->m_bVisible) || (bShow && !pTask->m_bVisible))
	{
		pTask->m_bVisible = bShow;

		AdjustScroll();
		ReposTasks();

		if (bRedraw)
		{
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
		}
	}

	return TRUE;
}

BOOL CMFCTasksPane::ShowTaskByCmdId(UINT uiCommandID, BOOL bShow, BOOL bRedraw)
{
	int nGroup, nTask;

	if (!GetTaskLocation(uiCommandID, nGroup, nTask))
		return FALSE;

	return ShowTask(nGroup, nTask, bShow, bRedraw);
}

BOOL CMFCTasksPane::RemoveTask(int nGroup, int nTask, BOOL bRedraw)
{
	ASSERT(nGroup >= 0);
	ASSERT(nGroup < m_lstTaskGroups.GetCount());

	POSITION pos = m_lstTaskGroups.FindIndex(nGroup);
	if (pos == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetAt(pos);
	ASSERT_VALID(pGroup);

	pos = pGroup->m_lstTasks.FindIndex(nTask);
	if (pos == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	delete pGroup->m_lstTasks.GetAt(pos);
	pGroup->m_lstTasks.RemoveAt(pos);

	AdjustScroll();
	ReposTasks();

	if (bRedraw)
	{
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	}

	return TRUE;
}

void CMFCTasksPane::RemoveAllTasks(int nGroup)
{
	ASSERT(nGroup >= 0);
	ASSERT(nGroup < m_lstTaskGroups.GetCount());

	CMFCTasksPaneTaskGroup* pGroup = GetTaskGroup(nGroup);
	ASSERT_VALID(pGroup);

	while (!pGroup->m_lstTasks.IsEmpty())
	{
		delete pGroup->m_lstTasks.RemoveHead();
	}

	AdjustScroll();
	ReposTasks();
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}

BOOL CMFCTasksPane::GetTaskLocation(UINT uiCommandID, int& nGroup, int& nTask) const
{
	nGroup = 0;
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition(); pos != NULL; ++nGroup)
	{
		CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetNext(pos);
		ASSERT_VALID(pGroup);

		nTask = 0;
		for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition(); posTask != NULL; ++nTask)
		{
			CMFCTasksPaneTask* pTask = (CMFCTasksPaneTask*) pGroup->m_lstTasks.GetNext(posTask);
			ASSERT_VALID(pTask);

			if (pTask->m_uiCommandID == uiCommandID)
			{
				return TRUE;
			}
		}
	}

	nGroup = -1;
	nTask = -1;

	return FALSE;
}

BOOL CMFCTasksPane::GetTaskLocation(HWND hwndTask, int& nGroup, int& nTask) const
{
	nGroup = 0;
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition(); pos != NULL; ++nGroup)
	{
		CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetNext(pos);
		ASSERT_VALID(pGroup);

		nTask = 0;
		for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition(); posTask != NULL; ++nTask)
		{
			CMFCTasksPaneTask* pTask = (CMFCTasksPaneTask*) pGroup->m_lstTasks.GetNext(posTask);
			ASSERT_VALID(pTask);

			if (pTask->m_hwndTask == hwndTask)
			{
				return TRUE;
			}
		}
	}

	nGroup = -1;
	nTask = -1;

	return FALSE;
}

CMFCTasksPaneTask* CMFCTasksPane::GetTask(int nGroup, int nTask) const
{
	ASSERT(nGroup >= 0);
	ASSERT(nGroup < m_lstTaskGroups.GetCount());

	CMFCTasksPaneTaskGroup* pGroup = GetTaskGroup(nGroup);
	ASSERT_VALID(pGroup);

	POSITION pos = pGroup->m_lstTasks.FindIndex(nTask);
	if (pos == NULL)
	{
		return NULL;
	}

	CMFCTasksPaneTask* pTask = (CMFCTasksPaneTask*) pGroup->m_lstTasks.GetAt(pos);
	ASSERT_VALID(pTask);

	return pTask;
}

BOOL CMFCTasksPane::GetTaskLocation(CMFCTasksPaneTask* pTask, int& nGroup, int& nTask) const
{
	ASSERT_VALID(pTask);
	ASSERT_VALID(pTask->m_pGroup);

	nGroup = -1;
	nTask = -1;

	CMFCTasksPaneTaskGroup* pGroupToFind = pTask->m_pGroup;

	int nGroupCount = 0;
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition(); pos != NULL; nGroupCount++)
	{
		CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetNext(pos);
		ASSERT_VALID(pGroup);

		if (pGroup == pGroupToFind)
		{
			int nTaskCount = 0;
			for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition(); posTask != NULL; nTaskCount++)
			{
				CMFCTasksPaneTask* pCurTask = (CMFCTasksPaneTask*) pGroup->m_lstTasks.GetNext(posTask);
				ASSERT_VALID(pCurTask);

				if (pCurTask == pTask)
				{
					nGroup = nGroupCount;
					nTask = nTaskCount;
					return TRUE;
				}
			}

			return FALSE;
		}
	}

	return FALSE;
}

int CMFCTasksPane::AddWindow(int nGroup, HWND hwndTask, int nWndHeight, BOOL bAutoDestroyWindow/* = FALSE*/, DWORD dwUserData/* = 0*/)
{
	POSITION pos = m_lstTaskGroups.FindIndex(nGroup);
	if (pos == NULL)
	{
		ASSERT(FALSE);
		return -1;
	}

	ASSERT(::IsWindow(hwndTask));

	CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetAt(pos);
	ASSERT_VALID(pGroup);

	pGroup->m_lstTasks.AddTail(new CMFCTasksPaneTask(pGroup, _T(""), -1, 0, dwUserData, hwndTask, bAutoDestroyWindow, nWndHeight));

	AdjustScroll();
	ReposTasks();

	return(int) pGroup->m_lstTasks.GetCount() - 1;
}

BOOL CMFCTasksPane::SetWindowHeight(int nGroup, HWND hwndTask, int nWndHeight)
{
	POSITION pos = m_lstTaskGroups.FindIndex(nGroup);
	if (pos == NULL)
	{
		ASSERT(FALSE);
		return -1;
	}

	ENSURE(::IsWindow(hwndTask));

	CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetAt(pos);
	ASSERT_VALID(pGroup);

	POSITION pos2 = pGroup->m_lstTasks.GetHeadPosition();
	while (pos2 != NULL)
	{
		CMFCTasksPaneTask* pTask = (CMFCTasksPaneTask*) pGroup->m_lstTasks.GetNext(pos2);

		if (pTask->m_hwndTask == hwndTask)
		{
			pTask->m_nWindowHeight = nWndHeight;

			if (!pGroup->m_bIsCollapsed)
			{
				AdjustScroll();
				ReposTasks();
				RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
			}

			return TRUE;
		}
	}

	return FALSE;
}

BOOL CMFCTasksPane::SetWindowHeight(HWND hwndTask, int nWndHeight)
{
	ENSURE(::IsWindow(hwndTask));

	int nGroup, nTask;
	if (GetTaskLocation(hwndTask, nGroup, nTask))
	{
		return SetWindowHeight(nGroup, hwndTask, nWndHeight);
	}

	return FALSE;
}

LRESULT CMFCTasksPane::OnSetFont(WPARAM wParam, LPARAM /*lParam*/)
{
	m_hFont = (HFONT) wParam;

	CreateFonts();
	AdjustScroll();
	ReposTasks();
	return 0;
}

LRESULT CMFCTasksPane::OnGetFont(WPARAM, LPARAM)
{
	return(LRESULT)(m_hFont != NULL ? m_hFont : ::GetStockObject(DEFAULT_GUI_FONT));
}

void CMFCTasksPane::CreateFonts()
{
	if (m_fontBold.GetSafeHandle() != NULL)
	{
		m_fontBold.DeleteObject();
	}
	if (m_fontBoldUnderline.GetSafeHandle() != NULL)
	{
		m_fontBoldUnderline.DeleteObject();
	}
	if (m_fontUnderline.GetSafeHandle() != NULL)
	{
		m_fontUnderline.DeleteObject();
	}

	CFont* pFont = CFont::FromHandle(m_hFont != NULL ? m_hFont :(HFONT) ::GetStockObject(DEFAULT_GUI_FONT));
	ASSERT_VALID(pFont);

	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));

	pFont->GetLogFont(&lf);

	lf.lfWeight = FW_BOLD;
	m_fontBold.CreateFontIndirect(&lf);

	lf.lfUnderline = TRUE;
	m_fontBoldUnderline.CreateFontIndirect(&lf);

	lf.lfWeight = FW_NORMAL;
	lf.lfUnderline = TRUE;
	m_fontUnderline.CreateFontIndirect(&lf);
}

void CMFCTasksPane::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CWnd::OnSettingChange(uFlags, lpszSection);

	AdjustScroll();
	ReposTasks();
}

HFONT CMFCTasksPane::SetFont(CDC* pDC)
{
	ASSERT_VALID(pDC);

	return(HFONT) ::SelectObject(pDC->GetSafeHdc(), m_hFont != NULL ? m_hFont : ::GetStockObject(DEFAULT_GUI_FONT));
}

CMFCTasksPaneTask* CMFCTasksPane::TaskHitTest(CPoint pt) const
{
	if (!m_rectTasks.PtInRect(pt))
	{
		return NULL;
	}

	// ---------------
	// Get active page
	// ---------------
	CMFCTasksPanePropertyPage* pActivePage = NULL;
	POSITION posPage = m_lstTasksPanes.FindIndex(m_arrHistoryStack[m_iActivePage]);
	ENSURE(posPage != NULL);

	pActivePage = (CMFCTasksPanePropertyPage*) m_lstTasksPanes.GetAt(posPage);
	ASSERT_VALID(pActivePage);

	// -----------------------------
	// Test all tasks in active page
	// -----------------------------
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition(); pos != NULL;)
	{
		CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetNext(pos);
		ASSERT_VALID(pGroup);

		if (pGroup->m_pPage == pActivePage)
		{
			for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition(); posTask != NULL;)
			{
				CMFCTasksPaneTask* pTask = (CMFCTasksPaneTask*) pGroup->m_lstTasks.GetNext(posTask);
				ASSERT_VALID(pTask);

				if (pTask->m_bVisible && pTask->m_rect.PtInRect(pt))
				{
					if (pTask->m_uiCommandID != 0) // ignore labels
					{
						return pTask;
					}
				}
			}
		}
	}

	return NULL;
}

CMFCTasksPaneTaskGroup* CMFCTasksPane::GroupCaptionHitTest(CPoint pt) const
{
	if (!m_bCanCollapse)
	{
		return NULL;
	}

	if (!m_rectTasks.PtInRect(pt))
	{
		return NULL;
	}

	// ---------------
	// Get active page
	// ---------------
	CMFCTasksPanePropertyPage* pActivePage = NULL;
	POSITION posPage = m_lstTasksPanes.FindIndex(m_arrHistoryStack[m_iActivePage]);
	ENSURE(posPage != NULL);

	pActivePage = (CMFCTasksPanePropertyPage*) m_lstTasksPanes.GetAt(posPage);
	ASSERT_VALID(pActivePage);

	// ------------------------------
	// Test all groups in active page
	// ------------------------------
	for (POSITION pos = m_lstTaskGroups.GetHeadPosition(); pos != NULL;)
	{
		CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetNext(pos);
		ASSERT_VALID(pGroup);

		if (pGroup->m_pPage == pActivePage)
		{
			if (pGroup->m_rect.PtInRect(pt))
			{
				return pGroup;
			}
		}
	}

	return NULL;
}

BOOL CMFCTasksPane::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint ptCursor;
	::GetCursorPos(&ptCursor);
	ScreenToClient(&ptCursor);

	CMFCTasksPaneTask* pTaskHit = TaskHitTest(ptCursor);
	if (m_pClickedTask != NULL && m_pClickedTask->m_bEnabled || pTaskHit != NULL && pTaskHit->m_bEnabled)
	{
		::SetCursor(afxGlobalData.GetHandCursor());
		return TRUE;
	}

	if (m_bCanCollapse &&(m_pClickedGroupCaption != NULL || GroupCaptionHitTest(ptCursor) != NULL))
	{
		::SetCursor(afxGlobalData.GetHandCursor());
		return TRUE;
	}

	return CDockablePane::OnSetCursor(pWnd, nHitTest, message);
}

void CMFCTasksPane::OnMouseMove(UINT nFlags, CPoint point)
{
	CDockablePane::OnMouseMove(nFlags, point);

	BOOL bUpdate = FALSE;

	if (m_bUseScrollButtons)
	{
		if (m_rectScrollUp.PtInRect(point) && IsScrollUpAvailable())
		{
			m_iScrollMode = -1;
			InvalidateRect(m_rectScrollUp);
		}
		else if (m_rectScrollDn.PtInRect(point) && IsScrollDnAvailable())
		{
			m_iScrollMode = 1;
			InvalidateRect(m_rectScrollDn);
		}
		else
		{
			m_iScrollMode = 0;
		}

		if (m_iScrollMode != 0)
		{
			SetTimer(nScrollTimerId, m_nScrollTimerDuration, NULL);
			return;
		}
	}

	CMFCTasksPaneTaskGroup* pHotGroup = GroupCaptionHitTest(point);
	CMFCTasksPaneTask* pHotTask = TaskHitTest(point);

	// ----------
	// No changes
	// ----------
	if (m_pHotTask == pHotTask && m_pHotGroupCaption == pHotGroup)
	{
		return;
	}

	// ----------------
	// No new hot areas
	// ----------------
	if (pHotTask == NULL && pHotGroup == NULL)
	{
		if (m_pHotGroupCaption != NULL)
		{
			// remove old group caption hotlight
			CRect rectUpdate = m_pHotGroupCaption->m_rect;
			m_pHotGroupCaption = NULL;
			if (m_pClickedGroupCaption == NULL)
			{
				ReleaseCapture();
			}

			RedrawWindow(rectUpdate, NULL, RDW_INVALIDATE | RDW_ERASE);
			bUpdate = TRUE;
		}

		if (m_pHotTask != NULL)
		{
			// remove old task hotlight
			CRect rectUpdate = m_pHotTask->m_rect;
			m_pHotTask = NULL;
			if (m_pClickedTask == NULL)
			{
				ReleaseCapture();
			}

			RedrawWindow(rectUpdate, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
			bUpdate = TRUE;
		}

		GetOwner()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);

		if (bUpdate)
		{
			UpdateWindow();
		}

		return;
	}

	// ---------------------
	// New hot group caption
	// ---------------------
	if (pHotGroup != NULL)
	{
		if (m_pHotGroupCaption == NULL)
		{
			if (GetCapture() != NULL)
			{
				return;
			}
			SetCapture();
		}
		else
		{
			// remove old group caption hotlight
			CRect rectTask = m_pHotGroupCaption->m_rect;
			m_pHotGroupCaption = NULL;
			RedrawWindow(rectTask, NULL, RDW_INVALIDATE | RDW_ERASE);

			bUpdate = TRUE;
		}

		// remove old task hotlight
		if (m_pHotTask != NULL)
		{
			CRect rectUpdate = m_pHotTask->m_rect;
			m_pHotTask = NULL;
			RedrawWindow(rectUpdate, NULL, RDW_INVALIDATE | RDW_ERASE);

			bUpdate = TRUE;
		}

		// add new group caption hotlight
		m_pHotGroupCaption = pHotGroup;
		RedrawWindow(pHotGroup->m_rect, NULL, RDW_INVALIDATE | RDW_ERASE);

		bUpdate = TRUE;
	}

	// ------------
	// New hot task
	// ------------
	else if (pHotTask != NULL)
	{
		if (!pHotTask->m_bEnabled)
		{
			GetOwner()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
			return;
		}

		if (m_pHotTask == NULL)
		{
			if (GetCapture() != NULL)
			{
				return;
			}
			SetCapture();
		}
		else
		{
			// remove old task hotlight
			CRect rectTask = m_pHotTask->m_rect;
			m_pHotTask = NULL;
			RedrawWindow(rectTask, NULL, RDW_INVALIDATE | RDW_ERASE);

			bUpdate = TRUE;
		}

		// remove old group caption hotlight
		if (m_pHotGroupCaption != NULL)
		{
			CRect rectUpdate = m_pHotGroupCaption->m_rect;
			m_pHotGroupCaption = NULL;
			RedrawWindow(rectUpdate, NULL, RDW_INVALIDATE | RDW_ERASE);

			bUpdate = TRUE;
		}

		// add new task hotlight
		m_pHotTask = pHotTask;
		RedrawWindow(pHotTask->m_rect, NULL, RDW_INVALIDATE | RDW_ERASE);

		bUpdate = TRUE;

		if (pHotTask->m_uiCommandID != 0)
		{
			ShowCommandMessageString(pHotTask->m_uiCommandID);
		}
	}

	if (bUpdate)
	{
		UpdateWindow();
	}
}

void CMFCTasksPane::OnLButtonUp(UINT nFlags, CPoint point)
{
	CDockablePane::OnLButtonUp(nFlags, point);

	if (m_pHotTask == NULL && m_pClickedTask == NULL && m_pHotGroupCaption == NULL && m_pClickedGroupCaption == NULL)
	{
		return;
	}

	ReleaseCapture();

	// --------------------------
	// Handle group caption click
	// --------------------------
	CMFCTasksPaneTaskGroup* pHotGroupCaption = m_pHotGroupCaption;
	BOOL bIsGroupCaptionClick = (m_pHotGroupCaption != NULL && m_pHotGroupCaption == GroupCaptionHitTest(point) && m_pClickedGroupCaption == m_pHotGroupCaption);

	m_pClickedGroupCaption = NULL;

	if (bIsGroupCaptionClick)
	{
		m_pHotGroupCaption = NULL;

		pHotGroupCaption->m_bIsCollapsed = !pHotGroupCaption->m_bIsCollapsed;

		if (m_bAnimationEnabled)
		{
			m_pAnimatedGroup = pHotGroupCaption;
			m_sizeAnim = m_pAnimatedGroup->m_rectGroup.Size();

			SetTimer(nAnimTimerId, m_nAnimTimerDuration, NULL);
			m_nLastAnimTime = clock();
		}

		AdjustScroll();
		ReposTasks();
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);

		// Trigger mouse move event(to change selection notification):
		SendMessage(WM_MOUSEMOVE, nFlags, MAKELPARAM(point.x, point.y));
		return;
	}
	else
	{
		CRect rectGroupCaption = (m_pHotGroupCaption != NULL) ? m_pHotGroupCaption->m_rect : CRect(0, 0, 0, 0);
		RedrawWindow(rectGroupCaption, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	}

	// -----------------
	// Handle task click
	// -----------------
	CMFCTasksPaneTask* pHotTask = m_pHotTask;
	BOOL bIsTaskClick = (m_pHotTask != NULL && m_pHotTask == TaskHitTest(point) && m_pClickedTask == m_pHotTask);

	CRect rectTask = (m_pHotTask != NULL) ? m_pHotTask->m_rect : CRect(0, 0, 0, 0);

	m_pHotTask = NULL;
	m_pClickedTask = NULL;

	RedrawWindow(rectTask, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);

	if (bIsTaskClick)
	{
		// Find task number and group number:
		ASSERT_VALID(pHotTask->m_pGroup);

		int nTaskNumber = -1;
		int i = 0;

		for (POSITION posTask = pHotTask->m_pGroup->m_lstTasks.GetHeadPosition(); posTask != NULL; i++)
		{
			CMFCTasksPaneTask* pTask = (CMFCTasksPaneTask*) pHotTask->m_pGroup->m_lstTasks.GetNext(posTask);
			ASSERT_VALID(pTask);

			if (pTask == pHotTask)
			{
				nTaskNumber = i;
				break;
			}
		}

		int nGroupNumber = -1;
		i = 0;

		for (POSITION pos = m_lstTaskGroups.GetHeadPosition(); pos != NULL; i++)
		{
			CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetNext(pos);
			ASSERT_VALID(pGroup);

			if (pHotTask->m_pGroup == pGroup)
			{
				nGroupNumber = i;
				break;
			}
		}

		OnClickTask(nGroupNumber, nTaskNumber, pHotTask->m_uiCommandID, pHotTask->m_dwUserData);
	}
}

void CMFCTasksPane::OnCancelMode()
{
	CDockablePane::OnCancelMode();

	if (m_pHotTask != NULL || m_pClickedTask != NULL)
	{
		CRect rectTask = m_pHotTask != NULL ? m_pHotTask->m_rect : CRect(0, 0, 0, 0);
		m_pHotTask = NULL;
		m_pClickedTask = NULL;
		ReleaseCapture();
		RedrawWindow(rectTask, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	}

	if (m_pHotGroupCaption != NULL || m_pClickedGroupCaption != NULL)
	{
		CRect rectTask = m_pHotGroupCaption != NULL ? m_pHotGroupCaption->m_rect : CRect(0, 0, 0, 0);
		m_pHotGroupCaption = NULL;
		m_pClickedGroupCaption = NULL;
		ReleaseCapture();
		RedrawWindow(rectTask, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	}

	m_pClickedTask = NULL;
	m_pClickedGroupCaption = NULL;
}

void CMFCTasksPane::OnClickTask(int /*nGroupNumber*/, int /*nTaskNumber*/, UINT uiCommandID, DWORD /*dwUserData*/)
{
	if (uiCommandID != 0)
	{
		GetOwner()->PostMessage(WM_COMMAND, uiCommandID);
	}
}

void CMFCTasksPane::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_pClickedGroupCaption = GroupCaptionHitTest(point);
	m_pClickedTask = TaskHitTest(point);

	if (m_pClickedTask != NULL)
	{
		ASSERT_VALID(m_pClickedTask);
		CRect rect = m_pClickedTask->m_rect;
		CPoint pt(rect.left, rect.top);
		ClientToScreen(&pt);
		LPARAM lParam = MAKELPARAM(pt.x, pt.y);

		::NotifyWinEvent(EVENT_OBJECT_FOCUS, GetSafeHwnd(), OBJID_CLIENT, (LONG)lParam);
	}

	CRect rectClient;
	GetClientRect(&rectClient);

	if (!rectClient.PtInRect(point) || m_rectToolbar.PtInRect(point))
	{
		CDockablePane::OnLButtonDown(nFlags, point);
	}
	else if (IsToolBox())
	{
		SetFocus();
	}
}

int CMFCTasksPane::AddMRUFilesList(int nGroup, int nMaxFiles /* = 4 */)
{
	POSITION pos = m_lstTaskGroups.FindIndex(nGroup);
	if (pos == NULL)
	{
		ASSERT(FALSE);
		return -1;
	}

	CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetAt(pos);
	ASSERT_VALID(pGroup);

	POSITION posFirstMRUFile = NULL;

	// Clean up old MRU files from the group:
	for (pos = pGroup->m_lstTasks.GetHeadPosition(); pos != NULL;)
	{
		POSITION posSave = pos;

		CMFCTasksPaneTask* pTask = (CMFCTasksPaneTask*) pGroup->m_lstTasks.GetNext(pos);
		ASSERT_VALID(pTask);

		if (pTask->m_uiCommandID >= ID_FILE_MRU_FILE1 && pTask->m_uiCommandID <= ID_FILE_MRU_FILE16)
		{
			posFirstMRUFile = posSave;
			pGroup->m_lstTasks.GetNext(posFirstMRUFile);

			delete pGroup->m_lstTasks.GetAt(posSave);
			pGroup->m_lstTasks.RemoveAt(posSave);
		}
	}

	CRecentFileList* pRecentFileList = ((CWinAppEx*)AfxGetApp())->m_pRecentFileList;
	if (pRecentFileList == NULL)
	{
		return(int) pGroup->m_lstTasks.GetCount() - 1;
	}

	int nNum = min(pRecentFileList->GetSize(), nMaxFiles);

	// Add new MRU files to the group:
	for (int i = 0; i < nNum; i++)
	{
		CString strName;
		if (GetMRUFileName(pRecentFileList, i, strName))
		{
			CMFCTasksPaneTask* pTask = new CMFCTasksPaneTask(pGroup, strName, -1, ID_FILE_MRU_FILE1 + i);
			ASSERT_VALID(pTask);

			if (posFirstMRUFile == NULL)
			{
				pGroup->m_lstTasks.AddTail(pTask);
			}
			else
			{
				pGroup->m_lstTasks.InsertBefore(posFirstMRUFile, pTask);
			}
		}
	}

	AdjustScroll();
	ReposTasks();
	return(int) pGroup->m_lstTasks.GetCount() - 1;
}

BOOL CMFCTasksPane::GetMRUFileName(CRecentFileList* pRecentFileList, int nIndex, CString &strName)
{
	ENSURE(pRecentFileList != NULL);

	if ((*pRecentFileList)[nIndex].GetLength() != 0)
	{
		const int MAX_NAME_LEN = 512;

		TCHAR lpcszBuffer [MAX_NAME_LEN + 1];
		memset(lpcszBuffer, 0, MAX_NAME_LEN * sizeof(TCHAR));

		if (GetFileTitle((*pRecentFileList)[nIndex], lpcszBuffer, MAX_NAME_LEN) == 0)
		{
			strName = lpcszBuffer;
			return TRUE;
		}

		ASSERT(FALSE);
	}

	return FALSE;
}

CScrollBar* CMFCTasksPane::GetScrollBarCtrl(int nBar) const
{
	if (nBar == SB_HORZ || m_wndScrollVert.GetSafeHwnd() == NULL)
	{
		return NULL;
	}

	return(CScrollBar* ) &m_wndScrollVert;
}

void CMFCTasksPane::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/)
{
	int nPrevOffset = m_nVertScrollOffset;

	switch(nSBCode)
	{
	case SB_LINEUP:
		m_nVertScrollOffset--;
		break;

	case SB_LINEDOWN:
		m_nVertScrollOffset++;
		break;

	case SB_TOP:
		m_nVertScrollOffset = 0;
		break;

	case SB_BOTTOM:
		m_nVertScrollOffset = m_nVertScrollTotal;
		break;

	case SB_PAGEUP:
		m_nVertScrollOffset -= m_nVertScrollPage;
		break;

	case SB_PAGEDOWN:
		m_nVertScrollOffset += m_nVertScrollPage;
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		m_nVertScrollOffset = nPos;
		break;

	default:
		return;
	}

	m_nVertScrollOffset = min(max(0, m_nVertScrollOffset), m_nVertScrollTotal - m_nVertScrollPage + 1);

	if (m_nVertScrollOffset == nPrevOffset)
	{
		return;
	}

	SetScrollPos(SB_VERT, m_nVertScrollOffset);

	AdjustScroll();
	ReposTasks();
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}

void CMFCTasksPane::SetScrollSizes()
{
	ASSERT_VALID(this);

	if (m_wndScrollVert.GetSafeHwnd() == NULL)
	{
		return;
	}

	if (m_nRowHeight == 0)
	{
		m_nVertScrollPage = 0;
		m_nVertScrollTotal = 0;
		m_nVertScrollOffset = 0;
	}
	else
	{
		int nPageHeight = m_rectTasks.Height();
		if (m_bUseScrollButtons)
		{
			nPageHeight -= m_iScrollBtnHeight + nBorderSize;
		}
		BOOL bMultiPage = (m_lstTasksPanes.GetCount() > 1);
		if ((m_bUseNavigationToolbar || ForceShowNavToolbar()) && bMultiPage)
		{
			nPageHeight += m_rectToolbar.Height();
		}

		m_nVertScrollPage = nPageHeight / m_nRowHeight - 1;

		int nTotalHeight = ReposTasks(TRUE);
		if (nTotalHeight == 0 || nTotalHeight <= nPageHeight)
		{
			m_nVertScrollPage = 0;
			m_nVertScrollTotal = 0;
			m_nVertScrollOffset = 0;
		}
		else
		{
			m_nVertScrollTotal = nTotalHeight / m_nRowHeight - 1;
		}

		m_nVertScrollOffset = min(max(0, m_nVertScrollOffset), m_nVertScrollTotal - m_nVertScrollPage + 1);
	}

	if (!m_bUseScrollButtons)
	{
		SCROLLINFO si;

		ZeroMemory(&si, sizeof(SCROLLINFO));
		si.cbSize = sizeof(SCROLLINFO);

		si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
		si.nMin = 0;
		si.nMax = m_nVertScrollTotal;
		si.nPage = m_nVertScrollPage;
		si.nPos = m_nVertScrollOffset;

		SetScrollInfo(SB_VERT, &si, TRUE);
	}
	m_wndScrollVert.EnableScrollBar(!m_bUseScrollButtons && m_nVertScrollTotal > 0 ? ESB_ENABLE_BOTH : ESB_DISABLE_BOTH);
}

void CMFCTasksPane::AdjustScroll()
{
	ASSERT_VALID(this);

	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	if (IsToolBox())
	{
		rectClient.DeflateRect(1, 1);
	}

	// --------------------------
	// Adjust navigation toolbar:
	// --------------------------
	CRect rectToolbarOld = m_rectToolbar;
	BOOL bMultiPage = (m_lstTasksPanes.GetCount() > 1);
	if ((m_bUseNavigationToolbar || ForceShowNavToolbar()) && bMultiPage)
	{
		int nToolbarHeight = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

		m_rectToolbar = rectClient;
		m_rectToolbar.bottom = m_rectToolbar.top + nToolbarHeight;

		rectClient.top += m_rectToolbar.Height();

		m_wndToolBar.SetWindowPos(NULL, m_rectToolbar.left, m_rectToolbar.top, m_rectToolbar.Width(), nToolbarHeight, SWP_NOACTIVATE | SWP_NOZORDER);
		m_wndToolBar.ShowWindow(TRUE);
	}
	else
	{
		m_rectToolbar.SetRectEmpty();
		m_wndToolBar.ShowWindow(FALSE);
	}

	// --------------------
	// Calculate work area:
	// --------------------
	m_rectTasks = rectClient;

	// ------------------
	// Adjust scroll bar:
	// ------------------
	SetScrollSizes();

	m_wndScrollVert.EnableWindow(!m_bUseScrollButtons);
	if (!m_bUseScrollButtons && m_nVertScrollTotal > 0)
	{
		int cxScroll = ::GetSystemMetrics(SM_CXHSCROLL);

		m_rectTasks.right -= cxScroll;

		m_wndScrollVert.SetWindowPos(NULL, rectClient.right - cxScroll, rectClient.top, cxScroll, rectClient.Height(), SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOACTIVATE);
		rectClient.right -= cxScroll;
	}
	else
	{
		m_wndScrollVert.SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW);
	}

	// ----------------------
	// Adjust scroll buttons:
	// ----------------------
	CRect rectScrollUpOld = m_rectScrollUp;
	CRect rectScrollDnOld = m_rectScrollDn;

	m_rectScrollUp.SetRectEmpty();
	m_rectScrollDn.SetRectEmpty();

	if (m_bUseScrollButtons)
	{
		if (IsScrollUpAvailable())
		{
			m_rectScrollUp = rectClient;
			m_rectScrollUp.top += nBorderSize;
			m_rectScrollUp.bottom = m_rectScrollUp.top + m_iScrollBtnHeight;

			rectClient.top += m_iScrollBtnHeight + nBorderSize;
		}

		if (IsScrollDnAvailable())
		{
			m_rectScrollDn = rectClient;
			m_rectScrollDn.top = m_rectScrollDn.bottom - m_iScrollBtnHeight;

			rectClient.bottom -= m_iScrollBtnHeight + nBorderSize;
		}

		m_rectTasks = rectClient;
	}
	else if (m_pAnimatedGroup != NULL/* animation is in progress */)
	{
		KillTimer(nScrollTimerId);
		m_iScrollMode = 0;
	}

	// ------------------------------
	// Invalidate navigation toolbar:
	// ------------------------------
	if (rectToolbarOld != m_rectToolbar)
	{
		InvalidateRect(m_rectToolbar);
		InvalidateRect(rectToolbarOld);
		UpdateWindow();
	}

	// --------------------------
	// Invalidate scroll buttons:
	// --------------------------
	BOOL bScrollButtonsChanged = FALSE;

	if (rectScrollUpOld != m_rectScrollUp)
	{
		InvalidateRect(rectScrollUpOld);
		InvalidateRect(m_rectScrollUp);

		bScrollButtonsChanged = TRUE;
	}

	if (rectScrollDnOld != m_rectScrollDn)
	{
		InvalidateRect(rectScrollDnOld);
		InvalidateRect(m_rectScrollDn);

		bScrollButtonsChanged = TRUE;
	}

	if (bScrollButtonsChanged)
	{
		UpdateWindow();
	}
}

void CMFCTasksPane::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CMemDC memDC(dc, this);
	CDC* pDC = &memDC.GetDC();

	CRect rect;
	GetClientRect(rect);

	CRect rectFrame = rect;
	rectFrame.SetRectEmpty();

	if (IsToolBox())
	{
		rectFrame = rect;
		rect.DeflateRect(1, 1);
	}

	OnDrawTasks(pDC, rect);

	if (!rectFrame.IsRectEmpty())
	{
		CMFCVisualManager::GetInstance()->OnDrawToolBoxFrame(pDC, rectFrame);
	}
}

BOOL CMFCTasksPane::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CMFCTasksPane::OnDestroy()
{
	while (!m_lstTaskGroups.IsEmpty())
	{
		delete m_lstTaskGroups.RemoveHead();
	}

	CDockablePane::OnDestroy();
}

void CMFCTasksPane::RebuildMenu()
{
	ASSERT(m_iActivePage >= 0);
	ASSERT(m_iActivePage <= m_arrHistoryStack.GetUpperBound());
	ASSERT(m_arrHistoryStack[m_iActivePage] >= 0);
	ASSERT(m_arrHistoryStack[m_iActivePage] <= m_lstTasksPanes.GetCount()-1);

	if (m_menuOther.m_hMenu != NULL)
	{
		m_menuOther.DestroyMenu();
	}

	HMENU hMenu = CreateMenu();
	m_menuOther.Attach(hMenu);

	m_wndToolBar.UpdateButtons();

	EnableButton(AFX_HTLEFTBUTTON, m_iActivePage > 0);
	EnableButton(AFX_HTRIGHTBUTTON, m_iActivePage < m_arrHistoryStack.GetUpperBound());
}

void CMFCTasksPane::SaveHistory(int nPageIdx)
{
	ASSERT(nPageIdx >= 0);
	ASSERT(nPageIdx <= m_lstTasksPanes.GetCount()-1);

	ASSERT(m_iActivePage >= 0);
	ASSERT(m_iActivePage <= m_arrHistoryStack.GetUpperBound());
	ASSERT(m_arrHistoryStack[m_iActivePage] >= 0);
	ASSERT(m_arrHistoryStack[m_iActivePage] <= m_lstTasksPanes.GetCount()-1);

	if (nPageIdx == m_arrHistoryStack[m_iActivePage])
	{
		return;
	}

	if (m_iActivePage < m_arrHistoryStack.GetUpperBound())
	{
		int nStackTailCount = (int) m_arrHistoryStack.GetUpperBound() - m_iActivePage;
		m_arrHistoryStack.RemoveAt(m_iActivePage+1, nStackTailCount);
	}
	if (m_arrHistoryStack.GetSize() == m_nMaxHistory)
	{
		m_arrHistoryStack.RemoveAt(0);
		if (m_iActivePage > 0)
		{
			m_iActivePage--;
		}
	}
	m_arrHistoryStack.Add(nPageIdx);
}

void CMFCTasksPane::ChangeActivePage(int nNewPageHistoryIdx, int nOldPageHistoryIdx)
{
	ASSERT(nNewPageHistoryIdx >= 0);
	ASSERT(nNewPageHistoryIdx <= m_arrHistoryStack.GetUpperBound());
	ASSERT(nOldPageHistoryIdx >= 0);
	ASSERT(nOldPageHistoryIdx <= m_arrHistoryStack.GetUpperBound());

	int nNewPageIdx = m_arrHistoryStack[nNewPageHistoryIdx];
	int nOldPageIdx = m_arrHistoryStack[nOldPageHistoryIdx];

	ASSERT(nNewPageIdx >= 0);
	ASSERT(nNewPageIdx <= m_lstTasksPanes.GetCount()-1);
	ASSERT(nOldPageIdx >= 0);
	ASSERT(nOldPageIdx <= m_lstTasksPanes.GetCount()-1);

	if (nNewPageIdx == nOldPageIdx)
	{
		// Already active, do nothing
		return;
	}

	if (GetSafeHwnd() == NULL)
	{
		OnActivateTasksPanePage();
		RebuildMenu();
		return;
	}

	// ------------------------------------------
	// Hide all windows for previous active page:
	// ------------------------------------------
	CMFCTasksPanePropertyPage* pOldPage = NULL;
	POSITION posPage = m_lstTasksPanes.FindIndex(nOldPageIdx);
	ENSURE(posPage != NULL);

	pOldPage = (CMFCTasksPanePropertyPage*) m_lstTasksPanes.GetAt(posPage);
	ASSERT_VALID(pOldPage);

	for (POSITION pos = m_lstTaskGroups.GetHeadPosition(); pos != NULL; )
	{
		CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetNext(pos);
		ASSERT_VALID(pGroup);

		if (pGroup->m_pPage == pOldPage)
		{
			for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition(); posTask != NULL;)
			{
				CMFCTasksPaneTask* pTask = (CMFCTasksPaneTask*) pGroup->m_lstTasks.GetNext(posTask);
				ASSERT_VALID(pTask);

				if (pTask->m_hwndTask != NULL)
				{
					CWnd* pChildWnd = CWnd::FromHandle(pTask->m_hwndTask);
					ASSERT_VALID(pChildWnd);

					pChildWnd->ShowWindow(SW_HIDE);
				}
			}
		}
	}

	// ------------------
	// Update page title:
	// ------------------
	UpdateCaption();

	// ------------------
	// Change active page
	// ------------------
	OnActivateTasksPanePage();

	RebuildMenu();

	m_nVertScrollOffset = 0;
	AdjustScroll();
	ReposTasks();

	Invalidate();
	UpdateWindow();
}

void CMFCTasksPane::SetActivePage(int nPageIdx)
{
	ASSERT(nPageIdx >= 0);
	ASSERT(nPageIdx < m_lstTasksPanes.GetCount());

	// ------------------------------------------
	// Activate the page specified by index
	// saving the current one in the history list
	// ------------------------------------------
	if (GetActivePage() != nPageIdx)
	{
		SaveHistory(nPageIdx);
		int nOldActivePage = m_iActivePage;
		m_iActivePage = (int) m_arrHistoryStack.GetUpperBound();
		ChangeActivePage(m_iActivePage, nOldActivePage);
	}
}

void CMFCTasksPane::SetCaptionButtons()
{
	CDockablePane::SetCaptionButtons();

	m_arrButtons.Add(new CMFCCaptionButton(AFX_HTLEFTBUTTON, TRUE));
	m_arrButtons.Add(new CMFCCaptionButton(AFX_HTRIGHTBUTTON, TRUE));

	CMFCCaptionMenuButton *pBtn = new CMFCCaptionMenuButton(AFX_HTMENU);
	pBtn->m_bOSMenu = FALSE;
	m_arrButtons.Add(pBtn);
}

BOOL CMFCTasksPane::GetPageByGroup(int nGroup, int &nPage) const
{
	ASSERT(nGroup >= 0);
	ASSERT(nGroup < m_lstTaskGroups.GetCount());

	CMFCTasksPaneTaskGroup* pGroup = GetTaskGroup(nGroup);
	ASSERT_VALID(pGroup);

	int nPageCount = 0;
	for (POSITION posPage = m_lstTasksPanes.GetHeadPosition(); posPage != NULL; nPageCount++)
	{
		CMFCTasksPanePropertyPage* pPage = (CMFCTasksPanePropertyPage*) m_lstTasksPanes.GetNext(posPage);
		ASSERT_VALID(pPage);

		if (pPage == pGroup->m_pPage)
		{
			nPage = nPageCount;
			return TRUE;
		}
	}

	ASSERT(FALSE);
	return FALSE;
}

void CMFCTasksPane::OnPressButtons(UINT nHit)
{
	CDockablePane::OnPressButtons(nHit);

	switch(nHit)
	{
	case AFX_HTLEFTBUTTON:
		OnPressBackButton();
		break;

	case AFX_HTRIGHTBUTTON:
		OnPressForwardButton();
		break;

	case AFX_HTMENU:
		{
			CMFCCaptionMenuButton* pbtn = (CMFCCaptionMenuButton*)FindButtonByHit(AFX_HTMENU);
			if (pbtn != NULL)
			{
				m_bMenuBtnPressed = TRUE;
				OnPressOtherButton(pbtn, this);
				m_bMenuBtnPressed = FALSE;
			}
		}
		break;
	}
}

void CMFCTasksPane::OnPressBackButton()
{
	// --------------------------
	// Handle Back caption button
	// --------------------------
	if (m_iActivePage > 0)
	{
		ASSERT(m_iActivePage >= 0);
		ASSERT(m_iActivePage <= m_arrHistoryStack.GetUpperBound());

		int nOldActivePage = m_iActivePage;
		m_iActivePage--;
		ChangeActivePage(m_iActivePage, nOldActivePage);
	}
}

void CMFCTasksPane::OnPressForwardButton()
{
	// -----------------------------
	// Handle Forward caption button
	// -----------------------------
	if (m_iActivePage < m_arrHistoryStack.GetUpperBound())
	{
		ASSERT(m_iActivePage >= 0);
		ASSERT(m_iActivePage <= m_arrHistoryStack.GetUpperBound());

		int nOldActivePage = m_iActivePage;
		m_iActivePage++;
		ChangeActivePage(m_iActivePage, nOldActivePage);
	}
}

void CMFCTasksPane::OnPressHomeButton()
{
	ASSERT(m_iActivePage >= 0);
	ASSERT(m_iActivePage <= m_arrHistoryStack.GetUpperBound());

	if (GetActivePage() != 0)
	{
		SetActivePage(0);
	}
}

void CMFCTasksPane::OnPressOtherButton(CMFCCaptionMenuButton* pbtn, CWnd* pWndOwner)
{
	ASSERT_VALID(pWndOwner);
	if (pbtn != NULL)
	{
		// ---------------------------
		// Handle Other caption button
		// ---------------------------
		ASSERT_VALID(pbtn);
		pbtn->ShowMenu(m_menuOther, pWndOwner);

		if (pbtn->m_nMenuResult != 0)
		{
			int nMenuIndex = CMFCPopupMenuBar::GetLastCommandIndex();
			if (nMenuIndex >= 0)
			{
				SetActivePage(nMenuIndex);
			}
		}
	}
}

void CMFCTasksPane::DrawCaption(CDC* pDC, CRect rectCaption)
{
	ASSERT_VALID(pDC);

	// Enable or disable Taskpane specific caption buttons:
	BOOL bMultiPage = (m_lstTasksPanes.GetCount() > 1);
	for (int i = 0; i < m_arrButtons.GetSize(); i ++)
	{
		CMFCCaptionButton* pbtn = m_arrButtons [i];
		ASSERT_VALID(pbtn);

		switch(pbtn->GetHit())
		{
		case AFX_HTLEFTBUTTON:
		case AFX_HTRIGHTBUTTON:
		case AFX_HTMENU:
			pbtn->m_bHidden = pbtn->m_bHidden || !bMultiPage || m_bUseNavigationToolbar;
			break;
		}

		if (pbtn->GetHit() == AFX_HTLEFTBUTTON)
		{
			pbtn->m_bEnabled = IsBackButtonEnabled();
		}
		if (pbtn->GetHit() == AFX_HTRIGHTBUTTON)
		{
			pbtn->m_bEnabled = IsForwardButtonEnabled();
		}
	}

	UpdateTooltips();

	CDockablePane::DrawCaption(pDC, rectCaption);
}

void CMFCTasksPane::Serialize(CArchive& ar)
{
	CDockablePane::Serialize(ar);

	if (ar.IsLoading())
	{
		// Load margin settings:
		ar >> m_nVertMargin;
		ar >> m_nHorzMargin;
		ar >> m_nGroupVertOffset;
		ar >> m_nGroupCaptionHeight;
		ar >> m_nGroupCaptionHorzOffset;
		ar >> m_nGroupCaptionVertOffset;
		ar >> m_nTasksHorzOffset;
		ar >> m_nTasksIconHorzOffset;
		ar >> m_nTasksIconVertOffset;

		// Load active page index:
		int nActivePage = 0;
		ar >> nActivePage;
		if (nActivePage < 0 || nActivePage >= m_lstTasksPanes.GetCount())
		{
			nActivePage = 0;
		}

		// Load the titles of pages:
		CStringArray arrPagesNames;
		arrPagesNames.Serialize(ar);
		if (arrPagesNames.GetSize() == m_lstTasksPanes.GetCount())
		{
			int i = 0;
			POSITION pos = m_lstTasksPanes.GetHeadPosition();
			while (pos != NULL && i < arrPagesNames.GetSize())
			{
				CMFCTasksPanePropertyPage* pPage = (CMFCTasksPanePropertyPage*) m_lstTasksPanes.GetNext(pos);
				ASSERT_VALID(pPage);

				pPage->m_strName = arrPagesNames[i++];
			}
		}

		// Change active page:
		SetActivePage(nActivePage);

		m_nVertScrollOffset = 0;
		AdjustScroll();

		// Load taskpane's caption:
		ar >> m_strCaption;
		UpdateCaption();
	}
	else
	{
		// Save margin settings:
		ar << m_nVertMargin;
		ar << m_nHorzMargin;
		ar << m_nGroupVertOffset;
		ar << m_nGroupCaptionHeight;
		ar << m_nGroupCaptionHorzOffset;
		ar << m_nGroupCaptionVertOffset;
		ar << m_nTasksHorzOffset;
		ar << m_nTasksIconHorzOffset;
		ar << m_nTasksIconVertOffset;

		// Save active page index:
		ar << GetActivePage();

		// Save the titles of pages:
		CStringArray arrPagesNames;
		for (POSITION pos = m_lstTasksPanes.GetHeadPosition(); pos != NULL;)
		{
			CMFCTasksPanePropertyPage* pPage = (CMFCTasksPanePropertyPage*) m_lstTasksPanes.GetNext(pos);
			ASSERT_VALID(pPage);

			arrPagesNames.Add(pPage->m_strName);
		}

		arrPagesNames.Serialize(ar);

		// Save taskpane's caption:
		ar << m_strCaption;
	}
}

void CMFCTasksPane::SetCaption(LPCTSTR lpszName)
{
	ENSURE(lpszName != NULL);

	m_strCaption = lpszName;
	SetWindowText(lpszName);

	UpdateCaption();
}

void CMFCTasksPane::SetPageCaption(int nPageIdx, LPCTSTR lpszName)
{
	ASSERT(nPageIdx >= 0);
	ASSERT(nPageIdx < m_lstTasksPanes.GetCount());
	ENSURE(lpszName != NULL);

	POSITION pos = m_lstTasksPanes.FindIndex(nPageIdx);
	ENSURE(pos != NULL);
	CMFCTasksPanePropertyPage* pPage = (CMFCTasksPanePropertyPage*)m_lstTasksPanes.GetAt(pos);
	ASSERT_VALID(pPage);

	pPage->m_strName = lpszName;

	UpdateCaption();
}

BOOL CMFCTasksPane::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_LBUTTONDBLCLK)
	{
		for (POSITION pos = m_lstTaskGroups.GetHeadPosition(); pos != NULL;)
		{
			CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetNext(pos);
			ASSERT_VALID(pGroup);

			for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition(); posTask != NULL;)
			{
				CMFCTasksPaneTask* pTask = (CMFCTasksPaneTask*) pGroup->m_lstTasks.GetNext(posTask);
				ASSERT_VALID(pTask);

				if (pTask->m_hwndTask == pMsg->hwnd)
					return CDockablePane::PreTranslateMessage(pMsg);
			}
		}

		CPoint point(LOWORD(pMsg->lParam), HIWORD(pMsg->lParam));

		CRect rectClient;
		GetClientRect(&rectClient);

		if (IsToolBox())
		{
			rectClient.DeflateRect(1, 1);
		}

		if (rectClient.PtInRect(point))
		{
			return TRUE;
		}
	}

	return CDockablePane::PreTranslateMessage(pMsg);
}

CPaneFrameWnd* CMFCTasksPane::CreateDefaultMiniframe(CRect rectInitial)
{
	ASSERT_VALID(this);

	if (GetStyle() & CBRS_FLOAT_MULTI)
	{
		m_pMiniFrameRTC = RUNTIME_CLASS(CMultiPaneFrameWnd);
	}
	else
	{
		m_pMiniFrameRTC = RUNTIME_CLASS(CMFCTasksPaneFrameWnd);
	}

	return CPane::CreateDefaultMiniframe(rectInitial);
}

void CMFCTasksPane::OnTrackCaptionButtons(CPoint point)
{
	if (!m_bMenuBtnPressed)
	{
		CDockablePane::OnTrackCaptionButtons(point);
	}
}

void CMFCTasksPane::StopCaptionButtonsTracking()
{
	if (!m_bMenuBtnPressed)
	{
		CDockablePane::StopCaptionButtonsTracking();
	}
}

BOOL CMFCTasksPane::OnNeedTipText(UINT id, NMHDR* pNMH, LRESULT* pResult)
{
	static CString strTipText;

	ENSURE(pNMH != NULL);

	if (m_pToolTip->GetSafeHwnd() == NULL || pNMH->hwndFrom != m_pToolTip->GetSafeHwnd())
	{
		return FALSE;
	}

	LPNMTTDISPINFO pTTDispInfo = (LPNMTTDISPINFO) pNMH;
	ASSERT((pTTDispInfo->uFlags & TTF_IDISHWND) == 0);

	if (pNMH->idFrom == AFX_CONTROLBAR_BUTTONS_NUM + 1)
	{
		ENSURE(strTipText.LoadString(ID_AFXBARRES_TASKPANE_BACK));

		pTTDispInfo->lpszText = const_cast<LPTSTR>((LPCTSTR) strTipText);
		return TRUE;
	}
	else if (pNMH->idFrom == AFX_CONTROLBAR_BUTTONS_NUM + 2)
	{
		ENSURE(strTipText.LoadString(ID_AFXBARRES_TASKPANE_FORWARD));

		pTTDispInfo->lpszText = const_cast<LPTSTR>((LPCTSTR) strTipText);
		return TRUE;
	}
	else if (pNMH->idFrom == AFX_CONTROLBAR_BUTTONS_NUM + 3)
	{
		ENSURE(strTipText.LoadString(ID_AFXBARRES_TASKPANE_OTHER));

		pTTDispInfo->lpszText = const_cast<LPTSTR>((LPCTSTR) strTipText);
		return TRUE;
	}

	return CDockablePane::OnNeedTipText(id, pNMH, pResult);
}

void CMFCTasksPane::OnOK()
{
	if (IsToolBox())
	{
		return;
	}

	CWnd* pFocusWnd = CWnd::GetFocus();

	// Send the IDOK command to the focused task window
	if (pFocusWnd != NULL && IsChild(pFocusWnd))
	{
		while (pFocusWnd != this)
		{
			CWnd* pParentWnd = pFocusWnd->GetParent();

			if (pParentWnd == this)
			{
				const MSG* pMsg = GetCurrentMessage();
				pFocusWnd->SendMessage(WM_COMMAND, pMsg->wParam, pMsg->lParam);
				break;
			}

			pFocusWnd = pFocusWnd->GetParent();
		}
	}
}

void CMFCTasksPane::OnCancel()
{
	if (IsToolBox())
	{
		return;
	}

	CWnd* pFocusWnd = CWnd::GetFocus();

	// Send the IDCANCEL command to the focused task window
	if (pFocusWnd != NULL && IsChild(pFocusWnd))
	{
		while (pFocusWnd != this)
		{
			CWnd* pParentWnd = pFocusWnd->GetParent();

			if (pParentWnd == this)
			{
				const MSG* pMsg = GetCurrentMessage();
				pFocusWnd->SendMessage(WM_COMMAND, pMsg->wParam, pMsg->lParam);
				break;
			}

			pFocusWnd = pFocusWnd->GetParent();
		}
	}

	// Set the focus to the application window
	CFrameWnd* pTopFrame = ::AFXGetTopLevelFrame(this);

	if (::IsWindow(pTopFrame->GetSafeHwnd()))
	{
		pTopFrame->SetFocus();
	}
}

BOOL CMFCTasksPane::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (CMFCPopupMenu::GetActiveMenu() != NULL)
	{
		return TRUE;
	}

	if (m_nVertScrollTotal > 0)
	{
		int iRemainingDelta = abs(zDelta);

		while (iRemainingDelta != 0)
		{
			if (zDelta > 0)
				OnVScroll(SB_LINEUP, 0, &m_wndScrollVert);
			else
				OnVScroll(SB_LINEDOWN, 0, &m_wndScrollVert);

			iRemainingDelta -= min(WHEEL_DELTA, iRemainingDelta);

			VERIFY(RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_UPDATENOW));
		}
	}

	return CDockablePane::OnMouseWheel(nFlags, zDelta, pt);
}

BOOL CMFCTasksPane::SaveState(LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CString strProfileName = ::AFXGetRegPath(strTasksPaneProfile, lpszProfileName);

	BOOL bResult = FALSE;

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
				bResult = reg.Write(AFX_REG_ENTRY_SETTINGS, lpbData, uiDataSize);
			}

			free(lpbData);
		}
	}
	catch(CMemoryException* pEx)
	{
		pEx->Delete();
		TRACE(_T("Memory exception in CMFCToolBar::SaveState()!\n"));
	}

	bResult = CDockablePane::SaveState(lpszProfileName, nIndex, uiID);

	return bResult;
}

BOOL CMFCTasksPane::LoadState(LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CString strProfileName = ::AFXGetRegPath(strTasksPaneProfile, lpszProfileName);

	BOOL bResult = FALSE;

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

	LPBYTE lpbData = NULL;
	UINT uiDataSize;

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (!reg.Open(strSection))
	{
		return FALSE;
	}

	if (!reg.Read(AFX_REG_ENTRY_SETTINGS, &lpbData, &uiDataSize))
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
		TRACE(_T("Memory exception in CMFCTasksPane::LoadState()!\n"));
	}
	catch(CArchiveException* pEx)
	{
		pEx->Delete();
		TRACE(_T("CArchiveException exception in CMFCTasksPane::LoadState()!\n"));
	}

	if (lpbData != NULL)
	{
		delete [] lpbData;
	}

	bResult = CDockablePane::LoadState(lpszProfileName, nIndex, uiID);

	return bResult;
}

void CMFCTasksPane::OnTimer(UINT_PTR nIDEvent)
{
	switch(nIDEvent)
	{
	case nAnimTimerId:
		if (m_pAnimatedGroup != NULL && m_nRowHeight != 0)
		{
			ASSERT_VALID(m_pAnimatedGroup);

			clock_t nCurrAnimTime = clock();

			int nDuration = nCurrAnimTime - m_nLastAnimTime;
			int nSteps = (int)(.5 +(float) nDuration / m_nAnimTimerDuration);

			// speed up animation
			const int MAX_ANIMATIONSTEPS_NUM = 9;
			int nAnimatedGroupHeight = m_sizeAnim.cy + m_nAnimGroupExtraHeight;
			int nStepsTotal = (int)(.5 +(float) nAnimatedGroupHeight / m_nRowHeight);
			if (nStepsTotal > MAX_ANIMATIONSTEPS_NUM)
			{
				nSteps = 1 + nSteps * nStepsTotal / MAX_ANIMATIONSTEPS_NUM;
			}

			if (m_pAnimatedGroup->m_bIsCollapsed) // collapsing
			{
				m_sizeAnim.cy -= nSteps * m_nRowHeight;
			}
			else // expanding
			{
				m_sizeAnim.cy += nSteps * m_nRowHeight;
			}

			CRect rectUpdate = m_rectTasks;
			rectUpdate.top = m_pAnimatedGroup->m_rect.top - 1;
			int nSaveTop = rectUpdate.top;
			InvalidateRect(rectUpdate);

			RedrawWindow(NULL, NULL, RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW);

			ReposTasks();

			rectUpdate = m_rectTasks;
			rectUpdate.top = min(m_pAnimatedGroup->m_rect.top - 1, nSaveTop);
			InvalidateRect(rectUpdate);

			RedrawWindow(NULL, NULL, RDW_ERASE);

			// stop rule:
			if (m_pAnimatedGroup->m_bIsCollapsed && m_sizeAnim.cy < 0 || !m_pAnimatedGroup->m_bIsCollapsed && m_sizeAnim.cy > m_pAnimatedGroup->m_rectGroup.Height())
			{
				m_pAnimatedGroup = NULL;
				m_sizeAnim = CSize(0, 0);
			}

			m_nLastAnimTime = nCurrAnimTime;
		}
		else
		{
			KillTimer(nAnimTimerId);
			m_pAnimatedGroup = NULL;
		}
		break;

	case nScrollTimerId:
		{
			CPoint point;
			::GetCursorPos(&point);
			ScreenToClient(&point);

			if (m_rectScrollUp.PtInRect(point) && m_iScrollMode < 0) // Scroll Up
			{
				m_nVertScrollOffset--;

				AdjustScroll();
				ReposTasks();
				RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
			}
			else if (m_rectScrollDn.PtInRect(point) && m_iScrollMode > 0) // Scroll Down
			{
				m_nVertScrollOffset++;

				AdjustScroll();
				ReposTasks();
				RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
			}
			else
			{
				KillTimer(nScrollTimerId);
				m_iScrollMode = 0;
				InvalidateRect(m_rectScrollDn);
				InvalidateRect(m_rectScrollUp);
				UpdateWindow();
			}
		}
		break;
	}

	CDockablePane::OnTimer(nIDEvent);
}

void CMFCTasksPane::RecalcLayout(BOOL bRedraw/* = TRUE*/)
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	AdjustScroll();
	ReposTasks();

	if (bRedraw)
	{
		RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMFCTasksPane idle update through CMFCTasksPaneToolBarCmdUI class

class CMFCTasksPaneToolBarCmdUI : public CCmdUI        // class private to this file !
{
	// m_nIndex - taskgroup index
	// m_pOther - taskspane pointer

public: // re-implementations only
	virtual void Enable(BOOL bOn);
	virtual void SetCheck(int /*nCheck*/) {} // ignore
	virtual void SetRadio(BOOL /*bOn*/ = TRUE) {} // ignore
	virtual void SetText(LPCTSTR /*lpszText*/);
};

void CMFCTasksPaneToolBarCmdUI::Enable(BOOL bOn)
{
	m_bEnableChanged = TRUE;
	CMFCTasksPane* pTasksPane = (CMFCTasksPane*)m_pOther;
	ENSURE(pTasksPane != NULL);
	ASSERT_KINDOF(CMFCTasksPane, pTasksPane);
	ASSERT(m_nIndex < m_nIndexMax);

	// Enable all tasks with uiCommandID in the taskgroup:
	CMFCTasksPaneTaskGroup* pGroup = pTasksPane->GetTaskGroup(m_nIndex);
	if (pGroup == NULL)
	{
		return;
	}

	for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition(); posTask != NULL;)
	{
		CMFCTasksPaneTask* pTask = (CMFCTasksPaneTask*) pGroup->m_lstTasks.GetNext(posTask);
		ASSERT_VALID(pTask);

		if (pTask->m_uiCommandID == m_nID)
		{
			if (pTask->m_bEnabled != bOn)
			{
				pTask->m_bEnabled = bOn;
				pTasksPane->InvalidateRect(pTask->m_rect);

				if (pTask->m_hwndTask != NULL)
				{
					CWnd* pChildWnd = CWnd::FromHandle(pTask->m_hwndTask);
					ASSERT_VALID(pChildWnd);

					pChildWnd->EnableWindow(bOn);
				}
			}
		}
	}
}

void CMFCTasksPaneToolBarCmdUI::SetText(LPCTSTR lpszText)
{
	ENSURE(lpszText != NULL);

	CMFCTasksPane* pTasksPane = (CMFCTasksPane*)m_pOther;
	ENSURE(pTasksPane != NULL);
	ASSERT_KINDOF(CMFCTasksPane, pTasksPane);
	ASSERT(m_nIndex < m_nIndexMax);

	//Remove any amperstands and trailing label(ex.:"\tCtrl+S")
	CString strNewText(lpszText);

	int iOffset = strNewText.Find(_T('\t'));
	if (iOffset != -1)
	{
		strNewText = strNewText.Left(iOffset);
	}

	// Set name for all tasks with uiCommandID in the taskgroup:
	CMFCTasksPaneTaskGroup* pGroup = pTasksPane->GetTaskGroup(m_nIndex);
	if (pGroup == NULL)
	{
		return;
	}

	for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition(); posTask != NULL;)
	{
		CMFCTasksPaneTask* pTask = (CMFCTasksPaneTask*) pGroup->m_lstTasks.GetNext(posTask);
		ASSERT_VALID(pTask);

		if (pTask->m_uiCommandID == m_nID)
		{
			if (pTask->m_strName != strNewText)
			{
				pTask->m_strName = strNewText;
				pTasksPane->InvalidateRect(pTask->m_rect);
			}
		}
	}
}

void CMFCTasksPane::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	CMFCTasksPaneToolBarCmdUI state;
	state.m_pOther = this;

	// update all tasks:
	state.m_nIndexMax = (UINT)GetGroupCount();
	state.m_nIndex = 0;
	for (POSITION posGroup = m_lstTaskGroups.GetHeadPosition(); posGroup != NULL; state.m_nIndex++)
	{
		CMFCTasksPaneTaskGroup* pGroup = (CMFCTasksPaneTaskGroup*) m_lstTaskGroups.GetNext(posGroup);
		ASSERT_VALID(pGroup);

		for (POSITION posTask = pGroup->m_lstTasks.GetHeadPosition(); posTask != NULL;)
		{
			CMFCTasksPaneTask* pTask = (CMFCTasksPaneTask*) pGroup->m_lstTasks.GetNext(posTask);
			ASSERT_VALID(pTask);

			if (afxUserToolsManager != NULL && afxUserToolsManager->IsUserToolCmd(pTask->m_uiCommandID))
			{
				bDisableIfNoHndler = FALSE;
			}

			//state.m_nIndex == taskgroup index
			state.m_nID = pTask->m_uiCommandID;

			// ignore separators and system commands
			if (pTask->m_uiCommandID != 0 && !IsSystemCommand(pTask->m_uiCommandID) && pTask->m_uiCommandID < AFX_IDM_FIRST_MDICHILD)
			{
				// check for handlers in the target(owner)
				state.DoUpdate(pTarget, bDisableIfNoHndler);
			}
		}

	}

	CDockablePane::OnUpdateCmdUI(pTarget, bDisableIfNoHndler);
}

void CMFCTasksPane::ShowCommandMessageString(UINT uiCmdId)
{
	GetOwner()->SendMessage(WM_SETMESSAGESTRING, uiCmdId == (UINT) -1 ? AFX_IDS_IDLEMESSAGE :(WPARAM) uiCmdId);
}

BOOL CMFCTasksPane::CreateNavigationToolbar()
{
	if (GetSafeHwnd() == NULL)
	{
		return FALSE;
	}

	// ---------------
	// Create toolbar:
	// ---------------
	if (!m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE | CBRS_TOOLTIPS | CBRS_FLYBY, nNavToolbarId))
	{
		return FALSE;
	}
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~CBRS_GRIPPER);

	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this bar, not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	CSize sizeNavImage = afxGlobalData.Is32BitIcons() ? CSize(16, 16) : CSize(12, 12);
	const int nImageMargin = 4;

	CSize sizeNavButton = sizeNavImage + CSize(nImageMargin, nImageMargin);

	// -----------------------
	// Load navigation images:
	// -----------------------
	if (m_uiToolbarBmpRes == 0)
	{
		//----------------------
		// Use default resource:
		//----------------------
		m_wndToolBar.SetLockedSizes(sizeNavButton, sizeNavImage);

		BOOL bIsLoaded = m_wndToolBar.LoadBitmap(afxGlobalData.Is32BitIcons() ? IDB_AFXBARRES_TASKPANE32 : IDB_AFXBARRES_TASKPANE, 0, 0, TRUE);
		ASSERT(bIsLoaded);
	}
	else
	{
		if (m_sizeToolbarImage != CSize(0, 0))
		{
			sizeNavImage = m_sizeToolbarImage;

			if (m_sizeToolbarButton != CSize(0, 0))
			{
				sizeNavButton = m_sizeToolbarButton;
			}
			else
			{
				sizeNavButton = sizeNavImage + CSize(nImageMargin, nImageMargin);
			}
		}

		m_wndToolBar.SetLockedSizes(sizeNavButton, sizeNavImage);

		BOOL bIsLoaded = m_wndToolBar.LoadBitmap(m_uiToolbarBmpRes, 0, 0, TRUE);
		ASSERT(bIsLoaded);
	}

	//-----------------------------
	// Load Task Pane text strings:
	//-----------------------------
	CString strBack;
	ENSURE(strBack.LoadString(ID_AFXBARRES_TASKPANE_BACK));

	CString strForward;
	ENSURE(strForward.LoadString(ID_AFXBARRES_TASKPANE_FORWARD));

	CString strHome;
	ENSURE(strHome.LoadString(ID_AFXBARRES_TASKPANE_HOME));

	CString strClose;
	ENSURE(strClose.LoadString(ID_AFXBARRES_TASKPANE_CLOSE));

	// --------------------
	// Add toolbar buttons:
	// --------------------
	m_wndToolBar.RemoveAllButtons();

	if (m_bHistoryMenuButtons)
	{
		// Create drop-down menubutton for the "Back" button:
		CTasksPaneHistoryButton* pBtnBack = new CTasksPaneHistoryButton(ID_AFXBARRES_TASKPANE_BACK, 0, strBack);
		m_wndToolBar.m_pBtnBack = pBtnBack;

		if (pBtnBack != NULL)
		{
			m_wndToolBar.InsertButton(pBtnBack);
			pBtnBack->SetMessageWnd(this);
			pBtnBack->OnChangeParentWnd(this);
			pBtnBack->m_bDrawDownArrow = TRUE;
		}

		// Create drop-down menubutton for the "Forward" button:
		CTasksPaneHistoryButton* pBtnForward = new CTasksPaneHistoryButton(ID_AFXBARRES_TASKPANE_FORWARD, 1, strForward);
		m_wndToolBar.m_pBtnForward = pBtnForward;

		if (pBtnForward != NULL)
		{
			m_wndToolBar.InsertButton(pBtnForward);
			pBtnForward->SetMessageWnd(this);
			pBtnForward->OnChangeParentWnd(this);
			pBtnForward->m_bDrawDownArrow = TRUE;
		}
	}
	else
	{
		m_wndToolBar.InsertButton(new CTasksPaneNavigateButton(ID_AFXBARRES_TASKPANE_BACK, 0, strBack));
		m_wndToolBar.InsertButton(new CTasksPaneNavigateButton(ID_AFXBARRES_TASKPANE_FORWARD, 1, strForward));
	}

	m_wndToolBar.InsertButton(new CTasksPaneNavigateButton(ID_AFXBARRES_TASKPANE_HOME, 2, strHome));

	m_wndToolBar.InsertSeparator();

	CTasksPaneMenuButton* pButton = new CTasksPaneMenuButton(m_menuOther.GetSafeHmenu());

	if (pButton != NULL)
	{
		m_wndToolBar.InsertButton(pButton);
		pButton->m_bText = TRUE;
		pButton->m_bImage = FALSE;
		pButton->m_bLocked = TRUE;
		pButton->m_strText = _T("Tasks Pane");;
		pButton->SetMessageWnd(this);
		pButton->OnChangeParentWnd(this);
	}

	m_wndToolBar.InsertButton(new CTasksPaneNavigateButton(ID_AFXBARRES_TASKPANE_CLOSE, 3, strClose));

	return TRUE;
}

void CMFCTasksPane::OnBack()
{
	if (m_bHistoryMenuButtons)
	{
		// Get index of the clicked history page
		int iPage = CMFCPopupMenuBar::GetLastCommandIndex();

		// Go back
		int nPrevPagesCount = m_iActivePage;
		if (iPage >= 0 && iPage < nPrevPagesCount)
		{
			int nOldActivePage = m_iActivePage;
			m_iActivePage -= iPage + 1;
			ChangeActivePage(m_iActivePage, nOldActivePage);

			return;
		}
	}

	OnPressBackButton();
}

void CMFCTasksPane::OnForward()
{
	if (m_bHistoryMenuButtons)
	{
		// Get index of the clicked history page
		int iPage = CMFCPopupMenuBar::GetLastCommandIndex();

		// Go forward
		int nNextPagesCount = (int) m_arrHistoryStack.GetUpperBound() - m_iActivePage;
		if (iPage >= 0 && iPage < nNextPagesCount)
		{
			int nOldActivePage = m_iActivePage;
			m_iActivePage += iPage + 1;
			ChangeActivePage(m_iActivePage, nOldActivePage);

			return;
		}
	}

	OnPressForwardButton();
}

void CMFCTasksPane::OnHome()
{
	OnPressHomeButton();
}

void CMFCTasksPane::OnClose()
{
	OnPressCloseButton();
}

void CMFCTasksPane::OnOther()
{
	// ------------------------------------
	// Handle "Other Task Pane" menubutton:
	// ------------------------------------
	int iPage = CMFCPopupMenuBar::GetLastCommandIndex();

	ASSERT(iPage >= 0);
	ASSERT(iPage < GetPagesCount());

	SetActivePage(iPage);
}

void CMFCTasksPane::OnUpdateBack(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsBackButtonEnabled());
}

void CMFCTasksPane::OnUpdateForward(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsForwardButtonEnabled());
}

void CMFCTasksPane::OnUpdateClose(CCmdUI* pCmdUI)
{
	CMFCTasksPane* pParentBar = DYNAMIC_DOWNCAST(CMFCTasksPane, FromHandle(m_hWndOwner));
	if (pParentBar != NULL)
	{
		pCmdUI->Enable(pParentBar->CanBeClosed());
	}
}

HMENU CMFCTasksPane::CreateMenu() const
{
	// ------------------------------------------------
	// Create popup menu with a list of taskpane pages:
	// ------------------------------------------------
	CMenu menu;
	menu.CreatePopupMenu();

	for (POSITION pos = m_lstTasksPanes.GetHeadPosition(); pos != NULL;)
	{
		CMFCTasksPanePropertyPage* pPage = (CMFCTasksPanePropertyPage*) m_lstTasksPanes.GetNext(pos);
		ASSERT_VALID(pPage);

		menu.AppendMenu(MF_STRING, ID_AFXBARRES_TASKPANE_OTHER, pPage->m_strName);
	}

	HMENU hMenu = menu.Detach();

	// ------------------------------------
	// Check menu item for the active page:
	// ------------------------------------
	if (hMenu != NULL)
	{
		int iPage = GetActivePage();
		::CheckMenuItem(hMenu, iPage, MF_BYPOSITION | MF_CHECKED);
	}

	return hMenu;
}

void CMFCTasksPane::EnableNavigationToolbar(BOOL bEnable, UINT uiToolbarBmpRes, CSize sizeToolbarImage, CSize sizeToolbarButton)
{
	BOOL bReloadImages = m_wndToolBar.GetSafeHwnd() != NULL && (m_uiToolbarBmpRes != uiToolbarBmpRes);

	m_bUseNavigationToolbar = bEnable;
	m_uiToolbarBmpRes = uiToolbarBmpRes;
	m_sizeToolbarImage = sizeToolbarImage;
	m_sizeToolbarButton = sizeToolbarButton;

	m_wndToolBar.m_bLargeIconsAreEnbaled = FALSE;

	if (bReloadImages)
	{
		CSize sizeNavImage = afxGlobalData.Is32BitIcons() ? CSize(16, 16) : CSize(12, 12);
		const int nImageMargin = 4;

		CSize sizeNavButton = sizeNavImage + CSize(nImageMargin, nImageMargin);

		m_wndToolBar.m_ImagesLocked.Clear();

		if (m_uiToolbarBmpRes == 0)
		{
			//----------------------
			// Use default resource:
			//----------------------
			m_wndToolBar.SetLockedSizes(sizeNavButton, sizeNavImage);

			BOOL bIsLoaded = m_wndToolBar.LoadBitmap(afxGlobalData.Is32BitIcons() ? IDB_AFXBARRES_TASKPANE32 : IDB_AFXBARRES_TASKPANE, 0, 0, TRUE);
			ASSERT(bIsLoaded);
		}
		else
		{
			if (m_sizeToolbarImage != CSize(0, 0))
			{
				sizeNavImage = m_sizeToolbarImage;

				if (m_sizeToolbarButton != CSize(0, 0))
				{
					sizeNavButton = m_sizeToolbarButton;
				}
				else
				{
					sizeNavButton = sizeNavImage + CSize(nImageMargin, nImageMargin);
				}
			}

			m_wndToolBar.SetLockedSizes(sizeNavButton, sizeNavImage);

			BOOL bIsLoaded = m_wndToolBar.LoadBitmap(m_uiToolbarBmpRes, 0, 0, TRUE);
			ASSERT(bIsLoaded);
		}
	}

	UpdateCaption();
}

void CMFCTasksPane::UpdateCaption()
{
	POSITION pos = m_lstTasksPanes.FindIndex(GetActivePage());
	ENSURE(pos != NULL);

	CMFCTasksPanePropertyPage* pPage = (CMFCTasksPanePropertyPage*) m_lstTasksPanes.GetAt(pos);
	ASSERT_VALID(pPage);

	BOOL bMultiPage = (m_lstTasksPanes.GetCount() > 1);
	if (m_bUseNavigationToolbar || ForceShowNavToolbar() || bMultiPage)
	{
		SetWindowText(m_strCaption);
	}
	else
	{
		SetWindowText(pPage->m_strName);
	}

	m_wndToolBar.UpdateMenuButtonText(pPage->m_strName);

	// Update caption in non-client area:
	UINT uiSWPFlags = SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE;

	CPaneFrameWnd* pParentMiniFrame = GetParentMiniFrame(TRUE);
	if (pParentMiniFrame != NULL)
	{
		ASSERT_VALID(pParentMiniFrame);
		pParentMiniFrame->SetWindowPos(NULL, -1, -1, -1, -1, uiSWPFlags);
	}
	else
	{
		SetWindowPos(NULL, -1, -1, -1, -1, uiSWPFlags);
	}
}

void CMFCTasksPane::Update()
{
	UpdateCaption();

	AdjustScroll();
	ReposTasks();
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
}

void CMFCTasksPane::GetPreviousPages(CStringList& lstPrevPages) const
{
	ASSERT(m_iActivePage >= 0);
	ASSERT(m_iActivePage <= m_arrHistoryStack.GetUpperBound());

	// -----------------------------------------
	// Collect names list of the previous pages:
	// -----------------------------------------
	lstPrevPages.RemoveAll();
	const int nCount = m_iActivePage;
	for (int i = 0; i < nCount; i++)
	{
		int nPageIdx = m_arrHistoryStack [m_iActivePage - 1 - i];

		POSITION posPage = m_lstTasksPanes.FindIndex(nPageIdx);
		ENSURE(posPage != NULL);

		CMFCTasksPanePropertyPage* pPage = (CMFCTasksPanePropertyPage*) m_lstTasksPanes.GetAt(posPage);
		ASSERT_VALID(pPage);

		lstPrevPages.AddTail(pPage->m_strName);
	}
}

void CMFCTasksPane::GetNextPages(CStringList& lstNextPages) const
{
	ASSERT(m_iActivePage >= 0);
	ASSERT(m_iActivePage <= m_arrHistoryStack.GetUpperBound());

	// -------------------------------------
	// Collect names list of the next pages:
	// -------------------------------------
	lstNextPages.RemoveAll();
	const int nCount = (int) m_arrHistoryStack.GetUpperBound() - m_iActivePage;
	for (int i = 0; i < nCount; i++)
	{
		int nPageIdx = m_arrHistoryStack [m_iActivePage + 1 + i];

		POSITION posPage = m_lstTasksPanes.FindIndex(nPageIdx);
		ENSURE(posPage != NULL);

		CMFCTasksPanePropertyPage* pPage = (CMFCTasksPanePropertyPage*) m_lstTasksPanes.GetAt(posPage);
		ASSERT_VALID(pPage);

		lstNextPages.AddTail(pPage->m_strName);
	}
}

void CMFCTasksPane::EnableHistoryMenuButtons(BOOL bEnable)
{
	if (m_bHistoryMenuButtons == bEnable)
	{
		return;
	}

	BOOL bRecreateToolBar = FALSE;

	if (m_wndToolBar.GetSafeHwnd() != NULL)
	{
		bRecreateToolBar = TRUE;
		m_wndToolBar.DestroyWindow();
	}

	m_bHistoryMenuButtons = bEnable;

	if (bRecreateToolBar)
	{
		CreateNavigationToolbar();
		m_wndToolBar.UpdateButtons();
	}
}

CSize CMFCTasksPane::GetTasksGroupBorders() const
{
	if (IsToolBox())
	{
		return CSize(0, 0);
	}

	return CSize(1, 1);
}

LRESULT CMFCTasksPane::OnSetText(WPARAM, LPARAM lParam)
{
	LRESULT lRes = Default();

	if (lParam != NULL)
	{
		m_strCaption = (LPCTSTR)lParam;
	}

	return lRes;
}


BOOL CMFCTasksPane::OnSetAccData(long lVal)
{
	ASSERT_VALID (this);

	CPoint pt(LOWORD(lVal), HIWORD(lVal));
	ScreenToClient(&pt);

	CMFCTasksPaneTaskGroup* pGroup = GroupCaptionHitTest(pt);
	CMFCTasksPaneTask* pTask = TaskHitTest(pt);

	if (pGroup == NULL && pTask == NULL)
	{
		POSITION pos = m_lstTasksPanes.FindIndex(GetActivePage());
		ASSERT(pos != NULL);
	
		CMFCTasksPanePropertyPage* pPage = (CMFCTasksPanePropertyPage*)m_lstTasksPanes.GetAt(pos);
		if (pPage != NULL)
		{
			ASSERT_VALID(pPage);
			pPage->SetACCData(this, m_AccData);
			m_AccData.m_rectAccLocation = m_rectTasks;
			ClientToScreen(&m_AccData.m_rectAccLocation);
		}
	}

	m_AccData.Clear();

	if (pGroup != NULL)
	{
		ASSERT_VALID(pGroup);
		pGroup->SetACCData(this, m_AccData);
		if (pGroup == m_pClickedGroupCaption)
		{
			m_AccData.m_bAccState |= STATE_SYSTEM_FOCUSED;
			m_AccData.m_bAccState |= STATE_SYSTEM_SELECTABLE;
		}
	}

	if (pTask != NULL)
	{
		ASSERT_VALID(pTask);
		pTask->SetACCData(this, m_AccData);
		if (pTask == m_pClickedTask)
		{
			m_AccData.m_bAccState |= STATE_SYSTEM_FOCUSED;
		}
	}

	return TRUE;
}
