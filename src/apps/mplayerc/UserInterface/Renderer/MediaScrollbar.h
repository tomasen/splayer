#pragma once

class MediaScrollbar
{
public:
    MediaScrollbar();
    ~MediaScrollbar();

    BEGIN_MSG_MAP(MediaScrollbar)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_LBUTTONUP, OnLBUp)
        MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLBDown)
        MESSAGE_HANDLER(WM_TIMER, OnTimer)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
    END_MSG_MAP()

    HRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    HRESULT OnLBUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    HRESULT OnLBDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    HRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
    HRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    HRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

    void AddScrollbar(HWND hWnd);
    void DrawScrollbar(WTL::CDC& dc);
    void ResetScrollbarPos();
    void GetBlockRect(RECT& rect);
    BOOL GetScrollState();

    void SetListPlane(int w, int h);
    void SetClientRect(const RECT& rc);
    int GetOffsetVal();

private:
    HWND m_parentwnd;
    RECT m_clientrc;
    WTL::CDC m_dc;

    BITMAP m_bmp;
    // scrollbar
    WTL::CBitmap m_defaultscrollbmp;
    WTL::CDC m_memscrollbmp;
    HBITMAP m_oldscrollbmp;
    
    int m_planewidth;
    int m_planeheight;

    POINT m_lbdownpos;
    POINT m_scrollbarpos;
    RECT m_scrollrc;

    int m_offsetdistance;
    BOOL m_scrollchange;
};