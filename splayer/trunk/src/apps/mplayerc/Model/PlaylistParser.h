#ifndef PLAYLISTPARSER_H
#define PLAYLISTPARSER_H

#include "PlaylistItem.h"

//////////////////////////////////////////////////////////////////////////
//
//  PlaylistParser handles file format of various kinds of playlists.
//
class PlaylistParser
{
public:
  PlaylistItem* GetPlaylistItem(std::wstring fn, std::vector<std::wstring>* subs);
  PlaylistItem* GetPlaylistItem(std::vector<std::wstring>& fns,
                                 std::vector<std::wstring>* subs);
  std::vector<PlaylistItem> GetPlaylistFromRar(std::wstring fn,
                                                std::vector<std::wstring>* subs);
  std::vector<PlaylistItem> Parse(std::wstring fn, std::vector<std::wstring>* subs);
  std::vector<PlaylistItem> Parse(std::vector<std::wstring>& fns,
                                   std::vector<std::wstring>* subs);
  std::vector<PlaylistItem> ParseBDMVPlayList(std::wstring fn);
  std::vector<PlaylistItem> ParseMPCPlayList(std::wstring fn);
  std::vector<PlaylistItem> ParseCUEPlayList(std::wstring fn);
  void MergeList(std::vector<PlaylistItem>& list, std::vector<PlaylistItem> listToAdd);
  bool FindFileInList(std::vector<std::wstring>& sl, std::wstring fn);
  bool SearchFiles(std::wstring mask, std::vector<std::wstring>& sl);
  //std::wstring CombinePath(CPath p, std::wstring fn);
};

#endif // PLAYLISTPARSER_H