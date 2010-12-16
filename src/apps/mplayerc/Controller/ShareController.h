
#pragma once

#include "LazyInstance.h"
#include <threadhelper.h>
#include "NetworkControlerImpl.h"
#include "../UserInterface/Dialogs/MovieComment_Win.h"

#define ID_USERSHARE_SUCCESS 32932

class UserShareController:
    public NetworkControlerImpl,
    public ThreadHelperImpl<UserShareController>
    //public LazyInstanceImpl<UserShareController>
{
public:

    UserShareController();
    ~UserShareController();

    void _Thread();
    void SetParentWnd(HWND hwnd);
    void ShareMovie(std::wstring uuid, std::wstring sphash);

    void ShowCommentGui();
    void HideCommentGui();

    std::wstring GenerateKey();
    std::wstring GetResponseData();


private:
    HWND m_parentwnd;

    std::wstring m_uuid;
    std::wstring m_sphash;
    std::wstring m_retdata;

    std::wstring m_abcdf;

    MovieComment m_commentgui;
};
