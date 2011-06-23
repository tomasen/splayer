#include "stdafx.h"
#include "ListImpl.h"

CListImpl::CListImpl(void)
{
}

CListImpl::~CListImpl(void)
{
}

void CListImpl::OnMouseMove(UINT wParam, WTL::CPoint pt)
{
  WTL::CRect pntrc;
  WTL::CRect rc;
  GetParent().GetWindowRect(&pntrc);
  GetWindowRect(&rc);
  rc = rc - pntrc.TopLeft();
  pt = pt + rc.TopLeft();
  ::PostMessage(GetParent(), WM_MOUSEMOVE, wParam, MAKELPARAM(pt.x, pt.y));
}

void CListImpl::OnLButtonDbClick(UINT wParam, WTL::CPoint pt)
{
  WTL::CRect pntrc;
  WTL::CRect rc;
  GetParent().GetWindowRect(&pntrc);
  GetWindowRect(&rc);
  rc = rc - pntrc.TopLeft();
  pt = pt + rc.TopLeft();
  ::PostMessage(GetParent(), WM_LBUTTONDBLCLK, wParam, MAKELPARAM(pt.x, pt.y));
}