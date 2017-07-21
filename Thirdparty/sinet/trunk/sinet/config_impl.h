#ifndef SINET_CONFIG_IMPL_H
#define SINET_CONFIG_IMPL_H

#include "config.h"

namespace sinet
{

class config_impl:
  public threadsafe_base<config>
{
public:
  virtual int get_strvar(int id, std::wstring& strvarout);
  virtual void set_strvar(int id, std::wstring strvarin);
  virtual int remove_strvar(int id);

private:
  critical_section            m_csconfig;
  std::map<int, std::wstring> m_strvar;
};

} // namespace sinet

#endif // SINET_CONFIG_IMPL_H