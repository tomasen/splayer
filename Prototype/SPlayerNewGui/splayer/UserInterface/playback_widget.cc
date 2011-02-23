#include "stdafx.h"
#include "playback_widget.h"
#include "bitmap_svc.h"
#include "wtl_aero.h"

PlaybackWidget::PlaybackWidget(HWND hwnd_parent):
  HybridChildwinUtils<PlaybackWidget>(hwnd_parent),
  m_widget_height(::GetSystemMetrics(SM_CXICON)*2),
  m_mouse_iscaptured(false),
  m_counter(0)
{
  SetThemeClassList(L"globals");

  CLogFont lf;
  lf.SetMenuFont();
  lf.SetBold();
  m_font.CreateFontIndirect(&lf);
}

PlaybackWidget::~PlaybackWidget()
{
  if (IsWindow())
    DestroyWindow();
}

void PlaybackWidget::Paint(CDCHandle dc, RECT& rc_client)
{
  BitmapService::GetInstance()->Paint(L"sample_btn2.1_14_12_15_14.png", 0, dc, 
    rc_client.left + ::GetSystemMetrics(SM_CXSMICON)/2, 
    rc_client.top + ::GetSystemMetrics(SM_CXSMICON)/2, 
    rc_client.right - rc_client.left - ::GetSystemMetrics(SM_CXSMICON),
    rc_client.bottom - rc_client.top - ::GetSystemMetrics(SM_CXSMICON));

  HFONT _font = dc.SelectFont(m_font);
  wchar_t str[256];
  _itow_s(m_counter, str, 256, 10);
  dc.SetBkMode(TRANSPARENT);

  // DrawThemeTextEx is supported by Vista and above.
  // When in layered or composited window, regular GDI DrawText won't take care
  // of the alpha channel resulting in some translucent text. DrawThemeText
  // and DrawThemeTextEx are the only text painting APIs (aside from DrawShadowText)
  // that are capable of dealing with the alpha channel when painting.
  //
  // Although layered window is supported by Windows 2000. Our frame implementation
  // will only use layered window in composite desktop mode (aero). Therefore we
  // took the chance to skip lots of validation and hacking, to use Vista+'s
  // DrawThemeTextEx API to do the painting.
  //
  if (IsWorkAsLayered() && IsThemingSupported() && AeroHelper::IsAeroSupported())
  {
    DTTOPTS dtt = {sizeof(DTTOPTS), DTT_TEXTCOLOR|DTT_COMPOSITED, RGB(0,0,0)};
    DrawThemeTextEx(dc, TEXT_BODYTITLE, 0, str, -1, 
      DT_CENTER|DT_VCENTER|DT_SINGLELINE, &rc_client, &dtt);
  }
  else
    dc.DrawText(str, -1, &rc_client, DT_CENTER|DT_VCENTER|DT_SINGLELINE);

  dc.SelectFont(_font);
}

void PlaybackWidget::PaintBkgnd(CDCHandle dc, RECT& rc_client)
{
  RECT rc_parent, rc_this;
  ::GetWindowRect(GetParentWin(), &rc_parent);
  GetWindowRect(&rc_this);

  // paint base window style
  RECT rc_paint = {rc_parent.left - rc_this.left, 
    rc_parent.top - rc_this.top,
    rc_parent.left - rc_this.left + rc_parent.right - rc_parent.left,
    rc_parent.top - rc_this.top + rc_parent.bottom - rc_parent.top};
  BitmapService::GetInstance()->Paint(L"window_style.3_10_20_10_10.png", 
    (int)::GetProp(GetParentWin(), L"__FRAME_STYLE"),
    dc, rc_paint.left, rc_paint.top, rc_paint.right - rc_paint.left,
    rc_paint.bottom - rc_paint.top);
}

void PlaybackWidget::AlignToParent(HWND hwnd_parent)
{
  RECT rc;

  // Given the HWND of the parent window, the widget here is free to do whatever
  // repositioning and alignment you wish. 

  // Note that this method might be called very often.
  // Events such as frame window moving and resizing, activation all requires
  // The child widgets to realign and reposition.
  // Therefore this method should be kept as simple as possible.

  if (IsWorkAsLayered())
    ::GetWindowRect(hwnd_parent, &rc);
  else
    ::GetClientRect(hwnd_parent, &rc);

  // Required to deal with Windows actively put the frame window on top
  // during WM_ACTIVATE and similar messages. This will attempt to keep
  // our widget on top of the frame.
  if (IsWindow())
    SetWindowPos(HWND_TOP, 
      rc.left, rc.top + rc.bottom - rc.top - m_widget_height, 
      rc.right - rc.left, m_widget_height, SWP_NOACTIVATE);

  Refresh();
}

LRESULT PlaybackWidget::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  SetTimer(1, 1000);
  bHandled = FALSE;

  // required for painting in layered window or composited window (aero)
  if (IsThemingSupported())
    OpenThemeData();
  return 0;
}

LRESULT PlaybackWidget::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  switch (wParam)
  {
  case 1:
    m_counter++;
    Refresh();
    break;
  }
  return 0;
}

LRESULT PlaybackWidget::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  if (!IsWorkAsLayered())
    return 0;

  // Sample implementation to support mouse dragging of this widget
  GetCursorPos(&m_captured_mousepos);
  RECT rc;
  GetWindowRect(&rc);
  m_captured_offset.x = m_captured_mousepos.x - rc.left;
  m_captured_offset.y = m_captured_mousepos.y - rc.top;
  SetCapture();
  m_mouse_iscaptured = true;
  return 0;
}

LRESULT PlaybackWidget::OnLButtonUp( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
  // Sample implementation to support mouse dragging of this widget
  m_mouse_iscaptured = false;
  ReleaseCapture();
  return 0;
}

LRESULT PlaybackWidget::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  // Sample implementation to support mouse dragging of this widget
  if (!m_mouse_iscaptured)
    return 0;

  POINT pt;
  GetCursorPos(&pt);

  int offset_x = pt.x - m_captured_mousepos.x;
  int offset_y = pt.y - m_captured_mousepos.y;

  SetWindowPos(NULL, pt.x - m_captured_offset.x,
    pt.y - m_captured_offset.y,
    0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

  return 0;
}