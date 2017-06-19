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

/////////////////////////////////////////////////////////////////////////////
// Private helper for read-only property exchange with OLE controls

static void DDX_OCPropertyRO(CDataExchange* pDX, int nIDC,
	DISPID dispid, VARTYPE vt, void* pValue)
{
	ENSURE_ARG(pDX!=NULL);
	if (pDX->m_bSaveAndValidate)
	{
		COleControlSite* pControl = pDX->PrepareOleCtrl(nIDC);
		ENSURE(pControl!=NULL);
		pControl->GetProperty(dispid, vt, pValue);
	}
}

/////////////////////////////////////////////////////////////////////////////
// Simple formatting to text item

void AFXAPI DDX_OCText(CDataExchange* pDX, int nIDC, DISPID dispid,
	CString& value)
{
	ENSURE_ARG(pDX!=NULL);
	COleControlSite* pControl = pDX->PrepareOleCtrl(nIDC);
	ENSURE(pControl!=NULL);
	if (pDX->m_bSaveAndValidate)
		pControl->GetProperty(dispid, VT_BSTR, &value);
	else
		pControl->SetProperty(dispid, VT_BSTR, (LPCTSTR)value);
}

void AFXAPI DDX_OCTextRO(CDataExchange* pDX, int nIDC, DISPID dispid,
	CString& value)
{
	ENSURE_ARG(pDX!=NULL);
	DDX_OCPropertyRO(pDX, nIDC, dispid, VT_BSTR, &value);
}

/////////////////////////////////////////////////////////////////////////////
// non-text properties

void AFXAPI DDX_OCBool(CDataExchange* pDX, int nIDC, DISPID dispid,
	BOOL& value)
{
	ENSURE_ARG(pDX!=NULL);
	COleControlSite* pControl = pDX->PrepareOleCtrl(nIDC);
	ENSURE(pControl!=NULL);
	if (pDX->m_bSaveAndValidate)
		pControl->GetProperty(dispid, VT_BOOL, &value);
	else
		pControl->SetProperty(dispid, VT_BOOL, value);
}

void AFXAPI DDX_OCBoolRO(CDataExchange* pDX, int nIDC, DISPID dispid,
	BOOL& value)
{
	ENSURE_ARG(pDX!=NULL);
	DDX_OCPropertyRO(pDX, nIDC, dispid, VT_BOOL, &value);
}

void AFXAPI DDX_OCInt(CDataExchange* pDX, int nIDC, DISPID dispid,
	int &value)
{
	ENSURE_ARG(pDX!=NULL);
	COleControlSite* pControl = pDX->PrepareOleCtrl(nIDC);
	ENSURE(pControl!=NULL);
	if (pDX->m_bSaveAndValidate)
		pControl->GetProperty(dispid, VT_I4, &value);
	else
		pControl->SetProperty(dispid, VT_I4, value);
}

void AFXAPI DDX_OCIntRO(CDataExchange* pDX, int nIDC, DISPID dispid,
	int &value)
{
	ENSURE_ARG(pDX!=NULL);
	DDX_OCPropertyRO(pDX, nIDC, dispid, VT_I4, &value);
}

void AFXAPI DDX_OCInt(CDataExchange* pDX, int nIDC, DISPID dispid,
	long &value)
{
	ENSURE_ARG(pDX!=NULL);
	COleControlSite* pControl = pDX->PrepareOleCtrl(nIDC);
	ENSURE(pControl!=NULL);
	if (pDX->m_bSaveAndValidate)
		pControl->GetProperty(dispid, VT_I4, &value);
	else
		pControl->SetProperty(dispid, VT_I4, value);
}

void AFXAPI DDX_OCIntRO(CDataExchange* pDX, int nIDC, DISPID dispid,
	long &value)
{
	ENSURE_ARG(pDX!=NULL);
	DDX_OCPropertyRO(pDX, nIDC, dispid, VT_I4, &value);
}

void AFXAPI DDX_OCShort(CDataExchange* pDX, int nIDC, DISPID dispid,
	short& value)
{
	ENSURE_ARG(pDX!=NULL);
	COleControlSite* pControl = pDX->PrepareOleCtrl(nIDC);
	ENSURE(pControl!=NULL);
	if (pDX->m_bSaveAndValidate)
		pControl->GetProperty(dispid, VT_I2, &value);
	else
		pControl->SetProperty(dispid, VT_I2, value);
}

void AFXAPI DDX_OCShortRO(CDataExchange* pDX, int nIDC, DISPID dispid,
	short& value)
{
	ENSURE_ARG(pDX!=NULL);
	DDX_OCPropertyRO(pDX, nIDC, dispid, VT_I2, &value);
}

void AFXAPI DDX_OCColor(CDataExchange* pDX, int nIDC, DISPID dispid,
	OLE_COLOR& value)
{
	ENSURE_ARG(pDX!=NULL);
	COleControlSite* pControl = pDX->PrepareOleCtrl(nIDC);
	ENSURE(pControl!=NULL);
	if (pDX->m_bSaveAndValidate)
		pControl->GetProperty(dispid, VT_COLOR, &value);
	else
		pControl->SetProperty(dispid, VT_COLOR, value);
}

void AFXAPI DDX_OCColorRO(CDataExchange* pDX, int nIDC, DISPID dispid,
	OLE_COLOR& value)
{
	ENSURE_ARG(pDX!=NULL);
	DDX_OCPropertyRO(pDX, nIDC, dispid, VT_COLOR, &value);
}

#endif // !_AFX_NO_OCC_SUPPORT
