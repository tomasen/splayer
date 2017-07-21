#ifndef CONFIG_CTOCPP_H
#define CONFIG_CTOCPP_H

#include "ctocpp.h"
#include "../sinet_dyn/sinet_capi.h"
#include "../sinet/config.h"

class config_ctocpp:
  public ctocpp<config_ctocpp, config, _config_t>
{
public:
  config_ctocpp(_config_t* cfg)
    : ctocpp<config_ctocpp, config, _config_t>(cfg) {}
  virtual ~config_ctocpp() {}

  virtual int get_strvar(int id, std::wstring& strvarout);
  virtual void set_strvar(int id, std::wstring strvarin);
  virtual int remove_strvar(int id);
};

#endif // CONFIG_CTOCPP_H