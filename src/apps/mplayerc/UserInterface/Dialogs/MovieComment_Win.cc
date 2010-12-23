
// mfchostDoc.cpp : CmfchostDoc 类的实现
//

#include "stdafx.h"
#include "MovieComment_Win.h"
#include <exdispid.h>

IMPLEMENT_DYNAMIC(MovieComment, CDHtmlDialog)

BEGIN_MESSAGE_MAP(MovieComment, CDHtmlDialog)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(MovieComment)
  //DHTML_EVENT_ONCLICK(L"win_close_btn", OnEventClose)
  //DHTML_EVENT_ONCLICK(L"open_newwnd_btn", OnOpenNewWindow)
END_DHTML_EVENT_MAP()

BEGIN_EVENTSINK_MAP(MovieComment, CDHtmlDialog)
  //ON_EVENT(MovieComment, AFX_IDC_BROWSER, DISPID_NEWWINDOW2, OnEventNewWindow, VTS_DISPATCH VTS_PBOOL)
  //ON_EVENT(MovieComment, AFX_IDC_BROWSER, DISPID_BEFORENAVIGATE2, OnEventBeforeNavigate2, VTS_DISPATCH VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PBOOL)
END_EVENTSINK_MAP()

BEGIN_DISPATCH_MAP(MovieComment, CDHtmlDialog)
  DISP_FUNCTION(MovieComment, "close_sharewnd", HideFrame, VT_EMPTY, VTS_NONE)
  DISP_FUNCTION(MovieComment, "open_sharewnd", ShowFrame, VT_EMPTY, VTS_NONE)
  DISP_FUNCTION(MovieComment, "open_newlink", OpenNewLink, VT_EMPTY, VTS_BSTR)
END_DISPATCH_MAP()

MovieComment::MovieComment():
  m_showframe(FALSE)
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
  
  RECT rc;
  HRGN rgn;

  CalcWndPos();

  GetClientRect(&rc);
  rgn = ::CreateRoundRectRgn(0, 0, rc.right-rc.left, rc.bottom-rc.top, 5, 5);
  SetWindowRgn(rgn, TRUE);

  SetHostFlags(DOCHOSTUIFLAG_THEME | DOCHOSTUIFLAG_SCROLL_NO | DOCHOSTUIFLAG_NO3DBORDER
         | DOCHOSTUIFLAG_DISABLE_HELP_MENU | DOCHOSTUIFLAG_DIALOG | DOCHOSTUIFLAG_DISABLE_SCRIPT_INACTIVE
         | DOCHOSTUIFLAG_OVERRIDEBEHAVIORFACTORY);

  EnableAutomation();
  SetExternalDispatch(GetIDispatch(TRUE));

  return TRUE;
}

HRESULT MovieComment::OnOpenNewWindow(IHTMLElement *pElement)
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
  if (str.find(L"http://") != std::string::npos)
    ShellExecute(NULL, L"open", str.c_str(), L"", L"", SW_SHOW);
}

void MovieComment::CalcWndPos()
{
  RECT rc;
  GetParent()->GetWindowRect(&rc);
  SetWindowPos(NULL, rc.left+20, rc.bottom-260, 373, 161, SWP_NOACTIVATE|SWP_HIDEWINDOW);
}

void MovieComment::HideFrame()
{
  ShowWindow(SW_HIDE);
  m_showframe = FALSE;
}

void MovieComment::ShowFrame()
{
  ShowWindow(SW_SHOW);
  m_showframe = TRUE;
}

BOOL MovieComment::IsShow()
{
  return m_showframe;
}

BOOL MovieComment::OnEventNewWindow(IDispatch **ppDisp, VARIANT_BOOL *Cancel)
{
  *Cancel = VARIANT_TRUE;
  return TRUE;
}

BOOL MovieComment::OnEventBeforeNavigate2(LPDISPATCH pDisp, VARIANT FAR* URL, VARIANT FAR* Flags, 
                                          VARIANT FAR* TargetFrameName, VARIANT FAR* PostData,
                                          VARIANT FAR* Headers, BOOL FAR* Cancel)
{
  //*Cancel = TRUE;
  return TRUE;
}