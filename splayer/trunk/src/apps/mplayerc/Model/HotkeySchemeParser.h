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
  std::wstring GetCmdNameByCmdCode(unsigned int cmdcode);
  unsigned int GetCmdCodeByCmdName(std::wstring cmdname);
  std::wstring GetKeyNameByKeyCode(unsigned int keycode);
  unsigned int GetKeyCodeByKeyName(std::wstring keyname);
  std::wstring GetMouseNameByMouseCode(unsigned int mousecode);
  unsigned int GetMouseCodeByMouseName(std::wstring mousename);
  std::wstring GetVirtNameByVirtCode(unsigned int virtcode);
  unsigned int GetVirtCodeByVirtName(std::wstring virtname);
  std::wstring GetAppCmdNameByAppCmdCode(unsigned int appcmdcode);
  unsigned int GetAppCmdCodeByAppCmdName(std::wstring appcmdname);

private:
  std::vector<HotkeyCmd>  m_list;
  std::wstring            m_schemename;
};

#endif // HOTKEYSCHEMEPARSER_H