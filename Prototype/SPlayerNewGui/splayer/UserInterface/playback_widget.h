#ifndef PLAYBACK_WIDGET_H
#define PLAYBACK_WIDGET_H

#include "hybrid_childwin_utils.h"
#include "layered_win_utils.h"
#include "widget_base.h"

///////////////////////////////////////////////////////////////////////////////
//
//  PlaybackWidget is a customized implementation of "floating tool window"
//  example typically used by media player software as their primary playback
//  control bar.
//
//  This implementation depends on the HybridChildwinUtils and LayeredWinUtils
//  to support reusing as a child-window or floating layered window.
//
class PlaybackWidget:
  public HybridChildwinUtils<PlaybackWidget>,
  public LayeredWinUtils<PlaybackWidget>,
  public WidgetBase<PlaybackWidget>,
  public CThemeImpl<PlaybackWidget>,
  public CWindowImpl<PlaybackWidget>
{
public:
  DECLARE_WND_CLASS(L"PlaybackWidget")

  PlaybackWidget(HWND hwnd_parent);
  virtual ~PlaybackWidget();

  BEGIN_MSG_MAP(PlaybackWidget)
    // do not put any message handlers before CHAIN_MSG_MAPs
    CHAIN_MSG_MAP(WidgetBase<PlaybackWidget>)
    CHAIN_MSG_MAP(HybridChildwinUtils<PlaybackWidget>)
    // message handlers goes here
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
    MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
    MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
    MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
  END_MSG_MAP()

  virtual void Paint(CDCHandle dc, RECT& rc_client);
  virtual void PaintBkgnd(CDCHandle dc, RECT& rc_client);
  virtual void AlignToParent(HWND hwnd_parent);

  LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
  int   m_widget_height;
  POINT m_captured_mousepos;
  POINT m_captured_offset;
  bool  m_mouse_iscaptured;

  int   m_counter;
  CFont m_font;
};

#endif // PLAYBACK_WIDGET_H