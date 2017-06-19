#ifndef SINET_TASK_IMPL_H
#define SINET_TASK_IMPL_H

#include "task.h"

namespace sinet
{

typedef void CURL;

class task_impl:
  public threadsafe_base<task>
{
public:
  task_impl(void);
  ~task_impl(void);

  virtual void append_request(refptr<request> request_in);
  virtual int erase_request(int request_id);
  virtual void clearall_requests();
  virtual int get_request_count();
  virtual void get_request_ids(std::vector<int>& ids_out);
  virtual refptr<request> get_request(int request_id);

  virtual void set_status(int status);
  virtual int get_status();

  virtual void attach_observer(itask_observer* observer_in);
  virtual void detach_observer();
  virtual itask_observer* get_observer();

  virtual void use_config(refptr<config> config);
  virtual refptr<config> get_config();

private:
  int m_status;
  int m_current_id;
  refptr<config>                  m_config;
  itask_observer*                 m_observer;
  critical_section                m_csrequests;
  std::map<int, refptr<request> > m_requests;
};

} // namespace sine t

#endif // SINET_TASK_IMPL_H