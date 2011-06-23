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
  m_mainstyle(0)
, m_secstyle(0)
{

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
  //m_subtitlestyle.Attach(GetDlgItem(IDC_LIST));
  m_subtitlestyle.SetParent(m_hWnd);
  m_subtitlestyle.SubclassWindow(GetDlgItem(IDC_LIST));
  m_subtitlestyle.GetWindowRect(&rc_stylelist);
  m_styleentry_width = rc_stylelist.right - rc_stylelist.left;
  m_subtitlestyle.InitializeList();
  //RefreshStyles();

  AppSettings& s = AfxGetAppSettings();
  // calculate current settings according to AppSettings class s.subdefstyle
  // retrieve subtitle style settings, and check which available style is closest to
  // the given settings, then set |m_mainstyle| to corresponding one.
  // s.subdefstyle compare font type, font color, border color shadow color
  for (int i = 0; i < m_subtitlestyle.GetCount(); i++)
  {
    StyleParam* mainsp = NULL;
    StyleParam* secondsp = NULL;
    m_subtitlestyle.GetItemData(i, &mainsp, &secondsp);
    if (mainsp->fontcolor == s.subdefstyle.colors[0] && mainsp->strokecolor == s.subdefstyle.colors[2] 
        && mainsp->shadowcolor == s.subdefstyle.colors[3] 
        /*&& (sp->_fontname == SubtitleStyle::DetectFontType((LPCTSTR)s.subdefstyle.fontName))*/ //×ÖÌå
       )
    {
      m_subtitlestyle.SetCurSel(i);
      break;
    }
    else
      m_subtitlestyle.SetCurSel(0);
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
  m_subtitlestyle.DrawItem(lpdis);
}

void OptionSubtitlePage::MeasureItem(LPMEASUREITEMSTRUCT lpmis)
{
  if (lpmis->CtlID == IDC_LIST)
    m_subtitlestyle.MeasureItem(lpmis);
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
  StyleParam* mainsp;
  StyleParam* secondsp;
  m_subtitlestyle.GetItemData(m_subtitlestyle.GetCurSel(), &mainsp, &secondsp);

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
  //m_subtitlestyle.SetCount(m_fontparams.GetStyleCount());
  m_subtitlestyle.Invalidate();
  m_subtitlestyle.UpdateWindow();
}


