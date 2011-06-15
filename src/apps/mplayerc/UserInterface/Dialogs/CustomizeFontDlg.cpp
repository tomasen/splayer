#include "stdafx.h"
#include "CustomizeFontDlg.h"

CustomizeFontDlg::CustomizeFontDlg(void)
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
  m_fontname = (LPCTSTR)cstr;

  Refresh();
}

void CustomizeFontDlg::OnFontSizeChange(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  WTL::CString cstr;
  m_fontsizelist.GetText(m_fontsizelist.GetCurSel(), cstr);
  m_fontsize = _wtoi(cstr);

  Refresh();
}

void CustomizeFontDlg::DrawItem(LPDRAWITEMSTRUCT lpdis)
{
  WTL::CDC dc;
  WTL::CBitmap bmp;
  WTL::CRect rc(0, 0, 2 * (lpdis->rcItem.right - lpdis->rcItem.left),
    2 * (lpdis->rcItem.bottom - lpdis->rcItem.top));
  
  dc.CreateCompatibleDC(lpdis->hDC);
  bmp.CreateCompatibleBitmap(lpdis->hDC, rc.Width(), rc.Height());
  HBITMAP oldbmp = dc.SelectBitmap(bmp);

  WTL::CPen pen;
  pen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
  HPEN oldpen = dc.SelectPen(pen);
  dc.Rectangle(0, 0, rc.Width(), rc.Height());

  //dc.FillRect(&rc, COLOR_3DFACE);
  if (lpdis->CtlID == IDC_FONTPREVIEW)
    Preview(dc, rc);
  else
    dc.FillSolidRect(&rc, m_colormap[lpdis->CtlID]);

  SetStretchBltMode(lpdis->hDC, HALFTONE);
  SetBrushOrgEx(lpdis->hDC, 0, 0, NULL);
  StretchBlt(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top, lpdis->rcItem.right - lpdis->rcItem.left, lpdis->rcItem.bottom - lpdis->rcItem.top,
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

  for (std::set<std::wstring>::iterator it = m_fontset.begin();
       it != m_fontset.end(); ++it)
    m_fontnamelist.AddString(it->c_str());

  m_fontnamelist.SetFocus();
  m_fontnamelist.SetCurSel(0);
}

void CustomizeFontDlg::InitFontSizeList()
{
  for (int i = 10;i != 31; ++i)
  {
    wchar_t wstr[100];
    _itow(i, wstr, 10);
    m_fontsizelist.AddString(wstr);
  }

  m_fontsizelist.SetFocus();
  m_fontsizelist.SetCurSel(0);
}

void CustomizeFontDlg::InitStrokeSizeComboBox()
{
  for (int i = 0; i != 4; ++i)
  {
    wchar_t str[100];
    _itow(i, str, 10);
    m_strokesizebox.AddString(str);
  }

  m_strokesizebox.SetCurSel(0);
}

void CustomizeFontDlg::InitShadowSizeComboBox()
{
  for (int i = 0; i != 4; ++i)
  {
    wchar_t str[100];
    _itow(i, str, 10);
    m_shadowsizebox.AddString(str);
  }

  m_shadowsizebox.SetCurSel(0);
}

void CustomizeFontDlg::Refresh()
{
  m_preview.Invalidate();
}

void CustomizeFontDlg::Preview(HDC hdc, WTL::CRect rc)
{
  StyleParam param(None, m_fontname, m_fontsize, m_fontcolor, m_strokesize,
    m_strokecolor, m_shadowsize, m_shadowcolor);

  DrawSubtitle drawsub;
  drawsub.SetFont(param);
  drawsub.SetSampleText(m_sampletext);
  drawsub.Paint(hdc, rc);
}

void CustomizeFontDlg::SetFontParam(StyleParam* param, std::wstring sample)
{
  m_fontname = param->fontname;
  m_fontsize = param->fontsize;
  m_fontcolor = param->fontcolor;
  m_strokesize = param->strokesize;
  m_strokecolor = param->strokecolor;
  m_shadowsize = param->shadowsize;
  m_shadowcolor = param->shadowcolor;
  m_sampletext = sample;
}

StyleParam* CustomizeFontDlg::GetFontParam()
{
  StyleParam* sp = NULL;
  
  sp = new StyleParam(None, m_fontname, m_fontsize, m_fontcolor, m_strokesize, m_strokecolor,
    m_shadowsize, m_shadowcolor);
  
  return sp;
}

void CustomizeFontDlg::OnColorSelect(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  switch(nID)
  {
  case IDC_FONTCOLOR_BUTTON1:
  case IDC_FONTCOLOR_BUTTON2:
  case IDC_FONTCOLOR_BUTTON3:
    m_fontcolor = m_colormap[nID];
    break;
  case IDC_STROKECOLOR:
    m_strokecolor = m_colormap[nID];
    break;
  case IDC_SHADOWCOLOR:
    m_shadowcolor = m_colormap[nID];
    break;
  default:
    break;
  }

  Refresh();
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
  
  Refresh();
}

void CustomizeFontDlg::OnOK(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  EndDialog(nID);
}

void CustomizeFontDlg::OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  EndDialog(nID);
}

void CustomizeFontDlg::OnStrokeSizeChange(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  WTL::CString cstr;
  m_strokesizebox.GetLBText(m_strokesizebox.GetCurSel(), cstr);
  int i = _wtoi(cstr);
  m_strokesize = i;
  Refresh();
}

void CustomizeFontDlg::OnShadowSizeChange(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  WTL::CString cstr;
  m_shadowsizebox.GetLBText(m_shadowsizebox.GetCurSel(), cstr);
  int i = _wtoi(cstr);
  m_shadowsize = i;
  Refresh();
}