#pragma once

////////////////////////////////////////////////////////////////////////////////
// MediaData and MediaPath
struct MediaData
{
  MediaData() : uniqueid(0), videotime(0) {}

  long long uniqueid;
  std::wstring path;
  std::wstring filename;
  std::wstring thumbnailpath;
  int videotime;
};

struct MediaPath
{
  MediaPath() : uniqueid(0), merit(0) {}

  long long uniqueid;
  std::wstring path;
  int merit;
};

typedef std::vector<MediaData> MediaDatas;
typedef std::vector<MediaPath> MediaPaths;


////////////////////////////////////////////////////////////////////////////////
// Media folder tree related structures
// time_t's unit is second!!!!!!!!!
struct MediaTreeRoot
{
  MediaTreeRoot() {}
};

struct MediaTreeFile
{
  MediaTreeFile() : tFileCreateTime(-1) {}

  std::wstring sFileHash;
  std::wstring sFilename;
  std::wstring sFileUID;  // unique id
  time_t tFileCreateTime; // -1 indicate invalid
};

typedef std::list<MediaTreeFile> MediaTreeFiles;

struct MediaTreeFolder
{
  MediaTreeFolder() : tFolderCreateTime(-1), tNextSpiderInterval(3), nMerit(0) {}

  std::wstring sFolderPath;  // dir's full path, include backslash
  std::wstring sFolderHash;
  time_t tFolderCreateTime;  // -1 indicate invalid
  time_t tNextSpiderInterval;  // 0 indicate spider can start next loop immediate
  int nMerit;  // 0 is the start value, spider can find this folder earlier than others

  MediaTreeFiles lsFiles;
};

typedef std::list<MediaTreeFolder> MediaTreeFolders;