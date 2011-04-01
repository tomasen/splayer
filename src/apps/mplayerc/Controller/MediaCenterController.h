#pragma once

#include "LazyInstance.h"
#include "../Model/MediaComm.h"
#include "../Model/MediaModel.h"
#include "../Model/MediaTreeModel.h"
#include "../UserInterface/Renderer/ListBlocks.h"
#include "MediaCheckDB.h"
#include "MediaSpiderFolderTree.h"
#include <map>

class MediaCenterController:
  public LazyInstanceImpl<MediaCenterController>
{
public:
  MediaCenterController();
  ~MediaCenterController();

public:
  // Gui control, should not for other use
  //void AddMediaToDB(MediaData& data);  // file info, to the media_data table
  //void AddMediaToTree(MediaData& data);  // file info, to the media_data table
  //void AddMediaPathToDB(MediaPath &mp);  // path info, to the detect_path table
  //void AddMediaPathToTree(MediaPath &mp);  // path info, to the detect_path table
  void DelMedia(UILayerBlock* block);
  void Playback(const MediaData& data);
  void Playback(std::wstring file);

  void ClickEvent();
  void GetMediaData(MediaDatas& data, int limit_start, int limit_end);

  void ShowPlane();
  void HidePlane();

  BOOL GetPlaneState();
  void SetFrame(HWND hwnd);

  void Update();

public:
  // Data control
  void SpiderStart();
  void SpiderStop();

private:
  // GUI
  HWND m_hwnd;
  BOOL m_planestate;
  MediaDatas m_mediadata;
  std::map<UILayerBlock*, MediaFindCondition> m_medialist;

public:
  ListBlocks m_plane;

  // Data
  MediaModel            m_model;
  MediaTreeModel        m_treeModel;
  MediaCheckDB          m_checkDB;
  MediaSpiderFolderTree m_spider;
};