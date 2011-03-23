#pragma once

#include "../SUIButton.h"
#include "threadhelper.h"

class AdController : public ThreadHelperImpl<AdController>
{
public:
  AdController();
  ~AdController();

  void _Thread();

  void SetVisible(bool bVisible);
  bool GetVisible();

  std::wstring GetCurAd();
  bool IsAdsEmpty();

  void GetAds(const std::wstring &sURL);
  void AllowAnimate(bool b);
  void ShowNextAd();
  bool IsCurAdShownDone();
  void SetRect(const RECT &rc, CMemoryDC *pDC);  // set the display rect
  void Paint(CMemoryDC *pDC); // re-Paint the display area

  void OnAdClick();
  time_t _mouseover_time;
protected:
  bool TryNextLoopWhenFail();
  void SplitAdData(const std::wstring& data);

private:
  struct tagAd
  {
    std::wstring sName;   // ad name
    std::wstring sLink;   // ad link
  };
  std::vector<tagAd > m_vtAds;       // store ads info
  int m_nCurAd;  // current display ad
  int m_nCurX;   // current ad's x
  int m_nCurY;   // current ad's y
  CSize m_szCurAd;

  std::wstring m_sURL;

  RECT m_rc;                         // display area
  bool m_bVisible;                   // is display area visible?
  bool m_bAllowAnimate;

  bool m_bTryNextLoopWhenFail;       // should try next loop when download ad failure?
  time_t m_lastAdTime;
};