#ifndef UPDATECONTROLLER_H
#define UPDATECONTROLLER_H

#include <string>
#include <CriticalSection.h>
#include "LazyInstance.h"
#include "windows.h"
#include "NetworkControlerImpl.h"
#include <threadhelper.h>

class UpdateController:
  public NetworkControlerImpl,
  public ThreadHelperImpl<UpdateController>,
  public LazyInstanceImpl<UpdateController>
{
public:
  UpdateController(void);

  void SetHashString(std::wstring str);
  bool CheckUpdateEXEUpdate();
  
  bool UploadCrashDmp(std::wstring file);
  bool UploadPinRenderDeadEnd(std::wstring file);
  bool UploadText2HttpServer(std::wstring str, std::wstring url);

  void Start();
  void _Thread();

private:
  
  int m_localversion;
  int m_localrelease;

  std::wstring m_hashstr;
  HANDLE m_stopevent;
  HANDLE m_thread;
};



#endif // UPDATECONTROLLER_H