
// mfchostDoc.cpp : CmfchostDoc 类的实现
//

#include "stdafx.h"
#include <exdispid.h>
#include "OAuthDlg.h"
#include "logging.h"

IMPLEMENT_DYNAMIC(OAuthDlg, CDHtmlDialog)

BEGIN_MESSAGE_MAP(OAuthDlg, CDHtmlDialog)
  //ON_WM_SIZE()
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

  std::wstring agent = L"User-Agent: Mozilla/5.0 (Linux; U; Android 0.5; en-us) AppleWebKit/522+ (KHTML, like Gecko) Safari/419.3";
  Navigate(L"http://jay.webpj.com:8888/oauths/geturl/sina", 0, 0, agent.c_str());

  return TRUE;
}