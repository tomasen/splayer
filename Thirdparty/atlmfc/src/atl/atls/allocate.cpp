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

#include "AtlTraceModuleManager.h"

#pragma comment(lib, "advapi32.lib")
#pragma warning(disable : 4711)	// function selected for automatic inline expansion

bool CAtlAllocator::Init(const CHAR *pszFileName, DWORD dwMaxSize)
{
	// We're relying on syncronization provided by the startup code (CRT DllMain/WinMain)
	Close();

	ATLASSERT(!m_hMap && !m_pBufferStart);

	HANDLE hThreadToken = NULL;
	__try
	{
		// If we're being loaded while impersonating a different client, 
		// we need to revert to self before we open the shared memory section
		BOOL bOpen = OpenThreadToken(GetCurrentThread(), TOKEN_IMPERSONATE|TOKEN_DUPLICATE, TRUE, &hThreadToken);
		if( bOpen && hThreadToken != NULL )
		{
			if( !RevertToSelf() )
			{
				CloseHandle(hThreadToken);
				hThreadToken = NULL;
				__leave;
			}
		}

		// Use a NULL SECURITY_ATTRIBUTES structure to get the default DACL.
		m_hMap = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL,
			PAGE_READWRITE | SEC_RESERVE, 0, dwMaxSize, pszFileName);
		if(!m_hMap)
		{
			__leave;
		}

		DWORD dwErr = ::GetLastError();

		m_pBufferStart = (BYTE *)
			MapViewOfFile(m_hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if(!m_pBufferStart)
		{
			__leave;
		}

		SYSTEM_INFO si;
		GetSystemInfo(&si);

		if(dwErr == ERROR_ALREADY_EXISTS)
		{
			m_pProcess = reinterpret_cast<CAtlTraceProcess *>(m_pBufferStart);

			// Looks like it's already mapped into this process space.
			// Let's do some checking...
			if(( m_pProcess==NULL ) || ( m_pProcess->Base()==NULL ) || 
				( 0 != memcmp(m_pBufferStart, m_pProcess->Base(), m_pProcess->m_dwFrontAlloc )))
			{
				// something's not right
				__leave;
			}

			// sure looks valid
			m_pProcess->IncRef();
			m_pProcess = static_cast<CAtlTraceProcess *>(m_pProcess->Base());

			UnmapViewOfFile(m_pBufferStart);
			m_pBufferStart = reinterpret_cast<BYTE *>(m_pProcess);
		}
		else
		{
			// This is just in case sizeof(CAtlTraceProcess) is
			// ever > dwPageSize (doubtful that could ever
			// happen, but it's easy enough to avoid here)
			DWORD dwCurrAlloc = si.dwPageSize;
			while(dwCurrAlloc < sizeof(CAtlTraceProcess))
			{
				dwCurrAlloc += si.dwPageSize;
			}

			if(!VirtualAlloc(m_pBufferStart, dwCurrAlloc, MEM_COMMIT, PAGE_READWRITE))
			{
				__leave;
			}

			m_pProcess = new(m_pBufferStart) CAtlTraceProcess(dwMaxSize);
			m_pProcess->m_dwFrontAlloc = dwCurrAlloc;
			m_pProcess->m_dwCurrFront = sizeof(CAtlTraceProcess);
		}
		m_dwPageSize = si.dwPageSize;
		m_bValid = true;
	}
	__finally
	{
		if( hThreadToken != NULL )
		{
			// Go back to impersonating whomever we were impersonating
			BOOL bSuccess;
			bSuccess = SetThreadToken(NULL, hThreadToken);
			ATLASSERT( bSuccess );
			CloseHandle( hThreadToken );
		}

		if(!m_bValid)
		{
			if(m_pBufferStart)
			{
				UnmapViewOfFile(m_pBufferStart);
				m_pBufferStart = NULL;
			}

			if(m_hMap)
			{
				CloseHandle(m_hMap);
				m_hMap = NULL;
			}
		}
	}
	return m_bValid;
}

bool CAtlAllocator::Open(const CHAR *pszFileName)
{
	Close();

	__try
	{
		m_hMap = OpenFileMappingA(FILE_MAP_WRITE, FALSE, pszFileName);
		if(!m_hMap)
			__leave;

		m_pBufferStart = (BYTE *)
			MapViewOfFile(m_hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if(!m_pBufferStart)
			__leave;

		m_pProcess = reinterpret_cast<CAtlTraceProcess *>(m_pBufferStart);
		m_pProcess->IncRef();

		SYSTEM_INFO si;
		GetSystemInfo(&si);
		m_dwPageSize = si.dwPageSize;

		m_bValid = true;
	}
	__finally
	{
		if(!m_bValid)
		{
			if(m_pBufferStart)
			{
				UnmapViewOfFile(m_pBufferStart);
				m_pBufferStart = NULL;
			}
			if(m_hMap)
			{
				CloseHandle(m_hMap);
				m_hMap = NULL;
			}

			m_pProcess = NULL;
		}
	}
	return m_bValid;
}

void CAtlAllocator::Close(bool bForceUnmap)
{
	if(m_bValid)
	{
		if(m_pProcess->DecRef() == 0 || bForceUnmap)
			UnmapViewOfFile(m_pBufferStart);
		m_pBufferStart = NULL;

		CloseHandle(m_hMap);
		m_hMap = NULL;

		m_bValid = false;
	}
}

CAtlTraceModule *CAtlAllocator::GetModule(int iModule) const
{
	if( iModule == -1 )
	{
		return NULL;
	}
        int modcnt = m_pProcess->ModuleCount();
	ATLASSERT(iModule < modcnt);
	if(iModule >=0 && iModule < modcnt)
	{
		BYTE *pb = reinterpret_cast<BYTE *>(m_pProcess) + sizeof(CAtlTraceProcess);
		return reinterpret_cast<CAtlTraceModule *>(pb) + iModule;
	}
	else
		return NULL;
}

/*
CAtlTraceCategory *CAtlAllocator::GetCategory(unsigned nModule, unsigned nCategory) const
{
	ATLASSERT(nModule < m_pProcess->ModuleCount());

	if(nModule < m_pProcess->ModuleCount())
	{
		BYTE *pb = reinterpret_cast<BYTE *>(m_pProcess) + sizeof(CAtlTraceProcess);
		CAtlTraceModule *pModule = reinterpret_cast<CAtlTraceModule *>(pb) + nModule;

		if(IsValidCategoryIndex(pModule->m_nFirstCategory))
		{
			unsigned nOldIndex, nIndex = pModule->m_nFirstCategory;
			CAtlTraceCategory *pCategory;
			do
			{
				pCategory = GetCategoryByIndex(nIndex);
				if(pCategory->m_nCategory == nCategory)
					return pCategory;

				nOldIndex = nIndex;
				nIndex = pCategory->m_nNext;
			}
			while(nOldIndex != nIndex);
		}
	}
	return NULL;
}
*/

/*
bool CAtlAllocator::IsValidCategoryIndex(unsigned nIndex) const
{
	return nIndex < m_pProcess->CategoryCount();
}
*/

CAtlTraceCategory *CAtlAllocator::GetCategory(int iCategory) const
{
	if(iCategory == m_pProcess->CategoryCount())
		return NULL;

	ATLASSERT((iCategory < m_pProcess->CategoryCount()) || (iCategory == -1));
	CAtlTraceCategory *pCategory = NULL;
	if(iCategory >= 0)
	{
		BYTE *pb = reinterpret_cast<BYTE *>(m_pProcess) + m_pProcess->MaxSize();
		pCategory = reinterpret_cast<CAtlTraceCategory *>(pb) - iCategory - 1;
	}
	return pCategory;
}

int CAtlAllocator::GetCategoryCount(int iModule) const
{
	UINT nCategories = 0;
	CAtlTraceModule* pModule = GetModule(iModule);
	ATLASSERT(pModule != NULL);
	if(pModule != NULL)
	{
		nCategories = GetCategoryCount( *pModule );
	}

	return nCategories;
}

int CAtlAllocator::GetCategoryCount(const CAtlTraceModule& rModule) const
{
	UINT nCategories = 0;
	int iCategory = rModule.m_iFirstCategory;
	while( iCategory != -1 )
	{
		CAtlTraceCategory* pCategory = GetCategory( iCategory );
		if( pCategory == NULL )
			break;
		nCategories++;
		iCategory = pCategory->m_iNextCategory;
	}
	return nCategories;
}

int CAtlAllocator::GetModuleCount() const
{
	ATLASSERT(m_pProcess);
	return m_pProcess->ModuleCount();
}

const ULONG kModuleBatchSize = 10;

bool CAtlAllocator::FindModule(const WCHAR *pszModulePath, unsigned *pnModule) const
{
	if(pnModule)
		for(int i = 0; i < m_pProcess->ModuleCount(); i++)
			if(0 == _wcsicmp(GetModule(i)->Path(), pszModulePath))
			{
				*pnModule = i;
				return true;
			}

	return false;
}

int CAtlAllocator::AddModule(HINSTANCE hInst)
{
	CAtlTraceProcess *pProcess = GetProcess();
	ATLASSERT(pProcess);
	int iFoundModule = -1;
	while( iFoundModule == -1 )
	{
		for(int iModule = 0; (iModule < pProcess->ModuleCount()) && (iFoundModule == -1); iModule++)
		{
			CAtlTraceModule *pModule = GetModule(iModule);
			ATLASSERT(pModule != NULL);
			bool bFound = pModule->TryAllocate();
			if( bFound )
			{
				pModule->Reset(hInst);
				pModule->m_iFirstCategory = -1;
				pModule->MarkValid( pProcess->GetNextCookie() );
				iFoundModule = iModule;
			}
		}
		if( iFoundModule == -1 )
		{
			ULONG nNewAllocSize = kModuleBatchSize*sizeof( CAtlTraceModule );
			void* pNewModules = reinterpret_cast<BYTE *>(pProcess) + pProcess->m_dwFrontAlloc;
			VirtualAlloc(pNewModules, nNewAllocSize, MEM_COMMIT, PAGE_READWRITE);
			pProcess->m_dwFrontAlloc += nNewAllocSize;
			for( ULONG iNewModule = 0; iNewModule < kModuleBatchSize; iNewModule++ )
			{
				CAtlTraceModule* pNewModule = static_cast< CAtlTraceModule* >( pNewModules )+iNewModule;
				new( pNewModule ) CAtlTraceModule;
			}
			pProcess->IncModuleCount( kModuleBatchSize );
		}
	}

	return iFoundModule;
}

const ULONG kCategoryBatchSize = 10;

int CAtlAllocator::AddCategory(int iModule, const WCHAR *szCategoryName)
{
	int iFoundCategory = -1;
	CAtlTraceProcess *pProcess = GetProcess();
	ATLASSERT(pProcess);
	CAtlTraceModule *pModule = GetModule(iModule);
	if(pModule)
	{
		pModule->TryAddRef();

		while( iFoundCategory == -1 )
		{
			for(int iCategory = 0; (iCategory < pProcess->CategoryCount()) && (iFoundCategory == -1); iCategory++)
			{
				CAtlTraceCategory *pCategory = GetCategory( iCategory );
				ATLASSERT(pCategory != NULL);
				bool bFound = pCategory->TryAllocate();
				if( bFound )
				{
					pCategory->Reset( szCategoryName, pModule->m_nCookie );
					pCategory->m_iNextCategory = pModule->m_iFirstCategory;
					pCategory->MarkValid( pProcess->GetNextCookie() );
					pModule->m_iFirstCategory = iCategory;
					::InterlockedIncrement( &pModule->m_nCategories );
					iFoundCategory = iCategory;
				}
			}

			if( iFoundCategory == -1 )
			{
				ULONG nNewAllocSize = kCategoryBatchSize*sizeof( CAtlTraceCategory );
				void* pNewCategories = reinterpret_cast<BYTE *>(pProcess) + pProcess->MaxSize()-pProcess->m_dwBackAlloc-nNewAllocSize;
				VirtualAlloc(pNewCategories, nNewAllocSize, MEM_COMMIT, PAGE_READWRITE);
				pProcess->m_dwBackAlloc += nNewAllocSize;
				for( ULONG iNewCategory = 0; iNewCategory < kCategoryBatchSize; iNewCategory++ )
				{
					CAtlTraceCategory* pNewCategory = static_cast< CAtlTraceCategory* >( pNewCategories )+iNewCategory;
					new( pNewCategory ) CAtlTraceCategory;
				}
				pProcess->IncCategoryCount( kCategoryBatchSize );
			}
		}

		pModule->Release();
	}

	pProcess->m_bLoaded = false;

	return( iFoundCategory );
}

bool CAtlAllocator::RemoveModule(int iModule)
{
	CAtlTraceModule* pModule = GetModule(iModule);
	if(pModule)
	{
		int iCategory = pModule->m_iFirstCategory;
		while( iCategory != -1 )
		{
			CAtlTraceCategory* pCategory = GetCategory( iCategory );
			if( pCategory == NULL )
			{
				pModule->Release();
				return false;
			}
			iCategory = pCategory->m_iNextCategory;
			::InterlockedDecrement( &pModule->m_nCategories );
			pModule->m_iFirstCategory = iCategory;
			pCategory->Release();
		}

		pModule->Release();
		return true;
	}
	return false;
}

void CAtlAllocator::CleanUp()
{
	Close();
}

void CAtlAllocator::TakeSnapshot()
{
	if( m_bSnapshot )
	{
		ReleaseSnapshot();
	}

	int nModules = GetModuleCount();
	for( int iModule = 0; iModule < nModules; iModule++ )
	{
		CAtlTraceModule* pModule = GetModule( iModule );
		bool bValidModule = pModule->TryAddRef();
		if( bValidModule )
		{
			CTraceSnapshot::CModuleInfo module;
			module.m_dwModule = DWORD_PTR( iModule )+1;
			module.m_iFirstCategory = m_snapshot.m_adwCategories.GetSize();
			module.m_nCategories = pModule->m_nCategories;

			int iCategory = pModule->m_iFirstCategory;
			bool bCategoriesValid = true;
			int nCategories = 0;
			while( (iCategory != -1) && bCategoriesValid )
			{
				CAtlTraceCategory* pCategory = GetCategory( iCategory );
				bool bValidCategory = pCategory->TryAddRef();
				if( bValidCategory )
				{
					if( pCategory->m_nModuleCookie != pModule->m_nCookie )
					{
						bValidCategory = false;
						pCategory->Release();
					}
					else
					{
						m_snapshot.m_adwCategories.Add( DWORD_PTR( iCategory ) );
						nCategories++;
						iCategory = pCategory->m_iNextCategory;
					}
				}
				if( !bValidCategory )
				{
					bCategoriesValid = false;
				}
			}
			if( !bCategoriesValid )
			{
				for( int iCategoryIndex = nCategories-1; iCategoryIndex >= 0; iCategoryIndex-- )
				{
					DWORD_PTR dwCategory = m_snapshot.m_adwCategories[module.m_iFirstCategory+iCategoryIndex];
					m_snapshot.m_adwCategories.RemoveAt( module.m_iFirstCategory+iCategoryIndex );
					GetCategory( int( dwCategory ) )->Release();
				}
				pModule->Release();
			}
			else
			{
				m_snapshot.m_aModules.Add( module );
			}
		}
	}

	m_bSnapshot = true;
}

void CAtlAllocator::ReleaseSnapshot()
{
	if( m_bSnapshot )
	{
		for( int iModule = 0; iModule < m_snapshot.m_aModules.GetSize(); iModule++ )
		{
			CAtlTraceModule *pModule = GetModule( int( m_snapshot.m_aModules[iModule].m_dwModule-1 ) );
			if( pModule != NULL )
				pModule->Release();
		}
		for( int iCategory = 0; iCategory < m_snapshot.m_adwCategories.GetSize(); iCategory++ )
		{
			CAtlTraceCategory *pCategory = GetCategory( int( m_snapshot.m_adwCategories[iCategory] ) );
			if( pCategory != NULL )
				pCategory->Release();
		}
		m_bSnapshot = false;
	}
}
