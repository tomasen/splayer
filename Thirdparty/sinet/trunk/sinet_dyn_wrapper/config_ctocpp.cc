#include "pch.h"
#include "config_ctocpp.h"

using namespace sinet;

refptr<config> config::create_instance()
{
  _config_t* impl = _config_create_instance();
  if (impl)
    return config_ctocpp::Wrap(impl);
  return NULL;
}

int config_ctocpp::get_strvar(int id, std::wstring& strvarout)
{
  if (_MEMBER_MISSING(struct_, get_strvar))
    return 0;
  _string_t _strvarout;
  if (struct_->get_strvar(struct_, id, &_strvarout))
  {
    strvarout = _strvarout;
    _string_free(_strvarout);
    return 1;
  }
  return 0;
}

void config_ctocpp::set_strvar(int id, std::wstring strvarin)
{
  if (_MEMBER_MISSING(struct_, set_strvar))
    return;
  _string_t _strvarin = _string_alloc(strvarin.c_str());
  struct_->set_strvar(struct_, id, _strvarin);
  _string_free(_strvarin);
}

int config_ctocpp::remove_strvar(int id)
{
  if (_MEMBER_MISSING(struct_, remove_strvar))
    return 0;
  struct_->remove_strvar(struct_, id);
  return 1;
}