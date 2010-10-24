#ifndef OSDCONTROLLER_H
#define OSDCONTROLLER_H

#include "LazyInstance.h"
#include "../Utils/CriticalSection.h"

class OSDController:
  public LazyInstanceImpl<OSDController>
{
public:
  void SetWindow(HWND hwnd);
  void SendMessage(std::wstring msg);
  std::wstring PopMessage();

private:
  CriticalSection m_cs;
  std::queue<std::wstring>  m_msg_list;
};

#endif // OSDCONTROLLER_H