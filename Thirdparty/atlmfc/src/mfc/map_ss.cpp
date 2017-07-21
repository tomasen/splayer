
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

/////////////////////////////////////////////////////////////////////////////
//
// Implementation of parmeterized Map from CString to value
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"



#include "elements.h"  // used for special creation

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////

CMapStringToString::CMapStringToString(INT_PTR nBlockSize)
{
	ASSERT(nBlockSize > 0);
	if (nBlockSize <= 0)
		nBlockSize = 10;	// default size

	m_pHashTable = NULL;
	m_nHashTableSize = 17;  // default size
	m_nCount = 0;
	m_pFreeList = NULL;
	m_pBlocks = NULL;
	m_nBlockSize = nBlockSize;
}

inline UINT CMapStringToString::HashKey(LPCTSTR key) const
{
	UINT nHash = 0;
	if (key == NULL)
	{
		AfxThrowInvalidArgException();
	}
	
	while (*key)
		nHash = (nHash<<5) + nHash + *key++;
	return nHash;
}

void CMapStringToString::InitHashTable(
	UINT nHashSize, BOOL bAllocNow)
//
// Used to force allocation of a hash table or to override the default
//   hash table size of (which is fairly small)
{
	ASSERT_VALID(this);
	ASSERT(m_nCount == 0);
	ASSERT(nHashSize > 0);
	if (nHashSize == 0)
		nHashSize = 17;	// default value

	if (m_pHashTable != NULL)
	{
		// free hash table
		delete[] m_pHashTable;
		m_pHashTable = NULL;
	}

	if (bAllocNow)
	{
		m_pHashTable = new CAssoc* [nHashSize];
		memset(m_pHashTable, 0, sizeof(CAssoc*) * nHashSize);
	}
	m_nHashTableSize = nHashSize;
}

void CMapStringToString::RemoveAll()
{
	ASSERT_VALID(this);

	if (m_pHashTable != NULL)
	{
		// destroy elements
		for (UINT nHash = 0; nHash < m_nHashTableSize; nHash++)
		{
			CAssoc* pAssoc;
			for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL;
			  pAssoc = pAssoc->pNext)
			{
				pAssoc->~CAssoc();
				//DestructElement((CString*)&pAssoc->key);  // free up string data
				//DestructElement(&pAssoc->value);

			}
		}

		// free hash table
		delete [] m_pHashTable;
		m_pHashTable = NULL;
	}

	m_nCount = 0;
	m_pFreeList = NULL;
	m_pBlocks->FreeDataChain();
	m_pBlocks = NULL;
}

CMapStringToString::~CMapStringToString()
{
	RemoveAll();
	ASSERT(m_nCount == 0);
}

/////////////////////////////////////////////////////////////////////////////
// Assoc helpers
// same as CList implementation except we store CAssoc's not CNode's
//    and CAssoc's are singly linked all the time

CMapStringToString::CAssoc*
CMapStringToString::NewAssoc(LPCTSTR key)
{
	if (m_pFreeList == NULL)
	{
		// add another block
		CPlex* newBlock = CPlex::Create(m_pBlocks, m_nBlockSize,
							sizeof(CMapStringToString::CAssoc));
		// chain them into free list
		CMapStringToString::CAssoc* pAssoc =
				(CMapStringToString::CAssoc*) newBlock->data();
		// free in reverse order to make it easier to debug
		pAssoc += m_nBlockSize - 1;
		for (INT_PTR i = m_nBlockSize-1; i >= 0; i--, pAssoc--)
		{
			pAssoc->pNext = m_pFreeList;
			m_pFreeList = pAssoc;
		}
	}
	ASSERT(m_pFreeList != NULL);  // we must have something

	CMapStringToString::CAssoc* pAssoc = m_pFreeList;
	m_pFreeList = m_pFreeList->pNext;
	m_nCount++;
	ASSERT(m_nCount > 0);  // make sure we don't overflow
#pragma push_macro("new")
#undef new
	::new(pAssoc) CAssoc(key);
#pragma pop_macro("new")
//	CString::Construct(&pAssoc->key);
//	ConstructElement(&pAssoc->value);

	return pAssoc;
}

void CMapStringToString::FreeAssoc(CMapStringToString::CAssoc* pAssoc)
{
	pAssoc->~CAssoc();
//	DestructElement((CString*)&pAssoc->key);  // free up string data
//	DestructElement(&pAssoc->value);

	pAssoc->pNext = m_pFreeList;
	m_pFreeList = pAssoc;
	m_nCount--;
	ASSERT(m_nCount >= 0);  // make sure we don't underflow

	// if no more elements, cleanup completely
	if (m_nCount == 0)
		RemoveAll();
}

CMapStringToString::CAssoc*
CMapStringToString::GetAssocAt(LPCTSTR key, UINT& nHashBucket, UINT& nHashValue) const
// find association (or return NULL)
{
	nHashValue = HashKey(key);
	nHashBucket = nHashValue % m_nHashTableSize;

	if (m_pHashTable == NULL)
		return NULL;

	// see if it exists
	CAssoc* pAssoc;
	for (pAssoc = m_pHashTable[nHashBucket]; pAssoc != NULL; pAssoc = pAssoc->pNext)
	{
		if (pAssoc->nHashValue == nHashValue && pAssoc->key == key)
			return pAssoc;
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////

const CMapStringToString::CPair *CMapStringToString::PLookup(LPCTSTR key) const
{
	ASSERT_VALID(this);

	UINT nHashBucket, nHashValue;
	CAssoc* pAssoc = GetAssocAt(key, nHashBucket, nHashValue);
	return pAssoc;
}

CMapStringToString::CPair *CMapStringToString::PLookup(LPCTSTR key)
{
	ASSERT_VALID(this);

	UINT nHashBucket, nHashValue;
	CAssoc* pAssoc = GetAssocAt(key, nHashBucket, nHashValue);
	return pAssoc;
}

BOOL CMapStringToString::Lookup(LPCTSTR key, CString& rValue) const
{
	ASSERT_VALID(this);

	UINT nHashBucket, nHashValue;
	CAssoc* pAssoc = GetAssocAt(key, nHashBucket, nHashValue);
	if (pAssoc == NULL)
		return FALSE;  // not in map

	rValue = pAssoc->value;
	return TRUE;
}

BOOL CMapStringToString::LookupKey(LPCTSTR key, LPCTSTR& rKey) const
{
	ASSERT_VALID(this);

	UINT nHashBucket, nHashValue;
	CAssoc* pAssoc = GetAssocAt(key, nHashBucket, nHashValue);
	if (pAssoc == NULL)
		return FALSE;  // not in map

	rKey = pAssoc->key;
	return TRUE;
}

CString& CMapStringToString::operator[](LPCTSTR key)
{
	ASSERT_VALID(this);

	UINT nHashBucket, nHashValue;
	CAssoc* pAssoc;
	if ((pAssoc = GetAssocAt(key, nHashBucket, nHashValue)) == NULL)
	{
		if (m_pHashTable == NULL)
			InitHashTable(m_nHashTableSize);

		// it doesn't exist, add a new Association
		pAssoc = NewAssoc(key);
		pAssoc->nHashValue = nHashValue;
		// 'pAssoc->value' is a constructed object, nothing more

		// put into hash table
		pAssoc->pNext = m_pHashTable[nHashBucket];
		m_pHashTable[nHashBucket] = pAssoc;
	}
	return pAssoc->value;  // return new reference
}


BOOL CMapStringToString::RemoveKey(LPCTSTR key)
// remove key - return TRUE if removed
{
	ASSERT_VALID(this);

	if (m_pHashTable == NULL)
		return FALSE;  // nothing in the table

	CAssoc** ppAssocPrev;
	UINT nHashValue;
	nHashValue = HashKey(key);
	ppAssocPrev = &m_pHashTable[nHashValue%m_nHashTableSize];

	CAssoc* pAssoc;
	for (pAssoc = *ppAssocPrev; pAssoc != NULL; pAssoc = pAssoc->pNext)
	{
		if ((pAssoc->nHashValue == nHashValue) && (pAssoc->key == key))
		{
			// remove it
			*ppAssocPrev = pAssoc->pNext;  // remove from list
			FreeAssoc(pAssoc);
			return TRUE;
		}
		ppAssocPrev = &pAssoc->pNext;
	}
	return FALSE;  // not found
}


/////////////////////////////////////////////////////////////////////////////
// Iterating

const CMapStringToString::CPair *CMapStringToString::PGetFirstAssoc() const
{
	ASSERT_VALID(this);
	ASSERT(m_pHashTable != NULL);  // never call on empty map

	if( m_nCount == 0 ) return NULL;

	CAssoc* pAssocRet = NULL;

	// find the first association
	for (UINT nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
		if ((pAssocRet = m_pHashTable[nBucket]) != NULL)
			break;
	ASSERT(pAssocRet != NULL);  // must find something

	return pAssocRet;
}

CMapStringToString::CPair *CMapStringToString::PGetFirstAssoc()
{
	ASSERT_VALID(this);
	ASSERT(m_pHashTable != NULL);  // never call on empty map

	if( m_nCount == 0 ) return NULL;

	CAssoc* pAssocRet = NULL;

	// find the first association
	for (UINT nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
		if ((pAssocRet = m_pHashTable[nBucket]) != NULL)
			break;
	ASSERT(pAssocRet != NULL);  // must find something

	return pAssocRet;
}

void CMapStringToString::GetNextAssoc(POSITION& rNextPosition,
	CString& rKey, CString& rValue) const
{
	ASSERT_VALID(this);
	ASSERT(m_pHashTable != NULL);  // never call on empty map

	CAssoc* pAssocRet = (CAssoc*)rNextPosition;
	ASSERT(pAssocRet != NULL);
	if (pAssocRet == NULL)
		return;

	if (pAssocRet == (CAssoc*) BEFORE_START_POSITION)
	{
		// find the first association
		for (UINT nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
		{
			if ((pAssocRet = m_pHashTable[nBucket]) != NULL)
			{
				break;
			}
		}
		ENSURE(pAssocRet != NULL);  // must find something
	}

	// find next association
	ASSERT(AfxIsValidAddress(pAssocRet, sizeof(CAssoc)));
	CAssoc* pAssocNext;
	if ((pAssocNext = pAssocRet->pNext) == NULL)
	{
		// go to next bucket
		for (UINT nBucket = (pAssocRet->nHashValue % m_nHashTableSize) + 1;
		  nBucket < m_nHashTableSize; nBucket++)
			if ((pAssocNext = m_pHashTable[nBucket]) != NULL)
				break;
	}

	rNextPosition = (POSITION) pAssocNext;

	// fill in return data
	rKey = pAssocRet->key;
	rValue = pAssocRet->value;
}

const CMapStringToString::CPair *CMapStringToString::PGetNextAssoc(const CPair *pAssoc) const
{
	ASSERT_VALID(this);
	ASSERT(m_pHashTable != NULL);  // never call on empty map

	CAssoc* pAssocRet = (CAssoc*)pAssoc;
	ASSERT(pAssocRet != NULL);
	
	if(m_pHashTable == NULL || pAssocRet == NULL)
		return NULL;
		
	ASSERT(pAssocRet != (CAssoc*)BEFORE_START_POSITION );

	// find next association
	ASSERT(AfxIsValidAddress(pAssocRet, sizeof(CAssoc)));
	CAssoc* pAssocNext;
	if ((pAssocNext = pAssocRet->pNext) == NULL)
	{
		// go to next bucket
		for (UINT nBucket = (pAssocRet->nHashValue % m_nHashTableSize) + 1;
		  nBucket < m_nHashTableSize; nBucket++)
			if ((pAssocNext = m_pHashTable[nBucket]) != NULL)
				break;
	}

	return pAssocNext;
}

CMapStringToString::CPair *CMapStringToString::PGetNextAssoc(const CPair *pAssoc)
{
	ASSERT_VALID(this);
	ASSERT(m_pHashTable != NULL);  // never call on empty map

	CAssoc* pAssocRet = (CAssoc*)pAssoc;
	ASSERT(pAssocRet != NULL);

	if(m_pHashTable == NULL || pAssocRet == NULL)
		return NULL;
	
	ASSERT(pAssocRet != (CAssoc*)BEFORE_START_POSITION );

	// find next association
	ASSERT(AfxIsValidAddress(pAssocRet, sizeof(CAssoc)));
	CAssoc* pAssocNext;
	if ((pAssocNext = pAssocRet->pNext) == NULL)
	{
		// go to next bucket
		for (UINT nBucket = (pAssocRet->nHashValue % m_nHashTableSize) + 1;
		  nBucket < m_nHashTableSize; nBucket++)
			if ((pAssocNext = m_pHashTable[nBucket]) != NULL)
				break;
	}

	return pAssocNext;
}


/////////////////////////////////////////////////////////////////////////////
// Serialization

void CMapStringToString::Serialize(CArchive& ar)
{
	ASSERT_VALID(this);

	CObject::Serialize(ar);

	if (ar.IsStoring())
	{
		ar.WriteCount(m_nCount);
		if (m_nCount == 0)
			return;  // nothing more to do

		ASSERT(m_pHashTable != NULL);
		for (UINT nHash = 0; nHash < m_nHashTableSize; nHash++)
		{
			CAssoc* pAssoc;
			for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL;
			  pAssoc = pAssoc->pNext)
			{
				ar << pAssoc->key;
				ar << pAssoc->value;
			}
		}
	}
	else
	{
		DWORD_PTR nNewCount = ar.ReadCount();
		CString newKey;
		CString newValue;
		while (nNewCount--)
		{
			ar >> newKey;
			ar >> newValue;
			SetAt(newKey, newValue);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Diagnostics

#ifdef _DEBUG
void CMapStringToString::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	dc << "with " << LONGLONG(m_nCount) << " elements";
	if (dc.GetDepth() > 0)
	{
		// Dump in format "[key] -> value"
		CString key;
		CString val;

		POSITION pos = GetStartPosition();
		while (pos != NULL)
		{
			GetNextAssoc(pos, key, val);
			dc << "\n\t[" << key << "] = " << val;
		}
	}

	dc << "\n";
}

void CMapStringToString::AssertValid() const
{
	CObject::AssertValid();

	ASSERT(m_nHashTableSize > 0);
	ASSERT(m_nCount == 0 || m_pHashTable != NULL);
		// non-empty map should have hash table
}
#endif //_DEBUG



IMPLEMENT_SERIAL(CMapStringToString, CObject, 0)

/////////////////////////////////////////////////////////////////////////////
