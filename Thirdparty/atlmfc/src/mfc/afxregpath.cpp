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
#include "afxregpath.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CString AFXGetRegPath(LPCTSTR lpszPostFix, LPCTSTR lpszProfileName)
{
	ENSURE(lpszPostFix != NULL);

	CString strReg;

	if (lpszProfileName != NULL && lpszProfileName [0] != 0)
	{
		strReg = lpszProfileName;
	}
	else
	{
		CWinApp* pApp = AfxGetApp();
		ASSERT_VALID(pApp);

		ENSURE(AfxGetApp()->m_pszRegistryKey != NULL);
		ENSURE(AfxGetApp()->m_pszProfileName != NULL);

		strReg = _T("SOFTWARE\\");

		CString strRegKey = pApp->m_pszRegistryKey;
		if (!strRegKey.IsEmpty())
		{
			strReg += strRegKey;
			strReg += _T("\\");
		}

		strReg += pApp->m_pszProfileName;
		strReg += _T("\\");
		strReg += lpszPostFix ;
		strReg += _T("\\");
	}

	return strReg;
}

