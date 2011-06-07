#include "stdafx.h"
#include "video_widget.h"
#include "bitmap_svc.h"

VideoWidget::VideoWidget(void)
{

}

LRESULT VideoWidget::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  switch (wParam)
  {
  case 1:
    GetParent().PostMessage(UWM_FADEOUT_CONTROLS);
    break;
  }
  return 0;
}

LRESULT VideoWidget::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  CPaintDC dc(m_hWnd);
  CMemoryDC mdc(dc, dc.m_ps.rcPaint);
  RECT rc;
  GetClientRect(&rc);
  CBrush brush;
  brush.CreateSolidBrush(RGB(64,64,64));
  mdc.FillRect(&rc, brush);

  BitmapService::GetInstance()->Paint(L"ts.png", 0, mdc, 0, 0, rc.right-rc.left, rc.bottom-rc.top);
  BitmapService::GetInstance()->Paint(L"test_bkgnd.2_2_2_2_2.png", 0, mdc, 50, 50, 100, 100);
  BitmapService::GetInstance()->Paint(L"test_bkgnd.2_2_2_2_2.png", 1, mdc, 50, 150, 100, 100);

  BitmapService::GetInstance()->Paint(L"sample_btn.1_8_6_7_8.png", 0, mdc, 200, 50, 100, 50);
  BitmapService::GetInstance()->Paint(L"sample_btn2.1_14_12_15_14.png", 0, mdc, 200, 100, 
    (rc.right-rc.left)/3, (rc.bottom-rc.top)/8);

  return 0;
}

LRESULT VideoWidget::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  KillTimer(1);
  GetParent().PostMessage(UWM_FADEIN_CONTROLS);
  SetTimer(1, 3000);
  return 0;
}
