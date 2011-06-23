
#pragma once

class SPlayerGUID
{
public:
  static int GenerateGUID(std::wstring& uuidstring);
  static std::wstring RandMakeGUID();

  static std::wstring GetComputerName();
  static std::wstring GetComputerID();
  static std::wstring GetUserName();
};