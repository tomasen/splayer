#include "StdAfx.h"
#include "ThemePkg.h"

bool ThemePkg::ReadThemeFromDir(std::wstring input_dir)
{
  m_theme_is_dir = true;
  // note, this function should be memory efficient
  // it should only enumerate files and keep them in |m_file_list|,
  // then maintain corresponding |m_base_dir| to record |input_dir|.
  // after that, generate a thumbnail for use with front-end
  // DO NOT STORE FILE CONTENTS IN INTERNAL BUFFER
  return false;
}

bool ThemePkg::ReadThemeFromPkg(std::wstring pkg_filename)
{
  m_theme_is_dir = false;
  // note, this function should be memory efficient
  // it should only enumerate files and keep them in |m_file_list|,
  // then maintain corresponding |m_pkg_filename| to record |pkg_filename|.
  // after that, generate a thumbnail for use with front-end
  // DO NOT STORE FILE CONTENTS IN INTERNAL BUFFER
  return false;
}

bool ThemePkg::WriteThemeToDir(std::wstring output_dir, std::vector<std::wstring>& output_filelist)
{
  // since contents are not cached during the previous read stage
  // this function should determine how to read real file contents
  // based on |m_theme_is_dir| and the above description of "read"
  // methods
  return false;
}

bool ThemePkg::WriteThemeToPkg(std::wstring pkg_filename)
{
  // since contents are not cached during the previous read stage
  // this function should determine how to read real file contents
  // based on |m_theme_is_dir| and the above description of "read"
  // methods
  return false;
}

std::vector<std::wstring> ThemePkg::GetFileList()
{
  return std::vector<std::wstring>();
}

std::wstring ThemePkg::GetBaseDir()
{
  return L"";
}

std::wstring ThemePkg::GetPkgFileName()
{
  return L"";
}

WTL::CBitmap ThemePkg::GetThumbnail()
{
  return WTL::CBitmap();
}