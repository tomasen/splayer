#ifndef USRBEHAVIORCONTROLLER_H
#define USRBEHAVIORCONTROLLER_H

#include "LazyInstance.h"
#include "../Utils/CriticalSection.h"
#include "../Model/UsrBehaviorData.h"

class UsrBehaviorController:
  public LazyInstanceImpl<UsrBehaviorController>
{
public:
  void AppendBhvEntry(int id, std::wstring data);
  void AppendEnvEntry(std::wstring name, std::wstring data);
private:

  UsrBehaviorData m_ubhvdata;
  CriticalSection m_cs;
};


#endif // USRBEHAVIORCONTROLLER_H