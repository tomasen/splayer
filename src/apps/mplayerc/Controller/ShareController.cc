
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

UserShareController::UserShareController() : 
m_retdata(L""),
m_parentwnd(NULL),
_dialogTemplate(NULL)
{

}

UserShareController::~UserShareController()
{
    _Stop();

    if (_dialogTemplate)
      free(_dialogTemplate);
}
void UserShareController::SetCommentPlaneParent(HWND hwnd)
{
  m_parentwnd = hwnd;
}

void UserShareController::CreateCommentPlane()
{
  if (m_commentplane.m_hWnd || !m_parentwnd || _dialogTemplate)
    return;

  // extra space are in order for successfully creating dialog
  _dialogTemplate = (DLGTEMPLATE*)calloc(1, sizeof(DLGTEMPLATE)+sizeof(DLGITEMTEMPLATE)+10);

  if (_dialogTemplate)
  {
    _dialogTemplate->style = DS_SETFONT | DS_FIXEDSYS | WS_POPUP | WS_DISABLED;
    _dialogTemplate->dwExtendedStyle = WS_EX_NOACTIVATE; //WS_EX_TOPMOST

    _dialogTemplate->cdit  = 0;
    _dialogTemplate->cx    = 316;
    _dialogTemplate->cy    = 183;


    if (0 == m_commentplane.CreateIndirect(_dialogTemplate, CWnd::FromHandle(m_parentwnd)))
    {
      free(_dialogTemplate);
      _dialogTemplate = 0;
      Logging(L"m_commentplane.CreateIndirect failed");
    }
  }
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

void UserShareController::_Thread()
{
  refptr<pool> pool = pool::create_instance();
  refptr<task> task = task::create_instance();
  refptr<config> cfg = config::create_instance();
  refptr<request> req = request::create_instance();
  refptr<postdata> data = postdata::create_instance();
  std::map<std::wstring, std::wstring> postform;
  PlayerPreference* pref = PlayerPreference::GetInstance();
  
  std::string filmstr =  Strings::WStringToUtf8String(m_film);
  filmstr = base64_encode((unsigned char*)filmstr.c_str(), filmstr.length());

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

  si_stringmap rps_headers;
  rps_headers[L"Film"] =  Strings::Utf8StringToWString(filmstr);
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

  CreateCommentPlane();
  for(int i = 0; i < 100; i++) // wait 10 sec till dialog init
  {
    if (m_commentplane.m_initialize == 0)
      Sleep(100);
    else
    {
      ::PostMessage(m_parentwnd, WM_COMMAND, ID_MOVIESHARE_OPEN, NULL);
      break;
    }
  }

}

BOOL UserShareController::OpenShooterMedia()
{
  if (m_retdata.empty())
    return FALSE;

  m_commentplane.Navigate(m_retdata.c_str());
  return TRUE;
}

void UserShareController::ToggleCommentPlane()
{
  if (m_commentplane.m_hWnd && m_commentplane.IsWindowEnabled())
    HideCommentPlane();
  else
    ShowCommentPlane();
}
BOOL UserShareController::ShowCommentPlane()
{
  //if (m_retdata.empty())
  //    return FALSE;

  CreateCommentPlane();
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