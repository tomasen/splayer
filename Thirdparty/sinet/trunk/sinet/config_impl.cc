#include "pch.h"
#include "config_impl.h"

using namespace sinet;

refptr<config> config::create_instance()
{
  refptr<config> _config(new config_impl());
  return _config;
}


int config_impl::get_strvar(int id, std::wstring& strvarout)
{
  std::map<int, std::wstring>::iterator it = m_strvar.find(id);
  if (it != m_strvar.end())
  {
    strvarout = it->second;
    return 1;
  }
  return 0;
}

void config_impl::set_strvar(int id, std::wstring strvarin)
{
  auto_criticalsection acs(m_csconfig);
  m_strvar[id] = strvarin;
}

int config_impl::remove_strvar(int id)
{
  std::map<int, std::wstring>::iterator it = m_strvar.find(id);
  if (it != m_strvar.end())
  {
    auto_criticalsection acs(m_csconfig);
    m_strvar.erase(it);
    return 1;
  }
  return 0;
}