#pragma once

#include "..\..\SeekBarTip.h"

class CSubtitleListBox : public ATL::CWindowImpl<CSubtitleListBox, WTL::CListBox >
{
public:
  CSubtitleListBox();
  ~CSubtitleListBox();

  BEGIN_MSG_MAP_EX(CSubtitleListBox)
    MSG_WM_MOUSEMOVE(OnMouseMove)
  END_MSG_MAP()

private:
  void OnMouseMove(UINT nFlags, CPoint point);

private:
  CSeekBarTip  m_tip;
};