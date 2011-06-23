#include "stdafx.h"
#include "../../resource.h"
#include "ListImpl.h"
#include <Strings.h>
#include "../Dialogs/CustomizeFontDlg.h"

CListImpl::CListImpl(void):
 m_highlightstat(0)
,m_styleentry_height(80)
{
  // init font sample string
  WTL::CString text;
  text.LoadString(IDS_SUBTITLESTYLES);
  Strings::Split(text, L"|", m_samplevec);

  m_listtextrc.clear(); 
  m_listhittestrc.clear();
}

CListImpl::~CListImpl(void)
{
}

void CListImpl::InitializeList()
{
  SetCount(m_fontparams.GetStyleCount());
  Invalidate();
  UpdateWindow();
}

void CListImpl::OnMouseMove(UINT wParam, WTL::CPoint pt)
{
  int index = GetCurSel();
  WTL::CRect mainrc = GetHittestDivideRect(2 * index);
  WTL::CRect seconrc = GetHittestDivideRect(2 * index + 1);

  if (PtInRect(&mainrc, pt))
  {
    if (m_highlightstat == 1)
      return;

    SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_HAND));
    ::SetCursor(LoadCursor(NULL, IDC_HAND));
    m_highlightstat = 1;
    SetCurSel(GetCurSel());
    return;
  }

  if (PtInRect(&seconrc, pt))
  {
    if (m_highlightstat == 2)
      return;

    SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_HAND));
    SetCursor(LoadCursor(NULL, IDC_HAND));
    m_highlightstat = 2;
    SetCurSel(GetCurSel());
    return;
  }

  int bhightlightorg = m_highlightstat;
  m_highlightstat = 0;

  SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));
  SetCursor(LoadCursor(NULL, IDC_ARROW));

  if (bhightlightorg != m_highlightstat)
    SetCurSel(GetCurSel());

}

void CListImpl::OnLButtonDbClick(UINT wParam, WTL::CPoint pt)
{
  int index = GetCurSel();
  WTL::CRect mainrc = GetHittestDivideRect(2 * index);
  WTL::CRect seconrc = GetHittestDivideRect(2 * index + 1);

  BOOL mainorsecon;
  int  vecindex = MAXINT;

  WTL::CString text;
  std::vector<std::wstring> vec;
  text.LoadString(IDS_CUSTOMIZEFONT_TITLE);
  Strings::Split(text, L"|", vec);

  if (PtInRect(&mainrc, pt))
  {
    mainorsecon = TRUE;
    vecindex = 0;
  }

  if (PtInRect(&seconrc, pt))
  {
    mainorsecon = FALSE;
    vecindex = 1;
  }

  if (vecindex != MAXINT)
  {
    CustomizeFontDlg fdlg;
    fdlg.SetTitleText(vec[vecindex]);
    fdlg.SetSampleText(m_samplevec[0], m_samplevec[1]);
    fdlg.SetFontParam(m_fontparams.GetFontParam(index), m_fontparams.GetFontParam(index, FALSE), mainorsecon);
    if (fdlg.DoModal() == IDC_FONTOK_BUTTON)
      m_fontparams.WriteProfile();
  } 

}

void CListImpl::DrawItem(LPDRAWITEMSTRUCT lpdis)
{
  StyleParam* spmain = NULL;
  StyleParam* spsecondary = NULL;
  spmain = m_fontparams.GetFontParam(lpdis->itemID, TRUE);
  spsecondary = m_fontparams.GetFontParam(lpdis->itemID, FALSE);

  int top = 0;
  if (spsecondary)
    top = PreserveItemDivideRect(lpdis->rcItem, lpdis->itemID, TRUE);
  else
    top = PreserveItemDivideRect(lpdis->rcItem, lpdis->itemID, FALSE);

  WTL::CDC dc;
  WTL::CBitmap bmp;
  dc.CreateCompatibleDC(lpdis->hDC);
  bmp.CreateCompatibleBitmap(lpdis->hDC, 2 * (lpdis->rcItem.right - lpdis->rcItem.left), 
    2 * (lpdis->rcItem.bottom - lpdis->rcItem.top));
  HBITMAP oldbmp = dc.SelectBitmap(bmp);

  WTL::CRect rc(0, 0, 2 * (lpdis->rcItem.right - lpdis->rcItem.left), 2 * (lpdis->rcItem.bottom - lpdis->rcItem.top));

  dc.FillRect(&rc, COLOR_3DFACE);
  if (lpdis->itemState & ODS_SELECTED)
  {
    PaintListItemBackground(dc, rc.Width(), rc.Height());
    if (m_highlightstat != 0)
    {
      WTL::CRect itemrc;
      WTL::CRect lightrc = rc;
      BOOL       bmain = TRUE;

      itemrc = GetHittestDivideRect(2 * lpdis->itemID);
      lightrc.bottom = lightrc.top + 2 * itemrc.Height();

      if (m_highlightstat == 2)
      {
        lightrc.top = lightrc.bottom;
        lightrc.bottom = rc.bottom;
        bmain = FALSE;
      }

      PaintHighLightListBckground(dc, lightrc, bmain);
    }
  }


  WTL::CRect mainrc = GetTextDivideRect( 2 * lpdis->itemID);
  int height = mainrc.Height();
  mainrc.top = 2 * top;
  mainrc.bottom = mainrc.top + 2 * height;

  if (spmain)
  {
    m_subtitle.SetFont(*m_fontparams.GetFontParam(lpdis->itemID));
    m_subtitle.SetSampleText(m_samplevec[0]);
    m_subtitle.Paint(dc, mainrc);
  }

  WTL::CRect seconrc = GetTextDivideRect(2 * lpdis->itemID + 1);
  int height2 = seconrc.Height();
  seconrc.top = mainrc.bottom;
  seconrc.bottom = seconrc.top + 2 * height2;
  if (spsecondary)
  {
    m_subtitle.SetFont(*m_fontparams.GetFontParam(lpdis->itemID, FALSE));
    m_subtitle.SetSampleText(m_samplevec[1]);
    m_subtitle.Paint(dc, seconrc);
  }

  SetStretchBltMode(lpdis->hDC, HALFTONE);
  SetBrushOrgEx(lpdis->hDC, 0, 0, NULL);
  StretchBlt(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top, lpdis->rcItem.right - lpdis->rcItem.left, lpdis->rcItem.bottom - lpdis->rcItem.top,
    dc, 0, 0, 2 * (lpdis->rcItem.right - lpdis->rcItem.left), 2 * (lpdis->rcItem.bottom - lpdis->rcItem.top), SRCCOPY);

  dc.SelectBitmap(oldbmp);
}

void CListImpl::MeasureItem(LPMEASUREITEMSTRUCT lpmis)
{
  lpmis->itemHeight = m_styleentry_height;
}

int CListImpl::PreserveItemDivideRect(WTL::CRect rc, int index, BOOL bdivide)
{
  WTL::CRect maintextrc = rc;
  WTL::CRect mainhittest = rc;
  WTL::CRect secondtextrc = rc;
  WTL::CRect secondhittest = rc;

  StyleParam* mainstyle = m_fontparams.GetFontParam(index);
  StyleParam* seconstyle = m_fontparams.GetFontParam(index, FALSE);

  int mainheight = mainstyle->fontsize + mainstyle->strokesize + mainstyle->shadowsize;
  int secondheight = seconstyle->fontsize + seconstyle->strokesize + seconstyle->shadowsize;

  if (!bdivide)
    secondheight = 0;

  maintextrc.top = rc.top + (rc.Height() - mainheight - secondheight) / 2;
  maintextrc.bottom =  rc.top + (rc.Height() - mainheight - secondheight) / 2 + mainheight + 2;
  secondtextrc.top = maintextrc.bottom;
  secondtextrc.bottom = secondtextrc.top + secondheight;

  mainhittest.bottom = maintextrc.bottom;
  secondhittest.top = secondtextrc.top;

  if (m_listtextrc.size() / 2 > index)
  {
    m_listtextrc[2 * index] = maintextrc;
    m_listtextrc[2 * index + 1] = secondtextrc;
    m_listhittestrc[2 * index] = mainhittest;
    m_listhittestrc[2 * index + 1] = secondhittest;
  }
  else
  {
    m_listtextrc.push_back(maintextrc);
    m_listtextrc.push_back(secondtextrc);
    m_listhittestrc.push_back(mainhittest);
    m_listhittestrc.push_back(secondhittest);
  }

  return (rc.Height() - mainheight - secondheight) / 2;
}

WTL::CRect CListImpl::GetHittestDivideRect(int index)
{
  if (index < 0 || index >= m_listhittestrc.size())
    return WTL::CRect(0, 0, 0, 0);

  return m_listhittestrc[index];
}

WTL::CRect CListImpl::GetTextDivideRect(int index)
{
  if (index < 0 || index >= m_listtextrc.size())
    return WTL::CRect(0, 0, 0, 0);

  return m_listtextrc[index];
}

void CListImpl::PaintListItemBackground(HDC hdc, int width, int height)
{
  TRIVERTEX        vert[2] ;
  GRADIENT_RECT    gRect;
  vert[0] .x      = 1;
  vert[0] .y      = 1;
  vert[0] .Red    = 0xdc00;
  vert[0] .Green  = 0xea00;
  vert[0] .Blue   = 0xfc00;
  vert[0] .Alpha  = 0x0000;

  vert[1] .x      = width - 1;
  vert[1] .y      = height - 1; 
  vert[1] .Red    = 0xc100;
  vert[1] .Green  = 0xdc00;
  vert[1] .Blue   = 0xfc00;
  vert[1] .Alpha  = 0x0000;

  gRect.UpperLeft  = 0;
  gRect.LowerRight = 1;
  WTL::CPen pen;
  pen.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_MENUHILIGHT));
  HPEN old_pen = (HPEN)SelectObject(hdc, pen);
  Rectangle(hdc, 0, 0, width, height);
  SelectObject(hdc, old_pen);
  ::GradientFill(hdc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_V);
}

void CListImpl::PaintHighLightListBckground(HDC dc, WTL::CRect rc, BOOL bmain)
{
  int color[4][4] = {{0xf000, 0xf800, 0xff00, 0x0000},
  {0xcf00, 0xec00, 0xff00, 0x0000},
  {0xc800, 0xe400, 0xff00, 0x0000},
  {0xe400, 0xf200, 0xff00, 0x0000}};

  int index = bmain?0:2;

  TRIVERTEX        vert[2] ;
  GRADIENT_RECT    gRect;
  vert[0] .x      = rc.left + 1;
  vert[0] .y      = rc.top + 1;
  vert[0] .Red    = color[index][0];
  vert[0] .Green  = color[index][1];
  vert[0] .Blue   = color[index][2];
  vert[0] .Alpha  = color[index][3];

  vert[1] .x      = rc.right - 1;
  vert[1] .y      = rc.bottom - 1; 
  vert[1] .Red    = color[index+1][0];
  vert[1] .Green  = color[index+1][1];
  vert[1] .Blue   = color[index+1][2];
  vert[1] .Alpha  = color[index+1][3];

  gRect.UpperLeft  = 0;
  gRect.LowerRight = 1;
  ::GradientFill(dc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_V);
}

void CListImpl::GetItemData(int index, StyleParam** mainsp, StyleParam** secondsp)
{
  *mainsp = m_fontparams.GetFontParam(index);
  *secondsp = m_fontparams.GetFontParam(index, FALSE);
}