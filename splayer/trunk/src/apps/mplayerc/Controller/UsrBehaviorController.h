#ifndef USRBEHAVIORCONTROLLER_H
#define USRBEHAVIORCONTROLLER_H

#include "LazyInstance.h"
#include "../Utils/CriticalSection.h"
#include "../Model/UsrBehaviorData.h"

class UsrBehaviorController:
  public LazyInstanceImpl<UsrBehaviorController>
{
public:
  void AppendEntry(int id, std::wstring data);

  // starting and ending uploading
  void Start();
  void Stop();

  // primary thread logics, should not be called directly
  static void _thread_dispatch(void* param);
  void _thread();

private:

  UsrBehaviorData m_ubhvdata;
  CriticalSection m_cs;
  HANDLE          m_thread;
  HANDLE          m_stopevent;
};


#endif // USRBEHAVIORCONTROLLER_H