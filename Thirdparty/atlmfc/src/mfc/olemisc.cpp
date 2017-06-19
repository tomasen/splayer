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

/////////////////////////////////////////////////////////////////////////////
// Debug diagnostics for SCODEs

#ifdef _DEBUG
LPCTSTR AFXAPI AfxGetScodeString(SCODE sc)
{
	struct SCODE_ENTRY
	{
		SCODE sc;
		LPCTSTR lpszName;
	};
	#define MAKE_SCODE_ENTRY(sc)    { sc, _T(#sc) }
	static const SCODE_ENTRY scNameTable[] =
	{
		MAKE_SCODE_ENTRY(S_OK),
		MAKE_SCODE_ENTRY(S_FALSE),

		MAKE_SCODE_ENTRY(CACHE_S_FORMATETC_NOTSUPPORTED),
		MAKE_SCODE_ENTRY(CACHE_S_SAMECACHE),
		MAKE_SCODE_ENTRY(CACHE_S_SOMECACHES_NOTUPDATED),
		MAKE_SCODE_ENTRY(CONVERT10_S_NO_PRESENTATION),
		MAKE_SCODE_ENTRY(DATA_S_SAMEFORMATETC),
		MAKE_SCODE_ENTRY(DRAGDROP_S_CANCEL),
		MAKE_SCODE_ENTRY(DRAGDROP_S_DROP),
		MAKE_SCODE_ENTRY(DRAGDROP_S_USEDEFAULTCURSORS),
		MAKE_SCODE_ENTRY(INPLACE_S_TRUNCATED),
		MAKE_SCODE_ENTRY(MK_S_HIM),
		MAKE_SCODE_ENTRY(MK_S_ME),
		MAKE_SCODE_ENTRY(MK_S_MONIKERALREADYREGISTERED),
		MAKE_SCODE_ENTRY(MK_S_REDUCED_TO_SELF),
		MAKE_SCODE_ENTRY(MK_S_US),
		MAKE_SCODE_ENTRY(OLE_S_MAC_CLIPFORMAT),
		MAKE_SCODE_ENTRY(OLE_S_STATIC),
		MAKE_SCODE_ENTRY(OLE_S_USEREG),
		MAKE_SCODE_ENTRY(OLEOBJ_S_CANNOT_DOVERB_NOW),
		MAKE_SCODE_ENTRY(OLEOBJ_S_INVALIDHWND),
		MAKE_SCODE_ENTRY(OLEOBJ_S_INVALIDVERB),
		MAKE_SCODE_ENTRY(OLEOBJ_S_LAST),
		MAKE_SCODE_ENTRY(STG_S_CONVERTED),
		MAKE_SCODE_ENTRY(VIEW_S_ALREADY_FROZEN),

		MAKE_SCODE_ENTRY(E_UNEXPECTED),
		MAKE_SCODE_ENTRY(E_NOTIMPL),
		MAKE_SCODE_ENTRY(E_OUTOFMEMORY),
		MAKE_SCODE_ENTRY(E_INVALIDARG),
		MAKE_SCODE_ENTRY(E_NOINTERFACE),
		MAKE_SCODE_ENTRY(E_POINTER),
		MAKE_SCODE_ENTRY(E_HANDLE),
		MAKE_SCODE_ENTRY(E_ABORT),
		MAKE_SCODE_ENTRY(E_FAIL),
		MAKE_SCODE_ENTRY(E_ACCESSDENIED),

		MAKE_SCODE_ENTRY(CACHE_E_NOCACHE_UPDATED),
		MAKE_SCODE_ENTRY(CLASS_E_CLASSNOTAVAILABLE),
		MAKE_SCODE_ENTRY(CLASS_E_NOAGGREGATION),
		MAKE_SCODE_ENTRY(CLIPBRD_E_BAD_DATA),
		MAKE_SCODE_ENTRY(CLIPBRD_E_CANT_CLOSE),
		MAKE_SCODE_ENTRY(CLIPBRD_E_CANT_EMPTY),
		MAKE_SCODE_ENTRY(CLIPBRD_E_CANT_OPEN),
		MAKE_SCODE_ENTRY(CLIPBRD_E_CANT_SET),
		MAKE_SCODE_ENTRY(CO_E_ALREADYINITIALIZED),
		MAKE_SCODE_ENTRY(CO_E_APPDIDNTREG),
		MAKE_SCODE_ENTRY(CO_E_APPNOTFOUND),
		MAKE_SCODE_ENTRY(CO_E_APPSINGLEUSE),
		MAKE_SCODE_ENTRY(CO_E_BAD_PATH),
		MAKE_SCODE_ENTRY(CO_E_CANTDETERMINECLASS),
		MAKE_SCODE_ENTRY(CO_E_CLASS_CREATE_FAILED),
		MAKE_SCODE_ENTRY(CO_E_CLASSSTRING),
		MAKE_SCODE_ENTRY(CO_E_DLLNOTFOUND),
		MAKE_SCODE_ENTRY(CO_E_ERRORINAPP),
		MAKE_SCODE_ENTRY(CO_E_ERRORINDLL),
		MAKE_SCODE_ENTRY(CO_E_IIDSTRING),
		MAKE_SCODE_ENTRY(CO_E_NOTINITIALIZED),
		MAKE_SCODE_ENTRY(CO_E_OBJISREG),
		MAKE_SCODE_ENTRY(CO_E_OBJNOTCONNECTED),
		MAKE_SCODE_ENTRY(CO_E_OBJNOTREG),
		MAKE_SCODE_ENTRY(CO_E_OBJSRV_RPC_FAILURE),
		MAKE_SCODE_ENTRY(CO_E_SCM_ERROR),
		MAKE_SCODE_ENTRY(CO_E_SCM_RPC_FAILURE),
		MAKE_SCODE_ENTRY(CO_E_SERVER_EXEC_FAILURE),
		MAKE_SCODE_ENTRY(CO_E_SERVER_STOPPING),
		MAKE_SCODE_ENTRY(CO_E_WRONGOSFORAPP),
		MAKE_SCODE_ENTRY(CONVERT10_E_OLESTREAM_BITMAP_TO_DIB),
		MAKE_SCODE_ENTRY(CONVERT10_E_OLESTREAM_FMT),
		MAKE_SCODE_ENTRY(CONVERT10_E_OLESTREAM_GET),
		MAKE_SCODE_ENTRY(CONVERT10_E_OLESTREAM_PUT),
		MAKE_SCODE_ENTRY(CONVERT10_E_STG_DIB_TO_BITMAP),
		MAKE_SCODE_ENTRY(CONVERT10_E_STG_FMT),
		MAKE_SCODE_ENTRY(CONVERT10_E_STG_NO_STD_STREAM),
		MAKE_SCODE_ENTRY(DISP_E_ARRAYISLOCKED),
		MAKE_SCODE_ENTRY(DISP_E_BADCALLEE),
		MAKE_SCODE_ENTRY(DISP_E_BADINDEX),
		MAKE_SCODE_ENTRY(DISP_E_BADPARAMCOUNT),
		MAKE_SCODE_ENTRY(DISP_E_BADVARTYPE),
		MAKE_SCODE_ENTRY(DISP_E_EXCEPTION),
		MAKE_SCODE_ENTRY(DISP_E_MEMBERNOTFOUND),
		MAKE_SCODE_ENTRY(DISP_E_NONAMEDARGS),
		MAKE_SCODE_ENTRY(DISP_E_NOTACOLLECTION),
		MAKE_SCODE_ENTRY(DISP_E_OVERFLOW),
		MAKE_SCODE_ENTRY(DISP_E_PARAMNOTFOUND),
		MAKE_SCODE_ENTRY(DISP_E_PARAMNOTOPTIONAL),
		MAKE_SCODE_ENTRY(DISP_E_TYPEMISMATCH),
		MAKE_SCODE_ENTRY(DISP_E_UNKNOWNINTERFACE),
		MAKE_SCODE_ENTRY(DISP_E_UNKNOWNLCID),
		MAKE_SCODE_ENTRY(DISP_E_UNKNOWNNAME),
		MAKE_SCODE_ENTRY(DRAGDROP_E_ALREADYREGISTERED),
		MAKE_SCODE_ENTRY(DRAGDROP_E_INVALIDHWND),
		MAKE_SCODE_ENTRY(DRAGDROP_E_NOTREGISTERED),
		MAKE_SCODE_ENTRY(DV_E_CLIPFORMAT),
		MAKE_SCODE_ENTRY(DV_E_DVASPECT),
		MAKE_SCODE_ENTRY(DV_E_DVTARGETDEVICE),
		MAKE_SCODE_ENTRY(DV_E_DVTARGETDEVICE_SIZE),
		MAKE_SCODE_ENTRY(DV_E_FORMATETC),
		MAKE_SCODE_ENTRY(DV_E_LINDEX),
		MAKE_SCODE_ENTRY(DV_E_NOIVIEWOBJECT),
		MAKE_SCODE_ENTRY(DV_E_STATDATA),
		MAKE_SCODE_ENTRY(DV_E_STGMEDIUM),
		MAKE_SCODE_ENTRY(DV_E_TYMED),
		MAKE_SCODE_ENTRY(INPLACE_E_NOTOOLSPACE),
		MAKE_SCODE_ENTRY(INPLACE_E_NOTUNDOABLE),
		MAKE_SCODE_ENTRY(MEM_E_INVALID_LINK),
		MAKE_SCODE_ENTRY(MEM_E_INVALID_ROOT),
		MAKE_SCODE_ENTRY(MEM_E_INVALID_SIZE),
		MAKE_SCODE_ENTRY(MK_E_CANTOPENFILE),
		MAKE_SCODE_ENTRY(MK_E_CONNECTMANUALLY),
		MAKE_SCODE_ENTRY(MK_E_ENUMERATION_FAILED),
		MAKE_SCODE_ENTRY(MK_E_EXCEEDEDDEADLINE),
		MAKE_SCODE_ENTRY(MK_E_INTERMEDIATEINTERFACENOTSUPPORTED),
		MAKE_SCODE_ENTRY(MK_E_INVALIDEXTENSION),
		MAKE_SCODE_ENTRY(MK_E_MUSTBOTHERUSER),
		MAKE_SCODE_ENTRY(MK_E_NEEDGENERIC),
		MAKE_SCODE_ENTRY(MK_E_NO_NORMALIZED),
		MAKE_SCODE_ENTRY(MK_E_NOINVERSE),
		MAKE_SCODE_ENTRY(MK_E_NOOBJECT),
		MAKE_SCODE_ENTRY(MK_E_NOPREFIX),
		MAKE_SCODE_ENTRY(MK_E_NOSTORAGE),
		MAKE_SCODE_ENTRY(MK_E_NOTBINDABLE),
		MAKE_SCODE_ENTRY(MK_E_NOTBOUND),
		MAKE_SCODE_ENTRY(MK_E_SYNTAX),
		MAKE_SCODE_ENTRY(MK_E_UNAVAILABLE),
		MAKE_SCODE_ENTRY(OLE_E_ADVF),
		MAKE_SCODE_ENTRY(OLE_E_ADVISENOTSUPPORTED),
		MAKE_SCODE_ENTRY(OLE_E_BLANK),
		MAKE_SCODE_ENTRY(OLE_E_CANT_BINDTOSOURCE),
		MAKE_SCODE_ENTRY(OLE_E_CANT_GETMONIKER),
		MAKE_SCODE_ENTRY(OLE_E_CANTCONVERT),
		MAKE_SCODE_ENTRY(OLE_E_CLASSDIFF),
		MAKE_SCODE_ENTRY(OLE_E_ENUM_NOMORE),
		MAKE_SCODE_ENTRY(OLE_E_INVALIDHWND),
		MAKE_SCODE_ENTRY(OLE_E_INVALIDRECT),
		MAKE_SCODE_ENTRY(OLE_E_NOCACHE),
		MAKE_SCODE_ENTRY(OLE_E_NOCONNECTION),
		MAKE_SCODE_ENTRY(OLE_E_NOSTORAGE),
		MAKE_SCODE_ENTRY(OLE_E_NOT_INPLACEACTIVE),
		MAKE_SCODE_ENTRY(OLE_E_NOTRUNNING),
		MAKE_SCODE_ENTRY(OLE_E_OLEVERB),
		MAKE_SCODE_ENTRY(OLE_E_PROMPTSAVECANCELLED),
		MAKE_SCODE_ENTRY(OLE_E_STATIC),
		MAKE_SCODE_ENTRY(OLE_E_WRONGCOMPOBJ),
		MAKE_SCODE_ENTRY(OLEOBJ_E_INVALIDVERB),
		MAKE_SCODE_ENTRY(OLEOBJ_E_NOVERBS),
		MAKE_SCODE_ENTRY(REGDB_E_CLASSNOTREG),
		MAKE_SCODE_ENTRY(REGDB_E_IIDNOTREG),
		MAKE_SCODE_ENTRY(REGDB_E_INVALIDVALUE),
		MAKE_SCODE_ENTRY(REGDB_E_KEYMISSING),
		MAKE_SCODE_ENTRY(REGDB_E_READREGDB),
		MAKE_SCODE_ENTRY(REGDB_E_WRITEREGDB),
		MAKE_SCODE_ENTRY(RPC_E_ATTEMPTED_MULTITHREAD),
		MAKE_SCODE_ENTRY(RPC_E_CALL_CANCELED),
		MAKE_SCODE_ENTRY(RPC_E_CALL_REJECTED),
		MAKE_SCODE_ENTRY(RPC_E_CANTCALLOUT_AGAIN),
		MAKE_SCODE_ENTRY(RPC_E_CANTCALLOUT_INASYNCCALL),
		MAKE_SCODE_ENTRY(RPC_E_CANTCALLOUT_INEXTERNALCALL),
		MAKE_SCODE_ENTRY(RPC_E_CANTCALLOUT_ININPUTSYNCCALL),
		MAKE_SCODE_ENTRY(RPC_E_CANTPOST_INSENDCALL),
		MAKE_SCODE_ENTRY(RPC_E_CANTTRANSMIT_CALL),
		MAKE_SCODE_ENTRY(RPC_E_CHANGED_MODE),
		MAKE_SCODE_ENTRY(RPC_E_CLIENT_CANTMARSHAL_DATA),
		MAKE_SCODE_ENTRY(RPC_E_CLIENT_CANTUNMARSHAL_DATA),
		MAKE_SCODE_ENTRY(RPC_E_CLIENT_DIED),
		MAKE_SCODE_ENTRY(RPC_E_CONNECTION_TERMINATED),
		MAKE_SCODE_ENTRY(RPC_E_DISCONNECTED),
		MAKE_SCODE_ENTRY(RPC_E_FAULT),
		MAKE_SCODE_ENTRY(RPC_E_INVALID_CALLDATA),
		MAKE_SCODE_ENTRY(RPC_E_INVALID_DATA),
		MAKE_SCODE_ENTRY(RPC_E_INVALID_DATAPACKET),
		MAKE_SCODE_ENTRY(RPC_E_INVALID_PARAMETER),
		MAKE_SCODE_ENTRY(RPC_E_INVALIDMETHOD),
		MAKE_SCODE_ENTRY(RPC_E_NOT_REGISTERED),
		MAKE_SCODE_ENTRY(RPC_E_OUT_OF_RESOURCES),
		MAKE_SCODE_ENTRY(RPC_E_RETRY),
		MAKE_SCODE_ENTRY(RPC_E_SERVER_CANTMARSHAL_DATA),
		MAKE_SCODE_ENTRY(RPC_E_SERVER_CANTUNMARSHAL_DATA),
		MAKE_SCODE_ENTRY(RPC_E_SERVER_DIED),
		MAKE_SCODE_ENTRY(RPC_E_SERVER_DIED_DNE),
		MAKE_SCODE_ENTRY(RPC_E_SERVERCALL_REJECTED),
		MAKE_SCODE_ENTRY(RPC_E_SERVERCALL_RETRYLATER),
		MAKE_SCODE_ENTRY(RPC_E_SERVERFAULT),
		MAKE_SCODE_ENTRY(RPC_E_SYS_CALL_FAILED),
		MAKE_SCODE_ENTRY(RPC_E_THREAD_NOT_INIT),
		MAKE_SCODE_ENTRY(RPC_E_UNEXPECTED),
		MAKE_SCODE_ENTRY(RPC_E_WRONG_THREAD),
		MAKE_SCODE_ENTRY(STG_E_ABNORMALAPIEXIT),
		MAKE_SCODE_ENTRY(STG_E_ACCESSDENIED),
		MAKE_SCODE_ENTRY(STG_E_CANTSAVE),
		MAKE_SCODE_ENTRY(STG_E_DISKISWRITEPROTECTED),
		MAKE_SCODE_ENTRY(STG_E_EXTANTMARSHALLINGS),
		MAKE_SCODE_ENTRY(STG_E_FILEALREADYEXISTS),
		MAKE_SCODE_ENTRY(STG_E_FILENOTFOUND),
		MAKE_SCODE_ENTRY(STG_E_INSUFFICIENTMEMORY),
		MAKE_SCODE_ENTRY(STG_E_INUSE),
		MAKE_SCODE_ENTRY(STG_E_INVALIDFLAG),
		MAKE_SCODE_ENTRY(STG_E_INVALIDFUNCTION),
		MAKE_SCODE_ENTRY(STG_E_INVALIDHANDLE),
		MAKE_SCODE_ENTRY(STG_E_INVALIDHEADER),
		MAKE_SCODE_ENTRY(STG_E_INVALIDNAME),
		MAKE_SCODE_ENTRY(STG_E_INVALIDPARAMETER),
		MAKE_SCODE_ENTRY(STG_E_INVALIDPOINTER),
		MAKE_SCODE_ENTRY(STG_E_LOCKVIOLATION),
		MAKE_SCODE_ENTRY(STG_E_MEDIUMFULL),
		MAKE_SCODE_ENTRY(STG_E_NOMOREFILES),
		MAKE_SCODE_ENTRY(STG_E_NOTCURRENT),
		MAKE_SCODE_ENTRY(STG_E_NOTFILEBASEDSTORAGE),
		MAKE_SCODE_ENTRY(STG_E_OLDDLL),
		MAKE_SCODE_ENTRY(STG_E_OLDFORMAT),
		MAKE_SCODE_ENTRY(STG_E_PATHNOTFOUND),
		MAKE_SCODE_ENTRY(STG_E_READFAULT),
		MAKE_SCODE_ENTRY(STG_E_REVERTED),
		MAKE_SCODE_ENTRY(STG_E_SEEKERROR),
		MAKE_SCODE_ENTRY(STG_E_SHAREREQUIRED),
		MAKE_SCODE_ENTRY(STG_E_SHAREVIOLATION),
		MAKE_SCODE_ENTRY(STG_E_TOOMANYOPENFILES),
		MAKE_SCODE_ENTRY(STG_E_UNIMPLEMENTEDFUNCTION),
		MAKE_SCODE_ENTRY(STG_E_UNKNOWN),
		MAKE_SCODE_ENTRY(STG_E_WRITEFAULT),
		MAKE_SCODE_ENTRY(TYPE_E_AMBIGUOUSNAME),
		MAKE_SCODE_ENTRY(TYPE_E_BADMODULEKIND),
		MAKE_SCODE_ENTRY(TYPE_E_BUFFERTOOSMALL),
		MAKE_SCODE_ENTRY(TYPE_E_CANTCREATETMPFILE),
		MAKE_SCODE_ENTRY(TYPE_E_CANTLOADLIBRARY),
		MAKE_SCODE_ENTRY(TYPE_E_CIRCULARTYPE),
		MAKE_SCODE_ENTRY(TYPE_E_DLLFUNCTIONNOTFOUND),
		MAKE_SCODE_ENTRY(TYPE_E_DUPLICATEID),
		MAKE_SCODE_ENTRY(TYPE_E_ELEMENTNOTFOUND),
		MAKE_SCODE_ENTRY(TYPE_E_INCONSISTENTPROPFUNCS),
		MAKE_SCODE_ENTRY(TYPE_E_INVALIDSTATE),
		MAKE_SCODE_ENTRY(TYPE_E_INVDATAREAD),
		MAKE_SCODE_ENTRY(TYPE_E_IOERROR),
		MAKE_SCODE_ENTRY(TYPE_E_LIBNOTREGISTERED),
		MAKE_SCODE_ENTRY(TYPE_E_NAMECONFLICT),
		MAKE_SCODE_ENTRY(TYPE_E_OUTOFBOUNDS),
		MAKE_SCODE_ENTRY(TYPE_E_QUALIFIEDNAMEDISALLOWED),
		MAKE_SCODE_ENTRY(TYPE_E_REGISTRYACCESS),
		MAKE_SCODE_ENTRY(TYPE_E_SIZETOOBIG),
		MAKE_SCODE_ENTRY(TYPE_E_TYPEMISMATCH),
		MAKE_SCODE_ENTRY(TYPE_E_UNDEFINEDTYPE),
		MAKE_SCODE_ENTRY(TYPE_E_UNKNOWNLCID),
		MAKE_SCODE_ENTRY(TYPE_E_UNSUPFORMAT),
		MAKE_SCODE_ENTRY(TYPE_E_WRONGTYPEKIND),
		MAKE_SCODE_ENTRY(VIEW_E_DRAW),
	};
	#undef MAKE_SCODE_ENTRY

	// look for it in the table
	for (int i = 0; i < _countof(scNameTable); i++)
	{
		if (sc == scNameTable[i].sc)
			return scNameTable[i].lpszName;
	}
	return NULL;    // not found
}

LPCTSTR AFXAPI AfxGetScodeRangeString(SCODE sc)
{
	struct RANGE_ENTRY
	{
		SCODE scFirst;
		SCODE scLast;
		LPCTSTR lpszName;
	};
	#define MAKE_RANGE_ENTRY(scRange) \
		{ scRange##_FIRST, scRange##_LAST, \
			_T(#scRange) _T("_FIRST...") _T(#scRange) _T("_LAST") }

	static const RANGE_ENTRY scRangeTable[] =
	{
		MAKE_RANGE_ENTRY(CACHE_E),
		MAKE_RANGE_ENTRY(CACHE_S),
		MAKE_RANGE_ENTRY(CLASSFACTORY_E),
		MAKE_RANGE_ENTRY(CLASSFACTORY_S),
		MAKE_RANGE_ENTRY(CLIENTSITE_E),
		MAKE_RANGE_ENTRY(CLIENTSITE_S),
		MAKE_RANGE_ENTRY(CLIPBRD_E),
		MAKE_RANGE_ENTRY(CLIPBRD_S),
		MAKE_RANGE_ENTRY(CONVERT10_E),
		MAKE_RANGE_ENTRY(CONVERT10_S),
		MAKE_RANGE_ENTRY(CO_E),
		MAKE_RANGE_ENTRY(CO_S),
		MAKE_RANGE_ENTRY(DATA_E),
		MAKE_RANGE_ENTRY(DATA_S),
		MAKE_RANGE_ENTRY(DRAGDROP_E),
		MAKE_RANGE_ENTRY(DRAGDROP_S),
		MAKE_RANGE_ENTRY(ENUM_E),
		MAKE_RANGE_ENTRY(ENUM_S),
		MAKE_RANGE_ENTRY(INPLACE_E),
		MAKE_RANGE_ENTRY(INPLACE_S),
		MAKE_RANGE_ENTRY(MARSHAL_E),
		MAKE_RANGE_ENTRY(MARSHAL_S),
		MAKE_RANGE_ENTRY(MK_E),
		MAKE_RANGE_ENTRY(MK_S),
		MAKE_RANGE_ENTRY(OLEOBJ_E),
		MAKE_RANGE_ENTRY(OLEOBJ_S),
		MAKE_RANGE_ENTRY(OLE_E),
		MAKE_RANGE_ENTRY(OLE_S),
		MAKE_RANGE_ENTRY(REGDB_E),
		MAKE_RANGE_ENTRY(REGDB_S),
		MAKE_RANGE_ENTRY(VIEW_E),
		MAKE_RANGE_ENTRY(VIEW_S),
	};
	#undef MAKE_RANGE_ENTRY

	// look for it in the table
	for (int i = 0; i < _countof(scRangeTable); i++)
	{
		if (sc >= scRangeTable[i].scFirst && sc <= scRangeTable[i].scLast)
			return scRangeTable[i].lpszName;
	}
	return NULL;    // not found
}

LPCTSTR AFXAPI AfxGetSeverityString(SCODE sc)
{
	static const TCHAR* rgszSEVERITY[] =
	{
		_T("SEVERITY_SUCCESS"),
		_T("SEVERITY_ERROR"),
	};
	return rgszSEVERITY[SCODE_SEVERITY(sc)];
}

LPCTSTR AFXAPI AfxGetFacilityString(SCODE sc)
{
	static const TCHAR* rgszFACILITY[] =
	{
		_T("FACILITY_NULL"),
		_T("FACILITY_RPC"),
		_T("FACILITY_DISPATCH"),
		_T("FACILITY_STORAGE"),
		_T("FACILITY_ITF"),
		_T("FACILITY_0x05"),
		_T("FACILITY_0x06"),
		_T("FACILITY_WIN32"),
		_T("FACILITY_WINDOWS"),
	};
	if (SCODE_FACILITY(sc) >= _countof(rgszFACILITY))
		return _T("<Unknown Facility>");

	return rgszFACILITY[SCODE_FACILITY(sc)];
}

LPCTSTR AFXAPI AfxGetFullScodeString(SCODE sc)
{
	static TCHAR szBuf[128] = _T("<scode string too long>");

	LPCTSTR lpsz;
	if ((lpsz = AfxGetScodeString(sc)) != NULL)
	{
		// found exact match
		static const TCHAR szFormat[] = _T("%s ($%08lX)");
		if( lstrlen(lpsz) + _countof(szFormat) - 2 /*%s*/ - 6 /*$%08lX*/ + 8 /*size of long value*/ < _countof(szBuf) )
		{
			_stprintf_s(szBuf, _countof(szBuf), szFormat, lpsz, sc);
		}
	}
	else if ((lpsz = AfxGetScodeRangeString(sc)) != NULL)
	{
		// found suitable range
		static const TCHAR szFormat[] = _T("range: %s ($%08lX)");
		if( lstrlen(lpsz) + _countof(szFormat) - 2 /*%s*/ - 6 /*$%08lX*/ + 8 /*size of long value*/ < _countof(szBuf) )
		{
			_stprintf_s(szBuf, _countof(szBuf), szFormat, lpsz, sc);
		}
	}
	else
	{
		// not found at all -- split it up into its parts
		LPCTSTR lpszSeverity = AfxGetSeverityString(sc);
		LPCTSTR lpszFacility = AfxGetFacilityString(sc);
		static const TCHAR szFormat[] = _T("severity: %s, facility: %s ($%08lX)");

		if( lstrlen(lpszSeverity) + lstrlen(lpszFacility) + _countof(szFormat) - 2 /*%s*/ - 6 /*$%08lX*/ + 8 /*size of long value*/ < _countof(szBuf) )
		{
			_stprintf_s(szBuf, _countof(szBuf), szFormat, lpszSeverity, lpszFacility, sc);
		}
	}
	return szBuf;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// Exceptions for OLE Support

void __declspec(noreturn) AFXAPI AfxThrowOleException(SCODE sc)
{
#ifdef _DEBUG
	TRACE(traceOle, 0, _T("Warning: constructing COleException, scode = %s.\n"),
		AfxGetFullScodeString(sc));
#endif
	COleException* pException = new COleException;
	pException->m_sc = sc;
	THROW(pException);
}

BOOL COleException::GetErrorMessage(_Out_z_cap_(nMaxError) LPTSTR lpszError, _In_ UINT nMaxError,
		_Out_opt_ PUINT pnHelpContext) const
{
	ASSERT(lpszError != NULL && AfxIsValidString(lpszError, nMaxError));

	if (pnHelpContext != NULL)
		*pnHelpContext = 0;

	LPTSTR lpBuffer;
	if (::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM,
				NULL, m_sc,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
				(LPTSTR) &lpBuffer, 0, NULL) == 0)
	{
		*lpszError = '\0';
		return FALSE;
	}
	else
	{
		Checked::tcsncpy_s(lpszError, nMaxError, lpBuffer, _TRUNCATE);
		LocalFree(lpBuffer);
		return TRUE;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Turn a caught exception into an OLE return code

SCODE PASCAL COleException::Process(const CException* pAnyException)
{
	SCODE sc;
	if (pAnyException->IsKindOf(RUNTIME_CLASS(COleException)))
		sc = ((COleException*)pAnyException)->m_sc;
	else if (pAnyException->IsKindOf(RUNTIME_CLASS(CMemoryException)))
		sc = E_OUTOFMEMORY;
	else if (pAnyException->IsKindOf(RUNTIME_CLASS(CNotSupportedException)))
		sc = E_NOTIMPL;
	else
		sc = E_UNEXPECTED;  // some other problem

	return sc;
}

/////////////////////////////////////////////////////////////////////////////
// Implementation helpers

HMENU AFXAPI AfxMergeMenus(HMENU hMenuShared, HMENU hMenuSource,
	LONG* lpMenuWidths, int iWidthIndex, BOOL bMergeHelpMenus /* = FALSE */)
{
	ASSERT(hMenuShared != NULL && IsMenu(hMenuShared));
	ASSERT(hMenuSource != NULL && IsMenu(hMenuSource));

	BOOL bHelpMergedAsSubMenu = FALSE;
	HMENU hHelpSubMenu = NULL;

	// copy the popups from the pMenuSource
	int cMenuItems = GetMenuItemCount(hMenuSource);
	int cGroupWidth = 0;
	int nPosition = 0;

	// insert at appropriate spot depending on iWidthIndex
	ASSERT(iWidthIndex == 0 || iWidthIndex == 1);
	if (iWidthIndex == 1)
		nPosition = (int)lpMenuWidths[0];

	for (int i = 0; i < cMenuItems; i++)
	{
		// get the HMENU of the popup
		HMENU hMenuPopup = ::GetSubMenu(hMenuSource, i);

		// separators move us to next group
		UINT state = GetMenuState(hMenuSource, i, MF_BYPOSITION);
		if (hMenuPopup == NULL && (state & MF_SEPARATOR) != 0)
		{
			ASSERT(iWidthIndex <= 5);   // servers should not touch past 5
			lpMenuWidths[iWidthIndex] = cGroupWidth;
			cGroupWidth = 0;
			if (iWidthIndex < 5)
				nPosition += (int)lpMenuWidths[iWidthIndex+1];
			iWidthIndex += 2;
		}
		else
		{
			HMENU hHelpMenu = NULL;

			// are we processing the help menu group?

			if (bMergeHelpMenus && iWidthIndex == 5)
			{
				// if so, see if the container has Help any menu items
				if (lpMenuWidths[iWidthIndex] == 1)
				{
					// get the help menu from the container
					hHelpMenu = GetSubMenu(hMenuShared, nPosition);
				}
			}

			// get the menu item text
			TCHAR szItemText[256];
			int nLen = GetMenuString(hMenuSource, i, szItemText,
				sizeof(szItemText)/sizeof(TCHAR), MF_BYPOSITION);

			// popups are handled differently than normal menu items
			if (hMenuPopup != NULL)
			{
				if (hHelpMenu != NULL)
				{
					CString strTearOff = AfxGetAppName();
					if (!strTearOff.IsEmpty())
						strTearOff += ' ';
					strTearOff += szItemText;

					// container has help items -- add ours to its submenu
					AppendMenu(hHelpMenu, MF_STRING | MF_POPUP,
						(UINT_PTR)hMenuPopup, strTearOff);

					// clear the count of Help group items and add
					// the help menu to the window group

					lpMenuWidths[iWidthIndex] = 0;
					lpMenuWidths[iWidthIndex-1]++;

					bHelpMergedAsSubMenu = TRUE;
					hHelpSubMenu = hMenuPopup;
				}
				else if (::GetMenuItemCount(hMenuPopup) != 0)
				{
					// strip the HIBYTE because it contains a count of items
					state = LOBYTE(state) | MF_POPUP;   // must be popup

					// non-empty popup -- add it to the shared menu bar
					InsertMenu(hMenuShared, nPosition, state | MF_BYPOSITION,
						(DWORD_PTR)hMenuPopup, szItemText);
					++nPosition;
					++cGroupWidth;
				}
			}
			else if (nLen > 0)
			{
				// only non-empty items are added
				ASSERT(szItemText[0] != 0);

				// here the state does not contain a count in the HIBYTE
				InsertMenu(hMenuShared, nPosition, state | MF_BYPOSITION,
					GetMenuItemID(hMenuSource, i), szItemText);
				++nPosition;
				++cGroupWidth;
			}
		}
	}

	// set the last group width

	if (!bHelpMergedAsSubMenu)
	{
		ASSERT(iWidthIndex <= 5);   // servers should not touch past 5
		lpMenuWidths[iWidthIndex] = cGroupWidth;
	}

	return hHelpSubMenu;
}

void AFXAPI AfxUnmergeMenus(HMENU hMenuShared, HMENU hMenuSource,
	HMENU hHelpMenuPopup /* = NULL */)
{
	ASSERT(hMenuShared != NULL && IsMenu(hMenuShared));
	ASSERT(hMenuSource != NULL && IsMenu(hMenuSource));
	ASSERT(hHelpMenuPopup == NULL || IsMenu(hHelpMenuPopup));

	int cOurItems = GetMenuItemCount(hMenuSource);
	int cMenuItems = GetMenuItemCount(hMenuShared);

	for (int i = cMenuItems-1; i >= 0; i--)
	{
		// check out the popup menus
		HMENU hMenuPopup = ::GetSubMenu(hMenuShared, i);
		if (hMenuPopup != NULL)
		{
			// if we have a Help submenu, check to see if it appears in this
			// submenu someplace... this normally happens only in
			// DocObject frame windows

			if (hHelpMenuPopup != NULL)
			{
				int cPopupItems = ::GetMenuItemCount(hMenuPopup);
				for (int k = 0; k < cPopupItems; k++)
				{
					if (::GetSubMenu(hMenuPopup, k) == hHelpMenuPopup)
					{
						::RemoveMenu(hMenuPopup, k, MF_BYPOSITION);
						hHelpMenuPopup = NULL;  // can only have one
						break;
					}
				}
			}
			else
			{
				// if it is one of ours, remove it from the pMenuShared
				for (int j = 0; j < cOurItems; j++)
				{
					if (::GetSubMenu(hMenuSource, j) == hMenuPopup)
					{
						// remove the menu from pMenuShared
						RemoveMenu(hMenuShared, i, MF_BYPOSITION);
						break;
					}
				}
			}
		}
	}
}

// Helper for creating default FORMATETC from cfFormat
LPFORMATETC AFXAPI _AfxFillFormatEtc(
	LPFORMATETC lpFormatEtc, CLIPFORMAT cfFormat, LPFORMATETC lpFormatEtcFill)
{
	ENSURE(lpFormatEtcFill != NULL);
	if (lpFormatEtc == NULL && cfFormat!=0)
	{
		lpFormatEtc = lpFormatEtcFill;
		lpFormatEtc->cfFormat = cfFormat;
		lpFormatEtc->ptd = NULL;
		lpFormatEtc->dwAspect = DVASPECT_CONTENT;
		lpFormatEtc->lindex = -1;
		lpFormatEtc->tymed = (DWORD) -1;
	}
	return lpFormatEtc;
}

AFX_STATIC HGLOBAL AFXAPI _AfxCopyGlobalMemory(HGLOBAL hDest, HGLOBAL hSource)
{
	ASSERT(hSource != NULL);

	// make sure we have suitable hDest
	ULONG_PTR nSize = ::GlobalSize(hSource);
	if (hDest == NULL)
	{
		hDest = ::GlobalAlloc(GMEM_SHARE|GMEM_MOVEABLE, nSize);
		if (hDest == NULL)
			return NULL;
	}
	else if (nSize > ::GlobalSize(hDest))
	{
		// hDest is not large enough
		return NULL;
	}

	// copy the bits
	LPVOID lpSource = ::GlobalLock(hSource);
	LPVOID lpDest = ::GlobalLock(hDest);
	ASSERT(lpDest != NULL);
	ASSERT(lpSource != NULL);
	Checked::memcpy_s(lpDest, (ULONG)::GlobalSize(hDest), lpSource, (ULONG)nSize);
	::GlobalUnlock(hDest);
	::GlobalUnlock(hSource);

	// success -- return hDest
	return hDest;
}

BOOL AFXAPI _AfxCopyStgMedium(
	CLIPFORMAT cfFormat, LPSTGMEDIUM lpDest, LPSTGMEDIUM lpSource)
{
	if (lpDest->tymed == TYMED_NULL)
	{
		ASSERT(lpSource->tymed != TYMED_NULL);
		switch (lpSource->tymed)
		{
		case TYMED_ENHMF:
		case TYMED_HGLOBAL:
			ASSERT(sizeof(HGLOBAL) == sizeof(HENHMETAFILE));
			lpDest->tymed = lpSource->tymed;
			lpDest->hGlobal = NULL;
			break;  // fall through to CopyGlobalMemory case

		case TYMED_ISTREAM:
			lpDest->pstm = lpSource->pstm;
			lpDest->pstm->AddRef();
			lpDest->tymed = TYMED_ISTREAM;
			return TRUE;

		case TYMED_ISTORAGE:
			lpDest->pstg = lpSource->pstg;
			lpDest->pstg->AddRef();
			lpDest->tymed = TYMED_ISTORAGE;
			return TRUE;

		case TYMED_MFPICT:
			{
				// copy LPMETAFILEPICT struct + embedded HMETAFILE
				HGLOBAL hDest = _AfxCopyGlobalMemory(NULL, lpSource->hGlobal);
				if (hDest == NULL)
					return FALSE;
				LPMETAFILEPICT lpPict = (LPMETAFILEPICT)::GlobalLock(hDest);
				ASSERT(lpPict != NULL);
				lpPict->hMF = ::CopyMetaFile(lpPict->hMF, NULL);
				if (lpPict->hMF == NULL)
				{
					::GlobalUnlock(hDest);
					::GlobalFree(hDest);
					return FALSE;
				}
				::GlobalUnlock(hDest);

				// fill STGMEDIUM struct
				lpDest->hGlobal = hDest;
				lpDest->tymed = TYMED_MFPICT;
			}
			return TRUE;

		case TYMED_GDI:
			lpDest->tymed = TYMED_GDI;
			lpDest->hGlobal = NULL;
			break;

		case TYMED_FILE:
			{
				lpDest->tymed = TYMED_FILE;
				ASSERT(lpSource->lpszFileName != NULL);
				if (lpSource->lpszFileName == NULL)
				{
					AfxThrowInvalidArgException();
				}
				UINT cbSrc = lstrlenW(lpSource->lpszFileName);
				LPOLESTR szFileName = (LPOLESTR)::ATL::AtlCoTaskMemCAlloc((cbSrc+1),sizeof(OLECHAR));
				lpDest->lpszFileName = szFileName;
				if (szFileName == NULL)
					return FALSE;

				Checked::memcpy_s(szFileName, (cbSrc+1)*sizeof(OLECHAR), 
					lpSource->lpszFileName,  (cbSrc+1)*sizeof(OLECHAR));
				return TRUE;
			}

		// unable to create + copy other TYMEDs
		default:
			return FALSE;
		}
	}
	ASSERT(lpDest->tymed == lpSource->tymed);

	switch (lpSource->tymed)
	{
	case TYMED_HGLOBAL:
		{
			HGLOBAL hDest = _AfxCopyGlobalMemory(lpDest->hGlobal,
				lpSource->hGlobal);
			if (hDest == NULL)
				return FALSE;

			lpDest->hGlobal = hDest;
		}
		return TRUE;

	case TYMED_ISTREAM:
		{
			ASSERT(lpDest->pstm != NULL);
			ASSERT(lpSource->pstm != NULL);

			// get the size of the source stream
			STATSTG stat;
			if (lpSource->pstm->Stat(&stat, STATFLAG_NONAME) != S_OK)
			{
				// unable to get size of source stream
				return FALSE;
			}
			ASSERT(stat.pwcsName == NULL);

			// always seek to zero before copy
			LARGE_INTEGER zero = { 0, 0 };
			lpDest->pstm->Seek(zero, STREAM_SEEK_SET, NULL);
			lpSource->pstm->Seek(zero, STREAM_SEEK_SET, NULL);

			// copy source to destination
			if (lpSource->pstm->CopyTo(lpDest->pstm, stat.cbSize,
				NULL, NULL) != NULL)
			{
				// copy from source to dest failed
				return FALSE;
			}

			// always seek to zero after copy
			lpDest->pstm->Seek(zero, STREAM_SEEK_SET, NULL);
			lpSource->pstm->Seek(zero, STREAM_SEEK_SET, NULL);
		}
		return TRUE;

	case TYMED_ISTORAGE:
		{
			ASSERT(lpDest->pstg != NULL);
			ASSERT(lpSource->pstg != NULL);

			// just copy source to destination
			if (lpSource->pstg->CopyTo(0, NULL, NULL, lpDest->pstg) != S_OK)
				return FALSE;
		}
		return TRUE;

	case TYMED_FILE:
		{
			ASSERT(lpSource->lpszFileName != NULL);
			ASSERT(lpDest->lpszFileName != NULL);
			CString strSource(lpDest->lpszFileName);
			CString strDest(lpDest->lpszFileName);
			return CopyFile(lpSource->lpszFileName ? strSource.GetString() : NULL,
				lpDest->lpszFileName ? strDest.GetString() : NULL, FALSE);
		}


	case TYMED_ENHMF:
	case TYMED_GDI:
		{
			ASSERT(sizeof(HGLOBAL) == sizeof(HENHMETAFILE));

			// with TYMED_GDI cannot copy into existing HANDLE
			if (lpDest->hGlobal != NULL)
				return FALSE;

			// otherwise, use OleDuplicateData for the copy
			lpDest->hGlobal = OleDuplicateData(lpSource->hGlobal, cfFormat, 0);
			if (lpDest->hGlobal == NULL)
				return FALSE;
		}
		return TRUE;

	// other TYMEDs cannot be copied
	default:
		return FALSE;
	}
}

/////////////////////////////////////////////////////////////////////////////
// OLE utility functions (some borrowed from OLE2UI library)

HGLOBAL AFXAPI _AfxOleGetObjectDescriptorData(
	CLSID       clsid,
	DWORD       dwDrawAspect,
	SIZEL       sizel,
	POINTL      pointl,
	DWORD       dwStatus,
	LPCOLESTR   lpszFullUserTypeName,
	LPCOLESTR   lpszSrcOfCopy)
{
	HGLOBAL     hMem = NULL;
	LPOBJECTDESCRIPTOR lpOD;
	DWORD       dwObjectDescSize, dwFullUserTypeNameLen, dwSrcOfCopyLen;

	// Get the length of Full User Type Name; Add 1 for the null terminator
	dwFullUserTypeNameLen = lpszFullUserTypeName ?
		ocslen(lpszFullUserTypeName)+1 : 0;

	// Get the Source of Copy string and it's length;
	//  Add 1 for the null terminator
	if (lpszSrcOfCopy && lpszSrcOfCopy[0] != '\0')
	   dwSrcOfCopyLen = lstrlenW(lpszSrcOfCopy)+1;
	else
	{
	   // No src moniker so use user type name as source string.
	   lpszSrcOfCopy =  lpszFullUserTypeName;
	   dwSrcOfCopyLen = dwFullUserTypeNameLen;
	}

	// Allocate space for OBJECTDESCRIPTOR and the additional string data
	dwObjectDescSize = sizeof(OBJECTDESCRIPTOR);
	hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE | GMEM_ZEROINIT,
	   dwObjectDescSize + (dwFullUserTypeNameLen + dwSrcOfCopyLen) *
		sizeof(OLECHAR));
	if (!hMem)
		return NULL;

	// Get pointer to memory from handle
	lpOD = (LPOBJECTDESCRIPTOR)GlobalLock(hMem);

	// Set the FullUserTypeName offset and copy the string
	if (lpszFullUserTypeName)
	{
		lpOD->dwFullUserTypeName = dwObjectDescSize;
		Checked::wcscpy_s((LPOLESTR)((LPBYTE)lpOD+lpOD->dwFullUserTypeName), 
			dwFullUserTypeNameLen, lpszFullUserTypeName);
	}
	else
	{
		// zero offset indicates that string is not present
		lpOD->dwFullUserTypeName = 0;  
	}

	// Set the SrcOfCopy offset and copy the string
	if (lpszSrcOfCopy)
	{
		lpOD->dwSrcOfCopy = dwObjectDescSize + dwFullUserTypeNameLen * sizeof(OLECHAR);
		Checked::wcscpy_s((LPOLESTR)((LPBYTE)lpOD+lpOD->dwSrcOfCopy), 
			dwSrcOfCopyLen, lpszSrcOfCopy);
	}
	else
	{
		// zero offset indicates that string is not present
		lpOD->dwSrcOfCopy = 0;  
	}

	// Initialize the rest of the OBJECTDESCRIPTOR
	lpOD->cbSize       = dwObjectDescSize +
		(dwFullUserTypeNameLen + dwSrcOfCopyLen) * sizeof(OLECHAR);
	lpOD->clsid        = clsid;
	lpOD->dwDrawAspect = dwDrawAspect;
	lpOD->sizel        = sizel;
	lpOD->pointl       = pointl;
	lpOD->dwStatus     = dwStatus;

	GlobalUnlock(hMem);
	return hMem;
}

HGLOBAL AFXAPI _AfxOleGetObjectDescriptorData(
	LPOLEOBJECT     lpOleObj,
	LPCOLESTR       lpszSrcOfCopy,
	DWORD           dwDrawAspect,
	POINTL          pointl,
	LPSIZEL         lpSizelHim)
{
	CLSID							clsid;
	::ATL::CComHeapPtr<OLECHAR>		sposzFullUserTypeName;
	::ATL::CComPtr<IMoniker>			spSrcMonikerOfCopy;
	HGLOBAL							hObjDesc = NULL;
	SCODE							sc;
	SIZEL							sizelHim;
	BOOL							bFreeSrcOfCopy = FALSE;
	::ATL::CComQIPtr<IViewObject2>	spViewObj2(lpOleObj);
	::ATL::CComQIPtr<IOleLink>		spOleLink(lpOleObj);
	BOOL							fIsLink;
	DWORD							dwStatus = 0;

	// query for IOleLink
	fIsLink = (spOleLink != NULL);

	// Get CLSID
	sc = lpOleObj->GetUserClassID(&clsid);
	if (sc != S_OK)
		clsid = CLSID_NULL;

	// Get FullUserTypeName
	sc = lpOleObj->GetUserType(USERCLASSTYPE_FULL, &sposzFullUserTypeName);

	// if object is a link, then expand usertypename to be "Linked %s"
	if (fIsLink && sposzFullUserTypeName)
	{
		// Note: If this LoadString call fails, it is likely that
		//  _AFX_NO_OLE_RESOURCES is defined in your .RC file.
		// To correct the situation remove the following line from your
		//  resource script:
		//      #define _AFX_NO_OLE_RESOURCES
		// This should be done using the Resource.Set Includes... command.
		CStringW strLinkedTypeFmt;
		ENSURE(strLinkedTypeFmt.LoadString(AFX_IDS_PASTELINKEDTYPE));

		int nBuf = ocslen(sposzFullUserTypeName) + strLinkedTypeFmt.GetLength() + 1;
		::ATL::CComHeapPtr<OLECHAR>	sposzNewFullUserTypeName;
		if(sposzNewFullUserTypeName.Allocate(nBuf))
		{
			ATL_CRT_ERRORCHECK_SPRINTF(_snwprintf_s(sposzNewFullUserTypeName, nBuf, nBuf - 1, strLinkedTypeFmt.GetString(), sposzFullUserTypeName));
			sposzFullUserTypeName.Attach(sposzNewFullUserTypeName.Detach());
		}
	}

	// get source of copy
	if (fIsLink)
	{
		sc = spOleLink->GetSourceDisplayName((LPOLESTR*)&lpszSrcOfCopy);
		bFreeSrcOfCopy = TRUE;
	}
	else if (lpszSrcOfCopy == NULL)
	{
		sc = lpOleObj->GetMoniker(
			OLEGETMONIKER_TEMPFORUSER, OLEWHICHMK_OBJFULL, &spSrcMonikerOfCopy);
		if (sc == S_OK)
		{
			::ATL::CComPtr<IBindCtx>		spbc;
			CreateBindCtx(0, &spbc);
			spSrcMonikerOfCopy->GetDisplayName(spbc, NULL,
				(LPOLESTR*)&lpszSrcOfCopy);
			bFreeSrcOfCopy = TRUE;
		}
	}

	if (lpSizelHim)
	{
		// Use extents passed by the caller
		sizelHim = *lpSizelHim;
	}
	else if (spViewObj2)
	{
		// Get the current extents from the object
		sc = spViewObj2->GetExtent(dwDrawAspect, -1, NULL,
			(LPSIZEL)&sizelHim);
		if (sc != S_OK)
			sizelHim.cx = sizelHim.cy = 0;
	}
	else
	{
		sizelHim.cx = sizelHim.cy = 0;
	}

	// Get dwStatus
	sc = lpOleObj->GetMiscStatus(dwDrawAspect, &dwStatus);
	if (sc != S_OK)
		dwStatus = 0;

	// Get OBJECTDESCRIPTOR
	hObjDesc = _AfxOleGetObjectDescriptorData(clsid, dwDrawAspect, sizelHim,
		pointl, dwStatus, sposzFullUserTypeName, lpszSrcOfCopy);

	// Clean up
	if (bFreeSrcOfCopy)
		CoTaskMemFree((LPOLESTR)lpszSrcOfCopy);

	return hObjDesc;
}

SCODE AFXAPI _AfxOleDoConvert(LPSTORAGE lpStg, REFCLSID rClsidNew)
{
	SCODE sc;
	CLSID clsidOld;
	if ((sc = ReadClassStg(lpStg, &clsidOld)) != S_OK)
	{
		clsidOld = CLSID_NULL;
		return sc;
	}

	// read old fmt/old user type; sets out params to NULL on error
	CLIPFORMAT cfOld;
	LPOLESTR lpszOld = NULL;
	sc = ReadFmtUserTypeStg(lpStg, &cfOld, &lpszOld);
	ASSERT(sc == S_OK || (cfOld == 0 && lpszOld == NULL));

	// get new user type name; if error, set to NULL string
	OLECHAR chZero = 0;
	LPOLESTR lpszNew = NULL;
	if (OleRegGetUserType(rClsidNew, USERCLASSTYPE_FULL, &lpszNew) != S_OK)
		lpszNew = &chZero;

	// write class stg
	if ((sc = WriteClassStg(lpStg, rClsidNew)) != S_OK)
		goto ErrorReturn;

	// write old fmt/new user type;
	if ((sc = WriteFmtUserTypeStg(lpStg, cfOld, lpszNew)) != S_OK)
		goto RewriteInfo;

	// set convert bit
	if ((sc = SetConvertStg(lpStg, TRUE)) != S_OK)
		goto RewriteInfo;

	goto ErrorReturn;

RewriteInfo:
	WriteClassStg(lpStg, clsidOld);
	WriteFmtUserTypeStg(lpStg, cfOld, lpszOld);

ErrorReturn:
	if (lpszNew != &chZero)
		CoTaskMemFree(lpszNew);

	CoTaskMemFree(lpszOld);
	return sc;
}

SCODE AFXAPI _AfxOleDoTreatAsClass(
	LPCTSTR lpszUserType, REFCLSID rclsid, REFCLSID rclsidNew)
{
	LPTSTR  lpszCLSID = NULL;
	HKEY    hKey;

	SCODE sc = CoTreatAsClass(rclsid, rclsidNew);
	if (sc != S_OK && lpszUserType != NULL)
	{
		AfxRegOpenKey(HKEY_CLASSES_ROOT, _T("CLSID"), &hKey);
		LPOLESTR lpOleStr = NULL;
		StringFromCLSID(rclsid, &lpOleStr);
		lpszCLSID = TASKSTRINGOLE2T(lpOleStr);
		RegSetValue(hKey, lpszCLSID, REG_SZ, lpszUserType,
			lstrlen(lpszUserType) * sizeof(TCHAR));

		CoTaskMemFree(lpszCLSID);
		sc = CoTreatAsClass(rclsid, rclsidNew);
		RegCloseKey(hKey);
	}
	return sc;
}

#define DEVNAMEPART(pdn, x) (pdn->x == 0 ? NULL : (LPCTSTR)pdn + pdn->x)

DVTARGETDEVICE* AFXAPI _AfxOleCreateTargetDevice(LPDEVNAMES pDN, LPDEVMODE pDM)
{
    USES_CONVERSION_EX;

	DVTARGETDEVICE* ptd = NULL;
	DWORD dwDevNamesSize, dwDevModeSize, dwPtdSize;

	LPCTSTR lpszDriverName = DEVNAMEPART(pDN, wDriverOffset);
	LPCTSTR lpszDeviceName = DEVNAMEPART(pDN, wDeviceOffset);
	LPCTSTR lpszPortName = DEVNAMEPART(pDN, wOutputOffset);

	ENSURE(lpszDriverName);
	ENSURE(lpszDeviceName);
	ENSURE(lpszPortName);
	CStringW strDriverNameOle(lpszDriverName);
	CStringW strDeviceNameOle(lpszDeviceName);
	CStringW strPortNameOle(lpszPortName);
	int nDriverNameSize = (int)((lpszDriverName == NULL) ? 0 : (strDriverNameOle.GetLength()+1)*sizeof(OLECHAR));
	int nDeviceNameSize = (int)((lpszDeviceName == NULL) ? 0 : (strDeviceNameOle.GetLength()+1)*sizeof(OLECHAR));
	int nPortNameSize = (int)((lpszPortName == NULL) ? 0 : (strPortNameOle.GetLength()+1)*sizeof(OLECHAR));

	LPDEVMODEOLE lpDevModeOle = DEVMODET2OLE_EX(pDM);
	ENSURE(lpDevModeOle);

	dwDevNamesSize = nDriverNameSize + nDeviceNameSize + nPortNameSize;
	dwDevModeSize = (DWORD)(lpDevModeOle->dmSize + lpDevModeOle->dmDriverExtra);

	dwPtdSize = sizeof(DVTARGETDEVICE) + dwDevNamesSize + dwDevModeSize;

	if ((ptd = (DVTARGETDEVICE*)CoTaskMemAlloc(dwPtdSize)) != NULL)
	{
		// copy in the info
		ptd->tdSize = (UINT)dwPtdSize;
		ptd->tdDriverNameOffset = sizeof(DVTARGETDEVICE);

		if(nDriverNameSize > 0)
		{
			Checked::wcscpy_s((LPOLESTR)((BYTE*)ptd + ptd->tdDriverNameOffset),
				(strDriverNameOle.GetLength()+1), strDriverNameOle.GetString());
		}

		ptd->tdDeviceNameOffset = (WORD)(ptd->tdDriverNameOffset + nDriverNameSize);
		if(nDeviceNameSize > 0)
		{
			Checked::wcscpy_s((LPOLESTR)((BYTE*)ptd + ptd->tdDeviceNameOffset), 
				(strDeviceNameOle.GetLength()+1), strDeviceNameOle.GetString());
		}

		ptd->tdPortNameOffset = (WORD)(ptd->tdDeviceNameOffset + nDeviceNameSize);
		if(nPortNameSize)
		{
			Checked::wcscpy_s((LPOLESTR)((BYTE*)ptd + ptd->tdPortNameOffset), 
				(strPortNameOle.GetLength()+1), strPortNameOle.GetString());
		}

		ptd->tdExtDevmodeOffset = (WORD)(ptd->tdPortNameOffset + nPortNameSize);
		Checked::memcpy_s((BYTE*)ptd + ptd->tdExtDevmodeOffset, dwPtdSize-ptd->tdExtDevmodeOffset, 
			lpDevModeOle, sizeof(DEVMODEOLE)+lpDevModeOle->dmDriverExtra);
	}
	return ptd;
}

DVTARGETDEVICE* AFXAPI _AfxOleCreateTargetDevice(LPPRINTDLG lpPrintDlg)
{
	DVTARGETDEVICE* ptd=NULL;
	LPDEVNAMES pDN;
	LPDEVMODE pDM;

	if ((pDN = (LPDEVNAMES)GlobalLock(lpPrintDlg->hDevNames)) == NULL)
		return NULL;

	if ((pDM = (LPDEVMODE)GlobalLock(lpPrintDlg->hDevMode)) == NULL)
	{
		GlobalUnlock(lpPrintDlg->hDevNames);
		return NULL;
	}

	ptd = _AfxOleCreateTargetDevice(pDN, pDM);

	GlobalUnlock(lpPrintDlg->hDevNames);
	GlobalUnlock(lpPrintDlg->hDevMode);

	return ptd;
}

LPMONIKER AFXAPI _AfxOleGetFirstMoniker(LPMONIKER lpmk)
{
	if (lpmk == NULL)
		return NULL;

	DWORD dwMksys;
	if (lpmk->IsSystemMoniker(&dwMksys) == S_OK
		&& dwMksys == MKSYS_GENERICCOMPOSITE)
	{
		LPENUMMONIKER lpenumMoniker = NULL;
		SCODE sc = lpmk->Enum(TRUE, &lpenumMoniker);
		if (sc != S_OK)
			return NULL;

		ASSERT(lpenumMoniker != NULL);
		LPMONIKER lpmkFirst = NULL;
		sc = lpenumMoniker->Next(1, &lpmkFirst, NULL);
		RELEASE(lpenumMoniker);
		return lpmkFirst;
	}

	// otherwise -- return the moniker itself
	lpmk->AddRef();
	return lpmk;
}

DWORD AFXAPI _AfxOleGetLenFilePrefixOfMoniker(LPMONIKER lpmk)
{
	if (lpmk == NULL)
		return 0;

	DWORD nLen = 0;
	LPMONIKER lpmkFirst = _AfxOleGetFirstMoniker(lpmk);
	if (lpmkFirst != NULL)
	{
		DWORD  dwMksys;
		if (lpmkFirst->IsSystemMoniker(&dwMksys) == S_OK &&
			dwMksys == MKSYS_FILEMONIKER)
		{
			LPBC lpbc = NULL;
			SCODE sc = CreateBindCtx(0, &lpbc);
			if (sc == S_OK)
			{
				LPOLESTR lpw = NULL;
				sc = lpmkFirst->GetDisplayName(lpbc, NULL, &lpw);
				if (sc == S_OK && lpw != NULL)
				{
					nLen = static_cast<DWORD>(wcslen(lpw));
					CoTaskMemFree(lpw);
				}
				RELEASE(lpbc);
			}
		}
		RELEASE(lpmkFirst);
	}
	return nLen;
}

DVTARGETDEVICE* AFXAPI _AfxOleCopyTargetDevice(DVTARGETDEVICE* ptdSrc)
{
	if (ptdSrc == NULL)
		return NULL;

	DVTARGETDEVICE* ptdDest =
		(DVTARGETDEVICE*)CoTaskMemAlloc(ptdSrc->tdSize);
	if (ptdDest == NULL)
		return NULL;

	Checked::memcpy_s(ptdDest, (size_t)ptdSrc->tdSize, 
		ptdSrc, (size_t)ptdSrc->tdSize);
	return ptdDest;
}

void AFXAPI _AfxOleCopyFormatEtc(LPFORMATETC petcDest, LPFORMATETC petcSrc)
{
	ASSERT(petcDest != NULL);
	ASSERT(petcSrc != NULL);

	petcDest->cfFormat = petcSrc->cfFormat;
	petcDest->ptd = _AfxOleCopyTargetDevice(petcSrc->ptd);
	petcDest->dwAspect = petcSrc->dwAspect;
	petcDest->lindex = petcSrc->lindex;
	petcDest->tymed = petcSrc->tymed;
}

HDC WINAPI _AfxOleCreateDC(DVTARGETDEVICE* ptd)
{
    USES_CONVERSION_EX;
	// return screen DC for NULL target device
	if (ptd == NULL)
		return ::CreateDC(_T("DISPLAY"), NULL, NULL, NULL);

	LPDEVMODEOLE lpDevMode;
	LPOLESTR lpszDriverName;
	LPOLESTR lpszDeviceName;
	LPOLESTR lpszPortName;

	if (ptd->tdExtDevmodeOffset == 0)
		lpDevMode = NULL;
	else
		lpDevMode  = (LPDEVMODEOLE) ((LPSTR)ptd + ptd->tdExtDevmodeOffset);

	lpszDriverName = (LPOLESTR)((BYTE*)ptd + ptd->tdDriverNameOffset);
	lpszDeviceName = (LPOLESTR)((BYTE*)ptd + ptd->tdDeviceNameOffset);
	lpszPortName   = (LPOLESTR)((BYTE*)ptd + ptd->tdPortNameOffset);

	return ::CreateDC(CString(lpszDriverName), CString(lpszDeviceName),
		CString(lpszPortName), DEVMODEOLE2T_EX(lpDevMode));
}

void AFXAPI _AfxDeleteMetafilePict(HGLOBAL hMetaPict)
{
	if (hMetaPict != NULL)
	{
		STGMEDIUM stgMedium;
		stgMedium.hGlobal = hMetaPict;
		stgMedium.tymed = TYMED_MFPICT;
		stgMedium.pUnkForRelease = NULL;
		ReleaseStgMedium(&stgMedium);
	}
}

#define HIMETRIC_PER_INCH   2540
#define MAP_PIX_TO_LOGHIM(x,ppli)   MulDiv(HIMETRIC_PER_INCH, (x), (ppli))
#define MAP_LOGHIM_TO_PIX(x,ppli)   MulDiv((ppli), (x), HIMETRIC_PER_INCH)

void AFXAPI _AfxXformSizeInPixelsToHimetric(
	HDC hDC, LPSIZEL lpSizeInPix, LPSIZEL lpSizeInHiMetric)
{
	int cxPPI;
	int cyPPI;

	if ((NULL == hDC) || (GetDeviceCaps(hDC, LOGPIXELSX) == 0))
	{
		cxPPI = afxData.cxPixelsPerInch;
		cyPPI = afxData.cyPixelsPerInch;
	}
	else
	{
		cxPPI = GetDeviceCaps(hDC, LOGPIXELSX);
		cyPPI = GetDeviceCaps(hDC, LOGPIXELSY);
	}

	lpSizeInHiMetric->cx = (long)MAP_PIX_TO_LOGHIM((int)lpSizeInPix->cx, cxPPI);
	lpSizeInHiMetric->cy = (long)MAP_PIX_TO_LOGHIM((int)lpSizeInPix->cy, cyPPI);
}

void AFXAPI _AfxXformSizeInHimetricToPixels(
	HDC hDC, LPSIZEL lpSizeInHiMetric, LPSIZEL lpSizeInPix)
{
	int cxPPI;
	int cyPPI;

	if ((NULL == hDC) || (GetDeviceCaps(hDC, LOGPIXELSX) == 0))
	{
		cxPPI = afxData.cxPixelsPerInch;
		cyPPI = afxData.cyPixelsPerInch;
	}
	else
	{
		cxPPI = GetDeviceCaps(hDC, LOGPIXELSX);
		cyPPI = GetDeviceCaps(hDC, LOGPIXELSY);
	}

	lpSizeInPix->cx = (long)MAP_LOGHIM_TO_PIX((int)lpSizeInHiMetric->cx, cxPPI);
	lpSizeInPix->cy = (long)MAP_LOGHIM_TO_PIX((int)lpSizeInHiMetric->cy, cyPPI);
}


IMPLEMENT_DYNAMIC(COleException, CException)

/////////////////////////////////////////////////////////////////////////////
