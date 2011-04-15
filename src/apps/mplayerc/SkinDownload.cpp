#include "stdafx.h"
#include "SkinDownload.h"
#include <exdispid.h>
#include <Strings.h>
#include "Controller\NetworkControlerImpl.h"
#include "afxinet.h"
#include "MainFrm.h"
#include "SkinFolderManager.h"

IMPLEMENT_DYNAMIC(SkinDownload, CDHtmlDialog)

BEGIN_MESSAGE_MAP(SkinDownload, CDHtmlDialog)
  ON_WM_CLOSE()
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(SkinDownload, CDHtmlDialog)
    ON_EVENT(SkinDownload, AFX_IDC_BROWSER, DISPID_BEFORENAVIGATE2, OnBeforeNavigate2,
            VTS_DISPATCH VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PBOOL)
END_EVENTSINK_MAP()

SkinDownload::SkinDownload(void)
{
}

SkinDownload::~SkinDownload(void)
{
  DestroyWindow();
}

BOOL SkinDownload::OnInitDialog()
{
  CDHtmlDialog::OnInitDialog();

  SetHostFlags(DOCHOSTUIFLAG_THEME | DOCHOSTUIFLAG_NO3DBORDER
    | DOCHOSTUIFLAG_DISABLE_HELP_MENU | DOCHOSTUIFLAG_DIALOG | DOCHOSTUIFLAG_ENABLE_ACTIVEX_INACTIVATE_MODE
    | DOCHOSTUIFLAG_DISABLE_SCRIPT_INACTIVE | DOCHOSTUIFLAG_OVERRIDEBEHAVIORFACTORY
    );

  return TRUE;
}

HRESULT STDMETHODCALLTYPE SkinDownload::ShowContextMenu(DWORD /*dwID*/, POINT *ppt, IUnknown* /*pcmdtReserved*/, IDispatch* /*pdispReserved*/)
{
  return S_OK;
}

void SkinDownload::OnClose()
{
  HideFrame();
}

void SkinDownload::ShowFrame()
{
  ModifyStyle(WS_DISABLED, 0);
  ShowWindow(SW_SHOW);
}


void SkinDownload::HideFrame()
{
  ModifyStyle(0, WS_DISABLED);
  ShowWindow(SW_HIDE);
}

void SkinDownload::OnBeforeNavigate2(LPDISPATCH pDisp, VARIANT FAR* URL, VARIANT FAR* Flags,
            VARIANT FAR* TargetFrameName, VARIANT FAR* PostData, VARIANT FAR* Headers, BOOL FAR* Cancel)
{
  std::wstring url = V_BSTR(URL);

  if (url.find(L".zip") != std::wstring::npos)
  {
    DownloadSkin(url);
    *Cancel = TRUE;
  }
}

void SkinDownload::DownloadSkin(std::wstring url)
{
  m_url = url;
  int pos = m_url.find_last_of(L"//");
  m_filename = m_url.substr(pos + 1);
  
  _Start();
}

void SkinDownload::_Thread()
{
/*
  refptr<request> req = request::create_instance();

  req->set_request_url(m_url.c_str());
  req->set_request_method(REQ_GET);

  if (req->get_response_errcode() != 0)
    return;

  si_buffer buffer = req->get_response_buffer();
  buffer.push_back(0);

  std::string results = (char*)&buffer[0];
  std::wstring retdata = Strings::Utf8StringToWString(results);*/

  HINTERNET hi=InternetOpen(L"http_down_dll",INTERNET_OPEN_TYPE_PRECONFIG,NULL,INTERNET_INVALID_PORT_NUMBER,0);
  if(hi==NULL)
  {
    return;
  }
  /*HINTERNET hUrl=InternetOpenUrl(hi,retdata.c_str(),NULL,0,INTERNET_FLAG_RELOAD,0);*/
  HINTERNET hUrl=InternetOpenUrl(hi,m_url.c_str(),NULL,0,INTERNET_FLAG_RELOAD,0);

  if(hUrl==NULL)
  {
    InternetCloseHandle(hi);
    return;
  }
  DWORD optbuf[256]={0};
  DWORD optlen=256;
  if(HttpQueryInfo(hUrl,HTTP_QUERY_CONTENT_LENGTH,optbuf,&optlen,NULL)==FALSE)
  {
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hi);
    return;
  }
  char tmpbuff[256]={0};
  sprintf(tmpbuff,"%s",optbuf);
  long flen=atol(tmpbuff);
  if(flen<=0)
  {
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hi);
    return;
  }

  std::wstring savepath = GetModuleFolder();
  savepath += L"skins\\";
  BOOL bl = CreateDirectory(savepath.c_str(), 0);
  if (bl == ERROR_PATH_NOT_FOUND)
    return;
  savepath += m_filename;
  HANDLE hFile=CreateFile(savepath.c_str(),GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
  if(hFile==NULL)
  {
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hi);
    return;
  }

  BYTE bufferstr[1024]={0};
  DWORD len=0,wlen=0;
  DWORD wsum=0;
  long sendlen=flen;

  while(InternetReadFile(hUrl,(LPVOID)bufferstr,1024,&len)&&len!=0)
    WriteFile(hFile,bufferstr,len,&wlen,NULL);
     
  CloseHandle(hFile);
  InternetCloseHandle(hUrl);
  InternetCloseHandle(hi);
  
  TCHAR szModuleFullPath[MAX_PATH];
  ::GetModuleFileName(0, szModuleFullPath, MAX_PATH);
  TCHAR szDrive[10] = {0};
  TCHAR szDir[MAX_PATH] = {0};
  ::_wsplitpath(szModuleFullPath, szDrive, szDir, 0, 0);
  CString skinpath;
  skinpath += szDrive;
  skinpath += szDir;
  skinpath += L"skins\\";
  SkinFolderManager skmn;
  skmn.ClearMap();
  skmn.SetSkinPath(skinpath);
  skmn.SeachFile(skinpath.GetBuffer(MAX_PATH));
  skinpath.ReleaseBuffer();
  ::SendMessage(GetParent()->m_hWnd, WM_USER + 34, 0, 0);

  HideFrame();

}

std::wstring SkinDownload::GetModuleFolder()
{
  TCHAR szModuleFullPath[MAX_PATH] = {0};
  ::GetModuleFileName(0, szModuleFullPath, MAX_PATH);

  TCHAR szDrive[10] = {0};
  TCHAR szDir[MAX_PATH] = {0};

  ::_wsplitpath(szModuleFullPath, szDrive, szDir, 0, 0);

  std::wstring sResult;
  sResult += szDrive;
  sResult += szDir;

  return sResult;
}