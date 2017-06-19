#ifndef TASK_CPPTOC_H
#define TASK_CPPTOC_H

#include "cpptoc.h"
#include "sinet_capi.h"
#include "../sinet/task.h"

class task_cpptoc:
  public cpptoc<task_cpptoc, task, _task_t>
{
public:
  task_cpptoc(task* cls);
  virtual ~task_cpptoc(void){}
};

#endif // TASK_CPPTOC_H