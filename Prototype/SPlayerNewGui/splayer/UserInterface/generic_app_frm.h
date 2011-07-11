#ifndef GENERIC_APP_FRAME_H
#define GENERIC_APP_FRAME_H

#ifndef __ATLFRAME_H__
#error generic_app_frm.h requires atlframe.h to be included first
#endif // __ATLFRAME_H__

#ifndef WTL_AERO_H
#error generic_app_frm.h requires wtl_aero.h to be included first
#endif // WTL_AERO_H

template<class T>
class GenericAppFrame:
  public CAeroExFrameImpl<T>
{
  typedef CAeroExFrameImpl<T> _baseClass;
public:
  GenericAppFrame(void):
    m_enable_aeroframe(true),
    m_frame_roundness(HIWORD(GetDialogBaseUnits())/2)
  {
  }

  BEGIN_MSG_MAP(GenericAppFrame)
    if(IsAeroFrameEnabled() && DwmDefWindowProc(hWnd, uMsg, wParam, lParam, &lResult))
      return TRUE;
    else
    {
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      MESSAGE_HANDLER(WM_NCPAINT, OnNcPaint)
      MESSAGE_HANDLER(WM_NCACTIVATE, OnNcActivate)
      MESSAGE_HANDLER(WM_SETTEXT, OnSetText)
      MESSAGE_HANDLER(WM_SIZE, OnSize)
      MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
    }
    CHAIN_MSG_MAP(_baseClass)
  END_MSG_MAP()

  // must be implemented in derived client to support painting of
  // the window frame
  virtual void PaintFrame(HDC hdc, LPRECT rcPaint) = 0;

  void EnableAeroFrame(bool enable)
  {
    m_enable_aeroframe = enable;
    if (IsWindow())
      ApplyFrameStyle();
  }
  bool IsAeroFrameEnabled()
  {
    return m_enable_aeroframe && IsCompositionEnabled();
  }

  void SetFrameRoundness(int roundness)
  {
    m_frame_roundness = roundness;
  }
  int GetFrameRoundness()
  {
    return m_frame_roundness;
  }

  // client who implemented the frame must call this method to force repaint 
  // of the frame (non-client) area
  void RedrawFrame()
  {
    SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_FRAMECHANGED);
  }

private:
  LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
  {
    SetMenu(NULL);
    ApplyFrameStyle();

    bHandled = FALSE;
    return 0;
  }

  LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
  {
    ScopedRedrawLock lock(m_hWnd);
    return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
  }

  LRESULT OnSetText(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
  {
    ScopedRedrawLock lock(m_hWnd);
    return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
  }

  LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
  {
    if (!IsAeroFrameEnabled())
    {
      //////////////////////////////////////////////////////////////////////////
      // get window rect and client rect relative to window
      CRect window_rect;
      GetWindowRect(&window_rect);
      CRect rcClient;
      GetWindowRect(&rcClient);
      rcClient.right -= rcClient.left;
      rcClient.bottom -= rcClient.top;
      rcClient.top = rcClient.left = 0;
      rcClient.left += GetSystemMetrics(SM_CXFRAME);
      rcClient.right -= GetSystemMetrics(SM_CXFRAME);
      rcClient.top += GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYCAPTION);
      rcClient.bottom -= GetSystemMetrics(SM_CYFRAME);

      CDCHandle dc;
      if(wParam == 1 || IsCompositionEnabled())
        dc = GetWindowDC();
      else
      {
        HRGN rgn = (HRGN) wParam;
        dc = GetDCEx(rgn, DCX_WINDOW|DCX_INTERSECTRGN|0x10000);
      }

      if (dc.m_hDC)
      {
        // exclude client area when updating the window frame
        dc.ExcludeClipRect(&rcClient);
      }

      //////////////////////////////////////////////////////////////////////////
      // actual painting

      window_rect -= window_rect.TopLeft();

      PaintFrame(dc, &window_rect);
  
      ReleaseDC(dc);
    }
    else
    {
      bHandled = FALSE;
    }

    return 0;
  }

  LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
  {
    bHandled = FALSE;
    switch(wParam)
    {
    case SIZE_MINIMIZED:
      break;
    case SIZE_MAXIMIZED:
      if (!IsAeroFrameEnabled())
        ResetWindowRegion(true);
      break;
    default:
      if (!IsAeroFrameEnabled())
        ResetWindowRegion(false);
      break;
    }
    return 0;
  }

  LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
  {
    return TRUE;
  }

protected:

  void ApplyFrameStyle()
  {
    ResetWindowRegion(true);
    RedrawFrame();
  }

  void ResetWindowRegion( bool force )
  {
    // A native frame uses the native window region, and we don't want to mess
    // with it.
    if (force)
      SetWindowRgn(NULL, TRUE);

    if (IsAeroFrameEnabled())
      return;

    // Changing the window region is going to force a paint. Only change the
    // window region if the region really differs.
    HRGN current_rgn = CreateRectRgn(0, 0, 0, 0);
    int current_rgn_result = GetWindowRgn(current_rgn);

    CRect window_rect;
    GetWindowRect(&window_rect);
    HRGN new_region = CreateRectRgn(0, 0, 0, 0);

    CRgn rgnPoly, rgnTL, rgnTR, rgnCircles;

    rgnTL.CreateEllipticRgn(0, 0, m_frame_roundness*2, m_frame_roundness*2);
    // don't know why here, but the region has to be 1 pixel shifted to the right
    // otherwise the round corner will not be displayed correctly
    rgnTR.CreateEllipticRgn(window_rect.Width() - m_frame_roundness * 2 + 1, 
      0, window_rect.Width()+1, m_frame_roundness * 2);

    POINT polyPoints[] = {{0, m_frame_roundness}, {m_frame_roundness, m_frame_roundness},
      {m_frame_roundness, 0}, {window_rect.Width() - m_frame_roundness, 0}, 
      {window_rect.Width() - m_frame_roundness, m_frame_roundness},
      {window_rect.Width(), m_frame_roundness}, {window_rect.Width(), window_rect.Height()},
      {0, window_rect.Height()}};
    rgnPoly.CreatePolygonRgn(polyPoints, sizeof(polyPoints)/sizeof(POINT), ALTERNATE);

    rgnCircles.CreateRectRgn(0, 0, 0, 0);
    rgnCircles.CombineRgn(rgnTL, rgnTR, RGN_OR);

    int nResult = CombineRgn(new_region, rgnCircles, rgnPoly, RGN_OR);

    if (current_rgn_result == ERROR || !EqualRgn(current_rgn, new_region))
      // SetWindowRgn takes ownership of the HRGN created by CreateHRGN.
      SetWindowRgn(new_region, TRUE);
    else
      DeleteObject(new_region);

    DeleteObject(current_rgn);
  }

  int m_device_cap;

private:
  bool  m_enable_aeroframe;
  int   m_frame_roundness;
};

#endif // GENERIC_APP_FRAME_H