// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the	
// Active Template Library product.

#if defined(__ATLBASE_H__) || defined (_INC_CRTDBG)
#error <atldbgmem.h> must be included before other ATL and CRT headers
#endif

#ifdef __AFX_H__
#error <atldbgmem.h> cannot be used in MFC projects. See AfxEnableMemoryTracking
#endif

#ifndef __ATLDBGMEM_H__
#define __ATLDBGMEM_H__

#pragma once

#pragma warning(push)

#pragma warning(disable: 4273)
#pragma warning(disable: 4127) // conditional expression is constant

#define _MFC_OVERRIDES_NEW
#define _CRTDBG_MAP_ALLOC
#include <new.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <crtdbg.h>
#include <stddef.h>
#include <tchar.h>
#include <atltrace.h>
#include <atlchecked.h>

#define ATLDBG_UNUSED(x) x

#pragma pack(push,_ATL_PACKING)
namespace ATL
{

const DWORD atlDbgMemTrackIndividualAllocations = 0x1;
const DWORD atlDbgMemTrackAggregateStats        = 0x2;
const DWORD atlDbgMemTrackAll                   = 0x3;

struct AtlAllocRecord
{
	char szPath[MAX_PATH];
	int nLine;
	size_t nSize;
	int nAllocations;
};

__declspec(selectany) DWORD g_dwFlags = atlDbgMemTrackIndividualAllocations;
__declspec(selectany) HANDLE g_hReportFile = INVALID_HANDLE_VALUE;
__declspec(selectany) HANDLE g_hMemMutex = NULL;
__declspec(selectany) AtlAllocRecord* g_pMemStats = NULL;
__declspec(selectany) int g_nAllocedRecords = 0;
__declspec(selectany) int g_nRecords = 0;

inline int __cdecl AtlNewHandler(size_t /* nSize */)
{
	return 0;
}

__declspec(selectany) _PNH _atlNewHandler = &AtlNewHandler;

inline _PNH AtlGetNewHandler(void)
{
	return _atlNewHandler;
}

inline _PNH AtlSetNewHandler(_PNH pfnNewHandler)
{
	_PNH pfnOldHandler = _atlNewHandler;
	_atlNewHandler = pfnNewHandler;
	return pfnOldHandler;
}

extern __declspec(selectany) const _PNH _pfnUninitialized = (_PNH)-1;

}; // namespace ATL
#pragma pack(pop)

inline void* __cdecl operator new(size_t nSize)
{
	void* pResult;
	for (;;)
	{
#if !defined(_ATL_NO_DEBUG_CRT) && defined(_DEBUG)
		pResult = _malloc_dbg(nSize, _NORMAL_BLOCK, NULL, 0);
#else
		pResult = malloc(nSize);
#endif // !defined(_ATL_NO_DEBUG_CRT) && defined(_DEBUG)
		if (pResult != NULL)
			return pResult;

		if (ATL::_atlNewHandler == NULL || (*ATL::_atlNewHandler)(nSize) == 0)
			break;
	}
	return pResult;
}

inline void __cdecl operator delete(void* p)
{
#if !defined(_ATL_NO_DEBUG_CRT) && defined(_DEBUG)
		_free_dbg(p, _NORMAL_BLOCK);
#else
		free(p);
#endif // !defined(_ATL_NO_DEBUG_CRT) && defined(_DEBUG)
}

inline void* __cdecl operator new[](size_t nSize)
{
	return ::operator new(nSize);
}

inline void __cdecl operator delete[](void* p)
{
	::operator delete(p);
}

inline void* __cdecl operator new(size_t nSize, int nType, LPCSTR lpszFileName, int nLine)
{
	ATLDBG_UNUSED(nType);
	ATLDBG_UNUSED(lpszFileName);
	ATLDBG_UNUSED(nLine);

#ifdef _ATL_NO_DEBUG_CRT
	return ::operator new(nSize);
#else
	void* pResult;
	for (;;)
	{
		pResult = _malloc_dbg(nSize, nType, lpszFileName, nLine);
		if (pResult != NULL)
			return pResult;

		if (ATL::_atlNewHandler == NULL || (*ATL::_atlNewHandler)(nSize) == 0)
			break;
	}
	return pResult;
#endif // _ATL_NO_DEBUG_CRT
}

inline void __cdecl operator delete(void* p, int nType, LPCSTR /* lpszFileName */, int /* nLine */)
{
	ATLDBG_UNUSED(nType);
#if !defined(_ATL_NO_DEBUG_CRT) && defined(_DEBUG)
		_free_dbg(p, nType);
#else
		free(p);
#endif // !defined(_ATL_NO_DEBUG_CRT) && defined(_DEBUG)
}

inline void* __cdecl operator new[](size_t nSize, int nType, LPCSTR lpszFileName, int nLine)
{
	return ::operator new(nSize, nType, lpszFileName, nLine);
}

inline void __cdecl operator delete[](void* p, int nType, LPCSTR lpszFileName, int nLine)
{
	::operator delete(p, nType, lpszFileName, nLine);
}

inline void* __cdecl operator new(size_t nSize, LPCSTR lpszFileName, int nLine)
{
	return ::operator new(nSize, _NORMAL_BLOCK, lpszFileName, nLine);
}

inline void* __cdecl operator new[](size_t nSize, LPCSTR lpszFileName, int nLine)
{
	return ::operator new[](nSize, _NORMAL_BLOCK, lpszFileName, nLine);
}

inline void __cdecl operator delete(void* pData, LPCSTR /* lpszFileName */, int /* nLine */)
{
	::operator delete(pData);
}

inline void __cdecl operator delete[](void* pData, LPCSTR /* lpszFileName */, int /* nLine */)
{
	::operator delete(pData);
}


#pragma pack(push,_ATL_PACKING)
namespace ATL
{

inline void* AtlAllocMemoryDebug(size_t nSize, LPCSTR lpszFileName, int nLine)
{
	ATLDBG_UNUSED(lpszFileName);
	ATLDBG_UNUSED(nLine);
	return _malloc_dbg(nSize, _NORMAL_BLOCK, lpszFileName, nLine);
}

inline void AtlFreeMemoryDebug(void* pbData)
{
	_free_dbg(pbData, _NORMAL_BLOCK);
}

#define new new(__FILE__, __LINE__)

/////////////////////////////////////////////////////////////////////////////
// allocation failure hook, tracking turn on

inline void _AtlDbgMemTrace(LPCSTR szBuf, int nLen)
{
	ATLTRACE(atlTraceAllocation, 0, szBuf);

	if (g_hReportFile != INVALID_HANDLE_VALUE)
	{
		 DWORD dwRet = WaitForSingleObject(g_hMemMutex, INFINITE);
		 if (dwRet == WAIT_OBJECT_0 || dwRet == WAIT_ABANDONED)
		{
			DWORD bytes;
			WriteFile(g_hReportFile, szBuf, nLen, &bytes, NULL);
			ReleaseMutex(g_hMemMutex);
		}
	}
}

inline void _AtlRecordAllocation(LPCSTR szFileName, int nLine, LPCSTR szAllocType, size_t nSize, int nRequest = 0)
{
	const int c_nSize = 512;
	char szBuf[c_nSize];
	int nLen;

	if (szFileName)
		nLen = sprintf_s(szBuf, c_nSize, "%s(%d): Memory operation: %s a %d-byte block (# %ld)\r\n",
			szFileName, nLine, szAllocType, nSize, nRequest);
	else
		nLen = sprintf_s(szBuf, c_nSize, "Memory operation: %s a %d-byte block (# %ld)\r\n", szAllocType, nSize, nRequest);

	if(nLen == -1 || nLen >= c_nSize)
	{
		// Truncate it
		szBuf[c_nSize -1] = '\0';
	}
	
	if (g_dwFlags & atlDbgMemTrackIndividualAllocations)
		_AtlDbgMemTrace(szBuf, nLen);

	if (g_pMemStats && WaitForSingleObject(g_hMemMutex, INFINITE) == WAIT_OBJECT_0)
	{
		if (!szFileName)
			szFileName = "(null)";
		int nIndex = 0;
		while (nIndex < g_nRecords)
		{
			if (strncmp(g_pMemStats[nIndex].szPath, szFileName, MAX_PATH) == 0 && g_pMemStats[nIndex].nLine == nLine && g_pMemStats[nIndex].nSize == nSize)
			{
				g_pMemStats[nIndex].nAllocations++;
				break;
			}
			nIndex++;
		}

		if (nIndex == g_nRecords)
		{
			if (g_nAllocedRecords == g_nRecords)
			{
				int nNewAllocSize = g_nAllocedRecords * 2;
				if ((nNewAllocSize<0) || 
					(nNewAllocSize>(INT_MAX/sizeof(AtlAllocRecord))))
				{
					ATLASSERT(FALSE);
					ReleaseMutex(g_hMemMutex);
					return; // out of memory
				}
				AtlAllocRecord* p = (AtlAllocRecord*) HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, g_pMemStats, nNewAllocSize*sizeof(AtlAllocRecord));
				if (p == NULL)
				{
					ATLASSERT(FALSE);
					ReleaseMutex(g_hMemMutex);
					return; // out of memory
				}
				g_pMemStats = p;
				g_nAllocedRecords = nNewAllocSize;
			}
			Checked::strncpy_s(g_pMemStats[nIndex].szPath, MAX_PATH, szFileName, _TRUNCATE);
			g_pMemStats[nIndex].szPath[MAX_PATH-1] = 0;
			g_pMemStats[nIndex].nLine = nLine;
			g_pMemStats[nIndex].nSize = nSize;
			g_pMemStats[nIndex].nAllocations = 1;
			g_nRecords++;
		}

		ReleaseMutex(g_hMemMutex);
	}
}

inline int __cdecl _AtlAllocReportHook(int nAllocType, void* /* pvData */, size_t nSize, int nBlockUse, long lRequest,
	const unsigned char* szFileName, int nLine)
{
	char *operation[] = { "", "allocating", "re-allocating", "freeing" };

	if (nBlockUse == _CRT_BLOCK)   // Ignore internal C runtime library allocations
		return TRUE;

#pragma warning(disable: 4127)
	_ASSERTE((nAllocType > 0) && (nAllocType < 4));
	_ASSERTE((nBlockUse >= 0) && (nBlockUse < 5));

	_AtlRecordAllocation((LPCSTR) szFileName, nLine, operation[nAllocType], nSize, lRequest);

	return TRUE;         // Allow the memory operation to proceed
}

extern __declspec(selectany) _CRT_ALLOC_HOOK pfnCrtAllocHook = NULL;

inline void AtlSetAllocHook()
{
	if (pfnCrtAllocHook == NULL)
		pfnCrtAllocHook = _CrtSetAllocHook(_AtlAllocReportHook);
}

inline void AtlSetReportFile(HANDLE hReportFile)
{
	if (g_hMemMutex == NULL)
		g_hMemMutex = CreateMutex(NULL, FALSE, NULL);

	if (g_hMemMutex != NULL)
		g_hReportFile = hReportFile;
}

inline void AtlEnableAllocationTracking(DWORD dwFlags = atlDbgMemTrackAll)
{
	g_dwFlags = dwFlags;

	if (dwFlags & atlDbgMemTrackAggregateStats)
	{
		if (g_hMemMutex == NULL)
			g_hMemMutex = CreateMutex(NULL, FALSE, NULL);

		if (g_pMemStats == NULL && g_hMemMutex != NULL)
		{
			g_pMemStats = (AtlAllocRecord*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 128*sizeof(AtlAllocRecord));
			g_nAllocedRecords = 128;
			g_nRecords = 0;
		}
	}
}

inline BOOL AtlDumpMemoryStats()
{
	if (g_pMemStats && WaitForSingleObject(g_hMemMutex, INFINITE) == WAIT_OBJECT_0)
	{
		const int nSize = 512;
		char szBuf[nSize];

		int nLen = sprintf_s(szBuf, nSize, "%d records in dump:\r\n", g_nRecords);
		if(nLen == -1 || nLen >= _countof(szBuf))
		{
			szBuf[nSize - 1] = '\0';
		}
		_AtlDbgMemTrace(szBuf, nLen);

		for (int n=0; n<g_nRecords; n++)
		{
			int nLen = sprintf_s(szBuf, nSize, "%s(%d): %d operations of size %d\r\n",
				g_pMemStats[n].szPath,
				g_pMemStats[n].nLine,
				g_pMemStats[n].nAllocations,
				g_pMemStats[n].nSize);
			if(nLen == -1 || nLen >= _countof(szBuf))
			{
				szBuf[nSize - 1] = '\0';
			}

			_AtlDbgMemTrace(szBuf, nLen);
		}
		ReleaseMutex(g_hMemMutex);
	}

	return TRUE;
}

// This can be set to TRUE to override all AtlEnableMemoryTracking calls,
// allowing all allocations to be tracked.
BOOL _atlMemoryLeakOverride = FALSE;

inline BOOL AtlEnableMemoryTracking(BOOL bTrack)
{
	if (_atlMemoryLeakOverride)
		return TRUE;

	int nOldState = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	if (bTrack)
		_CrtSetDbgFlag(nOldState | _CRTDBG_ALLOC_MEM_DF);
	else
		_CrtSetDbgFlag(nOldState & ~_CRTDBG_ALLOC_MEM_DF);
	return nOldState & _CRTDBG_ALLOC_MEM_DF;
}

/////////////////////////////////////////////////////////////////////////////
// stop on a specific memory request

// Obsolete API
inline void AtlSetAllocStop(LONG lRequestNumber)
{
	ATLDBG_UNUSED(lRequestNumber);
	_CrtSetBreakAlloc(lRequestNumber);
}

// check all of memory (look for memory tromps)
inline BOOL AtlCheckMemory()
{
	return _CrtCheckMemory();
}

// -- true if block of exact size, allocated on the heap
// -- set *plRequestNumber to request number (or 0)
inline BOOL AtlIsMemoryBlock(const void* pData, UINT nBytes,
		LONG* plRequestNumber)
{
	ATLDBG_UNUSED(plRequestNumber);
	ATLDBG_UNUSED(nBytes);
	ATLDBG_UNUSED(pData);
	return _CrtIsMemoryBlock(pData, nBytes, plRequestNumber, NULL, NULL);
}

inline BOOL AtlDumpMemoryLeaks()
{
	return _CrtDumpMemoryLeaks();
}

/////////////////////////////////////////////////////////////////////////////

inline HANDLE __stdcall _AtlHeapCreate(DWORD flOptions, SIZE_T dwInitialSize, SIZE_T dwMaximumSize, LPCSTR lpszFileName, int nLine)
{
	_AtlRecordAllocation(lpszFileName, nLine, "HeapCreate", 0);
	return HeapCreate(flOptions, dwInitialSize, dwMaximumSize);
}

inline BOOL __stdcall _AtlHeapDestroy(HANDLE hHeap, LPCSTR lpszFileName, int nLine)
{
	_AtlRecordAllocation(lpszFileName, nLine, "HeapDestroy", 0);
	return HeapDestroy(hHeap);
}

inline LPVOID __stdcall _AtlHeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T nSize, LPCSTR lpszFileName, int nLine)
{
	LPVOID p = NULL;
#ifndef _ATL_NO_TRACK_HEAP
	hHeap; // unused
	dwFlags; // unused
	p = _malloc_dbg(nSize, _NORMAL_BLOCK, lpszFileName, nLine);
#else
	p = HeapAlloc(hHeap, dwFlags, nSize);
#endif
	_AtlRecordAllocation(lpszFileName, nLine, "HeapAlloc", nSize);
	return p;
}

inline LPVOID __stdcall _AtlHeapReAlloc(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem, SIZE_T nSize, LPCSTR lpszFileName, int nLine)
{
	LPVOID p = NULL;
#ifndef _ATL_NO_TRACK_HEAP
	hHeap; // unused
	dwFlags; // unused
	p = _realloc_dbg(lpMem, nSize, _NORMAL_BLOCK, lpszFileName, nLine);
#else
	p = HeapReAlloc(hHeap, dwFlags, lpMem, nSize);
#endif

	_AtlRecordAllocation(lpszFileName, nLine, "HeapReAlloc", nSize);
	return p;
}

inline BOOL __stdcall _AtlHeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem, LPCSTR lpszFileName, int nLine)
{
	_AtlRecordAllocation(lpszFileName, nLine, "HeapFree", 0);
#ifndef _ATL_NO_TRACK_HEAP
	hHeap; // unused
	dwFlags; // unused
	_free_dbg(lpMem, _NORMAL_BLOCK);
	return TRUE;
#else
	return HeapFree(hHeap, dwFlags, lpMem);
#endif
}

inline SIZE_T __stdcall _AtlHeapSize(HANDLE hHeap, DWORD dwFlags, LPCVOID lpMem, LPCSTR lpszFileName, int nLine)
{
	_AtlRecordAllocation(lpszFileName, nLine, "HeapSize", 0);
	return HeapSize(hHeap, dwFlags, lpMem);
}

inline BOOL __stdcall _AtlHeapValidate(HANDLE hHeap, DWORD dwFlags, LPCVOID lpMem, LPCSTR lpszFileName, int nLine)
{
	_AtlRecordAllocation(lpszFileName, nLine, "HeapValidate", 0);
	return HeapValidate(hHeap, dwFlags, lpMem);
}

inline LPVOID __stdcall _AtlVirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect, LPCSTR lpszFileName, int nLine)
{
	_AtlRecordAllocation(lpszFileName, nLine, "VirtualAlloc", dwSize);
	return VirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect);
}

inline BOOL __stdcall _AtlVirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType, LPCSTR lpszFileName, int nLine)
{
	_AtlRecordAllocation(lpszFileName, nLine, "VirtualFree", 0);
	return VirtualFree(lpAddress, dwSize, dwFreeType);
}

inline SIZE_T __stdcall _AtlVirtualQuery(LPCVOID lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer, DWORD dwLength, LPCSTR lpszFileName, int nLine)
{
	_AtlRecordAllocation(lpszFileName, nLine, "VirtualQuery", 0);
	return VirtualQuery(lpAddress, lpBuffer, dwLength);
}

#define HeapCreate(flOptions, dwInitialSize, dwMaximumSize) ATL	::_AtlHeapCreate(flOptions, dwInitialSize, dwMaximumSize, __FILE__, __LINE__)
#define HeapDestroy(hHeap) ATL::_AtlHeapDestroy(hHeap, __FILE__, __LINE__)
#define HeapAlloc(hHeap, dwFlags, nSize) ATL::_AtlHeapAlloc(hHeap, dwFlags, nSize, __FILE__, __LINE__)
#define HeapReAlloc(hHeap, dwFlags, lpMem, nSize) ATL::_AtlHeapReAlloc(hHeap, dwFlags, lpMem, nSize, __FILE__, __LINE__)
#define HeapFree(hHeap, dwFlags, lpMem) ATL::_AtlHeapFree(hHeap, dwFlags, lpMem, __FILE__, __LINE__)
#define HeapSize(hHeap, dwFlags, lpMem) ATL::_AtlHeapSize(hHeap, dwFlags, lpMem, __FILE__, __LINE__)
#define HeapValidate(hHeap, dwFlags, lpMem) ATL::_AtlHeapValidate(hHeap, dwFlags, lpMem, __FILE__, __LINE__)
#define VirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect) ATL::_AtlVirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect, __FILE__, __LINE__)
#define VirtualFree(lpAddress,  dwSize, dwFreeType) ATL::_AtlVirtualFree(lpAddress,  dwSize, dwFreeType, __FILE__, __LINE__)
#define VirtualQuery(lpAddress, lpBuffer, dwLength) ATL::_AtlVirtualQuery(lpAddress, lpBuffer, dwLength, __FILE__, __LINE__)

}; // namespace ATL
#pragma pack(pop)

#pragma warning(pop)

#undef ATLDBG_UNUSED

#endif // __ATLDBGMEM_H__
