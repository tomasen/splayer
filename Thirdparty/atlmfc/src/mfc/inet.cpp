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
#include <afxinet.h>
#include "sal.h"

#pragma warning(disable: 4706) // assignment within conditional



#define new DEBUG_NEW

#ifndef _AFXDLL
#pragma comment(lib, "wininet.lib")
#endif

/////////////////////////////////////////////////////////////////////////////
// non-localized useful strings

typedef struct tagServiceTable {
	DWORD dwService;
	LPCTSTR pstrIdentifier;
} SvcTable;

AFX_STATIC_DATA const TCHAR _afxURLftp[] = _T("ftp://");
AFX_STATIC_DATA const TCHAR _afxURLgopher[] = _T("gopher://");
AFX_STATIC_DATA const TCHAR _afxURLhttp[] = _T("http://");

const LPCTSTR CHttpConnection::szHtmlVerbs[] = {
	_T("POST"),
	_T("GET"),
	_T("HEAD"),
	_T("PUT"),
	_T("LINK"),
	_T("DELETE"),
	_T("UNLINK"),
};


/////////////////////////////////////////////////////////////////////////////
// map of HINTERNETs to CInternetSessions* for callbacks

// forward declared because we need a #pragma -- see end of this file

class CSessionMapPtrToPtr : public CMapPtrToPtr
{
private:
	CCriticalSection m_sect;

public:
	CSessionMapPtrToPtr() { }
	~CSessionMapPtrToPtr() { }

	void SetAt(HINTERNET hInternet, CInternetSession* pSess)
	{
		m_sect.Lock();
		CMapPtrToPtr::SetAt(hInternet, pSess);
		m_sect.Unlock();
	}

	void RemoveKey(HINTERNET hInternet)
	{
		m_sect.Lock();
		CMapPtrToPtr::RemoveKey(hInternet);
		m_sect.Unlock();
	}

	BOOL Lookup(HINTERNET hInternet, CInternetSession*& refpSession)
	{
		BOOL bRet;
		m_sect.Lock();
		bRet = CMapPtrToPtr::Lookup(hInternet, (void*&) refpSession);
		m_sect.Unlock();
		return bRet;
	}
};

extern CSessionMapPtrToPtr _afxSessionMap;


/////////////////////////////////////////////////////////////////////////////
// Global Functions

AFX_STATIC BOOL AFXAPI _AfxParseURLWorker(LPCTSTR pstrURL,
	LPURL_COMPONENTS lpComponents, DWORD& dwServiceType,
	INTERNET_PORT& nPort, DWORD dwFlags)
{
	// this function will return bogus stuff if lpComponents
	// isn't set up to copy the components

	ASSERT(lpComponents != NULL && pstrURL != NULL);
	if (lpComponents == NULL || pstrURL == NULL)
		return FALSE;
	ASSERT(lpComponents->dwHostNameLength == 0 ||
			lpComponents->lpszHostName != NULL);
	ASSERT(lpComponents->dwUrlPathLength == 0 ||
			lpComponents->lpszUrlPath != NULL);
	ASSERT(lpComponents->dwUserNameLength == 0 ||
			lpComponents->lpszUserName != NULL);
	ASSERT(lpComponents->dwPasswordLength == 0 ||
			lpComponents->lpszPassword != NULL);

	ASSERT(AfxIsValidAddress(lpComponents, sizeof(URL_COMPONENTS), TRUE));

	LPTSTR pstrCanonicalizedURL;
	TCHAR szCanonicalizedURL[INTERNET_MAX_URL_LENGTH];
	DWORD dwNeededLength = INTERNET_MAX_URL_LENGTH;
	BOOL bRetVal;
	BOOL bMustFree = FALSE;

	// Decoding is done in InternetCrackUrl/UrlUnescape 
	// so we don't need the ICU_DECODE flag here.
	
	DWORD dwCanonicalizeFlags = dwFlags &
		(ICU_NO_ENCODE | ICU_NO_META |
		ICU_ENCODE_SPACES_ONLY | ICU_BROWSER_MODE);

	DWORD dwCrackFlags = 0;
 	
	BOOL bUnescape = FALSE;

	if((dwFlags & (ICU_ESCAPE | ICU_DECODE)) && (lpComponents->dwUrlPathLength != 0) )
	{
	
		// We use only the ICU_ESCAPE flag for decoding even if
		// ICU_DECODE is passed.
		
		// Also, if ICU_BROWSER_MODE is passed we do the unescaping
		// manually because InternetCrackUrl doesn't do
		// Browser mode unescaping
		
		if (dwFlags & ICU_BROWSER_MODE)
			bUnescape = TRUE;
		else
			dwCrackFlags |= ICU_ESCAPE;
	}

	bRetVal = InternetCanonicalizeUrl(pstrURL, szCanonicalizedURL,
		&dwNeededLength, dwCanonicalizeFlags);

	if (!bRetVal)
	{
		if (::GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			return FALSE;

		pstrCanonicalizedURL = new TCHAR[dwNeededLength];
		if (pstrCanonicalizedURL == NULL)
			return FALSE;

		bMustFree = TRUE;
		bRetVal = InternetCanonicalizeUrl(pstrURL, pstrCanonicalizedURL,
			&dwNeededLength, dwCanonicalizeFlags);
		if (!bRetVal)
		{
			delete [] pstrCanonicalizedURL;
			return FALSE;
		}
	}
	else
		pstrCanonicalizedURL = szCanonicalizedURL;

	// now that it's safely canonicalized, crack it

	bRetVal = InternetCrackUrl(pstrCanonicalizedURL, 0,
						dwCrackFlags, lpComponents);

	if(bUnescape)
	{
		if(FAILED(UrlUnescape(lpComponents->lpszUrlPath,NULL,NULL,URL_UNESCAPE_INPLACE | URL_DONT_UNESCAPE_EXTRA_INFO)))
		{
			if (bMustFree)
				delete [] pstrCanonicalizedURL;
		
			return FALSE;
		}
		
		lpComponents->dwUrlPathLength = lstrlen(lpComponents->lpszUrlPath);
	}
	
	if (bMustFree)
		delete [] pstrCanonicalizedURL;

	// convert to MFC-style service ID

	if (!bRetVal)
		dwServiceType = AFX_INET_SERVICE_UNK;
	else
	{
		nPort = lpComponents->nPort;
		switch (lpComponents->nScheme)
		{
		case INTERNET_SCHEME_FTP:
			dwServiceType = AFX_INET_SERVICE_FTP;
			break;

		case INTERNET_SCHEME_GOPHER:
			dwServiceType = AFX_INET_SERVICE_GOPHER;
			break;

		case INTERNET_SCHEME_HTTP:
			dwServiceType = AFX_INET_SERVICE_HTTP;
			break;

		case INTERNET_SCHEME_HTTPS:
			dwServiceType = AFX_INET_SERVICE_HTTPS;
			break;

		case INTERNET_SCHEME_FILE:
			dwServiceType = AFX_INET_SERVICE_FILE;
			break;

		case INTERNET_SCHEME_NEWS:
			dwServiceType = AFX_INET_SERVICE_NNTP;
			break;

		case INTERNET_SCHEME_MAILTO:
			dwServiceType = AFX_INET_SERVICE_MAILTO;
			break;

		default:
			dwServiceType = AFX_INET_SERVICE_UNK;
		}
	}

	return bRetVal;
}

BOOL AFXAPI AfxParseURLEx(LPCTSTR pstrURL, DWORD& dwServiceType,
	CString& strServer, CString& strObject, INTERNET_PORT& nPort,
	CString& strUsername, CString& strPassword, DWORD dwFlags/* = 0*/)
{
	dwServiceType = AFX_INET_SERVICE_UNK;

	ASSERT(pstrURL != NULL);
	if (pstrURL == NULL)
		return FALSE;

	URL_COMPONENTS urlComponents;
	memset(&urlComponents, 0, sizeof(URL_COMPONENTS));
	urlComponents.dwStructSize = sizeof(URL_COMPONENTS);

	urlComponents.dwHostNameLength = INTERNET_MAX_HOST_NAME_LENGTH;
	urlComponents.lpszHostName = strServer.GetBuffer(INTERNET_MAX_HOST_NAME_LENGTH+1);
	urlComponents.dwUrlPathLength = INTERNET_MAX_PATH_LENGTH;
	urlComponents.lpszUrlPath = strObject.GetBuffer(INTERNET_MAX_PATH_LENGTH+1);
	urlComponents.dwUserNameLength = INTERNET_MAX_USER_NAME_LENGTH;
	urlComponents.lpszUserName = strUsername.GetBuffer(INTERNET_MAX_USER_NAME_LENGTH+1);
	urlComponents.dwPasswordLength = INTERNET_MAX_PASSWORD_LENGTH;
	urlComponents.lpszPassword = strPassword.GetBuffer(INTERNET_MAX_PASSWORD_LENGTH+1);

	BOOL bRetVal = _AfxParseURLWorker(pstrURL, &urlComponents,
					dwServiceType, nPort, dwFlags);

	strServer.ReleaseBuffer();
	strObject.ReleaseBuffer();
	strUsername.ReleaseBuffer();
	strPassword.ReleaseBuffer();
	return bRetVal;
}

BOOL AFXAPI AfxParseURL(LPCTSTR pstrURL, DWORD& dwServiceType,
	CString& strServer, CString& strObject, INTERNET_PORT& nPort)
{
	dwServiceType = AFX_INET_SERVICE_UNK;

	ASSERT(pstrURL != NULL);
	if (pstrURL == NULL)
		return FALSE;

	URL_COMPONENTS urlComponents;
	memset(&urlComponents, 0, sizeof(URL_COMPONENTS));
	urlComponents.dwStructSize = sizeof(URL_COMPONENTS);

	urlComponents.dwHostNameLength = INTERNET_MAX_URL_LENGTH;
	urlComponents.lpszHostName = strServer.GetBuffer(INTERNET_MAX_URL_LENGTH+1);
	urlComponents.dwUrlPathLength = INTERNET_MAX_URL_LENGTH;
	urlComponents.lpszUrlPath = strObject.GetBuffer(INTERNET_MAX_URL_LENGTH+1);

	BOOL bRetVal = _AfxParseURLWorker(pstrURL, &urlComponents,
					dwServiceType, nPort, ICU_BROWSER_MODE);

	strServer.ReleaseBuffer();
	strObject.ReleaseBuffer();
	return bRetVal;
}


DWORD AFXAPI AfxGetInternetHandleType(HINTERNET hQuery)
{
	DWORD dwServiceType;
	DWORD dwTypeLen = sizeof(dwServiceType);
	if (hQuery == NULL ||
		!InternetQueryOption(hQuery, INTERNET_OPTION_HANDLE_TYPE,
			&dwServiceType, &dwTypeLen))
		return AFX_INET_SERVICE_UNK;
	else
		return dwServiceType;
}

AFX_STATIC BOOL AFXAPI 
_AfxQueryCStringInternetOption(HINTERNET hHandle, DWORD dwOption, CString& refString)
{
	DWORD dwLength = 0;
	LPTSTR pstrBuffer;

	if (hHandle == NULL)
		return FALSE;

	if (!InternetQueryOption(hHandle, dwOption, NULL, &dwLength) &&
		GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
		refString.Empty();
		return FALSE;
	}

	pstrBuffer = refString.GetBuffer(dwLength);
	BOOL bRet = InternetQueryOption(hHandle, dwOption, pstrBuffer, &dwLength);
	refString.ReleaseBuffer();
	return bRet;
}

#ifdef _DEBUG
void AFXAPI AfxInternetStatusCallbackDebug(HINTERNET hInternet,
	DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation,
	DWORD dwStatusInformationLength)
{
	UNUSED_ALWAYS(hInternet);
	TRACE(traceInternet, 1, "Internet ctxt=%d: ", dwContext);

	switch (dwInternetStatus)
	{
	case INTERNET_STATUS_RESOLVING_NAME:
		TRACE(traceInternet, 1, _T("resolving name for %s\n"), lpvStatusInformation);
		break;

	case INTERNET_STATUS_NAME_RESOLVED:
		TRACE(traceInternet, 1, _T("resolved name for %s!\n"), lpvStatusInformation);
		break;

	case INTERNET_STATUS_HANDLE_CREATED:
		TRACE(traceInternet, 1, "handle %8.8X created\n", hInternet);
		break;

	case INTERNET_STATUS_CONNECTING_TO_SERVER:
		{
		sockaddr* pSockAddr = (sockaddr*) lpvStatusInformation;
		TRACE(traceInternet, 1, _T("connecting to socket address '%s'\n"), pSockAddr->sa_data);
		}
		break;

	case INTERNET_STATUS_REQUEST_SENT:
		TRACE(traceInternet, 1, "request sent!\n");
		break;

	case INTERNET_STATUS_SENDING_REQUEST:
		TRACE(traceInternet, 1, "sending request...\n");
		break;

	case INTERNET_STATUS_CONNECTED_TO_SERVER:
		TRACE(traceInternet, 1, "connected to socket address!\n");
		break;

	case INTERNET_STATUS_RECEIVING_RESPONSE:
		TRACE(traceInternet, 1, "receiving response...\n");
		break;

	case INTERNET_STATUS_RESPONSE_RECEIVED:
		TRACE(traceInternet, 1, "response received!\n");
		break;

	case INTERNET_STATUS_CLOSING_CONNECTION:
		TRACE(traceInternet, 1, "closing connection %8.8X\n", hInternet);
		break;

	case INTERNET_STATUS_CONNECTION_CLOSED:
		TRACE(traceInternet, 1, "connection %8.8X closed!\n", hInternet);
		break;

	case INTERNET_STATUS_HANDLE_CLOSING:
		TRACE(traceInternet, 1, "handle %8.8X closed!\n", hInternet);
		break;

	case INTERNET_STATUS_REQUEST_COMPLETE:
		if (dwStatusInformationLength == sizeof(INTERNET_ASYNC_RESULT))
		{
			INTERNET_ASYNC_RESULT* pResult = (INTERNET_ASYNC_RESULT*) lpvStatusInformation;
			TRACE(traceInternet, 1, "request complete, dwResult = %8.8X, dwError = %8.8X\n",
				pResult->dwResult, pResult->dwError);
		}
		else
			TRACE(traceInternet, 1, "request complete.\n");
		break;

	case INTERNET_STATUS_CTL_RESPONSE_RECEIVED:
	case INTERNET_STATUS_REDIRECT:
	default:
		TRACE(traceInternet, 1, "Unknown status: %d\n", dwInternetStatus);
		break;
	}
}
#endif // _DEBUG

void AFXAPI AfxInternetStatusCallback(HINTERNET hInternet, DWORD_PTR dwContext,
	DWORD dwInternetStatus, LPVOID lpvStatusInformation,
	DWORD dwStatusInformationLength)
{
	CInternetSession* pSession;

#ifdef _DEBUG
	AfxInternetStatusCallbackDebug(hInternet, dwContext,
		dwInternetStatus, lpvStatusInformation, dwStatusInformationLength);
#endif

	if (_afxSessionMap.Lookup(hInternet, pSession))
	{
		pSession->OnStatusCallback(dwContext, dwInternetStatus,
			lpvStatusInformation, dwStatusInformationLength);
	}

	// note that an entry we can't match is simply ignored as
	// WININET can send notifications for handles that we can't
	// see -- such as when using InternetOpenURL()
}


/////////////////////////////////////////////////////////////////////////////
// CInternetSession

CInternetSession::~CInternetSession()
{
	Close();
}

CInternetSession::CInternetSession(LPCTSTR pstrAgent /* = NULL */,
	DWORD_PTR dwContext /* = 1 */,
	DWORD dwAccessType /* = PRE_CONFIG_INTERNET_ACCESS */,
	LPCTSTR pstrProxyName /* = NULL */,
	LPCTSTR pstrProxyBypass /* = NULL */,
	DWORD dwFlags /* = 0 */)
{
	ASSERT((dwFlags & INTERNET_FLAG_ASYNC) == 0);
	m_bCallbackEnabled = FALSE;
	m_pOldCallback = NULL;

	m_dwContext = dwContext;
	if (pstrAgent == NULL)
		pstrAgent = AfxGetAppName();
	m_hSession = InternetOpen(pstrAgent, dwAccessType,
		pstrProxyName, pstrProxyBypass, dwFlags);

	if (m_hSession == NULL)
		AfxThrowInternetException(m_dwContext);
	else
		_afxSessionMap.SetAt(m_hSession, this);
}

void CInternetSession::Close()
{
	if (m_bCallbackEnabled)
		EnableStatusCallback(FALSE);

	if (m_hSession != NULL)
	{
		InternetCloseHandle(m_hSession);
		_afxSessionMap.RemoveKey(m_hSession);
		m_hSession = NULL;
	}
}

CGopherConnection* CInternetSession::GetGopherConnection(LPCTSTR pstrServer,
	LPCTSTR pstrUserName /* = NULL */, LPCTSTR pstrPassword /* = NULL */,
	INTERNET_PORT nPort /* = INTERNET_INVALID_PORT_NUMBER */)
{
	ASSERT(AfxIsValidString(pstrServer));

	CGopherConnection* pResult = new CGopherConnection(this,
		pstrServer, pstrUserName, pstrPassword, m_dwContext, nPort);
	return pResult;
}

CFtpConnection* CInternetSession::GetFtpConnection(LPCTSTR pstrServer,
	LPCTSTR pstrUserName /* = NULL */, LPCTSTR pstrPassword /* = NULL */,
	INTERNET_PORT nPort /* = INTERNET_INVALID_PORT_NUMBER */,
	BOOL bPassive /* = FALSE */)
{
	ASSERT(AfxIsValidString(pstrServer));

	CFtpConnection* pResult = new CFtpConnection(this,
		pstrServer, pstrUserName, pstrPassword, m_dwContext,
		nPort, bPassive);
	return pResult;
}

CHttpConnection* CInternetSession::GetHttpConnection(LPCTSTR pstrServer,
	INTERNET_PORT nPort /* = INTERNET_INVALID_PORT_NUMBER */,
	LPCTSTR pstrUserName /* = NULL */, LPCTSTR pstrPassword /* = NULL */)
{
	ASSERT(AfxIsValidString(pstrServer));

	CHttpConnection* pResult = new CHttpConnection(this,
		pstrServer, nPort, pstrUserName, pstrPassword, m_dwContext);
	return pResult;
}

CHttpConnection* CInternetSession::GetHttpConnection(LPCTSTR pstrServer,
	DWORD dwFlags, INTERNET_PORT nPort /* = INTERNET_INVALID_PORT_NUMBER */,
	LPCTSTR pstrUserName /* = NULL */, LPCTSTR pstrPassword /* = NULL */)
{
	ASSERT(AfxIsValidString(pstrServer));
	ASSERT((dwFlags & INTERNET_FLAG_ASYNC) == 0);

	CHttpConnection* pResult = new CHttpConnection(this, pstrServer,
			dwFlags, nPort, pstrUserName, pstrPassword, m_dwContext);
	return pResult;
}

CStdioFile* CInternetSession::OpenURL(LPCTSTR pstrURL,
	DWORD_PTR dwContext /* = 0 */, DWORD dwFlags /* = INTERNET_FLAG_TRANSFER_BINARY */,
	LPCTSTR pstrHeaders /* = NULL */, DWORD dwHeadersLength /* = 0 */)
{
	ASSERT(AfxIsValidString(pstrURL));
	ASSERT(dwHeadersLength == 0 || pstrHeaders != NULL);
	ASSERT((dwFlags & INTERNET_FLAG_ASYNC) == 0);

	// must have TRANSFER_BINARY or TRANSFER_ASCII but not both
#define _AFX_TRANSFER_MASK (INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_TRANSFER_ASCII)
	ASSERT((dwFlags & _AFX_TRANSFER_MASK) != 0);
	ASSERT((dwFlags & _AFX_TRANSFER_MASK) != _AFX_TRANSFER_MASK);

	if (dwContext == 1)
		dwContext = m_dwContext;

	DWORD dwServiceType;
	CString strServer;
	CString strObject;
	INTERNET_PORT nPort;
	CStdioFile* pResult;

	BOOL bParsed = AfxParseURL(pstrURL, dwServiceType, strServer, strObject, nPort);

	// if it turns out to be a file...
	if (bParsed && dwServiceType == AFX_INET_SERVICE_FILE)
	{
		ENSURE( INTERNET_MAX_URL_LENGTH >= strObject.GetLength() );

		DWORD dwUnescapedUrlLen = INTERNET_MAX_URL_LENGTH+1;
		CString strUnescapedUrl;
		LPTSTR pstrUnescapedUrl = strUnescapedUrl.GetBuffer(INTERNET_MAX_URL_LENGTH+1);

		HRESULT hr = UrlUnescape((LPTSTR)((LPCTSTR)strObject),
								pstrUnescapedUrl,
								&dwUnescapedUrlLen,
								URL_DONT_UNESCAPE_EXTRA_INFO);

		strUnescapedUrl.ReleaseBuffer();

		if (FAILED(hr))
			AfxThrowInternetException(m_dwContext, hr);

		int nMode = CFile::modeRead | CFile::shareCompat;
		if (dwFlags & INTERNET_FLAG_TRANSFER_BINARY)
			nMode |= CFile::typeBinary;
		else
			nMode |= CFile::typeText;

		pResult = new CStdioFile(strUnescapedUrl, nMode);
	}
	else
	{
		HINTERNET hOpener;

		hOpener = InternetOpenUrl(m_hSession, pstrURL, pstrHeaders,
			dwHeadersLength, dwFlags, dwContext);

		if (hOpener == NULL)
			AfxThrowInternetException(m_dwContext);

		if (!bParsed)
			dwServiceType = AfxGetInternetHandleType(hOpener);

		switch (dwServiceType)
		{
			case INTERNET_HANDLE_TYPE_GOPHER_FILE:
			case AFX_INET_SERVICE_GOPHER:
			// WININET supplies no way to convert from a URL to a Gopher locator
				pResult = new CGopherFile(hOpener, m_hSession, _T(""),
					0, dwContext);
				_afxSessionMap.SetAt(hOpener, this);
				break;

			case INTERNET_HANDLE_TYPE_FTP_FILE:
			case AFX_INET_SERVICE_FTP:
				pResult = new CInternetFile(hOpener, m_hSession, strObject,
					strServer, dwContext, TRUE);
				_afxSessionMap.SetAt(hOpener, this);
				break;

			case INTERNET_HANDLE_TYPE_HTTP_REQUEST:
			case AFX_INET_SERVICE_HTTP:
			case AFX_INET_SERVICE_HTTPS:
				pResult = new CHttpFile(hOpener, m_hSession, strObject, strServer,
					CHttpConnection::szHtmlVerbs[CHttpConnection::HTTP_VERB_GET],
					dwContext);
				_afxSessionMap.SetAt(hOpener, this);
				break;

			default:
				TRACE(traceInternet, 0, "Error: Unidentified service type: %8.8X\n", dwServiceType);
				pResult = NULL;
		}
	}

	return pResult;
}

BOOL CInternetSession::SetOption(DWORD dwOption, LPVOID lpBuffer,
	DWORD dwBufferLength, DWORD dwFlags /* = 0 */)
{
	ASSERT(AfxIsValidAddress(lpBuffer, dwBufferLength, FALSE));
	ASSERT(dwOption >= INTERNET_FIRST_OPTION &&
		dwOption <= INTERNET_LAST_OPTION);
	ASSERT(dwBufferLength != 0);
	ASSERT((dwFlags & INTERNET_FLAG_ASYNC) == 0);

	// bogus flag?
	ASSERT(dwFlags == 0 || ((dwFlags & ISO_VALID_FLAGS) == dwFlags));

	return InternetSetOptionEx(m_hSession, dwOption,
		lpBuffer, dwBufferLength, dwFlags);
}

BOOL CInternetSession::QueryOption(DWORD dwOption, LPVOID lpBuffer,
	LPDWORD lpdwBufferLength) const
{
	ASSERT(dwOption >= INTERNET_FIRST_OPTION &&
		dwOption <= INTERNET_LAST_OPTION);
	ASSERT_POINTER(lpdwBufferLength, DWORD);
	ASSERT(AfxIsValidAddress(lpBuffer, *lpdwBufferLength));
	ASSERT(*lpdwBufferLength != 0);

	return InternetQueryOption(m_hSession, dwOption,
		lpBuffer, lpdwBufferLength);
}

BOOL CInternetSession::QueryOption(DWORD dwOption, DWORD& dwValue) const
{
	DWORD dwLen = sizeof(DWORD);
	return InternetQueryOption(m_hSession, dwOption,
		&dwValue, &dwLen);
}

void CInternetSession::OnStatusCallback(DWORD_PTR dwContext,
	DWORD dwInternetStatus, LPVOID lpvStatusInformation,
	DWORD dwStatusInformationLength)
{
	ASSERT(m_bCallbackEnabled != NULL);

	if (m_pOldCallback != NULL)
	{
		(*m_pOldCallback)(m_hSession, dwContext, dwInternetStatus,
			lpvStatusInformation, dwStatusInformationLength);
	}
}

BOOL CInternetSession::EnableStatusCallback(BOOL bEnable /* = TRUE */)
{
	ASSERT(!bEnable || m_hSession != NULL);
	if (m_hSession == NULL)
		return FALSE;

	BOOL bResult = TRUE;

	if (bEnable)
	{
		ASSERT(!m_bCallbackEnabled);
		if (!m_bCallbackEnabled)
		{
			INTERNET_STATUS_CALLBACK pRet =
				InternetSetStatusCallback(m_hSession, AfxInternetStatusCallback);

			if (pRet != INTERNET_INVALID_STATUS_CALLBACK)
			{
				m_pOldCallback = pRet;
				m_bCallbackEnabled = TRUE;
			}
			else
				AfxThrowInternetException(m_dwContext);
		}
	}
	else
	{
		ASSERT(m_bCallbackEnabled);

		if (m_bCallbackEnabled)
		{
			InternetSetStatusCallback(m_hSession, NULL);
			m_bCallbackEnabled = FALSE;
		}
	}

	return bResult;
}

BOOL CInternetSession::SetCookie(LPCTSTR pstrUrl, LPCTSTR pstrCookieName, LPCTSTR pstrCookieData)
{
	ASSERT(AfxIsValidString(pstrUrl));
	ASSERT(AfxIsValidString(pstrCookieName));
	return InternetSetCookie(pstrUrl, pstrCookieName, pstrCookieData);
}

BOOL CInternetSession::GetCookie(_In_z_ LPCTSTR pstrUrl, _In_z_ LPCTSTR pstrCookieName, _Out_z_cap_(dwBufLen) LPTSTR pstrCookieData, _In_ DWORD dwBufLen)
{
	ASSERT(AfxIsValidString(pstrUrl));
	ASSERT(AfxIsValidString(pstrCookieName));
	ASSERT(pstrCookieData != NULL);
	return InternetGetCookie(pstrUrl, pstrCookieName, pstrCookieData, &dwBufLen);
}

DWORD CInternetSession::GetCookieLength(LPCTSTR pstrUrl, LPCTSTR pstrCookieName)
{
	ASSERT(AfxIsValidString(pstrUrl));
	ASSERT(AfxIsValidString(pstrCookieName));

	DWORD dwRet;
	if (!InternetGetCookie(pstrUrl, pstrCookieName, NULL, &dwRet))
		dwRet = 0;
	return dwRet;
}

BOOL CInternetSession::GetCookie(LPCTSTR pstrUrl, LPCTSTR pstrCookieName, CString& strCookieData)
{
	ASSERT(AfxIsValidString(pstrUrl));
	ASSERT(AfxIsValidString(pstrCookieName));

	DWORD dwLen = GetCookieLength(pstrUrl, pstrCookieName);

	LPTSTR pstrTarget = strCookieData.GetBuffer(dwLen+1);
	BOOL bRetVal = InternetGetCookie(pstrUrl, pstrCookieName, pstrTarget, &dwLen);
	strCookieData.ReleaseBuffer(dwLen);

	if (!bRetVal)
		strCookieData.Empty();
	return bRetVal;
}

#ifdef _DEBUG
void CInternetSession::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
	dc << "m_hSession = " << m_hSession;
	dc << "\nm_dwContext = " << m_dwContext;
}
#endif


/////////////////////////////////////////////////////////////////////////////
// Internet Files

CInternetFile::CInternetFile(HINTERNET hFile, HINTERNET /* hSession */,
	LPCTSTR pstrFileName, LPCTSTR pstrServer, DWORD_PTR dwContext, BOOL bReadMode)
	: m_dwContext(dwContext)
{
	// caller must set _afxSessionMap()!

	ASSERT(AfxIsValidString(pstrServer));
	ASSERT(AfxIsValidString(pstrFileName));
	ASSERT(hFile != NULL);

	m_strFileName = pstrFileName;
	m_strServerName = pstrServer;

	m_hFile = hFile;
	m_bReadMode = bReadMode;

	m_pbReadBuffer = NULL;
	m_pbWriteBuffer = NULL;

	m_nReadBufferSize = 0;
	m_nReadBufferPos = 0;
	m_nWriteBufferSize = 0;
	m_nWriteBufferPos = 0;
	m_nReadBufferBytes = 0;
}

CInternetFile::CInternetFile(HINTERNET hFile,
	LPCTSTR pstrFileName, CInternetConnection* pConnection, BOOL bReadMode)
{
	ASSERT(AfxIsValidString(pstrFileName));
	ASSERT(pConnection != NULL);
	ASSERT_VALID(pConnection);
	ASSERT(hFile != NULL);

	_afxSessionMap.SetAt(hFile, pConnection->GetSession());

	m_strFileName = pstrFileName;

	m_dwContext = pConnection->GetContext();
	m_strServerName = pConnection->GetServerName();
	m_hFile = hFile;
	m_bReadMode = bReadMode;

	m_pbReadBuffer = NULL;
	m_pbWriteBuffer = NULL;

	m_nReadBufferSize = 0;
	m_nReadBufferPos = 0;
	m_nWriteBufferSize = 0;
	m_nWriteBufferPos = 0;
	m_nReadBufferBytes = 0;
}

BOOL CInternetFile::QueryOption(DWORD dwOption, LPVOID lpBuffer,
	LPDWORD lpdwBufferLength) const
{
	ASSERT(dwOption >= INTERNET_FIRST_OPTION &&
		dwOption <= INTERNET_LAST_OPTION);
	ASSERT_POINTER(lpdwBufferLength, DWORD);
	ASSERT(AfxIsValidAddress(lpBuffer, *lpdwBufferLength));
	ASSERT(*lpdwBufferLength != 0);
	ASSERT(m_hFile != NULL);

	return InternetQueryOption(m_hFile, dwOption,
		lpBuffer, lpdwBufferLength);
}

BOOL CInternetFile::QueryOption(DWORD dwOption, DWORD& dwValue) const
{
	ASSERT(m_hFile != NULL);

	DWORD dwLen = sizeof(DWORD);
	return InternetQueryOption(m_hFile, dwOption,
		&dwValue, &dwLen);
}

BOOL CInternetFile::SetOption(DWORD dwOption, LPVOID lpBuffer,
	DWORD dwBufferLength, DWORD dwFlags /* = 0 */)
{
	ASSERT(dwOption >= INTERNET_FIRST_OPTION &&
		dwOption <= INTERNET_LAST_OPTION);
	ASSERT(AfxIsValidAddress(lpBuffer, dwBufferLength, FALSE));
	ASSERT(dwBufferLength != 0);
	ASSERT((dwFlags & INTERNET_FLAG_ASYNC) == 0);

	// bogus flag?
	ASSERT(dwFlags == 0 || ((dwFlags & ISO_VALID_FLAGS) == dwFlags));

	return InternetSetOptionEx(m_hFile, dwOption,
		lpBuffer, dwBufferLength, dwFlags);
}

BOOL CInternetFile::SetReadBufferSize(UINT nReadSize)
{
	ASSERT_VALID(this);
	BOOL bRet = TRUE;

	if (nReadSize != -1 && nReadSize != m_nReadBufferSize)
	{
		if (m_nReadBufferPos > nReadSize)
			bRet = FALSE;
		else
		{
			if (nReadSize == 0)
			{
				delete [] m_pbReadBuffer;
				m_pbReadBuffer = NULL;
			}
			else if (m_pbReadBuffer == NULL)
			{
				m_pbReadBuffer = new BYTE[nReadSize];
				m_nReadBufferPos = nReadSize;
			}
			else
			{
				DWORD dwMoved = m_nReadBufferSize - m_nReadBufferPos;
				LPBYTE pbTemp = m_pbReadBuffer;
				m_pbReadBuffer = new BYTE[nReadSize];

				if (dwMoved > 0 && dwMoved <= nReadSize)
				{
					Checked::memcpy_s(m_pbReadBuffer, nReadSize, 
						pbTemp + m_nReadBufferPos, dwMoved);

					m_nReadBufferPos = 0;
					m_nReadBufferBytes = dwMoved;
				}
				else
				{
					m_nReadBufferBytes = 0;
					m_nReadBufferPos = nReadSize;
				}
				delete [] pbTemp;
			}

			m_nReadBufferSize = nReadSize;
		}
	}

	return bRet;
}

BOOL CInternetFile::SetWriteBufferSize(UINT nWriteSize)
{
	ASSERT_VALID(this);
	BOOL bRet = TRUE;

	if (nWriteSize != m_nWriteBufferSize)
	{
		if (m_nWriteBufferPos > nWriteSize)
			Flush();

		if (nWriteSize == 0)
		{
			delete [] m_pbWriteBuffer;
			m_pbWriteBuffer = NULL;
		}
		else if (m_pbWriteBuffer == NULL)
		{
			m_pbWriteBuffer = new BYTE[nWriteSize];
			m_nWriteBufferPos = 0;
		}
		else
		{
			LPBYTE pbTemp = m_pbWriteBuffer;
			m_pbWriteBuffer = new BYTE[nWriteSize];
			if (m_nWriteBufferPos <= nWriteSize)
			{
				Checked::memcpy_s(m_pbWriteBuffer, nWriteSize, 
					pbTemp, m_nWriteBufferPos);
			}
			delete [] pbTemp;
		}

		m_nWriteBufferSize = nWriteSize;
	}

	return bRet;
}

ULONGLONG CInternetFile::Seek(LONGLONG lOffset, UINT nFrom)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);
	ASSERT(m_bReadMode);
	ASSERT(m_pbReadBuffer == NULL);

	// can't do this on a file for writing
	// can't do this on a file that's got a buffer

	if (!m_bReadMode || m_pbReadBuffer != NULL)
		AfxThrowInternetException(m_dwContext, ERROR_INVALID_HANDLE);

	switch (nFrom)
	{
		case begin:
			nFrom = FILE_BEGIN;
			break;

		case current:
			nFrom = FILE_CURRENT;
			break;

		case end:
			nFrom = FILE_END;
			break;

		default:
			ASSERT(FALSE);  // got a bogus nFrom value
			AfxThrowInternetException(m_dwContext, ERROR_INVALID_PARAMETER);
			break;
	}

   if ((lOffset < LONG_MIN) || (lOffset > LONG_MAX))
   {
	  AfxThrowInternetException(m_dwContext, ERROR_INVALID_PARAMETER);
   }

	LONG lRet;
	lRet = InternetSetFilePointer(m_hFile, LONG(lOffset), NULL, nFrom, 
	  m_dwContext);
	if (lRet == -1)
		AfxThrowInternetException(m_dwContext);

	return lRet;
}

CInternetFile::~CInternetFile()
{
	if (m_hFile != NULL)
	{
#ifdef _DEBUG
		const CString strName(GetRuntimeClass()->m_lpszClassName);		
		TRACE(traceInternet, 0, _T("Warning: destroying an open %s with handle %8.8X\n"),
			strName.GetString(), m_hFile);
#endif
		Close();
	}

	if (m_pbReadBuffer != NULL)
		delete m_pbReadBuffer;

	if (m_pbWriteBuffer != NULL)
		delete m_pbWriteBuffer;
}

void CInternetFile::Abort()
{
	ASSERT_VALID(this);
	if (m_hFile != NULL)
		Close();
	m_strFileName.Empty();
}

void CInternetFile::Flush()
{
	if (m_pbWriteBuffer != NULL && m_nWriteBufferPos > 0)
	{
		DWORD dwBytes;

		if (!InternetWriteFile(m_hFile, m_pbWriteBuffer,
				m_nWriteBufferPos, &dwBytes))
			AfxThrowInternetException(m_dwContext);

		if (dwBytes != m_nWriteBufferPos)
			AfxThrowInternetException(m_dwContext);

		m_nWriteBufferPos = 0;
	}
}

void CInternetFile::Close()
{
	if (m_hFile != NULL)
	{
		Flush();
		InternetCloseHandle(m_hFile);
		_afxSessionMap.RemoveKey(m_hFile);
		m_hFile = NULL;

		if (m_pbWriteBuffer != NULL)
		{
			delete [] m_pbWriteBuffer;
			m_pbWriteBuffer = NULL;
		}

		if (m_pbReadBuffer != NULL)
		{
			delete [] m_pbReadBuffer;
			m_pbReadBuffer = NULL;
		}
	}
}

UINT CInternetFile::Read(LPVOID lpBuf, UINT nCount)
{
	ASSERT_VALID(this);
	ASSERT(AfxIsValidAddress(lpBuf, nCount));
	ASSERT(m_hFile != NULL);
	ASSERT(m_bReadMode);

	DWORD dwBytes;

	if (!m_bReadMode || m_hFile == NULL)
		AfxThrowInternetException(m_dwContext, ERROR_INVALID_HANDLE);

	if (m_pbReadBuffer == NULL)
	{
		if (!InternetReadFile(m_hFile, (LPVOID) lpBuf, nCount, &dwBytes))
				AfxThrowInternetException(m_dwContext);
		return dwBytes;
	}

	LPBYTE lpbBuf = (LPBYTE) lpBuf;

	// if the requested size is bigger than our buffer,
	// then handle it directly

	if (nCount >= m_nReadBufferSize)
	{
		DWORD dwMoved = max(0, (long)m_nReadBufferBytes - (long)m_nReadBufferPos);
		if (dwMoved <= nCount)
		{
			Checked::memcpy_s(lpBuf, nCount, 
				m_pbReadBuffer + m_nReadBufferPos, dwMoved);
		}
		else
		{
			return 0; // output buffer not big enough.
		}

		m_nReadBufferPos = m_nReadBufferSize;
		if (!InternetReadFile(m_hFile, lpbBuf+dwMoved, nCount-dwMoved, &dwBytes))
				AfxThrowInternetException(m_dwContext);
		dwBytes += dwMoved;
	}
	else
	{
		if (m_nReadBufferPos + nCount >= m_nReadBufferBytes)
		{
			DWORD dwMoved = max(0, (long)m_nReadBufferBytes - (long)m_nReadBufferPos);
			if (dwMoved <= nCount)
			{
				Checked::memcpy_s(lpbBuf, nCount, 
					m_pbReadBuffer + m_nReadBufferPos, dwMoved);
			}
			else
			{
				return 0;
			}

			DWORD dwRead;
			if (!InternetReadFile(m_hFile, m_pbReadBuffer, m_nReadBufferSize,
					&dwRead))
				AfxThrowInternetException(m_dwContext);
			m_nReadBufferBytes = dwRead;

			dwRead = min(nCount - dwMoved, m_nReadBufferBytes);
			Checked::memcpy_s(lpbBuf + dwMoved, nCount - dwMoved, m_pbReadBuffer, dwRead);
			m_nReadBufferPos = dwRead;
			dwBytes = dwMoved + dwRead;
		}
		else
		{
			Checked::memcpy_s(lpbBuf, nCount, m_pbReadBuffer + m_nReadBufferPos, nCount);
			m_nReadBufferPos += nCount;
			dwBytes = nCount;
		}
	}

	return dwBytes;
}

void CInternetFile::Write(const void* lpBuf, UINT nCount)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);
	ASSERT(AfxIsValidAddress(lpBuf, nCount, FALSE));
	ASSERT(m_bReadMode == FALSE || m_bReadMode == -1);

	if (m_bReadMode == TRUE || m_hFile == NULL)
		AfxThrowInternetException(m_dwContext, ERROR_INVALID_HANDLE);

	DWORD dwBytes;
	if (m_pbWriteBuffer == NULL)
	{
		if (!InternetWriteFile(m_hFile, lpBuf, nCount, &dwBytes))
			AfxThrowInternetException(m_dwContext);

		if (dwBytes != nCount)
			AfxThrowInternetException(m_dwContext);
	}
	else
	{
		if ((m_nWriteBufferPos + nCount) >= m_nWriteBufferSize)
		{
			// write what is in the buffer just now

			if (!InternetWriteFile(m_hFile, m_pbWriteBuffer,
					m_nWriteBufferPos, &dwBytes))
				AfxThrowInternetException(m_dwContext);

			// reset the buffer position since it is now clean

			m_nWriteBufferPos = 0;
		}

		// if we can't hope to buffer the write request,
		// do it immediately ... otherwise, buffer it!

		if (nCount >= m_nWriteBufferSize)
		{
			if (!InternetWriteFile(m_hFile, (LPVOID) lpBuf, nCount, &dwBytes))
				AfxThrowInternetException(m_dwContext);
		}
		else
		{
			if (m_nWriteBufferPos + nCount <= m_nWriteBufferSize)
			{
				Checked::memcpy_s(m_pbWriteBuffer + m_nWriteBufferPos, 
					m_nWriteBufferSize - m_nWriteBufferPos, lpBuf, nCount);
				m_nWriteBufferPos += nCount;
			}
		}
	}
}

void CInternetFile::WriteString(LPCTSTR pstr)
{
	ASSERT(m_bReadMode == FALSE || m_bReadMode == -1);
	ASSERT(AfxIsValidString(pstr));
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	if (m_bReadMode == TRUE)
		AfxThrowInternetException(m_dwContext, ERROR_INVALID_HANDLE);

	Write(pstr, lstrlen(pstr) * sizeof(TCHAR));
}

LPTSTR CInternetFile::ReadString(_Out_z_cap_(nMax) LPTSTR pstr, _In_ UINT nMax)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);
	ASSERT(AfxIsValidAddress(pstr, nMax*sizeof(TCHAR)));
	DWORD dwRead;

	// if we're reading line-by-line, we must have a buffer

	if (m_pbReadBuffer == NULL)
	{
		if (!SetReadBufferSize(4096))   // arbitrary but reasonable
			return NULL;
		if (!InternetReadFile(m_hFile, m_pbReadBuffer, m_nReadBufferSize,
				&dwRead))
			AfxThrowInternetException(m_dwContext);
		m_nReadBufferBytes = dwRead;
		m_nReadBufferPos = 0;
	}

	LPSTR pstrChar = (LPSTR) (m_pbReadBuffer + m_nReadBufferPos);
	LPSTR pstrTarget = (LPSTR)pstr;

	UINT nMaxChars = (nMax-1)*sizeof(TCHAR);
	while (nMaxChars)
	{
		if (m_nReadBufferPos >= m_nReadBufferBytes)
		{
			if (!InternetReadFile(m_hFile, m_pbReadBuffer, m_nReadBufferSize,
					&dwRead))
				AfxThrowInternetException(m_dwContext);
			m_nReadBufferBytes = dwRead;
			if (m_nReadBufferBytes == 0)
			{
				memset(pstrTarget, 0, (nMaxChars & (sizeof(TCHAR)-1)) + sizeof(TCHAR));
				if (pstrTarget == (LPCSTR)pstr)
					return NULL;
				else
					return pstr;
			}
			else
			{
				m_nReadBufferPos = 0;
				pstrChar = (LPSTR) m_pbReadBuffer;
			}
		}

		if (*pstrChar != '\r')
		{
			*pstrTarget++ = *pstrChar;
			nMaxChars--;
		}

		m_nReadBufferPos++;
		if (*pstrChar++ == '\n')
			break;
	}

	memset(pstrTarget, 0, (nMaxChars & (sizeof(TCHAR)-1)) + sizeof(TCHAR));
	return pstr;
}

BOOL CInternetFile::ReadString(CString& rString)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	rString = _T("");    // empty string without deallocating
	const int nMaxSize = 128;

	LPTSTR pstrPlace = rString.GetBuffer(nMaxSize);
	LPTSTR pstrResult;

	int nLen;

	do
	{
		pstrResult = ReadString(pstrPlace, nMaxSize);
		rString.ReleaseBuffer();

		// if string is read completely or EOF
		if (pstrResult == NULL ||
			(nLen = lstrlen(pstrPlace)) < (nMaxSize-1) ||
			pstrPlace[nLen-1] == '\n')
			break;

		nLen = rString.GetLength();
		pstrPlace = rString.GetBuffer(nMaxSize + nLen) + nLen;
	} while (1);

	// remove '\n' from end of string if present
	pstrPlace = rString.GetBuffer(0);
	nLen = rString.GetLength();
	if (nLen != 0 && pstrPlace[nLen-1] == '\n')
		pstrPlace[nLen-1] = '\0';
	rString.ReleaseBuffer();

	return ( (pstrResult != NULL) || (nLen != 0 ) );
}

ULONGLONG CInternetFile::GetLength() const
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	DWORD dwRet = 0;

	if (m_hFile != NULL)
	{
		if (!InternetQueryDataAvailable(m_hFile, &dwRet, 0, 0))
			dwRet = 0;
	}

	return dwRet;
}

void CInternetFile::LockRange(ULONGLONG /* dwPos */, ULONGLONG /* dwCount */)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	AfxThrowNotSupportedException();
}

void CInternetFile::UnlockRange(ULONGLONG /* dwPos */, ULONGLONG /* dwCount */)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	AfxThrowNotSupportedException();
}

void CInternetFile::SetLength(ULONGLONG)
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	AfxThrowNotSupportedException();
}

CFile* CInternetFile::Duplicate() const
{
	ASSERT_VALID(this);
	ASSERT(m_pStream != NULL);

	AfxThrowNotSupportedException();
}

#ifdef _DEBUG
void CInternetFile::AssertValid() const
{
	// Don't call CStdioFile's AsssertValid()
	CFile::AssertValid();

	ASSERT(m_hConnection != NULL);

	// make sure we really have a decent handle
	if (m_hFile != NULL)
	{
		DWORD dwResult = AfxGetInternetHandleType(m_hFile);

		if (IsKindOf(RUNTIME_CLASS(CHttpFile)))
		{
			ASSERT(dwResult == INTERNET_HANDLE_TYPE_HTTP_REQUEST);
		}
		else if (IsKindOf(RUNTIME_CLASS(CGopherFile)))
		{
			ASSERT(dwResult == INTERNET_HANDLE_TYPE_GOPHER_FILE ||
				dwResult == INTERNET_HANDLE_TYPE_GOPHER_FIND_HTML ||
				dwResult == INTERNET_HANDLE_TYPE_GOPHER_FILE_HTML ||
				dwResult == INTERNET_HANDLE_TYPE_HTTP_REQUEST);
		}
		else if (IsKindOf(RUNTIME_CLASS(CInternetFile)))
		{
			ASSERT(dwResult == INTERNET_HANDLE_TYPE_FTP_FILE ||
				dwResult == INTERNET_HANDLE_TYPE_FTP_FILE_HTML ||
				dwResult == INTERNET_HANDLE_TYPE_FTP_FIND_HTML ||
				dwResult == INTERNET_HANDLE_TYPE_HTTP_REQUEST);
		}
		else
			ASSERT(FALSE);  // some bogus object!
	}
}

void CInternetFile::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	dc << "\na " << GetRuntimeClass()->m_lpszClassName;
	dc << " with handle " << m_hFile;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CInternetConnection

CInternetConnection::CInternetConnection(CInternetSession* pSession,
	LPCTSTR pstrServerName,
	INTERNET_PORT nPort /* = INTERNET_INVALID_PORT_NUMBER */,
	DWORD_PTR dwContext /* = 1 */)
	: m_strServerName(pstrServerName)
{
	ASSERT(pSession != NULL);
	ASSERT_VALID(pSession);
	ASSERT(pstrServerName != NULL);

	m_nPort = nPort;
	m_pSession = pSession;
	m_hConnection = NULL;
	if (dwContext == 1)
		dwContext = pSession->GetContext();
	m_dwContext = dwContext;
}

CInternetConnection::~CInternetConnection()
{
	if (m_hConnection != NULL)
	{
#ifdef _DEBUG
		const CString strName(GetRuntimeClass()->m_lpszClassName);
		TRACE(traceInternet, 0, _T("Warning: Disconnecting %s handle %8.8X in context %8.8X at destruction.\n"),
			strName.GetString(), m_hConnection, m_dwContext);
#endif
		Close();
	}
}

BOOL CInternetConnection::SetOption(DWORD dwOption, LPVOID lpBuffer,
	DWORD dwBufferLength, DWORD dwFlags /* = 0 */)
{
	ASSERT(dwOption >= INTERNET_FIRST_OPTION &&
		dwOption <= INTERNET_LAST_OPTION);
	ASSERT(AfxIsValidAddress(lpBuffer, dwBufferLength, FALSE));
	ASSERT(dwBufferLength != 0);
	ASSERT((dwFlags & INTERNET_FLAG_ASYNC) == 0);

	// bogus flag?
	ASSERT(dwFlags == 0 || ((dwFlags & ISO_VALID_FLAGS) == dwFlags));

	return InternetSetOptionEx(m_hConnection, dwOption,
		lpBuffer, dwBufferLength, dwFlags);
}

BOOL CInternetConnection::QueryOption(DWORD dwOption, LPVOID lpBuffer,
	LPDWORD lpdwBufferLength) const
{
	ASSERT(dwOption >= INTERNET_FIRST_OPTION &&
		dwOption <= INTERNET_LAST_OPTION);
	ASSERT_POINTER(lpdwBufferLength, DWORD);
	ASSERT(AfxIsValidAddress(lpBuffer, *lpdwBufferLength));
	ASSERT(*lpdwBufferLength != 0);

	return InternetQueryOption(m_hConnection, dwOption,
		lpBuffer, lpdwBufferLength);
}

BOOL CInternetConnection::QueryOption(DWORD dwOption, DWORD& dwValue) const
{
	DWORD dwLen = sizeof(DWORD);
	return InternetQueryOption(m_hConnection, dwOption,
		&dwValue, &dwLen);
}

void CInternetConnection::Close()
{
	if (m_hConnection != NULL)
	{
		InternetCloseHandle(m_hConnection);
		_afxSessionMap.RemoveKey(m_hConnection);
		m_hConnection = NULL;
	}
}

#ifdef _DEBUG
void CInternetConnection::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
	dc << "m_hConnection = " << m_hConnection;
}

void CInternetConnection::AssertValid() const
{
	CObject::AssertValid();
}
#endif


/////////////////////////////////////////////////////////////////////////////
// CFtpConnection

CFtpConnection::~CFtpConnection()
{
}

CFtpConnection::CFtpConnection(CInternetSession* pSession,
	HINTERNET hConnected, LPCTSTR pstrServer, DWORD_PTR dwContext)
	: CInternetConnection(pSession, pstrServer, INTERNET_INVALID_PORT_NUMBER,
	dwContext)
{
	ASSERT(pSession != NULL);
	ASSERT(AfxIsValidString(pstrServer));

	BOOL bBadType = FALSE;
	if (AfxGetInternetHandleType(hConnected) != INTERNET_HANDLE_TYPE_CONNECT_FTP)
	{
		ASSERT(FALSE);      // used the wrong handle type
		bBadType = TRUE;
	}

	m_strServerName = pstrServer;
	m_hConnection = hConnected;
	if (m_hConnection == NULL || bBadType)
		AfxThrowInternetException(m_dwContext, ERROR_INVALID_HANDLE);
	else
		_afxSessionMap.SetAt(m_hConnection, m_pSession);
}

CFtpConnection::CFtpConnection(CInternetSession* pSession,
	LPCTSTR pstrServer, LPCTSTR pstrUserName /* = NULL */,
	LPCTSTR pstrPassword /* = NULL */, DWORD_PTR dwContext /* = 0 */,
	INTERNET_PORT nPort /* = INTERNET_INVALID_PORT_NUMBER */,
	BOOL bPassive /* = FALSE */)
	: CInternetConnection(pSession, pstrServer, nPort, dwContext)
{
	ASSERT(pSession != NULL);
	ASSERT_KINDOF(CInternetSession, pSession);
	ASSERT(AfxIsValidString(pstrServer));

	m_strServerName = pstrServer;

	m_hConnection = InternetConnect((HINTERNET) *pSession, pstrServer,
		nPort, pstrUserName, pstrPassword, INTERNET_SERVICE_FTP,
		(bPassive ? INTERNET_FLAG_PASSIVE : 0), m_dwContext);

	if (m_hConnection == NULL)
		AfxThrowInternetException(m_dwContext, ::GetLastError());
	else
		_afxSessionMap.SetAt(m_hConnection, m_pSession);
}

BOOL CFtpConnection::Remove(LPCTSTR pstrFileName)
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);
	ASSERT(AfxIsValidString(pstrFileName));

	return FtpDeleteFile(m_hConnection, pstrFileName);
}

BOOL CFtpConnection::Rename(LPCTSTR pstrExisting, LPCTSTR pstrNew)
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);
	ASSERT(AfxIsValidString(pstrExisting));
	ASSERT(AfxIsValidString(pstrNew));

	return FtpRenameFile(m_hConnection, pstrExisting, pstrNew);
}

BOOL CFtpConnection::CreateDirectory(LPCTSTR pstrDirName)
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);
	ASSERT(AfxIsValidString(pstrDirName));

	return FtpCreateDirectory(m_hConnection, pstrDirName);
}

BOOL CFtpConnection::RemoveDirectory(LPCTSTR pstrDirName)
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);
	ASSERT(AfxIsValidString(pstrDirName));

	return FtpRemoveDirectory(m_hConnection, pstrDirName);
}

BOOL CFtpConnection::SetCurrentDirectory(LPCTSTR pstrDirName)
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);
	ASSERT(AfxIsValidString(pstrDirName));

	return FtpSetCurrentDirectory(m_hConnection, pstrDirName);
}

BOOL CFtpConnection::GetCurrentDirectory(_Out_z_cap_post_count_(*lpdwLen, *lpdwLen) LPTSTR pstrDirName, 
	_Inout_ LPDWORD lpdwLen) const
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);
	ASSERT(AfxIsValidAddress(pstrDirName, *lpdwLen));
	ASSERT(lpdwLen != 0);

	return FtpGetCurrentDirectory(m_hConnection, pstrDirName, lpdwLen);
}

BOOL CFtpConnection::GetCurrentDirectoryAsURL(CString& strDirName) const
{
	CString strDirectory;
	if (!GetCurrentDirectory(strDirectory))
		return FALSE;

	strDirName = _afxURLftp;
	strDirName += GetServerName();

	if (strDirectory[0] != '/')
		strDirName += '/';

	strDirName += strDirectory;
	return TRUE;
}

BOOL CFtpConnection::GetCurrentDirectoryAsURL(_Out_z_cap_post_count_(*lpdwLen, *lpdwLen) LPTSTR pstrName, 
	_Inout_ LPDWORD lpdwLen) const
{
	ASSERT(lpdwLen != NULL);
	ASSERT_POINTER(lpdwLen, DWORD);
	ASSERT(AfxIsValidAddress(pstrName, *lpdwLen));
	ASSERT(*lpdwLen != 0);
	if (pstrName == NULL  || lpdwLen == NULL)
		return FALSE;

	CString strTemp;

	if (lpdwLen == NULL || !GetCurrentDirectoryAsURL(strTemp))
		return FALSE;

	if (pstrName == NULL)
		*lpdwLen = (DWORD)strTemp.GetLength();
	else if (*lpdwLen > 0)
		Checked::tcsncpy_s(pstrName, *lpdwLen, (LPCTSTR) strTemp, _TRUNCATE);

	return TRUE;
}

BOOL CFtpConnection::GetCurrentDirectory(CString& strDirName) const
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);

	DWORD dwLen = INTERNET_MAX_PATH_LENGTH;
	LPTSTR pstrTarget = strDirName.GetBufferSetLength(dwLen);
	BOOL bRet = FtpGetCurrentDirectory(m_hConnection, pstrTarget, &dwLen);

	if (bRet)
		strDirName.ReleaseBuffer(dwLen);
	else
		strDirName.ReleaseBuffer(0);

	return bRet;
}

CInternetFile* CFtpConnection::OpenFile(LPCTSTR pstrFileName,
	DWORD dwAccess /* = GENERIC_READ */,
	DWORD dwFlags /* = FTP_TRANSFER_TYPE_BINARY */,
	DWORD_PTR dwContext /* = 1 */)
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);
	ASSERT(dwAccess != (GENERIC_READ | GENERIC_WRITE));
	ASSERT(dwAccess == GENERIC_READ || dwAccess == GENERIC_WRITE);
	ASSERT(AfxIsValidString(pstrFileName));

	HINTERNET hFile;
	if (dwContext == 1)
		dwContext = m_dwContext;

	hFile = FtpOpenFile(m_hConnection, pstrFileName, dwAccess,
		dwFlags, dwContext);
	if (hFile == NULL)
		AfxThrowInternetException(dwContext);

	CInternetFile* pFile = new CInternetFile(hFile, pstrFileName, this,
		(dwAccess == GENERIC_READ));
	return pFile;
}

CInternetFile* CFtpConnection::Command(LPCTSTR pszCommand, CmdResponseType eResponse,
	DWORD dwFlags, DWORD_PTR dwContext)
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);
	ASSERT(AfxIsValidString(pszCommand));

	HINTERNET hFile = NULL;
	if (dwContext == 1)
		dwContext = m_dwContext;

	BOOL bSuccess;
	bSuccess = ::FtpCommand(m_hConnection, eResponse != CmdRespNone, dwFlags, pszCommand,
		dwContext, &hFile);
	if (!bSuccess)
		AfxThrowInternetException(dwContext);

	CInternetFile* pFile = NULL;
	if (hFile != NULL)
		pFile = new CInternetFile(hFile, pszCommand, this, eResponse == CmdRespRead);

	return pFile;
}

BOOL CFtpConnection::PutFile(LPCTSTR pstrLocalFile, LPCTSTR pstrRemoteFile,
	DWORD dwFlags /* = FTP_TRANSFER_TYPE_BINARY */,
	DWORD_PTR dwContext /* = 1 */)
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);
	ASSERT(AfxIsValidString(pstrRemoteFile));
	ASSERT(AfxIsValidString(pstrLocalFile));
	ASSERT((dwFlags & INTERNET_FLAG_ASYNC) == 0);

	if (dwContext == 1)
		dwContext = m_dwContext;

	return FtpPutFile(m_hConnection, pstrLocalFile, pstrRemoteFile,
		dwFlags, dwContext);
}

BOOL CFtpConnection::GetFile(LPCTSTR pstrRemoteFile, LPCTSTR pstrLocalFile,
	BOOL bFailIfExists /* = TRUE */,
	DWORD dwAttributes /* = FILE_ATTRIBUTE_NORMAL */,
	DWORD dwFlags /* = FTP_TRANSFER_TYPE_BINARY */, DWORD_PTR dwContext /* = 1 */)
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);
	ASSERT(AfxIsValidString(pstrRemoteFile));
	ASSERT(AfxIsValidString(pstrLocalFile));
	ASSERT(!(dwAttributes & FILE_ATTRIBUTE_DIRECTORY));
	ASSERT((dwFlags & INTERNET_FLAG_ASYNC) == 0);

	if (dwContext == 1)
		dwContext = m_dwContext;

	return FtpGetFile(m_hConnection, pstrRemoteFile, pstrLocalFile,
		bFailIfExists, dwAttributes, dwFlags, dwContext);
}

#ifdef _DEBUG
void CFtpConnection::Dump(CDumpContext& dc) const
{
	CInternetConnection::Dump(dc);
	dc << "\nm_strServerName = " << m_strServerName;
}

void CFtpConnection::AssertValid() const
{
	ASSERT(m_pSession != NULL);
	if (m_hConnection != NULL)
	{
		ASSERT(AfxGetInternetHandleType(m_hConnection)
				== INTERNET_HANDLE_TYPE_CONNECT_FTP);
	}
}
#endif


/////////////////////////////////////////////////////////////////////////////
// CGopherConnection

CGopherConnection::~CGopherConnection()
{
}

CGopherConnection::CGopherConnection(CInternetSession* pSession,
	LPCTSTR pstrServer, LPCTSTR pstrUserName /* = NULL */,
	LPCTSTR pstrPassword /* = NULL */, DWORD_PTR dwContext /* = 0 */,
	INTERNET_PORT nPort /* = INTERNET_INVALID_PORT_NUMBER */)
	: CInternetConnection(pSession, pstrServer, nPort, dwContext)
{
	ASSERT(pSession != NULL);
	ASSERT_KINDOF(CInternetSession, pSession);
	ASSERT(AfxIsValidString(pstrServer));

	m_hConnection = InternetConnect((HINTERNET) *pSession, pstrServer,
		nPort, pstrUserName, pstrPassword, INTERNET_SERVICE_GOPHER,
		0, m_dwContext);

	if (m_hConnection == NULL)
		AfxThrowInternetException(m_dwContext);
	else
		_afxSessionMap.SetAt(m_hConnection, m_pSession);
}

CGopherConnection::CGopherConnection(CInternetSession* pSession,
	HINTERNET hConnected, LPCTSTR pstrServer, DWORD_PTR dwContext)
	: CInternetConnection(pSession, pstrServer,
		INTERNET_INVALID_PORT_NUMBER, dwContext)
{
	ASSERT(pSession != NULL);
	ASSERT(AfxIsValidString(pstrServer));

	BOOL bBadType = FALSE;
	if (AfxGetInternetHandleType(hConnected) != INTERNET_HANDLE_TYPE_CONNECT_GOPHER)
	{
		ASSERT(FALSE);      // used the wrong handle type
		bBadType = TRUE;
	}

	m_hConnection = hConnected;
	if (m_hConnection == NULL || bBadType)
		AfxThrowInternetException(m_dwContext);
	else
		_afxSessionMap.SetAt(m_hConnection, m_pSession);
}

CGopherLocator CGopherConnection::CreateLocator(LPCTSTR pstrLocator)
{
	CGopherLocator ret(pstrLocator, lstrlen(pstrLocator));
	return ret;
}

CGopherLocator CGopherConnection::CreateLocator(LPCTSTR pstrServerName,
	LPCTSTR pstrDisplayString, LPCTSTR pstrSelectorString, DWORD dwGopherType,
	INTERNET_PORT nPort /* = INTERNET_INVALID_PORT_NUMBER */)
{
	TCHAR szLocator[MAX_GOPHER_LOCATOR_LENGTH];
	DWORD dwLocLen = MAX_GOPHER_LOCATOR_LENGTH;
	ASSERT(AfxIsValidString(pstrDisplayString));
	ASSERT(AfxIsValidString(pstrServerName));
	ASSERT(AfxIsValidString(pstrSelectorString));

	if (!GopherCreateLocator(pstrServerName, nPort,
			pstrDisplayString, pstrSelectorString, dwGopherType,
			szLocator, &dwLocLen))
		AfxThrowInternetException(0);

	CGopherLocator ret(szLocator, dwLocLen);
	return ret;
}

CGopherLocator CGopherConnection::CreateLocator(
	LPCTSTR pstrDisplayString, LPCTSTR pstrSelectorString, DWORD dwGopherType)
{
	TCHAR szLocator[MAX_GOPHER_LOCATOR_LENGTH];
	DWORD dwLocLen = MAX_GOPHER_LOCATOR_LENGTH;
	ASSERT(AfxIsValidString(pstrDisplayString));
	ASSERT(AfxIsValidString(pstrSelectorString));

	if (!GopherCreateLocator(m_strServerName, m_nPort,
			pstrDisplayString, pstrSelectorString, dwGopherType,
			szLocator, &dwLocLen))
		AfxThrowInternetException(m_dwContext);

	CGopherLocator ret(szLocator, dwLocLen);
	return ret;
}


BOOL CGopherConnection::GetAttribute(CGopherLocator& refLocator,
	CString strRequestedAttributes, CString& strResult)
{
	DWORD dwLen = 4*MIN_GOPHER_ATTRIBUTE_LENGTH; // more than the minimum
	BOOL bRet;
	LPTSTR pstrResult = strResult.GetBuffer(dwLen);

	if (!GopherGetAttribute(m_hConnection, (LPCTSTR) refLocator,
			pstrResult, NULL, dwLen, &dwLen,
			NULL, m_dwContext)) 
	{
		bRet = FALSE;
		strResult.ReleaseBuffer(0);
	}
	else
	{
		bRet = TRUE;
		strResult.ReleaseBuffer(dwLen);
	}

	return bRet;
}

CGopherFile* CGopherConnection::OpenFile(CGopherLocator& refLocator,
	DWORD dwFlags /* = 0 */, LPCTSTR pstrView /* = NULL */,
	DWORD_PTR dwContext /* = 1 */)
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);

	HINTERNET hFile;
	if (dwContext == 1)
		dwContext = m_dwContext;

	hFile = GopherOpenFile(m_hConnection, (LPCTSTR) refLocator, pstrView,
		dwFlags, dwContext);

	if (hFile == NULL)
		AfxThrowInternetException(dwContext);

	CGopherFile* pFile = new CGopherFile(hFile, refLocator, this);
	return pFile;
}

#ifdef _DEBUG
void CGopherConnection::Dump(CDumpContext& dc) const
{
	CInternetConnection::Dump(dc);
	dc << "\nm_strServerName = " << m_strServerName;
}

void CGopherConnection::AssertValid() const
{
	ASSERT(m_pSession != NULL);
	if (m_hConnection != NULL)
	{
		ASSERT(AfxGetInternetHandleType(m_hConnection)
				== INTERNET_HANDLE_TYPE_CONNECT_GOPHER);
	}
}
#endif


/////////////////////////////////////////////////////////////////////////////
// CHttpConnection

CHttpConnection::~CHttpConnection()
{
}

CHttpConnection::CHttpConnection(CInternetSession* pSession,
	HINTERNET hConnected, LPCTSTR pstrServer, DWORD_PTR dwContext /* = 0 */)
	: CInternetConnection(pSession, pstrServer, INTERNET_INVALID_PORT_NUMBER, dwContext)
{
	ASSERT(pSession != NULL);
	ASSERT(AfxIsValidString(pstrServer));

	BOOL bBadType = FALSE;
	if (AfxGetInternetHandleType(hConnected) != INTERNET_HANDLE_TYPE_CONNECT_HTTP)
	{
		ASSERT(FALSE);      // used the wrong handle type
		bBadType = TRUE;
	}

	m_hConnection = hConnected;
	if (m_hConnection == NULL || bBadType)
		AfxThrowInternetException(m_dwContext, ERROR_INVALID_HANDLE);
	else
		_afxSessionMap.SetAt(m_hConnection, m_pSession);
}

CHttpConnection::CHttpConnection(CInternetSession* pSession,
	LPCTSTR pstrServer,
	INTERNET_PORT nPort /* = INTERNET_INVALID_PORT_NUMBER */,
	LPCTSTR pstrUserName /* = NULL */,
	LPCTSTR pstrPassword /* = NULL */, DWORD_PTR dwContext /* = 1 */)
	: CInternetConnection(pSession, pstrServer, nPort, dwContext)
{
	ASSERT(pSession != NULL);
	ASSERT_KINDOF(CInternetSession, pSession);
	ASSERT(AfxIsValidString(pstrServer));

	m_hConnection = InternetConnect((HINTERNET) *pSession, pstrServer,
		nPort, pstrUserName, pstrPassword, INTERNET_SERVICE_HTTP,
		0, m_dwContext);

	if (m_hConnection == NULL)
		AfxThrowInternetException(m_dwContext);
	else
		_afxSessionMap.SetAt(m_hConnection, m_pSession);
}

CHttpConnection::CHttpConnection(CInternetSession* pSession,
	LPCTSTR pstrServer, DWORD dwFlags,
	INTERNET_PORT nPort /* = INTERNET_INVALID_PORT_NUMBER */,
	LPCTSTR pstrUserName /* = NULL */,
	LPCTSTR pstrPassword /* = NULL */,
	DWORD_PTR dwContext /* = 1 */)
	: CInternetConnection(pSession, pstrServer, nPort, dwContext)
{
	ASSERT(pSession != NULL);
	ASSERT_KINDOF(CInternetSession, pSession);
	ASSERT((dwFlags & INTERNET_FLAG_ASYNC) == 0);
	ASSERT(AfxIsValidString(pstrServer));

	m_hConnection = InternetConnect((HINTERNET) *pSession, pstrServer,
		nPort, pstrUserName, pstrPassword, INTERNET_SERVICE_HTTP,
		dwFlags, m_dwContext);

	if (m_hConnection == NULL)
		AfxThrowInternetException(m_dwContext);
	else
		_afxSessionMap.SetAt(m_hConnection, m_pSession);
}

CHttpFile* CHttpConnection::OpenRequest(LPCTSTR pstrVerb,
	LPCTSTR pstrObjectName, LPCTSTR pstrReferer, DWORD_PTR dwContext,
	LPCTSTR* ppstrAcceptTypes, LPCTSTR pstrVersion, DWORD dwFlags)
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);
	ASSERT((dwFlags & INTERNET_FLAG_ASYNC) == 0);

	if (dwContext == 1)
		dwContext = m_dwContext;

	if (pstrVersion == NULL)
		pstrVersion = HTTP_VERSION;

	HINTERNET hFile;
	hFile = HttpOpenRequest(m_hConnection, pstrVerb, pstrObjectName,
		pstrVersion, pstrReferer, ppstrAcceptTypes, dwFlags, dwContext);

	CHttpFile* pRet = new CHttpFile(hFile, pstrVerb, pstrObjectName, this);
	if (pRet != NULL)
		pRet->m_dwContext = dwContext;
	return pRet;
}

CHttpFile* CHttpConnection::OpenRequest(int nVerb,
	LPCTSTR pstrObjectName, LPCTSTR pstrReferer /* = NULL */, DWORD_PTR dwContext,
	LPCTSTR* ppstrAcceptTypes /* = NULL */,
	LPCTSTR pstrVersion /* = NULL */, DWORD dwFlags)
{
	ASSERT_VALID(this);
	ASSERT(m_hConnection != NULL);
	ASSERT((dwFlags & INTERNET_FLAG_ASYNC) == 0);
	ASSERT(AfxIsValidString(pstrObjectName));

	ASSERT(nVerb >= _HTTP_VERB_MIN && nVerb <= _HTTP_VERB_MAX);

	LPCTSTR pstrVerb;
	if (nVerb >= _HTTP_VERB_MIN && nVerb <= _HTTP_VERB_MAX)
		pstrVerb = szHtmlVerbs[nVerb];
	else
		pstrVerb = _T("");

	return OpenRequest(pstrVerb, pstrObjectName, pstrReferer,
		dwContext, ppstrAcceptTypes, pstrVersion, dwFlags);
}

#ifdef _DEBUG
void CHttpConnection::Dump(CDumpContext& dc) const
{
	CInternetConnection::Dump(dc);
	dc << "\nm_strServerName = " << m_strServerName;
}

void CHttpConnection::AssertValid() const
{
	ASSERT(m_pSession != NULL);
	if (m_hConnection != NULL)
	{
		ASSERT(AfxGetInternetHandleType(m_hConnection)
				== INTERNET_HANDLE_TYPE_CONNECT_HTTP);
	}
}
#endif


/////////////////////////////////////////////////////////////////////////////
// CHttpFile

CHttpFile::CHttpFile(HINTERNET hFile, HINTERNET hSession, LPCTSTR pstrObject,
	LPCTSTR pstrServer, LPCTSTR pstrVerb, DWORD_PTR dwContext)
 : CInternetFile(hFile, hSession, pstrObject, pstrServer, dwContext, TRUE),
	m_strVerb(pstrVerb), m_strObject(pstrObject)
{
	// caller must set _afxSessionMap!
	ASSERT(AfxIsValidString(pstrVerb));
	m_hConnection = hFile;
}


CHttpFile::CHttpFile(HINTERNET hFile, LPCTSTR pstrVerb, LPCTSTR pstrObject,
	CHttpConnection* pConnection)
 : CInternetFile(hFile, pstrObject, pConnection, TRUE),
	m_strVerb(pstrVerb), m_strObject(pstrObject)
{
	ASSERT(pstrVerb != NULL);
	ASSERT(pstrObject != NULL);
	ASSERT(pConnection != NULL);
	ASSERT_VALID(pConnection);
	m_hConnection = hFile;
}

CHttpFile::~CHttpFile()
{
}

DWORD CHttpFile::ErrorDlg(CWnd* pParent /* = NULL */,
	DWORD dwError /* = ERROR_INTERNET_INCORRECT_PASSWORD */,
	DWORD dwFlags /* = FLAGS_ERROR_UI_FLAGS_GENERATE_DATA | FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS*/,
	LPVOID* lppvData /* = NULL */)
{
	ASSERT((dwFlags & INTERNET_FLAG_ASYNC) == 0);
	HWND hWnd;
	LPVOID lpEmpty;
	LPVOID* lppvHolder;

	if (lppvData == NULL)
	{
		lpEmpty = NULL;
		lppvHolder = &lpEmpty;
	}
	else
		lppvHolder = lppvData;

	if (pParent == NULL || pParent->m_hWnd == NULL)
		hWnd = GetDesktopWindow();
	else
		hWnd = pParent->m_hWnd;

	return InternetErrorDlg(hWnd, m_hFile, dwError, dwFlags, lppvHolder);
}

CString CHttpFile::GetVerb() const
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	return m_strVerb;
}

CString CHttpFile::GetObject() const
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	return m_strObject;
}

CString CHttpFile::GetFileURL() const
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	CString str(_afxURLhttp);
	if (m_hConnection != NULL)
	{
		str += m_strServerName;
		INT_PTR nLen = m_strObject.GetLength();
		if (nLen > 0)
		{
			if (m_strObject[0] != '/' && m_strObject[0] != '\\')
				str += '/';
			str += m_strObject;
		}
	}

	return str;
}

BOOL CHttpFile::AddRequestHeaders(LPCTSTR pstrHeaders,
	DWORD dwModifiers /* = HTTP_ADDREQ_FLAG_ADD */,
	int dwHeadersLen /* = -1 */)
{
	ASSERT(AfxIsValidString(pstrHeaders));
	ASSERT(dwHeadersLen == 0 || pstrHeaders != NULL);
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	if (dwHeadersLen == -1)
		if (pstrHeaders == NULL)
			dwHeadersLen = 0;
		else
			dwHeadersLen = lstrlen(pstrHeaders);

	return HttpAddRequestHeaders(m_hFile, pstrHeaders, dwHeadersLen,
		dwModifiers);
}

BOOL CHttpFile::AddRequestHeaders(CString& str,
	DWORD dwModifiers /* = HTTP_ADDREQ_FLAG_ADD */)
{
	return AddRequestHeaders((LPCTSTR) str, dwModifiers, (int)str.GetLength());
}

BOOL CHttpFile::SendRequest(LPCTSTR pstrHeaders /* = NULL */,
	DWORD dwHeadersLen /* = 0 */, LPVOID lpOptional /* = NULL */,
	DWORD dwOptionalLen /* = 0 */)
{
	ASSERT(dwOptionalLen == 0 || lpOptional != NULL);
	ASSERT(dwHeadersLen == 0 || pstrHeaders != NULL);
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	BOOL bRet = HttpSendRequest(m_hFile,
		pstrHeaders, dwHeadersLen, lpOptional, dwOptionalLen);

	if (!bRet)
		AfxThrowInternetException(m_dwContext);

	return bRet;
}

BOOL CHttpFile::EndRequest(
	DWORD dwFlags /* = 0 */,
	LPINTERNET_BUFFERS lpBuffIn /* = NULL */, DWORD_PTR dwContext /* = 1 */)
{
	ASSERT(m_hFile != NULL);
	ASSERT(m_bReadMode == -1);

	if (dwContext == 1)
		dwContext = m_dwContext;

	BOOL bRet = HttpEndRequest(m_hFile, lpBuffIn, dwFlags, dwContext);

	if (!bRet)
		AfxThrowInternetException(m_dwContext);
	return bRet;
}

BOOL CHttpFile::SendRequestEx(DWORD dwTotalLen,
	DWORD dwFlags /* = HSR_INITIATE */, DWORD_PTR dwContext /* = 1 */)
{
	ASSERT(m_hFile != NULL);

	INTERNET_BUFFERS buffer;
	memset(&buffer, 0, sizeof(buffer));
	buffer.dwStructSize = sizeof(buffer);
	buffer.dwBufferTotal = dwTotalLen;

	if (dwContext == 1)
		dwContext = m_dwContext;

	return SendRequestEx(&buffer, NULL, dwFlags, dwContext);
}

BOOL CHttpFile::SendRequestEx(LPINTERNET_BUFFERS lpBuffIn,
	LPINTERNET_BUFFERS lpBuffOut, DWORD dwFlags /* = HSR_INITIATE */,
	DWORD_PTR dwContext /* = 1 */)
{
	ASSERT(m_hFile != NULL);
	ASSERT_NULL_OR_POINTER(lpBuffIn, INTERNET_BUFFERS);
	ASSERT_NULL_OR_POINTER(lpBuffOut, INTERNET_BUFFERS);

	if (dwContext == 1)
		dwContext = m_dwContext;

	BOOL bRet = HttpSendRequestEx(m_hFile, lpBuffIn, lpBuffOut,
		dwFlags, dwContext);

	if (!bRet)
		AfxThrowInternetException(m_dwContext);

	m_bReadMode = -1;
	return bRet;
}

BOOL CHttpFile::SendRequest(CString& strHeaders,
	LPVOID lpOptional /* = NULL */, DWORD dwOptionalLen /* = 0 */)
{
	ASSERT(dwOptionalLen == 0 || lpOptional != NULL);
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	return SendRequest((LPCTSTR) strHeaders, (ULONG)strHeaders.GetLength(),
		lpOptional, dwOptionalLen);
}

BOOL CHttpFile::QueryInfo(DWORD dwInfoLevel,
	LPVOID lpvBuffer, LPDWORD lpdwBufferLength, LPDWORD lpdwIndex) const
{
	ASSERT(((HTTP_QUERY_HEADER_MASK & dwInfoLevel) <= HTTP_QUERY_MAX || 
		dwInfoLevel == HTTP_QUERY_CUSTOM) && dwInfoLevel != 0);
	ASSERT(lpvBuffer != NULL && *lpdwBufferLength > 0);
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	return HttpQueryInfo(m_hFile, dwInfoLevel, lpvBuffer,
		lpdwBufferLength, lpdwIndex);
}

BOOL CHttpFile::QueryInfo(DWORD dwInfoLevel, DWORD& dwResult,
	LPDWORD lpdwIndex /* = NULL */) const
{
	dwInfoLevel |= HTTP_QUERY_FLAG_NUMBER;
	DWORD dwDWSize = sizeof(DWORD);
	return QueryInfo(dwInfoLevel, &dwResult, &dwDWSize, lpdwIndex);
}

BOOL CHttpFile::QueryInfo(DWORD dwInfoLevel, SYSTEMTIME* pSystemTime,
	LPDWORD lpdwIndex /* = NULL */) const
{
	dwInfoLevel |= HTTP_QUERY_FLAG_SYSTEMTIME;
	DWORD dwTimeSize = sizeof(SYSTEMTIME);
	return QueryInfo(dwInfoLevel, pSystemTime, &dwTimeSize, lpdwIndex);
}

BOOL CHttpFile::QueryInfoStatusCode(DWORD& dwStatusCode) const
{
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	TCHAR szBuffer[80];
	DWORD dwLen = _countof(szBuffer);
	BOOL bRet;

	bRet = HttpQueryInfo(m_hFile, HTTP_QUERY_STATUS_CODE,
				szBuffer, &dwLen, NULL);

	if (bRet)
		dwStatusCode = (DWORD) _ttol(szBuffer);
	return bRet;
}

BOOL CHttpFile::QueryInfo(DWORD dwInfoLevel, CString& str,
	LPDWORD lpdwIndex) const
{
	ASSERT(dwInfoLevel <= HTTP_QUERY_MAX);
	ASSERT_VALID(this);
	ASSERT(m_hFile != NULL);

	BOOL bRet;
	DWORD dwLen = 0;

	// ask for nothing to see how long the return really is

	str.Empty();
	if (HttpQueryInfo(m_hFile, dwInfoLevel, NULL, &dwLen, 0))
		bRet = TRUE;
	else
	{
		// now that we know how long it is, ask for exactly that much
		// space and really request the header from the API

		LPTSTR pstr = str.GetBufferSetLength(dwLen);
		bRet = HttpQueryInfo(m_hFile, dwInfoLevel, pstr, &dwLen, lpdwIndex);
		if (bRet)
			str.ReleaseBuffer(dwLen);
		else
			str.ReleaseBuffer(0);
	}

	return bRet;
}

#ifdef _DEBUG
void CHttpFile::Dump(CDumpContext& dc) const
{
	dc << "\nm_strFileName = " << m_strFileName;
	dc << "\nm_strVerb = " << m_strVerb;
}

void CHttpFile::AssertValid() const
{
	CInternetFile::AssertValid();
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CGopherFile

CGopherFile::CGopherFile(HINTERNET hFile, CGopherLocator& refLocator,
	CGopherConnection* pConnection)
	: CInternetFile(hFile, _T(""), pConnection, TRUE),
		m_Locator(refLocator)
{
	ASSERT(pConnection != NULL);
	ASSERT_VALID(pConnection);
}

CGopherFile::CGopherFile(HINTERNET hFile, HINTERNET hSession,
	LPCTSTR pstrLocator, DWORD dwLocLen, DWORD_PTR dwContext)
	: CInternetFile(hFile, hSession, _T(""), _T(""), dwContext, TRUE),
		m_Locator(pstrLocator, dwLocLen)
{
	// caller muset set _afxSessionMap!
}

CGopherFile::~CGopherFile()
{
}

void CGopherFile::Write(const void* lpBuf, UINT nCount)
{
	UNUSED_ALWAYS(lpBuf);
	UNUSED_ALWAYS(nCount);

	ASSERT(FALSE);
	AfxThrowNotSupportedException();
}

void CGopherFile::WriteString(LPCTSTR pstr)
{
	UNUSED_ALWAYS(pstr);

	ASSERT(FALSE);
	AfxThrowNotSupportedException();
}

#ifdef _DEBUG
void CGopherFile::Dump(CDumpContext& dc) const
{
	CInternetFile::Dump(dc);
}

void CGopherFile::AssertValid() const
{
	CInternetFile::AssertValid();
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CFtpFileFind

CFtpFileFind::CFtpFileFind(CFtpConnection* pConnection, DWORD_PTR dwContext)
{
	ASSERT(pConnection != NULL);

	if (NULL == pConnection)
	{
		AfxThrowInvalidArgException();
	}

	ASSERT_KINDOF(CFtpConnection, pConnection);

	m_pConnection = pConnection;
	if (dwContext == 1)
		dwContext = pConnection->GetContext();
	m_dwContext = dwContext;
	m_chDirSeparator = '/';
}

CFtpFileFind::~CFtpFileFind()
{
	Close();
}

BOOL CFtpFileFind::FindFile(LPCTSTR pstrName /* = NULL */,
	DWORD dwFlags /* = INTERNET_FLAG_RELOAD */)
{
	ASSERT((dwFlags & INTERNET_FLAG_ASYNC) == 0);
	ASSERT(m_pConnection != NULL);
	ASSERT_VALID(m_pConnection);

	if (m_pConnection == NULL)
		return FALSE;

	if ( lstrlen(pstrName) >= MAX_PATH )
		return FALSE; // WIN32_FIND_DATA.cFileName is MAX_PATH in length
	Close();
	m_pNextInfo = new WIN32_FIND_DATA;
	if (m_pNextInfo == NULL)
		return FALSE;

	if (pstrName == NULL)
		pstrName = _T("*");

	WIN32_FIND_DATA *pFindData = (WIN32_FIND_DATA *)m_pNextInfo;
	Checked::tcsncpy_s(pFindData->cFileName, _countof(pFindData->cFileName), pstrName, _TRUNCATE);

	m_hContext = FtpFindFirstFile((HINTERNET) *m_pConnection,
		pstrName, (LPWIN32_FIND_DATA) m_pNextInfo, dwFlags, m_dwContext);
	if (m_hContext == NULL)
	{
		Close();
		return FALSE;
	}

	LPCTSTR pstrRoot = _tcspbrk(pstrName, _T("\\/"));
	CString strCWD;
	m_pConnection->GetCurrentDirectory(strCWD);

	if (pstrRoot == NULL)
	{
		if (m_pConnection->SetCurrentDirectory(pstrName))
		{
			m_pConnection->GetCurrentDirectory(m_strRoot);
			m_pConnection->SetCurrentDirectory(strCWD);
		}
		else
			m_strRoot = strCWD;
	}
	else
	{
		// find the last forward or backward whack

		int nLast;
		LPCTSTR pstrOther = _tcsrchr(pstrName, '\\');
		pstrRoot = _tcsrchr(pstrName, '/');

		if (pstrRoot == NULL)
			pstrRoot = pstrName;
		if (pstrOther == NULL)
			pstrOther = pstrName;

		if (pstrRoot >= pstrOther)
			nLast = ULONG( pstrRoot - pstrName );
		else
			nLast = ULONG( pstrOther - pstrName );

		// from the start to the last whack is the root

		if (nLast == 0)
			nLast++;

		m_strRoot = pstrName;
		m_strRoot = m_strRoot.Left(nLast);
	}

	return TRUE;
}

BOOL CFtpFileFind::FindNextFile()
{
	ASSERT(m_hContext != NULL);
	if (m_hContext == NULL)
		return FALSE;

	if (m_pFoundInfo == NULL)
	{
		m_pFoundInfo = new WIN32_FIND_DATA;
		if (m_pFoundInfo == NULL)
			return FALSE;
	}

	ASSERT_VALID(this);
	void* pTemp = m_pFoundInfo;
	m_pFoundInfo = m_pNextInfo;
	m_pNextInfo = pTemp;

	return InternetFindNextFile(m_hContext, m_pNextInfo);
}

void CFtpFileFind::CloseContext()
{
	if (m_hContext != NULL && m_hContext != INVALID_HANDLE_VALUE)
	{
		InternetCloseHandle(m_hContext);
		m_hContext = NULL;
	}
}

CString CFtpFileFind::GetFileURL() const
{
	ASSERT_VALID(this);
	ASSERT(m_hContext != NULL);

	CString str;

	if (m_hContext != NULL)
	{
		str += _afxURLftp;
		str += m_pConnection->GetServerName();
		str += GetFilePath();
	}

	return str;
}

#ifdef _DEBUG
void CFtpFileFind::Dump(CDumpContext& dc) const
{
	CFileFind::Dump(dc);
	dc << "m_hContext = " << m_hContext;
}

void CFtpFileFind::AssertValid() const
{
	CFileFind::AssertValid();
}
#endif


/////////////////////////////////////////////////////////////////////////////
// CGopherFileFind

CGopherFileFind::CGopherFileFind(CGopherConnection* pConnection,
	DWORD_PTR dwContext)
{
	ASSERT(pConnection != NULL);
	ASSERT_KINDOF(CGopherConnection, pConnection);

	m_pConnection = pConnection;
	if (dwContext == 1)
		dwContext = pConnection->GetContext();
	m_dwContext = dwContext;
}

CGopherFileFind::~CGopherFileFind()
{
	Close();
}

BOOL CGopherFileFind::FindFile(LPCTSTR pstrString,
	DWORD dwFlags /* = INTERNET_FLAG_RELOAD */)
{
	ASSERT((dwFlags & INTERNET_FLAG_ASYNC) == 0);
	Close();

	m_pNextInfo = new GOPHER_FIND_DATA;
	if (m_pNextInfo == NULL)
		return FALSE;

	m_hContext = GopherFindFirstFile((HINTERNET) *m_pConnection,
		NULL, pstrString,
		(GOPHER_FIND_DATA*) m_pNextInfo, dwFlags, m_dwContext);

	if (m_hContext == NULL)
		Close();
	return (m_hContext != NULL);
}

BOOL CGopherFileFind::FindFile(CGopherLocator& refLocator,
	LPCTSTR pstrString, DWORD dwFlags /* = INTERNET_FLAG_RELOAD */)
{
	ASSERT((dwFlags & INTERNET_FLAG_ASYNC) == 0);
	Close();

	m_pNextInfo = new GOPHER_FIND_DATA;
	if (m_pNextInfo == NULL)
		return FALSE;

	m_pFoundInfo = new GOPHER_FIND_DATA;
	if (m_pFoundInfo == NULL)
	{
		delete m_pNextInfo;
		m_pNextInfo = NULL;
		return FALSE;
	}

	m_hContext = GopherFindFirstFile((HINTERNET) *m_pConnection,
		(LPCTSTR) refLocator, pstrString,
		(GOPHER_FIND_DATA*) m_pNextInfo, dwFlags, m_dwContext);

	if (m_hContext == NULL)
		Close();
	return (m_hContext != NULL);
}

BOOL CGopherFileFind::FindNextFile()
{
	ASSERT(m_hContext != NULL);
	if (m_hContext == NULL)
		return FALSE;

	if (m_pFoundInfo == NULL)
	{
		m_pFoundInfo = new GOPHER_FIND_DATA;
		if (m_pFoundInfo == NULL)
			return FALSE;
	}

	ASSERT_VALID(this);
	void* pTemp = m_pFoundInfo;
	m_pFoundInfo = m_pNextInfo;
	m_pNextInfo = pTemp;

	return InternetFindNextFile(m_hContext, m_pNextInfo);
}

void CGopherFileFind::CloseContext()
{
	if (m_hContext != NULL && m_hContext != INVALID_HANDLE_VALUE)
	{
		InternetCloseHandle(m_hContext);
		m_hContext = NULL;
	}
}

CString CGopherFileFind::GetFileName() const
{
	AfxThrowNotSupportedException();
}

CString CGopherFileFind::GetFilePath() const
{
	AfxThrowNotSupportedException();
}

CString CGopherFileFind::GetFileTitle() const
{
	AfxThrowNotSupportedException();
}

BOOL CGopherFileFind::IsDots() const
{
	// gophers never have dots
	return FALSE;
}

BOOL CGopherFileFind::GetLastWriteTime(FILETIME* pTimeStamp) const
{
	ASSERT(m_hContext != NULL);
	ASSERT_POINTER(pTimeStamp, FILETIME);
	ASSERT_VALID(this);

	if (m_pFoundInfo != NULL && pTimeStamp != NULL)
	{
		*pTimeStamp = ((LPGOPHER_FIND_DATA) m_pFoundInfo)->LastModificationTime;
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CGopherFileFind::GetLastAccessTime(FILETIME* pTimeStamp) const
{
	ASSERT_POINTER(pTimeStamp, FILETIME);
	return GetLastWriteTime(pTimeStamp);
}

BOOL CGopherFileFind::GetCreationTime(FILETIME* pTimeStamp) const
{
	ASSERT_POINTER(pTimeStamp, FILETIME);
	return GetLastWriteTime(pTimeStamp);
}

BOOL CGopherFileFind::GetLastWriteTime(CTime& refTime) const
{
	ASSERT(m_hContext != NULL);
	ASSERT_VALID(this);

	if (m_pFoundInfo != NULL)
	{
		refTime = CTime(((LPGOPHER_FIND_DATA) m_pFoundInfo)->LastModificationTime);
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CGopherFileFind::GetCreationTime(CTime& refTime) const
{
	return GetLastWriteTime(refTime);
}

BOOL CGopherFileFind::GetLastAccessTime(CTime& refTime) const
{
	return GetLastWriteTime(refTime);
}


CString CGopherFileFind::GetFileURL() const
{
	AfxThrowNotSupportedException();
}

CString CGopherFileFind::GetRoot() const
{
	AfxThrowNotSupportedException();
}

CGopherLocator CGopherFileFind::GetLocator() const
{
	ASSERT_VALID(this);
	ASSERT(m_pConnection != NULL && m_hContext != NULL);

	return m_pConnection->CreateLocator(
		((LPGOPHER_FIND_DATA) m_pFoundInfo)->Locator);
}

CString CGopherFileFind::GetScreenName() const
{
	ASSERT_VALID(this);
	ASSERT(m_hContext != NULL);

	CString str;

	if (m_pFoundInfo != NULL)
		str = ((LPGOPHER_FIND_DATA) m_pFoundInfo)->DisplayString;

	return str;
}

ULONGLONG CGopherFileFind::GetLength() const
{
	ASSERT_VALID(this);

   ULARGE_INTEGER nLength;

	if (m_pFoundInfo != NULL)
   {
	  nLength.LowPart = ((LPGOPHER_FIND_DATA) m_pFoundInfo)->SizeLow;
	  nLength.HighPart = ((LPGOPHER_FIND_DATA) m_pFoundInfo)->SizeHigh;
   }
   else
   {
	  nLength.QuadPart = 0;
   }

   return nLength.QuadPart;
}

#ifdef _DEBUG
void CGopherFileFind::Dump(CDumpContext& dc) const
{
	CFileFind::Dump(dc);
	dc << "m_hContext = " << m_hContext;
}

void CGopherFileFind::AssertValid() const
{
	CFileFind::AssertValid();
}
#endif


/////////////////////////////////////////////////////////////////////////////
// CGopherLocator

CGopherLocator::CGopherLocator(LPCTSTR pstrLocator, DWORD dwLocLen)
{
	ASSERT(AfxIsValidString(pstrLocator));
	LPTSTR pstr = m_Locator.GetBufferSetLength(dwLocLen);
	Checked::memcpy_s(pstr, dwLocLen, pstrLocator, dwLocLen);
	m_Locator.ReleaseBuffer(dwLocLen);
	m_dwBufferLength = dwLocLen;
}

CGopherLocator::~CGopherLocator()
{
}

/////////////////////////////////////////////////////////////////////////////
// exception handling

void AFXAPI AfxThrowInternetException(DWORD_PTR dwContext, DWORD dwError /* = 0 */)
{
	if (dwError == 0)
		dwError = ::GetLastError();

	CInternetException* pException = new CInternetException(dwError);
	pException->m_dwContext = dwContext;

	TRACE(traceInternet, 0, "Warning: throwing CInternetException for error %d\n", dwError);
	THROW(pException);
}


BOOL CInternetException::GetErrorMessage(_Out_z_cap_(nMaxError) LPTSTR pstrError, _In_ UINT nMaxError,
	_Out_opt_ PUINT pnHelpContext) const
{
	ASSERT(pstrError != NULL && AfxIsValidString(pstrError, nMaxError));

	if (pnHelpContext != NULL)
	{
		*pnHelpContext = 0;
	}

	LPTSTR lpBuffer;
	BOOL bRet = TRUE;

	HINSTANCE hWinINetLibrary;
	hWinINetLibrary = ::AfxCtxLoadLibrary(_T("WININET.DLL"));

	if (hWinINetLibrary == NULL ||
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
			hWinINetLibrary, m_dwError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
			(LPTSTR) &lpBuffer, 0, NULL) == 0)
	{
		// it failed! try Windows...

		bRet = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,  m_dwError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
			(LPTSTR) &lpBuffer, 0, NULL);
	}

	if (!bRet)
	{
		*pstrError = '\0';
	}
	else
	{
		if (m_dwError == ERROR_INTERNET_EXTENDED_ERROR)
		{
			LPTSTR lpExtended;
			DWORD dwLength = 0;
			DWORD dwError;

			// find the length of the error
			if (!InternetGetLastResponseInfo(&dwError, NULL, &dwLength) &&
				GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				lpExtended = (LPTSTR) LocalAlloc(LPTR, dwLength*sizeof(TCHAR));
				if(lpExtended)
				{
					InternetGetLastResponseInfo(&dwError, lpExtended, &dwLength);
					if( nMaxError >= dwLength )
					{
					    Checked::tcsncpy_s(pstrError, nMaxError, lpExtended, _TRUNCATE);
					}
					else
					{
					    *pstrError = '\0';
					    bRet=FALSE;
					}
					LocalFree(lpExtended);
				}
				else
				{
					*pstrError = '\0';
					bRet=FALSE;
				}
			}
			else
			{
				TRACE(traceInternet, 0, "Warning: Extended error reported with no response info\n");
			}
			bRet = TRUE;
		}
		else
		{
			Checked::tcsncpy_s(pstrError, nMaxError, lpBuffer, _TRUNCATE);
			bRet = TRUE;
		}

		LocalFree(lpBuffer);
	}

#ifndef _AFXDLL
	::FreeLibrary(hWinINetLibrary);
#endif
	return bRet;
}

CInternetException::CInternetException(DWORD dwError)
{
	m_dwError = dwError;
}

CInternetException::~CInternetException()
{
}

#ifdef _DEBUG
void CInternetException::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	dc << "m_dwError = " << m_dwError;
	dc << "\nm_dwContext = " << m_dwContext;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// Inline function declarations expanded out-of-line

#ifndef _AFX_ENABLE_INLINES

// expand inlines for OLE dialog APIs
#define _AFXINET_INLINE
#include "afxinet.inl"

#endif //!_AFX_ENABLE_INLINES

/////////////////////////////////////////////////////////////////////////////
// Pre-startup code


IMPLEMENT_DYNAMIC(CInternetException, CException)
IMPLEMENT_DYNAMIC(CInternetFile, CStdioFile)
IMPLEMENT_DYNAMIC(CHttpFile, CInternetFile)
IMPLEMENT_DYNAMIC(CGopherFile, CInternetFile)
IMPLEMENT_DYNAMIC(CInternetSession, CObject)
IMPLEMENT_DYNAMIC(CInternetConnection, CObject)
IMPLEMENT_DYNAMIC(CFtpConnection, CInternetConnection)
IMPLEMENT_DYNAMIC(CHttpConnection, CInternetConnection)
IMPLEMENT_DYNAMIC(CGopherConnection, CInternetConnection)
IMPLEMENT_DYNAMIC(CFtpFileFind, CFileFind)
IMPLEMENT_DYNAMIC(CGopherFileFind, CFileFind)

#pragma warning(disable: 4074)
#pragma init_seg(lib)

CSessionMapPtrToPtr _afxSessionMap;
