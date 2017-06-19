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
// CArchiveStream

#ifndef _AFX_NO_OLE_SUPPORT

CArchiveStream::CArchiveStream(CArchive* pArchive)
{
	m_pArchive = pArchive;
}

STDMETHODIMP_(ULONG)CArchiveStream::AddRef()
{
	return 1;
}

STDMETHODIMP_(ULONG)CArchiveStream::Release()
{
	return 0;
}

STDMETHODIMP CArchiveStream::QueryInterface(REFIID iid, LPVOID* ppvObj)
{
	if (iid == IID_IUnknown || iid == IID_IStream)
	{
		if (ppvObj == NULL) 
		{
			return E_POINTER;
		}
		*ppvObj = this;
		return NOERROR;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP CArchiveStream::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
	ASSERT(m_pArchive != NULL);
	ASSERT(m_pArchive->IsLoading());

	int nRead = 0;
	TRY
	{
		nRead = m_pArchive->Read(pv, cb);
	}
	CATCH_ALL(e)
	{
		DELETE_EXCEPTION(e);
		return E_UNEXPECTED;
	}
	END_CATCH_ALL

	if (pcbRead != NULL)
		*pcbRead = nRead;
	return NOERROR;
}

STDMETHODIMP CArchiveStream::Write(const void *pv, ULONG cb, ULONG *pcbWritten)
{
	ASSERT(m_pArchive != NULL);
	ASSERT(m_pArchive->IsStoring());

	int nWrite = 0;
	TRY
	{
		m_pArchive->Write(pv, cb);
		nWrite = cb;
	}
	CATCH_ALL(e)
	{
		DELETE_EXCEPTION(e);
		return E_UNEXPECTED;
	}
	END_CATCH_ALL

	if (pcbWritten != NULL)
		*pcbWritten = nWrite;
	return NOERROR;
}

STDMETHODIMP CArchiveStream::Seek(LARGE_INTEGER uliOffset, DWORD dwOrigin,
	ULARGE_INTEGER* puliNew)
{
	CFile* pFile = m_pArchive->GetFile();
	if (pFile == NULL)
		return E_NOTIMPL;
	m_pArchive->Flush();

	ASSERT(STREAM_SEEK_SET == CFile::begin);
	ASSERT(STREAM_SEEK_CUR == CFile::current);
	ASSERT(STREAM_SEEK_END == CFile::end);
	ULONGLONG lNew;
	TRY
	{
		lNew = pFile->Seek(uliOffset.QuadPart, (UINT)dwOrigin);
	}
	CATCH_ALL(e)
	{
		DELETE_EXCEPTION(e);
		return E_UNEXPECTED;
	}
	END_CATCH_ALL

	if (puliNew != NULL)
	  puliNew->QuadPart = lNew;

	return NOERROR;
}

STDMETHODIMP CArchiveStream::SetSize(ULARGE_INTEGER)
{
	return E_NOTIMPL;
}

STDMETHODIMP CArchiveStream::CopyTo(LPSTREAM, ULARGE_INTEGER, ULARGE_INTEGER*,
	ULARGE_INTEGER*)
{
	return E_NOTIMPL;
}

STDMETHODIMP CArchiveStream::Commit(DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP CArchiveStream::Revert()
{
	return E_NOTIMPL;
}

STDMETHODIMP CArchiveStream::LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP CArchiveStream::UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER,
	DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP CArchiveStream::Stat(STATSTG*, DWORD)
{
	return E_NOTIMPL;
}

STDMETHODIMP CArchiveStream::Clone(LPSTREAM*)
{
	return E_NOTIMPL;
}

#endif // _AFX_NO_OLE_SUPPORT
