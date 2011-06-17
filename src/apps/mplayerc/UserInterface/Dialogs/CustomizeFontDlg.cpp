#include "stdafx.h"
#include "CustomizeFontDlg.h"

CustomizeFontDlg::CustomizeFontDlg(void):
  m_fontpammain(0)
, m_fontpamsecon(0)
, m_fontpamcurr(0)
{
  m_colormap[IDC_FONTCOLOR_BUTTON1] = 0xFF0000;
  m_colormap[IDC_FONTCOLOR_BUTTON2] = 0x00FF00;
  m_colormap[IDC_FONTCOLOR_BUTTON3] = 0x0000FF;
  m_colormap[IDC_STROKECOLOR] = 0x000000;
  m_colormap[IDC_SHADOWCOLOR] = 0x000000;
}

CustomizeFontDlg::~CustomizeFontDlg(void)
{
}

int CALLBACK mEnumFontFamExProc(ENUMLOGFONTEX *lpelfe,NEWTEXTMETRICEX *lpntme,unsigned long FontType,long lParam)
{
  std::set<std::wstring>* set = (std::set<std::wstring>*)lParam;

  std::wstring ftnm = lpelfe->elfFullName;
  if (ftnm.find('@') == std::wstring::npos)
    (*set).insert(ftnm);

  return 1;
}

BOOL CustomizeFontDlg::OnInitDialog(HWND hwnd, LPARAM lParam)
{
  CenterWindow(GetParent());

  SetWindowText(m_titletext.c_str());

  m_fontnamelist.Attach(GetDlgItem(IDC_FONTNAME_LIST));
  m_fontsizelist.Attach(GetDlgItem(IDC_FONTSIZE_LIST));
  m_strokesizebox.Attach(GetDlgItem(IDC_STROKESIZE_COMBO));
  m_shadowsizebox.Attach(GetDlgItem(IDC_SHADOWSIZE_COMBO));
  m_preview.Attach(GetDlgItem(IDC_FONTPREVIEW));
  
  InitFontNameList();
  InitFontSizeList();
  InitStrokeSizeComboBox();
  InitShadowSizeComboBox();

  return 0;
}

BOOL CustomizeFontDlg::OnClose()
{
  *m_fontpamcurr = m_fontpamorg;
  EndDialog(NULL);
  return 0;
}

void CustomizeFontDlg::OnDestroy()
{
  m_fontnamelist.Detach();
  m_fontsizelist.Detach();
  m_strokesizebox.Detach();
  m_shadowsizebox.Detach();
}

void CustomizeFontDlg::OnFontNameChange(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  WTL::CString cstr;
  m_fontnamelist.GetText(m_fontnamelist.GetCurSel(), cstr);
  m_fontpamcurr->fontname = (LPCTSTR)cstr;

  FlushPreview();
}

void CustomizeFontDlg::OnFontSizeChange(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  WTL::CString cstr;
  m_fontsizelist.GetText(m_fontsizelist.GetCurSel(), cstr);
  m_fontpamcurr->fontsize = _wtoi(cstr);

  FlushPreview();
}

void CustomizeFontDlg::DrawItem(LPDRAWITEMSTRUCT lpdis)
{
  WTL::CDC dc;
  WTL::CBitmap bmp;

  int width = lpdis->rcItem.right - lpdis->rcItem.left;
  int height = lpdis->rcItem.bottom - lpdis->rcItem.top;
  WTL::CRect rc(0, 0, 2 * width, 2 * height);
  
  dc.CreateCompatibleDC(lpdis->hDC);
  bmp.CreateCompatibleBitmap(lpdis->hDC, rc.Width(), rc.Height());
  HBITMAP oldbmp = dc.SelectBitmap(bmp);

  WTL::CPen pen;
  pen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
  HPEN oldpen = dc.SelectPen(pen);
  dc.Rectangle(0, 0, rc.Width(), rc.Height());

  //dc.FillRect(&rc, COLOR_3DFACE);
  if (lpdis->CtlID == IDC_FONTPREVIEW)
  {
    WTL::CRect rcmain(lpdis->rcItem);
    WTL::CRect rcsecon(rc);
    rcsecon.top = rcmain.bottom;

    Preview(dc, rcmain, m_sampletextmain, m_fontpammain);
    Preview(dc, rcsecon, m_sampletextsecon, m_fontpamsecon);
  }
  else
    dc.FillSolidRect(&rc, m_colormap[lpdis->CtlID]);

  SetStretchBltMode(lpdis->hDC, HALFTONE);
  SetBrushOrgEx(lpdis->hDC, 0, 0, NULL);
  StretchBlt(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top, width, height,
    dc, 0, 0, rc.Width(), rc.Height(), SRCCOPY);

  dc.SelectBitmap(oldbmp);
  dc.SelectPen(oldpen);
}

// void CustomizeFontDlg::MeasureItem(LPMEASUREITEMSTRUCT lpmis)
// {
//   
// }

void CustomizeFontDlg::InitFontNameList()
{
  HDC dc = GetDC();
  LOGFONT lf;
  memset(&lf, 0, sizeof(lf));
  lf.lfCharSet = DEFAULT_CHARSET;
  lf.lfFaceName[0] = L'\0';
  lf.lfPitchAndFamily = 0;
  ::EnumFontFamiliesEx(dc,&lf,(FONTENUMPROC)mEnumFontFamExProc,(long)&m_fontset,0);  

  int i = 0;
  for (std::set<std::wstring>::iterator it = m_fontset.begin();
       it != m_fontset.end(); ++it, ++i)
  {
     m_fontnamelist.AddString(it->c_str());
     if (*it == m_fontpamcurr->fontname)
       m_fontnamelist.SetCurSel(i);
  }
}

void CustomizeFontDlg::InitFontSizeList()
{
  for (int i = 10;i != 31; ++i)
  {
    wchar_t wstr[100];
    _itow(i, wstr, 10);
    m_fontsizelist.AddString(wstr);

    if (i == m_fontpamcurr->fontsize)
      m_fontsizelist.SetCurSel(i - 10);
  }
}

void CustomizeFontDlg::InitStrokeSizeComboBox()
{
  for (int i = 1; i != 4; ++i)
  {
    wchar_t str[100];
    _itow(i, str, 10);
    m_strokesizebox.AddString(str);

    if (i == m_fontpamcurr->strokesize)
      m_strokesizebox.SetCurSel(i - 1);
  }

}

void CustomizeFontDlg::InitShadowSizeComboBox()
{
  for (int i = 0; i != 4; ++i)
  {
    wchar_t str[100];
    _itow(i, str, 10);
    m_shadowsizebox.AddString(str);

    if (i == m_fontpamcurr->shadowsize)
      m_shadowsizebox.SetCurSel(i);
  }

}

void CustomizeFontDlg::Refresh()
{
  for (int i = 0; i != m_fontnamelist.GetCount(); ++i)
  {
    WTL::CString cstr;
    m_fontnamelist.GetText(i, cstr);

    if (cstr == m_fontpamcurr->fontname.c_str())
    {
      m_fontnamelist.SetCurSel(i);
      break;
    }
  }

  m_fontsizelist.SetCurSel(m_fontpamcurr->fontsize - 10);
  m_strokesizebox.SetCurSel(m_fontpamcurr->strokesize - 1);
  m_shadowsizebox.SetCurSel(m_fontpamcurr->shadowsize);

  Invalidate();
}

void CustomizeFontDlg::FlushPreview()
{
  m_preview.Invalidate();
}

void CustomizeFontDlg::Preview(HDC hdc, WTL::CRect rc, std::wstring sample, StyleParam* style)
{
  DrawSubtitle drawsub;
  drawsub.SetFont(*style);
  drawsub.SetSampleText(sample);
  drawsub.Paint(hdc, rc);
}

void CustomizeFontDlg::SetSampleText(std::wstring samplemain, std::wstring samplesecon)
{
  m_sampletextmain = samplemain;
  m_sampletextsecon = samplesecon;
}

void CustomizeFontDlg::SetFontParam(StyleParam* parammain, StyleParam* paramsecon, BOOL bmain)
{
  m_fontpammain = parammain;
  m_fontpamsecon = paramsecon;

  if (bmain)
  {
    m_fontpamorg = *parammain;
    m_fontpamcurr = parammain;
  }
  else
  {
    m_fontpamorg = *paramsecon;
    m_fontpamcurr = paramsecon;
  }
  
  m_colormap[IDC_FONTCOLOR_BUTTON1] = m_fontpamcurr->fontcolor;
  m_colormap[IDC_STROKECOLOR] = m_fontpamcurr->strokecolor;
  m_colormap[IDC_SHADOWCOLOR] = m_fontpamcurr->shadowcolor;
}

// StyleParam* CustomizeFontDlg::GetFontParam()
// {
//   StyleParam* sp = NULL;
//   sp = new StyleParam(m_fontpammain);
//   
//   return sp;
// }

void CustomizeFontDlg::OnColorSelect(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  switch(nID)
  {
  case IDC_FONTCOLOR_BUTTON1:
  case IDC_FONTCOLOR_BUTTON2:
  case IDC_FONTCOLOR_BUTTON3:
    m_fontpamcurr->fontcolor = m_colormap[nID];
    break;
  case IDC_STROKECOLOR:
    m_fontpamcurr->strokecolor = m_colormap[nID];
    break;
  case IDC_SHADOWCOLOR:
    m_fontpamcurr->shadowcolor = m_colormap[nID];
    break;
  default:
    break;
  }

  FlushPreview();
}

void CustomizeFontDlg::OnColorDoubleClick(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  WTL::CColorDialog dlgColor;
  if (dlgColor.DoModal() == IDOK)
  {
    m_colormap[nID] = dlgColor.GetColor();
    PostMessage(WM_COMMAND, MAKEWPARAM(nID, BN_CLICKED), (LPARAM)wndCtl.m_hWnd);
    wndCtl.Invalidate();
  }
  
  FlushPreview();
}

void CustomizeFontDlg::OnOK(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  EndDialog(nID);
}

void CustomizeFontDlg::OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  *m_fontpamcurr = m_fontpamorg;
  EndDialog(nID);
}

void CustomizeFontDlg::OnStrokeSizeChange(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  WTL::CString cstr;
  m_strokesizebox.GetLBText(m_strokesizebox.GetCurSel(), cstr);
  int i = _wtoi(cstr);
  m_fontpamcurr->strokesize = i;
  FlushPreview();
}

void CustomizeFontDlg::OnShadowSizeChange(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  WTL::CString cstr;
  m_shadowsizebox.GetLBText(m_shadowsizebox.GetCurSel(), cstr);
  int i = _wtoi(cstr);
  m_fontpamcurr->shadowsize = i;
  FlushPreview();
}

void CustomizeFontDlg::SetTitleText(std::wstring title)
{
  m_titletext = title;
}

void CustomizeFontDlg::OnRestore(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  *m_fontpamcurr = m_fontpamorg;

  m_colormap[IDC_FONTCOLOR_BUTTON1] = m_fontpamcurr->fontcolor;
  m_colormap[IDC_STROKECOLOR] = m_fontpamcurr->strokecolor;
  m_colormap[IDC_SHADOWCOLOR] = m_fontpamcurr->shadowcolor;

  Refresh();
}