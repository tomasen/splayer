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



#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// COleStreamFile implementation

COleStreamFile::COleStreamFile(LPSTREAM lpStream)
{
	m_lpStream = lpStream;

	m_strStorageName.Empty();
	if (m_lpStream != NULL)
	{		
		STATSTG statstg;
		if (m_lpStream->Stat(&statstg, 0) == S_OK)
		{
			if (statstg.pwcsName != NULL)
			{
				TCHAR szTemp[_MAX_PATH];
				const CString strPath(statstg.pwcsName);

				if (strPath.GetLength() >= _MAX_PATH)
				{
					ASSERT(FALSE);

					// Before throwing the exception out, need free the
					// memory of statstg.pwcsName.
					CoTaskMemFree(statstg.pwcsName);

					// MFC requires paths with length < _MAX_PATH
					// No other way to handle the error from a constructor
					AfxThrowFileException(CFileException::badPath);
				}

				// We call AfxFullPath because in earlier versions of MFC,
				// we have called it to transform all paths to absolute paths.
				// AfxFullPath reverts to "plain" copy behavior for non-path
				// inputs, and returns FALSE.

				// By design, we ignore the return value of AfxFullPath because
				// OLE Stream names don't have to be valid paths.
				AfxFullPath(szTemp, strPath);
				CoTaskMemFree(statstg.pwcsName);
				m_strStorageName = szTemp;
			}
		}
	}
}

COleStreamFile::~COleStreamFile()
{
	AFX_BEGIN_DESTRUCTOR

	if (m_lpStream != NULL && m_bCloseOnDelete)
	{
		Close();
		ASSERT(m_lpStream == NULL);
	}

	AFX_END_DESTRUCTOR
}

LPSTREAM COleStreamFile::Detach()
{
	LPSTREAM lpStream = m_lpStream;
	m_lpStream = NULL;  // detach and transfer ownership of m_lpStream
	return lpStream;
}

void COleStreamFile::Attach(LPSTREAM lpStream)
{
	ASSERT(m_lpStream == NULL); // already attached to an LPSTREAM?
	ASSERT(lpStream != NULL);
	
	if (lpStream == NULL)
	{
		AfxThrowInvalidArgException();
	}

	m_lpStream = lpStream;
	m_bCloseOnDelete = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// OLE streams helper

HRESULT AFXAPI _AfxReadFromStream(LPSTREAM pStream, void* lpBuf, UINT nCount, DWORD& nRead)
{
	if (nCount == 0)
	{
		nRead = 0;
		return S_OK;
	}

	ASSERT(AfxIsValidAddress(lpBuf, nCount));
	ASSERT(pStream != NULL);

	if (pStream == NULL || lpBuf == NULL)
	{
		return E_INVALIDARG;
	}

	// read from the stream
	SCODE sc = pStream->Read(lpBuf, nCount, &nRead);
	return ResultFromScode(sc);
}

/////////////////////////////////////////////////////////////////////////////
// OLE CFileException helpers

void AFXAPI _AfxFillOleFileException(CFileException* pError, SCODE sc)
{
	ASSERT(pError != NULL);
	ASSERT(FAILED(sc));

	int cause;  // portable CFileException.m_cause

	// error codes 255 or less are DOS/Win32 error codes
	if (SCODE_SEVERITY(sc) == SEVERITY_ERROR &&
		SCODE_FACILITY(sc) == FACILITY_STORAGE &&
		SCODE_CODE(sc) < 0x100)
	{
		ASSERT(SCODE_CODE(sc) != 0);

		// throw an exception matching to the DOS error
		//  (NOTE: only the DOS error part of the SCODE becomes m_lOsError)
		cause = CFileException::OsErrorToException(SCODE_CODE(sc));
		sc = (SCODE)SCODE_CODE(sc);
	}
	else
	{
		// attempt some conversion of storage specific error codes to generic
		//  CFileException causes...
		switch (sc)
		{
		case STG_E_INUSE:
		case STG_E_SHAREREQUIRED:
			cause = CFileException::sharingViolation;
			break;

		case STG_E_NOTCURRENT:
		case STG_E_REVERTED:
		case STG_E_CANTSAVE:
		case STG_E_OLDFORMAT:
		case STG_E_OLDDLL:
			cause = CFileException::genericException;
			break;

		default:
			cause = CFileException::genericException;
			break;
		}
	}

	// fill in pError
	pError->m_cause = cause;
	pError->m_lOsError = (LONG)sc;
}

void AFXAPI _AfxThrowOleFileException(SCODE sc)
{
	// ignore non-failure codes
	if (!FAILED(sc))
		return;

	// otherwise, construct and exception and throw it
	CFileException e;
	_AfxFillOleFileException(&e, sc);
	AfxThrowFileException(e.m_cause, e.m_lOsError);
}

/////////////////////////////////////////////////////////////////////////////
// COleStreamFile Attributes

BOOL COleStreamFile::GetStatus(CFileStatus& rStatus) const
{
	ASSERT_VALID(this);
	ASSERT(m_lpStream != NULL);

	// get status of the stream
	STATSTG statstg;
	if (m_lpStream->Stat(&statstg, 0) != S_OK)
		return FALSE;

	if (!CTime::IsValidFILETIME(statstg.mtime) ||
		!CTime::IsValidFILETIME(statstg.ctime) ||
		!CTime::IsValidFILETIME(statstg.atime))
	{
		return FALSE;
	}

	// map to CFileStatus struct
	rStatus.m_mtime = CTime(statstg.mtime);
	rStatus.m_ctime = CTime(statstg.ctime);
	rStatus.m_atime = CTime(statstg.atime);
	ASSERT(statstg.cbSize.HighPart == 0);
	rStatus.m_size = statstg.cbSize.LowPart;
	rStatus.m_attribute = 0;
	rStatus.m_szFullName[0] = '\0';
	if (statstg.pwcsName != NULL)
	{
		const CString strPath(statstg.pwcsName);

		// name was returned -- copy and free it
		Checked::tcsncpy_s(rStatus.m_szFullName, _countof(rStatus.m_szFullName), strPath.GetString(), _TRUNCATE);
		CoTaskMemFree(statstg.pwcsName);
	}
	return TRUE;
}

const CString COleStreamFile::GetStorageName() const
{
	ASSERT_VALID(this);
	return m_strStorageName;
}


ULONGLONG COleStreamFile::GetPosition() const
{
	ASSERT_VALID(this);
	ASSERT(m_lpStream != NULL);

	ULARGE_INTEGER liPosition;
	LARGE_INTEGER liZero; liZero.QuadPart = 0;
	SCODE sc = m_lpStream->Seek(liZero, STREAM_SEEK_CUR, &liPosition);
	if (sc != S_OK)
		_AfxThrowOleFileException(sc);

	return liPosition.QuadPart;
}


/////////////////////////////////////////////////////////////////////////////
// COleStreamFile Operations

BOOL COleStreamFile::OpenStream(LPSTORAGE lpStorage, LPCTSTR lpszStreamName,
	DWORD nOpenFlags, CFileException* pError)
{
	ASSERT_VALID(this);
	ASSERT(m_lpStream == NULL);
	ASSERT(lpStorage != NULL);
	ASSERT(AfxIsValidString(lpszStreamName));
	ASSERT(pError == NULL ||
		AfxIsValidAddress(pError, sizeof(CFileException)));

	if (lpStorage == NULL || lpszStreamName == NULL)
	{
		return FALSE;
	}

	const CStringW strStreamName(lpszStreamName);	

	SCODE sc = lpStorage->OpenStream(strStreamName.GetString(), NULL, nOpenFlags, 0, &m_lpStream);
	if (FAILED(sc) && pError != NULL)
		_AfxFillOleFileException(pError, sc);

	ASSERT(FAILED(sc) || m_lpStream != NULL);
	return !FAILED(sc);
}

BOOL COleStreamFile::CreateStream(LPSTORAGE lpStorage, LPCTSTR lpszStreamName,
	DWORD nOpenFlags, CFileException* pError)
{
	ASSERT_VALID(this);
	ASSERT(m_lpStream == NULL);
	ASSERT(lpStorage != NULL);
	ASSERT(AfxIsValidString(lpszStreamName));
	ASSERT(pError == NULL ||
		AfxIsValidAddress(pError, sizeof(CFileException)));

	if (lpStorage == NULL || lpszStreamName == NULL)
	{
		return FALSE;
	}

	STATSTG statstg;
	if (lpStorage->Stat(&statstg, 0) == S_OK)
	{
		if (statstg.pwcsName != NULL)
		{
			TCHAR szTemp[_MAX_PATH];
			const CString strPath(statstg.pwcsName);			

			// We call AfxFullPath because in earlier versions of MFC,
			// we have called it to transform all paths to absolute paths.
			// AfxFullPath reverts to "plain" copy behavior for non-path
			// inputs, and returns FALSE.

			// By design, we ignore the return value of AfxFullPath because
			// OLE Stream names don't have to be valid paths.
			AfxFullPath(szTemp, strPath);
			CoTaskMemFree(statstg.pwcsName);
			m_strStorageName = szTemp;
		}
	}

	const CStringW strStreamName(lpszStreamName);	
	SCODE sc = lpStorage->CreateStream(strStreamName.GetString(), nOpenFlags,	0, 0, &m_lpStream);

	if (FAILED(sc) && pError != NULL)
		_AfxFillOleFileException(pError, sc);

	ASSERT(FAILED(sc) || m_lpStream != NULL);
	return !FAILED(sc);
}

BOOL COleStreamFile::CreateMemoryStream(CFileException* pError)
{
	ASSERT_VALID(this);
	ASSERT(pError == NULL ||
		AfxIsValidAddress(pError, sizeof(CFileException)));

	SCODE sc = CreateStreamOnHGlobal(NULL, TRUE, &m_lpStream);
	if (FAILED(sc) && pError != NULL)
		_AfxFillOleFileException(pError, sc);

	ASSERT(FAILED(sc) || m_lpStream != NULL);
	return !FAILED(sc);
}

/////////////////////////////////////////////////////////////////////////////
// COleStreamFile Overrides

CFile* COleStreamFile::Duplicate() const
{
	ASSERT_VALID(this);
	ASSERT(m_lpStream != NULL);

	LPSTREAM lpStream;
	SCODE sc = m_lpStream->Clone(&lpStream);
	if (FAILED(sc))
		_AfxThrowOleFileException(sc);

	ASSERT(lpStream != NULL);
	COleStreamFile* pFile = NULL;

	TRY
	{
		// attempt to create the stream
		pFile = new COleStreamFile(lpStream);
		pFile->m_bCloseOnDelete = m_bCloseOnDelete;
	}
	CATCH_ALL(e)
	{
		// cleanup cloned stream
		lpStream->Release();
		THROW_LAST();
	}
	END_CATCH_ALL

	ASSERT(pFile != NULL);
	return pFile;
}

ULONGLONG COleStreamFile::Seek(LONGLONG lOff, UINT nFrom)
{
	ASSERT_VALID(this);
	ASSERT(m_lpStream != NULL);

	ASSERT(STREAM_SEEK_SET == begin);
	ASSERT(STREAM_SEEK_CUR == current);
	ASSERT(STREAM_SEEK_END == end);

	ULARGE_INTEGER liNewPosition;
	LARGE_INTEGER liOff;
   liOff.QuadPart = lOff;
	SCODE sc = m_lpStream->Seek(liOff, nFrom, &liNewPosition);
	if (sc != S_OK)
		_AfxThrowOleFileException(sc);

	return liNewPosition.QuadPart;
}

void COleStreamFile::SetLength(ULONGLONG dwNewLen)
{
	ASSERT_VALID(this);
	ASSERT(m_lpStream != NULL);

	ULARGE_INTEGER liNewLen;
   liNewLen.QuadPart = dwNewLen;
	SCODE sc = m_lpStream->SetSize(liNewLen);
	if (sc != S_OK)
		_AfxThrowOleFileException(sc);
}

ULONGLONG COleStreamFile::GetLength() const
{
	ASSERT_VALID(this);
	ASSERT(m_lpStream != NULL);

	// get status of the stream
	STATSTG statstg;
	SCODE sc = m_lpStream->Stat(&statstg, STATFLAG_NONAME);
	if (sc != S_OK)
		_AfxThrowOleFileException(sc);

	// map to CFileStatus struct
	return statstg.cbSize.QuadPart;
}

UINT COleStreamFile::Read(void* lpBuf, UINT nCount)
{
	ASSERT_VALID(this);
	ASSERT(m_lpStream != NULL);

	DWORD dwBytesRead;
	HRESULT hr = _AfxReadFromStream(m_lpStream, lpBuf, nCount, dwBytesRead);

	if (hr != S_OK)
		_AfxThrowOleFileException(hr);

	// always return number of bytes read
	return (UINT)dwBytesRead;
}

void COleStreamFile::Write(const void* lpBuf, UINT nCount)
{
	ASSERT_VALID(this);
	ASSERT(m_lpStream != NULL);

	if (nCount == 0)
		return;

	ASSERT(AfxIsValidAddress(lpBuf, nCount, FALSE));

	if (lpBuf == NULL)
	{	
		AfxThrowInvalidArgException();
	}

	// write to the stream
	DWORD dwBytesWritten;
	SCODE sc = m_lpStream->Write(lpBuf, nCount, &dwBytesWritten);
	if (sc != S_OK)
		_AfxThrowOleFileException(sc);

	// if no error, all bytes should have been written
	ASSERT((UINT)dwBytesWritten == nCount);
}

void COleStreamFile::LockRange(ULONGLONG dwPos, ULONGLONG dwCount)
{
	ASSERT_VALID(this);
	ASSERT(m_lpStream != NULL);

	// convert parameters to long integers
	ULARGE_INTEGER liPos;
   liPos.QuadPart = dwPos;
	ULARGE_INTEGER liCount;
   liCount.QuadPart = dwCount;

	// then lock the region
	SCODE sc = m_lpStream->LockRegion(liPos, liCount, LOCK_EXCLUSIVE);
	if (sc != S_OK)
		_AfxThrowOleFileException(sc);
}

void COleStreamFile::UnlockRange(ULONGLONG dwPos, ULONGLONG dwCount)
{
	ASSERT_VALID(this);
	ASSERT(m_lpStream != NULL);

	// convert parameters to long integers
	ULARGE_INTEGER liPos;
   liPos.QuadPart = dwPos;
	ULARGE_INTEGER liCount;
   liCount.QuadPart = dwCount;

	// then lock the region
	SCODE sc = m_lpStream->UnlockRegion(liPos, liCount, LOCK_EXCLUSIVE);
	if (sc != S_OK)
		_AfxThrowOleFileException(sc);
}

void COleStreamFile::Abort()
{
	ASSERT_VALID(this);

	if (m_lpStream != NULL)
	{
		m_lpStream->Revert();
		RELEASE(m_lpStream);
	}

	m_strStorageName.Empty();
}

void COleStreamFile::Flush()
{
	ASSERT_VALID(this);
	ASSERT(m_lpStream != NULL);

	// commit will return an error only if the stream is transacted
	SCODE sc = m_lpStream->Commit(0);
	if (sc != S_OK)
		_AfxThrowOleFileException(sc);
}

void COleStreamFile::Close()
{
	ASSERT_VALID(this);

	if (m_lpStream != NULL)
	{
		// commit the stream via Flush (which can be overriden)
		Flush();
		RELEASE(m_lpStream);
	}

	m_strStorageName.Empty();
}

IStream* COleStreamFile::GetStream() const
{
	return m_lpStream;
}

/////////////////////////////////////////////////////////////////////////////
// COleStreamFile diagnostics

#ifdef _DEBUG
void COleStreamFile::AssertValid() const
{
	CFile::AssertValid();
}

void COleStreamFile::Dump(CDumpContext& dc) const
{
	CFile::Dump(dc);

	dc << "m_lpStream = " << m_lpStream;
	dc << "m_strStorageName = \"" << m_strStorageName;
	dc << "\"\n";
}
#endif

////////////////////////////////////////////////////////////////////////////


IMPLEMENT_DYNAMIC(COleStreamFile, CFile)

////////////////////////////////////////////////////////////////////////////
