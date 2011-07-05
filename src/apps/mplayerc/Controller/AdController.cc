#include "stdafx.h"
#include "AdController.h"
#include <regex>
#include "PlayerPreference.h"
#include "SPlayerDefs.h"
#include <time.h>
#include <sinet.h>
#include "Strings.h"

////////////////////////////////////////////////////////////////////////////////
// normal part
AdController::AdController()
: m_bVisible(false)
, m_bAllowAnimate(false)
, m_nCurAd(-1)
, m_nCurX(0)
, m_nCurY(0)
, m_lastAdTime(0)
, m_bTryNextLoopWhenFail(false)
, _mouseover(false)
, _mouseover_time(0)
, m_becheckhide(false)
, m_bCloseBtnShow(false)
, m_bCloseBtnCanClick(false)
, m_marginStr(4)
, m_marginBtn(7)
{
  m_szCurAd.SetSize(0, 0);

  m_vtAds.clear();
  m_hideAds.clear();

  std::wstring hideadstring = PlayerPreference::GetInstance()->GetStringVar(STRVAR_HIDEAD);
  SplitHideAdData(hideadstring);
  CleanUpOlderDatasFromHideAds();
}

AdController::~AdController()
{
  _Stop();
}

////////////////////////////////////////////////////////////////////////////////
// operations
void AdController::SetVisible(bool bVisible)
{
  if (bVisible && m_bVisible != bVisible)
    m_lastAdTime = time(NULL);
  m_bVisible = bVisible;
}

bool AdController::GetVisible()
{
  return m_bVisible;
}

std::wstring AdController::GetCurAd()
{
  if ((m_nCurAd < 0) || (m_nCurAd > m_vtAds.size() - 1))
  {
    return L"";
  }

  return m_vtAds[m_nCurAd].sName;
}

void AdController::SetRect(const RECT &rc, CMemoryDC *pDC)
{
  m_szCurAd = pDC->GetTextExtent(m_vtAds[m_nCurAd].sName.c_str());

  if ((m_szCurAd.cx != m_adrc.Width() || m_szCurAd.cy != m_adrc.Height())
      && m_adrc != rc)
  {
    m_rc = rc;

    int btnwidth = 7;
    int btnheight = 7;
    m_closeBtnrc = WTL::CRect(rc.right - btnwidth - m_marginBtn, m_rc.top + (m_rc.Height() - btnheight) / 2, 
      rc.right - m_marginBtn, m_rc.top + (m_rc.Height() - btnheight) / 2 + btnheight);

    m_adrc = WTL::CRect(rc.left, rc.top, rc.right - rc.left > m_szCurAd.cx? rc.left + m_szCurAd.cx:rc.right, rc.bottom);

/*    m_nCurAd = -1;*/
    m_nCurX = m_adrc.left;
    m_nCurY = m_adrc.top;
  }
}

const RECT& AdController::GetRect()
{
  return m_rc;
}

const RECT& AdController::GetAdRect()
{
  return m_adrc;
}

const RECT& AdController::GetCloseBtnRect()
{
  return m_closeBtnrc;
}

void AdController::_Thread()
{
  //////////////////////////////////////////////////////////////////////////////
  // Note:
  // "20110215firstad;http://#\nsecondad;http://#\n", first 8 characters are date
  // only get ad once per day
  // get the ads string
  if (PlayerPreference::GetInstance()->GetIntVar(INTVAR_PLAYAD))
    return;
  
  std::wstring sAds = PlayerPreference::GetInstance()->GetStringVar(STRVAR_AD);

  // check if need to download ad
  std::wstring sCurDate = GetLocalTimeString();
  std::wstring sAdDate;
  if (sAds.size() >= 8)  // must greater than 8, because has a date prefix
    sAdDate.assign(sAds.begin(), sAds.begin() + 8);

  while (true)
  {
    if (sCurDate == sAdDate)
    {
      SplitAdData(sAds);
      return;
    }

    Logging("fetching ads");
    sAds = sCurDate;
    // Get ads from web
    sinet::refptr<sinet::pool>    net_pool = sinet::pool::create_instance();
    sinet::refptr<sinet::task>    net_task = sinet::task::create_instance();
    sinet::refptr<sinet::request> net_rqst = sinet::request::create_instance();
    sinet::refptr<sinet::config>  net_cfg  = sinet::config::create_instance();

    net_task->use_config(net_cfg);

    net_rqst->set_request_url(m_sURL.c_str());
    net_rqst->set_request_method(REQ_GET);

    net_task->append_request(net_rqst);

    net_pool->execute(net_task);

    while (net_pool->is_running_or_queued(net_task))
    {
      if (_Exit_state(5000))  // can wait 5s until done this job
      {
        ::Sleep(2000);  // sleep for a moment
        if (TryNextLoopWhenFail())
          continue;  // continue next try
        else
          break;
      }
    }
    if (net_rqst->get_response_errcode() != 0)  // judge if successful
    {
      ::Sleep(2000);  // sleep for a moment
      if (TryNextLoopWhenFail())
        continue;  // continue next try
      else
        break;
    }

    std::vector<unsigned char> st_buffer = net_rqst->get_response_buffer();
    if (st_buffer.size() <= 15)
      break;

    if (st_buffer[st_buffer.size() - 1] != '\0')
      st_buffer.push_back('\0');

    sAds += Strings::Utf8StringToWString(std::string(st_buffer.begin(), st_buffer.end()));
    break;  // jump out the download loop
  }

  PlayerPreference::GetInstance()->SetStringVar(STRVAR_AD, sAds);

  if (sAds.length() < 15)
    return;

  SplitAdData(sAds);
  m_becheckhide = true;
}

void AdController::SplitAdData(const std::wstring& data)
{
  // split the data and store them into m_vtAds
  m_vtAds.clear();
  std::tr1::wregex rx(L"([^;]*);([^\\n]*)\\n");
  std::tr1::wsmatch mt;
  std::wstring::const_iterator itS = data.begin() + 8;   // not include the date prefix
  std::wstring::const_iterator itE = data.end();
  bool bMatched = std::tr1::regex_search(itS, itE, mt, rx);
  while (bMatched)
  {
    tagAd ad;
    ad.sName = mt.str(1);
    ad.sLink = mt.str(2);
    m_vtAds.push_back(ad);

    itS = mt[0].second;
    itE = data.end();
    bMatched = std::tr1::regex_search(itS, itE, mt, rx);
  }
}

void AdController::GetAds(const std::wstring &sURL)
{
  _Stop();
  m_sURL = sURL;
  _Start();
}

bool AdController::IsAdsEmpty()
{
  return m_vtAds.empty();
}

bool AdController::IsHideAdsEmpty()
{
  return m_hideAds.empty();
}

void AdController::AllowAnimate(bool b)
{
  m_bAllowAnimate = b;
}

void AdController::ShowNextAd()
{
  if (m_becheckhide)
  {
    CheckDisplayAds();

    if (m_nCurAd != -1)
      --m_nCurAd;

    m_becheckhide = false;
  }

  if (m_vtAds.empty())
    return;

  int old_ad = m_nCurAd;

  if (++m_nCurAd > m_vtAds.size() - 1)
    m_nCurAd = 0;

  //if (old_ad != m_nCurAd)
  m_lastAdTime = time(NULL);

  m_nCurX = m_adrc.left;
  m_nCurY = m_adrc.top;

}

bool AdController::IsCurAdShownDone()
{
  if ((m_nCurAd < 0) || (m_nCurAd > m_vtAds.size() - 1))
    return true;

  if ((time(NULL) - m_lastAdTime) > 15)
    return true;
  
  int closeBtnDimention = m_bCloseBtnShow? m_marginStr + m_closeBtnrc.Width() + m_marginBtn:0;
  if (m_nCurX + m_szCurAd.cx + closeBtnDimention <= m_rc.right)
    return true;
  else
    return false;
}

void AdController::Paint(CMemoryDC *pDC)
{
  //
  if (!GetVisible() || m_vtAds.empty())
    return;

  //
  if ((m_nCurAd < 0) || (m_nCurAd > m_vtAds.size() - 1))
    return;
  
  //
  if (m_bAllowAnimate)
  {
    CSize szCurAd = pDC->GetTextExtent(m_vtAds[m_nCurAd].sName.c_str());
    int closeBtnDimention = m_bCloseBtnShow? m_marginStr + m_closeBtnrc.Width() + m_marginBtn:0;
    if (m_nCurX + szCurAd.cx + closeBtnDimention > m_rc.right)
    {
      m_nCurX -= 2;
      m_bCloseBtnCanClick = false;
    }
    else
      m_bCloseBtnCanClick = true;
  }

  //
  CRgn rgn;
  rgn.CreateRectRgn(m_rc.left, m_rc.top, m_rc.right, m_rc.bottom);
  pDC->SelectClipRgn(&rgn);

  if (m_bCloseBtnShow && m_bCloseBtnCanClick)
    PaintCloseBtn(pDC, m_closeBtnrc);
  //m_szCurAd = pDC->GetTextExtent(m_vtAds[m_nCurAd].sName.c_str());
  pDC->TextOut(m_nCurX, m_nCurY, m_vtAds[m_nCurAd].sName.c_str());

  //
  m_bAllowAnimate = false;
}

void AdController::PaintCloseBtn(CMemoryDC* pDC, const WTL::CRect& rc)
{
  WTL::CPen pen;
  pen.CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
  HPEN oldpen = (HPEN)pDC->SelectObject(pen);

  // draw a diagonal from left_top corner to right_bottom corner
  // draw twice to make it more thicker
  pDC->MoveTo(rc.left, rc.top);
  pDC->LineTo(rc.right, rc.bottom);
  pDC->MoveTo(rc.left + 1, rc.top);
  pDC->LineTo(rc.right + 1, rc.bottom);

  // draw a diagonal from right_top corner to left_bottom corner
  // draw twice to make it more thicker
  pDC->MoveTo(rc.right - 1, rc.top);
  pDC->LineTo(rc.left - 1, rc.bottom);
  pDC->MoveTo(rc.right, rc.top);
  pDC->LineTo(rc.left, rc.bottom);

  pDC->SelectObject(oldpen);
}

void AdController::OnAdClick()
{
  if ((m_nCurAd < 0) || (m_nCurAd > m_vtAds.size() - 1))
    return;

  ::ShellExecute(0, L"open", m_vtAds[m_nCurAd].sLink.c_str(), 0, 0, SW_SHOW);
}

void AdController::DoHideAd()
{
  m_becheckhide = true;

  m_hideAds[m_vtAds[m_nCurAd].sName] = GetLocalTimeString();

  ShowNextAd();
}

bool AdController::TryNextLoopWhenFail()
{
  return m_bTryNextLoopWhenFail;
}

void AdController::SplitHideAdData(const std::wstring& data)
{
  if (data.empty())
    return;

  m_hideAds.clear();
  std::tr1::wregex rx(L"([^;]*);([^\\n]*)\\n");
  std::tr1::wsmatch mt;
  std::wstring::const_iterator itS = data.begin();   // not include the date prefix
  std::wstring::const_iterator itE = data.end();
  bool bMatched = std::tr1::regex_search(itS, itE, mt, rx);
  while (bMatched)
  {
    m_hideAds[mt.str(1)] = mt.str(2);
    
    itS = mt[0].second;
    itE = data.end();
    bMatched = std::tr1::regex_search(itS, itE, mt, rx);
  }
}

void AdController::CleanUpOlderDatasFromHideAds()
{
  std::map<std::wstring, std::wstring>::iterator it = m_hideAds.begin();

  while (it != m_hideAds.end())
  {
    if (ShouldBeCleanUp(it->second))
      it = m_hideAds.erase(it);
    else
      ++it;
  }

  std::map<std::wstring, std::wstring>::iterator itt = m_hideAds.begin();
  while(itt != m_hideAds.end())
  {
    std::wstring str = itt->first + L";" + itt->second + L"\n";
    Logging(L"%s", str.c_str());

    ++itt;
  }
}

bool AdController::ShouldBeCleanUp(const std::wstring& str)
{
  int year, month, day;

  std::wstring timestr(str.begin(), str.begin() + 4);
  year = _wtoi(timestr.c_str());

  timestr.assign(str.begin() + 4, str.begin() + 6);
  month = _wtoi(timestr.c_str());

  timestr.assign(str.begin() + 6, str.begin() + 8);
  day = _wtoi(timestr.c_str());

  time_t tCur = ::time(0);
  struct tm *pTM = ::localtime(&tCur);
  if (pTM->tm_year + 1900 - year > 0 &&
    pTM->tm_mon + 1 == month && 
    pTM->tm_mday == day)
    return TRUE;
  else
    return FALSE;
}

void AdController::CheckDisplayAds()
{
  std::vector<tagAd>::iterator it = m_vtAds.begin();
  while(it != m_vtAds.end())
  {
    if (m_hideAds.find(it->sName) != m_hideAds.end())
      it = m_vtAds.erase(it);
    else
      ++it;
  }

  PreserveDisplayAds();
  PreserveHideAds();
}

void AdController::PreserveDisplayAds()
{
  std::wstring str = GetLocalTimeString();

  std::vector<tagAd>::const_iterator it = m_vtAds.begin();
  while(it != m_vtAds.end())
  {
    str += it->sName + L";";
    str += it->sLink + L"\n";

    ++it;
  }

  PlayerPreference::GetInstance()->SetStringVar(STRVAR_AD, str);
}

void AdController::PreserveHideAds()
{
  std::wstring str(L"");

  std::map<std::wstring, std::wstring>::iterator it = m_hideAds.begin();
  while(it != m_hideAds.end())
  {
    str += it->first + L";";
    str += it->second + L"\n";

    ++it;
  }

  PlayerPreference::GetInstance()->SetStringVar(STRVAR_HIDEAD, str);
}

std::wstring AdController::GetLocalTimeString()
{
  wchar_t szYear[5] = {0};
  wchar_t szMonth[3] = {0};
  wchar_t szDay[3] = {0};
  time_t tCur = ::time(0);
  struct tm *pTM = ::localtime(&tCur);
  ::_ltow(pTM->tm_year + 1900, szYear, 10);
  ::_ltow(pTM->tm_mon + 1, szMonth, 10);
  ::_ltow(pTM->tm_mday, szDay, 10);

  if (szMonth[1] == L'\0')
  {
    szMonth[1] = szMonth[0];
    szMonth[0] = L'0';
  }

  if (szDay[1] == L'\0')
  {
    szDay[1] = szDay[0];
    szDay[0] = L'0';
  }

  std::wstring timestr = std::wstring() + szYear + szMonth + szDay;

  return timestr;
}

void AdController::SetCloseBtnDisplay(BOOL beshow)
{
  m_bCloseBtnShow = beshow;
}

bool AdController::IsCloseBtnCanClick()
{
  return m_bCloseBtnCanClick;
}