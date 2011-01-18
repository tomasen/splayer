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
  //::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 500, 500, 0);
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

  //int nPointFactor = 3;
  //int nLineFactor = 2;
  //SolidBrush sbGreen(Color(255, 0, 200, 0));
  //gpMem.FillRectangle(&sbGreen, rcViewfinder.left - nPointFactor, rcViewfinder.top - nPointFactor,
  //                    2 * nPointFactor, 2 * nPointFactor);  // NWPoint rectangle
  //gpMem.FillRectangle(&sbGreen, rcViewfinder.right - nPointFactor, rcViewfinder.top - nPointFactor,
  //                    2 * nPointFactor, 2 * nPointFactor);  // NEPoint rectangle
  //gpMem.FillRectangle(&sbGreen, rcViewfinder.right - nPointFactor, rcViewfinder.bottom - nPointFactor,
  //                    2 * nPointFactor, 2 * nPointFactor);  // SEPoint rectangle
  //gpMem.FillRectangle(&sbGreen, rcViewfinder.left - nPointFactor, rcViewfinder.bottom - nPointFactor,
  //                    2 * nPointFactor, 2 * nPointFactor);  // SWPoint rectangle

  //gpMem.FillRectangle(&sbGreen, rcViewfinder.right - rcViewfinder.Width() / 2 - nLineFactor, rcViewfinder.top - nLineFactor,
  //                    2 * nLineFactor, 2 * nLineFactor); // TopLine middle point
  //gpMem.FillRectangle(&sbGreen, rcViewfinder.right - nLineFactor, rcViewfinder.bottom - rcViewfinder.Height() / 2 - nLineFactor,
  //                    2 * nLineFactor, 2 * nLineFactor); // RightLine middle point
  //gpMem.FillRectangle(&sbGreen, rcViewfinder.right - rcViewfinder.Width() / 2 - nLineFactor, rcViewfinder.bottom - nLineFactor,
  //                    2 * nLineFactor, 2 * nLineFactor); // BottomLine middle point
  //gpMem.FillRectangle(&sbGreen, rcViewfinder.left - nLineFactor, rcViewfinder.bottom - rcViewfinder.Height() / 2 - nLineFactor,
  //                    2 * nLineFactor, 2 * nLineFactor); // LeftLine middle point

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

    // Show file save dialog
    WTL::CFileDialog fd(FALSE, 0, 0, OFN_OVERWRITEPROMPT,
                        L"png(*.png)\0*.png\0gif(*.gif)\0*.gif\0bmp(*.bmp)\0*.bmp\0jpeg(*.jpg;*.jpeg)\0*.jpg;*.jpeg\0all types(*.*)\0*.*\0");
    if (fd.DoModal() == IDOK)
    {
      CImage igViewfinder;

      wstring sFileName = fd.m_ofn.lpstrFile;
      wstring sFileExt = ::PathFindExtension(sFileName.c_str());
      if (sFileExt.empty())
      {
        // Don't select a extension
        switch (fd.m_ofn.nFilterIndex)
        {
        case 1:
          sFileName += L".png";
          break;
        case 2:
          sFileName += L".gif";
          break;
        case 3:
          sFileName += L".bmp";
          break;
        case 4:
          sFileName += L".jpg";
          break;
        }
      }

      HBITMAP hbmResult = 0;
      bmResult.GetHBITMAP(Color(255, 255, 255, 255), &hbmResult);
      igViewfinder.Attach(hbmResult);
      igViewfinder.Save(sFileName.c_str());

      EndDialog(0);
    }
  }
}