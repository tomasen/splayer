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


#ifdef _DEBUG
void PASCAL CException::operator delete(void* pbData)
{
	// check for proper exception object deletion
	CException* pException = (CException*)pbData;
	// use: pException->Delete(), do not use: delete pException
	ASSERT(pException->m_bReadyForDelete);
	ASSERT(pException->m_bAutoDelete > 0);

	// avoid crash when assert above is ignored
	if (pException->m_bReadyForDelete && pException->m_bAutoDelete > 0)
		CObject::operator delete(pbData);
}

#if _MSC_VER >= 1200
void PASCAL CException::operator delete(void* pbData,
	LPCSTR /* lpszFileName */, int /* nLine */)
{
	operator delete(pbData);
}
#endif



#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// AFX_EXCEPTION_CONTEXT (thread global state)

inline AFX_EXCEPTION_CONTEXT* AfxGetExceptionContext()
{
	DWORD lError = GetLastError();
	AFX_EXCEPTION_CONTEXT* pContext = &_afxThreadState->m_exceptionContext;
	SetLastError(lError);
	return pContext;
}

/////////////////////////////////////////////////////////////////////////////
// CException

CException::CException()
{
	// most exceptions are deleted when not needed
	m_bAutoDelete = TRUE;
#ifdef _DEBUG
	m_bReadyForDelete = FALSE;
#endif
}

CException::CException(BOOL bAutoDelete)
{
	// for exceptions which are not auto-delete (usually)
	m_bAutoDelete = bAutoDelete;
#ifdef _DEBUG
	m_bReadyForDelete = FALSE;
#endif
}

void CException::Delete()
{
	// delete exception if it is auto-deleting
	if (m_bAutoDelete > 0)
	{
#ifdef _DEBUG
		m_bReadyForDelete = TRUE;
#endif
		delete this;
	}
}

BOOL CException::GetErrorMessage(_Out_z_cap_(nMaxError) LPTSTR lpszError, _In_ UINT nMaxError,
		_Out_opt_ PUINT pnHelpContext /* = NULL */ ) const
{
	if (pnHelpContext != NULL)
		*pnHelpContext = 0;

	if (nMaxError != 0 && lpszError != NULL)
		*lpszError = '\0';

	return FALSE;
}

BOOL CException::GetErrorMessage(_Out_z_cap_(nMaxError) LPTSTR lpszError, _In_ UINT nMaxError,
		_Out_opt_ PUINT pnHelpContext /* = NULL */ )
{
	// Call the const version of GetErrorMessage
	return const_cast<const CException*>(this)->GetErrorMessage(lpszError, nMaxError, pnHelpContext);
}

int CException::ReportError(UINT nType /* = MB_OK */,
	UINT nError /* = 0 */)
{
	TCHAR   szErrorMessage[512];
	int     nDisposition;
	UINT    nHelpContext;

	if (GetErrorMessage(szErrorMessage, _countof(szErrorMessage), &nHelpContext))
		nDisposition = AfxMessageBox(szErrorMessage, nType, nHelpContext);
	else
	{
		if (nError == 0)
			nError = AFX_IDP_NO_ERROR_AVAILABLE;
		nDisposition = AfxMessageBox(nError, nType, nHelpContext);
	}
	return nDisposition;
}

/////////////////////////////////////////////////////////////////////////////
// AFX_EXCEPTION_LINK linked 'jmpbuf' and out-of-line helpers

AFX_EXCEPTION_LINK::AFX_EXCEPTION_LINK()
{
	// setup initial link state
	m_pException = NULL;    // no current exception yet

	// wire into top of exception link stack
	AFX_EXCEPTION_CONTEXT* pContext = AfxGetExceptionContext();
	m_pLinkPrev = pContext->m_pLinkTop;
	pContext->m_pLinkTop = this;
}

// out-of-line cleanup called from inline AFX_EXCEPTION_LINK destructor
void AFXAPI AfxTryCleanup()
{
	AFX_EXCEPTION_CONTEXT* pContext = AfxGetExceptionContext();
	AFX_EXCEPTION_LINK* pLinkTop = pContext->m_pLinkTop;

	// delete current exception
	ASSERT(pLinkTop != NULL);
	if (pLinkTop == NULL)
		return;
	if (pLinkTop->m_pException != NULL)
		pLinkTop->m_pException->Delete();

	// remove ourself from the top of the chain
	pContext->m_pLinkTop = pLinkTop->m_pLinkPrev;
}

// special out-of-line implementation of THROW_LAST (for auto-delete behavior)
void AFXAPI AfxThrowLastCleanup()
{
	AFX_EXCEPTION_CONTEXT* pContext = AfxGetExceptionContext();
	AFX_EXCEPTION_LINK* pLinkTop = pContext->m_pLinkTop;

	// check for THROW_LAST inside of auto-delete block
	if (pLinkTop != NULL)
	{
		// make sure current exception does not get auto-deleted
		pLinkTop->m_pException = NULL;
	}

	// THROW_LAST macro will do actual 'throw'
}



IMPLEMENT_DYNAMIC(CException, CObject)
IMPLEMENT_DYNAMIC(CSimpleException, CException)

IMPLEMENT_DYNAMIC(CMemoryException, CSimpleException)
CMemoryException _simpleMemoryException(FALSE, AFX_IDS_MEMORY_EXCEPTION);

IMPLEMENT_DYNAMIC(CNotSupportedException, CSimpleException)
CNotSupportedException _simpleNotSupportedException(FALSE, AFX_IDS_NOT_SUPPORTED_EXCEPTION);

IMPLEMENT_DYNAMIC(CInvalidArgException, CSimpleException)
CInvalidArgException _simpleInvalidArgException(FALSE, AFX_IDS_INVALID_ARG_EXCEPTION);

/////////////////////////////////////////////////////////////////////////////
// Standard exceptions


void CSimpleException::InitString()
{
	m_bInitialized = TRUE;
	m_bLoaded = (AfxLoadString(m_nResourceID,
		m_szMessage, _countof(m_szMessage)) != 0);
}

BOOL CSimpleException::GetErrorMessage(_Out_z_cap_(nMaxError) LPTSTR lpszError, _In_ UINT nMaxError,
		_Out_opt_ PUINT pnHelpContext) const
{
	ASSERT(lpszError != NULL && AfxIsValidString(lpszError, nMaxError));
	if (lpszError == NULL || nMaxError == 0)
		return FALSE;


	if (pnHelpContext != NULL)
		*pnHelpContext = 0;

	// if we didn't load our string (eg, we're a console app)
	// return a null string and FALSE

	if (!m_bInitialized)
		const_cast<CSimpleException*>(this)->InitString();

	if (m_bLoaded)
		Checked::tcsncpy_s(lpszError, nMaxError, m_szMessage, _TRUNCATE);
	else
		lpszError[0] = '\0';

	return m_bLoaded;
}

void __declspec(noreturn) AFXAPI AfxThrowMemoryException()
{
	THROW(&_simpleMemoryException);
}

void __declspec(noreturn) AFXAPI AfxThrowNotSupportedException()
{
	THROW(&_simpleNotSupportedException);
}

void __declspec(noreturn) AFXAPI AfxThrowInvalidArgException()
{
	THROW(&_simpleInvalidArgException);
}


/////////////////////////////////////////////////////////////////////////////
