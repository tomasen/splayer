#include "stdafx.h"
#include "mplayerc.h"
#include "ChkDefPlayerControlBar.h"
#include "MainFrm.h"
#include "PPageFormats.h"
#include "Controller/SPlayerDefs.h"
#include "Controller/PlayerPreference.h"
#include "UserInterface/Dialogs/OptionDlg_Win.h"
#include "Utils/FileAssoc_Win.h"
#include <algorithm>

IMPLEMENT_DYNAMIC(ChkDefPlayerControlBar, CSVPDialog)

ChkDefPlayerControlBar::ChkDefPlayerControlBar(void)
{
  m_cbnomoreques.SetButtonMode(2);
  m_csllabel.SetStaticMode(1);
  std::wstring szExts = (LPCTSTR)AfxGetAppSettings().szStartUPCheckExts;
  svpTool.Explode( szExts, L" " , &szaExt);
  __super::SetLastTime(10000);
}

ChkDefPlayerControlBar::~ChkDefPlayerControlBar(void)
{
}

#define MSKB_KEY _T("Software\\Microsoft\\Keyboard\\Native Media Players")

BEGIN_MESSAGE_MAP(ChkDefPlayerControlBar, CSVPDialog)
  ON_WM_CREATE()
  ON_WM_SIZE()
  ON_WM_SHOWWINDOW()
  ON_BN_CLICKED(IDC_CHECKBOXNOMOREQUES, OnNoMoreQuesCheck)
  ON_BN_CLICKED(IDC_BUTTONDEFPLAYEROK, OnButtonOK)
  ON_BN_CLICKED(IDC_BUTTONDEFPLAYERMANUAL, OnButtonManual)
END_MESSAGE_MAP()

// ChkDefPlayerControlBar message handlers

int ChkDefPlayerControlBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CSVPDialog::OnCreate(lpCreateStruct) == -1)
    return -1;

  CRect r;
  GetClientRect(r);

  GetSystemFontWithScale(&m_font);

  m_cbok.Create(ResStr(IDS_DEFINE_PLAYER_OK), WS_VISIBLE|WS_CHILD|BS_FLAT|BS_VCENTER|BS_CENTER,
    r, this, IDC_BUTTONDEFPLAYEROK);
  m_cbok.EnableWindow();
  m_cbok.SetFont(&m_font);

  m_cbmanual.Create(ResStr(IDS_DEFINE_PLAYER_MANUAL), WS_VISIBLE|WS_CHILD|BS_FLAT|BS_VCENTER|BS_CENTER,
    r , this, IDC_BUTTONDEFPLAYERMANUAL);
  m_cbmanual.EnableWindow();
  m_cbmanual.SetFont(&m_font);

  m_csllabel.Create(ResStr(IDS_DEFINE_PLAYER_NOTIFY), WS_CHILD|WS_VISIBLE|SS_CENTERIMAGE,
    r, this, IDC_STATIC);
  m_csllabel.EnableWindow();
  m_csllabel.m_dwAlign = DT_LEFT;
  m_csllabel.SetFont(&m_font);

  m_cbnomoreques.Create(ResStr(IDS_CHK_NOMOREQUES), WS_CHILD|WS_VISIBLE,
    r, this, IDC_CHECKBOXNOMOREQUES);
  m_cbnomoreques.EnableWindow();
  m_cbnomoreques.SetFont(&m_font);

  return 0;
}

void ChkDefPlayerControlBar::OnSize(UINT nType, int cx, int cy)
{
  __super::OnSize(nType, cx, cy);
  Relayout();
}

void ChkDefPlayerControlBar::Relayout()
{
  CRect r;
  GetClientRect(r);

  {
    CRect r2(r);
    r2.left += 45;
    r2.right = r2.left + 55;
    r2.top += 95;
    r2.bottom = r2.top + 23;
    m_cbok.MoveWindow(&r2);
    m_cbok.EnableWindow(TRUE);
  }

  {
    CRect r2(r);
    r2.left += 140;
    r2.right = r2.left + 55;
    r2.top += 95;
    r2.bottom = r2.top + 23;
    m_cbmanual.MoveWindow(&r2);
    m_cbmanual.EnableWindow(TRUE);
  }

  {
    CRect r2(r);
    r2.top += 20;
    r2.left += 20;
    r2.right = r2.left + 200;
    r2.bottom = r2.top + 40;
    m_csllabel.MoveWindow(&r2);
  }

  {
    CRect r2(r);
    r2.top += 60;
    r2.left += 69;
    r2.right = r2.left + 120;
    r2.bottom = r2.top + 20;
    m_cbnomoreques.MoveWindow(&r2);
  }
  Invalidate();
}

void ChkDefPlayerControlBar::OnNoMoreQuesCheck()
{
  if (m_cbnomoreques.IsChecked())
    m_cbnomoreques.SetCheckStatus(false);
  else
    m_cbnomoreques.SetCheckStatus(true);
  Invalidate();
}

void ChkDefPlayerControlBar::SetKeyboardNativeMediaPlayers()
{
  //open Microsoft Keyboard Native Media key OR if it does not exist, create it
  HKEY hKey;
  DWORD dwDisposition;
  if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CURRENT_USER, MSKB_KEY,
      0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0,
      &hKey, &dwDisposition))
    return;

  TCHAR buff[MAX_PATH];
  if (::GetModuleFileName(AfxGetInstanceHandle(), buff, MAX_PATH) == 0)
    return;

  //create a key for new media player (using DisplayString as key name)
  CString szDisplayString = ResStr(IDR_MAINFRAME);
  CString szExePath = buff;
  HKEY hSubKey;
  if (ERROR_SUCCESS != RegCreateKeyEx(
      hKey, szDisplayString,0, 0,REG_OPTION_NON_VOLATILE,
     KEY_ALL_ACCESS, 0, &hSubKey, &dwDisposition))
    return;


  //Add AppName string value to new key and copy in the DisplayString
  if (ERROR_SUCCESS != RegSetValueEx(
      hSubKey,_T("AppName"), 0, REG_SZ, (LPBYTE)(LPCTSTR) szDisplayString,
      (DWORD) (lstrlen(szDisplayString)+1)*sizeof(TCHAR)))
    return;


  //Add ExePath string value to new key and copy in the ExePath
  if (ERROR_SUCCESS != RegSetValueEx(
      hSubKey,_T("ExePath"), 0, REG_SZ, (LPBYTE)(LPCTSTR) szExePath,
      (DWORD) (lstrlen(szExePath)+1)*sizeof(TCHAR)))
    return;


  //close reg keys
  if ( ERROR_SUCCESS != RegCloseKey(hKey))
    return;
  if ( ERROR_SUCCESS != RegCloseKey(hSubKey))
    return;
}

void ChkDefPlayerControlBar::SetKeyboardNativeMediaPlayers2()
{
  CRegKey key;
  TCHAR buff[MAX_PATH];

  //\SOFTWARE\Microsoft\Windows\CurrentVersion\explorer\AppKey\16\Association
  if (ERROR_SUCCESS != key.Open(HKEY_LOCAL_MACHINE, 
      _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\explorer\\AppKey\\16"),
      KEY_READ|KEY_WRITE))
    return;
  ULONG len = MAX_PATH;
  if (ERROR_SUCCESS != key.QueryStringValue(
     L"Association", buff, &len))
    return;

  bool bAlreadyDefault = false;
  bool bUseMkv = false;
  for(int i = 0; i < szaNotExt.size();i++)
  {
    if (szaNotExt[i].compare(buff) == 0)
      bAlreadyDefault = true;
    if ( szaNotExt[i].compare(L".mkv") == 0)
      bUseMkv = true;
  }

  if(!bAlreadyDefault && szaNotExt.size() > 0)
  {
    std::wstring szext = L".mkv";
    if (!bUseMkv)
    {
      szext = szaNotExt.at(0);
      std::transform(szext.begin(), szext.end(), szext.begin(), tolower);
    }
    key.SetStringValue(L"Association" ,szext.c_str());
  }
  key.Close();
}

void ChkDefPlayerControlBar::SetDefaultPlayer(int ilimitime)
{
  CPPageFormats cpf;
  if(cpf.m_bInsufficientPrivileges)
  {
    AfxGetMyApp()->GainAdminPrivileges(0, FALSE);
    ShowWindow(SW_HIDE);
  }
  else
  {
    //for(int i = 0; i < szaNotExt.GetCount();i++){
    //	CPPageFormats::RegisterExt(szaNotExt.GetAt(i), TRUE);
    //}
    SetKeyboardNativeMediaPlayers();
    cpf.AddAutoPlayToRegistry(cpf.AP_VIDEO, true);
    cpf.AddAutoPlayToRegistry(cpf.AP_DVDMOVIE, true);
    cpf.AddAutoPlayToRegistry(cpf.AP_AUDIOCD, true);
    cpf.AddAutoPlayToRegistry(cpf.AP_MUSIC, true);
    cpf.AddAutoPlayToRegistry(cpf.AP_SVCDMOVIE, true);
    cpf.AddAutoPlayToRegistry(cpf.AP_VCDMOVIE, true);
    cpf.AddAutoPlayToRegistry(cpf.AP_BDMOVIE, true);
    cpf.AddAutoPlayToRegistry(cpf.AP_DVDAUDIO, true);
    cpf.AddAutoPlayToRegistry(cpf.AP_CAPTURECAMERA, true);
    SetKeyboardNativeMediaPlayers2();

    CMediaFormats& mf = AfxGetAppSettings().Formats;

    for(size_t i = 0; i < mf.GetCount(); i++)
    {
      int fAudioOnly = mf[i].IsAudioOnly();

      if(fAudioOnly != 0) continue; //only reg video file

      int j = 0;
      CString str = mf[i].GetExtsWithPeriod();
      for(CString ext = str.Tokenize(_T(" "), j); !ext.IsEmpty(); ext = str.Tokenize(_T(" "), j))
        CPPageFormats::RegisterExt(ext, true, L"video");
    }
  }
/*
	AddAutoPlayToRegistry(AP_MUSIC, !!m_apmusic.GetCheck());
	AddAutoPlayToRegistry(AP_AUDIOCD, !!m_apaudiocd.GetCheck());
*/
}

void ChkDefPlayerControlBar::OnButtonOK()
{
  // TODO: Add your control notification handler code here
  UpdateData(TRUE);
  AppSettings& s = AfxGetAppSettings();
  PlayerPreference::GetInstance()->SetIntVar(INTVAR_CHECKFILEASSOCONSTARTUP ,168);
  s.fPopupStartUpExtCheck = !m_bnomorequestion;
  s.UpdateData(TRUE);
  SetDefaultPlayer();
  SHChangeNotify(0x08000000, 0, 0, 0); //SHCNE_ASSOCCHANGED SHCNF_IDLIST
  ShowWindow(SW_HIDE);
}

void ChkDefPlayerControlBar::OnButtonManual()
{
  // TODO: Add your control notification handler code here
  OptionDlg dlg(OPTIONDLG_ASSOCIATION);

  if(dlg.DoModal() == IDOK)
    ShowWindow(SW_HIDE);
}

void ChkDefPlayerControlBar::DoDataExchange(CDataExchange* pDX)
{
   __super::DoDataExchange(pDX);
  m_bnomorequestion = m_cbnomoreques.IsChecked();
}

bool ChkDefPlayerControlBar::IsDefaultPlayer()
{
  return (FileAssoc::IsExtRegistered(L".avi") &&
    FileAssoc::IsExtRegistered(L".mkv"))?1:0;
}

afx_msg void ChkDefPlayerControlBar::OnShowWindow(BOOL bShow, UINT nStatus)
{
  __super::OnShowWindow(bShow, nStatus);
  if (!bShow)
  {
    UpdateData(true);
    AppSettings& s = AfxGetAppSettings();
    s.fPopupStartUpExtCheck = !m_bnomorequestion;
    s.UpdateData(true);
  }
  else
  {
    AppSettings& s = AfxGetAppSettings();
    m_bnomorequestion = !s.fPopupStartUpExtCheck;
    m_cbnomoreques.SetCheckStatus(m_bnomorequestion);
    UpdateData(true);
  }
}