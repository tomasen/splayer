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
#include <afxtempl.h>
#include <math.h>


/////////////////////////////////////////////////////////////////////////////
// helpers
typedef CFixedStringT<CStringW, 256> CTempStringW;

void AFXAPI AfxCheckError(SCODE sc)
{
	if (FAILED(sc))
	{
		if (sc == E_OUTOFMEMORY)
			AfxThrowMemoryException();
		else
			AfxThrowOleException(sc);
	}
}

AFX_STATIC BOOL AFXAPI _AfxCompareSafeArrays(SAFEARRAY* parray1, SAFEARRAY* parray2)
{
	BOOL bCompare = FALSE;

	// If one is NULL they must both be NULL to compare
	if (parray1 == NULL || parray2 == NULL)
	{
		return parray1 == parray2;
	}

	// Dimension must match and if 0, then arrays compare
	DWORD dwDim1 = ::SafeArrayGetDim(parray1);
	DWORD dwDim2 = ::SafeArrayGetDim(parray2);
	if (dwDim1 != dwDim2)
		return FALSE;
	else if (dwDim1 == 0)
		return TRUE;

	// Element size must match
	DWORD dwSize1 = ::SafeArrayGetElemsize(parray1);
	DWORD dwSize2 = ::SafeArrayGetElemsize(parray2);
	if (dwSize1 != dwSize2)
		return FALSE;

	long* pLBound1 = NULL;
	long* pLBound2 = NULL;
	long* pUBound1 = NULL;
	long* pUBound2 = NULL;

	void* pData1 = NULL;
	void* pData2 = NULL;

	TRY
	{
		// Bounds must match
		pLBound1 = new long[dwDim1];
		pLBound2 = new long[dwDim2];
		pUBound1 = new long[dwDim1];
		pUBound2 = new long[dwDim2];

		size_t nTotalElements = 1;

		// Get and compare bounds
		for (DWORD dwIndex = 0; dwIndex < dwDim1; dwIndex++)
		{
			AfxCheckError(::SafeArrayGetLBound(
				parray1, dwIndex+1, &pLBound1[dwIndex]));
			AfxCheckError(::SafeArrayGetLBound(
				parray2, dwIndex+1, &pLBound2[dwIndex]));
			AfxCheckError(::SafeArrayGetUBound(
				parray1, dwIndex+1, &pUBound1[dwIndex]));
			AfxCheckError(::SafeArrayGetUBound(
				parray2, dwIndex+1, &pUBound2[dwIndex]));

			// Check the magnitude of each bound
			if (pUBound1[dwIndex] - pLBound1[dwIndex] !=
				pUBound2[dwIndex] - pLBound2[dwIndex])
			{
				delete[] pLBound1;
				delete[] pLBound2;
				delete[] pUBound1;
				delete[] pUBound2;

				return FALSE;
			}

			// Increment the element count
			nTotalElements *= pUBound1[dwIndex] - pLBound1[dwIndex] + 1;
		}

		// Access the data
		AfxCheckError(::SafeArrayAccessData(parray1, &pData1));
		AfxCheckError(::SafeArrayAccessData(parray2, &pData2));

		// Calculate the number of bytes of data and compare
		size_t nSize = nTotalElements * dwSize1;
		int nOffset = memcmp(pData1, pData2, nSize);
		bCompare = nOffset == 0;

		// Release the array locks
		AfxCheckError(::SafeArrayUnaccessData(parray1));
		AfxCheckError(::SafeArrayUnaccessData(parray2));
	}
	CATCH_ALL(e)
	{
		// Clean up bounds arrays
		delete[] pLBound1;
		delete[] pLBound2;
		delete[] pUBound1;
		delete[] pUBound2;

		// Release the array locks
		if (pData1 != NULL)
			AfxCheckError(::SafeArrayUnaccessData(parray1));
		if (pData2 != NULL)
			AfxCheckError(::SafeArrayUnaccessData(parray2));

		THROW_LAST();
	}
	END_CATCH_ALL

	// Clean up bounds arrays
	delete[] pLBound1;
	delete[] pLBound2;
	delete[] pUBound1;
	delete[] pUBound2;

	return bCompare;
}

AFX_STATIC void AFXAPI _AfxCreateOneDimArray(VARIANT& varSrc, DWORD dwSize)
{
	UINT nDim;

	// Clear VARIANT and re-create SafeArray if necessary
	if (varSrc.vt != (VT_UI1 | VT_ARRAY) ||
		(nDim = ::SafeArrayGetDim(varSrc.parray)) != 1)
	{
		VERIFY(::VariantClear(&varSrc) == NOERROR);
		varSrc.vt = VT_UI1 | VT_ARRAY;

		SAFEARRAYBOUND bound;
		bound.cElements = dwSize;
		bound.lLbound = 0;
		varSrc.parray = ::SafeArrayCreate(VT_UI1, 1, &bound);
		if (varSrc.parray == NULL)
			AfxThrowMemoryException();
	}
	else
	{
		// Must redimension array if necessary
		long lLower, lUpper;
		AfxCheckError(::SafeArrayGetLBound(varSrc.parray, 1, &lLower));
		AfxCheckError(::SafeArrayGetUBound(varSrc.parray, 1, &lUpper));

		// Upper bound should always be greater than lower bound
		long lSize = lUpper - lLower;
		if (lSize < 0)
		{
			ASSERT(FALSE);
			lSize = 0;

		}

		if ((DWORD)lSize != dwSize)
		{
			SAFEARRAYBOUND bound;
			bound.cElements = dwSize;
			bound.lLbound = lLower;
			AfxCheckError(::SafeArrayRedim(varSrc.parray, &bound));
		}
	}
}

AFX_STATIC void AFXAPI _AfxCopyBinaryData(SAFEARRAY* parray, const void* pvSrc, DWORD dwSize)
{
	// Access the data, copy it and unaccess it.
	void* pDest;
	AfxCheckError(::SafeArrayAccessData(parray, &pDest));
	Checked::memcpy_s(pDest, dwSize, pvSrc, dwSize);
	AfxCheckError(::SafeArrayUnaccessData(parray));
}

/////////////////////////////////////////////////////////////////////////////
// COleVariant class

COleVariant::COleVariant(const VARIANT& varSrc)
{
	AfxVariantInit(this);
	AfxCheckError(::VariantCopy(this, (LPVARIANT)&varSrc));
}

COleVariant::COleVariant(LPCVARIANT pSrc)
{
	AfxVariantInit(this);
	AfxCheckError(::VariantCopy(this, (LPVARIANT)pSrc));
}

COleVariant::COleVariant(const COleVariant& varSrc)
{
	AfxVariantInit(this);
	AfxCheckError(::VariantCopy(this, (LPVARIANT)&varSrc));
}

COleVariant::COleVariant(LPCTSTR lpszSrc, VARTYPE vtSrc)
{
#if defined (UNICODE)
	ASSERT(vtSrc == VT_BSTR);
#else
	ASSERT(vtSrc == VT_BSTR || vtSrc == VT_BSTRT);
#endif
	UNUSED(vtSrc);

	vt = VT_BSTR;
	bstrVal = NULL;

	if (lpszSrc != NULL)
	{
#ifndef _UNICODE
		if (vtSrc == VT_BSTRT)
		{
			int nLen = lstrlen(lpszSrc);
			bstrVal = ::SysAllocStringByteLen(lpszSrc, nLen);
			
			if (bstrVal == NULL)
				AfxThrowMemoryException();
		}
		else
#endif
		{
			bstrVal = CTempStringW(lpszSrc).AllocSysString();
		}
	}
}



void COleVariant::SetString(LPCTSTR lpszSrc, VARTYPE vtSrc)
{
#if defined (UNICODE)
	ASSERT(vtSrc == VT_BSTR);
#else
	ASSERT(vtSrc == VT_BSTR || vtSrc == VT_BSTRT);
#endif
	UNUSED(vtSrc);

	// Free up previous VARIANT
	Clear();

	vt = VT_BSTR;
	bstrVal = NULL;

	if (lpszSrc != NULL)
	{
#ifndef _UNICODE
		if (vtSrc == VT_BSTRT)
		{
			int nLen = lstrlen(lpszSrc);
			bstrVal = ::SysAllocStringByteLen(lpszSrc, nLen);

			if (bstrVal == NULL)
				AfxThrowMemoryException();
		}
		else
#endif
		{
			bstrVal = CTempStringW(lpszSrc).AllocSysString();
		}
	}
}

COleVariant::COleVariant(short nSrc, VARTYPE vtSrc)
{
	ASSERT(vtSrc == VT_I2 || vtSrc == VT_BOOL);

	if (vtSrc == VT_BOOL)
	{
		vt = VT_BOOL;
		if (!nSrc)
			V_BOOL(this) = AFX_OLE_FALSE;
		else
			V_BOOL(this) = AFX_OLE_TRUE;
	}
	else
	{
		vt = VT_I2;
		iVal = nSrc;
	}
}

COleVariant::COleVariant(long lSrc, VARTYPE vtSrc)
{
	ASSERT(vtSrc == VT_I4 || vtSrc == VT_ERROR || vtSrc == VT_BOOL
		|| vtSrc == VT_UINT || vtSrc == VT_INT || vtSrc == VT_UI4
		|| vtSrc == VT_HRESULT);

	if (vtSrc == VT_ERROR)
	{
		vt = VT_ERROR;
		scode = lSrc;
	}
	else if (vtSrc == VT_BOOL)
	{
		vt = VT_BOOL;
		if (!lSrc)
			V_BOOL(this) = AFX_OLE_FALSE;
		else
			V_BOOL(this) = AFX_OLE_TRUE;
	}
	else if (vtSrc == VT_INT)
	{
		vt = VT_INT;
	 	V_INT(this) = lSrc;
	}
	else if (vtSrc == VT_UINT)
	{
		vt = VT_UINT;
	 	V_UINT(this) = lSrc;
	}
	else if (vtSrc == VT_HRESULT)
	{
		vt = VT_HRESULT;
		V_ERROR(this) = lSrc;
	}
	else if (vtSrc == VT_UI4)
	{
		vt = VT_UI4;
		lVal = lSrc;
	}
	else
	{
		vt = VT_I4;
		lVal = lSrc;
	}
}

// Operations
void COleVariant::ChangeType(VARTYPE vartype, LPVARIANT pSrc)
{
	// If pSrc is NULL, convert type in place
	if (pSrc == NULL)
		pSrc = this;
	if (pSrc != this || vartype != vt)
		AfxCheckError(::VariantChangeType(this, pSrc, 0, vartype));
}

void COleVariant::Attach(VARIANT& varSrc)
{
	// Free up previous VARIANT
	Clear();

	// give control of data to COleVariant 
	Checked::memcpy_s(this, sizeof(VARIANT), &varSrc, sizeof(varSrc));
	varSrc.vt = VT_EMPTY;
}

VARIANT COleVariant::Detach()
{
	VARIANT varResult = *this;
	vt = VT_EMPTY;
	return varResult;
}

void COleVariant::GetByteArrayFromVariantArray(CByteArray& bytes)
{
	ASSERT( V_ISARRAY(this) );

	LPVOID pSrc;
	LPVOID pDest;
	HRESULT hResult;
	ULONG nDim;
	LONG iLowerBound;
	LONG iUpperBound;
	LONG nElements;
	ULONG nBytes;

	hResult = ::SafeArrayAccessData(V_ARRAY(this), &pSrc);
	AfxCheckError(hResult);
	
	nDim = ::SafeArrayGetDim(V_ARRAY(this));
	ASSERT( (nDim == 0) || (nDim == 1) );

	if (nDim == 1)
	{
		::SafeArrayGetLBound(V_ARRAY(this), 1, &iLowerBound);
		::SafeArrayGetUBound(V_ARRAY(this), 1, &iUpperBound);
		nElements = (iUpperBound-iLowerBound)+1;
		nBytes = nElements*::SafeArrayGetElemsize(V_ARRAY(this));
		bytes.SetSize( nBytes );
		pDest = bytes.GetData();
		Checked::memcpy_s(pDest, nBytes, pSrc, nBytes);
	}
	else
	{
		bytes.SetSize(0);
	}

	::SafeArrayUnaccessData(V_ARRAY(this));
}

// Literal comparison. Types and values must match.
BOOL COleVariant::operator==(const VARIANT& var) const
{
	if (&var == this)
		return TRUE;

	// Variants not equal if types don't match
	if (var.vt != vt)
		return FALSE;

	// Check type specific values
	switch (vt)
	{
	case VT_EMPTY:
	case VT_NULL:
		return TRUE;

	case VT_BOOL:
		return V_BOOL(&var) == V_BOOL(this);

	case VT_I1:
		return var.cVal == cVal;

	case VT_UI1:
		return var.bVal == bVal;

	case VT_I2:
		return var.iVal == iVal;

	case VT_UI2:
		return var.uiVal == uiVal;

	case VT_I4:
		return var.lVal == lVal;

	case VT_UI4:
		return var.ulVal == ulVal;

	case VT_I8:
		return var.llVal == llVal;

	case VT_UI8:
		return var.ullVal == ullVal;

	case VT_CY:
		return (var.cyVal.Hi == cyVal.Hi && var.cyVal.Lo == cyVal.Lo);

	case VT_R4:
		return var.fltVal == fltVal;

	case VT_R8:
		return var.dblVal == dblVal;

	case VT_DATE:
		return var.date == date;

	case VT_BSTR:
		return SysStringByteLen(var.bstrVal) == SysStringByteLen(bstrVal) &&
			memcmp(var.bstrVal, bstrVal, SysStringByteLen(bstrVal)) == 0;

	case VT_ERROR:
		return var.scode == scode;

	case VT_DISPATCH:
	case VT_UNKNOWN:
		return var.punkVal == punkVal;

	default:
		if (vt & VT_ARRAY && !(vt & VT_BYREF))
			return _AfxCompareSafeArrays(var.parray, parray);
		else
			ASSERT(FALSE);  // VT_BYREF not supported
		// fall through
	}

	return FALSE;
}

const COleVariant& COleVariant::operator=(const VARIANT& varSrc)
{
	if(static_cast<LPVARIANT>(this) != &varSrc)
	{
	AfxCheckError(::VariantCopy(this, (LPVARIANT)&varSrc));
	}

	return *this;
}

const COleVariant& COleVariant::operator=(LPCVARIANT pSrc)
{
	if(static_cast<LPCVARIANT>(this) != pSrc)
	{
	AfxCheckError(::VariantCopy(this, (LPVARIANT)pSrc));
	}

	return *this;
}

const COleVariant& COleVariant::operator=(const COleVariant& varSrc)
{
	if(this != &varSrc)
	{
	AfxCheckError(::VariantCopy(this, (LPVARIANT)&varSrc));
	}

	return *this;
}

const COleVariant& COleVariant::operator=(const LPCTSTR lpszSrc)
{
	// Free up previous VARIANT
	Clear();

	vt = VT_BSTR;
	if (lpszSrc == NULL)
		bstrVal = NULL;
	else
	{
		bstrVal = CTempStringW(lpszSrc).AllocSysString();
	}
	return *this;
}

const COleVariant& COleVariant::operator=(const CString& strSrc)
{
	// Free up previous VARIANT
	Clear();

	vt = VT_BSTR;
	bstrVal = strSrc.AllocSysString();

	return *this;
}

const COleVariant& COleVariant::operator=(BYTE nSrc)
{
	// Free up previous VARIANT if necessary
	if (vt != VT_UI1)
	{
		Clear();
		vt = VT_UI1;
	}

	bVal = nSrc;
	return *this;
}

const COleVariant& COleVariant::operator=(short nSrc)
{
	if (vt == VT_I2)
		iVal = nSrc;
	else if (vt == VT_BOOL)
	{
		if (!nSrc)
			V_BOOL(this) = AFX_OLE_FALSE;
		else
			V_BOOL(this) = AFX_OLE_TRUE;
	}
	else
	{
		// Free up previous VARIANT
		Clear();
		vt = VT_I2;
		iVal = nSrc;
	}

	return *this;
}

const COleVariant& COleVariant::operator=(long lSrc)
{
	if (vt == VT_I4)
		lVal = lSrc;
	else if (vt == VT_ERROR)
		scode = lSrc;
	else if (vt == VT_BOOL)
	{
		if (!lSrc)
			V_BOOL(this) = AFX_OLE_FALSE;
		else
			V_BOOL(this) = AFX_OLE_TRUE;
	}
	else
	{
		// Free up previous VARIANT
		Clear();
		vt = VT_I4;
		lVal = lSrc;
	}

	return *this;
}

#if (_WIN32_WINNT >= 0x0501) || defined(_ATL_SUPPORT_VT_I8)
const COleVariant& COleVariant::operator=(LONGLONG nSrc)
{
	if (vt != VT_I8)
	{
		Clear();
		vt = VT_I8;
	}

	llVal = nSrc;
	return *this;
}

const COleVariant& COleVariant::operator=(ULONGLONG nSrc)
{
	if (vt != VT_UI8)
	{
		Clear();
		vt = VT_UI8;
	}

	ullVal = nSrc;
	return *this;
}
#endif

const COleVariant& COleVariant::operator=(const COleCurrency& curSrc)
{
	// Free up previous VARIANT if necessary
	if (vt != VT_CY)
	{
		Clear();
		vt = VT_CY;
	}

	cyVal = curSrc.m_cur;
	return *this;
}

const COleVariant& COleVariant::operator=(float fltSrc)
{
	// Free up previous VARIANT if necessary
	if (vt != VT_R4)
	{
		Clear();
		vt = VT_R4;
	}

	fltVal = fltSrc;
	return *this;
}

const COleVariant& COleVariant::operator=(double dblSrc)
{
	// Free up previous VARIANT if necessary
	if (vt != VT_R8)
	{
		Clear();
		vt = VT_R8;
	}

	dblVal = dblSrc;
	return *this;
}

const COleVariant& COleVariant::operator=(const COleDateTime& dateSrc)
{
	// Free up previous VARIANT if necessary
	if (vt != VT_DATE)
	{
		Clear();
		vt = VT_DATE;
	}

	date = dateSrc;
	return *this;
}

const COleVariant& COleVariant::operator=(const CByteArray& arrSrc)
{
	INT_PTR nSize = arrSrc.GetSize();
	if( nSize > LONG_MAX )
	{
		AfxThrowMemoryException();
	}

	// Set the correct type and make sure SafeArray can hold data
	_AfxCreateOneDimArray(*this, (DWORD)nSize);

	// Copy the data into the SafeArray
	_AfxCopyBinaryData(parray, arrSrc.GetData(), (DWORD)nSize);

	return *this;
}

const COleVariant& COleVariant::operator=(const CLongBinary& lbSrc)
{
	// Set the correct type and make sure SafeArray can hold data
	if( lbSrc.m_dwDataLength > LONG_MAX )
	{
		AfxThrowMemoryException();
	}
	_AfxCreateOneDimArray(*this, (ULONG)lbSrc.m_dwDataLength);

	// Copy the data into the SafeArray
	BYTE* pData = (BYTE*)::GlobalLock(lbSrc.m_hData);
	_AfxCopyBinaryData(parray, pData, (ULONG)lbSrc.m_dwDataLength);
	::GlobalUnlock(lbSrc.m_hData);

	return *this;
}

void AFXAPI AfxVariantInit(LPVARIANT pVar)
{
	memset(pVar, 0, sizeof(*pVar));
}

/////////////////////////////////////////////////////////////////////////////
// Diagnostics

#ifdef _DEBUG
CDumpContext& AFXAPI operator <<(CDumpContext& dc, COleVariant varSrc)
{
	LPCVARIANT pSrc = (LPCVARIANT)varSrc;

	dc << "\nCOleVariant Object:";
	dc << "\n\t vt = " << pSrc->vt;

	// No support for VT_BYREF & VT_ARRAY
	if (pSrc->vt & VT_BYREF || pSrc->vt & VT_ARRAY)
		return dc;

	switch (pSrc->vt)
	{
	case VT_BOOL:
		return dc << "\n\t VT_BOOL = " << V_BOOL(pSrc);

	case VT_I1:
		return dc << "\n\t cVal = " << pSrc->cVal;

	case VT_UI1:
		return dc << "\n\t bVal = " << pSrc->bVal;

	case VT_I2:
		return dc << "\n\t iVal = " << pSrc->iVal;

	case VT_UI2:
		return dc << "\n\t uiVal = " << pSrc->uiVal;

	case VT_I4:
		return dc << "\n\t lVal = " << pSrc->lVal;

	case VT_UI4:
		return dc << "\n\t ulVal = " << pSrc->ulVal;

	case VT_I8:
		return dc << "\n\t llVal = " << pSrc->llVal;

	case VT_UI8:
		return dc << "\n\t ullVal = " << pSrc->ullVal;

	case VT_CY:
		{
			COleVariant var(varSrc);
			var.ChangeType(VT_BSTR);
			return dc << "\n\t cyVal = " << var.bstrVal;
		}

	case VT_R4:
		return dc << "\n\t fltVal = " << pSrc->fltVal;

	case VT_R8:
		return dc << "\n\t dblVal = " << pSrc->dblVal;

	case VT_DATE:
		{
			COleVariant var(varSrc);
			var.ChangeType(VT_BSTR);
			return dc << "\n\t date = " << var.bstrVal;
		}

	case VT_BSTR:
		return dc << "\n\t bstrVal = " << pSrc->bstrVal;

	case VT_ERROR:
		return dc << "\n\t scode = " << pSrc->scode;

	case VT_DISPATCH:
	case VT_UNKNOWN:
		return dc << "\n\t punkVal = " << pSrc->punkVal;

	case VT_EMPTY:
	case VT_NULL:
		return dc;

	default:
		ASSERT(FALSE);
		return dc;
	}
}
#endif // _DEBUG

CArchive& AFXAPI operator<<(CArchive& ar, COleVariant varSrc)
{
	LPCVARIANT pSrc = (LPCVARIANT)varSrc;

	ar << pSrc->vt;

	// No support for VT_BYREF & VT_ARRAY
	if (pSrc->vt & VT_BYREF || pSrc->vt & VT_ARRAY)
		return ar;

	switch (pSrc->vt)
	{
	case VT_BOOL:
		return ar << (WORD)V_BOOL(pSrc);

	case VT_I1:
		return ar << pSrc->cVal;

	case VT_UI1:
		return ar << pSrc->bVal;

	case VT_I2:
		return ar << pSrc->iVal;

	case VT_UI2:
		return ar << pSrc->uiVal;

	case VT_I4:
		return ar << pSrc->lVal;

	case VT_UI4:
		return ar << pSrc->ulVal;

	case VT_I8:
		return ar << pSrc->llVal;

	case VT_UI8:
		return ar << pSrc->ullVal;

	case VT_CY:
		ar << pSrc->cyVal.Lo;
		return ar << pSrc->cyVal.Hi;

	case VT_R4:
		return ar << pSrc->fltVal;

	case VT_R8:
		return ar << pSrc->dblVal;

	case VT_DATE:
		return ar << pSrc->date;

	case VT_BSTR:
		{
			DWORD nLen = SysStringByteLen(pSrc->bstrVal);
			ar << nLen;
			if (nLen > 0)
				ar.Write(pSrc->bstrVal, nLen * sizeof(BYTE));

			return ar;
		}

	case VT_ERROR:
		return ar << pSrc->scode;

	case VT_DISPATCH:
	case VT_UNKNOWN:
		{
			LPPERSISTSTREAM pPersistStream;
			CArchiveStream stm(&ar);

			// QI for IPersistStream or IPeristStreamInit
			SCODE sc = pSrc->punkVal->QueryInterface(
				IID_IPersistStream, (void**)&pPersistStream);
#ifndef _AFX_NO_OCC_SUPPORT
			if (FAILED(sc))
				sc = pSrc->punkVal->QueryInterface(
					IID_IPersistStreamInit, (void**)&pPersistStream);
#endif
			AfxCheckError(sc);

			TRY
			{
				// Get and archive the CLSID (GUID)
				CLSID clsid;
				AfxCheckError(pPersistStream->GetClassID(&clsid));
				ar << clsid.Data1;
				ar << clsid.Data2;
				ar << clsid.Data3;
				ar.Write(&clsid.Data4[0], sizeof clsid.Data4);

				// Always assume object is dirty
				AfxCheckError(pPersistStream->Save(&stm, TRUE));
			}
			CATCH_ALL(e)
			{
				pPersistStream->Release();
				THROW_LAST();
			}
			END_CATCH_ALL
			pPersistStream->Release();
		}
		return ar;

	case VT_EMPTY:
	case VT_NULL:
		// do nothing
		return ar;

	default:
		ASSERT(FALSE);
		return ar;
	}
}

CArchive& AFXAPI operator>>(CArchive& ar, COleVariant& varSrc)
{
	LPVARIANT pSrc = &varSrc;

	// Free up current data if necessary
	if (pSrc->vt != VT_EMPTY)
		VariantClear(pSrc);
	ar >> pSrc->vt;

	// No support for VT_BYREF & VT_ARRAY
	if (pSrc->vt & VT_BYREF || pSrc->vt & VT_ARRAY)
		return ar;

	switch (pSrc->vt)
	{
	case VT_BOOL:
		return ar >> (WORD&)V_BOOL(pSrc);

	case VT_I1:
		return ar >> pSrc->cVal;

	case VT_UI1:
		return ar >> pSrc->bVal;

	case VT_I2:
		return ar >> pSrc->iVal;

	case VT_UI2:
		return ar >> pSrc->uiVal;

	case VT_I4:
		return ar >> pSrc->lVal;

	case VT_UI4:
		return ar >> pSrc->ulVal;

	case VT_I8:
		return ar >> pSrc->llVal;

	case VT_UI8:
		return ar >> pSrc->ullVal;

	case VT_CY:
		ar >> pSrc->cyVal.Lo;
		return ar >> pSrc->cyVal.Hi;

	case VT_R4:
		return ar >> pSrc->fltVal;

	case VT_R8:
		return ar >> pSrc->dblVal;

	case VT_DATE:
		return ar >> pSrc->date;

	case VT_BSTR:
		{
			DWORD nLen;
			ar >> nLen;
			if (nLen > 0)
			{
				pSrc->bstrVal = SysAllocStringByteLen(NULL, nLen);
				if (pSrc->bstrVal == NULL)
					AfxThrowMemoryException();
				ar.EnsureRead(pSrc->bstrVal, nLen * sizeof(BYTE));
			}
			else
				pSrc->bstrVal = NULL;

			return ar;
		}
		break;

	case VT_ERROR:
		return ar >> pSrc->scode;

	case VT_DISPATCH:
	case VT_UNKNOWN:
		{
			LPPERSISTSTREAM pPersistStream = NULL;
			CArchiveStream stm(&ar);

			// Retrieve the CLSID (GUID) and create an instance
			CLSID clsid;
			ar >> clsid.Data1;
			ar >> clsid.Data2;
			ar >> clsid.Data3;
			ar.EnsureRead(&clsid.Data4[0], sizeof clsid.Data4);

			// Create the object
			SCODE sc = CoCreateInstance(clsid, NULL, CLSCTX_ALL | CLSCTX_REMOTE_SERVER,
				pSrc->vt == VT_UNKNOWN ? IID_IUnknown : IID_IDispatch,
				(void**)&pSrc->punkVal);
			if (sc == E_INVALIDARG)
			{
				// may not support CLSCTX_REMOTE_SERVER, so try without
				sc = CoCreateInstance(clsid, NULL,
					CLSCTX_ALL & ~CLSCTX_REMOTE_SERVER,
					pSrc->vt == VT_UNKNOWN ? IID_IUnknown : IID_IDispatch,
					(void**)&pSrc->punkVal);
			}
			AfxCheckError(sc);

			TRY
			{
				// QI for IPersistStream or IPeristStreamInit
				sc = pSrc->punkVal->QueryInterface(
					IID_IPersistStream, (void**)&pPersistStream);
#ifndef _AFX_NO_OCC_SUPPORT
				if (FAILED(sc))
					sc = pSrc->punkVal->QueryInterface(
						IID_IPersistStreamInit, (void**)&pPersistStream);
#endif
				AfxCheckError(sc);

				// Always assumes object is dirty
				AfxCheckError(pPersistStream->Load(&stm));
			}
			CATCH_ALL(e)
			{
				// Clean up
				if (pPersistStream != NULL)
					pPersistStream->Release();

				pSrc->punkVal->Release();
				THROW_LAST();
			}
			END_CATCH_ALL

			pPersistStream->Release();
		}
		return ar;

	case VT_EMPTY:
	case VT_NULL:
		// do nothing
		return ar;

	default:
		ASSERT(FALSE);
		return ar;
	}
}


//////////////////////////////////////////////////////////////////////////////
// CComBSTR support

#ifdef __ATLBASE_H__

CArchive& AFXAPI operator<<(CArchive& ar, CComBSTR string)
{
	ar << (DWORD)string.Length();
	if( string.Length() > 0 )
		ar.Write(string.m_str, string.Length()*sizeof(OLECHAR));
	return ar;
}

CArchive& AFXAPI operator>>(CArchive& ar, CComBSTR& string)
{
	DWORD length;
	ar >> length;
	if( length > 0 )
	{
		BSTR temp = string.Detach();
		SysReAllocStringLen( &temp, NULL, length );
		ar.EnsureRead(temp, length*sizeof(OLECHAR));
		string.Attach( temp );
	}
	else
		string.Empty();
	return ar;
}

#endif // __ATLBASE_H__

/////////////////////////////////////////////////////////////////////////////
// COleVariant Helpers
#if _MSC_VER >= 1100
template <> void AFXAPI CopyElements<COleVariant> (COleVariant* pDest, const COleVariant* pSrc, INT_PTR nCount)
#else
void AFXAPI CopyElements(COleVariant* pDest, const COleVariant* pSrc, INT_PTR nCount)
#endif
{
	ENSURE_ARG(nCount == 0 || pDest != NULL && pSrc != NULL);
	ASSERT(nCount == 0 ||
		AfxIsValidAddress(pDest, (size_t)nCount * sizeof(COleVariant)));
	ASSERT(nCount == 0 ||
		AfxIsValidAddress(pSrc, (size_t)nCount * sizeof(COleVariant)));

	for (; nCount--; ++pDest, ++pSrc)
		*pDest = *pSrc;
}

#if _MSC_VER >= 1100
template <> void AFXAPI SerializeElements<COleVariant> (CArchive& ar, COleVariant* pElements, INT_PTR nCount)
#else
void AFXAPI SerializeElements(CArchive& ar, COleVariant* pElements, INT_PTR nCount)
#endif
{
	ENSURE_ARG(nCount == 0 || pElements != NULL);
	ASSERT(nCount == 0 ||
		AfxIsValidAddress(pElements, (size_t)nCount * sizeof(COleVariant)));

	if (ar.IsStoring())
	{
		for (; nCount--; ++pElements)
			ar << *pElements;
	}
	else
	{
		for (; nCount--; ++pElements)
			ar >> *pElements;
	}
}

#ifdef _DEBUG
#if _MSC_VER >= 1100
template <> void AFXAPI DumpElements<COleVariant> (CDumpContext& dc, const COleVariant* pElements, INT_PTR nCount)
#else
void AFXAPI DumpElements(CDumpContext& dc, const COleVariant* pElements, INT_PTR nCount)
#endif
{
	ENSURE_ARG(nCount == 0 || pElements != NULL);
	for (; nCount--; ++pElements)
		dc << *pElements;
}
#endif // _DEBUG

#if _MSC_VER >= 1100
template<> UINT AFXAPI HashKey<const struct tagVARIANT&> (const struct tagVARIANT& var)
#else
UINT AFXAPI HashKey(const struct tagVARIANT& var)
#endif
{
	switch (var.vt)
	{
	case VT_EMPTY:
	case VT_NULL:
		return 0;
	case VT_I2:
#if _MSC_VER >= 1100
		return HashKey<DWORD>((DWORD)var.iVal);
#else
		return HashKey((DWORD)var.iVal);
#endif
	case VT_I4:
#if _MSC_VER >= 1100
		return HashKey<DWORD>((DWORD)var.lVal);
#else
		return HashKey((DWORD)var.lVal);
#endif
	case VT_R4:
		return (UINT)(var.fltVal / 16);
	case VT_R8:
	case VT_CY:
		return (UINT)(var.dblVal / 16);
	case VT_BOOL:
#if _MSC_VER >= 1100
		return HashKey<DWORD>((DWORD)V_BOOL(&var));
#else
		return HashKey((DWORD)V_BOOL(&var));
#endif
	case VT_ERROR:
#if _MSC_VER >= 1100
		return HashKey<DWORD>((DWORD)var.scode);
#else
		return HashKey((DWORD)var.scode);
#endif
	case VT_DATE:
		return (UINT)(var.date / 16);
	case VT_BSTR:
#if _MSC_VER >= 1100
		return HashKey<LPCOLESTR>(var.bstrVal);
#else
		return HashKey((LPCOLESTR)var.bstrVal);
#endif
	case VT_DISPATCH:
	case VT_UNKNOWN:
#if _MSC_VER >= 1100
#ifdef _WIN64
		return HashKey<DWORD_PTR>((DWORD_PTR)var.punkVal);
#else
		return HashKey<DWORD>((DWORD)(DWORD_PTR)var.punkVal);
#endif
#else
		return HashKey((DWORD_PTR)var.punkVal);
#endif

	default:
		// No support for VT_BYREF, VT_ARRAY, VT_VARIANT, VT_DECIMAL, & VT_UI1
		ASSERT(FALSE);

		// Fall through
	}

	return 0;
}


/////////////////////////////////////////////////////////////////////////////
// COleCurrency class (internally currency is 8-byte int scaled by 10,000)

COleCurrency::COleCurrency(long nUnits, long nFractionalUnits)
{
	SetCurrency(nUnits, nFractionalUnits);
	SetStatus(valid);
}

const COleCurrency& COleCurrency::operator=(CURRENCY cySrc)
{
	m_cur = cySrc;
	SetStatus(valid);
	return *this;
}

const COleCurrency& COleCurrency::operator=(const COleCurrency& curSrc)
{
	m_cur = curSrc.m_cur;
	m_status = curSrc.m_status;
	return *this;
}

const COleCurrency& COleCurrency::operator=(const VARIANT& varSrc)
{
	if (varSrc.vt != VT_CY)
	{
		TRY
		{
			COleVariant varTemp(varSrc);
			varTemp.ChangeType(VT_CY);
			m_cur = varTemp.cyVal;
			SetStatus(valid);
		}
		// Catch COleException from ChangeType, but not CMemoryException
		CATCH(COleException, e)
		{
			// Not able to convert VARIANT to CURRENCY
		 m_cur.int64 = 0;
			SetStatus(invalid);
			DELETE_EXCEPTION(e);
		}
		END_CATCH
	}
	else
	{
		m_cur = varSrc.cyVal;
		SetStatus(valid);
	}

	return *this;
}

BOOL COleCurrency::operator<(const COleCurrency& cur) const
{
	ASSERT(GetStatus() == valid);
	ASSERT(cur.GetStatus() == valid);

   return(m_cur.int64 < cur.m_cur.int64);
}

BOOL COleCurrency::operator>(const COleCurrency& cur) const
{
	ASSERT(GetStatus() == valid);
	ASSERT(cur.GetStatus() == valid);

   return(m_cur.int64 > cur.m_cur.int64);
}

BOOL COleCurrency::operator<=(const COleCurrency& cur) const
{
	ASSERT(GetStatus() == valid);
	ASSERT(cur.GetStatus() == valid);

   return(m_cur.int64 <= cur.m_cur.int64);
}

BOOL COleCurrency::operator>=(const COleCurrency& cur) const
{
	ASSERT(GetStatus() == valid);
	ASSERT(cur.GetStatus() == valid);

   return(m_cur.int64 >= cur.m_cur.int64);
}

COleCurrency COleCurrency::operator+(const COleCurrency& cur) const
{
	COleCurrency curResult;

	// If either operand Null, result Null
	if (GetStatus() == null || cur.GetStatus() == null)
	{
		curResult.SetStatus(null);
		return curResult;
	}

	// If either operand Invalid, result Invalid
	if (GetStatus() == invalid || cur.GetStatus() == invalid)
	{
		curResult.SetStatus(invalid);
		return curResult;
	}

	// Add separate CURRENCY components
   curResult.m_cur.int64 = m_cur.int64+cur.m_cur.int64;

	// Overflow if operands same sign and result sign different
	if (!((m_cur.Hi ^ cur.m_cur.Hi) & 0x80000000) &&
		((m_cur.Hi ^ curResult.m_cur.Hi) & 0x80000000))
	{
		curResult.SetStatus(invalid);
	}

	return curResult;
}

COleCurrency COleCurrency::operator-(const COleCurrency& cur) const
{
	COleCurrency curResult;

	// If either operand Null, result Null
	if (GetStatus() == null || cur.GetStatus() == null)
	{
		curResult.SetStatus(null);
		return curResult;
	}

	// If either operand Invalid, result Invalid
	if (GetStatus() == invalid || cur.GetStatus() == invalid)
	{
		curResult.SetStatus(invalid);
		return curResult;
	}

	// Subtract separate CURRENCY components
   curResult.m_cur.int64 = m_cur.int64-cur.m_cur.int64;

	// Overflow if operands not same sign and result not same sign
	if (((m_cur.Hi ^ cur.m_cur.Hi) & 0x80000000) &&
		((m_cur.Hi ^ curResult.m_cur.Hi) & 0x80000000))
	{
		curResult.SetStatus(invalid);
	}

	return curResult;
}

COleCurrency COleCurrency::operator-() const
{
	// If operand not Valid, just return
	if (!GetStatus() == valid)
		return *this;

	COleCurrency curResult;

	// Negating MIN_CURRENCY,will set invalid
   if (m_cur.int64 == _I64_MIN)
	{
		curResult.SetStatus(invalid);
	}

   curResult.m_cur.int64 = -m_cur.int64;

	return curResult;
}

COleCurrency COleCurrency::operator*(long nOperand) const
{
	// If operand not Valid, just return
	if (GetStatus() != valid)
		return *this;

	COleCurrency curResult(m_cur);

	// Return now if one operand is 0 (optimization)
   if ((m_cur.int64 == 0) || (nOperand == 0))
	{
		curResult.m_cur.int64 = 0;
		return curResult;
	}

   ULONGLONG nAbsCur;  // |m_cur.int64|
	ULONG nAbsOp;  // |nOperand|
   ULONGLONG nTempHi;  // Product of nAbsOp and high-order 32 bits of nAbsCur
   ULONGLONG nTempLo;  // Product of nAbsOp and low-order 32 bits of nAbsCur
   ULONGLONG nTempProduct;  // Sum of both partial products
   bool bProductNegative;
   bool bOverflow;

   if( (nOperand^m_cur.Hi)&0x80000000 )
   {
	  bProductNegative = true;
   }
   else
   {
	  bProductNegative = false;
   }
   bOverflow = false;

	// Compute absolute values.
   nAbsCur = (m_cur.int64 < 0) ? -m_cur.int64 : m_cur.int64;
	nAbsOp = labs(nOperand);

   // Compute the high-order partial product
   nTempHi = (nAbsCur>>32)*nAbsOp;
   if( nTempHi > (ULONGLONG( LONG_MAX )+1) )
   {
	  // High-order multiplication overflowed
	  bOverflow = true;
   }
   else
   {
	  // Compute the low-order partial product.  Since nAbsOp is no larger 
	  // than 2**31, this multiplication can't overflow.
	  nTempLo = ULONGLONG( ULONG( nAbsCur ) )*nAbsOp;

	  // Sum the two partial products
	  nTempProduct = nTempLo+(nTempHi<<32);
	  if( nTempProduct < nTempLo )
	  {
		 // The sum overflowed the 64-bit result
		 bOverflow = true;
	  }
	  else
	  {
		 // At this point, nTempProduct may be still be out of range of a 
		 // positive signed 64-bit integer, but we do know that we haven't 
		 // overflowed out of our unsigned 64-bit integer variable.  
		 // If nTempProduct is greater than _I64_MAX, it will appear to be
		 // a negative signed 64-bit integer, so our result will appear to
		 // have the wrong sign.  The special case of nTempProduct == 
		 // -_I64_MIN works correctly, since negating that number is a NOP.
		 // That will

		 // Try to give the result the correct sign.
		 if( bProductNegative )
		 {
			curResult.m_cur.int64 = -LONGLONG( nTempProduct );
			// Possible cases:
			//   a) nTempProduct <= _I64_MAX: No problem.
			//   b) nTempProduct > -_I64_MIN: The result of the negation will 
			//      be positive, which will fail our check for the correct
			//      sign (which we'll do below).
			//   c) nTempProduct == -I64_MIN: The result of the negation will
			//      be _I64_MIN, which is the correct result.
		 }
		 else
		 {
			curResult.m_cur.int64 = nTempProduct;
			// Possible cases:
			//   a) nTempProduct <= _I64_MAX: No problem.
			//   b) nTempProduct > _I64_MAX: The result of the signed result
			//      will be negative, which will fail our check for the 
			//      correct sign (which we'll do below).
		 }
		 if( (curResult.m_cur.Hi^m_cur.Hi^nOperand)&0x80000000 )
		 {
			// The sign of the result isn't what we wanted, so there must have
			// been an overflow.
			bOverflow = true;
		 }
	  }
   }

   if( bOverflow )
   {
	  curResult.SetStatus( invalid );
	  if( bProductNegative )
	  {
		 curResult.m_cur.int64 = _I64_MIN;
	  }
	  else
	  {
		 curResult.m_cur.int64 = _I64_MAX;
	  }
   }

	return curResult;
}

COleCurrency COleCurrency::operator/(long nOperand) const
{
	// If operand not Valid, just return
	if (!GetStatus() == valid)
		return *this;

	COleCurrency curResult;

	// Check for divide by 0
	if (nOperand == 0)
	{
		curResult.SetStatus(invalid);

		// Set to maximum negative value
	  curResult.m_cur.int64 = _I64_MIN;

		return curResult;
	}

   // Check for MIN_CURRENCY/-1
   if ((nOperand == -1) && (m_cur.int64 == _I64_MIN))
   {
	  curResult.SetStatus(invalid);

	  curResult.m_cur.int64 = _I64_MAX;

	  return curResult;
   }

   curResult.m_cur.int64 = m_cur.int64/nOperand;

	return curResult;
}

void COleCurrency::SetCurrency(long nUnits, long nFractionalUnits)
{
	COleCurrency curUnits;              // Initializes to 0
	COleCurrency curFractionalUnits;    // Initializes to 0

	// Set temp currency value to Units (need to multiply by 10,000)
	curUnits.m_cur.Lo = (DWORD)labs(nUnits);
	curUnits = curUnits * 10000;
	if (nUnits < 0)
		curUnits = -curUnits;

	curFractionalUnits.m_cur.Lo = (DWORD)labs(nFractionalUnits);
	if (nFractionalUnits < 0)
		curFractionalUnits = -curFractionalUnits;

	// Now add together Units and FractionalUnits
	*this = curUnits + curFractionalUnits;

	SetStatus(valid);
}

BOOL COleCurrency::ParseCurrency(LPCTSTR lpszCurrency,
	DWORD dwFlags,  LCID lcid)
{
	SCODE sc;
	if ( FAILED(sc = VarCyFromStr(const_cast<LPOLESTR>(CStringW(lpszCurrency).GetString()),
		lcid, dwFlags, &m_cur)))
	{
		if (sc == DISP_E_TYPEMISMATCH)
		{
			// Can't convert string to CURRENCY, set 0 & invalid
		 m_cur.int64 = 0;
			SetStatus(invalid);
			return FALSE;
		}
		else if (sc == DISP_E_OVERFLOW)
		{
			// Can't convert string to CURRENCY, set max neg & invalid
		 m_cur.int64 = _I64_MIN;
			SetStatus(invalid);
			return FALSE;
		}
		else
		{
			TRACE(traceOle, 0, "\nCOleCurrency VarCyFromStr call failed.\n\t");
			if (sc == E_OUTOFMEMORY)
				AfxThrowMemoryException();
			else
				AfxThrowOleException(sc);
		}
	}

	SetStatus(valid);
	return TRUE;
}

CString COleCurrency::Format(DWORD dwFlags, LCID lcid) const
{
	CString strCur;

	// If null, return empty string
	if (GetStatus() == null)
		return strCur;

	// If invalid, return Currency resource string
	if (GetStatus() == invalid)
	{
		ENSURE(strCur.LoadString(AFX_IDS_INVALID_CURRENCY));
		return strCur;
	}

	COleVariant var;
	// Don't need to trap error. Should not fail due to type mismatch
	AfxCheckError(VarBstrFromCy(m_cur, lcid, dwFlags, &V_BSTR(&var)));
	var.vt = VT_BSTR;
	return V_BSTR(&var);
}


// serialization
#ifdef _DEBUG
CDumpContext& AFXAPI operator<<(CDumpContext& dc, COleCurrency curSrc)
{
	dc << "\nCOleCurrency Object:";
	dc << "\n\tm_status = " << (long)curSrc.m_status;

	return dc << "\n\tCurrency Hi = " << curSrc.m_cur.Hi << ", Lo = " << curSrc.m_cur.Lo;
}
#endif // _DEBUG

CArchive& AFXAPI operator<<(CArchive& ar, COleCurrency curSrc)
{
	ar << (long)curSrc.m_status;
	ar << curSrc.m_cur.Hi;
	return ar << curSrc.m_cur.Lo;
}

CArchive& AFXAPI operator>>(CArchive& ar, COleCurrency& curSrc)
{
	ar >> (long&)curSrc.m_status;
	ar >> curSrc.m_cur.Hi;
	return ar >> curSrc.m_cur.Lo;
}

// serialization
#ifdef _DEBUG
CDumpContext& AFXAPI operator<<(CDumpContext& dc, COleDateTime dateSrc)
{
	dc << "\nCOleDateTime Object:";
	dc << "\n\tm_status = " << (long)dateSrc.m_status;

	COleVariant var(dateSrc);
	var.ChangeType(VT_BSTR);

	return dc << "\n\tdate = " << var.bstrVal;
}
#endif // _DEBUG

CArchive& AFXAPI operator<<(CArchive& ar, COleDateTime dateSrc)
{
	ar << (long)dateSrc.m_status;
	return ar << dateSrc.m_dt;
}

CArchive& AFXAPI operator>>(CArchive& ar, COleDateTime& dateSrc)
{
	ar >> (long&)dateSrc.m_status;
	return ar >> dateSrc.m_dt;
}

// serialization
#ifdef _DEBUG
CDumpContext& AFXAPI operator<<(CDumpContext& dc, COleDateTimeSpan dateSpanSrc)
{
	dc << "\nCOleDateTimeSpan Object:";
	dc << "\n\tm_status = " << (long)dateSpanSrc.m_status;
	COleVariant var(dateSpanSrc.m_span);
	var.ChangeType(VT_BSTR);

	return dc << "\n\tdateSpan = " << var.bstrVal;
}
#endif // _DEBUG

CArchive& AFXAPI operator<<(CArchive& ar, COleDateTimeSpan dateSpanSrc)
{
	ar << (long)dateSpanSrc.m_status;
	return ar << dateSpanSrc.m_span;
}

CArchive& AFXAPI operator>>(CArchive& ar, COleDateTimeSpan& dateSpanSrc)
{
	ar >> (long&)dateSpanSrc.m_status;
	return ar >> dateSpanSrc.m_span;
}

/////////////////////////////////////////////////////////////////////////////
// COleSafeArray class

COleSafeArray::COleSafeArray(const SAFEARRAY& saSrc, VARTYPE vtSrc)
{
	AfxSafeArrayInit(this);
	vt = (VARTYPE)(vtSrc | VT_ARRAY);
	AfxCheckError(::SafeArrayCopy((LPSAFEARRAY)&saSrc, &parray));
	m_dwDims = GetDim();
	m_dwElementSize = GetElemSize();
}

COleSafeArray::COleSafeArray(LPCSAFEARRAY pSrc, VARTYPE vtSrc)
{
	AfxSafeArrayInit(this);
	vt = (VARTYPE)(vtSrc | VT_ARRAY);
	AfxCheckError(::SafeArrayCopy((LPSAFEARRAY)pSrc, &parray));
	m_dwDims = GetDim();
	m_dwElementSize = GetElemSize();
}

COleSafeArray::COleSafeArray(const COleSafeArray& saSrc)
{
	AfxSafeArrayInit(this);
	*this = saSrc;
	m_dwDims = GetDim();
	m_dwElementSize = GetElemSize();
}

COleSafeArray::COleSafeArray(const VARIANT& varSrc)
{
	AfxSafeArrayInit(this);
	*this = varSrc;
	m_dwDims = GetDim();
	m_dwElementSize = GetElemSize();
}

COleSafeArray::COleSafeArray(const COleVariant& varSrc)
{
	AfxSafeArrayInit(this);
	*this = varSrc;
	m_dwDims = GetDim();
	m_dwElementSize = GetElemSize();
}

COleSafeArray::COleSafeArray(LPCVARIANT pSrc)
{
	AfxSafeArrayInit(this);
	*this = pSrc;
	m_dwDims = GetDim();
	m_dwElementSize = GetElemSize();
}

// Operations
void COleSafeArray::Attach(VARIANT& varSrc)
{
	ASSERT(varSrc.vt & VT_ARRAY);
	
	if(!(varSrc.vt & VT_ARRAY))
		AfxThrowInvalidArgException();		
		
	// Free up previous safe array if necessary
	Clear();

	// give control of data to COleSafeArray 
	Checked::memcpy_s(this, sizeof(VARIANT), &varSrc, sizeof(varSrc));
	varSrc.vt = VT_EMPTY;
}

VARIANT COleSafeArray::Detach()
{
	VARIANT varResult = *this;
	vt = VT_EMPTY;
	return varResult;
}

void COleSafeArray::GetByteArray(CByteArray& bytes)
{
	LPVOID pSrc;
	LPVOID pDest;
	HRESULT hResult;
	ULONG nDim;
	LONG iLowerBound;
	LONG iUpperBound;
	LONG nElements;
	ULONG nBytes;

	ASSERT( V_ISARRAY(this) );

	hResult = ::SafeArrayAccessData(V_ARRAY(this), &pSrc);
	AfxCheckError(hResult);

	nDim = ::SafeArrayGetDim(V_ARRAY(this));
	ASSERT( (nDim == 0) || (nDim == 1) );

	if (nDim == 1)
	{
		::SafeArrayGetLBound(V_ARRAY(this), 1, &iLowerBound);
		::SafeArrayGetUBound(V_ARRAY(this), 1, &iUpperBound);
		nElements = (iUpperBound-iLowerBound)+1;
		nBytes = nElements*::SafeArrayGetElemsize(V_ARRAY(this));
		bytes.SetSize( nBytes );
		pDest = bytes.GetData();
		Checked::memcpy_s(pDest, nBytes, pSrc, nBytes);
	}
	else
	{
		bytes.SetSize(0);
	}

	::SafeArrayUnaccessData(V_ARRAY(this));
}

// Assignment operators
COleSafeArray& COleSafeArray::operator=(const COleSafeArray& saSrc)
{
	ASSERT(saSrc.vt & VT_ARRAY);

	if(!(saSrc.vt & VT_ARRAY))
		AfxThrowInvalidArgException();		

	AfxCheckError(::VariantCopy(this, (LPVARIANT)&saSrc));
	return *this;
}

COleSafeArray& COleSafeArray::operator=(const VARIANT& varSrc)
{
	ASSERT(varSrc.vt & VT_ARRAY);

	if(!(varSrc.vt & VT_ARRAY))
		AfxThrowInvalidArgException();		

	AfxCheckError(::VariantCopy(this, (LPVARIANT)&varSrc));
	return *this;
}

COleSafeArray& COleSafeArray::operator=(LPCVARIANT pSrc)
{
	ASSERT(pSrc->vt & VT_ARRAY);

	if(!(pSrc->vt & VT_ARRAY))
		AfxThrowInvalidArgException();		

	AfxCheckError(::VariantCopy(this, (LPVARIANT)pSrc));
	return *this;
}

COleSafeArray& COleSafeArray::operator=(const COleVariant& varSrc)
{
	ASSERT(varSrc.vt & VT_ARRAY);

	if(!(varSrc.vt & VT_ARRAY))
		AfxThrowInvalidArgException();		

	AfxCheckError(::VariantCopy(this, (LPVARIANT)&varSrc));
	return *this;
}

// Comparison operators
BOOL COleSafeArray::operator==(const SAFEARRAY& saSrc) const
{
	return _AfxCompareSafeArrays(parray, (LPSAFEARRAY)&saSrc);
}

BOOL COleSafeArray::operator==(LPCSAFEARRAY pSrc) const
{
	return _AfxCompareSafeArrays(parray, (LPSAFEARRAY)pSrc);
}

BOOL COleSafeArray::operator==(const COleSafeArray& saSrc) const
{
	if (vt != saSrc.vt)
		return FALSE;

	return _AfxCompareSafeArrays(parray, saSrc.parray);
}

BOOL COleSafeArray::operator==(const VARIANT& varSrc) const
{
	if (vt != varSrc.vt)
		return FALSE;

	return _AfxCompareSafeArrays(parray, varSrc.parray);
}

BOOL COleSafeArray::operator==(LPCVARIANT pSrc) const
{
	if (vt != pSrc->vt)
		return FALSE;

	return _AfxCompareSafeArrays(parray, pSrc->parray);
}

BOOL COleSafeArray::operator==(const COleVariant& varSrc) const
{
	if (vt != varSrc.vt)
		return FALSE;

	return _AfxCompareSafeArrays(parray, varSrc.parray);
}

#ifdef _DEBUG
void _AfxDumpSafeArrayElement(CDumpContext& dc, COleSafeArray& saSrc,
   long* piIndices)
{
   BYTE* pbData;

#pragma warning(push)
#pragma warning(disable:4068)
#pragma prefast(push)
#pragma prefast(disable:255, "alloca simple and efficient here, no need for __try in debug code")
   pbData = (BYTE*)_alloca( saSrc.GetElemSize() );
#pragma prefast(pop)
#pragma warning(pop)

   saSrc.GetElement(piIndices, pbData);
   switch(saSrc.vt&(~VT_ARRAY))
   {
   case VT_BOOL:
	  dc << *((VARIANT_BOOL*)pbData);
	  break;

   case VT_I1:
	  dc << *((char*)pbData);
	  break;

   case VT_I2:
	  dc << *((short*)pbData);
	  break;

   case VT_I4:
	  dc << *((long*)pbData);
	  break;

   case VT_I8:
	  dc << *((LONGLONG*)pbData);
	  break;

   case VT_UI1:
	  dc << *((BYTE*)pbData);
	  break;

   case VT_UI2:
	  dc << *((WORD*)pbData);
	  break;

   case VT_UI4:
	  dc << *((DWORD*)pbData);
	  break;

   case VT_UI8:
	  dc << *((ULONGLONG*)pbData);
	  break;

   case VT_R4:
	  dc << *((float*)pbData);
	  break;

   case VT_R8:
	  dc << *((double*)pbData);
	  break;

   case VT_CY:
	  {
		 COleVariant var;

		 var.vt = VT_CY;
		 var.cyVal = *((CY*)pbData);
		 var.ChangeType(VT_BSTR);
		 dc << var.bstrVal;
	  }
	  break;

   case VT_DATE:
	  {
		 COleVariant var;
		 var.vt = VT_DATE;
		 var.date = *((DATE*)pbData);
		 var.ChangeType(VT_BSTR);
		 dc << var.bstrVal;
	  }

   case VT_BSTR:
	  dc << *((BSTR*)pbData);
	  break;

   case VT_ERROR:
	  dc << *((SCODE*)pbData);
	  break;

   case VT_DISPATCH:
   case VT_UNKNOWN:
	  dc << *((IUnknown**)pbData);
	  break;

   case VT_VARIANT:
	  {
		 COleVariant var;

		 var = *((VARIANT*)pbData);
		 dc << var;
	  }
	  break;

   default:
	  ASSERT(FALSE);
	  break;
   }
}

CDumpContext& AFXAPI operator<<(CDumpContext& dc, COleSafeArray& saSrc)
{
   long iDimension;
   long nDimensions;
   long* piLBounds;
   long* piUBounds;
   long* piIndices;
   BOOL bWrapped;

   dc << "\nCOleSafeArray Object:";
   dc << "\n\tvt = " << saSrc.vt;
   dc << "\n\tbounds:";

   nDimensions = saSrc.GetDim();
   if (nDimensions > (LONG_MAX / sizeof(long)))
   {
	   AfxThrowMemoryException();
   }

#pragma warning(push)
#pragma warning(disable:4068)
#pragma prefast(push)
#pragma prefast(disable:255, "alloca simple and efficient here, no need for __try in debug code")
   piLBounds = (long*)_alloca( nDimensions*sizeof( long ) );
   piUBounds = (long*)_alloca( nDimensions*sizeof( long ) );
   piIndices = (long*)_alloca( nDimensions*sizeof( long ) );
#pragma prefast(pop)
#pragma warning(pop)
   // Dump the bounds
   for( iDimension = 0; iDimension < nDimensions; iDimension++ )
   {
	  saSrc.GetLBound( iDimension+1, &piLBounds[iDimension] );
	  saSrc.GetUBound( iDimension+1, &piUBounds[iDimension] );
	  dc << "\n\t(" << piLBounds[iDimension] << ", " <<
		 piUBounds[iDimension] << ")";
   }

   if( dc.GetDepth() > 0 )
   {
	  // Dump the contents of the array.
	  for( iDimension = 0; iDimension < nDimensions; iDimension++ )
	  {
		 piIndices[iDimension] = piLBounds[iDimension];
	  }

	  while( piIndices[0] <= piUBounds[0] )
	  {
		 dc << "\n\t";
		 for( iDimension = 0; iDimension < nDimensions; iDimension++ )
		 {
			dc << "[" << piIndices[iDimension] << "]";
		 }
		 dc << " = ";

		 // Dump the value of the element
		 _AfxDumpSafeArrayElement(dc, saSrc, piIndices);

		 // Increment the rightmost index, with wraparound and carry logic
		 iDimension = nDimensions-1;
		 bWrapped = TRUE;
		 do
		 {
			bWrapped = FALSE;
			piIndices[iDimension]++;
			if( piIndices[iDimension] > piUBounds[iDimension] )
			{
			   if( iDimension > 0 )
			   {
				  // We've overstepped the bounds of this dimension, so wrap
				  // around to the lower bound and make sure to increment the
				  // next dimension to the left.
				  bWrapped = TRUE;
				  piIndices[iDimension] = piLBounds[iDimension];
				  iDimension--;
			   }
			}
		 } while( bWrapped && (iDimension >= 0) );
	  }
   }

   return dc;
}
#endif // _DEBUG

void COleSafeArray::CreateOneDim(VARTYPE vtSrc, DWORD dwElements,
	const void* pvSrcData, long nLBound)
{
	ENSURE(dwElements > 0);
	
	if(!(dwElements > 0))
		AfxThrowInvalidArgException();
			
	// Setup the bounds and create the array
	SAFEARRAYBOUND rgsabound;
	rgsabound.cElements = dwElements;
	rgsabound.lLbound = nLBound;
	Create(vtSrc, 1, &rgsabound);

	// Copy over the data if neccessary
	if (pvSrcData != NULL)
	{
		void* pvDestData;
		AccessData(&pvDestData);
           
		 
		unsigned __int64 tmp_64=static_cast<unsigned __int64>(GetElemSize()) * static_cast<unsigned __int64>(dwElements);
	     
		ENSURE(tmp_64<=INT_MAX); //no overflow ENSURE  
	    

		Checked::memcpy_s(pvDestData, static_cast<size_t>(tmp_64), 
			pvSrcData, static_cast<size_t>(tmp_64));
		UnaccessData();
	}
}

DWORD COleSafeArray::GetOneDimSize()
{
	ENSURE(GetDim() == 1);

	long nUBound, nLBound;

	GetUBound(1, &nUBound);
	GetLBound(1, &nLBound);

	return nUBound + 1 - nLBound;
}

void COleSafeArray::ResizeOneDim(DWORD dwElements)
{
	ASSERT(GetDim() == 1);

	if(!(GetDim() == 1))
		AfxThrowInvalidArgException();

	SAFEARRAYBOUND rgsabound;

	rgsabound.cElements = dwElements;
	rgsabound.lLbound = 0;

	Redim(&rgsabound);
}

void COleSafeArray::Create(VARTYPE vtSrc, DWORD dwDims, DWORD* rgElements)
{
	ASSERT(rgElements != NULL);

	if(rgElements == NULL)
		AfxThrowInvalidArgException();
		
	// Allocate and fill proxy array of bounds (with lower bound of zero)
	SAFEARRAYBOUND* rgsaBounds = new SAFEARRAYBOUND[dwDims];

	for (DWORD dwIndex = 0; dwIndex < dwDims; dwIndex++)
	{
		// Assume lower bound is 0 and fill in element count
		rgsaBounds[dwIndex].lLbound = 0;	
		rgsaBounds[dwIndex].cElements = rgElements[dwIndex];
	}

	TRY
	{
		Create(vtSrc, dwDims, rgsaBounds);
	}
	CATCH_ALL(e)
	{
		// Must free up memory
		delete[] rgsaBounds;
		THROW_LAST();
	}
	END_CATCH_ALL

	delete[] rgsaBounds;
}

void COleSafeArray::Create(VARTYPE vtSrc, DWORD dwDims, SAFEARRAYBOUND* rgsabound)
{
	ASSERT(dwDims > 0);
	ASSERT(rgsabound != NULL);

	// Validate the VARTYPE for SafeArrayCreate call
	ASSERT(!(vtSrc & VT_ARRAY));
	ASSERT(!(vtSrc & VT_BYREF));
	ASSERT(!(vtSrc & VT_VECTOR));
	ASSERT(vtSrc != VT_EMPTY);
	ASSERT(vtSrc != VT_NULL);
	
	if(dwDims == 0 			|| 
		rgsabound == NULL 	|| 
		(vtSrc & VT_ARRAY) 	|| 
		(vtSrc & VT_BYREF) 	|| 
		(vtSrc & VT_VECTOR) 	|| 
		vtSrc == VT_EMPTY 	|| 
		vtSrc == VT_NULL)
	{
		AfxThrowInvalidArgException();
	}
		
	// Free up old safe array if necessary
	Clear();

	parray = ::SafeArrayCreate(vtSrc, dwDims, rgsabound);

	if (parray == NULL)
		AfxThrowMemoryException();

	vt = unsigned short(vtSrc | VT_ARRAY);
	m_dwDims = dwDims;
	m_dwElementSize = GetElemSize();
}

void COleSafeArray::AccessData(void** ppvData)
{
	AfxCheckError(::SafeArrayAccessData(parray, ppvData));
}

void COleSafeArray::UnaccessData()
{
	AfxCheckError(::SafeArrayUnaccessData(parray));
}

void COleSafeArray::AllocData()
{
	AfxCheckError(::SafeArrayAllocData(parray));
}

void COleSafeArray::AllocDescriptor(DWORD dwDims)
{
	AfxCheckError(::SafeArrayAllocDescriptor(dwDims, &parray));
}

void COleSafeArray::Copy(LPSAFEARRAY* ppsa)
{
	AfxCheckError(::SafeArrayCopy(parray, ppsa));
}

void COleSafeArray::GetLBound(DWORD dwDim, long* pLbound)
{
	AfxCheckError(::SafeArrayGetLBound(parray, dwDim, pLbound));
}

void COleSafeArray::GetUBound(DWORD dwDim, long* pUbound)
{
	AfxCheckError(::SafeArrayGetUBound(parray, dwDim, pUbound));
}

void COleSafeArray::GetElement(long* rgIndices, void* pvData)
{
	AfxCheckError(::SafeArrayGetElement(parray, rgIndices, pvData));
}

void COleSafeArray::PtrOfIndex(long* rgIndices, void** ppvData)
{
	AfxCheckError(::SafeArrayPtrOfIndex(parray, rgIndices, ppvData));
}

void COleSafeArray::PutElement(long* rgIndices, void* pvData)
{
	AfxCheckError(::SafeArrayPutElement(parray, rgIndices, pvData));
}

void COleSafeArray::Redim(SAFEARRAYBOUND* psaboundNew)
{
	AfxCheckError(::SafeArrayRedim(parray, psaboundNew));
}

void COleSafeArray::Lock()
{
	AfxCheckError(::SafeArrayLock(parray));
}

void COleSafeArray::Unlock()
{
	AfxCheckError(::SafeArrayUnlock(parray));
}

void COleSafeArray::Destroy()
{
	AfxCheckError(::SafeArrayDestroy(parray));
	// The underlying SafeArray object was destroyed, so we need to detach the object to void operating on it anymore.
	// We don't care the destroyed object, so we can just simply call Detach after ::SafeArrayDestroy to set vt to VT_EMPTY.
	Detach();
}

void COleSafeArray::DestroyData()
{
	AfxCheckError(::SafeArrayDestroyData(parray));
}

void COleSafeArray::DestroyDescriptor()
{
	AfxCheckError(::SafeArrayDestroyDescriptor(parray));
}

///////////////////////////////////////////////////////////////////////////////
// COleSafeArray Helpers
void AFXAPI AfxSafeArrayInit(COleSafeArray* psa)
{
	if(psa != NULL)
		memset(psa, 0, sizeof(*psa));
}

/////////////////////////////////////////////////////////////////////////////
// Simple field formatting to text item - see dlgdata.cpp for base types

void AFXAPI DDX_Text(CDataExchange* pDX, int nIDC, COleDateTime& value)
{
	ENSURE_ARG(pDX != NULL);
	pDX->PrepareEditCtrl(nIDC);
   HWND hWndCtrl;
   pDX->m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);
	if (pDX->m_bSaveAndValidate)
	{
		int nLen = ::GetWindowTextLength(hWndCtrl);
		CString strTemp;

		::GetWindowText(hWndCtrl, strTemp.GetBufferSetLength(nLen), nLen+1);
		strTemp.ReleaseBuffer();

		if (!value.ParseDateTime(strTemp))  // throws exception
		{
			// Can't convert string to datetime
			AfxMessageBox(AFX_IDP_PARSE_DATETIME);
			pDX->Fail();    // throws exception
		}
	}
	else
	{
		CString strTemp = value.Format();
		AfxSetWindowText(hWndCtrl, strTemp);
	}
}

void AFXAPI DDX_Text(CDataExchange* pDX, int nIDC, FILETIME& value)
{
	ENSURE_ARG(pDX != NULL);
	COleDateTime dt;
	SYSTEMTIME st;

	pDX->PrepareEditCtrl(nIDC);
   HWND hWndCtrl;
   pDX->m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);
	if (pDX->m_bSaveAndValidate)
	{
		int nLen = ::GetWindowTextLength(hWndCtrl);
		CString strTemp;

		::GetWindowText(hWndCtrl, strTemp.GetBufferSetLength(nLen), nLen+1);
		strTemp.ReleaseBuffer();

		if (!dt.ParseDateTime(strTemp))  // throws exception
		{
			// Can't convert string to datetime
			AfxMessageBox(AFX_IDP_PARSE_DATETIME);
			pDX->Fail();    // throws exception
		}
		dt.GetAsSystemTime(st);
		SystemTimeToFileTime(&st, &value);
	}
	else
	{
		FileTimeToSystemTime(&value, &st);
		dt.SetDateTime(st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		CString strTemp = dt.Format();
		AfxSetWindowText(hWndCtrl, strTemp);
	}
}

void AFXAPI DDX_Text(CDataExchange* pDX, int nIDC, COleCurrency& value)
{
	pDX->PrepareEditCtrl(nIDC);
   HWND hWndCtrl;
   pDX->m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);
	if (pDX->m_bSaveAndValidate)
	{
		int nLen = ::GetWindowTextLength(hWndCtrl);
		CString strTemp;

		::GetWindowText(hWndCtrl, strTemp.GetBufferSetLength(nLen), nLen+1);
		strTemp.ReleaseBuffer();

		if (!value.ParseCurrency(strTemp))  // throws exception
		{
			// Can't convert string to currency
			AfxMessageBox(AFX_IDP_PARSE_CURRENCY);
			pDX->Fail();    // throws exception
		}
	}
	else
	{
		CString strTemp = value.Format();
		AfxSetWindowText(hWndCtrl, strTemp);
	}
}

void AFXAPI DDX_Text(CDataExchange* pDX, int nIDC, DECIMAL& value)
{
	pDX->PrepareEditCtrl(nIDC);
   HWND hWndCtrl;
   pDX->m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);
	if (pDX->m_bSaveAndValidate)
	{
		int nLen = ::GetWindowTextLength(hWndCtrl);
		CString strTemp;

		::GetWindowText(hWndCtrl, strTemp.GetBufferSetLength(nLen), nLen+1);
		strTemp.ReleaseBuffer();

		BSTR bstr;
		HRESULT hResult;

		bstr = strTemp.AllocSysString();
		hResult = VarDecFromStr(bstr, ::GetThreadLocale(), 0, &value);
		::SysFreeString(bstr);
		if (FAILED(hResult))
		{
			AfxMessageBox(AFX_IDP_PARSE_REAL);
			pDX->Fail();
		}
	}
	else
	{
		BSTR bstr;
		HRESULT hResult;

		hResult = VarBstrFromDec(&value, ::GetThreadLocale(), 0, &bstr);
		if (FAILED(hResult))
		{
			TRACE(traceOle, 0, "Warning: invalid DECIMAL number for control ID %d.\n", pDX->m_idLastControl);
			return;
		}

		CString strText(bstr);
		::SysFreeString(bstr);
		AfxSetWindowText(hWndCtrl, strText);
	}
}

void AFXAPI DDX_Text(CDataExchange* pDX, int nIDC, GUID& value)
{
	pDX->PrepareEditCtrl(nIDC);
   HWND hWndCtrl;
   pDX->m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);
	if (pDX->m_bSaveAndValidate)
	{
		int nLen = ::GetWindowTextLength(hWndCtrl);
		CString strTemp;

		::GetWindowText(hWndCtrl, strTemp.GetBufferSetLength(nLen), nLen+1);
		strTemp.ReleaseBuffer();

		HRESULT hResult;

		hResult = CLSIDFromString(const_cast<LPOLESTR>(CStringW(strTemp).GetString()), &value);
		if (FAILED(hResult))
		{
			AfxMessageBox(AFX_IDP_PARSE_GUID);
			pDX->Fail();
		}
	}
	else
	{
		OLECHAR szGUID[64];

		StringFromGUID2(value, szGUID, 64);
		AfxSetWindowText(hWndCtrl, CFixedStringT<CString, 64>(szGUID));
	}
}

/////////////////////////////////////////////////////////////////////////////
