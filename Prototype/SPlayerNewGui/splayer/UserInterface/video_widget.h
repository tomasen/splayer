#ifndef VIDEO_WIDGET_H
#define VIDEO_WIDGET_H

#define UWM_FADEIN_CONTROLS   WM_USER+1
#define UWM_FADEOUT_CONTROLS  WM_USER+2

class VideoWidget:
  public CWindowImpl<VideoWidget>
{
public:
  DECLARE_WND_CLASS(L"VideoWidget")

  VideoWidget(void);

  BEGIN_MSG_MAP(VideoWidget)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
    MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
  END_MSG_MAP()

  LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled){ return TRUE; }
  LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

#endif // VIDEO_WIDGET_H