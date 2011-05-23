
// mfchostDoc.cpp : CmfchostDoc 类的实现
//

#include "stdafx.h"
#include <exdispid.h>
#include "OAuthDlg.h"
#include "logging.h"
#include "../../resource.h"

IMPLEMENT_DYNAMIC(OAuthDlg, CDHtmlDialog)

BEGIN_MESSAGE_MAP(OAuthDlg, CDHtmlDialog)
  ON_WM_SIZE()
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(OAuthDlg)

END_DHTML_EVENT_MAP()

BEGIN_DISPATCH_MAP(OAuthDlg, CDHtmlDialog)

END_DISPATCH_MAP()

OAuthDlg::OAuthDlg()
{
  SupportContextMenu(FALSE);
  HostFlags(DOCHOSTUIFLAG_THEME | DOCHOSTUIFLAG_SCROLL_NO | DOCHOSTUIFLAG_NO3DBORDER
    | DOCHOSTUIFLAG_DISABLE_HELP_MENU | DOCHOSTUIFLAG_DIALOG | DOCHOSTUIFLAG_ENABLE_ACTIVEX_INACTIVATE_MODE
    | DOCHOSTUIFLAG_DISABLE_SCRIPT_INACTIVE | DOCHOSTUIFLAG_OVERRIDEBEHAVIORFACTORY
    );
}

OAuthDlg::~OAuthDlg()
{
}

BOOL OAuthDlg::OnInitDialog()
{
  DhtmlDlgBase::OnInitDialog();

  CString strResourceURL;
  LPTSTR lpszModule = new TCHAR[_MAX_PATH];

  if (lpszModule && GetModuleFileName(NULL, lpszModule, _MAX_PATH))
  {
    // load resource html regardless by language
    strResourceURL.Format(_T("res://%s/%d"), lpszModule, IDR_HTML_BUSY);
    Navigate(strResourceURL, 0, 0, 0);
  }
  else
    Navigate(L"about:blank");


  return TRUE;
}

void OAuthDlg::CalcOauthPos()
{
  if (!IsWindowVisible())
    return;

  RECT rc;
  GetParent()->GetWindowRect(&rc);

  rc.top += (rc.bottom-rc.top-400)/2-10;
  rc.left += (rc.right-rc.left-500)/2;
  rc.right = 500;
  rc.bottom = 400;
  SetFramePos(rc);
}

void OAuthDlg::OnSize(UINT nType, int cx, int cy)
{
  __super::OnSize(nType, cx, cy);

  CRect rc;
  WINDOWPLACEMENT wp = {sizeof(WINDOWPLACEMENT)};
  GetWindowPlacement(&wp);
  GetWindowRect(&rc);
  rc-=rc.TopLeft();

  // destroy old region
  if((HRGN)m_rgn)
    m_rgn.DeleteObject();

  // create rounded rect region based on new window size
  if (wp.showCmd != SW_MAXIMIZE )
    m_rgn.CreateRoundRectRgn(0, 0, rc.Width(), rc.Height(), 7, 7);
  else
    m_rgn.CreateRectRgn( 0,0, rc.Width(), rc.Height() );

  SetWindowRgn(m_rgn,TRUE);
}

void OAuthDlg::SetUrl(std::wstring url)
{
  if (url.empty())
    return;

  std::wstring agent = L"User-Agent: Mozilla/5.0 (Linux; U; Android 0.5; en-us) AppleWebKit/522+ (KHTML, like Gecko) Safari/419.3";

  Navigate(url.c_str(), 0, 0, agent.c_str());
}