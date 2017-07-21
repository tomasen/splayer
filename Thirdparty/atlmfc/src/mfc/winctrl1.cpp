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
#ifndef _AFX_NO_OCC_SUPPORT
#include "occimpl.h"
#endif



#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// CStatic

BOOL CStatic::Create(LPCTSTR lpszText, DWORD dwStyle,
		const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	CWnd* pWnd = this;
	return pWnd->Create(_T("STATIC"), lpszText, dwStyle, rect, pParentWnd, nID);
}

CStatic::~CStatic()
{
	DestroyWindow();
}

// Derived class is responsible for implementing all of these handlers
//   for owner/self draw controls
void CStatic::DrawItem(LPDRAWITEMSTRUCT)
{
	ASSERT(FALSE);
}

BOOL CStatic::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam,
	LRESULT* pResult)
{
	if (message != WM_DRAWITEM)
		return CWnd::OnChildNotify(message, wParam, lParam, pResult);

	ASSERT(pResult == NULL);       // no return value expected
	UNUSED(pResult); // unused in release builds
	DrawItem((LPDRAWITEMSTRUCT)lParam);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CButton

BOOL CButton::Create(LPCTSTR lpszCaption, DWORD dwStyle,
		const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	CWnd* pWnd = this;
	return pWnd->Create(_T("BUTTON"), lpszCaption, dwStyle, rect, pParentWnd, nID);
}

CButton::~CButton()
{
	DestroyWindow();
}

// Helper for radio buttons
int CWnd::GetCheckedRadioButton(int nIDFirstButton, int nIDLastButton) const
{
	for (int nID = nIDFirstButton; nID <= nIDLastButton; nID++)
	{
		if (IsDlgButtonChecked(nID))
			return nID; // id that matched
	}
	return 0; // invalid ID
}

// Derived class is responsible for implementing all of these handlers
//   for owner/self draw controls
void CButton::DrawItem(LPDRAWITEMSTRUCT)
{
	ASSERT(FALSE);
}

BOOL CButton::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam,
	LRESULT* pResult)
{
	if (message != WM_DRAWITEM)
		return CWnd::OnChildNotify(message, wParam, lParam, pResult);

	ASSERT(pResult == NULL);       // no return value expected
	UNUSED(pResult); // unused in release builds
	DrawItem((LPDRAWITEMSTRUCT)lParam);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CSplitButton

#if (_WIN32_WINNT >= 0x600) && defined(UNICODE)
BEGIN_MESSAGE_MAP(CSplitButton, CButton)
	ON_NOTIFY_REFLECT(BCN_DROPDOWN, &CSplitButton::OnDropDown)
END_MESSAGE_MAP()

BOOL CSplitButton::Create(LPCTSTR lpszCaption, DWORD dwStyle,
		const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	return CButton::Create(lpszCaption, dwStyle|BS_SPLITBUTTON, rect, pParentWnd, nID);
}

CSplitButton::~CSplitButton()
{
	Cleanup();
}

void CSplitButton::SetDropDownMenu(UINT nMenuId, UINT nSubMenuId)
{
	Cleanup();
	m_nMenuId = nMenuId;
	m_nSubMenuId = nSubMenuId;
}

void CSplitButton::SetDropDownMenu(CMenu* pMenu)
{
	Cleanup();
	m_pMenu = pMenu;
}

void CSplitButton::Cleanup()
{
	if (m_pMenu)
	{
		delete m_pMenu;
	}
	m_pMenu = NULL;
	m_nMenuId = (UINT)-1;
	m_nSubMenuId = (UINT)-1;
}

void CSplitButton::OnDropDown(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	ASSERT(pResult != NULL);
	CMenu menu; // used only for loading menu from resource

	//use the pointer first
	CMenu* pMenu = m_pMenu; 
	// use the Menu IDs if pointer is NULL
	if (pMenu == NULL && m_nMenuId != (UINT)-1 && m_nSubMenuId != (UINT) -1)
	{
		menu.LoadMenu(m_nMenuId);
		pMenu = menu.GetSubMenu(m_nSubMenuId);
	}

	if (pMenu != NULL)
	{
		CRect rectButton;
		this->GetWindowRect(&rectButton);

		TPMPARAMS tpmParams;
		tpmParams.cbSize=sizeof(TPMPARAMS);
		tpmParams.rcExclude= rectButton;

		pMenu->TrackPopupMenuEx(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, rectButton.left, rectButton.bottom, this->GetParent(),&tpmParams);
	}
	*pResult = 0;
}
#endif // (_WIN32_WINNT >= 0x600) && defined(UNICODE)

/////////////////////////////////////////////////////////////////////////////
// CListBox

BOOL CListBox::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
		UINT nID)
{
	CWnd* pWnd = this;
	return pWnd->Create(_T("LISTBOX"), NULL, dwStyle, rect, pParentWnd, nID);
}

CListBox::~CListBox()
{
	DestroyWindow();
}

// Derived class is responsible for implementing these handlers
//   for owner/self draw controls (except for the optional DeleteItem)
void CListBox::DrawItem(LPDRAWITEMSTRUCT)
	{ ASSERT(FALSE); }
void CListBox::MeasureItem(LPMEASUREITEMSTRUCT)
	{ ASSERT(FALSE); }
int CListBox::CompareItem(LPCOMPAREITEMSTRUCT)
	{ ASSERT(FALSE); return 0; }
void CListBox::DeleteItem(LPDELETEITEMSTRUCT)
	{ /* default to nothing */ }
int CListBox::VKeyToItem(UINT, UINT)
//IA64: Assume item count < 2G
	{ return int(Default()); }
int CListBox::CharToItem(UINT, UINT)
//IA64: Assume item count < 2G
	{ return int(Default()); }

BOOL CListBox::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam,
	LRESULT* pResult)
{
	switch (message)
	{
	case WM_DRAWITEM:
		ASSERT(pResult == NULL);       // no return value expected
		DrawItem((LPDRAWITEMSTRUCT)lParam);
		break;
	case WM_MEASUREITEM:
		ASSERT(pResult == NULL);       // no return value expected
		MeasureItem((LPMEASUREITEMSTRUCT)lParam);
		break;
	case WM_COMPAREITEM:
		ASSERT(pResult != NULL);       // return value expected
		*pResult = CompareItem((LPCOMPAREITEMSTRUCT)lParam);
		break;
	case WM_DELETEITEM:
		ASSERT(pResult == NULL);       // no return value expected
		DeleteItem((LPDELETEITEMSTRUCT)lParam);
		break;
	case WM_VKEYTOITEM:
		*pResult = VKeyToItem(LOWORD(wParam), HIWORD(wParam));
		break;
	case WM_CHARTOITEM:
		*pResult = CharToItem(LOWORD(wParam), HIWORD(wParam));
		break;
	default:
		return CWnd::OnChildNotify(message, wParam, lParam, pResult);
	}
	return TRUE;
}

void CListBox::GetText(int nIndex, CString& rString) const
{
	ASSERT(::IsWindow(m_hWnd));
	GetText(nIndex, rString.GetBufferSetLength(GetTextLen(nIndex)));
	rString.ReleaseBuffer();
}

UINT CListBox::ItemFromPoint(CPoint pt, BOOL& bOutside) const
{
	ASSERT(::IsWindow(m_hWnd));
	DWORD dw = (DWORD)::SendMessage(m_hWnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y));
	bOutside = !!HIWORD(dw);
	return LOWORD(dw);
}

/////////////////////////////////////////////////////////////////////////////
// CComboBox

BOOL CComboBox::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
		UINT nID)
{
	CWnd* pWnd = this;
	return pWnd->Create(_T("COMBOBOX"), NULL, dwStyle, rect, pParentWnd, nID);
}

CComboBox::~CComboBox()
{
	DestroyWindow();
}

// Derived class is responsible for implementing these handlers
//   for owner/self draw controls (except for the optional DeleteItem)
void CComboBox::DrawItem(LPDRAWITEMSTRUCT)
	{ ASSERT(FALSE); }
void CComboBox::MeasureItem(LPMEASUREITEMSTRUCT)
	{ ASSERT(FALSE); }
int CComboBox::CompareItem(LPCOMPAREITEMSTRUCT)
	{ ASSERT(FALSE); return 0; }
void CComboBox::DeleteItem(LPDELETEITEMSTRUCT)
	{ /* default to nothing */ }

BOOL CComboBox::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam,
	LRESULT* pResult)
{
	switch (message)
	{
	case WM_DRAWITEM:
		ASSERT(pResult == NULL);       // no return value expected
		DrawItem((LPDRAWITEMSTRUCT)lParam);
		break;
	case WM_MEASUREITEM:
		ASSERT(pResult == NULL);       // no return value expected
		MeasureItem((LPMEASUREITEMSTRUCT)lParam);
		break;
	case WM_COMPAREITEM:
		ASSERT(pResult != NULL);       // return value expected
		*pResult = CompareItem((LPCOMPAREITEMSTRUCT)lParam);
		break;
	case WM_DELETEITEM:
		ASSERT(pResult == NULL);       // no return value expected
		DeleteItem((LPDELETEITEMSTRUCT)lParam);
		break;
	default:
		return CWnd::OnChildNotify(message, wParam, lParam, pResult);
	}
	return TRUE;
}

void CComboBox::GetLBText(int nIndex, CString& rString) const
{
	ASSERT(::IsWindow(m_hWnd));
	GetLBText(nIndex, rString.GetBufferSetLength(GetLBTextLen(nIndex)));
	rString.ReleaseBuffer();
}

/////////////////////////////////////////////////////////////////////////////
// CEdit

BOOL CEdit::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	CWnd* pWnd = this;
	return pWnd->Create(_T("EDIT"), NULL, dwStyle, rect, pParentWnd, nID);
}

CEdit::~CEdit()
{
	DestroyWindow();
}

/////////////////////////////////////////////////////////////////////////////
// CScrollBar

BOOL CScrollBar::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
		UINT nID)
{
	CWnd* pWnd = this;
	return pWnd->Create(_T("SCROLLBAR"), NULL, dwStyle, rect, pParentWnd, nID);
}

CScrollBar::~CScrollBar()
{
	DestroyWindow();
}


IMPLEMENT_DYNAMIC(CStatic, CWnd)
IMPLEMENT_DYNAMIC(CButton, CWnd)
#if (_WIN32_WINNT >= 0x600) && defined(UNICODE)
IMPLEMENT_DYNAMIC(CSplitButton, CButton)
#endif// (_WIN32_WINNT >= 0x600) && defined(UNICODE)
IMPLEMENT_DYNAMIC(CListBox, CWnd)
IMPLEMENT_DYNAMIC(CComboBox, CWnd)
IMPLEMENT_DYNAMIC(CEdit, CWnd)
IMPLEMENT_DYNAMIC(CScrollBar, CWnd)

/////////////////////////////////////////////////////////////////////////////
