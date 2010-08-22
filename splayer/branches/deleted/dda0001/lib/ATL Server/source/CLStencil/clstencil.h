// File: clstencil.h
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Classes Reference and related electronic
// documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft C++ Libraries products.

#pragma once

#include <atlisapi.h>
#include <stdio.h>
#include "resource.h"

ATL_NOINLINE inline void GetHrErrorDescription(HRESULT hr, CString& strErr) throw()
{
	LPTSTR pszMsg = NULL;
	::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM, NULL, hr, 0, (LPSTR)&pszMsg, 0, NULL);
	strErr+= pszMsg;
	::LocalFree(pszMsg);
}

inline bool Emit(CStringA& strBuffer, UINT uID, ...)
{
	_ATLTRY
	{
		CStringA str;
		if (str.LoadString(uID))
		{
			va_list arglist;
			va_start(arglist, uID);
			int nLen = _vscprintf((LPCSTR) str, arglist);
			char *szBuf = strBuffer.GetBuffer(nLen+1);
			if (szBuf != NULL)
			{
				vsprintf_s(szBuf, nLen+1, str, arglist);
				strBuffer.ReleaseBuffer(nLen);
			}
			else
			{
				strBuffer.ReleaseBuffer();
			}

			va_end(arglist);
			return true;
		}
	}
	_ATLCATCHALL()
	{
		return false;
	}
	return false;
}

class CSProcServerContext : public IHttpServerContext
{
protected:

	CAtlFile m_OutFile;
	CAtlFile m_InFile;
	CAtlFile m_ErrFile;
	
	bool m_bStdInput;
	bool m_bStdOutput;
	bool m_bStdError;

	char m_szPathTranslated[MAX_PATH];
	char m_szScriptPathTranslated[MAX_PATH];
	char m_szQueryString[ATL_URL_MAX_URL_LENGTH];
	char m_szContentType[ATL_URL_MAX_URL_LENGTH];
	char m_szVerb[MAX_PATH];

	void CloseFiles()
	{
		if(m_bStdInput)
			m_InFile.Detach();
		else
			m_InFile.Close();

		if(m_bStdOutput) 
			m_OutFile.Detach() ;
		else
			m_OutFile.Close();
		
		if(m_bStdError)
			m_ErrFile.Detach() ;
		else
			m_ErrFile.Close();
	}

public:
	CString m_strErr;

	CSProcServerContext()
	{
		m_szQueryString[0] = '\0';
		m_szScriptPathTranslated[0] = '\0';
		m_szQueryString[0] = '\0';
		m_szContentType[0] = '\0';
		m_szVerb[0] = '\0';
		m_bStdInput = false;
		m_bStdOutput = false;
		m_bStdError = false;
	}

	void LogServerError(LPCSTR szErr) throw()
	{
		if (m_ErrFile.m_h == NULL)
			return;

		CStringA strBuffer;
		if (Emit(strBuffer, IDS_NOTSUPPORTED, szErr))
		{
			m_ErrFile.Write(strBuffer, strBuffer.GetLength());
		}
	}

	// Implementation
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv) throw()
	{
		if (!ppv)
			return E_POINTER;
		if (InlineIsEqualGUID(riid, __uuidof(IUnknown)) ||
			InlineIsEqualGUID(riid, __uuidof(IHttpServerContext)))
		{
			*ppv = static_cast<IUnknown*>(this);
			AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	
	// Implementation
	ULONG STDMETHODCALLTYPE AddRef() throw()
	{
		return 1;
	}
	
	// Implementation
	ULONG STDMETHODCALLTYPE Release() throw()
	{
		return 1;
	}

	HRESULT Initialize(
		LPCSTR szFileName, 
		LPCSTR szQueryString, 
		LPCSTR szErrLog, 
		LPCSTR szFormFile,
		LPCSTR szContentType,
		LPCSTR szVerb) throw()
	{
		HRESULT hr;

		if (szFileName == NULL)
		{
			// use Standard output
			HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			if (hOut == INVALID_HANDLE_VALUE)
				return E_FAIL;
			m_OutFile.Attach(hOut);
			m_bStdOutput = true;		
			hr = S_OK;
		}
		else
		{
			hr = m_OutFile.Create(szFileName, GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS);
		}
		
		if (hr != S_OK)
		{
			m_strErr.Format("ERROR: %s : ", szFileName ? szFileName : "stdout");
			GetHrErrorDescription(hr, m_strErr);
			return hr;
		}

		if (szErrLog)
		{
			hr = m_ErrFile.Create(szErrLog, GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS);
		}
		else
		{
			HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);
			if (hErr == INVALID_HANDLE_VALUE)
				hr = E_FAIL;
			else
			{
				m_ErrFile.Attach(hErr);
				m_bStdError = true;
				hr = S_OK;
			}
		}
		if (hr != S_OK)
		{
			m_strErr.Format("ERROR: %s : ", szErrLog ? szErrLog : "stderr");
			GetHrErrorDescription(hr, m_strErr);
			CloseFiles();
			return hr;
		}

		if (szFormFile)
		{
			hr = m_InFile.Create(szFormFile, GENERIC_READ, FILE_SHARE_READ, OPEN_ALWAYS);
		}
		else
		{
			HANDLE hForm = GetStdHandle(STD_INPUT_HANDLE);
			if (hForm == INVALID_HANDLE_VALUE)
				hr = E_FAIL;
			else
			{
				m_InFile.Attach(hForm);
				m_bStdInput = true;
				hr = S_OK;
			}
		}
		if (hr != S_OK)
		{
			m_strErr.Format("ERROR: %s : ", szFormFile ? szFormFile : "stdin");
			GetHrErrorDescription(hr, m_strErr);
			CloseFiles();
			return hr;
		}

		if ( szQueryString != NULL)
			if( strlen(szQueryString) < ( sizeof(m_szQueryString)-1 ) )
				strcpy_s(m_szQueryString, sizeof(m_szQueryString), szQueryString);
			else
			{
				m_strErr.LoadString(IDS_QS_TOO_LONG);
				CloseFiles();
				return E_INVALIDARG;
			}

		if ( szContentType != NULL)
			if(strlen(szContentType) < ( sizeof(m_szContentType)-1 ) )
				strcpy_s(m_szContentType, sizeof(m_szContentType), szContentType);
			else
			{
				m_strErr.LoadString(IDS_CT_TOO_LONG);
				CloseFiles();
				return E_INVALIDARG;
			}

		if (szVerb == NULL)
			strcpy_s(m_szVerb, sizeof(m_szVerb), "GET");
		else
		{
			if ( strlen(szVerb) < ( sizeof(m_szVerb)-1 ) )
				strcpy_s(m_szVerb, sizeof(m_szVerb), szVerb);
			else
			{
				m_strErr.LoadString(IDS_VERB_TOO_LONG);
				CloseFiles();
				return E_INVALIDARG;
			}
		}

		return S_OK;
	}

	~CSProcServerContext() throw()
	{
	}

	// Returns the HTTP status code.
	// Equivalent to EXTENSION_CONTROL_BLOCK::dwHttpStatusCode.
	DWORD GetHttpStatusCode() throw()
	{
		return 0;
	}

	// Returns a nul-terminated string that contains the HTTP method of the current request.
	// Examples of common HTTP methods include "GET" and "POST".
	// Equivalent to the REQUEST_METHOD server variable or EXTENSION_CONTROL_BLOCK::lpszMethod.
	LPCSTR GetRequestMethod() throw()
	{
		return m_szVerb;
	}

	// Returns a nul-terminated string that contains the query information.
	// This is the part of the URL that appears after the question mark (?). 
	// Equivalent to the QUERY_STRING server variable or EXTENSION_CONTROL_BLOCK::lpszQueryString.
	LPCSTR GetQueryString() throw()
	{
		return m_szQueryString;
	}

	// Returns a nul-terminated string that contains the path of the current request.
	// This is the part of the URL that appears after the server name, but before the query string.
	// Equivalent to the PATH_INFO server variable or EXTENSION_CONTROL_BLOCK::lpszPathInfo.
	LPCSTR GetPathInfo() throw()
	{
		return m_szPathTranslated;
	}

	// Call this function to retrieve a nul-terminated string containing the physical path of the script.
	//
	// Returns TRUE on success, and FALSE on failure. Call GetLastError to get extended error information.
	//
	// On entry, pdwSize should point to a DWORD that indicates the size of the buffer in bytes.
	// On exit, the DWORD contains the number of bytes transferred or available to be transferred into the buffer (including the nul-terminating byte).
	// The script path is the same as GetPathTranslated up to the first .srf or .dll.
	// For example, if GetPathTranslated returns "c:\inetpub\vcisapi\hello.srf\goodmorning",
	// then this function returns "c:\inetpub\vcisapi\hello.srf".
	LPCSTR GetScriptPathTranslated() throw()
	{		
        return m_szScriptPathTranslated;
    }

	// Returns a nul-terminated string that contains the translated path of the requested resource.
	// This is the path of the resource on the local server.
	// Equivalent to the PATH_TRANSLATED server variable or EXTENSION_CONTROL_BLOCK::lpszPathTranslated.
	LPCSTR GetPathTranslated() throw()
	{
		return m_szPathTranslated;
	}

	BOOL SetPathTranslated(LPCSTR szPath)
	{
		ATLENSURE(szPath != NULL);
		m_szPathTranslated[0] = '\0';

		if ( strlen(szPath) < sizeof(m_szPathTranslated)-1 )
			strcpy_s(m_szPathTranslated, sizeof(m_szPathTranslated), szPath);
		else
		{
			ATLTRACE("SetPathTranslated got a translated path that was too long.\n");
			return FALSE;
		}

        // Initialize the translated script path
		LPCSTR szEnd = m_szPathTranslated;
		
		while (TRUE)
		{
			while (*szEnd != '.' && *szEnd != '\0')
				szEnd++;
			if (*szEnd == '\0')
                break;

			szEnd++;
			if (!_strnicmp(szEnd, "dll", 3) || 
				!_strnicmp(szEnd, c_AtlSRFExtension+1, 3))
			{
				szEnd += 3;
                if (!*szEnd || *szEnd == '/' || *szEnd == '\\' || *szEnd == '?' || *szEnd == '#')
				    break;
			}
		}
		
		ptrdiff_t nResult = szEnd - m_szPathTranslated;

		if (nResult >= 0 && nResult <= ( sizeof(m_szScriptPathTranslated)-1 ) )
			memcpy_s(m_szScriptPathTranslated, sizeof(m_szScriptPathTranslated), m_szPathTranslated, nResult);
		else if (nResult < 0)
		{
			ATLTRACE("Unexpected path length.\n");
			return FALSE;
		}
		else
		{
			ATLTRACE("Path translated was too long.\n");
			return FALSE;
		}

		m_szScriptPathTranslated[nResult] = '\0';
		return TRUE;
	}

	// Returns the total number of bytes to be received from the client.
	// If this value is 0xffffffff, then there are four gigabytes or more of available data.
	// In this case, ReadClient or AsyncReadClient should be called until no more data is returned.
	// Equivalent to the CONTENT_LENGTH server variable or EXTENSION_CONTROL_BLOCK::cbTotalBytes. 
	DWORD GetTotalBytes() throw()
	{
		ULONGLONG nLen = 0;
		m_InFile.GetSize(nLen);
		return (DWORD)nLen;
	}

	// Returns the number of bytes available in the request buffer accessible via GetAvailableData.
	// If GetAvailableBytes returns the same value as GetTotalBytes, the request buffer contains the whole request.
	// Otherwise, the remaining data should be read from the client using ReadClient or AsyncReadClient.
	// Equivalent to EXTENSION_CONTROL_BLOCK::cbAvailable.
	DWORD GetAvailableBytes() throw()
	{
//		LogServerError("IHttpServerContext::GetAvailableBytes not supported in command line mode.\r\n");
		return 0;
	}

	// Returns a pointer to the request buffer containing the data sent by the client.
	// The size of the buffer can be determined by calling GetAvailableBytes.
	// Equivalent to EXTENSION_CONTROL_BLOCK::lpbData
	BYTE *GetAvailableData() throw()
	{
//		LogServerError("IHttpServerContext::GetAvailableData not supported in command line mode.\r\n");
		return NULL;
	}

	// Returns a nul-terminated string that contains the content type of the data sent by the client.
	// Equivalent to the CONTENT_TYPE server variable or EXTENSION_CONTROL_BLOCK::lpszContentType.
	LPCSTR GetContentType() throw()
	{
//		LogServerError("IHttpServerContext::GetContentType not supported in command line mode.\r\n");
		return m_szContentType;
	}

	// Call this function to retrieve a nul-terminated string containing the value of the requested server variable.
	// Returns TRUE on success, and FALSE on failure. Call GetLastError to get extended error information.
	// On entry, pdwSize should point to a DWORD that indicates the size of the buffer in bytes.
	// On exit, the DWORD contains the number of bytes transferred or available to be transferred into the buffer (including the nul-terminating byte).
	// Equivalent to  EXTENSION_CONTROL_BLOCK::GetServerVariable.
	BOOL GetServerVariable(
		LPCSTR	szVariableName,
		LPSTR	pvBuffer,
		DWORD * pdwSize) throw()
	{
		if( !pdwSize )
			return FALSE;

		BOOL	bRet	=	TRUE;
			
		LPCSTR szVar;
		size_t retVal = 0;
		size_t bufSize = 0;
		
		// retrieve size of buffer
		getenv_s(&retVal, NULL, bufSize, szVariableName);

		if (retVal != 0)
		{
			char *buf = new char[retVal];

			bufSize = retVal;
			getenv_s(&retVal, buf, bufSize, szVariableName);
			szVar = buf;

			if( pvBuffer && *pdwSize  > strlen(szVar) )
				strcpy_s( pvBuffer, bufSize, szVar );
			else
			{
				::SetLastError( ERROR_INSUFFICIENT_BUFFER );
				*pdwSize	=	1 + (DWORD)strlen( szVar );
				bRet	=	FALSE;
			}
		}
		else
		{
			if( m_ErrFile.m_h != NULL )
			{
				CStringA	strErrMsg;
				Emit(strErrMsg, IDS_SERVER_VARIABLE_NOT_FOUND, szVariableName ? szVariableName : "<NULL>");
				m_ErrFile.Write(strErrMsg, strErrMsg.GetLength());
			}

			*pdwSize	=	0;
			::SetLastError( ERROR_NO_DATA);
			bRet	=	FALSE;
		}

		return bRet;


	}

	// Synchronously sends the data present in the given buffer to the client that made the request.
	// Returns TRUE on success, and FALSE on failure. Call GetLastError to get extended error information.
	// Equivalent to EXTENSION_CONTROL_BLOCK::WriteClient(..., HSE_IO_SYNC).
	BOOL WriteClient(void *pvBuffer, DWORD *pdwBytes) throw()
	{
		return (m_OutFile.Write(pvBuffer, *pdwBytes) == S_OK ? TRUE : FALSE);
		
	}

	// Asynchronously sends the data present in the given buffer to the client that made the request.
	// Returns TRUE on success, and FALSE on failure. Call GetLastError to get extended error information.
	// Equivalent to EXTENSION_CONTROL_BLOCK::WriteClient(..., HSE_IO_ASYNC).
	BOOL AsyncWriteClient(void *pvBuffer, DWORD *pdwBytes) throw()
	{
		return WriteClient(pvBuffer, pdwBytes);
	}

	// Call this function to synchronously read information from the body of the web client's HTTP request into the buffer supplied by the caller.
	// Returns TRUE on success, and FALSE on failure. Call GetLastError to get extended error information.
	// Equivalent to EXTENSION_CONTROL_BLOCK::ReadClient.
	BOOL ReadClient(void * pvBuffer, DWORD * pdwSize) throw()
	{
		ATLENSURE(pdwSize != NULL);
		DWORD dwRead = 0;
		HRESULT hr = m_InFile.Read(pvBuffer, *pdwSize, dwRead);
		*pdwSize = dwRead;
		return (hr == S_OK ? TRUE : FALSE);
	}

	// Call this function to asynchronously read information from the body of the web client's HTTP request into the buffer supplied by the caller.
	// Returns TRUE on success, and FALSE on failure. Call GetLastError to get extended error information.
	// Equivalent to the HSE_REQ_ASYNC_READ_CLIENT server support function.
	BOOL AsyncReadClient(void * pvBuffer, DWORD * pdwSize) throw()
	{
		return ReadClient(pvBuffer, pdwSize);
	}
	
	// Call this function to redirect the client to the specified URL.
	// The client receives a 302 (Found) HTTP status code.
	// Returns TRUE on success, and FALSE on failure.
	// Equivalent to the HSE_REQ_SEND_URL_REDIRECT_RESP server support function.
	BOOL SendRedirectResponse(LPCSTR /*pszRedirectURL*/) throw()
	{
		LogServerError("IHttpServerContext::SendRedirectResponse");
		return FALSE;
	}

	// Call this function to retrieve a handle to the impersonation token for this request.
	// An impersonation token represents a user context. You can use the handle in calls to ImpersonateLoggedOnUser or SetThreadToken.
	// Do not call CloseHandle on the handle.
	// Returns TRUE on success, and FALSE on failure.
	// Equivalent to the HSE_REQ_GET_IMPERSONATION_TOKEN server support function.

	BOOL GetImpersonationToken(HANDLE * /*pToken*/) throw()
	{
		LogServerError("IHttpServerContext::GetImpersonationToken");
		return FALSE;
	}

	// Call this function to send an HTTP response header to the client including the HTTP status, server version, message time, and MIME version.
	// Returns TRUE on success, and FALSE on failure.
	// Equivalent to the HSE_REQ_SEND_RESPONSE_HEADER_EX server support function.
	BOOL SendResponseHeader(
		LPCSTR /*pszHeader = "Content-Type: text/html\r\n\r\n"*/,
		LPCSTR /*pszStatusCode = "200 OK"*/,
		BOOL /*fKeepConn=FALSE*/) throw()
	{
		LogServerError("IHttpServerContext::SendResponseHeader");
		return FALSE;
	}

	// Call this function to terminate the session for the current request.
	// Returns TRUE on success, and FALSE on failure.
	// Equivalent to the HSE_REQ_DONE_WITH_SESSION server support function.
	BOOL DoneWithSession(DWORD /*dwStatusCode*/) throw()
	{
		LogServerError("IHttpServerContext::DoneWithSession");
		return FALSE;
	}

	// Call this function to set a special callback function that will be used for handling the completion of asynchronous I/O operations.
	// Returns TRUE on success, and FALSE on failure.
	// Equivalent to the HSE_REQ_IO_COMPLETION server support function.
	BOOL RequestIOCompletion(PFN_HSE_IO_COMPLETION /*pfn*/, DWORD * /*pdwContext*/) throw()
	{
		LogServerError("IHttpServerContext::RequestIOCompletion");
		return FALSE;
	}

	// Call this function to transmit a file asynchronously to the client.
	// Returns TRUE on success, and FALSE on failure.
	// Equivalent to the HSE_REQ_TRANSMIT_FILE server support function.
	BOOL TransmitFile(
		HANDLE hFile,
		PFN_HSE_IO_COMPLETION /*pfn*/,
		void * /*pContext*/,
		LPCSTR /*szStatusCode*/,
		DWORD /*dwBytesToWrite*/,
		DWORD /*dwOffset*/,
		void * /*pvHead*/,
		DWORD /*dwHeadLen*/,
		void * /*pvTail*/,
		DWORD /*dwTailLen*/,
		DWORD /*dwFlags*/) throw()
	{
		char szBuffer[1024];
		DWORD dwLen;
		OVERLAPPED overlapped;
		memset(&overlapped, 0, sizeof(OVERLAPPED));
		HANDLE hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
		if (!hEvent)
			return FALSE;
		overlapped.hEvent = hEvent;
		CHandle hdlEvent;
		hdlEvent.Attach(hEvent);
		DWORD dwErr;
		do
		{
			BOOL bRet = ::ReadFile(hFile, szBuffer, 1024, &dwLen, &overlapped);
			if (!bRet && (dwErr = GetLastError()) != ERROR_IO_PENDING && dwErr != ERROR_IO_INCOMPLETE)
				return FALSE;

			if (!GetOverlappedResult(hFile, &overlapped, &dwLen, TRUE))
				return FALSE;

			if (dwLen)
			{
				DWORD dwWritten = dwLen;
				if (!WriteClient(szBuffer, &dwWritten))
					return FALSE;
			}

		} while (dwLen != 0);

		return TRUE;
	}

    // Appends the string szMessage to the web server log for the current
    // request.
    // Returns TRUE on success, FALSE on failure.
    // Equivalent to the HSE_APPEND_LOG_PARAMETER server support function.
    BOOL AppendToLog(LPCSTR /*szMessage*/, DWORD * /*pdwLen*/) throw()
    {
		LogServerError("IHttpServerContext::AppendToLog");
		return FALSE;
    }

	BOOL MapUrlToPathEx(LPCSTR /*szLogicalPath*/, DWORD /*dwLen*/, HSE_URL_MAPEX_INFO * /*pumInfo*/)
	{
		LogServerError("IHttpServerContext::MapUrlToPathEx");
		return FALSE;
	}
}; // class CSprocServerContext


class CSProcExtension : public IServiceProvider, public IIsapiExtension
{
protected:

	typedef CStencilCache<CWorkerThread<> > stencilCacheType;
	CWorkerThread<> m_WorkerThread;
	CDllCache<CWorkerThread<>, CDllCachePeer> m_DllCache;
	CComObjectGlobal<CStencilCache<CWorkerThread<> > > m_StencilCache;
	CDefaultErrorProvider m_UserErrorProvider;

	CIsapiWorker m_Worker;

	// Dynamic services stuff
	struct ServiceNode
	{
		HINSTANCE hInst;
		IUnknown *punk;
		GUID guidService;
		IID riid;

		ServiceNode() throw()
		{
		}

		ServiceNode(const ServiceNode& that) throw()
			:hInst(that.hInst), punk(that.punk), guidService(that.guidService), riid(that.riid)
		{
		}

		const ServiceNode& operator=(const ServiceNode& that) throw()
		{
			if (this != &that)
			{
				hInst = that.hInst;
				punk = that.punk;
				memcpy_s(&guidService, sizeof(guidService), &that.guidService, sizeof(guidService));
				memcpy_s(&riid, sizeof(riid), &that.riid, sizeof(riid));
			}
			return *this;
		}
	};

	class CServiceEqualHelper
	{
	public:
		static bool IsEqual(const ServiceNode& t1, const ServiceNode& t2)
		{
			return (InlineIsEqualGUID(t1.guidService, t2.guidService) != 0);
		}
	};

	CSimpleArray<ServiceNode, CServiceEqualHelper> m_serviceMap;

public:

	CString m_strErr;
	
	CSProcExtension() throw()
	{
	}

	AtlServerRequest *CreateRequest()
	{
		AtlServerRequest *pRequest = (AtlServerRequest *) malloc(max(sizeof(AtlServerRequest), sizeof(CComObjectNoLock<CServerContext>)));
		if (!pRequest)
			return NULL;
		pRequest->cbSize = sizeof(AtlServerRequest);
		return pRequest;
	}

	void FreeRequest(AtlServerRequest *pRequest)
	{
		if (pRequest)
		{
			if (pRequest->pHandler)
				pRequest->pHandler->Release();
			if (pRequest->pServerContext)
				pRequest->pServerContext->Release();
			if (pRequest->pDllCache && pRequest->hInstDll)
				pRequest->pDllCache->ReleaseModule(pRequest->hInstDll);
		}

		//free(pRequest);
	}

	BOOL Initialize() throw()
	{
		if (!m_Worker.Initialize(static_cast<IIsapiExtension*>(this)))
			return FALSE;

		if (S_OK != m_WorkerThread.Initialize())
			return FALSE;

		if (FAILED(m_DllCache.Initialize(&m_WorkerThread, ATL_DLL_CACHE_TIMEOUT)))
		{
			m_WorkerThread.Shutdown();
			return FALSE;
		}

		if (S_OK != m_StencilCache.Initialize(static_cast<IServiceProvider*>(this), &m_WorkerThread, 
				ATL_STENCIL_CACHE_TIMEOUT, ATL_STENCIL_LIFESPAN))
		{
			m_DllCache.Uninitialize();
			m_WorkerThread.Shutdown();
			return FALSE;
		}

		return TRUE;
	}

	BOOL Uninitialize() throw()
	{
		for (int i=0; i < m_serviceMap.GetSize(); i++)
		{
			ATLASSERT(m_serviceMap[i].punk != NULL);
			m_serviceMap[i].punk->Release();
			m_DllCache.ReleaseModule(m_serviceMap[i].hInst);
		}

		m_StencilCache.Uninitialize();
		m_DllCache.Uninitialize();
		m_Worker.Terminate(static_cast<IIsapiExtension*>(this));
		m_WorkerThread.Shutdown();
		
		return TRUE;
	}

	void RequestComplete(AtlServerRequest * /*pRequestInfo*/, DWORD /*dwStatus*/, DWORD /*dwSubStatus*/) throw()
	{
	}

	HTTP_CODE GetHandlerName(LPCSTR szFileName, LPSTR szHandlerName) throw()
	{
		return _AtlGetHandlerName(szFileName, szHandlerName);
	}

	HTTP_CODE LoadDispatchFile(LPCSTR szFileName, AtlServerRequest *pRequestInfo) throw()
	{
		CStencil *pStencil = NULL;
		HCACHEITEM hStencil = NULL;
		CHAR szDllPath[MAX_PATH+1];
		CHAR szHandlerName[ATL_MAX_HANDLER_NAME_LEN+1];

		pRequestInfo->pHandler = NULL;
		pRequestInfo->hInstDll = NULL;

		USES_CONVERSION;

		m_StencilCache.LookupStencil(szFileName, &hStencil);
		if (!hStencil)
		{
			char szHandlerDllName[MAX_PATH+ATL_MAX_HANDLER_NAME_LEN+2];
			// not in the cache, so open the file
			HTTP_CODE hcErr = GetHandlerName(szFileName, szHandlerDllName);
			if (hcErr)
				return hcErr;
			DWORD dwDllPathLen = MAX_PATH+1;
			DWORD dwHandlerNameLen = ATL_MAX_HANDLER_NAME_LEN+1;
			if (!_AtlCrackHandler(szHandlerDllName, szDllPath, &dwDllPathLen, szHandlerName, &dwHandlerNameLen))
			{
				HTTP_ERROR(500, ISE_SUBERR_HANDLER_NOT_FOUND);
			}
			ATLASSERT(*szHandlerName);
			ATLASSERT(*szDllPath);
			if (!*szHandlerName)
				return HTTP_ERROR(500, ISE_SUBERR_HANDLER_NOT_FOUND);
		}
		else
		{
			m_StencilCache.GetStencil(hStencil, (void **) &pStencil);
			pStencil->GetHandlerName(szDllPath, MAX_PATH+1, szHandlerName, ATL_MAX_HANDLER_NAME_LEN+1);
			m_StencilCache.ReleaseStencil(hStencil);

			if (!*szHandlerName)
				return HTTP_ERROR(500, ISE_SUBERR_BADSRF); // bad srf file
		}
		return LoadRequestHandler(szDllPath, szHandlerName, pRequestInfo->pServerContext, &pRequestInfo->hInstDll, &pRequestInfo->pHandler);
	}


	HTTP_CODE LoadDllHandler(LPCSTR szFileName, AtlServerRequest *pRequestInfo) throw()
    {
        HTTP_CODE hcErr = HTTP_FAIL;
		CHttpRequest Request;
		BOOL bRet = Request.Initialize(pRequestInfo->pServerContext, 0);
        if (bRet)
        {
            LPCSTR szHandler = Request.QueryParams.Lookup("Handler");
            if (!szHandler)
            {
				szHandler = "Default";
            }

			hcErr = LoadRequestHandler(szFileName, szHandler, pRequestInfo->pServerContext, 
				&pRequestInfo->hInstDll, &pRequestInfo->pHandler);
        }
        else
		{
            hcErr = HTTP_ERROR(500, ISE_SUBERR_UNEXPECTED);
		}

        return hcErr;
    }


	BOOL DispatchStencilCall(AtlServerRequest * /*pRequestInfo*/) throw()
	{
		return FALSE;
	}

	virtual void ThreadTerminate(DWORD /*dwThreadId*/) { }
    virtual BOOL QueueRequest(AtlServerRequest * /*pRequestInfo*/) { return FALSE; }
	virtual CIsapiWorker *GetThreadWorker() { return &m_Worker; }
	virtual BOOL SetThreadWorker(CIsapiWorker * /*pWorker*/) { return TRUE; }
	BOOL OnThreadAttach() { return TRUE; }
	void OnThreadTerminate() {}


	BOOL DispatchStencilCall(
		LPCSTR szFile, 
		LPCSTR szOutputFile, 
		LPCSTR szQueryString, 
		LPCSTR szErrFile, 
		LPCSTR szFormFile,
		LPCSTR szContentType,
		LPCSTR szVerb)
	{
		ATLASSERT(szFile != NULL);

        CSProcServerContext ServerContext;
		HRESULT hr = ServerContext.Initialize(szOutputFile, szQueryString, szErrFile, szFormFile, szContentType, szVerb);
		if (hr != S_OK)
		{
			m_strErr = ServerContext.m_strErr;
			return FALSE;
		}

		if (!ServerContext.SetPathTranslated(szFile))
			return FALSE;

		AtlServerRequest RequestInfo;
		AtlServerRequest *pRequestInfo = &RequestInfo;
		pRequestInfo->pServerContext = static_cast<IHttpServerContext*>(&ServerContext);
		pRequestInfo->dwRequestType = ATLSRV_REQUEST_STENCIL;
		pRequestInfo->dwRequestState = ATLSRV_STATE_BEGIN;
		pRequestInfo->pExtension = static_cast<IIsapiExtension*>(this);
		pRequestInfo->pDllCache = static_cast<IDllCache *>(&m_DllCache);
		pRequestInfo->dwStartTicks = 0;

		HTTP_CODE hcErr = HTTP_SUCCESS;
		if (pRequestInfo->dwRequestState == ATLSRV_STATE_BEGIN)
        {
			LPCSTR szFileName = ServerContext.GetScriptPathTranslated();

            if (!szFileName)
			{
				m_strErr.Format("ERROR: Invalid file: %s", szFile);
				RequestComplete(pRequestInfo, 500, 8);
				return FALSE;
			}

			LPCSTR szDot = szFileName + strlen(szFileName)-4;
			if (_tcsicmp(szDot, c_AtlSRFExtension) == 0)
			{
				pRequestInfo->dwRequestType = ATLSRV_REQUEST_STENCIL;
			    hcErr = LoadDispatchFile(szFileName, pRequestInfo);
			}
			else if (_stricmp(szDot, ".dll") == 0)
		    {
                pRequestInfo->dwRequestType = ATLSRV_REQUEST_DLL;
		        hcErr = LoadDllHandler(szFileName, pRequestInfo);
            }
			else
			{
				hcErr = HTTP_FAIL;
			}

			if (hcErr)
			{
				if (hcErr == HTTP_ERROR(500, ISE_SUBERR_BADSRF))
					m_strErr.Format("ERROR: Invalid SRF file: %s", szFile);
				else if (hcErr == HTTP_ERROR(500, ISE_SUBERR_READFILEFAIL))
					m_strErr.Format("ERROR: Failed to read SRF file: %s", szFile);
				else if (hcErr == HTTP_ERROR(500, ISE_SUBERR_UNEXPECTED))
					m_strErr = "ERROR: Unexpected Error Loading DLL";
				RequestComplete(pRequestInfo, HTTP_ERROR_CODE(hcErr), HTTP_SUBERROR_CODE(hcErr));
				return FALSE;
			}

			// initialize the handler
			DWORD dwStatus = 0;

			hcErr = pRequestInfo->pHandler->GetFlags(&dwStatus);
			// Ignoring caching options
			// Log errors on async
			if (dwStatus & ATLSRV_INIT_USEASYNC)
			{
				// TODO: Log error, or come up with a way to make it work
				RequestComplete(pRequestInfo, 500, SUBERR_NONE);
				return FALSE;
			}

			hcErr = pRequestInfo->pHandler->InitializeHandler(pRequestInfo, static_cast<IServiceProvider*>(this));
			if (hcErr == HTTP_SUCCESS)
				hcErr = pRequestInfo->pHandler->HandleRequest(pRequestInfo, static_cast<IServiceProvider*>(this));
		}

        if (hcErr == HTTP_SUCCESS_ASYNC || hcErr == HTTP_SUCCESS_ASYNC_DONE)
		{
			return FALSE;
		}
        else
		{
			pRequestInfo->pHandler->UninitializeHandler();
		    RequestComplete(pRequestInfo, HTTP_ERROR_CODE(hcErr), HTTP_SUBERROR_CODE(hcErr));
		}

		FreeRequest(pRequestInfo);
		return TRUE;
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv) throw()
	{
		if (!ppv)
			return E_POINTER;
		if (InlineIsEqualGUID(riid, __uuidof(IUnknown)) ||
			InlineIsEqualGUID(riid, __uuidof(IServiceProvider)))
		{
			*ppv = static_cast<IUnknown*>(static_cast<IServiceProvider*>(this));
			AddRef();
			return S_OK;
		}
		if (InlineIsEqualGUID(riid, __uuidof(IIsapiExtension)))
		{
			*ppv = static_cast<IUnknown*>(static_cast<IIsapiExtension*>(this));
			AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	virtual HRESULT STDMETHODCALLTYPE QueryService(
		REFGUID guidService,
		REFIID riid,
		void **ppvObject) throw()
	{
		if (!ppvObject)
			return E_POINTER;

		if (InlineIsEqualGUID(guidService, __uuidof(IDllCache)))
			return m_DllCache.QueryInterface(riid, ppvObject);
		else if (InlineIsEqualGUID(guidService, __uuidof(IStencilCache)))
			return m_StencilCache.QueryInterface(riid, ppvObject);

		return E_NOINTERFACE;
	}


	virtual HRESULT AddService(REFGUID guidService, REFIID riid, IUnknown *punk, HINSTANCE hInstance) throw()
	{
		if (!m_DllCache.AddRefModule(hInstance))
			return E_FAIL;
		
		ServiceNode srvNode;
		srvNode.hInst = hInstance;
		srvNode.punk = punk;
		memcpy_s(&srvNode.guidService, sizeof(guidService), &guidService, sizeof(guidService));
		memcpy_s(&srvNode.riid, sizeof(guidService), &riid, sizeof(riid));
		
		// if the service is already there, return S_FALSE
		int nIndex = m_serviceMap.Find(srvNode);
		if (nIndex >= 0)
			return S_FALSE;

		if (!m_serviceMap.Add(srvNode))
			return E_OUTOFMEMORY;

		punk->AddRef();
		return S_OK;
	}

	virtual HRESULT RemoveService(REFGUID guidService, REFIID riid) throw()
	{	
		ServiceNode srvNode;
		memcpy_s(&srvNode.guidService, sizeof(guidService), &guidService, sizeof(guidService));
		memcpy_s(&srvNode.riid, sizeof(riid), &riid, sizeof(riid));
		int nIndex = m_serviceMap.Find(srvNode);
		if (nIndex < 0)
			return S_FALSE;

		ATLASSERT(m_serviceMap[nIndex].punk != NULL);
		m_serviceMap[nIndex].punk->Release();

		HINSTANCE hInstRemove = m_serviceMap[nIndex].hInst;

		m_serviceMap.RemoveAt(nIndex);

		if (!m_DllCache.ReleaseModule(hInstRemove))
			return S_FALSE;

		return S_OK;
	}

	HRESULT GetService(REFGUID guidService, REFIID riid, void **ppvObject) throw()
	{
		if (!ppvObject)
			return E_POINTER;

		ServiceNode srvNode;
		memcpy_s(&srvNode.guidService, sizeof(guidService), &guidService, sizeof(guidService));
		memcpy_s(&srvNode.riid, sizeof(guidService), &riid, sizeof(riid));
		
		int nIndex = m_serviceMap.Find(srvNode);
		if (nIndex < 0)
			return E_NOINTERFACE;

		ATLASSERT(m_serviceMap[nIndex].punk != NULL);
		return m_serviceMap[nIndex].punk->QueryInterface(riid, ppvObject);
	}

	ULONG STDMETHODCALLTYPE AddRef()
	{
		return 1;
	}
	
	ULONG STDMETHODCALLTYPE Release()
	{
		return 1;
	}

	HTTP_CODE LoadRequestHandler(LPCSTR szDllPath, LPCSTR szHandlerName, IHttpServerContext *pServerContext,
		HINSTANCE *phInstance, IRequestHandler **ppHandler) throw()
	{
		return _AtlLoadRequestHandler(szDllPath, szHandlerName, pServerContext, phInstance, 
			ppHandler, this, static_cast<IDllCache*>(&m_DllCache));
	} // LoadRequestHandler


	HTTP_CODE TransferRequest(
		AtlServerRequest *pRequest, 
		IServiceProvider *pServiceProvider,
		IWriteStream *pWriteStream,
		IHttpRequestLookup *pLookup,
		LPCSTR szNewUrl,
		WORD nCodePage,
		bool bContinueAfterProcess,
		CStencilState *pState)
	{
		return _AtlTransferRequest(pRequest, pServiceProvider, pWriteStream,
			pLookup, szNewUrl, nCodePage, bContinueAfterProcess, pState);
	}
};
