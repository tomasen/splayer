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
// Runtime Typing

// special runtime-class structure for CObject (no base class)
const struct CRuntimeClass CObject::classCObject =
	{ "CObject", sizeof(CObject), 0xffff, NULL, NULL, NULL };

CRuntimeClass* CObject::GetRuntimeClass() const
{
	return _RUNTIME_CLASS(CObject);
}

#ifdef _AFXDLL
CRuntimeClass* PASCAL CObject::_GetBaseClass()
{
	return NULL;
}
CRuntimeClass* PASCAL CObject::GetThisClass()
{
	return _RUNTIME_CLASS(CObject);
}
#endif

BOOL CObject::IsKindOf(const CRuntimeClass* pClass) const
{
	ENSURE(this != NULL);
	// it better be in valid memory, at least for CObject size
	ASSERT(AfxIsValidAddress(this, sizeof(CObject)));

	// simple SI case
	CRuntimeClass* pClassThis = GetRuntimeClass();

	ENSURE(pClassThis);
	return pClassThis->IsDerivedFrom(pClass);
}

CObject* AFX_CDECL AfxDynamicDownCast(CRuntimeClass* pClass, CObject* pObject)
{
	if (pObject != NULL && pObject->IsKindOf(pClass))
		return pObject;
	else
		return NULL;
}

#ifdef _DEBUG
CObject* AFX_CDECL AfxStaticDownCast(CRuntimeClass* pClass, CObject* pObject)
{
	ASSERT(pObject == NULL || pObject->IsKindOf(pClass));
	return pObject;
}
#endif

/////////////////////////////////////////////////////////////////////////////
// Diagnostic Support

#ifdef _DEBUG
void AFXAPI AfxAssertValidObject(const CObject* pOb,
	LPCSTR lpszFileName, int nLine)
{
	if (pOb == NULL)
	{
		TRACE(traceAppMsg, 0, "ASSERT_VALID fails with NULL pointer.\n");
		if (AfxAssertFailedLine(lpszFileName, nLine))
			AfxDebugBreak();
		return;     // quick escape
	}
	if (!AfxIsValidAddress(pOb, sizeof(CObject)))
	{
		TRACE(traceAppMsg, 0, "ASSERT_VALID fails with illegal pointer.\n");
		if (AfxAssertFailedLine(lpszFileName, nLine))
			AfxDebugBreak();
		return;     // quick escape
	}

	// check to make sure the VTable pointer is valid
	ASSERT(sizeof(CObject) == sizeof(void*));
	if (!AfxIsValidAddress(*(void**)pOb, sizeof(void*), FALSE))
	{
		TRACE(traceAppMsg, 0, "ASSERT_VALID fails with illegal vtable pointer.\n");
		if (AfxAssertFailedLine(lpszFileName, nLine))
			AfxDebugBreak();
		return;     // quick escape
	}

	if (!AfxIsValidAddress(pOb, pOb->GetRuntimeClass()->m_nObjectSize, FALSE))
	{
		TRACE(traceAppMsg, 0, "ASSERT_VALID fails with illegal pointer.\n");
		if (AfxAssertFailedLine(lpszFileName, nLine))
			AfxDebugBreak();
		return;     // quick escape
	}
	pOb->AssertValid();
}

void CObject::AssertValid() const
{
	ASSERT(this != NULL);
}

void CObject::Dump(CDumpContext& dc) const
{
	dc << "a " << GetRuntimeClass()->m_lpszClassName <<
		" at " << (void*)this << "\n";

	UNUSED(dc); // unused in release build
}
#endif //_DEBUG

////////////////////////////////////////////////////////////////////////////
// Allocation/Creation

CObject* CRuntimeClass::CreateObject()
{
	ENSURE(this);

	if (m_pfnCreateObject == NULL)
	{
		TRACE(traceAppMsg, 0,
			_T("Error: Trying to create object which is not ")
			_T("DECLARE_DYNCREATE \nor DECLARE_SERIAL: %hs.\n"),
			m_lpszClassName);
		return NULL;
	}

	CObject* pObject = NULL;
	TRY
	{
		pObject = (*m_pfnCreateObject)();
	}
	END_TRY

	return pObject;
}

////////////////////////////////////////////////////////////////////////////
// Class loader & class serialization

BOOL CObject::IsSerializable() const
{
	return (GetRuntimeClass()->m_wSchema != 0xffff);
}

void AFXAPI AfxClassInit(CRuntimeClass* pNewClass)
{
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	AfxLockGlobals(CRIT_RUNTIMECLASSLIST);
	pModuleState->m_classList.AddHead(pNewClass);
	AfxUnlockGlobals(CRIT_RUNTIMECLASSLIST);
}

BOOL CRuntimeClass::IsDerivedFrom(const CRuntimeClass* pBaseClass) const
{
	ENSURE(this != NULL);
	ASSERT(AfxIsValidAddress(this, sizeof(CRuntimeClass), FALSE));
	ENSURE(pBaseClass != NULL);
	ASSERT(AfxIsValidAddress(pBaseClass, sizeof(CRuntimeClass), FALSE));

	// simple SI case
	const CRuntimeClass* pClassThis = this;
#ifdef _AFXDLL
	for (;;)
#else
	while (pClassThis != NULL)
#endif
	{
		if (pClassThis == pBaseClass)
			return TRUE;
#ifdef _AFXDLL
		if (pClassThis->m_pfnGetBaseClass == NULL)
			break;
		pClassThis = (*pClassThis->m_pfnGetBaseClass)();
#else
		pClassThis = pClassThis->m_pBaseClass;
#endif
	}
	return FALSE;       // walked to the top, no match
}

////////////////////////////////////////////////////////////////////////////
// CRuntimeClass special diagnostics

#ifdef _DEBUG
void AFXAPI AfxDoForAllClasses(void (AFX_CDECL *pfn)(const CRuntimeClass*, void*),
	void* pContext)
{
	// just walk through the simple list of registered classes
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	AfxLockGlobals(CRIT_RUNTIMECLASSLIST);
	CRuntimeClass* pClass;
	for (pClass = pModuleState->m_classList; pClass != NULL;
		pClass = pClass->m_pNextClass)
	{
		(*pfn)(pClass, pContext);
	}
	AfxUnlockGlobals(CRIT_RUNTIMECLASSLIST);
#ifdef _AFXDLL
	// walk through the list of dynlink library registered classes
	AfxLockGlobals(CRIT_DYNLINKLIST);
	for (CDynLinkLibrary* pDLL = pModuleState->m_libraryList; pDLL != NULL;
		pDLL = pDLL->m_pNextDLL)
	{
        for (CRuntimeClass* pDLLClass = pDLL->m_classList; pDLLClass != NULL;
			pDLLClass = pDLLClass->m_pNextClass)
		{
			(*pfn)(pDLLClass, pContext);
		}
	}
	AfxUnlockGlobals(CRIT_DYNLINKLIST);
#endif
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
