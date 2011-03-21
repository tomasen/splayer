#include "stdafx.h"
#include "mplayerc.h"
#include "SkinPreviewDlg.h"
#include "SkinFolderManager.h"
#include "SUIButton.h"
#include "MainFrm.h"

SkinPreviewDlg::SkinPreviewDlg(void):_dialogTemplate(NULL), m_newskinname(L"")
{
}

SkinPreviewDlg::~SkinPreviewDlg(void)
{
  m_skinlist.Detach();
  m_pmail.Detach();
  m_phomepage.Detach();
  m_skinpicture.Detach();
  skindown.DestroyWindow();
  if (_dialogTemplate)
    free(_dialogTemplate);
}

LRESULT SkinPreviewDlg::OnSkinSelect(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, 
                                     BOOL& /*bHandled*/)
{
  
  int index = m_skinlist.GetCurSel();
  if (index == LB_ERR)
    return 0;
  CString itemstr;
  m_skinlist.GetText(index, itemstr.GetBuffer(MAX_PATH));
  itemstr.ReleaseBuffer();
  UINT id = FindMenuID(itemstr);
  ::PostMessage(GetParent(), WM_COMMAND, id, NULL);
  return 0;
}

LRESULT SkinPreviewDlg::OnSkinDelete(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, 
                                     BOOL& /*bHandled*/)
{
  int index = m_skinlist.GetCurSel();
  CString str;
  m_skinlist.GetText(index, str.GetBuffer(MAX_PATH));
  str.ReleaseBuffer();
  if (str.Find(L"(new)") != -1)
    str = str.Left(str.Find('('));
  m_skinlist.DeleteString(index);

  for (std::map<std::wstring, std::wstring>::iterator optionite = m_skinoption->begin();
       optionite != m_skinoption->end(); ++optionite)
    if (optionite->first.c_str() == str)
    {
      m_skinoption->erase(optionite);
      break;
    }
  
  CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
  for (std::map<UINT, std::wstring>::iterator skinite = pFrame->m_skin_map.begin();
       skinite != pFrame->m_skin_map.end(); ++skinite)
    if (skinite->second.c_str() == str)
    {
      pFrame->m_skin_map.erase(skinite);
      break;
    }

  SkinFolderManager skinmn;
  skinmn.DeleteFolder(str);

  AppSettings& s = AfxGetAppSettings();
  if (s.skinname.c_str() == str)
    ::PostMessage(GetParent(), WM_COMMAND, ID_SKIN_FIRST, 0);
  
  ChangeCLinkCtrlStr();

  ChangeCStaticPicture();

  return 0;
}

LRESULT SkinPreviewDlg::OnSkinMore(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, 
                                   BOOL& /*bHandled*/)
{

  if (_dialogTemplate == NULL)
  {
    _dialogTemplate = (DLGTEMPLATE*)calloc(1, sizeof(DLGTEMPLATE)+sizeof(DLGITEMTEMPLATE)+10);

    if (_dialogTemplate)
    {
      _dialogTemplate->style = DS_SETFONT | DS_FIXEDSYS | WS_POPUP | WS_DISABLED | DS_MODALFRAME
        | WS_CAPTION | WS_SYSMENU;
      _dialogTemplate->dwExtendedStyle = WS_EX_NOACTIVATE; //WS_EX_TOPMOST

      _dialogTemplate->cdit  = 0;
      _dialogTemplate->cx    = 237;
      _dialogTemplate->cy    = 178;

      if (0 == skindown.CreateIndirect(_dialogTemplate, CWnd::FromHandle(m_hWnd)))
      {
        free(_dialogTemplate);
        _dialogTemplate = NULL;
      }
      skindown.Navigate(L"http://webpj:8080/misc/skin.php");
    }
  }
  skindown.ShowFrame();

  return 0;
}

LRESULT SkinPreviewDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, 
                                 LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  CenterWindow();

  AppSettings& s = AfxGetAppSettings();

  HWND listhwnd;
  listhwnd = GetDlgItem(IDD_SKINPREVIEW_LIST);
  m_skinlist.Attach(listhwnd);

  for (std::map<std::wstring, std::wstring>::iterator ite = m_skinoption->begin();
       ite != m_skinoption->end(); ++ite)
    m_skinlist.AddString(ite->first.c_str());

  HWND picturehwnd = GetDlgItem(IDD_SKINPREVIEW_PICTURE);
  if (picturehwnd)
    m_skinpicture.Attach(picturehwnd);
  m_skinpicture.ModifyStyle(0xF,SS_BITMAP|SS_CENTERIMAGE);

  HWND makerhwnd = GetDlgItem(IDD_SKINPREVIEW_MAKERSTR);
  m_pmaker.Attach(makerhwnd);
  
  HWND homepage = GetDlgItem(IDD_SKINPREVIEW_HOMEPAGESTR);
  m_phomepage.Attach(homepage);
  m_phomepage.SetWindowText(_T("<A HREF=\"http://www.splayer.org/\">http://www.splayer.org</A>"));
  
  HWND mail = GetDlgItem(IDD_SKINPREVIEW_MAILSTR);
  m_pmail.Attach(mail);
  m_pmail.SetWindowText(_T("<A HREF=\"http://shooter.cn/\">http://shooter.cn</A>"));

  ChangeCLinkCtrlStr();

  int index = 0;
  for (int i = 0; i != m_skinlist.GetCount(); ++i)
  {
    TCHAR liststr[MAX_PATH];
    m_skinlist.GetText(i, liststr);
    if (s.skinname == liststr)
    {
      index = i;
      break;
    }
  }
  m_skinlist.SetCurSel(index);
  PostMessage(WM_COMMAND, MAKEWPARAM(IDD_SKINPREVIEW_LIST, LBN_SELCHANGE), 0);

  return 0;
}

LRESULT SkinPreviewDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, 
                                LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  //EndDialog();
  DestroyWindow();
  return 0;
}


void SkinPreviewDlg::SetSkinOption(std::map<std::wstring, std::wstring>* skinmap)
{
  m_skinoption = skinmap;
}

LRESULT SkinPreviewDlg::OnLbnSelchangeList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, 
                           BOOL& /*bHandled*/)
{
  //m_skinlist.SetFocus();
  int index = m_skinlist.GetCurSel();
  CString s;
  m_skinlist.GetText(index, s.GetBuffer(MAX_PATH));
  s.ReleaseBuffer();
  if (s.Find(L"(new)") != -1)
    s = s.Left(s.Find('('));

  std::wstring picturpath;
  for (std::map<std::wstring, std::wstring>::iterator ite = m_skinoption->begin();
       ite != m_skinoption->end(); ++ite)
    if (ite->first.c_str() == s)
      picturpath = ite->second;

  HBITMAP hbitmap = (HBITMAP)LoadImage(0, picturpath.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
 
  if (hbitmap)
    m_skinpicture.SetBitmap(hbitmap);

  return 0;
}

UINT SkinPreviewDlg::FindMenuID(CString itemname)
{
  if (itemname.Find(L"(new)") != -1)
    itemname = itemname.Left(itemname.Find('('));
  CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
  m_skinIDtoName_map = pFrame->m_skin_map;
  for (std::map<UINT, std::wstring>::iterator ite = m_skinIDtoName_map.begin();
       ite != m_skinIDtoName_map.end(); ++ite)
    if (ite->second.c_str() == itemname)
      return ite->first;
  
  return 0;
}

void SkinPreviewDlg::SetSkinIDtoNameMap(std::map<UINT, std::wstring>& map)
{
  m_skinIDtoName_map = map;
}

LRESULT SkinPreviewDlg::OnClickSyslink(int /*wParam*/, LPNMHDR lParam, BOOL& /*bHandled*/)
{
  PNMLINK pnmLink = (PNMLINK)lParam;
  
  ::ShellExecute(NULL, _T("open"), pnmLink->item.szUrl, NULL, NULL, SW_SHOWNORMAL);
  return 0;
}

LRESULT SkinPreviewDlg::OnUpdataSkinSelections(UINT /*uMsg*/, WPARAM wParam, 
                                    LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  m_skinoption = &SkinFolderManager::ReturnSkinMap();
  
  std::wstring newskin = L"";
  CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
  for (std::map<std::wstring, std::wstring>::iterator optionite = m_skinoption->begin();
    optionite != m_skinoption->end(); ++optionite)
  {
    BOOL bexist = FALSE;
    for (std::map<UINT, std::wstring>::iterator skinite = pFrame->m_skin_map.begin();
         skinite != pFrame->m_skin_map.end(); ++skinite)
      if (optionite->first == skinite->second)
      {
        bexist = TRUE;
        break;
      }

    if (!bexist)
      newskin = optionite->first;
   }

  m_skinlist.ResetContent();

  int id = ID_SKIN_FIRST;
  for (std::map<std::wstring, std::wstring>::iterator ite = m_skinoption->begin();
       ite != m_skinoption->end(); ++ite)
  {
    if (ite->first != newskin)
    {
      pFrame->m_skin_map[++id] = ite->first;
      m_skinlist.AddString(ite->first.c_str());
    }
  }
  if (newskin != L"")
  {
    pFrame->m_skin_map[++id] = newskin;
    newskin += L"(new)";
    m_skinlist.AddString(newskin.c_str());
  }

  ChangeCLinkCtrlStr();
  ChangeCStaticPicture();

  return 0;
}

void SkinPreviewDlg::ChangeCLinkCtrlStr()
{
  if (m_skinlist.GetCount() == 0)
  {
    m_pmaker.ShowWindow(SW_HIDE);
    m_phomepage.ModifyStyle(0, WS_DISABLED);
    m_pmail.ModifyStyle(0, WS_DISABLED);
    m_phomepage.ShowWindow(SW_HIDE);
    m_pmail.ShowWindow(SW_HIDE);

    GetDlgItem(IDD_SKINPREVIEW_MAKER).ShowWindow(SW_HIDE);
    GetDlgItem(IDD_SKINPREVIEW_HOMEPAGE).ShowWindow(SW_HIDE);
    GetDlgItem(IDD_SKINPREVIEW_MAIL).ShowWindow(SW_HIDE);
  }
  else
  {
    m_pmaker.ShowWindow(SW_SHOWNORMAL);
    m_phomepage.ModifyStyle(WS_DISABLED, 0);
    m_pmail.ModifyStyle(WS_DISABLED, 0);
    m_phomepage.ShowWindow(SW_SHOWNORMAL);
    m_pmail.ShowWindow(SW_SHOWNORMAL);

    GetDlgItem(IDD_SKINPREVIEW_MAKER).ShowWindow(SW_SHOWNORMAL);
    GetDlgItem(IDD_SKINPREVIEW_HOMEPAGE).ShowWindow(SW_SHOWNORMAL);
    GetDlgItem(IDD_SKINPREVIEW_MAIL).ShowWindow(SW_SHOWNORMAL);
  }
}

void SkinPreviewDlg::ChangeCStaticPicture()
{
  if (m_skinlist.GetCount() == 0)
    m_skinpicture.SetBitmap(0);
  else
  {
    m_skinlist.SetCurSel(m_skinlist.GetCount() - 1);
    PostMessage(WM_COMMAND, MAKEWPARAM(IDD_SKINPREVIEW_LIST, LBN_SELCHANGE), 0);
  }
}