#pragma once

#include "LazyInstance.h"
#include "../Model/MediaComm.h"
#include "../Model/MediaModel.h"
#include "../Model/MediaSpiderDisk.h"
#include "../UserInterface/Renderer/MediaCenterView.h"

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

    void NewDataNotice(MediaDatas& newdata);

    void SetFrame(HWND hwnd);
    // Gui
    HWND GetPlaneWnd();
    void ShowPlane();
    void HidePlane();

    BOOL GetPlaneState();
    void CreatePlane(HWND hwnd, int width, int height, RECT& margin);
    void ListenMsg(MSG* msg);
    void PaintPlane(HDC& dc, RECT rect);
    void CalcOnSize(const RECT& rc);

private:
    MediaCenterView m_plane;
    MediaSpiderDisk m_spider;
    MediaModel m_model;
    HWND m_parentwnd;
    BOOL m_planestate;
    BOOL m_createsuccess;
};