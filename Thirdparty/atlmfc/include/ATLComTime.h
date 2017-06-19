// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

#ifndef __ATLCOMTIME_H__
#define __ATLCOMTIME_H__

#pragma once

#pragma warning(push)
#pragma warning(disable:4159)

#include <atltime.h>

#if defined(_M_IX86)
#pragma pack(push, 4)
#else
#pragma pack(push, ATL_PACKING)
#endif

struct tagVARIANT;
typedef tagVARIANT VARIANT;

typedef double DATE;


 
namespace ATL
{

class COleDateTimeSpan
{
// Constructors
public:
	COleDateTimeSpan() throw();

	COleDateTimeSpan(double dblSpanSrc) throw();
	COleDateTimeSpan(LONG lDays, int nHours, int nMins, int nSecs) throw();

// Attributes
	enum DateTimeSpanStatus
	{
		valid = 0,
		invalid = 1,    // Invalid span (out of range, etc.)
		null = 2,       // Literally has no value
	};

	double m_span;
	DateTimeSpanStatus m_status;

	void SetStatus(DateTimeSpanStatus status) throw();
	DateTimeSpanStatus GetStatus() const throw();

	double GetTotalDays() const throw();    // span in days (about -3.65e6 to 3.65e6)
	double GetTotalHours() const throw();   // span in hours (about -8.77e7 to 8.77e6)
	double GetTotalMinutes() const throw(); // span in minutes (about -5.26e9 to 5.26e9)
	double GetTotalSeconds() const throw(); // span in seconds (about -3.16e11 to 3.16e11)

	LONG GetDays() const throw();       // component days in span
	LONG GetHours() const throw();      // component hours in span (-23 to 23)
	LONG GetMinutes() const throw();    // component minutes in span (-59 to 59)
	LONG GetSeconds() const throw();    // component seconds in span (-59 to 59)

// Operations
	COleDateTimeSpan& operator=(double dblSpanSrc) throw();

	bool operator==(const COleDateTimeSpan& dateSpan) const throw();
	bool operator!=(const COleDateTimeSpan& dateSpan) const throw();
	bool operator<(const COleDateTimeSpan& dateSpan) const throw();
	bool operator>(const COleDateTimeSpan& dateSpan) const throw();
	bool operator<=(const COleDateTimeSpan& dateSpan) const throw();
	bool operator>=(const COleDateTimeSpan& dateSpan) const throw();

	// DateTimeSpan math
	COleDateTimeSpan operator+(const COleDateTimeSpan& dateSpan) const throw();
	COleDateTimeSpan operator-(const COleDateTimeSpan& dateSpan) const throw();
	COleDateTimeSpan& operator+=(const COleDateTimeSpan dateSpan) throw();
	COleDateTimeSpan& operator-=(const COleDateTimeSpan dateSpan) throw();
	COleDateTimeSpan operator-() const throw();

	operator double() const throw();

	void SetDateTimeSpan(LONG lDays, int nHours, int nMins, int nSecs) throw();

	// formatting
	CString Format(LPCTSTR pFormat) const;
	CString Format(UINT nID) const;

// Implementation
	void CheckRange();

private:
	static const double OLE_DATETIME_HALFSECOND;
};

class COleDateTime
{
// Constructors
public:
	static COleDateTime WINAPI GetCurrentTime() throw();

	COleDateTime() throw();

	COleDateTime(const VARIANT& varSrc) throw();
	COleDateTime(DATE dtSrc) throw();

	COleDateTime(__time32_t timeSrc) throw();
	COleDateTime(__time64_t timeSrc) throw();

	COleDateTime(const SYSTEMTIME& systimeSrc) throw();
	COleDateTime(const FILETIME& filetimeSrc) throw();

	COleDateTime(int nYear, int nMonth, int nDay,
		int nHour, int nMin, int nSec) throw();
	COleDateTime(WORD wDosDate, WORD wDosTime) throw();

#ifdef __oledb_h__
	COleDateTime( const DBTIMESTAMP& dbts) throw();
	bool GetAsDBTIMESTAMP( DBTIMESTAMP& dbts ) const throw();
#endif

// Attributes
	enum DateTimeStatus
	{
		error = -1,
		valid = 0,
		invalid = 1,    // Invalid date (out of range, etc.)
		null = 2,       // Literally has no value
	};

	DATE m_dt;
	DateTimeStatus m_status;

	void SetStatus(DateTimeStatus status) throw();
	DateTimeStatus GetStatus() const throw();

	bool GetAsSystemTime(SYSTEMTIME& sysTime) const throw();
	bool GetAsUDATE( UDATE& udate ) const throw();

	int GetYear() const throw();
	// Month of year (1 = January)
	int GetMonth() const throw();
	// Day of month (1-31)
	int GetDay() const throw();
	// Hour in day (0-23)
	int GetHour() const throw();
	// Minute in hour (0-59)
	int GetMinute() const throw();
	// Second in minute (0-59)
	int GetSecond() const throw();
	// Day of week (1 = Sunday, 2 = Monday, ..., 7 = Saturday)
	int GetDayOfWeek() const throw();
	// Days since start of year (1 = January 1)
	int GetDayOfYear() const throw();

// Operations
	COleDateTime& operator=(const VARIANT& varSrc) throw();
	COleDateTime& operator=(DATE dtSrc) throw();

	COleDateTime& operator=(const __time32_t& timeSrc) throw();
	COleDateTime& operator=(const __time64_t& timeSrc) throw();

	COleDateTime& operator=(const SYSTEMTIME& systimeSrc) throw();
	COleDateTime& operator=(const FILETIME& filetimeSrc) throw();
	COleDateTime& operator=(const UDATE& udate) throw();

	bool operator==(const COleDateTime& date) const throw();
	bool operator!=(const COleDateTime& date) const throw();
	bool operator<(const COleDateTime& date) const throw();
	bool operator>(const COleDateTime& date) const throw();
	bool operator<=(const COleDateTime& date) const throw();
	bool operator>=(const COleDateTime& date) const throw();

	// DateTime math
	COleDateTime operator+(COleDateTimeSpan dateSpan) const throw();
	COleDateTime operator-(COleDateTimeSpan dateSpan) const throw();
	COleDateTime& operator+=(COleDateTimeSpan dateSpan) throw();
	COleDateTime& operator-=(COleDateTimeSpan dateSpan) throw();

	// DateTimeSpan math
	COleDateTimeSpan operator-(const COleDateTime& date) const throw();

	operator DATE() const throw();

	int SetDateTime(int nYear, int nMonth, int nDay,
		int nHour, int nMin, int nSec) throw();
	int SetDate(int nYear, int nMonth, int nDay) throw();
	int SetTime(int nHour, int nMin, int nSec) throw();
	bool ParseDateTime(LPCTSTR lpszDate, DWORD dwFlags = 0,
		LCID lcid = LANG_USER_DEFAULT) throw();

	// formatting
	CString Format(DWORD dwFlags = 0, LCID lcid = LANG_USER_DEFAULT) const;
	CString Format(LPCTSTR lpszFormat) const;
	CString Format(UINT nFormatID) const;

protected:
	static double WINAPI DoubleFromDate( DATE date ) throw();
	static DATE WINAPI DateFromDouble( double f ) throw();

	void CheckRange();	
	BOOL ConvertSystemTimeToVariantTime(const SYSTEMTIME& systimeSrc);
};

}	// namespace ATL
 

 

#ifndef _DEBUG
#define ATLCOMTIME_INLINE inline
#include <atlcomtime.inl>
#endif


 
namespace ATL
{

inline bool COleDateTime::ParseDateTime(LPCTSTR lpszDate, DWORD dwFlags, LCID lcid) throw()
{
	USES_CONVERSION_EX;
	LPCTSTR pszDate = ( lpszDate == NULL ) ? _T("") : lpszDate;

	HRESULT hr;
	LPOLESTR p = T2OLE_EX((LPTSTR)pszDate, _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
#ifndef _UNICODE
	if( p == NULL )
	{
		m_dt = 0;
		m_status = invalid;
		return false;
	}
#endif // _UNICODE

	if (FAILED(hr = VarDateFromStr( p, lcid, dwFlags, &m_dt )))
	{
		if (hr == DISP_E_TYPEMISMATCH)
		{
			// Can't convert string to date, set 0 and invalidate
			m_dt = 0;
			m_status = invalid;
			return false;
		}
		else if (hr == DISP_E_OVERFLOW)
		{
			// Can't convert string to date, set -1 and invalidate
			m_dt = -1;
			m_status = invalid;
			return false;
		}
		else
		{
			ATLTRACE(atlTraceTime, 0, _T("\nCOleDateTime VarDateFromStr call failed.\n\t"));
			// Can't convert string to date, set -1 and invalidate
			m_dt = -1;
			m_status = invalid;
			return false;
		}
	}

	m_status = valid;
	return true;
}

inline CString COleDateTimeSpan::Format(LPCTSTR pFormat) const
{
	// If null, return empty string
	if (GetStatus() == null)
		return _T("");

	CTimeSpan tmp(GetDays(), GetHours(), GetMinutes(), GetSeconds());
	return tmp.Format(pFormat);
}

inline CString COleDateTimeSpan::Format(UINT nFormatID) const
{
	CString strFormat;
	if (!strFormat.LoadString(nFormatID))
		AtlThrow(E_INVALIDARG);
	return Format(strFormat);
}

inline CString COleDateTime::Format(DWORD dwFlags, LCID lcid) const
{
	// If null, return empty string
	if (GetStatus() == null)
		return _T("");

	// If invalid, return DateTime global string
	if (GetStatus() == invalid)
	{
		CString str;
		if(str.LoadString(ATL_IDS_DATETIME_INVALID))
			return str;
		return szInvalidDateTime;
	}

	CComBSTR bstr;
	if (FAILED(::VarBstrFromDate(m_dt, lcid, dwFlags, &bstr)))
	{
		CString str;
		if(str.LoadString(ATL_IDS_DATETIME_INVALID))
			return str;
		return szInvalidDateTime;
	}

	CString tmp = CString(bstr);
	return tmp;
}

inline CString COleDateTime::Format(LPCTSTR pFormat) const
{
	ATLENSURE_THROW(pFormat != NULL, E_INVALIDARG);
	
	// If null, return empty string
	if(GetStatus() == null)
		return _T("");

	// If invalid, return DateTime global string
	if(GetStatus() == invalid)
	{
		CString str;
		if(str.LoadString(ATL_IDS_DATETIME_INVALID))
			return str;
		return szInvalidDateTime;
	}

	UDATE ud;
	if (S_OK != VarUdateFromDate(m_dt, 0, &ud))
	{
		CString str;
		if(str.LoadString(ATL_IDS_DATETIME_INVALID))
			return str;
		return szInvalidDateTime;
	}

	struct tm tmTemp;
	tmTemp.tm_sec	= ud.st.wSecond;
	tmTemp.tm_min	= ud.st.wMinute;
	tmTemp.tm_hour	= ud.st.wHour;
	tmTemp.tm_mday	= ud.st.wDay;
	tmTemp.tm_mon	= ud.st.wMonth - 1;
	tmTemp.tm_year	= ud.st.wYear - 1900;
	tmTemp.tm_wday	= ud.st.wDayOfWeek;
	tmTemp.tm_yday	= ud.wDayOfYear - 1;
	tmTemp.tm_isdst	= 0;

	CString strDate;
	LPTSTR lpszTemp = strDate.GetBufferSetLength(256);
	_tcsftime(lpszTemp, strDate.GetLength(), pFormat, &tmTemp);
	strDate.ReleaseBuffer();

	return strDate;
}

inline CString COleDateTime::Format(UINT nFormatID) const
{
	CString strFormat;
	ATLENSURE(strFormat.LoadString(nFormatID));
	return Format(strFormat);
}

#ifdef __oledb_h__
inline COleDateTime::COleDateTime(const DBTIMESTAMP& dbts)
{
	SYSTEMTIME st;
	::ZeroMemory(&st, sizeof(SYSTEMTIME));

	st.wYear = WORD(dbts.year);
	st.wMonth = WORD(dbts.month);
	st.wDay = WORD(dbts.day);
	st.wHour = WORD(dbts.hour);
	st.wMinute = WORD(dbts.minute);
	st.wSecond = WORD(dbts.second);

	m_status = ::SystemTimeToVariantTime(&st, &m_dt) ? valid : invalid;
}

inline bool COleDateTime::GetAsDBTIMESTAMP(DBTIMESTAMP& dbts) const
{
	UDATE ud;
	if (S_OK != VarUdateFromDate(m_dt, 0, &ud))
		return false;

	dbts.year = (SHORT) ud.st.wYear;
	dbts.month = (USHORT) ud.st.wMonth;
	dbts.day = (USHORT) ud.st.wDay;
	dbts.hour = (USHORT) ud.st.wHour;
	dbts.minute = (USHORT) ud.st.wMinute;
	dbts.second = (USHORT) ud.st.wSecond;
	dbts.fraction = 0;

	return true;
}
#endif

}	// namespace ATL
#pragma pack(pop)

#pragma warning(pop)

#endif	// __ATLCOMTIME_H__
