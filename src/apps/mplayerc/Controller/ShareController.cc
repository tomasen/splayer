
#include "stdafx.h"
#include "ShareController.h"
#include <shooterapi.key>
#include <Version.h>
#include "HashController.h"
#include <Strings.h>
#include "PlayerPreference.h"
#include "SPlayerDefs.h"

UserShareController::UserShareController() : m_retdata(L""), m_parentwnd(NULL)
{

}

UserShareController::~UserShareController()
{
    _Stop();
}

void UserShareController::SetParentWnd(HWND hwnd)
{
    m_parentwnd = hwnd;
}

std::wstring UserShareController::GetResponseData()
{
    return m_retdata;
}

std::wstring UserShareController::GenerateKey()
{
    char buf[4096];

    std::string uuidstr = Strings::WStringToUtf8String(m_uuid);
    std::string sphash = Strings::WStringToUtf8String(m_sphash);

    sprintf_s(buf, 4096, APIKEY, VERSION_REV, uuidstr.c_str(), sphash.c_str(), "");

    return HashController::GetInstance()->GetMD5Hash(buf, strlen(buf));
}

void UserShareController::ShareMovie(std::wstring uuid, std::wstring sphash)
{
    _Stop();
    m_uuid = uuid;
    m_sphash = sphash;
    m_abcdf = L"test.....";
    _Start();
}

void UserShareController::_Thread()
{
    if (!m_parentwnd)
        return;

    refptr<pool> pool = pool::create_instance();
    refptr<task> task = task::create_instance();
    refptr<request> req = request::create_instance();
    refptr<postdata> data = postdata::create_instance();
    std::map<std::wstring, std::wstring> postform;
    PlayerPreference* pref = PlayerPreference::GetInstance();

    postform[L"uuid"] = m_uuid;
    postform[L"sphash"] = m_sphash;
    postform[L"spkey"] = GenerateKey();
    MapToPostData(data, postform);

    std::wstring url = pref->GetStringVar(STRVAR_APIURL);
    //url += L"/share";
    url += L"/share.php";
    req->set_postdata(data);
    req->set_request_url(url.c_str());
    req->set_request_method(REQ_POST);

    task->append_request(req);
    pool->execute(task);

    while (pool->is_running_or_queued(task))
    {
        if (_Exit_state(500))
        {
            return;
        }
    }

    if (req->get_response_errcode() != 0)
    {
        return;
    }

    si_buffer buffer = req->get_response_buffer();
    buffer.push_back(0);

    std::string results = (char*)&buffer[0];
    m_retdata = Strings::Utf8StringToWString(results);

    // Send messages to the main thread
    PostMessage(m_parentwnd, ID_USERSHARE_SUCCESS, NULL, NULL);
}

void UserShareController::ShowCommentGui()
{
    if (m_retdata.empty() || m_parentwnd == NULL)
        return;

    m_commentgui.Create(m_parentwnd);
    m_commentgui.OpenUrl(m_retdata);
    m_commentgui.ShowWindow(SW_SHOW);
}

void UserShareController::HideCommentGui()
{
    m_commentgui.ShowWindow(SW_HIDE);
}