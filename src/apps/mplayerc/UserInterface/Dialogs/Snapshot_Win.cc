#include "stdafx.h"
#include "Snapshot_Win.h"

////////////////////////////////////////////////////////////////////////////////
// Note:
// The Viewfinder looks like this:
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////

CSnapshot_Win::CSnapshot_Win(const std::wstring &sSavePath)
: m_sSavePath(sSavePath)
{
  m_pdialogTemplate = (DLGTEMPLATE*)::calloc(1, sizeof(DLGTEMPLATE)+sizeof(DLGITEMTEMPLATE)+10);
  m_pdialogTemplate->style = DS_SETFONT | DS_FIXEDSYS | WS_POPUP;
  m_pdialogTemplate->dwExtendedStyle = 0;
  m_pdialogTemplate->cdit = 0;
  m_pdialogTemplate->x = 0;
  m_pdialogTemplate->y = 0;
  m_pdialogTemplate->cx = 0;
  m_pdialogTemplate->cy = 0;
}

CSnapshot_Win::~CSnapshot_Win()
{
  if (m_pdialogTemplate)
  {
    ::free(m_pdialogTemplate);
  }
}

INT_PTR CSnapshot_Win::DoModal(HWND hWndParent, LPARAM dwInitParam)
{
  BOOL result;

  ATLASSUME(m_hWnd == NULL);

  // Allocate the thunk structure here, where we can fail
  // gracefully.

  result = m_thunk.Init(NULL,NULL);
  if (result == FALSE) 
  {
    SetLastError(ERROR_OUTOFMEMORY);
    return -1;
  }

  _AtlWinModule.AddCreateWndData(&m_thunk.cd, (CDialogImplBaseT< CWindow >*)this);
#ifdef _DEBUG
  m_bModal = true;
#endif //_DEBUG

  return ::DialogBoxIndirectParam(_AtlBaseModule.GetModuleInstance(), m_pdialogTemplate,
    hWndParent, __super::StartDialogProc, dwInitParam);
}

////////////////////////////////////////////////////////////////////////////////
// Message handler
BOOL CSnapshot_Win::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
  // Delay for a moment
  ::Sleep(300);

  // Make this window to full screen and top most
  int nScreenWidth  = ::GetSystemMetrics(SM_CXSCREEN);
  int nScreenHeight = ::GetSystemMetrics(SM_CYSCREEN);

  ModifyStyle(WS_OVERLAPPEDWINDOW, 0, 0);
  //::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 500, 500, 0);   // for test
  ::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, nScreenWidth, nScreenHeight, 0);

  // Start GDI+ and load the background image
  CImage igForInit;    // Used only for start up GDI+
  igForInit.Load(L"");

  // Get the background image and bitblt to the two Bitmap objects
  HWND hDesktopWnd = ::GetDesktopWindow();
  HDC  hDesktopDC = ::GetDC(hDesktopWnd);
  HBITMAP hBKImage = ::CreateCompatibleBitmap(hDesktopDC, nScreenWidth, nScreenHeight);
  HDC hMemDC = ::CreateCompatibleDC(hDesktopDC);
  ::SelectObject(hMemDC, hBKImage);
  ::BitBlt(hMemDC, 0, 0, nScreenWidth, nScreenHeight, hDesktopDC, 0, 0, SRCCOPY);

  m_pigBKImage.reset(new Gdiplus::Bitmap(hBKImage, 0));
  m_pigMemImage.reset(new Gdiplus::Bitmap(nScreenWidth, nScreenHeight));

  m_pwndViewfinder.reset(new CSnapshot_Viewfinder(m_hWnd));

  ::DeleteDC(hMemDC);
  ::DeleteObject(hBKImage);

  return 0;
}

void CSnapshot_Win::OnDestroy()
{
}

void CSnapshot_Win::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  // If key is VK_ESCAPE, then quit the snapshot mode
  if (nChar == VK_ESCAPE)
  {
    EndDialog(0);
  }
}

BOOL CSnapshot_Win::OnEraseBkgnd(WTL::CDCHandle dc)
{
  return TRUE;
}

//static HANDLE g_h;

void CSnapshot_Win::OnPaint(WTL::CDCHandle dc)
{
  using Gdiplus::Graphics;
  using Gdiplus::Bitmap;
  using Gdiplus::SolidBrush;
  using Gdiplus::Pen;
  using Gdiplus::Color;
  using Gdiplus::Font;
  using Gdiplus::PointF;

  // General
  CRect rcClient;
  GetClientRect(&rcClient);
  CRect rcViewfinder = m_pwndViewfinder->m_rcViewfinder;

  // Paint
  Graphics gpMem(m_pigMemImage.get());

  // BK
  gpMem.DrawImage(m_pigBKImage.get(), 0, 0, m_pigBKImage->GetWidth(), m_pigBKImage->GetHeight());
  
  // Normal area(exclude the view finder)
  SolidBrush brGray(Color(70, 0, 0, 0));
  gpMem.FillRectangle(&brGray, 0, 0, rcClient.Width(), rcViewfinder.top);  // Top
  gpMem.FillRectangle(&brGray, 0, rcViewfinder.top, 
                      rcViewfinder.left, rcViewfinder.Height());  // Left
  gpMem.FillRectangle(&brGray, 0, rcViewfinder.bottom, rcClient.Width(), rcClient.bottom - rcViewfinder.bottom);  // Bottom
  gpMem.FillRectangle(&brGray, rcViewfinder.right, rcViewfinder.top, 
                      rcClient.Width() - rcViewfinder.right, rcViewfinder.Height());  // Right

  // View finder's selection rectangle
  Pen pnGreen(Color(255, 0, 200, 0));
  gpMem.DrawRectangle(&pnGreen, rcViewfinder.left, rcViewfinder.top, rcViewfinder.Width() - 1, rcViewfinder.Height() -1);  // Rectangle

  // Tooltip to the user
  if (!rcViewfinder.IsRectEmpty())
  {
    CRect rcTooltip;
    rcTooltip.left = rcViewfinder.left;
    
    // Adjust the tooltip's rect
    CSize szIdealSize(140, 45);
    if (rcViewfinder.top < (szIdealSize.cy + 7))
    {
      // Put tooltip inside the viewfinder
      rcTooltip.top = rcViewfinder.top + 3;
    } 
    else
    {
      // Put tooltip outside the viewfinder
      rcTooltip.top = rcViewfinder.top - szIdealSize.cy - 7;
    }

    rcTooltip.right = rcTooltip.left + szIdealSize.cx;
    rcTooltip.bottom = rcTooltip.top + szIdealSize.cy;

    SolidBrush brDarkGray(Color(180, 90, 90, 90));
    gpMem.FillRectangle(&brDarkGray, rcTooltip.left, rcTooltip.top, rcTooltip.Width(), rcTooltip.Height());

    CString sCurArea;
    sCurArea.Format(L"当前大小: %d*%d", rcViewfinder.Width(), rcViewfinder.Height());

    CString sUserTip;
    sUserTip.Format(L"双击即可完成截图");

    Font fnTooltip(L"Tahoma", 9);
    SolidBrush brWhite(Color(200, 255, 255, 255));
    gpMem.DrawString((LPCTSTR)sCurArea, -1, &fnTooltip, PointF(rcTooltip.left + 7, rcTooltip.top + 7), &brWhite);
    gpMem.DrawString((LPCTSTR)sUserTip, -1, &fnTooltip, PointF(rcTooltip.left + 7, rcTooltip.top + 27), &brWhite);
  }

  // Determine the update rect
  CRect rcUpdateArea;
  GetUpdateRect(&rcUpdateArea, FALSE);

  // Bitblt the mem dc to real dc
  Graphics gpReal(m_hWnd);
  gpReal.DrawImage(m_pigMemImage.get(), rcUpdateArea.left, rcUpdateArea.top,
                   rcUpdateArea.left, rcUpdateArea.top, rcUpdateArea.Width(), rcUpdateArea.Height(), Gdiplus::UnitPixel);

  // Validate the window
  ValidateRect(0);
}

void CSnapshot_Win::OnMouseMove(UINT nFlags, CPoint point)
{
  //if (::GetStdHandle(STD_OUTPUT_HANDLE) == 0)
  //{
  //  ::AllocConsole();
  //  g_h = ::GetStdHandle(STD_OUTPUT_HANDLE);
  //}

  static CRect rcLastViewfinder(-1, -1, -1, -1);

  bool bNeedUpdate = false;
  m_pwndViewfinder->OnMouseMove(nFlags, point, bNeedUpdate);

  if (bNeedUpdate)
  {
    CRect rcToUpdate(0, 0, 0, 0);

    if ((rcLastViewfinder.left == -1) &&
        (rcLastViewfinder.top == -1) &&
        (rcLastViewfinder.right == -1) &&
        (rcLastViewfinder.bottom == -1))
    {
      rcToUpdate = m_pwndViewfinder->m_rcViewfinder;
    } 
    else
    {
      // Union the old and new rect
      rcToUpdate.UnionRect(&rcLastViewfinder, &(m_pwndViewfinder->m_rcViewfinder));

      // Include the tooltip area
      rcToUpdate.InflateRect(150, 150, 150, 150);
    }

    //CString sss;
    //sss.Format(L"old = %d, %d, %d, %d\n\bnew = %d, %d, %d, %d\n\bupd = %d, %d, %d, %d\n",
    //  rcLastViewfinder.left, rcLastViewfinder.top, rcLastViewfinder.right, rcLastViewfinder.bottom,
    //  m_pwndViewfinder->m_rcViewfinder.left, m_pwndViewfinder->m_rcViewfinder.top, m_pwndViewfinder->m_rcViewfinder.right, m_pwndViewfinder->m_rcViewfinder.bottom,
    //  rcToUpdate.left, rcToUpdate.top, rcToUpdate.right, rcToUpdate.bottom);

    //::WriteConsole(g_h, (LPCTSTR)sss, sss.GetLength(), 0, 0);

    RedrawWindow(&rcToUpdate);
  }

  rcLastViewfinder = m_pwndViewfinder->m_rcViewfinder;
}

void CSnapshot_Win::OnLButtonDown(UINT nFlags, CPoint point)
{
  m_pwndViewfinder->OnLButtonDown(nFlags, point);
}

void CSnapshot_Win::OnLButtonUp(UINT nFlags, CPoint point)
{
  m_pwndViewfinder->OnLButtonUp(nFlags, point);
}

void CSnapshot_Win::OnRButtonUp(UINT nFlags, CPoint point)
{
  EndDialog(0);
}

void CSnapshot_Win::OnLButtonDblClk(UINT nFlags, CPoint point)
{
  using Gdiplus::Bitmap;
  using Gdiplus::Graphics;
  using Gdiplus::Color;
  using Gdiplus::UnitPixel;
  using std::wstring;

  // Save the picture
  if (m_pwndViewfinder->m_rcViewfinder.PtInRect(point))
  {
    // Save image
    CRect rcViewfinder = m_pwndViewfinder->m_rcViewfinder;
    Bitmap bmResult(rcViewfinder.Width(), rcViewfinder.Height());
    Graphics gpResult(&bmResult);

    gpResult.DrawImage(m_pigBKImage.get(), 0, 0, rcViewfinder.left, rcViewfinder.top,
                       rcViewfinder.Width(), rcViewfinder.Height(), UnitPixel);

    // Save image to default path and clipboard
    CImage igViewfinder;

    // Save to default path
    HBITMAP hbmResult = 0;
    bmResult.GetHBITMAP(Color(255, 255, 255, 255), &hbmResult);
    igViewfinder.Attach(hbmResult);
    igViewfinder.Save(m_sSavePath.c_str());

    // Save to clipboard
    BITMAP bmpScreen = {0};
    ::GetObject(hbmResult, sizeof(BITMAP), &bmpScreen);

    BITMAPINFOHEADER   bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);    
    bi.biWidth = bmpScreen.bmWidth;    
    bi.biHeight = bmpScreen.bmHeight;  
    bi.biPlanes = 1;    
    bi.biBitCount = 32;    
    bi.biCompression = BI_RGB;    
    bi.biSizeImage = 0;  
    bi.biXPelsPerMeter = 0;    
    bi.biYPelsPerMeter = 0;    
    bi.biClrUsed = 0;    
    bi.biClrImportant = 0;

    DWORD dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

    BYTE *pData = new BYTE[dwBmpSize];
    ::GetDIBits(GetDC(), hbmResult, 0, (UINT)bmpScreen.bmHeight, pData, (BITMAPINFO *)&bi, DIB_RGB_COLORS);

    HANDLE hMem = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, dwBmpSize + sizeof(bi));
    if(hMem)
    {
      void *pMem = GlobalLock(hMem);

      CopyMemory((BYTE *)pMem, &bi, sizeof(bi));               // DIB header info
      CopyMemory((BYTE *)pMem + sizeof(bi), pData, dwBmpSize); // DIB bits

      OpenClipboard();
      EmptyClipboard();

      HANDLE hHandle = SetClipboardData(CF_DIB, pMem);
      if(!hHandle)
      {
        AfxMessageBox(_T("SetClipboardData Failed"));
      }
      CloseClipboard();
      GlobalUnlock(hMem);
    }

    EndDialog(0);
  }
}