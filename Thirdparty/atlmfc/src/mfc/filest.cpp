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
#include <errno.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>



#define new DEBUG_NEW

////////////////////////////////////////////////////////////////////////////
// Status information for all file classes
// In this file so everyone doesn't get the CTime package

/////////////////////////////////////////////////////////////////////////////
// CFileStatus diagnostics

#ifdef _DEBUG
void CFileStatus::Dump(CDumpContext& dc) const
{
	dc << "a CFileStatus at " << (void*)this;

	dc << "\nm_ctime = " << m_ctime;
	dc << "\nm_mtime = " << m_mtime;
	dc << "\nm_atime = " << m_atime;
	dc << "\nm_size = " << m_size;
	dc << "\nm_attribute = " << m_attribute;
	dc << "\nm_szFullName = " << m_szFullName;

	dc << "\n";
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CFile name handlers

CString CFile::GetFileName() const
{
	ASSERT_VALID(this);

	CFileStatus status;
	GetStatus(status);
	CString strResult;
	AfxGetFileName(status.m_szFullName, strResult.GetBuffer(_MAX_FNAME),
		_MAX_FNAME);
	strResult.ReleaseBuffer();
	return strResult;
}

CString CFile::GetFileTitle() const
{
	ASSERT_VALID(this);

	CFileStatus status;
	GetStatus(status);
	CString strResult;
	AfxGetFileTitle(status.m_szFullName, strResult.GetBuffer(_MAX_FNAME),
		_MAX_FNAME);
	strResult.ReleaseBuffer();
	return strResult;
}

CString CFile::GetFilePath() const
{
	ASSERT_VALID(this);

	CFileStatus status;
	GetStatus(status);
	return status.m_szFullName;
}

/////////////////////////////////////////////////////////////////////////////
// CFile Status implementation

BOOL CFile::GetStatus(CFileStatus& rStatus) const
{
	ASSERT_VALID(this);

	memset(&rStatus, 0, sizeof(CFileStatus));

	// copy file name from cached m_strFileName
	Checked::tcsncpy_s(rStatus.m_szFullName, _countof(rStatus.m_szFullName), m_strFileName, _TRUNCATE);

	if (m_hFile != hFileNull)
	{
		// get time current file size
		FILETIME ftCreate, ftAccess, ftModify;
		if (!::GetFileTime(m_hFile, &ftCreate, &ftAccess, &ftModify))
			return FALSE;


		LARGE_INTEGER li;

		if (::GetFileSizeEx(m_hFile, &li) == 0)
			return FALSE;

		rStatus.m_size = li.QuadPart;


		if (m_strFileName.IsEmpty())
			rStatus.m_attribute = 0;
		else
		{
			DWORD dwAttribute = ::GetFileAttributes(m_strFileName);

			// don't return an error for this because previous versions of MFC didn't
			if (dwAttribute == 0xFFFFFFFF)
				rStatus.m_attribute = 0;
			else
			{
				rStatus.m_attribute = (BYTE) dwAttribute;
#ifdef _DEBUG
				// MFC BUG: m_attribute is only a BYTE wide
				if (dwAttribute & ~0xFF)
					TRACE(traceAppMsg, 0, "Warning: CFile::GetStatus() returns m_attribute without high-order flags.\n");
#endif
			}
		}

		// convert times as appropriate
		// some file systems may not record file creation time, file access time etc
		if (CTime::IsValidFILETIME(ftCreate))
		{
			rStatus.m_ctime = CTime(ftCreate);
		}
		else
		{
			rStatus.m_ctime = CTime();
		}

		if (CTime::IsValidFILETIME(ftAccess))
		{
			rStatus.m_atime = CTime(ftAccess);
		}
		else
		{
			rStatus.m_atime = CTime();
		}
		
		if (CTime::IsValidFILETIME(ftModify))
		{
			rStatus.m_mtime = CTime(ftModify);
		}
		else
		{
			rStatus.m_mtime = CTime();
		}

		if (rStatus.m_ctime.GetTime() == 0)
			rStatus.m_ctime = rStatus.m_mtime;

		if (rStatus.m_atime.GetTime() == 0)
			rStatus.m_atime = rStatus.m_mtime;
	}
	return TRUE;
}

BOOL PASCAL CFile::GetStatus(LPCTSTR lpszFileName, CFileStatus& rStatus)
{
	ASSERT( lpszFileName != NULL );

	if ( lpszFileName == NULL ) 
	{
		return FALSE;
	}

	if ( lstrlen(lpszFileName) >= _MAX_PATH )
	{
		ASSERT(FALSE); // MFC requires paths with length < _MAX_PATH
		return FALSE;
	}
	
	// attempt to fully qualify path first
	if (!AfxFullPath(rStatus.m_szFullName, lpszFileName))
	{
		rStatus.m_szFullName[0] = '\0';
		return FALSE;
	}

	WIN32_FILE_ATTRIBUTE_DATA fileAttributeData;
	if (!GetFileAttributesEx(lpszFileName, GetFileExInfoStandard, &fileAttributeData))
		return FALSE;

	// strip attribute of NORMAL bit, our API doesn't have a "normal" bit.
	rStatus.m_attribute = (BYTE)
		(fileAttributeData.dwFileAttributes & ~FILE_ATTRIBUTE_NORMAL);

	rStatus.m_size = fileAttributeData.nFileSizeHigh;
	rStatus.m_size <<= 32;
	rStatus.m_size |= fileAttributeData.nFileSizeLow;

	// convert times as appropriate
	if (CTime::IsValidFILETIME(fileAttributeData.ftCreationTime))
	{
		rStatus.m_ctime = CTime(fileAttributeData.ftCreationTime);
	}
	else
	{
		rStatus.m_ctime = CTime();
	}

	if (CTime::IsValidFILETIME(fileAttributeData.ftLastAccessTime))
	{
		rStatus.m_atime = CTime(fileAttributeData.ftLastAccessTime);
	}
	else
	{
		rStatus.m_atime = CTime();
	}

	if (CTime::IsValidFILETIME(fileAttributeData.ftLastWriteTime))
	{
		rStatus.m_mtime = CTime(fileAttributeData.ftLastWriteTime);
	}
	else
	{
		rStatus.m_mtime = CTime();
	}

	if (rStatus.m_ctime.GetTime() == 0)
		rStatus.m_ctime = rStatus.m_mtime;

	if (rStatus.m_atime.GetTime() == 0)
		rStatus.m_atime = rStatus.m_mtime;

	return TRUE;
}

void AFX_CDECL AfxTimeToFileTime(const CTime& time, LPFILETIME pFileTime)
{
	ASSERT(pFileTime != NULL);

	if (pFileTime == NULL) 
	{
		AfxThrowInvalidArgException();
	}

	SYSTEMTIME sysTime;
	sysTime.wYear = (WORD)time.GetYear();
	sysTime.wMonth = (WORD)time.GetMonth();
	sysTime.wDay = (WORD)time.GetDay();
	sysTime.wHour = (WORD)time.GetHour();
	sysTime.wMinute = (WORD)time.GetMinute();
	sysTime.wSecond = (WORD)time.GetSecond();
	sysTime.wMilliseconds = 0;

	// convert system time to local file time
	FILETIME localTime;
	if (!SystemTimeToFileTime((LPSYSTEMTIME)&sysTime, &localTime))
		CFileException::ThrowOsError((LONG)::GetLastError());

	// convert local file time to UTC file time
	if (!LocalFileTimeToFileTime(&localTime, pFileTime))
		CFileException::ThrowOsError((LONG)::GetLastError());
}

void PASCAL CFile::SetStatus(LPCTSTR lpszFileName, const CFileStatus& status)
{
	DWORD wAttr;
	FILETIME creationTime;
	FILETIME lastAccessTime;
	FILETIME lastWriteTime;
	LPFILETIME lpCreationTime = NULL;
	LPFILETIME lpLastAccessTime = NULL;
	LPFILETIME lpLastWriteTime = NULL;

	if ((wAttr = GetFileAttributes((LPTSTR)lpszFileName)) == (DWORD)-1L)
		CFileException::ThrowOsError((LONG)GetLastError(), lpszFileName);

	if ((DWORD)status.m_attribute != wAttr && (wAttr & readOnly))
	{
		// Set file attribute, only if currently readonly.
		// This way we will be able to modify the time assuming the
		// caller changed the file from readonly.

		if (!SetFileAttributes((LPTSTR)lpszFileName, (DWORD)status.m_attribute))
			CFileException::ThrowOsError((LONG)GetLastError(), lpszFileName);
	}

	// last modification time
	if (status.m_mtime.GetTime() != 0)
	{
		AfxTimeToFileTime(status.m_mtime, &lastWriteTime);
		lpLastWriteTime = &lastWriteTime;

		// last access time
		if (status.m_atime.GetTime() != 0)
		{
			AfxTimeToFileTime(status.m_atime, &lastAccessTime);
			lpLastAccessTime = &lastAccessTime;
		}

		// create time
		if (status.m_ctime.GetTime() != 0)
		{
			AfxTimeToFileTime(status.m_ctime, &creationTime);
			lpCreationTime = &creationTime;
		}

		HANDLE hFile = ::CreateFile(lpszFileName, GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hFile == INVALID_HANDLE_VALUE)
			CFileException::ThrowOsError((LONG)::GetLastError(), lpszFileName);

		if (!SetFileTime((HANDLE)hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime))
		{
			LONG sc=(LONG)::GetLastError();
			::CloseHandle(hFile);
			CFileException::ThrowOsError(sc, lpszFileName);
		}

		if (!::CloseHandle(hFile))
			CFileException::ThrowOsError((LONG)::GetLastError(), lpszFileName);
	}

	if ((DWORD)status.m_attribute != wAttr && !(wAttr & readOnly))
	{
		if (!SetFileAttributes((LPTSTR)lpszFileName, (DWORD)status.m_attribute))
			CFileException::ThrowOsError((LONG)GetLastError(), lpszFileName);
	}
}

///////////////////////////////////////////////////////////////////////////////
// CMemFile::GetStatus implementation

BOOL CMemFile::GetStatus(CFileStatus& rStatus) const
{
	ASSERT_VALID(this);

	rStatus.m_ctime = 0;
	rStatus.m_mtime = 0;
	rStatus.m_atime = 0;
	rStatus.m_size = m_nFileSize;
	rStatus.m_attribute = normal;
	rStatus.m_szFullName[0] = '\0';
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
