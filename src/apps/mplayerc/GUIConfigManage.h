#ifndef CONFIGBUTTONFILE_H
#define CONFIGBUTTONFILE_H

class GUIConfigManage
{
public:
  GUIConfigManage(void);
  ~GUIConfigManage(void);

  void ReadFromFile();
  void WriteToFile();
  void SetCfgFilePath(std::wstring s);
  std::wstring GetCfgFilePath();
  BOOL IsFileExist();
  std::vector<std::wstring>& GetCfgString();
  void SetCfgString(const std::vector<std::wstring>& vec);

private:
  std::vector<std::wstring> m_cfgfilestr_vec;
  BOOL m_breadfromfile;
  std::wstring m_filepath;
};

#endif
