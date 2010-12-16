#include "stdafx.h"
#include "MediaCenterView.h"

// MediaCenterView
MediaCenterView::MediaCenterView() : MediaScrollbar(),
                                     MediaListView()
{
}

MediaCenterView::~MediaCenterView()
{
}

void MediaCenterView::ReSize()
{
    ::GetClientRect(m_parentwnd, &m_clientrc);
    m_clientrc.top += 20;
    m_clientrc.bottom -= 20;
    m_clientrc.left += 20;
    m_clientrc.right -= 20;
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
    ReSize();
    return 0;
}

HRESULT MediaCenterView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    ReSize();
    AddScrollbar(m_hWnd);
    AddListView(m_hWnd);

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
    mdc.FillRect(&rc, COLOR_WINDOW);

    DrawListView(mdc, GetOffsetVal());
    DrawScrollbar(mdc);

    return 0;
}

void MediaCenterView::ShowMediaCenter(HWND hwnd, int boxwidth, int boxheight, RECT& margin)
{
    m_parentwnd = hwnd;

    Create(m_parentwnd);
    SetItemBox(boxwidth, boxheight, margin);

    std::vector<MEDIA_ITEMDATA> data;
    for (int i=0;i<10;i++)
    {
        wchar_t filename[80], imgpath[80];
        wsprintf(filename, L"filename_%d", i);
        wsprintf(imgpath, L"imgpath_%d", i);
        MEDIA_ITEMDATA rs = {filename, imgpath};

        data.push_back(rs);
    }

    AttachData(data);
}

void MediaCenterView::HideMediaCenter()
{

}