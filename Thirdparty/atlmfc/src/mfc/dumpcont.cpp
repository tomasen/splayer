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



/////////////////////////////////////////////////////////////////////////////
// Diagnostic Stream output

void CDumpContext::OutputString(LPCTSTR lpsz)
{
	// use C-runtime/OutputDebugString when m_pFile is NULL
	if (m_pFile == NULL)
	{
		TRACE(traceDumpContext, 0, _T("%s"), lpsz);
		return;
	}

	ASSERT( lpsz != NULL );
	if( lpsz == NULL )
		AfxThrowUserException();
	// otherwise, write the string to the file
	m_pFile->Write(lpsz, lstrlen(lpsz)*sizeof(TCHAR));
}

CDumpContext::CDumpContext(CFile* pFile)
{
	if (pFile)
		ASSERT_VALID(pFile);

	m_pFile = pFile;
	m_nDepth = 0;
}

void CDumpContext::Flush()
{
	if (m_pFile)
		m_pFile->Flush();
}

CDumpContext& CDumpContext::operator<<(LPCTSTR lpsz)
{
	if (lpsz == NULL)
	{
		OutputString(_T("NULL"));
		return *this;
	}

	ASSERT( lpsz != NULL );
	if( lpsz == NULL )
		AfxThrowUserException();

	if (m_pFile == NULL)
	{
		TCHAR szBuffer[512];
		LPTSTR lpBuf = szBuffer;
		while (*lpsz != '\0')
		{
			if (lpBuf > szBuffer + _countof(szBuffer) - 3)
			{
				*lpBuf = '\0';
				OutputString(szBuffer);
				lpBuf = szBuffer;
			}
			if (*lpsz == '\n')
				*lpBuf++ = '\r';
			*lpBuf++ = *lpsz++;
		}
		*lpBuf = '\0';
		OutputString(szBuffer);
		return *this;
	}

	m_pFile->Write(lpsz, lstrlen(lpsz)*sizeof(TCHAR));
	return *this;
}

CDumpContext& CDumpContext::operator<<(BYTE by)
{
	TCHAR szBuffer[32];

	_stprintf_s(szBuffer, _countof(szBuffer), _T("%d"), (DWORD)by);
	OutputString(szBuffer);

	return *this;
}

CDumpContext& CDumpContext::DumpAsHex(BYTE b)
{
	TCHAR szBuffer[32];

	_stprintf_s(szBuffer, _countof(szBuffer), _T("0x%02x"), (DWORD)b);
	OutputString(szBuffer);

	return *this;
}

CDumpContext& CDumpContext::operator<<(WORD w)
{
	TCHAR szBuffer[32];

	_stprintf_s(szBuffer, _countof(szBuffer), _T("%u"), (UINT) w);
	OutputString(szBuffer);

	return *this;
}

CDumpContext& CDumpContext::DumpAsHex(WORD w)
{
	TCHAR szBuffer[32];

	_stprintf_s(szBuffer, _countof(szBuffer), _T("0x%04x"), (DWORD)w);
	OutputString(szBuffer);

	return *this;
}

#ifdef _WIN64
CDumpContext& CDumpContext::operator<<(UINT u)
#else
CDumpContext& CDumpContext::operator<<(UINT_PTR u)
#endif
{
	TCHAR szBuffer[32];

	_stprintf_s(szBuffer, _countof(szBuffer), _T("%u"), u);
	OutputString(szBuffer);

	return *this;
}

#ifdef _WIN64
CDumpContext& CDumpContext::operator<<(LONG l)
#else
CDumpContext& CDumpContext::operator<<(LONG_PTR l)
#endif
{
	TCHAR szBuffer[32];

	_stprintf_s(szBuffer, _countof(szBuffer), _T("%d"), l);
	OutputString(szBuffer);

	return *this;
}

#ifdef _WIN64
CDumpContext& CDumpContext::operator<<(DWORD dw)
#else
CDumpContext& CDumpContext::operator<<(DWORD_PTR dw)
#endif
{
	TCHAR szBuffer[32];

	_stprintf_s(szBuffer, _countof(szBuffer), _T("%u"), dw);
	OutputString(szBuffer);

	return *this;
}

#ifdef _WIN64
CDumpContext& CDumpContext::operator<<(int n)
#else
CDumpContext& CDumpContext::operator<<(INT_PTR n)
#endif
{
	TCHAR szBuffer[32];

	_stprintf_s(szBuffer, _countof(szBuffer), _T("%d"), n);
	OutputString(szBuffer);

	return *this;
}

#ifdef _WIN64
CDumpContext& CDumpContext::DumpAsHex(UINT u)
#else
CDumpContext& CDumpContext::DumpAsHex(UINT_PTR u)
#endif
{
	TCHAR szBuffer[32];

	_stprintf_s(szBuffer, _countof(szBuffer), _T("0x%08x"), u);
	OutputString(szBuffer);

	return *this;
}

#ifdef _WIN64
CDumpContext& CDumpContext::DumpAsHex(LONG l)
#else
CDumpContext& CDumpContext::DumpAsHex(LONG_PTR l)
#endif
{
	TCHAR szBuffer[32];

	_stprintf_s(szBuffer, _countof(szBuffer), _T("0x%08x"), l);
	OutputString(szBuffer);

	return *this;
}

#ifdef _WIN64
CDumpContext& CDumpContext::DumpAsHex(DWORD dw)
#else
CDumpContext& CDumpContext::DumpAsHex(DWORD_PTR dw)
#endif
{
	TCHAR szBuffer[32];

	_stprintf_s(szBuffer, _countof(szBuffer), _T("0x%08x"), dw);
	OutputString(szBuffer);

	return *this;
}

#ifdef _WIN64
CDumpContext& CDumpContext::DumpAsHex(int n)
#else
CDumpContext& CDumpContext::DumpAsHex(INT_PTR n)
#endif
{
	TCHAR szBuffer[32];

	_stprintf_s(szBuffer, _countof(szBuffer), _T("0x%08x"), n);
	OutputString(szBuffer);

	return *this;
}

CDumpContext& CDumpContext::operator<<(LONGLONG n)
{
	TCHAR szBuffer[32];

	_stprintf_s(szBuffer, _countof(szBuffer), _T("%I64d"), n);
	OutputString(szBuffer);

	return *this;
}

CDumpContext& CDumpContext::operator<<(ULONGLONG n)
{
	TCHAR szBuffer[32];

	_stprintf_s(szBuffer, _countof(szBuffer), _T("%I64u"), n);
	OutputString(szBuffer);

	return *this;
}

CDumpContext& CDumpContext::DumpAsHex(LONGLONG n)
{
	TCHAR szBuffer[32];

	_stprintf_s(szBuffer, _countof(szBuffer), _T("0x%016I64x"), n);
	OutputString(szBuffer);

	return *this;
}

CDumpContext& CDumpContext::DumpAsHex(ULONGLONG n)
{
	TCHAR szBuffer[32];

	_stprintf_s(szBuffer, _countof(szBuffer), _T("0x%016I64x"), n);
	OutputString(szBuffer);

	return *this;
}

CDumpContext& CDumpContext::operator<<(const CObject* pOb)
{
	if (pOb == NULL)
		*this << _T("NULL");
	else
#ifdef _AFXDLL
		pOb->Dump(*this);
#else
		*this << _T("Unable to dump object in static release builds");
#endif

	return *this;
}

CDumpContext& CDumpContext::operator<<(const CObject& ob)
{
	return *this << &ob;
}

CDumpContext& CDumpContext::operator<<(const void* lp)
{
	TCHAR szBuffer[32];

	// prefix a pointer with "$" and print in hex
	_stprintf_s(szBuffer, _countof(szBuffer), _T("$%p"), lp);
	OutputString(szBuffer);

	return *this;
}

CDumpContext& CDumpContext::operator<<(HWND h)
{
	return *this << (void*)h;
}

CDumpContext& CDumpContext::operator<<(HDC h)
{
	return *this << (void*)h;
}

CDumpContext& CDumpContext::operator<<(HMENU h)
{
	return *this << (void*)h;
}

CDumpContext& CDumpContext::operator<<(HACCEL h)
{
	return *this << (void*)h;
}

CDumpContext& CDumpContext::operator<<(HFONT h)
{
	return *this << (void*)h;
}

/////////////////////////////////////////////////////////////////////////////
// Formatted output

void CDumpContext::HexDump(LPCTSTR lpszLine, BYTE* pby,
	int nBytes, int nWidth)
// do a simple hex-dump (8 per line) to a CDumpContext
//  the "lpszLine" is a string to print at the start of each line
//    (%lx should be used to expand the current address)
{
	ASSERT(nBytes > 0);
	if( nBytes <= 0 )
		AfxThrowInvalidArgException();
	ASSERT(nWidth > 0);
	if( nWidth <= 0 )
		AfxThrowInvalidArgException();
	ASSERT(AfxIsValidString(lpszLine));
	if( lpszLine == NULL )
		AfxThrowInvalidArgException();
	ASSERT(AfxIsValidAddress(pby, nBytes, FALSE));
	if( pby == NULL )
		AfxThrowInvalidArgException();

	int nRow = 0;
	TCHAR szBuffer[32];

	while (nBytes--)
	{
		if (nRow == 0)
		{
			_stprintf_s(szBuffer, _countof(szBuffer), lpszLine, pby);
			*this << szBuffer;
		}

		_stprintf_s(szBuffer, _countof(szBuffer), _T(" %02X"), *pby++);
		*this << szBuffer;

		if (++nRow >= nWidth)
		{
			*this << _T("\n");
			nRow = 0;
		}
	}
	if (nRow != 0)
		*this << _T("\n");
}

/////////////////////////////////////////////////////////////////////////////

#ifdef _UNICODE
// special version for ANSI characters
CDumpContext& CDumpContext::operator<<(LPCSTR lpsz)
{
	if (lpsz == NULL)
	{
		OutputString(L"(NULL)");
		return *this;
	}

	// limited length
	TCHAR szBuffer[512];
	_mbstowcsz(szBuffer, lpsz, _countof(szBuffer));
	szBuffer[511] = 0;
	return *this << szBuffer;
}
#else   //_UNICODE
// special version for WIDE characters
CDumpContext& CDumpContext::operator<<(LPCWSTR lpsz)
{
	if (lpsz == NULL)
	{
		OutputString("(NULL)");
		return *this;
	}

	// limited length
	char szBuffer[512];
	_wcstombsz(szBuffer, lpsz, _countof(szBuffer));
	szBuffer[511] = 0;
	return *this << szBuffer;
}
#endif  //!_UNICODE

/////////////////////////////////////////////////////////////////////////////
