
#include "stdafx.h"
#include "SUIButton.h"
#include "../../svplib/svplib.h"
#include "../../svplib/SVPToolBox.h"
#include <ResLoader.h>
#include "GUIConfigManage.h"
#include "ButtonManage.h"
#include "MakeMultiplyBmp.h"
CSUIButton::CSUIButton(LPCTSTR szBmpName, int iAlign, CRect marginTownd 
					   , BOOL bNotButton, UINT htMsgID, BOOL bHide 
					   ,UINT alignToButton  , CSUIButton * relativeToButton , CRect marginToBtn
             ,int  hidewidth, CString buttonname) : 
m_stat(0) ,
m_lastBtnDownStat(0),
m_bsingleormultiply(0)
{
  m_orgbtnSize.SetSize(0, 0);
  m_btnSize.SetSize(0, 0);

	m_NotButton = bNotButton;
	m_marginTownd  = marginTownd;
	m_iAlign = iAlign;
	m_htMsgID = htMsgID;

  ResLoader rlResLoader;
  if (szBmpName != L"NOBMP")
  {  
    HBITMAP hbitmap = rlResLoader.LoadBitmap(szBmpName);
    if (hbitmap)
      this->Attach(hbitmap);
  }

  m_szBmpName = szBmpName;
	m_hide = bHide;
	m_hidewidth = hidewidth;
  m_buttonname = buttonname;
	CountDPI();
	
	addAlignRelButton(alignToButton, relativeToButton , marginToBtn);
}

void CSUIButton::CountDPI(){
	if(nLogDPIX == 0){
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
					   ,UINT alignToButton  , CSUIButton * relativeToButton  , CRect marginToBtn
             ,int  hidewidth, CString buttonname) : 
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
  m_hidewidth = hidewidth;
  m_buttonname = buttonname;
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

	if(m_currenthide || m_NotButton || m_stat == 3)
  {
		return 0;
	}

	int old_stat = m_stat;
	if(bLBtnDown < 0)
  {
		bLBtnDown = m_lastBtnDownStat;
	}
  else
  {
		m_lastBtnDownStat = bLBtnDown;
	}

	if (m_rcHitest.PtInRect(pt) && bLBtnDown)
  {
    m_stat = 2;
  }
  else if (m_rcHitest.PtInRect(pt))
  {
    m_stat = 1;
  }
  else
  {
    m_stat = 0;
  }

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
		if( bRBtn->m_currenthide ){
			continue;
		}

		if( bAlignInfo->iAlign==ALIGN_TOP){
			int mTop = bAlignInfo->marginToBtn.top;
			if(mTop <= 0){ mTop = DEFAULT_MARGIN; }
			if( (bRBtn->m_rcHitest.bottom + mTop) > m_rcHitest.top){
				m_rcHitest.MoveToY( bRBtn->m_rcHitest.bottom  + mTop);
			}
		}
		if(bAlignInfo->iAlign==ALIGN_BOTTOM){
			int mBottom = bAlignInfo->marginToBtn.bottom;
			if(mBottom <= 0){ mBottom = DEFAULT_MARGIN; }
			if( (bRBtn->m_rcHitest.top - mBottom)  < m_rcHitest.bottom){
				m_rcHitest.MoveToY( bRBtn->m_rcHitest.top - mBottom - m_rcHitest.Height() );
			}
		}
		if(bAlignInfo->iAlign==ALIGN_LEFT){
			int mLeft = bAlignInfo->marginToBtn.left;
			if(mLeft <= 0){ mLeft = DEFAULT_MARGIN; }
			if( (bRBtn->m_rcHitest.right + mLeft) > m_rcHitest.left){
				m_rcHitest.MoveToX( bRBtn->m_rcHitest.right + mLeft);
			}
		}
		if(bAlignInfo->iAlign==ALIGN_RIGHT){
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
  m_dc = hDC;
	if (m_currenthide)
    return;
  
  rc = m_rcHitest - rc.TopLeft();

  if (m_buttonname == L"PLAYTIME" )
  {
    ::DrawText(*hDC, m_playtimestr, m_playtimestr.GetLength(), rc,  DT_LEFT|DT_END_ELLIPSIS|DT_SINGLELINE| DT_VCENTER);
    return;
  }

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
  m_bitmap.Detach();
  if (m_bsingleormultiply)
  {
    MakeMultiplyBmp mmbmp;
    mmbmp.SetBmpAlpha(200);
    mmbmp.SetBmpBrightness(1.4);
    HBITMAP hbitmp = mmbmp.MakeMultiplyBmpFromSingleBmp(bmp);
    m_bitmap.Attach(hbitmp);
  }
	else
	m_bitmap.Attach(bmp);
	PreMultiplyBitmap(m_bitmap,m_btnSize,m_NotButton);
}
void CSUIButton::PreMultiplyBitmap(CBitmap& bmp , CSize& sizeBmp, BOOL NotButton)
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

void CSUIButton::SetCurrentHideState(long iWidth,double skinsRate,int m_nLogDPIY)
{
  if (m_hide)
    m_currenthide = TRUE;
  else if (iWidth > m_hidewidth * skinsRate * m_nLogDPIY / 96)
    m_currenthide = FALSE;
  else
    m_currenthide = TRUE;
}

void CSUIButton::SetString(CString str)
{
  m_playtimestr = str;
}

CString CSUIButton::GetString()
{
  return m_playtimestr;
}

void CSUIButton::SetStrSize(CSize sz)
{
  m_btnSize = sz;
}

extern BOOL CheckMultiplyBmpOrSingle(std::wstring& bmpname);
/*CSUIBtnList*/
CSUIBtnList::CSUIBtnList()
{
}

CSUIBtnList::~CSUIBtnList()
{
  POSITION pos = GetHeadPosition();
  while(pos)
  {
    CSUIButton* cBtn =  GetNext(pos);
    if (cBtn)
      delete cBtn;
  }
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
void CSUIBtnList::SetDisableStat(UINT iMsgID, BOOL bDisable, BOOL bHittest){
  if (bHittest)
    return;
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
		cBtn->m_currenthide = bHide;

	
}
void CSUIBtnList::SetHideStat(UINT iMsgID, BOOL bHide){
	POSITION pos = GetHeadPosition();
	while(pos){
		CSUIButton* cBtn =  GetNext(pos);
		if( iMsgID == cBtn->m_htMsgID ){
			cBtn->m_currenthide = bHide;
			//break;
		}
	}

}
void CSUIBtnList::SetHideStat(LPCTSTR szBmpName, BOOL bHide){
	POSITION pos = GetHeadPosition();
	while(pos){
		CSUIButton* cBtn =  GetNext(pos);
		if( cBtn->m_szBmpName.Compare(szBmpName) == 0 ){
			cBtn->m_currenthide = bHide;
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

void CSUIBtnList::SetCurrentHideState(long iWidth,double skinsRate,int m_nLogDPIY)
{
  POSITION pos = GetHeadPosition();
  while(pos)
  {
    CSUIButton* cbtn = GetNext(pos);
    cbtn->SetCurrentHideState(iWidth, skinsRate, m_nLogDPIY);
  }
}

CSUIButton* CSUIBtnList::GetButton(CString s)
{
  POSITION pos = GetHeadPosition();
  while(pos)
  {
    CSUIButton* cbtn = GetNext(pos);
    if (cbtn->m_buttonname == s)
      return cbtn;
  }
  return 0;
}

int CSUIBtnList::GetRelativeMinLength(CRect WndRect, CSUIButton* btn)
{
  CRect rc = btn->m_rcHitest - WndRect.TopLeft();
  int min = MAXINT;
  POSITION pos = GetHeadPosition();
  while(pos){
    CSUIButton* cBtn =  GetNext(pos);
    if (cBtn->m_currenthide)
      continue;
   
    CRect rtrc = cBtn->m_rcHitest - WndRect.TopLeft();

    int i = rtrc.left - rc.left;

    if (i > 0 && i < min)
      min = i;
  }
  return min;
}

BOOL CSUIBtnList::ResReload(std::wstring folder, BOOL bl, std::wstring cfgfilename)
{
  
  std::wstring cfgfilepath(L"skins\\");
  cfgfilepath += folder;
  cfgfilepath += L"\\";
  cfgfilepath += cfgfilename;
  
  GUIConfigManage cfgfile;
  ButtonManage  cfgbtn;
  cfgfile.SetCfgFilePath(cfgfilepath);
  cfgfile.ReadFromFile();
  if (cfgfile.IsFileExist())
  {
    cfgbtn.SetParse(cfgfile.GetCfgString(), this);
    cfgbtn.ParseConfig(TRUE);
  }
  std::map<std::wstring, buttonattribute>& btnattribute 
    = cfgbtn.GetBtnAttributeStruct();

  ResLoader rlResLoader;
  BOOL bloadsuccess = FALSE;
  POSITION pos = GetHeadPosition();
  while(pos)
  {
    CSUIButton* cbtn = GetNext(pos);
    std::wstring bmpname;//(L"skins\\");
    bmpname += folder;
    bmpname += L"\\";
    bmpname += cbtn->m_szBmpName;
    if (cbtn->m_buttonname != L"PLAYTIME")
    {
      HBITMAP hbitmap;
      if (bl)
      {
        cbtn->m_bsingleormultiply = CheckMultiplyBmpOrSingle(bmpname);
        bmpname = L"skins\\" + bmpname;
        hbitmap = rlResLoader.LoadBitmapFromDisk(bmpname);
      }
      else
      {
        hbitmap = rlResLoader.LoadBitmapFromModule(cbtn->m_szBmpName.GetString());
        cbtn->m_bsingleormultiply = FALSE;
      }

      if (hbitmap)
      {
       cbtn->Attach(hbitmap);
       cbtn->CountDPI();
       bloadsuccess = TRUE;
      
      } 
      else
      {
       bloadsuccess = FALSE;
       break;
      }
    }

    if (!btnattribute.empty() && cbtn->m_buttonname != L"PLAYTIME")
    {
      buttonattribute btnstruct = btnattribute[cbtn->m_buttonname.GetString()];
      cbtn->m_marginTownd  = btnstruct.fixrect;
      cbtn->m_iAlign = btnstruct.fixalign;
      cbtn->m_hide = btnstruct.hide;
      cbtn->m_hidewidth = btnstruct.hidewidth;
      
      if (!btnstruct.relativevec.empty())
      {  
        
        POSITION pos = cbtn->btnAlignList.GetHeadPosition();
        while (pos)
        {
          delete (cbtn->btnAlignList.GetNext(pos));
        }
        cbtn->btnAlignList.RemoveAll();
        
        for (std::vector<relativebuttonattribute>::iterator ite = btnstruct.relativevec.begin();
             ite != btnstruct.relativevec.end(); ++ite)
          cbtn->addAlignRelButton(ite->relativealign, ite->relativebutton, ite->relativerect);
          
      }
    }
  }

  return bloadsuccess;
}

static BOOL CheckMultiplyBmpOrSingle(std::wstring& bmpname)
{
  BOOL beMultiplyOrSingle = FALSE;
  CSVPToolBox csvptbox;
  std::wstring fullpath = csvptbox.GetPlayerPath(L"skins\\");
  fullpath += bmpname;
  if (::PathFileExists(fullpath.c_str()))
    beMultiplyOrSingle = FALSE;
  else
  {
    int pos = bmpname.find_last_of('.');
    if (pos != std::wstring::npos)
    {
      std::wstring bmpNameNew = bmpname.substr(0, pos);
      std::wstring suffix(bmpname.substr(pos));
      bmpNameNew += L"_single";
      bmpNameNew += suffix;
      bmpname = bmpNameNew;
      fullpath = csvptbox.GetPlayerPath(L"skins\\");
      fullpath += bmpname;
    }
    if (::PathFileExists(fullpath.c_str()))
      beMultiplyOrSingle = TRUE;
  }

  return beMultiplyOrSingle;
}