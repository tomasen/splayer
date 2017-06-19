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
#include "sal.h"

/////////////////////////////////////////////////////////////////////////////
// CIPAddressCtrl


CIPAddressCtrl::~CIPAddressCtrl()
{
	DestroyWindow();
}

BOOL CIPAddressCtrl::Create(DWORD dwStyle, const RECT& rect,
	CWnd* pParentWnd, UINT nID)
{
	// initialize common controls
	VERIFY(AfxDeferRegisterClass(AFX_WNDCOMMCTL_INTERNET_REG));

	// the IP Address Control must be a child
	ASSERT(dwStyle & WS_CHILD);

	CWnd* pWnd = this;
	return pWnd->Create(WC_IPADDRESS, NULL, dwStyle, rect, pParentWnd, nID);
}

BOOL CIPAddressCtrl::CreateEx(DWORD dwExStyle, DWORD dwStyle,
	const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	BOOL bRet = Create(dwStyle, rect, pParentWnd, nID);
	if (bRet && dwExStyle != 0)
	{
		bRet = ModifyStyleEx(0, dwExStyle);
	}
	return bRet;
}

int CIPAddressCtrl::GetAddress(BYTE& nField0, BYTE& nField1, BYTE& nField2, BYTE& nField3) const
{
	ASSERT(::IsWindow(m_hWnd));
	DWORD dwAddress;
	LRESULT nRetVal = ::SendMessage(m_hWnd, IPM_GETADDRESS, 0, (LPARAM) &dwAddress);

	nField0 = (BYTE) FIRST_IPADDRESS(dwAddress);
	nField1 = (BYTE) SECOND_IPADDRESS(dwAddress);
	nField2 = (BYTE) THIRD_IPADDRESS(dwAddress);
	nField3 = (BYTE) FOURTH_IPADDRESS(dwAddress);

	//IA64: only four fields in an IP address, so an int won't overflow
	return int(nRetVal);
}

/////////////////////////////////////////////////////////////////////////////
// CComboBoxEx


BOOL CComboBoxEx::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
	UINT nID)
{
	// initialize common controls
	VERIFY(AfxDeferRegisterClass(AFX_WNDCOMMCTL_USEREX_REG));

	CWnd* pWnd = this;
	return pWnd->Create(WC_COMBOBOXEX, NULL, dwStyle, rect, pParentWnd, nID);
}

BOOL CComboBoxEx::CreateEx(DWORD dwExStyle, DWORD dwStyle, const RECT& rect,
	CWnd* pParentWnd, UINT nID)
{
	BOOL bRet = Create(dwStyle, rect, pParentWnd, nID);
	if (bRet && dwExStyle != 0)
	{
		bRet = ModifyStyleEx(0, dwExStyle);
	}
	return bRet;
}

int CComboBoxEx::DeleteItem(int iIndex)
{
	ASSERT(::IsWindow(m_hWnd));
	return (int) ::SendMessage(m_hWnd, CBEM_DELETEITEM, (WPARAM) iIndex, 0);
}

BOOL CComboBoxEx::GetItem(COMBOBOXEXITEM* pCBItem)
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT(pCBItem != NULL);
	ASSERT(AfxIsValidAddress(pCBItem, sizeof(COMBOBOXEXITEM)));

	return (int) ::SendMessage(m_hWnd, CBEM_GETITEM, 0, (LPARAM) pCBItem);
}

int CComboBoxEx::InsertItem(const COMBOBOXEXITEM* pCBItem)
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT(pCBItem != NULL);
	ASSERT(AfxIsValidAddress(pCBItem, sizeof(COMBOBOXEXITEM), FALSE));

	return (int) ::SendMessage(m_hWnd, CBEM_INSERTITEM, 0, (LPARAM) pCBItem);
}

BOOL CComboBoxEx::SetItem(const COMBOBOXEXITEM* pCBItem)
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT(pCBItem != NULL);
	ASSERT(AfxIsValidAddress(pCBItem, sizeof(COMBOBOXEXITEM), FALSE));

	return (int) ::SendMessage(m_hWnd, CBEM_SETITEM, 0, (LPARAM) pCBItem);
}

CComboBoxEx::~CComboBoxEx()
{
	DestroyWindow();
}

/////////////////////////////////////////////////////////////////////////////
// CReBarCtrl

CReBarCtrl::~CReBarCtrl()
{
	DestroyWindow();
}

BOOL CReBarCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
	UINT nID)
{
	// initialize common controls
	VERIFY(AfxDeferRegisterClass(AFX_WNDCOMMCTL_COOL_REG));

	CWnd* pWnd = this;
	return pWnd->Create(REBARCLASSNAME, NULL, dwStyle, rect, pParentWnd, nID);
}

BOOL CReBarCtrl::CreateEx(DWORD dwExStyle, DWORD dwStyle, const RECT& rect,
	CWnd* pParentWnd, UINT nID)
{
	BOOL bRet = Create(dwStyle, rect, pParentWnd, nID);
	if (bRet && dwExStyle != 0)
	{
		bRet = ModifyStyleEx(0, dwExStyle);
	}
	return bRet;
}

CImageList* CReBarCtrl::GetImageList() const
{
	REBARINFO rbi;
	rbi.cbSize = sizeof(rbi);
	rbi.fMask = RBIM_IMAGELIST;
	return GetBarInfo(&rbi) ? CImageList::FromHandle(rbi.himl) : NULL;
}

BOOL CReBarCtrl::SetImageList(CImageList* pImageList)
{
	REBARINFO rbi;
	rbi.cbSize = sizeof(rbi);
	rbi.fMask = RBIM_IMAGELIST;
	rbi.himl = (HIMAGELIST)pImageList->GetSafeHandle();
	return SetBarInfo(&rbi);
}

BOOL CReBarCtrl::GetColorScheme(COLORSCHEME* lpcs) const
{
	//IA64: sizeof(COLORSCHEME) should be way less than 4GB, so no overflow
	lpcs->dwSize = ULONG(sizeof(COLORSCHEME));
	//IA64: Returns non-zero if successful, so can't just truncate to 32 bits
	return (::SendMessage(m_hWnd, RB_GETCOLORSCHEME, 0, (LPARAM)lpcs) != 0);
}

void CReBarCtrl::SetColorScheme(const COLORSCHEME* lpcs)
{
	((COLORSCHEME*)lpcs)->dwSize = sizeof(COLORSCHEME);
	SendMessage(RB_SETCOLORSCHEME, 0, (LPARAM)lpcs);
}

/////////////////////////////////////////////////////////////////////////////
// CListCtrl

BOOL CListCtrl::SetColumnOrderArray(int iCount, LPINT piArray)
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT(AfxIsValidAddress(piArray, iCount * sizeof(int), FALSE));

	return (BOOL) ::SendMessage(m_hWnd, LVM_SETCOLUMNORDERARRAY,
					(WPARAM) iCount, (LPARAM) piArray);
}

BOOL CListCtrl::GetColumnOrderArray(LPINT piArray, int iCount /* = -1 */) const
{
	ASSERT(::IsWindow(m_hWnd));

	// if -1 was passed, find the count ourselves

	int nCount = iCount;
	if (nCount == -1)
	{
		CHeaderCtrl* pCtrl = GetHeaderCtrl();
		ASSERT(pCtrl != NULL);
		if (pCtrl != NULL)
			nCount = pCtrl->GetItemCount();
	}
	if (nCount == -1)
		return FALSE;

	ASSERT(AfxIsValidAddress(piArray, nCount * sizeof(int)));
	return (BOOL) ::SendMessage(m_hWnd, LVM_GETCOLUMNORDERARRAY,
		(WPARAM) nCount, (LPARAM) piArray);
}

BOOL CListCtrl::SetBkImage(HBITMAP hbm, BOOL fTile /*= TRUE*/, int xOffsetPercent /*= 0*/, int yOffsetPercent /*= 0*/)
{
	LVBKIMAGE lv;

	lv.ulFlags = LVBKIF_SOURCE_HBITMAP;
	if( fTile )
		lv.ulFlags |= LVBKIF_STYLE_TILE;
	else
		lv.ulFlags |= LVBKIF_STYLE_NORMAL;
	lv.hbm = hbm;
	lv.xOffsetPercent = xOffsetPercent;
	lv.yOffsetPercent = yOffsetPercent;
	return SetBkImage(&lv);
}

BOOL CListCtrl::SetBkImage(_In_z_ LPTSTR pszUrl, _In_ BOOL fTile /*= TRUE*/, _In_ int xOffsetPercent /*= 0*/, _In_ int yOffsetPercent /*= 0*/)
{
	LVBKIMAGE lv;

	lv.ulFlags = LVBKIF_SOURCE_URL;
	if( fTile )
		lv.ulFlags |= LVBKIF_STYLE_TILE;
	else
		lv.ulFlags |= LVBKIF_STYLE_NORMAL;
	lv.pszImage = pszUrl;
	lv.xOffsetPercent = xOffsetPercent;
	lv.yOffsetPercent = yOffsetPercent;
	return SetBkImage(&lv);
}

BOOL CListCtrl::GetCheck(int nItem) const
{
	ASSERT(::IsWindow(m_hWnd));
	int nState = (int)::SendMessage(m_hWnd, LVM_GETITEMSTATE, (WPARAM)nItem,
		(LPARAM)LVIS_STATEIMAGEMASK);
	// Return zero if it's not checked, or nonzero otherwise.
	return ((BOOL)(nState >> 12) -1);
}

BOOL CListCtrl::SetCheck(int nItem, BOOL fCheck)
{
	ASSERT(::IsWindow(m_hWnd));
	LVITEM lvi;
	lvi.stateMask = LVIS_STATEIMAGEMASK;

	/*
	Since state images are one-based, 1 in this macro turns the check off, and
	2 turns it on.
	*/
	lvi.state = INDEXTOSTATEIMAGEMASK((fCheck ? 2 : 1));
	return (BOOL) ::SendMessage(m_hWnd, LVM_SETITEMSTATE, nItem, (LPARAM)&lvi);
}

/////////////////////////////////////////////////////////////////////////////
// CHeaderCtrl

CHeaderCtrl* CListCtrl::GetHeaderCtrl() const
{
	ASSERT(::IsWindow(m_hWnd));

	HWND hWnd = (HWND) ::SendMessage(m_hWnd, LVM_GETHEADER, 0, 0);
	if (hWnd == NULL)
		return NULL;
	else
		return (CHeaderCtrl*) CHeaderCtrl::FromHandle(hWnd);
}


BOOL CHeaderCtrl::SetOrderArray(int iCount, LPINT piArray)
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT(AfxIsValidAddress(piArray, iCount * sizeof(int), FALSE));

	return (BOOL) ::SendMessage(m_hWnd, HDM_SETORDERARRAY,
					(WPARAM) iCount, (LPARAM) piArray);
}

BOOL CHeaderCtrl::GetOrderArray(LPINT piArray, int iCount) const
{
	ASSERT(::IsWindow(m_hWnd));

	// This method no longer has a default value for iCount
	// if -1 was passed, we ASSERT & fail

	int nCount = iCount;
	if (nCount < 0)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	ASSERT(AfxIsValidAddress(piArray, iCount * sizeof(int)));

	return (BOOL) ::SendMessage(m_hWnd, HDM_GETORDERARRAY,
		(WPARAM) nCount, (LPARAM) piArray);
}

/////////////////////////////////////////////////////////////////////////////
// CProgressCtrl

void CProgressCtrl::GetRange(int& nLower, int& nUpper) const
{
	ASSERT(::IsWindow(m_hWnd));
	PBRANGE range;
	::SendMessage(m_hWnd, PBM_GETRANGE, (WPARAM) FALSE, (LPARAM) &range);
	nLower = range.iLow;
	nUpper = range.iHigh;
	return;
}

/////////////////////////////////////////////////////////////////////////////
// CPagerCtrl

BOOL CPagerCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
	UINT nID)
{
	// initialize common controls
	ENSURE(AfxDeferRegisterClass(AFX_WNDCOMMCTL_PAGER_REG));

	CWnd* pWnd = this;
	return pWnd->Create(WC_PAGESCROLLER, NULL, dwStyle, rect, pParentWnd, nID);
}

BOOL CPagerCtrl::CreateEx(DWORD dwExStyle, DWORD dwStyle, const RECT& rect,
	CWnd* pParentWnd, UINT nID)
{
	BOOL bRet = Create(dwStyle, rect, pParentWnd, nID);
	if (bRet && dwExStyle != 0)
	{
		bRet = ModifyStyleEx(0, dwExStyle);
	}
	return bRet;
}

#if (_WIN32_WINNT >= 0x0501)

/////////////////////////////////////////////////////////////////////////////
// CLinkCtrl

BOOL CLinkCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
	UINT nID)
{
	// initialize common controls
	ENSURE(AfxDeferRegisterClass(AFX_WNDCOMMCTL_LINK_REG));

#ifndef _UNICODE
	ASSERT(0);
	dwStyle;
	rect;
	pParentWnd;
	nID;
	// Can only be used in UNICODE builds
	return FALSE;
#else
	CWnd* pWnd = this;
	return pWnd->Create(WC_LINK, NULL, dwStyle, rect, pParentWnd, nID);
#endif	// _UNICODE
}

BOOL CLinkCtrl::Create(LPCTSTR lpszLinkMarkup, DWORD dwStyle, const RECT& rect,
	CWnd* pParentWnd, UINT nID)
{
	// initialize common controls
	ENSURE(AfxDeferRegisterClass(AFX_WNDCOMMCTL_LINK_REG));

#ifndef _UNICODE
	ASSERT(0);
	lpszLinkMarkup;
	dwStyle;
	rect;
	pParentWnd;
	nID;
	// Can only be used in UNICODE builds
	return FALSE;
#else
	CWnd* pWnd = this;
	return pWnd->Create(WC_LINK, lpszLinkMarkup, dwStyle, rect, pParentWnd, nID);
#endif	// _UNICODE
}

BOOL CLinkCtrl::CreateEx(DWORD dwExStyle, DWORD dwStyle, const RECT& rect,
	CWnd* pParentWnd, UINT nID)
{
	BOOL bRet = Create(dwStyle, rect, pParentWnd, nID);
	if (bRet && dwExStyle != 0)
	{
		bRet = ModifyStyleEx(0, dwExStyle);
	}
	return bRet;
}

BOOL CLinkCtrl::CreateEx(LPCTSTR lpszLinkMarkup, DWORD dwExStyle, DWORD dwStyle,
	const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	BOOL bRet = Create(lpszLinkMarkup, dwStyle, rect, pParentWnd, nID);
	if (bRet && dwExStyle != 0)
	{
		bRet = ModifyStyleEx(0, dwExStyle);
	}
	return bRet;
}

#endif

#if (NTDDI_VERSION >= NTDDI_LONGHORN) && defined(UNICODE)

/////////////////////////////////////////////////////////////////////////////
// CNetAddressCtrl

BOOL CNetAddressCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
	UINT nID)
{
#ifndef _UNICODE
	ASSERT(0);
	dwStyle;
	rect;
	pParentWnd;
	nID;
	// Can only be used in UNICODE builds
	return FALSE;
#else
	// initialize common controls
	ENSURE(AfxInitNetworkAddressControl());

	CWnd* pWnd = this;
	return pWnd->Create(WC_NETADDRESS, NULL, dwStyle, rect, pParentWnd, nID);
#endif	// _UNICODE
}

BOOL CNetAddressCtrl::CreateEx(DWORD dwExStyle, DWORD dwStyle, const RECT& rect,
	CWnd* pParentWnd, UINT nID)
{
	BOOL bRet = Create(dwStyle, rect, pParentWnd, nID);
	if (bRet && dwExStyle != 0)
	{
		bRet = ModifyStyleEx(0, dwExStyle);
	}
	return bRet;
}

#endif // (NTDDI_VERSION >= NTDDI_LONGHORN) && defined(UNICODE)

/////////////////////////////////////////////////////////////////////////////

#ifndef _AFX_ENABLE_INLINES

#define _AFXCMN_INLINE
#include "afxcmn2.inl"

#endif //_AFX_ENABLE_INLINES

/////////////////////////////////////////////////////////////////////////////


IMPLEMENT_DYNAMIC(CIPAddressCtrl, CWnd)
IMPLEMENT_DYNAMIC(CReBarCtrl, CWnd)
IMPLEMENT_DYNAMIC(CPagerCtrl, CWnd)

#if (_WIN32_WINNT >= 0x0501)
IMPLEMENT_DYNAMIC(CLinkCtrl, CWnd)
#endif	// _WIN32_WINNT >= 0x0501

#if (NTDDI_VERSION >= NTDDI_LONGHORN) && defined(UNICODE)
IMPLEMENT_DYNAMIC(CNetAddressCtrl, CEdit)
#endif // (NTDDI_VERSION >= NTDDI_LONGHORN) && defined(UNICODE)


/////////////////////////////////////////////////////////////////////////////

