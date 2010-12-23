
// mfchostDoc.h : CmfchostDoc 类的接口
//


#pragma once

#include <afxdhtml.h>

class MovieComment : public CDHtmlDialog
{
  DECLARE_DYNAMIC(MovieComment)

  virtual HRESULT STDMETHODCALLTYPE ShowContextMenu(DWORD /*dwID*/, POINT *ppt, IUnknown* /*pcmdtReserved*/, IDispatch* /*pdispReserved*/);

public:
  MovieComment();
  ~MovieComment();

  BOOL MovieComment::OnEventBeforeNavigate2(LPDISPATCH pDisp, VARIANT FAR* URL, VARIANT FAR* Flags, 
    VARIANT FAR* TargetFrameName, VARIANT FAR* PostData,
    VARIANT FAR* Headers, BOOL FAR* Cancel);

  BOOL OnEventNewWindow(IDispatch **ppDisp, VARIANT_BOOL *Cancel);
  HRESULT OnOpenNewWindow(IHTMLElement *pElement);
  HRESULT OnEventClose(IHTMLElement *pElement);
  void CalcWndPos();
  void HideFrame();
  void ShowFrame();
  void OpenNewLink(LPCTSTR url);
  BOOL IsShow();

protected:
  virtual BOOL OnInitDialog();
  virtual BOOL IsExternalDispatchSafe();

  DECLARE_MESSAGE_MAP()
  DECLARE_DHTML_EVENT_MAP()
  DECLARE_EVENTSINK_MAP()
  DECLARE_DISPATCH_MAP()
private:
  BOOL m_showframe;
};


