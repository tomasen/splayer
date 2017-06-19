// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#ifndef __AFX_THEMEHELPER_H__
#define __AFX_THEMEHELPER_H__

#pragma once

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif

/////////////////////////////////////////////////////////////////////////////
class CThemeHelper
{
	typedef BOOL(__stdcall *PFNISAPPTHEMED)();
	typedef HTHEME(__stdcall *PFNOPENTHEMEDATA)(HWND hwnd, LPCWSTR pszClassList);
	typedef HRESULT(__stdcall *PFNCLOSETHEMEDATA)(HTHEME hTheme);
	typedef HRESULT(__stdcall *PFNDRAWTHEMEBACKGROUND)(HTHEME hTheme, HDC hdc, 
		int nPartId, int nStateId, const RECT *pRect, OPTIONAL const RECT *pClipRect);
	typedef HRESULT(__stdcall *PFNGETTHEMEPARTSIZE)(HTHEME hTheme, HDC hdc, 
		int nPartId, int nStateId, RECT * pRect, enum THEMESIZE eSize, OUT SIZE *psz);
	typedef BOOL(__stdcall *PFNISTHEMEBACKGROUNDPARTIALLYTRANSPARENT)(HTHEME hTheme,
		int nPartId, int nStateId);
	typedef HRESULT (__stdcall *PFNDRAWTHEMEPARENTBACKGROUND)(HWND hwnd, HDC hdc, 
		RECT *prc);
	typedef BOOL (__stdcall *PFNISTHEMEPARTDEFINED)(HTHEME hTheme, int nPartId, 
		int nStateId);

	static void* GetProc(LPCSTR szProc, void* pfnFail);
	static BOOL IsAppThemedFail();
	static HTHEME OpenThemeDataFail(HWND , LPCWSTR );
	static HRESULT CloseThemeDataFail(HTHEME);
	static HRESULT DrawThemeBackgroundFail(HTHEME, HDC, int, int, const RECT *, const RECT *);
	static HRESULT GetThemePartSizeFail(HTHEME, HDC, int, int, RECT *, enum THEMESIZE, SIZE *);
	static BOOL IsThemeBackgroundPartiallyTransparentFail(HTHEME , int , int );
	static HRESULT DrawThemeParentBackgroundFail(HWND , HDC , RECT *);
	static BOOL IsThemePartDefinedFail(HTHEME , int , int );

public:
	static BOOL IsAppThemed();
	static HTHEME OpenThemeData(HWND hwnd, LPCWSTR pszClassList);
	static HRESULT CloseThemeData(HTHEME hTheme);
	static HRESULT DrawThemeBackground(HTHEME hTheme, HDC hdc, 
		int nPartId, int nStateId, const RECT *pRect, const RECT *pClipRect);
	static HRESULT GetThemePartSize(HTHEME hTheme, HDC hdc, 
		int nPartId, int nStateId, RECT * pRect, enum THEMESIZE eSize, SIZE *psz);
	static BOOL IsThemeBackgroundPartiallyTransparent(HTHEME hTheme, int nPartId, int nStateId);
	static HRESULT DrawThemeParentBackground(HWND hwnd, HDC hdc, RECT *prc);
	static BOOL IsThemePartDefined(HTHEME hTheme, int nPartId, int nStateId);
};

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#endif // __AFX_THEMEHELPER_H__
