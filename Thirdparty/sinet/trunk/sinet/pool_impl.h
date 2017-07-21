#ifndef SINET_POOL_IMPL_H
#define SINET_POOL_IMPL_H

#include "pool.h"

typedef void CURLM;
typedef void CURL;
typedef void* HANDLE;

struct curl_httppost;
struct curl_slist;

namespace sinet
{

class pool_impl:
  public threadsafe_base<pool>
{
public:
  pool_impl(void);
  ~pool_impl(void);

  virtual void execute(refptr<task> task_in);
  virtual void cancel(refptr<task> task_in);
  virtual int is_running(refptr<task> task_in);
  virtual int is_queued(refptr<task> task_in);
  virtual int is_running_or_queued(refptr<task> task_in);
  virtual void clear_all();

  typedef struct _session_curl{
    CURL* hcurl;
    curl_httppost* post;
    curl_httppost* last;
    curl_slist* headerlist;
    std::vector<void*>  bufs;
  }session_curl;

  typedef struct _task_info{
    CURLM* hmaster;
    std::vector<session_curl> htasks;
    int running_handle;
  }task_info;

  // threading details for the pool
#if defined(_WINDOWS_)
  static void _thread_dispatch(void* param);
#elif defined(_MAC_) || defined(__linux__)
  static void* _thread_dispatch(void* param);
#endif
  void _thread();

private:
  // iterates thru refptr<task> and translate them into CURL details
  // called by pool_impl::execute
  void _prepare_task(refptr<task> task_in, task_info& taskinfo_in);
  // tell CURL to stop running tasks
  // called by pool_impl::cancel, pool_impl::clear_all, pool_impl::_clean_finished
  void _cancel_running_task(CURLM*& hmaster, std::vector<session_curl>& htasks);

  // stopping and cleanup master pool thread
  // called by destructor
  void _stop_thread();

#if defined(_WINDOWS_)
  HANDLE  m_thread;
  HANDLE  m_stop_event;
#elif defined(_MAC_) || defined(__linux__)
  pthread_t       m_thread;
  pthread_cond_t  m_stop_event;
#endif

  critical_section                  m_cstasks_running;
  critical_section                  m_cstask_queue;
  critical_section                  m_cstask_finished;
  std::map<refptr<task>, task_info> m_tasks_running;
  std::vector<refptr<task> >        m_task_queue;
  std::vector<refptr<task> >        m_task_finished;
};

} // namespace sinet

#endif // SINET_POOL_IMPL_H
