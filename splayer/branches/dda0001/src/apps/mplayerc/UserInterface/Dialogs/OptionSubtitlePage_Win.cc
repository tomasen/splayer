#include "stdafx.h"
#include "OptionSubtitlePage_Win.h"
#include "../../Utils/Strings.h"
#include "../Support/SubtitleStyle.h"

OptionSubtitlePage::OptionSubtitlePage(void)
{
  // init style entry height
  m_styleentry_height = ::GetSystemMetrics(SM_CYICON)*2;
}

BOOL OptionSubtitlePage::OnInitDialog(HWND hwnd, LPARAM lParam)
{
  // subtitle combo
  m_subtitletype.Attach(GetDlgItem(IDC_COMBO_SUBTITLETYPE));
  WTL::CString text;
  text.LoadString(IDS_SUBTITLETYPES);
  std::vector<std::wstring> text_ar;
  Strings::Split(text, L"|", text_ar);
  for (std::vector<std::wstring>::iterator it = text_ar.begin();
    it != text_ar.end(); it++)
    m_subtitletype.AddString(it->c_str());
  RECT rc_stylelist;
  m_subtitlestyle.Attach(GetDlgItem(IDC_LIST));
  m_subtitlestyle.GetWindowRect(&rc_stylelist);
  m_styleentry_width = rc_stylelist.right - rc_stylelist.left;
  // insert bogus entries
  m_subtitlestyle.SetCount(SubtitleStyle::GetStyleCount());
  return TRUE;
}

void OptionSubtitlePage::OnDestroy()
{
  m_subtitletype.Detach();
  m_subtitlestyle.Detach();
}

void OptionSubtitlePage::DrawItem(LPDRAWITEMSTRUCT lpdis)
{
  SubtitleStyle::Paint(lpdis->hDC, &lpdis->rcItem, lpdis->itemID, 
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