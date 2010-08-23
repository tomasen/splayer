#include "StdAfx.h"
#include "PlaylistView_Win.h"
#include "../../resource.h"
#include "../../Utils/Strings.h"
#include "../../Controller/PlaylistController.h"

PlaylistView::PlaylistView(void):
  m_caption_height(::GetSystemMetrics(SM_CYICON)*7/8),
  m_bottom_height(::GetSystemMetrics(SM_CYICON)),
  m_button_height(::GetSystemMetrics(SM_CYICON)*2/3),
  m_padding(::GetSystemMetrics(SM_CXICON)/4),
  m_basecolor(RGB(78,78,78)),
  m_basecolor2(RGB(32,32,32)),
  m_basecolor3(RGB(148,148,148)),
  m_textcolor(RGB(255,255,255))
{
  WTL::CLogFont lf;
  lf.SetMessageBoxFont();
  lf.SetBold();
  m_font_caption.CreateFontIndirect(&lf);

  WTL::CString text;
  text.LoadString(IDS_PLAYLIST);
  Strings::Split(text, L"|", m_texts);

  m_br_list.CreateSolidBrush(m_basecolor);
}

void PlaylistView::Refresh()
{
  m_list.SetCount(PlaylistController::GetInstance()->GetListDisplay().size());
  m_list.Invalidate();
}

int PlaylistView::OnCreate(LPCREATESTRUCT lpcs)
{
  m_list.Create(m_hWnd, NULL, NULL, 
    WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE |
    LBS_NODATA | LBS_OWNERDRAWFIXED);
  Refresh();
  return 0;
}

void PlaylistView::OnPaint(WTL::CDCHandle dc)
{
  RECT rc;
  GetClientRect(&rc);
  WTL::CPaintDC pdc(m_hWnd);
  WTL::CMemoryDC mdc(pdc, pdc.m_ps.rcPaint);
  _PaintWorker(mdc, rc);
}

BOOL PlaylistView::OnEraseBkgnd(WTL::CDCHandle dc)
{
  return TRUE;
}

void PlaylistView::OnSize(UINT nType, CSize size)
{
  RECT rc = {m_padding, m_caption_height + m_padding, 
    size.cx - m_padding, size.cy - m_bottom_height - m_padding};
  m_list.SetWindowPos(NULL, &rc, SWP_NOZORDER);
}

HBRUSH PlaylistView::OnColorListBox(WTL::CDCHandle dc, WTL::CListBox listBox)
{
  return m_br_list;
}

void PlaylistView::DrawItem(LPDRAWITEMSTRUCT lpdis)
{
  std::vector<std::wstring> list = 
    PlaylistController::GetInstance()->GetListDisplay();
  if (lpdis->itemID < list.size() && lpdis->itemID >= 0)
    ::DrawText(lpdis->hDC, list[lpdis->itemID].c_str(), -1, &lpdis->rcItem, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
}

void PlaylistView::MeasureItem(LPMEASUREITEMSTRUCT lpmis)
{
  lpmis->itemHeight = ::GetSystemMetrics(SM_CYSMICON);
}

void PlaylistView::_PaintWorker(HDC hdc, RECT rc)
{
  WTL::CDCHandle dc(hdc);
  WTL::CBrush bkgnd;
  bkgnd.CreateSolidBrush(m_basecolor);
  dc.FillRect(&rc, bkgnd);

//   WTL::CPen pen;
//   pen.CreatePen(PS_SOLID, 1, m_basecolor);
//   HPEN old_pen = dc.SelectPen(pen);
//   dc.MoveTo(rc.left, rc.top);
//   dc.LineTo(rc.right-1, rc.top);
//   dc.LineTo(rc.right-1, rc.top + m_caption_height - m_padding/2-1);
//   dc.LineTo(rc.left, rc.top + m_caption_height - m_padding/2-1);
//   dc.LineTo(rc.left, rc.top);
//   dc.SelectPen(old_pen);

  TRIVERTEX     vert[2] ;
  GRADIENT_RECT gRect;
  vert[0].x      = rc.left+1;
  vert[0].y      = rc.top+1;
  vert[0].Red    = GetRValue(m_basecolor3)<<8;
  vert[0].Green  = GetGValue(m_basecolor3)<<8;
  vert[0].Blue   = GetBValue(m_basecolor3)<<8;
  vert[0].Alpha  = 0x0000;
  vert[1].x      = rc.right-1;
  vert[1].y      = rc.top + m_caption_height - m_padding/2 -1; 
  vert[1].Red    = GetRValue(m_basecolor)<<8;
  vert[1].Green  = GetGValue(m_basecolor)<<8;
  vert[1].Blue   = GetBValue(m_basecolor)<<8;
  vert[1].Alpha  = 0x0000;
  gRect.UpperLeft  = 0;
  gRect.LowerRight = 1;
  dc.GradientFill(vert, 2, &gRect, 1, GRADIENT_FILL_RECT_V);
  vert[0].x      = rc.left;
  vert[0].y      = rc.top + m_caption_height - m_padding/2;
  vert[0].Red    = GetRValue(m_basecolor2)<<8;
  vert[0].Green  = GetGValue(m_basecolor2)<<8;
  vert[0].Blue   = GetBValue(m_basecolor2)<<8;
  vert[1].x      = rc.right;
  vert[1].y      = rc.top + m_caption_height; 
  gRect.UpperLeft  = 1;
  gRect.LowerRight = 0;
  dc.GradientFill(vert, 2, &gRect, 1, GRADIENT_FILL_RECT_V);

  HFONT old_font = dc.SelectFont(m_font_caption);
  RECT rc_text = {rc.left + m_padding, rc.top, rc.right - m_padding, rc.top + m_caption_height - m_padding/2};
  dc.SetBkMode(TRANSPARENT);
  dc.SetTextColor(m_textcolor);
  dc.DrawText(m_texts[0].c_str(), -1, &rc_text, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
  dc.SelectFont(old_font);
}
