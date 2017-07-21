// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

#ifndef __ATLTIME_INL__
#define __ATLTIME_INL__

#pragma once

#ifndef __ATLTIME_H__
	#error atltime.inl requires atltime.h to be included first
#endif


namespace ATL
{
/////////////////////////////////////////////////////////////////////////////
// CTimeSpan
/////////////////////////////////////////////////////////////////////////////

ATLTIME_INLINE CTimeSpan::CTimeSpan() throw() :
	m_timeSpan(0)
{
}

ATLTIME_INLINE CTimeSpan::CTimeSpan( __time64_t time ) throw() :
	m_timeSpan( time )
{
}

ATLTIME_INLINE CTimeSpan::CTimeSpan(LONG lDays, int nHours, int nMins, int nSecs) throw()
{
 	m_timeSpan = nSecs + 60* (nMins + 60* (nHours + __int64(24) * lDays));
}

ATLTIME_INLINE LONGLONG CTimeSpan::GetDays() const throw()
{
	return( m_timeSpan/(24*3600) );
}

ATLTIME_INLINE LONGLONG CTimeSpan::GetTotalHours() const throw()
{
	return( m_timeSpan/3600 );
}

ATLTIME_INLINE LONG CTimeSpan::GetHours() const throw()
{
	return( LONG( GetTotalHours()-(GetDays()*24) ) );
}

ATLTIME_INLINE LONGLONG CTimeSpan::GetTotalMinutes() const throw()
{
	return( m_timeSpan/60 );
}

ATLTIME_INLINE LONG CTimeSpan::GetMinutes() const throw()
{
	return( LONG( GetTotalMinutes()-(GetTotalHours()*60) ) );
}

ATLTIME_INLINE LONGLONG CTimeSpan::GetTotalSeconds() const throw()
{
	return( m_timeSpan );
}

ATLTIME_INLINE LONG CTimeSpan::GetSeconds() const throw()
{
	return( LONG( GetTotalSeconds()-(GetTotalMinutes()*60) ) );
}

ATLTIME_INLINE __time64_t CTimeSpan::GetTimeSpan() const throw()
{
	return( m_timeSpan );
}

ATLTIME_INLINE CTimeSpan CTimeSpan::operator+( CTimeSpan span ) const throw()
{
	return( CTimeSpan( m_timeSpan+span.m_timeSpan ) );
}

ATLTIME_INLINE CTimeSpan CTimeSpan::operator-( CTimeSpan span ) const throw()
{
	return( CTimeSpan( m_timeSpan-span.m_timeSpan ) );
}

ATLTIME_INLINE CTimeSpan& CTimeSpan::operator+=( CTimeSpan span ) throw()
{
	m_timeSpan += span.m_timeSpan;
	return( *this );
}

ATLTIME_INLINE CTimeSpan& CTimeSpan::operator-=( CTimeSpan span ) throw()
{
	m_timeSpan -= span.m_timeSpan;
	return( *this );
}

ATLTIME_INLINE bool CTimeSpan::operator==( CTimeSpan span ) const throw()
{
	return( m_timeSpan == span.m_timeSpan );
}

ATLTIME_INLINE bool CTimeSpan::operator!=( CTimeSpan span ) const throw()
{
	return( m_timeSpan != span.m_timeSpan );
}

ATLTIME_INLINE bool CTimeSpan::operator<( CTimeSpan span ) const throw()
{
	return( m_timeSpan < span.m_timeSpan );
}

ATLTIME_INLINE bool CTimeSpan::operator>( CTimeSpan span ) const throw()
{
	return( m_timeSpan > span.m_timeSpan );
}

ATLTIME_INLINE bool CTimeSpan::operator<=( CTimeSpan span ) const throw()
{
	return( m_timeSpan <= span.m_timeSpan );
}

ATLTIME_INLINE bool CTimeSpan::operator>=( CTimeSpan span ) const throw()
{
	return( m_timeSpan >= span.m_timeSpan );
}

/////////////////////////////////////////////////////////////////////////////
// CTime
/////////////////////////////////////////////////////////////////////////////

ATLTIME_INLINE CTime WINAPI CTime::GetCurrentTime() throw()
{
	return( CTime( ::_time64( NULL ) ) );
}

ATLTIME_INLINE BOOL WINAPI CTime::IsValidFILETIME(const FILETIME& fileTime) throw()
{
	FILETIME localTime;
	if (!FileTimeToLocalFileTime(&fileTime, &localTime))
	{
		return FALSE;
	}

	// then convert that time to system time
	SYSTEMTIME sysTime;
	if (!FileTimeToSystemTime(&localTime, &sysTime))
	{
		return FALSE;
	}

	return TRUE;
}

ATLTIME_INLINE CTime::CTime() throw() :
	m_time(0)
{
}

ATLTIME_INLINE CTime::CTime( __time64_t time )  throw():
	m_time( time )
{
}

ATLTIME_INLINE CTime::CTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec,
	int nDST)
{
#pragma warning (push)
#pragma warning (disable: 4127)  // conditional expression constant

	ATLENSURE( nYear >= 1900 );
	ATLENSURE( nMonth >= 1 && nMonth <= 12 );
	ATLENSURE( nDay >= 1 && nDay <= 31 );
	ATLENSURE( nHour >= 0 && nHour <= 23 );
	ATLENSURE( nMin >= 0 && nMin <= 59 );
	ATLENSURE( nSec >= 0 && nSec <= 59 );

#pragma warning (pop)

	struct tm atm;

	atm.tm_sec = nSec;
	atm.tm_min = nMin;
	atm.tm_hour = nHour;
	atm.tm_mday = nDay;
	atm.tm_mon = nMonth - 1;        // tm_mon is 0 based
	atm.tm_year = nYear - 1900;     // tm_year is 1900 based
	atm.tm_isdst = nDST;

	m_time = _mktime64(&atm);
	ATLASSUME(m_time != -1);       // indicates an illegal input time
	if(m_time == -1)
	{
		AtlThrow(E_INVALIDARG);
	}
}

ATLTIME_INLINE CTime::CTime(WORD wDosDate, WORD wDosTime, int nDST)
{
	struct tm atm;
	atm.tm_sec = (wDosTime & ~0xFFE0) << 1;
	atm.tm_min = (wDosTime & ~0xF800) >> 5;
	atm.tm_hour = wDosTime >> 11;

	atm.tm_mday = wDosDate & ~0xFFE0;
	atm.tm_mon = ((wDosDate & ~0xFE00) >> 5) - 1;
	atm.tm_year = (wDosDate >> 9) + 80;
	atm.tm_isdst = nDST;
	m_time = _mktime64(&atm);
	ATLASSUME(m_time != -1);       // indicates an illegal input time

	if(m_time == -1)
		AtlThrow(E_INVALIDARG);

}

ATLTIME_INLINE CTime::CTime(const SYSTEMTIME& sysTime, int nDST)
{
	if (sysTime.wYear < 1900)
	{
		__time64_t time0 = 0L;
		CTime timeT(time0);
		*this = timeT;
	}
	else
	{
		CTime timeT(
			(int)sysTime.wYear, (int)sysTime.wMonth, (int)sysTime.wDay,
			(int)sysTime.wHour, (int)sysTime.wMinute, (int)sysTime.wSecond,
			nDST);
		*this = timeT;
	}
}

ATLTIME_INLINE CTime::CTime(const FILETIME& fileTime, int nDST)
{
	// first convert file time (UTC time) to local time
	FILETIME localTime;
	if (!FileTimeToLocalFileTime(&fileTime, &localTime))
	{
		m_time = 0;
		AtlThrow(E_INVALIDARG);
		return;
	}

	// then convert that time to system time
	SYSTEMTIME sysTime;
	if (!FileTimeToSystemTime(&localTime, &sysTime))
	{
		m_time = 0;
		AtlThrow(E_INVALIDARG);		
		return;
	}

	// then convert the system time to a time_t (C-runtime local time)
	CTime timeT(sysTime, nDST);
	*this = timeT;
}

ATLTIME_INLINE CTime& CTime::operator=( __time64_t time ) throw()
{
	m_time = time;

	return( *this );
}

ATLTIME_INLINE CTime& CTime::operator+=( CTimeSpan span ) throw()
{
	m_time += span.GetTimeSpan();

	return( *this );
}

ATLTIME_INLINE CTime& CTime::operator-=( CTimeSpan span ) throw()
{
	m_time -= span.GetTimeSpan();

	return( *this );
}

ATLTIME_INLINE CTimeSpan CTime::operator-( CTime time ) const throw()
{
	return( CTimeSpan( m_time-time.m_time ) );
}

ATLTIME_INLINE CTime CTime::operator-( CTimeSpan span ) const throw()
{
	return( CTime( m_time-span.GetTimeSpan() ) );
}

ATLTIME_INLINE CTime CTime::operator+( CTimeSpan span ) const throw()
{
	return( CTime( m_time+span.GetTimeSpan() ) );
}

ATLTIME_INLINE bool CTime::operator==( CTime time ) const throw()
{
	return( m_time == time.m_time );
}

ATLTIME_INLINE bool CTime::operator!=( CTime time ) const throw()
{
	return( m_time != time.m_time );
}

ATLTIME_INLINE bool CTime::operator<( CTime time ) const throw()
{
	return( m_time < time.m_time );
}

ATLTIME_INLINE bool CTime::operator>( CTime time ) const throw()
{
	return( m_time > time.m_time );
}

ATLTIME_INLINE bool CTime::operator<=( CTime time ) const throw()
{
	return( m_time <= time.m_time );
}

ATLTIME_INLINE bool CTime::operator>=( CTime time ) const throw()
{
	return( m_time >= time.m_time );
}

ATLTIME_INLINE struct tm* CTime::GetGmtTm(struct tm* ptm) const
{
	// Ensure ptm is valid
	ATLENSURE( ptm != NULL );

	if (ptm != NULL)
	{
		struct tm ptmTemp;
		errno_t err = _gmtime64_s(&ptmTemp, &m_time);

		// Be sure the call succeeded
		if(err != 0) { return NULL; }

		*ptm = ptmTemp;
		return ptm;
	}

	return NULL;
}

ATLTIME_INLINE struct tm* CTime::GetLocalTm(struct tm* ptm) const
{
	// Ensure ptm is valid
	ATLENSURE( ptm != NULL );

	if (ptm != NULL)
	{
		struct tm ptmTemp;
		errno_t err = _localtime64_s(&ptmTemp, &m_time);

		if (err != 0)
		{
			return NULL;    // indicates that m_time was not initialized!
		}

		*ptm = ptmTemp;
		return ptm;
	}

	return NULL;
}

ATLTIME_INLINE bool CTime::GetAsSystemTime(SYSTEMTIME& timeDest) const throw()
{
	struct tm ttm;
	struct tm* ptm;

	ptm = GetLocalTm(&ttm);

	if(!ptm) { return false; }

	timeDest.wYear = (WORD) (1900 + ptm->tm_year);
	timeDest.wMonth = (WORD) (1 + ptm->tm_mon);
	timeDest.wDayOfWeek = (WORD) ptm->tm_wday;
	timeDest.wDay = (WORD) ptm->tm_mday;
	timeDest.wHour = (WORD) ptm->tm_hour;
	timeDest.wMinute = (WORD) ptm->tm_min;
	timeDest.wSecond = (WORD) ptm->tm_sec;
	timeDest.wMilliseconds = 0;

	return true;
}

ATLTIME_INLINE __time64_t CTime::GetTime() const throw()
{
	return( m_time );
}

ATLTIME_INLINE int CTime::GetYear() const
{ 
	struct tm ttm;
	struct tm * ptm;

	ptm = GetLocalTm(&ttm);
	return ptm ? (ptm->tm_year) + 1900 : 0 ; 
}

ATLTIME_INLINE int CTime::GetMonth() const
{ 
	struct tm ttm;
	struct tm * ptm;

	ptm = GetLocalTm(&ttm);
	return ptm ? ptm->tm_mon + 1 : 0;
}

ATLTIME_INLINE int CTime::GetDay() const
{
	struct tm ttm;
	struct tm * ptm;

	ptm = GetLocalTm(&ttm);
	return ptm ? ptm->tm_mday : 0 ; 
}

ATLTIME_INLINE int CTime::GetHour() const
{
	struct tm ttm;
	struct tm * ptm;

	ptm = GetLocalTm(&ttm);
	return ptm ? ptm->tm_hour : -1 ; 
}

ATLTIME_INLINE int CTime::GetMinute() const
{
	struct tm ttm;
	struct tm * ptm;

	ptm = GetLocalTm(&ttm);
	return ptm ? ptm->tm_min : -1 ; 
}

ATLTIME_INLINE int CTime::GetSecond() const
{ 
	struct tm ttm;
	struct tm * ptm;

	ptm = GetLocalTm(&ttm);
	return ptm ? ptm->tm_sec : -1 ;
}

ATLTIME_INLINE int CTime::GetDayOfWeek() const
{ 
	struct tm ttm;
	struct tm * ptm;

	ptm = GetLocalTm(&ttm);
	return ptm ? ptm->tm_wday + 1 : 0 ;
}

/////////////////////////////////////////////////////////////////////////////
// CFileTimeSpan
/////////////////////////////////////////////////////////////////////////////

ATLTIME_INLINE CFileTimeSpan::CFileTimeSpan() throw() :
	m_nSpan( 0 )
{
}

ATLTIME_INLINE CFileTimeSpan::CFileTimeSpan( const CFileTimeSpan& span ) throw() :
	m_nSpan( span.m_nSpan )
{
}

ATLTIME_INLINE CFileTimeSpan::CFileTimeSpan( LONGLONG nSpan ) throw() :
	m_nSpan( nSpan )
{
}

ATLTIME_INLINE CFileTimeSpan& CFileTimeSpan::operator=( const CFileTimeSpan& span ) throw()
{
	m_nSpan = span.m_nSpan;

	return( *this );
}

ATLTIME_INLINE CFileTimeSpan& CFileTimeSpan::operator+=( CFileTimeSpan span ) throw()
{
	m_nSpan += span.m_nSpan;

	return( *this );
}

ATLTIME_INLINE CFileTimeSpan& CFileTimeSpan::operator-=( CFileTimeSpan span ) throw()
{
	m_nSpan -= span.m_nSpan;

	return( *this );
}

ATLTIME_INLINE CFileTimeSpan CFileTimeSpan::operator+( CFileTimeSpan span ) const throw()
{
	return( CFileTimeSpan( m_nSpan+span.m_nSpan ) );
}

ATLTIME_INLINE CFileTimeSpan CFileTimeSpan::operator-( CFileTimeSpan span ) const throw()
{
	return( CFileTimeSpan( m_nSpan-span.m_nSpan ) );
}

ATLTIME_INLINE bool CFileTimeSpan::operator==( CFileTimeSpan span ) const throw()
{
	return( m_nSpan == span.m_nSpan );
}

ATLTIME_INLINE bool CFileTimeSpan::operator!=( CFileTimeSpan span ) const throw()
{
	return( m_nSpan != span.m_nSpan );
}

ATLTIME_INLINE bool CFileTimeSpan::operator<( CFileTimeSpan span ) const throw()
{
	return( m_nSpan < span.m_nSpan );
}

ATLTIME_INLINE bool CFileTimeSpan::operator>( CFileTimeSpan span ) const throw()
{
	return( m_nSpan > span.m_nSpan );
}

ATLTIME_INLINE bool CFileTimeSpan::operator<=( CFileTimeSpan span ) const throw()
{
	return( m_nSpan <= span.m_nSpan );
}

ATLTIME_INLINE bool CFileTimeSpan::operator>=( CFileTimeSpan span ) const throw()
{
	return( m_nSpan >= span.m_nSpan );
}

ATLTIME_INLINE LONGLONG CFileTimeSpan::GetTimeSpan() const throw()
{
	return( m_nSpan );
}

ATLTIME_INLINE void CFileTimeSpan::SetTimeSpan( LONGLONG nSpan ) throw()
{
	m_nSpan = nSpan;
}


/////////////////////////////////////////////////////////////////////////////
// CFileTime
/////////////////////////////////////////////////////////////////////////////

ATLTIME_INLINE CFileTime::CFileTime() throw()
{
	dwLowDateTime = 0;
	dwHighDateTime = 0;
}

ATLTIME_INLINE CFileTime::CFileTime( const FILETIME& ft ) throw()
{
	dwLowDateTime = ft.dwLowDateTime;
	dwHighDateTime = ft.dwHighDateTime;
}

ATLTIME_INLINE CFileTime::CFileTime( ULONGLONG nTime ) throw()
{
	dwLowDateTime = DWORD( nTime );
	dwHighDateTime = DWORD( nTime>>32 );
}

ATLTIME_INLINE CFileTime& CFileTime::operator=( const FILETIME& ft ) throw()
{
	dwLowDateTime = ft.dwLowDateTime;
	dwHighDateTime = ft.dwHighDateTime;

	return( *this );
}

ATLTIME_INLINE CFileTime WINAPI CFileTime::GetCurrentTime() throw()
{
	CFileTime ft;
	GetSystemTimeAsFileTime(&ft);
	return ft;
}

ATLTIME_INLINE CFileTime& CFileTime::operator+=( CFileTimeSpan span ) throw()
{
	SetTime( GetTime()+span.GetTimeSpan() );

	return( *this );
}

ATLTIME_INLINE CFileTime& CFileTime::operator-=( CFileTimeSpan span ) throw()
{
	SetTime( GetTime()-span.GetTimeSpan() );

	return( *this );
}

ATLTIME_INLINE CFileTime CFileTime::operator+( CFileTimeSpan span ) const throw()
{
	return( CFileTime( GetTime()+span.GetTimeSpan() ) );
}

ATLTIME_INLINE CFileTime CFileTime::operator-( CFileTimeSpan span ) const throw()
{
	return( CFileTime( GetTime()-span.GetTimeSpan() ) );
}

ATLTIME_INLINE CFileTimeSpan CFileTime::operator-( CFileTime ft ) const throw()
{
	return( CFileTimeSpan( GetTime()-ft.GetTime() ) );
}

ATLTIME_INLINE bool CFileTime::operator==( CFileTime ft ) const throw()
{
	return( GetTime() == ft.GetTime() );
}

ATLTIME_INLINE bool CFileTime::operator!=( CFileTime ft ) const throw()
{
	return( GetTime() != ft.GetTime() );
}

ATLTIME_INLINE bool CFileTime::operator<( CFileTime ft ) const throw()
{
	return( GetTime() < ft.GetTime() );
}

ATLTIME_INLINE bool CFileTime::operator>( CFileTime ft ) const throw()
{
	return( GetTime() > ft.GetTime() );
}

ATLTIME_INLINE bool CFileTime::operator<=( CFileTime ft ) const throw()
{
	return( GetTime() <= ft.GetTime() );
}

ATLTIME_INLINE bool CFileTime::operator>=( CFileTime ft ) const throw()
{
	return( GetTime() >= ft.GetTime() );
}

ATLTIME_INLINE ULONGLONG CFileTime::GetTime() const throw()
{
	return( (ULONGLONG( dwHighDateTime )<<32)|dwLowDateTime );
}

ATLTIME_INLINE void CFileTime::SetTime( ULONGLONG nTime ) throw()
{
	dwLowDateTime = DWORD( nTime );
	dwHighDateTime = DWORD( nTime>>32 );
}

ATLTIME_INLINE CFileTime CFileTime::UTCToLocal() const throw()
{
	CFileTime ftLocal;

	::FileTimeToLocalFileTime( this, &ftLocal );

	return( ftLocal );
}

ATLTIME_INLINE CFileTime CFileTime::LocalToUTC() const throw()
{
	CFileTime ftUTC;

	::LocalFileTimeToFileTime( this, &ftUTC );

	return( ftUTC );
}

}  // namespace ATL
#endif //__ATLTIME_INL__
