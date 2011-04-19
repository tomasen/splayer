#include "stdafx.h"
#include "bitmap_svc.h"
#include "../Controller/resource_bridge.h"
#include "../Utils/png_utils.h"
#include "../Utils/strings.h"

BitmapService::~BitmapService(void)
{
  for (std::map<std::wstring, HBITMAP>::iterator it =
    m_cache.begin(); it != m_cache.end(); it++)
    ::DeleteObject(it->second);
}

void ParsePath(std::wstring& path_in, int& num_cells, int& left_bound, int& top_bound,
               int& right_bound, int& bottom_bound)
{
  // path format is:
  //   test_bkgnd.2_2_2_2_2.png
  //   splash.png
  //
  // the path contains 5 delimited int values, which will be parsed into
  // |num_cells|    => number of image cells (placed vertically)
  // |left_bound|   => number of pixels of the left border
  // |top_bound|    => number of pixels of the top border
  // |right_bound|  => number of pixels of the right border
  // |bottom_bound| => number of pixels of the bottom border

  num_cells     = 1;
  left_bound    = 0;
  top_bound     = 0;
  right_bound   = 0;
  bottom_bound  = 0;

  std::vector<std::wstring> dot_dem;
  string_util::Split(path_in.c_str(), L".", dot_dem);

  if (dot_dem.size() < 3)
    return;

  std::vector<std::wstring> params;
  string_util::Split(dot_dem[1].c_str(), L"_", params);

  if (params.size() >= 1)
    num_cells     = _wtoi(params[0].c_str());
  if (params.size() >= 2)
    left_bound    = _wtoi(params[1].c_str());
  if (params.size() >= 3)
    top_bound     = _wtoi(params[2].c_str());
  if (params.size() >= 4)
    right_bound   = _wtoi(params[3].c_str());
  if (params.size() >= 5)
    bottom_bound  = _wtoi(params[4].c_str());
}

void PaintWorker(HBITMAP bmp, int bmp_width, int bmp_height, int cell_height, int num_cells,
                 int left_bound, int top_bound, int right_bound, int bottom_bound, 
                 int cell_idx, HDC hdc, int x, int y, int width, int height)
{
  if (cell_idx >= num_cells || cell_idx < 0)
    return;

  int y_start = cell_idx * cell_height;
  int y_end   = (cell_idx + 1) * cell_height;

  CDC dcImage;
  dcImage.CreateCompatibleDC(hdc);
  HBITMAP _bmp = dcImage.SelectBitmap(bmp);
  BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};

  if (left_bound > 0 && top_bound > 0)      // draw top-left corner
    ::AlphaBlend(hdc, x, y, left_bound, top_bound, dcImage, 0, y_start, 
      left_bound, top_bound, bf);

  if (right_bound > 0 && top_bound > 0)     // draw top-right corner
    ::AlphaBlend(hdc, x + width - right_bound, y , right_bound, top_bound, dcImage,
      bmp_width - right_bound, y_start, right_bound, top_bound, bf);

  if (left_bound > 0 && bottom_bound > 0)   // draw bottom-left corner
    ::AlphaBlend(hdc, x, y + height - bottom_bound, left_bound, bottom_bound,
      dcImage, 0, y_end - bottom_bound, 
      left_bound, bottom_bound, bf);

  if (right_bound > 0 && bottom_bound > 0)  // draw bottom-right corner
    ::AlphaBlend(hdc, x + width - right_bound, y + height - bottom_bound, right_bound,
      bottom_bound, dcImage, bmp_width - right_bound, 
      y_end - bottom_bound,
      right_bound, bottom_bound, bf);

  // we are going to stretch for these sides
  ::SetStretchBltMode(hdc, HALFTONE);
  ::SetBrushOrgEx(hdc, 0, 0, NULL);

  if (left_bound > 0)   // draw left side
    ::AlphaBlend(hdc, x, y + top_bound, left_bound, height - top_bound - bottom_bound,
      dcImage, 0, y_start + top_bound, 
      left_bound, y_end - y_start - top_bound - bottom_bound, bf);

  if (right_bound > 0)  // draw right side
    ::AlphaBlend(hdc, x + width - right_bound, y + top_bound, right_bound,
      height - bottom_bound - top_bound, dcImage,
      bmp_width - right_bound, y_start + top_bound, 
      right_bound, y_end - y_start - bottom_bound - top_bound, bf);

  if (top_bound > 0)    // draw top side
    ::AlphaBlend(hdc, x + left_bound, y, width - right_bound - left_bound, top_bound,
      dcImage, left_bound, y_start, bmp_width - left_bound - right_bound, top_bound, bf);

  if (bottom_bound > 0) // draw bottom side
    ::AlphaBlend(hdc, x + left_bound, y + height - bottom_bound, 
      width - right_bound - left_bound, bottom_bound,
      dcImage, left_bound, y_end - bottom_bound,
      bmp_width - left_bound - bottom_bound, bottom_bound, bf);

  // center
  ::AlphaBlend(hdc, x + left_bound, y + top_bound, width - right_bound - left_bound,
    height - bottom_bound - top_bound, dcImage,
    left_bound, y_start + top_bound, bmp_width - left_bound - right_bound,
    y_end - y_start - bottom_bound - top_bound, bf);

  dcImage.SelectBitmap(_bmp);

}

bool BitmapService::Paint(std::wstring path, int cell_idx, HDC hdc, 
                          int x, int y, int width, int height)
{
  std::map<std::wstring, HBITMAP>::iterator it;
  std::map<std::wstring, BitmapParameter>::iterator itp;

  if (!_ExtractBitmap(it, path, itp))
    return false;

  PaintWorker(it->second, itp->second.total_width, itp->second.total_height,
    itp->second.cell_height, itp->second.num_cells, itp->second.left_bound,
    itp->second.top_bound, itp->second.right_bound, itp->second.bottom_bound,
    cell_idx, hdc, x, y, width, height);

  return true;
}

bool BitmapService::GetParams(std::wstring path, int* total_width, int* total_height,
                              int* cell_height, int* num_cells, int* left_bound,
                              int* top_bound, int* right_bound, int* bottom_bound)
{
  std::map<std::wstring, HBITMAP>::iterator it;
  std::map<std::wstring, BitmapParameter>::iterator itp;

  if (!_ExtractBitmap(it, path, itp))
    return false;

  if (total_width)
    *total_width = itp->second.total_width;
  if (total_height)
    *total_height = itp->second.total_height;
  if (cell_height)
    *cell_height = itp->second.cell_height;
  if (num_cells)
    *num_cells = itp->second.num_cells;
  if (left_bound)
    *left_bound = itp->second.left_bound;
  if (top_bound)
    *top_bound = itp->second.top_bound;
  if (right_bound)
    *right_bound = itp->second.right_bound;
  if (bottom_bound)
    *bottom_bound = itp->second.bottom_bound;

  return true;
}

bool BitmapService::_ExtractBitmap(std::map<std::wstring, HBITMAP>::iterator &it,
                                   std::wstring &path,
                                   std::map<std::wstring, BitmapParameter>::iterator &itp)
{
  // locate the bitmap in cache
  m_cachelock.Lock();
  it = m_cache.find(path);
  itp = m_paramcache.find(path);
  m_cachelock.Unlock();
  // bitmap not found
  if (it == m_cache.end())
  {
    // load from resource bridge
    std::vector<unsigned char> buf = 
      ResourceBridge::GetInstance()->LoadBuffer(path.c_str());

    if (buf.empty())
      return false;

    HBITMAP bmp = png_utils::LoadBuffer(buf);
    if (!bmp)
      return false;

    // cache it
    lock::AutoLock alock(m_cachelock);
    m_cache[path] = bmp;
    it = m_cache.find(path);
  }
  // parameter not found
  if (itp == m_paramcache.end())
  {
    BitmapParameter bp;
    ParsePath(path, bp.num_cells, 
      bp.left_bound, bp.top_bound, bp.right_bound, bp.bottom_bound);
    BITMAP bitmap;
    if (GetObject(it->second, sizeof(BITMAP), &bitmap))
    {
      bp.total_width  = bitmap.bmWidth;
      bp.total_height = bitmap.bmHeight;
      bp.cell_height  = bitmap.bmHeight / bp.num_cells;
    }
    lock::AutoLock alock(m_cachelock);
    m_paramcache[path] = bp;
    itp = m_paramcache.find(path);
  }
  return true;
}