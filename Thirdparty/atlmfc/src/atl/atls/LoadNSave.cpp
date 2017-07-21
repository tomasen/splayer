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

namespace ATL
{
void WINAPI NotifyTool();

#define TRACE_SETTINGS_EXT ".trc"
#define TRACE_SETTINGS_EXTW L".trc"

static bool WINAPI SetSettings(CAtlTraceSettings *pTraceSettings, UINT nLevel, UINT nStatus)
{
	ATLASSERT(pTraceSettings);
	if(!pTraceSettings)
		return false;

	pTraceSettings->m_nLevel = nLevel;
	switch(nStatus)
	{
	case 0:
		pTraceSettings->m_eStatus = CAtlTraceSettings::Inherit;
		break;
	case 1:
		pTraceSettings->m_eStatus = CAtlTraceSettings::Enabled;
		break;
	case 2:
	default:
		pTraceSettings->m_eStatus = CAtlTraceSettings::Disabled;
		break;
	}
	return true;
}

static bool WINAPI GetSettings(const CAtlTraceSettings &rTraceSettings, UINT *pnStatus)
{
	ATLASSERT(pnStatus);
	if(!pnStatus)
		return false;

	switch(rTraceSettings.m_eStatus)
	{
	case CAtlTraceSettings::Inherit:
		*pnStatus = 0;
		break;
	case CAtlTraceSettings::Enabled:
		*pnStatus = 1;
		break;
	case CAtlTraceSettings::Disabled:
	default:
		*pnStatus = 2;
		break;
	}
	return true;
}

BOOL __stdcall AtlTraceLoadSettingsA(const CHAR *pszFileName, DWORD_PTR dwProcess /* = 0 */)
{
	CHAR szFileName[_MAX_PATH];
	if(!pszFileName)
	{
		CHAR szDrive[_MAX_DRIVE];
		CHAR szDir[_MAX_DIR];
		CHAR szFName[_MAX_FNAME];
		CHAR szExt[_MAX_EXT];

		DWORD dwret = ::GetModuleFileNameA(NULL, szFileName, MAX_PATH);
		if( dwret == 0 || dwret == MAX_PATH )
			return FALSE;
		ATL_CRT_ERRORCHECK(_splitpath_s(szFileName, szDrive, _countof(szDrive), szDir, _countof(szDir), szFName, _countof(szFName), szExt, _countof(szExt)));
		ATL_CRT_ERRORCHECK(strncpy_s(szExt, _MAX_EXT, TRACE_SETTINGS_EXT, sizeof(TRACE_SETTINGS_EXT)));
		ATL_CRT_ERRORCHECK(_makepath_s(szFileName, _MAX_PATH, szDrive, szDir, szFName, szExt));
		pszFileName = szFileName;
	}

	if(pszFileName)
	{
		if(-1 != GetFileAttributesA(pszFileName))
		{
			// file exists
			CHAR szSection[MAX_PATH], szKey[MAX_PATH], szValue[MAX_PATH];
			CHAR szName[MAX_PATH];
			UINT nModules, nCategories, nStatus, nLevel;
			UINT nModule;
			CAtlTraceProcess *pProcess;
			CAtlTraceModule *pModule;
			CAtlTraceCategory *pCategory;
			CHAR *pszProcess = "Process";
			CHAR cEnabled, cFuncAndCategoryNames, cFileNameAndLineInfo;
			CAtlAllocator *pAllocator = &g_Allocator;

			if (dwProcess)
				pAllocator = reinterpret_cast<CAtlAllocator*>(dwProcess);

			pProcess = pAllocator->GetProcess();
			ATLASSERT(pProcess);
			if(!pProcess)
				return FALSE;

			pProcess->m_bLoaded = true;

			::GetPrivateProfileStringA(pszProcess, "Info", "", szValue, MAX_PATH, pszFileName);
			szValue[MAX_PATH - 1] = 0;
			
			if(5 != sscanf_s(szValue, "ModuleCount:%u, Level:%u, Enabled:%c, "
				"FuncAndCategoryNames:%c, FileNameAndLineNo:%c", &nModules, &pProcess->m_nLevel, &cEnabled, sizeof(cEnabled),
				&cFuncAndCategoryNames, sizeof(cFuncAndCategoryNames), &cFileNameAndLineInfo, sizeof(cFileNameAndLineInfo)))
			{
				return FALSE;
			}
			pProcess->m_bEnabled = cEnabled != 'f';
			pProcess->m_bFuncAndCategoryNames = cFuncAndCategoryNames != 'f';
			pProcess->m_bFileNameAndLineNo = cFileNameAndLineInfo != 'f';

			for(UINT i = 0; i < nModules; i++)
			{
				if(-1 == sprintf_s(szKey, MAX_PATH, "Module%d", i+1))
					return FALSE;
				::GetPrivateProfileStringA(pszProcess, szKey, "", szSection, MAX_PATH, pszFileName);
				szSection[MAX_PATH -1] = 0;

				::GetPrivateProfileStringA(szSection, "Name", "", szName, MAX_PATH, pszFileName);
				szName[MAX_PATH -1] = 0;
				if(!pAllocator->FindModule(CA2W(szName), &nModule))
					continue;

				pModule = pAllocator->GetModule(nModule);
				ATLASSERT(pModule);
				if(!pModule)
					continue;

				::GetPrivateProfileStringA(szSection, "Settings", "", szValue, MAX_PATH, pszFileName);
				szValue[MAX_PATH -1] = 0;
				if(3 != sscanf_s(szValue, "CategoryCount:%u, Level:%u, Status:%u", &nCategories, &nLevel, &nStatus))
					continue;

				SetSettings(pModule, nLevel, nStatus);

				for(UINT j = 0; j < nCategories; j++)
				{
					if(-1 == sprintf_s(szKey, MAX_PATH, "Category%d", j+1))
						return FALSE;
					::GetPrivateProfileStringA(szSection, szKey, "", szValue, MAX_PATH, pszFileName);
					szValue[MAX_PATH -1] = 0;
					if(3 != sscanf_s(szValue, "Level:%u, Status:%u, Name:%s", &nLevel, &nStatus, szName, _countof(szName)))
						continue;

					UINT iCategory = pModule->m_iFirstCategory;
					while( iCategory != UINT( -1 ) )
					{
						pCategory = pAllocator->GetCategory(iCategory);

						if( lstrcmpA(CW2A(pCategory->Name()), szName) == 0 )
						{
							SetSettings(pCategory, nLevel, nStatus);
						}
						iCategory = pCategory->m_iNextCategory;
					}
				}
			}
			NotifyTool();
		}
	}
	return TRUE;
}

BOOL __stdcall AtlTraceSaveSettingsA(const CHAR *pszFileName, DWORD_PTR dwProcess /* = 0 */)
{
	ATLASSERT(pszFileName);
	if(!pszFileName)
		return FALSE;

	BOOL bRetVal = FALSE;

	CHAR szKey[MAX_PATH], szValue[MAX_PATH];
	UINT nCategories, nStatus;
	CAtlTraceProcess *pProcess;
	CAtlTraceModule *pModule;
	CAtlTraceCategory *pCategory;
	LPCSTR pszProcess = "Process";
	CAtlAllocator *pAllocator = &g_Allocator;

	if (dwProcess)
		pAllocator = reinterpret_cast<CAtlAllocator*>(dwProcess);

	pProcess = pAllocator->GetProcess();
	ATLASSERT(pProcess);
	if(!pProcess)
		return FALSE;

	bRetVal = TRUE;
	ATLTRACEPROCESSINFO info;
	AtlTraceGetProcessInfo(dwProcess, &info);

	if(-1 == sprintf_s(szValue, MAX_PATH, "ModuleCount:%u, Level:%u, Enabled:%c, "
		"FuncAndCategoryNames:%c, FileNameAndLineNo:%c", info.nModules, pProcess->m_nLevel,
		pProcess->m_bEnabled ? 't' : 'f', pProcess->m_bFuncAndCategoryNames ? 't' : 'f',
		pProcess->m_bFileNameAndLineNo ? 't' : 'f'))
	{
		return FALSE;
	}

	if(::WritePrivateProfileStringA(pszProcess, "Info", szValue, pszFileName) == 0)
		return FALSE;

	for(int i = 0; i <  info.nModules; i++)
	{
		pModule = pAllocator->GetModule(i);
		ATLASSERT(pModule);
		if(!pModule)
			return FALSE;

		if(-1 == sprintf_s(szKey, MAX_PATH, "Module%d", i+1))
			return FALSE;
		if(::WritePrivateProfileStringA(pszProcess, szKey, CW2A(pModule->Name()), pszFileName) == 0)
			return FALSE;
		GetSettings(*pModule, &nStatus);

		nCategories = pAllocator->GetCategoryCount(i);

		if(::WritePrivateProfileStringA(CW2A(pModule->Name()), "Name", CW2A(pModule->Path()), pszFileName) == 0)
			return FALSE;

		if(-1 == sprintf_s(szValue, MAX_PATH, "CategoryCount:%u, Level:%u, Status:%u", nCategories, pModule->m_nLevel, nStatus))
			return FALSE;
		if(::WritePrivateProfileStringA(CW2A(pModule->Name()), "Settings", szValue, pszFileName) == 0)
			return FALSE;

		int j = 0;
		UINT nCategory = pModule->m_iFirstCategory;
		while( nCategory != UINT( -1 ) )
		{
			pCategory = pAllocator->GetCategory(nCategory);

			GetSettings(*pCategory, &nStatus);

			if(-1 == sprintf_s(szKey, MAX_PATH, "Category%d", j+1))
				return FALSE;
			j++;
			if(-1 == sprintf_s(szValue, MAX_PATH, "Level:%u, Status:%u, Name:%S",
				pCategory->m_nLevel, nStatus, pCategory->Name()))
			{
				return FALSE;
			}

			if(::WritePrivateProfileStringA(CW2A(pModule->Name()), szKey, szValue, pszFileName) == 0)
				return FALSE;

			nCategory = pCategory->m_iNextCategory;
		}
	}
	return bRetVal;
}

BOOL __stdcall AtlTraceLoadSettingsU(const WCHAR *pszFileName, DWORD_PTR dwProcess /* = 0 */)
{
	WCHAR szFileName[MAX_PATH];
	if(!pszFileName)
	{
		WCHAR szDrive[_MAX_DRIVE];
		WCHAR szDir[_MAX_DIR];
		WCHAR szFName[_MAX_FNAME];
		WCHAR szExt[_MAX_EXT];

		DWORD dwret = ::GetModuleFileNameW(NULL, szFileName, MAX_PATH);
		if( dwret == 0 || dwret == MAX_PATH )
			return FALSE;
		ATL_CRT_ERRORCHECK(_wsplitpath_s(szFileName, szDrive, _countof(szDrive), szDir, _countof(szDir), szFName, _countof(szFName), szExt, _countof(szExt)));
		ATL_CRT_ERRORCHECK(wcsncpy_s(szExt, _MAX_EXT, TRACE_SETTINGS_EXTW, _countof(TRACE_SETTINGS_EXTW)));
		ATL_CRT_ERRORCHECK(_wmakepath_s(szFileName, MAX_PATH, szDrive, szDir, szFName, szExt));
		pszFileName = szFileName;
	}

	if(pszFileName)
	{
		if(-1 != GetFileAttributesW(pszFileName))
		{
			// file exists
			WCHAR szSection[MAX_PATH], szKey[MAX_PATH], szValue[MAX_PATH];
			WCHAR szName[MAX_PATH];
			UINT nModules, nCategories, nStatus, nLevel;
			UINT nModule;
			CAtlTraceProcess *pProcess;
			CAtlTraceModule *pModule;
			CAtlTraceCategory *pCategory;
			LPCWSTR pszProcess = L"Process";
			WCHAR cEnabled, cFuncAndCategoryNames, cFileNameAndLineInfo;
			CAtlAllocator *pAllocator = &g_Allocator;

			if (dwProcess)
				pAllocator = reinterpret_cast<CAtlAllocator*>(dwProcess);

			pProcess = pAllocator->GetProcess();
			ATLASSERT(pProcess);
			if(!pProcess)
				return FALSE;

			pProcess->m_bLoaded = true;

			::GetPrivateProfileStringW(pszProcess, L"Info", L"", szValue, MAX_PATH, pszFileName);
			szValue[MAX_PATH -1] = 0;
			if(5 != swscanf_s(szValue, L"ModuleCount:%u, Level:%u, Enabled:%c, "
				L"FuncAndCategoryNames:%c, FileNameAndLineNo:%c", &nModules, &pProcess->m_nLevel, &cEnabled, sizeof(cEnabled),
				&cFuncAndCategoryNames, sizeof(cFuncAndCategoryNames), &cFileNameAndLineInfo, sizeof(cFileNameAndLineInfo)))
			{
				return FALSE;
			}
			pProcess->m_bEnabled = cEnabled != L'f';
			pProcess->m_bFuncAndCategoryNames = cFuncAndCategoryNames != L'f';
			pProcess->m_bFileNameAndLineNo = cFileNameAndLineInfo != L'f';

			for(UINT i = 0; i < nModules; i++)
			{
				if(swprintf(szKey, MAX_PATH, L"Module%d", i+1) < 0)
					return FALSE;

				::GetPrivateProfileStringW(pszProcess, szKey, L"", szSection, MAX_PATH, pszFileName);
				szSection[MAX_PATH -1] = 0;
				::GetPrivateProfileStringW(szSection, L"Name", L"", szName, MAX_PATH, pszFileName);
				szName[MAX_PATH -1] = 0;
				if(!pAllocator->FindModule(szName, &nModule))
					continue;

				pModule = pAllocator->GetModule(nModule);
				ATLASSERT(pModule);
				if(!pModule)
					continue;

				::GetPrivateProfileStringW(szSection, L"Settings", L"", szValue, MAX_PATH, pszFileName);
				szValue[MAX_PATH -1] = 0;
				if(3 != swscanf_s(szValue, L"CategoryCount:%u, Level:%u, Status:%u", &nCategories, &nLevel, &nStatus))
					continue;

				SetSettings(pModule, nLevel, nStatus);

				for(UINT j = 0; j < nCategories; j++)
				{
					if(swprintf(szKey, MAX_PATH, L"Category%d", j+1) < 0)
						return FALSE;
					::GetPrivateProfileStringW(szSection, szKey, L"", szValue, MAX_PATH, pszFileName);
					szValue[MAX_PATH -1] = 0;
					if(3 != swscanf_s(szValue, L"Level:%u, Status:%u, Name:%s", &nLevel, &nStatus, szName, _countof(szName)))
						continue;

					UINT iCategory = pModule->m_iFirstCategory;
					while( iCategory != UINT( -1 ) )
					{
						pCategory = pAllocator->GetCategory(iCategory);

						if( lstrcmpW(pCategory->Name(), szName) == 0 )
						{
							SetSettings(pCategory, nLevel, nStatus);
						}
						iCategory = pCategory->m_iNextCategory;
					}
				}
			}
			NotifyTool();
		}
	}
	return TRUE;
}

BOOL __stdcall AtlTraceSaveSettingsU(const WCHAR *pszFileName, DWORD_PTR dwProcess /* = 0 */)
{
	ATLASSERT(pszFileName);
	if(!pszFileName)
		return FALSE;

	BOOL bRetVal = FALSE;

	WCHAR szKey[MAX_PATH], szValue[MAX_PATH];
	UINT nCategories, nStatus;
	CAtlTraceProcess *pProcess;
	CAtlTraceModule *pModule;
	CAtlTraceCategory *pCategory;
	LPCWSTR pszProcess = L"Process";
	CAtlAllocator *pAllocator = &g_Allocator;

	if (dwProcess)
		pAllocator = reinterpret_cast<CAtlAllocator*>(dwProcess);

	pProcess = pAllocator->GetProcess();
	ATLASSERT(pProcess);
	if(!pProcess)
		return FALSE;

	bRetVal = TRUE;
	ATLTRACEPROCESSINFO info;
	AtlTraceGetProcessInfo(dwProcess, &info);

	if(swprintf(szValue, MAX_PATH, L"ModuleCount:%u, Level:%u, Enabled:%c, "
		L"FuncAndCategoryNames:%c, FileNameAndLineNo:%c", info.nModules, pProcess->m_nLevel,
		pProcess->m_bEnabled ? L't' : L'f', pProcess->m_bFuncAndCategoryNames ? L't' : L'f',
		pProcess->m_bFileNameAndLineNo ? L't' : L'f') < 0)
		return FALSE;
		  
	if(::WritePrivateProfileStringW(pszProcess, L"Info", szValue, pszFileName) == 0)
		return FALSE;

	for(int i = 0; i <  info.nModules; i++)
	{
		pModule = pAllocator->GetModule(i);
		ATLASSERT(pModule);
		if(!pModule)
			return FALSE;

		if(swprintf(szKey, MAX_PATH,  L"Module%d", i+1) < 0)
			return FALSE;
		  
		if(::WritePrivateProfileStringW(pszProcess, szKey, pModule->Name(), pszFileName) == 0)
			return FALSE;
		GetSettings(*pModule, &nStatus);

		nCategories = pAllocator->GetCategoryCount(i);

		if(::WritePrivateProfileStringW(pModule->Name(), L"Name", pModule->Path(), pszFileName) == 0)
			return FALSE;
			
		if(swprintf(szValue, MAX_PATH, L"CategoryCount:%u, Level:%u, Status:%u", nCategories, pModule->m_nLevel, nStatus) < 0)
			return FALSE;
		  
		if(::WritePrivateProfileStringW(pModule->Name(), L"Settings", szValue, pszFileName) == 0)
			return FALSE;

		int j = 0;
		UINT nCategory = pModule->m_iFirstCategory;
		while( nCategory != UINT( -1 ) )
		{
			pCategory = pAllocator->GetCategory(nCategory);

			GetSettings(*pCategory, &nStatus);

			if(swprintf(szKey,MAX_PATH, L"Category%d", j+1) < 0)
				return FALSE;
			j++;
			if(swprintf(szValue, MAX_PATH,  L"Level:%u, Status:%u, Name:%s",
				pCategory->m_nLevel, nStatus, pCategory->Name()) < 0)
				return FALSE;
				
			if(::WritePrivateProfileStringW(pModule->Name(), szKey, szValue, pszFileName) == 0)
				return FALSE;

			nCategory = pCategory->m_iNextCategory;
		}
	}
	return bRetVal;
}

}; // namespace ATL
