#include "stdafx.h"
#include "SnapUploadController.h"
#include "PlayerPreference.h"
#include "SPlayerDefs.h"
#include "pool.h"
#include "task.h"
#include "request.h"
#include "postdata.h"
#include "postdataelem.h"
#include <string>
#include <sstream>
#include <Strings.h>
#include <io.h>

SnapUploadController::SnapUploadController(void):
  m_stopevent(::CreateEvent(NULL, TRUE, FALSE, NULL)),
  m_thread(NULL),
  m_lastsnapname(L"")
{

}

SnapUploadController::~SnapUploadController(void)
{
  Stop();
}

std::wstring SnapUploadController::GetTempDir()
{
  std::wstring temppath;
  temppath.resize(128);
  ::GetTempPath(128, &temppath[0]);
  return temppath;
}

void SnapUploadController::SetFrame(HWND hwnd)
{
  m_frame = hwnd;
}

void SnapUploadController::Start(const wchar_t* hash_str)
{
  PlayerPreference* pref = PlayerPreference::GetInstance();
  m_hash_str  = hash_str;
  m_totaltime = pref->GetIntVar(INTVAR_CURTOTALPLAYTIME);
  // we should stop running tasks first
  Stop();
  // record values, and current time
  // create thread
  m_thread = (HANDLE)::_beginthread(_thread_dispatch, 0, (void*)this);
  //set current time
  if (pref->GetIntVar(INTVAR_CURPLAYEDTIME) > 0)
    SetCurTime(pref->GetIntVar(INTVAR_CURPLAYEDTIME), ::clock());
  else
    return;
}

void SnapUploadController::Stop()
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

void SnapUploadController::_thread_dispatch(void* param)
{
  static_cast<SnapUploadController*>(param)->_thread();
}

void SnapUploadController::_thread()
{
  PlayerPreference* pref = PlayerPreference::GetInstance();
  // allow immediate cancel, pause execution for 1 sec
  if (::WaitForSingleObject(m_stopevent, 1000) == WAIT_OBJECT_0)
    return;

  // step 1. use sinet to retrieve upload requirements
  // declare sinet pool here
  m_shottime.clear();
  sinet::refptr<sinet::pool>    net_pool = sinet::pool::create_instance();
  sinet::refptr<sinet::task>    net_task = sinet::task::create_instance();
  sinet::refptr<sinet::request> net_rqst = sinet::request::create_instance();
  sinet::refptr<sinet::config>  net_cfg  = sinet::config::create_instance();
  net_task->use_config(net_cfg);
  wchar_t rqst_url[512];
  swprintf_s(rqst_url, 512,
    pref->GetStringVar(STRVAR_GETSNAPTIMEURL_ACT).c_str(),
    GetHashStr().c_str(), GetTotalTime());
  std::wstring urlact(rqst_url);
  std::wstring url = pref->GetStringVar(STRVAR_GETSNAPTIMEURL) + urlact;
  net_rqst->set_request_url(url.c_str());
  net_rqst->set_request_method(REQ_GET);
  net_task->append_request(net_rqst);
  net_pool->execute(net_task);

  while (net_pool->is_running_or_queued(net_task))
  {
    if (::WaitForSingleObject(m_stopevent, 1000) == WAIT_OBJECT_0)
      return;
  }

  // parse the HTTP response body
  std::vector<unsigned char> st_buffer = net_rqst->get_response_buffer();
  st_buffer.push_back(0);
  int         temptime;
  std::string rspstr = (char*)&st_buffer[0];
  if (rspstr.find(']') - rspstr.find('[') == 1)
    return;
  std::stringstream snaptimess(rspstr.c_str());
  snaptimess.ignore(snaptimess.str().length(), '[');
  while (!snaptimess.eof())
  {
    snaptimess >> temptime;
    m_shottime.push_back(temptime);
    snaptimess.ignore(snaptimess.str().length(), ' ');
  }

  if (m_shottime.empty())
    return;

  // step 2. iterate through time_to_shot and calculate 
  //         sleep period before taking a snapshot
  std::vector<unsigned int>::iterator it = m_shottime.begin();
  while (!m_shottime.empty())
  {
    it = m_shottime.begin();
    while (it != m_shottime.end() && *it < m_curtime)
      it++;

    if (::WaitForSingleObject(m_stopevent, 1000) == WAIT_OBJECT_0)
      return;
    if (it != m_shottime.end())
    {
      int wait_time = *it - m_curtime - (::clock() - m_cursystime);

      if (wait_time > 0 && wait_time < 5000)
      {
        pref->SetIntVar(INTVAR_CURSNAPTIME, *it);
        ::Sleep(wait_time);
        // we should have reached the point to take snapshot
        // step 3. send message to main frame to take snapshot
        ::SendMessage(m_frame, WM_COMMAND, ID_CONTROLLER_SAVE_IMAGE, NULL);
        // wait for 5 seconds
        ::Sleep(5000);
        // step 4. read temporary directory to locate this file, if failed, return
        if (m_lastsnapname == L"")
          continue;
        struct _stat buf;
        if (_wstat(m_lastsnapname.c_str(), &buf) != 0)
          continue;
        // step 5. if successful, use sinet to upload (similar to step 1's loop)
        UploadImage();
      }
    }
    if (pref->GetIntVar(INTVAR_CURPLAYEDTIME) > 0)
      SetCurTime(pref->GetIntVar(INTVAR_CURPLAYEDTIME), ::clock());
    else
      return;
  }
}

void SnapUploadController::SetCurTime(int curtime, long cursystime)
{
  m_curtime    = curtime;
  m_cursystime = cursystime;
}

std::wstring SnapUploadController::GetHashStr()
{
  return m_hash_str;
}

unsigned int SnapUploadController::GetTotalTime()
{
  return m_totaltime;
}

void SnapUploadController::RemoveShotTime(int usedtime)
{
  for (std::vector<unsigned int>::iterator it = m_shottime.begin();
       it != m_shottime.end(); it++)
    if (*it == usedtime)
    {
      m_shottime.erase(it);
      break;
    }
}

void SnapUploadController::SetLastSnapFile(std::wstring fn)
{
  m_lastsnapname = fn;
}

void SnapUploadController::UploadImage()
{
  sinet::refptr<sinet::pool>     net_pool = sinet::pool::create_instance();
  sinet::refptr<sinet::task>     net_task = sinet::task::create_instance();
  sinet::refptr<sinet::request>  net_rqst = sinet::request::create_instance();
  sinet::refptr<sinet::postdata> net_pd   = sinet::postdata::create_instance();

  sinet::refptr<sinet::postdataelem> net_pelem1 = sinet::postdataelem::create_instance();
  net_pelem1->set_name(L"file");
  net_pelem1->setto_file(m_lastsnapname.c_str());
  net_pd->add_elem(net_pelem1);

  sinet::refptr<sinet::postdataelem> net_pelem2 = sinet::postdataelem::create_instance();
  net_pelem2->set_name(L"videohash");
  net_pelem2->setto_text(GetHashStr().c_str());
  net_pd->add_elem(net_pelem2);

  sinet::refptr<sinet::postdataelem> net_pelem3 = sinet::postdataelem::create_instance();
  net_pelem3->set_name(L"action");
  net_pelem3->setto_text(L"upload_snapshot");
  net_pd->add_elem(net_pelem3);

  PlayerPreference* pref = PlayerPreference::GetInstance();
  net_rqst->set_request_url(pref->GetStringVar(STRVAR_GETSNAPTIMEURL).c_str());

  net_rqst->set_request_method(REQ_POST);
  net_rqst->set_postdata(net_pd);
  net_task->append_request(net_rqst);
  net_pool->execute(net_task);

  while (net_pool->is_running_or_queued(net_task))
  {
    if (::WaitForSingleObject(m_stopevent, 1000) == WAIT_OBJECT_0)
      return;
  }

  remove(Strings::WStringToUtf8String(m_lastsnapname).c_str());
}