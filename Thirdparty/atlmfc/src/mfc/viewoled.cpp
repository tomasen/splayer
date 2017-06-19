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
#include <afxoledb.h>



#define new DEBUG_NEW

class _AfxUINT128
{
public:
	_AfxUINT128()
	{
	}
	_AfxUINT128( ULONGLONG n ) :
		nLo( n ),
		nHi( 0 )
	{
	}
	_AfxUINT128& operator<<=( UINT n )
	{
		if( n < 64 )
		{
			nHi <<= n;
			nHi |= nLo>>(64-n);
			nLo <<= n;
		}
		else
		{
			nHi = nLo<<(n-64);
			nLo = 0;
		}

		return( *this );
	}
	_AfxUINT128& operator>>=( UINT n )
	{
		if( n < 64 )
		{
			nLo >>= n;
			nLo |= nHi<<(64-n);
			nHi >>= n;
		}
		else
		{
			nLo = nHi>>(n-64);
			nHi = 0;
		}

		return( *this );
	}
	_AfxUINT128& operator++( int )
	{
		nLo++;
		if( nLo == 0 )
		{
			nHi++;
		}

		return( *this );
	}

public:
	ULONGLONG nLo;
	ULONGLONG nHi;
};

static bool operator!=( const _AfxUINT128& a, const _AfxUINT128& b )
{
	return( (a.nHi != b.nHi) || (a.nLo != b.nLo) );
}

static bool operator>=( const _AfxUINT128& a, const _AfxUINT128& b )
{
	if( a.nHi < b.nHi )
	{
		return( false );
	}
	else if( a.nHi > b.nHi )
	{
		return( true );
	}
	else
	{
		return( a.nLo >= b.nLo );
	}
}

static _AfxUINT128 operator+( const _AfxUINT128& a, const _AfxUINT128& b )
{
	_AfxUINT128 s;

	s.nLo = a.nLo+b.nLo;
	s.nHi = a.nHi+b.nHi;
	if( s.nLo < a.nLo )
	{
		s.nHi++;
	}

	return( s );
}

static _AfxUINT128 operator-( const _AfxUINT128& a, const _AfxUINT128& b )
{
	_AfxUINT128 d;

	d.nLo = a.nLo-b.nLo;
	d.nHi = a.nHi-b.nHi;
	if( d.nLo > a.nLo )
	{
		d.nHi--;
	}

	return( d );
}

static _AfxUINT128 operator*( const _AfxUINT128& a, ULONG b )
{
	_AfxUINT128 p;
	_AfxUINT128 nTemp;

	p = ULONGLONG( ULONG( a.nLo ) )*b;

	nTemp = (a.nLo>>32)*b;
	nTemp <<= 32;
	p = p+nTemp;

	nTemp = ULONGLONG( ULONG( a.nHi ) )*b;
	nTemp <<= 64;
	p = p+nTemp;

	nTemp = (a.nHi>>32)*b;
	nTemp <<= 96;
	p = p+nTemp;

	return( p );
}

static _AfxUINT128 operator/( const _AfxUINT128& a, const _AfxUINT128& b )
{
	_AfxUINT128 q;
	_AfxUINT128 a_;
	_AfxUINT128 b_;

	q = 0;

	a_ = a;
	b_ = b;
	while( !(b_.nHi&(1i64<<63)) )
	{
		b_ <<= 1;
	}

	while( b_ >= b )
	{
		q <<= 1;
		if( a_ >= b_ )
		{
			q++;
			a_ = a_-b_;
		}
		b_ >>= 1;
	}

	return( q );
}

static _AfxUINT128 operator%( const _AfxUINT128& a, const _AfxUINT128& b )
{
	_AfxUINT128 q;
	_AfxUINT128 a_;
	_AfxUINT128 b_;

	q = 0;

	a_ = a;
	b_ = b;
	while( !(b_.nHi&(1i64<<63)) )
	{
		b_ <<= 1;
	}

	while( b_ >= b )
	{
		q <<= 1;
		if( a_ >= b_ )
		{
			q++;
			a_ = a_-b_;
		}
		b_ >>= 1;
	}

	return( a_ );
}

void AFXAPI DDX_Text(CDataExchange* pDX, int nIDC, DB_NUMERIC& value)
{
	ENSURE_ARG(pDX!=NULL);
	HWND hWndCtrl;

	pDX->PrepareEditCtrl(nIDC);
	pDX->m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);
	if (pDX->m_bSaveAndValidate)
	{
		CString strText;
		int nLength;
		_AfxUINT128 nValue;
		NUMPARSE parse;
		HRESULT hResult;
		LPBYTE pbDigit;
		int iDigit;

		nLength = ::GetWindowTextLength(hWndCtrl);
		::GetWindowText(hWndCtrl, strText.GetBufferSetLength(nLength), nLength+1);
		strText.ReleaseBuffer();

		ATL::CTempBuffer< BYTE > pbDigits(nLength);
		parse.cDig = nLength;
		parse.dwInFlags = NUMPRS_STD&~NUMPRS_HEX_OCT;
		parse.dwOutFlags = 0;
		parse.cchUsed = 0;
		parse.nBaseShift = 0;
		parse.nPwr10 = 0;
		hResult = VarParseNumFromStr(const_cast<LPOLESTR>(CStringW(strText).GetString()),
			::GetThreadLocale(), NUMPRS_STD&~NUMPRS_HEX_OCT, &parse, pbDigits);
		if (FAILED(hResult))
		{
			pDX->Fail();
		}
		if (parse.nPwr10 <= 0)
		{
			if (parse.nPwr10 < -38)
				pDX->Fail();
			value.scale = BYTE( -parse.nPwr10 );
		}
		else
		{
			if ((parse.nPwr10+parse.cDig) > 38)
				pDX->Fail();
			value.scale = 0;
		}
		value.precision = BYTE(parse.cDig+max(parse.nPwr10, 0));
		if (parse.dwOutFlags&NUMPRS_NEG)
		{
			value.sign = 0;
		}
		else
		{
			value.sign = 1;
		}

		nValue = 0;

		pbDigit = pbDigits;
		for (iDigit = 0; iDigit < parse.cDig; iDigit++)
		{
			nValue = nValue*10;
			nValue = nValue+*pbDigit;
			pbDigit++;
		}

		for (iDigit = 0; iDigit < parse.nPwr10; iDigit++)
		{
			nValue = nValue*10;
		}
		*(UNALIGNED ULONGLONG*)&value.val[0] = nValue.nLo;
		*(UNALIGNED ULONGLONG*)&value.val[8] = nValue.nHi;
	}
	else
	{
		TCHAR szText[41];  // 38 digits+sign+decimal+null
		_AfxUINT128 n;
		LPTSTR pszText;
		ULONG iDigit;

		ASSERT( value.precision <= 38 );
		n.nLo = *(UNALIGNED ULONGLONG*)&value.val[0];
		n.nHi = *(UNALIGNED ULONGLONG*)&value.val[8];

		szText[40] = _T( '\0' );
		pszText = &szText[40];
		iDigit = 0;
		while( (n != 0) || (iDigit <= value.scale) )
		{
			_AfxUINT128 nRemainder;

			if( (iDigit == value.scale) && (iDigit != 0) )
			{
				pszText--;
				*pszText = _T( '.' );
			}
			nRemainder = n%10;
			n = n/10;
			ASSERT( nRemainder.nHi == 0 );
			ASSERT( nRemainder.nLo < 10 );
			pszText--;
			*pszText = TCHAR( nRemainder.nLo+_T( '0' ) );
			iDigit++;
		}
		if( pszText == &szText[40] )
		{
			pszText--;
			*pszText = _T( '0' );
		}
		if( value.sign == 0 )
		{
			pszText--;
			*pszText = _T( '-' );
		}

		AfxSetWindowText(hWndCtrl, pszText);
	}
}

void AFXAPI DDX_Text(CDataExchange* pDX, int nIDC, DBDATE& value)
{
	ENSURE_ARG(pDX!=NULL);
	COleDateTime date;
	pDX->PrepareEditCtrl(nIDC);
	HWND hWndCtrl;
	pDX->m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);
	if (pDX->m_bSaveAndValidate)
	{
		int nLen = ::GetWindowTextLength(hWndCtrl);
		CString strTemp;

		::GetWindowText(hWndCtrl, strTemp.GetBufferSetLength(nLen), nLen+1);
		strTemp.ReleaseBuffer();

		if (!date.ParseDateTime(strTemp))  // throws exception
		{
			// Can't convert string to datetime
			AfxMessageBox(AFX_IDP_PARSE_DATE);
			pDX->Fail();    // throws exception
		}

		value.year = short(date.GetYear());
		value.month = USHORT(date.GetMonth());
		value.day = USHORT(date.GetDay());
	}
	else
	{
		date.SetDate(value.year, value.month, value.day);
		CString strTemp = date.Format();
		AfxSetWindowText(hWndCtrl, strTemp);
	}
}

void AFXAPI DDX_Text(CDataExchange* pDX, int nIDC, DBTIME& value)
{
	ENSURE_ARG(pDX!=NULL);
	COleDateTime date;
	pDX->PrepareEditCtrl(nIDC);
	HWND hWndCtrl;
	pDX->m_pDlgWnd->GetDlgItem(nIDC, &hWndCtrl);
	if (pDX->m_bSaveAndValidate)
	{
		int nLen = ::GetWindowTextLength(hWndCtrl);
		CString strTemp;

		::GetWindowText(hWndCtrl, strTemp.GetBufferSetLength(nLen), nLen+1);
		strTemp.ReleaseBuffer();

		if (!date.ParseDateTime(strTemp, VAR_TIMEVALUEONLY))  // throws exception
		{
			// Can't convert string to datetime
			AfxMessageBox(AFX_IDP_PARSE_TIME);
			pDX->Fail();    // throws exception
		}

		value.hour = USHORT(date.GetHour());
		value.minute = USHORT(date.GetMinute());
		value.second = USHORT(date.GetSecond());
	}
	else
	{
		date.SetTime(value.hour, value.minute, value.second);
		CString strTemp = date.Format();
		AfxSetWindowText(hWndCtrl, strTemp);
	}
}

void AFXAPI DDX_DateTimeCtrl(CDataExchange* pDX, int nIDC, DBDATE& value)
{
	ENSURE_ARG(pDX!=NULL);
	SYSTEMTIME st;

	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
	CDateTimeCtrl* pWnd = (CDateTimeCtrl*) CWnd::FromHandle(hWndCtrl);
	ENSURE(pWnd!=NULL);
	if (pDX->m_bSaveAndValidate)
	{
		pWnd->GetTime(&st);
		value.year = st.wYear;
		value.month = st.wMonth;
		value.day = st.wDay;
	}
	else
	{
		st.wYear = value.year;
		st.wMonth = value.month;
		st.wDayOfWeek = 0;
		st.wDay = value.day;
		st.wHour = 0;
		st.wMinute = 0;
		st.wSecond = 0;
		st.wMilliseconds = 0;
		pWnd->SetTime(&st);
	}
}

void AFXAPI DDX_DateTimeCtrl(CDataExchange* pDX, int nIDC, DBTIME& value)
{
	ENSURE_ARG(pDX!=NULL);
	SYSTEMTIME st;

	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
	CDateTimeCtrl* pWnd = (CDateTimeCtrl*) CWnd::FromHandle(hWndCtrl);
	ENSURE(pWnd!=NULL);
	if (pDX->m_bSaveAndValidate)
	{
		pWnd->GetTime(&st);
		value.hour = st.wHour;
		value.minute = st.wMinute;
		value.second = st.wSecond;
	}
	else
	{
		::GetSystemTime(&st);
		st.wHour = value.hour;
		st.wMinute = value.minute;
		st.wSecond = value.second;
		st.wMilliseconds = 0;
		pWnd->SetTime(&st);
	}
}

void AFXAPI DDX_MonthCalCtrl(CDataExchange* pDX, int nIDC,
	DBDATE& value)
{
	ENSURE_ARG(pDX!=NULL);
	SYSTEMTIME st;

	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
	CMonthCalCtrl* pWnd = (CMonthCalCtrl*) CWnd::FromHandle(hWndCtrl);
	ENSURE(pWnd!=NULL);
	if (pDX->m_bSaveAndValidate)
	{
		pWnd->GetCurSel(&st);
		value.year = st.wYear;
		value.month = st.wMonth;
		value.day = st.wDay;
	}
	else
	{
		st.wYear = value.year;
		st.wMonth = value.month;
		st.wDay = value.day;
		st.wDayOfWeek = 0;
		st.wHour = 0;
		st.wMinute = 0;
		st.wSecond = 0;
		st.wMilliseconds = 0;
		pWnd->SetCurSel(&st);
	}
}


/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(COleDBRecordView, CFormView)
	//{{AFX_MSG_MAP(COleDBRecordView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_COMMAND_EX(ID_RECORD_FIRST, &COleDBRecordView::OnMove)
	ON_UPDATE_COMMAND_UI(ID_RECORD_FIRST, &COleDBRecordView::OnUpdateRecordFirst)
	ON_COMMAND_EX(ID_RECORD_PREV, &COleDBRecordView::OnMove)
	ON_UPDATE_COMMAND_UI(ID_RECORD_PREV, &COleDBRecordView::OnUpdateRecordPrev)
	ON_COMMAND_EX(ID_RECORD_NEXT, &COleDBRecordView::OnMove)
	ON_UPDATE_COMMAND_UI(ID_RECORD_NEXT, &COleDBRecordView::OnUpdateRecordNext)
	ON_COMMAND_EX(ID_RECORD_LAST, &COleDBRecordView::OnMove)
	ON_UPDATE_COMMAND_UI(ID_RECORD_LAST, &COleDBRecordView::OnUpdateRecordLast)
END_MESSAGE_MAP()

void COleDBRecordView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
}

BOOL COleDBRecordView::OnMove(UINT nIDMoveCommand)
{
	CRowset<>* pSet = OnGetRowset();

	if (!UpdateData())
		return TRUE;
	pSet->SetData(0);

	HRESULT hr = E_UNEXPECTED;
	switch (nIDMoveCommand)
	{
		case ID_RECORD_PREV:
			hr = pSet->MovePrev();
			if (hr != S_OK)
				m_bOnFirstRecord = TRUE;
			else
				m_bOnLastRecord  = FALSE;
			break;

		case ID_RECORD_FIRST:
			hr = pSet->MoveFirst();
			if (hr == S_OK)
			{
				m_bOnFirstRecord = TRUE;
				m_bOnLastRecord  = FALSE;
			}
			break;

		case ID_RECORD_NEXT:
			hr = pSet->MoveNext();
			if (hr == S_OK)
				m_bOnFirstRecord = FALSE;
			else
				m_bOnLastRecord  = TRUE;
			break;

		case ID_RECORD_LAST:
			hr = pSet->MoveLast();
			if (hr == S_OK)
			{
				m_bOnFirstRecord = FALSE;
				m_bOnLastRecord  = TRUE;
			}
			break;

		default:
			// Unexpected case value
			ASSERT(FALSE);
	}

	if (hr != S_OK)
		return FALSE;

	// Show results of move operation
	UpdateData(FALSE);
	return TRUE;
}

void COleDBRecordView::OnUpdateRecordFirst(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_bOnFirstRecord);
}

void COleDBRecordView::OnUpdateRecordPrev(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_bOnFirstRecord);
}

void COleDBRecordView::OnUpdateRecordNext(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_bOnLastRecord);
}

void COleDBRecordView::OnUpdateRecordLast(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_bOnLastRecord);
}

//////////////////////////////////////////////////////////////////////////


IMPLEMENT_DYNAMIC(COleDBRecordView, CFormView)

//////////////////////////////////////////////////////////////////////////
