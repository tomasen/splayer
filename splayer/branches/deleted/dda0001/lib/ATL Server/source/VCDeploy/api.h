//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
#pragma once

#include <initguid.h>
#include <iadmw.h>
#include <iiscnfg.h>

#define ATLSDPLY_SUCCESS	0
#define ATLSDPLY_FAIL		0xFFFFFFFF
#define MAX_VERB_COUNT		10
#define MAX_VERB_BLOCKS		5
#define RETURN_ON_UNEXPECTED_HRP(hr, p) if (hr != ATLSDPLY_SUCCESS || p == NULL){PrintError(IDS_UNEXPECTED); return ATLSDPLY_FAIL;}
#define RETURN_ON_UNEXPECTED(hr) if (hr != ATLSDPLY_SUCCESS){PrintError(IDS_UNEXPECTED); return ATLSDPLY_FAIL;}
#define RETURN_ON_UNEXPECTED_P(p) if (p == NULL){PrintError(IDS_UNEXPECTED); return ATLSDPLY_FAIL;}
#define RETURN_ON_FAIL(hr) if (hr != ATLSDPLY_SUCCESS){return hr;}
#define RETURN_ON_FAIL2(hr, msgid) if (hr != ATLSDPLY_SUCCESS){PrintError(msgid); return ATLSDPLY_FAIL;}
#define RETURN_ON_FAIL2_HRP(hr, p, msgid) if (hr != ATLSDPLY_SUCCESS || p == NULL){PrintError(msgid); return ATLSDPLY_FAIL;}
#define RETURN_ON_FAIL2_P(p, msgid) if (p == NULL){PrintError(msgid); return ATLSDPLY_FAIL;}


typedef HRESULT (__stdcall *PFNRegisterServer)(void);

class CDepSettings;
class CADSIHelper;
int PrintError(unsigned int nMsgID);
int PrintError(unsigned int nMsgID, LPCTSTR szExtraInfo);
int PrintErrorWithWinLast32(unsigned int nMsgID);
int PrintErrorFormatted(LPCTSTR szError);
int PrintWarning(unsigned int nMsgID);
int PrintWarning(unsigned int nMsgID, LPCTSTR szExtraInfo);
int PrintWarningWithLastWin32(unsigned int nMsgID);
int PrintWarningFormatted(LPCTSTR szWarning);
int PrintMessage(LPCTSTR szPrefix, unsigned int nMsgID, LPCTSTR szExtraInfo);
int PrintMessage(LPCTSTR szPrefix, LPCTSTR szMsg, LPCTSTR szExtraInfo);
int PrintMessage(unsigned int nMsgID);

BOOL LoadStringFromModule(int nID, CString& str);
int ProcessSettings(CDepSettings *pSettings);
int CheckVirtualDir(const CComBSTR& bstrHostName,
					CADSIHelper *pAdsHelper,
					CDepSettings *pSettings);
int CreateVRoot(const CComBSTR& bstrHostName,
				CADSIHelper *pAdsHelper,
				CDepSettings *pSettings);

int ConfigureRestrictionList(CADSIHelper *pAdsHelper, 
							 CDepSettings *pSettings,
							 const wchar_t *wszHost);

int SetRootAppMappings(	CADSIHelper *pAdsHelper,
						CDepSettings *pSettings);
int LocalW3svcReset();

int CheckVRootExistance(const CComBSTR& bstrHostName,
						CADSIHelper *pAdsHelper,
						CDepSettings *pSettings);

int UpdateFileSystem(CDepSettings *pSettings);
int ConfigureVRoot(	CADSIHelper *pAdsHelper,
					CDepSettings *pSettings,
					const wchar_t *wszHost);
int RegisterExtension(CADSIHelper *pAdsHelper,
					  CDepSettings *pSettings);
int CheckMinVRootConfigSettings(CDepSettings *pSettings);
int RuntimeCheck();
HRESULT CheckFileDiff(LPCTSTR szSrcPath, LPCTSTR szDestPath);// returns S_OK if last update is the same
                                                              // S_FALSE if they aren't otherwise an error
                                                              // HRESULT
HRESULT ProcessAccessCheck(); // returns S_OK if process creator is a member of local admin group, S_FALSE
                              // if not and an error HRESULT if an error occurs.


class CADSIHelper
{
public:
	static HRESULT FastLocalRootCheck(LPCTSTR szVRootName)
	{
		CComPtr<IMSAdminBase> spAdmBase;
		HRESULT hr = spAdmBase.CoCreateInstance(CLSID_MSAdminBase);
		if (FAILED(hr))
			return E_FAIL;

		METADATA_HANDLE hRootHandle = NULL;
		CString strKey(_T("/LM/W3SVC/1/ROOT/"));
		strKey += szVRootName;
		CT2W strPath(strKey);

		hr = spAdmBase->OpenKey(METADATA_MASTER_ROOT_HANDLE,
							strPath,
							METADATA_PERMISSION_READ,
							20,
							&hRootHandle);
		if (hr != S_OK)
			return hr;

		spAdmBase->CloseKey(hRootHandle);
		return hr;

	}

	static HRESULT SaveMetabaseData()
	{
		CComPtr<IMSAdminBase> spAdmBase;
		HRESULT hr = spAdmBase.CoCreateInstance(CLSID_MSAdminBase);
		if (FAILED(hr))
			return E_FAIL;

		return spAdmBase->SaveData();
	}

	HRESULT Connect(LPCWSTR szPath)
	{
		CComPtr<IMoniker> spmkLocalHost;
		CComPtr<IBindCtx> spbc;
		ULONG cEaten = 0;
		HRESULT hr = E_FAIL;
		CComBSTR bstrApplPath(szPath);
		CComPtr<IDispatch> spDispAds;

		// try to connect to the iisadmin service
		hr = CreateBindCtx(NULL, &spbc);
		if (SUCCEEDED(hr))
		{
			hr = MkParseDisplayName(spbc, bstrApplPath, &cEaten, &spmkLocalHost);
			if (SUCCEEDED(hr))
			{
				hr = BindMoniker(spmkLocalHost, 0, IID_IDispatch, (void**)&spDispAds);
			}
		}
		if (hr != S_OK)
			return hr;

		m_spAds = spDispAds;
		if (!m_spAds)
			hr = E_NOINTERFACE;
		return hr;
	}

	void Disconnect()
	{
		if (m_spAds)
			m_spAds.Release();
	}

	HRESULT CreateApp(short nIso)
	{
		if (!m_spAds)
			return E_UNEXPECTED;

		CComQIPtr<IDispatch> spAdmin = m_spAds;
		if (!spAdmin)
			return E_NOINTERFACE;

		CComVariant vResult;
		CComVariant vParam(nIso);
		
		HRESULT hr = spAdmin.Invoke1(L"AppCreate2", &vParam, &vResult);
		if (hr == DISP_E_UNKNOWNNAME)
		{
			// NT4 no implement appcreate2, must try appcreate
			vResult.Clear();
			vParam.vt = VT_BOOL;
			vParam.boolVal = (nIso == 0 ? VARIANT_TRUE : VARIANT_FALSE);
			hr = spAdmin.Invoke1(L"AppCreate", &vParam, &vResult);
		}
		return hr;
	}

	HRESULT CreateAppOnly(LPCWSTR szRootName, short nIsolation, IADsContainer **ppAds)
	{
		if (!m_spAds)
			return E_UNEXPECTED;

		HRESULT hr = E_FAIL;
		CComPtr<IDispatch> spDispNewContainer;
		hr = m_spAds->Create(CComBSTR(L"IIsWebDirectory"),
						CComBSTR(szRootName),
						&spDispNewContainer);
		if (hr != S_OK)
			return hr;

		if (ppAds)
			hr = spDispNewContainer->QueryInterface(__uuidof(IADsContainer), (void**)ppAds);

		if (m_spAds)
			m_spAds.Release();

		hr = spDispNewContainer->QueryInterface(__uuidof(IADsContainer), (void**)&m_spAds);
		if (hr != S_OK)
			return hr;

		return CreateApp(nIsolation);
	}

	HRESULT CreateVRoot(LPCWSTR szRootName, short nIsolation, IADsContainer **ppAds)
	{
		if (!m_spAds)
			return E_UNEXPECTED;

		HRESULT hr = E_FAIL;
		CComPtr<IDispatch> spDispNewContainer;
		hr = m_spAds->Create(CComBSTR(L"IIsWebVirtualDir"),
						CComBSTR(szRootName),
						&spDispNewContainer);
		if (hr != S_OK)
			return hr;

		if (ppAds)
			hr = spDispNewContainer->QueryInterface(__uuidof(IADsContainer), (void**)ppAds);

		if (m_spAds)
			m_spAds.Release();

		hr = spDispNewContainer->QueryInterface(__uuidof(IADsContainer), (void**)&m_spAds);
		if (hr != S_OK)
			return hr;

		return CreateApp(nIsolation);
	}

	HRESULT EnableApp()
	{
		if (!m_spAds)
			return E_UNEXPECTED;
		CComQIPtr<IDispatch> spDispContainer = m_spAds;
		if (!spDispContainer)
			return E_NOINTERFACE;
		CComVariant vResult;
		return spDispContainer.Invoke0(L"AppEnable",&vResult);
	}

	HRESULT SetProperty(LPCWSTR szPropName,
						const VARIANT& vPropValue)
	{
		if (!m_spAds)
			return E_UNEXPECTED;
		CComQIPtr<IADs> spAds = m_spAds;
		if (!spAds)
			return E_NOINTERFACE;

		return spAds->Put(CComBSTR(szPropName), vPropValue);
	}

	HRESULT GetProperty(LPCWSTR szPropName,
						VARIANT& vPropValue)
	{
		if (!m_spAds)
			return E_UNEXPECTED;

		CComQIPtr<IADs> spAds = m_spAds;
		if (!spAds)
			return E_NOINTERFACE;
		CComBSTR bstrName(szPropName);
		CComVariant vValue;
		return spAds->GetEx(bstrName, &vPropValue);
	}

	HRESULT GetPropertySingle(LPCWSTR szPropName,
						VARIANT& vPropValue)
	{
		if (!m_spAds)
			return E_UNEXPECTED;

		CComQIPtr<IADs> spAds = m_spAds;
		if (!spAds)
			return E_NOINTERFACE;
		CComBSTR bstrName(szPropName);
		CComVariant vValue;
		return spAds->Get(bstrName, &vPropValue);
	}

	HRESULT SetInfo()
	{
		if (!m_spAds)
			return E_UNEXPECTED;
		CComQIPtr<IADs> spAds = m_spAds;
		if (!spAds)
			return E_NOINTERFACE;
		return spAds->SetInfo();
	}

	HRESULT DeleteVRoot(LPCWSTR wszRoot)
	{
		if (!m_spAds)
			return E_UNEXPECTED;

		CComQIPtr<IDispatch> spAdmin = m_spAds;
		if (!spAdmin)
			return E_NOINTERFACE;

		CComVariant vResult;
		CComVariant vRootName(wszRoot);
		CComVariant vVirtDir(L"IISWebVirtualDir");
		return spAdmin.Invoke2(L"Delete", &vVirtDir, &vRootName, &vResult);
		
	}
#ifdef _DEBUG
	void AssertValid()
	{
		ATLASSERT(m_spAds != NULL);
	}
#endif
protected:
	CComQIPtr<IADsContainer> m_spAds;
};

class CArgs
{
public:
	CArgs()
	{
		m_bNoLogo = false;
		m_bShowUsage = false;
	}

	bool Parse(int argc, char* argv[])
	{
		if (argc < 2)
		{
			m_bShowUsage = true;
			return true;
		}
		for (int i=1;i<argc;i++)
		{
			if (argv[i][0] == _T('-') || argv[i][0] == _T('/'))
			{
				// lcase argv[i];
				FindArg(&argv[i][1]);
			}
			else
			{
				// assume it's the path to the xml file
				m_strSettingsFile = argv[i];
			}
		}

		if (m_strSettingsFile.GetLength() <= 0)
			m_bShowUsage = true;
		return true;
	}

	void FindArg(TCHAR* pArg)
	{
		if (!pArg || *pArg == _T('\0'))
			return;

		if (!_tcsicmp(pArg, "nologo"))
			m_bNoLogo = true;
		if (!_tcsicmp(pArg, "?"))
			m_bShowUsage = true;
	}

	bool m_bNoLogo;
	bool m_bShowUsage;
	CString m_strSettingsFile;
};
HRESULT GetWWWRootPath(const CStringW strWebHostName,CStringW& strPath);
