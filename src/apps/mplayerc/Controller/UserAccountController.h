#pragma once

#include "LazyInstance.h"
#include <threadhelper.h>
#include "NetworkControlerImpl.h"

class UserAccountController:
  public NetworkControlerImpl,
  public ThreadHelperImpl<UserAccountController>,
  public LazyInstanceImpl<UserAccountController>
{
public:
  UserAccountController();
  ~UserAccountController();

  void _Thread();
};
