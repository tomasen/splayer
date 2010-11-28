#ifndef OSDCONTROLLER_H
#define OSDCONTROLLER_H

#include "LazyInstance.h"
#include <CriticalSection.h>

class OSDController:
  public LazyInstanceImpl<OSDController>
{
public:
  void SetWindow(HWND hwnd);
  void SendMessage(std::wstring msg);
  void SendMessage(unsigned int res_id, ...);
  std::wstring PopMessage();

private:
  CriticalSection m_cs;
  std::queue<std::wstring>  m_msg_list;
};

#endif // OSDCONTROLLER_H