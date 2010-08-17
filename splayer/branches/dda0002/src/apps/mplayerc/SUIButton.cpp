
#include "SUIButton.h"
#include "../../svplib/svplib.h"
#include "../../svplib/SVPToolBox.h"

CSUIButton::CSUIButton(LPCTSTR szBmpName, int iAlign, CRect marginTownd 
					   , BOOL bNotButton, UINT htMsgID, BOOL bHide 
					   ,UINT alignToButton  , CSUIButton * relativeToButton , CRect marginToBtn ) : 
m_stat(0) ,
m_lastBtnDownStat(0)
{
	m_NotButton = bNotButton;
	m_marginTownd  = marginTownd;
	m_iAlign = iAlign;
	m_htMsgID = htMsgID;
	CString szBmpPath(szBmpName);
	szBmpPath = CString(_T("skins\\")) + szBmpPath.Left(szBmpPath.GetLength()-4) + _T(".png");
	CSVPToolBox svpToolBox;
	szBmpPath = svpToolBox.GetPlayerPath(szBmpPath);
	BOOL bExtLoaded = false;
	//SVP_LogMsg(szBmpPath);
	if(svpToolBox.ifFileExist( szBmpPath)){
		m_png.Load( szBmpPath );
		if(m_png.IsDIBSection()){
			this->Attach((HBITMAP)m_png);
			bExtLoaded = true;
		}
	}
	if(!bExtLoaded)
		this->LoadImage(szBmpName);
	
	m_szBmpName = szBmpName;
	m_hide = bHide;


	
	CountDPI();
	
	addAlignRelButton(alignToButton, relativeToButton , marginToBtn);
	
}

void CSUIButton::CountDPI(){
	if(!nLogDPIX){
		CDC ScreenDC;
		ScreenDC.CreateIC(_T("DISPLAY"), NULL, NULL, NULL);
		nLogDPIX = ScreenDC.GetDeviceCaps(LOGPIXELSX), nLogDPIY = ScreenDC.GetDeviceCaps(LOGPIXELSY);
	}

	m_orgbtnSize = m_btnSize;

	m_btnSize.cx = nLogDPIX  * m_orgbtnSize.cx / 96;
	m_btnSize.cy = nLogDPIY * m_orgbtnSize.cy / 96;
}
CSUIButton::CSUIButton(UINT Imgid, int iAlign, CRect marginTownd 
					   , BOOL bNotButton, UINT htMsgID, BOOL bHide 
					   ,UINT alignToButton  , CSUIButton * relativeToButton  , CRect marginToBtn) : 
m_stat(0) 
{
	m_NotButton = bNotButton;
	m_marginTownd  = marginTownd;
	m_iAlign = iAlign;
	m_htMsgID = htMsgID;
	
	if( m_png.LoadFromResource( Imgid ) ){
		if(m_png.IsDIBSection()){
			this->Attach((HBITMAP)m_png);
		}
	}
	
	CountDPI();
	//m_szBmpName = MAKEINTRESOURCE(Imgid);

	m_hide = bHide;
	addAlignRelButton(alignToButton, relativeToButton , marginToBtn);

}


LONG CSUIButton::CalcRealMargin(LONG Mlen, LONG bW, LONG wW)
{
	if(Mlen >= 0){
		return Mlen;
	}
	else
	{
		Mlen = -Mlen;
		return ( wW * Mlen / 100) - bW / 2;
	}
}
int CSUIButton::OnHitTest(CPoint pt , int bLBtnDown){
	if(m_hide || m_NotButton || m_stat == 3){
		return 0;
	}
	int old_stat = m_stat;
	if(bLBtnDown < 0){
		bLBtnDown = m_lastBtnDownStat;
	}else{
		m_lastBtnDownStat = bLBtnDown;
	}
	if (m_rcHitest.PtInRect(pt) && bLBtnDown)  m_stat = 2; else if (m_rcHitest.PtInRect(pt)) m_stat = 1; else m_stat = 0;
	
	if(m_stat == old_stat){
		return -1;
	}else{
		return 1; //require redraw
	}
	
}
void CSUIButton::addAlignRelButton( UINT alignToButton  , CSUIButton * relativeToButton , CRect rmToBtn){
	if(alignToButton && relativeToButton){
		btnAlignList.AddTail(new CBtnAlign(alignToButton , (INT_PTR)relativeToButton , rmToBtn) );
	}

}
void CSUIButton::OnSize(CRect WndRect)
{

	LONG left = CalcRealMargin(m_marginTownd.left , m_btnSize.cx , WndRect.Width());
	LONG top = CalcRealMargin(m_marginTownd.top , m_btnSize.cy , WndRect.Height());
	LONG right = CalcRealMargin(m_marginTownd.right , m_btnSize.cx , WndRect.Width());
	LONG bottom =  CalcRealMargin(m_marginTownd.bottom , m_btnSize.cy , WndRect.Height());
	
	switch (m_iAlign){
		case ALIGN_TOPLEFT:
			m_rcHitest = CRect ( WndRect.left + left,
								WndRect.top + top,
								WndRect.left + m_btnSize.cx + left,
								WndRect.top+ top+m_btnSize.cy);
			
			break;
		case ALIGN_TOPRIGHT:
			m_rcHitest = CRect ( WndRect.right - m_btnSize.cx - right,
								WndRect.top + top,
								WndRect.right-right,
								WndRect.top+ top+m_btnSize.cy);

			break;
		case ALIGN_BOTTOMLEFT:
			m_rcHitest = CRect ( WndRect.left + left,
								WndRect.bottom - m_btnSize.cy - bottom,
								WndRect.left + m_btnSize.cx + left,
								WndRect.bottom - bottom);
			
			break;
		case ALIGN_BOTTOMRIGHT:
			m_rcHitest = CRect ( WndRect.right - m_btnSize.cx - right,
								WndRect.bottom - m_btnSize.cy - bottom,
								WndRect.right-right,
								WndRect.bottom - bottom);
			
			break;
	}
	
	POSITION pos = btnAlignList.GetHeadPosition();
	while(pos){
		CBtnAlign* bAlignInfo = btnAlignList.GetNext(pos);
		CSUIButton* bRBtn = (CSUIButton*) bAlignInfo->bBtn;
		if( bRBtn->m_hide ){
			continue;
		}

		if( bAlignInfo->iAlign&ALIGN_TOP){
			int mTop = bAlignInfo->marginToBtn.top;
			if(mTop <= 0){ mTop = DEFAULT_MARGIN; }
			if( (bRBtn->m_rcHitest.bottom + mTop) > m_rcHitest.top){
				m_rcHitest.MoveToY( bRBtn->m_rcHitest.bottom  + mTop);
			}
		}
		if(bAlignInfo->iAlign&ALIGN_BOTTOM){
			int mBottom = bAlignInfo->marginToBtn.bottom;
			if(mBottom <= 0){ mBottom = DEFAULT_MARGIN; }
			if( (bRBtn->m_rcHitest.top - mBottom)  < m_rcHitest.bottom){
				m_rcHitest.MoveToY( bRBtn->m_rcHitest.top - mBottom - m_rcHitest.Height() );
			}
		}
		if(bAlignInfo->iAlign&ALIGN_LEFT){
			int mLeft = bAlignInfo->marginToBtn.left;
			if(mLeft <= 0){ mLeft = DEFAULT_MARGIN; }
			if( (bRBtn->m_rcHitest.right + mLeft) > m_rcHitest.left){
				m_rcHitest.MoveToX( bRBtn->m_rcHitest.right + mLeft);
			}
		}
		if(bAlignInfo->iAlign&ALIGN_RIGHT){
			int mRight = bAlignInfo->marginToBtn.right;
			if(mRight <= 0){ mRight = DEFAULT_MARGIN; }
			if( (bRBtn->m_rcHitest.left - mRight)  < m_rcHitest.right){
				m_rcHitest.MoveToX( bRBtn->m_rcHitest.left - mRight - m_rcHitest.Width() );
			}
		}

	}

	//CString szLog;
	//szLog.Format(_T("%d %d %d %d %d %d %d %d %s"), WndRect.left, WndRect.top, WndRect.right, WndRect.bottom,
	//	m_rcHitest.left, m_rcHitest.top, m_rcHitest.right, m_rcHitest.bottom, m_szBmpName);
	//SVP_LogMsg(szLog);
}
void CSUIButton::OnPaint(CMemoryDC *hDC, CRect rc){
	if(m_hide) return;
	rc = m_rcHitest - rc.TopLeft();
	BLENDFUNCTION bf = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};//
	CDC dcBmp;
	dcBmp.CreateCompatibleDC(hDC);
	HBITMAP holdBmp = (HBITMAP)dcBmp.SelectObject(m_bitmap);
	BOOL ret = hDC->AlphaBlend(rc.left, rc.top, rc.Width(), rc.Height(),
		&dcBmp, 0, m_orgbtnSize.cy * m_stat, m_orgbtnSize.cx, m_orgbtnSize.cy, bf);
	dcBmp.SelectObject(holdBmp);
	dcBmp.DeleteDC();
	
	//SVP_LogMsg5(_T("%d %d %d %d %d %d %d %d %d Paint %s"),ret , rc.left, rc.top, rc.Width(), rc.Height(),
		//0, m_btnSize.cy * m_stat, m_btnSize.cx, m_btnSize.cy, m_szBmpName);
}
HBITMAP CSUIButton::SUILoadImage(LPCTSTR szBmpName){
	HBITMAP hGTmp = NULL;
	if(CMPlayerCApp::m_hResDll){
		hGTmp = (HBITMAP)::LoadImage(CMPlayerCApp::m_hResDll, szBmpName, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR|LR_CREATEDIBSECTION);
		if(!hGTmp){
			hGTmp = (HBITMAP)::LoadImage(AfxGetApp()->m_hInstance, szBmpName, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR|LR_CREATEDIBSECTION);
		}
	}else{
		hGTmp = (HBITMAP)::LoadImage(GetModuleHandle(NULL), szBmpName, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR|LR_CREATEDIBSECTION);
	}

	return hGTmp;
}
void CSUIButton::LoadImage(LPCTSTR szBmpName){

	
	this->Attach(SUILoadImage(szBmpName));
}

void CSUIButton::Attach(HBITMAP bmp){
	m_bitmap.Attach(bmp);
	
	PreMultiplyBitmap(m_bitmap,m_btnSize,m_NotButton);
}
void CSUIButton::PreMultiplyBitmap( CBitmap& bmp , CSize& sizeBmp, BOOL NotButton)
{

	BITMAP bm;
	bmp.GetBitmap(&bm);
	sizeBmp.cx = bm.bmWidth;
	sizeBmp.cy = bm.bmHeight;
	if(!NotButton){
		sizeBmp.cy = sizeBmp.cy /4;
	}
	if(bm.bmBitsPixel != 32){
		return;
	}
	for (int y=0; y<bm.bmHeight; y++)
	{
		BYTE * pPixel = (BYTE *) bm.bmBits + bm.bmWidth * 4 * y;
		for (int x=0; x<bm.bmWidth; x++)
		{
			pPixel[0] = pPixel[0] * pPixel[3] / 255; 
			pPixel[1] = pPixel[1] * pPixel[3] / 255; 
			pPixel[2] = pPixel[2] * pPixel[3] / 255; 
			pPixel += 4;
		}
	}
}


/*CSUIBtnList*/
CSUIBtnList::CSUIBtnList()
{
}

CSUIBtnList::~CSUIBtnList()
{
}
int CSUIBtnList::GetMaxHeight(){
    int nHeight = 0;
    POSITION pos = GetHeadPosition();
    while(pos){
        CSUIButton* cBtn =  GetNext(pos);
        nHeight = max(nHeight, cBtn->m_orgbtnSize.cy);
    }
    return nHeight;
}
void CSUIBtnList::SetDisableStat(UINT iMsgID, BOOL bDisable){
	POSITION pos = GetHeadPosition();
	while(pos){
		CSUIButton* cBtn =  GetNext(pos);
		if( iMsgID == cBtn->m_htMsgID ){
			if(bDisable)
				cBtn->m_stat = 3;
			else
				cBtn->m_stat = 0;
			break;
		}
	}

}
CRect CSUIBtnList::GetHTRect(UINT iMsgID){
	POSITION pos = GetHeadPosition();
	while(pos){
		CSUIButton* cBtn =  GetNext(pos);
		if( iMsgID == cBtn->m_htMsgID ){
			return cBtn->m_rcHitest;
			break;
		}
	}
	return NULL;
}
void CSUIBtnList::SetClickedStat(UINT iMsgID, BOOL bClicked){
	POSITION pos = GetHeadPosition();
	while(pos){
		CSUIButton* cBtn =  GetNext(pos);
		if( iMsgID == cBtn->m_htMsgID ){
			if(bClicked)
				cBtn->m_stat = 2;
			else
				cBtn->m_stat = 0;
			break;
		}
	}
}
void CSUIBtnList::SetHideStat(POSITION pos, BOOL bHide){
	CSUIButton* cBtn = GetAt(pos);
	if(cBtn)
		cBtn->m_hide = bHide;

	
}
void CSUIBtnList::SetHideStat(UINT iMsgID, BOOL bHide){
	POSITION pos = GetHeadPosition();
	while(pos){
		CSUIButton* cBtn =  GetNext(pos);
		if( iMsgID == cBtn->m_htMsgID ){
			cBtn->m_hide = bHide;
			//break;
		}
	}

}
void CSUIBtnList::SetHideStat(LPCTSTR szBmpName, BOOL bHide){
	POSITION pos = GetHeadPosition();
	while(pos){
		CSUIButton* cBtn =  GetNext(pos);
		if( cBtn->m_szBmpName.Compare(szBmpName) == 0 ){
			cBtn->m_hide = bHide;
			break;
		}
	}
}

UINT CSUIBtnList::OnHitTest(CPoint pt , CRect rc, int bLBtnDown){
	POSITION pos = GetHeadPosition();
	UINT iMsg = 0;
	HTRedrawRequired = FALSE;
	while(pos){
		CSUIButton* cBtn =  GetNext(pos);
		int ret = cBtn->OnHitTest(pt , bLBtnDown) ;
		if(ret == 1)
			HTRedrawRequired = TRUE;
		
		if(ret != 0 && cBtn->m_stat != 0)
			iMsg = cBtn->m_htMsgID;

	}
	return iMsg;
}
void CSUIBtnList::ClearStat(){
	POSITION pos = GetHeadPosition();
	while(pos){
		CSUIButton* cBtn =  GetNext(pos);
		cBtn->m_stat = 0;
	}
}
void CSUIBtnList::OnSize(CRect WndRect){
	POSITION pos = GetHeadPosition();
	while(pos){
		CSUIButton* cBtn =  GetNext(pos);
		cBtn->OnSize(WndRect);
	}
}	
void CSUIBtnList::PaintAll(CMemoryDC *hDC, CRect rc){

	POSITION pos = GetHeadPosition();
	while(pos){
		CSUIButton* cBtn =  GetNext(pos);
		cBtn->OnPaint(hDC,  rc);
	}
}
