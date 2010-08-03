#include "stdafx.h"
#include "OptionBasicPage_Win.h"
#include "../../Utils/Strings.h"
#include "../../mplayerc.h"

BOOL OptionBasicPage::OnInitDialog(HWND hwnd, LPARAM lParam)
{
  // background image picker
  m_userbkgnd_edit.SubclassWindow(GetDlgItem(IDC_EDIT_BKGNDFILE));
  m_userbkgnd_edit.Init(ID_BKGND_PICKER);
  // upgrade strategy combo
  m_upgradestrategy_combo.Attach(GetDlgItem(IDC_COMBO_AUTOUPDATEVER));
  WTL::CString text;
  text.LoadString(IDS_UPGRADESTRATEGY);
  std::vector<std::wstring> text_ar;
  Strings::Split(text, L"|", text_ar);
  for (std::vector<std::wstring>::iterator it = text_ar.begin();
    it != text_ar.end(); it++)
    m_upgradestrategy_combo.AddString(it->c_str());
  return TRUE;
}

void OptionBasicPage::OnDestroy()
{
  m_upgradestrategy_combo.Detach();
}

void OptionBasicPage::OnBkgndPicker(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  // call ::OpenFileDialog to prompt choosing a file
  WTL::CFileDialog fd(TRUE, NULL, NULL, OFN_EXPLORER, 
    L"*.bmp;*.jpg;*.gif;*.png\0*.bmp;*.jpg;*.gif;*.png\0\0", m_hWnd);

  if(fd.DoModal(m_hWnd) != IDOK)
    return;

  // set window text of edit control only
  m_userbkgnd_edit.SetWindowText(fd.m_szFileName);
}

int OptionBasicPage::OnSetActive()
{
  // retrieve variables from preference
  m_bkgnd = AfxGetAppSettings().logoext?1:0;  // logoext is "use external logo"
  // feed variables onto screen
  m_userbkgnd_edit.SetWindowText(AfxGetAppSettings().logofn);
  DoDataExchange();
  return 0;
}

int OptionBasicPage::OnApply()
{
  // retrieve variables from screen
  DoDataExchange(TRUE);
  // feed variables into preference
  AfxGetAppSettings().logoext = m_bkgnd==1?true:false;
  m_userbkgnd_edit.GetWindowText(AfxGetAppSettings().logofn);
  return PSNRET_NOERROR;
}