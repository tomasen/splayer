#include "stdafx.h"
#include "Ed2kController.h"
#include "rhash_ex.h"
#include "Strings.h"
#include "HashController.h"
#include "SPlayerDefs.h"
#include "PlayerPreference.h"
#include "base64.h"

Ed2kController::Ed2kController()
{

}

void Ed2kController::AddMediaFile(const std::wstring &path)
{
  m_cs.lock();
  m_lsMediaFiles.push_back(path);
  m_cs.unlock();
}

void Ed2kController::_Thread()
{
  // get monitor ptr
  m_monitor = MT::factory::get_monitor(MT::monitor_disk_busy_time);
  if (!m_monitor)
    return;

  // enter thread loop
  while (true)
  {
    // -------------------------------------------------------------------------
    // see if has files to be deal with
    if (!m_lsMediaFiles.empty())
    {
      // fetch a string
      m_cs.lock();
      std::wstring cur_path = m_lsMediaFiles.front();
      m_cs.unlock();

      // check to see if the ed2k already on the server
      ED2K_EXIST_ENUM exist_enum = IsEd2kExist(cur_path);
      if (exist_enum == ED2K_NOT_EXIST)
      {
        // calculate ed2k
        std::vector<std::string> vtEd2k;
        RHash::create_link(RHASH_ED2K, Strings::WStringToString(cur_path), vtEd2k);

        // upload ed2k
        if (!vtEd2k.empty())
          UploadEd2k(cur_path, Strings::StringToWString(vtEd2k[0]));
      }

      // pop the first string
      m_cs.lock();
      m_lsMediaFiles.pop_front();
      m_cs.unlock();
    }

    // -------------------------------------------------------------------------
    // Note:
    // sleep according to the disk io traffic
    // if the disk is too busy, then sleep until the disk is not busy
    // if the disk is not busy, then sleep just once
    if (_Exit_state(50))  // First, sleep for a very short time
      return;

    // Second, start monitoring
    m_monitor->start_monitoring();

    // Then, check the disk io in a while loop
    int nTime = 300;
    while (!_Exit_state(nTime))
    {
      // Get the current rate
      double dRate = m_monitor->get_cur_value();

      // If the rate is greater than 70%, then sleep for a longer time to wait next loop
      if (dRate > 70.0)
        nTime += 200;   // add the sleep time
      else
        break;
    }

    // Now, stop monitoring
    m_monitor->stop_monitoring();

    // Check the exit state again
    if (_Exit_state(0))
      return;
  }
}

Ed2kController::ED2K_EXIST_ENUM Ed2kController::IsEd2kExist(const std::wstring &file)
{
  // check the ed2k to see if it's already on the server
  sinet::refptr<sinet::pool>     net_pool = sinet::pool::create_instance();
  sinet::refptr<sinet::task>     net_task = sinet::task::create_instance();
  sinet::refptr<sinet::request>  net_rqst = sinet::request::create_instance();
  
  PlayerPreference* pref = PlayerPreference::GetInstance();
  std::wstring file_hash = HashController::GetInstance()->GetSPHash(file.c_str());
  CString api;
  api.Format(pref->GetStringVar(STRVAR_CHECK_ED2K_EXIST).c_str(), file_hash.c_str());
  net_rqst->set_request_url(api);
  net_rqst->set_request_method(REQ_GET);

  net_task->append_request(net_rqst);
  net_pool->execute(net_task);
  while (net_pool->is_running_or_queued(net_task))
  {
    if (_Exit_state(500))
      return ED2K_SERVER_ERROR;
  }

  std::vector<unsigned char> st_buffer = net_rqst->get_response_buffer();
  st_buffer.push_back(0);

  std::wstring ret_code = Strings::Utf8StringToWString(std::string(st_buffer.begin(), st_buffer.end()));

  if (ret_code == L"-1")
    return ED2K_NOTEXIST_NO_UPLOAD;
  else if (ret_code == L"0")
    return ED2K_NOT_EXIST;
  else if (ret_code == L"1")
    return ED2K_EXIST;
  else
    return ED2K_SERVER_ERROR;
}

bool Ed2kController::UploadEd2k(const std::wstring &file, const std::wstring &link)
{
  // upload the ed2k link to server
  sinet::refptr<sinet::pool>     net_pool = sinet::pool::create_instance();
  sinet::refptr<sinet::task>     net_task = sinet::task::create_instance();
  sinet::refptr<sinet::request>  net_rqst = sinet::request::create_instance();
  sinet::refptr<sinet::postdata> net_pd   = sinet::postdata::create_instance();

  PlayerPreference* pref = PlayerPreference::GetInstance();
  std::wstring file_hash = HashController::GetInstance()->GetSPHash(file.c_str());
  std::string link_utf8 = Strings::WStringToUtf8String(link);
  std::wstring link_base64 = Strings::StringToWString(base64_encode((const unsigned char*)(link_utf8.c_str()), link_utf8.size()));

  CString api;
  api.Format(pref->GetStringVar(STRVAR_UPLOAD_ED2K).c_str(), file_hash.c_str(), link_base64.c_str());
  net_rqst->set_request_url(api);
  net_rqst->set_request_method(REQ_GET);

  net_task->append_request(net_rqst);
  net_pool->execute(net_task);
  while (net_pool->is_running_or_queued(net_task))
  {
    if (_Exit_state(500))
      return false;
  }

  std::vector<unsigned char> st_buffer = net_rqst->get_response_buffer();
  st_buffer.push_back(0);

  std::wstring ret_code = Strings::Utf8StringToWString(std::string(st_buffer.begin(), st_buffer.end()));

  if (ret_code == L"0")
    return false;
  else
    return true;
}