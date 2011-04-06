#pragma once

#include "MediaComm.h"
#include "MediaModel.h"

class MediaTreeModel
{
public:
  MediaTreeModel();
  ~MediaTreeModel();

public:
  MediaTreeFolders mediaTreeFolders() const;

public:
  void addFolder(const MediaPath &mp);
  void addFile(const MediaData &md);
  void increaseMerit(const std::wstring &sPath);
  void setNextSpiderInterval(const std::wstring &sPath, time_t tInterval);
  void saveToDB();

  void splitPath(const std::wstring &sPath, std::vector<std::wstring> &vtResult);

protected:
  void assignMerit(const MediaPath &mp);
  std::wstring makePathPreferred(const std::wstring &sPath);

private:
  static MediaTreeFolders m_lsFolderTree;
  MediaModel       m_model;
};
