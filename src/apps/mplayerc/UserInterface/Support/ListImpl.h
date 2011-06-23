#pragma once
#include "FontParamsManage.h"

class CListImpl : public CWindowImpl<CListImpl,WTL::CListBox>
{
public:

  CListImpl();
  ~CListImpl();

  BEGIN_MSG_MAP_EX(CListImpl)
    MSG_WM_MOUSEMOVE(OnMouseMove)
    MSG_WM_LBUTTONDBLCLK(OnLButtonDbClick)
  END_MSG_MAP()

  void InitializeList();

  void OnMouseMove(UINT wParam, WTL::CPoint pt);
  void OnLButtonDbClick(UINT wParam, WTL::CPoint pt);

  void DrawItem(LPDRAWITEMSTRUCT lpdis);
  void MeasureItem(LPMEASUREITEMSTRUCT lpmis);

  // before draw list items.
  int PreserveItemDivideRect(WTL::CRect rc, int index, BOOL bdivide);
  WTL::CRect GetTextDivideRect(int index);
  WTL::CRect GetHittestDivideRect(int index);
  void PaintListItemBackground(HDC dc, int width, int height);
  void PaintHighLightListBckground(HDC dc, WTL::CRect rc, BOOL bmain = true);

  // Get selected item
  void GetItemData(int index, StyleParam** mainsp, StyleParam** secondsp);

private:
  int m_styleentry_height;

  FontParamsManage m_fontparams;
  DrawSubtitle m_subtitle;
  std::vector<WTL::CRect>   m_listtextrc;
  std::vector<WTL::CRect>   m_listhittestrc;
  std::vector<std::wstring> m_samplevec;
  WTL::CRect                m_highlightrc;
  int                       m_highlightstat;
  
};