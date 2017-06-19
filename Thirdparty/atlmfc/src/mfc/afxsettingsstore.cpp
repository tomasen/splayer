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
#include "afxsettingsstore.h"

IMPLEMENT_DYNCREATE(CSettingsStore, CObject)

static CString __stdcall PreparePath(LPCTSTR lpszPath)
{
	ENSURE(lpszPath != NULL);

	CString strPath = lpszPath;

	int iPathLen = strPath.GetLength();
	if (iPathLen > 0 && strPath [iPathLen - 1] == _T('\\'))
	{
		strPath = strPath.Left(iPathLen - 1);
	}

	return strPath;
}

CSettingsStore::CSettingsStore() :
	m_bReadOnly(FALSE), m_bAdmin(FALSE), m_dwUserData(0)
{
}

CSettingsStore::CSettingsStore(BOOL bAdmin, BOOL bReadOnly) :
	m_bReadOnly(bReadOnly), m_bAdmin(bAdmin), m_dwUserData(0)
{
	m_reg.m_hKey = bAdmin ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
}

CSettingsStore::~CSettingsStore()
{
	Close();
}

BOOL CSettingsStore::CreateKey(LPCTSTR lpszPath)
{
	if (m_bReadOnly)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return m_reg.Create(m_reg.m_hKey, PreparePath(lpszPath)) == ERROR_SUCCESS;
}

BOOL CSettingsStore::Open(LPCTSTR lpszPath)
{
	return m_reg.Open(m_reg.m_hKey, PreparePath(lpszPath),
		m_bReadOnly ?(KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS | KEY_NOTIFY) : KEY_ALL_ACCESS) == ERROR_SUCCESS;
}

void CSettingsStore::Close()
{
	m_reg.Close();
}

BOOL CSettingsStore::Write(LPCTSTR lpszValueName, int nValue)
{
	return Write(lpszValueName, (DWORD) nValue);
}

BOOL CSettingsStore::Write(LPCTSTR lpszValueName, DWORD dwValue)
{
	if (m_bReadOnly)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return m_reg.SetDWORDValue(lpszValueName, dwValue) == ERROR_SUCCESS;
}

BOOL CSettingsStore::Write(LPCTSTR lpszValueName, LPCTSTR lpszData)
{
	if (m_bReadOnly)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return m_reg.SetStringValue(lpszValueName, lpszData) == ERROR_SUCCESS;
}

BOOL CSettingsStore::Write(LPCTSTR lpszValueName, CObject& obj)
{
	if (m_bReadOnly)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	BOOL bRes = FALSE;

	try
	{
		CMemFile file;

		{
			CArchive ar(&file, CArchive::store);
			obj.Serialize(ar);
			ar.Flush();
		}

		DWORD dwDataSize = (DWORD) file.GetLength();
		LPBYTE lpbData = file.Detach();

		if (lpbData == NULL)
		{
			return FALSE;
		}

		bRes = Write(lpszValueName, lpbData, (UINT) dwDataSize);
		free(lpbData);
	}
	catch(CMemoryException* pEx)
	{
		pEx->Delete();
		TRACE(_T("Memory exception in CSettingsStore::Write()!\n"));
	}

	return bRes;
}

BOOL CSettingsStore::Write(LPCTSTR lpszValueName, CObject* pObj)
{
	ASSERT_VALID(pObj);
	return Write(lpszValueName, *pObj);
}

BOOL CSettingsStore::Write(LPCTSTR lpszValueName, const CRect& rect)
{
	if (m_bReadOnly)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	BOOL bRes = FALSE;
	try
	{
		CMemFile file;

		{
			CArchive ar(&file, CArchive::store);
			ar << rect;
			ar.Flush();
		}

		DWORD dwDataSize = (DWORD) file.GetLength();
		LPBYTE lpbData = file.Detach();

		if (lpbData == NULL)
		{
			return FALSE;
		}

		bRes = Write(lpszValueName, lpbData, (UINT) dwDataSize);
		free(lpbData);
	}
	catch(CMemoryException* pEx)
	{
		pEx->Delete();
		TRACE(_T("Memory exception in CSettingsStore::Write()!\n"));
		return FALSE;
	}

	return bRes;
}

BOOL CSettingsStore::Write(LPCTSTR lpszValueName, LPBYTE pData, UINT nBytes)
{
	if (m_bReadOnly)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return m_reg.SetBinaryValue(lpszValueName, pData, (ULONG) nBytes) == ERROR_SUCCESS;
}

BOOL CSettingsStore::Read(LPCTSTR lpszValueName, int& nValue)
{
	return Read(lpszValueName, (DWORD&)nValue);
}

BOOL CSettingsStore::Read(LPCTSTR lpszValueName, DWORD& dwValue)
{
	return m_reg.QueryDWORDValue(lpszValueName, dwValue) == ERROR_SUCCESS;
}

BOOL CSettingsStore::Read(LPCTSTR lpszValueName, CString& strValue)
{
	ENSURE(lpszValueName != NULL);

	strValue.Empty();

	DWORD dwCount = 0;
	if (m_reg.QueryStringValue(lpszValueName, NULL, &dwCount) != ERROR_SUCCESS)
	{
		return FALSE;
	}

	if (dwCount == 0)
	{
		return TRUE;
	}

	LPTSTR szValue = new TCHAR [dwCount + 1];

	BOOL bRes = m_reg.QueryStringValue(lpszValueName, szValue, &dwCount) == ERROR_SUCCESS;
	if (bRes)
	{
		strValue = szValue;
	}

	delete [] szValue;
	return bRes;
}

BOOL CSettingsStore::Read(LPCTSTR lpszValueName, CRect& rect)
{
	BOOL bSucess = FALSE;
	BYTE* pData = NULL;
	UINT uDataSize;

	if (!Read(lpszValueName, &pData, &uDataSize))
	{
		ENSURE(pData == NULL);
		return FALSE;
	}

	ENSURE(pData != NULL);

	try
	{
		CMemFile file(pData, uDataSize);
		CArchive ar(&file, CArchive::load);

		ar >> rect;
		bSucess = TRUE;
	}
	catch(CMemoryException* pEx)
	{
		pEx->Delete();
		TRACE(_T("Memory exception in CSettingsStore::Read()!\n"));
	}
	catch(CArchiveException* pEx)
	{
		pEx->Delete();
		TRACE(_T("CArchiveException exception in CSettingsStore::Read()!\n"));
	}

	delete [] pData;
	return bSucess;
}

BOOL CSettingsStore::Read(LPCTSTR lpszValueName, BYTE** ppData, UINT* pcbData)
{
	ENSURE(lpszValueName != NULL);
	ENSURE(ppData != NULL);
	ENSURE(pcbData != NULL);

	*ppData = NULL;
	*pcbData = 0;

	if (m_reg.QueryBinaryValue(lpszValueName, NULL, (ULONG*)pcbData) != ERROR_SUCCESS || *pcbData == 0)
	{
		return FALSE;
	}

	*ppData = new BYTE [*pcbData];

	if (m_reg.QueryBinaryValue(lpszValueName, *ppData, (ULONG*)pcbData) != ERROR_SUCCESS)
	{
		delete [] *ppData;
		*ppData = NULL;
		return FALSE;
	}

	return TRUE;
}

BOOL CSettingsStore::Read(LPCTSTR lpszValueName, CObject& obj)
{
	BOOL bSucess = FALSE;
	BYTE* pData = NULL;
	UINT uDataSize;

	if (!Read(lpszValueName, &pData, &uDataSize))
	{
		ENSURE(pData == NULL);
		return FALSE;
	}

	ENSURE(pData != NULL);

	try
	{
		CMemFile file(pData, uDataSize);
		CArchive ar(&file, CArchive::load);

		obj.Serialize(ar);
		bSucess = TRUE;
	}
	catch(CMemoryException* pEx)
	{
		pEx->Delete();
		TRACE(_T("Memory exception in CSettingsStore::Read()!\n"));
	}
	catch(CArchiveException* pEx)
	{
		pEx->Delete();
		TRACE(_T("CArchiveException exception in CSettingsStore::Read()!\n"));
	}

	delete [] pData;
	return bSucess;
}

BOOL CSettingsStore::Read(LPCTSTR lpszValueName, CObject*& pObj)
{
	BOOL bSucess = FALSE;
	BYTE* pData = NULL;
	UINT uDataSize;

	if (!Read(lpszValueName, &pData, &uDataSize))
	{
		ENSURE(pData == NULL);
		return FALSE;
	}

	ENSURE(pData != NULL);

	try
	{
		CMemFile file(pData, uDataSize);
		CArchive ar(&file, CArchive::load);
		ar >> pObj;

		bSucess = TRUE;
	}
	catch(CMemoryException* pEx)
	{
		pEx->Delete();
		TRACE(_T("Memory exception in CSettingsStore::Read()!\n"));
	}
	catch(CArchiveException* pEx)
	{
		pEx->Delete();
		TRACE(_T("CArchiveException exception in CSettingsStore::Read()!\n"));
	}

	delete [] pData;
	return bSucess;
}

BOOL CSettingsStore::DeleteValue(LPCTSTR lpszValue)
{
	return m_reg.DeleteValue(lpszValue) == ERROR_SUCCESS;
}

BOOL CSettingsStore::DeleteKey(LPCTSTR lpszPath, BOOL bAdmin)
{
	if (m_bReadOnly)
	{
		return FALSE;
	}

	m_reg.Close();
	m_reg.m_hKey = bAdmin ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;

	return m_reg.RecurseDeleteKey(PreparePath(lpszPath)) == ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// CSettingsStoreSP - Helper class that manages "safe" CSettingsStore pointer

CRuntimeClass* CSettingsStoreSP::m_pRTIDefault = NULL;

BOOL __stdcall CSettingsStoreSP::SetRuntimeClass(CRuntimeClass* pRTI)
{
	if (pRTI != NULL &&
		!pRTI->IsDerivedFrom(RUNTIME_CLASS(CSettingsStore)))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	m_pRTIDefault = pRTI;
	return TRUE;
}

CSettingsStore& CSettingsStoreSP::Create(BOOL bAdmin, BOOL bReadOnly)
{
	if (m_pRegistry != NULL)
	{
		ASSERT(FALSE);
		ASSERT_VALID(m_pRegistry);
		return *m_pRegistry;
	}

	if (m_pRTIDefault == NULL)
	{
		m_pRegistry = new CSettingsStore;
	}
	else
	{
		ASSERT(m_pRTIDefault->IsDerivedFrom(RUNTIME_CLASS(CSettingsStore)));
		m_pRegistry = DYNAMIC_DOWNCAST(CSettingsStore, m_pRTIDefault->CreateObject());
	}

	ASSERT_VALID(m_pRegistry);

	m_pRegistry->m_bReadOnly = bReadOnly;
	m_pRegistry->m_bAdmin = bAdmin;
	m_pRegistry->m_reg.m_hKey = bAdmin ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
	m_pRegistry->m_dwUserData = m_dwUserData;

	return *m_pRegistry;
}


