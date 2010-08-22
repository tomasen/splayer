//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
#include "stdafx.h"
#include "resource.h"
#include "depsettings.h"
#include "errno.h"
#include "api.h"
#include <atlutil.h>

CDepSettings::CDepSettings()
{
	m_bRegIsapi = false;
	m_bUnloadBeforeCopy = false;
	m_bDoNotCreateVirtDir = false;
	m_nAppIsolation = 0;
	m_nIISMajorVer = -1;
}

HRESULT CDepSettings::GetHostList(IXMLDOMNodeList **ppList)
{
	if (!ppList)
		return E_INVALIDARG;
	*ppList = NULL;
	if (m_spHostList)
		return m_spHostList.CopyTo(ppList);
	return E_FAIL;
}

HRESULT CDepSettings::GetAppMappingList(IXMLDOMNodeList **ppList)
{
	if (!ppList)
		return E_INVALIDARG;
	*ppList = NULL;
	if (m_spAppMappings)
		return m_spAppMappings.CopyTo(ppList);
	return S_OK;
}

HRESULT CDepSettings::GetFileGroups(IXMLDOMNodeList **ppList)
{
	if (!ppList)
		return E_INVALIDARG;
	*ppList = NULL;
	if (m_spAppFileGroups)
		return m_spAppFileGroups.CopyTo(ppList);
	return E_FAIL;
}

unsigned int CDepSettings::Load(LPCSTR szSettingsPath)
{
	HRESULT hr = E_FAIL;
	
	if (!szSettingsPath)
		return IDS_UNEXPECTED;

	// Create the DOM
	CComPtr<IXMLDOMDocument> spDOMDoc;
	CComPtr<IXMLDOMNode> spRoot;
	CComPtr<IXMLDOMNode> spResultNode;

	hr = spDOMDoc.CoCreateInstance(L"Microsoft.XMLDOM");
	if (FAILED(hr))
		return IDS_ERR_FAILEDTOCREATEDOM;
	
	hr = spDOMDoc->put_async(VARIANT_FALSE);
	if (FAILED(hr))
		return IDS_UNEXPECTED;

	// Load the document
	CComVariant varDocPath(szSettingsPath);
	VARIANT_BOOL  vResult = VARIANT_FALSE;
	hr = spDOMDoc->load(varDocPath, &vResult);
	if (FAILED(hr) || vResult == VARIANT_FALSE)
		return IDS_ERR_FAILEDTOLOAD_SETTINGS_XML;


	// find the root element
	CComBSTR bstrSearch(L"ATLSINSTSETTINGS");
	hr = spDOMDoc->selectSingleNode(bstrSearch, &spRoot);
	if (FAILED(hr) || !spRoot)
		return IDS_ERR_BADROOTNODE;

	CStringW strWebHostNameW;
	// load the list of web hosts
	bstrSearch = L"WEBHOSTNAME";
	hr = spRoot->selectNodes(bstrSearch, &m_spHostList);
	if (FAILED(hr) || !m_spHostList)
		return IDS_ERR_WEBHOSTNAME;
	else
	{
		long len = 0;
		m_spHostList->get_length(&len);
		if (len == 0)
			return IDS_ERR_WEBHOSTNAME;
		CComPtr<IXMLDOMNode> spFirstWebHost;
		hr = m_spHostList->get_item(0,&spFirstWebHost);
		if (SUCCEEDED(hr))
		{
			hr = spFirstWebHost->get_text(&bstrSearch);
			strWebHostNameW=bstrSearch;
		}
	}

	// load the virtual directory name on the host
	bstrSearch = L"VIRTDIRNAME";
	hr = spRoot->selectSingleNode(bstrSearch, &spResultNode);
	if (FAILED(hr) || !spResultNode)
		return IDS_ERR_NOVIRTDIR;

	hr = spResultNode->get_text(&bstrSearch);
	if (FAILED(hr) || *bstrSearch == L'\0')
		return IDS_ERR_BADVIRTDIRNODE;
	m_strVirtDirName = bstrSearch;

	// load the virtual directory file system path
	spResultNode.Release();
	bstrSearch = L"VIRTDIRFSPATH";
	hr = spRoot->selectSingleNode(bstrSearch, &spResultNode);
	if (FAILED(hr) || !spResultNode)
		return IDS_ERR_NOVIRTDIRFSPATH;
	
	spResultNode->get_text(&bstrSearch);
	if (FAILED(hr) || *bstrSearch == L'\0')
		return IDS_ERR_BADVIRTDIRSFPATHNODE;

	m_strVirtDirFSPath = bstrSearch;
	if (!AtlIsFullPathT(m_strVirtDirFSPath.GetString()))
	{
		CStringW strPath;
		hr=GetWWWRootPath(strWebHostNameW,strPath);
		if (FAILED(hr) || strPath.IsEmpty() )
		{
			return IDS_ERR_BADVIRTDIRSFPATHNODE;
		}
		m_strVirtDirFSPath=strPath;
		m_strVirtDirFSPath+=m_strVirtDirName;
	}

	// load the DoNotCreateVirtDir value
	spResultNode.Release();
	bstrSearch = L"DONOTCREATEVIRTDIR";
	hr = spRoot->selectSingleNode(bstrSearch, &spResultNode);
	if (hr == S_OK && spResultNode)
	{
		hr = spResultNode->get_text(&bstrSearch);
		if (FAILED(hr))
			PrintWarning(IDS_ERR_BADDONOTCREATEVIRTDIR);
		else
		{
			if (!_wcsicmp(bstrSearch, L"false"))
				m_bDoNotCreateVirtDir = false;
			else if (!_wcsicmp(bstrSearch, L"true"))
				m_bDoNotCreateVirtDir = true;
			else
			{
				PrintWarning(IDS_ERR_INVALIDDONOTCREATEVIRTDIR);
			}
		}
	}

	// load the registerisapi value
	spResultNode.Release();
	bstrSearch = L"REGISTERISAPI";
	hr = spRoot->selectSingleNode(bstrSearch, &spResultNode);
	if (hr == S_OK && spResultNode)
	{
		hr = spResultNode->get_text(&bstrSearch);
		if (FAILED(hr))
			PrintWarning(IDS_ERR_BADREGISTERISAPI);
		else
		{
			if (!_wcsicmp(bstrSearch, L"false"))
				m_bRegIsapi = false;
			else if (!_wcsicmp(bstrSearch, L"true"))
				m_bRegIsapi = true;
			else
			{
				PrintWarning(IDS_ERR_INVALIDREGISTERISAPI);
			}
		}
	}

	// load the unload before copy value
	spResultNode.Release();
	bstrSearch = L"UNLOADBEFORECOPY";
	hr = spRoot->selectSingleNode(bstrSearch, &spResultNode);
	if (hr == S_OK && spResultNode)
	{
		hr = spResultNode->get_text(&bstrSearch);
		if (FAILED(hr))
			PrintWarning(IDS_ERR_BADUNLOADBEFORECOPY);
		else
		{
			if (!_wcsicmp(bstrSearch, L"false"))
				m_bUnloadBeforeCopy = false;
			else if (!_wcsicmp(bstrSearch, L"true"))
				m_bUnloadBeforeCopy = true;
			else
				PrintWarning(IDS_ERR_INVALIDUNLOADBEFORECOPY);
		}
	}

	// load the app isolation value
	spResultNode.Release();
	bstrSearch = L"APPISOLATION";
	hr = spRoot->selectSingleNode(bstrSearch, &spResultNode);
	if (hr == S_OK && spResultNode)
	{
		hr = spResultNode->get_text(&bstrSearch);
		if (FAILED(hr))
			PrintWarning(IDS_ERR_BADAPPISOLATION);
		else
		{
			wchar_t *szEnd = NULL;
			short nIso = (short)wcstol(bstrSearch, &szEnd, 10);
			if (errno == ERANGE || nIso < 0 || nIso > 2)
			{
                PrintWarning(IDS_ERR_INVALIDAPPISOLATION);
			}
			else
			{
				switch(nIso)
				{
				case 0:
					m_nAppIsolation = nIso;
					break;
				case 1:
					m_nAppIsolation = 2;
					break;
				case 2:
					m_nAppIsolation = 1;
					break;
				}
			}
		}
	}

	// load the list of APPMAPPING nodes
	bstrSearch = L"APPMAPPING";
	hr = spRoot->selectNodes(bstrSearch, &m_spAppMappings);

	// load list of APPFILEGROUP nodes
	bstrSearch = L"APPFILEGROUP";
	hr = spRoot->selectNodes(bstrSearch, &m_spAppFileGroups);

	// see if there is a node for the extension. There better be only one because
	// we will only select the first one we run into.
	CComPtr<IXMLDOMNode> spIsapiNode;
	hr = spRoot->selectSingleNode(CComBSTR(L"//APPFILENAME[@type=\"extension\"] "), &spIsapiNode);
	if (hr == S_OK && spIsapiNode)
	{
		// select the <DEST> node
		CComPtr<IXMLDOMNode> spDestNode;
		hr = spIsapiNode->selectSingleNode(CComBSTR(L"DEST"), &spDestNode);
		if (hr == S_OK && spDestNode)
		{
			CComBSTR bstrPath;
			hr = spDestNode->get_text(&bstrPath);
			if (hr == S_OK && bstrPath[0] != L'\0')
			{
				m_strExtFileName = bstrPath;
			}
		}
	}

	GetIISMajorVer();
#ifdef _DEBUG
	// dump the settings
	printf("loaded settings:\n");
	printf("Virtual directory name: %s\n", m_strVirtDirName);
	printf("Virtual directory fs path: %s\n", m_strVirtDirFSPath);
	printf("Virtual directory isapi extension name: %s\n",m_strExtFileName);
	printf("Do not create virtual directory: %s\n", m_bDoNotCreateVirtDir ? "true" : "false");
	printf("Register ISAPI: %s\n", m_bRegIsapi ? "true" : "false");
	printf("Unload before copy: %s\n", m_bUnloadBeforeCopy ? "true" : "false");
	printf("App Isolation: %d\n", m_nAppIsolation);
	long lCount = 0;

	if (m_spHostList)
	{
		m_spHostList->get_length(&lCount);
		printf("Host count: %d\n", lCount);
	}
	else
	{
		printf("no hosts encountered\n");
	}

	if (m_spAppMappings)
	{
		lCount = 0;
		m_spAppMappings->get_length(&lCount);
		printf("App Mapping count: %d\n", lCount);
	}
	else
	{
		printf("no app mappings\n");
	}

	if (m_spAppFileGroups)
	{
		lCount = 0;
		m_spAppFileGroups->get_length(&lCount);
		printf("App file group count: %d\n", lCount);
	}
#endif
	return S_OK;
}

short CDepSettings::GetIISMajorVer()
{
	if (m_nIISMajorVer != -1)
		return m_nIISMajorVer;

	CRegKey verKey;
	if (ERROR_SUCCESS == verKey.Open(HKEY_LOCAL_MACHINE,
						_T("SOFTWARE\\Microsoft\\INetStp"),
						KEY_READ))
	{
		DWORD dwValue = 0xFFFFFFFF;
		if (ERROR_SUCCESS == verKey.QueryDWORDValue(_T("MajorVersion"), dwValue))
		{
			m_nIISMajorVer = (short)dwValue;
		}
	}
	return m_nIISMajorVer;
}