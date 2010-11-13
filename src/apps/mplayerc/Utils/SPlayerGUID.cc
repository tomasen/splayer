
#include "stdafx.h"
#include "SPlayerGUID.h"
#include <RpcDce.h>

// Return the string length is 13, The first element is flag bit
// The bit is 0 if UUID struct last six char non-zero, otherwise 1

int SPlayerGUID::GenerateGUID(std::wstring& splayerguid)
{
  UUID splayeruuid;

  splayerguid = L"";

  if (UuidCreateSequential(&splayeruuid) != RPC_S_OK)
    return 1;

  std::wstring tempstr;
  wchar_t str[12];

  // UUID struct last six char
  for (int i=2;i<8;i++)
  {
    swprintf(str, L"%02X", splayeruuid.Data4[i]);
    tempstr += str;
  }

  if (tempstr != L"000000000000")
    splayerguid = L"0";
  else
  {
    splayerguid = L"1";
    tempstr = L"";

    // splayeruuid.Data1 type unsigned long
    swprintf(str, L"%08X", splayeruuid.Data1);
    tempstr += str;

    // splayeruuid.Data2 type unsigned short
    swprintf(str, L"%04X", splayeruuid.Data2);
    tempstr += str;
  }

  splayerguid += tempstr;

  return 0;
}