
#include "stdafx.h"
#include "PingPongController.h"
#include "PlayerPreference.h"
#include "SPlayerDefs.h"

PingPongController::PingPongController() 
{

}

PingPongController::~PingPongController()
{
    _Stop();

}
void PingPongController::PingPong()
{
  _Stop();
  _Start();
}

void PingPongController::_Thread()
{
  refptr<pool> pool = pool::create_instance();
  refptr<task> task = task::create_instance();
  refptr<config> cfg = config::create_instance();
  refptr<request> req = request::create_instance();
  refptr<postdata> data = postdata::create_instance();

  std::wstring url = PlayerPreference::GetInstance()->GetStringVar(STRVAR_APIURL);
  url += L"/pong";

  SinetConfig(cfg, -1);
  //req->set_postdata(data);
  req->set_request_url(url.c_str());
  //req->set_request_method(REQ_POST);
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

}
