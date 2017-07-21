#include "pch.h"
#include "pool_ctocpp.h"
#include "task_ctocpp.h"

using namespace sinet;

refptr<pool> pool::create_instance()
{
  _pool_t* impl = _pool_create_instance();
  if (impl)
    return pool_ctocpp::Wrap(impl);
  return NULL;
}

void pool_ctocpp::execute(refptr<task> task_in)
{
  if (_MEMBER_MISSING(struct_, execute))
    return;

  struct_->execute(struct_, task_ctocpp::Unwrap(task_in));
}

void pool_ctocpp::cancel(refptr<task> task_in)
{
  if (_MEMBER_MISSING(struct_, cancel))
    return;

  struct_->cancel(struct_, task_ctocpp::Unwrap(task_in));
}

void pool_ctocpp::clear_all()
{
  if (_MEMBER_MISSING(struct_, clear_all))
    return;

  struct_->clear_all(struct_);
}

int pool_ctocpp::is_running(refptr<task> task_in)
{
  if (_MEMBER_MISSING(struct_, is_running))
    return 0;

  return struct_->is_running(struct_, task_ctocpp::Unwrap(task_in));
}

int pool_ctocpp::is_queued(refptr<task> task_in)
{
  if (_MEMBER_MISSING(struct_, is_queued))
    return 0;

  return struct_->is_queued(struct_, task_ctocpp::Unwrap(task_in));
}

int pool_ctocpp::is_running_or_queued(refptr<task> task_in)
{
  if (_MEMBER_MISSING(struct_, is_running_or_queued))
    return 0;

  return struct_->is_running_or_queued(struct_, task_ctocpp::Unwrap(task_in));
}