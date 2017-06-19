
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
// Implementation of parameterized Array
//
/////////////////////////////////////////////////////////////////////////////
// NOTE: we allocate an array of 'm_nMaxSize' elements, but only
//  the current size 'm_nSize' contains properly constructed
//  objects.

#include "stdafx.h"
#include <wchar.h>



#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////

CObArray::CObArray()
{
	m_pData = NULL;
	m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

CObArray::~CObArray()
{
	ASSERT_VALID(this);

	delete[] (BYTE*)m_pData;
}

void CObArray::SetSize(INT_PTR nNewSize, INT_PTR nGrowBy)
{
	ASSERT_VALID(this);
	ASSERT(nNewSize >= 0);
	
	if(nNewSize < 0 )
		AfxThrowInvalidArgException();

	if (nGrowBy >= 0)
		m_nGrowBy = nGrowBy;  // set new size

	if (nNewSize == 0)
	{
		// shrink to nothing
		delete[] (BYTE*)m_pData;
		m_pData = NULL;
		m_nSize = m_nMaxSize = 0;
	}
	else if (m_pData == NULL)
	{
		// create one with exact size
#ifdef SIZE_T_MAX
		ASSERT(nNewSize <= SIZE_T_MAX/sizeof(CObject*));    // no overflow
#endif
		m_pData = (CObject**) new BYTE[nNewSize * sizeof(CObject*)];

		memset(m_pData, 0, nNewSize * sizeof(CObject*));  // zero fill

		m_nSize = m_nMaxSize = nNewSize;
	}
	else if (nNewSize <= m_nMaxSize)
	{
		// it fits
		if (nNewSize > m_nSize)
		{
			// initialize the new elements

			memset(&m_pData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(CObject*));

		}

		m_nSize = nNewSize;
	}
	else
	{
		// otherwise, grow array
		INT_PTR nGrowBySize = m_nGrowBy;
		if (nGrowBySize == 0)
		{
			// heuristically determine growth when nGrowBy == 0
			//  (this avoids heap fragmentation in many situations)
			nGrowBySize = min(1024, max(4, m_nSize / 8));
		}
		INT_PTR nNewMax;
		if (nNewSize < m_nMaxSize + nGrowBySize)
			nNewMax = m_nMaxSize + nGrowBySize;  // granularity
		else
			nNewMax = nNewSize;  // no slush

		ASSERT(nNewMax >= m_nMaxSize);  // no wrap around
		
		if(nNewMax  < m_nMaxSize)
			AfxThrowInvalidArgException();
			
#ifdef SIZE_T_MAX
		ASSERT(nNewMax <= SIZE_T_MAX/sizeof(CObject*)); // no overflow
#endif
		CObject** pNewData = (CObject**) new BYTE[nNewMax * sizeof(CObject*)];

		// copy new data from old
		Checked::memcpy_s(pNewData, nNewMax * sizeof(CObject*), 
			m_pData, m_nSize * sizeof(CObject*));

		// construct remaining elements
		ASSERT(nNewSize > m_nSize);

		memset(&pNewData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(CObject*));


		// get rid of old stuff (note: no destructors called)
		delete[] (BYTE*)m_pData;
		m_pData = pNewData;
		m_nSize = nNewSize;
		m_nMaxSize = nNewMax;
	}
}

INT_PTR CObArray::Append(const CObArray& src)
{
	ASSERT_VALID(this);
	ASSERT(this != &src);   // cannot append to itself

	if(this == &src)
		AfxThrowInvalidArgException();
		
	INT_PTR nOldSize = m_nSize;
	SetSize(m_nSize + src.m_nSize);

	Checked::memcpy_s(m_pData + nOldSize, src.m_nSize * sizeof(CObject*), 
		src.m_pData, src.m_nSize * sizeof(CObject*));

	return nOldSize;
}

void CObArray::Copy(const CObArray& src)
{
	ASSERT_VALID(this);
	ASSERT(this != &src);   // cannot append to itself

	if(this != &src)
	{
		SetSize(src.m_nSize);
	
		Checked::memcpy_s(m_pData, src.m_nSize * sizeof(CObject*), 
			src.m_pData, src.m_nSize * sizeof(CObject*));
	}

}

void CObArray::FreeExtra()
{
	ASSERT_VALID(this);

	if (m_nSize != m_nMaxSize)
	{
		// shrink to desired size
#ifdef SIZE_T_MAX
		ASSERT(m_nSize <= SIZE_T_MAX/sizeof(CObject*)); // no overflow
#endif
		CObject** pNewData = NULL;
		if (m_nSize != 0)
		{
			pNewData = (CObject**) new BYTE[m_nSize * sizeof(CObject*)];

			// copy new data from old 
			Checked::memcpy_s(pNewData, m_nSize * sizeof(CObject*), 
				m_pData, m_nSize * sizeof(CObject*));
		}

		// get rid of old stuff (note: no destructors called)
		delete[] (BYTE*)m_pData;
		m_pData = pNewData;
		m_nMaxSize = m_nSize;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CObArray::SetAtGrow(INT_PTR nIndex, CObject* newElement)
{
	ASSERT_VALID(this);
	ASSERT(nIndex >= 0);
	
	if(nIndex < 0)
		AfxThrowInvalidArgException();

	if (nIndex >= m_nSize)
		SetSize(nIndex+1);
	m_pData[nIndex] = newElement;
}





void CObArray::InsertAt(INT_PTR nIndex, CObject* newElement, INT_PTR nCount)
{

	ASSERT_VALID(this);
	ASSERT(nIndex >= 0);    // will expand to meet need
	ASSERT(nCount > 0);     // zero or negative size not allowed

	if(nIndex < 0 || nCount <= 0)
		AfxThrowInvalidArgException();
		
	if (nIndex >= m_nSize)
	{
		// adding after the end of the array
		SetSize(nIndex + nCount);  // grow so nIndex is valid
	}
	else
	{
		// inserting in the middle of the array
		INT_PTR nOldSize = m_nSize;
		SetSize(m_nSize + nCount);  // grow it to new size

		// shift old data up to fill gap 
		Checked::memmove_s(&m_pData[nIndex+nCount], (m_nSize-(nIndex+nCount)) * sizeof(CObject*), 
			&m_pData[nIndex], (nOldSize-nIndex) * sizeof(CObject*));

		// re-init slots we copied from
		memset(&m_pData[nIndex], 0, nCount * sizeof(CObject*));
	}

	// insert new value in the gap
	ASSERT(nIndex + nCount <= m_nSize);



	// copy elements into the empty space
	while (nCount--)
		m_pData[nIndex++] = newElement;

}



void CObArray::RemoveAt(INT_PTR nIndex, INT_PTR nCount)
{
	ASSERT_VALID(this);
	ASSERT(nIndex >= 0);
	ASSERT(nCount >= 0);
	INT_PTR nUpperBound = nIndex + nCount;
	ASSERT(nUpperBound <= m_nSize && nUpperBound >= nIndex && nUpperBound >= nCount);

	if(nIndex < 0 || nCount < 0 || (nUpperBound > m_nSize) || (nUpperBound < nIndex) || (nUpperBound < nCount))
		AfxThrowInvalidArgException();
		
	// just remove a range
	INT_PTR nMoveCount = m_nSize - (nUpperBound);

	if (nMoveCount)
	{
		Checked::memmove_s(&m_pData[nIndex], nMoveCount * sizeof(CObject*), 
			&m_pData[nUpperBound], nMoveCount * sizeof(CObject*));
	}

	m_nSize -= nCount;
}

void CObArray::InsertAt(INT_PTR nStartIndex, CObArray* pNewArray)
{
	ASSERT_VALID(this);
	ASSERT(pNewArray != NULL);
	ASSERT_KINDOF(CObArray, pNewArray);
	ASSERT_VALID(pNewArray);
	ASSERT(nStartIndex >= 0);

	if(pNewArray == NULL || nStartIndex < 0)
		AfxThrowInvalidArgException();
		
	if (pNewArray->GetSize() > 0)
	{
		InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
		for (INT_PTR i = 0; i < pNewArray->GetSize(); i++)
			SetAt(nStartIndex + i, pNewArray->GetAt(i));
	}
}


/////////////////////////////////////////////////////////////////////////////
// Serialization

void CObArray::Serialize(CArchive& ar)
{
	ASSERT_VALID(this);

	CObject::Serialize(ar);

	if (ar.IsStoring())
	{
		ar.WriteCount(m_nSize);
		for (INT_PTR i = 0; i < m_nSize; i++)
			ar << m_pData[i];
	}
	else
	{
		DWORD_PTR nOldSize = ar.ReadCount();
		SetSize(nOldSize);
		for (INT_PTR i = 0; i < m_nSize; i++)
			ar >> m_pData[i];
	}
}



/////////////////////////////////////////////////////////////////////////////
// Diagnostics

#ifdef _DEBUG
void CObArray::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	dc << "with " << m_nSize << " elements";
	if (dc.GetDepth() > 0)
	{
		for (INT_PTR i = 0; i < m_nSize; i++)
			dc << "\n\t[" << i << "] = " << m_pData[i];
	}

	dc << "\n";
}

void CObArray::AssertValid() const
{
	CObject::AssertValid();

	if (m_pData == NULL)
	{
		ASSERT(m_nSize == 0);
		ASSERT(m_nMaxSize == 0);
	}
	else
	{
		ASSERT(m_nSize >= 0);
		ASSERT(m_nMaxSize >= 0);
		ASSERT(m_nSize <= m_nMaxSize);
		ASSERT(AfxIsValidAddress(m_pData, m_nMaxSize * sizeof(CObject*)));
	}
}
#endif //_DEBUG



IMPLEMENT_SERIAL(CObArray, CObject, 0)

/////////////////////////////////////////////////////////////////////////////
