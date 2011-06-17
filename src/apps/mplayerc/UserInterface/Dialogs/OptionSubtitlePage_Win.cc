#include "stdafx.h"
#include "OptionSubtitlePage_Win.h"
#include <Strings.h>
#include "../Support/SubtitleStyle.h"
#include "../../mplayerc.h"
#include "../../MainFrm.h"
#include "../../Controller/SPlayerDefs.h"
#include "../../Controller/PlayerPreference.h"
#include "CustomizeFontDlg.h"

OptionSubtitlePage::OptionSubtitlePage(void):
  m_mainstyle(0),
  m_secstyle(0)
{
  // init font sample string
  WTL::CString text;
  text.LoadString(IDS_SUBTITLESTYLES);
  Strings::Split(text, L"|", m_samplevec);
  
  // init style entry height
  m_styleentry_height = 80;//::GetSystemMetrics(SM_CYICON)*7/5;

  m_rcvec.clear();   
}

BOOL OptionSubtitlePage::OnInitDialog(HWND hwnd, LPARAM lParam)
{
  // subtitle combo
  m_secsubtitlestyle.Attach(GetDlgItem(IDC_COMBO_SECSTYLE));
  WTL::CString text;
  text.LoadString(IDS_SECSUBTITLESTYLES);
  std::vector<std::wstring> text_ar;
  Strings::Split(text, L"|", text_ar);
  for (std::vector<std::wstring>::iterator it = text_ar.begin();
    it != text_ar.end(); it++)
    m_secsubtitlestyle.AddString(it->c_str());
  RECT rc_stylelist;
  m_subtitlestyle.Attach(GetDlgItem(IDC_LIST));
  m_subtitlestyle.GetWindowRect(&rc_stylelist);
  m_styleentry_width = rc_stylelist.right - rc_stylelist.left;
  RefreshStyles();

  AppSettings& s = AfxGetAppSettings();
  // calculate current settings according to AppSettings class s.subdefstyle
  // retrieve subtitle style settings, and check which available style is closest to
  // the given settings, then set |m_mainstyle| to corresponding one.
  // s.subdefstyle compare font type, font color, border color shadow color
  for (int i = 0; i < m_fontparams.GetStyleCount(); i++)
  {
    StyleParam* sp = NULL;
    sp = m_fontparams.GetFontParam(i, TRUE);
    if (sp->fontcolor == s.subdefstyle.colors[0] && sp->strokecolor == s.subdefstyle.colors[2] 
        && sp->shadowcolor == s.subdefstyle.colors[3] 
        /*&& (sp->_fontname == SubtitleStyle::DetectFontType((LPCTSTR)s.subdefstyle.fontName))*/ //×ÖÌå
       )
    {
      m_subtitlestyle.SetCurSel(i);
      break;
    }
  }

  m_secsubtitlestyle.SetCurSel((s.subdefstyle2.scrAlignment == 2)?1:0);

  m_fetchsubtitlefromshooter = s.autoDownloadSVPSub;

  // Init the subtitle save path radio buttons
  WTL::CButton rdoSaveSame = (WTL::CButton)GetDlgItem(IDC_RADIO_SAVESUBTITLE_SAME_FOLDER);
  WTL::CButton rdoSaveCustom = (WTL::CButton)GetDlgItem(IDC_RADIO_SAVESUBTITLE_CUSTOM_FOLDER);
  CWindow edtCustomPath = (CWindow)GetDlgItem(IDC_EDIT_SAVESUBTITLE_CUSTOM_FOLDER);
  CWindow btnCustomPath = (CWindow)GetDlgItem(IDC_BUTTON_SAVESUBTITLE_CUSTOM_FOLDER);

  // default subtitle save path in appdata dir.
  PlayerPreference::GetInstance()->SetStringVar(STRVAR_SUBTITLE_SAVEMETHOD, wstring(L"custom"));

  wstring sSubtitleSaveOption = PlayerPreference::GetInstance()->GetStringVar(STRVAR_SUBTITLE_SAVEMETHOD);
  if (sSubtitleSaveOption == L"same")
  {
    rdoSaveSame.SetCheck(BST_CHECKED);
    rdoSaveCustom.SetCheck(BST_UNCHECKED);
    edtCustomPath.EnableWindow(FALSE);
    btnCustomPath.EnableWindow(FALSE);

    m_sCustomPath = PlayerPreference::GetInstance()->GetStringVar(STRVAR_SUBTITLE_SAVE_CUSTOMPATH).c_str();
  }
  else if (sSubtitleSaveOption == L"custom")
  {
    rdoSaveSame.SetCheck(BST_UNCHECKED);
    rdoSaveCustom.SetCheck(BST_CHECKED);
    edtCustomPath.EnableWindow(TRUE);
    btnCustomPath.EnableWindow(TRUE);

    m_sCustomPath = PlayerPreference::GetInstance()->GetStringVar(STRVAR_SUBTITLE_SAVE_CUSTOMPATH).c_str();
  }

  DoDataExchange();

  return TRUE;
}

void OptionSubtitlePage::OnDestroy()
{
  m_secsubtitlestyle.Detach();
  m_subtitlestyle.Detach();
}

void OptionSubtitlePage::OnSubtitleStyleChange(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  m_mainstyle = m_subtitlestyle.GetCurSel();
}

// Let user select the save path
void OptionSubtitlePage::OnBrowserForFolder(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  LPITEMIDLIST   lpitem = 0;  
  BROWSEINFO bf = {0};
  bf.hwndOwner = m_hWnd;
  bf.lpszTitle = L"Select path:";
  bf.ulFlags = BIF_RETURNONLYFSDIRS;
  lpitem = ::SHBrowseForFolder(&bf);

  WCHAR szBuffer[MAX_PATH] = {0};
  if (lpitem)
  {
    ::SHGetPathFromIDList(lpitem, szBuffer); 

    m_sCustomPath = szBuffer;
    DoDataExchange();
  }
}

void OptionSubtitlePage::OnSelectSameFolder(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  CWindow edtCustomFolder = GetDlgItem(IDC_EDIT_SAVESUBTITLE_CUSTOM_FOLDER);
  edtCustomFolder.EnableWindow(FALSE);

  CWindow btnCustomFolder = GetDlgItem(IDC_BUTTON_SAVESUBTITLE_CUSTOM_FOLDER);
  btnCustomFolder.EnableWindow(FALSE);
}

void OptionSubtitlePage::OnSelectCustomFolder(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  CWindow edtCustomFolder = GetDlgItem(IDC_EDIT_SAVESUBTITLE_CUSTOM_FOLDER);
  edtCustomFolder.EnableWindow(TRUE);

  CWindow btnCustomFolder = GetDlgItem(IDC_BUTTON_SAVESUBTITLE_CUSTOM_FOLDER);
  btnCustomFolder.EnableWindow(TRUE);
}

void OptionSubtitlePage::DrawItem(LPDRAWITEMSTRUCT lpdis)
{
//   SubtitleStyle::Paint(lpdis->hDC, &lpdis->rcItem, lpdis->itemID, -1,
//     (lpdis->itemState & ODS_SELECTED)?true:false);
  
  StyleParam* spmain = NULL;
  StyleParam* spsecondary = NULL;
  spmain = m_fontparams.GetFontParam(lpdis->itemID, TRUE);
  spsecondary = m_fontparams.GetFontParam(lpdis->itemID, FALSE);

  int top = 0;
  if (spsecondary)
    top = PreserveItemDivideRect(lpdis->rcItem, lpdis->itemID, TRUE);
  else
    top = PreserveItemDivideRect(lpdis->rcItem, lpdis->itemID, FALSE);

  //WTL::CMemoryDC dc(lpdis->hDC, lpdis->rcItem);
  WTL::CDC dc;
  WTL::CBitmap bmp;
  dc.CreateCompatibleDC(lpdis->hDC);
  bmp.CreateCompatibleBitmap(lpdis->hDC, 2 * (lpdis->rcItem.right - lpdis->rcItem.left), 
    2 * (lpdis->rcItem.bottom - lpdis->rcItem.top));
  HBITMAP oldbmp = dc.SelectBitmap(bmp);

  WTL::CRect rc(0, 0, 2 * (lpdis->rcItem.right - lpdis->rcItem.left), 2 * (lpdis->rcItem.bottom - lpdis->rcItem.top));

  dc.FillRect(&rc, COLOR_3DFACE);
  if (lpdis->itemState & ODS_SELECTED)
    PaintListItemBackground(dc, rc.Width(), rc.Height());
 
  WTL::CRect mainrc = GetItemDivideRect( 2 * lpdis->itemID);
  int height = mainrc.Height();
  mainrc.top = 2 * top;
  mainrc.bottom = mainrc.top + 2 * height;
  
  if (spmain)
  {
    m_subtitle.SetFont(*m_fontparams.GetFontParam(lpdis->itemID));
    m_subtitle.SetSampleText(m_samplevec[0]);
    m_subtitle.Paint(dc, mainrc);
  }
 
  WTL::CRect seconrc = GetItemDivideRect(2 * lpdis->itemID + 1);
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

void OptionSubtitlePage::MeasureItem(LPMEASUREITEMSTRUCT lpmis)
{
  if (lpmis->CtlID == IDC_LIST)
    lpmis->itemHeight = m_styleentry_height;
}

int OptionSubtitlePage::OnSetActive()
{
  return 0;
}

void OptionSubtitlePage::ApplySubtitleStyle()
{
  DoDataExchange(TRUE);
  AppSettings& s = AfxGetAppSettings();
  // retrieve variables from screen
  StyleParam* mainsp = m_fontparams.GetFontParam(m_subtitlestyle.GetCurSel());
  StyleParam* secondsp = m_fontparams.GetFontParam(m_subtitlestyle.GetCurSel(), FALSE);

  if (mainsp)
  {
    s.subdefstyle.colors[0] = s.subdefstyle.colors[1] = mainsp->fontcolor;
    s.subdefstyle.colors[2] = mainsp->strokecolor;
    s.subdefstyle.colors[3] = mainsp->shadowcolor;
    s.subdefstyle.fontName = mainsp->fontname.c_str();
    s.subdefstyle.fontSize = mainsp->fontsize;
    s.subdefstyle.fontWeight = (mainsp->fontname == L"WenQuanYi Micro Hei" 
                                || mainsp->fontname == L"\x6587\x6CC9\x9A7F\x5FAE\x737C\x9ED1"  // Chinese for "WenQuanYi Micro Hei"
                                )?FW_BOLD:FW_NORMAL; // Using BOLD if it is font WenQuanYi
    s.subdefstyle.scrAlignment = 2;
    s.subdefstyle.shadowDepthX = s.subdefstyle.shadowDepthY = mainsp->shadowsize;
    s.subdefstyle.outlineWidthX = s.subdefstyle.outlineWidthY = mainsp->strokesize;

    //s.subdefstyle2 = s.subdefstyle;
  }

  if (secondsp)
  {
    s.subdefstyle2.colors[0] = s.subdefstyle2.colors[1] = secondsp->fontcolor;
    s.subdefstyle2.colors[2] = secondsp->strokecolor;
    s.subdefstyle2.colors[3] = secondsp->shadowcolor;
    s.subdefstyle2.fontName = secondsp->fontname.c_str();
    s.subdefstyle2.fontSize = secondsp->fontsize;
    s.subdefstyle2.fontWeight = (secondsp->fontname == L"WenQuanYi Micro Hei" 
      || secondsp->fontname == L"\x6587\x6CC9\x9A7F\x5FAE\x737C\x9ED1"  // Chinese for "WenQuanYi Micro Hei"
      )?FW_BOLD:FW_NORMAL; // Using BOLD if it is font WenQuanYi
    s.subdefstyle2.scrAlignment = 2;
    s.subdefstyle2.shadowDepthX = s.subdefstyle2.shadowDepthY = secondsp->shadowsize;
    s.subdefstyle2.outlineWidthX = s.subdefstyle2.outlineWidthY = secondsp->strokesize;
  }

  s.subdefstyle2.scrAlignment = ( m_secsubtitlestyle.GetCurSel() == 1)?2:8;

  s.autoDownloadSVPSub = m_fetchsubtitlefromshooter;

  /* UpdateSubtitle Display on the fly */
  CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();
  pFrame->UpdateSubtitle(true);
  pFrame->UpdateSubtitle2(true);
}

int OptionSubtitlePage::ApplySubtitleSavePath()
{
  DoDataExchange(TRUE);
  CWindow rdoSameFolder = GetDlgItem(IDC_RADIO_SAVESUBTITLE_SAME_FOLDER);
  CWindow rdoCustomFolder = GetDlgItem(IDC_RADIO_SAVESUBTITLE_CUSTOM_FOLDER);

  int nRet = PSNRET_NOERROR;

  if (IsDlgButtonChecked(IDC_RADIO_SAVESUBTITLE_SAME_FOLDER) & BST_CHECKED)
  {
    // save subtitle to the media file's folder
    PlayerPreference::GetInstance()->SetStringVar(STRVAR_SUBTITLE_SAVEMETHOD, wstring(L"same"));
  }
  else if (IsDlgButtonChecked(IDC_RADIO_SAVESUBTITLE_CUSTOM_FOLDER) & BST_CHECKED)
  {
    if (m_sCustomPath.IsEmpty())
    {
      // if custom path is empty then set it to be the app data path
      AfxMessageBox(ResStr(IDS_MSG_SUBTITLE_SAVEPATH_EMPTY));
      nRet = PSNRET_INVALID;
    }
    else
    {
      // save subtitle to the custom folder
      PlayerPreference::GetInstance()->SetStringVar(STRVAR_SUBTITLE_SAVEMETHOD, wstring(L"custom"));
      PlayerPreference::GetInstance()->SetStringVar(STRVAR_SUBTITLE_SAVE_CUSTOMPATH, wstring((LPCTSTR)m_sCustomPath));
    }
  }

  return nRet;
}

int OptionSubtitlePage::OnApply()
{
  ApplySubtitleStyle();
//   int nRet = ApplySubtitleSavePath();
//   if (nRet == PSNRET_INVALID)
//   {
//     return PSNRET_INVALID;
//   }

  return PSNRET_NOERROR;
}

void OptionSubtitlePage::RefreshStyles()
{
  // insert bogus entries
  m_subtitlestyle.SetCount(m_fontparams.GetStyleCount());
  m_subtitlestyle.Invalidate();
  m_subtitlestyle.UpdateWindow();
}

int OptionSubtitlePage::PreserveItemDivideRect(WTL::CRect rc, int index, BOOL bdivide)
{
  WTL::CRect mainrc = rc;
  WTL::CRect secondaryrc = rc;

  StyleParam* mainstyle = m_fontparams.GetFontParam(index);
  StyleParam* seconstyle = m_fontparams.GetFontParam(index, FALSE);

  int mainheight = mainstyle->fontsize + mainstyle->strokesize + mainstyle->shadowsize;
  int seconheight = seconstyle->fontsize + seconstyle->strokesize + seconstyle->shadowsize;

  if (!bdivide)
    seconheight = 0;

  mainrc.top = rc.top + (rc.Height() - mainheight - seconheight) / 2;
  mainrc.bottom = mainrc.top + mainheight + 3;
  secondaryrc.top = mainrc.bottom;
  secondaryrc.bottom = secondaryrc.top + seconheight;
  
  if (m_rcvec.size() / 2 > index)
  {
    m_rcvec[2 * index] = mainrc;
    m_rcvec[2 * index + 1] = secondaryrc;
  }
  else
  {
    m_rcvec.push_back(mainrc);
    m_rcvec.push_back(secondaryrc);
  }

  return (rc.Height() - mainheight - seconheight) / 2;
}

WTL::CRect OptionSubtitlePage::GetItemDivideRect(int index)
{
  if (index < 0 || index >= m_rcvec.size())
    return WTL::CRect(0, 0, 0, 0);

  return m_rcvec[index];
}

void OptionSubtitlePage::PaintListItemBackground(HDC hdc, int width, int height)
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

void OptionSubtitlePage::OnListDoubleClick(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  POINT pt;
  GetCursorPos(&pt);
  ::ScreenToClient(m_subtitlestyle.m_hWnd, &pt);
  
  int index = m_subtitlestyle.GetCurSel();
  WTL::CRect mainrc = GetItemDivideRect(2 * index);
  WTL::CRect seconrc = GetItemDivideRect(2 * index + 1);
  WTL::CRect rc;
  m_subtitlestyle.GetClientRect(&rc);

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