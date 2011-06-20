
#include "stdafx.h"
#include "ShareController.h"
#include <shooterapi.key>
#include "HashController.h"
#include <Strings.h>
#include "PlayerPreference.h"
#include "SPlayerDefs.h"
#include "../resource.h"
#include "../revision.h"
#include "NetworkControlerImpl.h"
#undef __MACTYPES__
#include "../../../zlib/zlib.h"
#include "base64.h"
#include "logging.h"
#include "../Utils/SPlayerGUID.h"

UserShareController::UserShareController() : 
m_retdata(L""),
m_parentwnd(NULL)
{

}

UserShareController::~UserShareController()
{
  _Stop();
}

void UserShareController::SetCommentPlaneParent(HWND hwnd)
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

    sprintf_s(buf, 4096, APIKEY, SVP_REV_NUMBER, uuidstr.c_str(), sphash.c_str(), "");

    return HashController::GetInstance()->GetMD5Hash(buf, strlen(buf));
}

void UserShareController::ShareMovie(std::wstring uuid, std::wstring sphash, std::wstring film)
{
  _Stop();
  if (uuid.empty() || sphash.empty() || film.empty())
    return;

  m_uuid = uuid;
  m_sphash = sphash;
  m_film = film;
  _Start();
}

std::wstring UserShareController::EncodeString(std::wstring str)
{
  std::string tmpstr =  Strings::WStringToUtf8String(str);
  tmpstr = base64_encode((unsigned char*)tmpstr.c_str(), tmpstr.length());
  return Strings::Utf8StringToWString(tmpstr);
}

void UserShareController::_Thread()
{
  refptr<pool> pool = pool::create_instance();
  refptr<task> task = task::create_instance();
  refptr<config> cfg = config::create_instance();
  refptr<request> req = request::create_instance();
  refptr<postdata> data = postdata::create_instance();
  std::map<std::wstring, std::wstring> postform;
  PlayerPreference* pref = PlayerPreference::GetInstance();

  postform[L"uuid"] = m_uuid;
  postform[L"sphash"] = m_sphash;
  postform[L"spkey"] = GenerateKey();
  MapToPostData(data, postform);

  std::wstring url = pref->GetStringVar(STRVAR_APIURL);
  url += L"/share";
  
  wchar_t getdata[300];
  wsprintf(getdata, L"?sphash=%s&uuid=%s&spkey=%s", 
    m_sphash.c_str(), m_uuid.c_str(), (postform[L"spkey"]).c_str());
  url += getdata;

  std::wstring pcname = SPlayerGUID::GetComputerName();
  std::wstring loginuser = SPlayerGUID::GetUserName();

  si_stringmap rps_headers;
  rps_headers[L"Film"] =  EncodeString(m_film);
  rps_headers[L"PcName"] = EncodeString(pcname);
  rps_headers[L"User"] = EncodeString(loginuser);
  req->set_request_header(rps_headers);

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

  si_buffer buffer = req->get_response_buffer();
  buffer.push_back(0);
  
  std::string results = (char*)&buffer[0];
  m_retdata = Strings::Utf8StringToWString(results);

  ::PostMessage(m_parentwnd, WM_COMMAND, ID_MOVIESHARE_OPEN, NULL);
}

BOOL UserShareController::OpenShooterMedia()
{
  if (m_retdata.empty())
    return FALSE;

  if (m_commentplane.m_hWnd && m_commentplane.m_initialize)
    m_commentplane.Navigate(m_retdata.c_str());
  return TRUE;
}

BOOL UserShareController::CloseShooterMedia()
{
  if (m_commentplane.m_hWnd && m_commentplane.m_initialize)
  {
    m_commentplane.DestroyWindow();
    m_commentplane.m_hWnd = NULL;
    m_commentplane.m_initialize = NULL;
  }
  return TRUE;
}

BOOL UserShareController::ToggleCommentPlane()
{
  BOOL ret = TRUE;
  if (m_commentplane.m_hWnd && m_commentplane.IsWindowEnabled())
  {
    HideCommentPlane();
    ret = FALSE;
  }
  else
    ShowCommentPlane();

  return ret;
}

BOOL UserShareController::ShowCommentPlane()
{
  if (!m_commentplane.m_hWnd)
    m_commentplane.CreateFrame(DS_SETFONT|DS_FIXEDSYS|WS_POPUP|WS_DISABLED,WS_EX_NOACTIVATE);

  m_commentplane.ShowFrame();
  return TRUE;
}

void UserShareController::HideCommentPlane()
{
  if (!m_commentplane.m_hWnd || !m_parentwnd)
    return;
  m_commentplane.HideFrame();
}

void UserShareController::CalcCommentPlanePos()
{
  if (!m_commentplane.m_hWnd || !m_parentwnd)
    return;

  m_commentplane.CalcWndPos();
  if (m_commentplane.IsWindowVisible())
    m_commentplane.ShowFrame();
}

BOOL UserShareController::CloseShareWnd()
{
  BOOL ret = FALSE;
  if (m_commentplane.IsWindowVisible())
  {
    ret = TRUE;
    m_commentplane.HideFrame();
  }

  if (m_commentplane.m_oadlg && m_commentplane.m_oadlg->IsWindowVisible())
  {
    ret = TRUE;
    m_commentplane.CloseOAuth();
  }

  return ret;
}