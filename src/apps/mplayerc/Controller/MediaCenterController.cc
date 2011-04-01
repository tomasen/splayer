#include "StdAfx.h"
#include "MediaCenterController.h"
#include <boost/filesystem.hpp>

//void cDebug(const std::wstring &sDebugInfo, bool bAutoBreak = true)
//{
//  if (::GetStdHandle(STD_OUTPUT_HANDLE) == 0)
//    ::AllocConsole();
//
//  HANDLE h = ::GetStdHandle(STD_OUTPUT_HANDLE);
//
//  if (bAutoBreak)
//  {
//    ::WriteConsole(h, sDebugInfo.c_str(), sDebugInfo.size(), 0, 0);
//    ::WriteConsole(h, L"\n", 1, 0, 0);
//  } 
//  else
//  {
//    ::WriteConsole(h, sDebugInfo.c_str(), sDebugInfo.size(), 0, 0);
//  }
//}

////////////////////////////////////////////////////////////////////////////////
// normal part
MediaCenterController::MediaCenterController()
: m_planestate(FALSE)
{
  m_plane.SetPaintState(FALSE);

  MediaPaths mps;
  MediaPaths::iterator it;
  m_model.FindAll(mps);
  for (it = mps.begin(); it != mps.end(); ++it)
    m_treeModel.addFolder(*it);

  m_model.FindAll(m_mediadata);
}

MediaCenterController::~MediaCenterController()
{
}

////////////////////////////////////////////////////////////////////////////////
// GUI control
//void MediaCenterController::AddMediaToDB(MediaData& data)
//{
//  m_model.Add(data);  
//}
//
//void MediaCenterController::AddMediaToTree(MediaData& data)
//{
//  // add data to tree
//  m_folderTree.addFile(data.path, data.filename);
//
//  // we only add unique data, so we check it
//  std::map<UILayerBlock*, MediaFindCondition>::iterator itCur = m_medialist.begin();
//  while (itCur != m_medialist.end())
//  {
//    if (itCur->second.filename == data.filename)
//      return;
//
//    ++itCur;
//  }
//
//  // add data to UI
//  RECT rect;
//  UILayerBlock* block;
//  block = m_plane.AddBlock(data.filename, rect);
//  
//  MediaFindCondition c = {data.uniqueid, data.filename};
//  m_medialist[block] = c;
//
//  InvalidateRect(m_hwnd, &rect, FALSE);
//}

//void MediaCenterController::AddMediaPathToDB(MediaPath &mp)
//{
//  using namespace boost::filesystem;
//
//  if (!is_directory(mp.path))
//  {
//    wpath pt(mp.path);
//    mp.path = pt.parent_path().wstring();
//    if (mp.path[mp.path.size() - 1] != L'\\')
//      mp.path += L"\\";
//  }
//  m_model.Add(mp);
//}
//
//void MediaCenterController::AddMediaPathToTree(MediaPath &mp)
//{
//  using namespace boost::filesystem;
//
//  if (!is_directory(mp.path))
//  {
//    wpath pt(mp.path);
//    mp.path = pt.parent_path().wstring();
//    if (mp.path[mp.path.size() - 1] != L'\\')
//      mp.path += L"\\";
//  }
//
//  // analysis the path and add it to the detect_path table
//  std::wstring sTemp(mp.path);
//
//  while (!sTemp.empty() && sTemp != L"\\\\")
//  {
//    // deal with the path
//    if (is_directory(sTemp))
//      m_folderTree.addPath(sTemp);
//
//    // remove the back slash to avoid dead loop
//    if (sTemp[sTemp.size() - 1] == L'\\')
//      sTemp.erase(sTemp.end() - 1, sTemp.end());
//
//    // remove the last suffix
//    if (sTemp.find_last_of(L'\\') != std::wstring::npos)
//      sTemp.erase(sTemp.begin() + sTemp.find_last_of(L'\\') + 1, sTemp.end());
//    else
//      sTemp.clear();
//  }
//}

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

void MediaCenterController::ShowPlane()
{
  m_planestate = TRUE;
  m_plane.SetPaintState(TRUE);
  if (!m_mediadata.empty())
  {
    for (MediaDatas::iterator it=m_mediadata.begin(); it != m_mediadata.end(); ++it)
      m_treeModel.addFile(*it);
    Update();
  }
}

void MediaCenterController::HidePlane()
{
  m_planestate = FALSE;
  m_plane.SetPaintState(FALSE);
  Update();
}

BOOL MediaCenterController::GetPlaneState()
{
  return m_planestate;
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

void MediaCenterController::Update()
{
  ::InvalidateRect(m_hwnd, NULL, TRUE);
  ::PostMessage(m_hwnd, WM_SIZE, NULL, NULL);
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
