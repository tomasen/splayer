#include "stdafx.h"
#include "OptionSubtitlePage_Win.h"
#include "../../Utils/Strings.h"
#include "../Support/SubtitleStyle.h"
#include "../../mplayerc.h"
#include "../../MainFrm.h"

OptionSubtitlePage::OptionSubtitlePage(void):
  m_mainstyle(0),
  m_secstyle(0)
{
  // init style entry height
  m_styleentry_height = ::GetSystemMetrics(SM_CYICON)*7/5;
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
  for (int i = 0; i < SubtitleStyle::GetStyleCount(); i++)
  {
    SubtitleStyle::STYLEPARAM* sp = NULL;
    if (SubtitleStyle::GetStyleParams(i, -1, &sp) && sp->fontcolor == s.subdefstyle.colors[0] 
    && sp->strokecolor == s.subdefstyle.colors[2] && sp->shadowcolor == s.subdefstyle.colors[3] 
    && (sp->_fontname == SubtitleStyle::DetectFontType((LPCTSTR)s.subdefstyle.fontName)) //×ÖÌå
      )
    {
      m_subtitlestyle.SetCurSel(i);
      break;
    }
  }

  m_secsubtitlestyle.SetCurSel((s.subdefstyle2.scrAlignment == 2)?0:1);

  m_fetchsubtitlefromshooter = s.autoDownloadSVPSub;
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

void OptionSubtitlePage::DrawItem(LPDRAWITEMSTRUCT lpdis)
{
  SubtitleStyle::Paint(lpdis->hDC, &lpdis->rcItem, lpdis->itemID, -1,
    (lpdis->itemState & ODS_SELECTED)?true:false);
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
  SubtitleStyle::STYLEPARAM* sp = NULL;
  if (SubtitleStyle::GetStyleParams(m_subtitlestyle.GetCurSel(), -1, &sp))
  {
    s.subdefstyle.colors[0] = s.subdefstyle.colors[1] = sp->fontcolor;
    s.subdefstyle.colors[2] = sp->strokecolor;
    s.subdefstyle.colors[3] = sp->shadowcolor;
    s.subdefstyle.fontName = sp->fontname;
    s.subdefstyle.fontSize = sp->fontsize;
    s.subdefstyle.fontWeight = (sp->fontname == L"WenQuanYi Micro Hei" 
                                || sp->fontname == L"\x6587\x6CC9\x9A7F\x5FAE\x737C\x9ED1"  // Chinese for "WenQuanYi Micro Hei"
                                )?FW_BOLD:FW_NORMAL; // Using BOLD if it is font WenQuanYi
    s.subdefstyle.scrAlignment = 2;
    s.subdefstyle.shadowDepthX = s.subdefstyle.shadowDepthY = sp->shadowoffset;
    s.subdefstyle.outlineWidthX = s.subdefstyle.outlineWidthY = sp->strokesize;

    s.subdefstyle2 = s.subdefstyle;
  }
  s.subdefstyle2.scrAlignment = ( m_secsubtitlestyle.GetCurSel() == 1)?8:2;

  s.autoDownloadSVPSub = m_fetchsubtitlefromshooter;

  /* UpdateSubtitle Display on the fly */
  CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();
  pFrame->UpdateSubtitle(true);
  pFrame->UpdateSubtitle2(true);
}

int OptionSubtitlePage::OnApply()
{
  ApplySubtitleStyle();

  return PSNRET_NOERROR;
}

void OptionSubtitlePage::RefreshStyles()
{
  // insert bogus entries
  m_subtitlestyle.SetCount(SubtitleStyle::GetStyleCount(false));
  m_subtitlestyle.Invalidate();
}