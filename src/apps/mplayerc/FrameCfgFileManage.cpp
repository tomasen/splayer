#include "stdafx.h"
#include "FrameCfgFileManage.h"
#include <fstream>

int FrameCfgFileManage::m_framecornerwidth = 4;
int FrameCfgFileManage::m_framecornerheight = 4;
int FrameCfgFileManage::m_lframethickheight = 1;
int FrameCfgFileManage::m_tframethickwidth = 1;
int FrameCfgFileManage::m_rframethickheight = 1;
int FrameCfgFileManage::m_bframethickwidth = 1;
int FrameCfgFileManage::m_captionheight = 31;
int FrameCfgFileManage::m_lcaptionthickwidth = 4;
int FrameCfgFileManage::m_rcaptionthickwidth = 4;

FrameCfgFileManage::FrameCfgFileManage(void)
{
}

FrameCfgFileManage::~FrameCfgFileManage(void)
{
}

void FrameCfgFileManage::SetCfgFilePath(std::wstring path)
{
  m_filepath = path;
}

void FrameCfgFileManage::ReadFromCfgFile()
{
  std::wstring cfgString;
  std::wifstream readFile;
  readFile.open(m_filepath.c_str(), std::wifstream::in);

  while(getline(readFile, cfgString))
  {
    if (cfgString[0] == L' ' || cfgString == L"")
      continue;
    ParseCfgString(cfgString);
  }

  readFile.close();
}

void FrameCfgFileManage::ParseCfgString(std::wstring cfgstr)
{
  int pos = cfgstr.find(L":");
  if (pos == std::wstring::npos)
    return;
  std::wstring substr = cfgstr.substr(0, pos);
  cfgstr = cfgstr.substr(pos + 1);

  if (substr == L"FRAMECORNERWIDTH")
    m_framecornerwidth = ParseStrToInt(cfgstr);
  if (substr == L"FRAMECORNERHEIGHT")
    m_framecornerheight = ParseStrToInt(cfgstr);
  if (substr == L"LFRAMETHICKHEIGHT")
    m_lframethickheight = ParseStrToInt(cfgstr);
  if (substr == L"TFRAMETHICKWIDTH")
    m_tframethickwidth = ParseStrToInt(cfgstr);
  if (substr == L"RFRAMETHICKHEIGHT")
    m_rframethickheight = ParseStrToInt(cfgstr);
  if (substr == L"BFRAMETHICKHEIGHT")
    m_bframethickwidth = ParseStrToInt(cfgstr);
  if (substr == L"CAPTIONHEIGHT")
    m_captionheight = ParseStrToInt(cfgstr);
  if (substr == L"LCAPTIONTHICKWIDTH")
    m_lcaptionthickwidth = ParseStrToInt(cfgstr);
  if (substr == L"RCAPTIONTHICKWIDTH")
    m_rcaptionthickwidth = ParseStrToInt(cfgstr);
}

int  FrameCfgFileManage::ParseStrToInt(std::wstring str)
{
  int pos = str.find(L";");
  str = str.substr(0, pos);

  return _wtoi(str.c_str());
}

