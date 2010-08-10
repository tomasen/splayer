#ifndef OPTIONSUBTITLEPAGE_WIN_H
#define OPTIONSUBTITLEPAGE_WIN_H

#include "../../resource.h"

class OptionSubtitlePage:
  public WTL::CPropertyPageImpl<OptionSubtitlePage>,
  public WTL::COwnerDraw<OptionSubtitlePage>
{
public:
  enum { IDD = IDD_OPTION_SUBTITLE};
  OptionSubtitlePage(void);

  BEGIN_MSG_MAP(OptionSubtitlePage)
    MSG_WM_INITDIALOG(OnInitDialog)
    MSG_WM_DESTROY(OnDestroy)
    COMMAND_HANDLER_EX(IDC_LIST, LBN_SELCHANGE, OnSubtitleStyleChange)
    CHAIN_MSG_MAP(COwnerDraw<OptionSubtitlePage>)
  END_MSG_MAP()

  // message handlers
  BOOL OnInitDialog(HWND hwnd, LPARAM lParam);
  void OnDestroy();

  void OnSubtitleStyleChange(UINT uNotifyCode, int nID, CWindow wndCtl);

  // owner-draw logic for subtitle styles
  void DrawItem(LPDRAWITEMSTRUCT lpdis);
  void MeasureItem(LPMEASUREITEMSTRUCT lpmis);

  // activate/apply handler
  int OnSetActive();
  int OnApply();

  void RefreshStyles();

private:
  WTL::CComboBox  m_secsubtitlestyle;
  WTL::CListBox   m_subtitlestyle;

  int m_mainstyle;
  int m_secstyle;

  int m_styleentry_height;
  int m_styleentry_width;
};

#endif // OPTIONSUBTITLEPAGE_WIN_H