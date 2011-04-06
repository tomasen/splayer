#ifndef HUD_WIDGET_H
#define HUD_WIDGET_H

#include "hybrid_childwin_utils.h"
#include "layered_win_utils.h"
#include "widget_base.h"

///////////////////////////////////////////////////////////////////////////////
//
//  HudWidget is a sample implementation of a "hud window" typically used in
//  media player software to display some screen header level controls.
//
class HudWidget:
  public HybridChildwinUtils<HudWidget>,
  public LayeredWinUtils<HudWidget>,
  public WidgetBase<HudWidget>,
  public CWindowImpl<HudWidget>
{
public:
  DECLARE_WND_CLASS(L"HudWidget")

  HudWidget(HWND hwnd_parent);
  ~HudWidget();

  BEGIN_MSG_MAP(HudWidget)
    // do not put any message handlers before CHAIN_MSG_MAPs
    CHAIN_MSG_MAP(WidgetBase<HudWidget>)
    CHAIN_MSG_MAP(HybridChildwinUtils<HudWidget>)
    // message handlers goes here
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
  END_MSG_MAP()

  virtual void Paint(CDCHandle dc, RECT& rc_client);
  virtual void PaintBkgnd(CDCHandle dc, RECT& rc_client);
  virtual void AlignToParent(HWND hwnd_parent);

  LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
  int   m_counter;
  int   m_widget_height;
  CFont m_font;
};

#endif // HUD_WIDGET_H