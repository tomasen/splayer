//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
#pragma once

#define INVALID_SETTING -1

// CDepSettings holds all of our deployment settings
class CDepSettings
{
public:
	CDepSettings();
	unsigned int Load(LPCSTR szSettingsPath);
	HRESULT GetHostList(IXMLDOMNodeList **ppList);
	HRESULT GetAppMappingList(IXMLDOMNodeList **ppList);
	HRESULT GetFileGroups(IXMLDOMNodeList **ppList);
	LPCTSTR GetVirtDirName(){ return m_strVirtDirName; }
	LPCTSTR GetVirtDirFSPath(){ return m_strVirtDirFSPath; }
	LPCTSTR GetExtensionFileName(){ return m_strExtFileName; }
	bool GetRegIsapi(){ return m_bRegIsapi; }
	bool GetUnloadBeforeCopy(){ return m_bUnloadBeforeCopy; }
	bool SkipVirtDirCreation(){ return m_bDoNotCreateVirtDir; }
	int GetAppIsolation(){ return m_nAppIsolation == INVALID_SETTING ? 0 : m_nAppIsolation; }
	short GetIISMajorVer();
private:
	CComPtr<IXMLDOMNodeList> m_spHostList;
	CComPtr<IXMLDOMNodeList> m_spAppMappings;
	CComPtr<IXMLDOMNodeList> m_spAppFileGroups;
	CString m_strVirtDirName;
	CString m_strVirtDirFSPath;
	CString m_strExtFileName;
	bool m_bRegIsapi;
	bool m_bDoNotCreateVirtDir;
	bool m_bUnloadBeforeCopy;
	short m_nAppIsolation;
	short m_nIISMajorVer;

};