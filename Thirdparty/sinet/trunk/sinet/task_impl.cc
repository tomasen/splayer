#include "pch.h"
#include "task_impl.h"
#include <curl/curl.h>

using namespace sinet;

refptr<task> task::create_instance()
{
  refptr<task> _task(new task_impl());
  return _task;
}

task_impl::task_impl(void):
  m_status(taskstatus_initial),
  m_current_id(0),
  m_observer(NULL)
{
}

task_impl::~task_impl(void)
{
}

void task_impl::append_request(refptr<request> request_in)
{
  auto_criticalsection acs(m_csrequests);
  m_requests[m_current_id++] = request_in;
}

int task_impl::erase_request(int request_id)
{
  auto_criticalsection acs(m_csrequests);
  std::map<int, refptr<request> >::iterator it =
    m_requests.find(request_id);
  if (it != m_requests.end())
  {
    m_requests.erase(it);
    return 1;
  }
  return 0;
}

void task_impl::clearall_requests()
{
  auto_criticalsection acs(m_csrequests);
  m_requests.clear();
}

int task_impl::get_request_count()
{
  return m_requests.size();
}

void task_impl::get_request_ids(std::vector<int>& ids_out)
{
  ids_out.clear();
  for (std::map<int, refptr<request> >::iterator it = m_requests.begin();
       it != m_requests.end(); it++)
    ids_out.push_back(it->first);
}

refptr<request> task_impl::get_request(int request_id)
{
  std::map<int, refptr<request> >::iterator it = m_requests.find(request_id);
  if (it != m_requests.end())
    return it->second;
  return refptr<request>();
}

void task_impl::set_status(int status)
{
  if (status >= taskstatus_initial &&
      status <= taskstatus_canceled)
    m_status = status;
}

int task_impl::get_status()
{
  return m_status;
}

void task_impl::attach_observer(itask_observer* observer_in)
{
  m_observer = observer_in;
}

void task_impl::detach_observer()
{
  auto_criticalsection acs(m_csrequests);
  m_observer = NULL;
}

itask_observer* task_impl::get_observer()
{
  return m_observer;
}

void task_impl::use_config(refptr<config> config)
{
  m_config = config;
}

refptr<config> task_impl::get_config()
{
  return m_config;
}