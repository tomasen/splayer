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
#include "sal.h"



#define new DEBUG_NEW

#define GUID_CCH    39  // Characters in string form of guid, including '\0'

#define _AFX_REDIRECT_REGISTRY_HIVE(hKey, strSubKey)	\
	if( ( HKEY_CLASSES_ROOT == hKey ) && ( TRUE == AfxGetPerUserRegistration() ) )\
	{\
		strSubKey = _T("Software\\Classes\\") + strSubKey;\
		hKey = HKEY_CURRENT_USER;\
	}

inline BOOL _AfxRegDeleteKeySucceeded(LONG error)
{
	return (error == ERROR_SUCCESS) || (error == ERROR_BADKEY) ||
		(error == ERROR_FILE_NOT_FOUND);
}

// Under Win32, a reg key may not be deleted unless it is empty.
// Thus, to delete a tree,  one must recursively enumerate and
// delete all of the sub-keys.

LONG AFXAPI _AfxRecursiveRegDeleteKey(HKEY hParentKey, _In_ LPTSTR szKeyName)
{
	// one implementation for everybody
	return AfxDelRegTreeHelper(hParentKey, szKeyName);
}

void _AfxUnregisterInterfaces(ITypeLib* pTypeLib)
{
	TCHAR szKey[128];
	Checked::tcscpy_s(szKey, _countof(szKey), _T("Interface\\"));
	LPTSTR pszGuid = szKey + lstrlen(szKey);

	int cTypeInfo = pTypeLib->GetTypeInfoCount();

	for (int i = 0; i < cTypeInfo; i++)
	{
		TYPEKIND tk;
		if (SUCCEEDED(pTypeLib->GetTypeInfoType(i, &tk)) &&
			(tk == TKIND_DISPATCH || tk == TKIND_INTERFACE))
		{
			ITypeInfo* pTypeInfo = NULL;
			if (SUCCEEDED(pTypeLib->GetTypeInfo(i, &pTypeInfo)))
			{
				TYPEATTR* pTypeAttr;
				if (SUCCEEDED(pTypeInfo->GetTypeAttr(&pTypeAttr)))
				{
#if defined(_UNICODE)
					StringFromGUID2(pTypeAttr->guid, pszGuid, GUID_CCH);
#else
					WCHAR wszGuid[39];
					StringFromGUID2(pTypeAttr->guid, wszGuid, GUID_CCH);
					_wcstombsz(pszGuid, wszGuid, GUID_CCH);
#endif
					_AfxRecursiveRegDeleteKey(HKEY_CLASSES_ROOT, szKey);
					pTypeInfo->ReleaseTypeAttr(pTypeAttr);
				}

				pTypeInfo->Release();
			}
		}
	}
}

BOOL AFXAPI AfxOleRegisterTypeLib(HINSTANCE hInstance, REFGUID tlid,
	LPCTSTR pszFileName, LPCTSTR pszHelpDir)
{	
	BOOL bSuccess = FALSE;
	CStringW strPathNameW;
	wchar_t *szPathNameW = strPathNameW.GetBuffer(_MAX_PATH);
	::GetModuleFileNameW(hInstance, szPathNameW, _MAX_PATH);
	strPathNameW.ReleaseBuffer();
	LPTYPELIB ptlib = NULL;
	// If a filename was specified, replace final component of path with it.
	if (pszFileName != NULL)
	{
		int iBackslash = strPathNameW.ReverseFind('\\');
		if (iBackslash != -1)
			strPathNameW = strPathNameW.Left(iBackslash+1);
		strPathNameW += pszFileName;
	}
	if (SUCCEEDED(LoadTypeLib(strPathNameW.GetString(), &ptlib)))
	{
		ASSERT_POINTER(ptlib, ITypeLib);

		LPTLIBATTR pAttr;
		GUID tlidActual = GUID_NULL;

		if (SUCCEEDED(ptlib->GetLibAttr(&pAttr)))
		{
			ASSERT_POINTER(pAttr, TLIBATTR);
			tlidActual = pAttr->guid;
			ptlib->ReleaseTLibAttr(pAttr);
		}

		// Check that the guid of the loaded type library matches
		// the tlid parameter.
		ASSERT(IsEqualGUID(tlid, tlidActual));

		if (IsEqualGUID(tlid, tlidActual))
		{
			// Register the type library.
			const CStringW strHelpDir(pszHelpDir);

			typedef HRESULT (STDAPICALLTYPE *PFNREGISTERTYPELIB)(ITypeLib *, LPCOLESTR /* const szFullPath */, LPCOLESTR /* const szHelpDir */);
			PFNREGISTERTYPELIB pfnRegisterTypeLib = NULL;

			if( TRUE == AfxGetPerUserRegistration() )
			{
				HMODULE hmodOleAut=::GetModuleHandleW(L"OLEAUT32.DLL");
				if(hmodOleAut)
				{
					pfnRegisterTypeLib=reinterpret_cast<PFNREGISTERTYPELIB>(::GetProcAddress(hmodOleAut, "RegisterTypeLibForUser"));
				}
			}

			if( NULL == pfnRegisterTypeLib )
			{
				pfnRegisterTypeLib = (PFNREGISTERTYPELIB)&RegisterTypeLib;
			}

			if (SUCCEEDED(pfnRegisterTypeLib(ptlib,const_cast<LPWSTR>(strPathNameW.GetString()) , 
													const_cast<LPWSTR>(strHelpDir.GetString())  )))
			{
				bSuccess = TRUE;
			}
		}

		RELEASE(ptlib);
	}
	else
	{
		TRACE(traceAppMsg, 0, L"Warning: Could not load type library from %s\n", strPathNameW);
	}

	return bSuccess;
}

#define TYPELIBWIN   _T("win32")
#define TYPELIBWIN_2 _T("win16")

BOOL AFXAPI AfxOleUnregisterTypeLib(REFGUID tlid, WORD wVerMajor,
	WORD wVerMinor, LCID lcid)
{

	// Load type library before unregistering it.
	ITypeLib* pTypeLib = NULL;
	if (wVerMajor != 0)
	{
		if (FAILED(LoadRegTypeLib(tlid, wVerMajor, wVerMinor, lcid, &pTypeLib)))
			pTypeLib = NULL;
	}

	// Format typelib guid as a string
	OLECHAR szTypeLibID[GUID_CCH];
	int cchGuid = ::StringFromGUID2(tlid, szTypeLibID, GUID_CCH);

	ASSERT(cchGuid == GUID_CCH);    // Did StringFromGUID2 work?
	if (cchGuid != GUID_CCH)
		return FALSE;

	TCHAR szKeyTypeLib[_MAX_PATH];
	BOOL bSurgical = FALSE;
	LONG error = ERROR_SUCCESS;

	const CString strTypeLibID(szTypeLibID);
	if (-1 == _stprintf_s(szKeyTypeLib, _countof(szKeyTypeLib), _T("TYPELIB\\%s"), strTypeLibID.GetString()))
		return FALSE;

	HKEY hKeyTypeLib;
	if (AfxRegOpenKey(HKEY_CLASSES_ROOT, szKeyTypeLib, &hKeyTypeLib) ==
		ERROR_SUCCESS)
	{
		int iKeyVersion = 0;
		HKEY hKeyVersion;
		TCHAR szVersion[_MAX_PATH];

		// Iterate through all installed versions of the control

		while (RegEnumKey(hKeyTypeLib, iKeyVersion, szVersion, _MAX_PATH) ==
			ERROR_SUCCESS)
		{
			hKeyVersion = NULL;
			BOOL bSurgicalVersion = FALSE;

			if (RegOpenKey(hKeyTypeLib, szVersion, &hKeyVersion) !=
				ERROR_SUCCESS)
			{
				++iKeyVersion;
				continue;
			}

			int iKeyLocale = 0;
			HKEY hKeyLocale;
			TCHAR szLocale[_MAX_PATH];

			// Iterate through all registered locales for this version

			while (RegEnumKey(hKeyVersion, iKeyLocale, szLocale, _MAX_PATH) ==
				ERROR_SUCCESS)
			{
				// Don't remove HELPDIR or FLAGS keys.
				if ((::AfxInvariantStrICmp(szLocale, _T("HELPDIR")) == 0) ||
					(::AfxInvariantStrICmp(szLocale, _T("FLAGS")) == 0))
				{
					++iKeyLocale;
					continue;
				}

				hKeyLocale = NULL;

				if (RegOpenKey(hKeyVersion, szLocale, &hKeyLocale) !=
					ERROR_SUCCESS)
				{
					++iKeyLocale;
					continue;
				}

				// Check if a 16-bit key is found when unregistering 32-bit
				HKEY hkey;
				if (RegOpenKey(hKeyLocale, TYPELIBWIN_2, &hkey) ==
					ERROR_SUCCESS)
				{
					RegCloseKey(hkey);

					// Only remove the keys specific to the 32-bit version
					// of control, leaving things intact for 16-bit version.
					error = _AfxRecursiveRegDeleteKey(hKeyLocale, TYPELIBWIN);
					bSurgicalVersion = TRUE;
					RegCloseKey(hKeyLocale);
				}
				else
				{
					// Delete everything for this locale.
					RegCloseKey(hKeyLocale);
					if (_AfxRecursiveRegDeleteKey(hKeyVersion, szLocale) ==
						ERROR_SUCCESS)
					{
						// Start over again, so we don't skip anything.
						iKeyLocale = 0;
						continue;
					}
				}
				++iKeyLocale;
			}
			RegCloseKey(hKeyVersion);

			if (bSurgicalVersion)
			{
				bSurgical = TRUE;
			}
			else
			{
				if (_AfxRecursiveRegDeleteKey(hKeyTypeLib, szVersion) ==
					ERROR_SUCCESS)
				{
					// Start over again, to make sure we don't skip anything.
					iKeyVersion = 0;
					continue;
				}
			}

			++iKeyVersion;
		}
		RegCloseKey(hKeyTypeLib);
	}

	if (!bSurgical)
		error = _AfxRecursiveRegDeleteKey(HKEY_CLASSES_ROOT, szKeyTypeLib);

	if (_AfxRegDeleteKeySucceeded(error))
	{
		// If type library was unregistered successfully, then also unregister
		// interfaces.
		if (pTypeLib != NULL)
		{
			ITypeLib* pDummy = NULL;
			if (FAILED(LoadRegTypeLib(tlid, wVerMajor, wVerMinor, lcid, &pDummy)))
				_AfxUnregisterInterfaces(pTypeLib);
			else
				pDummy->Release();

			pTypeLib->Release();
		}
	}

	return _AfxRegDeleteKeySucceeded(error);
}

AFX_STATIC_DATA const LPCTSTR _afxCtrlProgID[] =
{
	_T("\0") _T("%1"),
	_T("CLSID\0") _T("%2"),
	NULL
};

#define INPROCSERVER   _T("InprocServer32")
#define INPROCSERVER_2 _T("InprocServer")
#define TOOLBOXBITMAP  _T("ToolboxBitmap32")

AFX_STATIC_DATA const LPCTSTR _afxCtrlClassID[] =
{
	_T("\0") _T("%1"),
	_T("ProgID\0") _T("%2"),
	INPROCSERVER _T("\0%3"),
	TOOLBOXBITMAP _T("\0%3, %4"),
	_T("MiscStatus\0") _T("0"),
	_T("MiscStatus\\1\0") _T("%5"),
	_T("Control\0") _T(""),
	_T("TypeLib\0") _T("%6"),
	_T("Version\0") _T("%7"),
	NULL
};

BOOL AFXAPI AfxOleRegisterControlClass(HINSTANCE hInstance,
	REFCLSID clsid, LPCTSTR pszProgID, UINT idTypeName, UINT idBitmap,
	int nRegFlags, DWORD dwMiscStatus, REFGUID tlid, WORD wVerMajor,
	WORD wVerMinor)
{	
	BOOL bSuccess = FALSE;

	// Format class ID as a string
	OLECHAR szClassID[GUID_CCH];
	int cchGuid = ::StringFromGUID2(clsid, szClassID, GUID_CCH);
	const CString strClassID(szClassID);

	ASSERT(cchGuid == GUID_CCH);    // Did StringFromGUID2 work?
	if (cchGuid != GUID_CCH)
		return FALSE;

	// Format typelib guid as a string
	
	CString strTypeLibID;	
	OLECHAR szTypeLibID[GUID_CCH];
	cchGuid = ::StringFromGUID2(tlid, szTypeLibID, GUID_CCH);

	ASSERT(cchGuid == GUID_CCH);    // Did StringFromGUID2 work?
	if (cchGuid != GUID_CCH)
		return FALSE;

	CString strPathName;
	AfxGetModuleShortFileName(hInstance, strPathName);

	CString strTypeName;
	if (!strTypeName.LoadString(idTypeName))
	{
		ASSERT(FALSE);  // Name string not present in resources
		strTypeName = strClassID; // Use Class ID instead
	}

	TCHAR szBitmapID[_MAX_PATH];
	Checked::itot_s(idBitmap, szBitmapID, _MAX_PATH, 10);

	TCHAR szMiscStatus[_MAX_PATH];
	Checked::ltot_s(dwMiscStatus, szMiscStatus, _MAX_PATH, 10);

	// Format version string as "major.minor"
	TCHAR szVersion[_MAX_PATH];
	_stprintf_s(szVersion, _countof(szVersion), _T("%d.%d"), wVerMajor, wVerMinor);

	const TCHAR* szPerUserRegistration = {_T("Software\\Classes\\")};

	HKEY hkeyRootKey = HKEY_CLASSES_ROOT;
	const TCHAR* szRedirection = {_T("")};

	if( TRUE == AfxGetPerUserRegistration() )
	{
		szRedirection = szPerUserRegistration;
		hkeyRootKey = HKEY_CURRENT_USER;
	}

	// Attempt to open registry keys.
	::ATL::CRegKey hkeyClassID;
	::ATL::CRegKey hkeyProgID;

	TCHAR szScratch[_MAX_PATH];
	TCHAR szProgID[_MAX_PATH];

	if (-1 == _stprintf_s(szScratch, _countof(szScratch), _T("%sCLSID\\%s"), szRedirection, strClassID.GetString()))
	{
		return FALSE;
	}
	if (-1 == _stprintf_s(szProgID, _countof(szProgID), _T("%s%s"), szRedirection, pszProgID))
	{
		return FALSE;
	}

	if (hkeyClassID.Create(hkeyRootKey, szScratch, NULL, 0, KEY_READ | KEY_WRITE, NULL, NULL) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	if (hkeyProgID.Create(hkeyRootKey, szProgID, NULL, 0, KEY_READ | KEY_WRITE, NULL, NULL) !=ERROR_SUCCESS) 
	{
        	return FALSE;
	}

	ASSERT(hkeyClassID != NULL);
	ASSERT(hkeyProgID != NULL);

	LPCTSTR rglpszSymbols[7];
	rglpszSymbols[0] = strTypeName;
	rglpszSymbols[1] = strClassID;
	bSuccess = AfxOleRegisterHelper(_afxCtrlProgID, rglpszSymbols, 2,
		TRUE, hkeyProgID);
	
	if (!bSuccess)
	{
		return FALSE;
	}

	rglpszSymbols[1] = pszProgID;
	rglpszSymbols[2] = strPathName;
	rglpszSymbols[3] = szBitmapID;
	rglpszSymbols[4] = szMiscStatus;
	strTypeLibID=szTypeLibID;
	rglpszSymbols[5] = strTypeLibID.GetString();
	rglpszSymbols[6] = szVersion;
	bSuccess = AfxOleRegisterHelper(_afxCtrlClassID, rglpszSymbols, 7,
		TRUE, hkeyClassID);

	if (!bSuccess)
	{
		return FALSE;
	}

	bSuccess = AfxOleInprocRegisterHelper(hkeyProgID, hkeyClassID, nRegFlags);

	return bSuccess;
}

BOOL AFXAPI AfxOleUnregisterClass(REFCLSID clsid, LPCTSTR pszProgID)
{	

	// Format class ID as a string
	OLECHAR szClassID[GUID_CCH];
	int cchGuid = ::StringFromGUID2(clsid, szClassID, GUID_CCH);
	const CString strClassID(szClassID);
	

	ASSERT(cchGuid == GUID_CCH);    // Did StringFromGUID2 work?
	if (cchGuid != GUID_CCH)
		return FALSE;

	TCHAR szKey[_MAX_PATH];
	long error;
	BOOL bRetCode = TRUE;

	// check to see if a 16-bit InprocServer key is found when unregistering
	// 32-bit (or vice versa).
	if (-1 == _stprintf_s(szKey, _countof(szKey), _T("CLSID\\%s\\%s"), strClassID.GetString(), INPROCSERVER_2))
		return FALSE;

	HKEY hkey=NULL;
	BOOL bSurgical = (AfxRegOpenKey(HKEY_CLASSES_ROOT, szKey, &hkey) == ERROR_SUCCESS);
	RegCloseKey(hkey);

	if (bSurgical)
	{
		// Only remove the keys specific to this version of the control,
		// leaving things in tact for the other version.
		if (-1 == _stprintf_s(szKey, _countof(szKey), _T("CLSID\\%s\\%s"), strClassID.GetString(), INPROCSERVER))
			return FALSE;
		error = AfxRegDeleteKey(HKEY_CLASSES_ROOT, szKey);
		bRetCode = bRetCode && _AfxRegDeleteKeySucceeded(error);

		if (-1 == _stprintf_s(szKey, _countof(szKey), _T("CLSID\\%s\\%s"), strClassID.GetString(), TOOLBOXBITMAP))
			return FALSE;
		error = AfxRegDeleteKey(HKEY_CLASSES_ROOT, szKey);
		bRetCode = bRetCode && _AfxRegDeleteKeySucceeded(error);
	}
	else
	{
		// No other versions of this control were detected,
		// so go ahead and remove the control completely.
		if (-1 == _stprintf_s(szKey, _countof(szKey), _T("CLSID\\%s"), strClassID.GetString()))
			return FALSE;
		error = _AfxRecursiveRegDeleteKey(HKEY_CLASSES_ROOT, szKey);
		bRetCode = bRetCode && _AfxRegDeleteKeySucceeded(error);

		if ((pszProgID != NULL) && (lstrlen(pszProgID) > 0))
		{
			error = _AfxRecursiveRegDeleteKey(HKEY_CLASSES_ROOT,
				(LPTSTR)pszProgID);
			bRetCode = bRetCode && _AfxRegDeleteKeySucceeded(error);
		}
	}

	return bRetCode;
}

AFX_STATIC_DATA const LPCTSTR _afxPropPageClass[] =
{
	_T("\0") _T("%1"),
	INPROCSERVER _T("\0%2"),
	NULL
};

BOOL AFXAPI AfxOleRegisterPropertyPageClass(HINSTANCE hInstance,
	REFCLSID clsid, UINT idTypeName)
{
	return AfxOleRegisterPropertyPageClass(hInstance, clsid, idTypeName, 0);
}

BOOL AFXAPI AfxOleRegisterPropertyPageClass(HINSTANCE hInstance,
	REFCLSID clsid, UINT idTypeName, int nRegFlags)
{
	ASSERT(!(nRegFlags & afxRegInsertable));    // can't be insertable

	BOOL bSuccess = FALSE;

	// Format class ID as a string
	OLECHAR szClassID[GUID_CCH];
	int cchGuid = ::StringFromGUID2(clsid, szClassID, GUID_CCH);
	const CString strClassID(szClassID);
	
	ASSERT(cchGuid == GUID_CCH);    // Did StringFromGUID2 work?
	if (cchGuid != GUID_CCH)
		return FALSE;

	CString strPathName;
	AfxGetModuleShortFileName(hInstance, strPathName);

	CString strTypeName;
	if (!strTypeName.LoadString(idTypeName))
	{
		ASSERT(FALSE);  // Name string not present in resources
		strTypeName = strClassID; // Use Class ID instead
	}

	HKEY hkeyClassID = NULL;

	TCHAR szKey[_MAX_PATH];
	if (-1 == _stprintf_s(szKey, _countof(szKey), _T("CLSID\\%s"), strClassID.GetString()))
		return FALSE;
	if (AfxRegCreateKey(HKEY_CLASSES_ROOT, szKey, &hkeyClassID) !=
		ERROR_SUCCESS)
		goto Error;

	LPCTSTR rglpszSymbols[2];
	rglpszSymbols[0] = strTypeName;
	rglpszSymbols[1] = strPathName;
	bSuccess = AfxOleRegisterHelper(_afxPropPageClass, rglpszSymbols,
		2, TRUE, hkeyClassID);

	if (!bSuccess)
		goto Error;

	AfxOleInprocRegisterHelper(NULL, hkeyClassID, nRegFlags);

Error:
	if (hkeyClassID != NULL)
		::RegCloseKey(hkeyClassID);

	return bSuccess;
}

LONG AFXAPI AfxRegCreateKey(HKEY hKey, LPCTSTR lpSubKey, PHKEY phkResult)
{
	CString strSubKey = lpSubKey;

	_AFX_REDIRECT_REGISTRY_HIVE(hKey, strSubKey)

	return ::RegCreateKey(hKey, strSubKey, phkResult);
}

LONG AFXAPI AfxRegOpenKey(HKEY hKey, LPCTSTR lpSubKey, PHKEY phkResult)
{
	CString strSubKey = lpSubKey;

	_AFX_REDIRECT_REGISTRY_HIVE(hKey, strSubKey)

	return ::RegOpenKey(hKey, strSubKey, phkResult);
}

LONG AFXAPI AfxRegOpenKeyEx(HKEY hKey, LPCTSTR lpSubKey,  DWORD ulOptions,  REGSAM samDesired, PHKEY phkResult)
{
	CString strSubKey = lpSubKey;

	_AFX_REDIRECT_REGISTRY_HIVE(hKey, strSubKey)

	return ::RegOpenKeyEx(hKey, strSubKey, ulOptions, samDesired, phkResult);
}

LONG AFXAPI AfxRegQueryValue(HKEY hKey, LPCTSTR lpSubKey,  LPTSTR lpValue,  PLONG lpcbValue)
{
	CString strSubKey = lpSubKey;

	_AFX_REDIRECT_REGISTRY_HIVE(hKey, strSubKey)

	return ::RegQueryValue(hKey, strSubKey, lpValue, lpcbValue);
}

LONG AFXAPI AfxRegSetValue(HKEY hKey, LPCTSTR lpSubKey,  DWORD dwType, LPCTSTR lpData,  DWORD cbData)
{
	CString strSubKey = lpSubKey;

	_AFX_REDIRECT_REGISTRY_HIVE(hKey, strSubKey)

	return ::RegSetValue(hKey , strSubKey, dwType, lpData, cbData);
}

LONG AFXAPI AfxRegDeleteKey(HKEY hKey, LPCTSTR lpSubKey)
{
	CString strSubKey = lpSubKey;

	_AFX_REDIRECT_REGISTRY_HIVE(hKey, strSubKey)

	return ::RegDeleteKey(hKey, strSubKey);
}

/////////////////////////////////////////////////////////////////////////////
// Force any extra compiler-generated code into AFX_INIT_SEG

