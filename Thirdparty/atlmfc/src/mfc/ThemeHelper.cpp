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
// CThemeHelper
void* CThemeHelper::GetProc(LPCSTR szProc, void* pfnFail)
{
	static HMODULE hThemeDll = AfxCtxLoadLibrary(_T("UxTheme.dll"));

	void* pRet = pfnFail;
	if (hThemeDll != NULL)
	{
		void *pFunc = GetProcAddress(hThemeDll, szProc);
		if(pFunc!=NULL)
		{
			pRet=pFunc;
		}
	}
	return pRet;
}

BOOL CThemeHelper::IsAppThemedFail()
{
	return FALSE;
}


HTHEME CThemeHelper::OpenThemeDataFail(HWND , LPCWSTR )
{
	return NULL;
}


HRESULT CThemeHelper::CloseThemeDataFail(HTHEME)
{
	return E_FAIL;
}


HRESULT CThemeHelper::DrawThemeBackgroundFail(HTHEME, HDC, int, int, const RECT *, const RECT *)
{
	return E_FAIL;
}


HRESULT CThemeHelper::GetThemePartSizeFail(HTHEME, HDC, int, int, RECT *, enum THEMESIZE, SIZE *)
{
	return E_FAIL;
}


BOOL CThemeHelper::IsThemeBackgroundPartiallyTransparentFail(HTHEME , int , int )
{
	return FALSE;
}

HRESULT CThemeHelper::DrawThemeParentBackgroundFail(HWND , HDC , RECT *)
{
	return E_FAIL;
}

BOOL CThemeHelper::IsThemePartDefinedFail(HTHEME , int , int )
{
	return FALSE;
}

BOOL CThemeHelper::IsAppThemed()
{
	static PFNISAPPTHEMED pfnIsAppThemed = (PFNISAPPTHEMED)GetProc("IsAppThemed", IsAppThemedFail);
	return (*pfnIsAppThemed)();
}

HTHEME CThemeHelper::OpenThemeData(HWND hwnd, LPCWSTR pszClassList)
{
	static PFNOPENTHEMEDATA pfnOpenThemeData = (PFNOPENTHEMEDATA)GetProc("OpenThemeData", OpenThemeDataFail);
	return (*pfnOpenThemeData)(hwnd, pszClassList);
}

HRESULT CThemeHelper::CloseThemeData(HTHEME hTheme)
{
	static PFNCLOSETHEMEDATA pfnCloseThemeData = (PFNCLOSETHEMEDATA)GetProc("CloseThemeData", CloseThemeDataFail);
	return (*pfnCloseThemeData)(hTheme);
}

HRESULT CThemeHelper::DrawThemeBackground(HTHEME hTheme, HDC hdc, 
	int nPartId, int nStateId, const RECT *pRect, const RECT *pClipRect)
{
	static PFNDRAWTHEMEBACKGROUND pfnDrawThemeBackground = 
		(PFNDRAWTHEMEBACKGROUND)GetProc("DrawThemeBackground", DrawThemeBackgroundFail);
	return (*pfnDrawThemeBackground)(hTheme, hdc, nPartId, nStateId, pRect, pClipRect);
}

HRESULT CThemeHelper::GetThemePartSize(HTHEME hTheme, HDC hdc, 
	int nPartId, int nStateId, RECT * pRect, enum THEMESIZE eSize, SIZE *psz)
{
	static PFNGETTHEMEPARTSIZE pfnGetThemePartSize = 
		(PFNGETTHEMEPARTSIZE)GetProc("GetThemePartSize", GetThemePartSizeFail);
	return (*pfnGetThemePartSize)(hTheme, hdc, nPartId, nStateId, pRect, eSize, psz);
}

BOOL CThemeHelper::IsThemeBackgroundPartiallyTransparent(HTHEME hTheme, int nPartId, int nStateId)
{
	static PFNISTHEMEBACKGROUNDPARTIALLYTRANSPARENT pfnIsThemeBackgroundPartiallyTransparent =
		(PFNISTHEMEBACKGROUNDPARTIALLYTRANSPARENT)GetProc("IsThemeBackgroundPartiallyTransparent", 
															IsThemeBackgroundPartiallyTransparentFail);
	return (*pfnIsThemeBackgroundPartiallyTransparent)(hTheme, nPartId, nStateId);
}

HRESULT CThemeHelper::DrawThemeParentBackground(HWND hwnd, HDC hdc, RECT *prc)
{
	static PFNDRAWTHEMEPARENTBACKGROUND pfnDrawThemeParentBackground =
		(PFNDRAWTHEMEPARENTBACKGROUND)GetProc("DrawThemeParentBackground", DrawThemeParentBackgroundFail);
	return (*pfnDrawThemeParentBackground)(hwnd, hdc, prc);
}

BOOL CThemeHelper::IsThemePartDefined(HTHEME hTheme, int nPartId, int nStateId)
{
	static PFNISTHEMEPARTDEFINED pfnIsThemePartDefined =
		(PFNISTHEMEPARTDEFINED)GetProc("IsThemePartDefined", IsThemePartDefinedFail);
	return (*pfnIsThemePartDefined)(hTheme, nPartId, nStateId);
}
