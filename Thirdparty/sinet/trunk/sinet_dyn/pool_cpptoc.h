#ifndef SINET_POOL_CPPTOC_H
#define SINET_POOL_CPPTOC_H

#include "cpptoc.h"
#include "sinet_capi.h"
#include "../sinet/pool.h"

class pool_cpptoc:
  public cpptoc<pool_cpptoc, pool, _pool_t>
{
public:
  pool_cpptoc(pool* cls);
  virtual ~pool_cpptoc(void){}
};

#endif // SINET_POOL_CPPTOC_H