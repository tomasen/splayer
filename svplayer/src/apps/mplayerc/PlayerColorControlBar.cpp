// PlayerColorControlBar.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"

#include "MainFrm.h"
#include "PlayerColorControlBar.h"


// CPlayerColorControlBar

IMPLEMENT_DYNAMIC(CPlayerColorControlBar, CDialogBar)

CPlayerColorControlBar::CPlayerColorControlBar()
{

}

CPlayerColorControlBar::~CPlayerColorControlBar()
{
}


BEGIN_MESSAGE_MAP(CPlayerColorControlBar, CDialogBar)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_HSCROLL()

END_MESSAGE_MAP()

// CPlayerColorControlBar message handlers
void CPlayerColorControlBar::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	
	AppSettings& s = AfxGetAppSettings();
	CMainFrame* pMFrame = (CMainFrame*)GetParentFrame();

		if(pMFrame->m_pMC){
			BOOL bChanged = FALSE;
			if (pScrollBar == GetDlgItem(IDC_SLIDER1) && s.dBrightness != csl_bright.GetPos()){
				s.dBrightness = csl_bright.GetPos();
				bChanged = TRUE;
			}else if (pScrollBar == GetDlgItem(IDC_SLIDER2) && s.dContrast != csl_const.GetPos()){
				s.dContrast = (float)csl_const.GetPos() / 100;
				bChanged = TRUE;
			}
			if(bChanged){
				pMFrame->SetVMR9ColorControl(s.dBrightness,s.dContrast,fDefaultHue,fDefaultSaturation);
			}
		}
		
	

}

int CPlayerColorControlBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if(CDialogBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect r;
	GetClientRect(r);

	GetSystemFontWithScale(&m_font);

	csBrightLabel.Create( _T("亮度: "),WS_CHILD|WS_VISIBLE|SS_CENTERIMAGE , 
		r, this, IDC_STATIC);
	csBrightLabel.SetFont(&m_font);

	csConstLabel.Create( _T("对比度: "),  WS_CHILD|WS_VISIBLE|SS_CENTERIMAGE, 
		r, this, IDC_STATIC);
	csConstLabel.SetFont(&m_font);

	csl_bright.Create( WS_CHILD|WS_VISIBLE|TBS_AUTOTICKS|TBS_HORZ|TBS_NOTICKS|TBS_TOOLTIPS  , r, this, IDC_SLIDER1);
	csl_const.Create( WS_CHILD|WS_VISIBLE|TBS_AUTOTICKS|TBS_HORZ|TBS_NOTICKS|TBS_TOOLTIPS  , r, this, IDC_SLIDER2);

	cb_reset.Create( _T("重置"), BS_PUSHBUTTON|WS_CHILD|WS_VISIBLE, r , this, IDC_BUTTONRESETCOLORCONTROL);
	cb_reset.SetFont(&m_font);
	
	cb_enablectrl.Create( _T("启用"), BS_PUSHBUTTON|WS_CHILD|WS_VISIBLE, r , this, IDC_BUTTONENABLECOLORCONTROL);
	cb_enablectrl.SetFont(&m_font);

	Relayout();
	CheckAbility();

	return 0;
}
void CPlayerColorControlBar::OnButtonReset(){
	CMainFrame* pMFrame = (CMainFrame*)GetParentFrame();
	AppSettings& s = AfxGetAppSettings();
	if(!!pMFrame->m_pMC ){
		CheckAbility();
		
		
		s.dBrightness = fDefaultBright;
		csl_bright.SetPos(fDefaultBright);
		s.dContrast = fDefaultConst;
		csl_const.SetPos(fDefaultConst * 100);
		pMFrame->SetVMR9ColorControl(fDefaultBright,fDefaultConst,fDefaultHue,fDefaultSaturation);
	}
}
void CPlayerColorControlBar::OnButtonEnable(){
	
}
void CPlayerColorControlBar::CheckAbility(){
	CMainFrame* pMFrame = (CMainFrame*)GetParentFrame();
	AppSettings& s = AfxGetAppSettings();
	m_bAbleControl = (!!pMFrame->m_pMC);
	
	csl_bright.EnableWindow(m_bAbleControl);
	csl_const.EnableWindow(m_bAbleControl);
	csConstLabel.EnableWindow(m_bAbleControl);
	csBrightLabel.EnableWindow(m_bAbleControl);

	if(m_bAbleControl){
		VMR9ProcAmpControlRange ClrRangeBright;
		ClrRangeBright.dwProperty = ProcAmpControl9_Brightness;
		ClrRangeBright.dwSize = sizeof(VMR9ProcAmpControlRange);
		pMFrame->m_pMC->GetProcAmpControlRange(0, &ClrRangeBright);
		fDefaultBright = ClrRangeBright.DefaultValue; 
		csl_bright.SetRange(ClrRangeBright.MinValue,  ClrRangeBright.MaxValue, 1);
		csl_bright.SetTic( ClrRangeBright.StepSize );
		csl_bright.SetPos( s.dBrightness );

		VMR9ProcAmpControlRange ClrRangeConst;
		ClrRangeConst.dwProperty = ProcAmpControl9_Contrast;
		ClrRangeConst.dwSize = sizeof(VMR9ProcAmpControlRange);
		pMFrame->m_pMC->GetProcAmpControlRange(0, &ClrRangeConst);
		fDefaultConst = ClrRangeConst.DefaultValue; 
		
		csl_const.SetRange(ClrRangeConst.MinValue * 100,  ClrRangeConst.MaxValue* 100, 1);
		csl_const.SetTic( ClrRangeConst.StepSize * 100);
		csl_const.SetPos( s.dContrast* 100 );
		
		VMR9ProcAmpControlRange ClrRangeHue;
		ClrRangeHue.dwProperty = ProcAmpControl9_Hue;
		ClrRangeHue.dwSize = sizeof(VMR9ProcAmpControlRange);
		pMFrame->m_pMC->GetProcAmpControlRange(0, &ClrRangeHue);
		fDefaultHue = ClrRangeHue.DefaultValue; 

		VMR9ProcAmpControlRange ClrRangeSat;
		ClrRangeSat.dwProperty = ProcAmpControl9_Saturation;
		ClrRangeSat.dwSize = sizeof(VMR9ProcAmpControlRange);
		pMFrame->m_pMC->GetProcAmpControlRange(0, &ClrRangeSat);
		fDefaultSaturation = ClrRangeSat.DefaultValue; 
	}

}
void CPlayerColorControlBar::Relayout()
{
	CRect r, r2;
	GetClientRect(r);

	
	r2 = r;
	r2.left += 5;
	r2.right = r2.left + 40;
	csBrightLabel.MoveWindow(&r2);

	r2 = r;
	r2.left += r.Width() / 2 - 40;
	r2.right = r2.left + 40;
	csConstLabel.MoveWindow(&r2);


	r2 = r;
	r2.top += 1;
	r2.left += 50;
	r2.right = r2.left + r.Width() / 2 - 110;
	csl_bright.MoveWindow(&r2);
	
	r2 = r;
	r2.top += 4;
	r2.left += (r.Width() - 50) / 2 + 30;
	r2.right = r2.left + r.Width() / 2 - 110;
	csl_const.MoveWindow(&r2);

	r2 = r;
	r2.top += 4;
	r2.right -= 10;
	r2.left = r2.right - 35;
	cb_reset.MoveWindow(&r2);
	cb_reset.EnableWindow(TRUE);

	r2 = r;
	r2.top += 1;
	r2.right -= 48;
	r2.left = r2.right - 35;
	cb_enablectrl.MoveWindow(&r2);
	cb_enablectrl.EnableWindow(TRUE);

	Invalidate();
}


BOOL CPlayerColorControlBar::OnEraseBkgnd(CDC* pDC)
{
	for(CWnd* pChild = GetWindow(GW_CHILD); pChild; pChild = pChild->GetNextWindow())
	{
		if(!pChild->IsWindowVisible()) continue;

		CRect r;
		pChild->GetClientRect(&r);
		pChild->MapWindowPoints(this, &r);
		pDC->ExcludeClipRect(&r);
	}

	CRect r;
	GetClientRect(&r);

	CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());


	if(pFrame->m_pLastBar != this || pFrame->m_fFullScreen)
		r.InflateRect(0, 0, 0, 1);

	r.InflateRect(1, 1, 1, 0);
	pDC->FillSolidRect(&r, GetSysColor(COLOR_BTNFACE));
	return TRUE;
}

void CPlayerColorControlBar::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect r;


	// Do not call CDialogBar::OnPaint() for painting messages
}

void CPlayerColorControlBar::OnSize(UINT nType, int cx, int cy)
{
	CDialogBar::OnSize(nType, cx, cy);

	Relayout();
}

BOOL CPlayerColorControlBar::Create(CWnd* pParentWnd)
{
	return CDialogBar::Create(pParentWnd, IDD_PLAYERCOLORCONTROLSBAR, WS_CHILD|WS_VISIBLE|CBRS_ALIGN_BOTTOM|SW_HIDE, IDD_PLAYERCOLORCONTROLSBAR); //not visible default
}

BOOL CPlayerColorControlBar::PreCreateWindow(CREATESTRUCT& cs)
{
	//cs.dwExStyle |= WS_EX_TRANSPARENT ;
	if(!CDialogBar::PreCreateWindow(cs))
		return FALSE;

	m_dwStyle &= ~CBRS_BORDER_TOP;
	m_dwStyle &= ~CBRS_BORDER_BOTTOM;
	m_dwStyle &= ~WS_VISIBLE;
	

	return TRUE;
}
