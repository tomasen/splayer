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



/////////////////////////////////////////////////////////////////////////////
// Help and other support

// Strings in format ".....%1 .... %2 ...." etc.

void AFXAPI AfxFormatStrings(CString& rString, UINT nIDS,
		LPCTSTR const* rglpsz, int nString)
{
	CString strFormat;
	if (!strFormat.LoadString(nIDS) != 0)
	{
		TRACE(traceAppMsg, 0, "Error: failed to load AfxFormatString string 0x%04x.\n", nIDS);
		ASSERT(FALSE);
		return;
	}
	AfxFormatStrings(rString, strFormat, rglpsz, nString);
}

void AFXAPI AfxFormatStrings(CString& rString, LPCTSTR lpszFormat,
		LPCTSTR const* rglpsz, int nString)
{
	ENSURE_ARG(lpszFormat != NULL);
	ENSURE_ARG(rglpsz != NULL);
	// determine length of destination string, not including null terminator
	int nTotalLen = 0;
	LPCTSTR pchSrc = lpszFormat;
	while (*pchSrc != '\0')
	{
		if (pchSrc[0] == '%' &&
			 ( (pchSrc[1] >= '1' && pchSrc[1] <= '9') ||
				(pchSrc[1] >= 'A' && pchSrc[1] <= 'Z')) )
		{
			// %A comes after %9 -- we'll need it someday
			int i;
			if (pchSrc[1] > '9')
				i = 9 + (pchSrc[1] - 'A');
			else
				i = pchSrc[1] - '1';
			pchSrc += 2;
			if (i >= nString)
				++nTotalLen;
			else if (rglpsz[i] != NULL)
				nTotalLen += lstrlen(rglpsz[i]);
		}
		else
		{
			if (_istlead(*pchSrc))
				++nTotalLen, ++pchSrc;
			++pchSrc;
			++nTotalLen;
		}
	}

	pchSrc = lpszFormat;
	LPTSTR pchDest = rString.GetBuffer(nTotalLen);
	while (*pchSrc != '\0')
	{
		if (pchSrc[0] == '%' &&
			 ( (pchSrc[1] >= '1' && pchSrc[1] <= '9') ||
				(pchSrc[1] >= 'A' && pchSrc[1] <= 'Z')) )
		{
			// %A comes after %9 -- we'll need it someday
			int i;
			if (pchSrc[1] > '9')
				i = 9 + (pchSrc[1] - 'A');
			else
				i = pchSrc[1] - '1';
			pchSrc += 2;
			if (i >= nString)
			{
				TRACE(traceAppMsg, 0, "Error: illegal string index requested %d.\n", i);
				*pchDest++ = '?';
				nTotalLen--;
			}
			else if (rglpsz[i] != NULL)
			{
				int nLen = lstrlen(rglpsz[i]);
				Checked::tcscpy_s(pchDest, nTotalLen + 1, rglpsz[i]);
				nTotalLen -= nLen;
				pchDest += nLen;
			}
		}
		else
		{
			if (_istlead(*pchSrc))
				*pchDest++ = *pchSrc++, nTotalLen--; // copy first of 2 bytes
			*pchDest++ = *pchSrc++;
			nTotalLen--;
		}
	}
	rString.ReleaseBuffer((int)((LPCTSTR)pchDest - (LPCTSTR)rString));
		// ReleaseBuffer will assert if we went too far
}

void AFXAPI AfxFormatString1(CString& rString, UINT nIDS, LPCTSTR lpsz1)
{
	AfxFormatStrings(rString, nIDS, &lpsz1, 1);
}

void AFXAPI AfxFormatString2(CString& rString, UINT nIDS, LPCTSTR lpsz1,
		LPCTSTR lpsz2)
{
	LPCTSTR rglpsz[2];
	rglpsz[0] = lpsz1;
	rglpsz[1] = lpsz2;
	AfxFormatStrings(rString, nIDS, rglpsz, 2);
}

/////////////////////////////////////////////////////////////////////////////
