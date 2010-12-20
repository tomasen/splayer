
#include "stdafx.h"
#include "ShareController.h"

UserShareController::UserShareController():
    m_apiret(UNKNOWN)
{

}

UserShareController::~UserShareController()
{
    _Stop();
}

void UserShareController::Share(std::wstring uuidstr, std::wstring hash)
{
    _Stop();
    m_uuid = uuidstr;
    m_sphash = hash;
    _Start();
}

void UserShareController::_Thread()
{
    refptr<pool> pool = pool::create_instance();
    refptr<task> task = task::create_instance();
    refptr<request> req = request::create_instance();
    refptr<postdata> data = postdata::create_instance();
    std::map<std::wstring, std::wstring> postform;

    postform[L"uuid"] = m_uuid;
    postform[L"sphash"] = m_sphash;
    MapToPostData(data, postform);

    req->set_postdata(data);
    req->set_request_url(L"");
    req->set_request_method(REQ_POST);

    task->append_request(req);
    pool->execute(task);

    while (pool->is_running_or_queued(task))
    {
        if (_Exit_state(500))
        {
            m_apiret = UNKNOWN;
            return;
        }
    }

    if (req->get_response_errcode() != 0)
    {
        m_apiret = UNKNOWN;
        return;
    }

    si_buffer buffer = req->get_response_buffer();
    buffer.push_back(0);

    std::string str = (char*)&buffer[0];
    size_t ret = atoi(str.c_str());
    m_apiret = (SP_APIRET)ret;

    // By response code send messages to the main thread
    //PostMessage()
}