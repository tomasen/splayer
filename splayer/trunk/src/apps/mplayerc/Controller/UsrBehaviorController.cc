#include "StdAfx.h"
#include "../Utils/Strings.h"
#include "../Utils/CriticalSection.h"
#include "../Model/UsrBehaviorData.h"
#include "pool.h"
#include "task.h"
#include "request.h"
#include "postdata.h"
#include "postdataelem.h"
#include "PlayerPreference.h"
#include "SPlayerDefs.h"
#include "UsrBehaviorController.h"
#include "../Utils/SPlayerGUID.h"

sinet::refptr<sinet::task> GetPreUploadTask()
{
  PlayerPreference* pref = PlayerPreference::GetInstance();
  sinet::refptr<sinet::task>    net_task = sinet::task::create_instance();
  sinet::refptr<sinet::request> net_rqst = sinet::request::create_instance();
  std::wstring rqst_url = pref->GetStringVar(STRVAR_UPLOADUSRBHVURL_ACT);
  std::wstring url      = pref->GetStringVar(STRVAR_UPLOADUSRBHVURL) + rqst_url;
  net_rqst->set_request_url(url.c_str());
  net_rqst->set_request_method(REQ_GET);
  net_task->append_request(net_rqst);
  return net_task;
}

sinet::refptr<sinet::task> GetUploadTask(std::wstring logfile)
{
  PlayerPreference* pref = PlayerPreference::GetInstance();
  sinet::refptr<sinet::task>     net_task = sinet::task::create_instance();
  sinet::refptr<sinet::request>  net_rqst = sinet::request::create_instance();
  sinet::refptr<sinet::postdata> net_pd   = sinet::postdata::create_instance();
  sinet::refptr<sinet::postdataelem> net_pelem1 = sinet::postdataelem::create_instance();
  net_pelem1->set_name(L"file");
  net_pelem1->setto_file(logfile.c_str());
  net_pd->add_elem(net_pelem1);

  std::wstring uid;
  SPlayerGUID::GenerateGUID(uid);
  sinet::refptr<sinet::postdataelem> net_pelem2 = sinet::postdataelem::create_instance();
  net_pelem2->set_name(L"uuid");
  net_pelem2->setto_text(uid.c_str());
  net_pd->add_elem(net_pelem2);

  sinet::refptr<sinet::postdataelem> net_pelem3 = sinet::postdataelem::create_instance();
  net_pelem3->set_name(L"action");
  net_pelem3->setto_text(L"upload_usrbhv_db");
  net_pd->add_elem(net_pelem3);

  net_rqst->set_request_url(pref->GetStringVar(STRVAR_UPLOADUSRBHVURL).c_str());
  net_rqst->set_request_method(REQ_POST);
  net_rqst->set_postdata(net_pd);
  net_task->append_request(net_rqst);

  return net_task;
}

void UsrBehaviorController::AppendEntry(int id, std::wstring data)
{
  AutoCSLock autocslock(m_cs);
  m_ubhvdata.AppendEntry(id, data);
}

void UsrBehaviorController::Start()
{
  // we should stop running tasks first
  Stop();
  m_thread = (HANDLE)::_beginthread(_thread_dispatch, 0, (void*)this);
}

void UsrBehaviorController::Stop()
{
  unsigned long thread_exitcode;
  if (m_thread && m_thread != INVALID_HANDLE_VALUE &&
    GetExitCodeThread(m_thread, &thread_exitcode) &&
    thread_exitcode == STILL_ACTIVE)
  {
    ::SetEvent(m_stopevent);
    ::WaitForSingleObject(m_thread, INFINITE);
  }
  m_thread = NULL;
  ::ResetEvent(m_stopevent);
}

void UsrBehaviorController::_thread_dispatch(void* param)
{
  static_cast<UsrBehaviorController*>(param)->_thread();
}

void UsrBehaviorController::_thread()
{
  if (::WaitForSingleObject(m_stopevent, 1000) == WAIT_OBJECT_0)
    return;

  std::wstring uid;
  SPlayerGUID::GenerateGUID(uid);
  wchar_t dbnameform[128], curdbname[128];
  wchar_t appdatapath[128], findfileform[256];
  swprintf_s(dbnameform, 128, L"splayer_ubdb_%s_*.log", uid.c_str());
  swprintf_s(curdbname, 128, DATABASE_NAME, uid.c_str(), UsrBehaviorData::GetWeekCount());
  ::GetEnvironmentVariable(L"APPDATA", appdatapath, 128);
  swprintf_s(findfileform, 256, L"%s\\SPlayer\\ubdata\\%s", appdatapath, dbnameform);

  std::vector<std::wstring> uploadfiles, removelist;
  HANDLE hfind;
  bool   bfound;
  WIN32_FIND_DATA findfiledata;
  hfind  = ::FindFirstFile(findfileform,&findfiledata);
  for (bfound = (hfind != INVALID_HANDLE_VALUE);
       bfound; bfound = FindNextFile(hfind, &findfiledata) ? true:false)
  {
    std::wstring curfindname = findfiledata.cFileName;
    if (lstrcmp(curfindname.c_str(), curdbname))
      uploadfiles.push_back(curfindname);
  }
  if (uploadfiles.size() == 0)
    // no old log files
    return;

  sinet::refptr<sinet::pool> net_pool = sinet::pool::create_instance();
  sinet::refptr<sinet::task> net_preupload_task = GetPreUploadTask();
  net_pool->execute(net_preupload_task);
  while (net_pool->is_running_or_queued(net_preupload_task))
    if (::WaitForSingleObject(m_stopevent, 1000) == WAIT_OBJECT_0)
      return;
  sinet::refptr<sinet::task> lasttask;
  if (net_preupload_task->get_request(0)->get_response_errcode())
    // server refuse the uploading
    return;
  for (std::vector<std::wstring>::iterator it = uploadfiles.begin();
       it != uploadfiles.end(); it++)
  {
    // ready to upload
    wchar_t uploadfile[256];
    swprintf_s(uploadfile, 256, L"%s\\SPlayer\\ubdata\\%s",
      appdatapath, (*it).c_str());
    sinet::refptr<sinet::task> net_upload_task = GetUploadTask(uploadfile);
    net_pool->execute(net_upload_task);
    lasttask = net_upload_task;
    removelist.push_back(uploadfile);
  }

  while (net_pool->is_running_or_queued(lasttask))
    if (::WaitForSingleObject(m_stopevent, 1000) == WAIT_OBJECT_0)
      return;
  // remove the old log files
  for (std::vector<std::wstring>::iterator it = removelist.begin();
       it != removelist.end(); it++)
    remove(Strings::WStringToUtf8String(*it).c_str());
}
