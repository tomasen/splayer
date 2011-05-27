
// mfchostDoc.cpp : CmfchostDoc 类的实现
//

#include "stdafx.h"
#include "DhtmlDlgBase.h"

IMPLEMENT_DYNAMIC(DhtmlDlgBase, CDHtmlDialog)


DhtmlDlgBase::DhtmlDlgBase():
  m_hostflags(0),
  m_allowContextMenu(FALSE),
  _dialogTemplate(NULL)
{

}

DhtmlDlgBase::~DhtmlDlgBase()
{
  DestroyWindow();
  FreeFrame();
}

void DhtmlDlgBase::SetUserAgent(std::string agent)
{
  if (!agent.empty())
    UrlMkSetSessionOption(URLMON_OPTION_USERAGENT, (LPVOID)agent.c_str(), agent.length(), 0);
}

HRESULT STDMETHODCALLTYPE DhtmlDlgBase::ShowContextMenu(DWORD /*dwID*/, POINT *ppt, IUnknown* /*pcmdtReserved*/, IDispatch* /*pdispReserved*/)
{
  return (m_allowContextMenu) ? S_FALSE : S_OK;
}

BOOL DhtmlDlgBase::IsExternalDispatchSafe()
{
  return TRUE;
}

BOOL DhtmlDlgBase::OnInitDialog()
{
  CDHtmlDialog::OnInitDialog();

  if (m_hostflags)
    SetHostFlags(m_hostflags);

  // suppress script error
  m_pBrowserApp->put_Silent(VARIANT_TRUE);

  return TRUE;
}

void DhtmlDlgBase::HostFlags(DWORD flags)
{
  m_hostflags = flags;
}

void DhtmlDlgBase::SetFramePos(RECT& rc)
{
  SetWindowPos(NULL, rc.left, rc.top, rc.right, rc.bottom, SWP_NOZORDER|SWP_NOACTIVATE);
}

void DhtmlDlgBase::HideFrame()
{
  ShowWindow(SW_HIDE);
  ModifyStyle(0, WS_DISABLED);
}

void DhtmlDlgBase::ShowFrame()
{
  ModifyStyle(WS_DISABLED, 0);
  ShowWindow(SW_SHOWNOACTIVATE);
}

void DhtmlDlgBase::SupportJSCallBack()
{
  EnableAutomation();
  SetExternalDispatch(GetIDispatch(TRUE));
}

void DhtmlDlgBase::SupportContextMenu(BOOL flag)
{
  m_allowContextMenu = flag;
}

void DhtmlDlgBase::CreateFrame(DWORD style, DWORD styleEx)
{
  // extra space are in order for successfully creating dialog
  if (!_dialogTemplate)
    _dialogTemplate = (DLGTEMPLATE*)calloc(1, sizeof(DLGTEMPLATE)+sizeof(DLGITEMTEMPLATE)+10);

  if (_dialogTemplate)
  {
    _dialogTemplate->style = style;
    _dialogTemplate->dwExtendedStyle = styleEx;

    _dialogTemplate->cdit  = 0;
    _dialogTemplate->cx    = 0;
    _dialogTemplate->cy    = 0;

    if (0 == CreateIndirect(_dialogTemplate))
    {
      free(_dialogTemplate);
      _dialogTemplate = 0;
    }
  }
}

void DhtmlDlgBase::FreeFrame()
{
  if (_dialogTemplate)
    free(_dialogTemplate);
}