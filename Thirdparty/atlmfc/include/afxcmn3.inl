// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// Inlines for AFXCMN.H
// Contains wrappers for API's messages not in all supported
// platforms

#if (_WIN32_WINNT >= 0x0501)

#include <atlchecked.h>

AFX_INLINE HRESULT CToolTipCtrl::SetWindowTheme(LPCWSTR pszSubAppName)
	{ ASSERT(::IsWindow(m_hWnd)); return (HRESULT)::SendMessage(m_hWnd, TTM_SETWINDOWTHEME, 0, (LPARAM)pszSubAppName); }

AFX_INLINE HRESULT CComboBoxEx::SetWindowTheme(LPCWSTR pszSubAppName)
	{ ASSERT(::IsWindow(m_hWnd)); return (HRESULT)::SendMessage(m_hWnd, CBEM_SETWINDOWTHEME, 0, (LPARAM)pszSubAppName); }

AFX_INLINE void CReBarCtrl::GetBandMargins(PMARGINS pMargins) const
	{ ASSERT(::IsWindow(m_hWnd)); ::SendMessage(m_hWnd, RB_GETBANDMARGINS, 0, (LPARAM)pMargins); }

AFX_INLINE HRESULT CReBarCtrl::SetWindowTheme(LPCWSTR pszSubAppName)
	{ ASSERT(::IsWindow(m_hWnd)); return (HRESULT)::SendMessage(m_hWnd, RB_SETWINDOWTHEME, 0, (LPARAM)pszSubAppName); }

AFX_INLINE void CToolBarCtrl::GetMetrics(LPTBMETRICS ptbm) const
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT(ptbm != NULL);
	::SendMessage(m_hWnd, TB_GETMETRICS, 0, (LPARAM)ptbm);
}

AFX_INLINE void CToolBarCtrl::SetMetrics(LPTBMETRICS ptbm)
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT(ptbm != NULL);
	::SendMessage(m_hWnd, TB_SETMETRICS, 0, (LPARAM)ptbm);
}

AFX_INLINE HRESULT CToolBarCtrl::SetWindowTheme(LPCWSTR pszSubAppName)
	{ ASSERT(::IsWindow(m_hWnd)); return (HRESULT)::SendMessage(m_hWnd, TB_SETWINDOWTHEME, 0, (LPARAM)pszSubAppName); }

#endif	// _WIN32_WINNT >= 0x0501

#if (_WIN32_WINNT >= 0x600) && defined(UNICODE)

AFX_INLINE CImageList* CToolBarCtrl::SetPressedImageList(int iImageID, CImageList* pImageList)
	{ ASSERT(::IsWindow(m_hWnd)); return CImageList::FromHandle((HIMAGELIST) ::SendMessage(m_hWnd, TB_SETPRESSEDIMAGELIST, (WPARAM)iImageID, (LPARAM)pImageList->GetSafeHandle())); }

AFX_INLINE CImageList* CToolBarCtrl::GetPressedImageList() const
{ ASSERT(::IsWindow(m_hWnd)); return CImageList::FromHandle((HIMAGELIST) ::SendMessage(m_hWnd, TB_GETPRESSEDIMAGELIST, 0, 0L)); }

#endif // _WIN32_WINNT >= 0x600 && defined(UNICODE)

#if (_WIN32_WINNT >= 0x0501)

AFX_INLINE void CListCtrl::SetSelectedColumn(int iCol)
{
	ASSERT(::IsWindow(m_hWnd));
	ListView_SetSelectedColumn(m_hWnd, iCol); // LVM_SETSELECTEDCOLUMN
}
AFX_INLINE DWORD CListCtrl::SetView(int iView)
{
	ASSERT(::IsWindow(m_hWnd));
	return ListView_SetView(m_hWnd, iView); // LVM_SETVIEW
}
AFX_INLINE DWORD CListCtrl::GetView() const
{
	ASSERT(::IsWindow(m_hWnd));
	return ListView_GetView(m_hWnd); // LVM_GETVIEW
}
AFX_INLINE int CListCtrl::InsertGroup(int index, PLVGROUP pgrp)
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)ListView_InsertGroup(m_hWnd, index, pgrp); // LVM_INSERTGROUP
}
AFX_INLINE int CListCtrl::SetGroupInfo(int iGroupId, PLVGROUP pGroup)
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)ListView_SetGroupInfo(m_hWnd, iGroupId, pGroup); // LVM_SETGROUPINFO
}
AFX_INLINE int CListCtrl::GetGroupInfo(int iGroupId, PLVGROUP pgrp) const
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)ListView_GetGroupInfo(m_hWnd, iGroupId, pgrp); // LVM_GETGROUPINFO
}
AFX_INLINE int CListCtrl::RemoveGroup(int iGroupId)
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)ListView_RemoveGroup(m_hWnd, iGroupId); // LVM_REMOVEGROUP
}
AFX_INLINE void CListCtrl::MoveGroup(int iGroupId, int toIndex)
{
	ASSERT(::IsWindow(m_hWnd));
	ListView_MoveGroup(m_hWnd, iGroupId, toIndex); // LVM_MOVEGROUP
}
AFX_INLINE void CListCtrl::MoveItemToGroup(int idItemFrom, int idGroupTo)
{
	ASSERT(::IsWindow(m_hWnd));
	ListView_MoveItemToGroup(m_hWnd, idItemFrom, idGroupTo); // LVM_MOVEITEMTOGROUP
}
AFX_INLINE void CListCtrl::SetGroupMetrics(PLVGROUPMETRICS pGroupMetrics)
{
	ASSERT(::IsWindow(m_hWnd));
	ListView_SetGroupMetrics(m_hWnd, pGroupMetrics); // LVM_SETGROUPMETRICS
}
AFX_INLINE void CListCtrl::GetGroupMetrics(PLVGROUPMETRICS pGroupMetrics) const
{
	ASSERT(::IsWindow(m_hWnd));
	ListView_GetGroupMetrics(m_hWnd, pGroupMetrics); // LVM_GETGROUPMETRICS
}
AFX_INLINE int CListCtrl::EnableGroupView(BOOL fEnable)
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)ListView_EnableGroupView(m_hWnd, fEnable); // LVM_ENABLEGROUPVIEW
}
AFX_INLINE BOOL CListCtrl::SortGroups(PFNLVGROUPCOMPARE _pfnGroupCompare, LPVOID _plv)
{
	ASSERT(::IsWindow(m_hWnd));
	return (BOOL)ListView_SortGroups(m_hWnd, _pfnGroupCompare, _plv); // LVM_SORTGROUPS
}
AFX_INLINE void CListCtrl::InsertGroupSorted(PLVINSERTGROUPSORTED pStructInsert)
{
	ASSERT(::IsWindow(m_hWnd));
	ListView_InsertGroupSorted(m_hWnd, pStructInsert); // LVM_INSERTGROUPSORTED
}
AFX_INLINE void CListCtrl::RemoveAllGroups()
{
	ASSERT(::IsWindow(m_hWnd));
	ListView_RemoveAllGroups(m_hWnd); // LVM_REMOVEALLGROUPS
}
AFX_INLINE BOOL CListCtrl::HasGroup(int iGroupId) const
{
	ASSERT(::IsWindow(m_hWnd));
	return (BOOL)ListView_HasGroup(m_hWnd, iGroupId); // LVM_HASGROUP
}
AFX_INLINE BOOL CListCtrl::SetTileViewInfo(PLVTILEVIEWINFO ptvi)
{
	ASSERT(::IsWindow(m_hWnd));
	return (BOOL)ListView_SetTileViewInfo(m_hWnd, ptvi); // LVM_SETTILEVIEWINFO
}
AFX_INLINE BOOL CListCtrl::GetTileViewInfo(PLVTILEVIEWINFO ptvi) const
{
	ASSERT(::IsWindow(m_hWnd));
	return (BOOL)ListView_GetTileViewInfo(m_hWnd, ptvi); // LVM_GETTILEVIEWINFO
}
AFX_INLINE BOOL CListCtrl::SetTileInfo(PLVTILEINFO pti)
{
	ASSERT(::IsWindow(m_hWnd));
	return (BOOL)ListView_SetTileInfo(m_hWnd, pti); // LVM_SETTILEINFO
}
AFX_INLINE BOOL CListCtrl::GetTileInfo(PLVTILEINFO pti) const
{
	ASSERT(::IsWindow(m_hWnd));
	return (BOOL)ListView_GetTileInfo(m_hWnd, pti); // LVM_GETTILEINFO
}
AFX_INLINE BOOL CListCtrl::SetInsertMark(LPLVINSERTMARK lvim)
{
	ASSERT(::IsWindow(m_hWnd));
	return (BOOL)ListView_SetInsertMark(m_hWnd, lvim); // LVM_SETINSERTMARK
}
AFX_INLINE BOOL CListCtrl::GetInsertMark(LPLVINSERTMARK lvim) const
{
	ASSERT(::IsWindow(m_hWnd));
	return (BOOL)ListView_GetInsertMark(m_hWnd, lvim); // LVM_GETINSERTMARK
}
AFX_INLINE int CListCtrl::InsertMarkHitTest(LPPOINT pPoint, LPLVINSERTMARK lvim) const
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)ListView_InsertMarkHitTest(m_hWnd, pPoint, lvim); // LVM_INSERTMARKHITTEST
}
AFX_INLINE int CListCtrl::GetInsertMarkRect(LPRECT pRect) const
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)ListView_GetInsertMarkRect(m_hWnd, pRect); // LVM_GETINSERTMARKRECT
}
AFX_INLINE COLORREF CListCtrl::SetInsertMarkColor(COLORREF color)
{
	ASSERT(::IsWindow(m_hWnd));
	return (COLORREF)ListView_SetInsertMarkColor(m_hWnd, color); // LVM_SETINSERTMARKCOLOR
}
AFX_INLINE COLORREF CListCtrl::GetInsertMarkColor() const
{
	ASSERT(::IsWindow(m_hWnd));
	return ListView_GetInsertMarkColor(m_hWnd); // LVM_GETINSERTMARKCOLOR
}
AFX_INLINE BOOL CListCtrl::SetInfoTip(PLVSETINFOTIP plvInfoTip)
{
	ASSERT(::IsWindow(m_hWnd));
	return ListView_SetInfoTip(m_hWnd, plvInfoTip); // LVM_SETINFOTIP
}
AFX_INLINE UINT CListCtrl::GetSelectedColumn() const
{
	ASSERT(::IsWindow(m_hWnd));
	return ListView_GetSelectedColumn(m_hWnd); // LVM_GETSELECTEDCOLUMN
}
AFX_INLINE BOOL CListCtrl::IsGroupViewEnabled() const
{
	ASSERT(::IsWindow(m_hWnd));
	return ListView_IsGroupViewEnabled(m_hWnd); // LVM_ISGROUPVIEWENABLED
}
AFX_INLINE COLORREF CListCtrl::GetOutlineColor() const
{
	ASSERT(::IsWindow(m_hWnd));
	return ListView_GetOutlineColor(m_hWnd); // LVM_GETOUTLINECOLOR
}
AFX_INLINE COLORREF CListCtrl::SetOutlineColor(COLORREF color)
{
	ASSERT(::IsWindow(m_hWnd));
	return ListView_SetOutlineColor(m_hWnd, color); // LVM_SETOUTLINECOLOR
}
AFX_INLINE void CListCtrl::CancelEditLabel()
{
	ASSERT(::IsWindow(m_hWnd));
	ListView_CancelEditLabel(m_hWnd); // LVM_CANCELEDITLABEL
}

#endif	// _WIN32_WINNT >= 0x0501

#if (_WIN32_WINNT >= 0x0600) && defined(UNICODE)

AFX_INLINE CString CListCtrl::GetEmptyText() const
{
	ASSERT(::IsWindow(m_hWnd));
	LPTSTR lpszText=NULL;
	int cchText = 0;
	CString strOut("");
	BOOL fRet = CWnd::EnlargeBufferGetText<BOOL>(FALSE, lpszText, cchText, LVM_GETEMPTYTEXT, (WPARAM &)cchText, (LPARAM &)lpszText, strOut);
	if (fRet)
		return strOut;
	else
		return CString("");
}
AFX_INLINE BOOL CListCtrl::GetItemIndexRect(PLVITEMINDEX pItemIndex, int iColumn, int rectType, LPRECT pRect) const
{
	ASSERT(::IsWindow(m_hWnd));
	return ListView_GetItemIndexRect(m_hWnd, pItemIndex, iColumn, rectType, pRect); // LVM_GETITEMINDEXRECT
}
AFX_INLINE HRESULT CListCtrl::SetItemIndexState(PLVITEMINDEX pItemIndex, DWORD dwState, DWORD dwMask)
{
	ASSERT(::IsWindow(m_hWnd));
	LV_ITEM lvi = {0};
	lvi.stateMask = dwMask;
	lvi.state = dwState;
	return (HRESULT)SNDMSG(m_hWnd, LVM_SETITEMINDEXSTATE, (WPARAM)pItemIndex, (LPARAM)&lvi);
}
AFX_INLINE BOOL CListCtrl::GetNextItemIndex(PLVITEMINDEX pItemIndex, int nFlags) const
{
	ASSERT(::IsWindow(m_hWnd));
	return ListView_GetNextItemIndex(m_hWnd, pItemIndex, nFlags); // LVM_GETNEXTITEMINDEX
}
AFX_INLINE int CListCtrl::GetGroupCount() const
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)ListView_GetGroupCount(m_hWnd); // LVM_GETGROUPCOUNT
}
AFX_INLINE BOOL CListCtrl::GetGroupInfoByIndex(int iIndex, PLVGROUP pGroup) const
{
	ASSERT(::IsWindow(m_hWnd));
	return (BOOL)ListView_GetGroupInfoByIndex(m_hWnd, iIndex, pGroup); // LVM_GETGROUPINFOBYINDEX
}
AFX_INLINE void CListCtrl::SetGroupState(int iGroupId, DWORD dwMask, DWORD dwState)
{
	ASSERT(::IsWindow(m_hWnd));
	ListView_SetGroupState(m_hWnd, iGroupId, dwMask, dwState); // LVM_SETGROUPINFO
}
AFX_INLINE UINT CListCtrl::GetGroupState(int iGroupId, DWORD dwMask) const
{
	ASSERT(::IsWindow(m_hWnd));
	return ListView_GetGroupState(m_hWnd, iGroupId, dwMask); // LVM_GETGROUPSTATE
}
AFX_INLINE int CListCtrl::GetFocusedGroup() const
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)ListView_GetFocusedGroup(m_hWnd); // LVM_GETFOCUSEDGROUP
}
AFX_INLINE BOOL CListCtrl::GetGroupRect(_In_ int iGroupId, _Out_ LPRECT lpRect, _In_ int iType /* = LVGGR_GROUP */) const
{
	ASSERT(::IsWindow(m_hWnd));
	return (BOOL)ListView_GetGroupRect(m_hWnd, iGroupId, iType, lpRect); // LVM_GETGROUPRECT
}
AFX_INLINE UINT CListCtrl::MapIndexToID(UINT index) const
{
	ASSERT(::IsWindow(m_hWnd));
	return ListView_MapIndexToID(m_hWnd, index); // LVM_MAPINDEXTOID
}
AFX_INLINE UINT CListCtrl::MapIDToIndex(UINT id) const
{
	ASSERT(::IsWindow(m_hWnd));
	return ListView_MapIDToIndex(m_hWnd, id); // LVM_MAPIDTOINDEX
}
AFX_INLINE BOOL CListCtrl::IsItemVisible(int index) const
{
	ASSERT(::IsWindow(m_hWnd));
	return (BOOL)ListView_IsItemVisible(m_hWnd, index); // LVM_ISITEMVISIBLE
}
#endif	// _WIN32_WINNT >= 0x0600 && defined(UNICODE)

/////////////////////////////////////////////////////////////////////////////
// CPagerCtrl

AFX_INLINE CPagerCtrl::CPagerCtrl()
{ }

AFX_INLINE CPagerCtrl::~CPagerCtrl()
{
	DestroyWindow();
}

// Attributes
AFX_INLINE void CPagerCtrl::SetChild(HWND hwndChild)
{
	ASSERT(::IsWindow(m_hWnd));
	return Pager_SetChild(m_hWnd, hwndChild); // PGM_SETCHILD
}

AFX_INLINE COLORREF CPagerCtrl::SetBkColor(COLORREF clrBk)
{
	ASSERT(::IsWindow(m_hWnd));
	return Pager_SetBkColor(m_hWnd, clrBk); // PGM_SETBKCOLOR
}

AFX_INLINE COLORREF CPagerCtrl::GetBkColor() const
{
	ASSERT(::IsWindow(m_hWnd));
	return Pager_GetBkColor(m_hWnd); // PGM_GETBKCOLOR
}

AFX_INLINE int CPagerCtrl::SetBorder(int iBorder)
{
	ASSERT(::IsWindow(m_hWnd));
	return Pager_SetBorder(m_hWnd, iBorder); // PGM_SETBORDER
}

AFX_INLINE int CPagerCtrl::GetBorder() const
{
	ASSERT(::IsWindow(m_hWnd));
	return Pager_GetBorder(m_hWnd); // PGM_GETBORDER
}

AFX_INLINE void CPagerCtrl::SetScrollPos(int iPos)
{
	ASSERT(::IsWindow(m_hWnd));
	Pager_SetPos(m_hWnd, iPos); // PGM_SETPOS
}

AFX_INLINE int CPagerCtrl::GetScrollPos() const
{
	ASSERT(::IsWindow(m_hWnd));
	return Pager_GetPos(m_hWnd); // PGM_GETPOS
}

AFX_INLINE int CPagerCtrl::SetButtonSize(int iButtonSize)
{
	ASSERT(::IsWindow(m_hWnd));
	return Pager_SetButtonSize(m_hWnd, iButtonSize); // PGM_SETBUTTONSIZE
}

AFX_INLINE int CPagerCtrl::GetButtonSize() const
{
	ASSERT(::IsWindow(m_hWnd));
	return Pager_GetButtonSize(m_hWnd); // PGM_GETBUTTONSIZE
}

AFX_INLINE BOOL CPagerCtrl::IsButtonInvisible(int iButton) const
{
	ASSERT(::IsWindow(m_hWnd));
	return(GetButtonState(iButton) == PGF_INVISIBLE);
}

AFX_INLINE BOOL CPagerCtrl::IsButtonNormal(int iButton) const
{
	ASSERT(::IsWindow(m_hWnd));
	return((GetButtonState(iButton) & PGF_NORMAL) == PGF_NORMAL);
}

AFX_INLINE BOOL CPagerCtrl::IsButtonGrayed(int iButton) const
{
	ASSERT(::IsWindow(m_hWnd));
	return((GetButtonState(iButton) & PGF_GRAYED) == PGF_GRAYED);
}

AFX_INLINE BOOL CPagerCtrl::IsButtonDepressed(int iButton) const
{
	ASSERT(::IsWindow(m_hWnd));
	return((GetButtonState(iButton) & PGF_DEPRESSED) == PGF_DEPRESSED);
}

AFX_INLINE BOOL CPagerCtrl::IsButtonHot(int iButton) const
{
	ASSERT(::IsWindow(m_hWnd));
	return((GetButtonState(iButton) & PGF_HOT) == PGF_HOT);
}

AFX_INLINE DWORD CPagerCtrl::GetButtonState(int iButton) const
{
	ASSERT(::IsWindow(m_hWnd));
	return Pager_GetButtonState(m_hWnd, iButton); // PGM_GETBUTTONSTATE
}

AFX_INLINE IDropTarget* CPagerCtrl::GetDropTarget() const
{
	ASSERT(::IsWindow(m_hWnd));
	IDropTarget* pdt = NULL;
	Pager_GetDropTarget(m_hWnd, &pdt); // PGM_GETDROPTARGET
	return pdt;
}

// Operations
AFX_INLINE void CPagerCtrl::RecalcSize()
{
	ASSERT(::IsWindow(m_hWnd));
	return Pager_RecalcSize(m_hWnd); // PGM_RECALCSIZE
}

AFX_INLINE void CPagerCtrl::ForwardMouse(BOOL bForward)
{
	ASSERT(::IsWindow(m_hWnd));
	return Pager_ForwardMouse(m_hWnd, bForward); // PGM_FORWARDMOUSE
}

#if (_WIN32_WINNT >= 0x0501)

/////////////////////////////////////////////////////////////////////////////
// CLinkCtrl

AFX_INLINE CLinkCtrl::CLinkCtrl()
{ }

AFX_INLINE CLinkCtrl::~CLinkCtrl()
{
	DestroyWindow();
}

// Attributes
AFX_INLINE int CLinkCtrl::GetIdealHeight() const
{
	ASSERT(::IsWindow(m_hWnd));
	return (int) ::SendMessage(m_hWnd, LM_GETIDEALHEIGHT, 0, 0);
}

#if (_WIN32_WINNT >= 0x0600) && defined(UNICODE)
AFX_INLINE int CLinkCtrl::GetIdealSize(int cxMaxWidth, SIZE* pSize) const
{
	ASSERT(::IsWindow(m_hWnd));
	return (int) ::SendMessage(m_hWnd, LM_GETIDEALSIZE, (WPARAM)cxMaxWidth, (LPARAM)pSize);
}
#endif // (_WIN32_WINNT >= 0x0600) && defined(UNICODE)

// operations
AFX_INLINE BOOL CLinkCtrl::SetItem(PLITEM pItem)
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT(pItem != NULL);
	return (BOOL) ::SendMessage(m_hWnd, LM_SETITEM, 0, (LPARAM)pItem);
}

AFX_INLINE BOOL CLinkCtrl::SetItemID(int iLink, LPCWSTR szID)
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT(lstrlenW(szID) <= MAX_LINKID_TEXT);
	if (lstrlenW(szID) > MAX_LINKID_TEXT)
		return FALSE;

	LITEM item;
	item.mask = LIF_ITEMINDEX | LIF_ITEMID;
	item.iLink = iLink;
	::ATL::Checked::wcscpy_s(item.szID, _countof(item.szID), szID);

	return SetItem(&item);
}

AFX_INLINE BOOL CLinkCtrl::SetItemUrl(int iLink, LPCWSTR szUrl)
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT(lstrlenW(szUrl) <= L_MAX_URL_LENGTH);
	if (lstrlenW(szUrl) >= L_MAX_URL_LENGTH)
		return FALSE;

	LITEM item;
	item.mask = LIF_ITEMINDEX | LIF_URL;
	item.iLink = iLink;
	::ATL::Checked::wcscpy_s(item.szUrl, _countof(item.szUrl), szUrl);

	return SetItem(&item);
}

AFX_INLINE BOOL CLinkCtrl::SetItemState(int iLink, UINT state, UINT stateMask /*= 0xFFFFFFFF*/)
{
	ASSERT(::IsWindow(m_hWnd));
	LITEM item;
	item.mask = LIF_ITEMINDEX | LIF_STATE;
	item.iLink = iLink;
	item.state = state;
	item.stateMask = stateMask;

	return SetItem(&item);
}

AFX_INLINE BOOL CLinkCtrl::GetItem(PLITEM pItem) const
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT(pItem != NULL);
	return (BOOL) ::SendMessage(m_hWnd, LM_GETITEM, 0, (LPARAM)pItem);
}

AFX_INLINE BOOL CLinkCtrl::GetItemID(int iLink, CString& strID) const
{
	ASSERT(::IsWindow(m_hWnd));
	LITEM item;
	item.mask = LIF_ITEMINDEX | LIF_ITEMID;
	item.iLink = iLink;

	if (!GetItem(&item))
		return FALSE;

	strID = item.szID;
	return TRUE;
}

AFX_INLINE BOOL CLinkCtrl::GetItemID(_In_ int iLink, _Out_z_cap_(cchID) LPWSTR szID, _In_ UINT cchID) const
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT(szID != NULL);
	ASSERT(cchID >= MAX_LINKID_TEXT);
	if (szID!=NULL)
	{
		szID[0]=L'\0';
	}
	if (szID == NULL || cchID < MAX_LINKID_TEXT)
		return FALSE;

	LITEM item;
	item.mask = LIF_ITEMINDEX | LIF_ITEMID;
	item.iLink = iLink;

	if (!GetItem(&item))
		return FALSE;

	::ATL::Checked::wcscpy_s(szID, cchID, item.szID);
	return TRUE;
}

AFX_INLINE BOOL CLinkCtrl::GetItemUrl(int iLink, CString& strUrl) const
{
	ASSERT(::IsWindow(m_hWnd));
	LITEM item;
	item.mask = LIF_ITEMINDEX | LIF_URL;
	item.iLink = iLink;

	if (!GetItem(&item))
		return FALSE;

	strUrl = item.szUrl;
	return TRUE;
}

AFX_INLINE BOOL CLinkCtrl::GetItemUrl(_In_ int iLink, _Out_z_cap_(cchUrl) LPWSTR szUrl, _In_ UINT cchUrl) const
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT(szUrl != NULL);
	ASSERT(cchUrl >= L_MAX_URL_LENGTH);
	if(szUrl!=NULL)
	{
		szUrl[0]=L'\0';
	}
	if (szUrl == NULL || cchUrl < L_MAX_URL_LENGTH)
		return FALSE;

	LITEM item;
	item.mask = LIF_ITEMINDEX | LIF_URL;
	item.iLink = iLink;

	if (!GetItem(&item))
		return FALSE;

	::ATL::Checked::wcscpy_s(szUrl, cchUrl, item.szUrl);
	return TRUE;
}

AFX_INLINE BOOL CLinkCtrl::GetItemState(int iLink, UINT *pnState, UINT stateMask /*= 0xFFFFFFFF*/) const
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT(pnState != NULL);
	if (pnState == NULL)
		return FALSE;

	LITEM item;
	item.mask = LIF_ITEMINDEX | LIF_URL;
	item.iLink = iLink;
	item.stateMask = stateMask;

	if (!GetItem(&item))
		return FALSE;

	*pnState = item.state;
	return TRUE;
}


AFX_INLINE BOOL CLinkCtrl::HitTest(PLHITTESTINFO phti)  const
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT(phti != NULL);
	return (BOOL) ::SendMessage(m_hWnd, LM_HITTEST, 0, (LPARAM)phti);
}

#endif	// _WIN32_WINNT >= 0x0501

#if (NTDDI_VERSION >= NTDDI_LONGHORN) && defined(UNICODE)

/////////////////////////////////////////////////////////////////////////////
// CNetAddressCtrl

AFX_INLINE CNetAddressCtrl::CNetAddressCtrl()
{ }

AFX_INLINE CNetAddressCtrl::~CNetAddressCtrl()
{
	DestroyWindow();
}

// Attributes
AFX_INLINE HRESULT CNetAddressCtrl::GetAddress(PNC_ADDRESS pAddress) const
{
	ASSERT(::IsWindow(m_hWnd));
	return NetAddr_GetAddress(m_hWnd, pAddress); // NCM_GETADDRESS
}

AFX_INLINE HRESULT CNetAddressCtrl::SetAllowType(DWORD dwAddrMask)
{
	ASSERT(::IsWindow(m_hWnd));
	return (HRESULT)NetAddr_SetAllowType(m_hWnd, dwAddrMask); // NCM_SETALLOWTYPE
}

AFX_INLINE DWORD CNetAddressCtrl::GetAllowType() const
{
	ASSERT(::IsWindow(m_hWnd));
	return NetAddr_GetAllowType(m_hWnd); // NCM_GETALLOWTYPE
}

// Operations
AFX_INLINE HRESULT CNetAddressCtrl::DisplayErrorTip()
{
	ASSERT(::IsWindow(m_hWnd));
	return NetAddr_DisplayErrorTip(m_hWnd); // NCM_DISPLAYERRORTIP
}

#endif // (NTDDI_VERSION >= NTDDI_LONGHORN) && defined(UNICDOE)
