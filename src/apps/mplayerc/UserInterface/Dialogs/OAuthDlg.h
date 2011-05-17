#pragma once

#include "DhtmlDlgBase.h"

class OAuthDlg : public DhtmlDlgBase
{
  DECLARE_DYNAMIC(OAuthDlg)

public:
  OAuthDlg();
  virtual ~OAuthDlg();

protected:
  virtual BOOL OnInitDialog();

  DECLARE_MESSAGE_MAP()
  DECLARE_DHTML_EVENT_MAP()
  DECLARE_DISPATCH_MAP()
};


