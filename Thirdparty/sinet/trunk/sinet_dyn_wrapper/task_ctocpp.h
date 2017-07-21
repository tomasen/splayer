#ifndef TASK_CTOCPP_H
#define TASK_CTOCPP_H

#include "ctocpp.h"
#include "../sinet_dyn/sinet_capi.h"
#include "../sinet/task.h"

class task_ctocpp:
  public ctocpp<task_ctocpp, task, _task_t>
{
public:
  task_ctocpp(_task_t* tsk)
    : ctocpp<task_ctocpp, task, _task_t>(tsk) {}
  virtual ~task_ctocpp() {}

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
};

#endif // TASK_CTOCPP_H