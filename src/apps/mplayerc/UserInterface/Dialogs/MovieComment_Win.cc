
// mfchostDoc.cpp : CmfchostDoc 类的实现
//

#include "stdafx.h"
#include "MovieComment_Win.h"
#include <exdispid.h>
#include "logging.h"

IMPLEMENT_DYNAMIC(MovieComment, CDHtmlDialog)

BEGIN_MESSAGE_MAP(MovieComment, CDHtmlDialog)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(MovieComment)
  DHTML_EVENT_ONCLICK(L"close_wnd", OnEventClose)
  //DHTML_EVENT_ONCLICK(L"open_newlink", OpenNewLink)
END_DHTML_EVENT_MAP()

BEGIN_EVENTSINK_MAP(MovieComment, CDHtmlDialog)
  ON_EVENT(MovieComment, AFX_IDC_BROWSER, DISPID_NEWWINDOW3, OnEventNewLink, VTS_DISPATCH VTS_PBOOL VTS_UI4 VTS_BSTR VTS_BSTR)
END_EVENTSINK_MAP()

BEGIN_DISPATCH_MAP(MovieComment, CDHtmlDialog)
//   DISP_FUNCTION(MovieComment, "close_sharewnd", HideFrame, VT_EMPTY, VTS_NONE)
//   DISP_FUNCTION(MovieComment, "open_sharewnd", ShowFrame, VT_EMPTY, VTS_NONE)
//   DISP_FUNCTION(MovieComment, "open_newlink", OpenNewLink, VT_EMPTY, VTS_BSTR)
END_DISPATCH_MAP()

MovieComment::MovieComment()
: m_initialize(0)
{

}

MovieComment::~MovieComment()
{
  DestroyWindow();
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
//   RECT rc;
//   HRGN rgn;
//   GetClientRect(&rc);
//   rgn = ::CreateRoundRectRgn(0, 0, rc.right-rc.left, rc.bottom-rc.top, 5, 5);
//   SetWindowRgn(rgn, TRUE);

  SetHostFlags(DOCHOSTUIFLAG_THEME | DOCHOSTUIFLAG_SCROLL_NO | DOCHOSTUIFLAG_NO3DBORDER
         | DOCHOSTUIFLAG_DISABLE_HELP_MENU | DOCHOSTUIFLAG_DIALOG | DOCHOSTUIFLAG_DISABLE_SCRIPT_INACTIVE
         | DOCHOSTUIFLAG_OVERRIDEBEHAVIORFACTORY);

  // EnableAutomation();
  // SetExternalDispatch(GetIDispatch(TRUE));
  // suppress script error
  m_pBrowserApp->put_Silent(VARIANT_TRUE);
  m_initialize = 1;
  return TRUE;
}

void MovieComment::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
  std::wstring url(szUrl);
  if (url.find(L"http:") != std::string::npos && url.length() > 8)
    ::PostMessage(GetParent()->m_hWnd, WM_COMMAND, ID_MOVIESHARE_RESPONSE, NULL);

}

BOOL MovieComment::OnEventNewLink(IDispatch **ppDisp, VARIANT_BOOL *Cancel,
                 DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl)
{
  m_newlink._Stop();
  m_newlink.SetOpenUrl(bstrUrl);
  m_newlink._Start();

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

void ThreadNewLink::SetOpenUrl(std::wstring url)
{
  if (url.empty() || url.find(L"http://") == std::string::npos)
    return;

  m_url = url;
}

void ThreadNewLink::_Thread()
{
  ShellExecute(NULL, L"open", m_url.c_str(), L"", L"", SW_SHOW);
}