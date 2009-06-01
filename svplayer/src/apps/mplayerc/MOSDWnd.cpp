// MOSDWnd.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "MOSDWnd.h"
#include "ChildView.h"
#include "MainFrm.h"
#include "../../svplib/svplib.h"
// MOSDWnd

IMPLEMENT_DYNAMIC(CMOSDWnd, CWnd)

CMOSDWnd::CMOSDWnd()
{

}

CMOSDWnd::~CMOSDWnd()
{
}


BEGIN_MESSAGE_MAP(CMOSDWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_PAINT()
END_MESSAGE_MAP()



// MOSDWnd message handlers



int CMOSDWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	 GetSystemFontWithScale(&m_statft, 14.0);
	
	m_msgList.AddTail(_T("haha"));
	CountSize();
	return 0;
}
void CMOSDWnd::SendOSDMsg(CString szMsg, UINT timeOut ){

	KillTimer(IDT_HIDE);
	m_msgList.AddTail(szMsg);
	if(m_msgList.GetCount() > 5){
		m_msgList.RemoveHead();
	}
	m_osdStr = szMsg;
	if(timeOut<= 1000){
		timeOut = 2000;
	}
	SetTimer(IDT_HIDE , timeOut, NULL);
	CString szLog;
	//szLog.Format(_T("%s %d")  ,szMsg, timeOut );
	//SVP_LogMsg(szLog);
	CountSize();
	//CMainFrame* pFrame = (CMainFrame*)GetParentFrame();
	((CChildView*)m_wndView)->SetMyRgn();
	Invalidate();
	//pFrame->m_wndView.Invalidate();

	return;
}
BOOL CMOSDWnd::CreateEx(DWORD dwExStyle, LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, LPVOID lpParam)
{
	// TODO: Add your specialized code here and/or call the base class

	return CWnd::CreateEx(dwExStyle, lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, lpParam);
}
void CMOSDWnd::CountSize(){
	if(m_osdStr.IsEmpty()){
		mSize.cx = 0;
		return;
	}
	CPaintDC dc(this);
	HFONT holdft = (HFONT)dc.SelectObject(m_statft); 
	//m_osdStr = m_msgList.GetTail();
	mSize = dc.GetTextExtent(m_osdStr); 
	mSize.cx += 4;
	CRect rc;
	GetWindowRect(&rc);
	CMainFrame* pFrame = (CMainFrame*)GetParentFrame();
	if(pFrame){
		pFrame->rePosOSD();
	}
	/*
	CRect rcMain;
		pFrame->GetClientRect(&rcMain);
		
		ClientToScreen(&rcMain);
		rc -= rcMain.TopLeft();
		//GetClientRect(&rc);
		rc.right = rc.left + mSize.cx;
		MoveWindow(rc);*/
	
	
}
void CMOSDWnd::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	switch(nIDEvent){
		case IDT_HIDE:
			KillTimer(IDT_HIDE);
			mSize = CSize(0,0);
			m_osdStr.Empty();
			CMainFrame* pFrame = (CMainFrame*)GetParentFrame();
			pFrame->rePosOSD();
			((CChildView*)m_wndView)->SetMyRgn();
			break;
	}
	CWnd::OnTimer(nIDEvent);
}

void CMOSDWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CWnd::OnPaint() for painting messages

	CRect rcClient;
	GetClientRect(&rcClient);
	CMemoryDC hdc(&dc, rcClient);
	hdc.FillSolidRect(rcClient, RGB(0x3f,0x3f,0x3f) );

	HFONT holdft = (HFONT)hdc.SelectObject(m_statft);
	hdc.SetTextColor(0xffffff);
	//m_osdStr = m_msgList.GetTail();
	if(!m_osdStr.IsEmpty()){
		//SVP_LogMsg(m_osdStr);
		rcClient.left += 2;

		::DrawText(hdc, m_osdStr, m_osdStr.GetLength(), rcClient,  DT_LEFT|DT_SINGLELINE| DT_VCENTER);
		
	}
	hdc.SelectObject(holdft);

}
