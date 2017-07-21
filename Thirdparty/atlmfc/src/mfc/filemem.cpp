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

////////////////////////////////////////////////////////////////////////////
// CMemFile implementation

CMemFile::CMemFile(UINT nGrowBytes)
{
	ASSERT(nGrowBytes <= UINT_MAX);

	m_nGrowBytes = nGrowBytes;
	m_nPosition = 0;
	m_nBufferSize = 0;
	m_nFileSize = 0;
	m_lpBuffer = NULL;
	m_bAutoDelete = TRUE;
}

CMemFile::CMemFile(BYTE* lpBuffer, UINT nBufferSize, UINT nGrowBytes)
{
	if (lpBuffer == NULL && nBufferSize != 0) 
	{
		AfxThrowInvalidArgException();
	}

	ASSERT(nGrowBytes <= UINT_MAX);

	m_nGrowBytes = nGrowBytes;
	m_nPosition = 0;
	m_nBufferSize = nBufferSize;
	m_nFileSize = nGrowBytes == 0 ? nBufferSize : 0;
	m_lpBuffer = lpBuffer;
	m_bAutoDelete = FALSE;
}

void CMemFile::Attach(BYTE* lpBuffer, UINT nBufferSize, UINT nGrowBytes) 
{
	if (lpBuffer == NULL && nBufferSize != 0) 
	{
		AfxThrowInvalidArgException();
	}
	
	ASSERT(m_lpBuffer == NULL);

	m_nGrowBytes = nGrowBytes;
	m_nPosition = 0;
	m_nBufferSize = nBufferSize;
	m_nFileSize = nGrowBytes == 0 ? nBufferSize : 0;
	m_lpBuffer = lpBuffer;
	m_bAutoDelete = FALSE;
}

BYTE* CMemFile::Detach()
{
	BYTE* lpBuffer = m_lpBuffer;
	m_lpBuffer = NULL;
	m_nFileSize = 0;
	m_nBufferSize = 0;
	m_nPosition = 0;

	return lpBuffer;
}

CMemFile::~CMemFile()
{
	// Close should have already been called, but we check anyway
	if (m_lpBuffer)
		Close();
	ASSERT(m_lpBuffer == NULL);

	m_nGrowBytes = 0;
	m_nPosition = 0;
	m_nBufferSize = 0;
	m_nFileSize = 0;
}

BYTE* CMemFile::Alloc(SIZE_T nBytes)
{
	return (BYTE*)malloc(nBytes);
}

BYTE* CMemFile::Realloc(BYTE* lpMem, SIZE_T nBytes)
{
	ASSERT(nBytes > 0);	// nBytes == 0 means free		
	return (BYTE*)realloc(lpMem, nBytes);
}

BYTE* CMemFile::Memcpy(BYTE* lpMemTarget, const BYTE* lpMemSource,
	SIZE_T nBytes)
{
	ASSERT(lpMemTarget != NULL);
	ASSERT(lpMemSource != NULL);

	ASSERT(AfxIsValidAddress(lpMemTarget, nBytes));
	ASSERT(AfxIsValidAddress(lpMemSource, nBytes, FALSE));

	Checked::memcpy_s(lpMemTarget, nBytes, lpMemSource, nBytes);
	return lpMemTarget;
}

void CMemFile::Free(BYTE* lpMem)
{
	ASSERT(lpMem != NULL);

	free(lpMem);
}

ULONGLONG CMemFile::GetPosition() const
{
	ASSERT_VALID(this);
	return m_nPosition;
}

void CMemFile::GrowFile(SIZE_T dwNewLen)
{
	ASSERT_VALID(this);

	if (dwNewLen > m_nBufferSize)
	{
		// grow the buffer
		SIZE_T dwNewBufferSize = m_nBufferSize;

		// watch out for buffers which cannot be grown!
		ASSERT(m_nGrowBytes != 0);
		if (m_nGrowBytes == 0)
			AfxThrowMemoryException();

		// determine new buffer size
		while (dwNewBufferSize < dwNewLen)
			dwNewBufferSize += m_nGrowBytes;

		// allocate new buffer
		BYTE* lpNew;
		if (m_lpBuffer == NULL)
			lpNew = Alloc(dwNewBufferSize);
		else
			lpNew = Realloc(m_lpBuffer, dwNewBufferSize);

		if (lpNew == NULL)
			AfxThrowMemoryException();

		m_lpBuffer = lpNew;
		m_nBufferSize = dwNewBufferSize;
	}
	ASSERT_VALID(this);
}

ULONGLONG CMemFile::GetLength() const
{
   ASSERT_VALID(this);

   return m_nFileSize;
}

void CMemFile::SetLength(ULONGLONG dwNewLen)
{
	ASSERT_VALID(this);

#ifdef WIN32
   if (dwNewLen > ULONG_MAX)
	  AfxThrowMemoryException();
#endif  // WIN32
	if (dwNewLen > m_nBufferSize)
		GrowFile((SIZE_T)dwNewLen);

	if (dwNewLen < m_nPosition)
		m_nPosition = (SIZE_T)dwNewLen;

	m_nFileSize = (SIZE_T)dwNewLen;
	ASSERT_VALID(this);
}

UINT CMemFile::Read(void* lpBuf, UINT nCount)
{
	ASSERT_VALID(this);

	if (nCount == 0)
		return 0;

	ASSERT(lpBuf != NULL);

	if (lpBuf == NULL) 
	{
		AfxThrowInvalidArgException();
	}

	ASSERT(AfxIsValidAddress(lpBuf, nCount));

	if (m_nPosition > m_nFileSize)
		return 0;

	UINT nRead;
	if (m_nPosition + nCount > m_nFileSize || m_nPosition + nCount < m_nPosition)
		nRead = (UINT)(m_nFileSize - m_nPosition);
	else
		nRead = nCount;

	Memcpy((BYTE*)lpBuf, (BYTE*)m_lpBuffer + m_nPosition, nRead);
	m_nPosition += nRead;

	ASSERT_VALID(this);

	return nRead;
}

void CMemFile::Write(const void* lpBuf, UINT nCount)
{
	ASSERT_VALID(this);

	if (nCount == 0)
		return;

	ASSERT(lpBuf != NULL);
	ASSERT(AfxIsValidAddress(lpBuf, nCount, FALSE));

	if (lpBuf == NULL) 
	{
		AfxThrowInvalidArgException();
	}
	//If we have no room for nCount, it must be an overflow
	if (m_nPosition + nCount < m_nPosition)
	{
		AfxThrowInvalidArgException();
	}

	if (m_nPosition + nCount > m_nBufferSize)
		GrowFile(m_nPosition + nCount);

	ENSURE(m_nPosition + nCount <= m_nBufferSize);

	Memcpy((BYTE*)m_lpBuffer + m_nPosition, (BYTE*)lpBuf, nCount);

	m_nPosition += nCount;

	if (m_nPosition > m_nFileSize)
		m_nFileSize = m_nPosition;

	ASSERT_VALID(this);
}

ULONGLONG CMemFile::Seek(LONGLONG lOff, UINT nFrom)
{
	ASSERT_VALID(this);
	ASSERT(nFrom == begin || nFrom == end || nFrom == current);

	LONGLONG lNewPos = m_nPosition;

	if (nFrom == begin)
		lNewPos = lOff;
	else if (nFrom == current)
		lNewPos += lOff;
	else if (nFrom == end) {
		if (lOff > 0) 
		{
			AfxThrowFileException(CFileException::badSeek);// offsets must be negative when seeking from the end
		}
		lNewPos = m_nFileSize + lOff;	
	} else
		return m_nPosition;

	if (lNewPos < 0)
		AfxThrowFileException(CFileException::badSeek);
	if (static_cast<DWORD>(lNewPos) > m_nFileSize)
		GrowFile((SIZE_T)lNewPos);

	m_nPosition = (SIZE_T)lNewPos;

	ASSERT_VALID(this);
	return m_nPosition;
}

void CMemFile::Flush()
{
	ASSERT_VALID(this);
}

void CMemFile::Close()
{
	ASSERT((m_lpBuffer == NULL && m_nBufferSize == 0) ||
		!m_bAutoDelete || AfxIsValidAddress(m_lpBuffer, (UINT)m_nBufferSize, FALSE));
	ASSERT(m_nFileSize <= m_nBufferSize);

	m_nGrowBytes = 0;
	m_nPosition = 0;
	m_nBufferSize = 0;
	m_nFileSize = 0;
	if (m_lpBuffer && m_bAutoDelete)
		Free(m_lpBuffer);
	m_lpBuffer = NULL;
}

void CMemFile::Abort()
{
	ASSERT_VALID(this);

	Close();
}

void CMemFile::LockRange(ULONGLONG /* dwPos */, ULONGLONG /* dwCount */)
{
	ASSERT_VALID(this);
	AfxThrowNotSupportedException();
}


void CMemFile::UnlockRange(ULONGLONG /* dwPos */, ULONGLONG /* dwCount */)
{
	ASSERT_VALID(this);
	AfxThrowNotSupportedException();
}

CFile* CMemFile::Duplicate() const
{
	ASSERT_VALID(this);
	AfxThrowNotSupportedException();
}

// only CMemFile supports "direct buffering" interaction with CArchive
UINT CMemFile::GetBufferPtr(UINT nCommand, UINT nCount,
	void** ppBufStart, void**ppBufMax)
{
	ASSERT(nCommand == bufferCheck || nCommand == bufferCommit ||
		nCommand == bufferRead || nCommand == bufferWrite);



	if (nCommand == bufferCheck)
	{
		// only allow direct buffering if we're 
		// growable
		if (m_nGrowBytes > 0)
			return bufferDirect;
		else
			return 0;
	}

	if (nCommand == bufferCommit)
	{
		// commit buffer
		ASSERT(ppBufStart == NULL);
		ASSERT(ppBufMax == NULL);
		m_nPosition += nCount;
		if (m_nPosition > m_nFileSize)
			m_nFileSize = m_nPosition;
		return 0;
	}


	ASSERT(nCommand == bufferWrite || nCommand == bufferRead);
	ASSERT(ppBufStart != NULL);
	ASSERT(ppBufMax != NULL);

	if (ppBufStart == NULL || ppBufMax == NULL) 
	{
		return 0;
	}

	// when storing, grow file as necessary to satisfy buffer request
	if (nCommand == bufferWrite)
	{
		if (m_nPosition + nCount < m_nPosition || m_nPosition + nCount < nCount)
		{
			AfxThrowInvalidArgException();
		}
		if (m_nPosition + nCount > m_nBufferSize)
		{
			GrowFile(m_nPosition + nCount);
		}
	}

	// store buffer max and min
	*ppBufStart = m_lpBuffer + m_nPosition;

	// end of buffer depends on whether you are reading or writing
	if (nCommand == bufferWrite)
		*ppBufMax = m_lpBuffer + min(m_nBufferSize, m_nPosition + nCount);
	else
	{
		if (nCount == (UINT)-1)
			nCount = UINT(m_nBufferSize - m_nPosition);
		*ppBufMax = m_lpBuffer + min(m_nFileSize, m_nPosition + nCount);
		m_nPosition += LPBYTE(*ppBufMax) - LPBYTE(*ppBufStart);
	}

	// return number of bytes in returned buffer space (may be <= nCount)
	return ULONG(LPBYTE(*ppBufMax) - LPBYTE(*ppBufStart));
}

/////////////////////////////////////////////////////////////////////////////
// CMemFile diagonstics

#ifdef _DEBUG
void CMemFile::Dump(CDumpContext& dc) const
{
	CFile::Dump(dc);

	dc << "m_nFileSize = " << ULONGLONG(m_nFileSize);
	dc << "\nm_nBufferSize = " << ULONGLONG(m_nBufferSize);
	dc << "\nm_nPosition = " << ULONGLONG(m_nPosition);
	dc << "\nm_nGrowBytes = " << ULONGLONG(m_nGrowBytes);

	dc << "\n";
}

void CMemFile::AssertValid() const
{
	CFile::AssertValid();

	ASSERT((m_lpBuffer == NULL && m_nBufferSize == 0) ||
		AfxIsValidAddress(m_lpBuffer, (UINT)m_nBufferSize, FALSE));
	ASSERT(m_nFileSize <= m_nBufferSize);
	// m_nPosition might be after the end of file, so we cannot ASSERT
	// its validity
}
#endif // _DEBUG


IMPLEMENT_DYNAMIC(CMemFile, CFile)

/////////////////////////////////////////////////////////////////////////////
