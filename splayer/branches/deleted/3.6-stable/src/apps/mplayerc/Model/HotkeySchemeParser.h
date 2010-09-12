#ifndef HOTKEYSCHEMEPARSER_H
#define HOTKEYSCHEMEPARSER_H

#include "HotkeyCmd.h"

//////////////////////////////////////////////////////////////////////////
//
//  HotkeySchemeParser parses and writes splayer's internal hotkey
//  scheme. 
//
//  Warning: This class is not thread-safe. The internal |m_list| and
//  |m_schemename| isn't locked during access.
//
class HotkeySchemeParser
{
public:
  // Fills |m_list| with default hotkey scheme settings (hard coded)
  void PopulateDefaultScheme();
  bool ReadFromFile(const wchar_t* filename);
  bool WriteToFile(const wchar_t* filename);
  std::wstring GetSchemeName();
  std::vector<HotkeyCmd> GetScheme();

private:
  std::vector<HotkeyCmd>  m_list;
  std::wstring            m_schemename;
};

#endif // HOTKEYSCHEMEPARSER_H