#pragma once

#include <utility>

class SkinFolderManager
{
public:
  SkinFolderManager(void);
  ~SkinFolderManager(void);
  
  void SeachFile(const wchar_t* lpath);

  int  GetFileCount();

  BOOL IsFolderEmpty();

  void SetSkinPath(CString lpFoldername);

  void DeleteFolder(LPCTSTR lpFolderName);

  void DeleteFile(LPCTSTR lpath);

  static std::map<std::wstring, std::wstring>& ReturnSkinMap();

  void ClearMap();
  static std::wstring GetSkinName(std::wstring tag, std::wstring language);
  static std::wstring RemoveSkinName(std::wstring skinname);
  int m_count;
private:
  std::wstring UnSkinzip(std::wstring path);
  void AddSkinName(std::wstring tag, std::wstring language, std::wstring name);

  BOOL m_bfolderempty;
  //std::vector<CString> m_foldername;
  static std::map<std::wstring, std::wstring> m_skinnametobmp_map;
  static std::map<std::wstring, std::pair<std::wstring, std::wstring> > m_skinnames;
  std::wstring m_foldername;
  std::wstring m_skinbmp;
  static std::wstring m_path;
  
};
