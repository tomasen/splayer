#ifndef THEMEPKGDLG_WIN_H
#define THEMEPKGDLG_WIN_H

#include "../../resource.h"

class OptionThemePage:
  public WTL::CPropertyPageImpl<OptionThemePage>
{
public:
  enum { IDD = IDD_OPTION_THEME };

  BEGIN_MSG_MAP(OptionThemePage)
    MSG_WM_INITDIALOG(OnInitDialog)
    MSG_WM_DESTROY(OnDestroy)
    MSG_WM_TIMER(OnTimer)
    COMMAND_HANDLER_EX(IDC_BUTTON_APPLYTHEME, BN_CLICKED, OnApplyTheme)
    CHAIN_MSG_MAP(WTL::CPropertyPageImpl<OptionThemePage>)
  END_MSG_MAP()

  // message handlers
  BOOL OnInitDialog(HWND hwnd, LPARAM lParam);
  void OnDestroy();
  void OnTimer(UINT_PTR nIDEvent);
  void OnApplyTheme(UINT uNotifyCode, int nID, CWindow wndCtl);

private:
};

#endif // THEMEPKGDLG_WIN_H