// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the	
// Active Template Library product.

#include "stdafx.h"
#include "Common.h"
#include "AtlTraceModuleManager.h"

extern HINSTANCE g_hInst;

CAtlTraceModuleInfo::CAtlTraceModuleInfo() :
	m_hInst(NULL),
	m_nCategories(0),
	m_iFirstCategory(-1)
{
	m_szPath[0] = L'\0';
	m_szName[0] = L'\0';
}

void CAtlTraceModuleInfo::Reset(HINSTANCE hInst)
{
	WCHAR szModulePath[MAX_PATH] = {L'\0'};
	
	GetModuleFileNameW(hInst, szModulePath, MAX_PATH);
	
	szModulePath[MAX_PATH - 1] = 0;
	
	ATL_CRT_ERRORCHECK(wcsncpy_s(m_szPath, _countof(m_szPath), szModulePath, _TRUNCATE));
	WCHAR *pszShortName = m_szPath + wcslen(m_szPath);
	while(pszShortName > m_szPath && *(pszShortName - 1) != L'\\')
		pszShortName--;
	ATL_CRT_ERRORCHECK(wcsncpy_s(m_szName, _countof(m_szName), pszShortName, ATL_TRACE_MAX_NAME_SIZE - 1));

	m_hInst = hInst;
	m_nCategories = 0;
	m_iFirstCategory = -1;
}

// Categories
CAtlTraceCategory::CAtlTraceCategory() : 
	m_nModuleCookie(0)
{
	Reset(L"", 0);
}

void CAtlTraceCategory::Reset(const WCHAR *pszName, LONG nModuleCookie)
{
	ATL_CRT_ERRORCHECK(wcsncpy_s(m_szName, _countof(m_szName), pszName, wcslen(pszName)));
	m_nModuleCookie = nModuleCookie;
}

// Modules
CAtlTraceModule::CAtlTraceModule() :
	m_pfnCrtDbgReport(NULL)
{
}

void CAtlTraceModule::CrtDbgReport(CAtlTraceModule::fnCrtDbgReport_t pfnCrtDbgReport)
{
#ifdef _DEBUG
	m_pfnCrtDbgReport = pfnCrtDbgReport ? pfnCrtDbgReport : _CrtDbgReport;
#else
	m_pfnCrtDbgReport = pfnCrtDbgReport ? pfnCrtDbgReport : NULL;
#endif
}

// Processes
CAtlTraceProcess::CAtlTraceProcess(DWORD_PTR dwMaxSize) :
	CAtlTraceModuleInfo(),
	m_dwId(GetCurrentProcessId()), m_nRef(1), m_dwMaxSize(dwMaxSize),
	m_dwFrontAlloc(0), m_dwBackAlloc(0), m_dwCurrFront(0), m_dwCurrBack(0),
	m_nLevel(0), m_bLoaded(false), m_bEnabled(true), m_bFuncAndCategoryNames(false), m_bFileNameAndLineNo(false),
	m_nNextCookie( 0 )
{
	m_pvBase = this;

	Reset( NULL );
}

LONG CAtlTraceProcess::GetNextCookie()
{
	return( ::InterlockedIncrement( &m_nNextCookie ) );
}
