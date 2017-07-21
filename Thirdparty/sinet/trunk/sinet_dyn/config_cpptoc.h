#ifndef CONFIG_CPPTOC_H
#define CONFIG_CPPTOC_H

#include "cpptoc.h"
#include "sinet_capi.h"
#include "../sinet/config.h"

class config_cpptoc:
  public cpptoc<config_cpptoc, config, _config_t>
{
public:
  config_cpptoc(config* cls);
  virtual ~config_cpptoc(void){}
};

#endif // CONFIG_CPPTOC_H