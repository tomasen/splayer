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
, m_bTryNextLoopWhenFail(false)
{
  GetAds(L"https://www.shooter.cn/api/v2/prom.php");
  m_szCurAd.SetSize(0, 0);
}

AdController::~AdController()
{
  _Stop();
}

////////////////////////////////////////////////////////////////////////////////
// operations
void AdController::SetVisible(bool bVisible)
{
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
  if ((m_rc.left != rc.left) || (m_rc.top != rc.top) || (m_rc.right != rc.right) || (m_rc.bottom != rc.bottom))
  {
    m_rc = rc;

    m_nCurAd = -1;
    m_nCurX = m_rc.left;
    m_nCurY = m_rc.top;
  }
}

void AdController::_Thread()
{
  ////////////////////////////////////////////////////////////////////////////////
  //// Demo
  //tagAd adxxx;
  //adxxx.sLink = L"http://splayer.org";
  //adxxx.sName = L"我用射手影音，就是这么自信2 -- 我用射手影音，就是这么自信";
  //m_vtAds.push_back(adxxx);

  //adxxx.sName = L"用射手影音，明智的选择";
  //m_vtAds.push_back(adxxx);

  //adxxx.sName = L"今天你射了没有？没有？赶快点我下载播放器";
  //m_vtAds.push_back(adxxx);

  //adxxx.sName = L"看电影，用射手影音吧";
  //m_vtAds.push_back(adxxx);

  //return;
  ////////////////////////////////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////////////
  // Note:
  // "20110215firstad;http://#\nsecondad;http://#\n", first 8 characters are date
  // only get ad once per day
  // get the ads string
  std::wstring sAds = PlayerPreference::GetInstance()->GetStringVar(STRVAR_AD);

  // check if need to download ad
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

  std::wstring sCurDate = std::wstring() + szYear + szMonth + szDay;
  std::wstring sAdDate;
  if (sAds.size() > 8)  // must greater than 8, because has a date prefix
    sAdDate.assign(sAds.begin(), sAds.begin() + 8);

  while (true)
  {
    if (sAds.empty() || (sCurDate != sAdDate))
    {
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
      if (st_buffer[st_buffer.size() - 1] != '\0')
        st_buffer.push_back('\0');

      sAds = Strings::Utf8StringToWString(std::string(st_buffer.begin(), st_buffer.end()));
      sAds = sCurDate + sAds;

      PlayerPreference::GetInstance()->SetStringVar(STRVAR_AD, sAds);
    }

    break;  // jump out the download loop
  }
  if (sAds.length() < 15)
    return;
  // split the sAds and store them into m_vtAds
  std::tr1::wregex rx(L"([^;]*);([^\\n]*)\\n");
  std::tr1::wsmatch mt;
  std::wstring::const_iterator itS = sAds.begin() + 8;   // not include the date prefix
  std::wstring::const_iterator itE = sAds.end();
  bool bMatched = std::tr1::regex_search(itS, itE, mt, rx);
  while (bMatched)
  {
    tagAd ad;
    ad.sName = mt.str(1);
    ad.sLink = mt.str(2);
    m_vtAds.push_back(ad);

    itS = mt[0].second;
    itE = sAds.end();
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

void AdController::AllowAnimate(bool b)
{
  m_bAllowAnimate = b;
}

void AdController::ShowNextAd()
{
  ++m_nCurAd;
  if (m_nCurAd > m_vtAds.size() - 1)
    m_nCurAd = 0;

  m_nCurX = m_rc.left;
  m_nCurY = m_rc.top;
}

bool AdController::IsCurAdShownDone()
{
  if ((m_nCurAd < 0) || (m_nCurAd > m_vtAds.size() - 1))
    return true;

  if (m_nCurX + m_szCurAd.cx <= m_rc.right)
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
    if (m_nCurX + szCurAd.cx > m_rc.right)
      m_nCurX -= 2;
  }

  //
  CRgn rgn;
  rgn.CreateRectRgn(m_rc.left, m_rc.top, m_rc.right, m_rc.bottom);
  pDC->SelectClipRgn(&rgn);
  m_szCurAd = pDC->GetTextExtent(m_vtAds[m_nCurAd].sName.c_str());
  pDC->TextOut(m_nCurX, m_nCurY, m_vtAds[m_nCurAd].sName.c_str());

  //
  m_bAllowAnimate = false;
}

void AdController::OnAdClick()
{
  if ((m_nCurAd < 0) || (m_nCurAd > m_vtAds.size() - 1))
    return;

  ::ShellExecute(0, L"open", m_vtAds[m_nCurAd].sLink.c_str(), 0, 0, SW_SHOW);
}

bool AdController::TryNextLoopWhenFail()
{
  return m_bTryNextLoopWhenFail;
}