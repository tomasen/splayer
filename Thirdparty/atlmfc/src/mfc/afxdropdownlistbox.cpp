// This MFC Library source code supports the Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Classes Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// http://msdn.microsoft.com/officeui.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

#include "stdafx.h"
#include "afxdropdownlistbox.h"
#include "afxtoolbarmenubutton.h"
#include "afxdialogex.h"
#include "afxpropertypage.h"
#include "afxribboncombobox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const UINT idStart = (UINT) -200;

/////////////////////////////////////////////////////////////////////////////
// CMFCDropDownListBox

IMPLEMENT_DYNAMIC(CMFCDropDownListBox, CMFCPopupMenu)

CMFCDropDownListBox::CMFCDropDownListBox()
{
	CommonInit();
}

CMFCDropDownListBox::CMFCDropDownListBox(CWnd* pEditCtrl)
{
	ASSERT_VALID(pEditCtrl);

	CommonInit();
	m_pEditCtrl = pEditCtrl;
}

CMFCDropDownListBox::CMFCDropDownListBox(CMFCRibbonComboBox* pRibbonCombo)
{
	ASSERT_VALID(pRibbonCombo);

	CommonInit();

	m_pRibbonCombo = pRibbonCombo;
	m_pEditCtrl = pRibbonCombo->m_pWndEdit;
}

void CMFCDropDownListBox::CommonInit()
{
	m_pEditCtrl = NULL;
	m_pRibbonCombo = NULL;

	m_bShowScrollBar = TRUE;
	m_nMaxHeight = -1;

	m_Menu.CreatePopupMenu();
	m_nCurSel = -1;

	m_nMinWidth = -1;
	m_bDisableAnimation = TRUE;

	SetAutoDestroy(FALSE);
}

CMFCDropDownListBox::~CMFCDropDownListBox()
{
}

BEGIN_MESSAGE_MAP(CMFCDropDownListBox, CMFCPopupMenu)
	//{{AFX_MSG_MAP(CMFCDropDownListBox)
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CMFCDropDownListBox::Track(CPoint point, CWnd *pWndOwner)
{
	if (!Create(pWndOwner, point.x, point.y, m_Menu, FALSE, TRUE))
	{
		return;
	}

	CMFCPopupMenuBar* pMenuBar = GetMenuBar();
	ASSERT_VALID(pMenuBar);

	pMenuBar->m_iMinWidth = m_nMinWidth;
	pMenuBar->m_bDisableSideBarInXPMode = TRUE;

	HighlightItem(m_nCurSel);

	CRect rect;
	GetWindowRect(&rect);
	UpdateShadow(&rect);

	CDialogEx* pParentDlg = NULL;
	if (pWndOwner != NULL && pWndOwner->GetParent() != NULL)
	{
		pParentDlg = DYNAMIC_DOWNCAST(CDialogEx, pWndOwner->GetParent());
		if (pParentDlg != NULL)
		{
			pParentDlg->SetActiveMenu(this);
		}
	}

	CMFCPropertyPage* pParentPropPage = NULL;
	if (pWndOwner != NULL && pWndOwner->GetParent() != NULL)
	{
		pParentPropPage = DYNAMIC_DOWNCAST(CMFCPropertyPage, pWndOwner->GetParent());
		if (pParentPropPage != NULL)
		{
			pParentPropPage->SetActiveMenu(this);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMFCDropDownListBox message handlers

int CMFCDropDownListBox::GetCount() const
{
	ASSERT_VALID(this);
	return m_Menu.GetMenuItemCount();
}

int CMFCDropDownListBox::GetCurSel()
{
	ASSERT_VALID(this);

	if (GetSafeHwnd() == NULL)
	{
		return m_nCurSel;
	}

	CMFCPopupMenuBar* pMenuBar = ((CMFCDropDownListBox*) this)->GetMenuBar();
	ASSERT_VALID(pMenuBar);

	CMFCToolBarButton* pSel = pMenuBar->GetHighlightedButton();
	if (pSel == NULL)
	{
		return -1;
	}

	int nIndex = 0;

	for (int i = 0; i < pMenuBar->GetCount(); i++)
	{
		CMFCToolBarButton* pItem = pMenuBar->GetButton(i);
		ASSERT_VALID(pItem);

		if (!(pItem->m_nStyle & TBBS_SEPARATOR))
		{
			if (pSel == pItem)
			{
				m_nCurSel = nIndex;
				return nIndex;
			}

			nIndex++;
		}
	}

	return -1;
}

int CMFCDropDownListBox::SetCurSel(int nSelect)
{
	ASSERT_VALID(this);

	const int nSelOld = GetCurSel();

	if (GetSafeHwnd() == NULL)
	{
		m_nCurSel = nSelect;
		return nSelOld;
	}

	CMFCPopupMenuBar* pMenuBar = ((CMFCDropDownListBox*) this)->GetMenuBar();
	ASSERT_VALID(pMenuBar);

	int nIndex = 0;

	for (int i = 0; i < pMenuBar->GetCount(); i++)
	{
		CMFCToolBarButton* pItem = pMenuBar->GetButton(i);
		ASSERT_VALID(pItem);

		if (!(pItem->m_nStyle & TBBS_SEPARATOR))
		{
			if (nIndex == nSelect)
			{
				HighlightItem(i);
				return nSelOld;
			}

			nIndex++;
		}
	}

	return -1;
}

void CMFCDropDownListBox::GetText(int nIndex, CString& rString) const
{
	ASSERT_VALID(this);

	CMFCToolBarButton* pItem = GetItem(nIndex);
	if (pItem == NULL)
	{
		return;
	}

	ASSERT_VALID(pItem);
	rString = pItem->m_strText;
}

void CMFCDropDownListBox::AddString(LPCTSTR lpszItem)
{
	ASSERT_VALID(this);
	ENSURE(lpszItem != NULL);
	ENSURE(GetSafeHwnd() == NULL);

	const UINT uiID = idStart - GetCount();
	m_Menu.AppendMenu(MF_STRING, uiID, lpszItem);
}

void CMFCDropDownListBox::ResetContent()
{
	ASSERT_VALID(this);
	ENSURE(GetSafeHwnd() == NULL);

	m_Menu.DestroyMenu();
	m_Menu.CreatePopupMenu();
}

void CMFCDropDownListBox::HighlightItem(int nIndex)
{
	ASSERT_VALID(this);

	CMFCPopupMenuBar* pMenuBar = GetMenuBar();
	ASSERT_VALID(pMenuBar);

	if (nIndex < 0)
	{
		return;
	}

	pMenuBar->m_iHighlighted = nIndex;

	SCROLLINFO scrollInfo;
	ZeroMemory(&scrollInfo, sizeof(SCROLLINFO));

	scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_ALL;

	m_wndScrollBarVert.GetScrollInfo(&scrollInfo);

	int iOffset = nIndex;
	int nMaxOffset = scrollInfo.nMax;

	iOffset = min(max(0, iOffset), nMaxOffset);

	if (iOffset != pMenuBar->GetOffset())
	{
		pMenuBar->SetOffset(iOffset);

		m_wndScrollBarVert.SetScrollPos(iOffset);
		AdjustScroll();

	}
}

CMFCToolBarButton* CMFCDropDownListBox::GetItem(int nIndex) const
{
	ASSERT_VALID(this);

	CMFCPopupMenuBar* pMenuBar = ((CMFCDropDownListBox*) this)->GetMenuBar();
	ASSERT_VALID(pMenuBar);

	int nCurrIndex = 0;

	for (int i = 0; i < pMenuBar->GetCount(); i++)
	{
		CMFCToolBarButton* pItem = pMenuBar->GetButton(i);
		ASSERT_VALID(pItem);

		if (!(pItem->m_nStyle & TBBS_SEPARATOR))
		{
			if (nIndex == nCurrIndex)
			{
				return pItem;
			}
		}
	}

	return NULL;
}

void CMFCDropDownListBox::OnDrawItem(CDC* pDC, CMFCToolBarMenuButton* pItem, BOOL bHighlight)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT_VALID(pItem);

	CRect rectText = pItem->Rect();
	rectText.DeflateRect(2 * AFX_TEXT_MARGIN, 0);

	if (m_pRibbonCombo != NULL)
	{
		ASSERT_VALID(m_pRibbonCombo);

		int nIndex = (int) idStart - pItem->m_nID;

		if (m_pRibbonCombo->OnDrawDropListItem(pDC, nIndex, pItem, bHighlight))
		{
			return;
		}
	}

	pDC->DrawText(pItem->m_strText, &rectText, DT_SINGLELINE | DT_VCENTER);
}

CSize CMFCDropDownListBox::OnGetItemSize(CDC* pDC, CMFCToolBarMenuButton* pItem, CSize sizeDefault)
{
	ASSERT_VALID(this);

	if (m_pRibbonCombo != NULL)
	{
		ASSERT_VALID(m_pRibbonCombo);

		int nIndex = (int) idStart - pItem->m_nID;

		CSize size = m_pRibbonCombo->OnGetDropListItemSize(pDC, nIndex, pItem, sizeDefault);

		if (size != CSize(0, 0))
		{
			return size;
		}
	}

	return sizeDefault;
}

void CMFCDropDownListBox::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	ASSERT_VALID(this);

	if (m_pEditCtrl->GetSafeHwnd() != NULL)
	{
		switch (nChar)
		{
		case VK_UP:
		case VK_DOWN:
		case VK_PRIOR:
		case VK_NEXT:
		case VK_ESCAPE:
		case VK_RETURN:
			break;

		default:
			m_pEditCtrl->SendMessage(WM_KEYDOWN, nChar, MAKELPARAM(nRepCnt, nFlags));
			return;
		}
	}

	CMFCPopupMenu::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CMFCDropDownListBox::OnChooseItem(UINT uiCommandID)
{
	ASSERT_VALID(this);

	CMFCPopupMenu::OnChooseItem(uiCommandID);

	int nIndex = (int) idStart - uiCommandID;

	if (m_pRibbonCombo != NULL)
	{
		ASSERT_VALID(m_pRibbonCombo);
		m_pRibbonCombo->OnSelectItem(nIndex);
	}
}

void CMFCDropDownListBox::OnChangeHot(int nHot)
{
	ASSERT_VALID(this);

	CMFCPopupMenu::OnChangeHot(nHot);

	if (m_pRibbonCombo != NULL)
	{
		ASSERT_VALID(m_pRibbonCombo);
		m_pRibbonCombo->NotifyHighlightListItem(nHot);
	}
}


