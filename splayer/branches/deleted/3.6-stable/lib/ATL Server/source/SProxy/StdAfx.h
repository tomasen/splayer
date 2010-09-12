//
// stdafx.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#pragma warning(1: 4927)
#pragma warning(1: 4928)

#define _WIN32_WINNT 0x0502

#ifdef _DEBUG
#define ATL_DEBUG_STENCILS
#endif

#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

#include <atlbase.h>
#include <atlpath.h>
#include <msxml2.h>
#include <atlstencil.h>
#include "SproxyColl.h"
#include "resource.h"

#ifdef _DEBUG
#include <crtdbg.h>
#endif

#define _MAKEWIDESTR( str ) L ## str
#define MAKEWIDESTR( str ) _MAKEWIDESTR( str )

#include "Namespaces.h"

typedef CAtlMap<CStringW, CStringW, CStringRefElementTraits<CStringW>, CStringRefElementTraits<CStringW> > NAMESPACEMAP;

const wchar_t * GetWSDLFile();

#include "DiscoMapDocument.h"

CDiscoMapDocument * GetDiscoMapDocument();

#ifdef _DEBUG

inline const TCHAR * GetTabs(DWORD dwLevel)
{
	static TCHAR s_szTabs[2048];

	dwLevel = min(dwLevel, 2047);
	for (DWORD i=0; i<dwLevel; i++)
	{
		s_szTabs[i] = _T('\t');
	}

	s_szTabs[dwLevel] = _T('\0');

	return s_szTabs;
}
#else // _DEBUG

#define GetTabs __noop

#endif // _DEBUG

HRESULT CreateSafeCppName(char **ppszName, const wchar_t *wszName);
HRESULT CreateSafeCppName(char **ppszName, const char *wszName);

HRESULT CreateSafeCppName(CStringA& strSafeName, const wchar_t *wszName);
HRESULT CreateSafeCppName(CStringA& strSafeName, const char *wszName);

#pragma warning(disable:4100)