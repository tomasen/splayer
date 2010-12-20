#pragma once

typedef struct
{
    std::wstring filename;
    std::wstring imgpath;
} MEDIA_ITEMDATA;

class MediaListView
{
public:
    BEGIN_MSG_MAP(MediaListView)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_LBUTTONUP, OnLBUp)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBKGnd)
    END_MSG_MAP()

public:
    MediaListView();
    ~MediaListView();

    typedef struct
    {
        POINT pos;
        MEDIA_ITEMDATA data;
    } MEDIA_ITEMBOX;

public:
    HRESULT OnLBUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    HRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
    HRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    HRESULT OnEraseBKGnd(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    HRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

public:
    void AddListView(HWND hWnd);
    void AttachData(std::vector<MEDIA_ITEMDATA>& data);
    void SetItemBox(int w, int h, const RECT& margin);

    void DrawItem(WTL::CDC& dc, POINT pt, int cell, std::wstring text);
    void DrawListView(WTL::CDC& dc, int y=0);

private:
    BOOL SelBox(POINT currpos, POINT boxpos);

private:
    HWND m_parentwnd;

    RECT m_boxmargin;
    int m_boxwidth;
    int m_boxheight;

    RECT m_clientrc;
    MEDIA_ITEMBOX m_selbox;
    std::vector<MEDIA_ITEMBOX> m_itemlist;

    // default box
    WTL::CBitmap m_defaultboxbmp;
    WTL::CDC m_memboxbmp;
    HBITMAP m_oldboxbmp;
};