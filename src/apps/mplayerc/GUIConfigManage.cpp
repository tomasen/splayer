#include "stdafx.h"
#include "GUIConfigManage.h"
#include <fstream>

GUIConfigManage::GUIConfigManage(void)
{
}

GUIConfigManage::~GUIConfigManage(void)
{
}

void GUIConfigManage::ReadFromFile()
{
  std::wifstream in_file;
  std::wstring  buttoninformation;
  in_file.open(m_filepath.c_str());
  if (!in_file)
  {
    m_breadfromfile = FALSE;
    return;
  }

  while (getline(in_file, buttoninformation))
  {
    if (buttoninformation[0] == ' ' || buttoninformation == L"")
      continue;

    m_cfgfilestr_vec.push_back(buttoninformation);
  }

  in_file.close();
  m_breadfromfile = TRUE;
}

void GUIConfigManage::WriteToFile()
{
  std::wofstream outfile;
  outfile.open(m_filepath.c_str(), std::wofstream::out | std::wofstream::trunc);
  if (outfile)
    for (std::vector<std::wstring>::const_iterator ite = m_cfgfilestr_vec.begin();
      ite != m_cfgfilestr_vec.end(); ++ite)
      outfile<<(*ite)<<L"\n";
}

void GUIConfigManage::SetCfgFilePath(std::wstring s)
{
  m_filepath = s;
}

std::wstring GUIConfigManage::GetCfgFilePath()
{
  return m_filepath;
}

BOOL GUIConfigManage::IsFileExist()
{
  return m_breadfromfile;
}

std::vector<std::wstring>& GUIConfigManage::GetCfgString()
{
  return m_cfgfilestr_vec;
}

void GUIConfigManage::SetCfgString(const std::vector<std::wstring>& vec)
{
  m_cfgfilestr_vec = vec;
}