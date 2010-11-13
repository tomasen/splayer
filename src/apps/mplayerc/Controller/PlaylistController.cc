#include "StdAfx.h"
#include "PlaylistController.h"
#include "PlayerPreference.h"
#include "SPlayerDefs.h"

PlaylistController::PlaylistController(void)
{
}

std::vector<std::wstring> PlaylistController::GetListDisplay()
{
  m_list = PlayerPreference::GetInstance()->GetStrArray(STRARRAY_PLAYLIST);
  return m_list;
}