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
    COMMAND_HANDLER_EX(IDC_EDIT_BKGNDFILE, EN_SETFOCUS, OnBkgndPickerSetFocused)
    COMMAND_HANDLER_EX(IDC_RADIO_NOBKGND, BN_CLICKED, OnBkgndUpdated)
    COMMAND_HANDLER_EX(IDC_RADIO_USERBKGND, BN_CLICKED, OnBkgndUpdated)
    COMMAND_HANDLER_EX(IDC_CHECK_AUTOUPGRADE, BN_CLICKED, OnAutoUpgradeChanged)
    COMMAND_HANDLER_EX(IDC_CHECK_HOTKEYSCHEME, BN_CLICKED, OnCustomHotkeyChanged)
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
    DDX_CHECK(IDC_CHECK_HOTKEYSCHEME, m_hotkeyscheme)
    DDX_CHECK(IDC_CHECK_MOUSEBTNCONTROLPLAY, m_leftclick2pause)
  END_DDX_MAP()

  // message handlers
  BOOL OnInitDialog(HWND hwnd, LPARAM lParam);
  void OnDestroy();
  void OnBkgndPicker(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnBkgndPickerSetFocused(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnBkgndUpdated(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnAutoUpgradeChanged(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnCustomHotkeyChanged(UINT uNotifyCode, int nID, CWindow wndCtl);

  // activate/apply handler
  int OnSetActive();
  int OnApply();

  // validate
  void Validate();

  // extra
  void PrepareHotkeySchemes();

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
  int m_leftclick2pause;

  // additional params
  int m_upgradestrategy;
  int m_hotkeyscheme;
  std::wstring  m_userbkgnd_location;
  std::vector<std::wstring> m_files;
  std::vector<std::wstring> m_schemenames;

  // controls
  CBtnEditCtrl   m_userbkgnd_edit;
  WTL::CComboBox m_upgradestrategy_combo;
  WTL::CComboBox m_hotkeyscheme_combo;
  WTL::CButton   m_autoscalecheckbox;
  WTL::CButton   m_aeroglasscheckbox;
};

#endif // OPTIONBASICPAGE_WIN_H