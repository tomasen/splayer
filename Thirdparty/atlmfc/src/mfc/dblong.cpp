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

/////////////////////////////////////////////////////////////////////////////
// CLongBinary class for holding LONG VARBINARY data

CLongBinary::CLongBinary()
{
	m_hData = NULL;
}

CLongBinary::~CLongBinary()
{
	if (m_hData != NULL)
	{
		::GlobalFree(m_hData);
		m_hData = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////
// CLongBinary diagnostics

#ifdef _DEBUG
void CLongBinary::AssertValid() const
{
	CObject::AssertValid();
}

void CLongBinary::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	dc << "m_hData = " << m_hData;
	dc << "\nm_dwDataLength = " << m_dwDataLength;
	dc << "\n";
}
#endif //_DEBUG

//////////////////////////////////////////////////////////////////////////////



IMPLEMENT_DYNAMIC(CLongBinary, CObject)

/////////////////////////////////////////////////////////////////////////////
