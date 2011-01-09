#include "stdafx.h"
#include "../Model/SubTransFormat.h"
#include "SubTransController.h"
#include "../Resource.h"
#include "HashController.h"
#include <Strings.h>
#include <sys/stat.h>
#include "PlayerPreference.h"
#include "SPlayerDefs.h"
#include "../revision.h"
#include <logging.h>
// #define TRANSDEBUG

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

void SubTransController::WetherNeedUploadSub(refptr<pool> pool, refptr<task> task, refptr<request> req,
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
  MapToPostData(data, postform);

  int rret = -1;
  SinetConfig(cfg, sid);

  std::wstring url = GetServerUrl('upsb', sid);
  req->set_request_url(url.c_str());

  req->set_request_method(REQ_POST);
  req->set_postdata(data);

  task->use_config(cfg);
  task->append_request(req);
  pool->execute(task);
}

void SubTransController::UploadSubFileByVideoAndHash(refptr<pool> pool,refptr<task> task,
                                refptr<request> req,
                                std::wstring fnVideoFilePath,
                                std::wstring szFileHash,
                                std::wstring szSubHash,
                                std::vector<std::wstring>* fnSubPaths,
                                int iDelayMS, int sid, std::wstring oem)
{
  Logging(L"UploadSubFileByVideoAndHash %s %s", fnVideoFilePath.c_str(), szSubHash.c_str());

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
  MapToPostData(data, postform);

  size_t iTotalFiles = fnSubPaths->size();
  for (size_t i = 0; i < iTotalFiles; i++)
  {
    wchar_t szFname[22];
    // Fill in the file upload field 
    std::wstring szgzFile = SubTransFormat::GetTempFileName(); //SubTransFormat::GetSameTmpName(fnSubPaths->at(i));
    int gzerr = SubTransFormat::PackGZfile(fnSubPaths->at(i), szgzFile);

    wsprintf(szFname, L"subfile[%d]", i);
    refptr<postdataelem> elem = postdataelem::create_instance();
    elem->set_name(szFname);
    elem->setto_file(szgzFile.c_str());
    Logging(L"gzfile %d %s", gzerr, szgzFile.c_str());
    data->add_elem(elem);
  }

  SinetConfig(cfg, sid);

  std::wstring url = GetServerUrl('upsb', sid);
  req->set_request_url(url.c_str());
  req->set_request_method(REQ_POST);
  req->set_postdata(data);

  task->use_config(cfg);
  task->append_request(req);
  pool->execute(task);


  while (pool->is_running_or_queued(task))
  {
    if (_Exit_state(500))
      return;
  }

  Logging(L"UploadSubFileByVideoAndHash %s %d", url.c_str(), req->get_response_errcode());

}

SubTransController::SubTransController(void):
  m_operation(Unknown)
{
}

SubTransController::~SubTransController(void)
{
  _Stop();
}

void SubTransController::SetFrame(HWND hwnd)
{
  m_frame = hwnd;
}


void SubTransController::SetSubperf(std::wstring str)
{
  m_subperf = str;
}

void SubTransController::Start(const wchar_t* video_filename, 
                               SubTransOperation operation, 
                               std::wstring lanuage, int subnum)
{
  _Stop();
  // record parameters
  m_language = lanuage;
  m_operation = operation;
  m_videofile.assign(video_filename);
  m_subnum = subnum;
  _Start();
}

void SubTransController::_Thread()
{
  Logging( L"SubTransController::_thread enter %x", m_operation);
  switch (m_operation)
  {
  case DownloadSubtitle:
    _thread_download();
    break;
  case UploadSubtitle:
    _thread_upload();
    break;
  }
  Logging( L"SubTransController::_thread exit %x", m_operation);
}

void SubTransController::_thread_download()
{
  std::vector<std::wstring> subtitles;
  std::wstring szLang = m_language;

  subtitles.push_back(m_videofile);
  subtitles.push_back(Strings::Format(L"%d", m_subnum));

  std::vector<std::wstring> szaSubDescs, tmpfiles;
  std::wstring szFileHash = HashController::GetInstance()->GetSPHash(m_videofile.c_str());
  Logging(L"Downloading sub for %s" ,szFileHash.c_str());

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
    MapToPostData(data, postform);

    req->set_postdata(data);
    req->set_request_url(url.c_str());
    req->set_request_method(REQ_POST);
    req->set_request_outmode(REQ_OUTFILE);
    req->set_outfile(tmpoutfile.c_str());

    SinetConfig(cfg, i);
    task->use_config(cfg);
    task->append_request(req);
    pool->execute(task);

    while (pool->is_running_or_queued(task))
    {
      if (_Exit_state(500))
        return;
    }
   
#ifdef TRANSDEBUG
    si_stringmap rps_headers = req->get_response_header();
    for (si_stringmap::iterator it = rps_headers.begin(); it != rps_headers.end(); 
         it++)
      Logging(L"header: %s %s", it->first.c_str(), it->second.c_str());
#endif

    if (req->get_response_errcode() == 0)
    {
      int ret = SubTransFormat::ExtractDataFromAiSubRecvBuffer_STL(m_handlemsgs, m_videofile,
        tmpoutfile, szaSubDescs,tmpfiles);
      if (ret == -404 && m_handlemsgs)
        m_handlemsgs->push_back(ResStr_STL(IDS_LOG_MSG_SVPSUB_NONE_MATCH_SUB));
      break;
    }

    if (_Exit_state(5000))
      return;
  }

  //load sub file to sublist
  std::wstring szSubFilePath;
  int iSubTotal = 0;
  int already_exist = 0;
  for (size_t i = 0; i < tmpfiles.size(); i++)
  {
    szSubFilePath = SubTransFormat::GetSubFileByTempid_STL(i,
      m_videofile, szaSubDescs, tmpfiles);

    if (!szSubFilePath.empty() && szSubFilePath != L"EXIST" )
    {
      subtitles.push_back(szSubFilePath);
      iSubTotal++;
    }

    if (szSubFilePath == L"EXIST" )
      already_exist++;
  }

  if (iSubTotal > 1)
  {
    if (!m_subperf.empty())
    {
      for (std::vector<std::wstring>::iterator iter = subtitles.begin()+2;
        iter != subtitles.end(); iter++)
        if (SubTransFormat::IsSpecFanSub((*iter), m_subperf))
        {
          std::wstring szFirst = subtitles.at(2);
          subtitles[2] = szSubFilePath;
          *iter = szFirst;
          break;
        }
    }
  }
  else if (iSubTotal <= 0 && already_exist)
    subtitles.push_back(L"EXIST");


  PlayerPreference::GetInstance()->SetStrArray(STRARRAY_QUERYSUBTITLE, subtitles);
  ::PostMessage(m_frame, WM_COMMAND, ID_COMPLETE_QUERY_SUBTITLE, NULL);
  
}

void SubTransController::_thread_upload()
{
  std::wstring szFileHash = HashController::GetInstance()->GetSPHash(m_videofile.c_str());

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

  std::wstring szSubHash = HashController::GetInstance()->GetMD5Hash(subfilestr.c_str());
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
      if (_Exit_state(500))
        return;
    }
#ifdef TRANSDEBUG
      si_stringmap rps_headers = req1->get_response_header();
      for (si_stringmap::iterator itsm = rps_headers.begin(); itsm != rps_headers.end(); 
           itsm++)
        Logging(L"header: %s %s", itsm->first.c_str(), itsm->second.c_str());
#endif

    // 200  需要上传该字幕	
    // 404  该字幕服务器上已经存在。不需要再上传	
    if (req1->get_response_errcode() == 404)
      return;

    if (req1->get_response_errcode() == 0)
    {
      refptr<request> req2 = request::create_instance();
      UploadSubFileByVideoAndHash(pool, task, req2, 
                                  m_videofile,szFileHash, szSubHash,
                                  &szaSubFiles, m_delayms, i, m_oemtitle);
      while (pool->is_running_or_queued(task))
      {
        if (_Exit_state(500))
          return;
      }
#ifdef TRANSDEBUG
        si_stringmap rps_headers = req2->get_response_header();
        for (si_stringmap::iterator itsm = rps_headers.begin(); itsm != rps_headers.end(); 
             itsm++)
          Logging(L"header: %s %s", itsm->first.c_str(), itsm->second.c_str());
#endif

      if (req2->get_response_errcode() == 0)
      {
        //m_handlemsgs->push_back(ResStr(IDS_LOG_MSG_SVPSUB_UPLOAD_FINISHED));
        Logging(L"SUB_UPLOAD_FINISHED %s %s %s", m_videofile.c_str(), 
                szFileHash.c_str(), szSubHash.c_str() );
        return;
      }
      else
        Logging(L"SUB_UPLOAD_ERROR %d", req2->get_response_errcode());
    }
    //Fail
    if (_Exit_state(2000))
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