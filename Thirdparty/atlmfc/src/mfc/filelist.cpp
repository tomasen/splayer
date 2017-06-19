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
// lpszCanon = C:\MYAPP\DEBUGS\C\TESWIN.C
//
// cchMax   b   Result
// ------   -   ---------
//  1- 7    F   <empty>
//  1- 7    T   TESWIN.C
//  8-14    x   TESWIN.C
// 15-16    x   C:\...\TESWIN.C
// 17-23    x   C:\...\C\TESWIN.C
// 24-25    x   C:\...\DEBUGS\C\TESWIN.C
// 26+      x   C:\MYAPP\DEBUGS\C\TESWIN.C

AFX_STATIC void AFXAPI _AfxAbbreviateName(_Inout_z_ LPTSTR lpszCanon, int cchMax, BOOL bAtLeastName)
{
	ENSURE_ARG(AfxIsValidString(lpszCanon));

	int cchFullPath, cchFileName, cchVolName;
	const TCHAR* lpszCur;
	const TCHAR* lpszBase;
	const TCHAR* lpszFileName;

	lpszBase = lpszCanon;
	cchFullPath = lstrlen(lpszCanon);

	cchFileName = AfxGetFileName(lpszCanon, NULL, 0) - 1;
	lpszFileName = lpszBase + (cchFullPath-cchFileName);

	// If cchMax is more than enough to hold the full path name, we're done.
	// This is probably a pretty common case, so we'll put it first.
	if (cchMax >= cchFullPath)
		return;

	// If cchMax isn't enough to hold at least the basename, we're done
	if (cchMax < cchFileName)
	{
		if (!bAtLeastName)
			lpszCanon[0] = _T('\0');
		else
			Checked::tcscpy_s(lpszCanon, cchFullPath + 1, lpszFileName);
		return;
	}

	// Calculate the length of the volume name.  Normally, this is two characters
	// (e.g., "C:", "D:", etc.), but for a UNC name, it could be more (e.g.,
	// "\\server\share").
	//
	// If cchMax isn't enough to hold at least <volume_name>\...\<base_name>, the
	// result is the base filename.

	lpszCur = lpszBase + 2;                 // Skip "C:" or leading "\\"

	if (lpszBase[0] == '\\' && lpszBase[1] == '\\') // UNC pathname
	{
		// First skip to the '\' between the server name and the share name,
		while (*lpszCur != '\\')
		{
			lpszCur = _tcsinc(lpszCur);
			ASSERT(*lpszCur != '\0');
		}
	}
	// if a UNC get the share name, if a drive get at least one directory
	ASSERT(*lpszCur == '\\');
	// make sure there is another directory, not just c:\filename.ext
	if (cchFullPath - cchFileName > 3)
	{
		lpszCur = _tcsinc(lpszCur);
		while (*lpszCur != '\\')
		{
			lpszCur = _tcsinc(lpszCur);
			ASSERT(*lpszCur != '\0');
		}
	}
	ASSERT(*lpszCur == '\\');

	cchVolName = int(lpszCur - lpszBase);
	if (cchMax < cchVolName + 5 + cchFileName)
	{
		Checked::tcscpy_s(lpszCanon, cchFullPath + 1, lpszFileName);
		return;
	}

	// Now loop through the remaining directory components until something
	// of the form <volume_name>\...\<one_or_more_dirs>\<base_name> fits.
	//
	// Assert that the whole filename doesn't fit -- this should have been
	// handled earlier.

	ASSERT(cchVolName + (int)lstrlen(lpszCur) > cchMax);
	while (cchVolName + 4 + (int)lstrlen(lpszCur) > cchMax)
	{
		do
		{
			lpszCur = _tcsinc(lpszCur);
			ASSERT(*lpszCur != '\0');
		}
		while (*lpszCur != '\\');
	}

	// Form the resultant string and we're done.
	int cch;
	if (cchVolName >= 0 && cchVolName < cchMax)
		cch = cchVolName;
	else cch = cchMax;
	Checked::memcpy_s(lpszCanon + cch, cchFullPath + 1 - cch, _T("\\..."), sizeof(_T("\\...")) );
	Checked::tcscat_s(lpszCanon, cchFullPath + 1, lpszCur);
}

/////////////////////////////////////////////////////////////////////////////
// CRecentFileList

#pragma warning(disable: 4267)
CRecentFileList::CRecentFileList(UINT nStart, LPCTSTR lpszSection,
	LPCTSTR lpszEntryFormat, int nSize, int nMaxDispLen)
{
	ENSURE_ARG(nSize >= 0);
	m_arrNames = new CString[nSize];
	ENSURE_ARG(m_arrNames != NULL);
	
	m_nSize = nSize;
	m_nStart = nStart;
	
	m_strSectionName = lpszSection;
	m_strEntryFormat = lpszEntryFormat;
	
	m_nMaxDisplayLength = nMaxDispLen;
}

CRecentFileList::~CRecentFileList()
{
	delete[] m_arrNames;
}

// Operations
void CRecentFileList::Add(LPCTSTR lpszPathName)
{
	ASSERT(m_arrNames != NULL);
	ASSERT(AfxIsValidString(lpszPathName));

	// fully qualify the path name
	TCHAR szTemp[_MAX_PATH];
	if ( lpszPathName == NULL || lstrlen(lpszPathName) >= _MAX_PATH )
	{
		ASSERT(FALSE);
		// MFC requires paths with length < _MAX_PATH
		// No other way to handle the error from a void function
		AfxThrowFileException(CFileException::badPath);
	}

	AfxFullPath(szTemp, lpszPathName);
	
	// update the MRU list, if an existing MRU string matches file name
	int iMRU;
	for (iMRU = 0; iMRU < m_nSize-1; iMRU++)
	{
		if (AfxComparePath(m_arrNames[iMRU], szTemp))
			break;      // iMRU will point to matching entry
	}
	// move MRU strings before this one down
	for (; iMRU > 0; iMRU--)
	{
		ASSERT(iMRU > 0);
		ASSERT(iMRU < m_nSize);
		m_arrNames[iMRU] = m_arrNames[iMRU-1];
	}
	// place this one at the beginning
	m_arrNames[0] = szTemp;
}

void CRecentFileList::Remove(int nIndex)
{
	ENSURE_ARG(nIndex >= 0 && nIndex < m_nSize);

	m_arrNames[nIndex].Empty();
	int iMRU;
	for (iMRU = nIndex; iMRU < m_nSize-1; iMRU++)
		m_arrNames[iMRU] = m_arrNames[iMRU+1];

	ASSERT(iMRU < m_nSize);
	m_arrNames[iMRU].Empty();
}

BOOL CRecentFileList::GetDisplayName(CString& strName, int nIndex,
	LPCTSTR lpszCurDir, int nCurDir, BOOL bAtLeastName) const
{
	ENSURE_ARG(lpszCurDir == NULL || AfxIsValidString(lpszCurDir, nCurDir));

	ASSERT(m_arrNames != NULL);
	ENSURE_ARG(nIndex < m_nSize);
	if (lpszCurDir == NULL || m_arrNames[nIndex].IsEmpty())
		return FALSE;

	int nLenName = m_arrNames[nIndex].GetLength();
	LPTSTR lpch = strName.GetBuffer( nLenName + 1);
	if (lpch == NULL)
	{
		AfxThrowMemoryException();
	}
	Checked::tcsncpy_s(lpch, nLenName + 1, m_arrNames[nIndex], _TRUNCATE);
	// nLenDir is the length of the directory part of the full path
	int nLenDir = nLenName - (AfxGetFileName(lpch, NULL, 0) - 1);
	BOOL bSameDir = FALSE;
	if (nLenDir == nCurDir)
	{
		TCHAR chSave = lpch[nLenDir];
		lpch[nCurDir] = 0;  // terminate at same location as current dir
		bSameDir = ::AfxComparePath(lpszCurDir, lpch);
		lpch[nLenDir] = chSave;
	}
	// copy the full path, otherwise abbreviate the name
	if (bSameDir)
	{
		// copy file name only since directories are same
		TCHAR szTemp[_MAX_PATH];
		AfxGetFileTitle(lpch+nCurDir, szTemp, _countof(szTemp));
		Checked::tcsncpy_s(lpch, nLenName + 1, szTemp, _TRUNCATE);
	}
	else if (m_nMaxDisplayLength != -1)
	{
		// strip the extension if the system calls for it
		TCHAR szTemp[_MAX_PATH];
		AfxGetFileTitle(lpch+nLenDir, szTemp, _countof(szTemp));
		Checked::tcsncpy_s(lpch+nLenDir, nLenName + 1 - nLenDir, szTemp, _TRUNCATE);

		// abbreviate name based on what will fit in limited space
		_AfxAbbreviateName(lpch, m_nMaxDisplayLength, bAtLeastName);
	}
	strName.ReleaseBuffer();
	return TRUE;
}

void CRecentFileList::UpdateMenu(CCmdUI* pCmdUI)
{
	ENSURE_ARG(pCmdUI != NULL);
	ASSERT(m_arrNames != NULL);

	CMenu* pMenu = pCmdUI->m_pMenu;
	if (m_strOriginal.IsEmpty() && pMenu != NULL)
		pMenu->GetMenuString(pCmdUI->m_nID, m_strOriginal, MF_BYCOMMAND);

	if (m_arrNames[0].IsEmpty())
	{
		// no MRU files
		if (!m_strOriginal.IsEmpty())
			pCmdUI->SetText(m_strOriginal);
		pCmdUI->Enable(FALSE);
		return;
	}

	if (pCmdUI->m_pMenu == NULL)
		return;

	int iMRU;
	for (iMRU = 0; iMRU < m_nSize; iMRU++)
		pCmdUI->m_pMenu->DeleteMenu(pCmdUI->m_nID + iMRU, MF_BYCOMMAND);

	TCHAR szCurDir[_MAX_PATH];
	DWORD dwDirLen = GetCurrentDirectory(_MAX_PATH, szCurDir);
	if( dwDirLen == 0 || dwDirLen >= _MAX_PATH )
		return;	// Path too long

	int nCurDir = lstrlen(szCurDir);
	ASSERT(nCurDir >= 0);
	szCurDir[nCurDir] = '\\';
	szCurDir[++nCurDir] = '\0';

	CString strName;
	CString strTemp;
	for (iMRU = 0; iMRU < m_nSize; iMRU++)
	{
		if (!GetDisplayName(strName, iMRU, szCurDir, nCurDir))
			break;

		// double up any '&' characters so they are not underlined
		LPCTSTR lpszSrc = strName;
		LPTSTR lpszDest = strTemp.GetBuffer(strName.GetLength()*2);
		while (*lpszSrc != 0)
		{
			if (*lpszSrc == '&')
				*lpszDest++ = '&';
			if (_istlead(*lpszSrc))
				*lpszDest++ = *lpszSrc++;
			*lpszDest++ = *lpszSrc++;
		}
		*lpszDest = 0;
		strTemp.ReleaseBuffer();

		// insert mnemonic + the file name
		TCHAR buf[10];
		int nItem = (iMRU + 1 + m_nStart) % _AFX_MRU_MAX_COUNT;

		// number &1 thru &9, then 1&0, then 11 thru ...
		if (nItem > 10)
			_stprintf_s(buf, _countof(buf), _T("%d "), nItem);
		else if (nItem == 10)
			Checked::tcscpy_s(buf, _countof(buf), _T("1&0 "));
		else
			_stprintf_s(buf, _countof(buf), _T("&%d "), nItem);

		pCmdUI->m_pMenu->InsertMenu(pCmdUI->m_nIndex++,
			MF_STRING | MF_BYPOSITION, pCmdUI->m_nID++,
			CString(buf) + strTemp);
	}

	// update end menu count
	pCmdUI->m_nIndex--; // point to last menu added
	pCmdUI->m_nIndexMax = pCmdUI->m_pMenu->GetMenuItemCount();

	pCmdUI->m_bEnableChanged = TRUE;    // all the added items are enabled
}

void CRecentFileList::WriteList()
{
	ASSERT(m_arrNames != NULL);
	ASSERT(!m_strSectionName.IsEmpty());
	ASSERT(!m_strEntryFormat.IsEmpty());
	int nLen = m_strEntryFormat.GetLength() + 10;
	LPTSTR pszEntry = new TCHAR[nLen];
	CWinApp* pApp = AfxGetApp();
	pApp->WriteProfileString(m_strSectionName, NULL, NULL);
	for (int iMRU = 0; iMRU < m_nSize; iMRU++)
	{
		_stprintf_s(pszEntry, nLen, m_strEntryFormat, iMRU + 1);
		if (!m_arrNames[iMRU].IsEmpty())
		{
			pApp->WriteProfileString(m_strSectionName, pszEntry,
				m_arrNames[iMRU]);
		}
	}
	delete[] pszEntry;
}

void CRecentFileList::ReadList()
{
	ASSERT(m_arrNames != NULL);
	ASSERT(!m_strSectionName.IsEmpty());
	ASSERT(!m_strEntryFormat.IsEmpty());
	int nLen = m_strEntryFormat.GetLength() + 10;
	LPTSTR pszEntry = new TCHAR[nLen];
	CWinApp* pApp = AfxGetApp();
	for (int iMRU = 0; iMRU < m_nSize; iMRU++)
	{
		_stprintf_s(pszEntry, nLen, m_strEntryFormat, iMRU + 1);
		m_arrNames[iMRU] = pApp->GetProfileString(
			m_strSectionName, pszEntry, _T(""));
	}
	delete[] pszEntry;
}

/////////////////////////////////////////////////////////////////////////////
