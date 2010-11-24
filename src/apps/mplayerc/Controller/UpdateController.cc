#include "stdafx.h"
#include "UpdateController.h"

#include "hashcontroller.h"
#include "LazyInstance.h"
#include "../utils/strings.h"
#include "../model/SubTransFormat.h"
#include "../../mplayerc\revision.h"

#include <sinet.h>
#include <windows.h>
#include <fstream>
#include <string>

using namespace sinet;


const std::wstring updatefilename = L"Updater.exe";


UpdateController::UpdateController(void):
  m_stopevent(::CreateEvent(NULL, TRUE, FALSE, NULL))
{
  m_localversion = SVP_REV_NUMBER;
}

UpdateController::~UpdateController(void)
{
  Stop();
}


void UpdateController::Stop()
{
  ::SetEvent(m_stopevent);
  ::WaitForSingleObject(m_thread, 3000);

  ::ResetEvent(m_stopevent);
}

bool UpdateController::CheckUpdateEXEUpdate()
{
  std::wstring ftmp, pwd;

  wchar_t path[MAX_PATH];
  ::GetModuleFileName(NULL, path, MAX_PATH);
  pwd = path;
  pwd = pwd.substr(0, pwd.find_last_of(L'\\')+1);

  std::wstring szLocalHash = 
                HashController::GetInstance()->GetMD5Hash((pwd+L"Updater.exe").c_str());
  
  // if dir not writable, return 0
  if(SubTransFormat::IfDirWritable_STL(pwd) == false)
    return false;

  if(::GetTempPath(MAX_PATH, path) == 0 )
    return false;

  ftmp = path;
 
  sinet::refptr<sinet::pool>     net_pool = sinet::pool::create_instance();
  sinet::refptr<sinet::task>     net_task = sinet::task::create_instance();
  sinet::refptr<sinet::request>  net_rqst = sinet::request::create_instance();
  sinet::refptr<sinet::postdata> net_pd   = sinet::postdata::create_instance();
  
  refptr<config> cfg = config::create_instance();
  //int sid; std::wstring oem;
  SinetConfig(cfg, 0, m_oemtitle);
  net_task->use_config(cfg);

  // string package format: "branch=updater%s&current=%s", BRANCHVER, hashfile
  // 将文件版本号+文件长度hash 发送给 http://svplayer.shooter.cn/api/updater.php 
  // http://cdimage.debian.org/debian-cd/5.0.6/i386/iso-cd/debian-506-i386-netinst.iso
  sinet::refptr<sinet::postdataelem> net_pelem1 = sinet::postdataelem::create_instance();
  net_pelem1->set_name(L"branch");
  std::wstring str = L"updater";
  str.append(BRANCHVER);
  net_pelem1->setto_text(str.c_str());
  net_pd->add_elem(net_pelem1);

  sinet::refptr<sinet::postdataelem> net_pelem2 = sinet::postdataelem::create_instance();
  net_pelem2->set_name(L"current");
  net_pelem2->setto_text(szLocalHash.c_str());
  net_pd->add_elem(net_pelem2);

  std::wstring url = GetServerUrl('upda', 0);

  OutputDebugString((str+szLocalHash+url).c_str());
  net_rqst->set_request_url(url.c_str());
  net_rqst->set_request_method(REQ_POST);
  net_rqst->set_postdata(net_pd);
  net_task->append_request(net_rqst);
  net_pool->execute(net_task);

  while (net_pool->is_running_or_queued(net_task))
  {
    if (::WaitForSingleObject(m_stopevent, 1000) == WAIT_OBJECT_0)
      return 0;
  }
  
  int err = net_rqst->get_response_errcode();
  if (err == 0) // successed
  {
    std::vector<unsigned char> st_buffer = net_rqst->get_response_buffer();  
    // put st_buffer save to tmp file
    ftmp.append(L"tmpsplayer.tmp");
    std::fstream ff;
    ff.open(ftmp.c_str(),std::ios::trunc);

    if(!ff.is_open())
      return false;

    std::vector<unsigned char>::iterator it = st_buffer.begin();
    for(;it!=st_buffer.end();it++)
      ff << *it;

    ff.close();

    // unpackGz the tmp file to updater.exe
    SubTransFormat::UnpackGZFile(ftmp, pwd);
  }
  return true;
}

