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
#include "afxtoolbareditboxbutton.h"
#include "afxvisualmanager.h"
#include "afxtrackmouse.h"
#include "afxcontextmenumanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CMFCToolBarEditBoxButton, CMFCToolBarButton, 1)

static const int nDefaultSize = 150;
static const int nHorzMargin = 3;
static const int nVertMargin = 1;

BOOL CMFCToolBarEditBoxButton::m_bFlat = TRUE;

// Construction/Destruction
CMFCToolBarEditBoxButton::CMFCToolBarEditBoxButton()
{
	m_dwStyle = WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL;
	m_iWidth = nDefaultSize;

	Initialize();
}

CMFCToolBarEditBoxButton::CMFCToolBarEditBoxButton(UINT uiId, int iImage, DWORD dwStyle, int iWidth) :
	CMFCToolBarButton(uiId, iImage)
{
	m_dwStyle = dwStyle | WS_CHILD | WS_VISIBLE;
	m_iWidth = (iWidth == 0) ? nDefaultSize : iWidth;

	Initialize();
}

void CMFCToolBarEditBoxButton::Initialize()
{
	m_pWndEdit = NULL;
	m_bHorz = TRUE;
	m_bChangingText = FALSE;
	m_bIsHotEdit = FALSE;
	m_uiMenuResID = 0;
}

CMFCToolBarEditBoxButton::~CMFCToolBarEditBoxButton()
{
	if (m_pWndEdit != NULL)
	{
		m_pWndEdit->DestroyWindow();
		delete m_pWndEdit;
	}
}

void CMFCToolBarEditBoxButton::CopyFrom(const CMFCToolBarButton& s)
{
	CMFCToolBarButton::CopyFrom(s);

	const CMFCToolBarEditBoxButton& src = (const CMFCToolBarEditBoxButton&) s;

	m_dwStyle = src.m_dwStyle;
	m_iWidth = src.m_iWidth;
	m_strContents = src.m_strContents;
	m_uiMenuResID = src.m_uiMenuResID;
}

void CMFCToolBarEditBoxButton::Serialize(CArchive& ar)
{
	CMFCToolBarButton::Serialize(ar);

	if (ar.IsLoading())
	{
		ar >> m_iWidth;
		m_rect.right = m_rect.left + m_iWidth;
		ar >> m_dwStyle;
		ar >> m_strContents;
		ar >> m_uiMenuResID;
	}
	else
	{
		ar << m_iWidth;
		ar << m_dwStyle;

		if (m_pWndEdit != NULL)
		{
			m_pWndEdit->GetWindowText(m_strContents);
		}
		else
		{
			m_strContents.Empty();
		}

		ar << m_strContents;
		ar << m_uiMenuResID;
	}
}

SIZE CMFCToolBarEditBoxButton::OnCalculateSize(CDC* pDC, const CSize& sizeDefault, BOOL bHorz)
{
	if (!IsVisible())
	{
		if (m_pWndEdit->GetSafeHwnd() != NULL)
		{
			m_pWndEdit->ShowWindow(SW_HIDE);
		}

		OnShowEditbox(FALSE);
		return CSize(0,0);
	}

	m_bHorz = bHorz;

	if (bHorz)
	{
		if (m_pWndEdit->GetSafeHwnd() != NULL && !m_bIsHidden)
		{
			m_pWndEdit->ShowWindow(SW_SHOWNOACTIVATE);
			OnShowEditbox(TRUE);
		}

		if (m_bTextBelow && !m_strText.IsEmpty())
		{
			CRect rectText(0, 0, m_iWidth, sizeDefault.cy);
			pDC->DrawText( m_strText, rectText, DT_CENTER | DT_CALCRECT | DT_WORDBREAK);
			m_sizeText = rectText.Size();
		}
		else
			m_sizeText = CSize(0,0);

		return CSize(m_iWidth, sizeDefault.cy + m_sizeText.cy);
	}
	else
	{
		if (m_pWndEdit->GetSafeHwnd() != NULL)
		{
			m_pWndEdit->ShowWindow(SW_HIDE);
			OnShowEditbox(FALSE);
		}

		m_sizeText = CSize(0,0);

		return CMFCToolBarButton::OnCalculateSize(pDC, sizeDefault, bHorz);
	}
}

void CMFCToolBarEditBoxButton::OnMove()
{
	if (m_pWndEdit->GetSafeHwnd() == NULL ||
		(m_pWndEdit->GetStyle() & WS_VISIBLE) == 0)
	{
		return;
	}

	int cy = afxGlobalData.GetTextHeight();
	int yOffset = max(0, (m_rect.Height() - m_sizeText.cy - cy) / 2);

	m_pWndEdit->SetWindowPos(NULL, m_rect.left + nHorzMargin, m_rect.top + yOffset, m_rect.Width() - 2 * nHorzMargin, cy, SWP_NOZORDER | SWP_NOACTIVATE);
	m_pWndEdit->SetSel(-1, 0);
}

void CMFCToolBarEditBoxButton::OnSize(int iSize)
{
	m_iWidth = iSize;
	m_rect.right = m_rect.left + m_iWidth;

	OnMove();
}

void CMFCToolBarEditBoxButton::OnChangeParentWnd(CWnd* pWndParent)
{
	CMFCToolBarButton::OnChangeParentWnd(pWndParent);

	if (m_pWndEdit->GetSafeHwnd() != NULL)
	{
		CWnd* pWndParentCurr = m_pWndEdit->GetParent();
		ENSURE(pWndParentCurr != NULL);

		if (pWndParent != NULL && pWndParentCurr->GetSafeHwnd() == pWndParent->GetSafeHwnd())
		{
			return;
		}

		m_pWndEdit->GetWindowText(m_strContents);

		m_pWndEdit->DestroyWindow();
		delete m_pWndEdit;
		m_pWndEdit = NULL;
	}

	if (pWndParent == NULL || pWndParent->GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rect = m_rect;
	rect.DeflateRect(nHorzMargin, nVertMargin);
	rect.bottom = rect.top + afxGlobalData.GetTextHeight();

	if ((m_pWndEdit = CreateEdit(pWndParent, rect)) == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	ASSERT_VALID(m_pWndEdit);

	OnMove();
	m_pWndEdit->SetFont(&afxGlobalData.fontRegular);

	CString sText;
	m_pWndEdit->GetWindowText(sText);
	if (sText.IsEmpty())
	{
		m_bChangingText = TRUE;
		m_pWndEdit->SetWindowText(m_strContents);
		m_bChangingText = FALSE;
	}
	else
	{
		m_strContents = sText;
	}
}

BOOL CMFCToolBarEditBoxButton::NotifyCommand(int iNotifyCode)
{
	if (m_pWndEdit->GetSafeHwnd() == NULL)
	{
		return FALSE;
	}

	switch (iNotifyCode)
	{
	case EN_UPDATE:
		{
			m_pWndEdit->GetWindowText(m_strContents);

			// Try set selection in ALL editboxes with the same ID:
			CObList listButtons;
			if (CMFCToolBar::GetCommandButtons(m_nID, listButtons) > 0)
			{
				for (POSITION posCombo = listButtons.GetHeadPosition(); posCombo != NULL;)
				{
					CMFCToolBarEditBoxButton* pEdit = DYNAMIC_DOWNCAST(CMFCToolBarEditBoxButton, listButtons.GetNext(posCombo));

					if ((pEdit != NULL) &&(pEdit != this))
					{
						pEdit->SetContents(m_strContents);
					}
				}
			}
		}

		return !m_bChangingText;
	}

	return FALSE;
}

void CMFCToolBarEditBoxButton::OnAddToCustomizePage()
{
	CObList listButtons; // Existing buttons with the same command ID

	if (CMFCToolBar::GetCommandButtons(m_nID, listButtons) == 0)
	{
		return;
	}

	CMFCToolBarEditBoxButton* pOther =
		(CMFCToolBarEditBoxButton*) listButtons.GetHead();
	ASSERT_VALID(pOther);
	ASSERT_KINDOF(CMFCToolBarEditBoxButton, pOther);

	CopyFrom(*pOther);
}

HBRUSH CMFCToolBarEditBoxButton::OnCtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
	pDC->SetTextColor(afxGlobalData.clrWindowText);
	pDC->SetBkColor(afxGlobalData.clrWindow);

	return(HBRUSH) afxGlobalData.brWindow.GetSafeHandle();
}

void CMFCToolBarEditBoxButton::OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages, BOOL bHorz,
	BOOL bCustomizeMode, BOOL bHighlight, BOOL bDrawBorder, BOOL bGrayDisabledButtons)
{
	if (m_pWndEdit->GetSafeHwnd() == NULL ||
		(m_pWndEdit->GetStyle() & WS_VISIBLE) == 0)
	{
		CMFCToolBarButton::OnDraw(pDC, rect, pImages, bHorz, bCustomizeMode, bHighlight, bDrawBorder, bGrayDisabledButtons);
		return;
	}

	BOOL bDisabled = (bCustomizeMode && !IsEditable()) || (!bCustomizeMode &&(m_nStyle & TBBS_DISABLED));

	CRect rectBorder;
	GetEditBorder(rectBorder);

	CMFCVisualManager::GetInstance()->OnDrawEditBorder(pDC, rectBorder, bDisabled, !m_bFlat || m_bIsHotEdit, this);

	if ((m_bTextBelow && bHorz) && !m_strText.IsEmpty())
	{
		// Draw button's text:
		bDisabled = (bCustomizeMode && !IsEditable()) || (!bCustomizeMode &&(m_nStyle & TBBS_DISABLED));

		pDC->SetTextColor(bDisabled ? afxGlobalData.clrGrayedText : (bHighlight) ? CMFCToolBar::GetHotTextColor() : afxGlobalData.clrBtnText);
		CRect rectText = rect;
		rectText.top = (rectBorder.bottom + rect.bottom - m_sizeText.cy) / 2;
		pDC->DrawText(m_strText, &rectText, DT_CENTER | DT_WORDBREAK);
	}
}

void CMFCToolBarEditBoxButton::GetEditBorder(CRect& rectBorder)
{
	ENSURE(m_pWndEdit->GetSafeHwnd() != NULL);

	m_pWndEdit->GetWindowRect(rectBorder);
	m_pWndEdit->GetParent()->ScreenToClient(rectBorder);
	rectBorder.InflateRect(1, 1);
}

BOOL CMFCToolBarEditBoxButton::OnClick(CWnd* /*pWnd*/, BOOL /*bDelay*/)
{
	return m_pWndEdit->GetSafeHwnd() != NULL && (m_pWndEdit->GetStyle() & WS_VISIBLE);
}

int CMFCToolBarEditBoxButton::OnDrawOnCustomizeList(CDC* pDC, const CRect& rect, BOOL bSelected)
{
	int iWidth = CMFCToolBarButton::OnDrawOnCustomizeList(pDC, rect, bSelected) + 10;

	// Simulate editbox appearance:
	CRect rectEdit = rect;
	int nEditWidth = max(8, rect.Width() - iWidth);

	rectEdit.left = rectEdit.right - nEditWidth;
	rectEdit.DeflateRect(2, 2);

	pDC->FillRect(rectEdit, &afxGlobalData.brWindow);
	pDC->Draw3dRect(rectEdit, afxGlobalData.clrBarShadow, afxGlobalData.clrBarShadow);

	return rect.Width();
}

CEdit* CMFCToolBarEditBoxButton::CreateEdit(CWnd* pWndParent, const CRect& rect)
{
	ASSERT_VALID(this);

	CMFCToolBarEditCtrl* pWndEdit = new CMFCToolBarEditCtrl(*this);
	if (!pWndEdit->Create(m_dwStyle, rect, pWndParent, m_nID))
	{
		delete pWndEdit;
		return NULL;
	}

	return pWndEdit;
}

void CMFCToolBarEditBoxButton::OnShow(BOOL bShow)
{
	if (m_pWndEdit->GetSafeHwnd() != NULL)
	{
		if (bShow)
		{
			m_pWndEdit->ShowWindow(SW_SHOWNOACTIVATE);
			OnMove();
		}
		else
		{
			m_pWndEdit->ShowWindow(SW_HIDE);
		}

		OnShowEditbox(bShow);
	}
}

void CMFCToolBarEditBoxButton::SetContents(const CString& sContents)
{
	if (m_strContents == sContents)
		return;

	m_strContents = sContents;
	if (m_pWndEdit != NULL)
	{
		m_bChangingText = TRUE;
		m_pWndEdit->SetWindowText(m_strContents);
		m_bChangingText = FALSE;
	}
}

const CRect CMFCToolBarEditBoxButton::GetInvalidateRect() const
{
	if ((m_bTextBelow && m_bHorz) && !m_strText.IsEmpty())
	{
		CRect rect;
		rect.left = (m_rect.left + m_rect.right - m_sizeText.cx) / 2;
		rect.right = (m_rect.left + m_rect.right + m_sizeText.cx) / 2;
		rect.top = m_rect.top;
		rect.bottom = m_rect.bottom + m_rect.top + m_sizeText.cy;
		return rect;
	}
	else
		return m_rect;
}

CMFCToolBarEditBoxButton* __stdcall CMFCToolBarEditBoxButton::GetByCmd(UINT uiCmd)
{
	CMFCToolBarEditBoxButton* pSrcEdit = NULL;

	CObList listButtons;
	if (CMFCToolBar::GetCommandButtons(uiCmd, listButtons) > 0)
	{
		for (POSITION posEdit= listButtons.GetHeadPosition(); pSrcEdit == NULL && posEdit != NULL;)
		{
			CMFCToolBarEditBoxButton* pEdit= DYNAMIC_DOWNCAST(CMFCToolBarEditBoxButton, listButtons.GetNext(posEdit));
			ENSURE(pEdit != NULL);

			pSrcEdit = pEdit;
		}
	}

	return pSrcEdit;
}

BOOL __stdcall CMFCToolBarEditBoxButton::SetContentsAll(UINT uiCmd, const CString& strContents)
{
	CMFCToolBarEditBoxButton* pSrcEdit = GetByCmd(uiCmd);

	if (pSrcEdit)
	{
		pSrcEdit->SetContents(strContents);
	}

	return pSrcEdit != NULL;
}

CString __stdcall CMFCToolBarEditBoxButton::GetContentsAll(UINT uiCmd)
{
	CMFCToolBarEditBoxButton* pSrcEdit = GetByCmd(uiCmd);
	CString str;

	if (pSrcEdit)
	{
		pSrcEdit->m_pWndEdit->GetWindowText(str);
	}

	return str;
}

void CMFCToolBarEditBoxButton::SetStyle(UINT nStyle)
{
	CMFCToolBarButton::SetStyle(nStyle);

	if (m_pWndEdit != NULL && m_pWndEdit->GetSafeHwnd() != NULL)
	{
		BOOL bDisabled = (CMFCToolBar::IsCustomizeMode() && !IsEditable()) || (!CMFCToolBar::IsCustomizeMode() &&(m_nStyle & TBBS_DISABLED));

		m_pWndEdit->EnableWindow(!bDisabled);
	}
}

void CMFCToolBarEditBoxButton::SetHotEdit(BOOL bHot)
{
	if (m_bIsHotEdit != bHot)
	{
		m_bIsHotEdit = bHot;

		if (m_pWndEdit->GetParent() != NULL)
		{
			CRect rect = m_rect;

			m_pWndEdit->GetParent()->InvalidateRect(m_rect);
			m_pWndEdit->GetParent()->UpdateWindow();
		}
	}
}

BOOL CMFCToolBarEditBoxButton::OnUpdateToolTip(CWnd* /*pWndParent*/, int /*iButtonIndex*/, CToolTipCtrl& wndToolTip, CString& str)
{
	CEdit* pEdit = GetEditBox();

	if ((pEdit != NULL) &&(::IsWindow(pEdit->GetSafeHwnd())))
	{
		CString strTips;

		if (OnGetCustomToolTipText(strTips))
		{
			wndToolTip.AddTool(pEdit, strTips, NULL, 0);
		}
		else
		{
			wndToolTip.AddTool(pEdit, str, NULL, 0);
		}

		return TRUE;
	}

	return FALSE;
}

void CMFCToolBarEditBoxButton::OnGlobalFontsChanged()
{
	CMFCToolBarButton::OnGlobalFontsChanged();

	if (m_pWndEdit->GetSafeHwnd() != NULL)
	{
		m_pWndEdit->SetFont(&afxGlobalData.fontRegular);
	}
}

BOOL CMFCToolBarEditBoxButton::SetACCData(CWnd* pParent, CAccessibilityData& data)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pParent);

	if (!CMFCToolBarButton::SetACCData(pParent, data))
	{
		return FALSE;
	}

	data.m_nAccRole = ROLE_SYSTEM_TEXT;
	data.m_bAccState = STATE_SYSTEM_FOCUSABLE;

	if (HasFocus())
	{
		data.m_bAccState |= STATE_SYSTEM_FOCUSED;
	}

	data.m_strAccDefAction = _T("Edit");
	data.m_strAccValue = m_strText;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarEditCtrl

CMFCToolBarEditCtrl::CMFCToolBarEditCtrl(CMFCToolBarEditBoxButton& edit) : m_buttonEdit(edit)
{
	m_bTracked = FALSE;
}

CMFCToolBarEditCtrl::~CMFCToolBarEditCtrl()
{
}

BEGIN_MESSAGE_MAP(CMFCToolBarEditCtrl, CMFCEditBrowseCtrl)
	//{{AFX_MSG_MAP(CMFCToolBarEditCtrl)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_MOUSEMOVE()
	ON_WM_CONTEXTMENU()
	ON_MESSAGE(WM_MOUSELEAVE, &CMFCToolBarEditCtrl::OnMouseLeave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarEditCtrl message handlers

BOOL CMFCToolBarEditCtrl::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
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
			if (GetTopLevelFrame() != NULL)
			{
				GetTopLevelFrame()->SetFocus();
				return TRUE;
			}

			break;
		}

		if (GetFocus() == this && GetKeyState(VK_CONTROL) & 0x8000 )
		{
			switch (pMsg->wParam)
			{
			case 'V':
				Paste();
				return TRUE;

			case 'C':
				Copy();
				return TRUE;

			case 'X':
				Cut();
				return TRUE;

			case 'Z':
				Undo();
				return TRUE;

			case VK_DELETE:
				Clear();
				return TRUE;
			}
		}
	}

	return CMFCEditBrowseCtrl::PreTranslateMessage(pMsg);
}

void CMFCToolBarEditCtrl::OnSetFocus(CWnd* pOldWnd)
{
	CMFCEditBrowseCtrl::OnSetFocus(pOldWnd);
	m_buttonEdit.SetHotEdit(TRUE);
}

void CMFCToolBarEditCtrl::OnKillFocus(CWnd* pNewWnd)
{
	CMFCEditBrowseCtrl::OnKillFocus(pNewWnd);
	m_buttonEdit.SetHotEdit(FALSE);
}

void CMFCToolBarEditCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	CMFCEditBrowseCtrl::OnMouseMove(nFlags, point);
	m_buttonEdit.SetHotEdit(TRUE);

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

LRESULT CMFCToolBarEditCtrl::OnMouseLeave(WPARAM,LPARAM)
{
	m_bTracked = FALSE;

	if (CWnd::GetFocus() != this)
	{
		m_buttonEdit.SetHotEdit(FALSE);
	}

	return 0;
}

void CMFCToolBarEditCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{

	if (m_buttonEdit.m_uiMenuResID != 0)
	{

		CWnd* pWndParent = pWnd->GetParent();

		HINSTANCE hInst = AfxFindResourceHandle(MAKEINTRESOURCE(m_buttonEdit.m_uiMenuResID), RT_MENU);

		if (hInst == NULL)
		{
			CMFCEditBrowseCtrl::OnContextMenu(pWnd, point) ;
			return;

		}

		HMENU hMenu = ::LoadMenu(hInst, MAKEINTRESOURCE(m_buttonEdit.m_uiMenuResID));

		if (hMenu == NULL)
		{
			CMFCEditBrowseCtrl::OnContextMenu(pWnd, point) ;
			return;
		}

		HMENU hPopupMenu = ::GetSubMenu(hMenu, 0);

		if (hPopupMenu == NULL)
		{
			CMFCEditBrowseCtrl::OnContextMenu(pWnd, point) ;
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
	}else
	{
		CMFCEditBrowseCtrl::OnContextMenu(pWnd, point) ;
	}
}


