// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// fixalloc.h - declarations for fixed block allocator

#ifndef __FIXALLOC_H__
#define __FIXALLOC_H__

#include "afxplex_.h"

/////////////////////////////////////////////////////////////////////////////
// CFixedAlloc

class CFixedAllocNoSync
{
// Constructors
public:
	CFixedAllocNoSync(UINT nAllocSize, UINT nBlockSize = 64);

// Attributes
	UINT GetAllocSize() { return m_nAllocSize; }

// Operations
public:
	void* Alloc();  // return a chunk of memory of nAllocSize
	void Free(void* p); // free chunk of memory returned from Alloc
	void FreeAll(); // free everything allocated from this allocator

// Implementation
public:
	~CFixedAllocNoSync();

protected:
	struct CNode
	{
		CNode* pNext;	// only valid when in free list
	};

	UINT m_nAllocSize;	// size of each block from Alloc
	UINT m_nBlockSize;	// number of blocks to get at a time
	CPlex* m_pBlocks;	// linked list of blocks (is nBlocks*nAllocSize)
	CNode* m_pNodeFree;	// first free node (NULL if no free nodes)
};

class CFixedAlloc : public CFixedAllocNoSync
{
	typedef class CFixedAllocNoSync base;

// Constructors
public:
	CFixedAlloc(UINT nAllocSize, UINT nBlockSize = 64);

// Operations
public:
	void* Alloc();	// return a chunk of memory of nAllocSize
	void Free(void* p);	// free chunk of memory returned from Alloc
	void FreeAll();	// free everything allocated from this allocator

// Implementation
public:
	~CFixedAlloc();

protected:
	CRITICAL_SECTION m_protect;
};

#ifndef _DEBUG

// DECLARE_FIXED_ALLOC -- used in class definition
#define DECLARE_FIXED_ALLOC(class_name) \
public: \
	void* operator new(size_t size) \
	{ \
		ASSERT(size == s_alloc.GetAllocSize()); \
		UNUSED(size); \
		return s_alloc.Alloc(); \
	} \
	void* operator new(size_t, void* p) \
		{ return p; } \
	void operator delete(void* p) { s_alloc.Free(p); } \
	void* operator new(size_t size, LPCSTR, int) \
	{ \
		ASSERT(size == s_alloc.GetAllocSize()); \
		UNUSED(size); \
		return s_alloc.Alloc(); \
	} \
protected: \
	static CFixedAlloc s_alloc \

// IMPLEMENT_FIXED_ALLOC -- used in class implementation file
#define IMPLEMENT_FIXED_ALLOC(class_name, block_size) \
CFixedAlloc class_name::s_alloc(sizeof(class_name), block_size) \

// DECLARE_FIXED_ALLOC -- used in class definition
#define DECLARE_FIXED_ALLOC_NOSYNC(class_name) \
public: \
	void* operator new(size_t size) \
	{ \
		ASSERT(size == s_alloc.GetAllocSize()); \
		UNUSED(size); \
		return s_alloc.Alloc(); \
	} \
	void* operator new(size_t, void* p) \
		{ return p; } \
	void operator delete(void* p) { s_alloc.Free(p); } \
	void* operator new(size_t size, LPCSTR, int) \
	{ \
		ASSERT(size == s_alloc.GetAllocSize()); \
		UNUSED(size); \
		return s_alloc.Alloc(); \
	} \
protected: \
	static CFixedAllocNoSync s_alloc \

// IMPLEMENT_FIXED_ALLOC_NOSYNC -- used in class implementation file
#define IMPLEMENT_FIXED_ALLOC_NOSYNC(class_nbame, block_size) \
CFixedAllocNoSync class_name::s_alloc(sizeof(class_name), block_size) \

#else //!_DEBUG

#define DECLARE_FIXED_ALLOC(class_name) // nothing in debug
#define IMPLEMENT_FIXED_ALLOC(class_name, block_size) // nothing in debug
#define DECLARE_FIXED_ALLOC_NOSYNC(class_name) // nothing in debug
#define IMPLEMENT_FIXED_ALLOC_NOSYNC(class_name, block_size) // nothing in debug

#endif //!_DEBUG

#ifndef _AFX_DISABLE_DEPRECATED
#pragma deprecated( CFixedAlloc )
#endif

#endif
