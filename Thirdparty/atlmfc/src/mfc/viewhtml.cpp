// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"

#include <exdispid.h>
#include <mshtmcid.h>	// CMDSETID_Forms3 definition
#include <mshtmhst.h>	// IDM_menu item definitions
#include <mshtml.h>


#ifdef _DEBUG
#define new DEBUG_NEW


#endif

/////////////////////////////////////////////////////////////////////////////
// CHtmlView

BEGIN_MESSAGE_MAP(CHtmlView, CFormView)
	//{{AFX_MSG_MAP(CHtmlView)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_COMMAND(ID_EDIT_COPY, &CHtmlView::OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, &CHtmlView::OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_CUT, &CHtmlView::OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, &CHtmlView::OnUpdateEditCut)
	ON_COMMAND(ID_EDIT_PASTE, &CHtmlView::OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, &CHtmlView::OnUpdateEditPaste)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CHtmlView::OnFilePrint)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CHtmlView, CFormView)
	ON_EVENT(CHtmlView, AFX_IDW_PANE_FIRST, DISPID_STATUSTEXTCHANGE, OnStatusTextChange, VTS_BSTR)
	ON_EVENT(CHtmlView, AFX_IDW_PANE_FIRST, DISPID_PROGRESSCHANGE, OnProgressChange, VTS_I4 VTS_I4)
	ON_EVENT(CHtmlView, AFX_IDW_PANE_FIRST, DISPID_COMMANDSTATECHANGE, OnCommandStateChange, VTS_I4 VTS_BOOL)
	ON_EVENT(CHtmlView, AFX_IDW_PANE_FIRST, DISPID_DOWNLOADBEGIN, OnDownloadBegin, VTS_NONE)
	ON_EVENT(CHtmlView, AFX_IDW_PANE_FIRST, DISPID_DOWNLOADCOMPLETE, OnDownloadComplete, VTS_NONE)
	ON_EVENT(CHtmlView, AFX_IDW_PANE_FIRST, DISPID_TITLECHANGE, OnTitleChange, VTS_BSTR)
	ON_EVENT(CHtmlView, AFX_IDW_PANE_FIRST, DISPID_NAVIGATECOMPLETE2, NavigateComplete2, VTS_DISPATCH VTS_PVARIANT)
	ON_EVENT(CHtmlView, AFX_IDW_PANE_FIRST, DISPID_BEFORENAVIGATE2, BeforeNavigate2, VTS_DISPATCH VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CHtmlView, AFX_IDW_PANE_FIRST, DISPID_PROPERTYCHANGE, OnPropertyChange, VTS_BSTR)
	ON_EVENT(CHtmlView, AFX_IDW_PANE_FIRST, DISPID_NEWWINDOW2, OnNewWindow2, VTS_PDISPATCH VTS_PBOOL)
	ON_EVENT(CHtmlView, AFX_IDW_PANE_FIRST, DISPID_DOCUMENTCOMPLETE, DocumentComplete, VTS_DISPATCH VTS_PVARIANT)
	ON_EVENT(CHtmlView, AFX_IDW_PANE_FIRST, DISPID_NAVIGATEERROR, NavigateError, VTS_DISPATCH VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CHtmlView, AFX_IDW_PANE_FIRST, DISPID_ONQUIT, OnQuit, VTS_NONE)
	ON_EVENT(CHtmlView, AFX_IDW_PANE_FIRST, DISPID_ONVISIBLE, OnVisible, VTS_BOOL)
	ON_EVENT(CHtmlView, AFX_IDW_PANE_FIRST, DISPID_ONTOOLBAR, OnToolBar, VTS_BOOL)
	ON_EVENT(CHtmlView, AFX_IDW_PANE_FIRST, DISPID_ONMENUBAR, OnMenuBar, VTS_BOOL)
	ON_EVENT(CHtmlView, AFX_IDW_PANE_FIRST, DISPID_ONSTATUSBAR, OnStatusBar, VTS_BOOL)
	ON_EVENT(CHtmlView, AFX_IDW_PANE_FIRST, DISPID_ONFULLSCREEN, OnFullScreen, VTS_BOOL)
	ON_EVENT(CHtmlView, AFX_IDW_PANE_FIRST, DISPID_ONTHEATERMODE, OnTheaterMode, VTS_BOOL)
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHtmlView construction/destruction

CHtmlView::CHtmlView()
	: CFormView((LPCTSTR) NULL)
{
}

CHtmlView::~CHtmlView()
{
}

BOOL CHtmlView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= WS_CLIPCHILDREN;

	return CFormView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CHtmlView drawing

void CHtmlView::OnDraw(CDC* /* pDC */)
{
	// this class should never do its own drawing;
	// the browser control should handle everything

	ASSERT(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CHtmlView printing

void CHtmlView::OnFilePrint()
{
	// get the HTMLDocument

	if (m_pBrowserApp != NULL)
	{		
		CComPtr<IDispatch> spDisp;
		m_pBrowserApp->get_Document(&spDisp);
		if (spDisp != NULL)
		{
			// the control will handle all printing UI

			CComQIPtr<IOleCommandTarget> spTarget = spDisp;
			if (spTarget != NULL)
				spTarget->Exec(NULL, OLECMDID_PRINT, 0, NULL, NULL);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CHtmlView diagnostics

#ifdef _DEBUG
void CHtmlView::AssertValid() const
{
	CFormView::AssertValid();
}

void CHtmlView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CHtmlView message handlers

void CHtmlView::OnDestroy()
{
}

void CHtmlView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	if (::IsWindow(m_wndBrowser.m_hWnd))
	{
		// need to push non-client borders out of the client area
		CRect rect;
		GetClientRect(rect);
		::AdjustWindowRectEx(rect,
			m_wndBrowser.GetStyle(), FALSE, WS_EX_CLIENTEDGE);
		m_wndBrowser.SetWindowPos(NULL, rect.left, rect.top,
			rect.Width(), rect.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
	}
}

void CHtmlView::OnPaint()
{
	Default();
}

/////////////////////////////////////////////////////////////////////////////
// CHtmlView operations

#pragma warning(disable:4100)

class CHtmlControlSite : public COleControlSite
{
public:
	CHtmlControlSite(COleControlContainer* pParentWnd);
	~CHtmlControlSite();

	CHtmlView* GetView() const;

	BEGIN_INTERFACE_PART(DocHostUIHandler, IDocHostUIHandler)
		STDMETHOD(ShowContextMenu)(DWORD, LPPOINT, LPUNKNOWN, LPDISPATCH);
		STDMETHOD(GetHostInfo)(DOCHOSTUIINFO*);
		STDMETHOD(ShowUI)(DWORD, LPOLEINPLACEACTIVEOBJECT,
			LPOLECOMMANDTARGET, LPOLEINPLACEFRAME, LPOLEINPLACEUIWINDOW);
		STDMETHOD(HideUI)(void);
		STDMETHOD(UpdateUI)(void);
		STDMETHOD(EnableModeless)(BOOL);
		STDMETHOD(OnDocWindowActivate)(BOOL);
		STDMETHOD(OnFrameWindowActivate)(BOOL);
		STDMETHOD(ResizeBorder)(LPCRECT, LPOLEINPLACEUIWINDOW, BOOL);
		STDMETHOD(TranslateAccelerator)(LPMSG, const GUID*, DWORD);
		STDMETHOD(GetOptionKeyPath)(OLECHAR **, DWORD);
		STDMETHOD(GetDropTarget)(LPDROPTARGET, LPDROPTARGET*);
		STDMETHOD(GetExternal)(LPDISPATCH*);
		STDMETHOD(TranslateUrl)(DWORD, OLECHAR*, OLECHAR **);
		STDMETHOD(FilterDataObject)(LPDATAOBJECT , LPDATAOBJECT*);
	END_INTERFACE_PART(DocHostUIHandler)

	DECLARE_INTERFACE_MAP()
};

BEGIN_INTERFACE_MAP(CHtmlControlSite, COleControlSite)
	INTERFACE_PART(CHtmlControlSite, IID_IDocHostUIHandler, DocHostUIHandler)
END_INTERFACE_MAP()

CHtmlControlSite::CHtmlControlSite(COleControlContainer* pContainer)
: COleControlSite(pContainer)
{
}

CHtmlControlSite::~CHtmlControlSite()
{
}

inline CHtmlView* CHtmlControlSite::GetView() const
{
	return STATIC_DOWNCAST(CHtmlView, m_pCtrlCont->m_pWnd);
}

STDMETHODIMP CHtmlControlSite::XDocHostUIHandler::GetExternal(LPDISPATCH *lppDispatch)
{
	METHOD_PROLOGUE_EX_(CHtmlControlSite, DocHostUIHandler)

	CHtmlView* pView = pThis->GetView();
	ASSERT_VALID(pView);
	if (pView == NULL)
	{
		return E_FAIL;
	}
	return pView->OnGetExternal(lppDispatch);
}

HRESULT CHtmlView::OnGetExternal(LPDISPATCH*)
{
	// default tells control we don't have an external
	return S_FALSE;
}

STDMETHODIMP CHtmlControlSite::XDocHostUIHandler::ShowContextMenu(
	DWORD dwID, LPPOINT ppt, LPUNKNOWN pcmdtReserved, LPDISPATCH pdispReserved)
{
	METHOD_PROLOGUE_EX_(CHtmlControlSite, DocHostUIHandler)

	CHtmlView* pView = pThis->GetView();
	ASSERT_VALID(pView);
	if (pView == NULL)
	{
		return E_FAIL;
	}
	return pView->OnShowContextMenu(dwID, ppt, pcmdtReserved, pdispReserved);
}

HRESULT CHtmlView::OnShowContextMenu(DWORD, LPPOINT, LPUNKNOWN, LPDISPATCH)
{
	// default tells control that we didn't show a menu,
	// and the control should show the menu

	return S_FALSE;
}

STDMETHODIMP CHtmlControlSite::XDocHostUIHandler::GetHostInfo(
	DOCHOSTUIINFO *pInfo)
{
	METHOD_PROLOGUE_EX_(CHtmlControlSite, DocHostUIHandler)
	CHtmlView* pView = pThis->GetView();
	ASSERT_VALID(pView);
	if (pView == NULL)
	{
		return E_FAIL;
	}
	return pView->OnGetHostInfo(pInfo);
}

HRESULT CHtmlView::OnGetHostInfo(DOCHOSTUIINFO*)
{
	// default indicates we don't have info
	return S_OK;
}

STDMETHODIMP CHtmlControlSite::XDocHostUIHandler::ShowUI(
	DWORD dwID, LPOLEINPLACEACTIVEOBJECT pActiveObject,
	LPOLECOMMANDTARGET pCommandTarget, LPOLEINPLACEFRAME pFrame,
	LPOLEINPLACEUIWINDOW pDoc)
{
	METHOD_PROLOGUE_EX_(CHtmlControlSite, DocHostUIHandler)

	CHtmlView* pView = pThis->GetView();
	ASSERT_VALID(pView);
	if (pView == NULL)
	{
		return E_FAIL;
	}
	return pView->OnShowUI(dwID, pActiveObject, pCommandTarget, pFrame, pDoc);
}

HRESULT CHtmlView::OnShowUI(DWORD, LPOLEINPLACEACTIVEOBJECT,
	LPOLECOMMANDTARGET, LPOLEINPLACEFRAME, LPOLEINPLACEUIWINDOW)
{
	// default means we don't have any UI, and control should show its UI

	return S_FALSE;
}

STDMETHODIMP CHtmlControlSite::XDocHostUIHandler::HideUI(void)
{
	METHOD_PROLOGUE_EX_(CHtmlControlSite, DocHostUIHandler)

	CHtmlView* pView = pThis->GetView();
	ASSERT_VALID(pView);
	if (pView == NULL)
	{
		return E_FAIL;
	}
	return pView->OnHideUI();
}

HRESULT CHtmlView::OnHideUI()
{
	// we don't have UI by default, so just pretend we hid it
	return S_OK;
}

STDMETHODIMP CHtmlControlSite::XDocHostUIHandler::UpdateUI(void)
{
	METHOD_PROLOGUE_EX_(CHtmlControlSite, DocHostUIHandler)

	CHtmlView* pView = pThis->GetView();
	ASSERT_VALID(pView);
	if (pView == NULL)
	{
		return E_FAIL;
	}
	return pView->OnUpdateUI();
}

HRESULT CHtmlView::OnUpdateUI()
{
	// we don't have UI by default, so just pretend we updated it
	return S_OK;
}

STDMETHODIMP CHtmlControlSite::XDocHostUIHandler::EnableModeless(BOOL fEnable)
{
	METHOD_PROLOGUE_EX_(CHtmlControlSite, DocHostUIHandler)
	CHtmlView* pView = pThis->GetView();
	ASSERT_VALID(pView);
	if (pView == NULL)
	{
		return E_FAIL;
	}
	return pView->OnEnableModeless(fEnable);
}

HRESULT CHtmlView::OnEnableModeless(BOOL)
{
	// we don't have any UI by default, so pretend we updated it
	return S_OK;
}

STDMETHODIMP CHtmlControlSite::XDocHostUIHandler::OnDocWindowActivate(BOOL fActivate)
{
	METHOD_PROLOGUE_EX_(CHtmlControlSite, DocHostUIHandler)
	CHtmlView* pView = pThis->GetView();
	ASSERT_VALID(pView);
	if (pView == NULL)
	{
		return E_FAIL;
	}
	return pView->OnDocWindowActivate(fActivate);
}

HRESULT CHtmlView::OnDocWindowActivate(BOOL)
{
	// we don't have any UI by default, so pretend we updated it
	return S_OK;
}

STDMETHODIMP CHtmlControlSite::XDocHostUIHandler::OnFrameWindowActivate(
	BOOL fActivate)
{
	METHOD_PROLOGUE_EX_(CHtmlControlSite, DocHostUIHandler)
	CHtmlView* pView = pThis->GetView();
	ASSERT_VALID(pView);
	if (pView == NULL)
	{
		return E_FAIL;
	}
	return pView->OnFrameWindowActivate(fActivate);
}

HRESULT CHtmlView::OnFrameWindowActivate(BOOL)
{
	// we don't have any UI by default, so pretend we updated it
	return S_OK;
}

STDMETHODIMP CHtmlControlSite::XDocHostUIHandler::ResizeBorder(
	LPCRECT prcBorder, LPOLEINPLACEUIWINDOW pUIWindow, BOOL fFrameWindow)
{
	METHOD_PROLOGUE_EX_(CHtmlControlSite, DocHostUIHandler)
	CHtmlView* pView = pThis->GetView();
	ASSERT_VALID(pView);
	if (pView == NULL)
	{
		return E_FAIL;
	}
	return pView->OnResizeBorder(prcBorder, pUIWindow, fFrameWindow);
}

HRESULT CHtmlView::OnResizeBorder(LPCRECT, LPOLEINPLACEUIWINDOW, BOOL)
{
	// we don't have any UI by default, so pretend we updated it
	return S_OK;
}

STDMETHODIMP CHtmlControlSite::XDocHostUIHandler::TranslateAccelerator(
	LPMSG lpMsg, const GUID* pguidCmdGroup, DWORD nCmdID)
{
	METHOD_PROLOGUE_EX_(CHtmlControlSite, DocHostUIHandler)
	CHtmlView* pView = pThis->GetView();
	ASSERT_VALID(pView);
	if (pView == NULL)
	{
		return E_FAIL;
	}
	return pView->OnTranslateAccelerator(lpMsg, pguidCmdGroup, nCmdID);
}

HRESULT CHtmlView::OnTranslateAccelerator(LPMSG, const GUID*, DWORD)
{
	// no translation here
	return S_FALSE;
}

STDMETHODIMP CHtmlControlSite::XDocHostUIHandler::GetOptionKeyPath(
	LPOLESTR* pchKey, DWORD dwReserved)
{
	METHOD_PROLOGUE_EX_(CHtmlControlSite, DocHostUIHandler)
	CHtmlView* pView = pThis->GetView();
	ASSERT_VALID(pView);
	if (pView == NULL)
	{
		return E_FAIL;
	}
	return pView->OnGetOptionKeyPath(pchKey, dwReserved);
}

HRESULT CHtmlView::OnGetOptionKeyPath(LPOLESTR*, DWORD)	
{
	// no replacement option key
	return S_FALSE;
}

STDMETHODIMP CHtmlControlSite::XDocHostUIHandler::GetDropTarget(
	LPDROPTARGET pDropTarget, LPDROPTARGET* ppDropTarget)
{
	METHOD_PROLOGUE_EX_(CHtmlControlSite, DocHostUIHandler)
	CHtmlView* pView = pThis->GetView();
	ASSERT_VALID(pView);
	if (pView == NULL)
	{
		return E_FAIL;
	}
	return pView->OnGetDropTarget(pDropTarget, ppDropTarget);
}

HRESULT CHtmlView::OnGetDropTarget(LPDROPTARGET, LPDROPTARGET*)
{
	// no additional drop target
	return S_FALSE;
}

STDMETHODIMP CHtmlControlSite::XDocHostUIHandler::TranslateUrl(
	DWORD dwTranslate, OLECHAR* pchURLIn, OLECHAR** ppchURLOut)
{
	METHOD_PROLOGUE_EX_(CHtmlControlSite, DocHostUIHandler)
	CHtmlView* pView = pThis->GetView();
	ASSERT_VALID(pView);
	if (pView == NULL)
	{
		return E_FAIL;
	}
	return pView->OnTranslateUrl(dwTranslate, pchURLIn, ppchURLOut);
}

HRESULT CHtmlView::OnTranslateUrl(DWORD, OLECHAR*, OLECHAR**)
{
	// no translation happens by default
	return S_FALSE;
}

STDMETHODIMP CHtmlControlSite::XDocHostUIHandler::FilterDataObject(
	LPDATAOBJECT pDataObject, LPDATAOBJECT* ppDataObject)
{
	METHOD_PROLOGUE_EX_(CHtmlControlSite, DocHostUIHandler)
	CHtmlView* pView = pThis->GetView();
	ASSERT_VALID(pView);
	if (pView == NULL)
	{
		return E_FAIL;
	}
	return pView->OnFilterDataObject(pDataObject, ppDataObject);
}

HRESULT CHtmlView::OnFilterDataObject(LPDATAOBJECT pDataObject,
									LPDATAOBJECT* ppDataObject)
{
	// no data objects by default
	return S_FALSE;
}

STDMETHODIMP_(ULONG) CHtmlControlSite::XDocHostUIHandler::AddRef()
{
	METHOD_PROLOGUE_EX_(CHtmlControlSite, DocHostUIHandler)
	return pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) CHtmlControlSite::XDocHostUIHandler::Release()
{
	METHOD_PROLOGUE_EX_(CHtmlControlSite, DocHostUIHandler)
	return pThis->ExternalRelease();
}

STDMETHODIMP CHtmlControlSite::XDocHostUIHandler::QueryInterface(
		  REFIID iid, LPVOID far* ppvObj)     
{
	METHOD_PROLOGUE_EX_(CHtmlControlSite, DocHostUIHandler)
	return pThis->ExternalQueryInterface(&iid, ppvObj);
}

BOOL CHtmlView::CreateControlSite(COleControlContainer* pContainer, 
   COleControlSite** ppSite, UINT /* nID */, REFCLSID /* clsid */)
{
	ASSERT(ppSite != NULL);
	*ppSite = new CHtmlControlSite(pContainer);
	return TRUE;
}

BOOL CHtmlView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
						DWORD dwStyle, const RECT& rect, CWnd* pParentWnd,
						UINT nID, CCreateContext* pContext)
{
	// create the view window itself
	m_pCreateContext = pContext;
	if (!CView::Create(lpszClassName, lpszWindowName,
				dwStyle, rect, pParentWnd,  nID, pContext))
	{
		return FALSE;
	}

	// assure that control containment is on
	AfxEnableControlContainer();

	RECT rectClient;
	GetClientRect(&rectClient);

	// create the control window
	// AFX_IDW_PANE_FIRST is a safe but arbitrary ID
	if (!m_wndBrowser.CreateControl(CLSID_WebBrowser, lpszWindowName,
				WS_VISIBLE | WS_CHILD, rectClient, this, AFX_IDW_PANE_FIRST))
	{
		DestroyWindow();
		return FALSE;
	}

	// cache the dispinterface
	LPUNKNOWN lpUnk = m_wndBrowser.GetControlUnknown();
	HRESULT hr = lpUnk->QueryInterface(IID_IWebBrowser2, (void**) &m_pBrowserApp);
	if (!SUCCEEDED(hr))
	{
		m_pBrowserApp = NULL;
		m_wndBrowser.DestroyWindow();
		DestroyWindow();
		return FALSE;
	}

	return TRUE;
}

BOOL CHtmlView::PreTranslateMessage(MSG* pMsg) 
{
	ASSERT(pMsg != NULL);
	ASSERT_VALID(this);
	ASSERT(m_hWnd != NULL);

	// allow tooltip messages to be filtered (skip CFormView)
	if (CView::PreTranslateMessage(pMsg))
		return TRUE;

	// don't translate dialog messages when in Shift+F1 help mode
	CFrameWnd* pFrameWnd = GetTopLevelFrame();
	if (pFrameWnd != NULL && pFrameWnd->m_bHelpMode)
		return FALSE;

	// call all frame windows' PreTranslateMessage first
	pFrameWnd = GetParentFrame();   // start with first parent frame
	while (pFrameWnd != NULL)
	{
		// allow owner & frames to translate
		if (pFrameWnd->PreTranslateMessage(pMsg))
			return TRUE;

		// try parent frames until there are no parent frames
		pFrameWnd = pFrameWnd->GetParentFrame();
	}

	// check if the browser control wants to handle the message
	BOOL bRet = FALSE;
	if(m_pBrowserApp != NULL)
	{
		CComQIPtr<IOleInPlaceActiveObject> spInPlace = m_pBrowserApp;
		if (spInPlace)
			bRet = (spInPlace->TranslateAccelerator(pMsg) == S_OK) ? TRUE : FALSE;
	}

	return bRet;
}

/////////////////////////////////////////////////////////////////////////////
// CHtmlView properties

CString CHtmlView::GetType() const
{
	ASSERT(m_pBrowserApp != NULL);

	CComBSTR bstr;
	HRESULT hr = m_pBrowserApp->get_Type(&bstr);
	if(FAILED(hr))
	{
		ASSERT(FALSE);
		return CString();
	}
		
	CString retVal(bstr);
	return retVal;
}

long CHtmlView::GetLeft() const
{
	ASSERT(m_pBrowserApp != NULL);

	long result;
	HRESULT hr = m_pBrowserApp->get_Left(&result);
	if(FAILED(hr))
		AfxThrowUserException();
	
	return result;
}


long CHtmlView::GetTop() const
{
	ASSERT(m_pBrowserApp != NULL);
	long result;
	HRESULT hr = m_pBrowserApp->get_Top(&result);
	if(FAILED(hr))
		AfxThrowUserException();
	
	return result;
}

int CHtmlView::GetToolBar() const
{
	ASSERT(m_pBrowserApp != NULL);
	int result;
	HRESULT hr = m_pBrowserApp->get_ToolBar(&result);
	if(FAILED(hr))
		AfxThrowUserException();
	
	return result;
}

long CHtmlView::GetHeight() const
{
	ASSERT(m_pBrowserApp != NULL);
	long result;
	HRESULT hr = m_pBrowserApp->get_Height(&result);
	if(FAILED(hr))
		AfxThrowUserException();
	
	return result;
}

long CHtmlView::GetWidth() const
{
	ASSERT(m_pBrowserApp != NULL);
	long result;
	HRESULT hr = m_pBrowserApp->get_Width(&result);
	if(FAILED(hr))
		AfxThrowUserException();
	
	return result;
}

BOOL CHtmlView::GetVisible() const
{
	ASSERT(m_pBrowserApp != NULL);

	VARIANT_BOOL result;
	HRESULT hr = m_pBrowserApp->get_Visible(&result);
	if(FAILED(hr))
	{
		ASSERT(FALSE);
		return FALSE;
	}
		
	return result;
}

CString CHtmlView::GetLocationName() const
{
	ASSERT(m_pBrowserApp != NULL);

	CComBSTR bstr;
	HRESULT hr = m_pBrowserApp->get_LocationName(&bstr);
	if(FAILED(hr))
	{
		ASSERT(FALSE);
		return CString();
	}

	CString retVal(bstr);
	return retVal;
}

CString CHtmlView::GetLocationURL() const
{
	ASSERT(m_pBrowserApp != NULL);

	CComBSTR bstr;
	HRESULT hr = m_pBrowserApp->get_LocationURL(&bstr);
	if(FAILED(hr))
	{
		ASSERT(FALSE);
		return CString();
	}
	
	CString retVal(bstr);
	return retVal;
}

BOOL CHtmlView::GetBusy() const
{
	ASSERT(m_pBrowserApp != NULL);

	VARIANT_BOOL result;
	HRESULT hr = m_pBrowserApp->get_Busy(&result);
	if(FAILED(hr))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return result;
}

READYSTATE CHtmlView::GetReadyState() const
{
	ASSERT(m_pBrowserApp != NULL);

	READYSTATE result;
	HRESULT hr = m_pBrowserApp->get_ReadyState(&result);
	if(FAILED(hr))
		AfxThrowUserException();
		
	return result;
}

BOOL CHtmlView::GetOffline() const
{
	ASSERT(m_pBrowserApp != NULL);

	VARIANT_BOOL result;
	HRESULT hr = m_pBrowserApp->get_Offline(&result);
	if(FAILED(hr))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return result;
}

BOOL CHtmlView::GetSilent() const
{
	ASSERT(m_pBrowserApp != NULL);

	VARIANT_BOOL result;
	HRESULT hr = m_pBrowserApp->get_Silent(&result);
	if(FAILED(hr))
	{
		ASSERT(FALSE);
		return FALSE;
	}
	
	return result;
}

LPDISPATCH CHtmlView::GetApplication() const
{
	ASSERT(m_pBrowserApp != NULL);

	LPDISPATCH result;
	HRESULT hr = m_pBrowserApp->get_Application(&result);
	if(FAILED(hr))
	{
		ASSERT(FALSE);
		return NULL;
	}
	
	return result;
}


LPDISPATCH CHtmlView::GetParentBrowser() const
{
	ASSERT(m_pBrowserApp != NULL);

	LPDISPATCH result;
	HRESULT hr = m_pBrowserApp->get_Parent(&result);
	if(FAILED(hr))
	{
		ASSERT(FALSE);
		return NULL;
	}
	
	return result;
}

LPDISPATCH CHtmlView::GetContainer() const
{
	ASSERT(m_pBrowserApp != NULL);

	LPDISPATCH result;
	HRESULT hr = m_pBrowserApp->get_Container(&result);
	if(FAILED(hr))
	{
		ASSERT(FALSE);
		return NULL;
	}
	
	return result;
}

LPDISPATCH CHtmlView::GetHtmlDocument() const
{
	ASSERT(m_pBrowserApp != NULL);

	LPDISPATCH result;
	HRESULT hr = m_pBrowserApp->get_Document(&result);
	if(FAILED(hr))
	{
		ASSERT(FALSE);
		return NULL;
	}
	
	return result;
}

BOOL CHtmlView::GetTopLevelContainer() const
{
	ASSERT(m_pBrowserApp != NULL);

	VARIANT_BOOL result;
	m_pBrowserApp->get_TopLevelContainer(&result);
	return result;
}

BOOL CHtmlView::GetMenuBar() const
{
	ASSERT(m_pBrowserApp != NULL);

	VARIANT_BOOL result;
	HRESULT hr = m_pBrowserApp->get_MenuBar(&result);
	if(FAILED(hr))
	{
		ASSERT(FALSE);
		return FALSE;
	}
	
	return result;
}

BOOL CHtmlView::GetFullScreen() const
{
	ASSERT(m_pBrowserApp != NULL);

	VARIANT_BOOL result;
	HRESULT hr = m_pBrowserApp->get_FullScreen(&result);
	if(FAILED(hr))
	{
		ASSERT(FALSE);
		return FALSE;
	}
	
	return result;
}

BOOL CHtmlView::GetStatusBar() const
{
	ASSERT(m_pBrowserApp != NULL);

	VARIANT_BOOL result;
	HRESULT hr = m_pBrowserApp->get_StatusBar(&result);
	if(FAILED(hr))
	{
		ASSERT(FALSE);
		return FALSE;
	}
	
	return result;
}

OLECMDF CHtmlView::QueryStatusWB(OLECMDID cmdID) const
{
	ASSERT(m_pBrowserApp != NULL);

	OLECMDF result;
	HRESULT hr = m_pBrowserApp->QueryStatusWB(cmdID, &result);
	if(FAILED(hr))
		AfxThrowUserException();
		
	return result;
}

void CHtmlView::ExecWB(OLECMDID cmdID, OLECMDEXECOPT cmdexecopt,
	VARIANT* pvaIn, VARIANT* pvaOut)
{
	ASSERT(m_pBrowserApp != NULL);

	m_pBrowserApp->ExecWB(cmdID, cmdexecopt, pvaIn, pvaOut);
}

HRESULT CHtmlView::ExecFormsCommand(DWORD dwCommandID,
	VARIANT* pVarIn, VARIANT* pVarOut)
{
	ENSURE(m_pBrowserApp != NULL);
	HRESULT hr = E_FAIL;
	CComPtr<IDispatch> spDispDocument;
	m_pBrowserApp->get_Document(&spDispDocument);
	CComQIPtr<IHTMLDocument2> spDoc = spDispDocument;
	if (spDoc != NULL)
	{
		CComQIPtr<IOleCommandTarget> spCmdTarget = spDoc;
		if (spCmdTarget != NULL)
			hr = spCmdTarget->Exec(&CMDSETID_Forms3, dwCommandID,
				OLECMDEXECOPT_DONTPROMPTUSER, pVarIn, pVarOut);
	}

	return hr;
}

HRESULT CHtmlView::QueryFormsCommand(DWORD dwCommandID,
	BOOL* pbSupported, BOOL* pbEnabled, BOOL* pbChecked)
{
	ENSURE(m_pBrowserApp!=NULL);
	HRESULT hr = E_FAIL;

	CComPtr<IDispatch> spDispDocument;
	m_pBrowserApp->get_Document(&spDispDocument);
	CComQIPtr<IHTMLDocument2> spDoc = spDispDocument;	
	if (spDoc != NULL)
	{
		CComQIPtr<IOleCommandTarget> spCmdTarget = spDoc;
		if (spCmdTarget != NULL)
		{
			OLECMD cmdInfo;
			cmdInfo.cmdID = dwCommandID;
			cmdInfo.cmdf = 0;

			hr = spCmdTarget->QueryStatus(&CMDSETID_Forms3, 1, &cmdInfo, NULL);

			if (SUCCEEDED(hr))
			{
				if (pbSupported != NULL)
					*pbSupported = (cmdInfo.cmdf & OLECMDF_SUPPORTED) ? TRUE : FALSE;
				if (pbEnabled != NULL)
					*pbEnabled = (cmdInfo.cmdf & OLECMDF_ENABLED) ? TRUE : FALSE;
				if (pbChecked != NULL)
					*pbChecked = (cmdInfo.cmdf & OLECMDF_LATCHED) ? TRUE : FALSE;
			}
		}
	}

	return hr;
}

BOOL CHtmlView::GetRegisterAsBrowser() const
{
	ASSERT(m_pBrowserApp != NULL);

	VARIANT_BOOL result;
	HRESULT hr = m_pBrowserApp->get_RegisterAsBrowser(&result);
	if(FAILED(hr))
	{
		ASSERT(FALSE);
		return FALSE;
	}
	
	return result;
}

BOOL CHtmlView::GetRegisterAsDropTarget() const
{
	ASSERT(m_pBrowserApp != NULL);

	VARIANT_BOOL result;
	HRESULT hr = m_pBrowserApp->get_RegisterAsDropTarget(&result);
	if(FAILED(hr))
	{
		ASSERT(FALSE);
		return FALSE;
	}
	
	return result;
}

BOOL CHtmlView::GetTheaterMode() const
{
	ASSERT(m_pBrowserApp != NULL);

	VARIANT_BOOL result;
	HRESULT hr = m_pBrowserApp->get_TheaterMode(&result);
	if(FAILED(hr))
	{
		ASSERT(FALSE);
		return FALSE;
	}
	
	return result;
}

BOOL CHtmlView::GetAddressBar() const
{
	ASSERT(m_pBrowserApp != NULL);

	VARIANT_BOOL result;
	HRESULT hr = m_pBrowserApp->get_AddressBar(&result);
	if(FAILED(hr))
	{
		ASSERT(FALSE);
		return FALSE;
	}
	
	return result;
}

/////////////////////////////////////////////////////////////////////////////
// CHtmlView operations

BOOL CHtmlView::GetSource(CString& refString)
{
	ENSURE(m_pBrowserApp != NULL);	

	BOOL bRetVal = FALSE;
	
	CComPtr<IDispatch> spDisp; 
	m_pBrowserApp->get_Document(&spDisp);	
	if (spDisp != NULL)
	{
		HGLOBAL hMemory;
		hMemory = GlobalAlloc(GMEM_MOVEABLE, 0);
		if (hMemory != NULL)
		{
			CComQIPtr<IPersistStreamInit> spPersistStream = spDisp;
			if (spPersistStream != NULL)
			{
				CComPtr<IStream> spStream;
				if (SUCCEEDED(CreateStreamOnHGlobal(hMemory, TRUE, &spStream)))
				{
					spPersistStream->Save(spStream, FALSE);

					LPCSTR pstr = static_cast<LPCSTR>(GlobalLock(hMemory));
					if (pstr != NULL)
					{
						// Stream is always ANSI, but CString
						// assignment operator will convert implicitly.

						bRetVal = TRUE;
						TRY
						{						
							refString = pstr;
						}
						CATCH_ALL(e)
						{
							bRetVal = FALSE;
							DELETE_EXCEPTION(e);
						}
						END_CATCH_ALL

						if(bRetVal == FALSE)
							GlobalFree(hMemory);
						else
							GlobalUnlock(hMemory);
					}
					else
					{
						GlobalFree(hMemory);
					}
				}
				else
				{
					GlobalFree(hMemory);
				}
			}
			else
			{
				GlobalFree(hMemory);
			}
		}
	}
	
	return bRetVal;
}

BOOL CHtmlView::LoadFromResource(LPCTSTR lpszResource)
{
	HINSTANCE hInstance = AfxGetResourceHandle();
	ASSERT(hInstance != NULL);

	CString strResourceURL;
	BOOL bRetVal = TRUE;
	LPTSTR lpszModule = new TCHAR[_MAX_PATH];

	if (GetModuleFileName(hInstance, lpszModule, _MAX_PATH))
	{
		strResourceURL.Format(_T("res://%s/%s"), lpszModule, lpszResource);
		Navigate(strResourceURL, 0, 0, 0);
	}
	else
		bRetVal = FALSE;

	delete [] lpszModule;
	return bRetVal;
}

BOOL CHtmlView::LoadFromResource(UINT nRes)
{
	HINSTANCE hInstance = AfxGetResourceHandle();
	ASSERT(hInstance != NULL);

	CString strResourceURL;
	BOOL bRetVal = TRUE;
	LPTSTR lpszModule = new TCHAR[_MAX_PATH];

	if (GetModuleFileName(hInstance, lpszModule, _MAX_PATH))
	{
		strResourceURL.Format(_T("res://%s/%d"), lpszModule, nRes);
		Navigate(strResourceURL, 0, 0, 0);
	}
	else
		bRetVal = FALSE;

	delete [] lpszModule;
	return bRetVal;
}

void CHtmlView::Navigate(LPCTSTR lpszURL, DWORD dwFlags /* = 0 */,
	LPCTSTR lpszTargetFrameName /* = NULL */ ,
	LPCTSTR lpszHeaders /* = NULL */, LPVOID lpvPostData /* = NULL */,
	DWORD dwPostDataLen /* = 0 */)
{
	CString strURL(lpszURL);
	CComBSTR bstrURL;
	bstrURL.Attach(strURL.AllocSysString());

	COleSafeArray vPostData;
	if (lpvPostData != NULL)
	{
		if (dwPostDataLen == 0)
			dwPostDataLen = lstrlen((LPCTSTR) lpvPostData);

		vPostData.CreateOneDim(VT_UI1, dwPostDataLen, lpvPostData);
	}

	m_pBrowserApp->Navigate(bstrURL,
		COleVariant((long) dwFlags, VT_I4),
		COleVariant(lpszTargetFrameName, VT_BSTR),
		vPostData,
		COleVariant(lpszHeaders, VT_BSTR));
}

void CHtmlView::Navigate2(LPITEMIDLIST pIDL, DWORD dwFlags /* = 0 */,
	LPCTSTR lpszTargetFrameName /* = NULL */)
{
	ASSERT(m_pBrowserApp != NULL);

	COleVariant vPIDL(pIDL);
	COleVariant empty;

	m_pBrowserApp->Navigate2(vPIDL,
		COleVariant((long) dwFlags, VT_I4),
		COleVariant(lpszTargetFrameName, VT_BSTR),
		empty, empty);
}

void CHtmlView::Navigate2(LPCTSTR lpszURL, DWORD dwFlags /* = 0 */,
	LPCTSTR lpszTargetFrameName /* = NULL */,
	LPCTSTR lpszHeaders /* = NULL */,
	LPVOID lpvPostData /* = NULL */, DWORD dwPostDataLen /* = 0 */)
{
	ASSERT(m_pBrowserApp != NULL);

	COleSafeArray vPostData;
	if (lpvPostData != NULL)
	{
		if (dwPostDataLen == 0)
			dwPostDataLen = lstrlen((LPCTSTR) lpvPostData);

		vPostData.CreateOneDim(VT_UI1, dwPostDataLen, lpvPostData);
	}

	COleVariant vURL(lpszURL, VT_BSTR);
	COleVariant vHeaders(lpszHeaders, VT_BSTR);
	COleVariant vTargetFrameName(lpszTargetFrameName, VT_BSTR);
	COleVariant vFlags((long) dwFlags, VT_I4);

	m_pBrowserApp->Navigate2(vURL,
		vFlags, vTargetFrameName, vPostData, vHeaders);
}

void CHtmlView::Navigate2(LPCTSTR lpszURL, DWORD dwFlags,
	CByteArray& baPostData, LPCTSTR lpszTargetFrameName /* = NULL */,
	LPCTSTR lpszHeaders /* = NULL */)
{
	ASSERT(m_pBrowserApp != NULL);

	COleVariant vPostData = baPostData;
	COleVariant vURL(lpszURL, VT_BSTR);
	COleVariant vHeaders(lpszHeaders, VT_BSTR);
	COleVariant vTargetFrameName(lpszTargetFrameName, VT_BSTR);
	COleVariant vFlags((long) dwFlags, VT_I4);

	ASSERT(m_pBrowserApp != NULL);

	m_pBrowserApp->Navigate2(vURL, vFlags, vTargetFrameName,
		vPostData, vHeaders);
}

void CHtmlView::PutProperty(LPCTSTR lpszProperty, const VARIANT& vtValue)
{
	ASSERT(m_pBrowserApp != NULL);

	CString strProp(lpszProperty);
	BSTR bstrProp = strProp.AllocSysString();
	m_pBrowserApp->PutProperty(bstrProp, vtValue);
	::SysFreeString(bstrProp);
}

BOOL CHtmlView::GetProperty(LPCTSTR lpszProperty, CString& strValue)
{
	ASSERT(m_pBrowserApp != NULL);

	CString strProperty(lpszProperty);
	BSTR bstrProperty = strProperty.AllocSysString();

	BOOL bResult = FALSE;
	VARIANT vReturn;
	vReturn.vt = VT_BSTR;
	vReturn.bstrVal = NULL;
	HRESULT hr = m_pBrowserApp->GetProperty(bstrProperty, &vReturn);

	if (SUCCEEDED(hr))
	{
		strValue = CString(vReturn.bstrVal);
		bResult = TRUE;
	}

	::SysFreeString(bstrProperty);
	return bResult;
}

COleVariant CHtmlView::GetProperty(LPCTSTR lpszProperty)
{
	COleVariant result;

	static BYTE parms[] =
		VTS_BSTR;
	m_wndBrowser.InvokeHelper(0x12f, DISPATCH_METHOD,
		VT_VARIANT, (void*)&result, parms, lpszProperty);

	return result;
}

CString CHtmlView::GetFullName() const
{
	ASSERT(m_pBrowserApp != NULL);

	CComBSTR bstr;
	HRESULT hr = m_pBrowserApp->get_FullName(&bstr);
	if(FAILED(hr))
	{
		ASSERT(FALSE);
		return CString();
	}
	
	CString retVal(bstr);
	return retVal;
}

/////////////////////////////////////////////////////////////////////////////
// CHtmlView event reflectors

void CHtmlView::NavigateComplete2(LPDISPATCH /* pDisp */, VARIANT* URL)
{
	ASSERT(V_VT(URL) == VT_BSTR);

	CString str(V_BSTR(URL));
	OnNavigateComplete2(str);
}

void CHtmlView::BeforeNavigate2(LPDISPATCH /* pDisp */, VARIANT* URL,
		VARIANT* Flags, VARIANT* TargetFrameName,
		VARIANT* PostData, VARIANT* Headers, VARIANT_BOOL* Cancel) 
{
	ASSERT(V_VT(URL) == VT_BSTR);
	ASSERT(V_VT(TargetFrameName) == VT_BSTR);
	ASSERT(V_VT(PostData) == (VT_VARIANT | VT_BYREF));
	ASSERT(V_VT(Headers) == VT_BSTR);
	ASSERT(Cancel != NULL);

	VARIANT* vtPostedData = V_VARIANTREF(PostData);
	CByteArray array;
	if (V_VT(vtPostedData) & VT_ARRAY)
	{
		// must be a vector of bytes
		ASSERT(vtPostedData->parray->cDims == 1 && vtPostedData->parray->cbElements == 1);

		vtPostedData->vt |= VT_UI1;
		COleSafeArray safe(vtPostedData);

		DWORD dwSize = safe.GetOneDimSize();
		LPVOID pVoid;
		safe.AccessData(&pVoid);

		array.SetSize(dwSize);
		LPBYTE lpByte = array.GetData();

		Checked::memcpy_s(lpByte, dwSize, pVoid, dwSize);
		safe.UnaccessData();
	}
	// make real parameters out of the notification

	CString strTargetFrameName(V_BSTR(TargetFrameName));
	CString strURL(V_BSTR(URL));
	CString strHeaders(V_BSTR(Headers));
	DWORD nFlags = V_I4(Flags);


	BOOL bCancel = FALSE;
	// notify the user's class
	OnBeforeNavigate2(strURL, nFlags, strTargetFrameName,
		array, strHeaders, &bCancel);

	if (bCancel)
		*Cancel = AFX_OLE_TRUE;
	else
		*Cancel = AFX_OLE_FALSE;
}

void CHtmlView::DocumentComplete(LPDISPATCH pDisp, VARIANT* URL)
{
	UNUSED_ALWAYS(pDisp);
	ASSERT(V_VT(URL) == VT_BSTR);

	CString str(V_BSTR(URL));
	OnDocumentComplete(str);
}

void CHtmlView::NavigateError(LPDISPATCH pDisp, VARIANT* pvURL,
	VARIANT* pvFrame, VARIANT* pvStatusCode, VARIANT_BOOL* pvbCancel)
{
	UNUSED_ALWAYS(pDisp);
	ASSERT(pvURL != NULL);
	ASSERT(pvFrame != NULL);
	ASSERT(pvStatusCode != NULL);
	ASSERT(pvbCancel != NULL);
	ASSERT(V_VT(pvURL) == VT_BSTR);
	ASSERT(V_VT(pvFrame) == VT_BSTR);

	CString strURL(V_BSTR(pvURL));
	CString strFrame(V_BSTR(pvFrame));
	DWORD dwError = V_I4(pvStatusCode);

	BOOL bCancel = FALSE;
	// notify the user's class
	OnNavigateError(strURL, strFrame, dwError, &bCancel);

	if (pvbCancel)
		*pvbCancel = bCancel ? AFX_OLE_TRUE : AFX_OLE_FALSE;
}

void CHtmlView::OnEditCopy() 
{
	ExecFormsCommand(IDM_COPY, NULL, NULL);
	return;
}

void CHtmlView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	BOOL bEnabled = FALSE;

	// Since input variables aren't touched for failure case of
	// QueryCommand(), we can ignore return value.
	QueryFormsCommand(IDM_COPY, NULL, &bEnabled, NULL);
	pCmdUI->Enable(bEnabled);
}

void CHtmlView::OnEditCut() 
{
	ExecFormsCommand(IDM_CUT, NULL, NULL);
	return;
}

void CHtmlView::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	BOOL bEnabled = FALSE;

	// Since input variables aren't touched for failure case of
	// QueryCommand(), we can ignore return value.
	QueryFormsCommand(IDM_CUT, NULL, &bEnabled, NULL);
	pCmdUI->Enable(bEnabled);
}

void CHtmlView::OnEditPaste() 
{
	ExecFormsCommand(IDM_PASTE, NULL, NULL);
	return;
}

void CHtmlView::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	BOOL bEnabled = FALSE;

	// Since input variables aren't touched for failure case of
	// QueryCommand(), we can ignore return value.
	QueryFormsCommand(IDM_PASTE, NULL, &bEnabled, NULL);
	pCmdUI->Enable(bEnabled);
}

/////////////////////////////////////////////////////////////////////////////
// CHtmlView Events

void CHtmlView::OnProgressChange(long lProgress, long lProgressMax)
{
	// user will override to handle this notification
	UNUSED_ALWAYS(lProgress);
	UNUSED_ALWAYS(lProgressMax);
}

void CHtmlView::OnCommandStateChange(long lCommand, BOOL bEnable)
{
	// user will override to handle this notification
	UNUSED_ALWAYS(lCommand);
	UNUSED_ALWAYS(bEnable);
}

void CHtmlView::OnDownloadBegin()
{
	// user will override to handle this notification
}

void CHtmlView::OnDownloadComplete()
{
	// user will override to handle this notification
}

void CHtmlView::OnTitleChange(LPCTSTR lpszText)
{
	// user will override to handle this notification
	UNUSED_ALWAYS(lpszText);
}

void CHtmlView::OnPropertyChange(LPCTSTR lpszProperty)
{
	// user will override to handle this notification
	UNUSED_ALWAYS(lpszProperty);
}

void CHtmlView::OnNewWindow2(LPDISPATCH* ppDisp, BOOL* bCancel)
{
	// default to continuing
	*bCancel = FALSE;

	// user will override to handle this notification
	UNUSED_ALWAYS(ppDisp);
}

void CHtmlView::OnDocumentComplete(LPCTSTR lpszURL)
{
	// user will override to handle this notification
	UNUSED_ALWAYS(lpszURL);
}

void CHtmlView::OnQuit()
{
	// user will override to handle this notification
}

void CHtmlView::OnVisible(BOOL bVisible)
{
	// user will override to handle this notification
	UNUSED_ALWAYS(bVisible);
}

void CHtmlView::OnToolBar(BOOL bToolBar)
{
	// user will override to handle this notification
	UNUSED_ALWAYS(bToolBar);
}

void CHtmlView::OnMenuBar(BOOL bMenuBar)
{
	// user will override to handle this notification
	UNUSED_ALWAYS(bMenuBar);
}

void CHtmlView::OnStatusBar(BOOL bStatusBar)
{
	// user will override to handle this notification
	UNUSED_ALWAYS(bStatusBar);
}

void CHtmlView::OnFullScreen(BOOL bFullScreen)
{
	// user will override to handle this notification
	UNUSED_ALWAYS(bFullScreen);
}

void CHtmlView::OnTheaterMode(BOOL bTheaterMode)
{
	// user will override to handle this notification
	UNUSED_ALWAYS(bTheaterMode);
}

void CHtmlView::OnNavigateComplete2(LPCTSTR lpszURL)
{
	// user will override to handle this notification
	UNUSED_ALWAYS(lpszURL);
}

void CHtmlView::OnBeforeNavigate2(LPCTSTR lpszURL, DWORD nFlags,
	LPCTSTR lpszTargetFrameName, CByteArray& baPostData,
	LPCTSTR lpszHeaders, BOOL* bCancel)
{
	// default to continuing
	*bCancel = FALSE;

	// user will override to handle this notification
	UNUSED_ALWAYS(lpszURL);
	UNUSED_ALWAYS(nFlags);
	UNUSED_ALWAYS(lpszTargetFrameName);
	UNUSED_ALWAYS(baPostData);
	UNUSED_ALWAYS(lpszHeaders);
}

void CHtmlView::OnStatusTextChange(LPCTSTR pszText)
{
	// try to set the status bar text via the frame

	CFrameWnd* pFrame = GetParentFrame();
	if (pFrame != NULL)
		pFrame->SetMessageText(pszText);
}

void CHtmlView::OnNavigateError(LPCTSTR lpszURL, LPCTSTR lpszFrame, DWORD dwError, BOOL *pbCancel)
{
	// default to continuing
	*pbCancel = FALSE;

	// user will override to handle this notification
	UNUSED_ALWAYS(lpszURL);
	UNUSED_ALWAYS(lpszFrame);

	TRACE(traceHtml, 0, "OnNavigateError called with status scode = 0x%X\n", dwError);
}

extern "C" void _AfxHtmlEditCtrlFakeEntry()
{
	ASSERT( FALSE );
}

/////////////////////////////////////////////////////////////////////////////
// CHtmlEditCtrl
CHtmlEditCtrl::CHtmlEditCtrl()
{
}

CHtmlEditCtrl::~CHtmlEditCtrl()
{
}

/////////////////////////////////////////////////////////////////////////////
// CHtmlEditCtrl message handlers
BEGIN_EVENTSINK_MAP(CHtmlEditCtrl, CWnd)
	ON_EVENT_REFLECT(CHtmlEditCtrl, 252 /* NavigateComplete2 */, _OnNavigateComplete2, VTS_DISPATCH VTS_PVARIANT)
END_EVENTSINK_MAP()

void CHtmlEditCtrl::_OnNavigateComplete2(LPDISPATCH pDisp, VARIANT FAR* URL)
{
	SetDesignMode(TRUE);
}


BOOL CHtmlEditCtrl::GetDHtmlDocument(IHTMLDocument2 **ppDocument) const
{
	if(ppDocument == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}
	
	*ppDocument = NULL;
	
	CHtmlEditCtrl *pThis = const_cast<CHtmlEditCtrl*>(this);
	CComQIPtr<IWebBrowser2> pBrowserApp = pThis->GetControlUnknown();
	ASSERT(pBrowserApp != NULL);
	if (pBrowserApp)
	{
		CComPtr<IDispatch> spDispDocument;
		HRESULT hr = pBrowserApp->get_Document(&spDispDocument);
		if (SUCCEEDED(hr) && spDispDocument)
		{
			return S_OK == spDispDocument->QueryInterface(ppDocument) ? TRUE : FALSE;
		}
	}
	return FALSE;
}

BOOL CHtmlEditCtrl::Create(LPCTSTR lpszWindowName, DWORD /*dwStyle*/, const RECT& rect, CWnd* pParentWnd,
						   int nID, CCreateContext *pContext) 
{
	BOOL bRet = FALSE;
	// create the control window

	AfxEnableControlContainer();
	if (CreateControl(CLSID_WebBrowser, lpszWindowName,
				WS_VISIBLE | WS_CHILD, rect, pParentWnd, nID))
	{
		// in order to put the webbrowser in design mode, you must
		// first load a document into it. "about:blank" seems to
		// be the safest thing to load for a good starting point.
		CComQIPtr<IWebBrowser2> pBrowserApp = GetControlUnknown();
		ASSERT(pBrowserApp);
		if (pBrowserApp)
		{
			CComVariant vEmpty;
			LPCTSTR szDoc = GetStartDocument();
			if (szDoc)
			{
				CComBSTR bstrStart(szDoc);
				if (S_OK == pBrowserApp->Navigate(bstrStart, 
													&vEmpty,
													&vEmpty,
													&vEmpty,
													&vEmpty))
				{
					bRet = TRUE;
				}
			}
			else
				bRet = TRUE;

		}
	}

	if (!bRet)
		DestroyWindow();
	return bRet;
}

LPCTSTR CHtmlEditCtrl::GetStartDocument()
{
	LPCTSTR szDefaultDoc = _T("about:blank");
	return szDefaultDoc;
}

/////////////////////////////////////////////////////////////////////////////
// CHtmlEditView
BEGIN_MESSAGE_MAP(CHtmlEditView, CFormView)
	ON_COMMAND(ID_FILE_PRINT, &CHtmlEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CHtmlEditView::OnFilePrint)
	ON_WM_PAINT()
	ON_WM_SIZE()
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////
// CHtmlEditView

void CHtmlEditView::OnPaint()
{
	Default();
}

CHtmlEditView::CHtmlEditView()
{
}

CHtmlEditView::~CHtmlEditView()
{
}

LPCTSTR CHtmlEditView::GetStartDocument()
{
	LPCTSTR szDefaultUrl = _T("about:blank");
	return szDefaultUrl;
}

BOOL CHtmlEditView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
	DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID,
	CCreateContext* pContext) 
{
	// create the view window itself
	if (!CHtmlView::Create(lpszClassName, lpszWindowName,
				dwStyle, rect, pParentWnd,	nID, pContext))
	{
		return FALSE;
	}
	LPCTSTR szDoc = GetStartDocument();
	if (szDoc)
		Navigate(szDoc);
	return TRUE;
}

BOOL CHtmlEditView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// if it's not something we're intersted in, let it go to the base
	if (nCode < (int)CN_UPDATE_COMMAND_UI)
		return CHtmlView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);

	// check for command availability
	BOOL bHasExecFunc = FALSE;
	UINT uiElemType = AFX_UI_ELEMTYPE_NORMAL;
	UINT dhtmlCmdID = GetDHtmlCommandMapping(nID, bHasExecFunc, uiElemType);
	if (dhtmlCmdID == AFX_INVALID_DHTML_CMD_ID)
	{
		// No mapping for this command. Use normal routing
		return CHtmlView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
	}

	long nStatus = QueryStatus(dhtmlCmdID);

	if (nCode == CN_UPDATE_COMMAND_UI)
	{
		// just checking status
		CCmdUI *pUI = static_cast<CCmdUI*>(pExtra);
		if (pUI)
		{
			if(!(nStatus & OLECMDF_LATCHED || nStatus & OLECMDF_ENABLED))
			{
				pUI->Enable(FALSE);
				if (uiElemType & AFX_UI_ELEMTYPE_CHECBOX)
				{
					if (nStatus & OLECMDF_LATCHED)
						pUI->SetCheck(TRUE);
					else
						pUI->SetCheck(FALSE);
				}
				else if (uiElemType & AFX_UI_ELEMTYPE_RADIO)
				{
					if (nStatus & OLECMDF_LATCHED)
						pUI->SetRadio(TRUE);
					else
						pUI->SetRadio(FALSE);
				}

			}
			else
			{
				pUI->Enable(TRUE); // enable
				// check to see if we need to do any other state
				// stuff
				if (uiElemType & AFX_UI_ELEMTYPE_CHECBOX)
				{
					if (nStatus & OLECMDF_LATCHED)
						pUI->SetCheck(TRUE);
					else
						pUI->SetCheck(FALSE);
				}
				else if (uiElemType & AFX_UI_ELEMTYPE_RADIO)
				{
					if (nStatus & OLECMDF_LATCHED)
						pUI->SetRadio(TRUE);
					else
						pUI->SetRadio(FALSE);
				}
			}
			return TRUE;
		}
		return FALSE;
	}

	// querystatus for this DHTML command to make sure it is enabled
	if(!(nStatus & OLECMDF_LATCHED || nStatus & OLECMDF_ENABLED))
	{

		// trying to execute a disabled command
		TRACE(traceHtml, 0, "Not executing disabled dhtml editing command %d", dhtmlCmdID);
		return TRUE;
	}

	if (bHasExecFunc)
	{
		return ExecHandler(nID);		
	}

	return S_OK == ExecCommand(dhtmlCmdID, OLECMDEXECOPT_DODEFAULT, NULL, NULL) ? TRUE : FALSE;
}

BOOL CHtmlEditView::GetDHtmlDocument(IHTMLDocument2 **ppDocument) const
{
	CComPtr<IDispatch> spDispDocument;
	if (!ppDocument)
		return FALSE;
	*ppDocument = NULL;

	HRESULT hr = m_pBrowserApp->get_Document(&spDispDocument);
	if (SUCCEEDED(hr) && spDispDocument)
	{
		return S_OK == spDispDocument->QueryInterface(ppDocument) ? TRUE : FALSE;
	}

	return FALSE;
}

void CHtmlEditView::OnNavigateComplete2(LPCTSTR strURL)
{
	SetDesignMode(TRUE);
}

BOOL CHtmlEditView::OnPreparePrinting(CPrintInfo* pInfo)
{
	return TRUE;
}

UINT CHtmlEditView::GetDHtmlCommandMapping(UINT nIDWindowsCommand, BOOL& bHasExecFunc, UINT& uiElemType)
{
	uiElemType = AFX_UI_ELEMTYPE_NORMAL;
	bHasExecFunc = FALSE;
	return AFX_INVALID_DHTML_CMD_ID;
}

BOOL CHtmlEditView::ExecHandler(UINT nCmdID)
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CHtmlEditView diagnostics

#ifdef _DEBUG
void CHtmlEditView::AssertValid() const
{
	CFormView::AssertValid();
}

void CHtmlEditView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

CHtmlEditDoc::CHtmlEditDoc()
{
}

CHtmlEditDoc::~CHtmlEditDoc()
{
}

CHtmlEditView* CHtmlEditDoc::GetView() const
{
	CHtmlEditView* pHtmlView = NULL;

	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		pHtmlView = DYNAMIC_DOWNCAST(CHtmlEditView, pView);
		if (pHtmlView != NULL)
			break;
	}

	return pHtmlView;
}

void CHtmlEditDoc::DeleteContents()
{
	CHtmlEditView* pView = GetView();
	if (pView)
	{
		pView->NewDocument();
		pView->Navigate(pView->GetStartDocument());
	}
}

BOOL CHtmlEditDoc::IsModified()
{
	CHtmlEditView* pView = GetView();

	if (pView)
	{
		return (pView->GetIsDirty() != S_FALSE);
	}

	return FALSE;
}

BOOL CHtmlEditDoc::OpenURL(LPCTSTR lpszURL)
{
	BOOL bRet = FALSE;
	CHtmlEditView* pView = GetView();
	if (pView != NULL && lpszURL != NULL && *lpszURL != '\0')
	{
		pView->Navigate(lpszURL);
		bRet = TRUE;
	}

	return bRet;
}

BOOL CHtmlEditDoc::OnOpenDocument(LPCTSTR lpszFileName)
{
	BOOL bRet = FALSE;
	CHtmlEditView* pView = GetView();
	if (pView != NULL)
	{
		pView->Navigate(lpszFileName);
		SetTitle(lpszFileName);
		bRet = TRUE;
	}

	return bRet;
}

BOOL CHtmlEditDoc::OnSaveDocument(LPCTSTR lpszFileName)
{
	BOOL bRet = FALSE;
	CHtmlEditView *pView = GetView();
	if (pView != NULL)
	{
		CFile file;
		if (file.Open(lpszFileName, CFile::modeCreate|CFile::modeWrite))
		{
			CArchive ar(&file, CArchive::store);
			CArchiveStream as(&ar);
			CComPtr<IHTMLDocument2> spHTMLDocument;
			CComQIPtr<IPersistStreamInit> spPSI;
			pView->GetDHtmlDocument(&spHTMLDocument);
			if (spHTMLDocument)
			{
				spPSI = spHTMLDocument;
				if (spPSI)
				{
					if (S_OK == spPSI->Save((IStream*)&as, TRUE))
					{
						SetModifiedFlag(FALSE);
						bRet = TRUE;
					}
				}
			}
		}
	}
	return bRet;
}

BOOL CHtmlEditDoc::OnNewDocument()
{
#ifdef _DEBUG
	if(IsModified())
		TRACE(traceAppMsg, 0, "Warning: OnNewDocument replaces an unsaved document.\n");
#endif

	DeleteContents(); // we want CHtmlEditDoc's DeleteContents to be called here
	m_strPathName.Empty();      // no path name yet
	SetModifiedFlag(FALSE);     // make clean

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CHtmlEditDoc diagnostics

#ifdef _DEBUG
void CHtmlEditDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CHtmlEditDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif _DEBUG

/////////////////////////////////////////////////////////////////////////////
// Inline function declarations expanded out-of-line

#ifndef _AFX_ENABLE_INLINES

// expand inlines for Html functions
#define _AFXHTML_INLINE
#include "afxhtml.inl"

#endif //!_AFX_ENABLE_INLINES

/////////////////////////////////////////////////////////////////////////////
// Pre-startup code


IMPLEMENT_DYNCREATE(CHtmlView, CFormView)
IMPLEMENT_DYNCREATE(CHtmlEditView, CHtmlView)
IMPLEMENT_DYNAMIC(CHtmlEditDoc, CDocument)
