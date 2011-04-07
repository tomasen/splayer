/* 
*	Copyright (C) 2003-2006 Gabest
*	http://www.gabest.org
*
*  This Program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2, or (at your option)
*  any later version.
*   
*  This Program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*  GNU General Public License for more details.
*   
*  You should have received a copy of the GNU General Public License
*  along with GNU Make; see the file COPYING.  If not, write to
*  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
*  http://www.gnu.org/copyleft/gpl.html
*
*/

// PlayerToolBar.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include <math.h>
#include <atlbase.h>
#include <afxpriv.h>
#include "PlayerToolBar.h"
#include "MainFrm.h"
#include <fstream>
#include <Strings.h>
#include "../../svplib/svplib.h"
#include "GUIConfigManage.h"
#include "ButtonManage.h"
#include "ResLoader.h"

typedef HRESULT (__stdcall * SetWindowThemeFunct)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
#define TIMER_ADPLAYSWITCH 7013
#define TIMER_ADPLAY 7014
#define TIMER_ADFETCHING 7015
#define ID_VOLUME_THUMB 126356

#define CONFIGBUTTON(btnname,bmp,fixalign,fixcrect,notbutton,id,hide,hidewidth,relativealign,pbuttonname,relativecrect) \
  m_btnList.AddTail(new CSUIButton(L#bmp,fixalign,fixcrect,notbutton,id,hide,relativealign,m_btnList.GetButton(L#pbuttonname), \
  relativecrect,hidewidth,L#btnname)); \
  

#define CONFIGADDALIGN(pbtnname,relativealign,pbuttonname,relativecrect) \
  m_btnList.GetButton(L#pbtnname)->addAlignRelButton(relativealign,m_btnList.GetButton(L#pbuttonname),relativecrect);

// CPlayerToolBar

IMPLEMENT_DYNAMIC(CPlayerToolBar, CToolBar)
CPlayerToolBar::CPlayerToolBar() :
m_hovering(0),
holdStatStr(0),
iButtonWidth (30),
m_pbtnList(&m_btnList),
m_bMouseDown(FALSE),
m_nHeight(90),
m_movieshare_hidestat(1)
{
}

CPlayerToolBar::~CPlayerToolBar()
{
}


BOOL CPlayerToolBar::Create(CWnd* pParentWnd)
{
  int iToolBarID = IDB_PLAYERTOOLBAR;
  /*
  CRect rcDesktop;
  GetDesktopWindow()->GetWindowRect(&rcDesktop);

  if( rcDesktop.Width() < 1200){
  iToolBarID = IDB_PLAYERTOOLBAR_SMALL;
  iButtonWidth = 20;
  }*/


  if(!__super::CreateEx(pParentWnd,
    TBSTYLE_FLAT|TBSTYLE_TRANSPARENT|TBSTYLE_AUTOSIZE,
    WS_CHILD|WS_VISIBLE|CBRS_ALIGN_BOTTOM , CRect(0,0,0,0))  //CBRS_TOOLTIPS NEW UI
    ) //|| !LoadToolBar(iToolBarID)
    return FALSE;

  GetToolBarCtrl().SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);

  AppSettings& s = AfxGetAppSettings();
  GUIConfigManage cfgfile;
  ButtonManage  cfgbtn;
  std::wstring cfgfilepath(L"skins\\");
  cfgfilepath += s.skinname;
  cfgfilepath += L"\\BottomToolBarButton.dat";

  //cfgfile.SetCfgFilePath(L"skins\\BottomToolBarButton.dat");
  cfgfile.SetCfgFilePath(cfgfilepath);
  cfgfile.ReadFromFile();
  if (cfgfile.IsFileExist())
  {
    cfgbtn.SetParse(cfgfile.GetCfgString(), &m_btnList);
    cfgbtn.ParseConfig(FALSE);
  }
  else
    DefaultButtonManage();

  PointVolumeBtn();

  cursorHand = ::LoadCursor(NULL, IDC_HAND);

  SetTimer(TIMER_ADPLAY, 100, NULL);
  SetTimer(TIMER_ADPLAYSWITCH, 5000, NULL);
  SetTimer(TIMER_ADFETCHING, 15000, NULL);
  
  CSVPToolBox svptoolbox;
  if (svptoolbox.bFontExist(L"Comic Sans MS"))
  {
    LOGFONT lf;
    lf.lfWidth = 0;
    lf.lfHeight = 15.0;
    lf.lfEscapement = 0;
    lf.lfWeight = 0;
    lf.lfPitchAndFamily = 0;
    lf.lfStrikeOut = FALSE;
    lf.lfItalic = FALSE;
    lf.lfUnderline = FALSE;
    wsprintf(lf.lfFaceName, L"Comic Sans MS");
    m_statft.CreateFontIndirect(&lf);
  }
  else
    GetSystemFontWithScale(&m_statft, 13.0);

  GetSystemFontWithScale(&m_adsft, 14.0);

  CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
  m_nLogDPIY = pFrame->m_nLogDPIY;

  m_volctrl.Create(this);

  EnableToolTips(TRUE);

  m_nHeight = max(45, m_btnList.GetMaxHeight());
  if (m_nHeight > 45)
    m_nHeight += 4;

  return TRUE;
}

void CPlayerToolBar::OnMove(int x, int y){
  __super::OnMove(x, y);
  ReCalcBtnPos();
}
void CPlayerToolBar::ReCalcBtnPos(){
  CRect rc;
  GetWindowRect(&rc);
  m_btnList.OnSize( rc);
}
void CPlayerToolBar::OnSize(UINT nType, int cx, int cy)
{
  __super::OnSize(nType, cx, cy);

  ArrangeControls();

}

BOOL CPlayerToolBar::PreCreateWindow(CREATESTRUCT& cs)
{
  if(!__super::PreCreateWindow(cs))
    return FALSE;

  m_dwStyle &= ~CBRS_BORDER_TOP;
  m_dwStyle &= ~CBRS_BORDER_LEFT;
  m_dwStyle &= ~CBRS_BORDER_RIGHT;
  m_dwStyle &= ~CBRS_BORDER_BOTTOM;
  //	m_dwStyle |= CBRS_SIZE_FIXED;

  return TRUE;
}

void CPlayerToolBar::HideMovieShareBtn(BOOL hide)
{
  m_movieshare_hidestat = hide;
}

void CPlayerToolBar::ArrangeControls()
{
  CRect rc;
  GetWindowRect(&rc);
  long iWidth = rc.Width();
  CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());
  double skinsRate = (double)pFrame->m_lMinFrameWidth / 310;
  BOOL isaudio = (pFrame && pFrame->IsSomethingLoaded() && pFrame->m_fAudioOnly)?TRUE:FALSE;

  m_btnList.SetCurrentHideState(rc.Width(),skinsRate,m_nLogDPIY);

  if(IsMuted())
    m_btnList.SetHideStat(L"VOLUME.BMP", TRUE);
  else
    //m_btnList.SetHideStat(L"VOLUME.BMP", FALSE|hideT1);
    m_btnList.SetHideStat(L"MUTED.BMP", TRUE);
  
  if(pFrame && pFrame->IsSomethingLoaded() && pFrame->m_fAudioOnly)
  {
    m_btnList.SetHideStat(ID_NAVIGATE_SKIPBACK , 0);
    m_btnList.SetHideStat(ID_NAVIGATE_SKIPFORWARD , 0);
    m_btnList.SetHideStat(ID_MOVIESHARE, 1);
  }

  m_btnList.SetHideStat(ID_PLAY_FWD , 0);
  m_btnList.SetHideStat(ID_PLAY_BWD , 0);

  m_btnList.OnSize(rc);

  if(!::IsWindow(m_volctrl.m_hWnd)) return;


}

void CPlayerToolBar::SetMute(bool fMute)
{

  AfxGetAppSettings().fMute = fMute;

}

bool CPlayerToolBar::IsMuted()
{

  return AfxGetAppSettings().fMute;
}

int CPlayerToolBar::GetVolume()
{
  int volume = min( m_volctrl.GetPos() , 100);
  volume = (int)((log10(1.0*volume)-2)*5000);
  volume = ((int)(volume/100)*100);
  //SVP_LogMsg3(("Vol %d") , volume);
  volume = max(min(volume, 0), -10000);
  return(IsMuted() ? -10000 : volume);
}

void CPlayerToolBar::SetVolume(int volume)
{
  /*
  volume = (int)pow(10, ((double)volume)/5000+2);
  volume = max(min(volume, 100), 1);
  */
  m_volctrl.SetPosInternal(volume);

}

BEGIN_MESSAGE_MAP(CPlayerToolBar, CToolBar)
  ON_MESSAGE_VOID(WM_INITIALUPDATE, OnInitialUpdate)
  ON_COMMAND_EX(ID_VOLUME_MUTE, OnVolumeMute)
  ON_UPDATE_COMMAND_UI(ID_VOLUME_MUTE, OnUpdateVolumeMute)
  ON_COMMAND_EX(ID_VOLUME_UP, OnVolumeUp)
  ON_COMMAND_EX(ID_VOLUME_DOWN, OnVolumeDown)
  ON_WM_NCPAINT()
  ON_WM_MOVE()
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_PAINT()
  ON_WM_SIZE()
  ON_WM_TIMER()
  ON_WM_NCCALCSIZE()
  ON_WM_MOUSEMOVE()
  ON_WM_SETCURSOR()
  ON_WM_ERASEBKGND()
  ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnTtnNeedText)
  ON_WM_RBUTTONUP()
  ON_WM_RBUTTONDOWN()
  ON_WM_MOUSELEAVE()
END_MESSAGE_MAP()

// CPlayerToolBar message handlers


BOOL CPlayerToolBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message){

  CPoint pt;
  ::GetCursorPos (&pt);
  ScreenToClient (&pt);

  if(m_nItemToTrack){	
    SetCursor(cursorHand );
    return TRUE;
  }
  return CWnd::OnSetCursor(pWnd, 0, 0);
}

CSize CPlayerToolBar::CalcFixedLayout(BOOL bStretch,BOOL bHorz ){


  CSize size( 32767, m_nHeight * m_nLogDPIY / 96 );

  if ( CWnd* pParent = GetParentFrame() )
  {
    CRect rc;
    pParent->GetWindowRect( &rc );
    size.cx = rc.Width() - 2;
  }

  CRect rc;
  GetWindowRect(&rc);
  m_btnList.OnSize( rc);

  return size;



  //return __super::CalcFixedLayout(bStretch,bHorz);
}
void CPlayerToolBar::OnPaint()
{
  //LONGLONG startTime = AfxGetMyApp()->GetPerfCounter();
  CPaintDC dc(this); // device context for painting
  CRect paintRect(dc.m_ps.rcPaint);
  AppSettings& s = AfxGetAppSettings();

  CRect rcClient;
  GetClientRect(&rcClient);
  CMemoryDC hdc(&dc, rcClient);
  hdc.SetBkMode(TRANSPARENT);

  CRect rc;
  GetWindowRect(&rc);

  if( paintRect == m_btnVolBG->m_rcHitest){
    m_btnVolBG->OnPaint( &hdc, rc);
    m_btnVolTm->OnPaint( &hdc, rc);

    return;
  }

  CRect rcBottomSqu = rcClient;
  rcBottomSqu.top = rcBottomSqu.bottom - 10;

  CRect rcUpperSqu = rcClient;

  CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());

  if (s.skinid == ID_SKIN_FIRST)
    hdc.FillSolidRect(rcUpperSqu, s.GetColorFromTheme(_T("ToolBarBG"), NEWUI_COLOR_TOOLBAR_UPPERBG));
  else
  {

    CBitmap* cbm = CBitmap::FromHandle(pFrame->m_btoolbarbg);
    CDC bmpDc;
    bmpDc.CreateCompatibleDC(&hdc);
    HBITMAP oldhbm = (HBITMAP)bmpDc.SelectObject(cbm);
    BITMAP btmp;
    cbm->GetBitmap(&btmp);
    hdc.StretchBlt(0, 0, rcUpperSqu.Width(), rcUpperSqu.Height(), &bmpDc, 0, 0, btmp.bmWidth, btmp.bmHeight - 2, SRCCOPY);
    bmpDc.SelectObject(oldhbm);
  }
  HFONT holdft = NULL;
  if (m_adctrl.GetVisible())
    holdft = (HFONT)hdc.SelectObject(m_adsft);
  else
    holdft = (HFONT)hdc.SelectObject(m_statft);
  hdc.SetTextColor(s.GetColorFromTheme(_T("ToolBarTimeText"), 0xffffff) );

  UpdateButtonStat();

  int volume = min( m_volctrl.GetPos() , m_volctrl.GetRangeMax() );
  m_btnVolTm->m_rcHitest.MoveToXY(m_btnVolBG->m_rcHitest.left +  ( m_btnVolBG->m_rcHitest.Width() * volume / m_volctrl.GetRangeMax() ) - m_btnVolTm->m_rcHitest.Width()/2
    , m_btnVolBG->m_rcHitest.top + (m_btnVolBG->m_rcHitest.Height() -  m_btnVolTm->m_rcHitest.Height() ) / 2 );
  
  m_btnList.PaintAll(&hdc, rc);

  std::wstring sTimeBtnString = m_adctrl.GetCurAd();

  if (!sTimeBtnString.empty())
  {
    CSize size = dc.GetTextExtent(sTimeBtnString.c_str());

    CSUIButton* cbtn = m_btnList.GetButton(L"SHARE");
    CRect btnrc = cbtn->m_rcHitest - rc.TopLeft();
    btnrc.left = btnrc.right + 2;
    int width = m_btnList.GetRelativeMinLength(rc, cbtn) - cbtn->m_rcHitest.Width() - 4;
    if (size.cx > 0)
      btnrc.right = btnrc.left + min(width, size.cx);
    else
      btnrc.right = btnrc.left;
    btnrc.top = (rc.Height() - size.cy) / 2;
    btnrc.bottom = btnrc.top + size.cy;

    m_adctrl.SetRect(btnrc, &hdc);
    m_adctrl.Paint(&hdc);  
  }
  hdc.SelectObject(holdft);

}
void CPlayerToolBar::UpdateButtonStat(){
  CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());
  BOOL fShow = pFrame->GetUIStat( ID_PLAY_MANUAL_STOP );
  m_btnList.SetHideStat( ID_PLAY_PLAY , fShow );
  //m_btnList.SetHideStat( ID_PLAY_MANUAL_STOP , !fShow );
  //m_btnList.SetHideStat( ID_PLAY_FRAMESTEP , !fShow );
  m_btnList.SetHideStat( ID_PLAY_PAUSE , !fShow );
  BOOL bLogo = pFrame->IsSomethingLoaded() ;
  m_btnList.SetHideStat(_T("SPLAYER.BMP"), bLogo);
  m_btnList.SetHideStat( ID_MOVIESHARE , m_movieshare_hidestat);
  //m_btnList.SetDisableStat(ID_SUBTOOLBARBUTTON, !bLogo);
  if(!bLogo)
    m_timerstr.Empty();

  BOOL bSub = pFrame->IsSubLoaded();
  m_btnList.SetDisableStat( ID_SUBDELAYINC, !bSub, m_nItemToTrack == ID_SUBDELAYINC);
  m_btnList.SetDisableStat( ID_SUBDELAYDEC, !bSub, m_nItemToTrack == ID_SUBDELAYDEC);
  ReCalcBtnPos();
  
}
void CPlayerToolBar::OnNcPaint() // when using XP styles the NC area isn't drawn for our toolbar...
{
  //New UI GetSysColor(COLOR_BTNFACE)

  //填充背景-----------------------------------------   
  //dc.FillSolidRect(wr, RGB(214,219,239) );   

  // Do not call CToolBar::OnNcPaint() for painting messages
}

void CPlayerToolBar::OnInitialUpdate()
{
  ArrangeControls();
}

void CPlayerToolBar::SetStatusTimer(CString str , UINT timer )
{
  if(m_timerstr == str) return;

  str.Trim();
  
  if(holdStatStr && !timer){
    m_timerqueryedstr = str;
  }else{
    m_timerstr = str;
    Invalidate();
  }
  if(timer){
    KillTimer(TIMER_STATERASER); 
    holdStatStr = TRUE;
    SetTimer(TIMER_STATERASER, timer , NULL);
  }

}

void CPlayerToolBar::SetStatusTimer(REFERENCE_TIME rtNow, REFERENCE_TIME rtDur, bool fHighPrecision, const GUID* pTimeFormat, double playRate)
{
  ASSERT(pTimeFormat);

  CString str;
  CString posstr, durstr;

  if(*pTimeFormat == TIME_FORMAT_MEDIA_TIME)
  {
    DVD_HMSF_TIMECODE tcNow = RT2HMSF(rtNow);
    DVD_HMSF_TIMECODE tcDur = RT2HMSF(rtDur);

    if(tcDur.bHours > 0 || (rtNow >= rtDur && tcNow.bHours > 0)) 
      posstr.Format(_T("%02d:%02d:%02d"), tcNow.bHours, tcNow.bMinutes, tcNow.bSeconds);
    else 
      posstr.Format(_T("%02d:%02d"), tcNow.bMinutes, tcNow.bSeconds);

    if(tcDur.bHours > 0)
      durstr.Format(_T("%02d:%02d:%02d"), tcDur.bHours, tcDur.bMinutes, tcDur.bSeconds);
    else
      durstr.Format(_T("%02d:%02d"), tcDur.bMinutes, tcDur.bSeconds);

    if(fHighPrecision)
    {
      str.Format(_T("%s.%03d"), posstr, (rtNow/10000)%1000);
      posstr = str;
      str.Format(_T("%s.%03d"), durstr, (rtDur/10000)%1000);
      durstr = str;
      str.Empty();
    }
  }
  else if(*pTimeFormat == TIME_FORMAT_FRAME)
  {
    posstr.Format(_T("%I64d"), rtNow);
    durstr.Format(_T("%I64d"), rtDur);
  }

  str = (/*start <= 0 &&*/ rtDur <= 0) ? posstr : posstr + _T(" / ") + durstr;

  SYSTEM_POWER_STATUS status;
  GetSystemPowerStatus(&status);
  CString szPower ;
  if ( status.BatteryFlag != 128 && status.BatteryFlag != 255 && status.BatteryLifePercent < 91 ){
    szPower.Format(ResStr(IDS_STATUS_BAR_LABEL_BATTRAY_WITH_PADDING), status.BatteryLifePercent);
  }else{
    //szPower = ResStr(IDS_STATUS_BAR_LABEL_BATTRAY_UNLIMIT);
  }
  CString szPlayrate;
  if(fabs(playRate - 1.0) > 0.02 && playRate > 0.01)	{
    szPlayrate.Format(ResStr(IDS_STATUS_BAR_LABEL_PLAY_SPEED_WITH_PADDING), playRate);
  }

  CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());
  CString szPlayingFileName = pFrame->GetCurPlayingFileName();
  if(!szPlayingFileName.IsEmpty()){
    //szPlayingFileName.Append(_T("  "));
  }

  //szPlayingFileName
  SetStatusTimer( str + szPlayrate + szPower + m_buffering );
}
BOOL CPlayerToolBar::OnVolumeMute(UINT nID)
{
  AfxGetAppSettings().fMute = !IsMuted();
  ArrangeControls();
  return FALSE;
}

void CPlayerToolBar::OnUpdateVolumeMute(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(true);
  pCmdUI->SetCheck(IsMuted());
}

BOOL CPlayerToolBar::OnVolumeUp(UINT nID)
{
  m_volctrl.IncreaseVolume();
  return FALSE;
}

BOOL CPlayerToolBar::OnVolumeDown(UINT nID)
{
  m_volctrl.DecreaseVolume();
  return FALSE;
}

bool  CPlayerToolBar::OnSetVolByMouse(CPoint point, BOOL byClick)
{
  CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());
  long nTBPos = point.x - m_btnVolBG->m_rcHitest.left;
  long TBMax = m_btnVolBG->m_rcHitest.right - m_btnVolBG->m_rcHitest.left;
  nTBPos = max(0 , min(TBMax , nTBPos) );
  int Vol = nTBPos * m_volctrl.GetRangeMax() / TBMax;
  if(byClick)
  {
    int oldVol = m_volctrl.GetPos();
    if(Vol > 100 )
    {
      Vol = max(oldVol,100) + 2;
    }
  }
  else
  {
    CString szVol;
    int VolPercent = Vol;
    if(VolPercent>100)
    {
      VolPercent = 100 + (VolPercent-100) * 900/ (m_volctrl.GetRangeMax() - 100);
    }
    szVol.Format(_T("%d%%") , VolPercent);
    CPoint posTip(m_btnVolBG->m_rcHitest.left + nTBPos, m_btnVolBG->m_rcHitest.top);

    pFrame->m_tip.SetTips(szVol, 1, &posTip);
  }
  m_volctrl.SetPosInternal( Vol );


  pFrame->OnPlayVolume(0);
  /*
  CRect rc;
  GetWindowRect(&rc);
  CRect pRect(m_btnVolBG->m_rcHitest);
  pRect -= rc.TopLeft();
  InvalidateRect( pRect,true);*/

  Invalidate(FALSE);

  return true;
}

INT_PTR CPlayerToolBar::OnToolHitTest(	CPoint point,TOOLINFO* pTI 	) const
{
  if(!pTI){
    return -1;
  }

  CRect rc;
  CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());
  GetWindowRect(&rc);
  point += rc.TopLeft() ;
  //CSUIBtnList* x = (CSUIBtnList*)&m_btnList;
  UINT ret = m_nItemToTrack;//m_pbtnList->OnHitTest(point,rc);
  //SendMessage(WM_USER+31, (point.x & 0xffff) << 16 | (0xffff & point.y), (UINT_PTR)&ret );
  if(ret){


    pTI->hwnd = m_hWnd;
    pTI->uId = (UINT) (ret);
    //pTI->uFlags = TTF_IDISHWND;
    pTI->lpszText = LPSTR_TEXTCALLBACK;
    RECT rcClient;
    GetClientRect(&rcClient);

    pTI->rect = rcClient;

    return pTI->uId;

  }
  return -1;

};
void CPlayerToolBar::OnMouseMove(UINT nFlags, CPoint point)
{
  int diffx = m_lastMouseMove.x - point.x;
  int diffy = m_lastMouseMove.y - point.y;
  CPoint lastMouseMove = m_lastMouseMove;
  CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());
  BOOL bMouseMoved =  diffx || diffy ;
  if(bMouseMoved || m_bMouseDown){
    m_lastMouseMove = point;
    KillTimer(TIMER_CLOSETOOLBAR);
    if(pFrame->IsSomethingLoaded() && pFrame->m_fFullScreen){
      SetTimer(TIMER_CLOSETOOLBAR, 5000, NULL);
    }

  }

  CRect rc;
  GetWindowRect(&rc);
  point += rc.TopLeft();

  // if move on ads
  static const HCURSOR hOrgCursor = (HCURSOR)::GetClassLong(GetSafeHwnd(), GCL_HCURSOR);
  if (m_adctrl.GetVisible())
  {
/*    CRect rcAd = m_btnplaytime->m_rcHitest - rc.TopLeft();*/
    CRect rcAd = m_adctrl.GetRect();
    CPoint pi = point;
    ScreenToClient(&pi);
    if (rcAd.PtInRect(pi))
    {
      SetCursor(cursorHand);
      m_adctrl._mouseover_time = time(NULL);
    }
  }

  if(m_nItemToTrack == ID_VOLUME_THUMB && m_bMouseDown)
  {
    if( bMouseMoved)
    {
      OnSetVolByMouse(point);
    }
  }
  else if(bMouseMoved)
  {
    UINT ret = m_btnList.OnHitTest(point, rc, -1);
    m_nItemToTrack = ret;
    if(ret)
    {
    }
    else if(!m_tooltip.IsEmpty())
    {
      m_tooltip.Empty();
    }

    if(!m_nItemToTrack)
    {
      pFrame->m_tip.SetTips(_T(""));
    }
    if(m_btnList.HTRedrawRequired)
    {
      Invalidate();
    }

    AppSettings& s = AfxGetAppSettings();
    if(s.bUserAeroUI() && bMouseMoved && m_bMouseDown && pFrame )
    {
      pFrame->m_lTransparentToolbarPosStat = 1;

      pFrame->m_wndFloatToolBar->PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
    }
  }

  // Add by cjbw1234
  // 当鼠标拖拽toolbar停止后,Windows并不向toolbar发送WM_LBUTTONUP和WM_MOVE消息,
  // 这造成了状态不一致,只好手动发送这个消息
  if (::GetKeyState(VK_LBUTTON) & 0x8000)
  {
    // The left button is still pressed
  } 
  else if (m_bMouseDown)
  {
    // The left button is released and the Windows don't send the WM_LBUTTONUP
    // and WM_MOVE
    PostMessage(WM_LBUTTONUP, 0, MAKELPARAM(point.x, point.y));
    PostMessage(WM_MOVE, 0, MAKELPARAM(point.x, point.y));

    // Note: The code bellow is not needed, because in WM_LBUTTONUP message handler 
    // will set it to FALSE
    //m_bMouseDown = FALSE;
  }

  TRACKMOUSEEVENT tmet;
  tmet.cbSize = sizeof(TRACKMOUSEEVENT);
  tmet.dwFlags = TME_LEAVE;
  tmet.hwndTrack = m_hWnd;
  _TrackMouseEvent(&tmet);
  return;
}

BOOL CPlayerToolBar::OnTtnNeedText(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
{
  UNREFERENCED_PARAMETER(id);

  TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;
  UINT_PTR nID = pNMHDR->idFrom;
  BOOL bRet = FALSE;


  // idFrom is actually the HWND of the tool
  CString toolTip;
  switch(nID){
        case ID_SUBDELAYDEC:
          toolTip = ResStr(IDS_TOOLTIP_SUBTITLE_DELAY_REDUCE);
          break;
        case ID_SUBDELAYINC:
          toolTip = ResStr(IDS_TOOLTIP_SUBTITLE_DELAY_INCREASE);
          break;
        case ID_SUBTOOLBARBUTTON:
          toolTip = ResStr(IDS_TOOLTIP_TOOLBAR_SUBTITLE_BUTTON);
          break;
        case ID_VIEW_PLAYLIST:
          toolTip = ResStr(IDS_TOOLTIP_TOOLBAR_BUTTON_PLAYLIST);
          break;
        case ID_VIEW_OPTIONS:
          toolTip = ResStr(IDS_TOOLTIP_TOOLBAR_BUTTON_SETTING_PANEL);
          break;
        case ID_FILE_SAVE_IMAGE:
          toolTip = ResStr(IDS_TOOLTIP_TOOLBAR_BUTTON_IMAGE_CAPTURE);
          break;
        case ID_FILE_OPENQUICK:
          toolTip = ResStr(IDS_TOOLTIP_TOOLBAR_BUTTON_FILE_OPEN);
          break;		
        case ID_SUBSETFONTBOTH:
          toolTip = ResStr(IDS_TOOLTIP_TOOLBAR_BUTTON_SET_SUB_FONT);
          break;	
        case ID_SUBFONTUPBOTH:
          toolTip = ResStr(IDS_TOOLTIP_TOOLBAR_BUTTON_SUB_FONT_INCREASE);
          break;	
        case ID_SUBFONTDOWNBOTH:
          toolTip = ResStr(IDS_TOOLTIP_TOOLBAR_BUTTON_SUB_FONT_DECREASE);
          break;	
        case ID_MOVIESHARE:
          toolTip = ResStr(IDS_TOOLTIP_TOOLBAR_BUTTON_MOVIESHARE);
          break;	
        default:
          toolTip = ResStr(nID);
          break;
  }



  if(AfxGetMyApp()->IsVista()  ){
    if(!toolTip.IsEmpty()){
      pTTT->lpszText = toolTip.GetBuffer();
      pTTT->hinst = AfxGetResourceHandle();
      bRet = TRUE;
    }
  }else{
    if(!toolTip.IsEmpty()){

      int iN = toolTip.Find(_T("\n"));
      if(iN > 0){
        toolTip = toolTip.Left(iN).Trim();
      }
      CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());
      pFrame->m_tip.SetTips(toolTip, TRUE);
    }

  }




  *pResult = 0;

  return bRet;
}


void CPlayerToolBar::OnRButtonUp(UINT nFlags, CPoint point)
{
  // TODO: Add your message handler code here and/or call default
  CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());
  KillTimer(TIMER_FASTFORWORD);
  KillTimer(TIMER_CLOSETOOLBAR);
  ReleaseCapture();

  CRect rc;
  GetWindowRect(&rc);

  CPoint xpoint = point + rc.TopLeft() ;
  UINT ret = m_btnList.OnHitTest(xpoint,rc,false);
  if( m_btnList.HTRedrawRequired ){

    Invalidate();
  }
  m_nItemToTrackR = ret;
  __super::OnRButtonUp(nFlags, point);


  if(!pFrame)
    return;


  AppSettings& s = AfxGetAppSettings();
  if(ID_VOLUME_MUTE == m_nItemToTrackR ||  m_btnVolBG->m_rcHitest.PtInRect(xpoint)){

    pFrame->SetupSVPAudioMenu();
    pFrame->OnMenu( &pFrame->m_navaudio );

  }else if(!m_nItemToTrackR && s.bUserAeroUI()){

    if( s.m_lTransparentToolbarPosOffset == 0 || s.m_lTransparentToolbarPosSavedOffset == s.m_lTransparentToolbarPosOffset ){

      pFrame->OnMenu(pFrame->m_popup.GetSubMenu(0));
      return;
    }

    enum 
    {
      M_RESET_POS=1 , M_SAVE_POS=2
    };
    CMenu m;
    m.CreatePopupMenu();
    m.AppendMenu(MF_STRING|MF_ENABLED, M_SAVE_POS, ResStr(IDS_MENU_TIEM_TOOLBAR_SAVE_POS));
    m.AppendMenu(MF_STRING|MF_ENABLED, M_RESET_POS, ResStr(IDS_COLOR_CONTROL_BUTTON_RESET));

    ClientToScreen(&point);
    int nID = (int)m.TrackPopupMenu(TPM_LEFTBUTTON|TPM_RETURNCMD, point.x, point.y, this);

    switch(nID)
    {
    case M_RESET_POS:
      s.m_lTransparentToolbarPosOffset = 0;

      break;
    case M_SAVE_POS:

      break;
    default:
      return;
      break;
    }
    AfxGetMyApp()->WriteProfileInt(ResStr(IDS_R_SETTINGS), ResStr(IDS_RS_TRANSPARENTTOOLBARPOSOFFSET)+_T("2"), s.m_lTransparentToolbarPosOffset);		
    s.m_lTransparentToolbarPosSavedOffset = s.m_lTransparentToolbarPosOffset;
    //::SetForegroundWindow(pFrame->m_hWnd);
    pFrame->rePosOSD();
    //pFrame->RedrawNonClientArea();

  }



}

void CPlayerToolBar::OnRButtonDown(UINT nFlags, CPoint point)
{
  // TODO: Add your message handler code here and/or call default
  KillTimer(TIMER_FASTFORWORD);
  KillTimer(TIMER_CLOSETOOLBAR);

  CRect rc;
  GetWindowRect(&rc);

  CPoint xpoint = point + rc.TopLeft() ;
  UINT ret = m_btnList.OnHitTest(xpoint,rc,true);
  if( m_btnList.HTRedrawRequired ){
    if(ret){
      SetCapture();
    }
    Invalidate();
  }
  m_nItemToTrackR = ret;
  //CToolBar::OnRButtonDown(nFlags, point);
}

void CPlayerToolBar::OnLButtonDown(UINT nFlags, CPoint point)
{
  KillTimer(TIMER_FASTFORWORD);
  KillTimer(TIMER_CLOSETOOLBAR);

  CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());
  iBottonClicked = -1;
  m_bMouseDown = TRUE;
  CRect rc;
  GetWindowRect(&rc);

  // if on ad
  if (m_adctrl.GetVisible())
  {
    CRect rcAd = m_adctrl.GetRect();
    if (rcAd.PtInRect(point))
    {
      SetCursor(cursorHand);
    }
  }

  point += rc.TopLeft() ;
  UINT ret = m_btnList.OnHitTest(point,rc,true);
  if( m_btnList.HTRedrawRequired ){
    if(ret)
    {
      SetCapture();
    }
    Invalidate();
  }
  m_nItemToTrack = ret;


  if(m_nItemToTrack == ID_PLAY_BWD || m_nItemToTrack == ID_PLAY_FWD){
    //pFrame->PostMessage( WM_COMMAND, ID_PLAY_PAUSE);
    iBottonClicked = m_nItemToTrack;
    iFastFFWCount = 0;
    SetTimer(TIMER_FASTFORWORD, 200, NULL);
  }else if(m_nItemToTrack == ID_SUBDELAYDEC || m_nItemToTrack == ID_SUBDELAYINC){
    iBottonClicked = m_nItemToTrack;
    iFastFFWCount = 0;
    SetTimer(TIMER_FASTFORWORD, 200, NULL);
  }else if(!ret){

    if( m_btnVolBG->m_rcHitest.PtInRect(point) ){
      OnSetVolByMouse(point, true);
    }
  }
  return;
  //New UI End


}

void CPlayerToolBar::OnLButtonUp(UINT nFlags, CPoint point)
{
  CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());
  KillTimer(TIMER_FASTFORWORD);
  KillTimer(TIMER_CLOSETOOLBAR);
  ReleaseCapture();

  CRect rc;
  GetWindowRect(&rc);

  // if click on ads
  if (m_adctrl.GetVisible())
  {
    CRect rcAd = m_adctrl.GetRect();
    if (rcAd.PtInRect(point))
    {
      SetCursor(cursorHand);
      m_adctrl.OnAdClick();
    }
  }

  CPoint xpoint = point + rc.TopLeft() ;
  UINT ret = m_btnList.OnHitTest(xpoint,rc,false);
  if( m_btnList.HTRedrawRequired ){
    if(ret)
      pFrame->PostMessage( WM_COMMAND, ret);
    Invalidate();
  }
  m_nItemToTrack = ret;

  if(m_nItemToTrack == iBottonClicked ){
    if(iFastFFWCount == 0){
      int iMsg = 0;
      CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());
      // not increase or decrease play rate
      if(iBottonClicked == ID_PLAY_BWD){
        iMsg = ID_PLAY_SEEKBACKWARDSMALLC;
      }else if(iBottonClicked == ID_PLAY_FWD){
        iMsg = ID_PLAY_SEEKFORWARDSMALLC;
      }
      if(iMsg)
        pFrame->PostMessage( WM_COMMAND, iMsg);

    }
  }

  //	__super::OnLButtonUp(nFlags, point);
  m_bMouseDown = FALSE;
  if(pFrame->IsSomethingLoaded() && pFrame->m_fFullScreen){
    SetTimer(TIMER_CLOSETOOLBAR, 5000, NULL);
  }
  return;//New UI End

  /*
  KillTimer(TIMER_FASTFORWORD);
  for(int i = 0, j = GetToolBarCtrl().GetButtonCount(); i < j; i++)
  {
  CRect r;
  GetItemRect(i, r);
  if(r.PtInRect(point))
  {
  UINT iButtonID, iStyle ;
  int iImage ;
  GetButtonInfo(i,iButtonID, iStyle , iImage );
  if(iButtonID == iBottonClicked ){
  if(iFastFFWCount == 0){
  int iMsg = 0;
  CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());
  // not increase or decrease play rate
  if(iBottonClicked == ID_PLAY_BWD){
  iMsg = ID_PLAY_SEEKBACKWARDSMALL;
  }else if(iBottonClicked == ID_PLAY_FWD){
  iMsg = ID_PLAY_SEEKFORWARDSMALL;
  }
  if(iMsg)
  pFrame->PostMessage( WM_COMMAND, iMsg);
  // 					if( iBottonClicked == ID_PLAY_BWD || iBottonClicked == ID_PLAY_FWD) 
  // 						pFrame->PostMessage( WM_COMMAND, ID_PLAY_PLAY);
  }
  }
  break;
  }
  }
  iBottonClicked = -1;

  __super::OnLButtonUp(nFlags, point);
  return;*/

}
void CPlayerToolBar::OnTimer(UINT nIDEvent){
  switch(nIDEvent){
    case TIMER_ADFETCHING:
      {
        KillTimer(TIMER_ADFETCHING);
        std::wstring ad_uri = Strings::Format(L"https://www.shooter.cn/api/v2/prom.php?lang=%d", AfxGetAppSettings().iLanguage);
        m_adctrl.GetAds(ad_uri.c_str());
      }
      break;
    case TIMER_ADPLAY:
      {
        // If no ads exists, then didn't show ads, otherwise show ads
        if (m_adctrl.IsAdsEmpty())
          break;

        m_adctrl.AllowAnimate(true);
        Invalidate();
        break;
      }
    case TIMER_ADPLAYSWITCH:
      {
        m_adctrl.SetVisible(true);

        // If no ads exists, then don't show ads
        if (m_adctrl.IsAdsEmpty())
        {
          m_adctrl.SetVisible(false);
          break;
        }

        KillTimer(TIMER_ADPLAYSWITCH);
        // otherwise show ads
        // 查看广告是否显示完，如果还在显示则等待下一次2秒
        if (m_adctrl.IsCurAdShownDone())
        {
          m_adctrl.ShowNextAd();
          if ((time(NULL) - m_adctrl._mouseover_time) > 3)
            SetTimer(TIMER_ADPLAYSWITCH, 5000, NULL);
          else
            SetTimer(TIMER_ADPLAYSWITCH, 3000, NULL);
        }
        break;
      }
    case TIMER_STATERASER:
      KillTimer(TIMER_STATERASER);
      if(!m_timerqueryedstr.IsEmpty()){
        m_timerstr = m_timerqueryedstr;
        m_timerqueryedstr.Empty();
        Invalidate();
      }

      holdStatStr = FALSE;
      break;
    case TIMER_STOPFASTFORWORD:
      iFastFFWCount = 0;
      KillTimer(TIMER_STOPFASTFORWORD);
      {
        // 				CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());
        // 				pFrame->PostMessage( WM_COMMAND, ID_PLAY_PLAY);
      }
      break;
    case TIMER_FASTFORWORD:
      if(iBottonClicked < 0 ){
        KillTimer(TIMER_FASTFORWORD);
        break;
      }
      iFastFFWCount++;
      //fast forward or backword
      {
        CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());
        int iMsg;
        if(iBottonClicked == ID_PLAY_BWD){
          iMsg = ID_PLAY_SEEKBACKWARDSMALL;
        }else if(iBottonClicked == ID_PLAY_FWD){
          iMsg = ID_PLAY_SEEKFORWARDSMALL;
        }else{
          iMsg = iBottonClicked;
        }
        if(iFastFFWCount > 10 && ( iBottonClicked == ID_PLAY_BWD || iBottonClicked == ID_PLAY_FWD) ){

          int iStepPow = (int)(iFastFFWCount / 10) * 2;
          iStepPow = min(8, iStepPow);
          iMsg += iStepPow;
        }

        KillTimer(TIMER_CLOSETOOLBAR);


        pFrame->KillTimer(pFrame->TIMER_FULLSCREENMOUSEHIDER);
        //	SVP_LogMsg5(L" IsSomethingLoaded %d %d ", pFrame->IsSomethingLoaded(), __LINE__);
        if( pFrame->IsSomethingLoaded()){
          AppSettings& s = AfxGetAppSettings();
          if(s.bUserAeroUI())
            pFrame->SetTimer(pFrame->TIMER_FULLSCREENMOUSEHIDER, 5000, NULL);
          else
            pFrame->SetTimer(pFrame->TIMER_FULLSCREENMOUSEHIDER, 3000, NULL);

          if( pFrame->m_fFullScreen){
            SetTimer(TIMER_CLOSETOOLBAR, 5000, NULL);
          }
        }


        pFrame->PostMessage( WM_COMMAND, iMsg);
      }
      break;
    case TIMER_CLOSETOOLBAR:
      {
        m_bMouseDown = FALSE;
        KillTimer(TIMER_CLOSETOOLBAR);
        CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());
        if(pFrame->m_fFullScreen && pFrame->IsSomethingLoaded()){
          AppSettings& s = AfxGetAppSettings();
          if(s.bUserAeroUI()){
            pFrame->KillTimer(pFrame->TIMER_FULLSCREENMOUSEHIDER);
            //SVP_LogMsg5(L" IsSomethingLoaded %d %d ", pFrame->IsSomethingLoaded(), __LINE__);
            if(pFrame->IsSomethingLoaded())
              pFrame->SetTimer(pFrame->TIMER_FULLSCREENMOUSEHIDER,3000, NULL);
          }else{
            pFrame->m_notshowtoolbarforawhile = 3;
            pFrame->ShowControls(0, FALSE);
          }
        }

      }
      break;
  }

  __super::OnTimer(nIDEvent);
}
BOOL CPlayerToolBar::OnEraseBkgnd(CDC* pDC)
{
  // TODO: Add your message handler code here and/or call default

  return true;
  //return CToolBar::OnEraseBkgnd(pDC);
}

BOOL CPlayerToolBar::PreTranslateMessage(MSG* pMsg)
{
  // TODO: Add your specialized code here and/or call the base class

  if( pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MYMOUSELAST && WM_MOUSEMOVE != pMsg->message ){
    CMainFrame* pFrame = (CMainFrame*) AfxGetMainWnd();
    if(pFrame){
      pFrame->KillTimer(pFrame->TIMER_FULLSCREENMOUSEHIDER);
      // SVP_LogMsg5(L" IsSomethingLoaded %d %d ", pFrame->IsSomethingLoaded(), __LINE__);
      KillTimer( TIMER_CLOSETOOLBAR);
      if( pFrame->IsSomethingLoaded()){
        AppSettings& s = AfxGetAppSettings();
        if(s.bUserAeroUI())
          pFrame->SetTimer(pFrame->TIMER_FULLSCREENMOUSEHIDER, 5000, NULL);
        else
          pFrame->SetTimer(pFrame->TIMER_FULLSCREENMOUSEHIDER, 3000, NULL);

        if( pFrame->m_fFullScreen){
          SetTimer(TIMER_CLOSETOOLBAR, 5000, NULL);
        }
      }
    }
  }
  return CToolBar::PreTranslateMessage(pMsg);
}

void CPlayerToolBar::DefaultButtonManage()
{
  
  CONFIGBUTTON(PLAY,BTN_PLAY.BMP,ALIGN_TOPLEFT,CRect(-50 , -50, 3,3),0,ID_PLAY_PLAY,FALSE,0,0,0,CRect(0,0,0,0))

  CONFIGBUTTON(PAUSE,BTN_PAUSE.BMP,ALIGN_TOPLEFT, CRect(-50 , -50, 3,3) , 0,ID_PLAY_PAUSE, FALSE,0,0,0,CRect(0,0,0,0))

  CONFIGBUTTON(STOP,BTN_STOP.BMP,ALIGN_TOPLEFT, CRect(-50,-50,3,3),0,ID_PLAY_MANUAL_STOP,FALSE,660,ALIGN_RIGHT,PAUSE,DEFAULT_MARGIN_TOBUTTON)
  CONFIGADDALIGN(STOP,ALIGN_RIGHT, PLAY , DEFAULT_MARGIN_TOBUTTON)

  CONFIGBUTTON(STEP,BTN_STEP.BMP,ALIGN_TOPLEFT,CRect(-50,-50,3,3), 0,ID_PLAY_FRAMESTEP,FALSE,660,ALIGN_LEFT,PAUSE,DEFAULT_MARGIN_TOBUTTON)
  CONFIGADDALIGN(STEP,ALIGN_LEFT,PLAY, DEFAULT_MARGIN_TOBUTTON)

  CONFIGBUTTON(FASTFORWORD,FAST_FORWORD.BMP,ALIGN_TOPLEFT,CRect(-50,-50,3,3),0,ID_PLAY_FWD,FALSE,440,ALIGN_LEFT,PAUSE,DEFAULT_MARGIN_TOBUTTON)
  CONFIGADDALIGN(FASTFORWORD,ALIGN_LEFT,PLAY,DEFAULT_MARGIN_TOBUTTON)
  CONFIGADDALIGN(FASTFORWORD,ALIGN_LEFT,STEP, DEFAULT_MARGIN_TOBUTTON)

  CONFIGBUTTON(FASTBACKWORD,FAST_BACKWORD.BMP,ALIGN_TOPLEFT,CRect(-50,-50,3,3),0,ID_PLAY_BWD,FALSE,440,ALIGN_RIGHT,PAUSE,DEFAULT_MARGIN_TOBUTTON)
  CONFIGADDALIGN(FASTBACKWORD,ALIGN_RIGHT,PLAY,DEFAULT_MARGIN_TOBUTTON)
  CONFIGADDALIGN(FASTBACKWORD,ALIGN_RIGHT,STOP,DEFAULT_MARGIN_TOBUTTON)
  
  CONFIGBUTTON(PREV,BTN_PREV.BMP,ALIGN_TOPLEFT,CRect(-50,-50,3,3),0,ID_NAVIGATE_SKIPBACK,FALSE,440,ALIGN_RIGHT,FASTBACKWORD,DEFAULT_MARGIN_TOBUTTON)
  CONFIGADDALIGN(PREV,ALIGN_RIGHT,PLAY,DEFAULT_MARGIN_TOBUTTON)
  CONFIGADDALIGN(PREV,ALIGN_RIGHT,PAUSE,DEFAULT_MARGIN_TOBUTTON)

  CONFIGBUTTON(NEXT,BTN_NEXT.BMP,ALIGN_TOPLEFT,CRect(-50,-50,3,3),0,ID_NAVIGATE_SKIPFORWARD,FALSE,440,ALIGN_LEFT,FASTFORWORD,DEFAULT_MARGIN_TOBUTTON)
  CONFIGADDALIGN(NEXT,ALIGN_LEFT,PLAY,DEFAULT_MARGIN_TOBUTTON)
  CONFIGADDALIGN(NEXT,ALIGN_LEFT,PAUSE,DEFAULT_MARGIN_TOBUTTON)

  CONFIGBUTTON(LOGO,SPLAYER.BMP,ALIGN_TOPLEFT,CRect(15,-50,3,3),TRUE,0,FALSE,0,0,0,CRect(0,0,0,0))

  //CONFIGBUTTON(SHARE,BTN_SHARE.BMP,ALIGN_TOPLEFT,CRect(15,-50,3,3),FALSE,ID_MOVIESHARE,FALSE,0,0,0,CRect(0,0,0,0))

/*
  CSUIButton* btnSubFont =   new CSUIButton(L"BTN_FONT.BMP" , ALIGN_TOPLEFT, CRect(-35 , -50, 3,3)  , 0, ID_SUBSETFONTBOTH, TRUE, ALIGN_RIGHT, btnPrev , CRect(20 , 10 , 20, 10) );
  btnSubFont->addAlignRelButton(ALIGN_RIGHT, btnFFBack   , CRect(20 , 10 , 20, 10) );
  m_btnList.AddTail( btnSubFont );

  CSUIButton* btnSubFontPlus =   new CSUIButton(L"BTN_FONT_PLUS.BMP" , ALIGN_TOPLEFT, CRect(-10 , -40, 3,3)  , 0, ID_SUBFONTUPBOTH , TRUE, ALIGN_LEFT, btnSubFont , CRect(3 , 10 , 3, 10) );
  m_btnList.AddTail( btnSubFontPlus );

  CSUIButton* btnSubFontMinus =   new CSUIButton(L"BTN_FONT_MINUS.BMP" , ALIGN_TOPLEFT, CRect(-10 , -55, 3,3)  , 0, ID_SUBFONTDOWNBOTH , TRUE, ALIGN_LEFT, btnSubFont , CRect(3 , 10 , 3, 10) );
  btnSubFontMinus->addAlignRelButton(ALIGN_TOP, btnSubFontPlus ,  CRect(3 , 0 , 3, 0) );
  m_btnList.AddTail( btnSubFontMinus );*/

  CONFIGBUTTON(SUBSWITCH,BTN_SUB.BMP,ALIGN_TOPLEFT,CRect(-23,-50,3,3),0,ID_SUBTOOLBARBUTTON,FALSE,560,ALIGN_RIGHT,FASTBACKWORD,CRect(20,10,22,10))
  CONFIGADDALIGN(SUBSWITCH,ALIGN_LEFT,LOGO,CRect(15,10,10,10))
  CONFIGADDALIGN(SUBSWITCH,ALIGN_RIGHT,PREV,CRect(20,10,22,10))

  CONFIGBUTTON(SUBREDUCE,BTN_SUB_DELAY_REDUCE.BMP,ALIGN_TOPLEFT,CRect(-42,-50,3,3),0,ID_SUBDELAYDEC,FALSE,560,ALIGN_RIGHT,SUBSWITCH,CRect(2,3,2,3))
  
  CONFIGBUTTON(SUBINCREASE,BTN_SUB_DELAY_INCREASE.BMP,ALIGN_TOPLEFT,CRect(-10,-50,3,3),0,ID_SUBDELAYINC,FALSE,560,ALIGN_LEFT,SUBSWITCH,CRect(2,3,2,3))

  CONFIGBUTTON(VOLUMEBG,VOLUME_BG.BMP,ALIGN_TOPRIGHT,CRect(3,-50,15,3),TRUE,0,FALSE,0,0,0,CRect(0,0,0,0))

  CONFIGBUTTON(MUTED,MUTED.BMP,ALIGN_TOPRIGHT,CRect(3,-50,105,3),FALSE,ID_VOLUME_MUTE,FALSE,440,ALIGN_RIGHT,VOLUMEBG,CRect(3,3,3,3))

  CONFIGBUTTON(VOLUME,VOLUME.BMP,ALIGN_TOPRIGHT,CRect(3,-50,105,3),FALSE,ID_VOLUME_MUTE,FALSE,440,ALIGN_RIGHT,VOLUMEBG,CRect(3,3,3,3))

  CONFIGBUTTON(SETTING,BTN_SETTING.BMP,ALIGN_TOPRIGHT,CRect(-70,-50,105,3),FALSE,ID_VIEW_OPTIONS,FALSE,600,ALIGN_RIGHT,MUTED,CRect(3,10,3,10))
  CONFIGADDALIGN(SETTING,ALIGN_RIGHT,VOLUME,CRect(3,10,3,10))

  CONFIGBUTTON(PLAYLIST,BTN_PLAYLIST.BMP,ALIGN_TOPRIGHT,CRect(3,-50,33,3),FALSE,ID_VIEW_PLAYLIST,FALSE,540,ALIGN_RIGHT,SETTING,CRect(3,10,3,10))
  CONFIGADDALIGN(PLAYLIST,ALIGN_RIGHT,VOLUME,CRect(3,10,3,10))
  CONFIGADDALIGN(PLAYLIST,ALIGN_RIGHT,MUTED,CRect(3,10,3,10))
  CONFIGADDALIGN(PLAYLIST,ALIGN_RIGHT,VOLUMEBG,CRect(3,10,3,10))
/*
  CSUIButton* btnCapture = new CSUIButton(L"BTN_CAPTURE.BMP" , ALIGN_TOPRIGHT, CRect(3 , -50, 105,3)  , FALSE, ID_FILE_SAVE_IMAGE_AUTO, TRUE , ALIGN_RIGHT, btnPlayList , CRect(3 , 10 , 3, 10)) ;
  m_btnList.AddTail( btnCapture );*/

  CONFIGBUTTON(OPENFILE,BTN_OPENFILE_SMALL.BMP,ALIGN_TOPRIGHT,CRect(3,-50,105,3),FALSE,ID_FILE_OPENQUICK,FALSE,660,ALIGN_RIGHT,PLAYLIST,CRect(3,10,3,10))

  CONFIGBUTTON(VOLUMETM,VOLUME_TM.BMP,ALIGN_TOPRIGHT,CRect(3,-50,65,3),FALSE,ID_VOLUME_THUMB,FALSE,0,0,0,CRect(0,0,0,0))

  

  CONFIGBUTTON(SHARE,BTN_SHARE.BMP,ALIGN_TOPLEFT,CRect(10,-50,3,3),FALSE,ID_MOVIESHARE,FALSE,0,0,0,CRect(0,0,0,0))
  //CONFIGADDALIGN(SHARE, ALIGN_RIGHT, FASTBACKWORD, CRect(1,1,1,1))
  //CONFIGADDALIGN(SHARE, ALIGN_RIGHT, SUBINCREASE, CRect(1,1,1,1))

/*  CONFIGBUTTON(PLAYTIME,NOBMP,ALIGN_TOPLEFT,CRect(3,-50,105,3),TRUE,0,FALSE,0,ALIGN_LEFT,SHARE,CRect(1,1,1,1))*/
  
}

void CPlayerToolBar::PointVolumeBtn()
{
  m_btnVolBG = m_btnList.GetButton(L"VOLUMEBG");
  m_btnVolTm = m_btnList.GetButton(L"VOLUMETM");
  btnLogo = m_btnList.GetButton(L"LOGO");
/*  m_btnplaytime = m_btnList.GetButton(L"PLAYTIME");*/
}

void CPlayerToolBar::OnMouseLeave()
{
  CRect rc;
  GetWindowRect(&rc);
  m_btnList.OnHitTest(CPoint(0,0), rc, -1);
  Invalidate();
}

void CPlayerToolBar::ResizeToolbarHeight()
{
  m_nHeight = max(45, m_btnList.GetMaxHeight());
  if (m_nHeight > 45)
    m_nHeight += 4;
}
/*
BottomToolBarButton.dat

VOLUMEBG:BMP,VOLUME_BG.BMP;ALIGN1,ALIGN_TOPRIGHT;CRECT1,3,-50,15,3;NOTBUTTON,TRUE;ID,0;HIDE,FALSE;
VOLUMETM:BMP,VOLUME_TM.BMP;ALIGN1,ALIGN_TOPRIGHT;CRECT1,3,-50,65,3;NOTBUTTON,FALSE;ID,ID_VOLUME_THUMB;HIDE,FALSE;
PLAY:BMP,BTN_PLAY.BMP;ALIGN1,ALIGN_TOPLEFT;CRECT1,-50,-50,3,3;NOTBUTTON,FALSE;ID,ID_PLAY_PLAY;HIDE,FALSE;
PAUSE:BMP,BTN_PAUSE.BMP;ALIGN1,ALIGN_TOPLEFT;CRECT1,-50,-50,3,3;NOTBUTTON,FALSE;ID,ID_PLAY_PAUSE;HIDE,TRUE;
STOP:BMP,BTN_STOP.BMP;ALIGN1,ALIGN_TOPLEFT;CRECT1,-50,-50,3,3;NOTBUTTON,FALSE;ID,ID_PLAY_MANUAL_STOP;HIDE,FALSE;HIDEWIDTH,660;ALIGN2,ALIGN_RIGHT;BUTTON,PAUSE;CRECT2,1,1,1,1;ADDALIGN,ALIGN2,ALIGN_RIGHT;BUTTON,PLAY;CRECT2,1,1,1,1;
STEP:BMP,BTN_STEP.BMP;ALIGN1,ALIGN_TOPLEFT;CRECT1,-50,-50,3,3;NOTBUTTON,FALSE;ID,ID_PLAY_FRAMESTEP;HIDE,FALSE;HIDEWIDTH,660;ALIGN2,ALIGN_LEFT;BUTTON,PAUSE;CRECT2,1,1,1,1;ADDALIGN,ALIGN2,ALIGN_LEFT;BUTTON,PLAY;CRECT2,1,1,1,1;
FASTFORWORD:BMP,FAST_FORWORD.BMP;ALIGN1,ALIGN_TOPLEFT;CRECT1,-50,-50,3,3;NOTBUTTON,FALSE;ID,ID_PLAY_FWD;HIDE,FALSE;ALIGN2,ALIGN_LEFT;BUTTON,PAUSE;CRECT2,1,1,1,1;ADDALIGN,ALIGN2,ALIGN_LEFT;BUTTON,PLAY;CRECT2,1,1,1,1;ALIGN2,ALIGN_LEFT;BUTTON,STEP;CRECT2,1,1,1,1;
FASTBACKWORD:BMP,FAST_BACKWORD.BMP;ALIGN1,ALIGN_TOPLEFT;CRECT1,-50,-50,3,3;NOTBUTTON,FALSE;ID,ID_PLAY_BWD;HIDE,FALSE;ALIGN2,ALIGN_RIGHT;BUTTON,PAUSE;CRECT2,1,1,1,1;ADDALIGN,ALIGN2,ALIGN_RIGHT;BUTTON,PLAY;CRECT2,1,1,1,1;ALIGN2,ALIGN_RIGHT;BUTTON,STOP;CRECT2,1,1,1,1;
PREV:BMP,BTN_PREV.BMP;ALIGN1,ALIGN_TOPLEFT;CRECT1,-50,-50,3,3;NOTBUTTON,FALSE;ID,ID_NAVIGATE_SKIPBACK;HIDE,FALSE;HIDEWIDTH,440;ALIGN2,ALIGN_RIGHT;BUTTON,FASTBACKWORD;CRECT2,1,1,1,1;ADDALIGN,ALIGN2,ALIGN_RIGHT;BUTTON,PLAY;CRECT2,1,1,1,1;ALIGN2,ALIGN_RIGHT;BUTTON,PAUSE;CRECT2,1,1,1,1;
NEXT:BMP,BTN_NEXT.BMP;ALIGN1,ALIGN_TOPLEFT;CRECT1,-50,-50,3,3;NOTBUTTON,FALSE;ID,ID_NAVIGATE_SKIPFORWARD;HIDE,FALSE;HIDEWIDTH,440;ALIGN2,ALIGN_LEFT;BUTTON,FASTFORWORD;CRECT2,1,1,1,1;ADDALIGN,ALIGN2,ALIGN_LEFT;BUTTON,PLAY;CRECT2,1,1,1,1;ALIGN2,ALIGN_LEFT;BUTTON,PAUSE;CRECT2,1,1,1,1;
LOGO:BMP,SPLAYER.BMP;ALIGN1,ALIGN_TOPLEFT;CRECT1,14,-50,3,3;NOTBUTTON,TRUE;ID,0;HIDE,FALSE;
VOLUME:BMP,VOLUME.BMP;ALIGN1,ALIGN_TOPRIGHT;CRECT1,3,-50,105,3;NOTBUTTON,FALSE;ID,ID_VOLUME_MUTE;HIDE,IsMuted;HIDEWIDTH,440;ALIGN2,ALIGN_RIGHT;BUTTON,VOLUMEBG;CRECT2,3,3,3,3;
MUTED:BMP,MUTED.BMP;ALIGN1,ALIGN_TOPRIGHT;CRECT1,3,-50,105,3;NOTBUTTON,FALSE;ID,ID_VOLUME_MUTE;HIDE,!IsMuted;HIDEWIDTH,440;ALIGN2,ALIGN_RIGHT;BUTTON,VOLUMEBG;CRECT2,3,3,3,3;
SETTING:BMP,BTN_SETTING.BMP;ALIGN1,ALIGN_TOPRIGHT;CRECT1,-70,-50,105,3;NOTBUTTON,FALSE;ID,ID_VIEW_OPTIONS;HIDE,TRUE;HIDEWIDTH,600;ALIGN2,ALIGN_RIGHT;BUTTON,MUTED;CRECT2,3,10,3,10;ADDALIGN,ALIGN2,ALIGN_RIGHT;BUTTON,VOLUME;CRECT2,3,10,3,10;
PLAYLIST:BMP,BTN_PLAYLIST.BMP;ALIGN1,ALIGN_TOPRIGHT;CRECT1,3,-50,33,3;NOTBUTTON,FALSE;ID,ID_VIEW_PLAYLIST;HIDE,FALSE;HIDEWIDTH,540;ALIGN2,ALIGN_RIGHT;BUTTON,SETTING;CRECT2,3,10,3,10;ADDALIGN,ALIGN2,ALIGN_RIGHT;BUTTON,VOLUME;CRECT2,3,10,3,10;ALIGN2,ALIGN_RIGHT;BUTTON,MUTED;CRECT2,3,10,3,10;ALIGN2,ALIGN_RIGHT;BUTTON,VOLUMEBG;CRECT2,3,10,3,10;
CAPTURE:BMP,BTN_CAPTURE.BMP;ALIGN1,ALIGN_TOPRIGHT;CRECT1,3,-50,105,3;NOTBUTTON,FALSE;ID,ID_FILE_SAVE_IMAGE_AUTO;HIDE,TRUE;HIDEWIDTH,MAXINT;ALIGN2,ALIGN_RIGHT;BUTTON,PLAYLIST;CRECT2,3,10,3,10;
OPENFILE:BMP,BTN_OPENFILE_SMALL.BMP;ALIGN1,ALIGN_TOPRIGHT;CRECT1,3,-50,105,3;NOTBUTTON,FALSE;ID,ID_FILE_OPENQUICK;HIDE,TRUE;HIDEWIDTH,660;ALIGN2,ALIGN_RIGHT;BUTTON,CAPTURE;CRECT2,3,10,3,10;ADDALIGN,ALIGN2,ALIGN_RIGHT;BUTTON,PLAYLIST;CRECT2,3,10,3,10;
SUBSWITCH:BMP,BTN_SUB.BMP;ALIGN1,ALIGN_TOPLEFT;CRECT1,-23,-50,3,3;NOTBUTTON,FALSE;ID,ID_SUBTOOLBARBUTTON;HIDE,TRUE;HIDEWIDTH,540;ALIGN2,ALIGN_RIGHT;BUTTON,FASTBACKWORD;CRECT2,20,10,22,10;ADDALIGN,ALIGN2,ALIGN_LEFT;BUTTON,LOGO;CRECT2,15,10,10,10;ALIGN2,ALIGN_RIGHT;BUTTON,PREV;CRECT2,20,10,22,10;
SUBDELAYREDUCE:BMP,BTN_SUB_DELAY_REDUCE.BMP;ALIGN1,ALIGN_TOPLEFT;CRECT1,-42,-50,3,3;NOTBUTTON,FALSE;ID,ID_SUBDELAYDEC;HIDE,TRUE;HIDEWIDTH,540;ALIGN2,ALIGN_RIGHT;BUTTON,SUBSWITCH;CRECT2,2,3,2,3;
SUBDELAYINCREASE:BMP,BTN_SUB_DELAY_INCREASE.BMP;ALIGN1,ALIGN_TOPLEFT;CRECT1,-10,-50,3,3;NOTBUTTON,FALSE;ID,ID_SUBDELAYINC;HIDE,TRUE;HIDEWIDTH,540;ALIGN2,ALIGN_LEFT;BUTTON,SUBSWITCH;CRECT2,2,3,2,3;
PLAYTIME:ALIGN1,ALIGN_TOPLEFT;CRECT1,10,15,3,3;
*/

