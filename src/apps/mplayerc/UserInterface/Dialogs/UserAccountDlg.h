#pragma once

#include "DhtmlDlgBase.h"
#include "OAuthDlg.h"

class UserAccountDlg : public DhtmlDlgBase
{
  DECLARE_DYNAMIC(UserAccountDlg)

public:
  UserAccountDlg();
  virtual ~UserAccountDlg();

  void SetUrl(std::wstring url);
  void HideFrame();

  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  CircleBtn m_btnclose;

  void OnDocumentComplete(IDispatch **ppDisp, VARIANT FAR*URL);
  void OnBeforeNavigate2(LPDISPATCH pDisp, VARIANT FAR* URL, VARIANT FAR* Flags,
    VARIANT FAR* TargetFrameName, VARIANT FAR* PostData, VARIANT FAR* Headers, BOOL FAR* Cancel);

protected:
  virtual BOOL OnInitDialog();
  void OnShowWindow(BOOL bShow, UINT nStatus);
  STDMETHOD(TranslateAccelerator)(LPMSG lpMsg, const GUID* pguidCmdGroup, DWORD nCmdID);

  DECLARE_MESSAGE_MAP()
  DECLARE_DHTML_EVENT_MAP()
  DECLARE_EVENTSINK_MAP()
  DECLARE_DISPATCH_MAP()

private:
  CRgn m_rgn;
};
