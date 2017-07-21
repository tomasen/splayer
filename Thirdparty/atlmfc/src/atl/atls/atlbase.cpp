// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the	
// Active Template Library product.

#include "StdAfx.H"

#pragma warning( disable: 4073 )  // initializers put in library initialization area

#ifndef _delayimp_h
extern "C" IMAGE_DOS_HEADER __ImageBase;
#endif

namespace ATL
{

// {B62F5910-6528-11d1-9611-0000F81E0D0D}
extern "C" const GUID GUID_ATLVer30 = { 0xb62f5910, 0x6528, 0x11d1, { 0x96, 0x11, 0x0, 0x0, 0xf8, 0x1e, 0xd, 0xd } };

// {394C3DE0-3C6F-11d2-817B-00C04F797AB7}
extern "C" const GUID GUID_ATLVer70 = { 0x394c3de0, 0x3c6f, 0x11d2, { 0x81, 0x7b, 0x0, 0xc0, 0x4f, 0x79, 0x7a, 0xb7 } };

CAtlBaseModule::CAtlBaseModule() throw()
{
	cbSize = sizeof(_ATL_BASE_MODULE);

	m_hInst = m_hInstResource = reinterpret_cast<HINSTANCE>(&__ImageBase);

	dwAtlBuildVer = _ATL_VER;
	pguidVer = &GUID_ATLVer70;

	if (FAILED(m_csResource.Init()))
	{
		ATLTRACE(atlTraceGeneral, 0, _T("ERROR : Unable to initialize critical section in CAtlBaseModule\n"));
		ATLASSERT(0);
		CAtlBaseModule::m_bInitFailed = true;
	}
}

CAtlBaseModule::~CAtlBaseModule() throw ()
{
	m_csResource.Term();
}

bool CAtlBaseModule::AddResourceInstance(HINSTANCE hInst) throw()
{
	CComCritSecLock<CComCriticalSection> lock(m_csResource, false);
	if (FAILED(lock.Lock()))
	{
		ATLTRACE(atlTraceGeneral, 0, _T("ERROR : Unable to lock critical section in CAtlBaseModule\n"));
		ATLASSERT(0);
		return false;
	}
	return m_rgResourceInstance.Add(hInst) != FALSE;
}

bool CAtlBaseModule::RemoveResourceInstance(HINSTANCE hInst) throw()
{
	CComCritSecLock<CComCriticalSection> lock(m_csResource, false);
	if (FAILED(lock.Lock()))
	{
			ATLTRACE(atlTraceGeneral, 0, _T("ERROR : Unable to lock critical section in CAtlBaseModule\n"));
		ATLASSERT(0);
		return false;
	}
	for (int i = 0; i < m_rgResourceInstance.GetSize(); i++)
	{
		if (m_rgResourceInstance[i] == hInst)
		{
			m_rgResourceInstance.RemoveAt(i);
			return true;
		}
	}
	return false;
}
HINSTANCE CAtlBaseModule::GetHInstanceAt(int i) throw()
{
	CComCritSecLock<CComCriticalSection> lock(m_csResource, false);
	if (FAILED(lock.Lock()))
	{
		ATLTRACE(atlTraceGeneral, 0, _T("ERROR : Unable to lock critical section in CAtlBaseModule\n"));
		ATLASSERT(0);
		return NULL;
	}
	if (i > m_rgResourceInstance.GetSize() || i < 0)
	{
		return NULL;
	}

	if (i == m_rgResourceInstance.GetSize())
	{
		return m_hInstResource;
	}

	return m_rgResourceInstance[i];
}

#pragma init_seg( lib )

CAtlBaseModule	_AtlBaseModule;
};  // namespace ATL
