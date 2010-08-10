#include "stdafx.h"
#include "OptionSubtitlePage_Win.h"
#include "../../Utils/Strings.h"
#include "../Support/SubtitleStyle.h"
#include "../../mplayerc.h"

OptionSubtitlePage::OptionSubtitlePage(void):
  m_mainstyle(0),
  m_secstyle(0)
{
  // init style entry height
  m_styleentry_height = ::GetSystemMetrics(SM_CYICON)*9/5;
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
  m_secsubtitlestyle.SetCurSel(m_secstyle);
  // todo: calculate current settings according to CMainFrame::Setting class
  // retrieve subtitle style settings, and check which available style is closest to
  // the given settings, then set |m_mainstyle| to corresponding one.
  // todo: the same processing should be done for |m_secstyle|
  RefreshStyles();
  m_subtitlestyle.SetCurSel(m_mainstyle);
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

int OptionSubtitlePage::OnApply()
{
  // retrieve variables from screen
  return PSNRET_NOERROR;
}

void OptionSubtitlePage::RefreshStyles()
{
  // insert bogus entries
  m_subtitlestyle.SetCount(SubtitleStyle::GetStyleCount(false));
  m_subtitlestyle.Invalidate();
}