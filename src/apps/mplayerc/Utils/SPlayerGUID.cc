
#include "stdafx.h"
#include "SPlayerGUID.h"
#include <RpcDce.h>
#include <time.h>
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

    if (hr != RPC_S_OK && hr != RPC_S_UUID_LOCAL_ONLY)
      uuidstring = RandMakeGUID();
    else if (UuidToString(&splayeruuid, &uuidstr) == RPC_S_OK)
    {
      // RPC_WSTR is unsigned short*.  wchar_t is a built-in type of Visual C++,
      // so the type cast is necessary.
      uuidstring.assign(reinterpret_cast<wchar_t*>(uuidstr));
      RpcStringFree(&uuidstr);
    }
    else
      uuidstring = RandMakeGUID();

    cfgdb->WriteProfileString(L"SPlayerGUID", L"GenerateGUID", uuidstring.c_str());
  }
  else
    uuidstring = cfguuid;

  delete cfgdb;
  return 0;
}

std::wstring SPlayerGUID::RandMakeGUID()
{
  int i;
  std::wstring newuuid, uuidchar;

  uuidchar = L"0123456789abcdefghijklmnopqrstuvwxyz";
  srand((unsigned)time(NULL));

  for (i = 0; i < 8; i++)
    newuuid.push_back(uuidchar.at(rand()%36));

  newuuid.push_back(L'-');

  for (int j = 0; j < 3; j++)
  {
    for (i = 0; i < 4; i++)
      newuuid.push_back(uuidchar.at(rand()%36));
    newuuid.push_back(L'-');
  }

  for (i = 0; i < 12; i++)
    newuuid.push_back(uuidchar.at(rand()%36));

  return newuuid;
}