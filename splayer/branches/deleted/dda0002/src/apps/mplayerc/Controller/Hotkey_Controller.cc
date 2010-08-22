#include "StdAfx.h"
#include "../Model/HotkeySchemeParser.h"
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

void HotkeyController::EnumAvailableSchemes(std::vector<std::wstring>& files_out, 
                                            std::vector<std::wstring>& names_out)
{
  files_out.resize(0);
  names_out.resize(0);
  // Retrieve .exe path, and scheme files
  // For this feature we use Windows API programming because there aren't
  // real C/C++ way to enumerate files for cross-platform, other than
  // boost of course.
  wchar_t exe_path[256];
  GetModuleFileName(NULL, exe_path, 256);
  PathRemoveFileSpec(exe_path);
  
  wcscat_s(exe_path, 256, L"\\hotkey\\");

  std::wstring basedir = exe_path;
  wcscat_s(exe_path, 256, L"*.key");

  WIN32_FIND_DATA fd;
  HANDLE hFind = ::FindFirstFile(exe_path, &fd);
  if (hFind != INVALID_HANDLE_VALUE) 
  {
    std::wstring schemefile;
    schemefile = basedir;
    schemefile += fd.cFileName;
    files_out.push_back(schemefile);
    while (::FindNextFile(hFind, &fd) != 0)
    {
      schemefile = basedir;
      schemefile += fd.cFileName;
      files_out.push_back(schemefile);
    }
  }
  ::FindClose(hFind);

  //////////////////////////////////////////////////////////////////////////

  for (std::vector<std::wstring>::iterator it = files_out.begin(); it != files_out.end(); it++)
  {
    HotkeySchemeParser parser;
    parser.ReadFromFile((*it).c_str());
    names_out.push_back(parser.GetSchemeName());
  }
}