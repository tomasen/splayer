#include "StdAfx.h"
#include "UsrBehaviorController.h"

UsrBehaviorController::~UsrBehaviorController()
{
  AppendEntry(USRBHV_CLOSESPLAYER, L"");
}

void UsrBehaviorController::AppendEntry(int id, std::wstring data)
{
  AutoCSLock autocslock(m_cs);
  m_ubhvdata.AppendEntry(id, data);
}