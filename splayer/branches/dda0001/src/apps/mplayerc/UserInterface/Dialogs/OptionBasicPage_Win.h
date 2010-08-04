#ifndef OPTIONBASICPAGE_WIN_H
#define OPTIONBASICPAGE_WIN_H

#include "../../resource.h"
#include "../Support/BtnEditCtrl.h"

class OptionBasicPage:
  public WTL::CPropertyPageImpl<OptionBasicPage>,
  public WTL::CWinDataExchange<OptionBasicPage>
{
public:
  enum { IDD = IDD_OPTION_BASIC };
  enum { ID_BKGND_PICKER = 1001 /* picker button in user background */ };

  BEGIN_MSG_MAP(OptionBasicPage)
    MSG_WM_INITDIALOG(OnInitDialog)
    MSG_WM_DESTROY(OnDestroy)
    COMMAND_HANDLER_EX(ID_BKGND_PICKER, BN_CLICKED, OnBkgndPicker)
    COMMAND_HANDLER_EX(IDC_RADIO_NOBKGND, BN_CLICKED, OnBkgndUpdated)
    COMMAND_HANDLER_EX(IDC_RADIO_USERBKGND, BN_CLICKED, OnBkgndUpdated)
    CHAIN_MSG_MAP(WTL::CPropertyPageImpl<OptionBasicPage>)
  END_MSG_MAP()

  BEGIN_DDX_MAP(OptionBasicPage)
    DDX_RADIO(IDC_RADIO_NOBKGND, m_bkgnd)
    DDX_CHECK(IDC_CHECK_BKGNDAUTOSCALE, m_autoscalebkgnd)
    DDX_CHECK(IDC_CHECK_AERO, m_useaero)
    DDX_CHECK(IDC_CHECK_REPEAT, m_repeat)
    DDX_CHECK(IDC_CHECK_MINIMIZETOTRAY, m_mintotray)
    DDX_CHECK(IDC_CHECK_AUTORESUME, m_autoresume)
    DDX_CHECK(IDC_CHECK_AUTOFULLSCREEN, m_autofullscreen)
    DDX_CHECK(IDC_CHECK_AUTOUPGRADE, m_autoupgrade)
  END_DDX_MAP()

  // message handlers
  BOOL OnInitDialog(HWND hwnd, LPARAM lParam);
  void OnDestroy();
  void OnBkgndPicker(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnBkgndUpdated(UINT uNotifyCode, int nID, CWindow wndCtl);

  // activate/apply handler
  int OnSetActive();
  int OnApply();

private:
  // DDX mapping variables
  int m_bkgnd;
  int m_autoscalebkgnd;

  int m_useaero;

  int m_repeat;
  int m_mintotray;
  int m_autoresume;
  int m_autofullscreen;
  int m_autoupgrade;

  // additional params
  int m_upgradestrategy;
  std::wstring  m_userbkgnd_location;

  // controls
  CBtnEditCtrl   m_userbkgnd_edit;
  WTL::CComboBox m_upgradestrategy_combo;
  WTL::CButton   m_autoscalecheckbox;
  WTL::CButton   m_aeroglasscheckbox;
};

#endif // OPTIONBASICPAGE_WIN_H