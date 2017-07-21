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
// Basic synchronization object

CSyncObject::CSyncObject(LPCTSTR pstrName)
{
	UNUSED(pstrName);   // unused in release builds

	m_hObject = NULL;

#ifdef _DEBUG
	m_strName = pstrName;
#endif
}

CSyncObject::~CSyncObject()
{
	if (m_hObject != NULL)
	{
		::CloseHandle(m_hObject);
		m_hObject = NULL;
	}
}

BOOL CSyncObject::Lock(DWORD dwTimeout)
{
	DWORD dwRet = ::WaitForSingleObject(m_hObject, dwTimeout);
	if (dwRet == WAIT_OBJECT_0 || dwRet == WAIT_ABANDONED)
		return TRUE;
	else
		return FALSE;
}

#ifdef _DEBUG

void CSyncObject::Dump(CDumpContext& dc) const
{
	dc << "Object ";
	dc << m_hObject;
	dc << " named " << m_strName << "\n";
	CObject::Dump(dc);
}

void CSyncObject::AssertValid() const
{
	CObject::AssertValid();
}

#endif

//////////////////////////////////////////////////////////////////////////////
// Inline function declarations expanded out-of-line

#ifndef _AFX_ENABLE_INLINES

#define _AFXMT_INLINE
#include "afxmt.inl"

#endif


IMPLEMENT_DYNAMIC(CCriticalSection, CSyncObject)
IMPLEMENT_DYNAMIC(CSyncObject, CObject)

/////////////////////////////////////////////////////////////////////////////
