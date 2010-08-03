#include "StdAfx.h"
#include "btneditctrl.h"

CBtnEditCtrl::CBtnEditCtrl(void)
{
}

CBtnEditCtrl::~CBtnEditCtrl(void)
{
}

HRESULT CBtnEditCtrl::Init(UINT uiButtonID)
{
  InsertButton(m_hWnd, uiButtonID);
  return S_OK;
}

BOOL CBtnEditCtrl::InsertButton(HWND hwnd, UINT uCmdId)
{
  InsBut *pbut;

  pbut = (InsBut*)HeapAlloc(GetProcessHeap(), 0, sizeof(InsBut));

  if(!pbut) return FALSE;

  pbut->uCmdId       = uCmdId;
  pbut->fButtonDown  = FALSE;

  // replace the old window procedure with our new one
  pbut->oldproc = (WNDPROC)::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)InsButProc);

  // associate our button state structure with the window
  ::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pbut);

  // force the edit control to update its non-client area
  ::SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER);

  return TRUE;
}

void CBtnEditCtrl::GetButtonRect(InsBut *pbut, RECT *rect)
{
  rect->right  -= pbut->cxRightEdge;
  rect->top    += pbut->cyTopEdge;
  rect->bottom -= pbut->cyBottomEdge;
  rect->left    = rect->right - (rect->bottom - rect->top)*5/4;

  if(pbut->cxRightEdge > pbut->cxLeftEdge)
    OffsetRect(rect, pbut->cxRightEdge - pbut->cxLeftEdge, 0);
}

void CBtnEditCtrl::RedrawNC(HWND hwnd)
{
  ::SetWindowPos(hwnd, 0, 0, 0, 0, 0, 
    SWP_DRAWFRAME|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER);
}

void CBtnEditCtrl::DrawInsertedButton(HWND hwnd, InsBut *pbut, RECT *prect)
{
  HDC     hdc;

  hdc = ::GetWindowDC(hwnd);

  FillRect(hdc, prect, GetSysColorBrush(COLOR_WINDOW)); 

  // now draw our inserted button:
  if(pbut->fButtonDown == TRUE)
  {
    bool fDrawByTheme = false;
    if (WTL::CTheme::IsThemingSupported())
    {
      WTL::CTheme theme;
      theme.OpenThemeData(hwnd, L"BUTTON");
      if (!theme.IsThemeNull())
      {
        fDrawByTheme = true;
        theme.DrawThemeBackground (hdc, BP_PUSHBUTTON, PBS_PRESSED, prect);
      }
    }
    if (!fDrawByTheme)
    {
      // draw a 3d-edge around the button. 
      DrawEdge(hdc, prect, EDGE_RAISED, BF_RECT | BF_FLAT | BF_ADJUST);
      // fill the inside of the button
      FillRect(hdc, prect, GetSysColorBrush(COLOR_BTNFACE));    
      OffsetRect(prect, 1, 1);
    }
  }
  else
  {
    bool fDrawByTheme = false;
    if (WTL::CTheme::IsThemingSupported())
    {
      WTL::CTheme theme;
      theme.OpenThemeData(hwnd, L"BUTTON");
      if (!theme.IsThemeNull())
      {
        fDrawByTheme = true;
        theme.DrawThemeBackground(hdc, BP_PUSHBUTTON, PBS_NORMAL, prect);
      }
    }
    if (!fDrawByTheme)
    {
      DrawEdge(hdc, prect, EDGE_RAISED, BF_RECT | BF_ADJUST);
      // fill the inside of the button
      FillRect(hdc, prect, GetSysColorBrush(COLOR_BTNFACE)); 
    }
  }

  HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
  HFONT hFontOld = (HFONT)SelectObject(hdc,font);
  SetBkMode(hdc, TRANSPARENT);
  DrawText(hdc, L"...", 3, prect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  SelectObject(hdc,hFontOld);

  ::ReleaseDC(hwnd, hdc);
}

LRESULT CALLBACK CBtnEditCtrl::InsButProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  WNDPROC oldproc;
  RECT    rect, oldrect;
  RECT   *prect;
  InsBut *pbut;
  POINT   pt;
  BOOL	oldstate;

  pbut    = (InsBut *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
  oldproc = pbut->oldproc;

  switch(msg)
  {
  case WM_NCDESTROY:
    HeapFree(GetProcessHeap(), 0, pbut);
    break;

  case WM_NCCALCSIZE:

    prect = (RECT *)lParam;
    oldrect = *prect;

    // let the old wndproc allocate space for the borders,
    // or any other non-client space.
    CallWindowProc(pbut->oldproc, hwnd, msg, wParam, lParam);

    // calculate what the size of each window border is,
    // we need to know where the button is going to live.
    pbut->cxLeftEdge   = prect->left     - oldrect.left; 
    pbut->cxRightEdge  = oldrect.right   - prect->right;
    pbut->cyTopEdge    = prect->top      - oldrect.top;
    pbut->cyBottomEdge = oldrect.bottom  - prect->bottom;   

    // now we can allocate additional space by deflating the
    // rectangle even further. Our button will go on the right-hand side,
    // and will be the same width as a scrollbar button
    prect->right -= (prect->bottom - prect->top)*5/4;

    // that's it! Easy or what!
    return 0;

  case WM_NCPAINT:

    // let the old window procedure draw the borders / other non-client
    // bits-and-pieces for us.
    ::CallWindowProc(pbut->oldproc, hwnd, msg, wParam, lParam);

    // get the screen coordinates of the window.
    // adjust the coordinates so they start from 0,0
    ::GetWindowRect(hwnd, &rect);
    ::OffsetRect(&rect, -rect.left, -rect.top);

    // work out where to draw the button
    GetButtonRect(pbut, &rect);

    DrawInsertedButton(hwnd, pbut, &rect);

    // that's it! This is too easy!
    return 0;

  case WM_NCHITTEST:

    // get the screen coordinates of the mouse
    pt.x = LOWORD(lParam);
    pt.y = HIWORD(lParam);

    // get the position of the inserted button
    ::GetWindowRect(hwnd, &rect);
    GetButtonRect(pbut, &rect);

    // check that the mouse is within the inserted button
    if(PtInRect(&rect, pt))
    {
      return HTBORDER;
    }
    else
    {
      break;
    }

  case WM_NCLBUTTONDBLCLK:
  case WM_NCLBUTTONDOWN:

    // get the screen coordinates of the mouse
    pt.x = LOWORD(lParam);
    pt.y = HIWORD(lParam);

    // get the position of the inserted button
    ::GetWindowRect(hwnd, &rect);
    pt.x -= rect.left;
    pt.y -= rect.top;
    ::OffsetRect(&rect, -rect.left, -rect.top);
    GetButtonRect(pbut, &rect);

    // check that the mouse is within the inserted button
    if(PtInRect(&rect, pt))
    {
      ::SetCapture(hwnd);

      pbut->fButtonDown = TRUE;
      pbut->fMouseDown  = TRUE;

      //redraw the non-client area to reflect the change
      DrawInsertedButton(hwnd, pbut, &rect);
    }

    break;

  case WM_MOUSEMOVE:

    if(pbut->fMouseDown == FALSE)
      break;

    // get the SCREEN coordinates of the mouse
    pt.x = LOWORD(lParam);
    pt.y = HIWORD(lParam);
    ::ClientToScreen(hwnd, &pt);

    // get the position of the inserted button
    ::GetWindowRect(hwnd, &rect);

    pt.x -= rect.left;
    pt.y -= rect.top;
    ::OffsetRect(&rect, -rect.left, -rect.top);

    GetButtonRect(pbut, &rect);

    oldstate = pbut->fButtonDown;

    // check that the mouse is within the inserted button
    if(PtInRect(&rect, pt))
      pbut->fButtonDown = 1;
    else
      pbut->fButtonDown = 0;        

    // redraw the non-client area to reflect the change.
    // to prevent flicker, we only redraw the button if its state
    // has changed
    if(oldstate != pbut->fButtonDown)
      DrawInsertedButton(hwnd, pbut, &rect);

    break;

  case WM_LBUTTONUP:

    if(pbut->fMouseDown != TRUE)
      break;

    // get the SCREEN coordinates of the mouse
    pt.x = LOWORD(lParam);
    pt.y = HIWORD(lParam);
    ::ClientToScreen(hwnd, &pt);

    // get the position of the inserted button
    ::GetWindowRect(hwnd, &rect);

    pt.x -= rect.left;
    pt.y -= rect.top;
    OffsetRect(&rect, -rect.left, -rect.top);

    GetButtonRect(pbut, &rect);

    // check that the mouse is within the inserted button
    if(PtInRect(&rect, pt))
      ::PostMessage(::GetParent(hwnd), WM_COMMAND, MAKEWPARAM(pbut->uCmdId, BN_CLICKED), 0);

    ::ReleaseCapture();
    pbut->fButtonDown  = FALSE;
    pbut->fMouseDown   = FALSE;

    // redraw the non-client area to reflect the change.
    DrawInsertedButton(hwnd, pbut, &rect);

    break;

  default:
    break;
  }

  return ::CallWindowProc(oldproc, hwnd, msg, wParam, lParam);
}