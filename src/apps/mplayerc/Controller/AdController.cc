#include "stdafx.h"
#include "AdController.h"

////////////////////////////////////////////////////////////////////////////////
// normal part
AdController::AdController()
: m_bVisible(false)
, m_bAllowAnimate(false)
, m_nCurAd(-1)
, m_nCurX(0)
, m_nCurY(0)
{
  GetAds(L"");
  m_szCurAd.SetSize(0, 0);
}

AdController::~AdController()
{
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

void AdController::GetAds(const std::wstring &sURL)
{
  tagAd ad;
  ad.sLink = L"http://www.splayer.org";

  ad.sName = L"一二三四五六七八九程序设计中国";
  m_vtAds.push_back(ad);

  ad.sName = L"chenjian";
  m_vtAds.push_back(ad);

  ad.sName = L"3333333333333555555555555555555";
  m_vtAds.push_back(ad);

  ad.sName = L"444444444444444";
  m_vtAds.push_back(ad);

  //WCHAR sStr[] = L"chen;http\nxiao;ftp\n";
  //std::wstring sSep = L"\n";
  //LPCTSTR p = 0;
  //p = ::wcstok(sStr, sSep.c_str());
  //while (p != 0)
  //{
  //  AfxMessageBox(p);
  //  p = ::wcstok(0, sSep.c_str());
  //}
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