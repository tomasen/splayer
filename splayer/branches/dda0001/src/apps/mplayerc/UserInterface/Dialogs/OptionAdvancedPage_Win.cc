#include "stdafx.h"
#include "OptionAdvancedPage_Win.h"
#include "../../mplayerc.h"

BOOL OptionAdvancedPage::OnInitDialog(HWND hwnd, LPARAM lParam)
{


  return TRUE;
}

void OptionAdvancedPage::OnDestroy()
{

}

int OptionAdvancedPage::OnSetActive()
{
  AppSettings& s = AfxGetAppSettings();

  m_videoqualitymode = s.iSVPRenderType;
  m_videoperformancemode = !s.iSVPRenderType;
  m_enablegpuaccel = s.useGPUAcel;

  m_usecustomspeakersetting = s.fCustomSpeakers;
  m_usenormalize = s.fAudioNormalize;

  DoDataExchange();
  return 0;
}

int OptionAdvancedPage::OnApply()
{
  // retrieve variables from screen
  DoDataExchange(TRUE);
  AppSettings& s = AfxGetAppSettings();
  // feed variables into preference
  if (m_videoqualitymode)
  {
    s.iSVPRenderType = 1;
    s.iDSVideoRendererType = 6;
    s.iRMVideoRendererType = 2;
    s.iQTVideoRendererType = 2;
    s.iAPSurfaceUsage = VIDRNDT_AP_TEXTURE3D;
  }
  else
  {
    s.iSVPRenderType = 0; 
    s.iDSVideoRendererType = 5;
    s.iRMVideoRendererType = 1;
    s.iQTVideoRendererType = 1;
  }
  s.useGPUAcel = m_enablegpuaccel;

  s.fbUseSPDIF = m_usespdifprority;
  s.fAudioNormalize = m_usenormalize;
  s.fAudioNormalizeRecover = TRUE;
  //TODO: if audio setting is changed and audio clip is running,
  //we should apply it by using CComQIPtr<IAudioSwitcherFilter> m_pASF etc

  s.fCustomSpeakers = m_usecustomspeakersetting;
  return PSNRET_NOERROR;
}
