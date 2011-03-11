#include "StdAfx.h"
#include "MediaCenterController.h"

MediaCenterController::MediaCenterController() :
    m_planestate(FALSE)
{
  m_plane.SetPaintState(FALSE);

  MediaPaths mps;
  MediaPaths::iterator it;
  m_model.FindAll(mps);
  for (it = mps.begin(); it != mps.end(); ++it)
    m_spider.AddDetectPath((*it).path);

  m_model.FindAll(m_mediadata);
}

MediaCenterController::~MediaCenterController()
{
}

void MediaCenterController::AddMediaPath(std::wstring& path)
{
  if (::PathIsDirectory(path.c_str()))
  {
    // directory
    MediaPath mp = {0, path, 0};
    m_model.Add(mp);
  } 
  else
  {
    // file, must detect the path
    ATL::CPath p(path.c_str());
    std::wstring dir = path.substr(0, p.FindFileName());
    if (m_spider.AddDetectPath(dir.c_str()))
    {
      MediaPath mp = {0, dir, 0};
      m_model.Add(mp);
    }
  }
}

void MediaCenterController::AddMedia(MediaData& data, BOOL todb)
{
  RECT rect;
  UILayerBlock* block;
  block = m_plane.AddBlock(data.filename, rect);
  
  MediaFindCondition c = {data.uniqueid, data.filename};
  m_medialist[block] = c;

  if (todb)
  {
    m_model.Add(data);  
    InvalidateRect(m_hwnd, &rect, FALSE);
  }
}

// this design too bad
void MediaCenterController::DelMedia(UILayerBlock* block)
{
  MediaFindCondition c = m_medialist[block];
  m_model.Delete(c);
  m_medialist.erase(block);
}

void MediaCenterController::Playback(std::wstring file)
{
  if (!m_spider.IsSupportExtension(file))
    return;

  AddMediaPath(file);
  std::wstring name(::PathFindFileName(file.c_str()));
  MediaFindCondition mc = {0, name};
  MediaData mdc;
  m_model.FindOne(mdc, mc);
  if (mdc.uniqueid == 0)
  {
    MediaData md = {0, file, name, L"", 0};
    AddMedia(md);
  }
}

void MediaCenterController::Playback(const MediaData& data)
{
  ::MessageBox(m_hwnd, L"playback,ohhh", L"", MB_OK);
}

void MediaCenterController::ClickEvent()
{
  int eventcode = m_plane.SelectBlockClick();

  if (eventcode == 0)
    return;

  else if (eventcode == 1)
  {
    // delete
    UILayerBlock* block = NULL;
    m_plane.GetClickSelBlock(&block);
    DelMedia(block);
    m_plane.DelBlock(block);
  }
  else if (eventcode == 2)
  {
    // playback
    UILayerBlock* block = NULL;
    m_plane.GetClickSelBlock(&block);
    MediaFindCondition c = m_medialist[block];
    MediaData md;
    m_model.FindOne(md, c);
    Playback(md);
  }
}

void MediaCenterController::GetMediaData(MediaDatas& data, int limit_start, int limit_end)
{

}

void MediaCenterController::SpiderStart()
{
  m_spider._Stop();
  m_checkDB._Stop();

  m_spider._Start();
  m_checkDB._Start();  // check the media.db, clean invalid records
}

void MediaCenterController::SpiderStop()
{
  m_spider._Stop();
  m_checkDB._Stop();
}

BOOL MediaCenterController::GetPlaneState()
{
  return m_planestate;
}

void MediaCenterController::ShowPlane()
{
  m_planestate = TRUE;
  m_plane.SetPaintState(TRUE);
  if (!m_mediadata.empty())
  {
    for (MediaDatas::iterator it=m_mediadata.begin(); it != m_mediadata.end(); ++it)
      AddMedia(*it, FALSE);
    Update();
  }
}

void MediaCenterController::Update()
{
  ::InvalidateRect(m_hwnd, NULL, TRUE);
  ::PostMessage(m_hwnd, WM_SIZE, NULL, NULL);
}

void MediaCenterController::HidePlane()
{
  m_planestate = FALSE;
  m_plane.SetPaintState(FALSE);
  Update();
}

void MediaCenterController::SetFrame(HWND hwnd)
{
  m_hwnd = hwnd;

  RECT block;
  RECT margin = {5, 20, 64, 64};
  block.left = 0;block.top = 0;
  block.right = 138;block.bottom = 138;
  m_plane.SetOption(block, margin, 30);
}