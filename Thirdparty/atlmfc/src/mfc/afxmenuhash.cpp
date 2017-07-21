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
#include "afxtoolbar.h"
#include "afxmenuhash.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMenuHash afxMenuHash;

// Construction/Destruction
CMenuHash::CMenuHash()
{
	m_bIsActive = FALSE;
}

CMenuHash::~CMenuHash()
{
}

BOOL CMenuHash::SaveMenuBar(HMENU hMenu, CMFCToolBar* pBar)
{
	ASSERT_VALID(pBar);

	if (pBar->GetCount() == 0)
	{
		return FALSE;
	}

	HANDLE hFileOld = NULL;
	if (m_StoredMenues.Lookup(hMenu, hFileOld))
	{
		// Free unused handle:
		::CloseHandle(hFileOld);
	}

	// Get the temp path...
	CString strTempPath;
	GetTempPath(MAX_PATH, strTempPath.GetBuffer(MAX_PATH));
	strTempPath.ReleaseBuffer();

	// Create a temporary file for the output....
	CString strTempName;
	GetTempFileName(strTempPath, _T("AFX"), 0, strTempName.GetBuffer(MAX_PATH));
	strTempName.ReleaseBuffer();

	HANDLE hFile = ::CreateFile(strTempName, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		TRACE(_T("Can't create temporary file!\n"));
		return FALSE;
	}

	try
	{
		// Write a menubar context to file:
		CFile file(hFile);
		CArchive ar(&file, CArchive::store);

		m_bIsActive = TRUE;

		pBar->Serialize(ar);
		ar.Flush();

		m_bIsActive = FALSE;
	}
	catch(CArchiveException* pEx)
	{
		TRACE(_T("Archive exception in CMenuHash::SaveMenuBar()!\n"));
		pEx->Delete();
		::CloseHandle(hFile);

		m_bIsActive = FALSE;
		return FALSE;
	}
	catch(CMemoryException* pEx)
	{
		TRACE(_T("Memory exception in CMenuHash::SaveMenuBar()!\n"));
		pEx->Delete();
		::CloseHandle(hFile);

		m_bIsActive = FALSE;
		return FALSE;
	}
	catch(CFileException* pEx)
	{
		TRACE(_T("File exception in CMenuHash::SaveMenuBar()!\n"));
		pEx->Delete();
		::CloseHandle(hFile);

		m_bIsActive = FALSE;
		return FALSE;
	}

	m_StoredMenues.SetAt(hMenu, hFile);
	return TRUE;
}

BOOL CMenuHash::LoadMenuBar(HMENU hMenu, CMFCToolBar* pBar)
{
	ASSERT_VALID(pBar);
	ENSURE(hMenu != NULL);

	// Find a file handler associated with the menu:
	HANDLE hFile;
	if (!m_StoredMenues.Lookup(hMenu, hFile))
	{
		return FALSE;
	}

	// Rewind the file:
	if (::SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == 0xFFFFFFFF)
	{
		TRACE(_T("CMenuHash::LoadMenuBar(). Invalid file handle\n"));
		return FALSE;
	}

	try
	{
		CFile file(hFile);
		CArchive ar(&file, CArchive::load);

		m_bIsActive = TRUE;

		pBar->Serialize(ar);

		m_bIsActive = FALSE;
	}
	catch(CArchiveException* pEx)
	{
		TRACE(_T("Archive exception in CMenuHash::LoadMenuBar()!\n"));
		pEx->Delete();

		m_bIsActive = FALSE;
		return FALSE;
	}
	catch(CMemoryException* pEx)
	{
		TRACE(_T("Memory exception in CMenuHash::LoadMenuBar()!\n"));
		pEx->Delete();

		m_bIsActive = FALSE;
		return FALSE;
	}
	catch(CFileException* pEx)
	{
		TRACE(_T("File exception in CMenuHash::LoadMenuBar()!\n"));
		pEx->Delete();

		m_bIsActive = FALSE;
		return FALSE;
	}

	return TRUE;
}

BOOL CMenuHash::RemoveMenu(HMENU hMenu)
{
	HANDLE hFile = NULL;
	if (m_StoredMenues.Lookup(hMenu, hFile))
	{
		// Free unused handle:
		::CloseHandle(hFile);
		m_StoredMenues.RemoveKey(hMenu);
		return TRUE;
	}

	return FALSE;
}

void CMenuHash::CleanUp()
{
	for (POSITION pos = m_StoredMenues.GetStartPosition(); pos != NULL;)
	{
		HMENU hMenu;
		HANDLE hFile;

		m_StoredMenues.GetNextAssoc(pos, hMenu, hFile);
		::CloseHandle(hFile);
	}

	m_StoredMenues.RemoveAll();
}


