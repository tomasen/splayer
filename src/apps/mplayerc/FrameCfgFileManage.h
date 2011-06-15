#pragma once

class FrameCfgFileManage
{
public:
  FrameCfgFileManage(void);
  ~FrameCfgFileManage(void);

  void SetCfgFilePath(std::wstring path);

  void ReadFromCfgFile();

  void ParseCfgString(std::wstring cfgstr);
  int  ParseStrToInt(std::wstring str);

  std::wstring m_filepath;
  static int m_framecornerwidth;
  static int m_framecornerheight;
  static int m_lframethickheight;
  static int m_tframethickwidth;
  static int m_rframethickheight;
  static int m_bframethickwidth;
  static int m_captionheight;
  static int m_lcaptionthickwidth;
  static int m_rcaptionthickwidth;
};
