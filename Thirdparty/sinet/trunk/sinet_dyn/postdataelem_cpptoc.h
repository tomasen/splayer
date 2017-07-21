#ifndef POSTDATAELEM_CPPTOC_H
#define POSTDATAELEM_CPPTOC_H

#include "cpptoc.h"
#include "sinet_capi.h"
#include "../sinet/postdataelem.h"

class postdataelem_cpptoc:
  public cpptoc<postdataelem_cpptoc, postdataelem, _postdataelem_t>
{
public:
  postdataelem_cpptoc(postdataelem* cls);
  virtual ~postdataelem_cpptoc(void){}
};

#endif // POSTDATAELEM_CPPTOC_H