// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the	
// Active Template Library product.

#include "stdafx.h"
#include "Common.h"
#include "Allocate.h"

#pragma warning(disable : 4074)
#pragma init_seg(compiler)

const char *g_pszUpdateEventName	= "AtlTraceModuleManager_ProcessAddedStatic3";
const char *g_pszAllocFileMapName	= "AtlDebugAllocator_FileMappingNameStatic3";

const char *g_pszKernelObjFmt = "%s_%0x";

CAtlAllocator g_Allocator;

static bool WINAPI Init()
{
	char szFileMappingName[MAX_PATH];

	int ret;
	ATL_CRT_ERRORCHECK_SPRINTF(ret = _snprintf_s(szFileMappingName, _countof(szFileMappingName), _countof(szFileMappingName) - 1, g_pszKernelObjFmt,
		g_pszAllocFileMapName, GetCurrentProcessId()));
	
	if(ret == -1 || ret >= MAX_PATH)
	{
		throw CAtlException( E_FAIL );
	}
	// surely four megs is enough?
	if( !g_Allocator.Init(szFileMappingName, 4*1024*1024 ) )
	{
		throw CAtlException( E_OUTOFMEMORY );
	}

	return true;
}

static const bool g_bInitialized = Init();

#ifdef _DEBUG

namespace ATL
{

CTrace CTrace::s_trace;

};

#endif
