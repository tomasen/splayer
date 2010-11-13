#ifndef OPTIONADVANCEDPAGE_WIN_H
#define OPTIONADVANCEDPAGE_WIN_H

#include "../../resource.h"

class OptionAdvancedPage:
  public WTL::CPropertyPageImpl<OptionAdvancedPage>,
  public WTL::CWinDataExchange<OptionAdvancedPage>
{
public:
  enum { IDD = IDD_OPTION_ADVANCED };

  BEGIN_MSG_MAP(OptionAdvancedPage)
    MSG_WM_INITDIALOG(OnInitDialog)
    MSG_WM_DESTROY(OnDestroy)
    COMMAND_HANDLER_EX(IDC_RADIO_PICTUREQUALITY, BN_CLICKED, OnVideoModeUpdated)
    COMMAND_HANDLER_EX(IDC_RADIO_PERFORMANCE, BN_CLICKED, OnVideoModeUpdated)
    COMMAND_HANDLER_EX(IDC_CHECK_CUSTOMSPEAKER, BN_CLICKED, OnCustomSpeakerChanged)
    CHAIN_MSG_MAP(WTL::CPropertyPageImpl<OptionAdvancedPage>)
  END_MSG_MAP()

  BEGIN_DDX_MAP(OptionAdvancedPage)
    DDX_RADIO(IDC_RADIO_PICTUREQUALITY, m_videoqualitymode)
    DDX_CHECK(IDC_CHECK_ENABLEGPUACCEL, m_enablegpuaccel)
    DDX_CHECK(IDC_CHECK_CUSTOMSPEAKER, m_usecustomspeakersetting)
    DDX_CHECK(IDC_CHECK_SPDIFPRIORITY, m_usespdifprority)
    DDX_CHECK(IDC_CHECK_NORMALIZE, m_usenormalize)
    DDX_CHECK(IDC_CHECK_AUDIOCENTERTOLRMAP, m_mapcenterch2lr)
  END_DDX_MAP()

  // message handlers
  BOOL OnInitDialog(HWND hwnd, LPARAM lParam);
  void OnDestroy();
  void OnVideoModeUpdated(UINT uNotifyCode, int nID, CWindow wndCtl);
  void OnCustomSpeakerChanged(UINT uNotifyCode, int nID, CWindow wndCtl);

  // activate/apply handler
  int OnSetActive();
  int OnApply();

private:
  // DDX mapping variables
  int m_videoqualitymode;
  int m_enablegpuaccel;
  int m_usecustomspeakersetting;
  int m_usespdifprority;
  int m_usenormalize;
  int m_mapcenterch2lr;

  // control
  WTL::CButton   m_gpuaccelcheckbox;
  WTL::CComboBox m_speakers_combo;
};

#endif // OPTIONADVANCEDPAGE_WIN_H