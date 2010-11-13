
#include "SVPLycShowBox.h"
//#include <gdiplus.h>
//using namespace Gdiplus;

SVPLycShowBox::SVPLycShowBox(void)
{
}

SVPLycShowBox::~SVPLycShowBox(void)
{
}
int SVPLycShowBox::ChangeFontColor( COLORREF rgb_color )
{
	return 0;
}



int SVPLycShowBox::ChangeFontColorByMouse(CPoint movement)
{
	return 0;
}


int SVPLycShowBox::ShowLycLine(CString szLycLine, int iLastingTime)
{
	return 0;
}

BEGIN_MESSAGE_MAP(SVPLycShowBox, CFrameWnd)
ON_WM_CREATE()
ON_WM_DESTROY()
ON_WM_ERASEBKGND()
ON_WM_MOUSEMOVE()
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONUP()
ON_WM_MOVE()
END_MESSAGE_MAP()

int SVPLycShowBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	m_nCurrentMouseAction= 0;
	m_bMouseInAction = FALSE;

	//////////////////////////////////////////////////////////////////////////
	// font setup
	GetSystemFontWithScale(&m_f, 14.0); //TODO Use better font

	//////////////////////////////////////////////////////////////////////////
	// preload the window background bitmap
	// in order for layered window to work, we need to draw the
	// bitmap onto the window DC. layered window requires a bitmap
	// to be pre-multiplied by its alpha value (essentially, RGB-to-HSL
	// conversion), so that blending operations can be done faster.
	// We chose to do pre-multiplication here, so that our bitmap
	// can be ready for painting (blending) afterwards.
	m_bmpWnd.Attach(CSUIButton::SUILoadImage(L"LYCBACKG.BMP"));
	m_sizeBmpWnd = m_bmpWnd.GetBitmapDimension();

	m_nBmpWndPadding = 20;

	/*
	CBitmap bmpTemp;
	bmpTemp.LoadBitmap(L"LYCBACKG.BMP");
	WTL bmpTemp.GetSize (m_sizeBmpWnd);
	
	BITMAPINFO bi = { { sizeof(BITMAPINFOHEADER), m_sizeBmpWnd.cx, m_sizeBmpWnd.cy, 1, 32 } };
	void *pBits = NULL;
	// note that we must create a device-independant bitmap (DIB section) to 
	// get access to pixels, multiplication is done per pixel.
	// therefore we have to create two DCs to bitblt a device-dependant bitmap (DDB)
	// to our DIB section.
	
	m_bmpWnd.CreateDIBSection (NULL, &bi, DIB_RGB_COLORS, &pBits, NULL, NULL);

	CDCHandle dc (::GetDC(NULL));
	CDC mdc1, mdc2;
	mdc1.CreateCompatibleDC (dc);
	mdc2.CreateCompatibleDC (dc);
	HBITMAP bmpold1 = mdc1.SelectBitmap (bmpTemp);
	HBITMAP bmpold2 = mdc2.SelectBitmap (m_bmpWnd);
	mdc2.BitBlt (0, 0, m_sizeBmpWnd.cx, m_sizeBmpWnd.cy, mdc1, 0, 0, SRCCOPY);
	// pre-multiply
	for (int y=0; y<m_sizeBmpWnd.cy; y++)
	{
		BYTE * pPixel = (BYTE *) pBits + m_sizeBmpWnd.cx * 4 * y;
		for (int x=0; x<m_sizeBmpWnd.cx; x++)
		{
			pPixel[0] = pPixel[0] * pPixel[3] / 255; 
			pPixel[1] = pPixel[1] * pPixel[3] / 255; 
			pPixel[2] = pPixel[2] * pPixel[3] / 255; 
			pPixel += 4;
		}
	}
	mdc1.SelectBitmap (bmpold1);
	mdc2.SelectBitmap (bmpold2);
	::ReleaseDC(NULL, dc);
	*/
	//////////////////////////////////////////////////////////////////////////
	// modify the window style, remove all borders and caption bars
	// and add the layered window bit
	ModifyStyle(-1, WS_POPUP|WS_SYSMENU);
	ModifyStyleEx(0, WS_EX_LAYERED);

	//////////////////////////////////////////////////////////////////////////
	// this routine will call UpdateLayeredWindow which is essential
	// for a layered window to become visible
	// WM_PAINT handling is not required
	DoUpdateWindow();


	//以下测试 Layered Popup Window
	//创建 Windows
	m_wndNewOSD = ::CreateWindowEx( WS_EX_NOACTIVATE|WS_EX_TOPMOST|WS_EX_LAYERED, _T("STATIC"), _T("OSD"), WS_POPUP, 20,20,200,200 , NULL,  0,GetModuleHandle(NULL),0);
	if(!m_wndNewOSD)
		::MessageBox(m_hWnd, L"OSD Wnd Create Fail" , L"", MB_OK);

	::SetLayeredWindowAttributes( m_wndNewOSD , 0, 0x80, LWA_ALPHA);


	return 0;
}

void SVPLycShowBox::DoUpdateWindow()
{
	CDC* dc = GetDC();
	RECT rect;
	GetWindowRect(&rect);

	CDC mdc, sdc;
	mdc.CreateCompatibleDC (dc);
	sdc.CreateCompatibleDC (dc);
	BITMAPINFO bi = { { sizeof(BITMAPINFOHEADER), rect.right-rect.left, rect.bottom-rect.top, 1, 32 } };
	void *pBits = NULL;
	CBitmap bmpWindowContent;
	bmpWindowContent.Attach(::CreateDIBSection (NULL, &bi, DIB_RGB_COLORS, &pBits, NULL, NULL) );
	//bmpWindowContent.CreateDIBSection (NULL, &bi, DIB_RGB_COLORS, &pBits, NULL, NULL);
	HBITMAP hBmpOld1 = (HBITMAP)mdc.SelectObject((HBITMAP)bmpWindowContent);
	HBITMAP hBmpOld2 = (HBITMAP)sdc.SelectObject((HBITMAP)m_bmpWnd);

	// 	mdc.SetStretchBltMode(HALFTONE);
	// 	mdc.SetBrushOrg(0,0, NULL);
	mdc.StretchBlt(0, 0, m_nBmpWndPadding, m_nBmpWndPadding, &sdc, 0, 0, m_nBmpWndPadding, m_nBmpWndPadding, SRCCOPY); // top-left
	mdc.StretchBlt(0, m_nBmpWndPadding, m_nBmpWndPadding, rect.bottom-rect.top-m_nBmpWndPadding*2, &sdc, 0, m_nBmpWndPadding, m_nBmpWndPadding, m_sizeBmpWnd.cy-m_nBmpWndPadding*2, SRCCOPY); // left
	mdc.StretchBlt(0, rect.bottom-rect.top-m_nBmpWndPadding, m_nBmpWndPadding, m_nBmpWndPadding, &sdc, 0, m_sizeBmpWnd.cy-m_nBmpWndPadding, m_nBmpWndPadding, m_nBmpWndPadding, SRCCOPY); // bottom-left
	mdc.StretchBlt(m_nBmpWndPadding, rect.bottom-rect.top-m_nBmpWndPadding, rect.right-rect.left-m_nBmpWndPadding*2, m_nBmpWndPadding, &sdc, m_nBmpWndPadding, m_sizeBmpWnd.cy-m_nBmpWndPadding, m_sizeBmpWnd.cx-m_nBmpWndPadding*2, m_nBmpWndPadding, SRCCOPY); // bottom
	mdc.StretchBlt(rect.right-rect.left-m_nBmpWndPadding, rect.bottom-rect.top-m_nBmpWndPadding, m_nBmpWndPadding, m_nBmpWndPadding, &sdc, m_sizeBmpWnd.cx-m_nBmpWndPadding, m_sizeBmpWnd.cy-m_nBmpWndPadding, m_nBmpWndPadding, m_nBmpWndPadding, SRCCOPY); // bottom-right
	mdc.StretchBlt(rect.right-rect.left-m_nBmpWndPadding, m_nBmpWndPadding, m_nBmpWndPadding, rect.bottom-rect.top-m_nBmpWndPadding*2, &sdc, m_sizeBmpWnd.cx-m_nBmpWndPadding, m_nBmpWndPadding, m_nBmpWndPadding, m_sizeBmpWnd.cy-m_nBmpWndPadding*2, SRCCOPY); // right
	mdc.StretchBlt(rect.right-rect.left-m_nBmpWndPadding, 0, m_nBmpWndPadding, m_nBmpWndPadding, &sdc, m_sizeBmpWnd.cx-m_nBmpWndPadding, 0, m_nBmpWndPadding, m_nBmpWndPadding, SRCCOPY); // top-right
	mdc.StretchBlt(m_nBmpWndPadding, 0, rect.right-rect.left-m_nBmpWndPadding*2, m_nBmpWndPadding, &sdc, m_nBmpWndPadding, 0, m_sizeBmpWnd.cx-m_nBmpWndPadding*2, m_nBmpWndPadding, SRCCOPY); // top
	mdc.StretchBlt(m_nBmpWndPadding, m_nBmpWndPadding, rect.right-rect.left-m_nBmpWndPadding*2, rect.bottom-rect.top-m_nBmpWndPadding*2, &sdc, m_nBmpWndPadding, m_nBmpWndPadding, m_sizeBmpWnd.cx-m_nBmpWndPadding*2, m_sizeBmpWnd.cy-m_nBmpWndPadding*2, SRCCOPY); // center

	HFONT hFontOld = (HFONT)mdc.SelectObject((HFONT)m_f);
	{
		//require GDI+ and figure out a way to work under win 2000
		//Graphics g(mdc);
		//Font f(mdc);
		//PointF ptf(40,40), ptfs(41,41);
		//SolidBrush br(Color(255,255,255,255));
		//SolidBrush brs(Color(128,0,0,0));
		//g.SetTextRenderingHint(TextRenderingHintAntiAlias);
		//g.DrawString(L"translucent window demo\r\nwith custom bitmap based window style,\r\nanti-aliased font rendering, and\r\ncustom window moving and sizing behavior\r\n", -1, &f, ptfs, NULL, &brs);
		//g.DrawString(L"translucent window demo\r\nwith custom bitmap based window style,\r\nanti-aliased font rendering, and\r\ncustom window moving and sizing behavior\r\n", -1, &f, ptf, NULL, &br);
	}
	mdc.SelectObject((HFONT)hFontOld);

	// calculate the new window position/size based on the bitmap size
	POINT ptWindowScreenPosition = {rect.left, rect.top};
	SIZE szWindow = {rect.right-rect.left, rect.bottom-rect.top};

	// setup the blend function
	BLENDFUNCTION blendPixelFunction= { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
	POINT ptSrc = {0,0}; // start point of the copy from dcMemory to dcScreen

	// perform the alpha blend
	BOOL bRet= ::UpdateLayeredWindow(m_hWnd, dc->m_hDC, &ptWindowScreenPosition, &szWindow, mdc,
		&ptSrc, 0, &blendPixelFunction, ULW_ALPHA);

	mdc.SelectObject((HBITMAP)hBmpOld1);
	sdc.SelectObject((HBITMAP)hBmpOld2);
	ReleaseDC ( dc);

}

void SVPLycShowBox::OnDestroy()
{
	PostQuitMessage(0);
	//bHandled = FALSE;
	
	__super::OnDestroy();

	// TODO: Add your message handler code here
}

BOOL SVPLycShowBox::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return CFrameWnd::OnEraseBkgnd(pDC);
}

void SVPLycShowBox::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	long x = point.x;
	long y = point.y;
	RECT rc;
	GetClientRect(&rc);
	if (m_bMouseInAction)
	{
		RECT rcWnd;
		GetWindowRect(&rcWnd);
		switch(m_nCurrentMouseAction)
		{
		case 1:
			SetWindowPos(NULL, rcWnd.left+x-m_ptMouse.x, rcWnd.top+y-m_ptMouse.y, 0, 0, SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);
			break;
		case 2:
			SetWindowPos(NULL, 0, 0, rcWnd.right-rcWnd.left+x-m_ptMouse.x, rcWnd.bottom-rcWnd.top+y-m_ptMouse.y, SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE);
			m_ptMouse.x = x;
			m_ptMouse.y = y;
			DoUpdateWindow();
			break;
		}
	}
	else
	{
		if(x>=rc.right-50 || y>=rc.bottom-50)
		{
			m_nCurrentMouseAction = 2;
			SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
		}
		else
		{
			m_nCurrentMouseAction = 1;
			SetCursor(LoadCursor(NULL, IDC_SIZEALL));
		}
	}
	//return ; //0
	__super::OnMouseMove(nFlags, point);
}

void SVPLycShowBox::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	m_ptMouse = point;
	
	SetCapture();
	m_bMouseInAction = TRUE;
	//return 0;
	__super::OnLButtonDown(nFlags, point);
}

void SVPLycShowBox::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	m_bMouseInAction = FALSE;
	ReleaseCapture();
	//return 0;
	__super::OnLButtonUp(nFlags, point);
}

void SVPLycShowBox::OnMove(int x, int y)
{

	
	::SetWindowPos(m_wndNewOSD, HWND_TOPMOST, x+20, y+20, 200, 200, SWP_SHOWWINDOW);
	

	//准备绘制 DC
	HDC hDC = ::GetDC(m_wndNewOSD);
	if (!hDC) {
		::DestroyWindow(m_wndNewOSD);
		::MessageBox(m_hWnd, L"Get DC Fail" , L"", MB_OK);
		//return ;//1
	}else{

		//绘制文字
		SIZE size;
		WCHAR aMessage[90] = L"Test";
		::GetTextExtentPoint32(hDC, aMessage, wcslen(aMessage), &size);
		::SetBkMode(hDC, TRANSPARENT);
		::TextOut(hDC, 10, 10, aMessage, wcslen(aMessage));
		::ReleaseDC(m_wndNewOSD, hDC);



		//CFrameWnd::OnMove(x, y);
		__super::OnMove(x, y);
	}
	return;
	// TODO: Add your message handler code here
}
