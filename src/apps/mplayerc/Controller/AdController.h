#pragma once

#include "../SUIButton.h"

class AdController
{
public:
  AdController();
  ~AdController();

  void SetVisible(bool bVisible);
  bool GetVisible();

  std::wstring GetCurAd();

  void GetAds(const std::wstring &sURL);
  void AllowAnimate(bool b);
  void ShowNextAd();
  bool IsCurAdShownDone();
  void SetRect(const RECT &rc, CMemoryDC *pDC);  // set the display rect
  void Paint(CMemoryDC *pDC); // re-Paint the display area

  void OnAdClick();

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

  RECT m_rc;                         // display area
  bool m_bVisible;                   // is display area visible?
  bool m_bAllowAnimate;
};