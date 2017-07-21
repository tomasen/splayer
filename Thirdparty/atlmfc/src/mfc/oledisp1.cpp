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
#include "dispimpl.h"
#include <atlconv.h>
#include "sal.h"



#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// Helpers and main implementation for CCmdTarget::IDispatch
void CCmdTarget::GetStandardProp(const AFX_DISPMAP_ENTRY* pEntry,
	VARIANT* pvarResult, UINT* puArgErr)
{
	ASSERT(pEntry != NULL);
	ASSERT(*puArgErr != 0);

	// it is a DISPATCH_PROPERTYGET (for standard, non-function property)
	void* pProp = (BYTE*)this + pEntry->nPropOffset;
	if (pEntry->vt != VT_VARIANT)
		pvarResult->vt = pEntry->vt;
	switch (pEntry->vt)
	{
	case VT_I1:
	case VT_UI1:
		pvarResult->bVal = *(BYTE*)pProp;
		break;
	case VT_I2:
	case VT_UI2:
		pvarResult->iVal = *(short*)pProp;
		break;
	case VT_I4:
	case VT_UI4:
		pvarResult->lVal = *(long*)pProp;
		break;
	case VT_I8:
	case VT_UI8:
		pvarResult->llVal = *(LONGLONG*)pProp;
		break;
	case VT_R4:
		pvarResult->fltVal = *(float*)pProp;
		break;
	case VT_R8:
		pvarResult->dblVal = *(double*)pProp;
		break;
	case VT_DATE:
		pvarResult->date = *(double*)pProp;
		break;
	case VT_CY:
		pvarResult->cyVal = *(CY*)pProp;
		break;
	case VT_BSTR:
		{
			CString* pString = (CString*)pProp;
			pvarResult->bstrVal = pString->AllocSysString();
		}
		break;
	case VT_ERROR:
		pvarResult->scode = *(SCODE*)pProp;
		break;
	case VT_BOOL:
		V_BOOL(pvarResult) = (VARIANT_BOOL)(*(BOOL*)pProp != 0 ? -1 : 0);
		break;
	case VT_VARIANT:
		if (VariantCopy(pvarResult, (LPVARIANT)pProp) != S_OK)
			*puArgErr = 0;
		break;
	case VT_DISPATCH:
	case VT_UNKNOWN:
		pvarResult->punkVal = *(LPDISPATCH*)pProp;
		if (pvarResult->punkVal != NULL)
			pvarResult->punkVal->AddRef();
		break;

	default:
		*puArgErr = 0;
	}
}

SCODE CCmdTarget::SetStandardProp(const AFX_DISPMAP_ENTRY* pEntry,
	DISPPARAMS* pDispParams, UINT* puArgErr)
{
	ASSERT(pEntry != NULL);
	ASSERT(*puArgErr != 0);

	// it is a DISPATCH_PROPERTYSET (for standard, non-function property)
	SCODE sc = S_OK;
	VARIANT va;
	AfxVariantInit(&va);
	VARIANT* pArg = &pDispParams->rgvarg[0];
	if (pEntry->vt != VT_VARIANT && pArg->vt != pEntry->vt)
	{
		// argument is not of appropriate type, attempt to coerce it
		sc = VariantChangeType(&va, pArg, 0, pEntry->vt);
		if (FAILED(sc))
		{
			TRACE(traceOle, 0, "Warning: automation property coercion failed.\n");
			*puArgErr = 0;
			return sc;
		}
		ASSERT(va.vt == pEntry->vt);
		pArg = &va;
	}

	void* pProp = (BYTE*)this + pEntry->nPropOffset;
	switch (pEntry->vt)
	{
	case VT_I1:
	case VT_UI1:
		*(BYTE*)pProp = pArg->bVal;
		break;
	case VT_I2:
	case VT_UI2:
		*(short*)pProp = pArg->iVal;
		break;
	case VT_I4:
	case VT_UI4:
		*(long*)pProp = pArg->lVal;
		break;
	case VT_I8:
	case VT_UI8:
		*(LONGLONG*)pProp = pArg->llVal;
		break;
	case VT_R4:
		*(float*)pProp = pArg->fltVal;
		break;
	case VT_R8:
		*(double*)pProp = pArg->dblVal;
		break;
	case VT_DATE:
		*(double*)pProp = pArg->date;
		break;
	case VT_CY:
		*(CY*)pProp = pArg->cyVal;
		break;
	case VT_BSTR:
		AfxBSTR2CString((CString*)pProp, pArg->bstrVal);
		break;
	case VT_ERROR:
		*(SCODE*)pProp = pArg->scode;
		break;
	case VT_BOOL:
		*(BOOL*)pProp = (V_BOOL(pArg) != 0);
		break;
	case VT_VARIANT:
		if (VariantCopy((LPVARIANT)pProp, pArg) != S_OK)
			*puArgErr = 0;
		break;
	case VT_DISPATCH:
	case VT_UNKNOWN:
		if (pArg->punkVal != NULL)
			pArg->punkVal->AddRef();
		_AfxRelease((LPUNKNOWN*)pProp);
		*(LPUNKNOWN*)pProp = pArg->punkVal;
		break;

	default:
		*puArgErr = 0;
		sc = DISP_E_BADVARTYPE;
	}
	VariantClear(&va);

	// if property was a DISP_PROPERTY_NOTIFY type, call pfnSet after setting
	if (!FAILED(sc) && pEntry->pfnSet != NULL)
	{
		AFX_MANAGE_STATE(m_pModuleState);
		(this->*pEntry->pfnSet)();
	}

	return sc;
}

UINT PASCAL CCmdTarget::GetEntryCount(const AFX_DISPMAP* pDispMap)
{
	ASSERT(pDispMap->lpEntryCount != NULL);

	// compute entry count cache if not available
	if (*pDispMap->lpEntryCount == -1)
	{
		// count them
		const AFX_DISPMAP_ENTRY* pEntry = pDispMap->lpEntries;
		while (pEntry->nPropOffset != -1)
			++pEntry;

		// store it
		*pDispMap->lpEntryCount = UINT(pEntry - pDispMap->lpEntries);
	}

	ASSERT(*pDispMap->lpEntryCount != -1);
	return *pDispMap->lpEntryCount;
}

MEMBERID PASCAL CCmdTarget::MemberIDFromName(
	const AFX_DISPMAP* pDispMap, LPCTSTR lpszName)
{
	// search all maps and their base maps
	UINT nInherit = 0;
#ifdef _AFXDLL
	for (;;)
#else
	while (pDispMap != NULL)
#endif
	{
		// search all entries in this map
		const AFX_DISPMAP_ENTRY* pEntry = pDispMap->lpEntries;
		UINT nEntryCount = GetEntryCount(pDispMap);
		for (UINT nIndex = 0; nIndex < nEntryCount; nIndex++)
		{
			if (pEntry->vt != VT_MFCVALUE &&
				::AfxInvariantStrICmp(pEntry->lpszName, lpszName) == 0)
			{
				if (pEntry->lDispID == DISPID_UNKNOWN)
				{
					// the MEMBERID is combination of nIndex & nInherit
					ASSERT(MAKELONG(nIndex+1, nInherit) != DISPID_UNKNOWN);
					return MAKELONG(nIndex+1, nInherit);
				}
				// the MEMBERID is specified as the lDispID
				return pEntry->lDispID;
			}
			++pEntry;
		}
#ifdef _AFXDLL
		if (pDispMap->pfnGetBaseMap == NULL)
			break;
		pDispMap = (*pDispMap->pfnGetBaseMap)();
#else
		pDispMap = pDispMap->pBaseMap;
#endif
		++nInherit;
	}
	return DISPID_UNKNOWN;  // name not found
}

const AFX_DISPMAP_ENTRY* PASCAL CCmdTarget::GetDispEntry(MEMBERID memid)
{
	const AFX_DISPMAP* pDerivMap = GetDispatchMap();
	const AFX_DISPMAP* pDispMap;
	const AFX_DISPMAP_ENTRY* pEntry;

	if (memid == DISPID_VALUE)
	{
		// DISPID_VALUE is a special alias (look for special alias entry)
		pDispMap = pDerivMap;
#ifdef _AFXDLL
		for (;;)
#else
		while (pDispMap != NULL)
#endif
		{
			// search for special entry with vt == VT_MFCVALUE
			pEntry = pDispMap->lpEntries;
			while (pEntry->nPropOffset != -1)
			{
				if (pEntry->vt == VT_MFCVALUE)
				{
					memid = pEntry->lDispID;
					if (memid == DISPID_UNKNOWN)
					{
						// attempt to map alias name to member ID
						memid = MemberIDFromName(pDerivMap, pEntry->lpszName);
						if (memid == DISPID_UNKNOWN)
							return NULL;
					}
					// break out and map the member ID to an entry
					goto LookupDispID;
				}
				++pEntry;
			}
#ifdef _AFXDLL
			if (pDispMap->pfnGetBaseMap == NULL)
				break;
			pDispMap = (*pDispMap->pfnGetBaseMap)();
#else
			pDispMap = pDispMap->pBaseMap;
#endif
		}
	}

LookupDispID:
	if ((long)memid > 0)
	{
		// find AFX_DISPMAP corresponding to HIWORD(memid)
		UINT nTest = 0;
		pDispMap = pDerivMap;
#ifdef _AFXDLL
		while (nTest < (UINT)HIWORD(memid))
#else
		while (pDispMap != NULL && nTest < (UINT)HIWORD(memid))
#endif
		{
#ifdef _AFXDLL
			if (pDispMap->pfnGetBaseMap == NULL)
				break;
			pDispMap = (*pDispMap->pfnGetBaseMap)();
#else
			pDispMap = pDispMap->pBaseMap;
#endif
			++nTest;
		}
		if (pDispMap != NULL)
		{
			UINT nEntryCount = GetEntryCount(pDispMap);
			if ((UINT)LOWORD(memid) <= nEntryCount)
			{
				pEntry = pDispMap->lpEntries + LOWORD(memid)-1;

				// must have automatic DISPID or same ID
				// if not then look manually
				if (pEntry->lDispID == DISPID_UNKNOWN ||
					pEntry->lDispID == memid)
				{
					return pEntry;
				}
			}
		}
	}

	// second pass, look for DISP_XXX_ID entries
	pDispMap = pDerivMap;
#ifdef _AFXDLL
	for (;;)
#else
	while (pDispMap != NULL)
#endif
	{
		// find AFX_DISPMAP_ENTRY where (pEntry->lDispID == memid)
		pEntry = pDispMap->lpEntries;
		while (pEntry->nPropOffset != -1)
		{
			if (pEntry->lDispID == memid)
				return pEntry;

			++pEntry;
		}
		// check base class
#ifdef _AFXDLL
		if (pDispMap->pfnGetBaseMap == NULL)
			break;
		pDispMap = (*pDispMap->pfnGetBaseMap)();
#else
		pDispMap = pDispMap->pBaseMap;
#endif
	}

	return NULL;    // no matching entry
}

/////////////////////////////////////////////////////////////////////////////
// Standard automation methods

void CCmdTarget::GetNotSupported()
{
	AfxThrowOleDispatchException(
		AFX_IDP_GET_NOT_SUPPORTED, AFX_IDP_GET_NOT_SUPPORTED);
}

void CCmdTarget::SetNotSupported()
{
	AfxThrowOleDispatchException(
		AFX_IDP_SET_NOT_SUPPORTED, AFX_IDP_SET_NOT_SUPPORTED);
}

/////////////////////////////////////////////////////////////////////////////
// Wiring to CCmdTarget

// enable this object for OLE automation, called from derived class ctor
void CCmdTarget::EnableAutomation()
{
	ASSERT(GetDispatchMap() != NULL);   // must have DECLARE_DISPATCH_MAP

	// construct an COleDispatchImpl instance just to get to the vtable
	COleDispatchImpl dispatch;

	// vtable pointer should be already set to same or NULL
	ASSERT(m_xDispatch.m_vtbl == NULL||
		*(DWORD_PTR*)&dispatch == m_xDispatch.m_vtbl);
	// sizeof(COleDispatchImpl) should be just a DWORD (vtable pointer)
	ASSERT(sizeof(m_xDispatch) == sizeof(COleDispatchImpl));

	// copy the vtable (and other data) to make sure it is initialized
	m_xDispatch.m_vtbl = *(DWORD_PTR*)&dispatch;
	*(COleDispatchImpl*)&m_xDispatch = dispatch;
}

// return addref'd IDispatch part of CCmdTarget object
LPDISPATCH CCmdTarget::GetIDispatch(BOOL bAddRef)
{
	ASSERT_VALID(this);
	ASSERT(m_xDispatch.m_vtbl != 0);    // forgot to call EnableAutomation?

	// AddRef the object if requested
	if (bAddRef)
		ExternalAddRef();

	// return pointer to IDispatch implementation
	return (LPDISPATCH)GetInterface(&IID_IDispatch);
}

// retrieve CCmdTarget* from IDispatch* (return NULL if not MFC IDispatch)
CCmdTarget* PASCAL CCmdTarget::FromIDispatch(LPDISPATCH lpDispatch)
{
	// construct an COleDispatchImpl instance just to get to the vtable
	COleDispatchImpl dispatch;

	ASSERT(*(DWORD*)&dispatch != 0);    // null vtable ptr?
	if (*(DWORD*)lpDispatch != *(DWORD*)&dispatch)
		return NULL;    // not our IDispatch*

	// vtable ptrs match, so must have originally been retrieved with
	//  CCmdTarget::GetIDispatch.
#ifndef _AFX_NO_NESTED_DERIVATION
	CCmdTarget* pTarget = (CCmdTarget*)
		((BYTE*)lpDispatch - ((COleDispatchImpl*)lpDispatch)->m_nOffset);
#else
	CCmdTarget* pTarget = (CCmdTarget*)
		((BYTE*)lpDispatch - offsetof(CCmdTarget, m_xDispatch));
#endif
	ASSERT_VALID(pTarget);
	return pTarget;
}

BOOL CCmdTarget::IsResultExpected()
{
	BOOL bResultExpected = m_bResultExpected;
	m_bResultExpected = TRUE;   // can only ask once
	return bResultExpected;
}

void COleDispatchImpl::Disconnect()
{
	METHOD_PROLOGUE_EX_(CCmdTarget, Dispatch)

	pThis->ExternalDisconnect();    // always disconnect the object
}

///////////////////////////////////////////////////////////////////////////////
// OLE BSTR support


/////////////////////////////////////////////////////////////////////////////
// Specifics of METHOD->C++ member function invocation

// Note: Although this code is written in C++, it is very dependent on the
//  specific compiler and target platform.  The current code below assumes
//  that the stack grows down, and that arguments are pushed last to first.

// calculate size of pushed arguments + retval reference

// size of arguments on stack when pushed by value
AFX_STATIC_DATA const UINT _afxByValue[] =
{
	0,                          // VTS_EMPTY
	0,                          // VTS_NULL
	sizeof(_STACK_SHORT),       // VTS_I2
	sizeof(_STACK_LONG),        // VTS_I4
	sizeof(_STACK_FLOAT),       // VTS_R4
	sizeof(_STACK_DOUBLE),      // VTS_R8
	sizeof(CY),                 // VTS_CY
	sizeof(DATE),               // VTS_DATE
	sizeof(LPCOLESTR),          // VTS_WBSTR (VT_BSTR)
	sizeof(LPDISPATCH),         // VTS_DISPATCH
	sizeof(SCODE),              // VTS_SCODE
	sizeof(BOOL),               // VTS_BOOL
	sizeof(const VARIANT*),     // VTS_VARIANT
	sizeof(LPUNKNOWN),           // VTS_UNKNOWN
	sizeof(LPCSTR),             // VTS_BSTR (VT_BSTRA -- MFC defined)
	0,
	sizeof(_STACK_CHAR),        // VTS_I1
	sizeof(_STACK_CHAR),        // VTS_UI1
	sizeof(_STACK_SHORT),       // VTS_UI2
	sizeof(_STACK_LONG),        // VTS_UI4
	sizeof(_STACK_LONGLONG),    // VTS_I8
	sizeof(_STACK_LONGLONG)     // VTS_UI8
};

// size of arguments on stack when pushed by reference
AFX_STATIC_DATA const UINT _afxByRef[] =
{
	0,                          // VTS_PEMPTY
	0,                          // VTS_PNULL
	sizeof(short*),             // VTS_PI2
	sizeof(long*),              // VTS_PI4
	sizeof(float*),             // VTS_PR4
	sizeof(double*),            // VTS_PR8
	sizeof(CY*),                // VTS_PCY
	sizeof(DATE*),              // VTS_PDATE
	sizeof(BSTR*),              // VTS_PBSTR
	sizeof(LPDISPATCH*),        // VTS_PDISPATCH
	sizeof(SCODE*),             // VTS_PSCODE
	sizeof(VARIANT_BOOL*),      // VTS_PBOOL
	sizeof(VARIANT*),           // VTS_PVARIANT
	sizeof(LPUNKNOWN*),         // VTS_PUNKNOWN
	sizeof(LPCSTR*),            // VTS_BSTRA
	0,
	sizeof(char*),              // VTS_PI1
	sizeof(BYTE*),              // VTS_PUI1
	sizeof(WORD*),              // VTS_PUI2
	sizeof(DWORD*),             // VTS_PUI4
	sizeof(LONGLONG*),          // VTS_PI8
	sizeof(ULONGLONG*)          // VTS_PUI8
};

AFX_STATIC_DATA const UINT _afxRetVal[] =
{
	0,                          // VT_EMPTY
	0,                          // VT_NULL
	0,                          // VT_I2
	0,                          // VT_I4
	0,                          // VT_R4
	0,                          // VT_R8
	sizeof(CY*),                // VT_CY
	0,                          // VT_DATE (same as VT_R8)
	0,                          // VT_BSTR
	0,                          // VT_DISPATCH
	0,                          // VT_SCODE
	0,                          // VT_BOOL
	sizeof(VARIANT*),           // VT_VARIANT
	0,                          // VT_UNKNOWN
	0,                          // VT_BSTRA
	0,
	0,                          // VT_I1
	0,                          // VT_UI1
	0,                          // VT_UI2
	0,                          // VT_UI4
	0,                          // VT_I8
	0                           // VT_UI8
};

UINT PASCAL CCmdTarget::GetStackSize(const BYTE* pbParams, VARTYPE vtResult)
{
	// sizeof 'this' pointer
	UINT nCount = sizeof(CCmdTarget*);
#ifdef _ALIGN_STACK
	nCount = (nCount + (_ALIGN_STACK-1)) & ~(_ALIGN_STACK-1);
#endif

	// count bytes in return value
	ENSURE((UINT)vtResult < _countof(_afxRetVal));
	nCount += _afxRetVal[vtResult];
#ifdef _ALIGN_STACK
	nCount = (nCount + (_ALIGN_STACK-1)) & ~(_ALIGN_STACK-1);
#endif

	// count arguments
	ASSERT(pbParams != NULL);
	while (*pbParams != 0)
	{
		if (*pbParams != VT_MFCMARKER)
		{
			// align if necessary
			// get and add appropriate byte count
			const UINT* rgnBytes;
			if (*pbParams & VT_MFCBYREF)
				rgnBytes = _afxByRef;
			else
				rgnBytes = _afxByValue;
			ENSURE(((BYTE)(*pbParams & ~VT_MFCBYREF)) < _countof(_afxByValue));
#ifdef _ALIGN_DOUBLES
			// align doubles on 8 byte for some platforms
			if (*pbParams == VT_R8)
				nCount = (nCount + _ALIGN_DOUBLES-1) & ~(_ALIGN_DOUBLES-1);
#endif
			
			nCount += rgnBytes[(BYTE)(*pbParams & ~VT_MFCBYREF)];
#ifdef _ALIGN_STACK
			nCount = (nCount + (_ALIGN_STACK-1)) & ~(_ALIGN_STACK-1);
#endif
		}
		++pbParams;
	}
#if defined(_ALIGN_DOUBLES) && defined(_SHADOW_DOUBLES)
	// align doubles on 8 byte for some platforms
	nCount = (nCount + _ALIGN_DOUBLES-1) & ~(_ALIGN_DOUBLES-1);
#endif
	return nCount;
}

// push arguments on stack appropriate for C++ call (compiler dependent)
#ifndef _SHADOW_DOUBLES
SCODE CCmdTarget::PushStackArgs(BYTE* pStack, const BYTE* pbParams,
	void* pResult, VARTYPE vtResult, DISPPARAMS* pDispParams, UINT* puArgErr,
	VARIANT* rgTempVars,CVariantBoolConverter* pTempStackArgs)
#else
SCODE CCmdTarget::PushStackArgs(BYTE* pStack, const BYTE* pbParams,
	void* pResult, VARTYPE vtResult, DISPPARAMS* pDispParams, UINT* puArgErr,
	VARIANT* rgTempVars, UINT nSizeArgs,CVariantBoolConverter* pTempStackArgs)
#endif
{
	ASSERT(pStack != NULL);
	ASSERT(pResult != NULL);
	ASSERT(pDispParams != NULL);
	ASSERT(puArgErr != NULL);

#ifdef _SHADOW_DOUBLES
	double* pDoubleShadow = (double*)(pStack + nSizeArgs);
	double* pDoubleShadowMax = pDoubleShadow + _SHADOW_DOUBLES;
#endif

	// C++ member functions use the __thiscall convention, where parameters
	//  are pushed last to first.  Assuming the stack grows down, this means
	//  that the first parameter is at the lowest memory address in the
	//  stack frame and the last parameter is at the highest address.


#ifdef _RETVAL_FIRST
	// push any necessary return value stuff on the stack (pre args)
	//  (an ambient pointer is pushed to stack relative data)
	if (vtResult == VT_CY || vtResult == VT_VARIANT)
	{
#ifdef _ALIGN_STACK
		ASSERT(((DWORD_PTR)pStack & (_ALIGN_STACK-1)) == 0);
#endif
		*(_STACK_PTR*)pStack = (_STACK_PTR)pResult;
		pStack += sizeof(_STACK_PTR);
#ifdef _ALIGN_STACK
		ASSERT(((DWORD_PTR)pStack & (_ALIGN_STACK-1)) == 0);
#endif
	}
#endif //_RETVAL_FIRST

	// push the 'this' pointer
#ifdef _ALIGN_STACK
	ASSERT(((DWORD_PTR)pStack & (_ALIGN_STACK-1)) == 0);
#endif
	*(_STACK_PTR*)pStack = (_STACK_PTR)this;
	pStack += sizeof(_STACK_PTR);
#ifdef _ALIGN_STACK
	ASSERT(((DWORD_PTR)pStack & (_ALIGN_STACK-1)) == 0);
#endif

#ifndef _RETVAL_FIRST
	// push any necessary return value stuff on the stack (post args)
	//  (an ambient pointer is pushed to stack relative data)
	if (vtResult == VT_CY || vtResult == VT_VARIANT)
	{
#ifdef _ALIGN_STACK
		ASSERT(((DWORD_PTR)pStack & (_ALIGN_STACK-1)) == 0);
#endif
		*(_STACK_PTR*)pStack = (_STACK_PTR)pResult;
		pStack += sizeof(_STACK_PTR);
#ifdef _ALIGN_STACK
		ASSERT(((DWORD_PTR)pStack & (_ALIGN_STACK-1)) == 0);
#endif
	}
#endif //!_RETVAL_FIRST

	// push the arguments (first to last, low address to high address)
	VARIANT* pArgs = pDispParams->rgvarg;
	BOOL bNamedArgs = FALSE;
	int iArg = pDispParams->cArgs; // start with positional arguments
	int iArgMin = pDispParams->cNamedArgs;

	ASSERT(pbParams != NULL);
	const BYTE* pb;
	for (pb = pbParams; *pb != '\0'; ++pb)
	{
		--iArg; // move to next arg

		// convert MFC parameter type to IDispatch VARTYPE
		VARTYPE vt = *pb;
		if (vt != VT_MFCMARKER && (vt & VT_MFCBYREF))
			vt = (VARTYPE)((vt & ~VT_MFCBYREF) | VT_BYREF);

		VARIANT* pArg;
		if (iArg >= iArgMin)
		{
			// hit named args before using all positional args?
			if (vt == VT_MFCMARKER)
				break;

			// argument specified by caller -- use it
			pArg = &pArgs[iArg];
			if (vt != VT_VARIANT && vt != pArg->vt)
			{
				// argument is not of appropriate type, attempt to coerce it
				VARIANT* pArgTemp = &rgTempVars[iArg];
				ASSERT(pArgTemp->vt == VT_EMPTY);
#if defined(_UNICODE)
				VARTYPE vtTarget = vt;
#else
				VARTYPE vtTarget = (VARTYPE) ((vt == VT_BSTRA) ? VT_BSTR : vt);
#endif
				if (pArg->vt != vtTarget)
				{
					SCODE sc = VariantChangeType(pArgTemp, pArg, 0, vtTarget);
					if (FAILED(sc))
					{
						TRACE(traceOle, 0, "Warning: automation argument coercion failed.\n");
						*puArgErr = iArg;
						return sc;
					}
					ASSERT(pArgTemp->vt == vtTarget);
				}

#if !defined(_UNICODE)
				if (vt == VT_BSTRA)
				{
					if (pArg->vt != vtTarget)
					{
						// coerce above created a new string
						// convert it to ANSI and free it
						ASSERT(pArgTemp->vt == VT_BSTR);
						BSTR bstrW = pArgTemp->bstrVal;
						pArgTemp->bstrVal = AfxBSTR2ABSTR(bstrW);
						SysFreeString(bstrW);
					}
					else
					{
						// convert the string to ANSI from original
						pArgTemp->bstrVal = AfxBSTR2ABSTR(pArg->bstrVal);
						pArgTemp->vt = VT_BSTR;
					}
					vt = VT_BSTR;
				}
#endif
				pArg = pArgTemp;
			}
		}
		else
		{
			if (vt == VT_MFCMARKER)
			{
				// start processing named arguments
				iArg = pDispParams->cNamedArgs;
				iArgMin = 0;
				bNamedArgs = TRUE;
				continue;
			}

			if (bNamedArgs || vt != VT_VARIANT)
				break;  // function not expecting optional argument

			// argument not specified by caller -- provide default variant
			static VARIANT vaDefault;   // Note: really is 'const'
			vaDefault.vt = VT_ERROR;
			vaDefault.scode = DISP_E_PARAMNOTFOUND;
			pArg = &vaDefault;
		}

		// push parameter value on the stack
		if (vt&VT_BYREF)
		{
			_STACK_PTR pByRef=NULL;
			if (vt == (VT_BOOL|VT_BYREF) && pTempStackArgs!=NULL)
			{
				BOOL bVal=(*V_BOOLREF(pArg))==VARIANT_FALSE ? FALSE : TRUE;
				BOOL* pb=new BOOL(bVal);
				pTempStackArgs->AddPair( CVariantBoolPair(pb,V_BOOLREF(pArg)) );
				pByRef=pb;
			} else
			{
				pByRef=pArg->byref;
			}
			// by reference parameter
			*(_STACK_PTR*)pStack = pByRef;
			pStack += sizeof(_STACK_PTR);
		}
		else
		{
			// by value parameters
			switch (vt)
			{
			case VT_I1:
				*(_STACK_CHAR*)pStack = (signed char)pArg->cVal;
				pStack += sizeof(_STACK_CHAR);
				break;

			case VT_UI1:
				*(_STACK_CHAR*)pStack = pArg->bVal; // 'BYTE' is passed as 'int'
				pStack += sizeof(_STACK_CHAR);
				break;

			case VT_I2:
				*(_STACK_SHORT*)pStack = pArg->iVal;
				pStack += sizeof(_STACK_SHORT);   // 'short' is passed as 'int'
				break;

			case VT_UI2:
				*(_STACK_SHORT*)pStack = pArg->uiVal;
				pStack += sizeof(_STACK_SHORT);
				break;

			case VT_I4:
				*(_STACK_LONG*)pStack = pArg->lVal;
				pStack += sizeof(_STACK_LONG);
				break;

			case VT_UI4:
				*(_STACK_LONG*)pStack = pArg->ulVal;
				pStack += sizeof(_STACK_LONG);
				break;

			case VT_I8:
				*(_STACK_LONGLONG*)pStack = pArg->llVal;
				pStack += sizeof(_STACK_LONGLONG);
				break;

			case VT_UI8:
				*(_STACK_LONGLONG*)pStack = pArg->ullVal;
				pStack += sizeof(_STACK_LONGLONG);
				break;

			case VT_R4:
				*(_STACK_FLOAT*)pStack = (_STACK_FLOAT)pArg->fltVal;
				pStack += sizeof(_STACK_FLOAT);
	#ifdef _SHADOW_DOUBLES
				if (pDoubleShadow < pDoubleShadowMax)
					*pDoubleShadow++ = (double)pArg->fltVal;
	#endif
				break;

			case VT_R8:
	#ifdef _ALIGN_DOUBLES
				// align doubles on 8 byte for some platforms
				pStack = (BYTE*)(((DWORD_PTR)pStack + _ALIGN_DOUBLES-1) &
					~(_ALIGN_DOUBLES-1));
	#endif
				*(_STACK_DOUBLE*)pStack = (_STACK_DOUBLE)pArg->dblVal;
				pStack += sizeof(_STACK_DOUBLE);
	#ifdef _SHADOW_DOUBLES
				if (pDoubleShadow < pDoubleShadowMax)
					*pDoubleShadow++ = pArg->dblVal;
	#endif
				break;

			case VT_DATE:
	#ifdef _ALIGN_DOUBLES
				// align doubles on 8 byte for some platforms
				pStack = (BYTE*)(((DWORD_PTR)pStack + _ALIGN_DOUBLES-1) &
					~(_ALIGN_DOUBLES-1));
	#endif
				*(_STACK_DOUBLE*)pStack = (_STACK_DOUBLE)pArg->date;
				pStack += sizeof(_STACK_DOUBLE);
	#ifdef _SHADOW_DOUBLES
				if (pDoubleShadow < pDoubleShadowMax)
					*pDoubleShadow++ = pArg->date;
	#endif
				break;

			case VT_CY:
				*(CY*)pStack = pArg->cyVal;
				pStack += sizeof(CY);
				break;

			case VT_BSTR:
				*(_STACK_PTR*)pStack = (_STACK_PTR)pArg->bstrVal;
				pStack += sizeof(_STACK_PTR);
				break;

			case VT_ERROR:
				*(_STACK_LONG*)pStack = (_STACK_LONG)pArg->scode;
				pStack += sizeof(_STACK_LONG);
				break;

			case VT_BOOL:
				*(_STACK_LONG*)pStack = (_STACK_LONG)(V_BOOL(pArg) != 0);
				pStack += sizeof(_STACK_LONG);
				break;

			case VT_VARIANT:
				*(_STACK_PTR*)pStack = (_STACK_PTR)pArg;
				pStack += sizeof(_STACK_PTR);
				break;

			case VT_DISPATCH:
			case VT_UNKNOWN:
				*(_STACK_PTR*)pStack = (_STACK_PTR)pArg->punkVal;
				pStack += sizeof(_STACK_PTR);
				break;

			default:
				ASSERT(FALSE);
				break;
			}
		}

#ifdef _ALIGN_STACK
		// align stack as appropriate for next parameter
		pStack = (BYTE*)(((DWORD_PTR)pStack + (_ALIGN_STACK-1)) &
			~(_ALIGN_STACK-1));
		ASSERT(((DWORD_PTR)pStack & (_ALIGN_STACK-1)) == 0);
#endif
	}

	// check that all source arguments were consumed
	if (iArg > 0)
	{
		*puArgErr = iArg;
		return DISP_E_BADPARAMCOUNT;
	}
	// check that all target arguments were filled
	if (*pb != '\0')
	{
		*puArgErr = pDispParams->cArgs;
		return DISP_E_PARAMNOTOPTIONAL;
	}
	return S_OK;    // success!
}

// indirect call helper (see OLECALL.CPP for implementation)

extern "C" DWORD_PTR AFXAPI
_AfxDispatchCall(AFX_PMSG pfn, void* pArgs, UINT nSizeArgs);

// invoke standard method given IDispatch parameters/return value, etc.
SCODE CCmdTarget::CallMemberFunc(const AFX_DISPMAP_ENTRY* pEntry, WORD wFlags,
	VARIANT* pvarResult, DISPPARAMS* pDispParams, UINT* puArgErr)
{
	AFX_MANAGE_STATE(m_pModuleState);

	ASSERT(pEntry != NULL);
	ASSERT(pEntry->pfn != NULL);

	// special union used only to hold largest return value possible
	union AFX_RESULT
	{
		VARIANT vaVal;
		CY cyVal;
		float fltVal;
		double dblVal;
		DWORD nVal;
		ULONGLONG ullVal;
	};

	// get default function and parameters
	BYTE bNoParams = 0;
	const BYTE* pbParams = (const BYTE*)pEntry->lpszParams;
	if (pbParams == NULL)
		pbParams = &bNoParams;
	UINT nParams = lstrlenA((LPCSTR)pbParams);

	// get default function and return value information
	AFX_PMSG pfn = pEntry->pfn;
	VARTYPE vtResult = pEntry->vt;

	// make DISPATCH_PROPERTYPUT look like call with one extra parameter
	if (wFlags & (DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF))
	{
		if(!ATL::_ATL_SAFE_ALLOCA_IMPL::_AtlVerifyStackAvailable(nParams+3))
			return E_OUTOFMEMORY;
 #pragma warning(push)
 #pragma warning(disable:4068) //Disable unknown pragma warning that prefast pragma causes.
 #pragma prefast(push)
 #pragma prefast(disable:255, "Already validated that there is enough space on the stack for this allocation")
		BYTE* pbPropSetParams = (BYTE*)_alloca(nParams+3);
 #pragma prefast(pop)
 #pragma warning(pop)
			
		ASSERT(pbPropSetParams != NULL);    // stack overflow?

		ASSERT(!(pEntry->vt & VT_BYREF));
		Checked::memcpy_s(pbPropSetParams, nParams+3, pbParams, nParams);
		pbParams = pbPropSetParams;

		VARTYPE vtProp = pEntry->vt;
#if !defined(_UNICODE)
		if (vtProp == VT_BSTR)
			vtProp = VT_BSTRA;
#endif
		// VT_MFCVALUE serves serves as marker denoting start of named params
		pbPropSetParams[nParams++] = (BYTE)VT_MFCMARKER;
		pbPropSetParams[nParams++] = (BYTE)vtProp;
		pbPropSetParams[nParams] = 0;

		// call "set" function instead of "get"
		ASSERT(pEntry->pfnSet != NULL);
		pfn = pEntry->pfnSet;
		vtResult = VT_EMPTY;
	}

	// allocate temporary space for VARIANT temps created by VariantChangeType
	if(!ATL::_ATL_SAFE_ALLOCA_IMPL::_AtlVerifyStackAvailable(pDispParams->cArgs * sizeof(VARIANT)))
			return E_OUTOFMEMORY;
	if (pDispParams->cArgs > (INT_MAX / sizeof(VARIANT)))
			return E_OUTOFMEMORY;
#pragma warning(push)
#pragma warning(disable:4068) //Disable unknown pragma warning that prefast pragma causes.
#pragma prefast(push)
#pragma prefast(disable:255, "Already validated that there is enough space on the stack for this allocation")
	VARIANT* rgTempVars =
		(VARIANT*)_alloca(pDispParams->cArgs * sizeof(VARIANT));
#pragma prefast(pop)
#pragma warning(pop)

	
	memset(rgTempVars, 0, pDispParams->cArgs * sizeof(VARIANT));

	// determine size of arguments and allocate stack space
	UINT nSizeArgs = GetStackSize(pbParams, vtResult);
	ASSERT(nSizeArgs != 0);
#pragma warning( push )
#pragma warning( disable: 4296 )
	if (nSizeArgs < _STACK_MIN)
#pragma warning( pop )
		nSizeArgs = _STACK_MIN;
	
	if(!ATL::_ATL_SAFE_ALLOCA_IMPL::_AtlVerifyStackAvailable(nSizeArgs + _SCRATCH_SIZE))
			return E_OUTOFMEMORY;
#pragma warning(push)
#pragma warning(disable:4068) //Disable unknown pragma warning that prefast pragma causes.
#pragma prefast(push)
#pragma prefast(disable:255, "Already validated that there is enough space on the stack for this allocation")
	BYTE* pStack = (BYTE*)_alloca(nSizeArgs + _SCRATCH_SIZE);
#pragma prefast(pop)
#pragma warning(pop)

	

	// push all the args on to the stack allocated memory
	AFX_RESULT result;
	CVariantBoolConverter tempArgs;	
#ifndef _SHADOW_DOUBLES
	SCODE sc = PushStackArgs(pStack, pbParams, &result, vtResult,
		pDispParams, puArgErr, rgTempVars,&tempArgs);
#else
	SCODE sc = PushStackArgs(pStack, pbParams, &result, vtResult,
		pDispParams, puArgErr, rgTempVars, nSizeArgs,&tempArgs);
#endif
	pStack += _STACK_OFFSET;

	DWORD_PTR dwResult = 0;
	if (sc == S_OK)
	{
		TRY
		{
			// PushStackArgs will fail on argument mismatches
			DWORD_PTR (AFXAPI *pfnDispatch)(AFX_PMSG, void*, UINT) =
				&_AfxDispatchCall;

			// floating point return values are a special case
			switch (vtResult)
			{
			case VT_R4:
				result.fltVal = ((float (AFXAPI*)(AFX_PMSG, void*, UINT))
					pfnDispatch)(pfn, pStack, nSizeArgs);
				break;
			case VT_R8:
				result.dblVal = ((double (AFXAPI*)(AFX_PMSG, void*, UINT))
					pfnDispatch)(pfn, pStack, nSizeArgs);
				break;
			case VT_DATE:
				result.dblVal = ((DATE (AFXAPI*)(AFX_PMSG, void*, UINT))
					pfnDispatch)(pfn, pStack, nSizeArgs);
				break;

			case VT_I8:
			case VT_UI8:
				result.ullVal = ((ULONGLONG (AFXAPI*)(AFX_PMSG, void*, UINT))
					pfnDispatch)(pfn, pStack, nSizeArgs);
				break;

			default:
				dwResult = pfnDispatch(pfn, pStack, nSizeArgs);
				break;
			}
		}
		CATCH_ALL(e)
		{
			// free temporaries created by VariantChangeType
			for (UINT iArg = 0; iArg < pDispParams->cArgs; ++iArg)
				VariantClear(&rgTempVars[iArg]);

			THROW_LAST();
		}
		END_CATCH_ALL
	}

	// free temporaries created by VariantChangeType
	for (UINT iArg = 0; iArg < pDispParams->cArgs; ++iArg)
		VariantClear(&rgTempVars[iArg]);

	// handle error during PushStackParams
	if (sc != S_OK)
		return sc;

	//Copy the values from BOOL* stack params into the VARIANT_BOOLs passed
	//in DISPPARAMS.
	tempArgs.CopyBOOLsIntoVarBools();
	// property puts don't touch the return value
	if (pvarResult != NULL)
	{
		// clear pvarResult just in case
		pvarResult->vt = vtResult;

		// build return value VARIANT from result union
		switch (vtResult)
		{
		case VT_I1:
		case VT_UI1:
			pvarResult->bVal = (BYTE)dwResult;
			break;
		case VT_I2:
		case VT_UI2:
			pvarResult->iVal = (short)dwResult;
			break;
		case VT_I4:
		case VT_UI4:
			pvarResult->lVal = (long)dwResult;
			break;

		case VT_I8:
		case VT_UI8:
			pvarResult->ullVal = result.ullVal;
			break;

		case VT_R4:
			pvarResult->fltVal = result.fltVal;
			break;
		case VT_R8:
			pvarResult->dblVal = result.dblVal;
			break;
		case VT_CY:
			pvarResult->cyVal = result.cyVal;
			break;
		case VT_DATE:
			pvarResult->date = result.dblVal;
			break;
		case VT_BSTR:
			pvarResult->bstrVal = reinterpret_cast<BSTR>(dwResult);
			break;
		case VT_ERROR:
			pvarResult->scode = (SCODE)dwResult;
			break;
		case VT_BOOL:
			{
			//VT_BOOL return value may be BOOL (4 bytes) or VARIANT_BOOL (2 bytes).
			//Test only lower 2 bytes (0 - false, otherwise - true).
			//BOOL users should return only TRUE or FALSE.
			VARIANT_BOOL sBool=(VARIANT_BOOL)dwResult;
			V_BOOL(pvarResult) = (sBool != 0 ? VARIANT_TRUE : VARIANT_FALSE);			
			break;
			}
		case VT_VARIANT:
			*pvarResult = result.vaVal;
			break;
		case VT_DISPATCH:
		case VT_UNKNOWN:
			pvarResult->punkVal = reinterpret_cast<LPUNKNOWN>(dwResult); // already AddRef
			break;
		}
	}
	else
	{
		// free unused return value
		switch (vtResult)
		{
		case VT_BSTR:
            ::SysFreeString(reinterpret_cast<BSTR>(dwResult));
			break;
		case VT_DISPATCH:
		case VT_UNKNOWN:
			if (reinterpret_cast<LPUNKNOWN>(dwResult) != 0)
				reinterpret_cast<LPUNKNOWN>(dwResult)->Release();
			break;
		case VT_VARIANT:
			VariantClear(&result.vaVal);
			break;
		}
	}

	return S_OK;    // success!
}

/////////////////////////////////////////////////////////////////////////////
// CCmdTarget::XDispatch implementation

STDMETHODIMP_(ULONG) COleDispatchImpl::AddRef()
{
	METHOD_PROLOGUE_EX_(CCmdTarget, Dispatch)
	return pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) COleDispatchImpl::Release()
{
	METHOD_PROLOGUE_EX_(CCmdTarget, Dispatch)
	return pThis->ExternalRelease();
}

STDMETHODIMP COleDispatchImpl::QueryInterface(REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE_EX_(CCmdTarget, Dispatch)
	return pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP COleDispatchImpl::GetTypeInfoCount(UINT* pctinfo)
{
	METHOD_PROLOGUE_EX_(CCmdTarget, Dispatch)
	*pctinfo = pThis->GetTypeInfoCount();
	return S_OK;
}

STDMETHODIMP COleDispatchImpl::GetTypeInfo(UINT itinfo, LCID lcid,
	ITypeInfo** pptinfo)
{
	METHOD_PROLOGUE_EX_(CCmdTarget, Dispatch)
	ASSERT_POINTER(pptinfo, LPTYPEINFO);

	if (itinfo != 0)
		return DISP_E_BADINDEX;

	IID iid;
	if (!pThis->GetDispatchIID(&iid))
		return DISP_E_BADINDEX;

	return pThis->GetTypeInfoOfGuid(lcid, iid, pptinfo);
}

STDMETHODIMP COleDispatchImpl::GetIDsOfNames(
	REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid)
{
	METHOD_PROLOGUE_EX_(CCmdTarget, Dispatch)
	ASSERT_POINTER(rgszNames, char*);
	ASSERT_POINTER(rgdispid, DISPID);

	// check arguments
	if (riid != IID_NULL)
		return DISP_E_UNKNOWNINTERFACE;

	SCODE sc;
	LPTYPEINFO lpTypeInfo = NULL;
	if (lcid != 0 && SUCCEEDED(sc = GetTypeInfo(0, lcid, &lpTypeInfo)))
	{
		// For non-zero lcid, let typeinfo do the work (when available)
		ASSERT(lpTypeInfo != NULL);
		sc = lpTypeInfo->GetIDsOfNames(rgszNames, cNames, rgdispid);
		lpTypeInfo->Release();
		if (sc == TYPE_E_ELEMENTNOTFOUND)
			sc = DISP_E_UNKNOWNNAME;
	}
	else
	{
		// fill in the member name
		const AFX_DISPMAP* pDerivMap = pThis->GetDispatchMap();
		CString strName(rgszNames[0]);
		rgdispid[0] = pThis->MemberIDFromName(pDerivMap, strName.GetString());
		if (rgdispid[0] == DISPID_UNKNOWN)
			sc = DISP_E_UNKNOWNNAME;
		else
			sc = S_OK;

		// argument names are always DISPID_UNKNOWN (for this implementation)
		for (UINT nIndex = 1; nIndex < cNames; nIndex++)
			rgdispid[nIndex] = DISPID_UNKNOWN;
	}

	return sc;
}

STDMETHODIMP COleDispatchImpl::Invoke(
	DISPID dispid, REFIID riid, LCID lcid,
	WORD wFlags, DISPPARAMS* pDispParams, LPVARIANT pvarResult,
	LPEXCEPINFO pexcepinfo, UINT* puArgErr)
{
	METHOD_PROLOGUE_EX_(CCmdTarget, Dispatch)
	ASSERT_NULL_OR_POINTER(pvarResult, VARIANT);
	ASSERT_NULL_OR_POINTER(pexcepinfo, EXCEPINFO);
	ASSERT_NULL_OR_POINTER(puArgErr, UINT);
	ASSERT(pDispParams != NULL);

	// check argument
	if(pDispParams == NULL)
	{
		return E_INVALIDARG;
	}

	// make sure pvarResult is initialized
	if (pvarResult != NULL)
		AfxVariantInit(pvarResult);

	// check arguments
	if (riid != IID_NULL)
		return DISP_E_UNKNOWNINTERFACE;

	// allow subclass to disable Invoke
	if (!pThis->IsInvokeAllowed(dispid))
		return E_UNEXPECTED;

	// copy param block for safety
	DISPPARAMS params = *pDispParams;
	pDispParams = &params;

	// most of the time, named arguments are not supported
	if (pDispParams->cNamedArgs != 0)
	{
		// only special PROPERTYPUT named argument is allowed
		if (pDispParams->cNamedArgs != 1 ||
			pDispParams->rgdispidNamedArgs[0] != DISPID_PROPERTYPUT)
		{
			return DISP_E_NONAMEDARGS;
		}
	}

	// get entry for the member ID
	const AFX_DISPMAP_ENTRY* pEntry = pThis->GetDispEntry(dispid);
	if (pEntry == NULL)
		return DISP_E_MEMBERNOTFOUND;

	// treat member calls on properties just like property get/set
	if ((wFlags == DISPATCH_METHOD) &&
		((pEntry->pfn == NULL && pEntry->pfnSet == NULL) ||
		 (pEntry->pfn == NULL && pEntry->pfnSet != NULL) ||
		 (pEntry->pfn != NULL && pEntry->pfnSet != NULL)))
	{
		// the entry describes a property but a method call is being
		//  attempted -- change it to a property get/set based on the
		//  number of parameters being passed.
		wFlags &= ~DISPATCH_METHOD;
		UINT nExpectedArgs = pEntry->lpszParams != NULL ?
			(UINT)lstrlenA(pEntry->lpszParams) : 0;
		if (pDispParams->cArgs <= nExpectedArgs)
		{
			// no extra param -- so treat as property get
			wFlags |= DISPATCH_PROPERTYGET;
		}
		else
		{
			// extra params -- treat as property set
			wFlags |= DISPATCH_PROPERTYPUTREF;
			pDispParams->cNamedArgs = 1;
		}
	}

	// property puts should not require a return value
	if (wFlags & (DISPATCH_PROPERTYPUTREF|DISPATCH_PROPERTYPUT))
	{
		pvarResult = NULL;
		// catch attempt to do property set on method
		if (pEntry->pfn != NULL && pEntry->pfnSet == NULL)
			return DISP_E_TYPEMISMATCH;
	}

	UINT uArgErr = (UINT)-1;    // no error yet
	SCODE sc = S_OK;

	// handle special cases of DISPATCH_PROPERTYPUT
	VARIANT* pvarParamSave = NULL;
	VARIANT vaParamSave;
	vaParamSave.vt = VT_ERROR;

	DISPPARAMS paramsTemp;
	VARIANT vaTemp;
	AfxVariantInit(&vaTemp);

	if (wFlags == DISPATCH_PROPERTYPUT)
	{
		// with PROPERTYPUT (no REF), the right hand side may need fixup
		if (pDispParams->rgvarg[0].vt == VT_DISPATCH &&
			pDispParams->rgvarg[0].pdispVal != NULL)
		{
			// remember old value for restore later
			pvarParamSave = &pDispParams->rgvarg[0];
			vaParamSave = pDispParams->rgvarg[0];
			AfxVariantInit(&pDispParams->rgvarg[0]);

			// get default value of right hand side
			memset(&paramsTemp, 0, sizeof(DISPPARAMS));
			sc = vaParamSave.pdispVal->Invoke(
				DISPID_VALUE, riid, lcid, DISPATCH_PROPERTYGET, &paramsTemp,
				&pDispParams->rgvarg[0], pexcepinfo, puArgErr);
		}

		// special handling for PROPERTYPUT (no REF), left hand side
		if (sc == S_OK && pEntry->vt == VT_DISPATCH)
		{
			memset(&paramsTemp, 0, sizeof(DISPPARAMS));

			// parameters are distributed depending on what the Get expects
			if (pEntry->lpszParams == NULL)
			{
				// paramsTemp is already setup for no parameters
				sc = Invoke(dispid, riid, lcid,
					DISPATCH_PROPERTYGET|DISPATCH_METHOD, &paramsTemp,
					&vaTemp, pexcepinfo, puArgErr);
				if (sc == S_OK &&
					(vaTemp.vt != VT_DISPATCH || vaTemp.pdispVal == NULL))
					sc = DISP_E_TYPEMISMATCH;
				else if (sc == S_OK)
				{
					ASSERT(vaTemp.vt == VT_DISPATCH && vaTemp.pdispVal != NULL);
					// we have the result, now call put on the default property
					sc = vaTemp.pdispVal->Invoke(
						DISPID_VALUE, riid, lcid, wFlags, pDispParams,
						pvarResult, pexcepinfo, puArgErr);
				}
			}
			else
			{
				// pass all but named params
				paramsTemp.rgvarg = &pDispParams->rgvarg[1];
				paramsTemp.cArgs = pDispParams->cArgs - 1;
				sc = Invoke(dispid, riid, lcid,
					DISPATCH_PROPERTYGET|DISPATCH_METHOD, &paramsTemp,
					&vaTemp, pexcepinfo, puArgErr);
				if (sc == S_OK &&
					(vaTemp.vt != VT_DISPATCH || vaTemp.pdispVal == NULL))
					sc = DISP_E_TYPEMISMATCH;
				else if (sc == S_OK)
				{
					ASSERT(vaTemp.vt == VT_DISPATCH && vaTemp.pdispVal != NULL);

					// we have the result, now call put on the default property
					paramsTemp = *pDispParams;
					paramsTemp.cArgs = paramsTemp.cNamedArgs;
					sc = vaTemp.pdispVal->Invoke(
						DISPID_VALUE, riid, lcid, wFlags, &paramsTemp,
						pvarResult, pexcepinfo, puArgErr);
				}
			}
			VariantClear(&vaTemp);

			if (sc != DISP_E_MEMBERNOTFOUND)
				goto Cleanup;
		}

		if (sc != S_OK && sc != DISP_E_MEMBERNOTFOUND)
			goto Cleanup;
	}

	// ignore DISP_E_MEMBERNOTFOUND from above
	ASSERT(sc == DISP_E_MEMBERNOTFOUND || sc == S_OK);

	// undo implied default value on right hand side on error
	if (sc != S_OK && pvarParamSave != NULL)
	{
		// default value stuff failed -- so try without default value
		pvarParamSave = NULL;
		VariantClear(&pDispParams->rgvarg[0]);
		pDispParams->rgvarg[0] = vaParamSave;
	}
	sc = S_OK;

	// check arguments against this entry
	UINT nOrigArgs; nOrigArgs = pDispParams->cArgs;
	if (wFlags & (DISPATCH_PROPERTYGET|DISPATCH_METHOD))
	{
		if (!(wFlags & DISPATCH_METHOD))
		{
			if (pEntry->vt == VT_EMPTY)
				return DISP_E_BADPARAMCOUNT;
			if (pvarResult == NULL)
				return DISP_E_PARAMNOTOPTIONAL;
		}
		if (pEntry->lpszParams == NULL && pDispParams->cArgs > 0)
		{
			if (pEntry->vt != VT_DISPATCH)
				return DISP_E_BADPARAMCOUNT;

			// it is VT_DISPATCH property/method but too many arguments supplied
			// transfer those arguments to the default property of the return value
			// after getting the return value from this call.  This is referred
			// to as collection lookup.
			pDispParams->cArgs = 0;
			if (pvarResult == NULL)
				pvarResult = &vaTemp;
		}
	}

	// make sure that parameters are not passed to a simple property
	if (pDispParams->cArgs > 1 &&
		(wFlags & (DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF)) &&
		pEntry->pfn == NULL)
	{
		sc = DISP_E_BADPARAMCOUNT;
		goto Cleanup;
	}

	// make sure that pvarResult is set for simple property get
	if (pEntry->pfn == NULL && pDispParams->cArgs == 0 && pvarResult == NULL)
	{
		sc = DISP_E_PARAMNOTOPTIONAL;
		goto Cleanup;
	}

	// make sure IsExpectingResult returns FALSE as appropriate
	BOOL bResultExpected;
	bResultExpected = pThis->m_bResultExpected;
	pThis->m_bResultExpected = pvarResult != NULL;

	TRY
	{
		if (pEntry->pfn == NULL)
		{
			// do standard property get/set
			if (pDispParams->cArgs == 0)
				pThis->GetStandardProp(pEntry, pvarResult, &uArgErr);
			else
				sc = pThis->SetStandardProp(pEntry, pDispParams, &uArgErr);
		}
		else
		{
			// do standard method call
			sc = pThis->CallMemberFunc(pEntry, wFlags,
				pvarResult, pDispParams, &uArgErr);
		}
	}
	CATCH(COleException, e)
	{
		sc = e->m_sc;
		DELETE_EXCEPTION(e);
	}
	AND_CATCH_ALL(e)
	{
		AFX_MANAGE_STATE(pThis->m_pModuleState);
		if (pexcepinfo != NULL)
		{
			// fill exception with translation of MFC exception
			COleDispatchException::Process(pexcepinfo, e);
		}
		DELETE_EXCEPTION(e);
		sc = DISP_E_EXCEPTION;
	}
	END_CATCH_ALL

	// restore original m_bResultExpected flag
	pThis->m_bResultExpected = bResultExpected;

	// handle special DISPATCH_PROPERTYGET collection lookup case
	if (sc == S_OK && nOrigArgs > pDispParams->cArgs)
	{
		ASSERT(wFlags & (DISPATCH_PROPERTYGET|DISPATCH_METHOD));
		ASSERT(pvarResult != NULL);
		// must be non-NULL dispatch, otherwise type mismatch
		if (pvarResult->vt != VT_DISPATCH || pvarResult->pdispVal == NULL)
		{
			sc = DISP_E_TYPEMISMATCH;
			goto Cleanup;
		}
		// otherwise, valid VT_DISPATCH was returned
		pDispParams->cArgs = nOrigArgs;
		LPDISPATCH lpTemp = pvarResult->pdispVal;
		if (pvarResult != &vaTemp)
			AfxVariantInit(pvarResult);
		else
			pvarResult = NULL;
		sc = lpTemp->Invoke(DISPID_VALUE, riid, lcid, wFlags,
			pDispParams, pvarResult, pexcepinfo, puArgErr);
		lpTemp->Release();
	}

Cleanup:
	// restore any arguments which were modified
	if (pvarParamSave != NULL)
	{
		VariantClear(&pDispParams->rgvarg[0]);
		pDispParams->rgvarg[0] = vaParamSave;
	}

	// fill error argument if one is available
	if (sc != S_OK && puArgErr != NULL && uArgErr != -1)
		*puArgErr = uArgErr;

	return sc;
}

/////////////////////////////////////////////////////////////////////////////
// IDispatch specific exception

COleDispatchException::~COleDispatchException()
{
	// destructor code is compiler generated
}

void PASCAL COleDispatchException::Process(
	EXCEPINFO* pInfo, const CException* pAnyException)
{

	ASSERT(AfxIsValidAddress(pInfo, sizeof(EXCEPINFO)));
	ASSERT_VALID(pAnyException);

	// zero default & reserved members
	memset(pInfo, 0, sizeof(EXCEPINFO));

	// get description based on type of exception
	TCHAR szDescription[256];
	LPCTSTR pszDescription = szDescription;
	if (pAnyException->IsKindOf(RUNTIME_CLASS(COleDispatchException)))
	{
		// specific IDispatch style exception
		COleDispatchException* e = (COleDispatchException*)pAnyException;
		pszDescription = e->m_strDescription;
		pInfo->wCode = e->m_wCode;
		pInfo->dwHelpContext = e->m_dwHelpContext;
		pInfo->scode = e->m_scError;

		// propagate source and help file if present
		if (!e->m_strHelpFile.IsEmpty())
			pInfo->bstrHelpFile = ::SysAllocString(CStringW(e->m_strHelpFile).GetString());
		if (!e->m_strSource.IsEmpty())
			pInfo->bstrSource = ::SysAllocString(CStringW(e->m_strSource).GetString());
	}
	else if (pAnyException->IsKindOf(RUNTIME_CLASS(CMemoryException)))
	{
		// failed memory allocation
		AfxLoadString(AFX_IDP_FAILED_MEMORY_ALLOC, szDescription);
		pInfo->wCode = AFX_IDP_FAILED_MEMORY_ALLOC;
	}
	else
	{
		// other unknown/uncommon error
		AfxLoadString(AFX_IDP_INTERNAL_FAILURE, szDescription);
		pInfo->wCode = AFX_IDP_INTERNAL_FAILURE;
	}

	// build up rest of EXCEPINFO struct
	pInfo->bstrDescription = ::SysAllocString(CStringW(pszDescription).GetString());
	if (pInfo->bstrSource == NULL)
		pInfo->bstrSource = ::SysAllocString(CStringW(AfxGetAppName()).GetString());
	if (pInfo->bstrHelpFile == NULL && pInfo->dwHelpContext != 0)
		pInfo->bstrHelpFile = ::SysAllocString(CStringW(AfxGetApp()->m_pszHelpFilePath).GetString());
}

COleDispatchException::COleDispatchException(
	LPCTSTR lpszDescription, UINT nHelpID, WORD wCode)
{
	m_dwHelpContext = nHelpID != 0 ? HID_BASE_DISPATCH+nHelpID : 0;
	m_wCode = wCode;
	if (lpszDescription != NULL)
		m_strDescription = lpszDescription;
	m_scError = wCode != 0 ? NOERROR : E_UNEXPECTED;
}

BOOL COleDispatchException::GetErrorMessage(_Out_z_cap_(nMaxError) LPTSTR lpszError, _In_ UINT nMaxError,
	_Out_opt_ PUINT pnHelpContext) const
{
	ASSERT(lpszError != NULL && AfxIsValidString(lpszError, nMaxError));

	if (pnHelpContext != NULL)
		*pnHelpContext = 0;

	Checked::tcsncpy_s(lpszError, nMaxError, m_strDescription, _TRUNCATE);
	return TRUE;
}

void AFXAPI AfxThrowOleDispatchException(WORD wCode, LPCTSTR lpszDescription,
	UINT nHelpID)
{
	ASSERT(AfxIsValidString(lpszDescription));
	THROW(new COleDispatchException(lpszDescription, nHelpID, wCode));
}

void AFXAPI AfxThrowOleDispatchException(WORD wCode, UINT nDescriptionID,
	UINT nHelpID)
{
	TCHAR szBuffer[256];
	VERIFY(AfxLoadString(nDescriptionID, szBuffer) != 0);
	if (nHelpID == -1)
		nHelpID = nDescriptionID;
	THROW(new COleDispatchException(szBuffer, nHelpID, wCode));
}


IMPLEMENT_DYNAMIC(COleDispatchException, CException)

/////////////////////////////////////////////////////////////////////////////
