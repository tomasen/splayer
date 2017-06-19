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
#include <afxtempl.h>
#include <wchar.h>
#include "sal.h"



#define new DEBUG_NEW

////////////////////////////////////////////////////////////////////////////
// Serialize member functions for low level classes put here
// for code swapping improvements

#ifdef _AFX_BYTESWAP
#error _AFX_BYTESWAP is not supported.
#endif

UINT_PTR AFXAPI AfxReadStringLength(CArchive& ar, int& nCharSize)
{
	ULONGLONG qwLength;
	DWORD dwLength;
	WORD wLength;
	BYTE bLength;

	nCharSize = sizeof(char);

	// First, try to read a one-byte length
	ar>>bLength;
	if (bLength < 0xff)
		return bLength;

	// Try a two-byte length
	ar>>wLength;
	if (wLength == 0xfffe)
	{
		// Unicode string.  Start over at 1-byte length
		nCharSize = sizeof(wchar_t);

		ar>>bLength;
		if (bLength < 0xff)
			return bLength;

		// Two-byte length
		ar>>wLength;
		// Fall through to continue on same branch as ANSI string
	}
	if (wLength < 0xffff)
		return wLength;

	// 4-byte length
	ar>>dwLength;
	if (dwLength < 0xffffffff)
		return dwLength;

	// 8-byte length
	ar>>qwLength;
#ifndef _WIN64  // Big strings aren't supported on Win32 clients
	if (qwLength > INT_MAX)
		AfxThrowArchiveException(CArchiveException::genericException);
#endif  // !_WIN64

	return (UINT_PTR)qwLength;
}

void AFXAPI AfxWriteStringLength(CArchive& ar, UINT_PTR nLength, BOOL bUnicode)
{
	if (bUnicode)
	{
		// Tag Unicode strings
		ar<<(BYTE)0xff;
		ar<<(WORD)0xfffe;
	}

	if (nLength < 255)
	{
		ar<<(BYTE)nLength;
	}
	else if (nLength < 0xfffe)
	{
		ar<<(BYTE)0xff;
		ar<<(WORD)nLength;
	}
	else if (nLength < 0xffffffff)
	{
		ar<<(BYTE)0xff;
		ar<<(WORD)0xffff;
		ar<<(DWORD)nLength;
	}
	else
	{
		ar<<(BYTE)0xff;
		ar<<(WORD)0xffff;
		ar<<(DWORD)0xffffffff;
		ar<<(ULONGLONG)nLength;
	}
}

// Runtime class serialization code
CObject* PASCAL CRuntimeClass::CreateObject(LPCSTR lpszClassName)
{
	ENSURE(lpszClassName);

	// attempt to find matching runtime class structure
	CRuntimeClass* pClass = FromName(lpszClassName);
	if (pClass == NULL)
	{
		// not found, trace a warning for diagnostic purposes
		TRACE(traceAppMsg, 0, "Warning: Cannot find %hs CRuntimeClass.  Class not defined.\n",
			lpszClassName);
		return NULL;
	}

	// attempt to create the object with the found CRuntimeClass
	CObject* pObject = pClass->CreateObject();
	return pObject;
}

CObject* PASCAL CRuntimeClass::CreateObject(LPCWSTR lpszClassName)
{	
	const CStringA strClassName(lpszClassName);
	return CRuntimeClass::CreateObject(lpszClassName ? strClassName.GetString() : NULL);
}

CRuntimeClass* PASCAL CRuntimeClass::FromName(LPCSTR lpszClassName)
{
	CRuntimeClass* pClass=NULL;

	ENSURE(lpszClassName);

	// search app specific classes
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	AfxLockGlobals(CRIT_RUNTIMECLASSLIST);
	for (pClass = pModuleState->m_classList; pClass != NULL;
		pClass = pClass->m_pNextClass)
	{
		if (lstrcmpA(lpszClassName, pClass->m_lpszClassName) == 0)
		{
			AfxUnlockGlobals(CRIT_RUNTIMECLASSLIST);
			return pClass;
		}
	}
	AfxUnlockGlobals(CRIT_RUNTIMECLASSLIST);
#ifdef _AFXDLL
	// search classes in shared DLLs
	AfxLockGlobals(CRIT_DYNLINKLIST);
	for (CDynLinkLibrary* pDLL = pModuleState->m_libraryList; pDLL != NULL;
		pDLL = pDLL->m_pNextDLL)
	{
		for (pClass = pDLL->m_classList; pClass != NULL;
			pClass = pClass->m_pNextClass)
		{
			if (lstrcmpA(lpszClassName, pClass->m_lpszClassName) == 0)
			{
				AfxUnlockGlobals(CRIT_DYNLINKLIST);
				return pClass;
			}
		}
	}
	AfxUnlockGlobals(CRIT_DYNLINKLIST);
#endif

	return NULL; // not found
}

CRuntimeClass* PASCAL CRuntimeClass::FromName(LPCWSTR lpszClassName)
{	
	const CStringA strClassName(lpszClassName);	
	if( lpszClassName == NULL )
		return NULL;
	return CRuntimeClass::FromName( strClassName.GetString() );
}

CRuntimeClass* PASCAL CRuntimeClass::Load(CArchive& ar, UINT* pwSchemaNum)
	// loads a runtime class description
{
	if(pwSchemaNum == NULL)
	{
		return NULL;
	}
	WORD nLen;
	char szClassName[64];

	WORD wTemp;
	ar >> wTemp; *pwSchemaNum = wTemp;
	ar >> nLen;

	// load the class name
	if (nLen >= _countof(szClassName) ||
		ar.Read(szClassName, nLen*sizeof(char)) != nLen*sizeof(char))
	{
		return NULL;
	}
	szClassName[nLen] = '\0';

	// match the string against an actual CRuntimeClass
	CRuntimeClass* pClass = FromName(szClassName);
	if (pClass == NULL)
	{
		// not found, trace a warning for diagnostic purposes
		TRACE(traceAppMsg, 0, "Warning: Cannot load %hs from archive.  Class not defined.\n",
			szClassName);
	}

	return pClass;
}

void CRuntimeClass::Store(CArchive& ar) const
	// stores a runtime class description
{
	WORD nLen = (WORD)lstrlenA(m_lpszClassName);
	ar << (WORD)m_wSchema << nLen;
	ar.Write(m_lpszClassName, nLen*sizeof(char));
}

////////////////////////////////////////////////////////////////////////////
// Archive object input/output

// minimum buffer size
enum { nBufSizeMin = 128 };

// default amount to grow m_pLoadArray upon insert
enum { nGrowSize = 64 };
// default size of hash table in m_pStoreMap when storing
enum { nHashSize = 137 };
// default size to grow collision blocks when storing
enum { nBlockSize = 16 };

////////////////////////////////////////////////////////////////////////////

CArchive::CArchive(CFile* pFile, UINT nMode, int nBufSize, void* lpBuf) 
{
	ASSERT_VALID(pFile);
	if(pFile == NULL)
	{
		AfxThrowInvalidArgException();
	}
	
	m_strFileName = pFile->GetFilePath();

	// initialize members not dependent on allocated buffer
	m_nMode = nMode;
	m_pFile = pFile;
	m_pSchemaMap = NULL;
	m_pLoadArray = NULL;
	m_pDocument = NULL;
	m_bForceFlat = TRUE;
	m_nObjectSchema = (UINT)-1; // start with invalid schema
	if (IsStoring())
		m_nGrowSize = nBlockSize;
	else
		m_nGrowSize = nGrowSize;
	m_nHashSize = nHashSize;

	// initialize the buffer.  minimum size is 128
	m_lpBufStart = (BYTE*)lpBuf;
	m_bUserBuf = TRUE;
	m_bDirectBuffer = FALSE;
	m_bBlocking = m_pFile->GetBufferPtr(CFile::bufferCheck)&CFile::bufferBlocking;

	if (nBufSize < nBufSizeMin)
	{
		// force use of private buffer of minimum size
		m_nBufSize = nBufSizeMin;
		m_lpBufStart = NULL;
	}
	else
		m_nBufSize = nBufSize;

	nBufSize = m_nBufSize;
	if (m_lpBufStart == NULL)
	{
		// check for CFile providing buffering support
		m_bDirectBuffer = m_pFile->GetBufferPtr(CFile::bufferCheck)&CFile::bufferDirect;
		if (!m_bDirectBuffer)
		{
			// no support for direct buffering, allocate new buffer
			m_lpBufStart = new BYTE[m_nBufSize];
			m_bUserBuf = FALSE;
		}
		else
		{
			// CFile* supports direct buffering!
			nBufSize = 0;   // will trigger initial FillBuffer
		}
	}

	if (!m_bDirectBuffer)
	{
		ASSERT(m_lpBufStart != NULL);
		ASSERT(AfxIsValidAddress(m_lpBufStart, nBufSize, IsStoring()));
	}
	m_lpBufMax = m_lpBufStart + nBufSize;
	m_lpBufCur = (IsLoading()) ? m_lpBufMax : m_lpBufStart;

	ASSERT(m_pStoreMap == NULL);        // same as m_pLoadArray
}

CArchive::~CArchive()
{
	// Close makes m_pFile NULL. If it is not NULL, we must Close the CArchive
	if (m_pFile != NULL && !(m_nMode & bNoFlushOnDelete))
		Close();

	Abort();    // abort completely shuts down the archive
}

void CArchive::Abort()
{
	ASSERT(m_bDirectBuffer || m_lpBufStart == NULL ||
		AfxIsValidAddress(m_lpBufStart, UINT(m_lpBufMax - m_lpBufStart), IsStoring()));
	ASSERT(m_bDirectBuffer || m_lpBufCur == NULL ||
		AfxIsValidAddress(m_lpBufCur, UINT(m_lpBufMax - m_lpBufCur), IsStoring()));

	// disconnect from the file
	m_pFile = NULL;

	if (!m_bUserBuf)
	{
		ASSERT(!m_bDirectBuffer);
		delete[] m_lpBufStart;
		m_lpBufStart = NULL;
		m_lpBufCur = NULL;
	}

	void* pTemp;
	if ( (NULL != m_pSchemaMap) && 
		(m_pSchemaMap->Lookup( 
				reinterpret_cast<void*>(static_cast<DWORD_PTR>(objTypeArrayRef)), 
				pTemp )))
	{
		CArray<LoadArrayObjType>* pObjTypeArray = static_cast<CArray<LoadArrayObjType>*>(pTemp);
		delete pObjTypeArray;
		pObjTypeArray = NULL;
	}

	delete m_pSchemaMap;
	m_pSchemaMap = NULL;

	// m_pStoreMap and m_pLoadArray are unioned, so we only need to delete one
	ASSERT((CObject*)m_pStoreMap == (CObject*)m_pLoadArray);
	delete (CObject*)m_pLoadArray;
	m_pLoadArray = NULL;
}

void CArchive::Close()
{
	ASSERT_VALID(m_pFile);

	Flush();
	m_pFile = NULL;
}

UINT CArchive::Read(void* lpBuf, UINT nMax)
{
	ASSERT_VALID(m_pFile);

	if (nMax == 0)
		return 0;

	ASSERT(lpBuf != NULL);

	if(lpBuf == NULL)
		return 0;

	ASSERT(AfxIsValidAddress(lpBuf, nMax));
	ASSERT(m_bDirectBuffer || m_lpBufStart != NULL);
	ASSERT(m_bDirectBuffer || m_lpBufCur != NULL);
	ASSERT(m_lpBufStart == NULL ||
		AfxIsValidAddress(m_lpBufStart, UINT(m_lpBufMax - m_lpBufStart), FALSE));
	ASSERT(m_lpBufCur == NULL ||
		AfxIsValidAddress(m_lpBufCur, UINT(m_lpBufMax - m_lpBufCur), FALSE));
	ASSERT(IsLoading());

	if(!IsLoading())
		AfxThrowArchiveException(CArchiveException::writeOnly,m_strFileName);

	// try to fill from buffer first
	UINT nMaxTemp = nMax;
	UINT nTemp = min(nMaxTemp, UINT(m_lpBufMax - m_lpBufCur));
	Checked::memcpy_s(lpBuf, nMaxTemp, m_lpBufCur, nTemp);
	m_lpBufCur += nTemp;
	lpBuf = (BYTE*)lpBuf + nTemp;
	nMaxTemp -= nTemp;

	if (nMaxTemp != 0)
	{
		ASSERT(m_lpBufCur == m_lpBufMax);

		// read rest in buffer size chunks
		nTemp = nMaxTemp - (nMaxTemp % m_nBufSize);
		UINT nRead = 0;

		UINT nLeft = nTemp;
		UINT nBytes;
		do
		{
			nBytes = m_pFile->Read(lpBuf, nLeft);
			lpBuf = (BYTE*)lpBuf + nBytes;
			nRead += nBytes;
			nLeft -= nBytes;
		}
		while ((nBytes > 0) && (nLeft > 0));

		nMaxTemp -= nRead;

		if (nMaxTemp > 0)
		{
			// read last chunk into buffer then copy
			if (nRead == nTemp)
			{
				ASSERT(m_lpBufCur == m_lpBufMax);
				ASSERT(nMaxTemp < UINT(m_nBufSize));

				// fill buffer (similar to CArchive::FillBuffer, but no exception)
				if (!m_bDirectBuffer)
				{
					UINT nLastLeft;
					UINT nLastBytes;

					if (!m_bBlocking)
						nLastLeft = max(nMaxTemp, UINT(m_nBufSize));
					else
						nLastLeft = nMaxTemp;
					BYTE* lpTemp = m_lpBufStart;
					nRead = 0;
					do
					{
						nLastBytes = m_pFile->Read(lpTemp, nLastLeft);
						lpTemp = lpTemp + nLastBytes;
						nRead += nLastBytes;
						nLastLeft -= nLastBytes;
					}
					while ((nLastBytes > 0) && (nLastLeft > 0) && nRead < nMaxTemp);

					m_lpBufCur = m_lpBufStart;
					m_lpBufMax = m_lpBufStart + nRead;
				}
				else
				{
					nRead = m_pFile->GetBufferPtr(CFile::bufferRead, m_nBufSize,
						(void**)&m_lpBufStart, (void**)&m_lpBufMax);
					ASSERT(nRead == size_t( m_lpBufMax-m_lpBufStart ));
					m_lpBufCur = m_lpBufStart;
				}

				// use first part for rest of read
				nTemp = min(nMaxTemp, UINT(m_lpBufMax - m_lpBufCur));
				Checked::memcpy_s(lpBuf, nMaxTemp, m_lpBufCur, nTemp);
				m_lpBufCur += nTemp;
				nMaxTemp -= nTemp;
			}
		}
	}
	return nMax - nMaxTemp;
}

void CArchive::Write(const void* lpBuf, UINT nMax)
{
	ASSERT_VALID(m_pFile);

	if (nMax == 0)
		return;

	ASSERT(lpBuf != NULL);

	if(lpBuf == NULL)
		return;

	ASSERT(AfxIsValidAddress(lpBuf, nMax, FALSE));  // read-only access needed
	ASSERT(m_bDirectBuffer || m_lpBufStart != NULL);
	ASSERT(m_bDirectBuffer || m_lpBufCur != NULL);
	ASSERT(m_lpBufStart == NULL ||
		AfxIsValidAddress(m_lpBufStart, UINT(m_lpBufMax - m_lpBufStart)));
	ASSERT(m_lpBufCur == NULL ||
		AfxIsValidAddress(m_lpBufCur, UINT(m_lpBufMax - m_lpBufCur)));
	ASSERT(IsStoring());

	if(!IsStoring())
		AfxThrowArchiveException(CArchiveException::readOnly,m_strFileName);

	// copy to buffer if possible
	UINT nTemp = min(nMax, (UINT)(m_lpBufMax - m_lpBufCur));
	Checked::memcpy_s(m_lpBufCur, (size_t)(m_lpBufMax - m_lpBufCur), lpBuf, nTemp);
	m_lpBufCur += nTemp;
	lpBuf = (BYTE*)lpBuf + nTemp;
	nMax -= nTemp;

	if (nMax > 0)
	{
		Flush();    // flush the full buffer

		// write rest of buffer size chunks
		nTemp = nMax - (nMax % m_nBufSize);
		m_pFile->Write(lpBuf, nTemp);
		lpBuf = (BYTE*)lpBuf + nTemp;
		nMax -= nTemp;

		if (m_bDirectBuffer)
		{
			// sync up direct mode buffer to new file position
			VERIFY(m_pFile->GetBufferPtr(CFile::bufferWrite, m_nBufSize,
				(void**)&m_lpBufStart, (void**)&m_lpBufMax) == (UINT)m_nBufSize);
			ASSERT((UINT)m_nBufSize == (UINT)(m_lpBufMax - m_lpBufStart));
			m_lpBufCur = m_lpBufStart;
		}

		// copy remaining to active buffer
		ENSURE(nMax < (UINT)m_nBufSize);
		ENSURE(m_lpBufCur == m_lpBufStart);
		Checked::memcpy_s(m_lpBufCur, nMax, lpBuf, nMax);
		m_lpBufCur += nMax;
	}
}

void CArchive::Flush()
{
	ASSERT_VALID(m_pFile);
	ASSERT(m_bDirectBuffer || m_lpBufStart != NULL);
	ASSERT(m_bDirectBuffer || m_lpBufCur != NULL);
	ASSERT(m_lpBufStart == NULL ||
		AfxIsValidAddress(m_lpBufStart, UINT(m_lpBufMax - m_lpBufStart), IsStoring()));
	ASSERT(m_lpBufCur == NULL ||
		AfxIsValidAddress(m_lpBufCur, UINT(m_lpBufMax - m_lpBufCur), IsStoring()));

	if (IsLoading())
	{
		// unget the characters in the buffer, seek back unused amount
		if (m_lpBufMax != m_lpBufCur)
			m_pFile->Seek(-(int(m_lpBufMax - m_lpBufCur)), CFile::current);
		m_lpBufCur = m_lpBufMax;    // empty
	}
	else
	{
		if (!m_bDirectBuffer)
		{
			// write out the current buffer to file
			if (m_lpBufCur != m_lpBufStart)
				m_pFile->Write(m_lpBufStart, ULONG(m_lpBufCur - m_lpBufStart));
		}
		else
		{
			// commit current buffer
			if (m_lpBufCur != m_lpBufStart)
				m_pFile->GetBufferPtr(CFile::bufferCommit, ULONG(m_lpBufCur - m_lpBufStart));
			// get next buffer
			VERIFY(m_pFile->GetBufferPtr(CFile::bufferWrite, m_nBufSize,
				(void**)&m_lpBufStart, (void**)&m_lpBufMax) == (UINT)m_nBufSize);
			ASSERT((UINT)m_nBufSize == (UINT)(m_lpBufMax - m_lpBufStart));
		}
		m_lpBufCur = m_lpBufStart;
	}
}

void CArchive::FillBuffer(UINT nBytesNeeded)
{
	ASSERT_VALID(m_pFile);
	ASSERT(IsLoading());

	if(!IsLoading())
		AfxThrowArchiveException(CArchiveException::writeOnly,m_strFileName);

	ASSERT(m_bDirectBuffer || m_lpBufStart != NULL);
	ASSERT(m_bDirectBuffer || m_lpBufCur != NULL);
	ASSERT(nBytesNeeded > 0);
	ASSERT(nBytesNeeded <= (UINT)m_nBufSize);
	ASSERT(m_lpBufStart == NULL ||
		AfxIsValidAddress(m_lpBufStart, UINT(m_lpBufMax - m_lpBufStart), FALSE));
	ASSERT(m_lpBufCur == NULL ||
		AfxIsValidAddress(m_lpBufCur, UINT(m_lpBufMax - m_lpBufCur), FALSE));

	UINT nUnused = UINT(m_lpBufMax - m_lpBufCur);
	ULONG nTotalNeeded = ((ULONG)nBytesNeeded) + nUnused;

	// fill up the current buffer from file
	if (!m_bDirectBuffer)
	{
		ASSERT(m_lpBufCur != NULL);
		ASSERT(m_lpBufStart != NULL);
		ASSERT(m_lpBufMax != NULL);

		if (m_lpBufCur > m_lpBufStart)
		{
			// copy unused
			if ((int)nUnused > 0)
			{
				Checked::memmove_s(m_lpBufStart, (size_t)(m_lpBufMax - m_lpBufStart), 
					m_lpBufCur, nUnused);
				m_lpBufCur = m_lpBufStart;
				m_lpBufMax = m_lpBufStart + nUnused;
			}

			// read to satisfy nBytesNeeded or nLeft if possible
			UINT nRead = nUnused;
			UINT nLeft;
			UINT nBytes;

			// Only read what we have to, to avoid blocking waiting on data 
			// we don't need
			if (m_bBlocking)  
				nLeft = nBytesNeeded-nUnused;
			else
				nLeft = m_nBufSize-nUnused;
			BYTE* lpTemp = m_lpBufStart + nUnused;
			do
			{
				nBytes = m_pFile->Read(lpTemp, nLeft);
				lpTemp = lpTemp + nBytes;
				nRead += nBytes;
				nLeft -= nBytes;
			}
			while (nBytes > 0 && nLeft > 0 && nRead < nTotalNeeded);

			m_lpBufCur = m_lpBufStart;
			m_lpBufMax = m_lpBufStart + nRead;
		}
	}
	else
	{
		// seek to unused portion and get the buffer starting there
		if (nUnused != 0)
			m_pFile->Seek(-(LONG)nUnused, CFile::current);
		UINT nActual = m_pFile->GetBufferPtr(CFile::bufferRead, m_nBufSize,
			(void**)&m_lpBufStart, (void**)&m_lpBufMax);
		ASSERT(nActual == (UINT)(m_lpBufMax - m_lpBufStart));
		m_lpBufCur = m_lpBufStart;
	}

	// not enough data to fill request?
	if ((ULONG)(m_lpBufMax - m_lpBufCur) < nTotalNeeded)
		AfxThrowArchiveException(CArchiveException::endOfFile);
}

void CArchive::WriteCount(DWORD_PTR dwCount)
{
	if (dwCount < 0xFFFF)
		*this << (WORD)dwCount;  // 16-bit count
	else
	{
		*this << (WORD)0xFFFF;
#ifndef _WIN64
		*this << (DWORD)dwCount;  // 32-bit count
#else  // _WIN64
		if (dwCount < 0xFFFFFFFF)
			*this << (DWORD)dwCount;  // 32-bit count
		else
		{
			*this << (DWORD)0xFFFFFFFF;
			*this << dwCount;
		}
#endif  // _WIN64
	}
}

DWORD_PTR CArchive::ReadCount()
{
	WORD wCount;
	*this >> wCount;
	if (wCount != 0xFFFF)
		return wCount;

	DWORD dwCount;
	*this >> dwCount;
#ifndef _WIN64
	return dwCount;
#else  // _WIN64
	if (dwCount != 0xFFFFFFFF)
		return dwCount;

	DWORD_PTR qwCount;
	*this >> qwCount;
	return qwCount;
#endif  // _WIN64
}

// special functions for text file input and output

void CArchive::WriteString(LPCTSTR lpsz)
{
	ASSERT(AfxIsValidString(lpsz));
	Write(lpsz, lstrlen(lpsz) * sizeof(TCHAR));
}

LPTSTR CArchive::ReadString(_Out_z_cap_(nMax+1) LPTSTR lpsz, _In_ UINT nMax)
{
	// if nMax is negative (such a large number doesn't make sense given today's
	// 2gb address space), then assume it to mean "keep the newline".
	int nStop = (int)nMax < 0 ? -(int)nMax : (int)nMax;
	ASSERT(AfxIsValidAddress(lpsz, (nStop+1) * sizeof(TCHAR)));

	if(lpsz == NULL)
		return NULL;

	_TUCHAR ch;
	int nRead = 0;

	TRY
	{
		while (nRead < nStop)
		{
			*this >> ch;

			// stop and end-of-line (trailing '\n' is ignored)
			if (ch == '\n' || ch == '\r')
			{
				if (ch == '\r')
					*this >> ch;
				// store the newline when called with negative nMax
				if ((int)nMax != nStop)
					lpsz[nRead++] = ch;
				break;
			}
			lpsz[nRead++] = ch;
		}
	}
	CATCH(CArchiveException, e)
	{
		if (e && e->m_cause == CArchiveException::endOfFile)
		{
			DELETE_EXCEPTION(e);
			if (nRead == 0)
				return NULL;
		}
		else
		{
			THROW_LAST();
		}
	}
	END_CATCH

	lpsz[nRead] = '\0';
	return lpsz;
}

BOOL CArchive::ReadString(CString& rString)
{
	rString = _T("");    // empty string without deallocating
	const int nMaxSize = 128;
	LPTSTR lpsz = rString.GetBuffer(nMaxSize);
	LPTSTR lpszResult;
	int nLen;
	for (;;)
	{
		lpszResult = ReadString(lpsz, (UINT)-nMaxSize); // store the newline
		rString.ReleaseBuffer();

		// if string is read completely or EOF
		if (lpszResult == NULL ||
			(nLen = (int)lstrlen(lpsz)) < nMaxSize ||
			lpsz[nLen-1] == '\n')
		{
			break;
		}

		nLen = rString.GetLength();
		lpsz = rString.GetBuffer(nMaxSize + nLen) + nLen;
	}

	// remove '\n' from end of string if present
	lpsz = rString.GetBuffer(0);
	nLen = rString.GetLength();
	if (nLen != 0 && lpsz[nLen-1] == '\n')
		rString.GetBufferSetLength(nLen-1);

	return lpszResult != NULL;
}

/////////////////////////////////////////////////////////////////////////////
