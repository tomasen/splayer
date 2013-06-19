#ifndef BITMAP_SVC_H
#define BITMAP_SVC_H

#include "../Controller/mod_inst.h"
#include "../Utils/lock.h"

///////////////////////////////////////////////////////////////////////////////
//
//  BitmapService class act as the paint service provider for the entire
//  program. When necessary client asks BitmapService to paint an image
//  based on "path", which BitmapService will attempt to load from
//  ResourceBridge, and cache it with its parameters.
//
//  When loading the resource, BitmapService perform extra operations such as
//  PNG -> HBITMAP conversion (images are all assumed to be PNGs), and
//  parameter gathering, such as number of cells in one image, and boundary
//  values.
//
//  BitmapService should be the only entry of front-end GUI painting.
//
class BitmapService:
  public ModuleInstanceImpl<BitmapService>
{
public:
  ~BitmapService(void);
  // Paint the image in the given path to the target |hdc| with coordinates. If 
  // more than one cell is available, |cell_idx| will be used to specify which
  // cell to paint.
  // Paint uses AlphaBlend API to paint images, and thus require the desktop to
  // be in 32bit color mode.
  // Depending on image boundary specification, there might be up to 9 calls
  // to AlphaBlend during this call.
  bool Paint(std::wstring path, int cell_idx, HDC hdc, int x, int y, 
             int width, int height);
  // Get parameter of the bitmap
  bool GetParams(std::wstring path, int* total_width, int* total_height,
                 int* cell_height, int* num_cells, int* left_bound,
                 int* top_bound, int* right_bound, int* bottom_bound);

private:
  typedef struct BitmapParameter{
    int total_width;
    int total_height;
    int cell_height;
    int num_cells;
    int left_bound;
    int top_bound;
    int right_bound;
    int bottom_bound;
  };

  bool _ExtractBitmap(std::map<std::wstring, HBITMAP>::iterator &it, 
                      std::wstring &path,
                      std::map<std::wstring, BitmapParameter>::iterator &itp);

  std::map<std::wstring, HBITMAP> m_cache;
  std::map<std::wstring, BitmapParameter> m_paramcache;
  lock::Mutex m_cachelock;
};

#endif // BITMAP_SVC_H