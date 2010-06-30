//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
#include "stdafx.h"
#include "api.h"
#include "depsettings.h"
#include "resource.h"
#include <adserr.h>


#define INITGUID // must be before iadmw.h
#include <iadmw.h>      // Interface header

// for the IID_IISWebService object
#include "iiisext.h"
#include "iisext_i.c"

#pragma warning(disable:4571) //catch(...) blocks compiled with /EHs do NOT catch or re-throw Structured Exceptions

class CIISMutex
{
public:
	HANDLE hMutex;
	CIISMutex()
	{
		hMutex = ::CreateMutex(NULL, FALSE, _T("VC8_VCDEPLOY"));
	}
	bool Lock()
	{
		printf("Trying to lock mutex %lx\n", (DWORD_PTR)hMutex);
		DWORD dw = WaitForSingleObject(hMutex, 10000);
		return (dw == WAIT_OBJECT_0);
	}
	~CIISMutex()
	{
		printf("Releasing mutex %lx\n", (DWORD_PTR)hMutex);
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
	}
};

HRESULT AddWebSvcExtention(WCHAR* wszRootWeb6, LPWSTR lpwszFileName,VARIANT_BOOL bEnabled,LPWSTR lpwszGroupID,VARIANT_BOOL bDeletableThruUI,LPWSTR lpwszGroupDescription)
{
    HRESULT hrRet = S_FALSE;

    CComPtr<IISWebService> spWeb;
    HRESULT hr = ADsGetObject(wszRootWeb6, IID_IISWebService, (void**)&spWeb);
    if (SUCCEEDED(hr) && spWeb != NULL)
    {
		CComVariant var1, var2;

        var1.vt = VT_BOOL;
        var1.boolVal = bEnabled;

        var2.vt = VT_BOOL;
        var2.boolVal = bDeletableThruUI;

		CComBSTR bstrFileName(lpwszFileName);
		CComBSTR bstrGroupID(lpwszGroupID);
		CComBSTR bstrGroupDescription(lpwszGroupDescription);
        hr = spWeb->AddExtensionFile(bstrFileName,var1,bstrGroupID,var2,bstrGroupDescription);
        if (SUCCEEDED(hr))
        {
            hrRet = S_OK;
        }
        else
        {
            OutputDebugString(_T("failed,probably already exists\r\n"));
        }
        VariantClear(&var1);
        VariantClear(&var2);
    }

    return hrRet;
}

HRESULT RemoveWebSvcExtention(WCHAR* wszRootWeb6, LPWSTR lpwszFileName)
{
    HRESULT hrRet = S_FALSE;

    IISWebService * pWeb = NULL;
    HRESULT hr = ADsGetObject(wszRootWeb6, IID_IISWebService, (void**)&pWeb);
    if (SUCCEEDED(hr) && NULL != pWeb)
    {
		CComBSTR bstrFileName(lpwszFileName);
        hr = pWeb->DeleteExtensionFileRecord(bstrFileName);
        if (SUCCEEDED(hr))
        {
            hrRet = S_OK;
        }
        else
        {
            OutputDebugString(_T("failed,probably already gone\r\n"));
        }
        pWeb->Release();
    }

    return hrRet;
}

HRESULT AddApplicationDependencyUponGroup(WCHAR* wszRootWeb6, LPWSTR lpwszAppName,LPWSTR lpwszGroupID)
{
    HRESULT hrRet = S_FALSE;

    IISWebService * pWeb = NULL;
    HRESULT hr = ADsGetObject(wszRootWeb6, IID_IISWebService, (void**)&pWeb);
    if (SUCCEEDED(hr) && NULL != pWeb)
    {
		CComBSTR bstrAppName(lpwszAppName);
		CComBSTR bstrGroupID(lpwszGroupID);
        hr = pWeb->AddDependency(bstrAppName,bstrGroupID);
        if (SUCCEEDED(hr))
        {
            hrRet = S_OK;
        }
        else
        {
            OutputDebugString(_T("failed,probably already exists\r\n"));
        }
        pWeb->Release();
    }

    return hrRet;
}

HRESULT RemoveApplicationDependencyUponGroup(WCHAR* wszRootWeb6, LPWSTR lpwszAppName,LPWSTR lpwszGroupID)
{
    HRESULT hrRet = S_FALSE;

    IISWebService * pWeb = NULL;
    HRESULT hr = ADsGetObject(wszRootWeb6, IID_IISWebService, (void**)&pWeb);
    if (SUCCEEDED(hr) && NULL != pWeb)
    {
		CComBSTR bstrAppName(lpwszAppName);
		CComBSTR bstrGroupID(lpwszGroupID);
        hr = pWeb->RemoveDependency(bstrAppName,bstrGroupID);
        if (SUCCEEDED(hr))
        {
            hrRet = S_OK;
        }
        else
        {
            OutputDebugString(_T("failed,probably already gone\r\n"));
        }
        pWeb->Release();
    }

    return hrRet;
}

LPSTR _ReverseFind(LPSTR szStr, size_t nLen, char ch)
{
	LPSTR sz = szStr+nLen;
	while (sz >= szStr)
	{
		if (*sz == ch)
		{
			return sz;
		}
		--sz;
	}

	return NULL;
}

BOOL RecursiveCreateDirectoryHelper(LPSTR szPath, size_t nLen)
{
	WIN32_FILE_ATTRIBUTE_DATA fad;
	BOOL bRet;
	LPSTR szCurrent;
	char chTmp;

	memset(&fad, 0x00, sizeof(fad));
	bRet = ::GetFileAttributesExA(szPath, GetFileExInfoStandard, &fad);
	if (!bRet)
	{
		if ((GetLastError() == ERROR_PATH_NOT_FOUND) ||
			(GetLastError() == ERROR_FILE_NOT_FOUND))
		{
			szCurrent = _ReverseFind(szPath, nLen, '\\');
			if (szCurrent != NULL)
			{
				nLen -= (szPath-szCurrent);
				chTmp = *szCurrent;
				*szCurrent = '\0';
				bRet = RecursiveCreateDirectoryHelper(szPath, nLen);
				if (!bRet)
				{
					return bRet;
				}

				*szCurrent = chTmp;
				bRet = ::CreateDirectory(szPath, NULL);
				// VSW#472354 - There is a race condition where if two projects are 
				// building at the same time, the deployment of the other can cause 
				// this directory to get created after we check that it doesn't exist.
				if (!bRet && (GetLastError() != ERROR_ALREADY_EXISTS))
				{
					return bRet;
				}
			}
		}
	}
	return TRUE;
}

BOOL RecursiveCreateDirectory(LPCSTR szDir)
{
	char szPath[MAX_PATH];
	size_t nLen;

	nLen = strlen(szDir);
	if (nLen >= MAX_PATH)
	{
		SetLastError(ERROR_BUFFER_OVERFLOW);
		return FALSE;
	}
	memcpy(szPath, szDir, nLen);
	szPath[nLen] = '\0';

	return RecursiveCreateDirectoryHelper(szPath, nLen);
}

int PrintWarning(unsigned int nMsgID)
{
	return PrintMessage(_T("warning"), nMsgID, NULL);
}

int PrintWarning(unsigned int nMsgID, TCHAR *szExtraInfo)
{
	return PrintMessage(_T("warning"), nMsgID, szExtraInfo);
}

int PrintWarningWithLastWin32(unsigned int nMsgID)
{
	TCHAR szMsg[512];
	if (::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
					NULL,
					GetLastError(),
					NULL,
					szMsg,
					512,
					NULL))
	{
		return PrintWarning(nMsgID, szMsg);
	}
	return PrintWarning(nMsgID);
}

int PrintWarningFormatted(LPCTSTR szWarning)
{
	return PrintMessage(_T("warning"), szWarning, NULL);
}

int PrintError(unsigned int nMsgID)
{
	return PrintMessage(_T("error"), nMsgID, NULL);
}

int PrintError(unsigned int nMsgID, LPCTSTR szExtraInfo)
{
	return PrintMessage(_T("error"), nMsgID, szExtraInfo);
}

int PrintErrorWithLastWin32(unsigned int nMsgID)
{
	TCHAR szMsg[512];
	if (::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
					NULL,
					GetLastError(),
					NULL,
					szMsg,
					512,
					NULL))
	{
		return PrintError(nMsgID, szMsg);
	}
	return PrintError(nMsgID);
}

int PrintErrorFormatted(LPCTSTR szError)
{
	return PrintMessage(_T("error"), szError, NULL);
}

int PrintMessage(LPCTSTR szPrefix, unsigned int nMsgID, LPCTSTR szExtraInfo)
{
	CString strMsg;
	if (!LoadStringFromModule(nMsgID, strMsg))
		_tprintf("Failed to load error string %x\n", nMsgID);
	else
	{
		return PrintMessage(szPrefix, (LPCTSTR)strMsg, szExtraInfo);
	}
	return 0;
}

int PrintMessage(LPCTSTR szPrefix, LPCTSTR szMsg, LPCTSTR szExtraInfo)
{
	if (szExtraInfo)
		_tprintf("vcdeploy : %s %s %s.\n", szPrefix, szMsg, szExtraInfo);
	else
		_tprintf("vcdeploy : %s %s\n", szPrefix, szMsg);

	return ATLSDPLY_SUCCESS;
}

int PrintMessage(unsigned int nMsgID)
{
	CString strMsg;
	if (!LoadStringFromModule(nMsgID, strMsg))
		_tprintf("Failed to load error string %x\n", nMsgID);
	else
	{
		_tprintf("%s\n", static_cast<const char *>(strMsg));
	}
	return 0;
}

bool FormatMsg(CString& strDest, UINT uId, ...)
{
    CString strFmt;
    if (!strFmt.LoadString(uId))
        return false;
	va_list argList;
	va_start( argList, uId );
    strDest.FormatV(strFmt, argList);
	va_end( argList );
	return true;
}

int ProcessSettings(CDepSettings *pSettings)
{
	int result = ATLSDPLY_FAIL;
	long nListLen = 0;
	CComPtr<IXMLDOMNodeList> spHostList;
	if (!pSettings)
	{
		PrintError(IDS_UNEXPECTED);
		return ATLSDPLY_FAIL;
	}

	// Get the list of host's from the settings object
	// We should have already failed loading
	// settings if there was no host list.
	result = pSettings->GetHostList(&spHostList);
	RETURN_ON_UNEXPECTED_HRP(result, spHostList); 
												
	result = spHostList->get_length(&nListLen);
	RETURN_ON_UNEXPECTED(result);

	if (nListLen == 0)
		return ATLSDPLY_SUCCESS; // nothing to do

	// start processing stuff for each host
	CComPtr<IXMLDOMNode> spHostNode;
	CComBSTR bstrHost;
    for (long i = 0; i<nListLen; i++)
	{
		CADSIHelper adsHelper;
		result = spHostList->get_item(i, &spHostNode);
		RETURN_ON_UNEXPECTED_HRP(result, spHostNode);

		result = spHostNode->get_text(&bstrHost);
		RETURN_ON_UNEXPECTED(result);

		if (bstrHost[0] == L'\0')
		{
			PrintWarning(IDS_WARNING_EMPTY_HOST_TAG);
			continue; // empty host name, we'll just press on in this case
		}

		if (S_OK == (result = CADSIHelper::FastLocalRootCheck(pSettings->GetVirtDirName())))
		{
			// VRoot exists
			// Unload IIS as required by settings document
			if (pSettings->GetUnloadBeforeCopy())
			{
				result = LocalW3svcReset();
				RETURN_ON_FAIL(result);
			}

			// do filesystem updates, this includes creating directories for
			// new files and copying filesystem files.
			result = UpdateFileSystem(pSettings);
			RETURN_ON_FAIL(result);

		//	result = CheckMinVRootConfigSettings(pSettings);
		//	if (result == ATLSDPLY_SUCCESS)
		//	{
				// connect to ads to configure the root
				result = CheckVRootExistance(bstrHost, &adsHelper, pSettings);
				RETURN_ON_FAIL(result);

				// configure the virtual directory according to the settings file.
				result = ConfigureVRoot(&adsHelper, pSettings, bstrHost);
				RETURN_ON_FAIL(result);
	
				// Commit any changes in the metabase.
				result = adsHelper.SetInfo();
				RETURN_ON_UNEXPECTED(result);

				result = CADSIHelper::SaveMetabaseData();
				if (result != S_OK)
				{
					CString strMsg;
					FormatMsg(strMsg, IDS_WARNING_FAILED_METABSESAVE, result);
					PrintWarningFormatted(strMsg);
				}

		//	}
			result = RegisterExtension(&adsHelper, pSettings);
			RETURN_ON_FAIL2(result,result);

		}
		else
		{
			// copy system files. Has to be done first so we 
			// don't get errors from IIS.
			result = UpdateFileSystem(pSettings);
			RETURN_ON_FAIL(result);
    		
			// create the virtual directory
			result = CreateVRoot(bstrHost, &adsHelper, pSettings);
			RETURN_ON_FAIL(result);

		//	result = CheckMinVRootConfigSettings(pSettings);
		//	if (result == ATLSDPLY_SUCCESS)
		//	{
				// configure the virtual directory according to the settings file.
				result = ConfigureVRoot(&adsHelper, pSettings, bstrHost);
				RETURN_ON_FAIL(result);
		//	}

			// Commit any changes in the metabase.
			result = adsHelper.SetInfo();
			RETURN_ON_UNEXPECTED(result);

			result = RegisterExtension(&adsHelper, pSettings);
			RETURN_ON_FAIL2(result,result);
			
			// finally, we persist the metabase changes. We don't check
			result = CADSIHelper::SaveMetabaseData();
			if (result != S_OK)
			{
				CString strMsg;
				FormatMsg(strMsg, IDS_WARNING_FAILED_METABSESAVE, result);
				PrintWarningFormatted(strMsg);
			}
			result = ATLSDPLY_SUCCESS;

		}
	}
	return ATLSDPLY_SUCCESS;
}

int ConfigureRestrictionList(CADSIHelper * /*pAdsHelper*/,
							 CDepSettings *pSettings,
							 const wchar_t* wszHost)
{
	HRESULT hr = E_FAIL;
	try
	{
		LPCTSTR szvdfspath = pSettings->GetVirtDirFSPath();
		LPCTSTR szextfilename = pSettings->GetExtensionFileName();

		// need all of these if we are going to get anywhere!
		if (szvdfspath == NULL || *szvdfspath == L'\0' ||
			szextfilename == NULL || *szextfilename == L'\0' ||
			wszHost == NULL || *wszHost == L'\0')
		{
			return ATLSDPLY_SUCCESS;
		}

		CStringW strSvc(L"IIS://");
		strSvc += wszHost;
		strSvc += L"/W3SVC";

		if( pSettings->GetIISMajorVer() < 6 )
			return ATLSDPLY_SUCCESS;


		// create the full path to the isapi
		CPathW strIsapiPath;
		CStringW strVirtDirPath(szvdfspath);
		CStringW strExtFileName(szextfilename);
		strIsapiPath.Combine(strVirtDirPath, strExtFileName);

		CStringW strDescription( pSettings->GetVirtDirName() );

		LPWSTR pwszSvc = const_cast<WCHAR*>((LPCWSTR)strSvc);
		LPWSTR pwszIsapiPath = const_cast<WCHAR*>((LPCWSTR)strIsapiPath);
		LPWSTR pwszDescription = const_cast<WCHAR*>((LPCWSTR)strDescription);

		// Add strIsapiPath to the restrictionlist, make sure it's enabled, 
		// and that the user is able to remove the entry thru the UI if they wanted to
		hr = AddWebSvcExtention( pwszSvc, pwszIsapiPath, VARIANT_TRUE, pwszDescription, VARIANT_TRUE, pwszDescription );

		if( hr == S_OK )
		{
			hr = AddApplicationDependencyUponGroup( pwszSvc, pwszDescription, pwszDescription );
		}

		if( hr == S_FALSE ) // the strIsapiPath has already been on the restriction list.
			hr = S_OK;
	}
	catch(...)
	{
		RETURN_ON_UNEXPECTED(ATLSDPLY_FAIL);
	}

	
	return hr == S_OK ? ATLSDPLY_SUCCESS : ATLSDPLY_FAIL;
}
/*
int ConfigureRestrictionList(CADSIHelper *pAdsHelper,
							 CDepSettings *pSettings,
							 const wchar_t* wszHost)
{
	HRESULT hr = E_FAIL;
	try
	{
		LPCTSTR szvdfspath = pSettings->GetVirtDirFSPath();
		LPCTSTR szextfilename = pSettings->GetExtensionFileName();

		// need all of these if we are going to get anywhere!
		if (szvdfspath == NULL || *szvdfspath == L'\0' ||
			szextfilename == NULL || *szextfilename == L'\0' ||
			wszHost == NULL || *wszHost == L'\0')
		{
			return ATLSDPLY_SUCCESS;
		}

		ATLASSERT(pAdsHelper != NULL);
		if (!pAdsHelper)
			RETURN_ON_UNEXPECTED(ATLSDPLY_FAIL);

		CADSIHelper service;
		CStringW strSvc(L"IIS://");
		strSvc += wszHost;
		strSvc += L"/W3SVC";

		if (S_OK != service.Connect(strSvc))
			RETURN_ON_UNEXPECTED(ATLSDPLY_FAIL);

		CComVariant vIRL;
		hr = service.GetProperty(L"IsapiRestrictionList", vIRL);
		if (hr != S_OK)
		{
			if (hr != E_ADS_PROPERTY_NOT_SUPPORTED && hr != DISP_E_UNKNOWNNAME)
			{
				RETURN_ON_UNEXPECTED(ATLSDPLY_FAIL);
			}
			else
				return ATLSDPLY_SUCCESS; // Property only exists on IIS > 6
		}

		if ( !(vIRL.vt & (VT_ARRAY | VT_VARIANT)) )
			RETURN_ON_UNEXPECTED(ATLSDPLY_FAIL);

		CComSafeArray<VARIANT> saCurrentList;
		saCurrentList.Attach(vIRL.parray);
		vIRL.vt = VT_EMPTY;
		// The first string in the list is either "1" or "0"
		// If it's one, then all ISAPI extensions are allowed to run except
		// the ones in the list. Since this is a new vroot for a new extension
		// we assume our isapi would not be on the list if the first string is
		// "1" so if it is "1", we're done.
		// If it's "0" then no isapis are allowed to run except those on the list
		// so we need to add ourselves to the list. 

		// look at the first string
		CComVariant v = saCurrentList.GetAt(0);
		if (v.vt != VT_BSTR)
			RETURN_ON_UNEXPECTED(ATLSDPLY_FAIL);

		if (!wcscmp(L"1", v.bstrVal))
			return ATLSDPLY_SUCCESS; // we're done, everything is allowed to run

		// create the full path to the isapi
		CPathW strIsapiPath;
		CStringW strVirtDirPath(szvdfspath);
		CStringW strExtFileName(szextfilename);
		strIsapiPath.Combine(strVirtDirPath, strExtFileName);

		// make sure this isapi isn't already in the list and skip
		// the first string because it's either "1" or "0"
		int count = saCurrentList.GetCount(); // count of dim 0
		for (int i=1; i<count; i++)
		{
			CComVariant &v = saCurrentList.GetAt(i);
			if (v.vt == VT_BSTR && !strIsapiPath.m_strPath.Compare(v.bstrVal))
				return ATLSDPLY_SUCCESS;
		}

		// otherwise, add the full path to this isapi to the IRL list
		CComSafeArray<VARIANT> saNewList(saCurrentList); // Copies source safearray to our new array
		if (!saNewList.m_psa)
			RETURN_ON_UNEXPECTED(ATLSDPLY_FAIL);

		hr = E_FAIL;
		CComVariant vNewItem(strIsapiPath);
		hr = saNewList.Add(vNewItem);
		if (hr == S_OK)
		{
			CComVariant vNewList(saNewList); // Copies the safearray into the variant.
			hr = service.SetProperty(L"IsapiRestrictionList", vNewList);
			if (hr == S_OK)
			{
				hr = service.SetInfo();
				if (hr == S_OK)
				{
					// Always notify the user that we have enabled
					// their ISAPI.
					CString strMsg;
					CString strPath(strIsapiPath);
					FormatMsg(strMsg, IDS_UPDATEIRL, (LPCTSTR)strPath);
					_tprintf((LPCTSTR)strMsg);
				}
			}
		}
	}
	catch(...)
	{
		RETURN_ON_UNEXPECTED(ATLSDPLY_FAIL);
	}

	
	return hr == S_OK ? ATLSDPLY_SUCCESS : ATLSDPLY_FAIL;
}
*/

int CheckVRootExistance(const CComBSTR& bstrHostName,
						CADSIHelper *pAdsHelper,
						CDepSettings *pSettings)
{
	ATLASSERT(pAdsHelper);
	ATLASSERT(pSettings);
	if (!pAdsHelper || !pSettings)
		RETURN_ON_UNEXPECTED(ATLSDPLY_FAIL);
	HRESULT hr = E_FAIL;

	try
	{
		CStringW strVirtDirName(pSettings->GetVirtDirName());
		CFixedStringT<CStringW, MAX_PATH> strAdsPathRoot(L"IIS://");
		strAdsPathRoot += bstrHostName;

		TCHAR szBuf[513];
		szBuf[0] = _T('\0');
		DWORD dwBufSize = (sizeof(szBuf)/sizeof(TCHAR))-1;
		// get environment variable for web site (just a number)
		dwBufSize = GetEnvironmentVariable(_T("VCDEPLOY_WEBSITE"), szBuf, dwBufSize);
		if ((!dwBufSize) || (dwBufSize >= (sizeof(szBuf)/sizeof(TCHAR))))
		{
			szBuf[0] = _T('1');
			dwBufSize = 1;
		}
		szBuf[dwBufSize] = _T('\0');

//		strAdsPathRoot += L"/W3SVC/1/ROOT";
		strAdsPathRoot.Append(L"/W3SVC/", sizeof("/W3SVC/")-1);
		strAdsPathRoot.Append(CT2CW(szBuf), dwBufSize);
		strAdsPathRoot.Append(L"/ROOT", sizeof("/ROOT")-1);

		CFixedStringT<CStringW, MAX_PATH> strAdsPathFull(strAdsPathRoot);
		strAdsPathFull += _T('/');
		strAdsPathFull += strVirtDirName;


		// first, see if we can bind to the full path, which would mean the virtual
		// directory already exists
		hr = pAdsHelper->Connect(strAdsPathFull);
		RETURN_ON_UNEXPECTED(hr);
	}
	catch(...)
	{
		// catches CString allocation problems
		RETURN_ON_FAIL2(hr, IDS_ERR_OUTOFMEM);
	}
	return hr;
}


int CreateVRoot(const CComBSTR& bstrHostName,
					CADSIHelper *pAdsHelper,
					CDepSettings *pSettings)
{
	HRESULT hr = E_FAIL;
	ATLASSERT(pAdsHelper);
	ATLASSERT(pSettings);
	if (!pAdsHelper || !pSettings)
		RETURN_ON_UNEXPECTED(ATLSDPLY_FAIL);

	try
	{
		CStringW strVirtDirName(pSettings->GetVirtDirName());
		CFixedStringT<CStringW, MAX_PATH> strAdsPathRoot(L"IIS://");
		strAdsPathRoot += bstrHostName;

		TCHAR szBuf[513];
		szBuf[0] = _T('\0');
		DWORD dwBufSize = (sizeof(szBuf)/sizeof(TCHAR))-1;
		// get environment variable for web site (just a number)
		dwBufSize = GetEnvironmentVariable(_T("VCDEPLOY_WEBSITE"), szBuf, dwBufSize);
		if ((!dwBufSize) || (dwBufSize >= (sizeof(szBuf)/sizeof(TCHAR))))
		{
			szBuf[0] = _T('1');
			dwBufSize = 1;
		}
		szBuf[dwBufSize] = _T('\0');

//		strAdsPathRoot += L"/W3SVC/1/ROOT";
		strAdsPathRoot.Append(L"/W3SVC/", sizeof("/W3SVC/")-1);
		strAdsPathRoot.Append(CT2CW(szBuf), dwBufSize);
		strAdsPathRoot.Append(L"/ROOT", sizeof("/ROOT")-1);

		// Root doesn't exist, try to create it
		hr = pAdsHelper->Connect(strAdsPathRoot);
		RETURN_ON_FAIL2(hr, IDS_ERR_CONNECTADSFAILED);

		short nIso = (short)pSettings->GetAppIsolation();
		if( pSettings->SkipVirtDirCreation() )
		{
			hr = pAdsHelper->CreateAppOnly(strVirtDirName, nIso, NULL);
			RETURN_ON_FAIL2(hr, IDS_ERR_CREATEVROOTFAILED);
		}
		else
		{
			//ATLASSERT(nIso >= 0 && nIso <= 2);
			hr = pAdsHelper->CreateVRoot(strVirtDirName, nIso, NULL);
			RETURN_ON_FAIL2(hr, IDS_ERR_CREATEVROOTFAILED);

			// the path to the root must be set at vroot creation time
			// it can't be accessed later.
			CComVariant val;
			val = pSettings->GetVirtDirFSPath();
			hr = pAdsHelper->SetProperty(L"Path", val);
			RETURN_ON_FAIL2(hr, IDS_ERR_SETADSPROPERTY);
		}
	}
	catch(...)
	{
		// catches CString allocation problems
		RETURN_ON_FAIL2(hr, IDS_ERR_OUTOFMEM);
	}
	return ATLSDPLY_SUCCESS;
}

int ConfigureVRoot(	CADSIHelper *pAdsHelper,
					CDepSettings *pSettings,
					const wchar_t* wszHost)
{
	ATLASSERT(pAdsHelper);
	ATLASSERT(pSettings);
	if (!pAdsHelper || !pSettings)
		return IDS_UNEXPECTED;

	HRESULT hr = E_FAIL;
	CComVariant val;

	// Set some general properties about the vroot
	val.vt = VT_BOOL;
	val.boolVal = VARIANT_TRUE;
	hr = pAdsHelper->SetProperty(L"AccessExecute", val);
	RETURN_ON_FAIL2(hr, IDS_ERR_SETADSPROPERTY);

	val = pSettings->GetVirtDirName();
	hr = pAdsHelper->SetProperty(L"AppPackageName", val);
	RETURN_ON_FAIL2(hr, IDS_ERR_SETADSPROPERTY);

    CComVariant varOrigName;
    hr = pAdsHelper->GetPropertySingle(L"AppFriendlyName", varOrigName);
	if (FAILED(hr) || varOrigName.vt != VT_BSTR || ::SysStringLen(varOrigName.bstrVal) == 0)
    {
    	val = pSettings->GetVirtDirName();
    	hr = pAdsHelper->SetProperty(L"AppFriendlyName", val);
    	RETURN_ON_FAIL2(hr, IDS_ERR_SETADSPROPERTY);
    }

	hr = SetRootAppMappings(pAdsHelper, pSettings);
	RETURN_ON_FAIL(hr);
	hr = ConfigureRestrictionList(pAdsHelper, pSettings, wszHost);
	return hr;
}

LONG FindMapping(CComSafeArray<VARIANT> *pArray, BSTR bstrExt)
{
	// if the array is empty, then the entry is not there
	if (!pArray->m_psa)
		return -1;

	// find an item that starts with bstrExt followed by ','
	CComBSTR bstrFullExt(bstrExt);
	bstrFullExt.Append(",");
	size_t nLen = wcslen(bstrFullExt);
	
	ULONG ulCount = pArray->GetCount();
	for (LONG lIndex=0; lIndex < (LONG) ulCount; lIndex++)
	{
		CComVariant var;
		var = pArray->GetAt(lIndex);
		if (var.vt == VT_BSTR && !_wcsnicmp(bstrFullExt, var.bstrVal, nLen))
			return lIndex;
	}
	return -1;
}

int SetRootAppMappings(	CADSIHelper *pAdsHelper,
						CDepSettings *pSettings)
{
	ATLASSERT(pAdsHelper);
	ATLASSERT(pSettings);
	if (!pAdsHelper || !pSettings)
		return IDS_UNEXPECTED;

	LPCTSTR szvdfspath = pSettings->GetVirtDirFSPath();
	LPCTSTR szextfilename = pSettings->GetExtensionFileName();
	if ((!szvdfspath || *szvdfspath == _T('\0') ) ||
		(!szextfilename || *szextfilename == _T('\0') )
		)
		return ATLSDPLY_SUCCESS; // not enough info!

	HRESULT hr = E_FAIL;
	CStringW rgMappings[MAX_VERB_BLOCKS];
	CComPtr<IXMLDOMNodeList> spMappingNodes;
	CPathW strIsapiPath;
	CStringW strVirtDirPath(szvdfspath);
	CStringW strExtFileName(szextfilename);
	strIsapiPath.Combine(strVirtDirPath, strExtFileName);
	hr = pSettings->GetAppMappingList(&spMappingNodes);
	RETURN_ON_UNEXPECTED(hr);

	if (!spMappingNodes)
		return ATLSDPLY_SUCCESS; // nothing to do
	
	long nMappingCount = 0, i=0;
	hr = spMappingNodes->get_length(&nMappingCount);
	RETURN_ON_UNEXPECTED(hr);

	if (nMappingCount <= 0)
		return ATLSDPLY_SUCCESS; // nothing to do

	if (nMappingCount > MAX_VERB_BLOCKS)
		RETURN_ON_FAIL2(ATLSDPLY_FAIL, ATLS_ERR_TOOMANYVERBBLOCKS);

	// get the current script mappings
	CComVariant vCurrentMappings;
	CComSafeArray<VARIANT> saCurrentMappings;

	hr = pAdsHelper->GetProperty(L"ScriptMaps", vCurrentMappings);
	if (SUCCEEDED(hr) && (vCurrentMappings.vt == (VT_ARRAY|VT_VARIANT)))
	{
		VARIANT vVal;
		VariantInit(&vVal);
		vCurrentMappings.Detach(&vVal);
		saCurrentMappings.Attach(vVal.parray);
	}

	CComBSTR strAttrName(L"fileext");
    // loop through the nodes and register the extensions
	for (i = 0; i<nMappingCount; i++)
	{
		CComPtr<IXMLDOMNode> spMappingNode, spExtAttr;
		CComPtr<IXMLDOMNamedNodeMap> spAttributes;
		CComBSTR bstrExt;
		hr = spMappingNodes->get_item(i, &spMappingNode);
		RETURN_ON_UNEXPECTED_HRP(hr, spMappingNode);

		// get the fileext attribute
		hr = spMappingNode->get_attributes(&spAttributes);
		RETURN_ON_FAIL2_HRP(hr, spAttributes, IDS_FILEEXTATTR_NOTFOUND);

		hr = spAttributes->getNamedItem(strAttrName, &spExtAttr);
		RETURN_ON_FAIL2_HRP(hr, spAttributes, IDS_FILEEXTATTR_NOTFOUND);

		hr = spExtAttr->get_text(&bstrExt);
		RETURN_ON_FAIL2(hr, IDS_FILEEXTATTR_INVALID);

		if (bstrExt[0] != L'.')
		{
			CComBSTR bstrTemp(L".");
			bstrTemp += bstrExt;
			bstrExt = bstrTemp;
		}

		// get the list of verbs to map to
		CComPtr<IXMLDOMNodeList> spExtVerbList;
		long nVerbs = 0;
		CComBSTR rgVerbs[MAX_VERB_COUNT];

		hr = spMappingNode->selectNodes(CComBSTR(L"VERB"), &spExtVerbList);
		if (hr != S_OK)
			continue; // no verbs, nothing to do

		RETURN_ON_UNEXPECTED_P(spExtVerbList);

		hr = spExtVerbList->get_length(&nVerbs);
		RETURN_ON_UNEXPECTED(hr);

		if (nVerbs == 0)
			continue; // no verbs, nothing to do

		if (nVerbs > MAX_VERB_COUNT)
		{
			PrintError(IDS_ERR_TOOMANYVERBS);
			return ATLSDPLY_FAIL;
		}

		CStringW strEntry;
		strEntry = bstrExt;
		strEntry += L",";
		strEntry += strIsapiPath;

		
		if (pSettings->GetIISMajorVer() > 4)
		{
			strEntry += L",0,"; // we already know nVerbs>0
			for (long z = 0; z<nVerbs; z++)
			{
				CComPtr<IXMLDOMNode> spVerbNode;
				CComBSTR bstrVerb;
				hr = spExtVerbList->get_item(z, &spVerbNode);
				RETURN_ON_UNEXPECTED(hr);

				hr = spVerbNode->get_text(&bstrVerb);
				RETURN_ON_UNEXPECTED(hr);

				strEntry += bstrVerb;
				if (z < nVerbs-1)
					strEntry +=  L',';
			}
		}
		else
			strEntry += L",0";
		LONG lIndex = FindMapping(&saCurrentMappings, bstrExt);
		if (lIndex >= 0)
		{
			// update the entry in the array
			CComVariant v(strEntry);
			saCurrentMappings.SetAt(lIndex, v, TRUE);
		}
		else
		{
			// add as a new entry
			rgMappings[i] = strEntry;
		}

#ifdef _DEBUG
		CString strOut;
		strOut.Format("Adding mapping %s\n",CW2A(strEntry));
		printf(strOut);
		ATLTRACE(L"Adding script mapping %s\n", strEntry);
#endif
	}

	CComSafeArray<VARIANT> rgsaMappings;
	for ( long y = 0; y<nMappingCount; y++)
	{
		if (rgMappings[y].GetLength() != 0)
		{
			if (S_OK != rgsaMappings.Add(CComVariant(CComBSTR(rgMappings[y]))))
				RETURN_ON_UNEXPECTED(E_FAIL);
		}
	}
	// set the safearray of BSTR 
	if (saCurrentMappings.m_psa)
	{
		if (rgsaMappings.m_psa)
			saCurrentMappings.Add(rgsaMappings);
		CComVariant vSafearray(saCurrentMappings);
		vSafearray.vt=VT_ARRAY|VT_VARIANT;
		hr = pAdsHelper->SetProperty(L"ScriptMaps", vSafearray);
	}
	else
	{
		CComVariant vSafearray(rgsaMappings);
		vSafearray.vt=VT_ARRAY|VT_VARIANT;
		hr = pAdsHelper->SetProperty(L"ScriptMaps", vSafearray);
	}
	RETURN_ON_FAIL2(hr, IDS_ERR_SETADSPROPERTY);
	return hr;
}

int LocalW3svcReset()
{
	// There is a race condition that can happen when deploying two projects
	// at the same time.  We need to stop/restart IIS as a single operation.
	// Use a named mutex that works across processes to serialize this action.
	CIISMutex iisMutex;
	iisMutex.Lock();

	DWORD dwRet = 0;
	// open service control manager
	SC_HANDLE hScm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT|SC_MANAGER_ENUMERATE_SERVICE);
	if (hScm==NULL)
	{
		PrintErrorWithLastWin32(IDS_FAILEDOPENSCM);
		return ATLSDPLY_FAIL;
	}

	// open W3SVC
	SC_HANDLE hW3Svc = OpenService(hScm, _T("W3SVC"), SERVICE_QUERY_STATUS|SERVICE_START|SERVICE_STOP);
	if (hW3Svc==NULL)
	{
		PrintErrorWithLastWin32(IDS_FAILEDOPENSVC);
		CloseServiceHandle(hScm);
		return ATLSDPLY_FAIL;
	}

	// stop W3SVC
	SERVICE_STATUS svcStatus;
	ZeroMemory(&svcStatus, sizeof(svcStatus));
	QueryServiceStatus(hW3Svc, &svcStatus);

	if (svcStatus.dwCurrentState == SERVICE_RUNNING)
	{
		ZeroMemory(&svcStatus, sizeof(svcStatus));
		if (!ControlService(hW3Svc, SERVICE_CONTROL_STOP, &svcStatus) &&
			GetLastError() != ERROR_SERVICE_NOT_ACTIVE)
		{
			// couldn't send the control service command plus
			// the service isn't already stopped.
			PrintErrorWithLastWin32(IDS_FAILEDSTOPCOMMAND);
			CloseServiceHandle(hW3Svc);
			CloseServiceHandle(hScm);
			return ATLSDPLY_FAIL;
		}
	}

	
	if (svcStatus.dwCurrentState == SERVICE_STOP_PENDING)
	{
		printf("stopping W3SVC.");
		// Stop is pending, we'll have to wait
		for (int i=0;i<1000; i++)
		{
			// sleep
			//Sleep(svcStatus.dwWaitHint != 0 ? svcStatus.dwWaitHint : 100);
			Sleep(50);
			printf(".");
						
			//interogate
			ZeroMemory(&svcStatus, sizeof(svcStatus));
			if (!QueryServiceStatus(hW3Svc,  &svcStatus))
			{
				PrintErrorWithLastWin32(IDS_FAILEDQUERYSTATUS);
				CloseServiceHandle(hW3Svc);
				CloseServiceHandle(hScm);
				return ATLSDPLY_FAIL;
			}
			if (svcStatus.dwCurrentState == SERVICE_STOPPED)
			{
				// service is stopped
				break;
			}
		}
		printf("\n");
	}
	
	int nRetryCount=0;
	if (svcStatus.dwCurrentState == SERVICE_STOPPED)
	{
		printf("starting W3SVC.");
    	ZeroMemory(&svcStatus, sizeof(svcStatus));
    	if (!StartService(hW3Svc, NULL, NULL))
    	{
    		PrintErrorWithLastWin32(IDS_FAILEDSTARTSVC);
    		CloseServiceHandle(hW3Svc);
    		CloseServiceHandle(hScm);
    		return ATLSDPLY_FAIL;
    	}
    	// check for pending status
    	ZeroMemory(&svcStatus, sizeof(svcStatus));
    	if (!QueryServiceStatus(hW3Svc, &svcStatus))
    	{
    		PrintErrorWithLastWin32(IDS_FAILEDQUERYSTATUS);
    		CloseServiceHandle(hW3Svc);
    		CloseServiceHandle(hScm);
    		return ATLSDPLY_FAIL;
    	}

		if (svcStatus.dwCurrentState == SERVICE_START_PENDING)
		{

			// Start is pending, we'll have to wait.
			for (int i=0;i<1000; i++)
			{
				// sleep
				// Sleep(svcStatus.dwWaitHint != 0 ? svcStatus.dwWaitHint : 100);
				Sleep(50); //polling is faster than using their hint
				//interogate
				printf(".");
				ZeroMemory(&svcStatus, sizeof(svcStatus));
				if (!QueryServiceStatus(hW3Svc,  &svcStatus))
				{
					PrintErrorWithLastWin32(IDS_FAILEDQUERYSTATUS);
					CloseServiceHandle(hW3Svc);
					CloseServiceHandle(hScm);
					return ATLSDPLY_FAIL;
				}
				
				//success?
				if (svcStatus.dwCurrentState == SERVICE_RUNNING)
				{
					// service is running! We're done
					printf("\n");
					break;
				}
				else if (svcStatus.dwCurrentState == SERVICE_STOPPED)
				{
					if (nRetryCount > 50)
						break;
					Sleep(100);
			    	if (!StartService(hW3Svc, NULL, NULL))
    				{
    					PrintErrorWithLastWin32(IDS_FAILEDSTARTSVC);
    					CloseServiceHandle(hW3Svc);
    					CloseServiceHandle(hScm);
    					return ATLSDPLY_FAIL;
    				}
					nRetryCount++;
				}
			}
		}
	}
	else
	{
		// it never stopped for some reason
		PrintError(IDS_ERR_NOSTOPW3SVC);
		dwRet = ATLSDPLY_FAIL;
	}
	
	// final check to see if it's running
	if (svcStatus.dwCurrentState != SERVICE_RUNNING)
	{
		// service should be running by now
		ATLASSERT(FALSE);
		PrintError(IDS_ERR_W3SVCFAILEDTOSTART);
		dwRet = ATLSDPLY_FAIL;
	}
	
	CloseServiceHandle(hW3Svc);
	CloseServiceHandle(hScm);
	return dwRet;
}

int UpdateFileSystem(CDepSettings *pSettings)
{
	CComPtr<IXMLDOMNodeList> spFileGroupsList;
	HRESULT hr = E_FAIL;
	long nAppFileGroups = 0, i=0;

	// Create the vroot root directory
	CPath FSPath(pSettings->GetVirtDirFSPath());

	if (FSPath.m_strPath.GetLength() != 0 && !FSPath.IsDirectory())
	{	
		if (!CreateDirectory(FSPath, NULL))
		{
			PrintErrorWithLastWin32(IDS_ERR_CREATING_DIRECTORY);
			return ATLSDPLY_FAIL;
		}
	}
	else if (FSPath.m_strPath.GetLength() == 0)
	{
		PrintError(IDS_ERR_CREATING_DIRECTORY);
		return ATLSDPLY_FAIL;
	}
	
	hr = pSettings->GetFileGroups(&spFileGroupsList);
	RETURN_ON_UNEXPECTED_HRP(hr, spFileGroupsList);

	//loop through each app file group
	hr = spFileGroupsList->get_length(&nAppFileGroups);
	RETURN_ON_UNEXPECTED(hr);

	if (nAppFileGroups <= 0)
		return ATLSDPLY_SUCCESS; // nothing to do

	for (i=0; i<nAppFileGroups; i++)
	{
		CComPtr<IXMLDOMNode> spAppFileGroupNode;	
		CComPtr<IXMLDOMNodeList> spFileNodeList;
		long nFileNameNodes = 0;

		hr = spFileGroupsList->get_item(i, &spAppFileGroupNode);
		RETURN_ON_UNEXPECTED_HRP(hr, spAppFileGroupNode);

		hr = spAppFileGroupNode->selectNodes(CComBSTR(L"APPFILENAME"), &spFileNodeList);
		RETURN_ON_UNEXPECTED_HRP(hr, spFileNodeList);

		hr = spFileNodeList->get_length(&nFileNameNodes);
		RETURN_ON_UNEXPECTED(hr);

		if (nFileNameNodes <=0)
			continue; // no files in node.

		for (long x=0; x<nFileNameNodes; x++)
		{
			// for each APPFILENAME in the APPFILEGROUP get the 
			// SRC and DEST tag and copy the file
			CComPtr<IXMLDOMNode> spFileNode, spSrc, spDest;
			CComBSTR bstrSrcPath, bstrDestPath;
			hr = spFileNodeList->get_item(x, &spFileNode);
			RETURN_ON_UNEXPECTED_HRP(hr, spFileNode);

			hr = spFileNode->selectSingleNode(CComBSTR(L"SRC"), &spSrc);
			RETURN_ON_UNEXPECTED_HRP(hr, spSrc);

			hr = spFileNode->selectSingleNode(CComBSTR(L"DEST"), &spDest);
			RETURN_ON_UNEXPECTED_HRP(hr, spDest);

			hr = spSrc->get_text(&bstrSrcPath);
			RETURN_ON_UNEXPECTED(hr);

			hr = spDest->get_text(&bstrDestPath);
			RETURN_ON_UNEXPECTED(hr);

			// src is a full path
			// dest is a path relative to the root of the vroot path
			// Calculate the full path of the file, then see if the
			// directory exists. If it doesn't, create it.
			CPath vrDestDir;
			vrDestDir.Combine(pSettings->GetVirtDirFSPath(), CW2T(bstrDestPath));
			CPath vrDestPath(vrDestDir);
			vrDestDir.RemoveFileSpec();
			if (!vrDestDir.IsDirectory())
			{
				if (!RecursiveCreateDirectory(vrDestDir))
				{
					PrintErrorWithLastWin32(IDS_ERR_CREATING_DIRECTORY_RELATIVE);
					return ATLSDPLY_FAIL;
				}
			}

			CW2T szDest(bstrSrcPath);
			// Copy the file
			int result = CheckFileDiff(szDest, vrDestPath);
			if (result == S_FALSE)
			{
				if (!::CopyFile(szDest, vrDestPath, FALSE))
				{
					PrintWarningWithLastWin32(IDS_WARN_COPYING_FILE);
				}
				else
				{
				    CString strMsg;
				    FormatMsg(strMsg, IDS_COPYFILE_MESSAGE, (LPCTSTR)szDest, (LPCTSTR)vrDestPath);
				    _fputts(static_cast<const TCHAR *>(strMsg),stdout);
					ATLTRACE("Copied %s to %s\n", (LPCTSTR)CW2T(bstrSrcPath), (LPCTSTR)vrDestPath);
				}
				
				// Remove the readonly attribute on the destination file
				// if it is set.
				DWORD dwAttrs = GetFileAttributes(vrDestPath);
				if (dwAttrs != 0xFFFFFFFF)
				{
					if (dwAttrs & FILE_ATTRIBUTE_READONLY)
					{
						dwAttrs &= ~FILE_ATTRIBUTE_READONLY;
						if (!SetFileAttributes(vrDestPath, dwAttrs))
						{
							ATLTRACE(_T("Failed to remove readonly attribute of target file %s\n"), vrDestPath);
						}
					}
				}
			}
			else if (result == S_OK)
			{
			    CString strMsg;
			    FormatMsg(strMsg, IDS_FILES_IDENTICAL, (LPCTSTR) szDest);
			    _fputts(static_cast<const TCHAR *>(strMsg), stdout);
			}
		}
	}
	return ATLSDPLY_SUCCESS;
}

int RegisterExtension(CADSIHelper* /*pAdsHelper*/,
					  CDepSettings *pSettings)
{

	if (!pSettings->GetRegIsapi())
		return ATLSDPLY_SUCCESS; // isapi doesn't need to be registered

	LPCTSTR szVirtDirPath = pSettings->GetVirtDirFSPath();
	LPCTSTR szExtFileName = pSettings->GetExtensionFileName();

	if (!szVirtDirPath || *szVirtDirPath == _T('\0'))
	{
        PrintWarning(IDS_ERR_REGISTERING_NOVDIRPATH);
		return ATLSDPLY_SUCCESS;
	}

	if (!szExtFileName || *szExtFileName == _T('\0'))
	{
		PrintWarning(IDS_ERR_REGISTERING_NOEXTFILE);
		return ATLSDPLY_SUCCESS;
	}
	CPathW strIsapiPath;
	int nRet = IDS_ERR_REGISTERING_EXTENSION;

	try
	{

	CStringW strVirtDirPath(szVirtDirPath);
	CStringW strExtFileName(szExtFileName);
	strIsapiPath.Combine(strVirtDirPath, strExtFileName);
	}
	catch(...)
	{
		// probably an allocation problem with the CStrings
		return nRet;
	}

	HINSTANCE hInstExtension = ::LoadLibraryW(strIsapiPath);
	if (!hInstExtension)
		return IDS_ERR_REGISTERING_EXTENSION;

	PFNRegisterServer pfnRegister = (PFNRegisterServer)GetProcAddress(hInstExtension, _T("DllRegisterServer"));
	if (pfnRegister)
	{
		if (S_OK == pfnRegister())
			nRet = ATLSDPLY_SUCCESS;
	}
	else
		return IDS_ERR_REGISTERING_EXTENSION;
	FreeLibrary(hInstExtension);
	return nRet;
}

BOOL LoadStringFromModule(int nID, CString& str)
{		
	if (!str.LoadString(nID))
		return FALSE;
	return TRUE;
}

int CheckMinVRootConfigSettings(CDepSettings *pSettings)
{
	if (!pSettings)
		return IDS_UNEXPECTED;
	
	// For now, if we don't have an ISAPI extension file name
	// or we don't have any script mappings, we can't create an
	// atls vroot
	LPCTSTR szExtensionFile = pSettings->GetExtensionFileName();
	if (!szExtensionFile || *szExtensionFile == _T('\0'))
		return ATLSDPLY_FAIL;

	CComPtr<IXMLDOMNodeList> spList;
	HRESULT hr = pSettings->GetAppMappingList(&spList);
	if (hr != S_OK && spList)
		return ATLSDPLY_FAIL;

	return ATLSDPLY_SUCCESS;
}

int RuntimeCheck()
{
	CComPtr<IMSAdminBase> spAdmBase;
	HRESULT hr = spAdmBase.CoCreateInstance(CLSID_MSAdminBase);
	if (hr != S_OK)
		return ATLSDPLY_FAIL;
	return ATLSDPLY_SUCCESS;
}

// This function checks the length of the files and then does
// a binary check to see if the files are the same. Since we could
// be copying accross different file systems and the timestamp resolution
// even on our own filesystems isn't that great I decided to do a binary
// compare to see if the files are different. It's expensive in terms of
// memory usage but this process should still run pretty fast because there
// shouldn't be that many files to check and the files shouldn't be that big.
// We don't support checking of files of size greater than ULONG_MAX (4 gig).
HRESULT CheckFileDiff(LPCTSTR szSrcPath, LPCTSTR szDestPath)
{
	CAtlFile fSrc, fDest;
	HRESULT hr = E_FAIL;

	if (S_OK == fSrc.Create(szSrcPath, GENERIC_READ, 0, OPEN_EXISTING))
	{
		if (S_OK == fDest.Create(szDestPath, GENERIC_READ, 0, OPEN_EXISTING))
		{
			ULONGLONG lenSrc=0,lenDest=0;
			if (S_OK == fSrc.GetSize(lenSrc) && 
				S_OK == fDest.GetSize(lenDest))
			{
				if (lenSrc != lenDest)
					hr = S_FALSE;
				else
				{
					if (lenSrc > ULONG_MAX ||
						lenDest > ULONG_MAX)
					{
						hr = E_FAIL;
					}
					else
					{
						if (lenSrc > ULONG_MAX || 
							lenDest > ULONG_MAX)
							hr = E_FAIL;
						else
						{
							CHeapPtr<BYTE> pBuffSrc;
							CHeapPtr<BYTE> pBuffDest;
							hr = S_OK;
							if (!pBuffSrc.Allocate((DWORD)lenSrc))
								hr = S_FALSE;
							if (hr == S_OK && !pBuffDest.Allocate((DWORD)lenDest))
								hr = S_FALSE;
							if (hr == S_OK && S_OK != fSrc.Read((LPVOID)(BYTE*)pBuffSrc, (DWORD)lenSrc))
								hr = S_FALSE;
							if (hr == S_OK && S_OK != fDest.Read((LPVOID)(BYTE*)pBuffDest, (DWORD)lenDest))
								hr = S_FALSE;
							if (hr == S_OK)
								hr = memcmp((LPVOID)(BYTE*)pBuffSrc, (LPVOID)(BYTE*)pBuffDest, (DWORD)lenDest) != 0 ? S_FALSE : S_OK;
						}
					}

				}
			}
		}
		else
			hr = S_FALSE; // dest doesn't exist
	}
	else
	{
		CString strFormat;
		DWORD dwLast = GetLastError();
		if (dwLast == ERROR_FILE_NOT_FOUND &&
		    strFormat.LoadString(IDS_WARN_SOURCE_NOT_EXIST))
		{
			CString strMsg;
			strMsg.Format(strFormat, szSrcPath);
			PrintWarningFormatted(strMsg);
		}
		else
		{
			if (strFormat.LoadString(IDS_WARN_SOURCE_ACCESS_ERROR))
			{
				CString strMsg;
				strMsg.Format(strFormat, dwLast);
				PrintWarningFormatted(strMsg);
			}
			
		}
	}
	return hr;
}

HRESULT ProcessAccessCheck()
{
	try
	{
		HANDLE hToken = NULL;
		// Current token will impersonate self
		if (!ImpersonateSelf(SecurityImpersonation))
			return 0;
			
		if(!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hToken))
		{
			// per Q118626, we check the process token
			// if there is no thread token
			if (GetLastError() != ERROR_NO_TOKEN)
				return E_FAIL;
			else
			{
				if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
					return E_FAIL;
			}
		}
		
		ATLASSERT(hToken);
		CAccessToken myToken;
		bool bIsMember = false;
		myToken.Attach(hToken);
		if (myToken.CheckTokenMembership(Sids::Admins(), &bIsMember))
			return bIsMember ? S_OK : S_FALSE;
		
	}
	catch(...){	}
	return E_UNEXPECTED; // should have returned something before here.
}

HRESULT GetWWWRootPath(const CStringW strWebHostName,CStringW& strPath)
{           
      CComPtr<IEnumVARIANT> spEnum; 
      CComPtr<IADsContainer> spCont;
      CStringW strIISPath=L"IIS://";
      strIISPath+=strWebHostName;
      strIISPath+=L"/W3SVC";
      HRESULT hr=ADsGetObject(strIISPath.GetString(), IID_IADsContainer, (void**) &spCont);
      CComPtr<IADs> spDeploySite;
      if (SUCCEEDED(hr) && spCont!=NULL)
      {
		CComPtr<IUnknown> spUnk;
        hr=spCont->get__NewEnum(&spUnk);
        if (SUCCEEDED(hr))
        {
			spCont.Release();             
            hr=spUnk.QueryInterface(&spEnum);
			if (SUCCEEDED(hr))
			{
				spUnk.Release();
				CComVariant var;                    
				CComPtr<IDispatch> spDisp;
				ULONG lFetch = 0;                   
				// Enumerate children of IIS service, searching for absolute path to "Default Web Site"
				// or if not found, the first web site.
				hr = spEnum->Next(1, &var, &lFetch);
				while(SUCCEEDED(hr) && lFetch > 0)
				{                             
					spDisp = V_DISPATCH(&var);
					CComPtr<IADs> spADs;                      
					hr=spDisp.QueryInterface(&spADs);
					if (SUCCEEDED(hr) && spADs!=NULL)
					{                       
						spDisp.Release();             
						CComBSTR bstrSchemaClass;
						hr=spADs->get_Class(&bstrSchemaClass);
						if (SUCCEEDED(hr) && bstrSchemaClass == "IIsWebServer")
						{
							if (!spDeploySite)
							{
								spDeploySite=spADs;
							}
							CComVariant varSiteName;
							hr=spADs->Get(L"ServerComment",&varSiteName);
							if (SUCCEEDED(hr))
							{
								CComBSTR bstrSiteName (V_BSTR(&varSiteName));
								if (bstrSiteName == "Default Web Site")
								{
									spDeploySite = spADs;
									break;
								}
							}
						}
					}
					var.Clear();
					hr = spEnum->Next(1, &var, &lFetch);
				} //End While
			}
		}
      }
	  hr=E_FAIL;
      if (spDeploySite!=NULL)
      {
		CComBSTR bstrWebsitePath;
		spDeploySite->get_ADsPath(&bstrWebsitePath);
		bstrWebsitePath+=L"/ROOT";
		CComPtr<IADs> spSite;
		hr=ADsGetObject(bstrWebsitePath, IID_IADs, (void**) &spSite);
		if (SUCCEEDED(hr))
		{
			CComVariant varPath;
			spSite->Get(L"Path",&varPath);
			strPath = V_BSTR(&varPath);
			strPath += L"\\";
		}
      }
      return hr;      
}

