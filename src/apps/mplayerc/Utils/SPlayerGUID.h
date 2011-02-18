
#pragma once

class SPlayerGUID
{
public:
  static int GenerateGUID(std::wstring& uuidstring);
  static std::wstring RandMakeGUID();
};