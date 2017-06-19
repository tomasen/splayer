// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// fixalloc.cpp - implementation of fixed block allocator

#include "stdafx.h"
#include "fixalloc.h"

#ifdef _DEBUG



#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// CFixedAllocNoSync

CFixedAllocNoSync::CFixedAllocNoSync(UINT nAllocSize, UINT nBlockSize)
{
	ASSERT(nAllocSize >= sizeof(CNode));
	ASSERT(nBlockSize > 1);
	
	if (nAllocSize < sizeof(CNode))
		nAllocSize = sizeof(CNode);
	if (nBlockSize <= 1)
		nBlockSize = 64;

	m_nAllocSize = nAllocSize;
	m_nBlockSize = nBlockSize;
	m_pNodeFree = NULL;
	m_pBlocks = NULL;
}

CFixedAllocNoSync::~CFixedAllocNoSync()
{
	FreeAll();
}

void CFixedAllocNoSync::FreeAll()
{
	m_pBlocks->FreeDataChain();
	m_pBlocks = NULL;
	m_pNodeFree = NULL;
}

void* CFixedAllocNoSync::Alloc()
{
	if (m_pNodeFree == NULL)
	{
		// add another block
		CPlex* pNewBlock = CPlex::Create(m_pBlocks, m_nBlockSize, m_nAllocSize);

		// chain them into free list
		CNode* pNode = (CNode*)pNewBlock->data();
		// free in reverse order to make it easier to debug
		(BYTE*&)pNode += (m_nAllocSize * m_nBlockSize) - m_nAllocSize;
		for (int i = m_nBlockSize-1; i >= 0; i--, (BYTE*&)pNode -= m_nAllocSize)
		{
			pNode->pNext = m_pNodeFree;
			m_pNodeFree = pNode;
		}
	}
	ASSERT(m_pNodeFree != NULL);  // we must have something

	// remove the first available node from the free list
	void* pNode = m_pNodeFree;
	m_pNodeFree = m_pNodeFree->pNext;
	return pNode;
}

void CFixedAllocNoSync::Free(void* p)
{
	if (p != NULL)
	{
		// simply return the node to the free list
		CNode* pNode = (CNode*)p;
		pNode->pNext = m_pNodeFree;
		m_pNodeFree = pNode;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CFixedAlloc

CFixedAlloc::CFixedAlloc(UINT nAllocSize, UINT nBlockSize)
	: base(nAllocSize, nBlockSize)
{
	InitializeCriticalSection(&m_protect);
}

CFixedAlloc::~CFixedAlloc()
{
	DeleteCriticalSection(&m_protect);
}

void CFixedAlloc::FreeAll()
{	
	EnterCriticalSection(&m_protect);
	__try
	{
		base::FreeAll();
	}
	__finally
	{
		LeaveCriticalSection(&m_protect);
	}
}

void* CFixedAlloc::Alloc()
{
	EnterCriticalSection(&m_protect);
	void* p = NULL;
	TRY
	{
		p = base::Alloc();
	}
	CATCH_ALL(e)
	{
		LeaveCriticalSection(&m_protect);
		THROW_LAST();
	}
	END_CATCH_ALL

	LeaveCriticalSection(&m_protect);
	return p;
}

void CFixedAlloc::Free(void* p)
{		
	if (p != NULL)
	{		
		EnterCriticalSection(&m_protect);
		__try
		{
			base::Free(p);
		}
		__finally
		{
			LeaveCriticalSection(&m_protect);
		}
	}
}

