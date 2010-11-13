#ifndef PLAYLISTPARSER_H
#define PLAYLISTPARSER_H
#include "..\Playlist.h"
//////////////////////////////////////////////////////////////////////////
//
//  PlaylistParser handles file format of various kinds of playlists.
//
class PlaylistParser
{
public:
  bool GetPlaylistItem(std::wstring fn,
                       std::vector<std::wstring>* subs,
                       CPlaylistItem* newPli);
  bool GetPlaylistItem(std::vector<std::wstring>& fns,
                       std::vector<std::wstring>* subs,
                       CPlaylistItem* newPli);
  std::vector<CPlaylistItem> GetPlaylistFromRar(std::wstring fn,
                                                std::vector<std::wstring>* subs);
  std::vector<CPlaylistItem> Parse(std::wstring fn, std::vector<std::wstring>* subs);
  std::vector<CPlaylistItem> Parse(std::vector<std::wstring>& fns,
                                   std::vector<std::wstring>* subs);
  std::vector<CPlaylistItem> ParseBDMVPlayList(std::wstring fn);
  std::vector<CPlaylistItem> ParseMPCPlayList(std::wstring fn);
  std::vector<CPlaylistItem> ParseCUEPlayList(std::wstring fn);
  void MergeList(std::vector<CPlaylistItem>& list, std::vector<CPlaylistItem>& listToAdd);
  bool FindFileInList(CAtlList<CString>& sl, std::wstring fn);
  bool SearchFiles(std::wstring mask, std::vector<std::wstring>& sl);
  std::wstring CombinePath(CPath p, std::wstring fn);
};

#endif // PLAYLISTPARSER_H