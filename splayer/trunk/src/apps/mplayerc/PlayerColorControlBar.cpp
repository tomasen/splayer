// PlayerColorControlBar.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"

#include "MainFrm.h"
#include "PlayerColorControlBar.h"


// CPlayerColorControlBar

IMPLEMENT_DYNAMIC(CPlayerColorControlBar, CSVPDialog)

CPlayerColorControlBar::CPlayerColorControlBar()
{

}

CPlayerColorControlBar::~CPlayerColorControlBar()
{
}


BEGIN_MESSAGE_MAP(CPlayerColorControlBar, CSVPDialog)
	//ON_WM_ERASEBKGND()
	//ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_HSCROLL()

	ON_WM_GETMINMAXINFO()

	ON_BN_CLICKED(IDC_BUTTONRESETCOLORCONTROL,   OnButtonReset)
	ON_BN_CLICKED(IDC_BUTTONENABLECOLORCONTROL,  OnColorControlButtonEnable)
	
END_MESSAGE_MAP()


void CPlayerColorControlBar::OnColorControlButtonEnable(){
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if(pFrame){
		AppSettings& s = AfxGetAppSettings();
		//s.fVMR9MixerMode = !s.fVMR9MixerMode;
		//if(s.fVMR9MixerMode){
			s.iDSVideoRendererType = 6;
			s.iRMVideoRendererType = 2;
			s.iQTVideoRendererType = 2;
		//}

		if( pFrame->IsSomethingLoaded()){
			pFrame->ReRenderOrLoadMedia();

		}
		CheckAbility();
		
	}

	Relayout();
}


// CPlayerColorControlBar message handlers
void CPlayerColorControlBar::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	
	AppSettings& s = AfxGetAppSettings();
	CMainFrame* pMFrame = (CMainFrame*)AfxGetMainWnd();

		//if(pMFrame->m_pMC){
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
		//}
		
	 

}

int CPlayerColorControlBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if(__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	
	CRect r;
	GetClientRect(r);

	GetSystemFontWithScale(&m_font);

	csBrightLabel.Create( ResStr(IDS_COLOR_CONTROL_LABEL_BRIGHT),WS_CHILD|WS_VISIBLE|SS_CENTERIMAGE , 
		r, this, IDC_STATIC);
	csBrightLabel.SetFont(&m_font);

	csConstLabel.Create( ResStr(IDS_COLOR_CONTROL_LABEL_CONSTANT),  WS_CHILD|WS_VISIBLE|SS_CENTERIMAGE, 
		r, this, IDC_STATIC);
	csConstLabel.SetFont(&m_font);

	csl_bright.Create( WS_CHILD|WS_VISIBLE|TBS_AUTOTICKS|TBS_HORZ|TBS_NOTICKS  , r, this, IDC_SLIDER1);
	csl_const.Create( WS_CHILD|WS_VISIBLE|TBS_AUTOTICKS|TBS_HORZ|TBS_NOTICKS  , r, this, IDC_SLIDER2);

	cb_reset.Create( ResStr(IDS_COLOR_CONTROL_BUTTON_RESET), WS_VISIBLE|WS_CHILD|BS_FLAT|BS_VCENTER|BS_CENTER, r , this, IDC_BUTTONRESETCOLORCONTROL);
	cb_reset.SetFont(&m_font);
	
	//cb_enablectrl.Create( ResStr(IDS_COLOR_CONTROL_BUTTON_ENABLE), WS_VISIBLE|WS_CHILD|BS_FLAT|BS_VCENTER|BS_CENTER, r , this, IDC_BUTTONENABLECOLORCONTROL);
	//cb_enablectrl.SetFont(&m_font);

	Relayout();
	CheckAbility();

	return 0;
}
void CPlayerColorControlBar::OnButtonReset(){
	CMainFrame* pMFrame = (CMainFrame*)GetParentFrame();
	AppSettings& s = AfxGetAppSettings();
	//if(!!pMFrame->m_pMC ){
		CheckAbility();
		
		
		s.dBrightness = fDefaultBright;
		csl_bright.SetPos(fDefaultBright);
		s.dContrast = fDefaultConst;
		csl_const.SetPos(fDefaultConst * 100);
		pMFrame->SetVMR9ColorControl(fDefaultBright,fDefaultConst,fDefaultHue,fDefaultSaturation);
	//}
	Relayout();
}

void CPlayerColorControlBar::CheckAbility(){
	CMainFrame* pMFrame = (CMainFrame*)AfxGetMainWnd();
	AppSettings& s = AfxGetAppSettings();
	

	
	

	if(s.bOldLumaControl){
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
	}else{
		csl_bright.SetRange(1,  200, 1);
		csl_bright.SetTic( 1 );
		fDefaultBright = 100;
		csl_bright.SetPos( s.dBrightness  );

		csl_const.SetRange(1,400,1);
		csl_const.SetTic( 1);
		fDefaultConst = 1.0;
		csl_const.SetPos(s.dContrast*  100 );
		m_bAbleControl = true;
		csl_bright.EnableWindow(m_bAbleControl);
		csl_const.EnableWindow(m_bAbleControl);
		csConstLabel.EnableWindow(m_bAbleControl);
		csBrightLabel.EnableWindow(m_bAbleControl);
	}
	

}
void CPlayerColorControlBar::Relayout()
{
	CRect r, r2;
	GetClientRect(r);

	
	r2 = r;
	r2.top += 9;
	r2.left += 5;
	r2.right = r2.left + 40;
	r2.bottom = r2.top + 20;
	csBrightLabel.MoveWindow(&r2);

	r2 = r;
	r2.left += 5;
	r2.right = r2.left + 40;
	r2.top += 29;
	r2.bottom = r2.top + 20;
	csConstLabel.MoveWindow(&r2);


	r2 = r;
	r2.top += 9;
	r2.left += 50;
	r2.right =  r2.right - 18;
	r2.bottom = r2.top + 20;
	csl_bright.MoveWindow(&r2);
	
	r2 = r;
	r2.top += 9;
	r2.left += 50;
	r2.right = r2.right - 18;
	r2.top += 20;
	r2.bottom = r2.top + 20;
	csl_const.MoveWindow(&r2);

	r2 = r;
	r2.left += 50;
	r2.right = r2.left + 35;
	r2.top += 54;
	r2.bottom = r2.top + 20;
	cb_reset.MoveWindow(&r2);
	cb_reset.EnableWindow(TRUE);

	/*
r2 = r;
	r2.left += 90;
	r2.right = r2.left + 35;
	r2.top += 54;
	r2.bottom = r2.top + 20;
	cb_enablectrl.MoveWindow(&r2);
	cb_enablectrl.EnableWindow(TRUE);
*/

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	AppSettings& s  = AfxGetAppSettings();
	bool bEnable = (s.iDSVideoRendererType != VIDRNDT_DS_VMR9RENDERLESS) || !s.fVMR9MixerMode;
	BOOL enable = !bEnable;
	/*
	if(pFrame){
			if(pFrame->IsSomethingLoaded() ){
				enable == !!pFrame->m_pMC;
				bEnable = !pFrame->m_pMC;
			}
		}*/
	
	cb_reset.EnableWindow(TRUE);
	cb_reset.Invalidate();
	
	//if(bEnable)
	//	cb_enablectrl.SetWindowText(ResStr(IDS_COLOR_CONTROL_BUTTON_ENABLE));
	//else
	//	cb_enablectrl.SetWindowText(ResStr(IDS_COLOR_CONTROL_BUTTON_DISABLE));
	
	//cb_enablectrl.Invalidate();

	Invalidate();
}


/*
BOOL CPlayerColorControlBar::OnEraseBkgnd(CDC* pDC)
{
	

	CRect r;
	GetClientRect(&r);

	
	pDC->FillSolidRect(&r, 0x00);
	return TRUE;
}
*/
/*

void CPlayerColorControlBar::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect r;


	// Do not call CDialogBar::OnPaint() for painting messages
}
*/

void CPlayerColorControlBar::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	Relayout();
}

BOOL CPlayerColorControlBar::PreCreateWindow(CREATESTRUCT& cs)
{
	if(!__super::PreCreateWindow(cs))
		return FALSE;
	

	return TRUE;
}

void CPlayerColorControlBar::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: Add your message handler code here and/or call default

	CSVPDialog::OnGetMinMaxInfo(lpMMI);
}
