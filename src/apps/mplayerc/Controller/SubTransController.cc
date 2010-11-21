#include "stdafx.h"
#include "../Model/SubTransFormat.h"
#include "SubTransController.h"
#include "../Resource.h"
#include "HashController.h"
#include "../Utils/Strings.h"
#include <sinet.h>
#include <sys/stat.h>
#include <sphash.h>
#include "PlayerPreference.h"
#include "SPlayerDefs.h"
#include "../revision.h"

using namespace sinet;


void SinetConfig(refptr<config> cfg, int sid, std::wstring oem)
{
  wchar_t agentbuff[MAX_PATH];
  if(oem.empty())
    wsprintf(agentbuff, L"SPlayer Build %d", SVP_REV_NUMBER);
  else
    wsprintf(agentbuff, L"SPlayer Build %d OEM%s", SVP_REV_NUMBER ,oem.c_str());
  cfg->set_strvar(CFG_STR_AGENT, agentbuff);

  std::wstring proxy;
  if (sid%2 == 0)
  {
    DWORD ProxyEnable = 0;
    wchar_t ProxyServer[256];
    DWORD ProxyPort = 0;

    ULONG len;
    CRegKey key;
    if( ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"), KEY_READ)
      && ERROR_SUCCESS == key.QueryDWORDValue(_T("ProxyEnable"), ProxyEnable) && ProxyEnable
      && ERROR_SUCCESS == key.QueryStringValue(_T("ProxyServer"), ProxyServer, &len))
    {
      proxy += L"http://";
      std::wstring proxystr(ProxyServer);
      proxy += proxystr.c_str();
    }
  }
 
  if (proxy.empty())
    cfg->set_strvar(CFG_STR_PROXY, L"");
  else
    cfg->set_strvar(CFG_STR_PROXY, proxy);
}

void StringMap2PostData(refptr<postdata> data ,std::map<std::wstring, std::wstring> &postform)
{
  for (std::map<std::wstring, std::wstring>::iterator it = postform.begin();
    it != postform.end(); it++)
  {
    refptr<postdataelem> elem = postdataelem::create_instance();
    elem->set_name((it->first).c_str());
    elem->setto_text((it->second).c_str());
    data->add_elem(elem);
  }
}

std::wstring GetServerUrl(int req_type , int tryid)
{

  std::wstring apiurl;
  wchar_t str[100] = L"https://www.shooter.cn/";

  if (tryid > 1 && tryid <= 11)
  {
    if (tryid >= 4)
    {
      int iSvrId = 4 + rand()%7;    
      if (tryid%2)
        wsprintf(str, L"https://splayer%d.shooter.cn/", iSvrId-1);
      else
        wsprintf(str, L"http://splayer%d.shooter.cn/", iSvrId-1);
    }
    else
      wsprintf(str, L"https://splayer%d.shooter.cn/", tryid-1);
  }
  else if (tryid > 11)
    wsprintf(str, L"http://svplayer.shooter.cn/");

  apiurl.assign(str);
  switch(req_type)
  {
    case 'upda':
      apiurl += L"api/updater.php";
      break;
    case 'upsb':
      apiurl += L"api/subup.php";
      break;
    case 'sapi':
      apiurl += L"api/subapi.php";
      break;
  }
  return apiurl;
}

int FindAllSubfile(std::wstring szSubPath , std::vector<std::wstring>* szaSubFiles)
{
  szaSubFiles -> clear();
  szaSubFiles -> push_back(szSubPath);
  std::vector<std::wstring> szaPathInfo;
  std::wstring szBaseName = SubTransFormat::GetVideoFileBasename(szSubPath.c_str(), &szaPathInfo);
  std::wstring szExt = szaPathInfo.at(1);

  if(szExt == L".idx")
  {
    szSubPath = szBaseName + L".sub";
    if(SubTransFormat::IfFileExist_STL(szSubPath.c_str()))
      szaSubFiles -> push_back(szSubPath);
    else
    {
      szSubPath = szBaseName + L".rar";
      if(SubTransFormat::IfFileExist_STL(szSubPath.c_str()))
      {
        std::wstring szSubFilepath = SubTransFormat::ExtractRarFile(szSubPath);
        if(!szSubFilepath.empty())
          szaSubFiles -> push_back(szSubFilepath);
      }
    }
  }
  //TODO: finding other subfile
  return 0;
}

void WetherNeedUploadSub(refptr<pool> pool, refptr<task> task, refptr<request> req,
                        std::wstring fnVideoFilePath, std::wstring szFileHash, 
                        std::wstring fnSubHash, int iDelayMS, int sid, std::wstring oem)
{
  std::map<std::wstring, std::wstring> postform;
  refptr<config> cfg = config::create_instance();

  postform[L"pathinfo"] = fnVideoFilePath;
  postform[L"filehash"] = szFileHash;
  postform[L"subhash"]  = fnSubHash;
 
  wchar_t delay[32];
  _itow_s(iDelayMS, delay, 32, 10);
  postform[L"subdelay"] = delay;

  refptr<postdata> data = postdata::create_instance();
  StringMap2PostData(data, postform);

  int rret = -1;
  SinetConfig(cfg, sid, oem);

  std::wstring url = GetServerUrl('upsb', sid);
  req->set_request_method(REQ_POST);
  req->set_postdata(data);

  task->use_config(cfg);
  task->append_request(req);
  pool->execute(task);
}

void UploadSubFileByVideoAndHash(refptr<pool> pool,refptr<task> task,
                                refptr<request> req,
                                std::wstring fnVideoFilePath,
                                std::wstring szFileHash,
                                std::wstring szSubHash,
                                std::vector<std::wstring>* fnSubPaths,
                                int iDelayMS, int sid, std::wstring oem)
{
  refptr<config> cfg = config::create_instance();
  std::map<std::wstring, std::wstring> postform;
  
  postform[L"pathinfo"] = fnVideoFilePath;
  postform[L"filehash"] = szFileHash;
  postform[L"subhash"]  = szSubHash;
  
  wchar_t delay[32];
  _itow_s(iDelayMS, delay, 32, 10);
  postform[L"subdelay"] = delay;

  std::wstring vhash = SubTransFormat::GetHashSignature(Strings::WStringToUtf8String(fnVideoFilePath).c_str(),
    Strings::WStringToUtf8String(szFileHash).c_str());
  if (!vhash.empty())
    postform[L"vhash"]  = vhash;

  refptr<postdata> data = postdata::create_instance();
  StringMap2PostData(data, postform);

  size_t iTotalFiles = fnSubPaths->size();
  for (size_t i = 0; i < iTotalFiles; i++)
  {
    wchar_t szFname[22];
    /* Fill in the file upload field */
    std::wstring szgzFile = SubTransFormat::GetSameTmpName(fnSubPaths->at(i));
    SubTransFormat::PackGZfile(fnSubPaths->at(i), szgzFile);

    wsprintf(szFname, L"subfile[%d]", i);
    refptr<postdataelem> elem = postdataelem::create_instance();
    elem->set_name(szFname);
    elem->setto_file(szgzFile.c_str());
    data->add_elem(elem);
  }
  
  SinetConfig(cfg, sid, oem);

  std::wstring url = GetServerUrl('upsb', sid);
  req->set_request_url(url.c_str());
  req->set_request_method(REQ_POST);
  req->set_postdata(data);

  task->use_config(cfg);
  task->append_request(req);
  pool->execute(task);
}

SubTransController::SubTransController(void):
  m_stopevent(::CreateEvent(NULL, TRUE, FALSE, NULL)),
  m_thread(NULL),
  m_operation(Unknown)
{
}

SubTransController::~SubTransController(void)
{
  Stop();
}

void SubTransController::SetFrame(HWND hwnd)
{
  m_frame = hwnd;
}

void SubTransController::SetOemTitle(std::wstring str)
{
  m_oemtitle = str;
}

void SubTransController::SetLanuage(std::wstring str)
{
  m_language = str;
}

void SubTransController::SetSubperf(std::wstring str)
{
  m_subperf = str;
}

void SubTransController::Start(const wchar_t* video_filename, 
                               SubTransOperation operation, 
                               StringList files_upload /*= StringList()*/)
{
  // we should stop running tasks first
  Stop();
  // record parameters
  m_operation = operation;
  m_videofile.assign(video_filename);
  // create thread
  m_thread = (HANDLE)::_beginthread(_thread_dispatch, 0, (void*)this);
}

void SubTransController::Stop()
{
  unsigned long thread_exitcode;
  if (m_thread && m_thread != INVALID_HANDLE_VALUE &&
    GetExitCodeThread(m_thread, &thread_exitcode) &&
    thread_exitcode == STILL_ACTIVE)
  {
    ::SetEvent(m_stopevent);
    ::WaitForSingleObject(m_thread, INFINITE);
  }
  m_thread = NULL;
  ::ResetEvent(m_stopevent);
}

void SubTransController::_thread_dispatch(void* param)
{
  static_cast<SubTransController*>(param)->_thread();
}

void SubTransController::_thread()
{
  switch (m_operation)
  {
  case DownloadSubtitle:
    _thread_download();
    break;
  case UploadSubtitle:
    _thread_upload();
    break;
  }
}

void SubTransController::_thread_download()
{
  std::vector<std::wstring> subtitles;
  std::wstring szLang = m_language;

  
  std::vector<std::wstring> szaSubDescs, tmpfiles;
  HashController::GetInstance()->SetFileName(m_videofile.c_str());
  std::wstring szFileHash = HashController::GetInstance()->GetHash();
  
  refptr<pool> pool = pool::create_instance();
  refptr<task> task = task::create_instance();
  refptr<config> cfg = config::create_instance();
  if (m_handlemsgs)
    m_handlemsgs->push_back(Strings::ResourceString(IDS_LOG_MSG_USING_SVPSUB_SYSTEM_LOOKINGFOR_SUB));

  for (int i = 1; i <= 7; i++)
  {
    std::map<std::wstring, std::wstring> postform;
    std::wstring vhash, shortname, url, tmpoutfile;

    tmpoutfile = SubTransFormat::GetTempFileName();
    if (tmpoutfile.empty())
      return;

    refptr<request> req = request::create_instance();
    refptr<postdata> data = postdata::create_instance();

    url = GetServerUrl('sapi', i);
    vhash = SubTransFormat::GetHashSignature(Strings::WStringToUtf8String(m_videofile).c_str(),
                            Strings::WStringToUtf8String(szFileHash).c_str());
    shortname = SubTransFormat::GetShortFileNameForSearch(m_videofile);

    if (!vhash.empty())
      postform[L"vhash"]  = vhash;

    if (!m_subperf.empty())
      postform[L"perf"] = m_subperf;

    if (!szLang.empty())
      postform[L"lang"] = szLang;

    postform[L"pathinfo"] = m_videofile;
    postform[L"filehash"] = szFileHash;
    postform[L"shortname"] = shortname;
    StringMap2PostData(data, postform);

    req->set_postdata(data);
    req->set_request_url(url.c_str());
    req->set_request_method(REQ_POST);
    req->set_request_outmode(REQ_OUTFILE);
    req->set_outfile(tmpoutfile.c_str());

    SinetConfig(cfg, i, m_oemtitle);
    task->use_config(cfg);
    task->append_request(req);
    pool->execute(task);

    while (pool->is_running_or_queued(task))
    {
      if (::WaitForSingleObject(m_stopevent, 1000) == WAIT_OBJECT_0)
        return;
    }

    if (req->get_response_errcode() != 0)
    {
      if (m_handlemsgs)
        m_handlemsgs->push_back(ResStr_STL(IDS_LOG_MSG_SVPSUB_NONE_MATCH_SUB));
      break;
    }

    if(0 == SubTransFormat::ExtractDataFromAiSubRecvBuffer_STL(m_handlemsgs, m_videofile,
                              tmpoutfile, szaSubDescs,tmpfiles))
      break;

    if (::WaitForSingleObject(m_stopevent, 2300) == WAIT_OBJECT_0)
      return;
  }

  //load sub file to sublist
  std::wstring szSubFilePath;
  int iSubTotal = 0;
  for (size_t i = 0; i < tmpfiles.size(); i++)
  {
    szSubFilePath = SubTransFormat::GetSubFileByTempid_STL(i,
      m_videofile, szaSubDescs, tmpfiles);

    if (!szSubFilePath.empty() && szSubFilePath != L"EXIST" )
    {
      subtitles.push_back(szSubFilePath);
      iSubTotal++;
    }
  }

  if (iSubTotal > 1)
  {
    if (!m_subperf.empty())
    {
      for (std::vector<std::wstring>::iterator iter = subtitles.begin();
        iter != subtitles.end(); iter++)
        if (SubTransFormat::IsSpecFanSub((*iter), m_subperf))
        {
          std::wstring szFirst = subtitles.at(0);
          subtitles[0] = szSubFilePath;
          *iter = szFirst;
          break;
        }
    }
  }
  
  PlayerPreference::GetInstance()->SetStrArray(STRARRAY_QUERYSUBTITLE, subtitles);
  ::SendMessage(m_frame, WM_COMMAND, ID_COMPLETE_QUERY_SUBTITLE, NULL);
  
}

void SubTransController::_thread_upload()
{
  HashController::GetInstance()->SetFileName(m_videofile.c_str());
  std::wstring szFileHash = HashController::GetInstance()->GetHash();

  std::vector<std::wstring> szaSubFiles;
  FindAllSubfile(m_subfile.c_str(), &szaSubFiles);

  std::wstring subfilestr;
  for (std::vector<std::wstring>::iterator it = szaSubFiles.begin();
    it != szaSubFiles.end(); it++)
  {
    if (it != szaSubFiles.begin())
      subfilestr += L"\0";
    subfilestr += (*it).c_str();
  }
  // the subfile string end with double null-terminating
  subfilestr.push_back(0);
  subfilestr.push_back(0);

  char subhash[300];
  int hashsize;
  
  hash_file(HASH_MOD_FILE_STR, HASH_ALGO_MD5,
            subfilestr.c_str(), subhash, &hashsize);
  if (hashsize == 0)
    return;

  std::string subhashstr(subhash);
  std::wstring szSubHash = Strings::StringToWString(subhashstr);
  if (szSubHash.empty())
    return;

  refptr<pool> pool = pool::create_instance();
  refptr<task> task = task::create_instance();

  for (int i = 1; i <= 7; i++)
  {
    int chk = 0;
    refptr<request> req1 = request::create_instance();
    WetherNeedUploadSub(pool, task, req1, m_videofile,
                        szFileHash, szSubHash, m_delayms, i, m_oemtitle);
    while (pool->is_running_or_queued(task))
    {
      if (::WaitForSingleObject(m_stopevent, 1000) == WAIT_OBJECT_0)
        return;
    }

    if (req1->get_response_errcode() == 0)
      chk = 1;
    else
      chk = 0;

    if (chk > 0)
    {
      refptr<request> req2 = request::create_instance();
      UploadSubFileByVideoAndHash(pool, task, req2, 
                                  m_videofile,szFileHash, szSubHash,
                                  &szaSubFiles, m_delayms, i, m_oemtitle);
      while (pool->is_running_or_queued(task))
      {
        if (::WaitForSingleObject(m_stopevent, 1000) == WAIT_OBJECT_0)
          return;
      }

      if (req2->get_response_errcode() == 0)
      {
        //m_handlemsgs->push_back(ResStr(IDS_LOG_MSG_SVPSUB_UPLOAD_FINISHED));
      }

     if(0 == chk)
        break;
    }
    //Fail
    if (::WaitForSingleObject(m_stopevent, 2000) == WAIT_OBJECT_0)
      return;
  }
}

void SubTransController::SetMsgs(std::list<std::wstring>* msgs)
{
  m_handlemsgs = msgs;
}

void SubTransController::SetSubfile(std::wstring subfile)
{
  m_subfile = subfile;
}

void SubTransController::SetDelayMs(int ms)
{
  m_delayms = ms;
}