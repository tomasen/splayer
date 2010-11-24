#pragma once

#include <sinet.h>

using namespace sinet;

class NetworkControlerImpl
{
public:
  NetworkControlerImpl(void);
  ~NetworkControlerImpl(void);
  
  void SinetConfig(sinet::refptr<sinet::config> cfg, int retryid);
  std::wstring GetServerUrl(int req_type , int retryid);

  void SetOemTitle(std::wstring str);
  std::wstring m_oemtitle;
  
};
