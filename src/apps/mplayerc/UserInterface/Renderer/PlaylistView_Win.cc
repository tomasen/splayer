#include "StdAfx.h"
#include "PlaylistView_Win.h"
#include "../../resource.h"
#include "../../Utils/Strings.h"
#include "../../Controller/SPlayerDefs.h"
#include "../../Controller/PlayerPreference.h"
#include "../../Controller/PlaylistController.h"

PlaylistView::PlaylistView(void):
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
  WTL::CLogFont lf;
  lf.SetMessageBoxFont();
  m_font_normal.CreateFontIndirect(&lf);
  lf.SetBold();
  m_font_bold.CreateFontIndirect(&lf);
  lf.SetMessageBoxFont();
  wcscpy_s(lf.lfFaceName, 32, L"Webdings");
  lf.lfHeight = lf.lfHeight*5/4;
  m_font_symbol.CreateFontIndirect(&lf);

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
    WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_VSCROLL |
    LBS_NODATA | LBS_OWNERDRAWFIXED | LBS_NOTIFY, 0, IDC_LISTBOX);
  m_btn_load.Create(m_hWnd, NULL, NULL,
    WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE, 0, IDC_BTNLOAD);
  m_btn_save.Create(m_hWnd, NULL, NULL,
    WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE, 0, IDC_BTNSAVE);
  m_btn_clear.Create(m_hWnd, NULL, NULL,
    WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE, 0, IDC_BTNCLEAR);
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
  HDWP hdwp = ::BeginDeferWindowPos(4);
  ::DeferWindowPos(hdwp, m_list, NULL, m_padding, m_caption_height + m_padding, 
    size.cx - m_padding*2, size.cy - m_bottom_height - m_caption_height, 
    SWP_NOACTIVATE | SWP_NOZORDER);
  int btn_top     = size.cy - m_bottom_height + (m_bottom_height - m_button_height)/2;
  int btn_width   = size.cx/4;
  ::DeferWindowPos(hdwp, m_btn_load, NULL, m_padding, btn_top, btn_width, m_button_height,
    SWP_NOACTIVATE | SWP_NOZORDER);
  ::DeferWindowPos(hdwp, m_btn_save, NULL, m_padding*3/2 + btn_width, btn_top, btn_width, m_button_height,
    SWP_NOACTIVATE | SWP_NOZORDER);
  ::DeferWindowPos(hdwp, m_btn_clear, NULL, size.cx - btn_width - m_padding, btn_top, btn_width, m_button_height,
    SWP_NOACTIVATE | SWP_NOZORDER);
  ::EndDeferWindowPos(hdwp);
}

HBRUSH PlaylistView::OnColorListBox(WTL::CDCHandle dc, WTL::CListBox listBox)
{
  return m_br_list;
}

LRESULT PlaylistView::OnLbnDblClk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
  PlayerPreference* pref = PlayerPreference::GetInstance();
  pref->SetIntVar(INTVAR_PLAYLIST_CURRENT, m_list.GetCurSel());
  ::SendMessage(reinterpret_cast<HWND>(pref->GetInt64Var(INT64VAR_MAINWINDOW)), 
    WM_SPLAYER_CMD, CMD_OPEN_CURRENTPLAYLISTITEM, 0);
  return 0;
}

LRESULT PlaylistView::OnBtnLoad(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
  PlayerPreference* pref = PlayerPreference::GetInstance();
  ::SendMessage(reinterpret_cast<HWND>(pref->GetInt64Var(INT64VAR_MAINWINDOW)), 
    WM_SPLAYER_CMD, CMD_OPEN_PLAYLIST, 0);
  return 0;
}

LRESULT PlaylistView::OnBtnSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
  return 0;
}

LRESULT PlaylistView::OnBtnClear(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
  return 0;
}

void PlaylistView::DrawItem(LPDRAWITEMSTRUCT lpdis)
{
  std::vector<std::wstring> list = 
    PlaylistController::GetInstance()->GetListDisplay();
  if (lpdis->itemID < list.size() && lpdis->itemID >= 0)
  {
    RECT& rc = lpdis->rcItem;
    WTL::CDCHandle dc(lpdis->hDC);
    WTL::CBrush bkgnd_brush;
    bkgnd_brush.CreateSolidBrush(m_basecolor);
    if (lpdis->itemState & ODS_SELECTED)
    {
      WTL::CPen pen;
      pen.CreatePen(PS_SOLID, 1, m_basecolor4);
      HPEN old_pen = dc.SelectPen(pen);
      _DrawRectNoCorner(dc, rc, 1);
      dc.SelectPen(old_pen);
      rc.left++;
      rc.top++;
      rc.bottom--;
      rc.right--;
      _FillGradient(dc, rc, m_basecolor3, m_basecolor);
    }
    else
      dc.FillRect(&rc, bkgnd_brush);

    rc.left += m_entry_padding*7;
    bool iscurrent = PlayerPreference::GetInstance()->GetIntVar(INTVAR_PLAYLIST_CURRENT) == lpdis->itemID;
    HFONT old_font = iscurrent?dc.SelectFont(m_font_bold):dc.SelectFont(m_font_normal);
    dc.SetBkMode(TRANSPARENT);
    dc.SetTextColor(iscurrent?m_textcolor_hilite:m_textcolor);
    dc.DrawText(list[lpdis->itemID].c_str(), -1, &rc, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
    if (iscurrent)
    {
      dc.SetTextColor(m_textcolor_hilite);
      dc.SelectFont(m_font_symbol);
      rc.left -= m_entry_padding*7;
      dc.DrawText(L"4", -1, &rc, DT_LEFT|DT_SINGLELINE|DT_VCENTER);
    }
    dc.SelectFont(old_font);
  }
}

void PlaylistView::MeasureItem(LPMEASUREITEMSTRUCT lpmis)
{
  lpmis->itemHeight = m_entry_height;
}

DWORD PlaylistView::OnPrePaint(int idCtrl, LPNMCUSTOMDRAW lpnmcd)
{
  WTL::CDCHandle dc(lpnmcd->hdc);
  RECT& rc = lpnmcd->rc;

  bool is_hot     = (lpnmcd->uItemState & CDIS_HOT)?true:false;
  bool is_pressed = (lpnmcd->uItemState & CDIS_SELECTED)?true:false;

  switch (idCtrl)
  {
  case IDC_BTNCLEAR:
  case IDC_BTNLOAD:
  case IDC_BTNSAVE:
    {
      WTL::CPen pen_dark, pen_bright, pen_vbright;
      pen_dark.CreatePen(PS_SOLID, 1, m_basecolor2);
      pen_bright.CreatePen(PS_SOLID, 1, m_basecolor3);
      pen_vbright.CreatePen(PS_SOLID, 1, is_hot?m_textcolor_hilite:m_basecolor4);
      dc.FillRect(&rc, m_br_list);
      HPEN old_pen = dc.SelectPen(pen_dark);
      _DrawRectNoCorner(dc, rc, 2);
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
      _FillGradient(dc, rc, is_pressed?m_basecolor:m_basecolor3, 
        is_pressed?m_basecolor3:m_basecolor);
    }
    break;
  }
  HFONT old_font = dc.SelectFont(m_font_normal);
  dc.SetTextColor(is_hot?m_textcolor_hilite:m_textcolor);
  dc.SetBkMode(TRANSPARENT);
  switch (idCtrl)
  {
  case IDC_BTNLOAD:
    dc.DrawText(m_texts[1].c_str(), -1, &rc, DT_VCENTER|DT_SINGLELINE|DT_CENTER);
    break;
  case IDC_BTNSAVE:
    dc.DrawText(m_texts[2].c_str(), -1, &rc, DT_VCENTER|DT_SINGLELINE|DT_CENTER);
    break;
  case IDC_BTNCLEAR:
    dc.DrawText(m_texts[3].c_str(), -1, &rc, DT_VCENTER|DT_SINGLELINE|DT_CENTER);
    break;
  }
  dc.SelectFont(old_font);
  return CDRF_SKIPDEFAULT;
}

void PlaylistView::_PaintWorker(HDC hdc, RECT rc)
{
  WTL::CDCHandle dc(hdc);
  WTL::CBrush bkgnd;
  bkgnd.CreateSolidBrush(m_basecolor);
  dc.FillRect(&rc, bkgnd);

  RECT rc_grad1 = {rc.left, rc.top, rc.right, rc.top + m_caption_height - m_padding/2};
  RECT rc_grad2 = {rc.left, rc.top + m_caption_height - m_padding/2, rc.right,
    rc.top + m_caption_height};
  _FillGradient(dc, rc_grad1, m_basecolor3, m_basecolor);
  _FillGradient(dc, rc_grad2, m_basecolor2, m_basecolor);

  HFONT old_font = dc.SelectFont(m_font_normal);
  RECT rc_text = {rc.left + m_padding, rc.top, rc.right - m_padding, rc.top + m_caption_height - m_padding/2};
  dc.SetBkMode(TRANSPARENT);
  dc.SetTextColor(m_textcolor);
  dc.DrawText(m_texts[0].c_str(), -1, &rc_text, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
  dc.SelectFont(old_font);
}

void PlaylistView::_FillGradient(HDC hdc, RECT rc, int color_tl, int color_br, bool vert /* = true */)
{
  WTL::CDCHandle dc(hdc);
  TRIVERTEX     v[2];
  GRADIENT_RECT grc;
  v[0].x      = rc.left;
  v[0].y      = rc.top;
  v[0].Red    = GetRValue(color_tl)<<8;
  v[0].Green  = GetGValue(color_tl)<<8;
  v[0].Blue   = GetBValue(color_tl)<<8;
  v[0].Alpha  = 0x0000;
  v[1].x      = rc.right;
  v[1].y      = rc.bottom; 
  v[1].Red    = GetRValue(color_br)<<8;
  v[1].Green  = GetGValue(color_br)<<8;
  v[1].Blue   = GetBValue(color_br)<<8;
  v[1].Alpha  = 0x0000;
  grc.UpperLeft  = 0;
  grc.LowerRight = 1;
  dc.GradientFill(v, 2, &grc, 1, vert?GRADIENT_FILL_RECT_V:GRADIENT_FILL_RECT_H);
}

void PlaylistView::_DrawRectNoCorner(WTL::CDCHandle& dc, RECT& rc, int offset)
{
  dc.MoveTo(rc.left + offset, rc.top);
  dc.LineTo(rc.right - offset, rc.top);
  dc.MoveTo(rc.right - 1, rc.top + offset);
  dc.LineTo(rc.right - 1, rc.bottom - offset);
  dc.MoveTo(rc.right - 1 - offset, rc.bottom - 1);
  dc.LineTo(rc.left + offset - 1, rc.bottom - 1);
  dc.MoveTo(rc.left, rc.bottom - 1 - offset);
  dc.LineTo(rc.left, rc.top + offset - 1);
}
