#ifndef SPLAYER_FRM_H
#define SPLAYER_FRM_H

template <class T>
class SPlayerFrameStyler
{
  typedef SPlayerFrameStyler<T> _baseClass;
public:
  SPlayerFrameStyler(void):
    m_style(-1),
    m_close_status(0),
    m_min_status(0),
    m_max_status(0)
  {
    CLogFont lf;
    lf.SetMessageBoxFont();
    m_gui_font.CreateFontIndirect(&lf);
    lf.SetBold();
    m_gui_bfont.CreateFontIndirect(&lf);

    CDC dc;
    dc.CreateCompatibleDC();
    HFONT _font = dc.SelectFont(m_gui_font);
    TEXTMETRIC tm;
    dc.GetTextMetrics( &tm );
    dc.SelectFont(_font);

    m_dlg_unit = tm.tmHeight;
  }

  BEGIN_MSG_MAP(SPlayerFrameStyler)
    MESSAGE_HANDLER(WM_NCLBUTTONDOWN, OnNcLButtonDown)
    MESSAGE_HANDLER(WM_NCLBUTTONUP, OnNcLButtonUp)
    MESSAGE_HANDLER(WM_NCHITTEST, OnNcHitTest)
  END_MSG_MAP()

  void PaintFrame(HDC hdc, LPRECT rcPaint)
  {
    // establish double buffering
    WTL::CMemoryDC mdc(hdc, *rcPaint);

    // paint base window style
    BitmapService::GetInstance()->Paint(L"window_style.3_10_20_10_10.png", m_style,
      mdc, rcPaint->left, rcPaint->top, rcPaint->right - rcPaint->left,
      rcPaint->bottom - rcPaint->top);

    // paint custom min/max/close button series
    int btn_top = rcPaint->top + 1;
    WINDOWPLACEMENT wp = {0};
    T* p = static_cast<T*>(this);
    p->GetWindowPlacement(&wp);
    if (wp.showCmd == SW_SHOWMAXIMIZED)
      btn_top += ::GetSystemMetrics(SM_CYFRAME) - 1;

    BitmapService::GetInstance()->Paint(L"btn_close.4.png", m_close_status, mdc, 
      rcPaint->right - 43 - m_dlg_unit, btn_top, 
      43, 19);
    BitmapService::GetInstance()->Paint(
      wp.showCmd == SW_SHOWMAXIMIZED?L"btn_restore.4.png":L"btn_max.4.png", m_max_status, mdc, 
      rcPaint->right-43-25-m_dlg_unit, btn_top, 
      25, 19);
    BitmapService::GetInstance()->Paint(L"btn_min.4.png", m_min_status, mdc, 
      rcPaint->right-43-25-27-m_dlg_unit, btn_top, 
      27, 19);

    // paint decoration etch
//     BitmapService::GetInstance()->Paint(L"window_etch.1_1_1_1_1.png", 0, mdc,
//       rcPaint->left + ::GetSystemMetrics(SM_CXFRAME) - 1,
//       rcPaint->top + ::GetSystemMetrics(SM_CYFRAME) + ::GetSystemMetrics(SM_CYCAPTION) - 1,
//       rcPaint->right - rcPaint->left - ::GetSystemMetrics(SM_CXFRAME) * 2 + 2,
//       rcPaint->bottom - rcPaint->top - ::GetSystemMetrics(SM_CYFRAME) * 2 -
//       ::GetSystemMetrics(SM_CYCAPTION) + 2);

    // paint title
    HFONT _font = mdc.SelectFont(m_gui_bfont);
    CRect rc_text(
      rcPaint->left + ::GetSystemMetrics(SM_CXFRAME) * 3/2 + 1 + ::GetSystemMetrics(SM_CXSMICON), 
      1,
      rcPaint->right - ::GetSystemMetrics(SM_CXFRAME) + 1,
      ::GetSystemMetrics(SM_CYFRAME) + ::GetSystemMetrics(SM_CYCAPTION) + 1);
    CString window_title;
    p->GetWindowText(window_title);
    mdc.SetBkMode(TRANSPARENT);
    mdc.SetTextColor(RGB(32,32,32));
    mdc.DrawText(window_title, -1, &rc_text, DT_SINGLELINE|DT_VCENTER|DT_LEFT);
    rc_text.OffsetRect(-1, -1);
    mdc.SetTextColor(RGB(252,252,252));
    mdc.DrawText(window_title, -1, &rc_text, DT_SINGLELINE|DT_VCENTER|DT_LEFT);
    mdc.SelectFont(_font);

    // paint icon
    HICON icon = (HICON)::GetClassLongPtr(*p, GCLP_HICONSM);
    mdc.DrawIconEx(::GetSystemMetrics(SM_CXFRAME), 
      (::GetSystemMetrics(SM_CYFRAME) + ::GetSystemMetrics(SM_CYCAPTION) - ::GetSystemMetrics(SM_CYSMICON))/2, 
      icon, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0, 
      NULL, DI_NORMAL | DI_COMPAT);
  }

  void SetFrameStyle(int style)
  {
    T* p = static_cast<T*>(this);
    ::SetProp(p->m_hWnd, L"__FRAME_STYLE", (HANDLE)style);
    m_style = style;
  }

  int GetFrameStyle()
  {
    return m_style;
  }

  LRESULT OnNcLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
  {
    bHandled = FALSE;
    T* p = static_cast<T*>(this);
    if (!p->IsAeroFrameEnabled())
    {
      if (wParam == HTCLOSE || wParam == HTMAXBUTTON || wParam == HTMINBUTTON)
      {
        bHandled = TRUE;
        p->RedrawFrame();
      }
    }
    return 0;
  }
  LRESULT OnNcLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
  {
    bHandled = FALSE;
    T* p = static_cast<T*>(this);
    if (!p->IsAeroFrameEnabled())
    {
      if (wParam == HTCLOSE || wParam == HTMAXBUTTON || wParam == HTMINBUTTON)
      {
        bHandled = TRUE;
        p->RedrawFrame();
        WINDOWPLACEMENT wp = {0};
        p->GetWindowPlacement(&wp);
        if (wParam == HTCLOSE)
          p->PostMessage(WM_CLOSE);
        else if(wParam == HTMAXBUTTON)
        {
          m_min_status = m_max_status = m_close_status = 0;
          if (wp.showCmd == SW_MAXIMIZE)
            p->ShowWindow(SW_NORMAL);
          else
            p->ShowWindow(SW_MAXIMIZE);
        }
        else if (wParam == HTMINBUTTON)
        {
          m_min_status = m_max_status = m_close_status = 0;
          p->ShowWindow(SW_MINIMIZE);
        }
      }
    }
    return 0;
  }

  LRESULT OnNcHitTest( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
  {
    bHandled = FALSE;
    T* p = static_cast<T*>(this);
    if (!p->IsAeroFrameEnabled())
    {
      WINDOWPLACEMENT wp = {0};
      p->GetWindowPlacement(&wp);

      RECT rc;
      p->GetWindowRect(&rc);

      int btn_top = rc.top + 1;
      if (wp.showCmd == SW_SHOWMAXIMIZED)
        btn_top += ::GetSystemMetrics(SM_CYFRAME) - 1;

      CRect rcClose(rc.right-43-m_dlg_unit, btn_top, rc.right-m_dlg_unit, btn_top + 18);
      CRect rcMax(rc.right-43-25-m_dlg_unit, btn_top, rc.right-m_dlg_unit-43, btn_top + 18);
      CRect rcMin(rc.right-43-25-27-m_dlg_unit, btn_top, rc.right-43-25-m_dlg_unit, btn_top + 18);

      int _min_status   = m_min_status;
      int _max_status   = m_max_status;
      int _close_status = m_close_status;

      CPoint pt(lParam);
      SHORT bLBtnDown = GetAsyncKeyState(VK_LBUTTON);
      if (rcClose.PtInRect(pt) && bLBtnDown)
        m_close_status = 2;
      else if (rcClose.PtInRect(pt))
        m_close_status = 1;
      else
        m_close_status = 0;

      if (rcMax.PtInRect(pt) && bLBtnDown)
        m_max_status = 2;
      else if (rcMax.PtInRect(pt))
        m_max_status = 1;
      else
        m_max_status = 0;

      if (rcMin.PtInRect(pt) && bLBtnDown)
        m_min_status = 2;
      else if (rcMin.PtInRect(pt))
        m_min_status = 1;
      else
        m_min_status = 0;

      if (_min_status != m_min_status || 
        _max_status != m_max_status || 
        _close_status != m_close_status)
        p->RedrawFrame();

      if (m_close_status != 0)
      {
        bHandled = TRUE;
        return HTCLOSE;
      }
      else if (m_max_status != 0)
      {
        bHandled = TRUE;
        return HTMAXBUTTON;
      }
      else if (m_min_status != 0)
      {
        bHandled = TRUE;
        return HTMINBUTTON;
      }
    }
    return 0;
  }


private:
  int m_style;
  int m_min_status;
  int m_max_status;
  int m_close_status;

  int m_dlg_unit;

  CFont m_gui_font;
  CFont m_gui_bfont;
};

#endif // SPLAYER_FRM_H