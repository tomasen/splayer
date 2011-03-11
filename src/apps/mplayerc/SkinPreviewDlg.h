#pragma once
#include "afxwin.h"
#include "resource.h"
#include "SkinDownload.h"

class SkinPreviewDlg : public CDialogImpl<SkinPreviewDlg>,
  public WTL::CWinDataExchange<SkinPreviewDlg>
{
public:
  SkinPreviewDlg(void);
  ~SkinPreviewDlg(void);

  enum { IDD = IDD_SKIN_PREVIEW };

  BEGIN_MSG_MAP_EX(SkinPreviewDlg)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_CLOSE, OnClose)
    MESSAGE_HANDLER(WM_USER+34, OnUpdataSkinSelections)
    //MESSAGE_HANDLER(WM_NOTIFY, OnClickSyslink1)
    COMMAND_ID_HANDLER(IDD_SKINPREVIEW_SELECT, OnSkinSelect)
    COMMAND_ID_HANDLER(IDD_SKINPREVIEW_DELETE, OnSkinDelete)
    COMMAND_ID_HANDLER(IDD_SKINPREVIEW_MOREBUTTON, OnSkinMore)
    COMMAND_HANDLER(IDD_SKINPREVIEW_LIST, LBN_SELCHANGE, OnLbnSelchangeList)
    NOTIFY_HANDLER(IDD_SKINPREVIEW_HOMEPAGE, NM_CLICK, OnClickSyslink)
    NOTIFY_HANDLER(IDD_SKINPREVIEW_MAIL, NM_CLICK, OnClickSyslink)
  END_MSG_MAP()


  LRESULT OnSkinSelect(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, 
                       BOOL& /*bHandled*/);

  LRESULT OnSkinDelete(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, 
                       BOOL& /*bHandled*/);

  LRESULT OnClickSyslink(int wParam, LPNMHDR lParam, BOOL& bHandled);

  LRESULT OnSkinMore(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, 
                       BOOL& /*bHandled*/);

  LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, 
                       LPARAM /*lParam*/, BOOL& /*bHandled*/);

  LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, 
                       LPARAM /*lParam*/, BOOL& /*bHandled*/);

  LRESULT OnLbnSelchangeList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, 
                             BOOL& /*bHandled*/);

  void SetSkinOption(std::map<std::wstring, std::wstring>* skinmap);

  UINT FindMenuID(CString itemname);

  void SetSkinIDtoNameMap(std::map<UINT, std::wstring>& map);

  LRESULT OnUpdataSkinSelections(UINT /*uMsg*/, WPARAM /*wParam*/, 
                                 LPARAM /*lParam*/, BOOL& /*bHandled*/);

  LRESULT OnDownloadDlgClose(UINT /*uMsg*/, WPARAM /*wParam*/, 
                                 LPARAM /*lParam*/, BOOL& /*bHandled*/);
private:
  std::map<std::wstring, std::wstring>* m_skinoption;
  std::map<UINT, std::wstring> m_skinIDtoName_map;
  WTL::CListBox m_skinlist;
  WTL::CStatic  m_skinpicture;

  SkinDownload  skindown;
  DLGTEMPLATE*  _dialogTemplate;
  CMenu*        m_skinmenu;

  CLinkCtrl     m_phomepage;
  CLinkCtrl     m_pmail;

  std::wstring  m_newskinname;

};
