
// mfchostDoc.h : CmfchostDoc 类的接口
//


#pragma once
#include <afxdhtml.h>
#include <threadhelper.h>

#define ID_MOVIESHARE_RESPONSE 32932

class MovieComment : public CDHtmlDialog
{
  DECLARE_DYNAMIC(MovieComment)

  virtual HRESULT STDMETHODCALLTYPE ShowContextMenu(DWORD /*dwID*/, POINT *ppt, IUnknown* /*pcmdtReserved*/, IDispatch* /*pdispReserved*/);
  virtual void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl);

public:
  MovieComment();
  ~MovieComment();

  BOOL OnEventNewLink(IDispatch **ppDisp, VARIANT_BOOL *Cancel,
    DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl);
  HRESULT OpenNewLink(IHTMLElement *pElement);
  HRESULT OnEventClose(IHTMLElement *pElement);

  HRESULT OnEventCapture(IHTMLElement* pElement);

  void ClearFrame();
  void CalcWndPos();
  void HideFrame();
  void ShowFrame();
  void OpenNewLink(LPCTSTR url);

  int m_initialize;
protected:
  virtual BOOL OnInitDialog();
  virtual BOOL IsExternalDispatchSafe();

  DECLARE_MESSAGE_MAP()
  DECLARE_DHTML_EVENT_MAP()
  DECLARE_EVENTSINK_MAP()
  DECLARE_DISPATCH_MAP()

};


