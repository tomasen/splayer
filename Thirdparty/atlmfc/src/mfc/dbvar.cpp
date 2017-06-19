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

//////////////////////////////////////////////////////////////////////////////
// Implementation of CDBVariant

CDBVariant::CDBVariant()
{
	// Initialize type and value
	m_dwType = DBVT_NULL;
}

void CDBVariant::Clear()
{
	switch(m_dwType)
	{
	case DBVT_NULL:
		return;

	case DBVT_BOOL:
	case DBVT_UCHAR:
	case DBVT_SHORT:
	case DBVT_LONG:
	case DBVT_SINGLE:
	case DBVT_DOUBLE:
		break;

	case DBVT_STRING:
		delete m_pstring;
		break;

	case DBVT_BINARY:
		delete m_pbinary;
		break;

	case DBVT_DATE:
		delete m_pdate;
		break;

	case DBVT_ASTRING:
		delete m_pstringA;
		break;

	case DBVT_WSTRING:
		delete m_pstringW;
		break;

	default:
		ASSERT(FALSE);
		break;
	}
	m_dwType = DBVT_NULL;
}

CDBVariant::~CDBVariant()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
