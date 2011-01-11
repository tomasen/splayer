
#pragma once

#include "LazyInstance.h"
#include <threadhelper.h>
#include "NetworkControlerImpl.h"
#include "../UserInterface/Dialogs/MovieComment_Win.h"
#include <comutil.h>

#define ID_MOVIESHARE_RESPONSE 32932
class UserShareController:
    public NetworkControlerImpl,
    public ThreadHelperImpl<UserShareController>,
    public LazyInstanceImpl<UserShareController>
{
public:

    UserShareController();
    ~UserShareController();

    void _Thread();
    void ShareMovie(std::wstring uuid, std::wstring sphash);

    void CreateCommentPlane(HWND hwnd);
    void CalcCommentPlanePos();
    BOOL ShowCommentPlane();
    void HideCommentPlane();

    std::wstring GenerateKey();
    std::wstring GetResponseData();

private:

    std::wstring m_uuid;
    std::wstring m_sphash;
    std::wstring m_retdata;

    MovieComment m_commentplane;
    HWND m_parentwnd;
};
