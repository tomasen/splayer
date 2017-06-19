#ifndef POOL_CTOCPP_H
#define POOL_CTOCPP_H

#include "ctocpp.h"
#include "../sinet_dyn/sinet_capi.h"
#include "../sinet/pool.h"

class pool_ctocpp:
  public ctocpp<pool_ctocpp, pool, _pool_t>
{
public:
  pool_ctocpp(_pool_t* plt)
    : ctocpp<pool_ctocpp, pool, _pool_t>(plt) {}
  virtual ~pool_ctocpp() {}

  virtual int is_running(refptr<task> task_in);
  virtual int is_queued(refptr<task> task_in);
  virtual int is_running_or_queued(refptr<task> task_in);

  virtual void execute(refptr<task> task_in);
  virtual void cancel(refptr<task> task_in);
  virtual void clear_all();
};

#endif // POOL_CTOCPP_H