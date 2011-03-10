#pragma once

#include "LazyInstance.h"
#include "../Model/MediaComm.h"
#include "../Model/MediaModel.h"
#include "../Model/MediaSpiderDisk.h"
#include "../UserInterface/Renderer/ListBlocks.h"
#include <map>

class MediaCenterController:
  public LazyInstanceImpl<MediaCenterController>
{
public:
    MediaCenterController();
    ~MediaCenterController();

    void AddMedia(MediaData& data, BOOL todb = TRUE);
    void DelMedia(UILayerBlock* block);
    void Playback(const MediaData& data);
    void Playback(std::wstring file);

    void ClickEvent();
    void GetMediaData(MediaDatas& data, int limit_start, int limit_end);

    void SpiderStart();
    void SpiderStop();
    void AddMediaPath(std::wstring& path);

    // Gui control
    void ShowPlane();
    void HidePlane();

    BOOL GetPlaneState();
    void SetFrame(HWND hwnd);

    void Update();
    ListBlocks m_plane;
private:
    MediaDatas m_mediadata;
    std::map<UILayerBlock*, MediaFindCondition> m_medialist;
    MediaSpiderDisk m_spider;
    MediaModel m_model;
    BOOL m_planestate;
    HWND m_hwnd;
};