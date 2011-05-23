#ifndef SPLAYER_WIDGET_HOST_H
#define SPLAYER_WIDGET_HOST_H

#include "playback_widget.h"
#include "hud_widget.h"
#include "video_widget.h"

#include <memory>

///////////////////////////////////////////////////////////////////////////////
//
//  SPlayerWidgetHost is a sample implementation of a "host" of widgets. This
//  can be done inside MainFrame directly. However, there are lots of messages
//  such as WM_SIZING WM_WINDOWPOSCHANGED that may cause z-order chaos, and
//  position/size invalidation.
//
//  We choose to deal with such messages here for house-keeping tasks, so that
//  the real MainFrame class can be kept clean.
//
template <class T>
class SPlayerWidgetHost
{
public:
  // PrepareWidgets should be called once the frame window is ready to have
  // child window being created.
  void PrepareWidgets(HWND hwnd_parent)
  {
    m_hwndparent = hwnd_parent;
    ATLASSERT(m_playback_widget.get() == 0);
    ATLASSERT(m_hud_widget.get() == 0);
    ATLASSERT(m_video_widget.get() == 0);
    m_playback_widget.reset(new PlaybackWidget(hwnd_parent));
    m_hud_widget.reset(new HudWidget(hwnd_parent));
    m_video_widget.reset(new VideoWidget());
    m_video_widget->Create(hwnd_parent);
  }

  // Frame window should use SetWorkAsLayered to tell the widgets below
  // to go into corresponding mode.
  void SetWorkAsLayered(bool aslayered)
  {
    ATLASSERT(m_playback_widget.get() != 0);
    ATLASSERT(m_hud_widget.get() != 0);
    ATLASSERT(m_video_widget.get() != 0);
    m_playback_widget->SetWorkAsLayered(aslayered);
    m_hud_widget->SetWorkAsLayered(aslayered);
    if (m_hud_widget->IsWindow() && m_playback_widget->IsWindow())
      InvalidateWidgetPos(aslayered);
  }

  BEGIN_MSG_MAP(SPlayerWidgetHost)
    MESSAGE_HANDLER(WM_MOVING, OnMovedSized)
    MESSAGE_HANDLER(WM_SIZING, OnMovedSized)
    MESSAGE_HANDLER(WM_MOVE, OnMovedSized)
    MESSAGE_HANDLER(WM_SIZE, OnMovedSized)
    MESSAGE_HANDLER(WM_ACTIVATE, OnMovedSized)          // changing foreground window from another to this frame
    MESSAGE_HANDLER(WM_WINDOWPOSCHANGED, OnMovedSized)  // clicking on frame window caption bar
  END_MSG_MAP()

  LRESULT OnMovedSized(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
  {
    // important to set bHandled to FALSE so that upper implementation still
    // have a chance to process it.
    bHandled = FALSE;
    T* p = static_cast<T*>(this);
    // calling GenericAppFrame::IsAeroFrameEnabled()
    InvalidateWidgetPos(p->IsAeroFrameEnabled());
    return 0;
  }

  void InvalidateWidgetPos(bool workaslayered) 
  {
    if (!m_hud_widget.get())
      return;
    m_hud_widget->AlignToParent(m_hwndparent);

    if (!m_playback_widget.get())
      return;
    m_playback_widget->AlignToParent(m_hwndparent);

    if (!m_video_widget.get())
      return;

    RECT rc_frm_client;
    T* p = static_cast<T*>(this);
    p->GetClientRect(&rc_frm_client);
    if (workaslayered)
    {
      m_video_widget->SetWindowPos(NULL, 0, 0, rc_frm_client.right - rc_frm_client.left,
        rc_frm_client.bottom - rc_frm_client.top, SWP_NOZORDER | SWP_NOACTIVATE);
    }
    else
    {
      RECT rc_hud, rc_playback;
      m_hud_widget->GetClientRect(&rc_hud);
      m_playback_widget->GetClientRect(&rc_playback);

      m_video_widget->SetWindowPos(NULL, 0, rc_hud.bottom, rc_frm_client.right - rc_frm_client.left,
        rc_frm_client.bottom - rc_frm_client.top - rc_hud.bottom - rc_playback.bottom, 
        SWP_NOZORDER | SWP_NOACTIVATE);
    }
  }

protected:
  std::auto_ptr<PlaybackWidget> m_playback_widget;
  std::auto_ptr<HudWidget>      m_hud_widget;
  std::auto_ptr<VideoWidget>    m_video_widget;
  HWND  m_hwndparent;
};

#endif // SPLAYER_WIDGET_HOST_H