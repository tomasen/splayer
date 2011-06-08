#ifndef THEMEPKG_H
#define THEMEPKG_H

class ThemePkg
{
public:
  // utility function for building/unpacking a theme pkg file
  bool ReadThemeFromDir(std::wstring input_dir);
  bool ReadThemeFromPkg(std::wstring pkg_filename);
  bool WriteThemeToDir(std::wstring output_dir, std::vector<std::wstring>* output_filelist = NULL);
  bool WriteThemeToPkg(std::wstring pkg_filename);

private:
  std::vector<std::wstring> m_file_list;
  bool          m_theme_is_dir; // true if read from dir, false if read from pkg file
  std::wstring  m_base_dir;
  std::wstring  m_pkg_filename;
  WTL::CBitmap  m_thumb;
};

#endif // THEMEPKG_H