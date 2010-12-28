#include "stdafx.h"
#include "MediaCenterView.h"
#include "../../Model/MediaComm.h"

// MediaCenterView
MediaCenterView::MediaCenterView() : MediaScrollbar(),
                                     MediaListView()
{
}

MediaCenterView::~MediaCenterView()
{
}

void MediaCenterView::CalcWnd()
{
  ::GetClientRect(m_parentwnd, &m_clientrc);
  SetWindowPos(0, &m_clientrc, SWP_NOACTIVATE|SWP_SHOWWINDOW);
}

HRESULT MediaCenterView::OnLBUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{

    return 0;
}

HRESULT MediaCenterView::OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{

    return 0;
}

HRESULT MediaCenterView::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    CalcWnd();
    return 0;
}

HRESULT MediaCenterView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    CalcWnd();
    AddListView(m_hWnd);
    AddScrollbar(m_hWnd);

    return 0;
}

HRESULT MediaCenterView::OnEraseBKGnd(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{

    return 0;
}

HRESULT MediaCenterView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    WTL::CPaintDC dc(m_hWnd);

    ::GetClientRect(m_parentwnd, &m_clientrc);
    WTL::CMemoryDC mdc(dc, m_clientrc);
    mdc.FillRect(&m_clientrc, COLOR_BACKGROUND);

    RECT rc;
    GetBlockRect(rc);
    mdc.FillRect(&rc, COLOR_BACKGROUND);

    DrawListView(mdc, GetOffsetVal());
    DrawScrollbar(mdc);

    return 0;
}

void MediaCenterView::ShowMediaCenter(HWND hwnd, int boxwidth, int boxheight, RECT& margin)
{
  if (!IsWindow())
  {
    m_parentwnd = hwnd;
    //Create(m_parentwnd);//, NULL, NULL, WS_POPUP|WS_CLIPSIBLINGS|WS_CLIPCHILDREN, WS_EX_TOPMOST);
    SetItemBox(boxwidth, boxheight, margin);
    GetPlantWH(m_width, m_height);
    SetListPlane(m_width, m_height);
  }
  MediaDatas data;
  MediaFindCondition condition;
  m_model.Find(data, condition, 0, 20);
  AttachData(data);
}

void MediaCenterView::HideMediaCenter()
{

}
