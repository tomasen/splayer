#include "stdafx.h"
#include "NetworkControlerImpl.h"
#include "../revision.h"

NetworkControlerImpl::NetworkControlerImpl(void)
: m_oemtitle(L"")
{
}

NetworkControlerImpl::~NetworkControlerImpl(void)
{
}


void NetworkControlerImpl::SetOemTitle(std::wstring str)
{
  m_oemtitle = str;
}

std::wstring NetworkControlerImpl::GetServerUrl(int req_type , int tryid)
{

  std::wstring apiurl;
  wchar_t str[100] = L"https://www.shooter.cn/";

  if (tryid > 1 && tryid <= 11)
  {
    if (tryid >= 4)
    {
      int iSvrId = 4 + rand()%7;    
      if (tryid%2)
        wsprintf(str, L"https://splayer%d.shooter.cn/", iSvrId-1);
      else
        wsprintf(str, L"http://splayer%d.shooter.cn/", iSvrId-1);
    }
    else
      wsprintf(str, L"https://splayer%d.shooter.cn/", tryid-1);
  }
  else if (tryid > 11)
    wsprintf(str, L"http://svplayer.shooter.cn/");

  apiurl.assign(str);
  switch(req_type)
  {
    case 'upda':
      apiurl = L"https://updater.shooter.cn/api/updater.php";
      break;
    case 'upsb':
      apiurl += L"api/subup.php";
      break;
    case 'sapi':
      apiurl += L"api/subapi.php";
      break;
  }
  return apiurl;
}
void NetworkControlerImpl::SinetConfig(sinet::refptr<sinet::config> cfg, int retryid)
{
  wchar_t agentbuff[MAX_PATH];
  if(m_oemtitle.empty())
    wsprintf(agentbuff, L"SPlayer Build %d", SVP_REV_NUMBER);
  else
    wsprintf(agentbuff, L"SPlayer Build %d OEM%s", SVP_REV_NUMBER ,m_oemtitle.c_str());
  cfg->set_strvar(CFG_STR_AGENT, agentbuff);

  std::wstring proxy;
  if (retryid%2 == 0)
  {
    DWORD ProxyEnable = 0;
    wchar_t ProxyServer[256];
    DWORD ProxyPort = 0;

    ULONG len;
    CRegKey key;
    if( ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"), KEY_READ)
      && ERROR_SUCCESS == key.QueryDWORDValue(_T("ProxyEnable"), ProxyEnable) && ProxyEnable
      && ERROR_SUCCESS == key.QueryStringValue(_T("ProxyServer"), ProxyServer, &len))
    {
      proxy += L"http://";
      std::wstring proxystr(ProxyServer);
      proxy += proxystr.c_str();
    }
  }
 
  if (!proxy.empty())
    cfg->set_strvar(CFG_STR_PROXY, proxy);
}

void NetworkControlerImpl::MapToPostData(refptr<postdata> data ,std::map<std::wstring, std::wstring> &postform)
{
    for (std::map<std::wstring, std::wstring>::iterator it = postform.begin();
        it != postform.end(); it++)
    {
        refptr<postdataelem> elem = postdataelem::create_instance();
        elem->set_name((it->first).c_str());
        elem->setto_text((it->second).c_str());
        data->add_elem(elem);
    }
}