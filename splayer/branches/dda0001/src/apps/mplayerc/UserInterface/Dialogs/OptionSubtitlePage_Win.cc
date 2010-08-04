#include "stdafx.h"
#include "OptionSubtitlePage_Win.h"
#include "../../Utils/Strings.h"

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
  return TRUE;
}

void OptionSubtitlePage::OnDestroy()
{
  m_subtitletype.Detach();
}

void OptionSubtitlePage::DrawItem(LPDRAWITEMSTRUCT lpdis)
{

}

void OptionSubtitlePage::MeasureItem(LPMEASUREITEMSTRUCT lpmis)
{

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