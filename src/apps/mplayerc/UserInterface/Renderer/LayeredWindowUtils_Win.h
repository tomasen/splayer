#ifndef LAYEREDWINDOWUTILS_WIN_H
#define LAYEREDWINDOWUTILS_WIN_H

template<class T>
class LayeredWindowUtils
{
public:
  void DoUpdateWindow_()
  {
    T* p = static_cast<T*>(this);

    WTL::CDCHandle dc(::GetDC(NULL));

    RECT rect, rcClient;
    p->GetWindowRect(&rect);
    p->GetClientRect(&rcClient);

    WTL::CDC mdc;
    WTL::CBitmap bmp;
    mdc.CreateCompatibleDC(dc);
    bmp.CreateCompatibleBitmap(dc, rect.right-rect.left, rect.bottom-rect.top);
    ::ReleaseDC (NULL, dc);
    HBITMAP hbmpOld = mdc.SelectBitmap (bmp);

    p->DoLayeredPaint((HDC)mdc, rcClient);

    // calculate the new window position/size based on the bitmap size
    POINT ptWindowScreenPosition = {rect.left, rect.top};
    SIZE szWindow = {rect.right - rect.left, rect.bottom - rect.top};

    // setup the blend function
    BLENDFUNCTION blendPixelFunction= { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    POINT ptSrc = {0,0}; // start point of the copy from dcMemory to dcScreen

    // perform the alpha blend
    BOOL bRet= ::UpdateLayeredWindow(p->m_hWnd, NULL, &ptWindowScreenPosition, &szWindow, mdc,
      &ptSrc, 0, &blendPixelFunction, ULW_ALPHA);

    mdc.SelectBitmap (hbmpOld);

  }

};

#endif // LAYEREDWINDOWUTILS_WIN_H