#ifndef HYBRID_CHILDWIN_UTILS_H
#define HYBRID_CHILDWIN_UTILS_H

#include <mmsystem.h>

///////////////////////////////////////////////////////////////////////////////
//
//  HybridChildwinUtils provides templated implementation to simplify usage
//  of so called "floating tool window" as both child window inside a frame
//  and floating layered window.
//
//  It takes care of window show/hide, wraps WM_PAINT logic and layered
//  window painting logic by providing "Paint" and "PaintBkgnd" interface to
//  the upper implementation. Note that layered window paint and fade logic
//  depends on LayeredWinUtils, so that the upper implementation must also
//  be implemented using it.
//
//  "AlignToParent" interface is provided for repositioning of the hybrid
//  window. It gets called when necessary. The upper implementation must take
//  care of layered/child mode difference itself.
//
template <class T>
class HybridChildwinUtils
{
public:
  HybridChildwinUtils(HWND hwnd_parent):
    m_hwndparent(hwnd_parent),
    m_work_aslayered(false),
    m_timer(0),
    m_fade_var(0),
    m_fade_step(0)
  {
  }
  virtual ~HybridChildwinUtils()
  {
    if (m_timer)
      ::timeKillEvent(m_timer);
  }

  BEGIN_MSG_MAP(HybridChildwinUtils)
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
  END_MSG_MAP()

  // fade in window (layered), or
  // show window (child)
  void Show()
  {
    T* p = static_cast<T*>(this);

    // are we in between a fading sequence, or already showing?
    if (m_work_aslayered && m_fade_step != 0)
      return;

    if (m_work_aslayered && m_fade_var == 255)
      return;

    CreateWindowCondition(false);
    SwitchWindowStyle();
    AlignToParent(m_hwndparent);
    if (!p->IsWindowVisible())
      p->ShowWindow(SW_SHOWNOACTIVATE);
    if (m_work_aslayered)
    {
      p->SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
      // multimedia timer (deprecated, but better than WM_TIMER)
      if (m_timer)
        ::timeKillEvent(m_timer);
      m_fade_step = 15;
      // calling LayeredWinUtils::SetAlpha
      p->SetAlpha(m_fade_var);
      m_timer = ::timeSetEvent(20, 20, TimerCallback, (DWORD_PTR)this, TIME_PERIODIC);
    }
  }

  // fade out window (layered)
  // no effect (child)
  void Hide()
  {
    T* p = static_cast<T*>(this);
    // if working as child window, we should not support
    // any "window hiding"
    if (!m_work_aslayered)
      return;

    // are we in between a fading sequence or already hidden?
    if (m_fade_step != 0 || m_fade_var == 0)
      return;

    // multimedia timer (deprecated, but better than WM_TIMER)
    if (m_timer)
      ::timeKillEvent(m_timer);
    m_fade_step = -25;
    // calling LayeredWinUtils::SetAlpha
    p->SetAlpha(m_fade_var);
    m_timer = ::timeSetEvent(20, 20, TimerCallback, (DWORD_PTR)this, TIME_PERIODIC);
  }

  // reset window working mode to |aslayered|
  // immediate window style switching will occur
  void SetWorkAsLayered(bool aslayered)
  {
    if (m_timer)
      ::timeKillEvent(m_timer);
    m_timer     = 0;
    m_fade_var  = 0;
    m_fade_step = 0;

    if (m_work_aslayered == aslayered)
      return;

    m_work_aslayered = aslayered;

    SwitchWindowStyle();
    Refresh();
  }
  bool IsWorkAsLayered()
  {
    return m_work_aslayered;
  }

  HWND GetParentWin()
  {
    return m_hwndparent;
  }

  // upper implementation must call Refresh to get "Paint" and "PaintBkgnd"
  // interfaces to work. Use only Refresh() to invalidate window contents
  // and not calling InvalidateRect directly.
  void Refresh()
  {
    T* p = static_cast<T*>(this);
    if (!p->IsWindow())
      return;
    if (m_work_aslayered)
      p->DoUpdateWindow_();
    else
      p->Invalidate();
  }

  virtual void Paint(CDCHandle dc, RECT& rc_client) = 0;
  virtual void PaintBkgnd(CDCHandle dc, RECT& rc_client) = 0;
  virtual void AlignToParent(HWND hwnd_parent) = 0;
  // called by LayeredWinUtils
  void DoLayeredPaint(CDCHandle dc, RECT& rc_client)
  {
    Paint(dc, rc_client);
  }

  LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
  {
    T* p = static_cast<T*>(this);
    CPaintDC dc(p->m_hWnd);
    CMemoryDC mdc(dc, dc.m_ps.rcPaint);
    RECT rc;
    p->GetClientRect(&rc);
    PaintBkgnd((HDC)mdc, rc);
    Paint((HDC)mdc, rc);
    return 0;
  }
  LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&){ return TRUE; }

  // timer callback
  static void CALLBACK TimerCallback(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
  {
    ((HybridChildwinUtils*)dwUser)->TimerCallback_();
  }
  void TimerCallback_()
  {
    T* p = static_cast<T*>(this);
    m_fade_var += m_fade_step;
    if (m_fade_var > 255)
    {
      m_fade_var = 255;
      ::timeKillEvent(m_timer);
      m_timer = 0;
      m_fade_step = 0;
    }
    else if (m_fade_var < 0)
    {
      m_fade_var = 0;
      ::timeKillEvent(m_timer);
      m_timer = 0;
      m_fade_step = 0;
    }
    p->SetAlpha(m_fade_var);
    p->DoPaintWindow_();
  }

private:
  void CreateWindowCondition(bool force_destroy)
  {
    T* p = static_cast<T*>(this);
    if (p->IsWindow() && force_destroy)
      p->DestroyWindow();
    if (!p->IsWindow())
      p->Create(m_hwndparent, NULL, 0, 
        WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
  }
  void SwitchWindowStyle()
  {
    T* p = static_cast<T*>(this);
    if (!p->IsWindow())
      return;

    if (m_work_aslayered)
    {
      CreateWindowCondition(true);
      p->SetParent(NULL);
      p->ModifyStyle(WS_CHILD, WS_POPUP);
      p->ModifyStyleEx(0, WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW);
    }
    else
    {
      p->ModifyStyleEx(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW, 0);
      p->ModifyStyle(WS_POPUP, WS_CHILD);
      p->SetParent(m_hwndparent);
    }
  }

  HWND  m_hwndparent;
  bool  m_work_aslayered;
  int   m_fade_var;
  int   m_fade_step;
  MMRESULT  m_timer;
};

#endif // HYBRID_CHILDWIN_UTILS_H