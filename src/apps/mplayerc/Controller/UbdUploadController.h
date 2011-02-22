#ifndef USRUPLOADCONTROLLER_H
#define USRUPLOADCONTROLLER_H

#include "LazyInstance.h"


class UbdUploadController:
  public LazyInstanceImpl<UbdUploadController>
{
public:

  // starting and ending uploading
  void Start();
  void Stop();

  // primary thread logics, should not be called directly
  static void _thread_dispatch(void* param);
  void _thread();

private:

  HANDLE          m_thread;
  HANDLE          m_stopevent;
};


#endif // USRUPLOADCONTROLLER_H