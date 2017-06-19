#ifndef REQUEST_CPPTOC_H
#define REQUEST_CPPTOC_H

#include "cpptoc.h"
#include "sinet_capi.h"
#include "../sinet/request.h"

class request_cpptoc:
  public cpptoc<request_cpptoc, request, _request_t>
{
public:
  request_cpptoc(request* cls);
  virtual ~request_cpptoc(void){}
};

#endif // REQUEST_CPPTOC_H