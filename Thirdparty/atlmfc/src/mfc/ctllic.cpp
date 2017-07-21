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



#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// AfxVerifyLicFile - Checks that a license file exists and contains a
//    specific byte pattern.

BOOL AFXAPI AfxVerifyLicFile(HINSTANCE hInstance, LPCTSTR pszLicFileName,
	LPCOLESTR pszLicFileContents, UINT cch)
{
	// Assume the worst...
	BOOL bVerified = FALSE;

	// Look for license file in same directory as this DLL.
	TCHAR szPathName[_MAX_PATH];
	UINT retVal = ::GetModuleFileName(hInstance, szPathName, _MAX_PATH);
	if (retVal == 0 || retVal == _MAX_PATH)
		return FALSE;

	// Attach the file name in pszLicFileName to the szPathName
	LPTSTR pszFileName = _tcsrchr(szPathName, '\\') + 1;
	if (pszLicFileName != NULL && pszFileName > szPathName &&
		lstrlen(pszLicFileName) < (_MAX_PATH - (pszFileName - szPathName)) ) 
		Checked::tcscpy_s(pszFileName, _countof(szPathName) - (pszFileName - szPathName), pszLicFileName);
	else
		return FALSE;

	LPSTR pszKey = NULL;
	LPBYTE pbContent = NULL;

	TRY
	{
		// Open file, read content and compare.

		CFile file(szPathName, CFile::modeRead);

		if (cch == -1)
			cch = (UINT)wcslen(pszLicFileContents);

		pszKey = (char*)_alloca(cch*2 + 1);
		cch = _wcstombsz(pszKey, pszLicFileContents, cch*2 + 1);

		if (cch != 0)
		{
			--cch;  // license file won't contain the terminating null char
			pbContent = (BYTE*)_alloca(cch);
			file.Read(pbContent, cch);

			if (memcmp(pszKey, pbContent, (size_t)cch) == 0)
				bVerified = TRUE;
		}
	}
	END_TRY

	return bVerified;
}

/////////////////////////////////////////////////////////////////////////////
// Force any extra compiler-generated code into AFX_INIT_SEG

