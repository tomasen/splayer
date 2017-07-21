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
#include "afxpropertypage.h"
#include "afxpropertysheet.h"
#include "afxvisualmanager.h"
#include "afxtrackmouse.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int idTree = 101;
const int idTab = 102;
const int idList = 103;

/////////////////////////////////////////////////////////////////////////////
// CMFCOutlookBarPaneList

BOOL CMFCOutlookBarPaneList::OnSendCommand(const CMFCToolBarButton* pButton)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pParent);

	CWaitCursor wait;
	m_pParent->SetActivePage(ButtonToIndex(pButton));

	return TRUE;
}

void CMFCOutlookBarPaneList::EnsureVisible(int iButton)
{
	ASSERT_VALID(this);

	CMFCToolBarButton* pButton = GetButton(iButton);
	ASSERT_VALID(pButton);

	CRect rectButton = pButton->Rect();

	CRect rectWork;
	GetClientRect(rectWork);

	if (rectButton.Height() >= rectWork.Height())
	{
		// Work area is too small, nothing to do
		return;
	}

	if (rectButton.top >= rectWork.top && rectButton.bottom <= rectWork.bottom)
	{
		// Already visible
		return;
	}

	if (rectButton.top < rectWork.top)
	{
		while (pButton->Rect().top < rectWork.top)
		{
			int iScrollOffset = m_iScrollOffset;

			ScrollUp();

			if (iScrollOffset == m_iScrollOffset)
			{
				break;
			}
		}
	}
	else
	{
		while (pButton->Rect().bottom > rectWork.bottom)
		{
			int iScrollOffset = m_iScrollOffset;

			ScrollDown();

			if (iScrollOffset == m_iScrollOffset)
			{
				break;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMFCProperySheetListBox

CMFCProperySheetListBox::CMFCProperySheetListBox()
{
	m_nHighlightedItem = -1;
	m_bTracked = FALSE;
	m_pParent = NULL;
}

BEGIN_MESSAGE_MAP(CMFCProperySheetListBox, CListBox)
	ON_WM_DRAWITEM_REFLECT()
	ON_WM_MEASUREITEM_REFLECT()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, &CMFCProperySheetListBox::OnMouseLeave)
END_MESSAGE_MAP()

void CMFCProperySheetListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	lpMeasureItemStruct->itemHeight = afxGlobalData.GetTextHeight() + 12;
}

void CMFCProperySheetListBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	ASSERT_VALID(m_pParent);

	int nIndex = lpDIS->itemID;

	if (nIndex < 0)
	{
		return;
	}

	CRect rect = lpDIS->rcItem;

	CPropertyPage* pPage = (CPropertyPage*) GetItemData(nIndex);
	ASSERT_VALID(pPage);

	const BOOL bIsSelected = m_pParent->GetActivePage() == pPage;
	const BOOL bIsHighlihted = nIndex == m_nHighlightedItem;

	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	ASSERT_VALID(pDC);

	pDC->SetBkMode(TRANSPARENT);

	CFont* pOldFont = pDC->SelectObject(&afxGlobalData.fontRegular);
	ASSERT_VALID(pOldFont);

	COLORREF clrText = (COLORREF)-1;

	if (bIsHighlihted || bIsSelected)
	{
		clrText = CMFCVisualManager::GetInstance()->OnDrawPropertySheetListItem(pDC, m_pParent, rect, bIsHighlihted, bIsSelected);
	}

	if (clrText == (COLORREF)-1)
	{
		pDC->SetTextColor(afxGlobalData.clrWindowText);
	}
	else
	{
		pDC->SetTextColor(clrText);
	}

	CRect rectText = rect;
	rectText.DeflateRect(10, 0);

	CString strText;
	GetText(nIndex, strText);

	pDC->DrawText(strText, rectText, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
	pDC->SelectObject(pOldFont);
}

void CMFCProperySheetListBox::OnMouseMove(UINT nFlags, CPoint point)
{
	CListBox::OnMouseMove(nFlags, point);

	ASSERT(this->IsWindowEnabled());

	CRect rectItem;

	int nHighlightedItem = -1;

	for (int i = 0; i < GetCount(); i++)
	{
		GetItemRect(i, rectItem);

		if (rectItem.PtInRect(point))
		{
			nHighlightedItem = i;
			break;
		}
	}

	if (!m_bTracked)
	{
		m_bTracked = TRUE;

		TRACKMOUSEEVENT trackmouseevent;
		trackmouseevent.cbSize = sizeof(trackmouseevent);
		trackmouseevent.dwFlags = TME_LEAVE;
		trackmouseevent.hwndTrack = GetSafeHwnd();
		trackmouseevent.dwHoverTime = HOVER_DEFAULT;
		AFXTrackMouse(&trackmouseevent);
	}

	if (nHighlightedItem != m_nHighlightedItem)
	{
		if (m_nHighlightedItem >= 0)
		{
			GetItemRect(m_nHighlightedItem, rectItem);
			InvalidateRect(rectItem);
		}

		m_nHighlightedItem = nHighlightedItem;

		if (m_nHighlightedItem >= 0)
		{
			GetItemRect(m_nHighlightedItem, rectItem);
			InvalidateRect(rectItem);
		}

		UpdateWindow();
	}
}

LRESULT CMFCProperySheetListBox::OnMouseLeave(WPARAM,LPARAM)
{
	m_bTracked = FALSE;

	if (m_nHighlightedItem >= 0)
	{
		CRect rectItem;
		GetItemRect(m_nHighlightedItem, rectItem);

		m_nHighlightedItem = -1;
		RedrawWindow(rectItem);
	}

	return 0;
}
/////////////////////////////////////////////////////////////////////////////
// CMFCPropertySheetTabCtrl

CMFCPropertySheetTabCtrl::CMFCPropertySheetTabCtrl()
{
	m_bIsDlgControl = TRUE;
}

BOOL CMFCPropertySheetTabCtrl::SetActiveTab(int iTab)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pParent);

	CWaitCursor wait;

	if (m_pParent->GetActiveIndex() != iTab)
	{
		if (!m_pParent->SetActivePage(iTab))
		{
			return FALSE;
		}
	}

	CMFCTabCtrl::SetActiveTab(iTab);

	CRect rectWndArea = m_rectWndArea;
	MapWindowPoints(m_pParent, rectWndArea);

	CPropertyPage* pPage = m_pParent->GetPage(iTab);
	if (pPage != NULL)
	{
		pPage->SetWindowPos(NULL, rectWndArea.left, rectWndArea.top, rectWndArea.Width(), rectWndArea.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// CMFCPropertySheetCategoryInfo

IMPLEMENT_DYNAMIC(CMFCPropertySheetCategoryInfo, CObject)

CMFCPropertySheetCategoryInfo::CMFCPropertySheetCategoryInfo(LPCTSTR lpszName, int nIcon, int nSelectedIcon, const CMFCPropertySheetCategoryInfo* pParentCategory, CMFCPropertySheet& propSheet) :
	m_strName(lpszName), m_nIcon(nIcon), m_nSelectedIcon(nSelectedIcon), m_pParentCategory((CMFCPropertySheetCategoryInfo*) pParentCategory), m_propSheet(propSheet)
{
	m_hTreeItem = NULL;
	m_hLastSelectedItem = NULL;

	if (m_pParentCategory != NULL)
	{
		ASSERT_VALID(m_pParentCategory);
		m_pParentCategory->m_lstSubCategories.AddTail(this);
	}
}

CMFCPropertySheetCategoryInfo::~CMFCPropertySheetCategoryInfo()
{
	while (!m_lstSubCategories.IsEmpty())
	{
		delete m_lstSubCategories.RemoveHead();
	}

	if (m_propSheet.GetSafeHwnd() != NULL)
	{
		for (POSITION pos = m_lstPages.GetHeadPosition(); pos != NULL;)
		{
			CMFCPropertyPage* pPage = m_lstPages.GetNext(pos);
			ASSERT_VALID(pPage);

			m_propSheet.RemovePage(pPage);
		}

		if (m_propSheet.m_wndTree.GetSafeHwnd() != NULL && m_hTreeItem != NULL)
		{
			m_propSheet.m_wndTree.DeleteItem(m_hTreeItem);
		}
	}

	if (m_pParentCategory != NULL)
	{
		ASSERT_VALID(m_pParentCategory);

		POSITION pos = m_pParentCategory->m_lstSubCategories.Find(this);
		if (pos != NULL)
		{
			m_pParentCategory->m_lstSubCategories.RemoveAt(pos);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMFCPropertySheet

#define AFX_UM_AFTERACTIVATEPAGE (WM_USER + 1001)

IMPLEMENT_DYNAMIC(CMFCPropertySheet, CPropertySheet)

#pragma warning(disable : 4355)

CMFCPropertySheet::CMFCPropertySheet() : m_Impl(*this)
{
	CommonInit();
}

CMFCPropertySheet::CMFCPropertySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	: CPropertySheet(nIDCaption, pParentWnd, iSelectPage), m_Impl(*this)
{
	CommonInit();
}

CMFCPropertySheet::CMFCPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	: CPropertySheet(pszCaption, pParentWnd, iSelectPage), m_Impl(*this)
{
	CommonInit();
}

#pragma warning(default : 4355)

void CMFCPropertySheet::SetLook(PropSheetLook look, int nNavBarWidth)
{
	ENSURE(GetSafeHwnd() == NULL);

	m_look = look;
	m_nBarWidth = nNavBarWidth;

	if (m_look != PropSheetLook_Tabs)
	{
		EnableStackedTabs(FALSE);
	}
}

CMFCPropertySheet::~CMFCPropertySheet()
{
	while (!m_lstTreeCategories.IsEmpty())
	{
		delete m_lstTreeCategories.RemoveHead();
	}
}

void CMFCPropertySheet::CommonInit()
{
	m_nBarWidth = 100;
	m_nActivePage = -1;
	m_look = PropSheetLook_Tabs;
	m_bIsInSelectTree = FALSE;
	m_bAlphaBlendIcons = FALSE;
	m_nHeaderHeight = 0;
}

//{{AFX_MSG_MAP(CMFCPropertySheet)
BEGIN_MESSAGE_MAP(CMFCPropertySheet, CPropertySheet)
	ON_WM_SYSCOLORCHANGE()
	ON_WM_SETTINGCHANGE()
	ON_MESSAGE(AFX_UM_AFTERACTIVATEPAGE, &CMFCPropertySheet::OnAfterActivatePage)
	ON_NOTIFY(TVN_SELCHANGEDA, idTree, &CMFCPropertySheet::OnSelectTree)
	ON_NOTIFY(TVN_SELCHANGEDW, idTree, &CMFCPropertySheet::OnSelectTree)
	ON_NOTIFY(TVN_GETDISPINFOA, idTree, &CMFCPropertySheet::OnGetDispInfo)
	ON_NOTIFY(TVN_GETDISPINFOW, idTree, &CMFCPropertySheet::OnGetDispInfo)
	ON_LBN_SELCHANGE(idList, &CMFCPropertySheet::OnSelectList)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// CMFCPropertySheet message handlers

void CMFCPropertySheet::AddPage(CPropertyPage* pPage)
{
	CPropertySheet::AddPage(pPage);

	if (GetSafeHwnd() == NULL || m_look == PropSheetLook_Tabs)
	{
		return;
	}

	CTabCtrl* pTab = GetTabControl();
	ASSERT_VALID(pTab);

	InternalAddPage(pTab->GetItemCount() - 1);
}

void CMFCPropertySheet::InternalAddPage(int nTab)
{
	CTabCtrl* pTab = GetTabControl();
	ASSERT_VALID(pTab);

	TCHAR szTab [256];
	lstrcpy (szTab, _T(""));

	TCITEM item;
	item.mask = TCIF_TEXT;
	item.cchTextMax = 255;
	item.pszText = szTab;

	if (!pTab->GetItem(nTab, &item))
	{
		ASSERT(FALSE);
		return;
	}

	ENSURE(szTab != NULL);

	if (m_wndPane1.GetSafeHwnd() != NULL)
	{
		HICON hIcon = m_Icons.ExtractIcon(nTab);
		m_wndPane1.AddButton(hIcon, szTab, 0, -1, m_bAlphaBlendIcons);
		::DestroyIcon(hIcon);
	}

	if (m_wndTree.GetSafeHwnd() != NULL)
	{
		CMFCPropertyPage* pPage = DYNAMIC_DOWNCAST(CMFCPropertyPage, GetPage(nTab));
		if (pPage == NULL)
		{
			ASSERT(FALSE);
			return;
		}

		HTREEITEM hParent = NULL;
		if (pPage->m_pCategory != NULL)
		{
			ASSERT_VALID(pPage->m_pCategory);
			hParent = pPage->m_pCategory->m_hTreeItem;
		}

		HTREEITEM hTreeItem = m_wndTree.InsertItem(szTab, I_IMAGECALLBACK, I_IMAGECALLBACK, 
			hParent == NULL ? TVI_ROOT : hParent);
		m_wndTree.SetItemData(hTreeItem, (DWORD_PTR) pPage);
		pPage->m_hTreeNode = hTreeItem;
	}

	if (m_wndList.GetSafeHwnd() != NULL)
	{
		CMFCPropertyPage* pPage = DYNAMIC_DOWNCAST(CMFCPropertyPage, GetPage(nTab));
		if (pPage == NULL)
		{
			ASSERT(FALSE);
			return;
		}

		int nIndex = m_wndList.AddString(szTab);
		m_wndList.SetItemData(nIndex, (DWORD_PTR) pPage);
	}

	if (m_wndTab.GetSafeHwnd() != NULL)
	{
		CMFCPropertyPage* pPage = DYNAMIC_DOWNCAST(CMFCPropertyPage, GetPage(nTab));
		if (pPage == NULL)
		{
			ASSERT(FALSE);
			return;
		}

		UINT uiImage = m_Icons.GetSafeHandle() == NULL ?(UINT)-1 : nTab;

		m_wndTab.AddTab(pPage, szTab, uiImage, FALSE);
	}
}

void CMFCPropertySheet::RemovePage(CPropertyPage* pPage)
{
	int nPage = GetPageIndex(pPage);
	ASSERT(nPage >= 0);

	CPropertySheet::RemovePage(nPage);

	if (m_wndPane1.GetSafeHwnd() != NULL)
	{
		m_wndPane1.RemoveButton(nPage);
	}

	if (m_wndTree.GetSafeHwnd() != NULL)
	{
		if (!OnRemoveTreePage(pPage))
		{
			return;
		}
	}

	if (m_wndList.GetSafeHwnd() != NULL)
	{
		m_wndList.DeleteString(FindPageIndexInList(pPage));
	}
}

void CMFCPropertySheet::RemovePage(int nPage)
{
	if (m_wndTree.GetSafeHwnd() != NULL)
	{
		if (!OnRemoveTreePage(GetPage(nPage)))
		{
			return;
		}
	}

	if (m_wndList.GetSafeHwnd() != NULL)
	{
		m_wndList.DeleteString(FindPageIndexInList(GetPage(nPage)));
	}

	CPropertySheet::RemovePage(nPage);

	if (m_wndPane1.GetSafeHwnd() != NULL)
	{
		m_wndPane1.RemoveButton(nPage);
	}
}

void CMFCPropertySheet::RemoveCategory(CMFCPropertySheetCategoryInfo* pCategory)
{
	ASSERT_VALID(pCategory);

	POSITION pos = m_lstTreeCategories.Find(pCategory);
	if (pos != NULL)
	{
		m_lstTreeCategories.RemoveAt(pos);
	}

	delete pCategory;
}

CMFCPropertySheetCategoryInfo* CMFCPropertySheet::AddTreeCategory(LPCTSTR lpszLabel, int nIconNum, int nSelectedIconNum, const CMFCPropertySheetCategoryInfo* pParentCategory)
{
	ASSERT_VALID(this);
	ASSERT(m_look == PropSheetLook_Tree);
	ENSURE(lpszLabel != NULL);

	if (nSelectedIconNum == -1)
	{
		nSelectedIconNum = nIconNum;
	}

	CMFCPropertySheetCategoryInfo* pCategory = new CMFCPropertySheetCategoryInfo(lpszLabel, nIconNum, nSelectedIconNum, pParentCategory, *this);

	if (m_wndTree.GetSafeHwnd() != NULL)
	{
		HTREEITEM hParent = NULL;
		if (pParentCategory != NULL)
		{
			hParent = pParentCategory->m_hTreeItem;
		}

		pCategory->m_hTreeItem = m_wndTree.InsertItem(lpszLabel, I_IMAGECALLBACK, I_IMAGECALLBACK, 
			hParent == NULL ? TVI_ROOT : hParent);
		m_wndTree.SetItemData(pCategory->m_hTreeItem, (DWORD_PTR) pCategory);
	}

	if (pParentCategory == NULL)
	{
		m_lstTreeCategories.AddTail(pCategory);
	}

	return pCategory;
}

void CMFCPropertySheet::AddPageToTree(CMFCPropertySheetCategoryInfo* pCategory, CMFCPropertyPage* pPage, int nIconNum, int nSelIconNum)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pPage);
	ASSERT(m_look == PropSheetLook_Tree);

	if (pCategory != NULL)
	{
		ASSERT_VALID(pCategory);
		pCategory->m_lstPages.AddTail(pPage);
	}

	pPage->m_pCategory = pCategory;
	pPage->m_nIcon = nIconNum;
	pPage->m_nSelIconNum = nSelIconNum;

	CPropertySheet::AddPage(pPage);

	if (GetSafeHwnd() != NULL)
	{
		CTabCtrl* pTab = GetTabControl();
		ASSERT_VALID(pTab);

		InternalAddPage(pTab->GetItemCount() - 1);
	}
}

BOOL CMFCPropertySheet::OnInitDialog()
{
	BOOL bResult = CPropertySheet::OnInitDialog();

	CWnd* pWndNavigator = InitNavigationControl();

	if (m_wndTab.GetSafeHwnd() != NULL)
	{
		CTabCtrl* pTab = GetTabControl();
		ASSERT_VALID(pTab);

		CRect rectTab;
		pTab->GetWindowRect(rectTab);
		ScreenToClient(rectTab);

		rectTab.InflateRect(2, 0);
		m_wndTab.MoveWindow(rectTab);

		pTab->ModifyStyle(WS_TABSTOP, 0);
		pTab->ShowWindow(SW_HIDE);

		if (pTab->GetItemCount() > 0)
		{
			m_wndTab.SetActiveTab(GetActiveIndex());
		}

		return bResult;
	}

	if (pWndNavigator != NULL)
	{
		CTabCtrl* pTab = GetTabControl();
		ASSERT_VALID(pTab);

		pTab->ModifyStyle(WS_TABSTOP, 0);

		CRect rectTabItem;
		pTab->GetItemRect(0, rectTabItem);
		pTab->MapWindowPoints(this, &rectTabItem);

		const int nVertMargin = 5;
		const int nHorzMargin = 5;
		const int nTabsHeight = rectTabItem.Height() + nVertMargin;

		CRect rectClient;
		GetClientRect(rectClient);

		SetWindowPos(NULL, -1, -1, rectClient.Width() + m_nBarWidth, rectClient.Height() - nTabsHeight + 3 * nVertMargin + m_nHeaderHeight, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);

		GetClientRect(rectClient);
		pTab->MoveWindow(m_nBarWidth, -nTabsHeight, rectClient.right, rectClient.bottom - 2 * nVertMargin);

		CRect rectTab;
		pTab->GetWindowRect(rectTab);
		ScreenToClient(rectTab);

		CRect rectNavigator = rectClient;
		rectNavigator.right = rectNavigator.left + m_nBarWidth;
		rectNavigator.bottom = rectTab.bottom;
		rectNavigator.DeflateRect(1, 1);

		if (m_look == PropSheetLook_List)
		{
			rectNavigator.bottom--;
		}

		pWndNavigator->SetWindowPos(&wndTop, rectNavigator.left, rectNavigator.top, rectNavigator.Width(), rectNavigator.Height(), SWP_NOACTIVATE);

		SetActivePage(GetActivePage());

		int ids[] = { IDOK, IDCANCEL, ID_APPLY_NOW, IDHELP };

		int nTotalButtonsWidth = 0;

		for (int iStep = 0; iStep < 2; iStep++)
		{
			for (int i = 0; i < sizeof(ids) / sizeof(ids [0]); i++)
			{
				CWnd* pButton = GetDlgItem(ids[i]);

				if (pButton != NULL)
				{
					if (ids [i] == IDHELP &&(m_psh.dwFlags & PSH_HASHELP) == 0)
					{
						continue;
					}

					if (ids [i] == ID_APPLY_NOW &&(m_psh.dwFlags & PSH_NOAPPLYNOW))
					{
						continue;
					}

					CRect rectButton;
					pButton->GetWindowRect(rectButton);
					ScreenToClient(rectButton);

					if (iStep == 0)
					{
						// Align buttons at the bottom
						pButton->SetWindowPos(&wndTop, rectButton.left, rectClient.bottom - rectButton.Height() - nVertMargin, -1, -1, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);

						nTotalButtonsWidth = rectButton.right;
					}
					else
					{
						// Right align the buttons
						pButton->SetWindowPos(&wndTop, rectButton.left + rectClient.right - nTotalButtonsWidth - nHorzMargin, rectButton.top, -1, -1, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
					}
				}
			}
		}
	}

	return bResult;
}

CWnd* CMFCPropertySheet::InitNavigationControl()
{
	ASSERT_VALID(this);

	CTabCtrl* pTab = GetTabControl();
	ASSERT_VALID(pTab);

	if (m_look == PropSheetLook_OutlookBar)
	{
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_ALIGN_LEFT;
		DWORD dwControlBarStyle = 0;
		m_wndOutlookBar.Create(_T(""), this, CRect(0, 0, 100, 100), AFX_IDW_TOOLBAR, dwStyle, dwControlBarStyle);

		CMFCBaseTabCtrl* pWndTab = m_wndOutlookBar.GetUnderlyingWindow();

		ASSERT_VALID(pWndTab);

		pWndTab->HideSingleTab();

		m_wndPane1.Create(&m_wndOutlookBar, AFX_DEFAULT_TOOLBAR_STYLE, 1);
		m_wndPane1.m_pParent = this;
		m_wndOutlookBar.AddTab(&m_wndPane1);

		m_wndPane1.EnableTextLabels(TRUE);
		m_wndPane1.SetOwner(this);

		ASSERT(m_Icons.GetSafeHandle() != NULL);
		ASSERT(m_Icons.GetImageCount() >= pTab->GetItemCount());

		for (int nTab = 0; nTab < pTab->GetItemCount(); nTab++)
		{
			InternalAddPage(nTab);
		}

		return &m_wndOutlookBar;
	}

	if (m_look == PropSheetLook_Tree)
	{
		CRect rectDummy(0, 0, 0, 0);
		const DWORD dwTreeStyle = WS_CHILD | WS_VISIBLE;
		m_wndTree.Create(dwTreeStyle, rectDummy, this, (UINT) idTree);

		m_wndTree.ModifyStyleEx(0, WS_EX_CLIENTEDGE);

		if (m_Icons.GetSafeHandle() != NULL)
		{
			m_wndTree.SetImageList(&m_Icons, TVSIL_NORMAL);
			m_wndTree.SetImageList(&m_Icons, TVSIL_STATE);
		}

		// Add categories:
		for (POSITION pos = m_lstTreeCategories.GetHeadPosition(); pos != NULL;)
		{
			AddCategoryToTree(m_lstTreeCategories.GetNext(pos));
		}

		// Add pages:
		for (int nTab = 0; nTab < pTab->GetItemCount(); nTab++)
		{
			InternalAddPage(nTab);
		}

		return &m_wndTree;
	}

	if (m_look == PropSheetLook_List)
	{
		CRect rectDummy(0, 0, 0, 0);
		const DWORD dwListStyle = LBS_OWNERDRAWFIXED | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | LBS_NOTIFY;
		m_wndList.Create(dwListStyle, rectDummy, this, (UINT) idList);
		m_wndList.m_pParent = this;

		m_wndList.ModifyStyleEx(0, WS_EX_CLIENTEDGE);

		// Add pages:
		for (int nTab = 0; nTab < pTab->GetItemCount(); nTab++)
		{
			InternalAddPage(nTab);
		}

		return &m_wndList;
	}

	if (m_look == PropSheetLook_OneNoteTabs)
	{
		const int nActiveTab = GetActiveIndex();

		CRect rectDummy(0, 0, 0, 0);

		m_wndTab.Create(CMFCTabCtrl::STYLE_3D_ONENOTE, rectDummy, this,
			(UINT) idTab, CMFCTabCtrl::LOCATION_TOP, FALSE);

		m_wndTab.m_pParent = this;
		m_wndTab.EnableTabSwap(FALSE);
		m_wndTab.AutoDestroyWindow(FALSE);

		if (m_Icons.GetSafeHandle() != NULL)
		{
			ASSERT(m_Icons.GetImageCount() >= pTab->GetItemCount());
			m_wndTab.SetImageList(m_Icons.GetSafeHandle());
		}

		for (int nTab = 0; nTab < pTab->GetItemCount(); nTab++)
		{
			InternalAddPage(nTab);
		}

		SetActivePage(nActiveTab);
		return &m_wndTab;
	}

	if (m_look == PropSheetLook_Tabs)
	{
		if (m_Icons.GetSafeHandle() != NULL)
		{
			ASSERT(m_Icons.GetImageCount() >= pTab->GetItemCount());
			pTab->SetImageList(&m_Icons);

			TCITEM tci;
			::ZeroMemory(&tci, sizeof(tci));
			tci.mask = TCIF_IMAGE;

			for (int nTab = 0; nTab < pTab->GetItemCount(); nTab++)
			{
				tci.iImage = nTab;
				pTab->SetItem(nTab, &tci);
			}
		}
	}

	return NULL;
}

void CMFCPropertySheet::SetIconsList(HIMAGELIST hIcons)
{
	ASSERT_VALID(this);
	ENSURE(hIcons != NULL);
	ENSURE(m_Icons.GetSafeHandle() == NULL);

	m_Icons.Create(CImageList::FromHandle(hIcons));
}

void CMFCPropertySheet::AddCategoryToTree(CMFCPropertySheetCategoryInfo* pCategory)
{
	ASSERT_VALID(this);
	ENSURE(pCategory != NULL);
	ASSERT_VALID(pCategory);
	ASSERT(m_look == PropSheetLook_Tree);

	HTREEITEM hParent = NULL;
	if (pCategory->m_pParentCategory != NULL)
	{
		hParent = pCategory->m_pParentCategory->m_hTreeItem;
	}

	pCategory->m_hTreeItem = m_wndTree.InsertItem(pCategory->m_strName, I_IMAGECALLBACK, I_IMAGECALLBACK, 
		hParent == NULL ? TVI_ROOT : hParent);
	m_wndTree.SetItemData(pCategory->m_hTreeItem, (DWORD_PTR) pCategory);

	for (POSITION pos = pCategory->m_lstSubCategories.GetHeadPosition(); pos != NULL;)
	{
		AddCategoryToTree(pCategory->m_lstSubCategories.GetNext(pos));
	}
}

BOOL CMFCPropertySheet::SetIconsList(UINT uiImageListResID, int cx, COLORREF clrTransparent)
{
	ASSERT_VALID(this);

	LPCTSTR lpszResourceName = MAKEINTRESOURCE(uiImageListResID);
	ENSURE(lpszResourceName != NULL);

	HBITMAP hbmp = NULL;

	// Try to load PNG image first:
	CPngImage pngImage;
	if (pngImage.Load(lpszResourceName))
	{
		hbmp = (HBITMAP) pngImage.Detach();
	}
	else
	{
		hbmp = (HBITMAP) ::LoadImage(AfxFindResourceHandle(lpszResourceName, RT_BITMAP), lpszResourceName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	}

	if (hbmp == NULL)
	{
		TRACE(_T("Can't load image: %x\n"), uiImageListResID);
		return FALSE;
	}

	CImageList icons;
	m_bAlphaBlendIcons = FALSE;

	BITMAP bmpObj;
	::GetObject(hbmp, sizeof(BITMAP), &bmpObj);

	UINT nFlags = (clrTransparent == (COLORREF) -1) ? 0 : ILC_MASK;

	switch (bmpObj.bmBitsPixel)
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
		m_bAlphaBlendIcons = TRUE;
		break;
	}

	icons.Create(cx, bmpObj.bmHeight, nFlags, 0, 0);
	icons.Add(CBitmap::FromHandle(hbmp), clrTransparent);

	SetIconsList(icons);

	::DeleteObject(hbmp);
	return TRUE;
}

void CMFCPropertySheet::OnActivatePage(CPropertyPage* pPage)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pPage);

	if (m_wndPane1.GetSafeHwnd() != NULL)
	{
		int nPage = GetPageIndex(pPage);
		ASSERT(nPage >= 0);

		if (m_nActivePage >= 0)
		{
			m_wndPane1.SetButtonStyle(m_nActivePage, 0);
		}

		m_nActivePage = nPage;

		PostMessage(AFX_UM_AFTERACTIVATEPAGE);
	}

	if (m_wndTree.GetSafeHwnd() != NULL)
	{
		CMFCPropertyPage* pPropPageExtra = DYNAMIC_DOWNCAST(CMFCPropertyPage, pPage);
		if (pPropPageExtra != NULL)
		{
			if (!m_bIsInSelectTree)
			{
				m_wndTree.SelectItem(pPropPageExtra->m_hTreeNode);
			}

			m_wndTree.EnsureVisible(pPropPageExtra->m_hTreeNode);
		}
	}

	if (m_wndList.GetSafeHwnd() != NULL)
	{
		int nIdex = FindPageIndexInList(pPage);

		m_wndList.SetCurSel(nIdex);
		PostMessage(AFX_UM_AFTERACTIVATEPAGE);
	}

	if (m_wndTab.GetSafeHwnd() != NULL)
	{
		const int nTab = GetPageIndex(pPage);

		m_wndTab.SetActiveTab(nTab);
		m_wndTab.EnsureVisible(nTab);
	}
}

LRESULT CMFCPropertySheet::OnAfterActivatePage(WPARAM,LPARAM)
{
	ASSERT_VALID(this);

	if (m_nActivePage >= 0)
	{
		if (m_wndPane1.GetSafeHwnd() != NULL)
		{
			m_wndPane1.SetButtonStyle(m_nActivePage, TBBS_CHECKED);
			m_wndPane1.EnsureVisible(m_nActivePage);
		}
	}

	if (m_wndList.GetSafeHwnd() != NULL)
	{
		m_wndList.RedrawWindow();
	}

	return 0;
}

void CMFCPropertySheet::OnSelectTree(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = 0;

	HTREEITEM hTreeItem = m_wndTree.GetSelectedItem();
	if (hTreeItem == NULL)
	{
		return;
	}

	CMFCPropertySheetCategoryInfo* pNewCategory = NULL;
	CMFCPropertySheetCategoryInfo* pOldCategory = NULL;

	CMFCPropertyPage* pCurrPage = DYNAMIC_DOWNCAST(CMFCPropertyPage, GetActivePage());
	if (pCurrPage != NULL)
	{
		ASSERT_VALID(pCurrPage);
		pOldCategory = pCurrPage->m_pCategory;
	}

	m_bIsInSelectTree = TRUE;

	CMFCPropertyPage* pPage = DYNAMIC_DOWNCAST(CMFCPropertyPage,
		(CObject*) m_wndTree.GetItemData(hTreeItem));

	if (pPage == pCurrPage)
	{
		m_bIsInSelectTree = FALSE;
		return;
	}

	if (pPage != NULL)
	{
		CMFCPropertyPage* pPrevPage = DYNAMIC_DOWNCAST(CMFCPropertyPage, GetActivePage());

		ASSERT_VALID(pPage);
		if (!SetActivePage(pPage))
		{
			if (pCurrPage != NULL)
			{
				m_wndTree.SendMessage(TVM_SELECTITEM, (WPARAM)TVGN_CARET, (LPARAM)pCurrPage->m_hTreeNode);
			}

			m_bIsInSelectTree = FALSE;
			return;
		}

		pNewCategory = pPage->m_pCategory;
		if (pNewCategory != NULL)
		{
			HTREEITEM hLastSelectedItem = hTreeItem;

			for (CMFCPropertySheetCategoryInfo* pCategory = pNewCategory; pCategory != NULL; pCategory = pCategory->m_pParentCategory)
			{
				pCategory->m_hLastSelectedItem = hLastSelectedItem;
				hLastSelectedItem = pCategory->m_hTreeItem;
			}
		}

		if (pPrevPage != NULL)
		{
			ASSERT_VALID(pPrevPage);

			CRect rectItem;
			m_wndTree.GetItemRect(pPrevPage->m_hTreeNode, rectItem, FALSE);
			m_wndTree.InvalidateRect(rectItem);
		}
	}
	else
	{
		CMFCPropertySheetCategoryInfo* pCategory = DYNAMIC_DOWNCAST(CMFCPropertySheetCategoryInfo,
			(CObject*) m_wndTree.GetItemData(hTreeItem));
		if (pCategory != NULL)
		{
			ASSERT_VALID(pCategory);

			BOOL bIsPageSelected = FALSE;

			while (pCategory->m_hLastSelectedItem != NULL && !bIsPageSelected)
			{
				CMFCPropertySheetCategoryInfo* pChildCategory = DYNAMIC_DOWNCAST(CMFCPropertySheetCategoryInfo, (CObject*) m_wndTree.GetItemData(pCategory->m_hLastSelectedItem));
				if (pChildCategory == NULL)
				{
					CMFCPropertyPage* pSelPage = DYNAMIC_DOWNCAST(CMFCPropertyPage, (CObject*) m_wndTree.GetItemData(pCategory->m_hLastSelectedItem));
					if (pSelPage != NULL)
					{
						SetActivePage(pSelPage);

						CRect rectItem;
						m_wndTree.GetItemRect(pSelPage->m_hTreeNode, rectItem, FALSE);
						m_wndTree.InvalidateRect(rectItem);

						bIsPageSelected = TRUE;
					}
				}
				else
				{
					pCategory = pChildCategory;
				}
			}

			if (!bIsPageSelected)
			{
				while (!pCategory->m_lstSubCategories.IsEmpty())
				{
					pCategory = pCategory->m_lstSubCategories.GetHead();
					ASSERT_VALID(pCategory);
				}

				if (!pCategory->m_lstPages.IsEmpty())
				{
					pPage = pCategory->m_lstPages.GetHead();
					ASSERT_VALID(pPage);

					SetActivePage(pPage);

					CRect rectItem;
					m_wndTree.GetItemRect(pPage->m_hTreeNode, rectItem, FALSE);
					m_wndTree.InvalidateRect(rectItem);
				}
			}

			pNewCategory = pCategory;
		}
	}

	if (pNewCategory != pOldCategory)
	{
		if (pOldCategory != NULL)
		{
			ASSERT_VALID(pOldCategory);
			HTREEITEM hItem = pOldCategory->m_hTreeItem;

			do
			{
				m_wndTree.Expand(hItem, TVE_COLLAPSE);
				hItem = m_wndTree.GetParentItem(hItem);
			}
			while (hItem != NULL);
		}

		if (pNewCategory != NULL)
		{
			ASSERT_VALID(pNewCategory);
			HTREEITEM hItem = pNewCategory->m_hTreeItem;

			do
			{
				m_wndTree.Expand(hItem, TVE_EXPAND);
				hItem = m_wndTree.GetParentItem(hItem);
			}
			while (hItem != NULL);
		}
	}

	m_bIsInSelectTree = FALSE;
}

void CMFCPropertySheet::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	ENSURE(pNMHDR != NULL);

	LPNMTVDISPINFO lptvdi = (LPNMTVDISPINFO) pNMHDR;

	CMFCPropertyPage* pPage = DYNAMIC_DOWNCAST(CMFCPropertyPage, (CObject*) m_wndTree.GetItemData(lptvdi->item.hItem));
	if (pPage != NULL)
	{
		ASSERT_VALID(pPage);

		if (pPage == GetActivePage())
		{
			lptvdi->item.iImage = pPage->m_nSelIconNum;
			lptvdi->item.iSelectedImage = pPage->m_nSelIconNum;
		}
		else
		{
			lptvdi->item.iImage = pPage->m_nIcon;
			lptvdi->item.iSelectedImage = pPage->m_nIcon;
		}
	}

	CMFCPropertySheetCategoryInfo* pCategory = DYNAMIC_DOWNCAST(CMFCPropertySheetCategoryInfo, (CObject*) m_wndTree.GetItemData(lptvdi->item.hItem));
	if (pCategory != NULL)
	{
		ASSERT_VALID(pCategory);

		if (lptvdi->item.state & TVIS_EXPANDED)
		{
			lptvdi->item.iImage = pCategory->m_nSelectedIcon;
			lptvdi->item.iSelectedImage = pCategory->m_nSelectedIcon;
		}
		else
		{
			lptvdi->item.iImage = pCategory->m_nIcon;
			lptvdi->item.iSelectedImage = pCategory->m_nIcon;
		}
	}

	*pResult = 0;
}

CMFCTabCtrl& CMFCPropertySheet::GetTab() const
{
	ASSERT_VALID(this);
	ASSERT(m_look == PropSheetLook_OneNoteTabs);

	return(CMFCTabCtrl&) m_wndTab;
}

BOOL CMFCPropertySheet::PreTranslateMessage(MSG* pMsg)
{
	if (m_Impl.PreTranslateMessage(pMsg))
	{
		return TRUE;
	}

	return CPropertySheet::PreTranslateMessage(pMsg);
}

BOOL CMFCPropertySheet::OnRemoveTreePage(CPropertyPage* pPage)
{
	ASSERT(m_look == PropSheetLook_Tree);

	if (pPage == NULL)
	{
		return FALSE;
	}

	CMFCPropertyPage* pDelPage = DYNAMIC_DOWNCAST(CMFCPropertyPage, pPage);
	if (pDelPage == NULL)
	{
		ASSERT(!_T("DYNAMIC_DOWNCAST(CMFCPropertyPage, pPage)"));
		return FALSE;
	}

	ENSURE(pDelPage->m_hTreeNode != NULL);

	BOOL bResult = m_wndTree.DeleteItem(pDelPage->m_hTreeNode);
	ENSURE(pDelPage->m_pCategory != NULL);

	POSITION pos = (pDelPage->m_pCategory->m_lstPages).Find(pDelPage);
	if (pos != NULL)
	{
		(pDelPage->m_pCategory->m_lstPages).RemoveAt(pos);
		bResult = TRUE;
	}

	return bResult;
}

void CMFCPropertySheet::OnSysColorChange()
{
	CPropertySheet::OnSysColorChange();

	if (AfxGetMainWnd() == this)
	{
		afxGlobalData.UpdateSysColors();
	}
}

void CMFCPropertySheet::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CPropertySheet::OnSettingChange(uFlags, lpszSection);

	if (AfxGetMainWnd() == this)
	{
		afxGlobalData.OnSettingChange();
	}
}

int CMFCPropertySheet::FindPageIndexInList(CPropertyPage* pPage)
{
	for (int i = 0; i < m_wndList.GetCount(); i++)
	{
		if ((CPropertyPage*) m_wndList.GetItemData(i) == pPage)
		{
			return i;
		}
	}

	return -1;
}

void CMFCPropertySheet::OnSelectList()
{
	int nCurSel = m_wndList.GetCurSel();

	if (nCurSel < 0)
	{
		return;
	}

	CPropertyPage* pPage = (CPropertyPage*) m_wndList.GetItemData(nCurSel);
	ASSERT_VALID(pPage);

	SetActivePage(pPage);
	m_wndList.RedrawWindow();
}

void CMFCPropertySheet::EnablePageHeader(int nHeaderHeight)
{
	ENSURE(GetSafeHwnd() == NULL);

	m_nHeaderHeight = nHeaderHeight;
}

void CMFCPropertySheet::OnDrawPageHeader(CDC* /*pDC*/, int /*nPage*/, CRect /*rectHeader*/)
{
}



