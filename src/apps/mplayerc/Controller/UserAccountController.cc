#include "stdafx.h"
#include "UserAccountController.h"
#include "..\Utils\SPlayerGUID.h"
#include "..\..\..\base\Strings.h"
#include "PlayerPreference.h"
#include "SPlayerDefs.h"

UserAccountController::UserAccountController()
{

}

UserAccountController::~UserAccountController()
{

}

void UserAccountController::_Thread()
{
  refptr<pool> pool = pool::create_instance();
  refptr<task> task = task::create_instance();
  refptr<config> cfg = config::create_instance();
  refptr<request> req = request::create_instance();
  refptr<postdata> data = postdata::create_instance();

  std::wstring splayer_uuid;
  SPlayerGUID::GenerateGUID(splayer_uuid);
  std::wstring url = L"https://www.shooter.cn/api/v2/checkBind?uuid=" + splayer_uuid;
  SinetConfig(cfg, -1);

  req->set_request_url(url.c_str());
  req->set_request_method(REQ_GET);

  task->use_config(cfg);
  task->append_request(req);
  pool->execute(task);

  while (pool->is_running_or_queued(task))
  {
    if (_Exit_state(100))
      return;
  }
  if (req->get_response_errcode() != 0)
    return;

  std::vector<unsigned char> st_buffer = req->get_response_buffer();

  if (st_buffer.empty())
    return;

  if (st_buffer[st_buffer.size() - 1] != '\0')
    st_buffer.push_back('\0');

  std::wstring sAccountName = Strings::Utf8StringToWString(std::string(st_buffer.begin(), st_buffer.end()));
  PlayerPreference::GetInstance()->SetStringVar(STRVAR_USER_ACCOUNT_NAME, sAccountName);
}