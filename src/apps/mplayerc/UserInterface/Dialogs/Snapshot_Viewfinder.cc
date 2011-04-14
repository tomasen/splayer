#include "stdafx.h"
#include "Snapshot_Viewfinder.h"

//static HANDLE g_h;

CSnapshot_Viewfinder::CSnapshot_Viewfinder(HWND hParent)
  : m_hParent(hParent)
{
  //if (::GetStdHandle(STD_OUTPUT_HANDLE) == 0)
  //{
  //  ::AllocConsole();
  //  g_h = ::GetStdHandle(STD_OUTPUT_HANDLE);
  //}

  m_mpCursors[L"Normal"]     = ::LoadCursor(0, IDC_ARROW);
  m_mpCursors[L"Move"]       = ::LoadCursor(0, IDC_SIZEALL);
  m_mpCursors[L"NWPoint"]    = ::LoadCursor(0, IDC_SIZENWSE);
  m_mpCursors[L"SEPoint"]    = ::LoadCursor(0, IDC_SIZENWSE);
  m_mpCursors[L"NEPoint"]    = ::LoadCursor(0, IDC_SIZENESW);
  m_mpCursors[L"SWPoint"]    = ::LoadCursor(0, IDC_SIZENESW);
  m_mpCursors[L"LeftLine"]   = ::LoadCursor(0, IDC_SIZEWE);
  m_mpCursors[L"RightLine"]  = ::LoadCursor(0, IDC_SIZEWE);
  m_mpCursors[L"TopLine"]    = ::LoadCursor(0, IDC_SIZENS);
  m_mpCursors[L"BottomLine"] = ::LoadCursor(0, IDC_SIZENS);

  m_mpAreaHandler[L"Move"]       = &CSnapshot_Viewfinder::HandleMoveArea;
  m_mpAreaHandler[L"NWPoint"]    = &CSnapshot_Viewfinder::HandleNWPointArea;
  m_mpAreaHandler[L"NEPoint"]    = &CSnapshot_Viewfinder::HandleNEPointArea;
  m_mpAreaHandler[L"SEPoint"]    = &CSnapshot_Viewfinder::HandleSEPointArea;
  m_mpAreaHandler[L"SWPoint"]    = &CSnapshot_Viewfinder::HandleSWPointArea;
  m_mpAreaHandler[L"TopLine"]    = &CSnapshot_Viewfinder::HandleTopLineArea;
  m_mpAreaHandler[L"RightLine"]  = &CSnapshot_Viewfinder::HandleRightLineArea;
  m_mpAreaHandler[L"BottomLine"] = &CSnapshot_Viewfinder::HandleBottomLineArea;
  m_mpAreaHandler[L"LeftLine"]   = &CSnapshot_Viewfinder::HandleLeftLineArea;

  ::SetRect(&m_rcViewfinder, -1, -1, -1, -1);
  m_piLastCursorPos.SetPoint(0, 0);

  m_rcParent.SetRect(0, 0, 0, 0);
  ::GetClientRect(m_hParent, &m_rcParent);
}

CSnapshot_Viewfinder::~CSnapshot_Viewfinder()
{
}

void CSnapshot_Viewfinder::DetermineRectEachState()
{
  // ---------------------------------------------------------------------------
  // Note : 这里的坐标与绘画时（Paint）时不同，这里边距稍微多一些，是为了让用户
  // 更方便拖拽四个边框及四个角
  // ---------------------------------------------------------------------------
  // Re-calculate the rect of each state
  int nPointFactor = 3;
  int nLineFactor = 2;
  CRect rcTemp(m_rcViewfinder);
  rcTemp.DeflateRect(nLineFactor, nLineFactor, nLineFactor, nLineFactor);
  m_mpRectEachState[L"Move"]       = rcTemp;
  m_mpRectEachState[L"NWPoint"]    = CRect(m_rcViewfinder.left - nPointFactor, m_rcViewfinder.top - nPointFactor,
                                           m_rcViewfinder.left + nPointFactor, m_rcViewfinder.top + nPointFactor);
  m_mpRectEachState[L"NEPoint"]    = CRect(m_rcViewfinder.right - nPointFactor, m_rcViewfinder.top - nPointFactor,
                                           m_rcViewfinder.right + nPointFactor, m_rcViewfinder.top + nPointFactor);
  m_mpRectEachState[L"SEPoint"]    = CRect(m_rcViewfinder.right - nPointFactor, m_rcViewfinder.bottom - nPointFactor,
                                           m_rcViewfinder.right + nPointFactor, m_rcViewfinder.bottom + nPointFactor);
  m_mpRectEachState[L"SWPoint"]    = CRect(m_rcViewfinder.left - nPointFactor, m_rcViewfinder.bottom - nPointFactor,
                                           m_rcViewfinder.left + nPointFactor, m_rcViewfinder.bottom + nPointFactor);
  m_mpRectEachState[L"TopLine"]    = CRect(m_rcViewfinder.left + nPointFactor, m_rcViewfinder.top - nLineFactor,
                                           m_rcViewfinder.right - nPointFactor, m_rcViewfinder.top + nLineFactor);
  m_mpRectEachState[L"RightLine"]  = CRect(m_rcViewfinder.right - nLineFactor, m_rcViewfinder.top + nPointFactor,
                                           m_rcViewfinder.right + nLineFactor, m_rcViewfinder.bottom - nPointFactor);
  m_mpRectEachState[L"BottomLine"] = CRect(m_rcViewfinder.left + nPointFactor, m_rcViewfinder.bottom - nLineFactor,
                                           m_rcViewfinder.right - nPointFactor, m_rcViewfinder.bottom + nLineFactor);
  m_mpRectEachState[L"LeftLine"]   = CRect(m_rcViewfinder.left - nLineFactor, m_rcViewfinder.top + nPointFactor,
                                           m_rcViewfinder.left + nLineFactor, m_rcViewfinder.bottom - nPointFactor);
}

void CSnapshot_Viewfinder::DetermineCurrentState(CPoint point)
{
  //////////////////////////////////////////////////////////////////////////////
  // Mouse is the first time enter the snapshot window
  // Must handle carefully
  if ((m_rcViewfinder.left == -1) &&
    (m_rcViewfinder.top == -1) &&
    (m_rcViewfinder.right == -1) &&
    (m_rcViewfinder.bottom == -1) &&
    (::GetKeyState(VK_LBUTTON) & 0x8000))
  {
    m_rcViewfinder.left = point.x;
    m_rcViewfinder.top = point.y;
    m_rcViewfinder.right = m_rcViewfinder.left + 1;
    m_rcViewfinder.bottom = m_rcViewfinder.top + 1;

    m_sCurState = L"NWPoint";
    return;
  }

  //////////////////////////////////////////////////////////////////////////////
  // Mouse is not the first time enter the snapshot window
  // Set the current state
  std::map<std::wstring, CRect>::iterator it = m_mpRectEachState.begin();
  while (it != m_mpRectEachState.end())
  {
    if (it->second.PtInRect(point))
    {
      m_sCurState = it->first;
      return;
    }

    ++it;
  }

  // The normal state
  m_sCurState = L"Normal";
}

void CSnapshot_Viewfinder::SetCursorByCurState(CPoint point)
{
  if (::GetClassLong(m_hParent, GCL_HCURSOR) != (LONG)(m_mpCursors[m_sCurState]))
  {
    ::SetClassLong(m_hParent, GCL_HCURSOR, (LONG)(m_mpCursors[m_sCurState]));
  }
}

void CSnapshot_Viewfinder::ExchangeStateAndRect()
{
  // See if need to change state and rect
  bool bXNegative = m_rcViewfinder.right < m_rcViewfinder.left;
  bool bYNegative = m_rcViewfinder.bottom < m_rcViewfinder.top;

  if (bXNegative && bYNegative)
  {
    int nTemp = 0;
    nTemp = m_rcViewfinder.left;
    m_rcViewfinder.left = m_rcViewfinder.right;
    m_rcViewfinder.right = nTemp;

    nTemp = m_rcViewfinder.top;
    m_rcViewfinder.top = m_rcViewfinder.bottom;
    m_rcViewfinder.bottom = nTemp;

    if (m_sCurState == L"NWPoint")
    {
      m_sCurState = L"SEPoint";
    }
    else if (m_sCurState == L"NEPoint")
    {
      m_sCurState = L"SWPoint";
    }
    else if (m_sCurState == L"SEPoint")
    {
      m_sCurState = L"NWPoint";
    }
    else if (m_sCurState == L"SWPoint")
    {
      m_sCurState = L"NEPoint";
    }

    return;
  }

  if (bXNegative)
  {
    int nTemp = m_rcViewfinder.left;
    m_rcViewfinder.left = m_rcViewfinder.right;
    m_rcViewfinder.right = nTemp;

    if (m_sCurState == L"NWPoint")
    {
      m_sCurState = L"NEPoint";
    } 
    else if (m_sCurState == L"NEPoint")
    {
      m_sCurState = L"NWPoint";
    }
    else if (m_sCurState == L"SEPoint")
    {
      m_sCurState = L"SWPoint";
    }
    else if (m_sCurState == L"SWPoint")
    {
      m_sCurState = L"SEPoint";
    }
    else if (m_sCurState == L"LeftLine")
    {
      m_sCurState = L"RightLine";
    }
    else if (m_sCurState == L"RightLine")
    {
      m_sCurState = L"LeftLine";
    }

    return;
  }

  if (bYNegative)
  {
    int nTemp = m_rcViewfinder.top;
    m_rcViewfinder.top = m_rcViewfinder.bottom;
    m_rcViewfinder.bottom = nTemp;

    if (m_sCurState == L"NWPoint")
    {
      m_sCurState = L"SWPoint";
    }
    else if (m_sCurState == L"SWPoint")
    {
      m_sCurState = L"NWPoint";
    }
    else if (m_sCurState == L"NEPoint")
    {
      m_sCurState = L"SEPoint";
    }
    else if (m_sCurState == L"SEPoint")
    {
      m_sCurState = L"NEPoint";
    }
    else if (m_sCurState == L"TopLine")
    {
      m_sCurState = L"BottomLine";
    }
    else if (m_sCurState == L"BottomLine")
    {
      m_sCurState = L"TopLine";
    }

    return;
  }
}

void CSnapshot_Viewfinder::OnLButtonDown(UINT nFlags, CPoint point)
{
  DetermineCurrentState(point);

  m_piLastCursorPos = point;

  SetCursorByCurState(point);
}

void CSnapshot_Viewfinder::OnLButtonUp(UINT nFlags, CPoint point)
{
  m_piLastCursorPos.SetPoint(0, 0);
  m_sCurState = L"Normal";

  SetCursorByCurState(point);
}

void CSnapshot_Viewfinder::OnMouseMove(UINT nFlags, CPoint point, bool &bNeedUpdate)
{
  //CString sss;
  //sss.Format(L"NWPoint.left = %d, top = %d, right = %d, bottom = %d, width = %d, height = %d\n",
  //           m_mpRectEachState[L"NWPoint"].left, m_mpRectEachState[L"NWPoint"].top,
  //           m_mpRectEachState[L"NWPoint"].right, m_mpRectEachState[L"NWPoint"].bottom,
  //           m_mpRectEachState[L"NWPoint"].Width(), m_mpRectEachState[L"NWPoint"].Height());
  //sss.AppendFormat(L"CurPoint.x = %d, y = %d\n\n", point.x, point.y);
  //::WriteConsole(g_h, (LPCTSTR)sss, sss.GetLength(), 0, 0);

  DetermineRectEachState();

  if (!(nFlags & MK_LBUTTON))
  {
    DetermineCurrentState(point);
  }

  // Find the general handler
  if (m_mpAreaHandler.find(m_sCurState) != m_mpAreaHandler.end())
  {
    pfnAreaHandler p = m_mpAreaHandler[m_sCurState];
    (this->*p)(nFlags, point, bNeedUpdate);

    return;
  }

  // Handle the normal area
  HandleNormalArea(nFlags, point, bNeedUpdate);

  SetCursorByCurState(point);
}

void CSnapshot_Viewfinder::HandleNormalArea(UINT nFlags, CPoint point, bool &bNeedUpdate)
{
  SetCursorByCurState(point);
}

void CSnapshot_Viewfinder::HandleMoveArea(UINT nFlags, CPoint point, bool &bNeedUpdate)
{
  SetCursorByCurState(point);

  if (nFlags & MK_LBUTTON)
  {
    // X and Y increase, maybe negative
    int nXInc = point.x - m_piLastCursorPos.x;
    int nYInc = point.y - m_piLastCursorPos.y;

    // Change the view finder's rect
    m_rcViewfinder.OffsetRect(nXInc, nYInc);

    // Re-change the view finder's rect to avoid out of the screen
    CSize szViewFinder(m_rcViewfinder.Width(), m_rcViewfinder.Height());
    m_rcViewfinder.left < m_rcParent.left ? m_rcViewfinder.left = m_rcParent.left : 0;
    m_rcViewfinder.top < m_rcParent.top ? m_rcViewfinder.top = m_rcParent.top : 0;
    m_rcViewfinder.right > m_rcParent.right ? m_rcViewfinder.left = m_rcParent.right - szViewFinder.cx : 0;
    m_rcViewfinder.bottom > m_rcParent.bottom ? m_rcViewfinder.top = m_rcParent.bottom - szViewFinder.cy : 0;

    m_rcViewfinder.right = m_rcViewfinder.left + szViewFinder.cx;
    m_rcViewfinder.bottom = m_rcViewfinder.top + szViewFinder.cy;

    // Store current cursor point
    m_piLastCursorPos = point;

    bNeedUpdate = true;
  }
  else
  {
    bNeedUpdate = false;
  }
}

void CSnapshot_Viewfinder::HandleNWPointArea(UINT nFlags, CPoint point, bool &bNeedUpdate)
{
  SetCursorByCurState(point);

  if (nFlags & MK_LBUTTON)
  {
    // X and Y increase, maybe negative
    int nXInc = point.x - m_piLastCursorPos.x;
    int nYInc = point.y - m_piLastCursorPos.y;

    // Change the view finder's rect
    m_rcViewfinder.left += nXInc;
    m_rcViewfinder.top  += nYInc;

    // See if need to exchange the state
    ExchangeStateAndRect();

    // Store current cursor point
    m_piLastCursorPos = point;

    // See if need to Change the rect
    ExchangeStateAndRect();

    bNeedUpdate = true;
  } 
  else
  {
    bNeedUpdate = false;
  }
}

void CSnapshot_Viewfinder::HandleNEPointArea(UINT nFlags, CPoint point, bool &bNeedUpdate)
{
  SetCursorByCurState(point);

  if (nFlags & MK_LBUTTON)
  {
    // X and Y increase, maybe negative
    int nXInc = point.x - m_piLastCursorPos.x;
    int nYInc = point.y - m_piLastCursorPos.y;

    // Change the view finder's rect
    m_rcViewfinder.right += nXInc;
    m_rcViewfinder.top   += nYInc;

    // Store current cursor point
    m_piLastCursorPos = point;

    // See if need to Change the rect
    ExchangeStateAndRect();

    bNeedUpdate = true;
  } 
  else
  {
    bNeedUpdate = false;
  }
}

void CSnapshot_Viewfinder::HandleSEPointArea(UINT nFlags, CPoint point, bool &bNeedUpdate)
{
  SetCursorByCurState(point);

  if (nFlags & MK_LBUTTON)
  {
    // X and Y increase, maybe negative
    int nXInc = point.x - m_piLastCursorPos.x;
    int nYInc = point.y - m_piLastCursorPos.y;

    // Change the view finder's rect
    m_rcViewfinder.right  += nXInc;
    m_rcViewfinder.bottom += nYInc;

    // Store current cursor point
    m_piLastCursorPos = point;

    // See if need to Change the rect
    ExchangeStateAndRect();

    bNeedUpdate = true;
  } 
  else
  {
    bNeedUpdate = false;
  }
}

void CSnapshot_Viewfinder::HandleSWPointArea(UINT nFlags, CPoint point, bool &bNeedUpdate)
{
  SetCursorByCurState(point);

  if (nFlags & MK_LBUTTON)
  {
    // X and Y increase, maybe negative
    int nXInc = point.x - m_piLastCursorPos.x;
    int nYInc = point.y - m_piLastCursorPos.y;

    // Change the view finder's rect
    m_rcViewfinder.left   += nXInc;
    m_rcViewfinder.bottom += nYInc;

    // Store current cursor point
    m_piLastCursorPos = point;

    // See if need to Change the rect
    ExchangeStateAndRect();

    bNeedUpdate = true;
  } 
  else
  {
    bNeedUpdate = false;
  }
}

void CSnapshot_Viewfinder::HandleTopLineArea(UINT nFlags, CPoint point, bool &bNeedUpdate)
{
  SetCursorByCurState(point);

  if (nFlags & MK_LBUTTON)
  {
    // X and Y increase, maybe negative
    int nYInc = point.y - m_piLastCursorPos.y;

    // Change the view finder's rect
    m_rcViewfinder.top += nYInc;

    // Store current cursor point
    m_piLastCursorPos = point;

    // See if need to Change the rect
    ExchangeStateAndRect();

    bNeedUpdate = true;
  } 
  else
  {
    bNeedUpdate = false;
  }
}

void CSnapshot_Viewfinder::HandleRightLineArea(UINT nFlags, CPoint point, bool &bNeedUpdate)
{
  SetCursorByCurState(point);

  if (nFlags & MK_LBUTTON)
  {
    // X and Y increase, maybe negative
    int nXInc = point.x - m_piLastCursorPos.x;

    // Change the view finder's rect
    m_rcViewfinder.right += nXInc;

    // Store current cursor point
    m_piLastCursorPos = point;

    // See if need to Change the rect
    ExchangeStateAndRect();

    bNeedUpdate = true;
  } 
  else
  {
    bNeedUpdate = false;
  }
}

void CSnapshot_Viewfinder::HandleBottomLineArea(UINT nFlags, CPoint point, bool &bNeedUpdate)
{
  SetCursorByCurState(point);

  if (nFlags & MK_LBUTTON)
  {
    // X and Y increase, maybe negative
    int nYInc = point.y - m_piLastCursorPos.y;

    // Change the view finder's rect
    m_rcViewfinder.bottom += nYInc;

    // Store current cursor point
    m_piLastCursorPos = point;

    // See if need to Change the rect
    ExchangeStateAndRect();

    bNeedUpdate = true;
  } 
  else
  {
    bNeedUpdate = false;
  }
}

void CSnapshot_Viewfinder::HandleLeftLineArea(UINT nFlags, CPoint point, bool &bNeedUpdate)
{
  SetCursorByCurState(point);

  if (nFlags & MK_LBUTTON)
  {
    // X and Y increase, maybe negative
    int nXInc = point.x - m_piLastCursorPos.x;

    // Change the view finder's rect
    m_rcViewfinder.left += nXInc;

    // Store current cursor point
    m_piLastCursorPos = point;

    // See if need to Change the rect
    ExchangeStateAndRect();

    bNeedUpdate = true;
  } 
  else
  {
    bNeedUpdate = false;
  }
}