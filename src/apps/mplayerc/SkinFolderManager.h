#pragma once

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

  int m_count;
private:
  std::wstring UnSkinzip(std::wstring path);

  BOOL m_bfolderempty;
  //std::vector<CString> m_foldername;
  static std::map<std::wstring, std::wstring> m_skinnametobmp_map;
  std::wstring m_foldername;
  std::wstring m_skinbmp;
  static std::wstring m_path;
  
};
