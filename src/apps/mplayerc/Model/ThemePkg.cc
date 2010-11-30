#include "StdAfx.h"
#include "ThemePkg.h"

bool ThemePkg::ReadThemeFromDir(std::wstring input_dir)
{
  m_theme_is_dir = true;
  // note, this function should cache the entire description
  // of theme related files into internal buffer
  return false;
}

bool ThemePkg::ReadThemeFromPkg(std::wstring pkg_filename)
{
  m_theme_is_dir = false;
  // note, this function should cache the entire description
  // of theme related files into internal buffer
  return false;
}

bool ThemePkg::WriteThemeToDir(std::wstring output_dir, 
                               std::vector<std::wstring>* output_filelist /* = NULL */)
{
  // based on |m_theme_is_dir| and the above description of "read"
  // methods, write internal buffers to corresponding dir
  return false;
}

bool ThemePkg::WriteThemeToPkg(std::wstring pkg_filename)
{
  // based on |m_theme_is_dir| and the above description of "read"
  // methods, write internal buffers to corresponding file format
  return false;
}
