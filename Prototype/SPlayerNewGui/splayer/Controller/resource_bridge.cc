#include "stdafx.h"
#include "resource_bridge.h"
#include "../Model/rsc_format.h"

ResourceBridge::ResourceBridge(void)
{
  m_rsc_basename = L"splayer.rsc";

  // "splayer.rsc" is assumed to be in the same folder of
  // the exe module

  TCHAR path[MAX_PATH] = {0};
  ::GetModuleFileName(NULL, path, MAX_PATH);
  ::PathRemoveFileSpec(path);
  wcscat_s(path, MAX_PATH, L"\\");
  wcscat_s(path, MAX_PATH, L"splayer.rsc");

  rsc_format::Parse(path, m_strings, m_buffers);
}

void ResourceBridge::SetLocale(const wchar_t* locale)
{
  m_locale = locale;
}

std::wstring ResourceBridge::GetLocale()
{
  return m_locale;
}

std::wstring ResourceBridge::LoadString(const wchar_t* path)
{
  // locate locale-based path
  std::map<std::wstring, std::wstring>::iterator it = 
    m_strings.find(GetRealPath(path));
  if (it != m_strings.end())
    return it->second;

  // locate bare path
  it = m_strings.find(path);
  if (it != m_strings.end())
    return it->second;

  return std::wstring();
}

std::vector<unsigned char> ResourceBridge::LoadBuffer(const wchar_t* path)
{
  // locate locale-based path
  std::map<std::wstring, std::vector<unsigned char> >::iterator it = 
    m_buffers.find(GetRealPath(path));
  if (it != m_buffers.end())
    return it->second;

  // locate bare path
  it = m_buffers.find(path);
  if (it != m_buffers.end())
    return it->second;

  return std::vector<unsigned char>();
}

std::wstring ResourceBridge::GetRealPath(const wchar_t* path)
{
  std::wstring realpath = path;
  if (!m_locale.empty())
  {
    realpath += L"@";
    realpath += m_locale;
  }
  return realpath;
}