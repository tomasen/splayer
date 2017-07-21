#include "pch.h"
#include "task_ctocpp.h"
#include "request_ctocpp.h"
#include "config_ctocpp.h"

using namespace sinet;

refptr<task> task::create_instance()
{
  _task_t* impl = _task_create_instance();
  if (impl)
    return task_ctocpp::Wrap(impl);
  return NULL;
}

void task_ctocpp::append_request(refptr<request> request_in)
{
  if (_MEMBER_MISSING(struct_, append_request))
    return;
  struct_->append_request(struct_, request_ctocpp::Unwrap(request_in));
}

int task_ctocpp::erase_request(int request_id)
{
  if (_MEMBER_MISSING(struct_, erase_request))
    return 0;
  return struct_->erase_request(struct_, request_id);
}

void task_ctocpp::clearall_requests()
{
  if (_MEMBER_MISSING(struct_, clearall_requests))
    return;
  struct_->clearall_requests(struct_);
}

int task_ctocpp::get_request_count()
{
  if (_MEMBER_MISSING(struct_, get_request_count))
    return 0;
  return struct_->get_request_count(struct_);
}

void task_ctocpp::get_request_ids(std::vector<int>& ids_out)
{
  if (_MEMBER_MISSING(struct_, get_request_ids))
    return;
  _intlist_t _ids_out;
  struct_->get_request_ids(struct_, &_ids_out);
  size_t id_size = _intlist_size(_ids_out);
  ids_out.resize(id_size);
  if (_ids_out)
  {
     memcpy(&ids_out[0], _intlist_get(_ids_out), id_size * sizeof(int));
    _intlist_free(_ids_out);
  }
}

refptr<request> task_ctocpp::get_request(int request_id)
{
  if (_MEMBER_MISSING(struct_, get_request))
    return NULL;
  return request_ctocpp::Wrap(struct_->get_request(struct_, request_id));
}

void task_ctocpp::set_status(int status)
{
  if (_MEMBER_MISSING(struct_, set_status))
    return;
  struct_->set_status(struct_, status);
}

int task_ctocpp::get_status()
{
  if (_MEMBER_MISSING(struct_, get_status))
    return 0;
  return struct_->get_status(struct_);
}

void task_ctocpp::attach_observer(itask_observer* observer_in)
{
  if (_MEMBER_MISSING(struct_, attach_observer))
    return;
  struct_->attach_observer(struct_, observer_in);
}

void task_ctocpp::detach_observer()
{
  if (_MEMBER_MISSING(struct_, detach_observer))
    return;
  struct_->detach_observer(struct_);
}

itask_observer* task_ctocpp::get_observer()
{
  if (_MEMBER_MISSING(struct_, get_observer))
    return NULL;
  return struct_->get_observer(struct_);
}

void task_ctocpp::use_config(refptr<config> config)
{
  if (_MEMBER_MISSING(struct_, use_config))
    return;
  struct_->use_config(struct_, config_ctocpp::Unwrap(config));
}

refptr<config> task_ctocpp::get_config()
{
  if (_MEMBER_MISSING(struct_, get_config))
    return NULL;
  return config_ctocpp::Wrap(struct_->get_config(struct_));
}