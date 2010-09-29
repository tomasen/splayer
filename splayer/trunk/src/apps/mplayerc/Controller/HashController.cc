#include "stdafx.h"
#include "HashController.h"

HashController::HashController(void)
{

}

void HashController::SetFileName(const wchar_t* filename)
{
  AutoCSLock lock(m_cs);
  // to do, set current hash to empty
  // set file name
}

std::wstring HashController::GetHash()
{
  AutoCSLock lock(m_cs);
  // to do, return hash if it's set
  // calculate if it's not set
  return L"";
}