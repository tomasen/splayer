#include "stdafx.h"
#include "OptionSubtitlePage_Win.h"
#include <Strings.h>
#include "../Support/SubtitleStyle.h"
#include "../../mplayerc.h"
#include "../../MainFrm.h"
#include "../../Controller/SPlayerDefs.h"
#include "../../Controller/PlayerPreference.h"
#include <sstream>

OptionSubtitlePage::OptionSubtitlePage()
: m_mainstyle(0)
, m_secstyle(0)
, m_dOldMainSize(-1)
, m_dOldSecondSize(-1)
, m_crOldMainColor(0)
, m_crOldSecondColor(0)
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

  m_subtitlestyle.SubclassWindow(GetDlgItem(IDC_LIST));
  m_subtitlestyle.GetWindowRect(&rc_stylelist);
  m_styleentry_width = rc_stylelist.right - rc_stylelist.left;
  RefreshStyles();

  AppSettings& s = AfxGetAppSettings();

  // main style
  for (int i = 0; i < SubtitleStyle::GetStyleCount(); i++)
  {
    SubtitleStyle::STYLEPARAM* sp = NULL;
    if (SubtitleStyle::GetStyleParams(i, -1, &sp) && s.subdefstyle.fontName == sp->fontname)
    {
      m_subtitlestyle.SetCurSel(i);
      m_mainstyle = i;
      break;
    }
  }

  if (m_subtitlestyle.GetCount() > 0 && m_subtitlestyle.GetCurSel() < 0)
  {
    m_subtitlestyle.SetCurSel(0);
    m_mainstyle = 0;
  }

  // second style
  for (int i = 0; i < SubtitleStyle::GetStyleCount(); i++)
  {
    SubtitleStyle::STYLEPARAM* sp = NULL;
    if (SubtitleStyle::GetStyleParams(-1, i, &sp) && s.subdefstyle2.fontName == sp->fontname)
    {
      m_secstyle = i;
      break;
    }
  }

  if (m_secstyle == -1)
    m_secstyle = 0;

  m_secsubtitlestyle.SetCurSel((s.subdefstyle2.scrAlignment == 2)?1:0);

  m_fetchsubtitlefromshooter = s.autoDownloadSVPSub;

  // init which subtitle should be set combo box
  std::wstring sSetSubtitle = (LPCTSTR)ResStr(IDS_SET_SUBTITLE);
  std::wstring sSetSubtitlePart1(sSetSubtitle.begin(), sSetSubtitle.begin() + sSetSubtitle.find(L'|'));
  std::wstring sSetSubtitlePart2(sSetSubtitle.begin() + sSetSubtitle.find(L'|') + 1, sSetSubtitle.end());
  m_cmbSetSubtitle.Attach(GetDlgItem(IDC_CMB_SET_SUBTITLE));
  m_cmbSetSubtitle.AddString(sSetSubtitlePart1.c_str());
  m_cmbSetSubtitle.AddString(sSetSubtitlePart2.c_str());
  m_cmbSetSubtitle.SetCurSel(0);

  // init the subtitle's old size and color
  m_dOldMainSize = s.subdefstyle.fontSize;
  m_dOldSecondSize = s.subdefstyle2.fontSize;
  m_crOldMainColor = s.subdefstyle.colors[0];
  m_crOldSecondColor = s.subdefstyle2.colors[0];

  // init the subtitle's size and color controls
  m_edtFontSize.Attach(GetDlgItem(IDC_EDIT_SUBTITLE_FONTSIZE));
  m_btnFontColor.Attach(GetDlgItem(IDC_BUTTON_SUBTITLE_FONTCOLOR));
  m_spnFontSize.Attach(GetDlgItem(IDC_SPIN_SUBTITLE_FONTSIZE));

  m_spnFontSize.SetRange(10, 25);
  m_spnFontSize.SetPos(s.subdefstyle.fontSize);

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
  m_edtFontSize.Detach();
  m_spnFontSize.Detach();
  m_btnFontColor.Detach();
}

void OptionSubtitlePage::OnSubtitleStyleChange(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  switch (m_cmbSetSubtitle.GetCurSel())
  {
  case 0:
    {
      // set the main subtitle style
      m_mainstyle = m_subtitlestyle.GetCurSel();
      break;
    }

  case 1:
    {
      // set the second subtitle style
      m_secstyle = m_subtitlestyle.GetCurSel();
      break;
    }
  }
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

// change subtitle's size
void OptionSubtitlePage::OnSubtitleSizeChange(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  AppSettings &s = AfxGetAppSettings();

  switch (m_cmbSetSubtitle.GetCurSel())
  {
  case 0:
    {
      // main subtitle
      // m_dOldMainSize must init before do the following job
      if (m_dOldMainSize == -1)
        return;

      wchar_t szSize[32] = {0};
      m_edtFontSize.GetWindowText(szSize, 31);
      s.subdefstyle.fontSize = ::_wtol(szSize);

      m_subtitlestyle.Invalidate();
      break;
    }

  case 1:
    {
      // second subtitle
      // m_dOldSecondSize must init before do the following job
      if (m_dOldSecondSize == -1)
        return;

      wchar_t szSize[32] = {0};
      m_edtFontSize.GetWindowText(szSize, 31);
      s.subdefstyle2.fontSize = ::_wtol(szSize);

      m_subtitlestyle.Invalidate();
      break;
    }
  }
}

// change subtitle's color
void OptionSubtitlePage::OnSubtitleColorChange(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  AppSettings &s = AfxGetAppSettings();

  switch (m_cmbSetSubtitle.GetCurSel())
  {
  case 0:
    {
      // main subtitle
      WTL::CColorDialog dlgColor(s.subdefstyle.colors[0]);
      if (dlgColor.DoModal() == IDOK)
      {
        s.subdefstyle.colors[0] = s.subdefstyle.colors[1] = dlgColor.GetColor();

        m_subtitlestyle.Invalidate();
        m_btnFontColor.Invalidate();
      }
      break;
    }

  case 1:
    {
      // second subtitle
      WTL::CColorDialog dlgColor(s.subdefstyle2.colors[0]);
      if (dlgColor.DoModal() == IDOK)
      {
        s.subdefstyle2.colors[0] = s.subdefstyle2.colors[1] = dlgColor.GetColor();

        m_subtitlestyle.Invalidate();
        m_btnFontColor.Invalidate();
      }
      break;
    }
  }
}

void OptionSubtitlePage::OnComboSetSubtitleChange(UINT uNotifyCode, int nID, CWindow wndCtl)
{
  AppSettings &s = AfxGetAppSettings();

  switch (m_cmbSetSubtitle.GetCurSel())
  {
  case 0:
    {
      // main subtitle
      m_spnFontSize.SetPos(s.subdefstyle.fontSize);
      m_btnFontColor.Invalidate();
      m_subtitlestyle.SetCurSel(m_mainstyle);
      m_subtitlestyle.Invalidate();
      break;
    }

  case 1:
    {
      // second subtitle
      m_spnFontSize.SetPos(s.subdefstyle2.fontSize);
      m_btnFontColor.Invalidate();
      m_subtitlestyle.SetCurSel(m_secstyle);
      m_subtitlestyle.Invalidate();
      break;
    }
  }
}

void OptionSubtitlePage::DrawItem(LPDRAWITEMSTRUCT lpdis)
{
  AppSettings &s = AfxGetAppSettings();
  switch (lpdis->CtlID)
  {
  case IDC_LIST:
    {
      if (m_cmbSetSubtitle.GetCurSel() == 0)
      {
        // main subtitle
        SubtitleStyle::Paint(lpdis->hDC, &lpdis->rcItem, lpdis->itemID, -1,
          (lpdis->itemState & ODS_SELECTED)?true:false);
      } 
      else
      {
        // second subtitle
        SubtitleStyle::Paint(lpdis->hDC, &lpdis->rcItem, -1, lpdis->itemID,
          (lpdis->itemState & ODS_SELECTED)?true:false);
      }
      break;
    }

  case IDC_BUTTON_SUBTITLE_FONTCOLOR:
    {
      WTL::CRect rcColorButton;
      m_btnFontColor.GetClientRect(&rcColorButton);
      WTL::CDC dc(lpdis->hDC);
      if (m_cmbSetSubtitle.GetCurSel() == 0)
      {
        // main subtitle
        dc.FillSolidRect(rcColorButton, s.subdefstyle.colors[0]);
      } 
      else
      {
        // second subtitle
        dc.FillSolidRect(rcColorButton, s.subdefstyle2.colors[0]);
      }

      break;
    }
  }
}

void OptionSubtitlePage::MeasureItem(LPMEASUREITEMSTRUCT lpmis)
{
  if (lpmis->CtlID == IDC_LIST)
    lpmis->itemHeight = m_styleentry_height;
}

void OptionSubtitlePage::ApplySubtitleStyle()
{
  DoDataExchange(TRUE);
  AppSettings& s = AfxGetAppSettings();

  SubtitleStyle::STYLEPARAM* sp_main = 0;
  SubtitleStyle::STYLEPARAM* sp_sec = 0;
  SubtitleStyle::GetStyleParams(m_mainstyle, -1, &sp_main);
  SubtitleStyle::GetStyleParams(-1, m_secstyle, &sp_sec);

  if (sp_main)
  {
    s.subdefstyle.colors[0] = s.subdefstyle.colors[1] = sp_main->fontcolor;
    s.subdefstyle.colors[2] = sp_main->strokecolor;
    s.subdefstyle.colors[3] = sp_main->shadowcolor;
    s.subdefstyle.fontName = sp_main->fontname;
    s.subdefstyle.fontSize = sp_main->fontsize;
    s.subdefstyle.fontWeight = (sp_main->fontname == L"WenQuanYi Micro Hei" 
                                || sp_main->fontname == L"\x6587\x6CC9\x9A7F\x5FAE\x737C\x9ED1"  // Chinese for "WenQuanYi Micro Hei"
                                )?FW_BOLD:FW_NORMAL; // Using BOLD if it is font WenQuanYi
    s.subdefstyle.scrAlignment = 2;
    s.subdefstyle.shadowDepthX = s.subdefstyle.shadowDepthY = sp_main->shadowoffset;
    s.subdefstyle.outlineWidthX = s.subdefstyle.outlineWidthY = sp_main->strokesize;
  }

  if (sp_sec)
  {
    s.subdefstyle2.colors[0] = s.subdefstyle2.colors[1] = sp_sec->fontcolor;
    s.subdefstyle2.colors[2] = sp_sec->strokecolor;
    s.subdefstyle2.colors[3] = sp_sec->shadowcolor;
    s.subdefstyle2.fontName = sp_sec->fontname;
    s.subdefstyle2.fontSize = sp_sec->fontsize;
    s.subdefstyle2.fontWeight = (sp_sec->fontname == L"WenQuanYi Micro Hei" 
                                || sp_sec->fontname == L"\x6587\x6CC9\x9A7F\x5FAE\x737C\x9ED1"  // Chinese for "WenQuanYi Micro Hei"
                                )?FW_BOLD:FW_NORMAL; // Using BOLD if it is font WenQuanYi
    s.subdefstyle2.scrAlignment = 2;
    s.subdefstyle2.shadowDepthX = s.subdefstyle2.shadowDepthY = sp_sec->shadowoffset;
    s.subdefstyle2.outlineWidthX = s.subdefstyle2.outlineWidthY = sp_sec->strokesize;
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
  m_subtitlestyle.SetCount(SubtitleStyle::GetStyleCount(false));
  m_subtitlestyle.Invalidate();
}

BOOL OptionSubtitlePage::OnQueryCancel()
{
  // restore the font size and color
  AppSettings& s = AfxGetAppSettings();
  s.subdefstyle.fontSize = m_dOldMainSize;
  s.subdefstyle.colors[0] = s.subdefstyle2.colors[1] = m_crOldMainColor;
  s.subdefstyle2.fontSize = m_dOldSecondSize;
  s.subdefstyle2.colors[0] = s.subdefstyle2.colors[1] = m_crOldSecondColor;

  return FALSE;  // FALSE = allow cancel, TRUE = prevent cancel
}