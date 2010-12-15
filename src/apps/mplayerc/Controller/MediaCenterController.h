#pragma once

#include "LazyInstance.h"
#include <Model/MediaComm.h>
#include <Model/MediaModel.h>
#include <Model/MediaSpiderDisk.h>
#include <UserInterface/Renderer/MediaCenterView.h>

class MediaCenterController:
  public LazyInstanceImpl<MediaCenterController>
{
public:
    MediaCenterController();
    ~MediaCenterController();

    void AddMedia(const MediaData& data);
    void Playback(const MediaData& data);

    void GetMediaData(MediaDatas& data, int limit_start, int limit_end);

    void SpiderStart();
    void SpiderStop();

    void ShowGUI();
    void HideGUI();
    void NewDataNotice(MediaDatas& newdata);

private:
    MediaSpiderDisk m_spider;
    MediaModel m_model;
    MediaCenterView m_gui;
};