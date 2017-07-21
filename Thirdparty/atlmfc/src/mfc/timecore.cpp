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



#ifdef _DEBUG
CDumpContext& AFXAPI operator <<(CDumpContext& dc, CTime time)
{
	char psz[32];
	psz[0] = '\0';

	__time64_t tmp = time.GetTime();
	errno_t err = _ctime64_s(psz, sizeof(psz), &tmp);

	if ((err != 0) || (psz[0] == '\0') || (time.GetTime() == 0))
	{
		return dc << "CTime(invalid #" << time.GetTime() << ")";
	}

	// format it
	return dc << "CTime(\"" << psz << "\")";
}
#endif

CArchive& CTime::Serialize64(CArchive& ar)
{
	if (ar.IsStoring())
		return ar << (__int64) m_time;
	else
		return ar >> (__int64) m_time;
}
const __int32 cTime64Mark = INT_MIN + 10;
CArchive& AFXAPI operator <<(CArchive& ar, CTime time)
{
	// This version serializes 32 bits of invalid time and after 64 bits
	// time64. New code will support both 64 serialization and 32/64 deserialization.
	__time64_t ttime = time.GetTime();
	ar << (DWORD)cTime64Mark;	
	return ar << (__int64) ttime;
}

CArchive& AFXAPI operator >>(CArchive& ar, CTime& rtime)
{
	//his version  support both 32 and 64 deserialization.
	//If the first 32 bits are cTime64Mark (which is invalid time32 value), then
	//The next 64 bits are read to form time64 value, else, it must be a legacy (VC6-7.1) time32 value.
	__time32_t timeOld;
	CArchive& retRef = ar >> (DWORD&) timeOld;
	if (static_cast<__int32>(timeOld)==cTime64Mark)
	{
		__time64_t t64;
		ar >> (__int64) t64;
		rtime = t64;
	} else
	{
	rtime = __time64_t(timeOld);
	}
	return retRef;
}

/////////////////////////////////////////////////////////////////////////////
// CTimeSpan - relative time

#ifdef _DEBUG
CDumpContext& AFXAPI operator <<(CDumpContext& dc, CTimeSpan timeSpan)
{
	return dc << "CTimeSpan(" << timeSpan.GetDays() << " days, " <<
		 timeSpan.GetHours() << " hours, " <<
		 timeSpan.GetMinutes() << " minutes and " <<
		 timeSpan.GetSeconds() << " seconds)";
}
#endif

CArchive& CTimeSpan::Serialize64(CArchive& ar)
{
	if (ar.IsStoring())
		return ar << (__int64) m_timeSpan;
	else
		return ar >> (__int64) m_timeSpan;
}

CArchive& AFXAPI operator <<(CArchive& ar, CTimeSpan timeSpan)
{
	// this version serializes only 32 bits
	__time64_t timespan = timeSpan.GetTimeSpan();
	ASSERT( timespan <= UINT_MAX);
	if( timespan > UINT_MAX)
		AfxThrowInvalidArgException();
	time_t timeConverted = (time_t)timespan;
	return ar << (DWORD) timeConverted;
}

CArchive& AFXAPI operator >>(CArchive& ar, CTimeSpan& rtimeSpan)
{
	// this version serializes only 32 bits
	__time32_t timeOld;
	CArchive& retRef = ar >> (DWORD&) timeOld;
	rtimeSpan = __time64_t(timeOld);
	return retRef;
}
