// PlayerToolTopBar.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "PlayerToolTopBar.h"


// PlayerToolTopBar.cpp : implementation file
#include "MainFrm.h"
#include "../../svplib/svplib.h"
// CPlayerToolTopBar

IMPLEMENT_DYNAMIC(CPlayerToolTopBar, CWnd)

CPlayerToolTopBar::CPlayerToolTopBar():
m_hovering(0),
m_pbtnList(&m_btnList)
{

}

CPlayerToolTopBar::~CPlayerToolTopBar()
{
}


BEGIN_MESSAGE_MAP(CPlayerToolTopBar, CWnd)
	ON_WM_CREATE()
	ON_WM_MOVE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_NCPAINT()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnTtnNeedText)
	ON_WM_ENABLE()
	ON_WM_ACTIVATE()
	ON_WM_NCHITTEST()
	ON_WM_NCCALCSIZE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_MOUSELEAVE()
	ON_WM_SHOWWINDOW()
	ON_WM_NCACTIVATE()
	ON_WM_SETFOCUS()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_ACTIVATEAPP()
END_MESSAGE_MAP()



// CPlayerToolTopBar message handlers

BOOL CPlayerToolTopBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message){

	//CPoint pt;
	//::GetCursorPos (&pt);
	//ScreenToClient (&pt);

	if(m_nItemToTrack){	
		SetCursor(cursorHand );
		return TRUE;
	}
	return CWnd::OnSetCursor(pWnd, 0, 0);
}

BOOL CPlayerToolTopBar::OnTtnNeedText(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
{
	//AfxMessageBox(_T("x")); //where is my tooltip!?!
	UNREFERENCED_PARAMETER(id);

	TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;
	UINT_PTR nID = pNMHDR->idFrom;
	BOOL bRet = FALSE;

	if(m_nItemToTrack){
		// idFrom is actually the HWND of the tool
		CString toolTip;
		m_lastTipItem = m_nItemToTrack;
		switch(m_nItemToTrack){
					case ID_VIEW_VF_FROMINSIDE:
						toolTip = _T("标准画面");
						break;
					case ID_VIEW_VF_FROMOUTSIDE:
						toolTip = _T("智能去黑边");
						break;
					case ID_VIEW_FULLSCREEN:
						toolTip = _T("全屏切换");
						break;
					case ID_VIEW_ZOOM_100:
						toolTip = _T("100% 尺寸");
						break;
					case ID_VIEW_ZOOM_200:
						toolTip = _T("200% 尺寸");
						break;
					case ID_MENU_AUDIO:
						toolTip = _T("音轨选择");
						break;
					case ID_MENU_VIDEO:
						toolTip = _T("画面增益");
						break;
					case ID_ONTOP_ALWAYS:
						toolTip = _T("使窗口钉在最前端");
						break;
					case ID_ONTOP_NEVER:
						toolTip = _T("恢复正常窗口");
						break;
					case ID_ROTATE_90:
						toolTip = _T("顺时针旋转 90℃");
						break;
					case ID_ROTATE_V:
						toolTip = _T("垂直翻转");
						break;
					case ID_FILE_SAVE_IMAGE:
						toolTip = _T("截图至文件和剪贴板");
						break;
					case ID_SHOWTRANSPRANTBAR:
						toolTip = _T("半透明界面");
						break;
					
					case ID_SHOWCOLORCONTROLBAR:
						toolTip = _T("亮度与对比度");
						break;
					default:
						toolTip = ResStr(m_nItemToTrack);
						break;
		}
		//AfxMessageBox(toolTip);
		//if(toolTip.IsEmpty())
		//	toolTip = _T("Unkown");
		
			if(AfxGetMyApp()->IsVista() ){
				if(!toolTip.IsEmpty()){
					pTTT->lpszText = toolTip.GetBuffer();
					pTTT->hinst = AfxGetResourceHandle();
					bRet = TRUE;
				}
			}else{
				
				CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());
				pFrame->m_tip.SetTips(toolTip, TRUE);
			}
		
	}else{
		CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());
		pFrame->m_tip.SetTips(ResStr(nID), TRUE);
	}


	*pResult = 0;

	return bRet;
}
int CPlayerToolTopBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	cursorHand = ::LoadCursor(NULL, IDC_HAND);
	cursorArrow = ::LoadCursor(NULL, IDC_ARROW);

	GetSystemFontWithScale(&m_statft, 14.0);

	BOOL bExtenedBtn = FALSE;

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	m_nLogDPIY = pFrame->m_nLogDPIY;

	CSUIButton* btnClose = new CSUIButton(L"TOP_CLOSE.BMP" , ALIGN_TOPRIGHT, CRect(1 , 1, 1,1)  , 0, ID_FILE_EXIT, TRUE, 0, 0 ) ;//ID_FILE_BTN_EXIT
	m_btnList.AddTail( btnClose );

	CSUIButton* btnRestore = new CSUIButton(L"TOP_RESTORE.BMP" , ALIGN_TOPRIGHT, CRect(1 , 1, 1,1)  , 0, ID_VIEW_FULLSCREEN, TRUE,  ALIGN_RIGHT,  btnClose, CRect(1,1,1,1)) ;
	//btnRestore->addAlignRelButton( ALIGN_RIGHT,  btnPin1, CRect(1,1,1,1));
	m_btnList.AddTail( btnRestore );

	CSUIButton* btnPin1 = new CSUIButton(L"PINAIL.BMP" , ALIGN_TOPRIGHT, CRect(1 , 1, 1,1)  , 0, ID_ONTOP_ALWAYS, FALSE,ALIGN_RIGHT,  btnClose, CRect(1,1,1,1)) ;
	btnPin1->addAlignRelButton( ALIGN_RIGHT,  btnRestore, CRect(1,1,1,1));
	m_btnList.AddTail( btnPin1 );
	CSUIButton* btnPin2 = new CSUIButton(L"PINAIL2.BMP" , ALIGN_TOPRIGHT, CRect(1 , 1, 1,1)  , 0, ID_ONTOP_NEVER, TRUE, ALIGN_RIGHT,  btnClose, CRect(1,1,1,1) ) ;
	btnPin2->addAlignRelButton( ALIGN_RIGHT,  btnRestore, CRect(1,1,1,1));
	m_btnList.AddTail( btnPin2 );

	CSUIButton* btnFull =  new CSUIButton(L"TOP_FULLSCREEN.BMP" , ALIGN_TOPRIGHT, CRect(1 , 1, 1,1)  , 0, ID_VIEW_FULLSCREEN, FALSE, ALIGN_RIGHT,  btnPin1, CRect(1,1,1,1) ) ;
	btnFull->addAlignRelButton(  ALIGN_RIGHT,  btnPin2, CRect(1,1,1,1));
	m_btnList.AddTail(btnFull);

	if(bExtenedBtn){
		CSUIButton* btnRotate  = new CSUIButton(L"TOP_ROTATE.BMP" , ALIGN_TOPRIGHT, CRect(1 , 1, 1,1)  , 0, ID_ROTATE_90, FALSE, ALIGN_RIGHT, btnFull  , CRect(1,1,1,1)   ) ;
		btnRotate->addAlignRelButton( ALIGN_RIGHT, btnRestore  , CRect(1,1,1,1)  );
		btnRotate->addAlignRelButton( ALIGN_RIGHT, btnPin1  , CRect(1,1,1,1)  );
		btnRotate->addAlignRelButton( ALIGN_RIGHT, btnPin2  , CRect(1,1,1,1)  );
		m_btnList.AddTail(btnRotate);

		m_btnList.AddTail( new CSUIButton(L"TOP_FLIP.BMP" , ALIGN_TOPRIGHT, CRect(1 , 1, 1,1)  , 0, ID_ROTATE_V, FALSE, ALIGN_RIGHT, m_btnList.GetTail() , CRect(1,1,1,1)  ) );

	}

	
	CSUIButton* btnCapture = new CSUIButton(L"TOP_CAPTURE.BMP" , ALIGN_TOPRIGHT, CRect(1 , 1, 1,1)  , 0, ID_FILE_SAVE_IMAGE, FALSE, ALIGN_RIGHT, m_btnList.GetTail() , CRect(1,1,1,1)  ) ;
	btnCapture->addAlignRelButton( ALIGN_RIGHT, btnRestore  , CRect(1,1,1,1)  );
	btnCapture->addAlignRelButton( ALIGN_RIGHT, btnPin1  , CRect(1,1,1,1)  );
	btnCapture->addAlignRelButton( ALIGN_RIGHT, btnPin2  , CRect(1,1,1,1)  );
	m_btnList.AddTail(btnCapture);

	if(bExtenedBtn){
		m_btnList.AddTail( new CSUIButton(L"TOP_TRANS.BMP" , ALIGN_TOPRIGHT, CRect(1 , 1, 1,1)  , 0, ID_SHOWTRANSPRANTBAR, FALSE, ALIGN_RIGHT, m_btnList.GetTail() , CRect(1,1,1,1)  ) );
	}

	m_btnList.AddTail( new CSUIButton(L"TOP_GAMMA.BMP" , ALIGN_TOPRIGHT, CRect(1 , 1, 1,1)  , 0, ID_SHOWCOLORCONTROLBAR, FALSE, ALIGN_RIGHT, m_btnList.GetTail() , CRect(1,1,1,1)  ) );
	
/*
#define ID_VIEW_VF_STRETCH              838
#define ID_VIEW_VF_FROMINSIDE           839
#define ID_VIEW_VF_FROMOUTSIDE          840
#define ID_VIEW_VF_KEEPASPECTRATIO      841*/


	m_btnList.AddTail( new CSUIButton(L"TOP_1X.BMP" , ALIGN_TOPLEFT, CRect(1 , 1, 1,1)  , 0, ID_VIEW_ZOOM_100, FALSE, 0, 0 ) );
	CSUIButton* btn2X =  new CSUIButton(L"TOP_2X.BMP" , ALIGN_TOPLEFT, CRect(1 , 1, 1,1)  , 0, ID_VIEW_ZOOM_200, FALSE, ALIGN_LEFT, m_btnList.GetTail() , CRect(1,1,1,1)  );
	m_btnList.AddTail(btn2X );


	CSUIButton* btnNormal = new CSUIButton(L"TOP_NORMAL.BMP" , ALIGN_TOPLEFT, CRect(1 , 1, 1,1)  , 0, ID_VIEW_VF_FROMINSIDE, FALSE, ALIGN_LEFT, btn2X , CRect(1,1,1,1)  );
	m_btnList.AddTail( btnNormal );

	CSUIButton* btnNormalWider = new CSUIButton(L"TOP_NORMAL_WIDER.BMP" , ALIGN_TOPLEFT, CRect(1 , 1, 1,1)  , 0, ID_VIEW_VF_FROMINSIDE, TRUE, ALIGN_LEFT, btn2X , CRect(1,1,1,1)  );
	m_btnList.AddTail( btnNormalWider );

	CSUIButton* btnLetter  = new CSUIButton(L"TOP_LETTERBOX.BMP" , ALIGN_TOPLEFT, CRect(1 , 1, 1,1)  , 0, ID_VIEW_VF_FROMOUTSIDE, TRUE, ALIGN_LEFT, btn2X , CRect(1,1,1,1)  ) ;
	m_btnList.AddTail( btnLetter);

	CSUIButton* btnLetterWider  = new CSUIButton(L"TOP_LETTERBOX_WIDER.BMP" , ALIGN_TOPLEFT, CRect(1 , 1, 1,1)  , 0, ID_VIEW_VF_FROMOUTSIDE, TRUE, ALIGN_LEFT, btn2X, CRect(1,1,1,1)  ) ;
	m_btnList.AddTail( btnLetterWider);


	CSUIButton* btnAudio  = new CSUIButton(L"TOP_AUDIO.BMP" , ALIGN_TOPLEFT, CRect(1 , 1, 1,1)  , 0, ID_MENU_AUDIO, FALSE, ALIGN_LEFT, btnLetter , CRect(5,1,1,1)  ) ;
	btnAudio->addAlignRelButton( ALIGN_LEFT, btnLetterWider , CRect(5,1,1,1) );
	btnAudio->addAlignRelButton( ALIGN_LEFT, btnNormalWider , CRect(5,1,1,1) );
	btnAudio->addAlignRelButton( ALIGN_LEFT, btnNormal , CRect(5,1,1,1) );
	m_btnList.AddTail( btnAudio);
	
	
	m_btnList.AddTail( new CSUIButton(L"TOP_VIDEO.BMP" , ALIGN_TOPLEFT, CRect(1 , 1, 1,1)  , 0, ID_MENU_VIDEO, FALSE, ALIGN_LEFT, m_btnList.GetTail() , CRect(1,1,1,1)  ) );

	m_toolTip.Create(this);
	m_ti.cbSize = sizeof(m_ti);
	m_ti.uFlags = 0  ;//TTF_TRACK|TTF_ABSOLUTE
	m_ti.hwnd = m_hWnd;
	m_ti.hinst = NULL;
	m_ti.uId = (UINT)0;
	m_ti.lpszText = LPSTR_TEXTCALLBACK;
	m_ti.rect.left = 0;    
	m_ti.rect.top = 0;
	m_ti.rect.right = 0;
	m_ti.rect.bottom = 0;

	m_toolTip.SendMessage(TTM_ADDTOOL, 0, (LPARAM)&m_ti);

	return 0;
}
void CPlayerToolTopBar::ReCalcBtnPos(){
	UpdateButtonStat();
	CRect rc;
	GetWindowRect(&rc);
	m_btnList.OnSize( rc);
	POSITION pos = m_btnList.GetHeadPosition();
	while(pos){
		break;
	}
	
}
void CPlayerToolTopBar::UpdateButtonStat(){
	CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());
	BOOL ontop = !!AfxGetAppSettings().iOnTop ;
	BOOL fullscreen = pFrame->m_fFullScreen;
	BOOL bCaptionHidden = pFrame->IsCaptionMenuHidden();
	
	m_btnList.SetHideStat( ID_ONTOP_NEVER , !ontop );
	m_btnList.SetHideStat( ID_ONTOP_ALWAYS , ontop );

	m_btnList.SetHideStat( ID_FILE_EXIT , !fullscreen && !bCaptionHidden );
	m_btnList.SetHideStat( L"TOP_FULLSCREEN.BMP" , fullscreen  );
	m_btnList.SetHideStat( L"TOP_RESTORE.BMP" , !fullscreen );

	BOOL bViewFROMOUTSIDE = (AfxGetAppSettings().iDefaultVideoSize == 5);

	m_btnList.SetHideStat( L"TOP_LETTERBOX_WIDER.BMP" , pFrame->m_fScreenHigherThanVideo || bViewFROMOUTSIDE);
	m_btnList.SetHideStat( L"TOP_LETTERBOX.BMP" , !pFrame->m_fScreenHigherThanVideo || bViewFROMOUTSIDE);
	m_btnList.SetHideStat( L"TOP_NORMAL_WIDER.BMP" , pFrame->m_fScreenHigherThanVideo || !bViewFROMOUTSIDE);
	m_btnList.SetHideStat( L"TOP_NORMAL.BMP" , !pFrame->m_fScreenHigherThanVideo || !bViewFROMOUTSIDE);

}
void CPlayerToolTopBar::OnMove(int x, int y)
{
	__super::OnMove(x, y);

	ReCalcBtnPos();
	Invalidate();
}

void CPlayerToolTopBar::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	
	AppSettings& s = AfxGetAppSettings();
	
	CRect rcClient;
	GetClientRect(&rcClient);
	CMemoryDC hdc(&dc, rcClient);

	CRect rc;
	GetWindowRect(&rc);

	CRect rcBottomSqu = rcClient;
	rcBottomSqu.top = rcBottomSqu.bottom - 1;
	//hdc.FillSolidRect(rcBottomSqu, NEWUI_COLOR_BG);

	CRect rcUpperSqu = rcClient;
	rcUpperSqu.bottom--;
	//rcUpperSqu.right--;
	hdc.FillSolidRect(rcUpperSqu, s.GetColorFromTheme(_T("TopToolBarBG"), RGB(61,65,69) ));

	hdc.FillSolidRect(rcBottomSqu,s.GetColorFromTheme(_T("TopToolBarBorder"), RGB(89,89,89)));

	//rcBottomSqu = rcClient;
	//rcBottomSqu.left = rcBottomSqu.right - 1;
	//hdc.FillSolidRect(rcBottomSqu, RGB(89,89,89));
	UpdateButtonStat();
	m_btnList.PaintAll(&hdc, rc);
	
}

void CPlayerToolTopBar::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	ReCalcBtnPos();
	Invalidate();
}


BOOL CPlayerToolTopBar::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return __super::OnEraseBkgnd(pDC);
}

void CPlayerToolTopBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());
	
	ReleaseCapture();

	CRect rc;
	GetWindowRect(&rc);

	CPoint xpoint = point + rc.TopLeft() ;
	UINT ret = m_btnList.OnHitTest(xpoint,rc);
	if( m_btnList.HTRedrawRequired ){
		if(ret)
			pFrame->PostMessage( WM_COMMAND, ret);
		m_toolTip.SendMessage(TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_ti);
		
		Invalidate();
	}
	m_nItemToTrack = ret;

	KillTimer(IDT_CLOSE);
	//SetTimer(IDT_CLOSE, 8000 , NULL);


	//	__super::OnLButtonUp(nFlags, point);
	
	return;
	//__super::OnLButtonUp(nFlags, point);
}

void CPlayerToolTopBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	
	CRect rc;
	GetWindowRect(&rc);

	point += rc.TopLeft() ;
	UINT ret = m_btnList.OnHitTest(point,rc);
	if( m_btnList.HTRedrawRequired ){
		//if(ret)
		//	SetCapture();
		Invalidate();
	}
	m_nItemToTrack = ret;

	KillTimer(IDT_CLOSE);
	//SetTimer(IDT_CLOSE, 8000 , NULL);

	//__super::OnLButtonDown(nFlags, point);
}

void CPlayerToolTopBar::OnNcPaint()
{
	// TODO: Add your message handler code here
	// Do not call CToolBar::OnNcPaint() for painting messages
}

void CPlayerToolTopBar::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if(IDT_TIPS == nIDEvent){
		KillTimer(IDT_TIPS);
		if(m_nItemToTrack){
			CPoint pt;
			if(m_lastTipItem != m_nItemToTrack) {
				
				GetCursorPos(&pt);
				//ClientToScreen(&pt);
				pt.y+=10;
				//m_ti.uId = ret;

				m_toolTip.SendMessage(TTM_TRACKPOSITION, 0, (LPARAM)MAKELPARAM(pt.x, pt.y));
				m_toolTip.SendMessage(TTM_TRACKACTIVATE, TRUE, (LPARAM)&m_ti);
			}
		}
	}else if(IDT_CLOSE == nIDEvent){
		KillTimer(IDT_CLOSE);
		ShowWindow(SW_HIDE);
	}
	__super::OnTimer(nIDEvent);
}
static CPoint m_lastMouseMove;
void CPlayerToolTopBar::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CSize diff = m_lastMouseMove - point;
	CMainFrame* pFrame = ((CMainFrame*)AfxGetMainWnd());
	
	BOOL bMouseMoved =  diff.cx || diff.cy ;
	if(bMouseMoved){
		KillTimer(IDT_CLOSE);
		pFrame->KillTimer(pFrame->TIMER_FULLSCREENMOUSEHIDER);

		if(pFrame->IsSomethingLoaded()){
			pFrame->SetTimer(pFrame->TIMER_FULLSCREENMOUSEHIDER, 5000, NULL); 
		}
		
		//SetTimer(IDT_CLOSE, 8000 , NULL);
		m_lastMouseMove = point;
	}

	CRect rc;
	//CMainFrame* pFrame = ((CMainFrame*)GetParentFrame());
	GetWindowRect(&rc);
	point += rc.TopLeft() ;
	

	if( bMouseMoved){

		UINT ret = m_btnList.OnHitTest(point,rc);
		m_nItemToTrack = ret;
		if(ret){
			if( GetCursor() == NULL )
				SetCursor(cursorHand);

			SetTimer(IDT_TIPS, 100 , NULL);
			
		}else{
			m_toolTip.SendMessage(TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_ti);
			pFrame->m_tip.SetTips(_T(""));
			if( GetCursor() == NULL ) 
				SetCursor(cursorArrow);
		}
		if( m_btnList.HTRedrawRequired ){
			Invalidate();
		}
	}

	__super::OnMouseMove(nFlags, point);
}

INT_PTR CPlayerToolTopBar::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	// TODO: Add your specialized code here and/or call the base class
		
	if(!pTI){
		return -1;
	}

	
	UINT ret = m_nItemToTrack;
	
	if(ret){

		
		pTI->hwnd = AfxGetMainWnd()->m_hWnd;
		pTI->uId = (UINT) (ret);
		//pTI->uFlags = TTF_SUBCLASS TTF_IDISHWND;
		pTI->lpszText = LPSTR_TEXTCALLBACK;
		RECT rcClient;
		GetClientRect(&rcClient);
		//SVP_LogMsg3("Tooltip %d" , ret);
		pTI->rect = rcClient;

		
		return pTI->uId;

	}
	return -1;
	
}

void CPlayerToolTopBar::OnEnable(BOOL bEnable)
{
	//CWnd::OnEnable(TRUE);

	return;
	// TODO: Add your message handler code here
}

BOOL CPlayerToolTopBar::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: Add your specialized code here and/or call the base class
	
	return __super::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);

}

BOOL CPlayerToolTopBar::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class

	
	return CWnd::PreTranslateMessage(pMsg);
}

LRESULT CPlayerToolTopBar::OnNcHitTest(CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	return CWnd::OnNcHitTest(point);
}

void CPlayerToolTopBar::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{

	__super::OnNcCalcSize(bCalcValidRects, lpncsp);
	
}

void CPlayerToolTopBar::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	//CWnd::OnClose();
}

void CPlayerToolTopBar::OnDestroy()
{
	CWnd::OnDestroy();

	// TODO: Add your message handler code here
}
void CPlayerToolTopBar::OnRealClose(){
	__super::OnClose();
}
void CPlayerToolTopBar::OnMouseLeave()
{
	// TODO: Add your message handler code here and/or call default
	m_toolTip.SendMessage(TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_ti);
	m_nItemToTrack = 0 ;
	CWnd::OnMouseLeave();
}

void CPlayerToolTopBar::OnShowWindow(BOOL bShow, UINT nStatus)
{
	if(bShow != SW_SHOW){
		m_toolTip.SendMessage(TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_ti);
		m_nItemToTrack = 0 ;
	}
	
	CWnd::OnShowWindow(bShow, nStatus);

}

void CPlayerToolTopBar::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{

//
	CWnd::OnActivate(WA_INACTIVE, pWndOther, bMinimized); //


}

BOOL CPlayerToolTopBar::OnNcActivate(BOOL bActive)
{
	// TODO: Add your message handler code here and/or call default
	//CWnd::
	bActive = false;
	return __super::OnNcActivate(bActive);;
}

void CPlayerToolTopBar::OnSetFocus(CWnd* pOldWnd)
{
	
	__super::OnSetFocus(pOldWnd);

	//AfxGetMainWnd()->SendMessage(WM_SETFOCUS, (WPARAM )m_hWnd, NULL);

	//::SetForegroundWindow( AfxGetMainWnd()->m_hWnd );

	// TODO: Add your message handler code here
}

void CPlayerToolTopBar::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	//lpwndpos->flags |= SWP_NOACTIVATE;
	CWnd::OnWindowPosChanging(lpwndpos);

	// TODO: Add your message handler code here
}

void CPlayerToolTopBar::OnActivateApp(BOOL bActive, DWORD dwThreadID)
{
	//bActive = false;
	CWnd::OnActivateApp(bActive, dwThreadID);

	// TODO: Add your message handler code here
}
