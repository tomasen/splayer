#include "stdafx.h"
#include "SubtitleListBox.h"
#include "..\Support\SubtitleStyle.h"

CSubtitleListBox::CSubtitleListBox()
{

}

CSubtitleListBox::~CSubtitleListBox()
{

}

void CSubtitleListBox::OnMouseMove(UINT nFlags, CPoint point)
{
  AppSettings &s = AfxGetAppSettings();

  // create tooltip
  if (!::IsWindow(m_tip.m_hWnd))
  {
    DWORD dwTransparent = s.bUserAeroUI() ? WS_EX_TRANSPARENT : 0;
    m_tip.CreateEx(WS_EX_NOACTIVATE | WS_EX_TOPMOST | dwTransparent, _T("SVPLayered"),
                   _T("TIPS"), WS_POPUP, CRect(20,20,21,21), 0, 0);

    if (s.bUserAeroUI())
    {
      m_tip.ModifyStyleEx(0, WS_EX_LAYERED);
      m_tip.SetLayeredWindowAttributes(0, s.lAeroTransparent, LWA_ALPHA);
    }
  }

  // show or hide tooltip
  BOOL bOutside = FALSE;
  int nItem = ItemFromPoint(point, bOutside);
  static int nLastItem = -1;
  if (!bOutside && nItem >= 0)
  {
    // clear stat
    if (nLastItem != nItem)
    {
      m_tip.ClearStat();
      nLastItem = nItem;
    }

    // show tooltip
    SubtitleStyle::STYLEPARAM *sp = 0;
    SubtitleStyle::GetStyleParams(nItem, -1, &sp);
    if (sp)
    {
      if (!m_tip.IsWindowVisible())
      {
        m_tip.SetTips(sp->fontname, TRUE, 0, 500);
      }
    }
  }
  else
  {
    nLastItem = -1;
  }
}