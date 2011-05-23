#include "stdafx.h"
#include "MediaSQLite.h"

sqlitepp::session g_dbMediaSQLite;
CriticalSection g_csMediaSQLite;