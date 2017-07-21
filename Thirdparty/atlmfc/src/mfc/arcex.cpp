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
#include "sal.h"



#define new DEBUG_NEW

#ifdef _DEBUG
// character strings to use for dumping CArchiveException
static const LPCSTR rgszCArchiveExceptionCause[] =
{
	"none",
	"generic",
	"readOnly",
	"endOfFile",
	"writeOnly",
	"badIndex",
	"badClass",
	"badSchema",
};
static const char szUnknown[] = "unknown";
#endif

BOOL CArchiveException::GetErrorMessage(_Out_z_cap_(nMaxError) LPTSTR lpszError, _In_ UINT nMaxError,
		_Out_opt_ PUINT pnHelpContext) const
{
	ASSERT(lpszError != NULL && AfxIsValidString(lpszError, nMaxError));
	if (lpszError == NULL)
		return FALSE;

	if (pnHelpContext != NULL)
		*pnHelpContext = m_cause + AFX_IDP_ARCH_NONE;

	// we can use CString here; archive errors aren't caused
	// by being out of memory.

	TRY
	{
		CString strMessage;
		CString strFileName = m_strFileName;
		if (strFileName.IsEmpty())
			strFileName.LoadString(AFX_IDS_UNNAMED_FILE);
		AfxFormatString1(strMessage,
			m_cause + AFX_IDP_ARCH_NONE, strFileName);
		Checked::tcsncpy_s(lpszError, nMaxError, strMessage, _TRUNCATE);
	}
	CATCH_ALL( pEx )
	{
		return FALSE;
	}
	END_CATCH_ALL

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CArchiveException

#ifdef _DEBUG
void CArchiveException::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	dc << " m_cause = ";
	if (m_cause >= 0 && m_cause < _countof(rgszCArchiveExceptionCause))
		dc << rgszCArchiveExceptionCause[m_cause];
	else
		dc << szUnknown;

	dc << "\n";
}
#endif //_DEBUG

void __declspec(noreturn) AFXAPI AfxThrowArchiveException(int cause,
	LPCTSTR lpszArchiveName /* = NULL */)
{
#ifdef _DEBUG
	LPCSTR lpsz;
	if (cause >= 0 && cause < _countof(rgszCArchiveExceptionCause))
		lpsz = rgszCArchiveExceptionCause[cause];
	else
		lpsz = szUnknown;
	TRACE(traceAppMsg, 0, "CArchive exception: %hs.\n", lpsz);

#endif //_DEBUG

	THROW(new CArchiveException(cause, lpszArchiveName));
}


IMPLEMENT_DYNAMIC(CArchiveException, CException)

/////////////////////////////////////////////////////////////////////////////
