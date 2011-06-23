#pragma once

class CListImpl : public CWindowImpl<CListImpl,WTL::CListBox>
{
public:

  CListImpl();
  ~CListImpl();

  BEGIN_MSG_MAP_EX(CListImpl)
    MSG_WM_MOUSEMOVE(OnMouseMove)
    MSG_WM_LBUTTONDBLCLK(OnLButtonDbClick)
  END_MSG_MAP()

  void OnMouseMove(UINT wParam, WTL::CPoint pt);
  void OnLButtonDbClick(UINT wParam, WTL::CPoint pt);
  
};