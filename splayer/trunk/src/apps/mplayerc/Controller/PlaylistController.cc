#include "StdAfx.h"
#include "PlaylistController.h"

PlaylistController::PlaylistController(void)
{
  m_list.push_back(L"splayer_1373_48i4r6.dmp");
  m_list.push_back(L"binkw32.dll");
  m_list.push_back(L"CSMX.dll");
  m_list.push_back(L"splayer_1373_48i4r6.dmp");
  m_list.push_back(L"splayer_1373_48i4r6.dmp");
}

std::vector<std::wstring> PlaylistController::GetListDisplay()
{
  return m_list;
}