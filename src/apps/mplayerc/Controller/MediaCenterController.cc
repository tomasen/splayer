#include "StdAfx.h"
#include "MediaCenterController.h"

MediaCenterController::MediaCenterController() : m_parentwnd(NULL), m_planestate(FALSE)
{

}

MediaCenterController::~MediaCenterController()
{

}

void MediaCenterController::AddMedia(const MediaData& data)
{

}

void MediaCenterController::Playback(const MediaData& data)
{

}

void MediaCenterController::GetMediaData(MediaDatas& data, int limit_start, int limit_end)
{

}

void MediaCenterController::SpiderStart()
{

}

void MediaCenterController::SpiderStop()
{

}

HWND MediaCenterController::GetPlaneWnd()
{
  return m_plane.m_hWnd;
}

void MediaCenterController::NewDataNotice(MediaDatas& newdata)
{

}

BOOL MediaCenterController::GetPlaneState()
{
  return m_planestate;
}

void MediaCenterController::ShowPlane()
{
  m_planestate = TRUE;
}

void MediaCenterController::HidePlane()
{
  m_planestate = FALSE;
}

void MediaCenterController::SetFrame(HWND hwnd)
{
  m_parentwnd = hwnd;
}

void MediaCenterController::PaintPlane(HDC& dc, RECT rect)
{
  WTL::CMemoryDC mdc(dc, rect);
  m_plane.CalcWnd();
  m_plane.MediaListView::DrawListView(mdc, m_plane.GetOffsetVal());
  m_plane.MediaScrollbar::DrawScrollbar(mdc);
}

void MediaCenterController::CalcOnSize(const RECT& rc)
{
  if (m_planestate == FALSE)
    return;

  int width, hight;
  m_plane.MediaListView::SetClientRect(rc);
  m_plane.MediaListView::GetPlantWH(width, hight);
  m_plane.MediaScrollbar::SetListPlane(width, hight);
  m_plane.MediaScrollbar::SetClientRect(rc);
}

void MediaCenterController::CreatePlane(HWND hwnd, int width, int height, RECT& margin)
{  
  SetFrame(hwnd);
  if (!m_parentwnd)
    return;

  m_plane.ShowMediaCenter(m_parentwnd, width, height, margin);
  m_plane.AddListView(hwnd);
  m_plane.AddScrollbar(hwnd);
}

void MediaCenterController::ListenMsg(MSG* msg)
{ 
  if (m_planestate == FALSE)
    return;

  if (!m_parentwnd)
    return;

  LRESULT wtlbHandled;
  m_plane.MediaListView::ProcessWindowMessage(m_parentwnd, msg->message, msg->wParam, msg->lParam, wtlbHandled, 0);
  m_plane.MediaScrollbar::ProcessWindowMessage(m_parentwnd, msg->message, msg->wParam, msg->lParam, wtlbHandled, 0);
}