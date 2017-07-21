// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

#ifndef __ATLCUR_H__
#define __ATLCUR_H__

#pragma once

#include <atlbase.h>


#pragma pack(push,_ATL_PACKING)
namespace ATL
{

const LONGLONG CY_MIN_INTEGER	= -922337203685477;
const LONGLONG CY_MAX_INTEGER	= 922337203685477;
const SHORT CY_MIN_FRACTION		= -9999;
const SHORT CY_MAX_FRACTION		= 9999;
const SHORT CY_SCALE			= 10000;

class CComCurrency
{
public:

// constructors
	CComCurrency() throw()
	{
		 m_currency.int64 = 0;
	}
	CComCurrency(CURRENCY cySrc) throw()
	{
		m_currency.int64 = cySrc.int64;
	}
	CComCurrency(const CComCurrency& curSrc) throw()
	{
		*this = curSrc;
	}
	CComCurrency(LONGLONG nInteger, SHORT nFraction)
	{
		m_currency.int64 = 0;
		HRESULT hRes = SetInteger(nInteger);
		if (FAILED(hRes))
			AtlThrow(hRes);
		hRes = SetFraction(nFraction);
		if (FAILED(hRes))
			AtlThrow(hRes);
	}
	CComCurrency(BYTE bSrc)
	{
		*this = bSrc;
	}
	CComCurrency(SHORT sSrc)
	{
		*this = sSrc;
	}
	CComCurrency(LONG lSrc)
	{
		*this = lSrc;
	}
	CComCurrency(FLOAT fSrc)
	{
		*this = fSrc;
	}
	CComCurrency(DOUBLE dSrc)
	{
		*this = dSrc;
	}
	CComCurrency(CHAR cSrc)
	{
		*this = cSrc;
	}
	CComCurrency(USHORT usSrc)
	{
		*this = usSrc;
	}
	CComCurrency(ULONG ulSrc)
	{
		*this = ulSrc;
	}
	CComCurrency(DECIMAL dSrc)
	{
		*this = dSrc;
	}
	explicit CComCurrency(LPCSTR szSrc)
	{
		ATLASSERT(szSrc);
		if( szSrc == NULL )
			AtlThrow(E_INVALIDARG);

		USES_CONVERSION_EX;
		LPOLESTR p = A2OLE_EX(szSrc, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
		if( p == NULL )
			AtlThrow(E_OUTOFMEMORY);

		HRESULT hRes = VarCyFromStr(p, GetThreadLocale(), LOCALE_NOUSEROVERRIDE, &m_currency);
		if (FAILED(hRes))
			AtlThrow(hRes);
	}
	explicit CComCurrency(LPCWSTR szSrc)
	{
		ATLENSURE(szSrc);
		HRESULT hRes = VarCyFromStr(const_cast<LPWSTR>(szSrc), GetThreadLocale(), LOCALE_NOUSEROVERRIDE, &m_currency);
		if (FAILED(hRes))
			AtlThrow(hRes);
	}
	explicit CComCurrency(const VARIANT& varSrc)
	{
		VARIANT var;
		VariantInit(&var);
		HRESULT hRes = VariantChangeType(&var, const_cast<VARIANT*>(&varSrc), 0, VT_CY);
		if (FAILED(hRes))
			AtlThrow(hRes);
		m_currency.int64 = V_CY(&var).int64;
	}
	explicit CComCurrency(LPDISPATCH pDispSrc)
	{
		ATLENSURE(pDispSrc);
		HRESULT hRes = VarCyFromDisp(pDispSrc, GetThreadLocale(), &m_currency);
		if (FAILED(hRes))
			AtlThrow(hRes);
	}

// assignment operators
	const CComCurrency& operator=(CURRENCY cySrc) throw()
	{
		m_currency.int64 = cySrc.int64;
		return *this;
	}
	const CComCurrency& operator=(const CComCurrency& curSrc) throw()
	{
		m_currency.int64 = curSrc.m_currency.int64;
		return *this;
	}
	const CComCurrency& operator=(BYTE bSrc)
	{
		HRESULT hRes = VarCyFromUI1(bSrc, &m_currency);
		if (FAILED(hRes))
			AtlThrow(hRes);
		return *this;
	}
	const CComCurrency& operator=(SHORT sSrc)
	{
		HRESULT hRes = VarCyFromI2(sSrc, &m_currency);
		if (FAILED(hRes))
			AtlThrow(hRes);
		return *this;
	}
	const CComCurrency& operator=(LONG lSrc)
	{
		HRESULT hRes = VarCyFromI4(lSrc, &m_currency);
		if (FAILED(hRes))
			AtlThrow(hRes);
		return *this;
	}
	const CComCurrency& operator=(FLOAT fSrc)
	{
		HRESULT hRes = VarCyFromR4(fSrc, &m_currency);
		if (FAILED(hRes))
			AtlThrow(hRes);
		return *this;
	}
	const CComCurrency& operator=(DOUBLE dSrc)
	{
		HRESULT hRes = VarCyFromR8(dSrc, &m_currency);
		if (FAILED(hRes))
			AtlThrow(hRes);
		return *this;
	}
	const CComCurrency& operator=(CHAR cSrc)
	{
		HRESULT hRes = VarCyFromI1(cSrc, &m_currency);
		if (FAILED(hRes))
			AtlThrow(hRes);
		return *this;
	}
	const CComCurrency& operator=(USHORT usSrc)
	{
		HRESULT hRes = VarCyFromUI2(usSrc, &m_currency);
		if (FAILED(hRes))
			AtlThrow(hRes);
		return *this;
	}
	const CComCurrency& operator=(ULONG ulSrc)
	{
		HRESULT hRes = VarCyFromUI4(ulSrc, &m_currency);
		if (FAILED(hRes))
			AtlThrow(hRes);
		return *this;
	}
	const CComCurrency& operator=(DECIMAL dSrc)
	{
		HRESULT hRes = VarCyFromDec(&dSrc, &m_currency);
		if (FAILED(hRes))
			AtlThrow(hRes);
		return *this;
	}

// comparison operators
	bool operator==(const CComCurrency& cur) const
	{
		return (static_cast<HRESULT>(VARCMP_EQ) == VarCyCmp(m_currency, cur.m_currency));
	}
	bool operator!=(const CComCurrency& cur) const
	{
		return (static_cast<HRESULT>(VARCMP_EQ) != VarCyCmp(m_currency, cur.m_currency));
	}
	bool operator<(const CComCurrency& cur) const
	{
		return (static_cast<HRESULT>(VARCMP_LT) == VarCyCmp(m_currency, cur.m_currency));
	}
	bool operator>(const CComCurrency& cur) const
	{
		return (static_cast<HRESULT>(VARCMP_GT) == VarCyCmp(m_currency, cur.m_currency));
	}
	bool operator<=(const CComCurrency& cur) const
	{
		HRESULT hRes = VarCyCmp(m_currency, cur.m_currency);
		return (static_cast<HRESULT>(VARCMP_LT) == hRes || (HRESULT)VARCMP_EQ == hRes);
	}
	bool operator>=(const CComCurrency& cur) const
	{
		HRESULT hRes = VarCyCmp(m_currency, cur.m_currency);
		return (static_cast<HRESULT>(VARCMP_GT) == hRes || static_cast<HRESULT>(VARCMP_EQ) == hRes);
	}

// math operators
	CComCurrency operator+(const CComCurrency& cur) const
	{
		CURRENCY cy;
		HRESULT hRes = VarCyAdd(m_currency, cur.m_currency, &cy);
		if (FAILED(hRes))
			AtlThrow(hRes);
		return cy;
	}
	CComCurrency operator-(const CComCurrency& cur) const
	{
		CURRENCY cy;
		HRESULT hRes = VarCySub(m_currency, cur.m_currency, &cy);
		if (FAILED(hRes))
			AtlThrow(hRes);
		return cy;
	}
	const CComCurrency& operator+=(const CComCurrency& cur)
	{
		HRESULT hRes = VarCyAdd(m_currency, cur.m_currency, &m_currency);
		if (FAILED(hRes))
			AtlThrow(hRes);
		return *this;
	}
	const CComCurrency& operator-=(const CComCurrency& cur)
	{
		HRESULT hRes = VarCySub(m_currency, cur.m_currency, &m_currency);
		if (FAILED(hRes))
			AtlThrow(hRes);
		return *this;
	}
	CComCurrency operator*(const CComCurrency& cur) const
	{
		CURRENCY cy;
		HRESULT hRes = VarCyMul(m_currency, cur.m_currency, &cy);
		if (FAILED(hRes))
			AtlThrow(hRes);
		return cy;
	}
	const CComCurrency& operator*=(const CComCurrency& cur)
	{
		HRESULT hRes = VarCyMul(m_currency, cur.m_currency, &m_currency);
		if (FAILED(hRes))
			AtlThrow(hRes);
		return *this;
	}
	CComCurrency operator*(long nOperand) const
	{
		CURRENCY cy;
		HRESULT hRes = VarCyMulI4(m_currency, nOperand, &cy);
		if (FAILED(hRes))
			AtlThrow(hRes);
		return cy;
	}
	const CComCurrency& operator*=(long nOperand)
	{
		HRESULT hRes = VarCyMulI4(m_currency, nOperand, &m_currency);
		if (FAILED(hRes))
			AtlThrow(hRes);
		return *this;
	}
	CComCurrency operator-() const
	{
		CURRENCY cy;
		HRESULT hRes = VarCyNeg(m_currency, &cy);
		if (FAILED(hRes))
			AtlThrow(hRes);
		return cy;
	}
	CComCurrency operator/(long nOperand) const
	{
		ATLASSERT(nOperand);
		if( nOperand == 0 )
			AtlThrow(E_INVALIDARG);

		CURRENCY cy;
		cy.int64 = m_currency.int64 / nOperand;
		return cy;
	}
	const CComCurrency& operator/=(long nOperand)
	{
		ATLASSERT(nOperand);
		if( nOperand == 0 )
			AtlThrow(E_INVALIDARG);

		m_currency.int64 /= nOperand;
		return *this;
	}

// cast operators
	operator CURRENCY&() throw()
	{
		return m_currency;
	}
	operator const CURRENCY&() const throw()
	{
		return m_currency;
	}
	CURRENCY* GetCurrencyPtr() throw()
	{
		return &m_currency;
	}

// misc functions
	HRESULT Round(int nDecimals)
	{
		ATLASSERT(nDecimals >= 0 && nDecimals <= 4);
		if( nDecimals < 0 || nDecimals > 4 )
			return E_INVALIDARG;

		return VarCyRound(m_currency, nDecimals, &m_currency);
	}

	HRESULT SetInteger(LONGLONG nInteger)
	{
		// check if within range
		ATLASSERT(nInteger >= CY_MIN_INTEGER && nInteger <= CY_MAX_INTEGER);
		if( nInteger < CY_MIN_INTEGER || nInteger > CY_MAX_INTEGER )
			return E_INVALIDARG;

		if (m_currency.int64)
		{
			// signs must match
			if ((m_currency.int64 < 0 && nInteger > 0) ||
				(m_currency.int64 > 0 && nInteger < 0))
				return TYPE_E_TYPEMISMATCH;

			CURRENCY cyTemp;
			// get fractional part
			cyTemp.int64 = m_currency.int64 % CY_SCALE;
			// check if within range again
			if ((nInteger == CY_MAX_INTEGER && cyTemp.int64 > 5807) ||
				(nInteger == CY_MIN_INTEGER && cyTemp.int64 < -5808))
				return TYPE_E_OUTOFBOUNDS;
			// set to fractional part, wiping out integer part
			m_currency.int64 = cyTemp.int64;
		}
		// add new integer part scaled by CY_SCALE
		m_currency.int64 += nInteger * CY_SCALE;
		return S_OK;
	}

	// Based on 4 digits.  To set .2, pass 2000, to set .0002, pass a 2
	HRESULT SetFraction(SHORT nFraction)
	{
		// check if within range
		ATLASSERT(nFraction >= CY_MIN_FRACTION && nFraction <= CY_MAX_FRACTION);
		if( nFraction < CY_MIN_FRACTION || nFraction > CY_MAX_FRACTION )
			return E_INVALIDARG;

		if (m_currency.int64)
		{
			// signs must match
			if ((m_currency.int64 < 0 && nFraction > 0) ||
				(m_currency.int64 > 0 && nFraction < 0))
				return TYPE_E_TYPEMISMATCH;

			CURRENCY cyTemp;
			// get integer part, wiping out fractional part
			cyTemp.int64 = m_currency.int64 / CY_SCALE;
			// check if within range again
			if ((cyTemp.int64 == CY_MAX_INTEGER && nFraction > 5807) ||
				(cyTemp.int64 == CY_MIN_INTEGER && nFraction < -5808))
				return TYPE_E_OUTOFBOUNDS;
			// scale to CY_SCALE
			m_currency.int64 = cyTemp.int64 * CY_SCALE;
		}
		m_currency.int64 += nFraction;
		return S_OK;
	}

	LONGLONG GetInteger() const
	{
		if (m_currency.int64)
			return (m_currency.int64 / CY_SCALE);
		else
			return 0;		
	}

	SHORT GetFraction() const
	{
		if (m_currency.int64)
			// get fractional part
			return static_cast<SHORT>(m_currency.int64 % CY_SCALE);
		else
			return 0;
	}

	CURRENCY m_currency;
};

}; //namespace ATL
#pragma pack(pop)
#endif //__ATLSAFE_H__
