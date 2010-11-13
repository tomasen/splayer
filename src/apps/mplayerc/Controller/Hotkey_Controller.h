#ifndef HOTKEY_CONTROLLER_H
#define HOTKEY_CONTROLLER_H

#include "../Model/HotkeyCmd.h"
#include "LazyInstance.h"

//////////////////////////////////////////////////////////////////////////
//
//  HotkeyController behave as the main controller that provides hotkey
//  list access, initialization, and scheme parsing capabilities.
//
class HotkeyController:
  public LazyInstanceImpl<HotkeyController>
{
public:
  HotkeyController(void);

  bool UpdateSchemeFromFile(const wchar_t* filename);
  std::wstring GetSchemeName();
  std::vector<HotkeyCmd> GetScheme();
  HotkeyCmd GetHotkeyCmdById(unsigned short cmd_id, int* index_out = NULL);
  HotkeyCmd GetHotkeyCmdByAppCmdId(unsigned short appcmd_id, int* index_out = NULL);
  HotkeyCmd GetHotkeyCmdByMouse(unsigned int mouse);
  void EnumAvailableSchemes(std::vector<std::wstring>& files_out, std::vector<std::wstring>& names_out);

private:
  std::vector<HotkeyCmd>  m_schemes;
  std::wstring            m_schemename;
};

#endif // HOTKEY_CONTROLLER_H