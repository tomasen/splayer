#include "StdAfx.h"

#include "pool.h"
#include "task.h"
#include "request.h"
#include "postdata.h"
#include "postdataelem.h"
#include "PlayerPreference.h"
#include "SPlayerDefs.h"
#include "UbdUploadController.h"

#include "../Model/UsrBehaviorData.h"
#include "../Utils/SPlayerGUID.h"
#include <Strings.h>

sinet::refptr<sinet::task> UploadForm(std::wstring file, std::wstring uid, std::wstring url)
{
  sinet::refptr<sinet::postdata> postdata = sinet::postdata::create_instance();
  sinet::refptr<sinet::postdataelem> elem1 = sinet::postdataelem::create_instance();
  sinet::refptr<sinet::postdataelem> elem2 = sinet::postdataelem::create_instance();
  sinet::refptr<sinet::postdataelem> elem3 = sinet::postdataelem::create_instance();
  sinet::refptr<sinet::request> upreq = sinet::request::create_instance();
  sinet::refptr<sinet::task> uptask = sinet::task::create_instance();

  elem1->set_name(L"file");
  elem1->setto_file(file.c_str());
  postdata->add_elem(elem1);

  elem2->set_name(L"uuid");
  elem2->setto_text(uid.c_str());
  postdata->add_elem(elem2);

  elem3->set_name(L"action");
  elem3->setto_text(L"upload_usrbhv_db");
  postdata->add_elem(elem3);

  upreq->set_request_url(url.c_str());
  upreq->set_request_method(REQ_POST);
  upreq->set_postdata(postdata);
  uptask->append_request(upreq);
  return uptask;
}

void UbdUploadController::Start()
{
  // we should stop running tasks first
  Stop();
  m_thread = (HANDLE)::_beginthread(_thread_dispatch, 0, (void*)this);
}

void UbdUploadController::Stop()
{
  unsigned long thread_exitcode;
  if (m_thread && m_thread != INVALID_HANDLE_VALUE &&
    GetExitCodeThread(m_thread, &thread_exitcode) &&
    thread_exitcode == STILL_ACTIVE)
  {
    ::SetEvent(m_stopevent);
    ::WaitForSingleObject(m_thread, 3001);
  }
  m_thread = NULL;
  ::ResetEvent(m_stopevent);
}

void UbdUploadController::_thread_dispatch(void* param)
{
  static_cast<UbdUploadController*>(param)->_thread();
}

void UbdUploadController::_thread()
{
  int year, weekcount;
  std::wstring uid, format, path;
  WIN32_FIND_DATA findfiledata;
  std::vector<std::wstring> uploadfiles, removelist;
  wchar_t curdbname[MAX_PATH], apppath[MAX_PATH], findfileform[MAX_PATH];

  sinet::refptr<sinet::pool> pool = sinet::pool::create_instance();
  sinet::refptr<sinet::task> task = sinet::task::create_instance();
  
  // request server can we upload files?
  PlayerPreference* pref = PlayerPreference::GetInstance();
  sinet::refptr<sinet::request> req = sinet::request::create_instance();
  std::wstring url = pref->GetStringVar(STRVAR_UPLOADUSRBHVURL) + pref->GetStringVar(STRVAR_UPLOADUSRBHVURL_ACT);
  
  req->set_request_url(url.c_str());
  req->set_request_method(REQ_GET);
  task->append_request(req);
  pool->execute(task);
 
  while (pool->is_running_or_queued(task))
  {
    if (::WaitForSingleObject(m_stopevent, 1000) == WAIT_OBJECT_0)
      return;
  }
  // no need upload
  if (req->get_response_errcode())
    return;

  // find upload files
  SPlayerGUID::GenerateGUID(uid);
  UsrBehaviorData::GetYearAndWeekcount(year, weekcount);
  ::GetEnvironmentVariable(L"APPDATA", apppath, MAX_PATH);
 
  path = apppath;
  path += L"\\SPlayer\\ubdata\\";

  format = path;
  format += LOGFILE_FORMAT;

  swprintf_s(findfileform, MAX_PATH, format.c_str(), uid.c_str(), year);
  swprintf_s(curdbname, MAX_PATH, DATABASE_NAME, uid.c_str(), year, weekcount);

  HANDLE hfind;
  BOOL search = TRUE;
  hfind  = ::FindFirstFile(findfileform, &findfiledata);
  if (hfind == INVALID_HANDLE_VALUE)
    return;

  while (search)
  {
    // putting the non-current logfile into uploadfiles
    if (wcscmp(findfiledata.cFileName, curdbname) != 0)
      uploadfiles.push_back(std::wstring(findfiledata.cFileName));

    search = ::FindNextFile(hfind, &findfiledata);
  }
  ::FindClose(hfind);

  if (uploadfiles.empty())
    return;

  sinet::refptr<sinet::task> lasttask;
  std::wstring posturl = pref->GetStringVar(STRVAR_UPLOADUSRBHVURL);
  for (std::vector<std::wstring>::iterator it = uploadfiles.begin();
       it != uploadfiles.end(); it++)
  {
    std::wstring file = path + *it;
    lasttask = UploadForm(file, uid, posturl);
    pool->execute(lasttask);
    removelist.push_back(file);
  }

  while (pool->is_running_or_queued(lasttask))
  {
    if (::WaitForSingleObject(m_stopevent, 1000) == WAIT_OBJECT_0)
      return;
  }

  // remove upload log files
  for (std::vector<std::wstring>::iterator it = removelist.begin();
       it != removelist.end(); it++)
    remove(Strings::WStringToUtf8String(*it).c_str());
}
