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
#include "sal.h"



#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// CDBByteArray db specific class for holding byte array data
class CDBByteArray : public CByteArray
{
	DECLARE_DYNAMIC(CDBByteArray)

// Operations
	void SetLength(int nNewSize);
};

inline void CDBByteArray::SetLength(int nNewSize)
{
	// Can't grow buffer since ODBC has been SQLBindCol'd on it.
	ASSERT(nNewSize <= m_nMaxSize);
	if(nNewSize > m_nMaxSize)
		AfxThrowInvalidArgException();
	m_nSize = nNewSize;
}

//////////////////////////////////////////////////////////////////////////////
// CFieldExchange

CFieldExchange::CFieldExchange(UINT nOperation, CRecordset* prs, void* pvField)
{
#ifdef _DEBUG
#pragma warning( push )
#pragma warning( disable: 4296 )
	ASSERT(nOperation >= BindParam && nOperation <= DumpField);
#pragma warning( pop )
#endif
	ENSURE_VALID(prs);
	ENSURE(prs->m_hstmt != SQL_NULL_HSTMT);

	m_nFieldType = (UINT) noFieldType;
	m_nOperation = nOperation;
	m_prs = prs;
	m_pvField = pvField;

	m_nFields = 0;
	m_nParams = 0;
	m_nParamFields = 0;
	m_bField = FALSE;
	m_pstr = NULL;
	m_hstmt = SQL_NULL_HSTMT;

	m_lDefaultLBFetchSize = 0x00010000;
	m_lDefaultLBReallocSize = 0x00010000;
}

BOOL CFieldExchange::IsFieldType(UINT* pnField)
{
	ENSURE_ARG(pnField != NULL);
	if (m_nFieldType == outputColumn)
	{
		*pnField = ++m_nFields;
		// Recordset's m_nFields must match number of Fields!
		ASSERT(m_nFields <= m_prs->m_nFields);
	}
	else
	{
		// Make sure SetFieldType was called
		ASSERT(m_nFieldType == inputParam ||
			m_nFieldType == outputParam ||
			m_nFieldType == inoutParam);

		*pnField = ++m_nParams;
		// Recordset's m_nParams must match number of Params!
		ASSERT(m_nParams <= m_prs->m_nParams);
	}

	if (m_nOperation == BindParam || m_nOperation == RebindParam)
	{
		// only valid on a param field type
		return m_nFieldType != outputColumn;
	}
	else
	if (m_nOperation == Fixup)
	{
		return m_nFieldType == outputColumn ||
			   m_nFieldType == outputParam  ||
			   m_nFieldType == inoutParam;
	}
	else
	{
		// valid only on an outputColumn field type
		return m_nFieldType == outputColumn;
	}
}

// Default implementation for RFX functions
void CFieldExchange::Default(LPCTSTR szName,
	void* pv, LONG_PTR* plLength, int nCType, SQLULEN cbValue, SQLULEN cbPrecision)
{
	RETCODE nRetCode;
	UINT nField = (m_nFieldType == outputColumn)? m_nFields: m_nParams;
	switch (m_nOperation)
	{
	case BindParam:
		ENSURE_ARG(plLength != NULL);
		if (m_prs->IsParamStatusNull(nField - 1))
			*plLength = SQL_NULL_DATA;
		else
			*plLength = cbValue;
		// For params, CType is same as SQL type
		AFX_SQL_SYNC(::SQLBindParameter(m_hstmt, (UWORD)nField,
			(SWORD)m_nFieldType, (SWORD)nCType, (SWORD)nCType, cbPrecision, 0,
			pv, 0, plLength));
		if (nRetCode != SQL_SUCCESS)
			m_prs->ThrowDBException(nRetCode, m_hstmt);

		// Add the member address to the param map
		m_prs->m_mapParamIndex.SetAt(pv, (void*)(UINT_PTR)nField);
		return;

	case RebindParam:
		ENSURE_ARG(plLength != NULL);
		// Only need to reset param length
		*plLength = m_prs->IsParamStatusNull(nField - 1) ? SQL_NULL_DATA : cbValue;
		return;

	case BindFieldForUpdate:
		ENSURE_ARG(plLength != NULL);
		if (!m_prs->IsFieldStatusDirty(nField - 1))
		{
			// If not dirty, set length to SQL_IGNORE for SQLSetPos updates
			*plLength = SQL_IGNORE;
		}
		else if (!m_prs->IsFieldStatusNull(nField - 1))
		{
			// Reset the length as it may have changed for var length fields
			*plLength = cbValue;
		}
		else if (m_prs->IsFieldStatusNull(nField - 1))
			*plLength = SQL_NULL_DATA;

		return;

	case UnbindFieldForUpdate:
		// Reset bound length to actual length to clear SQL_IGNOREs
		if (!m_prs->IsFieldStatusDirty(nField - 1))
		{
			ENSURE_ARG(plLength != NULL);
			*plLength = cbValue;
		}
		return;

	case BindFieldToColumn:
		AFX_SQL_SYNC(::SQLBindCol(m_prs->m_hstmt, (UWORD)nField, (SWORD)nCType,
			pv, cbValue, plLength));
		if (!m_prs->Check(nRetCode))
			m_prs->ThrowDBException(nRetCode);

		// Add the member address to the field map
		m_prs->m_mapFieldIndex.SetAt(pv, (void*)(UINT_PTR)nField);
		return;

	case Name:
		if (m_prs->IsFieldStatusDirty(nField - 1))
		{
			// We require a name
			ASSERT(lstrlen(szName) != 0);

			*m_pstr += szName;
			*m_pstr += m_lpszSeparator;
		}
		return;

	case NameValue:
		if (m_prs->IsFieldStatusDirty(nField - 1))
		{
			*m_pstr += szName;
			*m_pstr += '=';
		}

		// Fall through
	case Value:
		if (m_prs->IsFieldStatusDirty(nField - 1))
		{
			ENSURE_ARG(plLength != NULL);
			// If user marked column NULL, reflect this in length
			if (m_prs->IsFieldStatusNull(nField - 1))
				*plLength = SQL_NULL_DATA;
			else
				*plLength = cbValue;

			// If optimizing for bulk add, only need lengths set correctly
			if(!(m_prs->m_dwOptions & CRecordset::optimizeBulkAdd))
			{
				*m_pstr += '?';
				*m_pstr += m_lpszSeparator;
				m_nParamFields++;

				// Assumes all bound fields BEFORE unbound fields
				CODBCFieldInfo* pODBCInfo =
					&m_prs->m_rgODBCFieldInfos[nField - 1];

				AFX_SQL_SYNC(::SQLBindParameter(m_hstmt,
					(UWORD)m_nParamFields, SQL_PARAM_INPUT,
					(SWORD)nCType, pODBCInfo->m_nSQLType,
					pODBCInfo->m_nPrecision, pODBCInfo->m_nScale,
					pv, 0, plLength));
				if (nRetCode != SQL_SUCCESS)
					m_prs->ThrowDBException(nRetCode, m_hstmt);
			}
		}
		return;

	case MarkForUpdate:
		{
			// Get the field data
			CFieldInfo* pInfo = &m_prs->m_rgFieldInfos[nField - 1];

			// If user changed field value from previous value, mark field dirty
			if ((pInfo->m_bStatus & AFX_SQL_FIELD_FLAG_NULL))
			{
				if (!m_prs->IsFieldStatusNull(nField - 1))
					m_prs->SetDirtyFieldStatus(nField - 1);
			}
			else
			{
				// Saved field is not NULL. current field null, so field dirty
				BOOL bDirty = m_prs->IsFieldStatusNull(nField - 1);

				// If values differ, then field dirty
				void* pvDataCache;

				if (pInfo->m_nDataType == AFX_RFX_BOOL ||
					pInfo->m_nDataType == AFX_RFX_BYTE ||
					pInfo->m_nDataType == AFX_RFX_INT ||
					pInfo->m_nDataType == AFX_RFX_LONG ||
					pInfo->m_nDataType == AFX_RFX_SINGLE)
				{
					// If caching data by value, pass a ref
					pvDataCache = &pInfo->m_pvDataCache;
				}
				else
					pvDataCache = pInfo->m_pvDataCache;

				if (bDirty || !AfxCompareValueByRef(pv, pvDataCache, pInfo->m_nDataType))
					m_prs->SetDirtyFieldStatus(nField - 1);
			}

#ifdef _DEBUG
			// Field address must not change - ODBC's SQLBindCol depends upon this
			void* pvBind;
			
			switch (pInfo->m_nDataType)
			{
			default:
				pvBind = pv;
				break;

			case AFX_RFX_LPWSTR:
			case AFX_RFX_LPASTR:
				pvBind = pv;
				break;

			case AFX_RFX_WTEXT:
				ENSURE_ARG(pv != NULL);
				pvBind = static_cast<CStringW *>(pv)->GetBuffer(0);
				static_cast<CStringW *>(pv)->ReleaseBuffer();
				break;

			case AFX_RFX_ATEXT:
				ENSURE_ARG(pv != NULL);
				pvBind = static_cast<CStringA *>(pv)->GetBuffer(0);
				static_cast<CStringA *>(pv)->ReleaseBuffer();
				break;

			case AFX_RFX_OLEDATE:
			case AFX_RFX_DATE:
				pvBind = m_prs->m_pvFieldProxy[nField-1];
				break;

			case AFX_RFX_BINARY:
				ENSURE_ARG(pv != NULL);
				pvBind = ((CByteArray*)pv)->GetData();
				break;
			}

			if (pInfo->m_pvBindAddress != pvBind)
			{
				TRACE(traceDatabase, 0, "Error: field address (column %u) has changed!\n",
					nField);
				ASSERT(FALSE);
			}
#endif // _DEBUG

			if ((m_pvField == NULL  || m_pvField == pv) &&
				m_prs->IsFieldStatusDirty(nField - 1))
			{
				m_bField = TRUE;
			}
		}
		return;

	case StoreField:
		AfxStoreField(*m_prs, nField, pv);
		return;

	case LoadField:
		AfxLoadField(*m_prs, nField, pv, plLength);
		return;

	default:
		ASSERT(FALSE);
	}
}

namespace nsRFX_Text
{
inline int RFX_Text_strlen(_In_ LPSTR  psz){return ::lstrlenA(psz);}
inline int RFX_Text_strlen(_In_ LPWSTR psz){return ::lstrlenW(psz);}

inline void RFX_Text_inc(LPSTR *ppsz)
	#ifdef _MBCS
		{*ppsz = reinterpret_cast<char *>(_mbsinc(reinterpret_cast<unsigned char *>(*ppsz)));}
	#else
		{(*ppsz)++;}
	#endif
inline void RFX_Text_inc(_Inout_ LPWSTR *ppsz){(*ppsz)++;}

template<typename CharType>
void RFX_Text(CFieldExchange* pFX, LPCTSTR szName,
	CharType *value, int nMaxLength, int nColumnType, short nScale,
	short nFieldType, CharType cSpace)
{
	ENSURE_ARG(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ENSURE_ARG(AfxIsValidString(szName));
	ENSURE_ARG(AfxIsValidAddress(value, nMaxLength * sizeof(CharType)));

	RETCODE nRetCode;
	UINT nField;
	if (!pFX->IsFieldType(&nField))
		return;

	LONG_PTR* plLength = pFX->m_prs->GetFieldLengthBuffer(
		nField - 1, pFX->m_nFieldType);
	switch (pFX->m_nOperation)
	{
	default:
		pFX->Default(szName, value, plLength,
			nFieldType, RFX_Text_strlen(value) * sizeof(CharType), nMaxLength);
		return;

	case CFieldExchange::BindParam:
		{
			void* pvParam = value;

			*plLength = pFX->m_prs->IsParamStatusNull(nField - 1) ?
				SQL_NULL_DATA : SQL_NTS;

			AFX_SQL_SYNC(::SQLBindParameter(pFX->m_hstmt, (UWORD)nField,
				 (SWORD)pFX->m_nFieldType, nFieldType, (SWORD)nColumnType,
				 nMaxLength, nScale, pvParam, nMaxLength * sizeof(CharType),
				 plLength));

			if (nRetCode != SQL_SUCCESS)
				pFX->m_prs->ThrowDBException(nRetCode, pFX->m_hstmt);

			// Add the member address to the param map
			pFX->m_prs->m_mapParamIndex.SetAt(value, (void*)(UINT_PTR)nField);
		}
		return;

	case CFieldExchange::BindFieldToColumn:
		{
			// Assumes all bound fields BEFORE unbound fields
			CODBCFieldInfo* pODBCInfo =
				&pFX->m_prs->m_rgODBCFieldInfos[nField - 1];
			SQLULEN cbColumn = pODBCInfo->m_nPrecision;

			switch (pODBCInfo->m_nSQLType)
			{
			default:
#ifdef _DEBUG
				// Warn of possible field schema mismatch
				TRACE(traceDatabase, 1, "Warning: CString converted from SQL type %ld.\n",
					pODBCInfo->m_nSQLType);
#endif // _DEBUG

				// Add room for extra information like sign, decimal point, etc.
				cbColumn += 10;
				break;

			case SQL_LONGVARCHAR:
			case SQL_CHAR:
			case SQL_VARCHAR:
			case SQL_WLONGVARCHAR:
			case SQL_WCHAR:
			case SQL_WVARCHAR:
				break;

			case SQL_FLOAT:
			case SQL_REAL:
			case SQL_DOUBLE:
				// Add room for sign, decimal point and " E +XXX"
				cbColumn += 10;
				break;

			case SQL_DECIMAL:
			case SQL_NUMERIC:
				// Add room for sign and decimal point
				cbColumn += 2;
				break;

			case SQL_TIMESTAMP:
			case SQL_DATE:
			case SQL_TIME:
				// May need extra space, i.e. "{TS mm/dd/yyyy hh:mm:ss}"
				cbColumn += 10;
				break;

			case SQL_TINYINT:
			case SQL_SMALLINT:
			case SQL_INTEGER:
			case SQL_BIGINT:
				// Add room for sign
				cbColumn += 1;
				break;
			}

			// Constrain to user specified max length, subject to 256 byte min
			if (cbColumn > (SQLULEN)nMaxLength || cbColumn < 256)
				cbColumn = nMaxLength;

			// Set up binding addres

			void* pvData = value;
			value[cbColumn] = 0;

			AFX_SQL_SYNC(::SQLBindCol(pFX->m_prs->m_hstmt, (UWORD)nField,
				nFieldType, pvData, (cbColumn+1) * sizeof(CharType), plLength));
			if (!pFX->m_prs->Check(nRetCode))
				pFX->m_prs->ThrowDBException(nRetCode);

			// Add the member address to the field map
			pFX->m_prs->m_mapFieldIndex.SetAt(value, (void*)(UINT_PTR)nField);
		}
		return;

	case CFieldExchange::Fixup:
		if (*plLength == SQL_NULL_DATA)
		{
			pFX->m_prs->SetNullFieldStatus(nField - 1);
			value[0] = 0;
		}
		else
		{
			CharType *lpsz = value;
			if (pFX->m_prs->m_pDatabase->m_bStripTrailingSpaces)
			{
				// find first trailing space
				CharType *lpszFirstTrailing = NULL;
				while (*lpsz != 0)
				{
					if (*lpsz != cSpace)
						lpszFirstTrailing = NULL;
					else
					{
						if (lpszFirstTrailing == NULL)
							lpszFirstTrailing = lpsz;
					}
					RFX_Text_inc(&lpsz);
				}
				// truncate
				if (lpszFirstTrailing != NULL)
					*lpszFirstTrailing = 0;

			}
			*plLength = RFX_Text_strlen(value) * sizeof(CharType);
		}
		return;

	case CFieldExchange::SetFieldNull:
		if ((pFX->m_pvField == NULL &&
			pFX->m_nFieldType == CFieldExchange::outputColumn) ||
			pFX->m_pvField == value)
		{
			if (pFX->m_bField)
			{
				// Mark fields null
				pFX->m_prs->SetNullFieldStatus(nField - 1);
				// Set string 0 length
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				pFX->m_prs->ClearNullFieldStatus(nField - 1);
				*plLength = SQL_NTS;
			}
#ifdef _DEBUG
			pFX->m_nFieldFound = nField;
#endif
		}
		return;

	case CFieldExchange::MarkForAddNew:
		// can force writing of psuedo-null value (as a non-null) by setting field dirty
		if (*value != 0)
		{
			pFX->m_prs->SetDirtyFieldStatus(nField - 1);
			pFX->m_prs->ClearNullFieldStatus(nField - 1);
		}
		return;

	case CFieldExchange::MarkForUpdate:
		if (*value == 0)
			pFX->m_prs->SetNullFieldStatus(nField - 1);
		else
			pFX->m_prs->ClearNullFieldStatus(nField - 1);
		pFX->Default(szName, value, plLength,
			nFieldType, RFX_Text_strlen(value) * sizeof(CharType), nMaxLength);
		return;

	case CFieldExchange::LoadField:
		{
			// Get the field data
			CFieldInfo* pInfo = &pFX->m_prs->m_rgFieldInfos[nField - 1];

			// Restore the status
			pFX->m_prs->SetFieldStatus(nField - 1, pInfo->m_bStatus);

			// If not NULL, restore the value and length
			if (!pFX->m_prs->IsFieldStatusNull(nField - 1))
			{
				value = static_cast<CharType *>(pInfo->m_pvDataCache);
				*plLength = RFX_Text_strlen(value) * sizeof(CharType);
			}
			else
				*plLength = SQL_NULL_DATA;

#ifdef _DEBUG
			// Buffer address must not change - ODBC's SQLBindCol depends upon this
			void *pvBind = value;
			if (pvBind != pInfo->m_pvBindAddress)
			{
				TRACE(traceDatabase, 0, "Error: buffer (column %u) address has changed!\n",
					nField);
				ASSERT(FALSE);
			}
#endif // _DEBUG
		}
		return;

	case CFieldExchange::StoreField:
		AfxStoreField(*pFX->m_prs, nField, value);
		return;

	case CFieldExchange::AllocCache:
		{
			CFieldInfo* pInfo = &pFX->m_prs->m_rgFieldInfos[nField - 1];
			pInfo->m_pvDataCache = new CharType[nMaxLength];
			pInfo->m_nDataType = nFieldType == SQL_C_WCHAR ? AFX_RFX_LPWSTR : AFX_RFX_LPASTR;
		}
		return;

#ifdef _DEBUG
	case CFieldExchange::DumpField:
		*pFX->m_pdcDump << "\n" << szName << " = " << value;
		return;
#endif // _DEBUG
	}
}

// Note: CString.m_pchData must not be changed.  This address is registered
// with ODBC and must remain valid until the recordset is released.
template<typename StringType>
void RFX_Text(CFieldExchange* pFX, LPCTSTR szName,
	StringType &value, int nMaxLength, int nColumnType, short nScale,
	short nFieldType, typename StringType::XCHAR cSpace)
{
	ENSURE_ARG(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ENSURE_ARG(AfxIsValidString(szName));
	ENSURE_ARG(AfxIsValidAddress(&value, sizeof(StringType)));

	RETCODE nRetCode;
	UINT nField;
	if (!pFX->IsFieldType(&nField))
		return;

	LONG_PTR* plLength = pFX->m_prs->GetFieldLengthBuffer(
		nField - 1, pFX->m_nFieldType);
	switch (pFX->m_nOperation)
	{
	default:
		pFX->Default(szName, value.GetBuffer(0), plLength,
			nFieldType, value.GetLength() * sizeof(StringType::XCHAR), nMaxLength);
		value.ReleaseBuffer();
		return;

	case CFieldExchange::BindParam:
		{
			// Preallocate to nMaxLength and setup binding address
			value.GetBufferSetLength(nMaxLength);
			void *pvParam = value.LockBuffer();

			*plLength = pFX->m_prs->IsParamStatusNull(nField - 1) ?
				SQL_NULL_DATA : SQL_NTS;

			AFX_SQL_SYNC(::SQLBindParameter(pFX->m_hstmt, (UWORD)nField,
				 (SWORD)pFX->m_nFieldType, nFieldType, (SWORD)nColumnType,
				 nMaxLength, nScale, pvParam, nMaxLength * sizeof(StringType::XCHAR),
				 plLength));

			value.ReleaseBuffer();

			if (nRetCode != SQL_SUCCESS)
				pFX->m_prs->ThrowDBException(nRetCode, pFX->m_hstmt);

			// Add the member address to the param map
			pFX->m_prs->m_mapParamIndex.SetAt(&value, (void*)(UINT_PTR)nField);
		}
		return;

	case CFieldExchange::BindFieldToColumn:
		{
			// Assumes all bound fields BEFORE unbound fields
			CODBCFieldInfo* pODBCInfo =
				&pFX->m_prs->m_rgODBCFieldInfos[nField - 1];
			SQLULEN cbColumn = pODBCInfo->m_nPrecision;

			switch (pODBCInfo->m_nSQLType)
			{
			default:
#ifdef _DEBUG
				// Warn of possible field schema mismatch
				TRACE(traceDatabase, 1, "Warning: CString converted from SQL type %ld.\n",
					pODBCInfo->m_nSQLType);
#endif // _DEBUG

				// Add room for extra information like sign, decimal point, etc.
				cbColumn += 10;
				break;

			case SQL_LONGVARCHAR:
			case SQL_CHAR:
			case SQL_VARCHAR:
			case SQL_WLONGVARCHAR:
			case SQL_WCHAR:
			case SQL_WVARCHAR:
				break;

			case SQL_FLOAT:
			case SQL_REAL:
			case SQL_DOUBLE:
				// Add room for sign, decimal point and " E +XXX"
				cbColumn += 10;
				break;

			case SQL_DECIMAL:
			case SQL_NUMERIC:
				// Add room for sign and decimal point
				cbColumn += 2;
				break;

			case SQL_TIMESTAMP:
			case SQL_DATE:
			case SQL_TIME:
				// May need extra space, i.e. "{TS mm/dd/yyyy hh:mm:ss}"
				cbColumn += 10;
				break;

			case SQL_TINYINT:
			case SQL_SMALLINT:
			case SQL_INTEGER:
			case SQL_BIGINT:
				// Add room for sign
				cbColumn += 1;
				break;
			}

			// Constrain to user specified max length, subject to 256 byte min
			if (cbColumn > (SQLULEN)nMaxLength || cbColumn < 256)
				cbColumn = nMaxLength;

			// Set up binding addres
			void* pvData;
			if (cbColumn > (INT_MAX-1))
				AfxThrowMemoryException();
			value.GetBufferSetLength((int)cbColumn+1);  
			pvData = value.LockBuffer();

			AFX_SQL_SYNC(::SQLBindCol(pFX->m_prs->m_hstmt, (UWORD)nField,
				nFieldType, pvData, (cbColumn+1) * sizeof(StringType::XCHAR),
				plLength));
			value.ReleaseBuffer();
			if (!pFX->m_prs->Check(nRetCode))
				pFX->m_prs->ThrowDBException(nRetCode);

			// Add the member address to the field map
			pFX->m_prs->m_mapFieldIndex.SetAt(&value, (void*)(UINT_PTR)nField);
		}
		return;

	case CFieldExchange::Fixup:
		if (*plLength == SQL_NULL_DATA)
		{
			pFX->m_prs->SetNullFieldStatus(nField - 1);
			value.GetBufferSetLength(0);
			value.ReleaseBuffer();
		}
		else
		{
			StringType::XCHAR *lpsz = value.GetBuffer(0);
			if (pFX->m_prs->m_pDatabase->m_bStripTrailingSpaces)
			{
				// find first trailing space
				StringType::XCHAR *lpszFirstTrailing = NULL;
				while (*lpsz != 0)
				{
					if (*lpsz != cSpace)
						lpszFirstTrailing = NULL;
					else
					{
						if (lpszFirstTrailing == NULL)
							lpszFirstTrailing = lpsz;
					}
					RFX_Text_inc(&lpsz);
				}
				// truncate
				if (lpszFirstTrailing != NULL)
					*lpszFirstTrailing = 0;

			}
			value.ReleaseBuffer();
			*plLength = value.GetLength() * sizeof(StringType::XCHAR);
		}
		return;

	case CFieldExchange::SetFieldNull:
		if ((pFX->m_pvField == NULL &&
			pFX->m_nFieldType == CFieldExchange::outputColumn) ||
			pFX->m_pvField == &value)
		{
			if (pFX->m_bField)
			{
				// Mark fields null
				pFX->m_prs->SetNullFieldStatus(nField - 1);
				// Set string 0 length
				value.GetBufferSetLength(0);
				value.ReleaseBuffer();
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				pFX->m_prs->ClearNullFieldStatus(nField - 1);
				*plLength = SQL_NTS;
			}
#ifdef _DEBUG
			pFX->m_nFieldFound = nField;
#endif
		}
		return;

	case CFieldExchange::MarkForAddNew:
		// can force writing of psuedo-null value (as a non-null) by setting field dirty
		if (!value.IsEmpty())
		{
			pFX->m_prs->SetDirtyFieldStatus(nField - 1);
			pFX->m_prs->ClearNullFieldStatus(nField - 1);
		}
		return;

	case CFieldExchange::MarkForUpdate:
		if (value.IsEmpty())
			pFX->m_prs->SetNullFieldStatus(nField - 1);
		else
			pFX->m_prs->ClearNullFieldStatus(nField - 1);
		pFX->Default(szName, &value, plLength,
			nFieldType, value.GetLength() * sizeof(StringType::XCHAR), nMaxLength);
		return;

	case CFieldExchange::LoadField:
		{
			// Get the field data
			CFieldInfo* pInfo = &pFX->m_prs->m_rgFieldInfos[nField - 1];

			// Restore the status
			pFX->m_prs->SetFieldStatus(nField - 1, pInfo->m_bStatus);

			// If not NULL, restore the value and length
			if (!pFX->m_prs->IsFieldStatusNull(nField - 1))
			{
				value = *static_cast<StringType *>(pInfo->m_pvDataCache);
				*plLength = value.GetLength() * sizeof(StringType::XCHAR);
			}
			else
				*plLength = SQL_NULL_DATA;

#ifdef _DEBUG
			// Buffer address must not change - ODBC's SQLBindCol depends upon this
			void* pvBind = value.GetBuffer(0);
			value.ReleaseBuffer();
			if (pvBind != pInfo->m_pvBindAddress)
			{
				TRACE(traceDatabase, 0, "Error: CString buffer (column %u) address has changed!\n",
					nField);
				ASSERT(FALSE);
			}
#endif // _DEBUG
		}
		return;

	case CFieldExchange::StoreField:
		AfxStoreField(*pFX->m_prs, nField, &value);
		return;

	case CFieldExchange::AllocCache:
		{
			CFieldInfo* pInfo = &pFX->m_prs->m_rgFieldInfos[nField - 1];
			pInfo->m_pvDataCache = new StringType;
			pInfo->m_nDataType = nFieldType == SQL_C_WCHAR ? AFX_RFX_WTEXT : AFX_RFX_ATEXT;
		}
		return;

#ifdef _DEBUG
	case CFieldExchange::DumpField:
		*pFX->m_pdcDump << "\n" << szName << " = " << value;
		return;
#endif // _DEBUG

	}
}
} // namespace

void AFXAPI RFX_Text(_In_ CFieldExchange* pFX, _In_z_ LPCTSTR szName,
	_Out_cap_(nMaxLength) _Pre_notnull_ _Post_z_ LPWSTR value, _In_ int nMaxLength, _In_ int nColumnType, _In_ short nScale)
{
	nsRFX_Text::RFX_Text(pFX, szName, value, nMaxLength, nColumnType, nScale, SQL_C_WCHAR, L' ');
}

void AFXAPI RFX_Text(_In_ CFieldExchange* pFX, _In_ LPCTSTR szName,
	_Out_cap_(nMaxLength) _Pre_notnull_ _Post_z_ LPSTR value, _In_ int nMaxLength, _In_ int nColumnType, _In_ short nScale)
{
	nsRFX_Text::RFX_Text(pFX, szName, value, nMaxLength, nColumnType, nScale, SQL_C_CHAR, ' ');
}

void AFXAPI RFX_Text(CFieldExchange* pFX, LPCTSTR szName,
	CStringW &value, int nMaxLength, int nColumnType, short nScale)
{
	nsRFX_Text::RFX_Text(pFX, szName, value, nMaxLength, nColumnType, nScale, SQL_C_WCHAR, L' ');
}

void AFXAPI RFX_Text(CFieldExchange* pFX, LPCTSTR szName,
	CStringA &value, int nMaxLength, int nColumnType, short nScale)
{
	nsRFX_Text::RFX_Text(pFX, szName, value, nMaxLength, nColumnType, nScale, SQL_C_CHAR, ' ');
}

void AFXAPI RFX_Int(CFieldExchange* pFX, LPCTSTR szName, int& value)
{
	ENSURE_ARG(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ENSURE_ARG(AfxIsValidString(szName));

	UINT nField;
	if (!pFX->IsFieldType(&nField))
		return;

	LONG_PTR* plLength = pFX->m_prs->GetFieldLengthBuffer(
		nField - 1, pFX->m_nFieldType);
	switch (pFX->m_nOperation)
	{
	case CFieldExchange::BindFieldToColumn:
		{
#ifdef _DEBUG
			// Assumes all bound fields BEFORE unbound fields
			CODBCFieldInfo* pODBCInfo =
				&pFX->m_prs->m_rgODBCFieldInfos[nField - 1];

			if (pODBCInfo->m_nSQLType != SQL_C_SHORT)
			{
				// Warn of possible field schema mismatch
				TRACE(traceDatabase, 1, "Warning: int converted from SQL type %ld.\n",
					pODBCInfo->m_nSQLType);
			}
#endif // _DEBUG
		}
		// fall through

	default:
LDefault:
		pFX->Default(szName, &value, plLength, SQL_C_LONG,
			sizeof(value), 5);
		return;

	case CFieldExchange::Fixup:
		if (*plLength == SQL_NULL_DATA)
		{
			pFX->m_prs->SetNullFieldStatus(nField - 1);
			value = AFX_RFX_INT_PSEUDO_NULL;
		}
		return;

	case CFieldExchange::SetFieldNull:
		if ((pFX->m_pvField == NULL &&
			pFX->m_nFieldType == CFieldExchange::outputColumn) ||
			pFX->m_pvField == &value)
		{
			if (pFX->m_bField)
			{
				// Mark fields null
				pFX->m_prs->SetNullFieldStatus(nField - 1);
				value = AFX_RFX_INT_PSEUDO_NULL;
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				pFX->m_prs->ClearNullFieldStatus(nField - 1);
				*plLength = sizeof(value);
			}
#ifdef _DEBUG
			pFX->m_nFieldFound = nField;
#endif
		}
		return;

	case CFieldExchange::MarkForAddNew:
		// can force writing of psuedo-null value (as a non-null) by setting field dirty
		if (value != AFX_RFX_INT_PSEUDO_NULL)
		{
			pFX->m_prs->SetDirtyFieldStatus(nField - 1);
			pFX->m_prs->ClearNullFieldStatus(nField - 1);
		}
		return;

	case CFieldExchange::MarkForUpdate:
		if (value != AFX_RFX_INT_PSEUDO_NULL)
			pFX->m_prs->ClearNullFieldStatus(nField - 1);
		goto LDefault;

	case CFieldExchange::AllocCache:
		{
			CFieldInfo* pInfo = &pFX->m_prs->m_rgFieldInfos[nField - 1];

			// Data cached by value, no allocation necessary
			pInfo->m_nDataType = AFX_RFX_INT;
		}
		return;

#ifdef _DEBUG
	case CFieldExchange::DumpField:
		*pFX->m_pdcDump << "\n" << szName << " = " << value;
		return;
#endif // _DEBUG

	}
}

void AFXAPI RFX_BigInt(CFieldExchange* pFX, LPCTSTR szName, LONGLONG& value)
{
	ENSURE_ARG(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ENSURE_ARG(AfxIsValidString(szName));

	UINT nField;
	if (!pFX->IsFieldType(&nField))
		return;

	LONG_PTR* plLength = pFX->m_prs->GetFieldLengthBuffer(
		nField - 1, pFX->m_nFieldType);
	switch (pFX->m_nOperation)
	{
	case CFieldExchange::BindFieldToColumn:
		{
#ifdef _DEBUG
			// Assumes all bound fields BEFORE unbound fields
			CODBCFieldInfo* pODBCInfo =
				&pFX->m_prs->m_rgODBCFieldInfos[nField - 1];

			if (pODBCInfo->m_nSQLType != SQL_C_SBIGINT)
			{
				// Warn of possible field schema mismatch
				TRACE(traceDatabase, 1, "Warning: long converted from SQL type %ld.\n",
					pODBCInfo->m_nSQLType);
			}
#endif // _DEBUG
		}
		// fall through

	default:
LDefault:
		pFX->Default(szName, &value, plLength, SQL_C_SBIGINT,
			sizeof(value), 19);
		return;

	case CFieldExchange::Fixup:
		if (*plLength == SQL_NULL_DATA)
		{
			pFX->m_prs->SetNullFieldStatus(nField - 1);
			value = AFX_RFX_BIGINT_PSEUDO_NULL;
		}
		return;

	case CFieldExchange::SetFieldNull:
		if ((pFX->m_pvField == NULL &&
			pFX->m_nFieldType == CFieldExchange::outputColumn) ||
			pFX->m_pvField == &value)
		{
			if (pFX->m_bField)
			{
				// Mark fields null
				pFX->m_prs->SetNullFieldStatus(nField - 1);
				value = AFX_RFX_BIGINT_PSEUDO_NULL;
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				pFX->m_prs->ClearNullFieldStatus(nField - 1);
				*plLength = sizeof(value);
			}
#ifdef _DEBUG
			pFX->m_nFieldFound = nField;
#endif
		}
		return;

	case CFieldExchange::MarkForAddNew:
		// can force writing of psuedo-null value (as a non-null) by setting field dirty
		if (value != AFX_RFX_BIGINT_PSEUDO_NULL)
		{
			pFX->m_prs->SetDirtyFieldStatus(nField - 1);
			pFX->m_prs->ClearNullFieldStatus(nField - 1);
		}
		return;

	case CFieldExchange::MarkForUpdate:
		if (value != AFX_RFX_LONG_PSEUDO_NULL)
			pFX->m_prs->ClearNullFieldStatus(nField - 1);
		goto LDefault;

	case CFieldExchange::AllocCache:
		{
			CFieldInfo* pInfo = &pFX->m_prs->m_rgFieldInfos[nField - 1];
			pInfo->m_pvDataCache = new LONGLONG;
			pInfo->m_nDataType = AFX_RFX_BIGINT;
		}
		return;

#ifdef _DEBUG
	case CFieldExchange::DumpField:
		*pFX->m_pdcDump << "\n" << szName << " = " << value;
		return;
#endif // _DEBUG

	}
}

void AFXAPI RFX_Long(CFieldExchange* pFX, LPCTSTR szName, long& value)
{
	ENSURE_ARG(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ENSURE_ARG(AfxIsValidString(szName));

	UINT nField;
	if (!pFX->IsFieldType(&nField))
		return;

	LONG_PTR* plLength = pFX->m_prs->GetFieldLengthBuffer(
		nField - 1, pFX->m_nFieldType);
	switch (pFX->m_nOperation)
	{
	case CFieldExchange::BindFieldToColumn:
		{
#ifdef _DEBUG
			// Assumes all bound fields BEFORE unbound fields
			CODBCFieldInfo* pODBCInfo =
				&pFX->m_prs->m_rgODBCFieldInfos[nField - 1];

			if (pODBCInfo->m_nSQLType != SQL_C_LONG)
			{
				// Warn of possible field schema mismatch
				TRACE(traceDatabase, 1, "Warning: long converted from SQL type %ld.\n",
					pODBCInfo->m_nSQLType);
			}
#endif // _DEBUG
		}
		// fall through

	default:
LDefault:
		pFX->Default(szName, &value, plLength, SQL_C_LONG,
			sizeof(value), 10);
		return;

	case CFieldExchange::Fixup:
		if (*plLength == SQL_NULL_DATA)
		{
			pFX->m_prs->SetNullFieldStatus(nField - 1);
			value = AFX_RFX_LONG_PSEUDO_NULL;
		}
		return;

	case CFieldExchange::SetFieldNull:
		if ((pFX->m_pvField == NULL &&
			pFX->m_nFieldType == CFieldExchange::outputColumn) ||
			pFX->m_pvField == &value)
		{
			if (pFX->m_bField)
			{
				// Mark fields null
				pFX->m_prs->SetNullFieldStatus(nField - 1);
				value = AFX_RFX_LONG_PSEUDO_NULL;
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				pFX->m_prs->ClearNullFieldStatus(nField - 1);
				*plLength = sizeof(value);
			}
#ifdef _DEBUG
			pFX->m_nFieldFound = nField;
#endif
		}
		return;

	case CFieldExchange::MarkForAddNew:
		// can force writing of psuedo-null value (as a non-null) by setting field dirty
		if (value != AFX_RFX_LONG_PSEUDO_NULL)
		{
			pFX->m_prs->SetDirtyFieldStatus(nField - 1);
			pFX->m_prs->ClearNullFieldStatus(nField - 1);
		}
		return;

	case CFieldExchange::MarkForUpdate:
		if (value != AFX_RFX_LONG_PSEUDO_NULL)
			pFX->m_prs->ClearNullFieldStatus(nField - 1);
		goto LDefault;

	case CFieldExchange::AllocCache:
		{
			CFieldInfo* pInfo = &pFX->m_prs->m_rgFieldInfos[nField - 1];

			// Data cached by value, no allocation necessary
			pInfo->m_nDataType = AFX_RFX_LONG;
		}
		return;

#ifdef _DEBUG
	case CFieldExchange::DumpField:
		*pFX->m_pdcDump << "\n" << szName << " = " << value;
		return;
#endif // _DEBUG

	}
}

void AFXAPI RFX_Byte(CFieldExchange* pFX, LPCTSTR szName, BYTE& value)
{
	ENSURE_ARG(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ENSURE_ARG(AfxIsValidString(szName));

	UINT nField;
	if (!pFX->IsFieldType(&nField))
		return;

	LONG_PTR* plLength = pFX->m_prs->GetFieldLengthBuffer(
		nField - 1, pFX->m_nFieldType);
	switch (pFX->m_nOperation)
	{
	case CFieldExchange::BindFieldToColumn:
		{
#ifdef _DEBUG
			// Assumes all bound fields BEFORE unbound fields
			CODBCFieldInfo* pODBCInfo =
				&pFX->m_prs->m_rgODBCFieldInfos[nField - 1];

			if (pODBCInfo->m_nSQLType != SQL_TINYINT)
			{
				// Warn of possible field schema mismatch
				TRACE(traceDatabase, 1, "Warning: BYTE converted from SQL type %ld.\n",
					pODBCInfo->m_nSQLType);
			}
#endif // _DEBUG
		}
		// fall through

	default:
LDefault:
		pFX->Default(szName, &value, plLength, SQL_TINYINT,
			sizeof(value), 3);
		break;

	case CFieldExchange::Fixup:
		if (*plLength == SQL_NULL_DATA)
		{
			pFX->m_prs->SetNullFieldStatus(nField - 1);
			value = AFX_RFX_BYTE_PSEUDO_NULL;
		}
		return;

	case CFieldExchange::SetFieldNull:
		if ((pFX->m_pvField == NULL &&
			pFX->m_nFieldType == CFieldExchange::outputColumn) ||
			pFX->m_pvField == &value)
		{
			if (pFX->m_bField)
			{
				// Mark fields null
				pFX->m_prs->SetNullFieldStatus(nField - 1);
				value = AFX_RFX_BYTE_PSEUDO_NULL;
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				pFX->m_prs->ClearNullFieldStatus(nField - 1);
				*plLength = sizeof(value);
			}
#ifdef _DEBUG
			pFX->m_nFieldFound = nField;
#endif
		}
		return;

	case CFieldExchange::MarkForAddNew:
		// can force writing of psuedo-null value (as a non-null) by setting field dirty
		if (value != AFX_RFX_BYTE_PSEUDO_NULL)
		{
			pFX->m_prs->SetDirtyFieldStatus(nField - 1);
			pFX->m_prs->ClearNullFieldStatus(nField - 1);
		}
		return;

	case CFieldExchange::MarkForUpdate:
		if (value != AFX_RFX_BYTE_PSEUDO_NULL)
			pFX->m_prs->ClearNullFieldStatus(nField - 1);
		goto LDefault;

	case CFieldExchange::AllocCache:
		{
			CFieldInfo* pInfo = &pFX->m_prs->m_rgFieldInfos[nField - 1];

			// Data cached by value, no allocation necessary
			pInfo->m_nDataType = AFX_RFX_BYTE;
		}
		return;

#ifdef _DEBUG
	case CFieldExchange::DumpField:
		*pFX->m_pdcDump << "\n" << szName << " = " << value;
		return;
#endif // _DEBUG

	}
}

void AFXAPI RFX_Bool(CFieldExchange* pFX, LPCTSTR szName, BOOL& value)
{
	ENSURE_ARG(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ENSURE_ARG(AfxIsValidString(szName));

	UINT nField;
	if (!pFX->IsFieldType(&nField))
		return;

	LONG_PTR* plLength = pFX->m_prs->GetFieldLengthBuffer(
		nField - 1, pFX->m_nFieldType);
	switch (pFX->m_nOperation)
	{
	case CFieldExchange::BindFieldToColumn:
		{
#ifdef _DEBUG
			// Assumes all bound fields BEFORE unbound fields
			CODBCFieldInfo* pODBCInfo =
				&pFX->m_prs->m_rgODBCFieldInfos[nField - 1];

			if (pODBCInfo->m_nSQLType != SQL_BIT)
			{
				// Warn of possible field schema mismatch
				TRACE(traceDatabase, 1, "Warning: BOOL converted from SQL type %ld.\n",
					pODBCInfo->m_nSQLType);
			}
#endif // _DEBUG
		}
		// Fall through

	default:
LDefault:
		pFX->Default(szName, &value, plLength, SQL_BIT,
			sizeof(value), 1);
		return;

	case CFieldExchange::Fixup:
		if (*plLength == SQL_NULL_DATA)
		{
			pFX->m_prs->SetNullFieldStatus(nField - 1);
			value = AFX_RFX_BOOL_PSEUDO_NULL;
		}
		else
			// Cast BYTE into BOOL (int)
			value = *(BYTE *)&value;
		return;

	case CFieldExchange::SetFieldNull:
		if ((pFX->m_pvField == NULL &&
			pFX->m_nFieldType == CFieldExchange::outputColumn) ||
			pFX->m_pvField == &value)
		{
			if (pFX->m_bField)
			{
				// Mark fields null
				pFX->m_prs->SetNullFieldStatus(nField - 1);
				value = AFX_RFX_BOOL_PSEUDO_NULL;
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				pFX->m_prs->ClearNullFieldStatus(nField - 1);
				*plLength = sizeof(value);
			}
#ifdef _DEBUG
			pFX->m_nFieldFound = nField;
#endif
		}
		return;

	case CFieldExchange::MarkForAddNew:
		// can force writing of psuedo-null value (as a non-null) by setting field dirty
		if (value != AFX_RFX_BOOL_PSEUDO_NULL)
		{
			pFX->m_prs->SetDirtyFieldStatus(nField - 1);
			pFX->m_prs->ClearNullFieldStatus(nField - 1);
		}
		return;

	case CFieldExchange::MarkForUpdate:
		if (value != AFX_RFX_BOOL_PSEUDO_NULL)
			pFX->m_prs->ClearNullFieldStatus(nField - 1);
		goto LDefault;

	case CFieldExchange::AllocCache:
		{
			CFieldInfo* pInfo = &pFX->m_prs->m_rgFieldInfos[nField - 1];

			// Data cached by value, no allocation necessary
			pInfo->m_nDataType = AFX_RFX_BOOL;
		}
		return;

#ifdef _DEBUG
	case CFieldExchange::DumpField:
		*pFX->m_pdcDump << "\n" << szName << " = " << value;
		return;
#endif // _DEBUG

	}
}

// Note: CByteArray.m_pData must not be changed.  This address is registered
// with ODBC and must remain valid until the recordset is released.
void AFXAPI RFX_Binary(CFieldExchange* pFX, LPCTSTR szName,
	CByteArray& value, INT_PTR nMaxLength)
{
	ENSURE_ARG(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ENSURE_ARG(AfxIsValidString(szName));

	RETCODE nRetCode;
	UINT nField;
	if (!pFX->IsFieldType(&nField))
		return;

	LONG_PTR* plLength = pFX->m_prs->GetFieldLengthBuffer(
		nField - 1, pFX->m_nFieldType);

	BOOL bByRef = FALSE;
	switch (pFX->m_nOperation)
	{
	default:
LDefault:
		{
			void* pvData = NULL;
			if (value.GetSize() > 0)
			{
				if (bByRef)
					pvData = &value;
				else
					pvData = &value[0];
			}

			pFX->Default(szName, pvData, plLength, SQL_C_BINARY,
				(int)value.GetSize(), (UINT)value.GetSize());
		}
		return;

	case CFieldExchange::BindFieldToColumn:
		{
			// Assumes all bound fields BEFORE unbound fields
			CODBCFieldInfo* pODBCInfo =
				&pFX->m_prs->m_rgODBCFieldInfos[nField - 1];
			ULONG_PTR cbColumn = pODBCInfo->m_nPrecision;

#ifdef _DEBUG
			if (pODBCInfo->m_nSQLType != SQL_BINARY &&
				pODBCInfo->m_nSQLType != SQL_VARBINARY &&
				pODBCInfo->m_nSQLType != SQL_LONGVARBINARY)
			{
				// Warn of possible field schema mismatch
				TRACE(traceDatabase, 1, "Warning: CByteArray converted from SQL type %ld.\n",
					pODBCInfo->m_nSQLType);
			}
#endif // _DEBUG

			// Constrain to user specified max length
			if (cbColumn > (UINT_PTR)nMaxLength)
				cbColumn = nMaxLength;
			value.SetSize(cbColumn);
			AFX_SQL_SYNC(::SQLBindCol(pFX->m_prs->m_hstmt, (UWORD)nField,
				SQL_C_BINARY, &value[0], (LONG)cbColumn, plLength));
			if (!pFX->m_prs->Check(nRetCode))
				pFX->m_prs->ThrowDBException(nRetCode);

			// Add the member address to the field map
			pFX->m_prs->m_mapFieldIndex.SetAt(&value, (void*)(UINT_PTR)nField);
		}
		return;

	case CFieldExchange::Fixup:
		if (*plLength == SQL_NULL_DATA)
		{
			pFX->m_prs->SetNullFieldStatus(nField - 1);
			value.SetSize(1);
			value[0] = AFX_RFX_BYTE_PSEUDO_NULL;
		}
		else
		{
			ASSERT(*plLength <= (LONG)nMaxLength);
			((CDBByteArray&)value).SetLength((UINT)*plLength);
		}
		return;

	case CFieldExchange::SetFieldNull:
		if ((pFX->m_pvField == NULL &&
			pFX->m_nFieldType == CFieldExchange::outputColumn) ||
			pFX->m_pvField == &value)
		{
			if (pFX->m_bField)
			{
				// Mark fields null
				pFX->m_prs->SetNullFieldStatus(nField - 1);
				value.SetSize(1);
				value[0] = AFX_RFX_BYTE_PSEUDO_NULL;
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				pFX->m_prs->ClearNullFieldStatus(nField - 1);
				*plLength = value.GetSize();
			}
#ifdef _DEBUG
			pFX->m_nFieldFound = nField;
#endif
		}
		return;

	case CFieldExchange::MarkForAddNew:
		// can force writing of psuedo-null value (as a non-null) by setting field dirty
		if (value.GetSize() != 1 || value[0] != AFX_RFX_BYTE_PSEUDO_NULL)
		{
			pFX->m_prs->SetDirtyFieldStatus(nField - 1);
			pFX->m_prs->ClearNullFieldStatus(nField - 1);
		}
		return;

	case CFieldExchange::MarkForUpdate:
		if (value.GetSize() != 1 || value[0] != AFX_RFX_BYTE_PSEUDO_NULL)
			pFX->m_prs->ClearNullFieldStatus(nField - 1);
		bByRef = TRUE;
		goto LDefault;

	case CFieldExchange::StoreField:
		AfxStoreField(*pFX->m_prs, nField, &value);
		return;

	case CFieldExchange::LoadField:
		AfxLoadField(*pFX->m_prs, nField, &value, plLength);
		return;

	case CFieldExchange::AllocCache:
		{
			CFieldInfo* pInfo = &pFX->m_prs->m_rgFieldInfos[nField - 1];
			pInfo->m_pvDataCache = new CByteArray;
			pInfo->m_nDataType = AFX_RFX_BINARY;
		}
		return;

#ifdef _DEBUG
	case CFieldExchange::DumpField:
		*pFX->m_pdcDump << "\n" << szName << ":";
		value.Dump(*pFX->m_pdcDump);
		return;
#endif // _DEBUG

	}
}

void AFXAPI RFX_Date(CFieldExchange* pFX, LPCTSTR szName, CTime& value)
{
	ENSURE_ARG(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ENSURE_ARG(AfxIsValidString(szName));

	RETCODE nRetCode=0;
	UINT nField=0;
	if (!pFX->IsFieldType(&nField))
		return;

	ENSURE(pFX->m_prs);
	LONG_PTR* plLength = pFX->m_prs->GetFieldLengthBuffer(
		nField - 1, pFX->m_nFieldType);
	ENSURE(plLength);

	switch (pFX->m_nOperation)
	{
	default:
LDefault:
		pFX->Default(szName, &value, plLength, SQL_C_TIMESTAMP,
			sizeof(value), TIMESTAMP_PRECISION);
		return;

	case CFieldExchange::BindParam:
		{
			TIMESTAMP_STRUCT* pts=NULL;
			pFX->m_prs->m_bRebindParams = TRUE;

			// Allocate proxy array if necessary
			if (pFX->m_prs->m_pvParamProxy == NULL)
			{
				pFX->m_prs->m_pvParamProxy = new void*[pFX->m_prs->m_nParams];
				memset(pFX->m_prs->m_pvParamProxy, 0,
					pFX->m_prs->m_nParams*sizeof(void*));
				pFX->m_prs->m_nProxyParams = pFX->m_prs->m_nParams;
			}

			// Allocate TIMESTAMP_STRUCT if necessary for SQLBindParameter
			if (pFX->m_prs->m_pvParamProxy[nField-1] == NULL)
				pFX->m_prs->m_pvParamProxy[nField-1] = new TIMESTAMP_STRUCT;

			pts = (TIMESTAMP_STRUCT *)pFX->m_prs->m_pvParamProxy[nField-1];

			if (pFX->m_prs->IsParamStatusNull(nField - 1))
				*plLength = SQL_NULL_DATA;
			else
			{
				pts->year = (SWORD)value.GetYear();
				pts->month = (UWORD)value.GetMonth();
				pts->day = (UWORD)value.GetDay();
				pts->hour = (UWORD)value.GetHour();
				pts->minute = (UWORD)value.GetMinute();
				pts->second = (UWORD)value.GetSecond();
				pts->fraction = 0;
				*plLength = sizeof(TIMESTAMP_STRUCT);
			}

			AFX_SQL_SYNC(::SQLBindParameter(pFX->m_hstmt, (UWORD)nField,
				(SWORD)pFX->m_nFieldType, SQL_C_TIMESTAMP, SQL_C_TIMESTAMP,
				TIMESTAMP_PRECISION, 0, pts, 0, plLength));
			if (nRetCode != SQL_SUCCESS)
				pFX->m_prs->ThrowDBException(nRetCode, pFX->m_hstmt);

			// Add the member address to the param map
			pFX->m_prs->m_mapParamIndex.SetAt(&value, (void*)(UINT_PTR)nField);
		}
		return;

	case CFieldExchange::RebindParam:
		{
			if(pFX->m_prs->IsParamStatusNull(nField - 1))
				*plLength =  SQL_NULL_DATA;
			else
			{
				ASSERT(pFX->m_prs->m_nProxyParams != 0);
				// Fill buffer (expected by SQLBindParameter) with new param data
				TIMESTAMP_STRUCT* pts = (TIMESTAMP_STRUCT *)pFX->m_prs->m_pvParamProxy[nField-1];
				pts->year = (SWORD)value.GetYear();
				pts->month = (UWORD)value.GetMonth();
				pts->day = (UWORD)value.GetDay();
				pts->hour = (UWORD)value.GetHour();
				pts->minute = (UWORD)value.GetMinute();
				pts->second = (UWORD)value.GetSecond();
				pts->fraction = 0;
				*plLength = sizeof(TIMESTAMP_STRUCT);
			}
		}
		return;

	case CFieldExchange::BindFieldToColumn:
		{
#ifdef _DEBUG
			// Assumes all bound fields BEFORE unbound fields
			CODBCFieldInfo* pODBCInfo =
				&pFX->m_prs->m_rgODBCFieldInfos[nField - 1];

			if (pODBCInfo->m_nSQLType != SQL_DATE &&
				pODBCInfo->m_nSQLType != SQL_TIME &&
				pODBCInfo->m_nSQLType != SQL_TIMESTAMP)
			{
				// Warn of possible field schema mismatch
				TRACE(traceDatabase, 1, "Warning: CTime converted from SQL type %ld.\n",
					pODBCInfo->m_nSQLType);
			}
#endif // _DEBUG

			// Allocate proxy array if necessary
			if (pFX->m_prs->m_pvFieldProxy == NULL)
			{
				pFX->m_prs->m_pvFieldProxy = new void*[pFX->m_prs->m_nFields];
				memset(pFX->m_prs->m_pvFieldProxy, 0,
					pFX->m_prs->m_nFields*sizeof(void*));
				pFX->m_prs->m_nProxyFields = pFX->m_prs->m_nFields;
			}

			// Allocate TIMESTAMP_STRUCT for SQLBindCol (not necessary on Requery)
			if (pFX->m_prs->m_pvFieldProxy[nField-1] == NULL)
				pFX->m_prs->m_pvFieldProxy[nField-1] = new TIMESTAMP_STRUCT;

			AFX_SQL_SYNC(::SQLBindCol(pFX->m_prs->m_hstmt, (UWORD)nField,
				SQL_C_TIMESTAMP, pFX->m_prs->m_pvFieldProxy[nField-1],
				sizeof(TIMESTAMP_STRUCT), plLength));
			if (!pFX->m_prs->Check(nRetCode))
				pFX->m_prs->ThrowDBException(nRetCode);

			// Add the member address to the field map
			pFX->m_prs->m_mapFieldIndex.SetAt(&value, (void*)(UINT_PTR)nField);
		}
		return;

	case CFieldExchange::BindFieldForUpdate:
		if (pFX->m_prs->m_nProxyFields != 0)
		{
			// Fill buffer (expected by SQLSetPos) with new field data
			TIMESTAMP_STRUCT* pts=NULL;
			pts = (TIMESTAMP_STRUCT *)pFX->m_prs->m_pvFieldProxy[nField-1];
			pts->year = (SWORD)value.GetYear();
			pts->month = (UWORD)value.GetMonth();
			pts->day = (UWORD)value.GetDay();
			pts->hour = (UWORD)value.GetHour();
			pts->minute = (UWORD)value.GetMinute();
			pts->second = (UWORD)value.GetSecond();
			pts->fraction = 0;

			pFX->Default(szName, (void *)pts, plLength, SQL_C_TIMESTAMP,
				sizeof(TIMESTAMP_STRUCT), TIMESTAMP_PRECISION);
		}
		return;

	case CFieldExchange::Fixup:
		if (*plLength == SQL_NULL_DATA)
		{
			pFX->m_prs->SetNullFieldStatus(nField - 1);
			value = AFX_RFX_DATE_PSEUDO_NULL;
		}
		else
		{
			TIMESTAMP_STRUCT* pts =
				(TIMESTAMP_STRUCT*)pFX->m_prs->m_pvFieldProxy[nField-1];
 
		 
#ifdef _DEBUG
				if (pts->fraction != 0)
					TRACE(traceDatabase, 1, "Warning: ignoring milliseconds.\n");
#endif
				
				_ATLTRY{
				    
					value = CTime(pts->year, pts->month, pts->day,
					pts->hour, pts->minute, pts->second);
				
				}_ATLCATCHALL(){ 
				    //time is invalid 
					 pFX->m_prs->SetNullFieldStatus(nField - 1);
     				 value = AFX_RFX_DATE_PSEUDO_NULL;
	
				}
			 
		}
		return;

	case CFieldExchange::NameValue:
		if (pFX->m_prs->IsFieldStatusDirty(nField - 1))
		{
			*pFX->m_pstr += szName;
			*pFX->m_pstr += '=';
		}
		// Fall through

	case CFieldExchange::Value:
		if (pFX->m_prs->IsFieldStatusDirty(nField - 1))
		{
			TIMESTAMP_STRUCT* pts =
				(TIMESTAMP_STRUCT*)pFX->m_prs->m_pvFieldProxy[nField-1];
			if (pFX->m_prs->IsFieldStatusNull(nField - 1))
			{
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				pts->year = (SWORD)value.GetYear();
				pts->month = (UWORD)value.GetMonth();
				pts->day = (UWORD)value.GetDay();
				pts->hour = (UWORD)value.GetHour();
				pts->minute = (UWORD)value.GetMinute();
				pts->second = (UWORD)value.GetSecond();
				pts->fraction = 0;
				*plLength = sizeof(TIMESTAMP_STRUCT);
			}

			// If optimizing for bulk add, only need lengths & proxy set correctly
			if(!(pFX->m_prs->m_dwOptions & CRecordset::optimizeBulkAdd))
			{
				*pFX->m_pstr += '?';
				*pFX->m_pstr += pFX->m_lpszSeparator;
				pFX->m_nParamFields++;

				// Assumes all bound fields BEFORE unbound fields
				CODBCFieldInfo* pODBCInfo =
					&pFX->m_prs->m_rgODBCFieldInfos[nField - 1];

				AFX_SQL_SYNC(::SQLBindParameter(pFX->m_hstmt,
					(UWORD)pFX->m_nParamFields, SQL_PARAM_INPUT,
					SQL_C_TIMESTAMP, pODBCInfo->m_nSQLType,
					TIMESTAMP_PRECISION, 0, pts, 0, plLength));
			}
		}
		return;

	case CFieldExchange::SetFieldNull:
		if ((pFX->m_pvField == NULL &&
			pFX->m_nFieldType == CFieldExchange::outputColumn) ||
			pFX->m_pvField == &value)
		{
			if (pFX->m_bField)
			{
				// Mark fields null
				pFX->m_prs->SetNullFieldStatus(nField - 1);
				value = AFX_RFX_DATE_PSEUDO_NULL;
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				pFX->m_prs->ClearNullFieldStatus(nField - 1);
				*plLength = sizeof(TIMESTAMP_STRUCT);
			}
#ifdef _DEBUG
			pFX->m_nFieldFound = nField;
#endif
		}
		return;

	case CFieldExchange::MarkForAddNew:
		{
			// can force writing of psuedo-null value (as a non-null) by setting field dirty
			CTime timeNull = AFX_RFX_DATE_PSEUDO_NULL;
			if (value != timeNull)
			{
				pFX->m_prs->SetDirtyFieldStatus(nField - 1);
				pFX->m_prs->ClearNullFieldStatus(nField - 1);
			}
		}
		return;

	case CFieldExchange::MarkForUpdate:
		{
			CTime timeNull = AFX_RFX_DATE_PSEUDO_NULL;
			if (value != timeNull)
				pFX->m_prs->ClearNullFieldStatus(nField - 1);
		}
		goto LDefault;

	case CFieldExchange::LoadField:
		{
			// Get the field data
			CFieldInfo* pInfo = &pFX->m_prs->m_rgFieldInfos[nField - 1];

			// Restore the status
			pFX->m_prs->SetFieldStatus(nField - 1, pInfo->m_bStatus);

			// If not NULL, restore the value, length and proxy
			if (!pFX->m_prs->IsFieldStatusNull(nField - 1))
			{
				AfxCopyValueByRef(pInfo->m_pvDataCache, &value,
					plLength, pInfo->m_nDataType);

				// Restore proxy for correct WHERE CURRENT OF operations
				TIMESTAMP_STRUCT* pts =
					(TIMESTAMP_STRUCT*)pFX->m_prs->m_pvFieldProxy[nField-1];

				pts->year = (SWORD)value.GetYear();
				pts->month = (UWORD)value.GetMonth();
				pts->day = (UWORD)value.GetDay();
				pts->hour = (UWORD)value.GetHour();
				pts->minute = (UWORD)value.GetMinute();
				pts->second = (UWORD)value.GetSecond();
				pts->fraction = 0;
			}
			else
				*plLength = SQL_NULL_DATA;

#ifdef _DEBUG
			// Buffer address must not change - ODBC's SQLBindCol depends upon this
			if (pInfo->m_pvBindAddress != pFX->m_prs->m_pvFieldProxy[nField-1])
			{
				TRACE(traceDatabase, 0, "Error: CString buffer (column %u) address has changed!\n",
					nField);
				ASSERT(FALSE);
			}
#endif // _DEBUG
		}
		return;

	case CFieldExchange::AllocCache:
		{
			CFieldInfo* pInfo = &pFX->m_prs->m_rgFieldInfos[nField - 1];
			pInfo->m_pvDataCache = new CTime;
			pInfo->m_nDataType = AFX_RFX_DATE;
		}
		return;

#ifdef _DEBUG
	case CFieldExchange::DumpField:
		*pFX->m_pdcDump << "\n" << szName << " = " << value;
		return;
#endif // _DEBUG

	}
}

void AFXAPI RFX_Date(CFieldExchange* pFX, LPCTSTR szName,
	TIMESTAMP_STRUCT& value)
{
	ENSURE_ARG(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ENSURE_ARG(AfxIsValidString(szName));

	UINT nField=0;
	if (!pFX->IsFieldType(&nField))
		return;

	ENSURE(pFX->m_prs);
	LONG_PTR* plLength = pFX->m_prs->GetFieldLengthBuffer(
		nField - 1, pFX->m_nFieldType);
	ENSURE(plLength);

	switch (pFX->m_nOperation)
	{
	case CFieldExchange::BindFieldToColumn:
		{
#ifdef _DEBUG
			// Assumes all bound fields BEFORE unbound fields
			CODBCFieldInfo* pODBCInfo =
				&pFX->m_prs->m_rgODBCFieldInfos[nField - 1];

			ENSURE(pODBCInfo);

			if (pODBCInfo->m_nSQLType != SQL_DATE &&
				pODBCInfo->m_nSQLType != SQL_TIME &&
				pODBCInfo->m_nSQLType != SQL_TIMESTAMP)
			{
				// Warn of possible field schema mismatch
				TRACE(traceDatabase, 1, "Warning: TIMESTAMP_STRUCT converted from SQL type %ld.\n",
					pODBCInfo->m_nSQLType);
			}
#endif // _DEBUG
			// fall through
		}

	default:
LDefault:
		pFX->Default(szName, &value, plLength, SQL_C_TIMESTAMP,
			sizeof(value), TIMESTAMP_PRECISION);
		return;

	case CFieldExchange::Fixup:
		if (*plLength == SQL_NULL_DATA)
		{
			pFX->m_prs->SetNullFieldStatus(nField - 1);
			value.year = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
			value.month = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
			value.day = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
			value.hour = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
			value.minute = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
			value.second = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
			value.fraction = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
		}
		return;

	case CFieldExchange::SetFieldNull:
		if ((pFX->m_pvField == NULL &&
			pFX->m_nFieldType == CFieldExchange::outputColumn) ||
			pFX->m_pvField == &value)
		{
			if (pFX->m_bField)
			{
				// Mark fields null
				pFX->m_prs->SetNullFieldStatus(nField - 1);
				value.year = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
				value.month = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
				value.day = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
				value.hour = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
				value.minute = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
				value.second = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
				value.fraction = AFX_RFX_TIMESTAMP_PSEUDO_NULL;
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				pFX->m_prs->ClearNullFieldStatus(nField - 1);
				*plLength = sizeof(TIMESTAMP_STRUCT);
			}
#ifdef _DEBUG
			pFX->m_nFieldFound = nField;
#endif
		}
		return;

	case CFieldExchange::MarkForAddNew:
		// can force writing of psuedo-null value (as a non-null) by setting field dirty
		if (!(value.year == AFX_RFX_TIMESTAMP_PSEUDO_NULL &&
			value.month == AFX_RFX_TIMESTAMP_PSEUDO_NULL &&
			value.day == AFX_RFX_TIMESTAMP_PSEUDO_NULL &&
			value.hour == AFX_RFX_TIMESTAMP_PSEUDO_NULL &&
			value.minute == AFX_RFX_TIMESTAMP_PSEUDO_NULL &&
			value.second == AFX_RFX_TIMESTAMP_PSEUDO_NULL &&
			value.fraction == AFX_RFX_TIMESTAMP_PSEUDO_NULL ))
		{
			pFX->m_prs->SetDirtyFieldStatus(nField - 1);
			pFX->m_prs->ClearNullFieldStatus(nField - 1);
		}
		return;

	case CFieldExchange::MarkForUpdate:
		if (!(value.year == AFX_RFX_TIMESTAMP_PSEUDO_NULL &&
			value.month == AFX_RFX_TIMESTAMP_PSEUDO_NULL &&
			value.day == AFX_RFX_TIMESTAMP_PSEUDO_NULL &&
			value.hour == AFX_RFX_TIMESTAMP_PSEUDO_NULL &&
			value.minute == AFX_RFX_TIMESTAMP_PSEUDO_NULL &&
			value.second == AFX_RFX_TIMESTAMP_PSEUDO_NULL &&
			value.fraction == AFX_RFX_TIMESTAMP_PSEUDO_NULL ))
		{
			pFX->m_prs->ClearNullFieldStatus(nField - 1);
		}
		goto LDefault;

	case CFieldExchange::AllocCache:
		{
			CFieldInfo* pInfo = &pFX->m_prs->m_rgFieldInfos[nField - 1];
			pInfo->m_pvDataCache = new TIMESTAMP_STRUCT;
			pInfo->m_nDataType = AFX_RFX_TIMESTAMP;
		}
		return;

#ifdef _DEBUG
	case CFieldExchange::DumpField:
		*pFX->m_pdcDump << "\n" << szName << ".year = " << (int)value.year;
		*pFX->m_pdcDump << "\n" << szName << ".month = " << value.month;
		*pFX->m_pdcDump << "\n" << szName << ".day = " << value.day;
		*pFX->m_pdcDump << "\n" << szName << ".hour = " << value.hour;
		*pFX->m_pdcDump << "\n" << szName << ".minute = " << value.minute;
		*pFX->m_pdcDump << "\n" << szName << ".second = " << value.second;
		*pFX->m_pdcDump << "\n" << szName << ".fraction = " << value.fraction;
		return;
#endif // _DEBUG

	}
}

void AFXAPI RFX_Date(CFieldExchange* pFX, LPCTSTR szName,
	COleDateTime& value)
{
	ENSURE_ARG(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ENSURE_ARG(AfxIsValidString(szName));

	UINT nField=0;
	if (!pFX->IsFieldType(&nField))
		return;

	RETCODE nRetCode=0;
	ENSURE(pFX->m_prs);
	LONG_PTR* plLength = pFX->m_prs->GetFieldLengthBuffer(
		nField - 1, pFX->m_nFieldType);
	ENSURE(plLength);
	switch (pFX->m_nOperation)
	{
	case CFieldExchange::BindParam:
		{
			TIMESTAMP_STRUCT* pts=NULL;
			pFX->m_prs->m_bRebindParams = TRUE;

			if (pFX->m_prs->IsParamStatusNull(nField - 1))
			{
				pts = NULL;
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				// Allocate proxy array if necessary
				if (pFX->m_prs->m_pvParamProxy == NULL)
				{
					pFX->m_prs->m_pvParamProxy = new void*[pFX->m_prs->m_nParams];
					memset(pFX->m_prs->m_pvParamProxy, 0,
						pFX->m_prs->m_nParams*sizeof(void*));
					pFX->m_prs->m_nProxyParams = pFX->m_prs->m_nParams;
				}

				// Allocate TIMESTAMP_STRUCT if necessary for SQLBindParameter
				if (pFX->m_prs->m_pvParamProxy[nField-1] == NULL)
				{
					pts = new TIMESTAMP_STRUCT;
					pFX->m_prs->m_pvParamProxy[nField-1] = pts;
				}
				else
					pts = (TIMESTAMP_STRUCT *)pFX->m_prs->m_pvParamProxy[nField-1];

				pts->year = (SWORD)value.GetYear();
				pts->month = (UWORD)value.GetMonth();
				pts->day = (UWORD)value.GetDay();
				pts->hour = (UWORD)value.GetHour();
				pts->minute = (UWORD)value.GetMinute();
				pts->second = (UWORD)value.GetSecond();
				pts->fraction = 0;
				*plLength = sizeof(TIMESTAMP_STRUCT);
			}

			AFX_SQL_SYNC(::SQLBindParameter(pFX->m_hstmt, (UWORD)nField,
				(SWORD)pFX->m_nFieldType, SQL_C_TIMESTAMP, SQL_C_TIMESTAMP,
				TIMESTAMP_PRECISION, 0, pts, 0, plLength));
			if (nRetCode != SQL_SUCCESS)
				pFX->m_prs->ThrowDBException(nRetCode, pFX->m_hstmt);

			// Add the member address to the param map
			pFX->m_prs->m_mapParamIndex.SetAt(&value, (void*)(UINT_PTR)nField);
		}
		return;

	case CFieldExchange::NameValue:
		if (pFX->m_prs->IsFieldStatusDirty(nField - 1))
		{
			*pFX->m_pstr += szName;
			*pFX->m_pstr += '=';
		}
		// Fall through

	case CFieldExchange::Value:
		if (pFX->m_prs->IsFieldStatusDirty(nField - 1))
		{
			TIMESTAMP_STRUCT* pts =
				(TIMESTAMP_STRUCT*)pFX->m_prs->m_pvFieldProxy[nField-1];
			if (pFX->m_prs->IsFieldStatusNull(nField - 1))
			{
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				pts->year = (SWORD)value.GetYear();
				pts->month = (UWORD)value.GetMonth();
				pts->day = (UWORD)value.GetDay();
				pts->hour = (UWORD)value.GetHour();
				pts->minute = (UWORD)value.GetMinute();
				pts->second = (UWORD)value.GetSecond();
				pts->fraction = 0;
				*plLength = sizeof(TIMESTAMP_STRUCT);
			}

			// If optimizing for bulk add, only need lengths & proxy set correctly
			if(!(pFX->m_prs->m_dwOptions & CRecordset::optimizeBulkAdd))
			{
				*pFX->m_pstr += '?';
				*pFX->m_pstr += pFX->m_lpszSeparator;
				pFX->m_nParamFields++;

				// Assumes all bound fields BEFORE unbound fields
				CODBCFieldInfo* pODBCInfo =
					&pFX->m_prs->m_rgODBCFieldInfos[nField - 1];

				AFX_SQL_SYNC(::SQLBindParameter(pFX->m_hstmt,
					(UWORD)pFX->m_nParamFields, SQL_PARAM_INPUT,
					SQL_C_TIMESTAMP, pODBCInfo->m_nSQLType,
					TIMESTAMP_PRECISION, 0, pts, 0, plLength));
			}
		}
		return;

	case CFieldExchange::RebindParam:
		{
			*plLength = pFX->m_prs->IsParamStatusNull(nField - 1) ?
				SQL_NULL_DATA : sizeof(TIMESTAMP_STRUCT);
			if (pFX->m_prs->m_nProxyParams != 0)
			{
				// Fill buffer (expected by SQLBindParameter) with new param data
				TIMESTAMP_STRUCT* pts;
				pts = (TIMESTAMP_STRUCT *)pFX->m_prs->m_pvParamProxy[nField-1];
				pts->year = (SWORD)value.GetYear();
				pts->month = (UWORD)value.GetMonth();
				pts->day = (UWORD)value.GetDay();
				pts->hour = (UWORD)value.GetHour();
				pts->minute = (UWORD)value.GetMinute();
				pts->second = (UWORD)value.GetSecond();
				pts->fraction = 0;
			}
		}
		return;

	case CFieldExchange::BindFieldForUpdate:
		if (pFX->m_prs->m_nProxyFields != 0)
		{
			// Fill buffer (expected by SQLSetPos) with new field data
			TIMESTAMP_STRUCT* pts=NULL;
			pts = (TIMESTAMP_STRUCT *)pFX->m_prs->m_pvFieldProxy[nField-1];
			pts->year = (SWORD)value.GetYear();
			pts->month = (UWORD)value.GetMonth();
			pts->day = (UWORD)value.GetDay();
			pts->hour = (UWORD)value.GetHour();
			pts->minute = (UWORD)value.GetMinute();
			pts->second = (UWORD)value.GetSecond();
			pts->fraction = 0;

			pFX->Default(szName, (void *)pts, plLength, SQL_C_TIMESTAMP,
				sizeof(TIMESTAMP_STRUCT), TIMESTAMP_PRECISION);
		}
		return;

	case CFieldExchange::LoadField:
		{
			// Get the field data
			CFieldInfo* pInfo = &pFX->m_prs->m_rgFieldInfos[nField - 1];

			// Restore the status
			pFX->m_prs->SetFieldStatus(nField - 1, pInfo->m_bStatus);

			// If not NULL, restore the value, length and proxy
			if (!pFX->m_prs->IsFieldStatusNull(nField - 1))
			{
				AfxCopyValueByRef(pInfo->m_pvDataCache, &value,
					plLength, pInfo->m_nDataType);

				// Restore proxy for correct WHERE CURRENT OF operations
				TIMESTAMP_STRUCT* pts =
					(TIMESTAMP_STRUCT*)pFX->m_prs->m_pvFieldProxy[nField-1];

				pts->year = (SWORD)value.GetYear();
				pts->month = (UWORD)value.GetMonth();
				pts->day = (UWORD)value.GetDay();
				pts->hour = (UWORD)value.GetHour();
				pts->minute = (UWORD)value.GetMinute();
				pts->second = (UWORD)value.GetSecond();
				pts->fraction = 0;
			}
			else
				*plLength = SQL_NULL_DATA;

#ifdef _DEBUG
			// Buffer address must not change - ODBC's SQLBindCol depends upon this
			if (pInfo->m_pvBindAddress != pFX->m_prs->m_pvFieldProxy[nField-1])
			{
				TRACE(traceDatabase, 0, "Error: CString buffer (column %u) address has changed!\n",
					nField);
				ASSERT(FALSE);
			}
#endif // _DEBUG
		}
		return;

	case CFieldExchange::BindFieldToColumn:
		{
#ifdef _DEBUG
			// Assumes all bound fields BEFORE unbound fields
			CODBCFieldInfo* pODBCInfo =
				&pFX->m_prs->m_rgODBCFieldInfos[nField - 1];

			ENSURE(pODBCInfo);

			if (pODBCInfo->m_nSQLType != SQL_DATE &&
				pODBCInfo->m_nSQLType != SQL_TIME &&
				pODBCInfo->m_nSQLType != SQL_TIMESTAMP)
			{
				// Warn of possible field schema mismatch
				TRACE(traceDatabase, 1, "Warning: COleDateTime converted from SQL type %ld.\n",
					pODBCInfo->m_nSQLType);
			}
#endif // _DEBUG

			// Allocate proxy array if necessary
			if (pFX->m_prs->m_pvFieldProxy == NULL)
			{
				pFX->m_prs->m_pvFieldProxy = new void*[pFX->m_prs->m_nFields];
				memset(pFX->m_prs->m_pvFieldProxy, 0,
					pFX->m_prs->m_nFields*sizeof(void*));
				pFX->m_prs->m_nProxyFields = pFX->m_prs->m_nFields;
			}

			// Allocate TIMESTAMP_STRUCT for SQLBindCol (not necessary on Requery)
			if (pFX->m_prs->m_pvFieldProxy[nField-1] == NULL)
				pFX->m_prs->m_pvFieldProxy[nField-1] = new TIMESTAMP_STRUCT;

			AFX_SQL_SYNC(::SQLBindCol(pFX->m_prs->m_hstmt, (UWORD)nField,
				SQL_C_TIMESTAMP, pFX->m_prs->m_pvFieldProxy[nField-1],
				sizeof(TIMESTAMP_STRUCT), plLength));
			if (!pFX->m_prs->Check(nRetCode))
				pFX->m_prs->ThrowDBException(nRetCode);

			// Add the member address to the field map
			pFX->m_prs->m_mapFieldIndex.SetAt(&value, (void*)(UINT_PTR)nField);
		}
		return;

	default:
LDefault:
		pFX->Default(szName, &value, plLength, SQL_C_TIMESTAMP,
			sizeof(value), TIMESTAMP_PRECISION);
		return;

	case CFieldExchange::AllocCache:
		{
			CFieldInfo* pInfo = &pFX->m_prs->m_rgFieldInfos[nField - 1];
			pInfo->m_pvDataCache = new COleDateTime;
			pInfo->m_nDataType = AFX_RFX_OLEDATE;
		}
		return;

	case CFieldExchange::Fixup:
		if (*plLength == SQL_NULL_DATA)
		{
			pFX->m_prs->SetNullFieldStatus(nField - 1);
			value.SetStatus(COleDateTime::null);
		}
		else
		{
			TIMESTAMP_STRUCT* pts =
				(TIMESTAMP_STRUCT*)pFX->m_prs->m_pvFieldProxy[nField-1];
#ifdef _DEBUG
			if (pts->fraction != 0)
				TRACE(traceDatabase, 1, "Warning: ignoring milliseconds.\n");
#endif
			value = COleDateTime(pts->year, pts->month, pts->day,
				pts->hour, pts->minute, pts->second);
		}
		return;

	case CFieldExchange::SetFieldNull:
		if ((pFX->m_pvField == NULL &&
			pFX->m_nFieldType == CFieldExchange::outputColumn) ||
			pFX->m_pvField == &value)
		{
			if (pFX->m_bField)
			{
				// Mark fields null
				pFX->m_prs->SetNullFieldStatus(nField - 1);
				value.SetStatus(COleDateTime::null);
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				pFX->m_prs->ClearNullFieldStatus(nField - 1);
				*plLength = sizeof(TIMESTAMP_STRUCT);
			}
#ifdef _DEBUG
			pFX->m_nFieldFound = nField;
#endif
		}
		return;

	case CFieldExchange::MarkForAddNew:
		// can force writing of psuedo-null value (as a non-null) by setting field dirty
		if (value.GetStatus() != COleDateTime::null)
		{
			pFX->m_prs->SetDirtyFieldStatus(nField - 1);
			pFX->m_prs->ClearNullFieldStatus(nField - 1);
		}
		return;

	case CFieldExchange::MarkForUpdate:
		if (value.GetStatus() != COleDateTime::null)
			pFX->m_prs->ClearNullFieldStatus(nField - 1);
		goto LDefault;

#ifdef _DEBUG
	case CFieldExchange::DumpField:
		CString str;
		str = value.Format();
		*pFX->m_pdcDump << "\n" << str;
		return;
#endif // _DEBUG

	}
}

void AFXAPI RFX_LongBinary(CFieldExchange* pFX, LPCTSTR szName,
	CLongBinary& value)
{
	ENSURE_ARG(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ENSURE_ARG(AfxIsValidString(szName));


  
	RETCODE nRetCode;
	UINT nField;
	if (!pFX->IsFieldType(&nField))
		return;

	LONG_PTR* plLength = pFX->m_prs->GetFieldLengthBuffer(
		nField - 1, pFX->m_nFieldType);
 



	switch (pFX->m_nOperation )
	{
	case CFieldExchange::Name:
		pFX->m_prs->m_bLongBinaryColumns = TRUE;
		pFX->Default(szName, &value, plLength, SQL_C_DEFAULT, 0, 0);
		return;

	case CFieldExchange::BindFieldToColumn:
		// Don't bind if using update SQL, simply do SQLGetData on Fixup
		if (!pFX->m_prs->m_bUseUpdateSQL &&
			(pFX->m_prs->CanUpdate() || pFX->m_prs->CanAppend()))
		{
			// Bind for updates with cb=0 now. Driver may not support post Execute or ExtendedFetch binding
			AFX_SQL_SYNC(::SQLBindCol(pFX->m_prs->m_hstmt, (UWORD)nField, SQL_C_DEFAULT,
				&value, 0, plLength));
			if (!pFX->m_prs->Check(nRetCode))
				pFX->m_prs->ThrowDBException(nRetCode);
		}

		// Add the member address to the field map
		pFX->m_prs->m_mapFieldIndex.SetAt(&value, (void*)(UINT_PTR)nField);
		return;

#ifdef _DEBUG
	case CFieldExchange::BindParam:
		// CLongBinary parameters are not supported
		ASSERT(FALSE);

	case CFieldExchange::MarkForAddNew:
	case CFieldExchange::MarkForUpdate:
		// We do not archive LongBinary values
	case CFieldExchange::StoreField:
	case CFieldExchange::LoadField:
		// We do not archive LongBinary values
#endif // _DEBUG
	default:
		return;

	case CFieldExchange::Fixup:

		// Get the size of the long binary field
		*plLength = pFX->GetLongBinarySize(nField);

	 
		if (*plLength != SQL_NULL_DATA && *plLength !=0)
			pFX->GetLongBinaryData(nField, value, plLength);

		// Set the status and length
		if (*plLength == SQL_NULL_DATA)
		{
			// Field NULL, set length and status
			value.m_dwDataLength = 0;
			pFX->m_prs->SetNullFieldStatus(nField - 1);
		}
		else if (*plLength==0) {
			value.m_dwDataLength = 0;
			pFX->m_prs->ClearNullFieldStatus(nField - 1);
                                
		}	else	{
			// Field not NULL, clear the status (length already set)
			pFX->m_prs->ClearNullFieldStatus(nField - 1);
		}

		return;

	 
	case CFieldExchange::NameValue:
		if (pFX->m_prs->IsFieldStatusDirty(nField - 1))
		{
			*pFX->m_pstr += szName;
			*pFX->m_pstr += '=';
		}

		// Fall through
	case CFieldExchange::Value:
		if (pFX->m_prs->IsFieldStatusDirty(nField - 1))
		{
			// If user marked column NULL, reflect this in length
			if (pFX->m_prs->IsFieldStatusNull(nField - 1))
				*plLength = SQL_NULL_DATA;
			else
			{
				// Indicate data will be sent after SQLExecute
				// Length is signed value, it's limited by LONG_MAX
				if (value.m_dwDataLength >
					(ULONG)(LONG_MAX - labs(SQL_LEN_DATA_AT_EXEC_OFFSET)))
				{
					ASSERT(FALSE);
					*plLength = LONG_MAX - labs(SQL_LEN_DATA_AT_EXEC_OFFSET);
				}
				else
					*plLength = value.m_dwDataLength;

				*plLength = SQL_LEN_DATA_AT_EXEC(*plLength);
			}

			// If optimizing for bulk add, only need lengths set correctly
			if(!(pFX->m_prs->m_dwOptions & CRecordset::optimizeBulkAdd))
			{
				*pFX->m_pstr += '?';
				*pFX->m_pstr += pFX->m_lpszSeparator;
				pFX->m_nParamFields++;

				// Assumes all bound fields BEFORE unbound fields
				CODBCFieldInfo* pODBCInfo =
					&pFX->m_prs->m_rgODBCFieldInfos[nField - 1];

				AFX_SQL_SYNC(::SQLBindParameter(pFX->m_hstmt,
					(UWORD)pFX->m_nParamFields, SQL_PARAM_INPUT,
					SQL_C_DEFAULT, pODBCInfo->m_nSQLType,
					(SQLULEN)value.m_dwDataLength, 0, &value, 0, plLength));
				if (nRetCode != SQL_SUCCESS)
					pFX->m_prs->ThrowDBException(nRetCode, pFX->m_hstmt);
			}
		}
		return;

	case CFieldExchange::BindFieldForUpdate:
		if (pFX->m_prs->IsFieldStatusDirty(nField - 1))
		{
			// If user marked column NULL, reflect this in length
			if (pFX->m_prs->IsFieldStatusNull(nField - 1))
				*plLength = SQL_NULL_DATA;
			else
			{
				// Length is signed value, it's limited by LONG_MAX
				if (value.m_dwDataLength >
					(ULONG)(LONG_MAX - labs(SQL_LEN_DATA_AT_EXEC_OFFSET)))
				{
					ASSERT(FALSE);
					*plLength = LONG_MAX - labs(SQL_LEN_DATA_AT_EXEC_OFFSET);
				}
				else
					*plLength = value.m_dwDataLength;

				*plLength = SQL_LEN_DATA_AT_EXEC(*plLength);
			}
		}
		else
			*plLength = SQL_IGNORE;

		return;

	case CFieldExchange::UnbindFieldForUpdate:
		*plLength = value.m_dwDataLength;
		return;

	case CFieldExchange::SetFieldNull:
		if ((pFX->m_pvField == NULL &&
			pFX->m_nFieldType == CFieldExchange::outputColumn) ||
			pFX->m_pvField == &value)
		{
			if (pFX->m_bField)
			{
				// Mark fields null
				pFX->m_prs->SetNullFieldStatus(nField - 1);
				value.m_dwDataLength = 0;
				*plLength = SQL_NULL_DATA;
			}
			else
			{
				pFX->m_prs->ClearNullFieldStatus(nField - 1);

				// Length is signed value, it's limited by LONG_MAX
				if (value.m_dwDataLength >
					(ULONG)(LONG_MAX - labs(SQL_LEN_DATA_AT_EXEC_OFFSET)))
				{
					ASSERT(FALSE);
					*plLength = LONG_MAX - labs(SQL_LEN_DATA_AT_EXEC_OFFSET);
				}
				else
					*plLength = value.m_dwDataLength;

				*plLength = SQL_LEN_DATA_AT_EXEC(*plLength);
			}
#ifdef _DEBUG
			pFX->m_nFieldFound = nField;
#endif
		}
		return;

	case CFieldExchange::AllocCache:
		// Caching not supported for long binary
		return;

#ifdef _DEBUG
	case CFieldExchange::DumpField:
		*pFX->m_pdcDump << "\n" << szName << " = ";
		value.Dump(*pFX->m_pdcDump);
		return;
#endif // _DEBUG

	}
}

SQLLEN CFieldExchange::GetLongBinarySize(int nField)
{
	RETCODE nRetCode;
	int nDummy;
	SQLLEN lSize;

	// Give empty buffer to find size of entire LongBinary
	AFX_ODBC_CALL(::SQLGetData(m_prs->m_hstmt,
		(UWORD)nField, SQL_C_DEFAULT, &nDummy, 0, &lSize));

	switch (nRetCode)
	{
		case SQL_SUCCESS:
			break;

		case SQL_SUCCESS_WITH_INFO:
#ifdef _DEBUG
			{
				CDBException* e = new CDBException(nRetCode);
				e->BuildErrorString(m_prs->m_pDatabase,
					m_prs->m_hstmt, FALSE);

				// Ignore data truncated messages
				if (e->m_strStateNativeOrigin.Find(_T("State:01004")) < 0)
				{
					TRACE(traceDatabase, 0, "Warning: ODBC Success With Info, ");
					e->TraceErrorMessage(e->m_strError);
					e->TraceErrorMessage(e->m_strStateNativeOrigin);
				}
				e->Delete();
			}
#endif // _DEBUG
			break;

		default:
			m_prs->ThrowDBException(nRetCode);
	}

	return lSize;
}

void CFieldExchange::GetLongBinaryData(int nField, CLongBinary& lb,
	SQLLEN* plSize)
{
	ENSURE_ARG(plSize!=NULL);

	RETCODE nRetCode;
	SQLLEN lActualDataSize = 0;
	SQLLEN lChunkDataSize;
	SQLLEN lReallocSize;
	const BYTE* lpLongBinary;

	lb.m_dwDataLength = 0;

	// Determine initial chunk sizes
	if (*plSize == SQL_NO_TOTAL)
	{
		lChunkDataSize = m_lDefaultLBFetchSize;
		lReallocSize = m_lDefaultLBReallocSize;
	}
	else
	{
		lChunkDataSize = *plSize;
		lReallocSize = *plSize;
	}

	do
	{
		SQLLEN lNewChunkDataSize = lb.m_dwDataLength + lChunkDataSize;
		SQLLEN lNewReallocSize = lb.m_dwDataLength + lReallocSize;
		if ((lNewChunkDataSize < (SQLLEN)lb.m_dwDataLength) || (lNewChunkDataSize < lChunkDataSize))
                        AfxThrowMemoryException();
		if ((lNewReallocSize < (SQLLEN)lb.m_dwDataLength) || (lNewReallocSize < lReallocSize))
                        AfxThrowMemoryException();

		// Check if CLongBianary is big enough
		lpLongBinary = ReallocLongBinary(lb,
			lNewChunkDataSize,
			lNewReallocSize);

		// Adjust the pointer so that data added at end
		lpLongBinary += lb.m_dwDataLength;

		AFX_ODBC_CALL(::SQLGetData(m_prs->m_hstmt, (UWORD)nField,
			SQL_C_BINARY, (UCHAR*)lpLongBinary, lChunkDataSize, &lActualDataSize));
		::GlobalUnlock(lb.m_hData);

		switch (nRetCode)
		{
			case SQL_NO_DATA_FOUND:
				m_prs->SetNullFieldStatus(nField - 1);
				*plSize = SQL_NULL_DATA;
				break;

			case SQL_SUCCESS:
				// All data fetched
				lb.m_dwDataLength += lActualDataSize;
				*plSize = lb.m_dwDataLength;
				return;

			case SQL_SUCCESS_WITH_INFO:
				{
					CDBException* e = new CDBException(nRetCode);
					e->BuildErrorString(m_prs->m_pDatabase, m_prs->m_hstmt,
						FALSE);

					// Ignore data truncated messages
					if (e->m_strStateNativeOrigin.Find(_T("State:01004")) < 0)
					{
#ifdef _DEBUG
						TRACE(traceDatabase, 0, "Warning: ODBC Success With Info, ");
						e->TraceErrorMessage(e->m_strError);
						e->TraceErrorMessage(e->m_strStateNativeOrigin);
#endif // _DEBUG
						// Must be some other warning, should be finished
						lb.m_dwDataLength += lActualDataSize;
					}
					else
					{
						// Should only happen if further calls to SQLGetData necessary

						// Increment the length by the chunk size for subsequent SQLGetData call
						lb.m_dwDataLength += lChunkDataSize;

						// Recalculate chunk and alloc sizes
						lChunkDataSize = m_prs->GetLBFetchSize(lChunkDataSize);
						lReallocSize = m_prs->GetLBReallocSize(lReallocSize);
						// force loop to repeat
						lActualDataSize = SQL_NO_TOTAL;
					}

					*plSize = lb.m_dwDataLength;
					e->Delete();
				}
				break;

			default:
				m_prs->ThrowDBException(nRetCode);
		}

	} while (lActualDataSize == SQL_NO_TOTAL);
}

BYTE* CFieldExchange::ReallocLongBinary(CLongBinary& lb, SQLLEN lSizeRequired,
	SQLLEN lReallocSize)
{
	// realloc max of lSizeRequired, lReallocSize (m_dwDataLength untouched)

	if (lSizeRequired < 0)
	{
		ASSERT(FALSE);
		lSizeRequired = 0;
	}

	HGLOBAL hOldData = NULL;

	// Allocate or Realloc as req'd
	if (lb.m_hData == NULL)
		lb.m_hData = ::GlobalAlloc(GMEM_MOVEABLE, lReallocSize);
	else
	{
		ULONG_PTR dwSize = ::GlobalSize(lb.m_hData);
		if (dwSize < (DWORD)lSizeRequired)
		{
			// Save the old handle in case ReAlloc fails
			hOldData = lb.m_hData;

			// Allocate more memory if necessary
			lb.m_hData = ::GlobalReAlloc(lb.m_hData,
				__max(lSizeRequired, lReallocSize), GMEM_MOVEABLE);
		}
	}

	// Validate the memory was allocated and can be locked
	if (lb.m_hData == NULL)
	{
		// Restore the old handle (not NULL if Realloc failed)
		lb.m_hData = hOldData;
		AfxThrowMemoryException();
	}

	BYTE* lpLongBinary = (BYTE*)::GlobalLock(lb.m_hData);
	if (lpLongBinary == NULL)
	{
		::GlobalFree(lb.m_hData);
		lb.m_hData = NULL;
		AfxThrowMemoryException();
	}

	return lpLongBinary;
}

//////////////////////////////////////////////////////////////////////////////
// Recordset Field Exchange Helpers


void AFXAPI AfxStoreField(CRecordset& rs, UINT nField, void* pvField)
{
	ENSURE(rs.m_rgFieldInfos && nField >= 1 && nField <= rs.m_nFields);
	// Get the field data
	CFieldInfo* pInfo = &rs.m_rgFieldInfos[nField - 1];
	// Cache the status
	pInfo->m_bStatus = rs.GetFieldStatus(nField - 1);

	// Save the data
	if (!rs.IsFieldStatusNull(nField - 1))
	{
		// Don't need to save length for variable len
		// objects as CString and CByteArray track len internally
		LONG_PTR nDummyLength;

		if (pInfo->m_nDataType == AFX_RFX_BOOL ||
			pInfo->m_nDataType == AFX_RFX_BYTE ||
			pInfo->m_nDataType == AFX_RFX_INT ||
			pInfo->m_nDataType == AFX_RFX_LONG ||
			pInfo->m_nDataType == AFX_RFX_SINGLE)
		{
			// If caching data by value, pass a ref
			AfxCopyValueByRef(pvField, &pInfo->m_pvDataCache,
				&nDummyLength, pInfo->m_nDataType);
		}
		else
		{
			AfxCopyValueByRef(pvField, pInfo->m_pvDataCache,
				&nDummyLength, pInfo->m_nDataType);
		}
	}

#ifdef _DEBUG
	// Cache the bind address expected by ODBC
	switch(pInfo->m_nDataType)
	{
	default:
		// All types that are bound directly
		pInfo->m_pvBindAddress = pvField;
		break;

	case AFX_RFX_WTEXT:
		ENSURE_ARG(pvField);
		pInfo->m_pvBindAddress = static_cast<CStringW *>(pvField)->GetBuffer(0);
		static_cast<CStringW *>(pvField)->ReleaseBuffer();
		break;

	case AFX_RFX_ATEXT:
		ENSURE_ARG(pvField);
		pInfo->m_pvBindAddress = static_cast<CStringA *>(pvField)->GetBuffer(0);
		static_cast<CStringA *>(pvField)->ReleaseBuffer();
		break;

	case AFX_RFX_LPWSTR:
	case AFX_RFX_LPASTR:
		pInfo->m_pvBindAddress = pvField;
		break;

	case AFX_RFX_DATE:
	case AFX_RFX_OLEDATE:
		pInfo->m_pvBindAddress = rs.m_pvFieldProxy[nField-1];
		break;

	case AFX_RFX_BINARY:
		ENSURE_ARG(pvField);
		pInfo->m_pvBindAddress = ((CByteArray*)pvField)->GetData();
		break;
	}
#endif
}

void AFXAPI AfxLoadField(CRecordset& rs, UINT nField,
	void* pvField, LONG_PTR* plLength)
{
	// Get the field data
	CFieldInfo* pInfo = &rs.m_rgFieldInfos[nField - 1];

	// Assumes old field status cleared out
	rs.SetFieldStatus(nField - 1, pInfo->m_bStatus);

	// If not NULL, restore the value and the length
	if (!rs.IsFieldStatusNull(nField - 1))
	{
		if (pInfo->m_nDataType == AFX_RFX_BOOL ||
			pInfo->m_nDataType == AFX_RFX_BYTE ||
			pInfo->m_nDataType == AFX_RFX_INT ||
			pInfo->m_nDataType == AFX_RFX_LONG ||
			pInfo->m_nDataType == AFX_RFX_SINGLE)
		{
			// If caching data by value, pass a ref
			AfxCopyValueByRef(&pInfo->m_pvDataCache, pvField,
				plLength, pInfo->m_nDataType);
		}
		else
		{
			AfxCopyValueByRef(pInfo->m_pvDataCache, pvField,
				plLength, pInfo->m_nDataType);
		}
	}
	else
		*plLength = SQL_NULL_DATA;

#ifdef _DEBUG
	// Buffer address must not change - ODBC's SQLBindCol depends upon this
	if (pInfo->m_nDataType == AFX_RFX_BINARY)
	{
		// Change pvField to point to the data of the CByteArray
		pvField = ((CByteArray*)pvField)->GetData();
	}

	if (pInfo->m_pvBindAddress != pvField)
	{
		TRACE(traceDatabase, 0, "Error: field address (column %u) has changed!\n",
			nField);
		ASSERT(FALSE);
	}
#endif // _DEBUG
}

BOOL AFXAPI AfxCompareValueByRef(void* pvSrc, void* pvDest, int nSrcType)
{
	ENSURE_ARG(pvSrc != NULL);
	ENSURE_ARG(pvDest != NULL);

	switch(nSrcType)
	{
	case AFX_RFX_LONGBINARY:
		// Caching long binary Src not supported
	default:
		return FALSE;

	case AFX_RFX_LPWSTR:
		return ::lstrcmpW(static_cast<LPCWSTR>(pvDest), static_cast<LPCWSTR>(pvSrc)) == 0;

	case AFX_RFX_LPASTR:
		return ::lstrcmpA(static_cast<LPCSTR>(pvDest), static_cast<LPCSTR>(pvSrc)) == 0;

	case AFX_RFX_WTEXT:
		return *static_cast<CStringW *>(pvDest) == *static_cast<CStringW *>(pvSrc);

	case AFX_RFX_ATEXT:
		return *static_cast<CStringA *>(pvDest) == *static_cast<CStringA *>(pvSrc);

	case AFX_RFX_BINARY:
		{
			CByteArray* pByteArraySrc = (CByteArray*)pvSrc;
			CByteArray* pByteArrayDest = (CByteArray*)pvDest;

			// If sizes compare, compare the Src
			INT_PTR nSize = pByteArraySrc->GetSize();
			return nSize == pByteArrayDest->GetSize() &&
				   memcmp(pByteArrayDest->GetData(), pByteArraySrc->GetData(), nSize) == 0;
		}

	case AFX_RFX_BOOL:
		return *(BOOL*)pvDest == *(BOOL*)pvSrc;

	case AFX_RFX_BYTE:
		return *(BYTE*)pvDest == *(BYTE*)pvSrc;

	case AFX_RFX_INT:
		return *(int*)pvDest == *(int*)pvSrc;

	case AFX_RFX_BIGINT:
		return *(LONGLONG*)pvDest == *(LONGLONG*)pvSrc;

	case AFX_RFX_LONG:
		return *(long*)pvDest == *(long*)pvSrc;

	case AFX_RFX_SINGLE:
		return *(float*)pvDest == *(float*)pvSrc;

	case AFX_RFX_DOUBLE:
		return *(double*)pvDest == *(double*)pvSrc;

	case AFX_RFX_OLEDATE:
		return *(COleDateTime*)pvDest == *(COleDateTime*)pvSrc;

	case AFX_RFX_DATE:
		return *(CTime*)pvDest == *(CTime*)pvSrc;

	case AFX_RFX_TIMESTAMP:
		{
			TIMESTAMP_STRUCT* pSrc = (TIMESTAMP_STRUCT*)pvSrc;
			TIMESTAMP_STRUCT* pDest = (TIMESTAMP_STRUCT*)pvDest;
			return pSrc->year == pDest->year &&
				pSrc->month == pDest->month &&
				pSrc->day == pDest->day &&
				pSrc->hour == pDest->hour &&
				pSrc->minute == pDest->minute &&
				pSrc->second == pDest->second &&
				pSrc->fraction == pDest->fraction;
		}
	}
}

void AFXAPI AfxCopyValueByRef(void* pvSrc, void* pvDest, LONG_PTR* plLength, int nDestType)
{
	ENSURE_ARG(pvSrc != NULL);
	ENSURE_ARG(pvDest != NULL);
	ENSURE_ARG(plLength != NULL);

	switch (nDestType)
	{
	case AFX_RFX_LONGBINARY:
		// Caching long binary Dest not supported
	default:
		ASSERT(FALSE);
		break;

	case AFX_RFX_LPWSTR:
		lstrcpyW(static_cast<LPWSTR>(pvDest), static_cast<LPCWSTR>(pvSrc));
		*plLength = lstrlenW(static_cast<LPCWSTR>(pvDest)) * sizeof(WCHAR);
		break;

	case AFX_RFX_LPASTR:
		lstrcpyA(static_cast<LPSTR>(pvDest), static_cast<LPCSTR>(pvSrc));
		*plLength = lstrlenA(static_cast<LPCSTR>(pvDest))  * sizeof(CHAR);
		break;

	case AFX_RFX_WTEXT:
		*static_cast<CStringW *>(pvDest) = *static_cast<CStringW *>(pvSrc);
		*plLength = static_cast<CStringW *>(pvDest)->GetLength() * sizeof(WCHAR);
		break;

	case AFX_RFX_ATEXT:
		*static_cast<CStringA *>(pvDest) = *static_cast<CStringA *>(pvSrc);
		*plLength = static_cast<CStringA *>(pvDest)->GetLength() * sizeof(CHAR);
		break;

	case AFX_RFX_BINARY:
		((CByteArray*)pvDest)->Copy(*(CByteArray*)pvSrc);
		*plLength = ((CByteArray*)pvSrc)->GetSize();
		break;

	case AFX_RFX_BOOL:
		*(BOOL*)pvDest = *(BOOL*)pvSrc;
		*plLength = sizeof(BOOL);
		break;

	case AFX_RFX_BYTE:
		*(BYTE*)pvDest = *(BYTE*)pvSrc;
		*plLength = sizeof(BYTE);
		break;

	case AFX_RFX_INT:
		*(int*)pvDest = *(int*)pvSrc;
		*plLength = sizeof(int);
		break;

	case AFX_RFX_BIGINT:
		*(LONGLONG*)pvDest = *(LONGLONG*)pvSrc;
		*plLength = sizeof(LONGLONG);
		break;

	case AFX_RFX_LONG:
		*(long*)pvDest = *(long*)pvSrc;
		*plLength = sizeof(long);
		break;

	case AFX_RFX_SINGLE:
		*(float*)pvDest = *(float*)pvSrc;
		*plLength = sizeof(float);
		break;

	case AFX_RFX_DOUBLE:
		*(double*)pvDest = *(double*)pvSrc;
		*plLength = sizeof(double);
		break;

	case AFX_RFX_DATE:
		*(CTime*)pvDest = *(CTime*)pvSrc;
		*plLength = sizeof(TIMESTAMP_STRUCT);
		break;

	case AFX_RFX_OLEDATE:
		*(COleDateTime*)pvDest = *(COleDateTime*)pvSrc;
		*plLength = sizeof(TIMESTAMP_STRUCT);
		break;

	case AFX_RFX_TIMESTAMP:
		{
			TIMESTAMP_STRUCT* pDest = (TIMESTAMP_STRUCT*)pvDest;
			TIMESTAMP_STRUCT* pSrc = (TIMESTAMP_STRUCT*)pvSrc;

			pDest->year = pSrc->year;
			pDest->month = pSrc->month;
			pDest->day = pSrc->day;
			pDest->hour = pSrc->hour;
			pDest->minute = pSrc->minute;
			pDest->second = pSrc->second;
			pDest->fraction = pSrc->fraction;

			*plLength = sizeof(TIMESTAMP_STRUCT);
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////
// Bulk Recordset Field Exchange
template<typename CharType>
inline void RFX_Text_Bulk(CFieldExchange* pFX, LPCTSTR szName,
	_Out_ _Deref_post_cap_(nMaxLength) CharType **prgStrVals, LONG_PTR** prgLengths, int nMaxLength,
	short nFieldType)
{
	ENSURE_ARG(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ENSURE_ARG(AfxIsValidString(szName));
	

	UINT nField;
	if (!pFX->IsFieldType(&nField))
		return;

	ENSURE_ARG(prgStrVals!=NULL && prgLengths!=NULL);
	switch (pFX->m_nOperation)
	{
	case CFieldExchange::AllocMultiRowBuffer:
		{
			// The buffer pointer better be initialized to NULL
			// or cleanup in exceptional cases mail fail
			ASSERT(*prgStrVals == NULL);
			ASSERT(*prgLengths == NULL);

			int nRowsetSize = pFX->m_prs->GetRowsetSize();

			// Allocate buffers to hold data and length
			*prgStrVals = new CharType[nRowsetSize * nMaxLength]; // Make sure bulk-row fetching works for Unicode
			*prgLengths = new LONG_PTR[nRowsetSize];
		}
		break;

	case CFieldExchange::DeleteMultiRowBuffer:
		delete [] *prgStrVals;
		*prgStrVals = NULL;

		delete [] *prgLengths;
		*prgLengths = NULL;
		break;

	default:
		AfxRFXBulkDefault(pFX, szName, *prgStrVals, *prgLengths,
			nFieldType, nMaxLength);
		break;
	}
}

void AFXAPI RFX_Text_Bulk(CFieldExchange* pFX, LPCTSTR szName,
	_Out_ _Deref_post_cap_(nMaxLength) LPWSTR *prgStrVals, LONG_PTR** prgLengths, int nMaxLength)
{
	RFX_Text_Bulk(pFX, szName, prgStrVals, prgLengths, nMaxLength, SQL_C_WCHAR);
}

void AFXAPI RFX_Text_Bulk(CFieldExchange* pFX, LPCTSTR szName,
	_Out_ _Deref_post_cap_(nMaxLength) LPSTR *prgStrVals, LONG_PTR** prgLengths, int nMaxLength)
{
	RFX_Text_Bulk(pFX, szName, prgStrVals, prgLengths, nMaxLength, SQL_C_CHAR);
}

void AFXAPI RFX_Bool_Bulk(CFieldExchange* pFX, LPCTSTR szName,
	BOOL** prgBoolVals, LONG_PTR** prgLengths)
{
	ENSURE_ARG(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ENSURE_ARG(AfxIsValidString(szName));

	UINT nField;
	if (!pFX->IsFieldType(&nField))
		return;

	ENSURE_ARG(prgBoolVals!=NULL && prgLengths!=NULL);
	switch (pFX->m_nOperation)
	{
	case CFieldExchange::AllocMultiRowBuffer:
		{
			// The buffer pointer better be initialized to NULL
			// or cleanup in exceptional cases mail fail
			ASSERT(*prgBoolVals == NULL);
			ASSERT(*prgLengths == NULL);

			int nRowsetSize = pFX->m_prs->GetRowsetSize();

			// Allocate buffers to hold data and length
			*prgBoolVals = new BOOL[nRowsetSize];
			*prgLengths = new LONG_PTR[nRowsetSize];
		}
		break;

	case CFieldExchange::DeleteMultiRowBuffer:
		delete [] *prgBoolVals;
		*prgBoolVals = NULL;

		delete [] *prgLengths;
		*prgLengths = NULL;
		break;

	default:
		AfxRFXBulkDefault(pFX, szName, *prgBoolVals, *prgLengths,
			SQL_C_LONG, sizeof(BOOL));
		break;
	}
}

void AFXAPI RFX_Int_Bulk(CFieldExchange* pFX, LPCTSTR szName,
	int** prgIntVals, LONG_PTR** prgLengths)
{
	ENSURE_ARG(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ENSURE_ARG(AfxIsValidString(szName));

	UINT nField;
	if (!pFX->IsFieldType(&nField))
		return;

	ENSURE_ARG(prgIntVals!=NULL && prgLengths!=NULL);
	switch (pFX->m_nOperation)
	{
	case CFieldExchange::AllocMultiRowBuffer:
		{
			// The buffer pointer better be initialized to NULL
			// or cleanup in exceptional cases mail fail
			ASSERT(*prgIntVals == NULL);
			ASSERT(*prgLengths == NULL);

			int nRowsetSize = pFX->m_prs->GetRowsetSize();

			// Allocate buffers to hold data and length
			*prgIntVals = new int[nRowsetSize];
			*prgLengths = new LONG_PTR[nRowsetSize];
		}
		break;

	case CFieldExchange::DeleteMultiRowBuffer:
		delete [] *prgIntVals;
		*prgIntVals = NULL;

		delete [] *prgLengths;
		*prgLengths = NULL;
		break;

	default:
		AfxRFXBulkDefault(pFX, szName, *prgIntVals, *prgLengths,
			SQL_C_LONG, sizeof(int));
		break;
	}
}

void AFXAPI RFX_Long_Bulk(CFieldExchange* pFX, LPCTSTR szName,
	long** prgLongVals, LONG_PTR** prgLengths)
{
	ENSURE_ARG(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ENSURE_ARG(AfxIsValidString(szName));
	
	UINT nField;
	if (!pFX->IsFieldType(&nField))
		return;

	ENSURE_ARG(prgLongVals!=NULL && prgLengths!=NULL);
	switch (pFX->m_nOperation)
	{
	case CFieldExchange::AllocMultiRowBuffer:
		{
			// The buffer pointer better be initialized to NULL
			// or cleanup in exceptional cases mail fail
			ASSERT(*prgLongVals == NULL);
			ASSERT(*prgLengths == NULL);

			int nRowsetSize = pFX->m_prs->GetRowsetSize();

			// Allocate buffers to hold data and length
			*prgLongVals = new long[nRowsetSize];
			*prgLengths = new LONG_PTR[nRowsetSize];
		}
		break;

	case CFieldExchange::DeleteMultiRowBuffer:
		delete [] *prgLongVals;
		*prgLongVals = NULL;

		delete [] *prgLengths;
		*prgLengths = NULL;
		break;

	default:
		AfxRFXBulkDefault(pFX, szName, *prgLongVals, *prgLengths,
			SQL_C_LONG, sizeof(long));
		break;
	}
}

void AFXAPI RFX_Date_Bulk(CFieldExchange* pFX, LPCTSTR szName,
	TIMESTAMP_STRUCT** prgTSVals, LONG_PTR** prgLengths)
{
	ENSURE_ARG(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ENSURE_ARG(AfxIsValidString(szName));

	UINT nField;
	if (!pFX->IsFieldType(&nField))
		return;

	ENSURE_ARG(prgTSVals!=NULL && prgLengths!=NULL);
	switch (pFX->m_nOperation)
	{
	case CFieldExchange::AllocMultiRowBuffer:
		{
			// The buffer pointer better be initialized to NULL
			// or cleanup in exceptional cases mail fail
			ASSERT(*prgTSVals == NULL);
			ASSERT(*prgLengths == NULL);

			int nRowsetSize = pFX->m_prs->GetRowsetSize();

			// Allocate buffers to hold data and length
			*prgTSVals = new TIMESTAMP_STRUCT[nRowsetSize];
			*prgLengths = new LONG_PTR[nRowsetSize];
		}
		break;

	case CFieldExchange::DeleteMultiRowBuffer:
		delete [] *prgTSVals;
		*prgTSVals = NULL;

		delete [] *prgLengths;
		*prgLengths = NULL;
		break;

	default:
		AfxRFXBulkDefault(pFX, szName, *prgTSVals, *prgLengths,
			SQL_C_TIMESTAMP, sizeof(TIMESTAMP_STRUCT));
		break;
	}
}

void AFXAPI RFX_Byte_Bulk(CFieldExchange* pFX, LPCTSTR szName,
	BYTE** prgByteVals, LONG_PTR** prgLengths)
{
	ENSURE_ARG(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ENSURE_ARG(AfxIsValidString(szName));

	UINT nField;
	if (!pFX->IsFieldType(&nField))
		return;

	ENSURE_ARG(prgByteVals!=NULL && prgLengths!=NULL);
	switch (pFX->m_nOperation)
	{
	case CFieldExchange::AllocMultiRowBuffer:
		{
			// The buffer pointer better be initialized to NULL
			// or cleanup in exceptional cases mail fail
			ASSERT(*prgByteVals == NULL);
			ASSERT(*prgLengths == NULL);

			int nRowsetSize = pFX->m_prs->GetRowsetSize();

			// Allocate buffers to hold data and length
			*prgByteVals = new BYTE[nRowsetSize];
			*prgLengths = new LONG_PTR[nRowsetSize];
		}
		break;

	case CFieldExchange::DeleteMultiRowBuffer:
		delete [] *prgByteVals;
		*prgByteVals = NULL;

		delete [] *prgLengths;
		*prgLengths = NULL;
		break;

	default:
		AfxRFXBulkDefault(pFX, szName, *prgByteVals, *prgLengths,
			SQL_C_TINYINT, sizeof(BYTE));
		break;
	}
}

void AFXAPI RFX_Binary_Bulk(CFieldExchange* pFX, LPCTSTR szName,
	BYTE** prgByteVals, LONG_PTR** prgLengths, int nMaxLength)
{
	ENSURE_ARG(AfxIsValidAddress(pFX, sizeof(CFieldExchange)));
	ENSURE_ARG(AfxIsValidString(szName));

	UINT nField;
	if (!pFX->IsFieldType(&nField))
		return;

	ENSURE_ARG(prgByteVals!=NULL && prgLengths!=NULL);
	switch (pFX->m_nOperation)
	{
	case CFieldExchange::AllocMultiRowBuffer:
		{
			// The buffer pointer better be initialized to NULL
			// or cleanup in exceptional cases mail fail
			ASSERT(*prgByteVals == NULL);
			ASSERT(*prgLengths == NULL);

			int nRowsetSize = pFX->m_prs->GetRowsetSize();

			// Allocate buffers to hold data and length
			*prgByteVals = new BYTE[nRowsetSize * nMaxLength];
			*prgLengths = new LONG_PTR[nRowsetSize];
		}
		break;

	case CFieldExchange::DeleteMultiRowBuffer:
		delete [] *prgByteVals;
		*prgByteVals = NULL;

		delete [] *prgLengths;
		*prgLengths = NULL;
		break;

	default:
		AfxRFXBulkDefault(pFX, szName, *prgByteVals, *prgLengths,
			SQL_C_BINARY, nMaxLength);
		break;
	}
}

void AFXAPI AfxRFXBulkDefault(CFieldExchange* pFX, LPCTSTR szName,
	void* pv, LONG_PTR* rgLengths, int nCType, SQLULEN cbValue)
{
	ENSURE_ARG(pFX!=NULL);

	RETCODE nRetCode;

	switch(pFX->m_nOperation)
	{
	default:
		// Operation not valid for bulk fetch
		ASSERT(FALSE);
		return;

	case CFieldExchange::Name:
		ENSURE_ARG(szName!=NULL);
		// We require a name
		ASSERT(lstrlen(szName) != 0);

		*pFX->m_pstr += szName;
		*pFX->m_pstr += pFX->m_lpszSeparator;
		break;

	case CFieldExchange::BindFieldToColumn:
		AFX_SQL_SYNC(::SQLBindCol(pFX->m_prs->m_hstmt,
			(UWORD)pFX->m_nFields, (SWORD)nCType, pv, cbValue, rgLengths));
		if (!pFX->m_prs->Check(nRetCode))
			pFX->m_prs->ThrowDBException(nRetCode);
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////
// Inline function declarations expanded out-of-line

#ifndef _AFX_ENABLE_INLINES

#define _AFXDBRFX_INLINE
#include "afxdb.inl"

#endif


/////////////////////////////////////////////////////////////////////////////
