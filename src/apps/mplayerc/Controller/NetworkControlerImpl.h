#pragma once

#include <sinet.h>

using namespace sinet;

class NetworkControlerImpl
{
public:
  NetworkControlerImpl(void);
  ~NetworkControlerImpl(void);
  
  void SinetConfig(sinet::refptr<sinet::config> cfg, int sid, std::wstring oem);
  std::wstring GetServerUrl(int req_type , int tryid);

  void SetOemTitle(std::wstring str);
  std::wstring m_oemtitle;
  
};
