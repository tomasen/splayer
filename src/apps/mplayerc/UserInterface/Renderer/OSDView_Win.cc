#include "stdafx.h"
#include "OSDView_Win.h"

void OSDView::Open(HWND hwnd_parent)
{
  if (IsWindow())
  {
    DoUpdateWindow_();
    return;
  }

  Create(hwnd_parent, NULL, 0, 
    WS_POPUP|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, WS_EX_TOPMOST|WS_EX_TOOLWINDOW);
  ModifyStyleEx(0, WS_EX_LAYERED);

  ShowWindow(SW_SHOW);
  DoUpdateWindow_();
}

void OSDView::DoLayeredPaint(WTL::CDCHandle dc, RECT rcclient)
{

}