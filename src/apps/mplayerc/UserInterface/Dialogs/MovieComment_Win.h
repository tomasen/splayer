
#pragma once

#include <atlcomcli.h>
#include <ExDispid.h>

template <class T,class Interface>
class DispEventWB2 : public IDispEventImpl<0,T>
{
public:
  CComPtr<IUnknown> m_pUnk;

  HRESULT AdviseWB2(IUnknown* pUnk)
  {
    HRESULT hr;
    m_pUnk = pUnk;
    AtlGetObjectSourceInterface(pUnk,&m_libid, &m_iid, &m_wMajorVerNum, &m_wMinorVerNum);
    hr = DispEventAdvise(pUnk, &m_iid);
    return hr;
  }

  HRESULT UnadviseWB2()
  {
    HRESULT hr = S_OK;

    if (m_pUnk)
    {
      AtlGetObjectSourceInterface(m_pUnk,&m_libid, &m_iid, &m_wMajorVerNum, &m_wMinorVerNum);
      hr =  DispEventUnadvise(m_pUnk, &m_iid);
      m_pUnk.Release();
    }
    return hr;
  }

};

template <class T, class Interface>
class WebBrowserControl : public CComPtr<Interface>,
    public CWindowImpl<WebBrowserControl<T, Interface>, CAxWindow>,
    public DispEventWB2<T, Interface>,
    public IDispEventImpl<1, T, &__uuidof(HTMLDocumentEvents2), &LIBID_MSHTML, 4, 0>
{
public:

#define SP_DOCHOSTUIFLAG DOCHOSTUIFLAG_THEME | DOCHOSTUIFLAG_SCROLL_NO | DOCHOSTUIFLAG_NO3DBORDER \
        | DOCHOSTUIFLAG_DISABLE_HELP_MENU | DOCHOSTUIFLAG_DIALOG | DOCHOSTUIFLAG_DISABLE_SCRIPT_INACTIVE \
        | DOCHOSTUIFLAG_OVERRIDEBEHAVIORFACTORY

  BEGIN_MSG_MAP(WebBrowserControl)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
  END_MSG_MAP()

  virtual ~WebBrowserControl() {}

  CComQIPtr<IWebBrowser2> m_wb2;
  CComPtr<IHTMLDocument2> m_doc2;

  void AdviseEvtDoc2()
  {
    HRESULT hr;
    LPDISPATCH  lpDisp;
    if (!m_wb2)
      return;

    hr = m_wb2->get_Document(&lpDisp);
    if(FAILED(hr))
      return;

    hr = lpDisp->QueryInterface(IID_IHTMLDocument2,(void**)&m_doc2);
    lpDisp->Release();
    if(SUCCEEDED(hr))
    {
      hr = _IDispEventLocator<1, &__uuidof(HTMLDocumentEvents2)>::DispEventAdvise(m_doc2, &__uuidof(HTMLDocumentEvents2));
      ATLASSERT(SUCCEEDED(hr));
    }
  }

  void UnadviseEvtDoc2()
  {
    if (!m_doc2)
      return;

    _IDispEventLocator<1, &__uuidof(HTMLDocumentEvents2)>::DispEventUnadvise(m_doc2, &__uuidof(HTMLDocumentEvents2));
  }

  LRESULT OnCreate( UINT uMsg, WPARAM wParam , LPARAM lParam, BOOL & bHandled )
  {
    LRESULT lRet;
    // We must call DefWindowProc before we can attach to the control.
    lRet = DefWindowProc( uMsg, wParam,lParam );
    // Get the Pointer to the control with Events (true)
    AttachControl();

    //Hide context menu and set the window style
    CComPtr<IUnknown> spUnk;
    AtlAxGetHost(m_hWnd, &spUnk);
    CComQIPtr<IAxWinAmbientDispatch> spWinAmb(spUnk);
    if (spWinAmb)
    {
      spWinAmb->put_AllowContextMenu(VARIANT_FALSE);
      spWinAmb->put_DocHostFlags(SP_DOCHOSTUIFLAG);
    }
    return lRet;
  }

  LRESULT OnDestroy(UINT uMsg, WPARAM wParam , LPARAM lParam, BOOL & bHandled)
  {
    UnadviseWB2();
    UnadviseEvtDoc2();
    return 0;
  }


  HRESULT AttachControl()
  {
    HRESULT hr = S_OK;
    CComPtr<IUnknown> spUnk;
    // Get the IUnknown interface of control
    hr |= AtlAxGetControl( m_hWnd, &spUnk);

    if (FAILED(hr))
      return hr;

    if (!spUnk)
      return hr;

    m_wb2 = spUnk;
    // Query our interface
    hr |= spUnk->QueryInterface( __uuidof(Interface), (void**) (CComPtr<Interface>*)this);
    hr|= AdviseWB2( spUnk );
    AdviseEvtDoc2();
    return hr;
  }

};


class MovieComment : public WebBrowserControl<MovieComment, IWebBrowser2>
{
public:
  // BEGIN_MSG_MAP is optional, you don't need to define or use it if you don't want
  BEGIN_MSG_MAP(MovieComment)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)			
  END_MSG_MAP()

#define EVENTFN void __stdcall

  // BEGIN_SINK_MAP is optional, you don't need to define or use it if you don't want
  BEGIN_SINK_MAP( MovieComment )
      SINK_ENTRY_EX(1, __uuidof(HTMLDocumentEvents2), DISPID_HTMLELEMENTEVENTS2_ONCLICK, OnEventClick)
  END_SINK_MAP()

  STDMETHOD(OnEventClick)(IHTMLEventObj* pEvtObj)
  {
    CComPtr<IHTMLEventObj>  evtobj;
    CComPtr<IHTMLWindow2>   wnd;
    CComPtr<IHTMLElement>   elem;
    CComBSTR    elemid;

    if(!pEvtObj)
    {
        WebBrowserControl<MovieComment, IWebBrowser2>::m_doc2->get_parentWindow(&wnd);
        wnd->get_event(&evtobj);
    }
    else
        evtobj = pEvtObj;

    evtobj->get_srcElement(&elem);
    elem->get_id(&elemid);

    if (elemid == L"win_close_btn")
        HideFrame();

    return S_OK;
  }

  MovieComment();
  ~MovieComment();

  void ShowFrame();
  void HideFrame();
  void CloseFrame();
  BOOL IsShow();
  void OpenUrl(std::wstring url);
  RECT CalcWndPos();
  HRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

private:
  BOOL m_showframe;
};