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
#include "afxshelllistctrl.h"
#include "afxshelltreectrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CMFCShellListCtrl, CMFCListCtrl)

UINT AFX_WM_CHANGE_CURRENT_FOLDER = ::RegisterWindowMessage(_T("AFX_WM_CHANGE_CURRENT_FOLDER"));

IContextMenu2* CMFCShellListCtrl::m_pContextMenu2 = NULL;

/////////////////////////////////////////////////////////////////////////////
// CMFCShellListCtrl

CMFCShellListCtrl::CMFCShellListCtrl()
{
	m_psfCurFolder = NULL;
	m_pidlCurFQ = NULL;
	m_bContextMenu = TRUE;
	m_hwndRelatedTree = NULL;
	m_bIsDesktop = FALSE;
	m_bNoNotify = FALSE;
	m_nTypes = (SHCONTF)(SHCONTF_FOLDERS | SHCONTF_NONFOLDERS);
}

CMFCShellListCtrl::~CMFCShellListCtrl()
{
}

BEGIN_MESSAGE_MAP(CMFCShellListCtrl, CMFCListCtrl)
	//{{AFX_MSG_MAP(CMFCShellListCtrl)
	ON_WM_CREATE()
	ON_WM_CONTEXTMENU()
	ON_WM_DESTROY()
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, &CMFCShellListCtrl::OnDeleteitem)
	ON_NOTIFY_REFLECT(NM_DBLCLK, &CMFCShellListCtrl::OnDblClk)
	ON_NOTIFY_REFLECT(NM_RETURN, &CMFCShellListCtrl::OnReturn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCShellListCtrl message handlers

void CMFCShellListCtrl::ReleaseCurrFolder()
{
	ASSERT_VALID(afxShellManager);

	if (m_psfCurFolder != NULL)
	{
		m_psfCurFolder->Release();
		m_psfCurFolder = NULL;

		afxShellManager->FreeItem(m_pidlCurFQ);
		m_pidlCurFQ = NULL;
	}
}

int CMFCShellListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMFCListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!InitList())
	{
		return -1;
	}

	return 0;
}

HIMAGELIST CMFCShellListCtrl::GetShellImageList(BOOL bLarge)
{
	TCHAR szWinDir [MAX_PATH + 1];
	if (GetWindowsDirectory(szWinDir, MAX_PATH) == 0)
	{
		return NULL;
	}

	SHFILEINFO sfi;
	HIMAGELIST hImageList = (HIMAGELIST) SHGetFileInfo(szWinDir, 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX |(bLarge ? 0 : SHGFI_SMALLICON));
	return hImageList;
}

HRESULT CMFCShellListCtrl::LockCurrentFolder(LPAFX_SHELLITEMINFO pItemInfo)
{
	ASSERT_VALID(afxShellManager);

	HRESULT hr = E_FAIL;
	m_pidlCurFQ = NULL;

	if (pItemInfo != NULL && pItemInfo->pParentFolder != NULL)
	{
		ENSURE(pItemInfo->pidlRel != NULL);
		hr = pItemInfo->pParentFolder->BindToObject(pItemInfo->pidlRel, NULL, IID_IShellFolder, (LPVOID*)&m_psfCurFolder);

		m_bIsDesktop = FALSE;
	}
	else
	{
		hr = SHGetDesktopFolder(&m_psfCurFolder);
		m_bIsDesktop = TRUE;
	}

	if (SUCCEEDED(hr) && pItemInfo != NULL)
	{
		m_pidlCurFQ = afxShellManager->CopyItem(pItemInfo->pidlFQ);
	}

	return hr;
}

HRESULT CMFCShellListCtrl::DisplayFolder(LPAFX_SHELLITEMINFO pItemInfo)
{
	HRESULT hr = E_FAIL;

	if (afxShellManager == NULL)
	{
		ASSERT(FALSE);
		return hr;
	}

	if (pItemInfo != NULL)
	{
		ReleaseCurrFolder();
		hr = LockCurrentFolder(pItemInfo);

		if (FAILED(hr))
		{
			return hr;
		}
	}

	DeleteAllItems();

	if (m_psfCurFolder != NULL)
	{
		CWaitCursor wait;
		SetRedraw(FALSE);

		hr = EnumObjects(m_psfCurFolder, m_pidlCurFQ);

		if (GetStyle() & LVS_REPORT)
		{
			Sort(AFX_ShellList_ColumnName);
		}

		SetRedraw(TRUE);
		RedrawWindow();
	}

	if (SUCCEEDED(hr) && pItemInfo != NULL)
	{
		CMFCShellTreeCtrl* pTree = GetRelatedTree();
		if (pTree != NULL && !m_bNoNotify)
		{
			ASSERT_VALID(pTree);
			pTree->SelectPath(m_pidlCurFQ);
		}

		if (GetParent() != NULL)
		{
			GetParent()->SendMessage(AFX_WM_CHANGE_CURRENT_FOLDER);
		}
	}

	return hr;
}

HRESULT CMFCShellListCtrl::DisplayParentFolder()
{
	ASSERT_VALID(afxShellManager);

	HRESULT hr = E_FAIL;
	if (m_pidlCurFQ == NULL)
	{
		return hr;
	}

	AFX_SHELLITEMINFO info;
	int nLevel = afxShellManager->GetParentItem(m_pidlCurFQ, info.pidlFQ);

	if (nLevel < 0)
	{
		return hr;
	}

	if (nLevel == 0) // Desktop
	{
		hr = DisplayFolder(&info);
	}
	else
	{
		LPSHELLFOLDER pDesktopFolder;
		hr = SHGetDesktopFolder(&pDesktopFolder);

		if (SUCCEEDED(hr))
		{
			info.pParentFolder = pDesktopFolder;
			info.pidlRel = info.pidlFQ;

			hr = DisplayFolder(&info);
			pDesktopFolder->Release();
		}
	}

	afxShellManager->FreeItem(info.pidlFQ);
	return hr;
}

HRESULT CMFCShellListCtrl::DisplayFolder(LPCTSTR lpszPath)
{
	if (afxShellManager == NULL)
	{
		ASSERT(FALSE);
		return E_FAIL;
	}

	ENSURE(lpszPath != NULL);
	ASSERT_VALID(afxShellManager);

	AFX_SHELLITEMINFO info;
	HRESULT hr = afxShellManager->ItemFromPath(lpszPath, info.pidlRel);

	if (FAILED(hr))
	{
		return hr;
	}

	LPSHELLFOLDER pDesktopFolder;
	hr = SHGetDesktopFolder(&pDesktopFolder);

	if (SUCCEEDED(hr))
	{
		info.pParentFolder = pDesktopFolder;
		info.pidlFQ = info.pidlRel;

		hr = DisplayFolder(&info);
		pDesktopFolder->Release();
	}

	afxShellManager->FreeItem(info.pidlFQ);
	return hr;
}

HRESULT CMFCShellListCtrl::Refresh()
{
	return DisplayFolder((LPAFX_SHELLITEMINFO) NULL);
}

HRESULT CMFCShellListCtrl::EnumObjects(LPSHELLFOLDER pParentFolder, LPITEMIDLIST pidlParent)
{
	ASSERT_VALID(this);
	ASSERT_VALID(afxShellManager);

	LPENUMIDLIST pEnum;
	HRESULT hRes = pParentFolder->EnumObjects(NULL, m_nTypes, &pEnum);

	if (SUCCEEDED(hRes))
	{
		LPITEMIDLIST pidlTemp;
		DWORD dwFetched = 1;
		LPAFX_SHELLITEMINFO pItem;

		//enumerate the item's PIDLs
		while (pEnum->Next(1, &pidlTemp, &dwFetched) == S_OK && dwFetched)
		{
			LVITEM lvItem;
			ZeroMemory(&lvItem, sizeof(lvItem));

			//fill in the TV_ITEM structure for this item
			lvItem.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;

			//AddRef the parent folder so it's pointer stays valid
			pParentFolder->AddRef();

			//put the private information in the lParam
			pItem = (LPAFX_SHELLITEMINFO)GlobalAlloc(GPTR, sizeof(AFX_SHELLITEMINFO));

			pItem->pidlRel = pidlTemp;
			pItem->pidlFQ = afxShellManager->ConcatenateItem(pidlParent, pidlTemp);

			pItem->pParentFolder = pParentFolder;
			lvItem.lParam = (LPARAM)pItem;

			lvItem.pszText = _T("");
			lvItem.iImage = OnGetItemIcon(GetItemCount(), pItem);

			//determine if the item is shared
			DWORD dwAttr = SFGAO_DISPLAYATTRMASK;
			pParentFolder->GetAttributesOf(1, (LPCITEMIDLIST*)&pidlTemp, &dwAttr);

			if (dwAttr & SFGAO_SHARE)
			{
				lvItem.mask |= LVIF_STATE;
				lvItem.stateMask |= LVIS_OVERLAYMASK;
				lvItem.state |= INDEXTOOVERLAYMASK(1); //1 is the index for the shared overlay image
			}

			if (dwAttr & SFGAO_GHOSTED)
			{
				lvItem.mask |= LVIF_STATE;
				lvItem.stateMask |= LVIS_CUT;
				lvItem.state |= LVIS_CUT;
			}

			int iItem = InsertItem(&lvItem);
			if (iItem >= 0)
			{
				// Set columns:
				const int nColumns = m_wndHeader.GetItemCount();
				for (int iColumn = 0; iColumn < nColumns; iColumn++)
				{
					SetItemText(iItem, iColumn, OnGetItemText(iItem, iColumn, pItem));
				}
			}

			dwFetched = 0;
		}

		pEnum->Release();
	}

	return hRes;
}

void CMFCShellListCtrl::DoDefault(int iItem)
{
	LVITEM lvItem;

	ZeroMemory(&lvItem, sizeof(lvItem));
	lvItem.mask = LVIF_PARAM;
	lvItem.iItem = iItem;

	if (!GetItem(&lvItem))
	{
		return;
	}

	LPAFX_SHELLITEMINFO pInfo = (LPAFX_SHELLITEMINFO) lvItem.lParam;
	if (pInfo == NULL || pInfo->pParentFolder == NULL || pInfo->pidlRel == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	IShellFolder *psfFolder = pInfo->pParentFolder;
	if (psfFolder == NULL)
	{
		HRESULT hr = SHGetDesktopFolder(&psfFolder);
		if (FAILED(hr))
		{
			ASSERT(FALSE);
			return;
		}
	}
	else
	{
		psfFolder->AddRef();
	}

	if (psfFolder == NULL)
	{
		return;
	}

	// If specified element is a folder, try to display it:
	ULONG ulAttrs = SFGAO_FOLDER;
	psfFolder->GetAttributesOf(1, (const struct _ITEMIDLIST **) &pInfo->pidlRel, &ulAttrs);

	if (ulAttrs & SFGAO_FOLDER)
	{
		DisplayFolder(pInfo);
	}
	else
	{
		// Invoke a default menu command:
		IContextMenu *pcm;
		HRESULT hr = psfFolder->GetUIObjectOf(GetSafeHwnd(), 1, (LPCITEMIDLIST*)&pInfo->pidlRel, IID_IContextMenu, NULL, (LPVOID*)&pcm);

		if (SUCCEEDED(hr))
		{
			HMENU hPopup = CreatePopupMenu();

			if (hPopup != NULL)
			{
				hr = pcm->QueryContextMenu(hPopup, 0, 1, 0x7fff, CMF_DEFAULTONLY | CMF_EXPLORE);

				if (SUCCEEDED(hr))
				{
					UINT idCmd = ::GetMenuDefaultItem(hPopup, FALSE, 0);
					if (idCmd != 0 && idCmd != (UINT)-1)
					{
						CMINVOKECOMMANDINFO cmi;
						cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
						cmi.fMask = 0;
						cmi.hwnd = GetParent()->GetSafeHwnd();
						cmi.lpVerb = (LPCSTR)(INT_PTR)(idCmd - 1);
						cmi.lpParameters = NULL;
						cmi.lpDirectory = NULL;
						cmi.nShow = SW_SHOWNORMAL;
						cmi.dwHotKey = 0;
						cmi.hIcon = NULL;

						hr = pcm->InvokeCommand(&cmi);

						if (SUCCEEDED(hr) && GetParent() != NULL)
						{
							GetParent()->SendMessage(AFX_WM_ON_AFTER_SHELL_COMMAND, (WPARAM) idCmd);
						}
					}
				}
			}

			pcm->Release();
		}
	}

	psfFolder->Release();
}

void CMFCShellListCtrl::OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult)
{
	ASSERT_VALID(afxShellManager);

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	ENSURE(pNMListView != NULL);

	LPAFX_SHELLITEMINFO pItem = (LPAFX_SHELLITEMINFO)pNMListView->lParam;

	//free up the pidls that we allocated
	afxShellManager->FreeItem(pItem->pidlFQ);
	afxShellManager->FreeItem(pItem->pidlRel);

	//this may be NULL if this is the root item
	if (pItem->pParentFolder != NULL)
	{
		pItem->pParentFolder->Release();
		pItem->pParentFolder = NULL;
	}

	GlobalFree((HGLOBAL) pItem);

	*pResult = 0;
}

void CMFCShellListCtrl::OnDblClk(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	int nItem = GetNextItem(-1, LVNI_FOCUSED);
	if (nItem != -1)
	{
		DoDefault(nItem);
	}

	*pResult = 0;
}

void CMFCShellListCtrl::OnReturn(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	int nItem = GetNextItem(-1, LVNI_FOCUSED);
	if (nItem != -1)
	{
		DoDefault(nItem);
	}

	*pResult = 0;
}

BOOL CMFCShellListCtrl::GetItemPath(CString& strPath, int iItem) const
{
	ASSERT_VALID(this);

	strPath.Empty();

	LPAFX_SHELLITEMINFO pItem = (LPAFX_SHELLITEMINFO) GetItemData(iItem);
	if (pItem == NULL || pItem->pidlFQ == NULL)
	{
		return FALSE;
	}

	TCHAR szPath [MAX_PATH];
	if (!SHGetPathFromIDList(pItem->pidlFQ, szPath))
	{
		return FALSE;
	}

	strPath = szPath;
	return TRUE;
}

BOOL CMFCShellListCtrl::GetCurrentFolder(CString& strPath) const
{
	ASSERT_VALID(this);

	strPath.Empty();

	if (m_pidlCurFQ == NULL)
	{
		return FALSE;
	}

	TCHAR szPath [MAX_PATH];
	if (!SHGetPathFromIDList(m_pidlCurFQ, szPath))
	{
		return FALSE;
	}

	strPath = szPath;
	return TRUE;
}

BOOL CMFCShellListCtrl::GetCurrentFolderName(CString& strName) const
{
	ASSERT_VALID(this);

	strName.Empty();

	if (m_pidlCurFQ == NULL)
	{
		return FALSE;
	}

	SHFILEINFO sfi;
	if (!SHGetFileInfo((LPCTSTR)m_pidlCurFQ, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_DISPLAYNAME))
	{
		return FALSE;
	}

	strName = sfi.szDisplayName;
	return TRUE;
}

CString CMFCShellListCtrl::OnGetItemText(int /*iItem*/, int iColumn, LPAFX_SHELLITEMINFO pItem)
{
	ASSERT_VALID(this);
	ENSURE(pItem != NULL);

	SHFILEINFO sfi;
	TCHAR szPath [MAX_PATH];

	switch (iColumn)
	{
	case AFX_ShellList_ColumnName:
		if (SHGetFileInfo((LPCTSTR)pItem->pidlFQ, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_DISPLAYNAME))
		{
			return sfi.szDisplayName;
		}
		break;

	case AFX_ShellList_ColumnType:
		if (SHGetFileInfo((LPCTSTR)pItem->pidlFQ, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_TYPENAME))
		{
			return sfi.szTypeName;
		}
		break;

	case AFX_ShellList_ColumnSize:
	case AFX_ShellList_ColumnModified:
		if (SHGetPathFromIDList(pItem->pidlFQ, szPath))
		{
			CFileStatus fs;
			if (CFile::GetStatus(szPath, fs))
			{
				CString str;

				if (iColumn == 1)
				{
					if ((fs.m_attribute &(CFile::directory | CFile ::volume)) == 0)
					{
						OnFormatFileSize(fs.m_size, str);
					}
				}
				else
				{
					OnFormatFileDate(fs.m_mtime, str);
				}

				return str;
			}
		}
		break;

	default:
		ASSERT(FALSE);
		break;
	}

	return _T("");
}

int CMFCShellListCtrl::OnGetItemIcon(int /*iItem*/, LPAFX_SHELLITEMINFO pItem)
{
	ASSERT_VALID(this);
	if (pItem == NULL)
	{
		ASSERT(FALSE);
		return -1;
	}

	SHFILEINFO sfi;
	int iIcon = -1;

	if (SHGetFileInfo((LPCTSTR)pItem->pidlFQ, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_LINKOVERLAY))
	{
		iIcon = sfi.iIcon;
	}

	return iIcon;
}

int CMFCShellListCtrl::OnCompareItems(LPARAM lParam1, LPARAM lParam2, int iColumn)
{
	ASSERT_VALID(this);
	LPAFX_SHELLITEMINFO pItem1 = (LPAFX_SHELLITEMINFO)lParam1;
	LPAFX_SHELLITEMINFO pItem2 = (LPAFX_SHELLITEMINFO)lParam2;

	ENSURE(pItem1 != NULL);
	ENSURE(pItem2 != NULL);

	SHFILEINFO sfi1;
	SHFILEINFO sfi2;

	TCHAR szPath1 [MAX_PATH];
	TCHAR szPath2 [MAX_PATH];

	CFileStatus fs1;
	CFileStatus fs2;

	int nRes = 0;

	switch (iColumn)
	{
	case AFX_ShellList_ColumnName:
		{
			HRESULT hr = pItem1->pParentFolder->CompareIDs(0, pItem1->pidlRel, pItem2->pidlRel);

			if (FAILED(hr))
			{
				return 0;
			}

			nRes = (short) SCODE_CODE(GetScode(hr));
		}
		break;

	case AFX_ShellList_ColumnType:
		if (SHGetFileInfo((LPCTSTR)pItem1->pidlFQ, 0, &sfi1, sizeof(sfi1), SHGFI_PIDL | SHGFI_TYPENAME) &&
			SHGetFileInfo((LPCTSTR)pItem2->pidlFQ, 0, &sfi2, sizeof(sfi2), SHGFI_PIDL | SHGFI_TYPENAME))
		{
			nRes = lstrcmpi(sfi1.szTypeName, sfi2.szTypeName);
		}
		break;

	case AFX_ShellList_ColumnSize:
	case AFX_ShellList_ColumnModified:
		if (SHGetPathFromIDList(pItem1->pidlFQ, szPath1) && CFile::GetStatus(szPath1, fs1))
		{
			if (SHGetPathFromIDList(pItem2->pidlFQ, szPath2) && CFile::GetStatus(szPath2, fs2))
			{
				if (iColumn == AFX_ShellList_ColumnSize)
				{
					if (fs1.m_attribute & CFile::directory)
					{
						nRes = -1;
					}
					else if (fs2.m_attribute & CFile::directory)
					{
						nRes = 1;
					}
					else
					{
						nRes = fs1.m_size < fs2.m_size ? -1 : fs1.m_size > fs2.m_size ? 1 : 0;
					}
				}
				else
				{
					nRes = fs1.m_mtime < fs2.m_mtime ? -1 : fs1.m_mtime > fs2.m_mtime ? 1 : 0;
				}
			}
			else
			{
				nRes = 1;
			}
		}
		else
		{
			nRes = -1;
		}
		break;
	}

	return nRes;
}

void CMFCShellListCtrl::OnSetColumns()
{
	const TCHAR* szName [] = {
		_T("Name"), _T("Size"), _T("Type"), _T("Modified"), };

		for (int iColumn = 0; iColumn < 4; iColumn++)
		{
			int nFormat = (iColumn == AFX_ShellList_ColumnSize) ? LVCFMT_RIGHT : LVCFMT_LEFT;

			InsertColumn(iColumn, szName [iColumn], nFormat, 100, iColumn);
		}
}

void CMFCShellListCtrl::PreSubclassWindow()
{
	CMFCListCtrl::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (pThreadState->m_pWndInit == NULL)
	{
		if (!InitList())
		{
			ASSERT(FALSE);
		}
	}
}

BOOL CMFCShellListCtrl::InitList()
{
	if (afxShellManager == NULL)
	{
		TRACE0("You need to initialize CShellManager first\n");
		return FALSE;
	}

	ModifyStyle(0, LVS_SHAREIMAGELISTS);

	// Set shell images:
	SetImageList(CImageList::FromHandle(GetShellImageList(TRUE)), LVSIL_NORMAL);
	SetImageList(CImageList::FromHandle(GetShellImageList(FALSE)), LVSIL_SMALL);

	// Add columns:
	OnSetColumns();

	if (m_psfCurFolder == NULL)
	{
		// Display desktop:
		AFX_SHELLITEMINFO info;

		if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &info.pidlFQ)))
		{
			DisplayFolder(&info);
			afxShellManager->FreeItem(info.pidlFQ);
		}
	}

	return TRUE;
}

void CMFCShellListCtrl::OnFormatFileSize(__int64 lFileSize, CString& str)
{
	str.Empty();

	if (lFileSize == 0)
	{
		str = _T("0");
	}
	else
	{
		lFileSize = lFileSize / 1024 + 1;
		str.Format(_T("%I64d"), lFileSize);

		// Convert number to the system format:
		TCHAR szNumOut [256];
		GetNumberFormat(LOCALE_USER_DEFAULT, LOCALE_NOUSEROVERRIDE, str, NULL, szNumOut, 255);

		str = szNumOut;

		// Truncate trailing fractal digits:
		TCHAR szDec [10];
		GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, szDec, 10);

		int nDecLen = lstrlen(szDec);
		if (nDecLen > 0)
		{
			for (int i = str.GetLength() - nDecLen - 1; i >= 0; i--)
			{
				if (str.Mid(i, nDecLen) == szDec)
				{
					str = str.Left(i);
					break;
				}
			}
		}
	}

	str += _T(" KB");
}

void CMFCShellListCtrl::OnFormatFileDate(const CTime& tmFile, CString& str)
{
	COleDateTime dateFile(tmFile.GetTime());
	str = dateFile.Format();
}

void CMFCShellListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	ASSERT_VALID(this);
	ASSERT_VALID(afxShellManager);
	ENSURE(afxShellManager->m_pMalloc != NULL);

	if (m_pContextMenu2 != NULL)
	{
		return;
	}

	if (!m_bContextMenu)
	{
		Default();
		return;
	}

	if (m_psfCurFolder == NULL)
	{
		return;
	}

	UINT nSelItems = GetSelectedCount();
	int nClickedItem = -1;

	if (point.x == -1 && point.y == -1)
	{
		// Keyboard, show menu for the currently selected item(s):
		if (nSelItems == 0)
		{
			return;
		}

		int nCurItem = -1;
		int nLastSelItem = -1;

		for (UINT i = 0; i < nSelItems; i++)
		{
			nCurItem = GetNextItem(nCurItem, LVNI_SELECTED);
			nLastSelItem = nCurItem;
		}

		CRect rectItem;
		if (GetItemRect(nLastSelItem, rectItem, LVIR_BOUNDS))
		{
			point.x = rectItem.left;
			point.y = rectItem.bottom + 1;

			ClientToScreen(&point);
		}
	}
	else
	{
		// Clicked on specifed item:
		LVHITTESTINFO lvhti;
		lvhti.pt = point;
		ScreenToClient(&lvhti.pt);

		lvhti.flags = LVHT_NOWHERE;

		HitTest(&lvhti);

		if ((lvhti.flags & LVHT_ONITEM) == 0)
		{
			// Click ouside of items, do nothing
			return;
		}

		nClickedItem = lvhti.iItem;
	}

	LPITEMIDLIST* pPidls = (LPITEMIDLIST*) afxShellManager->m_pMalloc->Alloc(sizeof(LPITEMIDLIST) * nSelItems);
	ENSURE(pPidls != NULL);

	// Get the selected items:
	LVITEM lvItem;
	ZeroMemory(&lvItem, sizeof(lvItem));
	lvItem.mask = LVIF_PARAM;

	LPAFX_SHELLITEMINFO pClickedInfo = (LPAFX_SHELLITEMINFO)lvItem.lParam;

	if (nClickedItem >= 0)
	{
		// Put the item clicked on first in the list:
		lvItem.iItem = nClickedItem;

		if (GetItem(&lvItem))
		{
			pClickedInfo = (LPAFX_SHELLITEMINFO)lvItem.lParam;
			pPidls [0] = pClickedInfo->pidlRel;
		}
	}

	int nCurItem = -1;
	for (UINT i = nClickedItem >= 0 ? 1 : 0; i < nSelItems; i++)
	{
		nCurItem = GetNextItem(nCurItem, LVNI_SELECTED);
		if (nCurItem != nClickedItem)
		{
			lvItem.iItem = nCurItem;

			if (GetItem(&lvItem))
			{
				LPAFX_SHELLITEMINFO pInfo = (LPAFX_SHELLITEMINFO)lvItem.lParam;
				pPidls [i] = pInfo->pidlRel;

				if (pClickedInfo == NULL)
				{
					pClickedInfo = pInfo;
				}
			}
		}
		else
		{
			i--;
		}
	}

	if (pPidls [0] == NULL)
	{
		afxShellManager->m_pMalloc->Free(pPidls);
		return;
	}

	IContextMenu* pcm;
	HRESULT hr = m_psfCurFolder->GetUIObjectOf(GetSafeHwnd(), nSelItems, (LPCITEMIDLIST*)pPidls, IID_IContextMenu, NULL, (LPVOID*)&pcm);

	if (SUCCEEDED(hr))
	{
		hr = pcm->QueryInterface(IID_IContextMenu2, (LPVOID*)&m_pContextMenu2);

		if (SUCCEEDED(hr))
		{
			HMENU hPopup = CreatePopupMenu();
			if (hPopup != NULL)
			{
				hr = m_pContextMenu2->QueryContextMenu(hPopup, 0, 1, 0x7fff, CMF_NORMAL | CMF_EXPLORE);

				if (SUCCEEDED(hr))
				{
					UINT idCmd = TrackPopupMenu(hPopup, TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, point.x, point.y, 0, GetSafeHwnd(), NULL);

					if (idCmd != 0)
					{
						BOOL bIsFolder = FALSE;

						if (nSelItems == 1 && idCmd == ::GetMenuDefaultItem(hPopup, FALSE, 0))
						{
							// If specified element is a folder, try to display it:
							ULONG ulAttrs = SFGAO_FOLDER;
							m_psfCurFolder->GetAttributesOf(1, (const struct _ITEMIDLIST **) &pClickedInfo->pidlRel, &ulAttrs);

							if (ulAttrs & SFGAO_FOLDER)
							{
								bIsFolder = TRUE;
								DisplayFolder(pClickedInfo);
							}
						}

						if (!bIsFolder)
						{
							CMINVOKECOMMANDINFO cmi;
							cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
							cmi.fMask = 0;
							cmi.hwnd = (HWND) GetParent();
							cmi.lpVerb = (LPCSTR)(INT_PTR)(idCmd - 1);
							cmi.lpParameters = NULL;
							cmi.lpDirectory = NULL;
							cmi.nShow = SW_SHOWNORMAL;
							cmi.dwHotKey = 0;
							cmi.hIcon = NULL;

							hr = pcm->InvokeCommand(&cmi);

							if (SUCCEEDED(hr) && GetParent() != NULL)
							{
								GetParent()->SendMessage(AFX_WM_ON_AFTER_SHELL_COMMAND, (WPARAM) idCmd);
							}
						}
					}
				}
			}

			if (m_pContextMenu2 != NULL)
			{
				m_pContextMenu2->Release();
				m_pContextMenu2 = NULL;
			}
		}

		pcm->Release();
	}

	afxShellManager->m_pMalloc->Free(pPidls);
}

void CMFCShellListCtrl::EnableShellContextMenu(BOOL bEnable)
{
	m_bContextMenu = bEnable;
}

void CMFCShellListCtrl::OnDestroy()
{
	CMFCShellTreeCtrl* pTree = GetRelatedTree();
	if (pTree != NULL)
	{
		ASSERT_VALID(pTree);
		pTree->m_hwndRelatedList = NULL;
	}

	ReleaseCurrFolder();
	CMFCListCtrl::OnDestroy();
}

CMFCShellTreeCtrl* CMFCShellListCtrl::GetRelatedTree() const
{
	if (m_hwndRelatedTree == NULL)
	{
		return NULL;
	}

	return DYNAMIC_DOWNCAST(CMFCShellTreeCtrl, CWnd::FromHandlePermanent(m_hwndRelatedTree));
}

void CMFCShellListCtrl::SetItemTypes(SHCONTF nTypes)
{
	ASSERT_VALID(this);

	if (m_nTypes != nTypes)
	{
		m_nTypes = nTypes;
		if (GetSafeHwnd() != NULL)
		{
			Refresh();
		}
	}
}

LRESULT CMFCShellListCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITMENUPOPUP:
	case WM_DRAWITEM:
	case WM_MEASUREITEM:
		if (m_pContextMenu2 != NULL)
		{
			m_pContextMenu2->HandleMenuMsg(message, wParam, lParam);
			return 0;
		}
		break;
	}

	return CMFCListCtrl::WindowProc(message, wParam, lParam);
}



