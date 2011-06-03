#include "stdafx.h"
#include "UserAccountDlg.h"
#include <exdispid.h>
#include "..\..\Controller\UserAccountController.h"

IMPLEMENT_DYNAMIC(UserAccountDlg, CDHtmlDialog)

BEGIN_MESSAGE_MAP(UserAccountDlg, CDHtmlDialog)
  ON_WM_SIZE()
  ON_WM_CREATE()
  ON_WM_ERASEBKGND()
  ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(UserAccountDlg)

END_DHTML_EVENT_MAP()

BEGIN_EVENTSINK_MAP(UserAccountDlg, CDHtmlDialog)
  ON_EVENT(UserAccountDlg, AFX_IDC_BROWSER, DISPID_DOCUMENTCOMPLETE,
  OnDocumentComplete, VTS_DISPATCH VTS_PVARIANT)
  ON_EVENT(UserAccountDlg, AFX_IDC_BROWSER, DISPID_BEFORENAVIGATE2, OnBeforeNavigate2,
  VTS_DISPATCH VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PBOOL)
END_EVENTSINK_MAP()

BEGIN_DISPATCH_MAP(UserAccountDlg, CDHtmlDialog)
END_DISPATCH_MAP()

UserAccountDlg::UserAccountDlg()
{
  SupportContextMenu(FALSE);
  HostFlags(DOCHOSTUIFLAG_THEME | DOCHOSTUIFLAG_SCROLL_NO | DOCHOSTUIFLAG_NO3DBORDER
    | DOCHOSTUIFLAG_DISABLE_HELP_MENU | DOCHOSTUIFLAG_DIALOG | DOCHOSTUIFLAG_ENABLE_ACTIVEX_INACTIVATE_MODE
    | DOCHOSTUIFLAG_DISABLE_SCRIPT_INACTIVE | DOCHOSTUIFLAG_OVERRIDEBEHAVIORFACTORY
    );
}

UserAccountDlg::~UserAccountDlg()
{
}

void UserAccountDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
  if (!bShow)
  {
    // get the account info from web
    UserAccountController::GetInstance()->_Stop();
    UserAccountController::GetInstance()->_Start();
  }
}

BOOL UserAccountDlg::OnEraseBkgnd(CDC* pDC)
{
  pDC->SetBkColor(RGB(100,103,108));
  return TRUE;
}

int UserAccountDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  m_btnclose.Create(NULL, L"oauthclose", WS_CHILD|WS_VISIBLE, CRect(450, 5, 486, 41), this, 666);

  return __super::OnCreate(lpCreateStruct);
}

BOOL UserAccountDlg::OnInitDialog()
{
  DhtmlDlgBase::OnInitDialog();

  SetUserAgent("Mozilla/5.0 (Linux; U; Android 0.5; en-us) AppleWebKit/522+ (KHTML, like Gecko) Safari/419.3");
  m_btnclose.ShowWindow(SW_MINIMIZE);

  return TRUE;
}

void UserAccountDlg::OnBeforeNavigate2(LPDISPATCH pDisp, VARIANT FAR* URL, VARIANT FAR* Flags,
                                 VARIANT FAR* TargetFrameName, VARIANT FAR* PostData, VARIANT FAR* Headers, BOOL FAR* Cancel)
{
  if (URL->bstrVal)
  {
    std::wstring url = V_BSTR(URL);
    if (url.find(L"res://") != std::wstring::npos)
      m_btnclose.ShowWindow(SW_MINIMIZE); // SW_HIDE Not working on CWnd created button 
  }
}
void UserAccountDlg::OnDocumentComplete(IDispatch **ppDisp, VARIANT FAR *URL)
{
  if (URL->bstrVal)
  {
    std::wstring url = V_BSTR(URL);
    //Logging(L"OnDocumentComplete %s", url.c_str());
    if (url.find(L"res://") == std::wstring::npos)
      m_btnclose.ShowWindow(SW_SHOWNORMAL);
  }

  return;
}

void UserAccountDlg::HideFrame()
{
  m_btnclose.ShowWindow(SW_HIDE);
  DhtmlDlgBase::HideFrame();
}

void UserAccountDlg::SetUrl(std::wstring url)
{
  if (url.empty())
    return;

  m_btnclose.ShowWindow(SW_HIDE);
  Navigate(url.c_str());

}

STDMETHODIMP UserAccountDlg::TranslateAccelerator(LPMSG lpMsg, const GUID* /*pguidCmdGroup*/, DWORD /*nCmdID*/)
{
  switch (lpMsg->message)
  {
  case WM_KEYDOWN:
    switch (lpMsg->wParam)
    {
    case VK_RETURN:
      return S_OK;
    case VK_ESCAPE:
      HideFrame();
      return S_OK;  // let me handle
    }
    break;
  }

  return S_FALSE;
}