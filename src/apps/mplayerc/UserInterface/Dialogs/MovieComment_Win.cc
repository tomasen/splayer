#include "stdafx.h"
#include "MovieComment_Win.h"

MovieComment::MovieComment():m_showframe(FALSE)
{
}

MovieComment::~MovieComment()
{

}

HRESULT MovieComment::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

  RECT rc = CalcWndPos();  
  HRGN rgn;
  rgn = ::CreateRoundRectRgn(0, 0, rc.right-rc.left, rc.bottom-rc.top, 7, 7);
  SetWindowRgn(rgn, TRUE);

  return WebBrowserControl<MovieComment, IWebBrowser2>::OnCreate(uMsg, wParam, lParam, bHandled);
}


void MovieComment::OpenUrl(std::wstring url)
{
  if (m_wb2 == NULL)
      return;

  CComVariant v;
  m_wb2->Navigate(CComBSTR(url.c_str()), &v, &v, &v, &v);
}

RECT MovieComment::CalcWndPos()
{
  RECT rc, newrc;
  GetParent().GetWindowRect(&rc);
  newrc.left = rc.left+20;
  newrc.top = rc.bottom-260;
  newrc.right = rc.left+20+373;
  newrc.bottom = newrc.top+161;
  SetWindowPos(NULL, &newrc, SWP_NOACTIVATE|SWP_HIDEWINDOW);

  return newrc;
}

void MovieComment::ShowFrame()
{
  m_showframe = TRUE;
  ShowWindow(SW_SHOW);
}

void MovieComment::HideFrame()
{
  m_showframe = FALSE;
  ShowWindow(SW_HIDE);
}

void MovieComment::CloseFrame()
{
  PostMessage(WM_CLOSE, NULL, NULL);
}

BOOL MovieComment::IsShow()
{
  return m_showframe;
}