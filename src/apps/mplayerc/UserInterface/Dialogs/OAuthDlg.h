#pragma once

#include "DhtmlDlgBase.h"

class OAuthDlg : public DhtmlDlgBase
{
  DECLARE_DYNAMIC(OAuthDlg)

public:
  OAuthDlg();
  virtual ~OAuthDlg();

  void CalcOauthPos();
  void OnSize(UINT nType, int cx, int cy);
  void SetUrl(std::wstring url);

protected:
  virtual BOOL OnInitDialog();
  STDMETHOD(TranslateAccelerator)(LPMSG lpMsg, const GUID* pguidCmdGroup, DWORD nCmdID);

  DECLARE_MESSAGE_MAP()
  DECLARE_DHTML_EVENT_MAP()
  DECLARE_DISPATCH_MAP()

private:
  CRgn m_rgn;
};


