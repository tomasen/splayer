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
// OLE UNICODE conversion support

void AFXAPI AfxBSTR2CString(CString* pStr, BSTR bstr)
{
	ASSERT(pStr != NULL);
	if (pStr == NULL)
	{
		return;
	}

	int nLen = SysStringLen(bstr);
#if defined(_UNICODE)
	LPTSTR lpsz = pStr->GetBufferSetLength(nLen);
	ASSERT(lpsz != NULL);
	Checked::memcpy_s(lpsz, nLen*sizeof(TCHAR), bstr, nLen*sizeof(TCHAR));
	pStr->ReleaseBuffer(nLen);
#else
	int nBytes = WideCharToMultiByte(CP_ACP, 0, bstr, nLen, NULL, NULL, NULL,
		NULL);
	LPSTR lpsz = pStr->GetBufferSetLength(nBytes);
	ASSERT(lpsz != NULL);
	WideCharToMultiByte(CP_ACP, 0, bstr, nLen, lpsz, nBytes, NULL, NULL);
	pStr->ReleaseBuffer(nBytes);
#endif
	
}

#if !defined(_UNICODE)
// this function creates a BSTR but it actually has an ANSI string inside
BSTR AFXAPI AfxBSTR2ABSTR(BSTR bstrW)
{
#pragma warning(push)
#pragma warning(disable:4068)
#pragma prefast(push)
#pragma prefast(disable:325, "We want to duplicate NULL semantics on the way out of this function")
	if (bstrW == NULL)
		return NULL;
#pragma prefast(pop)
#pragma warning(pop)

	int nLen = SysStringLen(bstrW); //not including NULL
	int nBytes = WideCharToMultiByte(CP_ACP, 0, bstrW, nLen,
		NULL, NULL, NULL, NULL); //number of bytes not including NULL
	BSTR bstrA = SysAllocStringByteLen(NULL, nBytes); // allocates nBytes
	if(!bstrA)
	{
		AfxThrowMemoryException();
	}
	VERIFY(WideCharToMultiByte(CP_ACP, 0, bstrW, nLen, (LPSTR)bstrA, nBytes, NULL,
		NULL) == nBytes);
	return bstrA;
}

LPWSTR AFXAPI AfxTaskStringA2W(LPCSTR lpa)
{
	LPWSTR lpw = AtlAllocTaskWideString(lpa);
	CoTaskMemFree((void*)lpa);
	return lpw;
}

LPSTR AFXAPI AfxTaskStringW2A(LPCWSTR lpw)
{
	LPSTR lpa = AtlAllocTaskAnsiString(lpw);
	CoTaskMemFree((void*)lpw);
	return lpa;
}

LPDEVMODEW AFXAPI AfxDevModeA2W(LPDEVMODEW lpDevModeW, LPDEVMODEA lpDevModeA)
{
	if (lpDevModeA == NULL)
		return NULL;
	ASSERT(lpDevModeW != NULL);
	AfxA2WHelper(lpDevModeW->dmDeviceName, (LPCSTR)lpDevModeA->dmDeviceName, CCHDEVICENAME);
	Checked::memcpy_s(&lpDevModeW->dmSpecVersion, offsetof(DEVMODEW, dmFormName) - offsetof(DEVMODEW, dmSpecVersion),
		&lpDevModeA->dmSpecVersion, offsetof(DEVMODEW, dmFormName) - offsetof(DEVMODEW, dmSpecVersion));
	AfxA2WHelper(lpDevModeW->dmFormName, (LPCSTR)lpDevModeA->dmFormName, CCHFORMNAME);
	Checked::memcpy_s(&lpDevModeW->dmLogPixels, sizeof(DEVMODEW) - offsetof(DEVMODEW, dmLogPixels), 
		&lpDevModeA->dmLogPixels, sizeof(DEVMODEW) - offsetof(DEVMODEW, dmLogPixels));

	if (lpDevModeA->dmDriverExtra != 0)
	{
		Checked::memcpy_s(lpDevModeW+1, lpDevModeA->dmDriverExtra, 
			lpDevModeA+1, lpDevModeA->dmDriverExtra);
	}
	lpDevModeW->dmSize = sizeof(DEVMODEW);
	return lpDevModeW;
}

LPDEVMODEA AFXAPI AfxDevModeW2A(LPDEVMODEA lpDevModeA, LPDEVMODEW lpDevModeW)
{
	if (lpDevModeW == NULL)
		return NULL;
	ASSERT(lpDevModeA != NULL);
	AfxW2AHelper((LPSTR)lpDevModeA->dmDeviceName, lpDevModeW->dmDeviceName, CCHDEVICENAME*sizeof(char));
	Checked::memcpy_s(&lpDevModeA->dmSpecVersion, offsetof(DEVMODEA, dmFormName) - offsetof(DEVMODEA, dmSpecVersion), 
		&lpDevModeW->dmSpecVersion, offsetof(DEVMODEA, dmFormName) - offsetof(DEVMODEA, dmSpecVersion));
	AfxW2AHelper((LPSTR)lpDevModeA->dmFormName, lpDevModeW->dmFormName, CCHFORMNAME*sizeof(char));
	Checked::memcpy_s(&lpDevModeA->dmLogPixels, sizeof(DEVMODEA) - offsetof(DEVMODEA, dmLogPixels), 
		&lpDevModeW->dmLogPixels, sizeof(DEVMODEA) - offsetof(DEVMODEA, dmLogPixels));

	if (lpDevModeW->dmDriverExtra != 0)
	{
		Checked::memcpy_s(lpDevModeA+1, lpDevModeW->dmDriverExtra, 
			lpDevModeW+1, lpDevModeW->dmDriverExtra);
	}
	lpDevModeA->dmSize = sizeof(DEVMODEA);
	return lpDevModeA;
}

LPTEXTMETRICW AFXAPI AfxTextMetricA2W(LPTEXTMETRICW lptmW, LPTEXTMETRICA lptmA)
{
	if (lptmA == NULL)
		return NULL;
	ASSERT(lptmW != NULL);
	Checked::memcpy_s(lptmW, sizeof(LONG) * 11, lptmA, sizeof(LONG) * 11);
	Checked::memcpy_s(&lptmW->tmItalic, sizeof(BYTE) * 5, &lptmA->tmItalic, sizeof(BYTE) * 5);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&lptmA->tmFirstChar, 1, &lptmW->tmFirstChar, 1);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&lptmA->tmLastChar, 1, &lptmW->tmLastChar, 1);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&lptmA->tmDefaultChar, 1, &lptmW->tmDefaultChar, 1);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&lptmA->tmBreakChar, 1, &lptmW->tmBreakChar, 1);
	return lptmW;
}

LPTEXTMETRICA AFXAPI AfxTextMetricW2A(LPTEXTMETRICA lptmA, LPTEXTMETRICW lptmW)
{
	if (lptmW == NULL)
		return NULL;
	ASSERT(lptmA != NULL);
	Checked::memcpy_s(lptmA, sizeof(LONG) * 11, lptmW, sizeof(LONG) * 11);
	Checked::memcpy_s(&lptmA->tmItalic, sizeof(BYTE) * 5, &lptmW->tmItalic, sizeof(BYTE) * 5);
	WideCharToMultiByte(CP_ACP, 0, &lptmW->tmFirstChar, 1, (LPSTR)&lptmA->tmFirstChar, 1, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, &lptmW->tmLastChar, 1, (LPSTR)&lptmA->tmLastChar, 1, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, &lptmW->tmDefaultChar, 1, (LPSTR)&lptmA->tmDefaultChar, 1, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, &lptmW->tmBreakChar, 1, (LPSTR)&lptmA->tmBreakChar, 1, NULL, NULL);
	return lptmA;
}
#endif
