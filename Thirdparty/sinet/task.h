#ifndef SINET_TASK_H
#define SINET_TASK_H

#include "api_base.h"
#include "api_refptr.h"
#include "request.h"
#include "task_observer.h"
#include "config.h"

namespace sinet
{

#define taskstatus_initial    0
#define taskstatus_queued     1
#define taskstatus_running    2
#define taskstatus_completed  3
#define taskstatus_canceled   4

//////////////////////////////////////////////////////////////////////////
//
//  task class
//
//    A task can contain any number of request, for the pool to execute.
//    All requests inside a task are executed simultaneously whenever
//    started. There might be internal mechanisms in curl to limit
//    the number of simultaneous connections and reuse them but we are
//    not sure.
//
//    Once the task is being executed by the pool, one should not modify
//    the requests in it, or append, erase requests.
//
class task:
  public base
{
public:
  static refptr<task> create_instance();

  // append a new request to the task
  // @returns request_id (int)
  virtual void append_request(refptr<request> request_in) = 0;
  // erase a request by id
  // @returns 1 if succeeded, 0 if failed
  virtual int erase_request(int request_id) = 0;
  // clear all requests
  virtual void clearall_requests() = 0;
  // get request count
  // @returns request count
  virtual int get_request_count() = 0;
  // get request ids
  // @returns a vector of int containing request ids
  virtual void get_request_ids(std::vector<int>& ids_out) = 0;
  // get request by id
  // @returns request if found, or null if not found
  virtual refptr<request> get_request(int request_id) = 0;

  // set task status, should only be called by the pool
  virtual void set_status(int status) = 0;
  // get task status
  // @returns task status value
  virtual int get_status() = 0;

  // for observer/callback
  virtual void attach_observer(itask_observer* observer_in) = 0;
  virtual void detach_observer() = 0;
  virtual itask_observer* get_observer() = 0;

  // for config definition
  virtual void use_config(refptr<config> config) = 0;
  virtual refptr<config> get_config() = 0;
};

} // namespace sinet


#endif // SINET_TASK_H