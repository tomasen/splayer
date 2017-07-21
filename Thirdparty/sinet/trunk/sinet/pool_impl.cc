#include "pch.h"
#include "pool_impl.h"
#include "strings.h"
#include <curl/curl.h>
#if defined(_WINDOWS_)
#include <process.h>
#include <algorithm>
#include <ctype.h>
#elif defined(_MAC_) || defined(__linux__)
#include <pthread.h>
#endif

using namespace sinet;

refptr<pool> pool::create_instance()
{
  refptr<pool> _pool(new pool_impl());
  return _pool;
}

static size_t write_header_callback(void* ptr, size_t size, size_t nmemb, void* data)
{
  int ret = size*nmemb;

  refptr<request> request_in((request*)data);
  si_stringmap header = request_in->get_response_header();

  std::string str((char*)ptr);
  std::wstring newstr = strings::utf8string_wstring(str);
  
  // the request header is end with '\r\n'
  if (newstr == L"\r\n")
   return ret;

  std::wstring key, value;
  std::wstring::size_type pos;

  // format:
  // Connection: keep-alive
  pos = newstr.find(L":");
  if (pos != std::string::npos)
  {
    key = newstr.substr(0, pos);
    value = newstr.substr(pos+1, newstr.max_size());
    header[key] = value;
    if (key == L"Content-Length")
      request_in->set_response_size(wcstol(value.c_str(), 0, 10));
  }
  else 
  {
    // save HTTP response status
    header[L""] = newstr;  
    std::wstring reqstatus = newstr.substr(newstr.find_first_of(L" ")+1, 3);
    request_in->set_response_errcode(wcstol(reqstatus.c_str(), 0, 10));
  }
  
  request_in->set_response_header(header);

  return ret;
}

static size_t write_mem_callback(void* ptr, size_t size, size_t nmemb, void* data)
{
  size_t realsize = size * nmemb;

  refptr<request> request_in = (request*)data;
  request_in->set_appendbuffer(ptr, realsize);

  return realsize;
}

pool_impl::pool_impl(void)
#if defined(_WINDOWS_)
  :m_stop_event(::CreateEvent(NULL, TRUE, FALSE, NULL))
#endif
{
#if defined(_WINDOWS_)
  m_thread = (HANDLE)::_beginthread(_thread_dispatch, 0, (void*)this);
#elif defined(_MAC_) || defined(__linux__)
  pthread_cond_init(&m_stop_event, NULL);
  ::pthread_create(&m_thread, NULL, _thread_dispatch, this);
#endif
}

pool_impl::~pool_impl(void)
{
  _stop_thread();
  clear_all();
#if defined(_WINDOWS_)
  ::CloseHandle(m_stop_event);
#elif defined(_MAC_) || defined(__linux__)
  pthread_cond_destroy(&m_stop_event);
#endif
  // when a thread created by _beginthread is gracefully closed, it'll
  // call CloseHandle automatically, so there's no need for additional
  // free up operation.
}

void pool_impl::execute(refptr<task> task_in)
{
  if (!task_in)
    return;

  // execute operation only push the task into queue.
  // later on the coordinating thread should pick it up
  // whenever possible.

  task_in->set_status(taskstatus_queued);

  m_cstask_queue.lock();
  m_task_queue.push_back(task_in);
  m_cstask_queue.unlock();
}

void pool_impl::cancel(refptr<task> task_in)
{
  // stop task if it's running
  m_cstasks_running.lock();
  m_cstask_queue.lock();
  std::map<refptr<task>, task_info>::iterator it = m_tasks_running.find(task_in);
  if (it != m_tasks_running.end())
  {
    _cancel_running_task(it->second.hmaster, it->second.htasks);
    it->first->set_status(taskstatus_canceled);
    m_cstask_finished.lock();
    m_task_finished.push_back(it->first);
    m_cstask_finished.unlock();
    m_tasks_running.erase(it);
  }
  // clear task if it's queued
  for (std::vector<refptr<task> >::iterator it = m_task_queue.begin();
      it != m_task_queue.end(); it++)
  {
    if (*it == task_in)
    {
     m_task_queue.erase(it);            // Mod
     break;
    }
  }
  m_cstask_queue.unlock();
  m_cstasks_running.unlock();
}

int pool_impl::is_running(refptr<task> task_in)
{
  int ret = 0;
  m_cstasks_running.lock();
  std::map<refptr<task>, task_info>::iterator it = m_tasks_running.find(task_in);
  if (it != m_tasks_running.end() && it->second.running_handle > 0)
    ret = 1;
  m_cstasks_running.unlock();
  return ret;
}

int pool_impl::is_queued(refptr<task> task_in)
{
  int ret = 0;
  m_cstask_queue.lock();
  for (std::vector<refptr<task> >::iterator it = m_task_queue.begin();
    it != m_task_queue.end(); it++)
  {
    if (*it == task_in)
    {
      ret = 1;
      break;
    }
  }
  m_cstask_queue.unlock();
  return ret;
}

int pool_impl::is_running_or_queued(refptr<task> task_in)
{
  int ret = 0;
  m_cstasks_running.lock();
  m_cstask_queue.lock();
  std::map<refptr<task>, task_info>::iterator it = m_tasks_running.find(task_in);
  if (it != m_tasks_running.end() && it->second.running_handle > 0)
    ret = 1;
  for (std::vector<refptr<task> >::iterator it = m_task_queue.begin();
    it != m_task_queue.end(); it++)
  {
    if (*it == task_in)
    {
      ret = 1;
      break;
    }
  }
  m_cstask_queue.unlock();
  m_cstasks_running.unlock();
  return ret;
}

void pool_impl::clear_all()
{
  // clear running tasks
  m_cstasks_running.lock();
  for (std::map<refptr<task>, task_info>::iterator it = m_tasks_running.begin();
    it != m_tasks_running.end(); it++)
    _cancel_running_task(it->second.hmaster, it->second.htasks);
  m_tasks_running.clear();
  m_cstasks_running.unlock();
  // clear tasks in queue
  m_cstask_queue.lock();
  m_task_queue.resize(0);
  m_cstask_queue.unlock();
  // clear finished tasks
  m_cstask_finished.lock();
  m_task_finished.resize(0);
  m_cstask_finished.unlock();
}

#if defined(_WINDOWS_)
void pool_impl::_thread_dispatch(void* param)
#elif defined(_MAC_) || defined(__linux__)
void* pool_impl::_thread_dispatch(void* param)
#endif
{
  static_cast<pool_impl*>(param)->_thread();
}

void pool_impl::_thread()
{
  // this is is the master pool thread, controlling pool workflow
  // to enable task executing and stopping

  // the while loop only exists when m_stop_event is signaled by
  // _stop_thread()

  // we set a dynamic sleep period here to allow partial sleeping
  // when there are no running tasks and queue

  const size_t sleep_period_defined = 5;
  const size_t sleep_period_max = 500;
  int sleep_period = 5;
  
#if defined(_MAC_) || defined(__linux__)
  pthread_mutex_t mut_wait = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_lock(&mut_wait);
  
  struct timeval now;
  struct timespec timeout;
  long long future_us;
  gettimeofday(&now, NULL);
  future_us = now.tv_usec + sleep_period * 1000;
  timeout.tv_nsec = (future_us % 1000000) * 1000;
  timeout.tv_sec = now.tv_sec + future_us / 1000000;
#endif

#if defined(_WINDOWS_)
  while (::WaitForSingleObject(m_stop_event, sleep_period) == WAIT_TIMEOUT)
#elif defined(_MAC_) || defined(__linux__)
  while (pthread_cond_timedwait(&m_stop_event, &mut_wait, &timeout) == ETIMEDOUT)
#endif
  {
    // printf("thread fired %d\n", (int)(future_us/1000));
    // thread loop procedure is as follows:
    // 1. iterate through |m_tasks_running| and try curl_multi_perform
    //    on all of them, if one's |ti.running_handles| equals 0, move
    //    it to |m_tasks_finished|
    // 2. if all tasks' |ti.running_handles| in |m_tasks_running|
    //    equals 0, go to step 3, otherwise, go back to beginning
    // 3. check |m_tasks_queued| to see if there are anything left
    //    should be prepared for running

    // step 1.
    int tasks_still_running = 0;
    std::vector<std::map<refptr<task>, task_info>::iterator > its_running_toclear;

    m_cstasks_running.lock();
    for (std::map<refptr<task>, task_info>::iterator it = m_tasks_running.begin();
      it != m_tasks_running.end(); it++)
    {
      sleep_period = sleep_period_defined;
      ::curl_multi_perform(it->second.hmaster, &it->second.running_handle);
      if (it->second.running_handle == 0)
      {
        its_running_toclear.push_back(it);  // prepare to remove
        m_cstask_finished.lock();
        m_task_finished.push_back(it->first);
        m_cstask_finished.unlock();
      }
      else
        tasks_still_running++;
    }
    for (std::vector<std::map<refptr<task>, task_info>::iterator >::iterator it = 
      its_running_toclear.begin(); it != its_running_toclear.end(); it++)
    {
      _cancel_running_task((*it)->second.hmaster, (*it)->second.htasks);
      (*it)->first->set_status(taskstatus_completed);
      std::vector<int> ids;
      (*it)->first->get_request_ids(ids);
      for (std::vector<int>::iterator iit = ids.begin(); iit != ids.end(); iit++)
      {
        refptr<request> req = (*it)->first->get_request((*iit));
        req->close_outfile();
      }
      m_tasks_running.erase(*it);
    }
    m_cstasks_running.unlock();

    // step 2.
    if (tasks_still_running == 0)
    {
      // step 3.
      m_cstasks_running.lock();
      m_cstask_queue.lock();
      std::vector<refptr<task> >::iterator it = m_task_queue.begin();
      if (it != m_task_queue.end())
      {
        task_info ti;
        _prepare_task(*it, ti);
        ti.running_handle = 1;
        m_tasks_running[*it] = ti;
        m_task_queue.erase(it);
        sleep_period = sleep_period_defined;
      }
      else if (sleep_period < sleep_period_max)
      {
        sleep_period *= 2;
        if (sleep_period > sleep_period_max)
          sleep_period = sleep_period_max;
      }
      m_cstask_queue.unlock();
      m_cstasks_running.unlock();
    }

#if defined(_MAC_) || defined(__linux__)
    gettimeofday(&now, NULL);
    future_us = now.tv_usec + sleep_period * 1000;
    timeout.tv_nsec = (future_us % 1000000) * 1000;
    timeout.tv_sec = now.tv_sec + future_us / 1000000;
#endif
  }
  
#if defined(_MAC_) || defined(__linux__)
  pthread_mutex_unlock(&mut_wait);
#endif
}

void pool_impl::_prepare_task(refptr<task> task_in, task_info& taskinfo_in_out)
{
  // things below are bogus curl calls for testing workflow
  // _prepare_task should iterator through all requests in refptr<task>
  // and make corresponding curl calls
  std::vector<int> reqids(0);
  
  taskinfo_in_out.running_handle = 0;

  taskinfo_in_out.hmaster = ::curl_multi_init();

  std::wstring proxyurl, useragent;
  refptr<config> cfg = task_in->get_config();
  if (cfg)
  {
    cfg->get_strvar(CFG_STR_PROXY, proxyurl);
    cfg->get_strvar(CFG_STR_AGENT, useragent);
  }
  
 
  task_in->get_request_ids(reqids);
  
  for (std::vector<int>::iterator it = reqids.begin(); it != reqids.end(); it++)
  {
    refptr<request> req = task_in->get_request(*it);
    
    session_curl scurl;

    CURL* curl = ::curl_easy_init();
    if (!curl)
      continue;

    scurl.hcurl = curl;
    scurl.post = NULL;
    scurl.last = NULL;
    scurl.headerlist = NULL;

    ::curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_header_callback);
    ::curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void *)req.get());
    ::curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_mem_callback);
    ::curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)req.get());
    ::curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    ::curl_easy_setopt(curl, CURLOPT_URL, strings::wstring_utf8string(req->get_request_url()).c_str());

    // set the proxy
    if (!proxyurl.empty())
      ::curl_easy_setopt(curl, CURLOPT_PROXY, strings::wstring_utf8string(proxyurl).c_str());

    // set the ssl
    ::curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    // set header
    bool useagent = false;
    si_stringmap header = req->get_request_header();
    for (si_stringmap::iterator it = header.begin(); it != header.end(); it++)
    {
      std::wstring item;
      // format:
      // host:shooter.cn
      item = (*it).first + L":" + (*it).second;
      scurl.headerlist = ::curl_slist_append(scurl.headerlist, strings::wstring_utf8string(item).c_str());
      
      std::wstring headerkey = (*it).first;
      std::transform(headerkey.begin(), headerkey.end(), headerkey.begin(), towlower);
      if (headerkey == L"user-agent")
        useagent = true;
    }

    if (!useagent && !useragent.empty())
      ::curl_easy_setopt(curl, CURLOPT_USERAGENT, strings::wstring_utf8string(useragent).c_str());

    std::wstring reqmethod = req->get_request_method();
    if (reqmethod == REQ_GET)
    {
      ;
    }
    else if (reqmethod == REQ_POST)
    {
      std::vector<refptr<postdataelem> > elems;
      refptr<postdata> postdata = req->get_postdata();
      postdata->get_elements(elems);

      // make a post form
      for (std::vector<refptr<postdataelem> >::iterator it = elems.begin(); it != elems.end(); it++)
      {
        postdataelem_type_t elemtype = (*it)->get_type();
        std::string name = strings::wstring_utf8string((*it)->get_name());

        if (elemtype == PDE_TYPE_TEXT)
        {
          std::string cont = strings::wstring_utf8string((*it)->get_text());
          ::curl_formadd(&scurl.post, &scurl.last, CURLFORM_COPYNAME, name.c_str(), 
                          CURLFORM_COPYCONTENTS, cont.c_str(), CURLFORM_END);
        }
        else if (elemtype == PDE_TYPE_FILE)
        {
          std::string cont = strings::wstring_utf8string((*it)->get_file());
          ::curl_formadd(&scurl.post, &scurl.last, CURLFORM_COPYNAME, name.c_str(),
                          CURLFORM_FILE, cont.c_str(), CURLFORM_END);
        }
        else if (elemtype == PDE_TYPE_BYTES)
        {
          size_t buffsize = (*it)->get_buffer_size();
          void* buffer = malloc(buffsize);
          (*it)->copy_buffer_to(buffer, buffsize);
          
          std::string cont = strings::wstring_utf8string((*it)->get_text());

          ::curl_formadd(&scurl.post, &scurl.last, CURLFORM_COPYNAME, name.c_str(), CURLFORM_BUFFER, cont.c_str(), CURLFORM_BUFFERPTR,
            (char*)buffer, CURLFORM_BUFFERLENGTH, buffsize, CURLFORM_END);

          scurl.bufs.push_back(buffer);
        }

        ::curl_easy_setopt(curl, CURLOPT_HTTPPOST, scurl.post);
      }
    }
    ::curl_easy_setopt(curl, CURLOPT_HTTPHEADER, scurl.headerlist);
    taskinfo_in_out.htasks.push_back(scurl);
    ::curl_multi_add_handle(taskinfo_in_out.hmaster, curl);
  }
}

void pool_impl::_cancel_running_task(CURLM*& hmaster, std::vector<session_curl>& htasks) 
{
  for (std::vector<session_curl>::iterator it = htasks.begin(); it != htasks.end(); it++)
  {
    ::curl_multi_remove_handle(hmaster, (*it).hcurl);
    ::curl_easy_cleanup((*it).hcurl);
    ::curl_formfree((*it).post);
    ::curl_slist_free_all((*it).headerlist);
    for (std::vector<void*>::iterator bfit = (*it).bufs.begin(); bfit != (*it).bufs.end(); bfit++)
      free((*bfit));
  }
  ::curl_multi_cleanup(hmaster);
}

void pool_impl::_stop_thread()
{
#if defined(_WINDOWS_)
  ::SetEvent(m_stop_event);
  if (m_thread && m_thread != INVALID_HANDLE_VALUE)
    ::WaitForSingleObject(m_thread, INFINITE);
  m_thread = NULL;
#elif defined(_MAC_) || defined(__linux__)
  pthread_cond_signal(&m_stop_event);
  pthread_join(m_thread, NULL);
#endif
}
