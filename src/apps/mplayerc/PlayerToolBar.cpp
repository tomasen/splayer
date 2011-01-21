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
#include "../../svplib/svplib.h"

typedef HRESULT (__stdcall * SetWindowThemeFunct)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);

#define BMP 11
#define ALIGN1 12
#define CRECT1 13
#define NOTBUTTON 14
#define ID 15
#define HIDE 16
#define HIDEWIDTH 17
#define ALIGN2 18
#define BUTTON 19
#define CRECT2 110
#define ADDALIGN 111

ToolBarButton::ToolBarButton()
{
  mybutton = NULL;
  buttonname = L"";
  bmpstr = L"";
  pbuttonname = L"";  
  align1 = 0;
  rect1 = CRect(0,0,0,0);
  bnotbutton = FALSE;
  id = 0;
  bhide = FALSE;
  width = 0;
  align2 = 0;
  rect2 = CRect(0,0,0,0);
  baddalign = FALSE;
  pbutton = NULL;
}

ToolBarButton::ToolBarButton(std::wstring btname, int agn1, CRect rc1)
{
  mybutton = 0;
  buttonname = btname;
  align1 = agn1;
  rect1 = rc1;
  bnotbutton = 0;
  id = 0;
  bhide = 0;
  width = 0;
  align2 = 0;
  rect2 = CRect(0,0,0,0);
  baddalign = FALSE;
  pbutton = 0;
  pbuttonname = L"";
}

ToolBarButton::ToolBarButton(std::wstring btname, std::wstring bmp, int agn1, CRect rc1, BOOL bnbutton,
                 int idi, BOOL bhd, int wdt)
{
  mybutton = 0;
  buttonname = btname;
  bmpstr = bmp;
  align1 = agn1;
  rect1 = rc1;
  bnotbutton = bnbutton;
  id = idi;
  bhide = bhd;
  width = wdt;
  align2 = 0;
  rect2 = CRect(0,0,0,0);
  baddalign = FALSE;
  pbutton = 0;
  pbuttonname = L"";
}

ToolBarButton::ToolBarButton(std::wstring btname, std::wstring bmp,int agn1, CRect rc1, 
                 BOOL bnbutton, int idi, BOOL bhd, int wdt, int agn2, 
                 std::wstring pbtname,CRect rc2, BOOL badd)
{
  buttonname = btname;
  bmpstr = bmp;
  pbuttonname = pbtname;
  align1 = agn1;
  rect1 = rc1;
  bnotbutton = bnbutton;
  id =idi;
  bhide = bhd;
  width = wdt;
  align2 = agn2;
  rect2 = rc2;
  baddalign = badd;
  pbutton = 0;
  mybutton = 0;
}

ToolBarButton::~ToolBarButton(){}

AddButton::AddButton()
{
  align = 0;
  pbuttonname = L"";
  pbutton = 0;
  rect = CRect(0,0,0,0);
}

AddButton::AddButton(int agn, std::wstring pn, CRect rc)
{
  align = agn;
  pbuttonname = pn;
  rect = rc;
  pbutton = 0;
}

AddButton::~AddButton(){}

// CPlayerToolBar

IMPLEMENT_DYNAMIC(CPlayerToolBar, CToolBar)
CPlayerToolBar::CPlayerToolBar() :
m_hovering(0),
holdStatStr(0),
iButtonWidth (30),
m_pbtnList(&m_btnList),
m_bMouseDown(FALSE),
m_nHeight(90)
{
	 
}

CPlayerToolBar::~CPlayerToolBar()
{
  for (std::vector<ToolBarButton*>::iterator ite = m_struct_vec.begin();
       ite != m_struct_vec.end(); ++ite)
       delete *ite;
}
#define ID_VOLUME_THUMB 126356

BOOL CPlayerToolBar::Create(CWnd* pParentWnd)
{
#define ADDCLASSIFICATIONNAME(x) m_classificationname_map[L#x] = x

  ADDCLASSIFICATIONNAME(BMP);
  ADDCLASSIFICATIONNAME(ALIGN1);
  ADDCLASSIFICATIONNAME(CRECT1);
  ADDCLASSIFICATIONNAME(NOTBUTTON);
  ADDCLASSIFICATIONNAME(ID);
  ADDCLASSIFICATIONNAME(HIDE);
  ADDCLASSIFICATIONNAME(HIDEWIDTH);
  ADDCLASSIFICATIONNAME(ALIGN2);
  ADDCLASSIFICATIONNAME(BUTTON);
  ADDCLASSIFICATIONNAME(CRECT2);
  ADDCLASSIFICATIONNAME(ADDALIGN);

#define ADDALIGN1(x) m_align1_map[L#x] = x

  ADDALIGN1(ALIGN_TOPLEFT);
  ADDALIGN1(ALIGN_TOPRIGHT);
  ADDALIGN1(ALIGN_BOTTOMLEFT);
  ADDALIGN1(ALIGN_BOTTOMRIGHT);

#define ADDALIGN2(x) m_align2_map[L#x] = x

  ADDALIGN2(ALIGN_TOP);
  ADDALIGN2(ALIGN_LEFT);
  ADDALIGN2(ALIGN_RIGHT);
  ADDALIGN2(ALIGN_BOTTOM);

#define ADDID(x) m_id_map[L#x] = x

  ADDID(ID_PLAY_PLAY);
  ADDID(ID_PLAY_PAUSE);
  ADDID(ID_PLAY_MANUAL_STOP);
  ADDID(ID_PLAY_FRAMESTEP);
  ADDID(ID_PLAY_FWD);
  ADDID(ID_PLAY_BWD);
  ADDID(ID_NAVIGATE_SKIPBACK);
  ADDID(ID_NAVIGATE_SKIPFORWARD);
  ADDID(ID_SUBTOOLBARBUTTON);
  ADDID(ID_SUBDELAYDEC);
  ADDID(ID_SUBDELAYINC);
  ADDID(ID_VOLUME_MUTE);
  ADDID(ID_VIEW_OPTIONS);
  ADDID(ID_VIEW_PLAYLIST);
  ADDID(ID_FILE_SAVE_IMAGE_AUTO);
  ADDID(ID_FILE_OPENQUICK);
  ADDID(ID_VOLUME_THUMB);

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

  breadfromfile = ReadFromFile();
  if (breadfromfile)
  {
    LineStringToVector();
    SetButton();
  }
  else
    DefaultInitializeButton();

	cursorHand = ::LoadCursor(NULL, IDC_HAND);

	GetSystemFontWithScale(&m_statft, 14.0);

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

void CPlayerToolBar::ArrangeControls()
{
  CRect rc;
  GetWindowRect(&rc);
  long iWidth = rc.Width();
  CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());
  double skinsRate = (double)pFrame->m_lMinFrameWidth / 310;
  BOOL isaudio = (pFrame && pFrame->IsSomethingLoaded() && pFrame->m_fAudioOnly)?TRUE:FALSE;

  if (breadfromfile)
  {
    for (std::vector<ToolBarButton*>::iterator ite = m_struct_vec.begin(); ite != m_struct_vec.end();
         ++ite)
    {
      if (iWidth > ((*ite)->width * skinsRate * m_nLogDPIY / 96))
        (*ite)->bhide = FALSE;
      else
        (*ite)->bhide = TRUE;
      
      ShowButton(*ite,isaudio);
    }

    m_btnList.OnSize(rc);
    return;
  }

  BOOL hideT1 = TRUE;
  BOOL hideT15 = TRUE;
  BOOL hideT2 = TRUE;
  BOOL hideT3 = TRUE;
  BOOL hideT4 = TRUE;
  if( iWidth > (440 * skinsRate * m_nLogDPIY / 96) ){
    hideT1 = false;
  }
  if( iWidth > (480 * skinsRate * m_nLogDPIY / 96) ){
    hideT15 = false;
  }
  if( iWidth > (540 * skinsRate * m_nLogDPIY / 96) ){
    hideT2 = false;
  }
  if( iWidth > (600 * skinsRate * m_nLogDPIY / 96) ){
    hideT3 = false;
  }
  if( iWidth > (660 * skinsRate * m_nLogDPIY / 96) ){
    hideT4 = false;
  }
  if(IsMuted()){
    m_btnList.SetHideStat(L"VOLUME.BMP", TRUE|hideT1);
    m_btnList.SetHideStat(L"MUTED.BMP", FALSE|hideT1);
  }else{
    m_btnList.SetHideStat(L"VOLUME.BMP", FALSE|hideT1);
    m_btnList.SetHideStat(L"MUTED.BMP", TRUE|hideT1);
  }

  if(pFrame && pFrame->IsSomethingLoaded() && pFrame->m_fAudioOnly){
    m_btnList.SetHideStat(ID_PLAY_FWD , hideT1);
    m_btnList.SetHideStat(ID_PLAY_BWD , hideT1);
    m_btnList.SetHideStat(ID_NAVIGATE_SKIPBACK , 0);
    m_btnList.SetHideStat(ID_NAVIGATE_SKIPFORWARD , 0);

    m_btnList.SetHideStat(ID_VIEW_PLAYLIST , hideT2);
  }else{
    m_btnList.SetHideStat(ID_PLAY_FWD , 0);
    m_btnList.SetHideStat(ID_PLAY_BWD , 0);
    m_btnList.SetHideStat(ID_NAVIGATE_SKIPBACK , hideT1);
    m_btnList.SetHideStat(ID_NAVIGATE_SKIPFORWARD , hideT1);

    m_btnList.SetHideStat(ID_VIEW_PLAYLIST , hideT2);
    
  }



  m_btnList.SetHideStat(ID_SUBTOOLBARBUTTON , hideT2);
  m_btnList.SetHideStat(ID_SUBDELAYDEC , hideT2);
  m_btnList.SetHideStat(ID_SUBDELAYINC , hideT2);

  m_btnList.SetHideStat(ID_FILE_SAVE_IMAGE , hideT4);
  m_btnList.SetHideStat(ID_VIEW_OPTIONS , hideT3);


  m_btnList.SetHideStat(ID_SUBSETFONTBOTH , hideT4);
  m_btnList.SetHideStat(ID_SUBFONTUPBOTH , hideT4);
  m_btnList.SetHideStat(ID_SUBFONTDOWNBOTH , hideT4);

  m_btnList.SetHideStat(ID_FILE_OPENQUICK , hideT4);

  m_btnList.SetHideStat(ID_PLAY_FRAMESTEP , hideT4);
  m_btnList.SetHideStat(ID_PLAY_MANUAL_STOP , hideT4);



  m_btnList.OnSize(rc);

	if(!::IsWindow(m_volctrl.m_hWnd)) return;

	/*
	CRect r;
		GetClientRect(&r);
	
		CRect br = GetBorders();
	
		CRect r10;
		GetItemRect(18, &r10);
	
		CRect vr;
		m_volctrl.GetClientRect(&vr);
		CRect vr2(r.right+br.right-iButtonWidth*2, r.top-1, r.right+br.right+6, r.bottom);
		m_volctrl.MoveWindow(vr2);
	
		UINT nID;
		UINT nStyle;
		int iImage;
		GetButtonInfo(20, nID, nStyle, iImage);
		SetButtonInfo(19, GetItemID(19), TBBS_SEPARATOR, vr2.left - iImage - r10.right - 19);*/
	
}

void CPlayerToolBar::SetMute(bool fMute)
{
	/*
CToolBarCtrl& tb = GetToolBarCtrl();
	TBBUTTONINFO bi;
	bi.cbSize = sizeof(bi);
	bi.dwMask = TBIF_IMAGE;
	bi.iImage = fMute?21:20;
	tb.SetButtonInfo(ID_VOLUME_MUTE, &bi);
*/
	
	AfxGetAppSettings().fMute = fMute;
   
}

bool CPlayerToolBar::IsMuted()
{
/*
	CToolBarCtrl& tb = GetToolBarCtrl();
	TBBUTTONINFO bi;
	bi.cbSize = sizeof(bi);
	bi.dwMask = TBIF_IMAGE;
	tb.GetButtonInfo(ID_VOLUME_MUTE, &bi);
	return(bi.iImage==21);
*/

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

	CRect rc;
	GetWindowRect(&rc);

	if( paintRect == m_btnVolBG->m_rcHitest){
		m_btnVolBG->OnPaint( &hdc, rc);
		m_btnVolTm->OnPaint( &hdc, rc);
		//SVP_LogMsg5(_T("Just Paint Vol"));
		return;
	}
	//SVP_LogMsg5(_T("Just Paint All %d %d ") , paintRect.left , paintRect.top);
	CRect rcBottomSqu = rcClient;
	rcBottomSqu.top = rcBottomSqu.bottom - 10;
	//hdc.FillSolidRect(rcBottomSqu, NEWUI_COLOR_BG);

	CRect rcUpperSqu = rcClient;
	//rcUpperSqu.bottom = rcUpperSqu.bottom - 10;
	hdc.FillSolidRect(rcUpperSqu, s.GetColorFromTheme(_T("ToolBarBG"), NEWUI_COLOR_TOOLBAR_UPPERBG));
	CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());

	if (!m_timerstr.IsEmpty() && pFrame && pFrame->IsSomethingLoaded())
    ShowPlayTime(&hdc, s, rcClient);
		
  UpdateButtonStat();
	
	int volume = min( m_volctrl.GetPos() , m_volctrl.GetRangeMax() );
    //SVP_LogMsg5(_T("Vol  %d %d ") , m_btnVolBG->m_rcHitest.top , m_btnVolBG->m_rcHitest.top + (m_btnVolBG->m_rcHitest.Height() -  m_btnVolTm->m_rcHitest.Height() ) / 2 );
	m_btnVolTm->m_rcHitest.MoveToXY(m_btnVolBG->m_rcHitest.left +  ( m_btnVolBG->m_rcHitest.Width() * volume / m_volctrl.GetRangeMax() ) - m_btnVolTm->m_rcHitest.Width()/2
        , m_btnVolBG->m_rcHitest.top + (m_btnVolBG->m_rcHitest.Height() -  m_btnVolTm->m_rcHitest.Height() ) / 2 );
	/*
	SVP_LogMsg5(_T("Vol left %d") , m_btnVolTm->m_rcHitest.left);
	
		CString szLog;
		szLog.Format(_T("TM POS %d %d"), volume , m_btnVolTm->m_rcHitest.left );
		SVP_LogMsg(szLog);*/
	
 	m_btnList.PaintAll(&hdc, rc);

	//LONGLONG costTime = AfxGetMyApp()->GetPerfCounter() - startTime;
	//SVP_LogMsg3("Toolbar Paint @ %I64d cost %I64d" , startTime , costTime);
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
	//m_btnList.SetDisableStat(ID_SUBTOOLBARBUTTON, !bLogo);
	if(!bLogo){
		m_timerstr.Empty();
	}
	BOOL bSub = pFrame->IsSubLoaded();
	m_btnList.SetDisableStat( ID_SUBDELAYINC, !bSub);
	m_btnList.SetDisableStat( ID_SUBDELAYDEC, !bSub);
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
bool  CPlayerToolBar::OnSetVolByMouse(CPoint point, BOOL byClick){
	CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());
	long nTBPos = point.x - m_btnVolBG->m_rcHitest.left;
	long TBMax = m_btnVolBG->m_rcHitest.right-m_btnVolBG->m_rcHitest.left ;
	nTBPos = max(0 , min(TBMax , nTBPos) );
	int Vol = 	nTBPos * m_volctrl.GetRangeMax() / TBMax;
	if(byClick){
		int oldVol = m_volctrl.GetPos();
		if(Vol > 100 ){
			Vol = max(oldVol,100) + 2;
		}
	}else{
		CString szVol;
		int VolPercent = Vol;
		if(VolPercent>100){
			VolPercent = 100 + (VolPercent-100) * 900/ (m_volctrl.GetRangeMax() - 100);
		}
		szVol.Format(_T("%d%%") , VolPercent);
		CPoint posTip( m_btnVolBG->m_rcHitest.left + nTBPos, m_btnVolBG->m_rcHitest.top);
		//ClientToScreen( &posTip );
		pFrame->m_tip.SetTips(  szVol , 1, &posTip);
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
void CPlayerToolBar::OnMouseMove(UINT nFlags, CPoint point){

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
	point += rc.TopLeft() ;
	
	if( m_nItemToTrack == ID_VOLUME_THUMB && m_bMouseDown ){
		if( bMouseMoved){
			OnSetVolByMouse(point);
			
		}
	}else if(bMouseMoved){
		
		UINT ret = m_btnList.OnHitTest(point,rc,-1);
		m_nItemToTrack = ret;
		if(ret){
			
		}else if(!m_tooltip.IsEmpty()){
			m_tooltip.Empty();
		}
		if(!m_nItemToTrack){
			pFrame->m_tip.SetTips(_T(""));
		}
		if( m_btnList.HTRedrawRequired ){
			Invalidate();
		}

		AppSettings& s = AfxGetAppSettings();
		if(s.bUserAeroUI() && bMouseMoved && m_bMouseDown && pFrame ){
			pFrame->m_lTransparentToolbarPosStat = 1;
			
			pFrame->m_wndFloatToolBar->PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
		}
	}
	
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

	point += rc.TopLeft() ;
	UINT ret = m_btnList.OnHitTest(point,rc,true);
	if( m_btnList.HTRedrawRequired ){
		if(ret)
			SetCapture();
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

BOOL CPlayerToolBar::ReadFromFile()
{
  std::wifstream in_file;
  std::wstring  buttoninformation;
  in_file.open(L"skins\\BottomToolBarButton.dat");
  if (!in_file)
    return FALSE;
  
  while (getline(in_file, buttoninformation))
  {
    if (buttoninformation[0] == ' ' || buttoninformation == L"")
      continue;

    m_string_vec.push_back(buttoninformation);
  }

  in_file.close();
  return TRUE;
}

void CPlayerToolBar::LineStringToVector()
{
  for (std::vector<std::wstring>::iterator ite = m_string_vec.begin();
       ite != m_string_vec.end(); ++ite)
  {
    std::wstring buttoninformation = *ite;
    std::wstring buttonname;
    ToolBarButton* buttonstruct = new ToolBarButton;
    int pos = buttoninformation.find_first_of(L":");
    buttonname = buttoninformation.substr(0, pos);
    buttonstruct->buttonname = buttonname;
    buttoninformation = buttoninformation.substr(pos + 1);
    StringToStruct(buttoninformation, buttonstruct);
    m_struct_vec.push_back(buttonstruct);
  }
}

void CPlayerToolBar::StringToStruct(std::wstring& buttoninformation, ToolBarButton* buttonstruct)
{
  FillStruct(buttoninformation, buttonstruct);
  SolveAddalign(buttoninformation, buttonstruct);
  
}

void CPlayerToolBar::FillStruct(std::wstring& buttoninformation, ToolBarButton* buttonstruct)
{
  std::wstring s;
  std::wstring classificationname;
  int pos;

  while ((pos = buttoninformation.find_first_of(L",")) != std::wstring::npos)
  {
    classificationname = buttoninformation.substr(0, pos);
    buttoninformation = buttoninformation.substr(pos + 1);
    pos = buttoninformation.find_first_of(L";");
    s   = buttoninformation.substr(0, pos);
    switch (m_classificationname_map[classificationname])
    {
    case BMP:
      buttonstruct->bmpstr = s;
      break;
    case ALIGN1:
      buttonstruct->align1 = m_align1_map[s];
      break;
    case CRECT1:
      buttonstruct->rect1 = GetCRect(s);
      break;
    case NOTBUTTON:
      if (buttoninformation[0] == 'T')
        buttonstruct->bnotbutton = TRUE;
      if (buttoninformation[0] == 'F')
        buttonstruct->bnotbutton = FALSE;
      break;
    case ID:
      buttonstruct->id = m_id_map[s];
      break;
    case HIDE:
      if (buttoninformation[0] == 'T')
        buttonstruct->bhide = TRUE;
      if (buttoninformation[0] == 'F')
        buttonstruct->bhide = FALSE;
      if (buttoninformation[0] == '!')
        buttonstruct->bhide = !IsMuted();
      if (buttoninformation[0] == 'I')
        buttonstruct->bhide = IsMuted();
      break;
    case HIDEWIDTH:
      if (s == L"MAXINT")
        buttonstruct->width = MAXINT;
      else
        buttonstruct->width = _wtoi(s.c_str());
      break;
    case ALIGN2:
      buttonstruct->align2 = m_align2_map[s];
      break;
    case BUTTON:
      buttonstruct->pbuttonname = s;
      break;
    case CRECT2:
      buttonstruct->rect2 = GetCRect(s);
      break;
    case ADDALIGN:
      buttonstruct->baddalign = TRUE;
      return;
    }
   buttoninformation = buttoninformation.substr(pos + 1);
  }
}

void CPlayerToolBar::SolveAddalign(std::wstring& buttoninformation, ToolBarButton* buttonstruct)
{
  if (!buttonstruct->baddalign)
    return;
  
  std::wstring s;
  std::wstring addalignstr;
  int pos;
  int align2 = 0;
  std::wstring pbuttonname = L"";
  CRect rect2 = CRect(0,0,0,0);

  while ((pos = buttoninformation.find_first_of(L",")) != std::wstring::npos)
  {
    s = buttoninformation.substr(0,pos);
    buttoninformation = buttoninformation.substr(pos + 1);
    pos = buttoninformation.find_first_of(L";");
    addalignstr = buttoninformation.substr(0, pos);
    switch (m_classificationname_map[s])
    {
    case ALIGN2:
      align2 = m_align2_map[addalignstr];
      break;
    case BUTTON:
      pbuttonname = addalignstr;
      break;
    case CRECT2:
      rect2 = GetCRect(addalignstr);
      break;
    }
    buttoninformation = buttoninformation.substr(pos + 1);
    if ((align2 != 0) && (pbuttonname != L"") && (rect2 != CRect(0,0,0,0)))
    {
      AddButton* addbutton = new AddButton(align2, pbuttonname,rect2);
      buttonstruct->addbuttonvec.push_back(addbutton);

      align2 = 0;
      pbuttonname = L"";
      rect2 = CRect(0,0,0,0);
    }
  }
  
}

void CPlayerToolBar::SetButton()
{
  for (std::vector<ToolBarButton*>::iterator ite = m_struct_vec.begin();
       ite != m_struct_vec.end(); ++ite)
  {
    if ((*ite)->buttonname != L"PLAYTIME")
    {
      if (!(*ite)->pbuttonname.empty())
        (*ite)->pbutton = m_pbutton_map[(*ite)->pbuttonname];
      (*ite)->mybutton = new CSUIButton((*ite)->bmpstr.c_str(), (*ite)->align1, (*ite)->rect1, 
        (*ite)->bnotbutton, (*ite)->id, (*ite)->bhide, (*ite)->align2, 
        (*ite)->pbutton, (*ite)->rect2);
      m_pbutton_map[(*ite)->buttonname] = (*ite)->mybutton;

      if ((*ite)->baddalign)
      {
        for (std::vector<AddButton*>::iterator iter = (*ite)->addbuttonvec.begin();
             iter != (*ite)->addbuttonvec.end(); ++iter)
        {
          (*iter)->pbutton = m_pbutton_map[(*iter)->pbuttonname];
          (*ite)->mybutton->addAlignRelButton((*iter)->align, (*iter)->pbutton, (*iter)->rect);
        }
      }

      m_btnList.AddTail((*ite)->mybutton);

      if ((*ite)->buttonname == L"VOLUMEBG")
        m_btnVolBG = (*ite)->mybutton;
      if ((*ite)->buttonname == L"SUBSWITCH")
        btnSubSwitch = (*ite)->mybutton;
      if ((*ite)->buttonname == L"VOLUMETM")
        m_btnVolTm = (*ite)->mybutton;
    }
  }
}

void CPlayerToolBar::DefaultInitializeButton()
{
  FillStruct();
  StructToString();
  WriteToFile();
}

CRect CPlayerToolBar::GetCRect(std::wstring rectstr)
{
  int left, top, right, bottom;
  int pos = rectstr.find_first_of(L",");
  std::wstring str;
  str = rectstr.substr(0, pos);
  left = _wtoi(str.c_str());
  rectstr = rectstr.substr(pos + 1);
  pos = rectstr.find_first_of(L",");
  str = rectstr.substr(0, pos);
  top = _wtoi(str.c_str());
  rectstr = rectstr.substr(pos + 1);
  pos = rectstr.find_first_of(L",");
  str = rectstr.substr(0, pos);
  right = _wtoi(str.c_str());
  rectstr = rectstr.substr(pos + 1);
  bottom = _wtoi(rectstr.c_str());
  CRect rc(left, top, right, bottom);
  return rc;
}

void CPlayerToolBar::ShowButton(ToolBarButton* tbb,BOOL bl)
{

    m_btnList.SetHideStat(tbb->id, tbb->bhide);
    
    switch (tbb->id)
    {
    case ID_VOLUME_MUTE:
    
      if(IsMuted())
      {
        m_btnList.SetHideStat(L"VOLUME.BMP", TRUE);
        m_btnList.SetHideStat(L"MUTED.BMP", FALSE | tbb->bhide);
      }else
      {
        m_btnList.SetHideStat(L"VOLUME.BMP", FALSE | tbb->bhide);
        m_btnList.SetHideStat(L"MUTED.BMP", TRUE);
      }
    break;
    case ID_PLAY_FWD:
    case ID_PLAY_BWD:
    case ID_NAVIGATE_SKIPBACK:
    case ID_NAVIGATE_SKIPFORWARD:
     if (bl)
      {
        m_btnList.SetHideStat(tbb->id, tbb->bhide);
        m_btnList.SetHideStat(ID_NAVIGATE_SKIPBACK , 0);
        m_btnList.SetHideStat(ID_NAVIGATE_SKIPFORWARD , 0);
      }
     else
     {
       m_btnList.SetHideStat(tbb->id, tbb->bhide);
       m_btnList.SetHideStat(ID_PLAY_FWD , 0);
       m_btnList.SetHideStat(ID_PLAY_BWD , 0);
     }
     break;
   }
  
}

void CPlayerToolBar::ShowPlayTime(CDC* dc, AppSettings& s, CRect& rcClient)
{
  HFONT holdft = (HFONT)dc->SelectObject(m_statft);

  dc->SetTextColor(s.GetColorFromTheme(_T("ToolBarTimeText"), 0xffffff) );
  CSize size = dc->GetTextExtent(m_timerstr);
  CRect frc;
  int   textalign;
  size.cx = min( rcClient.Width() /3, size.cx);
  for (std::vector<ToolBarButton*>::iterator ite = m_struct_vec.begin(); ite != m_struct_vec.end();
       ++ite)
    if ((*ite)->buttonname == L"PLAYTIME")
    {
      frc = (*ite)->rect1;
      textalign = (*ite)->align1;
    }
    PlayTimeRect(frc, rcClient);

    switch (textalign)
    {
    case ALIGN_TOPLEFT:
      frc = CRect ( rcClient.left + frc.left,
        rcClient.top + frc.top,
        rcClient.left + size.cx + frc.left,
        rcClient.top+ frc.top+size.cy);
      break;
    case ALIGN_TOPRIGHT:
      frc = CRect ( rcClient.right - size.cx - frc.right,
        rcClient.top + frc.top,
        rcClient.right-frc.right,
        rcClient.top+ frc.top+size.cy);
      break;
    case ALIGN_BOTTOMLEFT:
      frc = CRect ( rcClient.left + frc.left,
        rcClient.bottom - size.cy - frc.bottom,
        rcClient.left + size.cx + frc.left,
        rcClient.bottom - frc.bottom);
      break;
    case ALIGN_BOTTOMRIGHT:
      frc = CRect ( rcClient.right - size.cx - frc.right,
        rcClient.bottom - size.cy - frc.bottom,
        rcClient.right-frc.right,
        rcClient.bottom - frc.bottom);
      break;
    }
    ::DrawText(*dc, m_timerstr, m_timerstr.GetLength(), frc,  DT_LEFT|DT_END_ELLIPSIS|DT_SINGLELINE| DT_VCENTER);
    dc->SelectObject(holdft);
}

void CPlayerToolBar::PlayTimeRect(CRect& frc, CRect rcClient)
{
  if (frc.left < 0)
    frc.left = (-frc.left)*rcClient.Width()/100;
  if (frc.right < 0)
    frc.right = (-frc.right)*rcClient.Width()/100;
  if (frc.top < 0)
    frc.top = (-frc.top)*rcClient.Height()/100;
  if (frc.bottom < 0)
    frc.bottom = (-frc.top)*rcClient.Height()/100;
}

void CPlayerToolBar::FillStruct()
{
  ToolBarButton* buttonstruct;
  AddButton* addbutton;

  buttonstruct = new ToolBarButton(L"VOLUMEBG", L"VOLUME_BG.BMP", ALIGN_TOPRIGHT,
    CRect(3,-50,15,3), TRUE, 0, FALSE,0);
  m_struct_vec.push_back(buttonstruct);

  buttonstruct = new ToolBarButton(L"VOLUMETM", L"VOLUME_TM.BMP", ALIGN_TOPRIGHT,
    CRect(3,-50,65,3),FALSE,ID_VOLUME_THUMB,FALSE,0);
  m_struct_vec.push_back(buttonstruct);

  buttonstruct = new ToolBarButton(L"PLAY", L"BTN_PLAY.BMP", ALIGN_TOPLEFT, CRect(-50,-50,3,3),
    FALSE,ID_PLAY_PLAY,FALSE,0);
  m_struct_vec.push_back(buttonstruct);

  buttonstruct = new ToolBarButton(L"PAUSE",L"BTN_PAUSE.BMP",ALIGN_TOPLEFT,CRect(-50,-50,3,3),
    FALSE,ID_PLAY_PAUSE,TRUE,0);
  m_struct_vec.push_back(buttonstruct);
  
  buttonstruct = new ToolBarButton(L"STOP",L"BTN_STOP.BMP",ALIGN_TOPLEFT,CRect(-50,-50,3,3),
    FALSE,ID_PLAY_MANUAL_STOP,FALSE,660,ALIGN_RIGHT,L"PAUSE",CRect(1,1,1,1),TRUE);
  addbutton = new AddButton(ALIGN_RIGHT, L"PLAY", CRect(1,1,1,1));
  buttonstruct->addbuttonvec.push_back(addbutton);
  m_struct_vec.push_back(buttonstruct);

  buttonstruct = new ToolBarButton(L"STEP",L"BTN_STEP.BMP",ALIGN_TOPLEFT,CRect(-50,-50,3,3),
    FALSE,ID_PLAY_FRAMESTEP,FALSE,660,ALIGN_LEFT,L"PAUSE",CRect(1,1,1,1),TRUE);
  addbutton = new AddButton(ALIGN_LEFT,L"PLAY",CRect(1,1,1,1));
  buttonstruct->addbuttonvec.push_back(addbutton);
  m_struct_vec.push_back(buttonstruct);

  buttonstruct = new ToolBarButton(L"FASTFORWORD",L"FAST_FORWORD.BMP",ALIGN_TOPLEFT,CRect(-50,-50,3,3),FALSE,
    ID_PLAY_FWD,FALSE,440,ALIGN_LEFT,L"PAUSE",CRect(1,1,1,1),TRUE);
  addbutton = new AddButton(ALIGN_LEFT,L"PLAY",CRect(1,1,1,1));
  buttonstruct->addbuttonvec.push_back(addbutton);
  addbutton = new AddButton(ALIGN_LEFT,L"STEP",CRect(1,1,1,1));
  buttonstruct->addbuttonvec.push_back(addbutton);
  m_struct_vec.push_back(buttonstruct);

  buttonstruct = new ToolBarButton(L"FASTBACKWORD",L"FAST_BACKWORD.BMP",ALIGN_TOPLEFT,CRect(-50,-50,3,3),FALSE,ID_PLAY_BWD,FALSE,
    440,ALIGN_RIGHT,L"PAUSE",CRect(1,1,1,1),TRUE);
  addbutton = new AddButton(ALIGN_RIGHT,L"PLAY",CRect(1,1,1,1));
  buttonstruct->addbuttonvec.push_back(addbutton);
  addbutton = new AddButton(ALIGN_RIGHT,L"STOP",CRect(1,1,1,1));
  buttonstruct->addbuttonvec.push_back(addbutton);
  m_struct_vec.push_back(buttonstruct);

  buttonstruct = new ToolBarButton(L"PREV",L"BTN_PREV.BMP",ALIGN_TOPLEFT,CRect(-50,-50,3,3),FALSE,ID_NAVIGATE_SKIPBACK,FALSE,
    440,ALIGN_RIGHT,L"FASTBACKWORD",CRect(1,1,1,1),TRUE);
  addbutton = new AddButton(ALIGN_RIGHT,L"PLAY",CRect(1,1,1,1));
  buttonstruct->addbuttonvec.push_back(addbutton);
  addbutton = new AddButton(ALIGN_RIGHT,L"PAUSE",CRect(1,1,1,1));
  buttonstruct->addbuttonvec.push_back(addbutton);
  m_struct_vec.push_back(buttonstruct);

  buttonstruct = new ToolBarButton(L"NEXT",L"BTN_NEXT.BMP",ALIGN_TOPLEFT,CRect(-50,-50,3,3),FALSE,
    ID_NAVIGATE_SKIPFORWARD,FALSE,440,ALIGN_LEFT,L"FASTFORWORD",CRect(1,1,1,1),TRUE);
  addbutton = new AddButton(ALIGN_LEFT,L"PLAY",CRect(1,1,1,1));
  buttonstruct->addbuttonvec.push_back(addbutton);
  addbutton =new AddButton(ALIGN_LEFT,L"PAUSE",CRect(1,1,1,1));
  buttonstruct->addbuttonvec.push_back(addbutton);
  m_struct_vec.push_back(buttonstruct);

  buttonstruct = new ToolBarButton(L"LOGO",L"SPLAYER.BMP",ALIGN_TOPLEFT,CRect(14,-50,3,3),TRUE,0,FALSE,0);
  m_struct_vec.push_back(buttonstruct);

  buttonstruct = new ToolBarButton(L"VOLUME",L"VOLUME.BMP",ALIGN_TOPRIGHT,CRect(3,-50,105,3),FALSE,
    ID_VOLUME_MUTE,FALSE,440,ALIGN_RIGHT,L"VOLUMEBG",CRect(3,3,3,3),FALSE);
  m_struct_vec.push_back(buttonstruct);
  
  buttonstruct = new ToolBarButton(L"MUTED",L"MUTED.BMP",ALIGN_TOPRIGHT,CRect(3,-50,105,3),FALSE,
    ID_VOLUME_MUTE,TRUE,440,ALIGN_RIGHT,L"VOLUMEBG",CRect(3,3,3,3),FALSE);
  m_struct_vec.push_back(buttonstruct);

  buttonstruct = new ToolBarButton(L"SETTING",L"BTN_SETTING.BMP",ALIGN_TOPRIGHT,CRect(-70,-50,105,3),FALSE,
    ID_VIEW_OPTIONS,TRUE,600,ALIGN_RIGHT,L"MUTED",CRect(3,10,3,10),TRUE);
  addbutton = new AddButton(ALIGN_RIGHT,L"VOLUME",CRect(3,10,3,10));
  buttonstruct->addbuttonvec.push_back(addbutton);
  m_struct_vec.push_back(buttonstruct);

  buttonstruct = new ToolBarButton(L"PLAYLIST",L"BTN_PLAYLIST.BMP",ALIGN_TOPRIGHT,CRect(3,-50,33,3),FALSE,ID_VIEW_PLAYLIST,
    FALSE,540,ALIGN_RIGHT,L"SETTING",CRect(3,10,3,10),TRUE);
  addbutton = new AddButton(ALIGN_RIGHT,L"VOLUME",CRect(3,10,3,10));
  buttonstruct->addbuttonvec.push_back(addbutton);
  addbutton = new AddButton(ALIGN_RIGHT,L"MUTED",CRect(3,10,3,10));
  buttonstruct->addbuttonvec.push_back(addbutton);
  addbutton = new AddButton(ALIGN_RIGHT,L"VOLUMEBG",CRect(3,10,3,10));
  buttonstruct->addbuttonvec.push_back(addbutton);
  m_struct_vec.push_back(buttonstruct);

  buttonstruct = new ToolBarButton(L"CAPTURE",L"BTN_CAPTURE.BMP",ALIGN_TOPRIGHT,CRect(3,-50,105,3),FALSE,
    ID_FILE_SAVE_IMAGE_AUTO,TRUE,MAXINT,ALIGN_RIGHT,L"PLAYLIST",CRect(3,10,3,10),FALSE);
  m_struct_vec.push_back(buttonstruct);

  buttonstruct = new ToolBarButton(L"OPENFILE",L"BTN_OPENFILE_SMALL.BMP",ALIGN_TOPRIGHT,CRect(3,-50,105,3),FALSE,
    ID_FILE_OPENQUICK,TRUE,660,ALIGN_RIGHT,L"CAPTURE",CRect(3,10,3,10),TRUE);
  addbutton = new AddButton(ALIGN_RIGHT,L"PLAYLIST",CRect(3,10,3,10));
  buttonstruct->addbuttonvec.push_back(addbutton);
  m_struct_vec.push_back(buttonstruct);

  buttonstruct = new ToolBarButton(L"SUBSWITCH",L"BTN_SUB.BMP",ALIGN_TOPLEFT,CRect(-23,-50,3,3),FALSE,
    ID_SUBTOOLBARBUTTON,TRUE,540,ALIGN_RIGHT,L"FASTBACKWORD",CRect(20,10,22,10),TRUE);
  addbutton = new AddButton(ALIGN_LEFT,L"LOGO",CRect(15,10,10,10));
  buttonstruct->addbuttonvec.push_back(addbutton);
  addbutton = new AddButton(ALIGN_RIGHT,L"PREV",CRect(20,10,22,10));
  buttonstruct->addbuttonvec.push_back(addbutton);
  m_struct_vec.push_back(buttonstruct);

  buttonstruct = new ToolBarButton(L"SUBDELAYREDUCE",L"BTN_SUB_DELAY_REDUCE.BMP",ALIGN_TOPLEFT,CRect(-42,-50,3,3),FALSE,
    ID_SUBDELAYDEC,TRUE,540,ALIGN_RIGHT,L"SUBSWITCH",CRect(2,3,2,3),FALSE);
  m_struct_vec.push_back(buttonstruct);

  buttonstruct = new ToolBarButton(L"SUBDELAYINCREASE",L"BTN_SUB_DELAY_INCREASE.BMP",ALIGN_TOPLEFT,CRect(-10,-50,3,3),FALSE,
    ID_SUBDELAYINC,TRUE,540,ALIGN_LEFT,L"SUBSWITCH",CRect(2,3,2,3),FALSE);
  m_struct_vec.push_back(buttonstruct);

  buttonstruct = new ToolBarButton(L"PLAYTIME",ALIGN_TOPLEFT,CRect(10,15,3,3));
  m_struct_vec.push_back(buttonstruct);

  SetButton();
}

void CPlayerToolBar::StructToString()
{
  for (std::vector<ToolBarButton*>::iterator ite = m_struct_vec.begin();
    ite != m_struct_vec.end(); ++ite)
     m_string_vec.push_back(FillString(*ite));
}

std::wstring CPlayerToolBar::FillString(ToolBarButton* ttb)
{
  std::wstring buttoninformation;
  std::wstring btname,bmp,agn1,rc1,bnbutton,id,bhd,wdt;
  std::wstring agn2,pbtname,rc2;

  btname = ttb->buttonname + L":";
  bmp = L"BMP," + ttb->bmpstr + L";";
  agn1 = L"ALIGN1," + GetAlignorIdString(ttb->align1, m_align1_map) + L";";
  rc1 = L"CRECT1," + RectToString(ttb->rect1) + L";";
  bnbutton = L"NOTBUTTON," + BoolString(ttb->bnotbutton) + L";";
  id = L"ID," + GetAlignorIdString(ttb->id, m_id_map) + L";";
  bhd = L"HIDE," + BoolString(ttb->bhide) + L";";
  wdt = L"HIDEWIDTH," + GetWidth(ttb->width) + L";";
  if (ttb->buttonname == L"PLAYTIME")
    buttoninformation = btname + agn1 + rc1;
  else
    buttoninformation = btname + bmp + agn1 + rc1 + bnbutton + id + bhd + wdt;
  
  if (ttb->align2 != 0)
  {
    agn2 = L"ALIGN2," + GetAlignorIdString(ttb->align2, m_align2_map) + L";";
    pbtname = L"BUTTON," + ttb->pbuttonname + L";";
    rc2 = L"CRECT2," + RectToString(ttb->rect2) + L";";
    buttoninformation += agn2 + pbtname + rc2;
  }

  if (ttb->baddalign)
  {
    buttoninformation += L"ADDALIGN,";
    for (std::vector<AddButton*>::iterator ite = ttb->addbuttonvec.begin();
         ite != ttb->addbuttonvec.end(); ++ite)
    {
      agn2 = L"ALIGN2," + GetAlignorIdString((*ite)->align, m_align2_map) + L";";
      pbtname = L"BUTTON," + (*ite)->pbuttonname + L";";
      rc2 = L"CRECT2," + RectToString((*ite)->rect) + L";";
      buttoninformation += agn2 + pbtname + rc2;
    }
  }

  return buttoninformation;

}

std::wstring CPlayerToolBar::GetAlignorIdString(int i,std::map<std::wstring, int> mp)
{
  if (i == 0)
    return L"0";
  for (std::map<std::wstring, int>::iterator ite = mp.begin();
       ite != mp.end(); ++ite)
  {
    if (ite->second == i)
      return ite->first;
  }
  return L"";
}

std::wstring CPlayerToolBar::RectToString(CRect& rc)
{
  wchar s[10];
  std::wstring str;
  int left = rc.left;
  _itow(left, s, 10);
  str =str + s + L",";
  int top  = rc.top;
  _itow(top, s, 10);
  str =str + s + L",";
  int right = rc.right;
  _itow(right, s, 10);
  str =str + s + L",";
  int bottom = rc.bottom;
  _itow(bottom, s ,10);
  str += s;

  return str;
  
}

std::wstring CPlayerToolBar::BoolString(BOOL bl)
{
  if (bl)
    return L"TRUE";
  else
    return L"FALSE";
}

std::wstring CPlayerToolBar::GetWidth(int wid)
{
  if (wid == MAXINT)
    return L"MAXINT";
  wchar s[10];
  _itow(wid, s, 10);
  return s;
}

void CPlayerToolBar::WriteToFile()
{
  std::wofstream outfile;
  outfile.open(L"skins\\BottomToolBarButton.dat");
  for (std::vector<std::wstring>::iterator ite = m_string_vec.begin();
       ite != m_string_vec.end(); ++ite)
    outfile<<(*ite)<<L"\n";
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