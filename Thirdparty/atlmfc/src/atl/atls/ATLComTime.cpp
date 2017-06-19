// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the	
// Active Template Library product.

#include "StdAfx.H"

#ifdef _DEBUG
#define ATLCOMTIME_INLINE
#include <atlcomtime.inl>
#endif	// _DEBUG

#include <oledb.h>

namespace ATL
{
bool CTime::GetAsDBTIMESTAMP( DBTIMESTAMP& dbts ) const
{
	struct tm tmLocal;
	GetLocalTm(&tmLocal);

	dbts.year = (SHORT) (1900 + tmLocal.tm_year);
	dbts.month = (USHORT) (1 + tmLocal.tm_mon);
	dbts.day = (USHORT) tmLocal.tm_mday;
	dbts.hour = (USHORT) tmLocal.tm_hour;
	dbts.minute = (USHORT) tmLocal.tm_min;
	dbts.second = (USHORT) tmLocal.tm_sec;
	dbts.fraction = 0;

	return true;
}
}