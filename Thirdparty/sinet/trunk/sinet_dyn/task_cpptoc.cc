#include "pch.h"
#include "task_cpptoc.h"
#include "request_cpptoc.h"
#include "config_cpptoc.h"

using namespace sinet;

SINET_DYN_API _task_t* _task_create_instance()
{
  refptr<task> impl = task::create_instance();
  if (impl.get())
    return task_cpptoc::Wrap(impl);
  return NULL;
}

void SINET_DYN_CALLBACK _append_request(struct __task_t* self, _request_t* request_in)
{
  task_cpptoc::Get(self)->append_request(request_cpptoc::Unwrap(request_in));
}

int SINET_DYN_CALLBACK _erase_request(struct __task_t* self, int request_id)
{
  return task_cpptoc::Get(self)->erase_request(request_id);
}

void SINET_DYN_CALLBACK _clearall_requests(struct __task_t* self)
{
  task_cpptoc::Get(self)->clearall_requests();
}

int SINET_DYN_CALLBACK _get_request_count(struct __task_t* self)
{
  return task_cpptoc::Get(self)->get_request_count();
}

void SINET_DYN_CALLBACK _get_request_ids(struct __task_t* self, _intlist_t* ids_out)
{
  std::vector<int> _ids_out;
  task_cpptoc::Get(self)->get_request_ids(_ids_out);
  if (_ids_out.size() > 0)
    *ids_out = _intlist_alloc(&_ids_out[0], _ids_out.size());
  else
    *ids_out = NULL;
}

_request_t* SINET_DYN_CALLBACK _get_request(struct __task_t* self, int request_id)
{
  return request_cpptoc::Wrap(task_cpptoc::Get(self)->get_request(request_id));
}

void SINET_DYN_CALLBACK _set_status(struct __task_t* self, int status)
{
  task_cpptoc::Get(self)->set_status(status);
}

int SINET_DYN_CALLBACK _get_status(struct __task_t* self)
{
  return task_cpptoc::Get(self)->get_status();
}

void SINET_DYN_CALLBACK _attach_observer(struct __task_t* self, itask_observer* observer_in)
{
  task_cpptoc::Get(self)->attach_observer(observer_in);
}

void SINET_DYN_CALLBACK _detach_observer(struct __task_t* self)
{
  task_cpptoc::Get(self)->detach_observer();
}

itask_observer* SINET_DYN_CALLBACK _get_observer(struct __task_t* self)
{
  return task_cpptoc::Get(self)->get_observer();
}

void SINET_DYN_CALLBACK _use_config(struct __task_t* self, _config_t* config)
{
  task_cpptoc::Get(self)->use_config(config_cpptoc::Unwrap(config));
}

_config_t* SINET_DYN_CALLBACK _get_config(struct __task_t* self)
{
  return config_cpptoc::Wrap(task_cpptoc::Get(self)->get_config());
}

task_cpptoc::task_cpptoc(task* cls):
cpptoc<task_cpptoc, task, _task_t>(cls)
{
  struct_.struct_.append_request    = _append_request;
  struct_.struct_.attach_observer   = _attach_observer;
  struct_.struct_.clearall_requests = _clearall_requests;
  struct_.struct_.detach_observer   = _detach_observer;
  struct_.struct_.erase_request     = _erase_request;
  struct_.struct_.get_config        = _get_config;
  struct_.struct_.get_observer      = _get_observer;
  struct_.struct_.get_request       = _get_request;
  struct_.struct_.get_request_count = _get_request_count;
  struct_.struct_.get_request_ids   = _get_request_ids;
  struct_.struct_.get_status        = _get_status;
  struct_.struct_.set_status        = _set_status;
  struct_.struct_.use_config        = _use_config;
}