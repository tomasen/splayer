#include "stdafx.h"
#include "OptionAssociationPage_Win.h"
#include "../../Utils/FileAssoc_Win.h"

BOOL OptionAssociationPage::OnInitDialog(HWND hwnd, LPARAM lParam)
{
  m_assoc_video = (FileAssoc::IsExtRegistered(L".avi") &&
    FileAssoc::IsExtRegistered(L".mkv"))?1:0;
  m_assoc_audio = (FileAssoc::IsExtRegistered(L".mp3") &&
    FileAssoc::IsExtRegistered(L".wma"))?1:0;
  m_ap_video = FileAssoc::IsAutoPlayRegistered(FileAssoc::AP_VIDEO)?1:0;
  m_ap_audio = FileAssoc::IsAutoPlayRegistered(FileAssoc::AP_MUSIC)?1:0;
  m_ap_dvd = FileAssoc::IsAutoPlayRegistered(FileAssoc::AP_DVDMOVIE)?1:0;
  m_ap_cd = FileAssoc::IsAutoPlayRegistered(FileAssoc::AP_AUDIOCD)?1:0;

  m_oassoc_video = m_assoc_video;
  m_oassoc_audio = m_assoc_audio;
  m_oap_video = m_ap_video;
  m_oap_audio = m_ap_audio;
  m_oap_dvd = m_ap_dvd;
  m_oap_cd = m_ap_cd;

  DoDataExchange();
  return TRUE;
}

void OptionAssociationPage::OnDestroy()
{

}

int OptionAssociationPage::OnSetActive()
{
  return 0;
}

int OptionAssociationPage::OnApply()
{
  DoDataExchange(TRUE);


  return PSNRET_NOERROR;
}