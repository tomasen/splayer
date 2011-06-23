//
// SproxyColl.h
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once

#include "stdafx.h"

template <typename E, class ETraits = CElementTraits<E> >
class CAtlPtrList : public CAtlList<E, ETraits>
{
private:
	typedef CAtlList<E, ETraits> Base;
	BOOL m_bAttached;

public:

	CAtlPtrList()
		:m_bAttached(TRUE)
	{
	}

	void Detach()
	{
		m_bAttached = FALSE;
	}

	void RemoveAll()
	{
		if (m_bAttached)
		{
			POSITION pos = GetHeadPosition();
			while (pos != NULL)
			{
				delete (GetAt(pos));
				GetAt(pos) = NULL;
				GetNext(pos);
			}
		}
		Base::RemoveAll();
	}

	~CAtlPtrList()
	{
		RemoveAll();
	}
}; // class CAtlPtrList


template <typename K, typename V, class KTraits = CElementTraits<K>, class VTraits = CElementTraits<V> >
class CAtlPtrMap : public CAtlMap<K, V, KTraits, VTraits>
{
private:
	typedef CAtlMap<K, V, KTraits, VTraits> Base;
	BOOL m_bAttached;

public:

	CAtlPtrMap()
		:m_bAttached(TRUE)
	{
	}

	void Detach()
	{
		m_bAttached = FALSE;
	}

	void RemoveAll()
	{
		Base::DisableAutoRehash();
		if (m_bAttached)
		{
			POSITION pos = GetStartPosition();
			while (pos != NULL)
			{
				delete (GetValueAt(pos));
				GetValueAt(pos) = NULL;
				GetNext(pos);
			}
		}
		Base::RemoveAll();
		Base::EnableAutoRehash();
	}

	~CAtlPtrMap()
	{
		RemoveAll();
	}
}; // class CAtlPtrMap