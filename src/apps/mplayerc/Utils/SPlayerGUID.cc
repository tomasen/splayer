
#include "stdafx.h"
#include "SPlayerGUID.h"
#include <RpcDce.h>
#include "../Model/appSQLlite.h"
#include "logging.h"

int SPlayerGUID::GenerateGUID(std::wstring& uuidstring)
{
  UUID splayeruuid;
  RPC_WSTR uuidstr;
  std::wstring dbpath;
  wchar_t apppath[MAX_PATH];

  ::GetEnvironmentVariable(L"APPDATA", apppath, MAX_PATH);
  dbpath = apppath;
  dbpath += L"\\SPlayer\\local.db";

  SQLliteapp* cfgdb = new SQLliteapp(dbpath);
  CString cfguuid = cfgdb->GetProfileString(L"SPlayerGUID", L"GenerateGUID");
  if (cfguuid.IsEmpty())
  {
    RPC_STATUS hr = UuidCreateSequential(&splayeruuid);

    if (hr != RPC_S_OK || hr == RPC_S_UUID_LOCAL_ONLY)
      return 1;

    if (UuidToString(&splayeruuid, &uuidstr) != RPC_S_OK)
      return 1;

    // RPC_WSTR is unsigned short*.  wchar_t is a built-in type of Visual C++,
    // so the type cast is necessary.
    uuidstring.assign(reinterpret_cast<wchar_t*>(uuidstr));
    RpcStringFree(&uuidstr);
    cfgdb->WriteProfileString(L"SPlayerGUID", L"GenerateGUID", uuidstring.c_str());
  }
  else
    uuidstring = cfguuid;

  delete cfgdb;
  return 0;
}