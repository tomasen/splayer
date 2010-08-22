//
// sproxy.cpp : Defines the entry point for the console application.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#include "stdafx.h"
#include "WSDLParser.h"
#include "DiscoMapParser.h"
#include "ErrorHandler.h"
#include "WSDLDocument.h"
#include "DiscoMapDocument.h"
#include "CodeTypeBuilder.h"
#include "CppCodeGenerator.h"
#include "Emit.h"
#include <atlbuild.h>
#include "resource.h"
//////////////////////////////////////////////////////////////////////
//Resource (satellite) dll search and load routines. see LoadLocResDll
//an example.
//Note: Change made here must be sync with all other tools.

__inline
errno_t __cdecl DuplicateEnvString(TCHAR **ppszBuffer, size_t *pnBufferSizeInTChars, const TCHAR *pszVarName)
{	    
    /* validation section */
	if (ppszBuffer == NULL) { return  EINVAL; }
    *ppszBuffer = NULL;
    if (pnBufferSizeInTChars != NULL)
    {
        *pnBufferSizeInTChars = 0;
    }
    /* varname is already validated in getenv */
	TCHAR szDummyBuff[1] = {0};
	size_t nSizeNeeded = 0;
    errno_t ret=_tgetenv_s(&nSizeNeeded,szDummyBuff,1,pszVarName);
	if (nSizeNeeded > 0)
	{	    		
		*ppszBuffer = new TCHAR[nSizeNeeded];
		if (*ppszBuffer != NULL)
		{
			size_t nSizeNeeded2 = 0;
			ret=_tgetenv_s(&nSizeNeeded2,*ppszBuffer,nSizeNeeded,pszVarName);
			if (nSizeNeeded2!=nSizeNeeded)
			{
				ret=ERANGE;
			} else if (pnBufferSizeInTChars != NULL)
			{
				*pnBufferSizeInTChars = nSizeNeeded;
			}				
		} else
		{
			ret=ENOMEM;
		}	    		
	}
    return ret;
}

#define _TCSNLEN(sz,c) (min(_tcslen(sz), c))
#define PATHLEFT(sz) (_MAX_PATH - _TCSNLEN(sz, (_MAX_PATH-1)) - 1)

typedef LANGID (WINAPI* PFNGETUSERDEFAULTUILANGUAGE)();

static BOOL CALLBACK _EnumResLangProc(HMODULE /*hModule*/, LPCTSTR /*pszType*/, 
	LPCTSTR /*pszName*/, WORD langid, LONG_PTR lParam)
{
	if(lParam == NULL)
	{
		return FALSE;
	}
		
	LANGID* plangid = reinterpret_cast< LANGID* >( lParam );
	*plangid = langid;

	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
//Purpose: GetUserDefaultUILanguage for downlevel platforms (Win9x, NT4).
//Input: szDllName - the string resource dll name to search. Ex: ToolUI.dll 
//Output: TCHAR *szPathOut - filled with absolute path to dll, if found.
//		  size_t sizeInCharacters - buffer size in characters
//Returns: Success - HMODULE of found dll, Failure - NULL
//////////////////////////////////////////////////////////////////////////
HRESULT GetUserDefaultUILanguageLegacyCompat(LANGID* pLangid)
{
	HRESULT hr=E_FAIL;	
	if (pLangid == NULL) { return E_POINTER; }
	PFNGETUSERDEFAULTUILANGUAGE pfnGetUserDefaultUILanguage;	
	HINSTANCE hKernel32 = ::GetModuleHandle(_T("kernel32.dll"));
	pfnGetUserDefaultUILanguage = (PFNGETUSERDEFAULTUILANGUAGE)::GetProcAddress(hKernel32, "GetUserDefaultUILanguage");
	if(pfnGetUserDefaultUILanguage != NULL)
	{
		*pLangid = pfnGetUserDefaultUILanguage();
		hr = S_OK;
	} else
	{
		// We're not on an MUI-capable system.
		OSVERSIONINFO version;
		memset(&version, 0, sizeof(version));
		version.dwOSVersionInfoSize = sizeof(version);
		::GetVersionEx(&version);
		if( version.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		{
			// We're on Windows 9x, so look in the registry for the UI language
			HKEY hKey = NULL;
			LONG nResult = ::RegOpenKeyEx(HKEY_CURRENT_USER, 
				_T( "Control Panel\\Desktop\\ResourceLocale" ), 0, KEY_READ, &hKey);
			if (nResult == ERROR_SUCCESS)
			{
				DWORD dwType;
				TCHAR szValue[16];
				ULONG nBytes = sizeof( szValue );
				nResult = ::RegQueryValueEx(hKey, NULL, NULL, &dwType, LPBYTE( szValue ), 
					&nBytes );
				if ((nResult == ERROR_SUCCESS) && (dwType == REG_SZ))
				{
					DWORD dwLangID;
					int nFields = _stscanf_s( szValue, _T( "%x" ), &dwLangID );
					if( nFields == 1 )
					{
						*pLangid = LANGID( dwLangID );
						hr = S_OK;
					}
				}

				::RegCloseKey(hKey);
			}
		}
		else
		{
			// We're on NT 4.  The UI language is the same as the language of the version
			// resource in ntdll.dll
			HMODULE hNTDLL = ::GetModuleHandle( _T( "ntdll.dll" ) );
			if (hNTDLL != NULL)
			{
				*pLangid = 0;
				::EnumResourceLanguages( hNTDLL, RT_VERSION, MAKEINTRESOURCE( 1 ), 
					_EnumResLangProc, reinterpret_cast< LONG_PTR >( pLangid ) );
				if (*pLangid != 0)
				{
					hr = S_OK;
				}
			}
		}
	}
	return hr;
}

//////////////////////////////////////////////////////////////////////////
//Purpose: Searches for a resource dll in sub directories using a search order
//		   based on szPath - a directory to search res dll below.
//		   see example at .
//Input: szDllName - the string resource dll name to search. Ex: ToolUI.dll 
//Output: TCHAR *szPathOut - filled with absolute path to dll, if found.
//		  size_t sizeInCharacters - buffer size in characters
//Returns: Success (found dll) - S_OK , Failure - E_FAIL or E_UNEXPECTED
//////////////////////////////////////////////////////////////////////////
HRESULT LoadUILibrary(LPCTSTR szPath, LPCTSTR szDllName, DWORD dwExFlags, 
                      HINSTANCE *phinstOut, LPTSTR szFullPathOut,size_t sizeInCharacters,
                      LCID *plcidOut)
{
    TCHAR szPathTemp[_MAX_PATH + 1] = _T("");
    HRESULT hr = E_FAIL;
    LCID lcidFound = (LCID)-1;
    size_t nPathEnd = 0;

    // Gotta have this stuff!
	if (szPath==NULL || *szPath == '\0')	   
	{ 
		return E_POINTER; 
	}

    if (szDllName==NULL || *szDllName == '\0') 
	{ 
		return E_POINTER; 
	}

    if (!szPath || !*szPath || !szDllName || !*szDllName)
	{
        return E_INVALIDARG;
	}

    if (phinstOut != NULL)
    {
        *phinstOut = NULL;
    }

    szPathTemp[_MAX_PATH-1] = L'\0';

    // Add \ to the end if necessary
    _tcsncpy_s(szPathTemp,_countof(szPathTemp), szPath, _MAX_PATH-1);
    if (szPathTemp[_TCSNLEN(szPathTemp, _MAX_PATH-1) - 1] != L'\\')
    {
        _tcsncat_s(szPathTemp,_countof(szPathTemp), _T("\\"), PATHLEFT(szPathTemp));
    }

    // Check if given path even exists
    if (GetFileAttributes(szPathTemp) == 0xFFFFFFFF)
	{
        return E_FAIL;
	}

    nPathEnd = _TCSNLEN(szPathTemp, _MAX_PATH-1);
    
    {	        
		LANGID langid=0;
		if (FAILED(GetUserDefaultUILanguageLegacyCompat(&langid)))
		{
			return E_UNEXPECTED;
		}
        const LCID lcidUser = MAKELCID(langid, SORT_DEFAULT);
        
        LCID rglcid[3];
        rglcid[0] = lcidUser;
        rglcid[1] = MAKELCID(MAKELANGID(PRIMARYLANGID(lcidUser), SUBLANG_DEFAULT), SORTIDFROMLCID(lcidUser));
        rglcid[2] = 0x409;
        for (int i = 0; i < _countof(rglcid); i++)
        {
            TCHAR szNumBuf[10];
            
            // Check if it's the same as any LCID already checked,
            // which is very possible
			int n = 0;
            for (n = 0; n < i; n++)
            {
                if (rglcid[n] == rglcid[i])
                    break;
            }

            if (n < i)
			{
                continue;
			}
            
            szPathTemp[nPathEnd] = L'\0';
			_itot_s(rglcid[i], szNumBuf,_countof(szNumBuf), 10);
            _tcsncat_s(szPathTemp, _countof(szPathTemp),szNumBuf , PATHLEFT(szPathTemp));
            _tcsncat_s(szPathTemp,_countof(szPathTemp), _T("\\"), PATHLEFT(szPathTemp));
            _tcsncat_s(szPathTemp,_countof(szPathTemp), szDllName, PATHLEFT(szPathTemp));

            if (GetFileAttributes(szPathTemp) != 0xFFFFFFFF)
            {
                lcidFound = rglcid[i];

                hr = S_OK;
                goto Done;
            }
        }
    }

    // None of the default choices exists, so now look for the dll in a folder below
	//the given path (szPath)
    {        
        szPathTemp[nPathEnd] = L'\0';
        _tcsncat_s(szPathTemp,_countof(szPathTemp), _T("*.*"), PATHLEFT(szPathTemp));

		WIN32_FIND_DATA wfdw;
        HANDLE hDirs = FindFirstFile(szPathTemp, &wfdw);
        nPathEnd = _TCSNLEN(szPathTemp, _MAX_PATH-1)-3;
        if (hDirs != INVALID_HANDLE_VALUE)
        {
            while (FindNextFile(hDirs, &wfdw))
            {
                // We are only interested in directories, since at this level, that should
                // be the only thing in this directory, i.e, LCID sub dirs
                if (wfdw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    // Skip current and previous dirs, "." and ".."
                    if (!_tcscmp(wfdw.cFileName, _T(".")) || !_tcscmp(wfdw.cFileName, _T("..")))
                        continue;

                    // Does this dir have a copy of the dll?
                    szPathTemp[nPathEnd] = L'\0';
                    _tcsncat_s(szPathTemp,_countof(szPathTemp), wfdw.cFileName, PATHLEFT(szPathTemp));
                    _tcsncat_s(szPathTemp,_countof(szPathTemp), _T("\\"), PATHLEFT(szPathTemp));
                    _tcsncat_s(szPathTemp,_countof(szPathTemp), szDllName, PATHLEFT(szPathTemp));

                    if (GetFileAttributes(szPathTemp) != 0xFFFFFFFF)
                    {
                        // Got it!
                        lcidFound = (LCID)_tstol(wfdw.cFileName);

                        hr = S_OK;
                        break;
                    }
                }
            }

            FindClose(hDirs);
        }
    }

Done:
    if (SUCCEEDED(hr))
    {
        // Set the default LCID
        if (plcidOut)
        {
			if (lcidFound == (LCID)-1) 
			{ 
				return E_UNEXPECTED; 
			}
            *plcidOut = lcidFound;
        }

        // Finally, attempt to load the library
        // Beware!  A dll loaded with LOAD_LIBRARY_AS_DATAFILE won't
        // let you use LoadIcon and things like that (only general calls like
        // FindResource and LoadResource).
        if (phinstOut != NULL)
        {
            *phinstOut = LoadLibraryEx(szPathTemp, NULL, dwExFlags);
            hr = (*phinstOut) ? S_OK : E_FAIL;
        }
        if ( szFullPathOut )
		{
            _tcsncpy_s(szFullPathOut,sizeInCharacters, szPathTemp, _MAX_PATH-1);
		}
    }
 
    return hr;
}
//////////////////////////////////////////////////////////////////////////
//Purpose: Iterates env("PATH") directories to try to find (using LoadUILibrary)
//		   resource dll a directory below PATH dirs. Ex: if PATH="c:\bin;d:\win"
//		   and szDllName="ToolUI.dll", then the first of c:\bin\1033\ToolUI.dll 
//		   and d:\win\SomeFolder\ToolUI.dll will be loaded.
//		   See LoadLocResDll doc (below) for example.
//Input: szDllName - the string resource dll name to search. Ex: ToolUI.dll 
//Output: TCHAR *szPathOut - filled with absolute path to dll, if found.
//		  size_t sizeInCharacters - buffer size in characters
//Returns: Success - HMODULE of found dll, Failure - NULL
//////////////////////////////////////////////////////////////////////////
HMODULE LoadSearchPath(LPCTSTR szDllName,TCHAR *szPathOut, size_t sizeInCharacters)
{
    TCHAR * szEnvPATH = NULL;
	TCHAR * szEnvPATHBuff = NULL;    
    int nPathLen = 0;
    int nPathIndex = 0;
    HMODULE hmod = NULL;	
    if (DuplicateEnvString(&szEnvPATHBuff,NULL,_T("PATH"))==0 && (szEnvPATH=szEnvPATHBuff) != NULL) 
	{
        while (*szEnvPATH) 
		{
            /* skip leading white space and nop semicolons */
            for (; *szEnvPATH == L' ' || *szEnvPATH == L';'; ++szEnvPATH)
            {} /* NOTHING */

            if (*szEnvPATH == L'\0')
			{
                break;
			}

            ++nPathIndex;

            /* copy this chunk of the path into our trypath */
            nPathLen = 0;
			TCHAR szPath[_MAX_PATH+1];
			TCHAR * pszTry = NULL;
            for (pszTry = szPath; *szEnvPATH != L'\0' && *szEnvPATH != L';'; ++szEnvPATH) 
			{
				++nPathLen;
                if (nPathLen < _MAX_PATH) 
				{                    
                    *pszTry++ = *szEnvPATH;
                } else 
				{
                    break;
                }
            }
            *pszTry = L'\0';

            if (nPathLen == 0 || nPathLen >= _MAX_PATH)
			{
                continue;
			}

            LoadUILibrary(szPath, szDllName, LOAD_LIBRARY_AS_DATAFILE, 
                          &hmod, szPathOut,sizeInCharacters, NULL);
            if ( hmod )
			{
                break;
			}
        }
    }
	if (szEnvPATHBuff!=NULL)
	{
		delete [] szEnvPATHBuff;
	}
    return hmod;
}
//Example: Say PATH="c:\bin;d:\win", resource dll name (szDllName) is "ToolUI.dll",
//		   user locale is 936, and the .exe calling LoadLocResDll is c:\MyTools\Tool.exe
//			Search order:
//			a) c:\MyTools\936\ToolUI.dll (exe path + user default UI lang)			   
//			b) c:\MyTools\1033 (same with eng)
//			c) c:\MyTools\*\ToolUI.dll (where * is sub folder).
//			d) c:\bin\936\ToolUI.dll (first in path)
//			e) c:\bin\1033\ToolUI.dll (first in path + eng)
//			f) c:\bin\*\ToolUI.dll
//			g) d:\win\936\ToolUI.dll  (second in path)
//			h) d:\win\1033\ToolUI.dll (second in path + eng)
//			i) d:\win\*\ToolUI.dll (second in path + eng)
//			j) if bExeDefaultModule and not found, return exe HINSTANCE.
//			Note: The primary lang (without the sublang) is tested after the user ui lang.
// Main Input: szDllName - the name of the resource dll <ToolName>ui.dll. Ex: vcdeployUI.dll
// Main Output: HMODULE of resource dll or NULL - if not found (see bExeDefaultModule).
HMODULE LoadLocResDll(LPCTSTR szDllName,BOOL bExeDefaultModule=TRUE,DWORD dwExFlags=LOAD_LIBRARY_AS_DATAFILE,LPTSTR pszPathOut = NULL,size_t sizeInCharacters = 0  )
{
    HMODULE hmod = NULL;
    TCHAR driverpath[_MAX_PATH + 1], exepath[_MAX_PATH + 1];
    LPTSTR p = NULL;
    
    GetModuleFileName(GetModuleHandle(NULL), driverpath, _MAX_PATH);
    // find path of tool
    p = driverpath + _TCSNLEN(driverpath, _MAX_PATH-1)-1;
    while ( *p != L'\\' && p != driverpath)
	{
        p--;
	}
    *p = '\0';

    LoadUILibrary(driverpath, szDllName, dwExFlags, 
                  &hmod, exepath,_countof(exepath), NULL);

    if ( hmod == NULL ) 
	{
        // search PATH\<lcid> for <ToolName>ui.dll
        hmod = LoadSearchPath(szDllName,exepath,_countof(exepath));
    }

    if ( hmod && pszPathOut )
	{
        _tcsncpy_s(pszPathOut,sizeInCharacters, exepath, _MAX_PATH-1);
	}
	//Not found dll, return the exe HINSTANCE as a fallback.
	if (hmod == NULL && bExeDefaultModule)
	{
		hmod=GetModuleHandle(NULL);
	}
    return hmod;
}
//End loc routines
////////////////////////////////////////////////////////////////////
const TCHAR* szSproxyUIDll=_T("sproxyUI.dll");

#define E_SAX_LOADFAILED   0x800C0006
#define E_SAX_FILENOTFOUND 0x80070002 // AtlHresultFromWin32(ERROR_FILE_NOT_FOUND)
#define E_SAX_PATHNOTFOUND 0x80070003 // AtlHresultFromWin32(ERROR_PATH_NOT_FOUND)
#define E_SAX_ACCESSDENIED 0x80070005 // AtlHresultFromWin32(ERROR_ACCESS_DENIED)

#ifdef _DEBUG

CDebugReportHook g_Hook;
#define DUMP_LEAKS() _CrtDumpMemoryLeaks( ), _CrtSetDbgFlag((_CRTDBG_LEAK_CHECK_DF) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))

#else

#define DUMP_LEAKS()

#endif // _DEBUG

const DWORD SPROXYFLAG_SETOUT          = 0x00000001;
const DWORD SPROXYFLAG_NOWARN          = 0x00000002;
const DWORD SPROXYFLAG_NOLOGO          = 0x00000004;
const DWORD SPROXYFLAG_USAGE           = 0x00000008;
const DWORD SPROXYFLAG_NOPRAGMA        = 0x00000010;
const DWORD SPROXYFLAG_NOCLOBBER       = 0x00000020;
const DWORD SPROXYFLAG_SETNAMESPACE    = 0x00000040;
const DWORD SPROXYFLAG_NONAMESPACE     = 0x00000080;
const DWORD SPROXYFLAG_NOPROXY         = 0x00000100;
const DWORD SPROXYFLAG_WSDLINPUT       = 0x00000200;
const DWORD SPROXYFLAG_NOWCHAR_T       = 0x00000400;

int sproxy();
void PrintHeader();
int PrintUsage(bool bFull = false);
int PrintGenerateFailure(const wchar_t *wszOut);
void PreProcessCommandLine(int argc, wchar_t *argvW[], CStringW & wszOut, CStringA & szNamespace, unsigned __int64 *pnFlags, LPBOOL pfHasInvalid);
void ParseCommandLine(int argc, wchar_t *argvW[], CStringW & wszUrl, BOOL bHasInvalid);

CStringW g_wszFile ;
CDiscoMapDocument * g_pDMDoc ;
bool g_bUseWchar_t ;

int main()
{	
	HINSTANCE hInst=LoadLocResDll(szSproxyUIDll);	
	_AtlBaseModule.SetResourceInstance(hInst);
	int nRet = 0;
	
	__try
	{
		nRet = sproxy();
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		printf("\r\nsproxy : ISE error SDL0000 : Internal Sproxy Error (most likely out of memory). Please contact technical support.\r\n");
		return 1;
	}

	DUMP_LEAKS();

	return nRet;
}

int sproxy()
{
	unsigned __int64 nFlags;
	BOOL bHasInvalid;
	CStringW wszUrl;
	CStringW wszOut;
	CStringA szNamespace;
	wchar_t **argvW = NULL ;
	int argc = 0 ;

	argvW = CommandLineToArgvW(GetCommandLineW(),&argc);
	if(argvW == NULL)
	{
		EmitError(IDS_SDL_CMDLINE_FAILURE, GetLastError());
		return 1;
	}
	PreProcessCommandLine(argc, argvW, wszOut, szNamespace, &nFlags, &bHasInvalid);
	
	g_bUseWchar_t = ((nFlags & SPROXYFLAG_NOWCHAR_T) == 0);

	if ((nFlags & SPROXYFLAG_NOLOGO) == 0)
	{
		PrintHeader();
	}
	
	if (nFlags & SPROXYFLAG_USAGE)
	{
		PrintUsage(true);
		GlobalFree(argvW);
		return 0;
	}

	ParseCommandLine(argc, argvW, wszUrl, bHasInvalid);

	GlobalFree(argvW);	

	bHasInvalid = FALSE;
	if (wszUrl.IsEmpty())
	{
		EmitCmdLineError(IDS_SDL_MISSING_OPTION, nFlags & SPROXYFLAG_WSDLINPUT ?"<wsdl_location>":"<discomap_location>");
		bHasInvalid = TRUE;
	}

	if (bHasInvalid != FALSE)
	{
		printf("\r\n");
		PrintUsage();
		return 1;
	}

	if (nFlags & SPROXYFLAG_NOWARN)
	{
		SetEmitWarnings(false);
	}

	CHeapPtr<char> spSzNamespace;

	if (szNamespace && *szNamespace)
	{
		if (FAILED(CreateSafeCppName(&spSzNamespace, szNamespace)))
		{
			EmitErrorHr(E_OUTOFMEMORY);
			return 1;
		}
	}

	CoInitialize(NULL);
	HRESULT hr = S_OK;
	{
		CAutoPtr<CDiscoMapParser> dmParser;
		if(!(nFlags & SPROXYFLAG_WSDLINPUT))
		{
			CComPtr<ISAXXMLReader> spDMReader;
			hr = CoCreateInstance(__uuidof(SAXXMLReader30), NULL, CLSCTX_ALL,
				__uuidof(ISAXXMLReader), (void **)&spDMReader);

			if (FAILED(hr))
			{
				ATLTRACE( _T("CoCreateInstance failed!\n") );
				return 1;
			}

			dmParser.Attach(new CDiscoMapParser(spDMReader, NULL, 0));
			if(dmParser == NULL)
			{
				ATLTRACE(_T("Failed to create Discomap Parser : out of memory!\n"));
				return 1;
			}
			dmParser->SetDynamicAlloc(FALSE);
			hr = spDMReader->putContentHandler( dmParser );
			if (FAILED(hr))
			{
				ATLTRACE( _T("putContentHandler failed!\n") );
				return 1;
			}
			
			CErrorHandler errDM;
			errDM.SetLocation(wszUrl);

			hr = spDMReader->putErrorHandler( &errDM);
			if (FAILED(hr))
			{
				ATLTRACE( _T("putErrorHandler failed!\n") );
				return 1;
			}

			g_pDMDoc = dmParser->GetDiscoMapDocument();
			if (g_pDMDoc == NULL)
			{
				ATLTRACE( _T("failed to create document: out of memory!\n") );
				return 1;
			}
			

			hr = g_pDMDoc->SetDocumentUri( wszUrl, wszUrl.GetLength());

			if (FAILED(hr))
			{
				ATLTRACE( _T("failed to set document uri: out of memory!\n") );
				return 1;
			}

			g_wszFile = wszUrl;

			hr = spDMReader->parseURL( wszUrl );
			if (FAILED(hr))
			{
				ATLTRACE( _T("parseURL failed!\n") );
				if ((hr == E_SAX_LOADFAILED) || 
					(hr == E_SAX_FILENOTFOUND) ||
					(hr == E_SAX_PATHNOTFOUND) ||
					(hr == E_SAX_ACCESSDENIED))
				{
					EmitError(IDS_SDL_FAILED_DM_OPEN, wszUrl);
				}
				EmitError(IDS_SDL_PROCESS_DM_FAILURE, wszUrl);
				return 1;
			}
		}

		if(nFlags & SPROXYFLAG_WSDLINPUT)
			g_wszFile = wszUrl;
		else
			g_wszFile = g_pDMDoc->GetWSDLFile();

		CComPtr<ISAXXMLReader> spReader;
		hr = CoCreateInstance(__uuidof(SAXXMLReader30), NULL, CLSCTX_ALL,
			__uuidof(ISAXXMLReader), (void **)&spReader);

		if (FAILED(hr))
		{
			ATLTRACE( _T("CoCreateInstance failed!\n") );
			return 1;
		}

		CWSDLParser parser(spReader, NULL, 0);
		parser.SetDynamicAlloc(FALSE);
		hr = spReader->putContentHandler( &parser );
		if (FAILED(hr))
		{
			ATLTRACE( _T("putContentHandler failed!\n") );
			return 1;
		}

		CErrorHandler err;
		err.SetLocation(g_wszFile);

		hr = spReader->putErrorHandler( &err );
		if (FAILED(hr))
		{
			ATLTRACE( _T("putErrorHandler failed!\n") );
			return 1;
		}

		CWSDLDocument * pDoc = parser.GetWSDLDocument();
		if (pDoc == NULL)
		{
			ATLTRACE( _T("failed to create document: out of memory!\n") );
			return 1;
		}
		
		hr = pDoc->SetDocumentUri( g_wszFile, g_wszFile.GetLength());
		if (FAILED(hr))
		{
			ATLTRACE( _T("failed to set document uri: out of memory!\n") );
			return 1;
		}

		wchar_t wszTmp[ATL_URL_MAX_URL_LENGTH];
		if(g_wszFile.Find(L"://") != -1 && ((g_wszFile.Left(5)).MakeLower() != L"file:") )
		{
			// The URL needs to be escaped only if it's not a local file
			if(AtlEscapeUrl(g_wszFile,wszTmp,0,ATL_URL_MAX_URL_LENGTH-1,ATL_URL_BROWSER_MODE) == FALSE)
			{
				ATLTRACE( _T("failed to escape uri!\n") );
				return 1;
			}

			hr = spReader->parseURL( wszTmp );
		}
		else
		{
			hr = spReader->parseURL( g_wszFile );
		}
		
		if (FAILED(hr))
		{
			ATLTRACE( _T("parseURL failed!\n") );
			if ((hr == E_SAX_LOADFAILED) || 
			    (hr == E_SAX_FILENOTFOUND) ||
			    (hr == E_SAX_PATHNOTFOUND) ||
			    (hr == E_SAX_ACCESSDENIED))
			{
				EmitError(IDS_SDL_FAILED_WSDL_OPEN, g_wszFile);
			}
			EmitError(IDS_SDL_PROCESS_FAILURE, g_wszFile);
			return 1;
		}

		CCodeTypeBuilder builder;
		CCodeProxy proxy;
		hr = builder.Initialize(parser.GetWSDLDocument(), &proxy);

		if (FAILED(hr))
		{
			ATLTRACE( _T("builder.Initialize failed!\n") );
			return PrintGenerateFailure(wszOut);
		}

		hr = builder.Build();
		if (FAILED(hr))
		{
			ATLTRACE( _T("builder.Build failed!\n") );
			return PrintGenerateFailure(wszOut);
		}

		if (wszOut.IsEmpty())
		{
			wszOut.Preallocate(proxy.GetServiceName().GetLength()+4);
			wszOut = (LPCSTR)proxy.GetServiceName();
			wszOut.Append(L".h", 2);
		}

		Emit(IDS_SDL_SUCCESS, wszOut);
		CComObjectStack<CCppCodeGenerator> gen;
		hr = gen.Generate(wszOut, &proxy, 
				(nFlags & SPROXYFLAG_NOPRAGMA) ? false : true, 
				(nFlags & SPROXYFLAG_NOCLOBBER) ? true : false,
				(nFlags & SPROXYFLAG_NONAMESPACE) ? false : true,
				(nFlags & SPROXYFLAG_NOPROXY) ? false : true,
				spSzNamespace ? spSzNamespace : (szNamespace.IsEmpty() ? 0 : LPCSTR(szNamespace)));
		if (FAILED(hr))
		{
			ATLTRACE( _T("gen.Generate failed!\n") );
			return PrintGenerateFailure(wszOut);
		}
	}
	CoUninitialize();

	return 0;
}

void PrintHeader()
{
	Emit(IDS_SDL_HEADER);
}

int PrintUsage(bool bFull)
{
	Emit(IDS_SDL_USAGE);
	if (bFull != false)
	{
		Emit(IDS_SDL_USAGE_EX);
	}
	return 1;
}

int PrintGenerateFailure(const wchar_t *szOut)
{
	EmitError(IDS_SDL_GENERATE_FAILURE, szOut ? szOut : L"<unspecified>");
	return 1;
}

const wchar_t * GetWSDLFile()
{
	return g_wszFile;
}

ATL_NOINLINE DWORD GetOption(wchar_t *wszCmd)
{
	struct _cmd_item
	{
		const wchar_t *wszOption;
		DWORD dwFlags;
	};

	const static _cmd_item s_options[] =
	{
		{ L"/nologo",          SPROXYFLAG_NOLOGO },
		{ L"/nowarn",          SPROXYFLAG_NOWARN },
		{ L"/?",               SPROXYFLAG_USAGE },
		{ L"/nopragma",        SPROXYFLAG_NOPRAGMA },
		{ L"/noclobber",       SPROXYFLAG_NOCLOBBER },
		{ L"/help",            SPROXYFLAG_USAGE },
		{ L"/nonamespace",     SPROXYFLAG_NONAMESPACE },
		{ L"/nowchar_t",       SPROXYFLAG_NOWCHAR_T },
		{ L"/wsdl",            SPROXYFLAG_WSDLINPUT},
		{ L"/noproxy",         SPROXYFLAG_NOPROXY }				
	};

	for (int j=0; j<(sizeof(s_options)/sizeof(s_options[0])); j++)
	{
		if (!_wcsicmp(s_options[j].wszOption, wszCmd))
		{
			return s_options[j].dwFlags;
		}
	}

	if (!_wcsnicmp(wszCmd, L"/out:", (sizeof(L"/out:")/2) -1))
	{
		return SPROXYFLAG_SETOUT;
	}
	if (!_wcsnicmp(wszCmd, L"/namespace:", (sizeof(L"/namespace:")/2) -1))
	{
		return SPROXYFLAG_SETNAMESPACE;
	}

	return 0;
}

ATL_NOINLINE void PreProcessCommandLine(int argc, wchar_t *argvW[], CStringW & wszOut, CStringA & szNamespace, unsigned __int64 *pnFlags, LPBOOL pfHasInvalid)
{
	*pfHasInvalid = FALSE;
	*pnFlags = 0;

	unsigned __int64 nFlags = 0;
	for (int i=1; i<argc; i++)
	{
		if (argvW[i][0] == L'/')
		{
			DWORD dwOpt = GetOption(argvW[i]);
			if (dwOpt != 0)
			{
				if (dwOpt == SPROXYFLAG_SETOUT)
				{
					wszOut = (argvW[i]+(sizeof(L"/out:")/2)) -1;
				}
				else if (dwOpt == SPROXYFLAG_SETNAMESPACE)
				{
					szNamespace = (argvW[i]+sizeof(L"/namespace:")/2)-1;
				}
				nFlags |= dwOpt;
			}
			else
			{
				*pfHasInvalid = TRUE;
			}
		}
	}

	*pnFlags = nFlags;
}

ATL_NOINLINE void ParseCommandLine(
	int argc, 
	wchar_t *argvW[], 
	CStringW & wszUrl, 
	BOOL fHasInvalid)
{
	for (int i=1; i<argc; i++)
	{
		if (argvW[i][0] != L'/')
		{
			if (wszUrl.IsEmpty())
			{
				wszUrl = argvW[i];
			}
			else
			{
				EmitCmdLineWarning(IDS_SDL_IGNORE_CMDITEM, argvW[i]);
			}
		}
		else if (fHasInvalid != FALSE)
		{
			DWORD dwOpt = GetOption(argvW[i]);
			if (dwOpt == 0)
			{
				EmitCmdLineWarning(IDS_SDL_IGNORE_CMDITEM, argvW[i]);
			}
		}
	}

	return;
}
CDiscoMapDocument * GetDiscoMapDocument()
{
	return g_pDMDoc;
}
