#ifndef LAYERED_WIN_UTILS_H
#define LAYERED_WIN_UTILS_H

#include "../Utils/lock.h"

///////////////////////////////////////////////////////////////////////////////
//
//  LayeredWinUtils provides templated wrapper for easier painting on layered
//  windows. It provides an interface DoLayeredPaint(HDC, RECT&); for the
//  upper implementation to take care of painting.
//
//  LayeredWinUtils has internal device-context and bitmap based buffering.
//  When window contents need to be updated, client typically call
//  DoUpdateWindow_() which in turn calls DoLayeredPaint, then DoPaintWindow_().
//  Because the current window content is cached, clients can call SetAlpha,
//  and DoPaintWindow_ to implement alpha fading without redrawing window
//  content.
//
//  Note that the default implementation initializes m_alpha to 0. Which means
//  contents won't be visible after a SetAlpha call plus DoUpdateWindow_.
//
template<class T>
class LayeredWinUtils
{
public:
  LayeredWinUtils(void):
    m_alpha(0),
    m_oldbitmap(NULL),
    m_width(0),
    m_height(0)
  {
    WTL::CDCHandle dc(::GetDC(NULL));
    m_dc.CreateCompatibleDC(dc);
    ::ReleaseDC(NULL, dc);
  }
  virtual ~LayeredWinUtils(void)
  {
    m_dc.SelectBitmap(m_oldbitmap);
  }

  // redraw window contents and display it
  void DoUpdateWindow_()
  {
    T* p = static_cast<T*>(this);

    RECT rc_wnd, rc_client;
    p->GetWindowRect(&rc_wnd);
    p->GetClientRect(&rc_client);

    m_draw_lock.Lock();

    if (rc_wnd.right - rc_wnd.left != m_width ||
      rc_wnd.bottom - rc_wnd.top != m_height)
    {
      m_width = rc_wnd.right - rc_wnd.left;
      m_height = rc_wnd.bottom - rc_wnd.top;
      m_dc.SelectBitmap(m_oldbitmap);
      if (m_bitmap)
        m_bitmap.DeleteObject();
      BITMAPINFO bi = {0};
      bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
      bi.bmiHeader.biWidth    = m_width;
      bi.bmiHeader.biHeight   = -m_height;
      bi.bmiHeader.biPlanes   = 1;
      bi.bmiHeader.biBitCount = 32;
      VOID* bytes = NULL;
      m_bitmap.CreateDIBSection(m_dc, &bi, DIB_RGB_COLORS, &bytes, NULL, 0);
      m_oldbitmap = m_dc.SelectBitmap(m_bitmap);
    }

    m_dc.FillRect(&rc_client, (HBRUSH)::GetStockObject(BLACK_BRUSH));
    p->DoLayeredPaint((HDC)m_dc, rc_client);

    m_draw_lock.Unlock();

    DoPaintWindow_();
  }

  // display a cached version of window contents
  void DoPaintWindow_()
  {
    T* p = static_cast<T*>(this);

    RECT rc_wnd;
    p->GetWindowRect(&rc_wnd);

    // calculate the new window position/size based on the bitmap size
    POINT ptWindowScreenPosition = {rc_wnd.left, rc_wnd.top};
    SIZE szWindow = {rc_wnd.right - rc_wnd.left, rc_wnd.bottom - rc_wnd.top};

    // setup the blend function
    BLENDFUNCTION blendPixelFunction= { AC_SRC_OVER, 0, m_alpha, AC_SRC_ALPHA };
    POINT ptSrc = {0,0}; // start point of the copy from dcMemory to dcScreen

    m_draw_lock.Lock();

    // perform the alpha blend
    BOOL bRet= ::UpdateLayeredWindow(p->m_hWnd, NULL, 
      &ptWindowScreenPosition, &szWindow, m_dc,
      &ptSrc, 0, &blendPixelFunction, ULW_ALPHA);

    m_draw_lock.Unlock();

  }

  // change current alpha value for internal window content to screen
  void SetAlpha(unsigned char alpha)
  {
    m_alpha = alpha;
  }
  unsigned char GetAlpha()
  {
    return m_alpha;
  }

private:
  unsigned char m_alpha;
  WTL::CDC      m_dc;
  WTL::CBitmap  m_bitmap;
  HBITMAP       m_oldbitmap;
  int           m_width;
  int           m_height;
  lock::Mutex   m_draw_lock;
};

#endif // LAYERED_WIN_UTILS_H