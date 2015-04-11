
#include "stdafx.h"
#include "SPlayerGUID.h"
#include <RpcDce.h>
#include <time.h>
#include "../Model/appSQLlite.h"
#include "logging.h"
#include "pcid/pcid/PCIDCalculator.h"
#include "../mplayerc.h"

static std::wstring uuid_cache;

int SPlayerGUID::GenerateGUID(std::wstring& uuidstring)
{
  if (!uuid_cache.empty())
    uuidstring = uuid_cache;
  else
  {

    UUID splayeruuid;
    RPC_WSTR uuidstr;
    std::wstring dbpath;
    
    SQLliteapp* cfgdb = AfxGetMyApp()->sqlite_local_record;
    CString cfguuid;
    if (cfgdb)
      cfguuid= cfgdb->GetProfileString(L"SPlayerGUID", L"GenerateGUID");
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

      uuidstring += L"_" + GetComputerID();
      if (cfgdb)
        cfgdb->WriteProfileString(L"SPlayerGUID", L"GenerateGUID", uuidstring.c_str());
    }
    else
      uuidstring = cfguuid;

    Logging(L"uuid %s", uuidstring.c_str());
    uuid_cache = uuidstring;
  }
  
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

std::wstring SPlayerGUID::GetComputerName(){  std::wstring sRet;  DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;  wchar_t szComputerName[MAX_COMPUTERNAME_LENGTH + 1 + 1] = {0};  ::GetComputerName(szComputerName, &dwSize);  sRet = szComputerName;  std::transform(sRet.begin(), sRet.end(), sRet.begin(), towupper);  return sRet;}std::wstring SPlayerGUID::GetComputerID(){  char buf[16];  if (!GetPCID(buf, PCID_ALL))    return L"";  wchar_t ret[128] = {0};
  unsigned long* p = (unsigned long*)buf;
  swprintf_s(ret, 128, L"%0.8X%0.8X%0.8X%0.8X", p[0], p[1], p[2], p[3]);  return ret;}std::wstring SPlayerGUID::GetUserName()
{
  std::wstring sRet;

  DWORD dwSize = UNLEN + 1;
  wchar_t szUserName[UNLEN + 1 + 1] = {0};
  ::GetUserName(szUserName, &dwSize);

  sRet = szUserName;

  return sRet;
}
