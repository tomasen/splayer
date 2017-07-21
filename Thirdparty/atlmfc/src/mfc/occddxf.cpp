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
#include "occimpl.h"



#define new DEBUG_NEW

#ifndef _AFX_NO_OCC_SUPPORT

void AFXAPI DDX_OCFloat(CDataExchange* pDX, int nIDC, DISPID dispid,
	float &value)
{
	ENSURE_ARG(pDX!=NULL);
	COleControlSite* pControl = pDX->PrepareOleCtrl(nIDC);
	ENSURE(pControl!=NULL);
	if (pDX->m_bSaveAndValidate)
		pControl->GetProperty(dispid, VT_R4, &value);
	else
		pControl->SetProperty(dispid, VT_R4, value);
}

void AFXAPI DDX_OCFloatRO(CDataExchange* pDX, int nIDC, DISPID dispid,
	float &value)
{
	ENSURE_ARG(pDX!=NULL);
	if (pDX->m_bSaveAndValidate)
	{
		COleControlSite* pControl = pDX->PrepareOleCtrl(nIDC);
		ENSURE(pControl!=NULL);
		pControl->GetProperty(dispid, VT_R4, &value);
	}
}

void AFXAPI DDX_OCFloat(CDataExchange* pDX, int nIDC, DISPID dispid,
	double &value)
{
	ENSURE_ARG(pDX!=NULL);
	COleControlSite* pControl = pDX->PrepareOleCtrl(nIDC);
	ENSURE(pControl!=NULL);
	if (pDX->m_bSaveAndValidate)
		pControl->GetProperty(dispid, VT_R8, &value);
	else
		pControl->SetProperty(dispid, VT_R8, value);
}

void AFXAPI DDX_OCFloatRO(CDataExchange* pDX, int nIDC, DISPID dispid,
	double &value)
{
	ENSURE_ARG(pDX!=NULL);
	if (pDX->m_bSaveAndValidate)
	{
		COleControlSite* pControl = pDX->PrepareOleCtrl(nIDC);
		ENSURE(pControl!=NULL);
		pControl->GetProperty(dispid, VT_R8, &value);
	}
}

#endif // !_AFX_NO_OCC_SUPPORT
