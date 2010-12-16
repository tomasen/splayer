#include "stdafx.h"
#include "MovieComment_Win.h"

MovieComment::MovieComment()
{

}

MovieComment::~MovieComment()
{

}

HRESULT MovieComment::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    CComPtr<IUnknown> webctrl;
    CAxWindow webwindow;
    RECT parentrc;
    int width, height;

    GetParent().GetClientRect(&parentrc);
    width = (parentrc.right-parentrc.left)/2;
    height = (parentrc.bottom-parentrc.top)/2;
    RECT rc = {0, 0, 373, 161};
    SetWindowPos(HWND_TOP, &rc, SWP_NOACTIVATE|SWP_HIDEWINDOW);

    RECT webrc = {0, 0, width, height};
    webwindow.Create(m_hWnd, webrc, L"", WS_CHILD|WS_VISIBLE);
    if (webwindow.m_hWnd)
    {
        HRESULT hr = webwindow.CreateControlEx(L"{8856F961-340A-11D0-A96B-00C04FD705A2}", 
            NULL, NULL, &webctrl);
        if (SUCCEEDED(hr) && webctrl)
            m_ie = webctrl;
    }

    return 0;
}

void MovieComment::OpenUrl(std::wstring url)
{
    if (m_ie == NULL)
        return;

    CComVariant v;
    m_ie->Navigate(CComBSTR(url.c_str()), &v, &v, &v, &v);
}