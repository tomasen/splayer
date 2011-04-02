#include "StdAfx.h"
#include "MediaCenterController.h"
#include <boost/filesystem.hpp>

////////////////////////////////////////////////////////////////////////////////
// normal part
MediaCenterController::MediaCenterController()
: m_planestate(FALSE)
{
  MediaPaths mps;
  MediaPaths::iterator it;
  m_model.FindAll(mps);
  for (it = mps.begin(); it != mps.end(); ++it)
    m_treeModel.addFolder(*it);

  //m_model.FindAll(m_mediadata);
}

MediaCenterController::~MediaCenterController()
{
}

void MediaCenterController::Playback(std::wstring file)
{
  if (!m_spider.IsSupportExtension(file))
    return;

  MediaPath mp;
  mp.path = file;
  m_treeModel.addFolder(mp);
  m_treeModel.increaseMerit(mp.path);

  std::wstring name(::PathFindFileName(file.c_str()));
  MediaFindCondition mc = {0, name};
  MediaData mdc;
  m_model.FindOne(mdc, mc);
  if (mdc.uniqueid == 0)
  {
    MediaData md;
    md.path = file;
    md.filename = name;

    m_treeModel.addFile(md);
  }
}

void MediaCenterController::SetFrame(HWND hwnd)
{
  m_hwnd = hwnd;
}


////////////////////////////////////////////////////////////////////////////////
// data control
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
  m_treeModel.saveToDB();
}
