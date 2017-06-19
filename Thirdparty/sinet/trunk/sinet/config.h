#ifndef SINET_CONFIG_H
#define SINET_CONFIG_H

#include "api_base.h"
#include "api_refptr.h"

namespace sinet
{

#define CFG_STR_PROXY        1
#define CFG_STR_AGENT        2

class config:
  public base
{
public:
  static refptr<config> create_instance();

  //get / set / remove string vars
  virtual int get_strvar(int id, std::wstring& strvarout) = 0;
  virtual void set_strvar(int id, std::wstring strvarin) = 0;
  virtual int remove_strvar(int id) = 0;
};

} // namespace sinet

#endif // SINET_CONFIG_H