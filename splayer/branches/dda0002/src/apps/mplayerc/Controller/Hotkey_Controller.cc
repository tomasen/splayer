#include "StdAfx.h"
#include "HotkeySchemeParser.h"
#include "Hotkey_Controller.h"

HotkeyController::HotkeyController(void)
{
  HotkeySchemeParser parser;
  parser.PopulateDefaultScheme();
  m_schemes = parser.GetScheme();
  m_schemename = parser.GetSchemeName();
}

bool HotkeyController::UpdateSchemeFromFile(const wchar_t* filename)
{
  HotkeySchemeParser parser;
  if (parser.ReadFromFile(filename))
  {
    m_schemename = parser.GetSchemeName();
    m_schemes = parser.GetScheme();
    return true;
  }
  return false;
}

std::wstring HotkeyController::GetSchemeName()
{
  return m_schemename;
}

std::vector<HotkeyCmd> HotkeyController::GetScheme()
{
  return m_schemes;
}

HotkeyCmd HotkeyController::GetHotkeyCmdById(unsigned short cmd_id, int* index_out)
{
  HotkeyCmd ret;
  for (std::vector<HotkeyCmd>::iterator it = m_schemes.begin(); it != m_schemes.end(); it++)
  {
    if (it->cmd == cmd_id)
    {
      if (index_out)
        *index_out = std::distance(m_schemes.begin(), it);
      return *it;
    }
  }
  if (index_out)
    *index_out = -1;
  return ret;
}

HotkeyCmd HotkeyController::GetHotkeyCmdByMouse(unsigned int mouse)
{
  HotkeyCmd ret;
  for (std::vector<HotkeyCmd>::iterator it = m_schemes.begin(); it != m_schemes.end(); it++)
  {
    if (it->mouse == mouse)
      return *it;
  }
  return ret;
}