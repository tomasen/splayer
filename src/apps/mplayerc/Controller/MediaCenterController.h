#pragma once

#include "LazyInstance.h"
#include "../Model/MediaComm.h"
#include "../Model/MediaModel.h"
#include "../Model/MediaTreeModel.h"
//#include "../UserInterface/Renderer/ListBlocks.h"
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

   void SetFrame(HWND hwnd);

   void Playback(std::wstring file);

public:
  // Data control
  void SpiderStart();
  void SpiderStop();

private:
  // GUI
  HWND m_hwnd;
  BOOL m_planestate;
  MediaDatas m_mediadata;

  // Data
  MediaModel            m_model;
  MediaTreeModel        m_treeModel;
  MediaCheckDB          m_checkDB;
  MediaSpiderFolderTree m_spider;
};