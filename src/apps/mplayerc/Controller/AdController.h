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
  bool IsHideAdsEmpty();

  void SetCloseBtnDisplay(BOOL beshow);
  bool IsCloseBtnCanClick();

  void GetAds(const std::wstring &sURL);
  void AllowAnimate(bool b);
  void ShowNextAd();
  bool IsCurAdShownDone();
  void SetRect(const RECT &rc, CMemoryDC *pDC);  // set the display rect
  const RECT& GetRect();
  const RECT& GetAdRect();
  const RECT& GetCloseBtnRect();
  void Paint(CMemoryDC *pDC); // re-Paint the display area
  void PaintCloseBtn(CMemoryDC* pDC, const WTL::CRect& rc);

  void OnAdClick();
  void DoHideAd();
  DWORD  _mouseover_time;
  bool   _mouseover;
  
  void CleanUpOlderDatasFromHideAds();
  bool ShouldBeCleanUp(const std::wstring& str);

  void CheckDisplayAds();
  void PreserveDisplayAds();
  void PreserveHideAds();

protected:
  bool TryNextLoopWhenFail();
  void SplitAdData(const std::wstring& data);       //parse string to data into display ads vector
  void SplitHideAdData(const std::wstring& data);   //parse string to data into hide ads map
  std::wstring GetLocalTimeString();

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

  WTL::CRect m_rc;                         // display area
  WTL::CRect m_adrc;                       // ads area
  WTL::CRect m_closeBtnrc;                 // close button area
  int  m_marginStr;                        // margin between close button and ad string
  int  m_marginBtn;                        // margin between close button and other button
  bool m_bVisible;                   // is display area visible?
  bool m_bAllowAnimate;

  bool m_bCloseBtnShow;
  bool m_bCloseBtnCanClick;

  bool m_bTryNextLoopWhenFail;       // should try next loop when download ad failure?
  time_t m_lastAdTime;

  std::map<std::wstring, std::wstring> m_hideAds; // the container that stores the hide ads
  bool m_becheckhide;                             // if this valuable is true, so check the hide ads container, 
                                                  // and delete the same datas from vtAds vector

};