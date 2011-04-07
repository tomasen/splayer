#include "stdafx.h"
#include "hud_widget.h"
#include "bitmap_svc.h"

HudWidget::HudWidget(HWND hwnd_parent):
  HybridChildwinUtils<HudWidget>(hwnd_parent),
  m_widget_height(::GetSystemMetrics(SM_CXICON)),
  m_counter(0)
{
  CLogFont lf;
  lf.SetMenuFont();
  lf.SetBold();
  m_font.CreateFontIndirect(&lf);
}

HudWidget::~HudWidget()
{
  if (IsWindow())
    DestroyWindow();
}

void HudWidget::Paint(CDCHandle dc, RECT& rc_client)
{
  BitmapService::GetInstance()->Paint(L"shade.1_0_12.png", 0, dc, 
    rc_client.left, 
    rc_client.top, 
    rc_client.right - rc_client.left,
    rc_client.bottom - rc_client.top);
  BitmapService::GetInstance()->Paint(L"sample_btn.1_8_6_7_8.png", 0, dc, 
    rc_client.right - 50, rc_client.top, 50, rc_client.bottom - rc_client.top);

  HFONT _font = dc.SelectFont(m_font);
  wchar_t str[256];
  _itow_s(m_counter, str, 256, 10);
  dc.SetBkMode(TRANSPARENT);
  dc.DrawShadowText(str, -1, &rc_client, DT_LEFT|DT_VCENTER|DT_SINGLELINE,
    RGB(250,250,250), RGB(0,0,0), 1, 1);
  dc.SelectFont(_font);
}

void HudWidget::PaintBkgnd(CDCHandle dc, RECT& rc_client)
{
  CBrush brush;
  brush.CreateSolidBrush(RGB(32, 32, 32));
  dc.FillRect(&rc_client, brush);
}

void HudWidget::AlignToParent(HWND hwnd_parent)
{
  RECT rc;

  // Given the HWND of the parent window, the widget here is free to do whatever
  // repositioning and alignment you wish. 

  // Note that this method might be called very often.
  // Events such as frame window moving and resizing, activation all requires
  // The child widgets to realign and reposition.
  // Therefore this method should be kept as simple as possible.

  if (IsWorkAsLayered())
  {
    ::GetWindowRect(hwnd_parent, &rc);
    rc.top += ::GetSystemMetrics(SM_CYFRAME) + ::GetSystemMetrics(SM_CYCAPTION);
    rc.left += ::GetSystemMetrics(SM_CXFRAME);
    rc.right -= ::GetSystemMetrics(SM_CXFRAME);
  }
  else
    ::GetClientRect(hwnd_parent, &rc);

  // Required to deal with Windows actively put the frame window on top
  // during WM_ACTIVATE and similar messages. This will attempt to keep
  // our widget on top of the frame.
  if (IsWindow())
    SetWindowPos(HWND_TOP, 
      rc.left, rc.top, 
      rc.right - rc.left, m_widget_height, SWP_NOACTIVATE);

  Refresh();
}

LRESULT HudWidget::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  SetTimer(1, 1000);
  bHandled = FALSE;
  return 0;
}

LRESULT HudWidget::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
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