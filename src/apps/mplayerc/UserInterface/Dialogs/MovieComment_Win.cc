
// mfchostDoc.cpp : CmfchostDoc 类的实现
//

#include "stdafx.h"
#include "MovieComment_Win.h"
#include <exdispid.h>
#include "logging.h"
#include "../../resource.h"
#include "../../MainFrm.h"
#include "base64.h"
#include "strings.h"
#include <fstream>
#include "../../Controller/ShareController.h"
#include "..\..\svplib\SVPToolBox.h"

IMPLEMENT_DYNAMIC(MovieComment, CDHtmlDialog)

BEGIN_MESSAGE_MAP(MovieComment, CDHtmlDialog)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(MovieComment)
  DHTML_EVENT_ONCLICK(L"close_wnd", OnEventClose)
  DHTML_EVENT_ONCLICK(L"open_newlink", OpenNewLink)
  //DHTML_EVENT_ONCLICK(L"cut-image-btn", OnEventCapture)
END_DHTML_EVENT_MAP()

BEGIN_EVENTSINK_MAP(MovieComment, CDHtmlDialog)
  //ON_EVENT(MovieComment, AFX_IDC_BROWSER, DISPID_NEWWINDOW3, OnEventNewLink, VTS_DISPATCH VTS_PBOOL VTS_UI4 VTS_BSTR VTS_BSTR)
END_EVENTSINK_MAP()

BEGIN_DISPATCH_MAP(MovieComment, CDHtmlDialog)
//   DISP_FUNCTION(MovieComment, "close_sharewnd", HideFrame, VT_EMPTY, VTS_NONE)
//   DISP_FUNCTION(MovieComment, "open_sharewnd", ShowFrame, VT_EMPTY, VTS_NONE)
//   DISP_FUNCTION(MovieComment, "open_newlink", OpenNewLink, VT_EMPTY, VTS_BSTR)
  DISP_FUNCTION(MovieComment, "CallSPlayer", CallSPlayer, VT_BSTR, VTS_BSTR VTS_BSTR)
END_DISPATCH_MAP()

MovieComment::MovieComment()
: m_initialize(0)
{

}

MovieComment::~MovieComment()
{
  DestroyWindow();
}

BSTR MovieComment::CallSPlayer(LPCTSTR p, LPCTSTR param)
{
  CString ret = L"0";
  std::wstring cmd(p);

  if (cmd.empty())
    ret = L"-1";
  else if (cmd == L"rev")
    ret = L"1";
  else if (cmd == L"reload")
  {
    UserShareController::GetInstance()->_Stop();
    UserShareController::GetInstance()->_Start();
  }
  else if (cmd == L"open")
    ShowFrame();
  else if (cmd == L"close")
    HideFrame();
  else if (cmd == L"openlink")
    OpenNewLink(param);
  else if (cmd == L"snapshoot") 
    OnEventCapture(NULL);
  else if (cmd == L"curtime")
    ret = GetMovieTime(1).c_str();
  else if (cmd == L"totaltime")
    ret = GetMovieTime(0).c_str();
  else
    ret = L"-1";

  return ret.AllocSysString();
}

std::wstring MovieComment::GetMovieTime(int i)
{
  __int64 rtDur = 0;
  CMainFrame* mf = (CMainFrame*)AfxGetMainWnd();

  if (mf && mf->pMC)
    i==0 ? mf->pMS->GetDuration(&rtDur) :
           mf->pMS->GetCurrentPosition(&rtDur);

  wchar_t buf[64];
  _i64tow(rtDur, buf, 10);
  return std::wstring(buf);
}

HRESULT STDMETHODCALLTYPE MovieComment::ShowContextMenu(DWORD /*dwID*/, POINT *ppt, IUnknown* /*pcmdtReserved*/, IDispatch* /*pdispReserved*/)
{
  return S_OK;
}

BOOL MovieComment::IsExternalDispatchSafe()
{
  return TRUE;
}

BOOL MovieComment::OnInitDialog()
{
  CDHtmlDialog::OnInitDialog();

  CalcWndPos();
  HideFrame();

  SetHostFlags(DOCHOSTUIFLAG_THEME | DOCHOSTUIFLAG_SCROLL_NO | DOCHOSTUIFLAG_NO3DBORDER
         | DOCHOSTUIFLAG_DISABLE_HELP_MENU | DOCHOSTUIFLAG_DIALOG | DOCHOSTUIFLAG_ENABLE_ACTIVEX_INACTIVATE_MODE
         | DOCHOSTUIFLAG_DISABLE_SCRIPT_INACTIVE | DOCHOSTUIFLAG_OVERRIDEBEHAVIORFACTORY
         );

  EnableAutomation();
  SetExternalDispatch(GetIDispatch(TRUE));
  // suppress script error
  m_pBrowserApp->put_Silent(VARIANT_TRUE);
  m_initialize = 1;
  ClearFrame();

  return TRUE;
}
void MovieComment::ClearFrame()
{
  if (!m_hWnd || !m_initialize)
    return;

  CString strResourceURL;
  LPTSTR lpszModule = new TCHAR[_MAX_PATH];

  if (GetModuleFileName(NULL, lpszModule, _MAX_PATH))
  {
    // load resource html regardless by language
    strResourceURL.Format(_T("res://%s/%d"), lpszModule, IDR_HTML_BUSY);
    Navigate(strResourceURL, 0, 0, 0);
  }
  else
    Navigate(L"about:blank");
}
void MovieComment::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{

}

HRESULT MovieComment::OnEventCapture(IHTMLElement* pElement)
{
  try
  {
    IHTMLStyle* imgstyle = NULL;

    IHTMLElement* imgele = NULL;
    IHTMLElement* dataele = NULL;
    IHTMLElement* extele = NULL;
    IHTMLElement* test = NULL;

    GetElement(L"cut-image-data", &dataele);
    GetElement(L"cut-image-ext", &extele);
    GetElement(L"cut-image-div", &imgele);
    if (!dataele || !extele || !imgele)
      return S_FALSE;

    imgele->get_style(&imgstyle);
    if (!imgstyle)
      return S_FALSE;

    CSVPToolBox svptool;
    std::wstring imgpath;
    CString tpath;

    svptool.GetAppDataPath(imgpath);
    imgpath += L"\\SVPSub";
    _wmkdir(imgpath.c_str());

    tpath.Format(L"%s\\tmp_smst_%d.jpg", imgpath.c_str(), time(NULL));
    imgpath = tpath;

    CMainFrame* cmf = (CMainFrame*)AfxGetMainWnd();
    cmf->SnapShootImage(imgpath, (cmf->GetVideoSize().cx > 960)?TRUE:FALSE);

    CComVariant imgsrc = imgpath.c_str();
    if (S_FALSE == imgele->setAttribute(L"src", imgsrc))
      return S_FALSE;
    imgstyle->put_display(L"");

    std::ifstream fs;
    fs.open(imgpath.c_str(), std::ios::binary|std::ios::in);
    if (fs.bad())
      return S_FALSE;

    fs.seekg(0, std::ios::end);
    UINT filesize = fs.tellg();
    fs.seekg(0, std::ios_base::beg);

    unsigned char* buf = new unsigned char[filesize];

    fs.read((char*)buf, filesize);
    fs.close();

    std::string bufstr = base64_encode(buf, filesize);
    delete [] buf;

    CComVariant imgdata = Strings::Utf8StringToWString(bufstr).c_str();
    dataele->setAttribute(L"value", imgdata);
    CComVariant imgext = AfxGetAppSettings().SnapShotExt;
    extele->setAttribute(L"value", imgext);
  }
  catch (...)
  {
    ;
  }

  return S_FALSE;
}

BOOL MovieComment::OnEventNewLink(IDispatch **ppDisp, VARIANT_BOOL *Cancel,
                 DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl)
{
  OpenNewLink(bstrUrl);

  *Cancel = VARIANT_TRUE;

  return S_OK;  
}

HRESULT MovieComment::OpenNewLink(IHTMLElement *pElement)
{
  CComBSTR tagName;
  CComBSTR url;
  CComVariant v;
  pElement->get_tagName(&tagName);
  pElement->getAttribute(L"href", 0, &v);
  v.CopyTo(&url);
  if (tagName == L"A" && url)
    ShellExecute(NULL, L"open", url, L"", L"", SW_SHOW);
  return S_OK;
}

HRESULT MovieComment::OnEventClose(IHTMLElement* /*pElement*/)
{
  HideFrame();
  return S_OK;
}

void MovieComment::OpenNewLink(LPCTSTR url)
{
  std::wstring str(url);
  if (str.find(L"http://") != std::string::npos && str.length() > 8)
    ShellExecute(NULL, L"open", str.c_str(), L"", L"", SW_SHOW);
}

void MovieComment::CalcWndPos()
{
  RECT rc;
  GetParent()->GetWindowRect(&rc);
  SetWindowPos(NULL, rc.left+20, rc.bottom-420, 240, 320, SWP_NOZORDER|SWP_NOACTIVATE);
}

void MovieComment::HideFrame()
{
  ShowWindow(SW_HIDE);
  ModifyStyle(0, WS_DISABLED);
}

void MovieComment::ShowFrame()
{
  ModifyStyle(WS_DISABLED, 0);
  ShowWindow(SW_SHOWNOACTIVATE);
}
