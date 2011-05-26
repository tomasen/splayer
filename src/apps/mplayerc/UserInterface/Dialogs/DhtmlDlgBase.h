
#pragma once
#include <afxdhtml.h>

class DhtmlDlgBase : public CDHtmlDialog
{
  DECLARE_DYNAMIC(DhtmlDlgBase)

public:
  DhtmlDlgBase();
  virtual ~DhtmlDlgBase();

  void CreateFrame(DWORD style, DWORD styleEx);
  void FreeFrame();

  void ShowFrame();
  void HideFrame();

  void SupportJSCallBack();
  void SupportContextMenu(BOOL flag);

  void SetFramePos(RECT& rc);
  void HostFlags(DWORD flags);
  
  void SetUserAgent(std::string agent);

protected:
  virtual HRESULT STDMETHODCALLTYPE ShowContextMenu(DWORD /*dwID*/, POINT *ppt,
    IUnknown* /*pcmdtReserved*/, IDispatch* /*pdispReserved*/);
  virtual BOOL OnInitDialog();
  virtual BOOL IsExternalDispatchSafe();

private:
  DWORD m_hostflags;
  BOOL m_allowContextMenu;

  DLGTEMPLATE* _dialogTemplate;
};