#include "stdafx.h"
#include "MediaListView.h"
#include "../../resource.h"

// MediaCenterView
MediaListView::MediaListView()
{
}

MediaListView::~MediaListView()
{
    m_memboxbmp.SelectBitmap(m_oldboxbmp);
    m_memboxbmp.DeleteDC();
}

// margin-right Box间距
void MediaListView::SetItemBox(int w, int h, const RECT& margin)
{
    m_boxwidth = w;
    m_boxheight = h;
    m_boxmargin = margin;
}

void MediaListView::AddListView(HWND hWnd)
{
    m_parentwnd = hWnd;
    GetClientRect(m_parentwnd, &m_clientrc);
    //m_defaultboxbmp.LoadBitmap(IDB_BITMAP1);
    m_memboxbmp.CreateCompatibleDC(WTL::CClientDC(m_parentwnd));
    m_oldboxbmp = m_memboxbmp.SelectBitmap(m_defaultboxbmp);
}

void MediaListView::AttachData(std::vector<MEDIA_ITEMDATA>& data)
{
    m_itemlist.clear();

    int row = 0;
    int lines = 0;
    std::vector<MEDIA_ITEMDATA>::iterator it;
    for (it = data.begin(); it != data.end(); it++)
    {
        // Break line
        if (row > 2)
        {
            row = 0;
            lines++;
        }

        MEDIA_ITEMBOX box;
        box.pos.x = m_boxmargin.left+row*m_boxwidth+m_boxmargin.right*row;
        box.pos.y = m_boxmargin.top+lines*m_boxheight+m_boxmargin.right*lines;

        box.data = *it;
        m_itemlist.push_back(box);
        row++;
    }
}

BOOL MediaListView::SelBox(POINT currpos, POINT boxpos)
{
    RECT rc;
    rc.top = boxpos.y;
    rc.left = boxpos.x;
    rc.right = boxpos.x+m_boxwidth;
    rc.bottom = boxpos.y+m_boxheight;

    return PtInRect(&rc, currpos);
}

void MediaListView::DrawItem(WTL::CDC& dc, POINT pos, int cell, std::wstring text)
{
    dc.SetBkMode(TRANSPARENT);
    dc.BitBlt(pos.x, pos.y, m_boxwidth, m_boxheight, m_memboxbmp, 0, cell*m_boxheight, SRCCOPY);

    RECT rc = {pos.x, pos.y+m_boxheight, pos.x+m_boxwidth, pos.y+m_boxheight-20};
    dc.DrawText(text.c_str(), text.size(), &rc, DT_SINGLELINE|DT_CENTER|DT_VCENTER);
}

void MediaListView::DrawListView(WTL::CDC& dc, int y)
{

    std::vector<MEDIA_ITEMBOX>::iterator it;

    int miny = m_itemlist.begin()->pos.y;
    int maxy = m_itemlist.at(m_itemlist.size()-1).pos.y;

    // y>0 滚动条向上滑动， y<0 滚动条向下滑动
    if (y>0 && (miny > m_boxmargin.top) ||
        y<0 && ((y+maxy+m_boxheight) < m_clientrc.bottom-m_boxmargin.top)) 
    {
        for (it = m_itemlist.begin(); it != m_itemlist.end(); it++)
        {
            if (it->pos.y >= m_clientrc.bottom)
                continue;
            DrawItem(dc, it->pos, 0, it->data.filename);
        }
    }
    else
    {
        for (it = m_itemlist.begin(); it != m_itemlist.end(); it++)
        {
            if (it->pos.x == m_selbox.pos.x && it->pos.y == m_selbox.pos.y)
                m_selbox.pos.y += y;
            it->pos.y += y;

            if (it->pos.y >= m_clientrc.bottom)
                continue;
            DrawItem(dc, it->pos, 0, it->data.filename);
        }
    }

}

HRESULT MediaListView::OnLBUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    if (m_selbox.pos.x && m_selbox.pos.y)
        ::MessageBox(m_parentwnd, m_selbox.data.filename.c_str(), L"", MB_OK);
    return 0;
}

HRESULT MediaListView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    bHandled = FALSE;

    return 0;
}

HRESULT MediaListView::OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
    POINT currpt;
    WTL::CClientDC dc = WTL::CClientDC(m_parentwnd);

    currpt.x = GET_X_LPARAM(lParam);
    currpt.y = GET_Y_LPARAM(lParam);

    if (m_selbox.pos.x && m_selbox.pos.y)
    {
        if (SelBox(currpt, m_selbox.pos))
            return 0;
        else
        {
            DrawItem(dc, m_selbox.pos, 0, m_selbox.data.filename);
            m_selbox.pos.x = 0;
            m_selbox.pos.y = 0;
            return 0;
        }
    }

    std::vector<MEDIA_ITEMBOX>::iterator it;
    for (it = m_itemlist.begin(); it != m_itemlist.end(); it++)
    {
        if (SelBox(currpt, it->pos))
        {
            m_selbox = *it;
            DrawItem(dc, it->pos, 1, it->data.filename);
            return 0;
        }
    }

    return 0;
}

HRESULT MediaListView::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
    bHandled = FALSE;

    //::GetClientRect(m_parentwnd, &m_clientrc);

    return 0;
}


HRESULT MediaListView::OnEraseBKGnd(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
    bHandled = FALSE;

    return 0;
}