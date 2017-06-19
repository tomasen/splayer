#ifndef POSTDATA_CPPTOC_H
#define POSTDATA_CPPTOC_H

#include "cpptoc.h"
#include "sinet_capi.h"
#include "../sinet/postdata.h"

class postdata_cpptoc:
  public cpptoc<postdata_cpptoc, postdata, _postdata_t>
{
public:
  postdata_cpptoc(postdata* cls);
  virtual ~postdata_cpptoc(void){}
};

#endif // POSTDATA_CPPTOC_H