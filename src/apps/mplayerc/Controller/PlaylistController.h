#ifndef PLAYLISTCONTROLLER_H
#define PLAYLISTCONTROLLER_H

#include "LazyInstance.h"

class PlaylistController:
  public LazyInstanceImpl<PlaylistController>
{
public:
  PlaylistController(void);
  std::vector<std::wstring> GetListDisplay();

private:
  std::vector<std::wstring> m_list;
};

#endif // PLAYLISTCONTROLLER_H