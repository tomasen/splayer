#include "stdafx.h"
#include "factory.h"
#include <boost/tuple/tuple_comparison.hpp>

std::map<MT::factory::key_type, MT::factory::monitor_ptr > MT::factory::m_stored;

MT::factory::monitor_ptr MT::factory::get_monitor(MT::monitor_type_enum type, const std::wstring& counter_object /* = L"_Total" */)
{
  key_type key = boost::tuples::make_tuple(type, counter_object);
  if (m_stored.find(key) == m_stored.end())
  {
    m_stored[key] = boost::shared_ptr<monitor_base>();
    switch (type)
    {
    case monitor_cpu_time:
      m_stored[key].reset(new cpu(counter_object, L"% Processor Time"));
      break;
      
    case monitor_disk_bytes:
      m_stored[key].reset(new disk(counter_object, L"Disk bytes/sec"));
      break;

    case monitor_disk_busy_time:
      m_stored[key].reset(new disk(counter_object, L"% Disk Time"));
      break;
    }
  }

  return m_stored[key];
}