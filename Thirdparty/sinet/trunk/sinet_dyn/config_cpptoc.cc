#include "pch.h"
#include "config_cpptoc.h"

using namespace sinet;

SINET_DYN_API _config_t* _config_create_instance()
{
  refptr<config> impl = config::create_instance();
  if (impl.get())
    return config_cpptoc::Wrap(impl);
  return NULL;
}

int SINET_DYN_CALLBACK _get_strvar(struct __config_t* self, int id, _string_t* strvarout)
{
  std::wstring strvar;
  if (config_cpptoc::Get(self)->get_strvar(id, strvar))
  {
    *strvarout = _string_alloc(strvar.c_str());
    return 1;
  }
  return 0;
}

void SINET_DYN_CALLBACK _set_strvar(struct __config_t* self, int id, _string_t strvarin)
{
  config_cpptoc::Get(self)->set_strvar(id, strvarin);
}

int SINET_DYN_CALLBACK _remove_strvar(struct __config_t* self, int id)
{
  return config_cpptoc::Get(self)->remove_strvar(id);
}

config_cpptoc::config_cpptoc(config* cls) :
  cpptoc<config_cpptoc, config, _config_t>(cls)
{
  struct_.struct_.get_strvar    = _get_strvar;
  struct_.struct_.remove_strvar = _remove_strvar;
  struct_.struct_.set_strvar    = _set_strvar;
}