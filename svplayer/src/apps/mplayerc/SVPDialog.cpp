// SVPDialog.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "SVPDialog.h"


// CSVPDialog

IMPLEMENT_DYNAMIC(CSVPDialog, CWnd)

CSVPDialog::CSVPDialog()
: m_bFocused(0)
{
	AppSettings& s = AfxGetAppSettings();
	m_btnClose.m_btnMode = 1; //x
	m_btnClose.m_borderColor = s.GetColorFromTheme(_T("FloatDialogCloseButtonBorder"), 0x787878);
	m_bgColor = s.GetColorFromTheme(_T("FloatDialogBG"), 0x333333);
	m_borderColor = s.GetColorFromTheme(_T("FloatDialogBorder"), 0xefefef);
}

CSVPDialog::~CSVPDialog()
{
}


BEGIN_MESSAGE_MAP(CSVPDialog, CWnd)
	ON_WM_ENABLE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON1,  OnButtonClose)
	ON_WM_MOVE()
	ON_WM_TIMER()
	ON_WM_SHOWWINDOW()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

// CSVPDialog message handlers
void CSVPDialog::OnButtonClose(){
	ShowWindow(SW_HIDE);
}

void CSVPDialog::OnEnable(BOOL bEnable)
{
	return ;
	//CWnd::OnEnable(bEnable);

}

void CSVPDialog::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	CRect r,cr;

	{ //New UI
		CRect rc;
		GetWindowRect(&rc);
		rc-=rc.TopLeft();



		// destroy old region
		if((HRGN)m_rgn)
		{
			m_rgn.DeleteObject();
		}

		m_rgn.CreateRoundRectRgn(0,0,rc.Width()-1,rc.Height()-1, 3,3);                 // rounded rect w/50 pixel corners

		m_rgnBorder.CreateRoundRectRgn(1,1,rc.Width()-2,rc.Height()-2, 3,3);            

		SetWindowRgn(m_rgn,TRUE);

		m_btnClose.MoveWindow( rc.right-16,rc.top+3, 12, 12);
	}

	
}

void CSVPDialog::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect rcClient;
	GetClientRect(&rcClient);
	CMemoryDC hdc(&dc, rcClient);
	hdc.FillSolidRect(rcClient, m_bgColor);

	CRect rc;
	GetWindowRect(&rc);
	rc-=rc.TopLeft();
	hdc.FrameRgn(&m_rgnBorder, &m_brushBorder,1,1);

}

int CSVPDialog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	GetSystemFontWithScale(&m_statft, 14.0);

	m_brushBorder.CreateSolidBrush(m_borderColor);

	m_btnClose.Create( _T("x"), WS_VISIBLE|WS_CHILD|BS_FLAT|BS_VCENTER|BS_CENTER, CRect(0,0,0,0) , this, IDC_BUTTON1);
	
	return 0;
}

void CSVPDialog::OnClose()
{
	ShowWindow(SW_HIDE);
	//CWnd::OnClose();
}

void CSVPDialog::OnRealClose(){
	CWnd::OnClose();
}

BOOL CSVPDialog::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class

	if(( pMsg->message == WM_KEYDOWN || (pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MYMOUSELAST))){

		KillTimer(IDT_CLOSE);
		SetTimer(IDT_CLOSE, 3000, NULL);
	}
	
	return CWnd::PreTranslateMessage(pMsg);
}

void CSVPDialog::OnMove(int x, int y)
{
	CWnd::OnMove(x, y);

	// TODO: Add your message handler code here
}

void CSVPDialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	switch(nIDEvent){
		case IDT_CLOSE:
			{
                HWND hMenu = ::FindWindow(_T("#32768"), NULL);
                //SVP_LogMsg6( "IsMenuUp %d" , (bool)(hMenu && ::IsWindowVisible(hMenu)));
                
                if(! (bool)(hMenu && ::IsWindowVisible(hMenu))) //detect if a popup menu is up
                {
				    KillTimer(IDT_CLOSE);
				    OnClose();
                }
			}
			break;
	}
	CWnd::OnTimer(nIDEvent);
}

void CSVPDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CWnd::OnShowWindow(bShow, nStatus);

	KillTimer(IDT_CLOSE);
	
	if(bShow)
		SetTimer(IDT_CLOSE, 3000, NULL);
	// TODO: Add your message handler code here
}

void CSVPDialog::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);

	m_bFocused = TRUE;

	AfxGetMainWnd()->SendMessage(WM_NCACTIVATE, TRUE, NULL);;
}

void CSVPDialog::OnKillFocus(CWnd* pNewWnd)
{
	CWnd::OnKillFocus(pNewWnd);

	m_bFocused = FALSE;

	
}
