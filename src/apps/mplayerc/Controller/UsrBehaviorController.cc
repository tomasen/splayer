#include "StdAfx.h"

#include "UsrBehaviorController.h"
#include "../Utils/CriticalSection.h"


void UsrBehaviorController::AppendBhvEntry(int id, std::wstring data)
{
  AutoCSLock autocslock(m_cs);
  m_ubhvdata.AppendBhvEntry(id, data);
}

void UsrBehaviorController::AppendEnvEntry(std::wstring name, std::wstring data)
{
  AutoCSLock autocslock(m_cs);
  m_ubhvdata.AppendEnvEntry(name, data);
}