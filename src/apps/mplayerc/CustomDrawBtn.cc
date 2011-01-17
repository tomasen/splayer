#include "StdAfx.h"
#include "CustomDrawBtn.h"
#include "UserInterface/Renderer/PlaylistView_Win.h"

IMPLEMENT_DYNAMIC(CustomDrawBtn, CButton)

BEGIN_MESSAGE_MAP(CustomDrawBtn, CButton)
  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CustomDrawBtn::OnCustomDraw)
END_MESSAGE_MAP()

CustomDrawBtn::CustomDrawBtn(void):
  m_caption_height(::GetSystemMetrics(SM_CYICON)*7/8),
  m_bottom_height(::GetSystemMetrics(SM_CYICON)),
  m_button_height(::GetSystemMetrics(SM_CYICON)*2/3),
  m_padding(::GetSystemMetrics(SM_CXICON)/4),
  m_entry_height(::GetSystemMetrics(SM_CYSMICON)*5/4),
  m_entry_padding(::GetSystemMetrics(SM_CYSMICON)/8),
  m_basecolor(RGB(78,78,78)),
  m_basecolor2(RGB(32,32,32)),
  m_basecolor3(RGB(128,128,128)),
  m_basecolor4(RGB(192,192,192)),
  m_textcolor(RGB(255,255,255)),
  m_textcolor_hilite(RGB(255,200,20))
{
  //////////////////////////////////////////////////////////////////////////

  WTL::CLogFont lf;
  lf.SetMessageBoxFont();
  m_font_normal.CreateFontIndirect(&lf);
  lf.SetBold();
  m_font_bold.CreateFontIndirect(&lf);
  lf.SetMessageBoxFont();
  wcscpy_s(lf.lfFaceName, 32, L"Webdings");
  lf.lfHeight = lf.lfHeight*5/4;
  m_font_symbol.CreateFontIndirect(&lf);

  m_br_list.CreateSolidBrush(m_basecolor);

  //////////////////////////////////////////////////////////////////////////

}

void CustomDrawBtn::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)

{
  NMCUSTOMDRAW* lpnmcd = (NMCUSTOMDRAW*)pNMHDR;
  switch (lpnmcd->dwDrawStage)
  {
  case CDDS_PREPAINT:
    {
      WTL::CDCHandle dc(lpnmcd->hdc);
      RECT& rc = lpnmcd->rc;

      bool is_hot     = (lpnmcd->uItemState & CDIS_HOT)?true:false;
      bool is_pressed = (lpnmcd->uItemState & CDIS_SELECTED)?true:false;

      WTL::CPen pen_dark, pen_bright, pen_vbright;
      pen_dark.CreatePen(PS_SOLID, 1, m_basecolor2);
      pen_bright.CreatePen(PS_SOLID, 1, m_basecolor3);
      pen_vbright.CreatePen(PS_SOLID, 1, is_hot?m_textcolor_hilite:m_basecolor4);
      dc.FillRect(&rc, m_br_list);
      HPEN old_pen = dc.SelectPen(pen_dark);
      PlaylistView::_DrawRectNoCorner(dc, rc, 2);
      rc.left++;
      rc.right--;
      rc.top++;
      rc.bottom--;
      dc.SelectPen(pen_bright);
      dc.Rectangle(&rc);
      dc.SelectPen(pen_vbright);
      dc.MoveTo(rc.left, rc.top);
      dc.LineTo(rc.right, rc.top);
      dc.SelectPen(old_pen);
      rc.left++;
      rc.right--;
      rc.top++;
      if (is_pressed)
        rc.bottom--;
      PlaylistView::_FillGradient(dc, rc, is_pressed?m_basecolor:m_basecolor3, 
        is_pressed?m_basecolor3:m_basecolor);

      HFONT old_font = dc.SelectFont(m_font_normal);
      dc.SetTextColor(is_hot?m_textcolor_hilite:m_textcolor);
      dc.SetBkMode(TRANSPARENT);
      CString text;
      GetWindowText(text);
      dc.DrawText(text, -1, &rc, DT_VCENTER|DT_SINGLELINE|DT_CENTER);
      dc.SelectFont(old_font);
    }
    *pResult = CDRF_SKIPDEFAULT;
    break;
  }
}