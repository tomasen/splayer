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
#include "afxtoolbar.h"
#include "afxglobals.h"
#include "afxtoolbarcomboboxbutton.h"
#include "afxtoolbarmenubutton.h"
#include "afxmenuimages.h"
#include "afxtrackmouse.h"
#include "afxvisualmanager.h"
#include "afxcontextmenumanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CMFCToolBarComboBoxButton, CMFCToolBarButton, 1)

static const int nDefaultComboHeight = 150;
static const int nDefaultSize = 150;
static const int nHorzMargin = 1;

BOOL CMFCToolBarComboBoxButton::m_bFlat = TRUE;
BOOL CMFCToolBarComboBoxButton::m_bCenterVert = TRUE;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFCToolBarComboBoxButton::CMFCToolBarComboBoxButton()
{
	m_dwStyle = WS_CHILD | WS_VISIBLE | CBS_NOINTEGRALHEIGHT | CBS_DROPDOWNLIST | WS_VSCROLL;
	m_iWidth = nDefaultSize;

	Initialize();
}

CMFCToolBarComboBoxButton::CMFCToolBarComboBoxButton(UINT uiId, int iImage, DWORD dwStyle, int iWidth) :
	CMFCToolBarButton(uiId, iImage)
{
	m_dwStyle = dwStyle | WS_CHILD | WS_VISIBLE | WS_VSCROLL;
	m_iWidth = (iWidth == 0) ? nDefaultSize : iWidth;

	Initialize();
}

void CMFCToolBarComboBoxButton::Initialize()
{
	m_iSelIndex = -1;
	m_pWndCombo = NULL;
	m_pWndEdit = NULL;
	m_bHorz = TRUE;
	m_rectCombo.SetRectEmpty();
	m_rectButton.SetRectEmpty();
	m_nDropDownHeight = nDefaultComboHeight;
	m_bIsHotEdit = FALSE;
	m_uiMenuResID = 0;
	m_bIsRibbon = FALSE;
}

CMFCToolBarComboBoxButton::~CMFCToolBarComboBoxButton()
{
	if (m_pWndCombo != NULL)
	{
		m_pWndCombo->DestroyWindow();
		delete m_pWndCombo;
	}

	if (m_pWndEdit != NULL)
	{
		m_pWndEdit->DestroyWindow();
		delete m_pWndEdit;
	}
}

void CMFCToolBarComboBoxButton::CopyFrom(const CMFCToolBarButton& s)
{
	CMFCToolBarButton::CopyFrom(s);
	POSITION pos;

	m_lstItems.RemoveAll();

	const CMFCToolBarComboBoxButton& src = (const CMFCToolBarComboBoxButton&) s;
	for (pos = src.m_lstItems.GetHeadPosition(); pos != NULL;)
	{
		m_lstItems.AddTail(src.m_lstItems.GetNext(pos));
	}

	ClearData();

	m_lstItemData.RemoveAll();
	for (pos = src.m_lstItemData.GetHeadPosition(); pos != NULL;)
	{
		m_lstItemData.AddTail(src.m_lstItemData.GetNext(pos));
	}

	DuplicateData();
	ASSERT(m_lstItemData.GetCount() == m_lstItems.GetCount());

	m_dwStyle = src.m_dwStyle;
	m_iWidth = src.m_iWidth;
	m_iSelIndex = src.m_iSelIndex;
	m_nDropDownHeight = src.m_nDropDownHeight;
	m_uiMenuResID = src.m_uiMenuResID;

	m_bIsRibbon = src.m_bIsRibbon;
}

void CMFCToolBarComboBoxButton::Serialize(CArchive& ar)
{
	CMFCToolBarButton::Serialize(ar);

	if (ar.IsLoading())
	{
		ar >> m_iWidth;
		m_rect.right = m_rect.left + m_iWidth;
		ar >> m_dwStyle;
		ar >> m_iSelIndex;
		ar >> m_strEdit;
		ar >> m_nDropDownHeight;
		ar >> m_uiMenuResID;

		m_lstItems.Serialize(ar);

		ClearData();
		m_lstItemData.RemoveAll();

		for (int i = 0; i < m_lstItems.GetCount(); i ++)
		{
			long lData;
			ar >> lData;
			m_lstItemData.AddTail((DWORD_PTR) lData);
		}

		DuplicateData();
		ASSERT(m_lstItemData.GetCount() == m_lstItems.GetCount());

		SelectItem(m_iSelIndex);
	}
	else
	{
		ar << m_iWidth;
		ar << m_dwStyle;
		ar << m_iSelIndex;
		ar << m_strEdit;
		ar << m_nDropDownHeight;
		ar << m_uiMenuResID;

		if (m_pWndCombo != NULL)
		{
			m_lstItems.RemoveAll();
			ClearData();
			m_lstItemData.RemoveAll();

			for (int i = 0; i < m_pWndCombo->GetCount(); i ++)
			{
				CString str;
				m_pWndCombo->GetLBText(i, str);

				m_lstItems.AddTail(str);
				m_lstItemData.AddTail(m_pWndCombo->GetItemData(i));
			}
		}

		m_lstItems.Serialize(ar);

		for (POSITION pos = m_lstItemData.GetHeadPosition(); pos != NULL;)
		{
			DWORD_PTR dwData = m_lstItemData.GetNext(pos);
			ar <<(long) dwData;
		}

		ASSERT(m_lstItemData.GetCount() == m_lstItems.GetCount());
	}
}

SIZE CMFCToolBarComboBoxButton::OnCalculateSize(CDC* pDC, const CSize& sizeDefault, BOOL bHorz)
{
	m_bHorz = bHorz;
	m_sizeText = CSize(0, 0);

	if (!IsVisible())
	{

		if (m_bFlat)
		{
			if (m_pWndEdit->GetSafeHwnd() != NULL &&
				(m_pWndEdit->GetStyle() & WS_VISIBLE))
			{
				m_pWndEdit->ShowWindow(SW_HIDE);
			}

		}

		if (m_pWndCombo->GetSafeHwnd() != NULL &&
			(m_pWndCombo->GetStyle() & WS_VISIBLE))
		{
			m_pWndCombo->ShowWindow(SW_HIDE);
		}

		return CSize(0,0);
	}

	if (m_bFlat && m_pWndCombo->GetSafeHwnd() != NULL &&
		(m_pWndCombo->GetStyle() & WS_VISIBLE))
	{
		m_pWndCombo->ShowWindow(SW_HIDE);
	}

	if (bHorz)
	{
		if (!m_bFlat && m_pWndCombo->GetSafeHwnd() != NULL && !m_bIsHidden)
		{
			m_pWndCombo->ShowWindow(SW_SHOWNOACTIVATE);
		}

		if (m_bTextBelow && !m_strText.IsEmpty())
		{
			CRect rectText(0, 0, m_iWidth, sizeDefault.cy);
			pDC->DrawText(m_strText, rectText, DT_CENTER | DT_CALCRECT | DT_WORDBREAK);
			m_sizeText = rectText.Size();
		}

		int cy = sizeDefault.cy;

		if (m_pWndCombo != NULL && m_pWndCombo->GetSafeHwnd() != NULL)
		{
			CRect rectCombo;
			m_pWndCombo->GetWindowRect(&rectCombo);

			cy = rectCombo.Height();
		}

		if (!m_bIsHidden && m_pWndEdit->GetSafeHwnd() != NULL && (m_pWndCombo->GetStyle() & WS_VISIBLE) == 0)
		{
			m_pWndEdit->ShowWindow(SW_SHOWNOACTIVATE);
		}

		return CSize(m_iWidth, cy + m_sizeText.cy);

	}
	else
	{
		if (m_pWndCombo->GetSafeHwnd() != NULL && (m_pWndCombo->GetStyle() & WS_VISIBLE))
		{
			m_pWndCombo->ShowWindow(SW_HIDE);
		}

		if (m_pWndEdit->GetSafeHwnd() != NULL && (m_pWndEdit->GetStyle() & WS_VISIBLE))
		{
			m_pWndEdit->ShowWindow(SW_HIDE);
		}

		return CMFCToolBarButton::OnCalculateSize(pDC, sizeDefault, bHorz);
	}
}

void CMFCToolBarComboBoxButton::OnMove()
{
	if (m_pWndCombo->GetSafeHwnd() != NULL)
	{
		AdjustRect();
	}
}

void CMFCToolBarComboBoxButton::OnSize(int iSize)
{
	m_iWidth = iSize;
	m_rect.right = m_rect.left + m_iWidth;

	if (m_pWndCombo->GetSafeHwnd() != NULL)
	{
		AdjustRect();
	}
}

void CMFCToolBarComboBoxButton::OnChangeParentWnd(CWnd* pWndParent)
{
	CMFCToolBarButton::OnChangeParentWnd(pWndParent);

	if (m_pWndCombo->GetSafeHwnd() != NULL)
	{
		CWnd* pWndParentCurr = m_pWndCombo->GetParent();
		ENSURE(pWndParentCurr != NULL);

		if (pWndParent != NULL && pWndParentCurr->GetSafeHwnd() == pWndParent->GetSafeHwnd())
		{
			return;
		}

		m_pWndCombo->DestroyWindow();
		delete m_pWndCombo;
		m_pWndCombo = NULL;

		if (m_pWndEdit != NULL)
		{
			m_pWndEdit->DestroyWindow();
			delete m_pWndEdit;
			m_pWndEdit = NULL;
		}
	}

	if (pWndParent == NULL || pWndParent->GetSafeHwnd() == NULL)
	{
		return;
	}

	BOOL bDisabled = CMFCToolBar::IsCustomizeMode() ||(m_nStyle & TBBS_DISABLED);

	CRect rect = m_rect;
	rect.InflateRect(-2, 0);
	rect.bottom = rect.top + m_nDropDownHeight;

	if ((m_pWndCombo = CreateCombo(pWndParent, rect)) == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	if (m_pWndCombo != NULL && m_pWndCombo->GetSafeHwnd() != NULL)
	{
		m_pWndCombo->EnableWindow(!bDisabled);
		m_pWndCombo->RedrawWindow();
	}

	if (m_bFlat &&(m_pWndCombo->GetStyle() & CBS_DROPDOWNLIST) == CBS_DROPDOWN)
	{
		DWORD dwEditStyle = WS_CHILD | WS_VISIBLE | ES_WANTRETURN | ES_AUTOHSCROLL;
		if (m_pWndCombo->GetStyle() & WS_TABSTOP)
		{
			dwEditStyle |= WS_TABSTOP;
		}

		if ((m_pWndEdit = CreateEdit(pWndParent, rect, dwEditStyle)) == NULL)
		{
			ASSERT(FALSE);
			return;
		}

		m_pWndEdit->SetFont(&afxGlobalData.fontRegular);
		m_pWndEdit->SetOwner(m_pWndCombo->GetParent()->GetOwner());

		if (m_pWndEdit != NULL && m_pWndEdit->GetSafeHwnd() != NULL)
		{
			m_pWndEdit->EnableWindow(!bDisabled);
			m_pWndEdit->RedrawWindow();
		}
	}

	AdjustRect();

	m_pWndCombo->SetFont(&afxGlobalData.fontRegular);

	if (m_pWndCombo->GetCount() > 0)
	{
		m_lstItems.RemoveAll();

		ClearData();
		m_lstItemData.RemoveAll();

		for (int i = 0; i < m_pWndCombo->GetCount(); i ++)
		{
			CString str;
			m_pWndCombo->GetLBText(i, str);

			m_lstItems.AddTail(str);
			m_lstItemData.AddTail(m_pWndCombo->GetItemData(i));
		}

		m_iSelIndex = m_pWndCombo->GetCurSel();
	}
	else
	{
		m_pWndCombo->ResetContent();
		ASSERT(m_lstItemData.GetCount() == m_lstItems.GetCount());

		POSITION posData = m_lstItemData.GetHeadPosition();
		for (POSITION pos = m_lstItems.GetHeadPosition(); pos != NULL;)
		{
			ENSURE(posData != NULL);

			CString strItem = m_lstItems.GetNext(pos);
			int iIndex = m_pWndCombo->AddString(strItem);

			m_pWndCombo->SetItemData(iIndex, m_lstItemData.GetNext(posData));
		}

		if (m_iSelIndex != CB_ERR)
		{
			m_pWndCombo->SetCurSel(m_iSelIndex);
		}
	}

	if (m_iSelIndex != CB_ERR && m_iSelIndex < m_pWndCombo->GetCount())
	{
		m_pWndCombo->GetLBText(m_iSelIndex, m_strEdit);
		m_pWndCombo->SetWindowText(m_strEdit);

		if (m_pWndEdit != NULL)
		{
			m_pWndEdit->SetWindowText(m_strEdit);
		}
	}
}

INT_PTR CMFCToolBarComboBoxButton::AddItem(LPCTSTR lpszItem, DWORD_PTR dwData)
{
	ENSURE(lpszItem != NULL);

	if (m_strEdit.IsEmpty())
	{
		m_strEdit = lpszItem;
		if (m_pWndEdit != NULL)
		{
			m_pWndEdit->SetWindowText(m_strEdit);
		}
	}

	if (FindItem(lpszItem) < 0)
	{
		m_lstItems.AddTail(lpszItem);
		m_lstItemData.AddTail(dwData);
	}

	if (m_pWndCombo->GetSafeHwnd() != NULL)
	{
		int iIndex = m_pWndCombo->FindStringExact(-1, lpszItem);

		if (iIndex == CB_ERR)
		{
			iIndex = m_pWndCombo->AddString(lpszItem);
		}

		m_pWndCombo->SetCurSel(iIndex);
		m_pWndCombo->SetItemData(iIndex, dwData);
		m_pWndCombo->SetEditSel(-1, 0);
	}

	return m_lstItems.GetCount() - 1;
}

LPCTSTR CMFCToolBarComboBoxButton::GetItem(int iIndex) const
{
	if (iIndex == -1) // Current selection
	{
		if (m_pWndCombo->GetSafeHwnd() == NULL)
		{
			if ((iIndex = m_iSelIndex) == -1)
			{
				return 0;
			}
		}
		else
		{
			iIndex = m_pWndCombo->GetCurSel();
		}
	}

	POSITION pos = m_lstItems.FindIndex(iIndex);
	if (pos == NULL)
	{
		return NULL;
	}

	return m_lstItems.GetAt(pos);
}

DWORD_PTR CMFCToolBarComboBoxButton::GetItemData(int iIndex) const
{
	if (iIndex == -1) // Current selection
	{
		if (m_pWndCombo->GetSafeHwnd() == NULL)
		{
			if ((iIndex = m_iSelIndex) == -1)
			{
				return 0;
			}
		}
		else
		{
			iIndex = m_pWndCombo->GetCurSel();
		}
	}

	if (m_pWndCombo->GetSafeHwnd() != NULL)
	{
		return m_pWndCombo->GetItemData(iIndex);
	}
	else
	{
		POSITION pos = m_lstItemData.FindIndex(iIndex);
		if (pos == NULL)
		{
			return 0;
		}

		return m_lstItemData.GetAt(pos);
	}
}

void CMFCToolBarComboBoxButton::RemoveAllItems()
{
	m_lstItems.RemoveAll();

	ClearData();
	m_lstItemData.RemoveAll();

	if (m_pWndCombo->GetSafeHwnd() != NULL)
	{
		m_pWndCombo->ResetContent();
	}

	m_strEdit.Empty();

	if (m_pWndEdit->GetSafeHwnd() != NULL)
	{
		m_pWndEdit->SetWindowText(m_strEdit);
	}
}

INT_PTR CMFCToolBarComboBoxButton::GetCount() const
{
	return m_lstItems.GetCount();
}

void CMFCToolBarComboBoxButton::AdjustRect()
{
	if (m_pWndCombo->GetSafeHwnd() == NULL || m_rect.IsRectEmpty() || !m_bHorz)
	{
		m_rectCombo.SetRectEmpty();
		m_rectButton.SetRectEmpty();
		return;
	}

	if (m_bCenterVert &&(!m_bTextBelow || m_strText.IsEmpty()))
	{
		CMFCToolBar* pParentBar = NULL;
		CWnd* pNextBar = m_pWndCombo->GetParent();

		while (pParentBar == NULL && pNextBar != NULL)
		{
			pParentBar = DYNAMIC_DOWNCAST(CMFCToolBar, pNextBar);
			pNextBar = pNextBar->GetParent();
		}

		if (pParentBar != NULL)
		{
			const int nRowHeight = pParentBar->GetRowHeight();
			const int yOffset = max(0, (nRowHeight - m_rect.Height()) / 2);

			m_rectButton.OffsetRect(0, yOffset);
			m_rectCombo.OffsetRect(0, yOffset);
			m_rect.OffsetRect(0, yOffset);
		}
	}

	CRect rectParent;
	m_pWndCombo->SetWindowPos(NULL, m_rect.left + nHorzMargin, m_rect.top, m_rect.Width() - 2 * nHorzMargin, m_nDropDownHeight, SWP_NOZORDER | SWP_NOACTIVATE);
	m_pWndCombo->SetEditSel(-1, 0);

	{
		CRect rect;
		m_pWndCombo->GetWindowRect(&m_rectCombo);
		m_pWndCombo->ScreenToClient(&m_rectCombo);
		m_pWndCombo->MapWindowPoints(m_pWndCombo->GetParent(), &m_rectCombo);

	}

	if (m_bFlat)
	{
		m_rectButton = m_rectCombo;
		m_rectButton.left = m_rectButton.right - CMenuImages::Size().cx * 2;

		m_rectButton.DeflateRect(2, 2);

		m_rect.left = m_rectCombo.left - nHorzMargin;
		m_rect.right = m_rectCombo.right + nHorzMargin;

		if (!m_bTextBelow || m_strText.IsEmpty())
		{
			m_rect.top = m_rectCombo.top;
			m_rect.bottom = m_rectCombo.bottom;
		}

		if (m_pWndEdit != NULL)
		{
			CRect rectEdit = m_rect;

			const int iBorderOffset = 3;

			m_pWndEdit->SetWindowPos(NULL, m_rect.left + nHorzMargin + iBorderOffset, m_rect.top + iBorderOffset,
				m_rect.Width() - 2 * nHorzMargin - m_rectButton.Width() - iBorderOffset - 3, m_rectCombo.Height() - 2 * iBorderOffset, SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}
	else
	{
		m_rectButton.SetRectEmpty();
	}
}

void CMFCToolBarComboBoxButton::SetHotEdit(BOOL bHot)
{
	if (m_bIsHotEdit != bHot)
	{
		m_bIsHotEdit = bHot;

		if (m_pWndCombo->GetParent() != NULL)
		{
			m_pWndCombo->GetParent()->InvalidateRect(m_rectCombo);
			m_pWndCombo->GetParent()->UpdateWindow();
		}
	}
}

BOOL CMFCToolBarComboBoxButton::NotifyCommand(int iNotifyCode)
{
	if (m_pWndCombo->GetSafeHwnd() == NULL)
	{
		return FALSE;
	}

	if (m_bFlat && iNotifyCode == 0)
	{
		return TRUE;
	}

	if (m_bFlat && m_pWndCombo->GetParent() != NULL)
	{
		m_pWndCombo->GetParent()->InvalidateRect(m_rectCombo);
		m_pWndCombo->GetParent()->UpdateWindow();
	}

	switch (iNotifyCode)
	{
	case CBN_SELENDOK:
		{
			m_iSelIndex = m_pWndCombo->GetCurSel();
			if (m_iSelIndex < 0)
			{
				return FALSE;
			}

			m_pWndCombo->GetLBText(m_iSelIndex, m_strEdit);
			if (m_pWndEdit != NULL)
			{
				m_pWndEdit->SetWindowText(m_strEdit);
			}

			// Try set selection in ALL comboboxes with the same ID:
			CObList listButtons;
			if (CMFCToolBar::GetCommandButtons(m_nID, listButtons) > 0)
			{
				for (POSITION posCombo = listButtons.GetHeadPosition(); posCombo != NULL;)
				{
					CMFCToolBarComboBoxButton* pCombo = DYNAMIC_DOWNCAST(CMFCToolBarComboBoxButton, listButtons.GetNext(posCombo));

					if (pCombo != NULL && pCombo != this)
					{
						pCombo->SelectItem(m_pWndCombo->GetCurSel(), FALSE /* Don't notify */);

						if (pCombo->m_pWndCombo->GetSafeHwnd() != NULL && pCombo->m_pWndCombo->GetParent() != NULL)
						{
							pCombo->m_pWndCombo->GetParent()->InvalidateRect(pCombo->m_rectCombo);
							pCombo->m_pWndCombo->GetParent()->UpdateWindow();
						}
					}
				}
			}
		}

		if (m_pWndEdit != NULL)
		{
			m_pWndEdit->SetFocus();
		}

		return TRUE;

	case CBN_KILLFOCUS:
	case CBN_EDITUPDATE:
		return TRUE;

	case CBN_SETFOCUS:
		if (m_pWndEdit != NULL)
		{
			m_pWndEdit->SetFocus();
		}
		return TRUE;

	case CBN_SELCHANGE: // yurig: process selchange
		if (m_pWndEdit != NULL)
		{
			CString strEdit;
			m_pWndCombo->GetLBText(m_pWndCombo->GetCurSel(), strEdit);
			m_pWndEdit->SetWindowText(strEdit);
		}

		return TRUE;

	case CBN_EDITCHANGE:
		{
			m_pWndCombo->GetWindowText(m_strEdit);

			if (m_pWndEdit != NULL && m_pWndEdit->GetSafeHwnd() != NULL)
			{
				CString str;
				m_pWndEdit->GetWindowText(str);
				CComboBox* pBox = GetComboBox();
				if (pBox != NULL && pBox->GetSafeHwnd() != NULL)
				{
					int nCurSel = pBox->GetCurSel();
					int nNextSel = pBox->FindStringExact(nCurSel + 1, str);
					if (nNextSel == -1)
					{
						nNextSel = pBox->FindString(nCurSel + 1, str);
					}

					if (nNextSel != -1)
					{
						pBox->SetCurSel(nNextSel);
					}

					pBox->SetWindowText(str);
				}
			}

			// Try set text of ALL comboboxes with the same ID:
			CObList listButtons;
			if (CMFCToolBar::GetCommandButtons(m_nID, listButtons) > 0)
			{
				for (POSITION posCombo = listButtons.GetHeadPosition(); posCombo != NULL;)
				{
					CMFCToolBarComboBoxButton* pCombo = DYNAMIC_DOWNCAST(CMFCToolBarComboBoxButton, listButtons.GetNext(posCombo));

					if (pCombo != NULL && pCombo != this)
					{
						if (pCombo->GetComboBox() != NULL)
						{
							pCombo->GetComboBox()->SetWindowText(m_strEdit);
						}

						pCombo->m_strEdit = m_strEdit;
					}
				}
			}
		}
		return TRUE;
	}

	return FALSE;
}

void CMFCToolBarComboBoxButton::OnAddToCustomizePage()
{
	CObList listButtons; // Existing buttons with the same command ID

	if (CMFCToolBar::GetCommandButtons(m_nID, listButtons) == 0)
	{
		return;
	}

	CMFCToolBarComboBoxButton* pOther = (CMFCToolBarComboBoxButton*) listButtons.GetHead();
	ASSERT_VALID(pOther);
	ASSERT_KINDOF(CMFCToolBarComboBoxButton, pOther);

	CopyFrom(*pOther);
}

HBRUSH CMFCToolBarComboBoxButton::OnCtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
	pDC->SetTextColor(afxGlobalData.clrWindowText);
	pDC->SetBkColor(afxGlobalData.clrWindow);

	return(HBRUSH) afxGlobalData.brWindow.GetSafeHandle();
}

void CMFCToolBarComboBoxButton::OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages, BOOL bHorz, BOOL bCustomizeMode, BOOL bHighlight, BOOL bDrawBorder, BOOL bGrayDisabledButtons)
{
	if (m_pWndCombo == NULL || m_pWndCombo->GetSafeHwnd() == NULL || !bHorz)
	{
		CMFCToolBarButton::OnDraw(pDC, rect, pImages, bHorz, bCustomizeMode, bHighlight, bDrawBorder, bGrayDisabledButtons);
		return;
	}

	BOOL bDisabled = (bCustomizeMode && !IsEditable()) || (!bCustomizeMode &&(m_nStyle & TBBS_DISABLED));

	pDC->SetTextColor(bDisabled ? afxGlobalData.clrGrayedText : (bHighlight) ? CMFCToolBar::GetHotTextColor() : afxGlobalData.clrBarText);

	if (m_bFlat)
	{
		if (m_bIsHotEdit)
		{
			bHighlight = TRUE;
		}

		// Draw combbox:
		CRect rectCombo = m_rectCombo;

		// Draw border:
		CMFCVisualManager::GetInstance()->OnDrawComboBorder(pDC, rectCombo, bDisabled, m_pWndCombo->GetDroppedState(), bHighlight, this);

		rectCombo.DeflateRect(2, 2);

		int nPrevTextColor = pDC->GetTextColor();

		pDC->FillSolidRect(rectCombo, bDisabled ? afxGlobalData.clrBtnFace : afxGlobalData.clrWindow);

		if (bDisabled)
		{
			pDC->Draw3dRect(&rectCombo, afxGlobalData.clrBarHilite, afxGlobalData.clrBarHilite);
		}

		// Draw drop-down button:
		CRect rectButton = m_rectButton;
		if (afxGlobalData.m_bIsBlackHighContrast)
		{
			rectButton.DeflateRect(1, 1);
		}

		if (rectButton.left > rectCombo.left + 1)
		{
			CMFCVisualManager::GetInstance()->OnDrawComboDropButton(pDC, rectButton, bDisabled, m_pWndCombo->GetDroppedState(), bHighlight, this);
		}

		pDC->SetTextColor(nPrevTextColor);

		// Draw combo text:
		if (!m_strEdit.IsEmpty())
		{
			CRect rectText = rectCombo;
			rectText.right = m_rectButton.left;
			rectText.DeflateRect(2, 2);

			if (m_pWndEdit == NULL)
			{
				if (m_pWndCombo->GetStyle() &(CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE))
				{
					DRAWITEMSTRUCT dis;
					memset(&dis, 0, sizeof(DRAWITEMSTRUCT));

					dis.hDC = pDC->GetSafeHdc();
					dis.rcItem = rectText;
					dis.CtlID = m_nID;
					dis.itemID = m_pWndCombo->GetCurSel();
					dis.hwndItem = m_pWndCombo->GetSafeHwnd();
					dis.CtlType = ODT_COMBOBOX;
					dis.itemState |= ODS_COMBOBOXEDIT;
					dis.itemData = m_pWndCombo->GetItemData(dis.itemID);

					if (bDisabled)
					{
						dis.itemState |= ODS_DISABLED;
					}

					m_pWndCombo->DrawItem(&dis);
				}
				else
				{
					COLORREF cltTextOld = pDC->SetTextColor(afxGlobalData.clrWindowText);
					pDC->DrawText(m_strEdit, rectText, DT_VCENTER | DT_SINGLELINE);
					pDC->SetTextColor(cltTextOld);
				}
			}

		}

		pDC->SetTextColor(nPrevTextColor);
	}

	if ((m_bTextBelow && bHorz) && !m_strText.IsEmpty())
	{
		CRect rectText = rect;
		rectText.top = (m_rectCombo.bottom + rect.bottom - m_sizeText.cy) / 2;

		pDC->DrawText(m_strText, &rectText, DT_CENTER | DT_WORDBREAK);
	}
}

BOOL CMFCToolBarComboBoxButton::OnClick(CWnd* pWnd, BOOL /*bDelay*/)
{
	if (m_pWndCombo == NULL || m_pWndCombo->GetSafeHwnd() == NULL || !m_bHorz)
	{
		return FALSE;
	}

	if (m_bFlat)
	{
		if (m_pWndEdit == NULL)
		{
			m_pWndCombo->SetFocus();
		}
		else
		{
			m_pWndEdit->SetFocus();
		}

		m_pWndCombo->ShowDropDown();

		if (pWnd != NULL)
		{
			pWnd->InvalidateRect(m_rectCombo);
		}
	}

	return TRUE;
}

BOOL CMFCToolBarComboBoxButton::SelectItem(int iIndex, BOOL bNotify)
{
	if (iIndex >= m_lstItems.GetCount())
	{
		return FALSE;
	}

	m_iSelIndex = max(-1, iIndex);

	if (m_pWndCombo->GetSafeHwnd() == NULL)
	{
		return TRUE;
	}

	if (m_iSelIndex >= 0)
	{
		m_pWndCombo->GetLBText(iIndex, m_strEdit);
	}
	else
	{
		m_strEdit.Empty();
	}

	if (m_pWndEdit != NULL)
	{
		CString strEdit;
		m_pWndEdit->GetWindowText(strEdit);

		if (strEdit != m_strEdit)
		{
			m_pWndEdit->SetWindowText(m_strEdit);
		}
	}

	if (m_pWndCombo->GetCurSel() == iIndex)
	{
		// Already selected
		return TRUE;
	}

	if (m_pWndCombo->SetCurSel(iIndex) != CB_ERR)
	{
		if (bNotify)
		{
			NotifyCommand(CBN_SELENDOK);
		}

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CMFCToolBarComboBoxButton::SelectItem(DWORD_PTR dwData)
{
	int iIndex = 0;
	for (POSITION pos = m_lstItemData.GetHeadPosition(); pos != NULL; iIndex ++)
	{
		if (m_lstItemData.GetNext(pos) == dwData)
		{
			return SelectItem(iIndex);
		}
	}

	return FALSE;
}

BOOL CMFCToolBarComboBoxButton::SelectItem(LPCTSTR lpszText)
{
	ENSURE(lpszText != NULL);

	int iIndex = FindItem(lpszText);
	if (iIndex < 0)
	{
		return FALSE;
	}

	return SelectItem(iIndex);
}

BOOL CMFCToolBarComboBoxButton::DeleteItem(int iIndex)
{
	if (iIndex < 0 || iIndex >= m_lstItems.GetCount())
	{
		return FALSE;
	}

	POSITION pos = m_lstItems.FindIndex(iIndex);
	if (pos == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	m_lstItems.RemoveAt(pos);

	pos = m_lstItemData.FindIndex(iIndex);
	if (pos == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	m_lstItemData.RemoveAt(pos);

	if (m_pWndCombo->GetSafeHwnd() != NULL)
	{
		m_pWndCombo->DeleteString(iIndex);
	}

	if (iIndex == m_iSelIndex)
	{
		int iSelIndex = m_iSelIndex;
		if (iSelIndex >= m_lstItems.GetCount())
		{
			iSelIndex = (int) m_lstItems.GetCount() - 1;
		}

		SelectItem(iSelIndex, FALSE);
	}

	return TRUE;
}

BOOL CMFCToolBarComboBoxButton::DeleteItem(DWORD_PTR dwData)
{
	int iIndex = 0;
	for (POSITION pos = m_lstItemData.GetHeadPosition(); pos != NULL; iIndex ++)
	{
		if (m_lstItemData.GetNext(pos) == dwData)
		{
			return DeleteItem(iIndex);
		}
	}

	return FALSE;
}

BOOL CMFCToolBarComboBoxButton::DeleteItem(LPCTSTR lpszText)
{
	ENSURE(lpszText != NULL);

	int iIndex = FindItem(lpszText);
	if (iIndex < 0)
	{
		return FALSE;
	}

	return DeleteItem(iIndex);
}

int CMFCToolBarComboBoxButton::FindItem(LPCTSTR lpszText) const
{
	ENSURE(lpszText != NULL);

	int iIndex = 0;
	for (POSITION pos = m_lstItems.GetHeadPosition(); pos != NULL; iIndex++)
	{
		if (m_lstItems.GetNext(pos).CompareNoCase(lpszText) == 0)
		{
			return iIndex;
		}
	}

	return CB_ERR;
}

int CMFCToolBarComboBoxButton::OnDrawOnCustomizeList(CDC* pDC, const CRect& rect, BOOL bSelected)
{
	int iWidth = CMFCToolBarButton::OnDrawOnCustomizeList(pDC, rect, bSelected) + 10;

	// Simulate combobox appearance:
	CRect rectCombo = rect;
	int nComboWidth = max(20, rect.Width() - iWidth);

	rectCombo.left = rectCombo.right - nComboWidth;

	int nMargin = 1;
	rectCombo.DeflateRect(nMargin, nMargin);

	pDC->FillRect(rectCombo, &afxGlobalData.brWindow);

	pDC->Draw3dRect(rectCombo, afxGlobalData.clrBarShadow, afxGlobalData.clrBarShadow);

	CRect rectBtn = rectCombo;
	rectBtn.left = rectBtn.right - rectBtn.Height() + 2;
	rectBtn.DeflateRect(nMargin, nMargin);

	CMFCVisualManager::GetInstance()->OnDrawComboDropButton(pDC, rectBtn, FALSE, FALSE, FALSE, this);

	return rect.Width();
}

CComboBox* CMFCToolBarComboBoxButton::CreateCombo(CWnd* pWndParent, const CRect& rect)
{
	CComboBox* pWndCombo = new CComboBox;
	if (!pWndCombo->Create(m_dwStyle, rect, pWndParent, m_nID))
	{
		delete pWndCombo;
		return NULL;
	}

	return pWndCombo;
}

CMFCToolBarComboBoxEdit* CMFCToolBarComboBoxButton::CreateEdit(CWnd* pWndParent, const CRect& rect, DWORD dwEditStyle)
{
	CMFCToolBarComboBoxEdit* pWndEdit = new CMFCToolBarComboBoxEdit(*this);

	if (!pWndEdit->Create(dwEditStyle, rect, pWndParent, m_nID))
	{
		delete pWndEdit;
		return NULL;
	}

	return pWndEdit;
}

void CMFCToolBarComboBoxButton::OnShow(BOOL bShow)
{
	if (m_pWndCombo->GetSafeHwnd() != NULL)
	{
		if (bShow && m_bHorz)
		{
			OnMove();
			m_pWndCombo->ShowWindow(m_bFlat ? SW_HIDE : SW_SHOWNOACTIVATE);
		}
		else
		{
			m_pWndCombo->ShowWindow(SW_HIDE);
		}
	}

	if (m_pWndEdit->GetSafeHwnd() != NULL)
	{
		if (bShow && m_bHorz)
		{
			m_pWndEdit->ShowWindow(SW_SHOWNOACTIVATE);
		}
		else
		{
			m_pWndEdit->ShowWindow(SW_HIDE);
		}
	}
}

BOOL CMFCToolBarComboBoxButton::ExportToMenuButton(CMFCToolBarMenuButton& menuButton) const
{
	CString strMessage;
	int iOffset;

	if (strMessage.LoadString(m_nID) && (iOffset = strMessage.Find(_T('\n'))) != -1)
	{
		menuButton.m_strText = strMessage.Mid(iOffset + 1);
	}

	return TRUE;
}

void CMFCToolBarComboBoxButton::SetDropDownHeight(int nHeight)
{
	if (m_nDropDownHeight == nHeight)
	{
		return;
	}

	m_nDropDownHeight = nHeight;
	OnMove();
}

void CMFCToolBarComboBoxButton::SetText(LPCTSTR lpszText)
{
	ENSURE(lpszText != NULL);

	if (!SelectItem(lpszText))
	{
		m_strEdit = lpszText;

		if (m_pWndCombo != NULL && !m_bFlat)
		{
			CString strText;
			m_pWndCombo->GetWindowText(strText);

			if (strText != lpszText)
			{
				m_pWndCombo->SetWindowText(lpszText);
				NotifyCommand(CBN_EDITCHANGE);
			}
		}

		if (m_pWndEdit != NULL)
		{
			CString strText;
			m_pWndEdit->GetWindowText(strText);

			if (strText != lpszText)
			{
				m_pWndEdit->SetWindowText(lpszText);
			}
		}
	}
}

CMFCToolBarComboBoxButton* __stdcall CMFCToolBarComboBoxButton::GetByCmd(UINT uiCmd, BOOL bIsFocus)
{
	CMFCToolBarComboBoxButton* pSrcCombo = NULL;

	CObList listButtons;
	if (CMFCToolBar::GetCommandButtons(uiCmd, listButtons) > 0)
	{
		for (POSITION posCombo= listButtons.GetHeadPosition(); posCombo != NULL;)
		{
			CMFCToolBarComboBoxButton* pCombo = DYNAMIC_DOWNCAST(CMFCToolBarComboBoxButton, listButtons.GetNext(posCombo));
			ENSURE(pCombo != NULL);

			if (pCombo != NULL &&(!bIsFocus || pCombo->HasFocus()))
			{
				pSrcCombo = pCombo;
				break;
			}
		}
	}

	return pSrcCombo;
}

BOOL __stdcall CMFCToolBarComboBoxButton::SelectItemAll(UINT uiCmd, int iIndex)
{
	CMFCToolBarComboBoxButton* pSrcCombo = GetByCmd(uiCmd);

	if (pSrcCombo)
	{
		pSrcCombo->SelectItem(iIndex);
	}

	return pSrcCombo != NULL;
}

BOOL __stdcall CMFCToolBarComboBoxButton::SelectItemAll(UINT uiCmd, DWORD_PTR dwData)
{
	CMFCToolBarComboBoxButton* pSrcCombo = GetByCmd(uiCmd);

	if (pSrcCombo)
	{
		pSrcCombo->SelectItem(dwData);
	}

	return pSrcCombo != NULL;
}

BOOL __stdcall CMFCToolBarComboBoxButton::SelectItemAll(UINT uiCmd, LPCTSTR lpszText)
{
	CMFCToolBarComboBoxButton* pSrcCombo = GetByCmd(uiCmd);

	if (pSrcCombo)
	{
		pSrcCombo->SelectItem(lpszText);
	}

	return pSrcCombo != NULL;
}

int __stdcall CMFCToolBarComboBoxButton::GetCountAll(UINT uiCmd)
{
	CMFCToolBarComboBoxButton* pSrcCombo = GetByCmd(uiCmd);

	if (pSrcCombo)
	{
		return(int) pSrcCombo->GetCount();
	}

	return CB_ERR;
}

int __stdcall CMFCToolBarComboBoxButton::GetCurSelAll(UINT uiCmd)
{
	CMFCToolBarComboBoxButton* pSrcCombo = GetByCmd(uiCmd);

	if (pSrcCombo)
	{
		return pSrcCombo->GetCurSel();
	}

	return CB_ERR;
}

LPCTSTR __stdcall CMFCToolBarComboBoxButton::GetItemAll(UINT uiCmd, int iIndex)
{
	CMFCToolBarComboBoxButton* pSrcCombo = GetByCmd(uiCmd);

	if (pSrcCombo)
	{
		return pSrcCombo->GetItem(iIndex);
	}

	return NULL;
}

DWORD_PTR __stdcall CMFCToolBarComboBoxButton::GetItemDataAll(UINT uiCmd, int iIndex)
{
	CMFCToolBarComboBoxButton* pSrcCombo = GetByCmd(uiCmd);

	if (pSrcCombo)
	{
		return pSrcCombo->GetItemData(iIndex);
	}

	return(DWORD_PTR)CB_ERR;
}

void* __stdcall CMFCToolBarComboBoxButton::GetItemDataPtrAll(UINT uiCmd, int iIndex)
{
	CMFCToolBarComboBoxButton* pSrcCombo = GetByCmd(uiCmd);

	if (pSrcCombo)
	{
		return pSrcCombo->GetComboBox()->GetItemDataPtr(iIndex);
	}

	return NULL;
}

LPCTSTR __stdcall CMFCToolBarComboBoxButton::GetTextAll(UINT uiCmd)
{
	CMFCToolBarComboBoxButton* pSrcCombo = GetByCmd(uiCmd);

	if (pSrcCombo)
	{
		return pSrcCombo->GetText();
	}

	return NULL;
}

void CMFCToolBarComboBoxButton::SetStyle(UINT nStyle)
{
	CMFCToolBarButton::SetStyle(nStyle);

	BOOL bDisabled = (CMFCToolBar::IsCustomizeMode() || !IsEditable() || (m_nStyle & TBBS_DISABLED));

	if (m_pWndCombo != NULL && m_pWndCombo->GetSafeHwnd() != NULL)
	{
		m_pWndCombo->EnableWindow(!bDisabled);
		m_pWndCombo->RedrawWindow();
	}

	if (m_pWndEdit != NULL && m_pWndEdit->GetSafeHwnd() != NULL)
	{
		m_pWndEdit->EnableWindow(!bDisabled);
		m_pWndEdit->RedrawWindow();
	}
}

BOOL CMFCToolBarComboBoxButton::HasFocus() const
{
	if (m_pWndCombo == NULL)
	{
		return FALSE;
	}

	CWnd* pWndFocus = CWnd::GetFocus();

	if (m_pWndCombo->GetDroppedState() || pWndFocus == m_pWndCombo || m_pWndCombo->IsChild(pWndFocus))
	{
		return TRUE;
	}

	if (m_pWndEdit == NULL)
	{
		return FALSE;
	}

	return pWndFocus == m_pWndEdit || m_pWndEdit->IsChild(pWndFocus);
}

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarComboBoxEdit

CMFCToolBarComboBoxEdit::CMFCToolBarComboBoxEdit(CMFCToolBarComboBoxButton& combo) : m_combo(combo)
{
	m_bTracked = FALSE;
}

CMFCToolBarComboBoxEdit::~CMFCToolBarComboBoxEdit()
{
}

BEGIN_MESSAGE_MAP(CMFCToolBarComboBoxEdit, CEdit)
	//{{AFX_MSG_MAP(CMFCToolBarComboBoxEdit)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_MOUSEMOVE()
	ON_WM_CONTEXTMENU()
	ON_WM_PAINT()
	ON_MESSAGE(WM_MOUSELEAVE, &CMFCToolBarComboBoxEdit::OnMouseLeave)
	ON_CONTROL_REFLECT(EN_CHANGE, &CMFCToolBarComboBoxEdit::OnChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarComboBoxEdit message handlers

BOOL CMFCToolBarComboBoxEdit::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_MOUSEWHEEL && m_combo.GetComboBox() != NULL && m_combo.GetComboBox()->GetDroppedState())
	{
		m_combo.GetComboBox()->SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
		return TRUE;
	}

	if (pMsg->message == WM_KEYDOWN)
	{
		if ((GetKeyState(VK_MENU) >= 0) &&(GetKeyState(VK_CONTROL) >= 0) && m_combo.GetComboBox() != NULL)
		{
			switch (pMsg->wParam)
			{
			case VK_UP:
			case VK_DOWN:
			case VK_HOME:
			case VK_END:
			case VK_NEXT:
			case VK_PRIOR:
				if (!m_combo.GetComboBox()->GetDroppedState())
				{
					break;
				}

			case VK_RETURN:
				SetFocus();

				if (m_combo.GetComboBox()->GetDroppedState())
				{
					m_combo.GetComboBox()->SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
				}
				else if (m_combo.GetComboBox()->GetOwner() != NULL)
				{
					GetWindowText(m_combo.m_strEdit);
					m_combo.GetComboBox()->GetOwner()->PostMessage(WM_COMMAND, MAKEWPARAM(m_combo.m_nID, 0), (LPARAM) m_combo.GetComboBox()->GetSafeHwnd());
				}

				return TRUE;
			}
		}

		switch (pMsg->wParam)
		{
		case VK_TAB:
			if (GetParent() != NULL)
			{
				ASSERT_VALID(GetParent());
				GetParent()->GetNextDlgTabItem(this)->SetFocus();
				return TRUE;
			}
			break;

		case VK_ESCAPE:
			if (m_combo.GetComboBox() != NULL)
			{
				m_combo.GetComboBox()->ShowDropDown(FALSE);
			}

			if (GetTopLevelFrame() != NULL)
			{
				GetTopLevelFrame()->SetFocus();
				return TRUE;
			}

			break;

		case VK_UP:
		case VK_DOWN:
			if ((GetKeyState(VK_MENU) >= 0) &&(GetKeyState(VK_CONTROL) >=0) && m_combo.GetComboBox() != NULL)
			{
				if (!m_combo.GetComboBox()->GetDroppedState())
				{
					m_combo.GetComboBox()->ShowDropDown();

					if (m_combo.GetComboBox()->GetParent() != NULL)
					{
						m_combo.GetComboBox()->GetParent()->InvalidateRect(m_combo.m_rectCombo);
					}
				}
				return TRUE;
			}
		}
	}

	return CEdit::PreTranslateMessage(pMsg);
}

void CMFCToolBarComboBoxEdit::OnSetFocus(CWnd* pOldWnd)
{
	CEdit::OnSetFocus(pOldWnd);
	m_combo.SetHotEdit();
	m_combo.NotifyCommand(CBN_SETFOCUS);
}

void CMFCToolBarComboBoxEdit::OnKillFocus(CWnd* pNewWnd)
{
	CEdit::OnKillFocus(pNewWnd);

	if (::IsWindow(m_combo.GetHwnd()))
	{
		m_combo.SetHotEdit(FALSE);
		m_combo.NotifyCommand(CBN_KILLFOCUS);
	}
}

void CMFCToolBarComboBoxEdit::OnChange()
{
	m_combo.NotifyCommand(CBN_EDITCHANGE);
}

void CMFCToolBarComboBoxEdit::OnMouseMove(UINT nFlags, CPoint point)
{
	CEdit::OnMouseMove(nFlags, point);
	m_combo.SetHotEdit();

	if (!m_bTracked)
	{
		m_bTracked = TRUE;

		TRACKMOUSEEVENT trackmouseevent;
		trackmouseevent.cbSize = sizeof(trackmouseevent);
		trackmouseevent.dwFlags = TME_LEAVE;
		trackmouseevent.hwndTrack = GetSafeHwnd();
		trackmouseevent.dwHoverTime = HOVER_DEFAULT;
		::AFXTrackMouse(&trackmouseevent);
	}
}

afx_msg LRESULT CMFCToolBarComboBoxEdit::OnMouseLeave(WPARAM,LPARAM)
{
	m_bTracked = FALSE;

	if (CWnd::GetFocus() != this)
	{
		m_combo.SetHotEdit(FALSE);
	}

	return 0;
}

void CMFCToolBarComboBoxEdit::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (m_combo.m_uiMenuResID != 0)
	{

		CWnd* pWndParent = pWnd->GetParent();

		HINSTANCE hInst = AfxFindResourceHandle(MAKEINTRESOURCE(m_combo.m_uiMenuResID), RT_MENU);

		if (hInst == NULL)
		{
			CEdit::OnContextMenu(pWnd, point) ;
			return;

		}

		HMENU hMenu = ::LoadMenu(hInst, MAKEINTRESOURCE(m_combo.m_uiMenuResID));
		if (hMenu == NULL)
		{
			CEdit::OnContextMenu(pWnd, point) ;
			return;
		}

		HMENU hPopupMenu = ::GetSubMenu(hMenu, 0);

		if (hPopupMenu == NULL)
		{
			CEdit::OnContextMenu(pWnd, point) ;
			return;
		}

		if (afxContextMenuManager != NULL)
		{
			afxContextMenuManager->ShowPopupMenu(hPopupMenu, point.x, point.y, pWndParent);

		}
		else
		{
			::TrackPopupMenu(hPopupMenu, TPM_CENTERALIGN | TPM_LEFTBUTTON, point.x, point.y, 0, pWndParent->GetSafeHwnd(), NULL);
		}
	}
	else
	{
		CEdit::OnContextMenu(pWnd, point) ;
	}
}

INT_PTR CMFCToolBarComboBoxButton::AddSortedItem(LPCTSTR lpszItem, DWORD_PTR dwData)
{
	ENSURE(lpszItem != NULL);

	if (m_strEdit.IsEmpty())
	{
		m_strEdit = lpszItem;
		if (m_pWndEdit != NULL)
		{
			m_pWndEdit->SetWindowText(m_strEdit);
		}
	}

	int nIndex = 0;
	BOOL bInserted = FALSE;

	if (FindItem(lpszItem) < 0)
	{
		for (nIndex =0; nIndex < m_lstItems.GetCount(); nIndex++)
		{
			POSITION pos = m_lstItems.FindIndex(nIndex);
			LPCTSTR str = (LPCTSTR) m_lstItems.GetAt(pos);
			if (Compare(lpszItem, str) < 0)
			{
				m_lstItems.InsertBefore(pos, lpszItem);
				POSITION posData = m_lstItemData.FindIndex(nIndex);
				m_lstItemData.InsertBefore(posData, dwData);
				bInserted = TRUE;
				break;
			};
		}

		if (!bInserted)
		{
			m_lstItems.AddTail(lpszItem);
			m_lstItemData.AddTail(dwData);
		}
	}

	if (m_pWndCombo->GetSafeHwnd() != NULL)
	{
		int iIndex = m_pWndCombo->FindStringExact(-1, lpszItem);

		if (iIndex == CB_ERR)
		{
			if (!bInserted)
			{
				iIndex = m_pWndCombo->AddString(lpszItem);
			}
			else
			{
				iIndex = m_pWndCombo->InsertString(nIndex, lpszItem);
			}
		}

		m_pWndCombo->SetCurSel(iIndex);
		m_pWndCombo->SetItemData(iIndex, dwData);
		m_pWndCombo->SetEditSel(-1, 0);
	}

	if (bInserted)
	{
		return nIndex;
	}

	return m_lstItems.GetCount() - 1;

}

int CMFCToolBarComboBoxButton::Compare(LPCTSTR lpszItem1, LPCTSTR lpszItem2)
{
	return _tcscmp(lpszItem1, lpszItem2);
}

void CMFCToolBarComboBoxButton::OnGlobalFontsChanged()
{
	CMFCToolBarButton::OnGlobalFontsChanged();

	if (m_pWndEdit->GetSafeHwnd() != NULL)
	{
		m_pWndEdit->SetFont(&afxGlobalData.fontRegular);
	}

	if (m_pWndCombo->GetSafeHwnd() != NULL)
	{
		m_pWndCombo->SetFont(&afxGlobalData.fontRegular);
	}
}

BOOL CMFCToolBarComboBoxButton::OnUpdateToolTip(CWnd* pWndParent, int iButtonIndex, CToolTipCtrl& wndToolTip, CString& strTipText)
{
	if (!m_bHorz)
	{
		return FALSE;
	}

	CString strTips;

	if (OnGetCustomToolTipText(strTips))
	{
		strTipText = strTips;
	}

	if (CMFCToolBarComboBoxButton::IsFlatMode())
	{
		CComboBox* pCombo = GetComboBox();

		if (pCombo != NULL &&(pCombo->GetStyle() & CBS_DROPDOWNLIST) == CBS_DROPDOWN)
		{
			CEdit* pEdit = GetEditCtrl();
			if (pEdit != NULL)
			{
				wndToolTip.AddTool(pEdit, strTipText, NULL, 0);
			}
		}
		else
		{
			wndToolTip.AddTool(pWndParent, strTipText, Rect(), iButtonIndex + 1);
		}
	}
	else
	{
		CComboBox* pCombo = GetComboBox();
		if (pCombo != NULL)
		{
			wndToolTip.AddTool(pCombo, strTipText, NULL, 0);
		}
	}

	return TRUE;
}

BOOL CMFCToolBarComboBoxButton::SetACCData(CWnd* pParent, CAccessibilityData& data)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pParent);

	if (!CMFCToolBarButton::SetACCData(pParent, data))
	{
		return FALSE;
	}

	CComboBox* pCombo = GetComboBox();
	if (pCombo != NULL && (pCombo->GetStyle() & CBS_DROPDOWNLIST) == CBS_DROPDOWNLIST)
	{
		data.m_nAccRole = ROLE_SYSTEM_DROPLIST;
	}
	else
	{
		data.m_nAccRole = ROLE_SYSTEM_COMBOBOX;
	}

	data.m_bAccState = STATE_SYSTEM_FOCUSABLE;

	if (HasFocus())
	{
		data.m_bAccState |= STATE_SYSTEM_FOCUSED;
	}

	data.m_strAccDefAction = _T("Open");
	data.m_strAccValue = GetText();

	return TRUE;
}

void CMFCToolBarComboBoxEdit::OnPaint()
{
	CString str;
	GetWindowText(str);

	if (!str.IsEmpty() || m_combo.GetPrompt().IsEmpty() || GetFocus() == this)
	{
		Default();
		return;
	}

	CRect rect;
	GetClientRect(rect);

	CPaintDC dc(this);
	dc.FillRect(rect, &afxGlobalData.brWindow);

	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(afxGlobalData.clrGrayedText);
	CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontRegular);

	rect.DeflateRect(1, 1);
	dc.DrawText(m_combo.GetPrompt(), rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

	dc.SelectObject(pOldFont);
}


