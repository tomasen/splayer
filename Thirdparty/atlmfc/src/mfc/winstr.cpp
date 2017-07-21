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
// Windows extensions to strings

#ifndef _AFXDLL
int AFXAPI AfxLoadString(_In_ UINT nID, _Out_z_cap_post_count_(nMaxBuf, return + 1) LPSTR lpszBuf, _In_ UINT nMaxBuf)
{
	ASSERT(AfxIsValidAddress(lpszBuf, nMaxBuf*sizeof(CHAR)));
	if( lpszBuf == NULL || nMaxBuf == 0)
		AfxThrowInvalidArgException();

	const ATLSTRINGRESOURCEIMAGE* pImage;
	int nBytes;

	pImage = AtlGetStringResourceImage(AfxGetResourceHandle(), nID);
	if (pImage == NULL)
	{
		lpszBuf[0] = '\0';
		return 0;
	}
	ASSERT(pImage->nLength != 0);
	nBytes = ::WideCharToMultiByte(CP_ACP, 0, pImage->achString, pImage->nLength, lpszBuf, nMaxBuf-1, NULL, NULL);
	lpszBuf[nBytes] = '\0';

	return nBytes;
}

int AFXAPI AfxLoadString(_In_ UINT nID, _Out_z_cap_post_count_(nMaxBuf, return + 1) LPWSTR lpszBuf, _In_ UINT nMaxBuf)
{
	ASSERT(AfxIsValidAddress(lpszBuf, nMaxBuf*sizeof(WCHAR)));
	if( lpszBuf == NULL || nMaxBuf == 0)
		AfxThrowInvalidArgException();

	const ATLSTRINGRESOURCEIMAGE* pImage;
	int nCharsToCopy;

	pImage = AtlGetStringResourceImage(AfxGetResourceHandle(), nID);
	if (pImage == NULL)
	{
		lpszBuf[0] = L'\0';
		return 0;
	}
	ASSERT(pImage->nLength != 0);
	nCharsToCopy = min(nMaxBuf-1, pImage->nLength);
	Checked::memcpy_s(lpszBuf, (nMaxBuf-1)*sizeof(WCHAR), pImage->achString, nCharsToCopy*sizeof(WCHAR));
	lpszBuf[nCharsToCopy] = L'\0';

	return nCharsToCopy;
}

HINSTANCE AFXAPI AfxFindStringResourceHandle(UINT /* nID */)
{
	return AfxGetResourceHandle();
}

#endif

/////////////////////////////////////////////////////////////////////////////

BOOL AFXAPI AfxExtractSubString(CString& rString, LPCTSTR lpszFullString,
	int iSubString, TCHAR chSep)
{
	if (lpszFullString == NULL)
		return FALSE;

	while (iSubString--)
	{
		lpszFullString = _tcschr(lpszFullString, chSep);
		if (lpszFullString == NULL)
		{
			rString.Empty();        // return empty string as well
			return FALSE;
		}
		lpszFullString++;       // point past the separator
	}
	LPCTSTR lpchEnd = _tcschr(lpszFullString, chSep);
	int nLen = (lpchEnd == NULL) ?
		lstrlen(lpszFullString) : (int)(lpchEnd - lpszFullString);
	ASSERT(nLen >= 0);
	Checked::memcpy_s(rString.GetBufferSetLength(nLen), nLen*sizeof(TCHAR),
		lpszFullString, nLen*sizeof(TCHAR));
	rString.ReleaseBuffer();	// Need to call ReleaseBuffer 
								// after calling GetBufferSetLength
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
