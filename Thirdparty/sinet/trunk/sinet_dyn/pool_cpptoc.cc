#include "pch.h"
#include "pool_cpptoc.h"
#include "task_cpptoc.h"

using namespace sinet;

SINET_DYN_API _pool_t* _pool_create_instance()
{
  refptr<pool> impl = pool::create_instance();
  if (impl.get())
    return pool_cpptoc::Wrap(impl);
  return NULL;
}

void SINET_DYN_CALLBACK _execute(struct __pool_t* self, _task_t* task)
{
  refptr<sinet::task> task_in = task_cpptoc::Unwrap(task);
  pool_cpptoc::Get(self)->execute(task_in);
}

void SINET_DYN_CALLBACK _cancel(struct __pool_t* self, _task_t* task)
{
  refptr<sinet::task> task_in = task_cpptoc::Unwrap(task);
  pool_cpptoc::Get(self)->cancel(task_in);
}

void SINET_DYN_CALLBACK _clear_all(struct __pool_t* self)
{
  pool_cpptoc::Get(self)->clear_all();
}

int SINET_DYN_CALLBACK _is_running(struct __pool_t* self, _task_t* task)
{
  refptr<sinet::task> task_in = task_cpptoc::Unwrap(task);
  return pool_cpptoc::Get(self)->is_running(task_in);
}

int SINET_DYN_CALLBACK _is_queued(struct __pool_t* self, _task_t* task)
{
  refptr<sinet::task> task_in = task_cpptoc::Unwrap(task);
  return pool_cpptoc::Get(self)->is_queued(task_in);
}

int SINET_DYN_CALLBACK _is_running_or_queued(struct __pool_t* self, _task_t* task)
{
  refptr<sinet::task> task_in = task_cpptoc::Unwrap(task);
  return pool_cpptoc::Get(self)->is_running_or_queued(task_in);
}

pool_cpptoc::pool_cpptoc(pool* cls) :
  cpptoc<pool_cpptoc, pool, _pool_t>(cls)
{
  struct_.struct_.execute                = _execute;
  struct_.struct_.cancel                 = _cancel;
  struct_.struct_.clear_all              = _clear_all;
  struct_.struct_.is_running             = _is_running;
  struct_.struct_.is_queued              = _is_queued;
  struct_.struct_.is_running_or_queued   = _is_running_or_queued;
}