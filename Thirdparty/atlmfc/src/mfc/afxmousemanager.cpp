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
#include "afxmousemanager.h"
#include "afxsettingsstore.h"
#include "afxregpath.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const CString strRegEntryName = _T("Mouse");
static const CString strMouseProfile = _T("MouseManager");

CMouseManager* afxMouseManager = NULL;

IMPLEMENT_SERIAL(CMouseManager, CObject, 1)

// Construction/Destruction
CMouseManager::CMouseManager()
{
	ENSURE(afxMouseManager == NULL);
	afxMouseManager = this;
}

CMouseManager::~CMouseManager()
{
	afxMouseManager = NULL;
}

BOOL CMouseManager::AddView(int iViewId, UINT uiViewNameResId, UINT uiIconId)
{
	CString strViewName;
	ENSURE(strViewName.LoadString(uiViewNameResId));

	return AddView(iViewId, strViewName, uiIconId);
}

BOOL CMouseManager::AddView(int iViewId, LPCTSTR lpszViewName, UINT uiIconId)
{
	ENSURE(lpszViewName != NULL);

	int iId;
	if (m_ViewsNames.Lookup(lpszViewName, iId)) // Already exist
	{
		return FALSE;
	}

	m_ViewsNames.SetAt(lpszViewName, iViewId);

	if (uiIconId != 0)
	{
		m_ViewsToIcons.SetAt(iViewId, uiIconId);
	}

	return TRUE;
}

UINT CMouseManager::GetViewDblClickCommand(int iId) const
{
	UINT uiCmd;

	if (!m_ViewsToCommands.Lookup(iId, uiCmd))
	{
		return 0;
	}

	return uiCmd;
}

void CMouseManager::GetViewNames(CStringList& listOfNames) const
{
	listOfNames.RemoveAll();

	for (POSITION pos = m_ViewsNames.GetStartPosition(); pos != NULL;)
	{
		CString strName;
		int iId;

		m_ViewsNames.GetNextAssoc(pos, strName, iId);
		listOfNames.AddTail(strName);
	}
}

int CMouseManager::GetViewIdByName(LPCTSTR lpszName) const
{
	ENSURE(lpszName != NULL);

	int iId;

	if (!m_ViewsNames.Lookup(lpszName, iId))
	{
		return -1;
	}

	return iId;
}

BOOL CMouseManager::LoadState(LPCTSTR lpszProfileName)
{
	CString strProfileName = ::AFXGetRegPath(strMouseProfile, lpszProfileName);

	BOOL bResult = FALSE;

	LPBYTE lpbData = NULL;
	UINT uiDataSize;

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (!reg.Open(strProfileName))
	{
		TRACE(_T("CMouseManager::LoadState: Can't open registry %s!\n"), strProfileName);
		return FALSE;
	}

	if (!reg.Read(strRegEntryName, &lpbData, &uiDataSize))
	{
		TRACE(_T("CMouseManager::LoadState: Can't load registry data %s!\n"), strProfileName);
		return FALSE;
	}

	try
	{
		CMemFile file(lpbData, uiDataSize);
		CArchive ar(&file, CArchive::load);

		Serialize(ar);
		bResult = TRUE;
	}
	catch(CMemoryException* pEx)
	{
		pEx->Delete();
		TRACE(_T("Memory exception in CMouseManager::LoadState()!\n"));
	}
	catch(CArchiveException* pEx)
	{
		pEx->Delete();
		TRACE(_T("CArchiveException exception in CMouseManager::LoadState()!\n"));
	}

	if (lpbData != NULL)
	{
		delete [] lpbData;
	}

	return bResult;
}

BOOL CMouseManager::SaveState(LPCTSTR lpszProfileName)
{
	CString strProfileName = ::AFXGetRegPath(strMouseProfile, lpszProfileName);

	BOOL bResult = FALSE;

	try
	{
		CMemFile file;

		{
			CArchive ar(&file, CArchive::store);

			Serialize(ar);
			ar.Flush();
		}

		UINT uiDataSize = (UINT) file.GetLength();
		LPBYTE lpbData = file.Detach();

		if (lpbData != NULL)
		{
			CSettingsStoreSP regSP;
			CSettingsStore& reg = regSP.Create(FALSE, FALSE);

			if (reg.CreateKey(strProfileName))
			{
				bResult = reg.Write(strRegEntryName, lpbData, uiDataSize);
			}

			free(lpbData);
		}
	}
	catch(CMemoryException* pEx)
	{
		pEx->Delete();
		TRACE(_T("Memory exception in CMouseManager::SaveState()!\n"));
	}
	catch(CArchiveException* pEx)
	{
		pEx->Delete();
		TRACE(_T("CArchiveException exception in CMouseManager::SaveState()!\n"));
	}

	return bResult;
}

void CMouseManager::Serialize(CArchive& ar)
{
	CObject::Serialize(ar);

	if (ar.IsLoading())
	{
		m_ViewsToCommands.RemoveAll();

		int iCount;
		ar >> iCount;

		for (int i = 0; i < iCount; i ++)
		{
			int iViewId;
			ar >> iViewId;

			UINT uiCmd;
			ar >> uiCmd;

			m_ViewsToCommands.SetAt(iViewId, uiCmd);
		}
	}
	else
	{
		int iCount = (int) m_ViewsToCommands.GetCount();
		ar << iCount;

		for (POSITION pos = m_ViewsToCommands.GetStartPosition(); pos != NULL;)
		{
			int iViewId;
			UINT uiCmd;

			m_ViewsToCommands.GetNextAssoc(pos, iViewId, uiCmd);

			ar << iViewId;
			ar << uiCmd;
		}
	}
}

void CMouseManager::SetCommandForDblClk(int iViewId, UINT uiCmd)
{
	if (uiCmd > 0)
	{
		m_ViewsToCommands.SetAt(iViewId, uiCmd);
	}
	else
	{
		m_ViewsToCommands.RemoveKey(iViewId);
	}
}

UINT CMouseManager::GetViewIconId(int iViewId) const
{
	UINT uiIconId;
	if (!m_ViewsToIcons.Lookup(iViewId, uiIconId))
	{
		return 0;
	}

	return uiIconId;
}



