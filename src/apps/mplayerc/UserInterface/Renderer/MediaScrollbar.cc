#include "stdafx.h"
#include "MediaScrollbar.h"
#include "../../resource.h"

MediaScrollbar::MediaScrollbar():m_offsetdistance(0),m_scrollchange(FALSE)
{

}

MediaScrollbar::~MediaScrollbar()
{
    m_memscrollbmp.SelectBitmap(m_oldscrollbmp);
    m_memscrollbmp.DeleteDC();
}

HRESULT MediaScrollbar::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
    bHandled = FALSE;

    if (wParam == 1000 && m_scrollchange)
    {
        if (m_offsetdistance>0)
            m_offsetdistance += 5;
        else
            m_offsetdistance -= 5;
        ::InvalidateRect(m_parentwnd, &m_clientrc, TRUE);
    }

    return 0;
}

HRESULT MediaScrollbar::OnLBDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    bHandled = FALSE;

    ::GetCursorPos(&m_lbdownpos);
    ::ScreenToClient(m_parentwnd, &m_lbdownpos);

    if (PtInRect(&m_scrollrc, m_lbdownpos))
    {
        ::SetCapture(m_parentwnd);
        ::SetTimer(m_parentwnd, 1000, 100, NULL);
    }
    
    return 0;
}

HRESULT MediaScrollbar::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    bHandled = FALSE;

    //ResetScrollbarPos();

    return 0;
}

HRESULT MediaScrollbar::OnLBUp(UINT , WPARAM , LPARAM , BOOL& bHandled)
{
    m_lbdownpos.x = 0;
    m_lbdownpos.y = 0;

    
    if (m_scrollchange)
    {
        ResetScrollbarPos();

        ::ReleaseCapture();
        ::KillTimer(m_parentwnd, 1000);
        m_scrollchange = FALSE;
    }

    bHandled = FALSE;
    return 0;
}

HRESULT MediaScrollbar::OnMouseMove(UINT , WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = FALSE;

    POINT currpt;
    currpt.x = GET_X_LPARAM(lParam);
    currpt.y = GET_Y_LPARAM(lParam);

    if (PtInRect(&m_scrollrc, currpt) && MK_LBUTTON == wParam)
    {
        m_offsetdistance = m_lbdownpos.y - currpt.y;//currpt.y - m_lbdownpos.y;
        m_scrollrc.top -= m_offsetdistance;
        m_scrollrc.bottom -= m_offsetdistance;
        m_scrollbarpos.y -= m_offsetdistance;
        
        m_lbdownpos.y = currpt.y;
        m_scrollchange = TRUE;

        if (m_scrollbarpos.y+m_bmp.bmHeight >= m_clientrc.bottom)
        {
            m_scrollbarpos.y = m_clientrc.bottom - m_bmp.bmHeight;

            m_scrollrc.top = m_scrollbarpos.y;
            m_scrollrc.bottom = m_scrollrc.top + m_bmp.bmHeight;
        }
        else if (m_scrollbarpos.y <= m_clientrc.top)
        {
            m_scrollbarpos.y = m_clientrc.top;

            m_scrollrc.top = m_scrollbarpos.y;
            m_scrollrc.bottom = m_bmp.bmHeight;
        }

        ::InvalidateRect(m_parentwnd, &m_clientrc, TRUE);
    }

    return 0;
}

HRESULT MediaScrollbar::OnPaint(UINT , WPARAM , LPARAM , BOOL& bHandled)
{
    bHandled = FALSE;

    return 0;
}

BOOL MediaScrollbar::GetScrollState()
{
    return m_scrollchange;
}

void MediaScrollbar::DrawScrollbar(WTL::CDC& dc)
{
    dc.BitBlt(m_scrollbarpos.x, m_scrollbarpos.y, 23, 90, m_memscrollbmp, 0, 0, SRCCOPY);
}

void MediaScrollbar::ResetScrollbarPos()
{
    m_scrollbarpos.x = m_clientrc.right-50;
    m_scrollbarpos.y = m_clientrc.bottom/3;
    
    m_scrollrc.top = m_scrollbarpos.y;
    m_scrollrc.left = m_scrollbarpos.x;
    m_scrollrc.right = m_scrollbarpos.x+m_bmp.bmWidth;
    m_scrollrc.bottom = m_scrollbarpos.y+m_bmp.bmHeight;

    ::InvalidateRect(m_parentwnd, &m_clientrc, TRUE);
}

void MediaScrollbar::GetBlockRect(RECT& rect)
{
    rect = m_scrollrc;
}

void MediaScrollbar::AddScrollbar(HWND hWnd)
{
    m_parentwnd = hWnd;
    ::GetClientRect(m_parentwnd, &m_clientrc);

    m_defaultscrollbmp.LoadBitmap(IDB_BITMAP2);
    m_defaultscrollbmp.GetBitmap(m_bmp);

    m_memscrollbmp.CreateCompatibleDC(WTL::CClientDC(m_parentwnd));
    m_oldscrollbmp = m_memscrollbmp.SelectBitmap(m_defaultscrollbmp);

    ResetScrollbarPos();
}

int MediaScrollbar::GetOffsetVal()
{
    return m_offsetdistance;
}