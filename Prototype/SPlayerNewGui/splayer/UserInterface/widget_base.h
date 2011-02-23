#ifndef WIDGET_BASE_H
#define WIDGET_BASE_H

// A simple template to take care of WM_MOUSEACTIVATE. A window implemented
// by WidgetBase will not be activated due to mouse clicks. Notification of
// this message is still supported via OnWidgetMouseActivate(), but you might
// not be really using it.
template <class T>
class WidgetBase
{
public:
  BEGIN_MSG_MAP(WidgetBase)
    MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
  END_MSG_MAP()

  virtual void OnWidgetMouseActivate(){}

  LRESULT OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
  {
    OnWidgetMouseActivate();
    return MA_NOACTIVATE;
  }
};

#endif // WIDGET_BASE_H