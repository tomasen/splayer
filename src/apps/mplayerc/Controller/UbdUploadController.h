#ifndef USRUPLOADCONTROLLER_H
#define USRUPLOADCONTROLLER_H

#include "LazyInstance.h"
#include <threadhelper.h>

class UbdUploadController:
  public ThreadHelperImpl<UbdUploadController>,
  public LazyInstanceImpl<UbdUploadController>
{
public:

  // starting and ending uploading
  void Start();
  // primary thread logics, should not be called directly
  static void _thread_dispatch(void* param);
  void _Thread();
};


#endif // USRUPLOADCONTROLLER_H