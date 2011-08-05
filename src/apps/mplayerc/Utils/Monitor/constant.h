#pragma once

namespace MT
{
  enum monitor_type_enum
  {
    monitor_cpu_time,        // (% Processor Time), percentage 0% ~ 100%
    monitor_disk_bytes,      // (Disk bytes/sec)
    monitor_disk_busy_time   // (% Disk Time), busy time percentage 0% ~ 100%
  };
}