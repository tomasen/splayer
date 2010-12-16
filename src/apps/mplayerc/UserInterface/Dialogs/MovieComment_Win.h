
#pragma once

#include <atlcomcli.h>

class MovieComment :
    public ATL::CWindowImpl<MovieComment>

{
public:
    DECLARE_WND_CLASS(L"Movie CommentView")
    
    BEGIN_MSG_MAP(MovieComment)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
    END_MSG_MAP()

    MovieComment();
    ~MovieComment();

    void OpenUrl(std::wstring url);
    HRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

private:
    CComQIPtr<IWebBrowser2> m_ie;
};