#ifndef OPTIONASSOCIATIONPAGE_WIN_H
#define OPTIONASSOCIATIONPAGE_WIN_H

#include "../../resource.h"

class OptionAssociationPage:
  public WTL::CPropertyPageImpl<OptionAssociationPage>,
  public WTL::CWinDataExchange<OptionAssociationPage>
{
public:
  enum { IDD = IDD_OPTION_ASSOCIATION };

  BEGIN_MSG_MAP(OptionAssociationPage)
    MSG_WM_INITDIALOG(OnInitDialog)
    MSG_WM_DESTROY(OnDestroy)
    CHAIN_MSG_MAP(WTL::CPropertyPageImpl<OptionAssociationPage>)
  END_MSG_MAP()

  BEGIN_DDX_MAP(OptionBasicPage)
    DDX_CHECK(IDC_CHECK_ASSOCIATEVIDEO, m_assoc_video)
    DDX_CHECK(IDC_CHECK_ASSOCIATEAUDIO, m_assoc_audio)
    DDX_CHECK(IDC_CHECK_AUTOPLAYVIDEO, m_ap_video)
    DDX_CHECK(IDC_CHECK_AUTOPLAYAUDIO, m_ap_audio)
    DDX_CHECK(IDC_CHECK_AUTOPLAYDVD, m_ap_dvd)
    DDX_CHECK(IDC_CHECK_AUTOPLAYCD, m_ap_cd)
  END_DDX_MAP()

  // message handlers
  BOOL OnInitDialog(HWND hwnd, LPARAM lParam);
  void OnDestroy();

  // activate/apply handler
  int OnSetActive();
  int OnApply();

private:
  // we use another set of variables to record old settings
  // and detect changes
  int m_assoc_video;
  int m_assoc_audio;
  int m_ap_video;
  int m_ap_audio;
  int m_ap_dvd;
  int m_ap_cd;

  int m_oassoc_video;
  int m_oassoc_audio;
  int m_oap_video;
  int m_oap_audio;
  int m_oap_dvd;
  int m_oap_cd;
};

#endif // OPTIONASSOCIATIONPAGE_WIN_H