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
#include <dlgs.h>       // for standard control IDs for commdlg



#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// Windows 2000 Print dialog

#include "initguid.h"
#define INITGUID

DEFINE_GUID(IID_IPrintDialogCallback, 0x5852a2c3, 0x6530, 0x11d1, 0xb6, 0xa3, 0x0, 0x0, 0xf8, 0x75, 0x7b, 0xf9);
DEFINE_GUID(IID_IPrintDialogServices, 0x509aaeda, 0x5639, 0x11d1, 0xb6, 0xa1, 0x0, 0x0, 0xf8, 0x75, 0x7b, 0xf9);

BEGIN_MESSAGE_MAP(CPrintDialogEx, CCommonDialog)
	//{{AFX_MSG_MAP(CPrintDialogEx)
	ON_MESSAGE(WM_INITDIALOG, &CPrintDialogEx::HandleInitDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CPrintDialogEx::CPrintDialogEx(DWORD dwFlags, CWnd* pParentWnd)
	: m_pUnkSite(NULL), CCommonDialog(pParentWnd)
{
	memset(&m_pdex, 0, sizeof(m_pdex));

	m_pdex.lStructSize = sizeof(m_pdex);
	m_pdex.Flags = dwFlags;
	m_pdex.nStartPage = START_PAGE_GENERAL;

	m_pdex.Flags &= ~PD_RETURNIC; // do not support information context
}

INT_PTR CPrintDialogEx::DoModal()
{
	ASSERT_VALID(this);

	m_pdex.hwndOwner = PreModal();
	ASSERT(m_pdex.lpCallback == NULL);
	m_pdex.lpCallback = (IUnknown*)(IPrintDialogCallback*)this;
	INT_PTR nResult = ::AfxCtxPrintDlgEx(&m_pdex);
	PostModal();
	return nResult;
}

// Create an HDC without calling DoModal.
HDC CPrintDialogEx::CreatePrinterDC()
{
	ASSERT_VALID(this);
	m_pdex.hDC = AfxCreateDC(m_pdex.hDevNames, m_pdex.hDevMode);
	return m_pdex.hDC;
}

int CPrintDialogEx::GetCopies() const
{
	ASSERT_VALID(this);

	if (m_pdex.Flags & PD_USEDEVMODECOPIES)
    {
        LPDEVMODE pDevMode=GetDevMode();
        ENSURE(pDevMode);
		return pDevMode->dmCopies;
    }
	else
    {
		return m_pdex.nCopies;
    }
}

LPDEVMODE CPrintDialogEx::GetDevMode() const
{
	if (m_pdex.hDevMode == NULL)
		return NULL;

	return (LPDEVMODE)::GlobalLock(m_pdex.hDevMode);
}

CString CPrintDialogEx::GetDriverName() const
{
	if (m_pdex.hDevNames == NULL)
		return (LPCTSTR)NULL;

	LPDEVNAMES lpDev = (LPDEVNAMES)GlobalLock(m_pdex.hDevNames);
	return (LPCTSTR)lpDev + lpDev->wDriverOffset;
}

CString CPrintDialogEx::GetDeviceName() const
{
	if (m_pdex.hDevNames == NULL)
		return (LPCTSTR)NULL;

	LPDEVNAMES lpDev = (LPDEVNAMES)GlobalLock(m_pdex.hDevNames);
	return (LPCTSTR)lpDev + lpDev->wDeviceOffset;
}

CString CPrintDialogEx::GetPortName() const
{
	if (m_pdex.hDevNames == NULL)
		return (LPCTSTR)NULL;

	LPDEVNAMES lpDev = (LPDEVNAMES)GlobalLock(m_pdex.hDevNames);
	return (LPCTSTR)lpDev + lpDev->wOutputOffset;
}

// this function must not be in afxdlgs.inl because of DLL delay loading
BOOL CPrintDialogEx::GetDefaults()
{
	m_pdex.Flags |= PD_RETURNDEFAULT;
	return SUCCEEDED(::AfxCtxPrintDlgEx(&m_pdex));
}

////////////////////////////////////////////////////////////////////////////
// CPrintDialogEx interfaces

// IUnknown
HRESULT CPrintDialogEx::QueryInterface(REFIID riid, void** ppvObject)
{
	if(ppvObject == NULL)
		return E_POINTER;

	if(InlineIsEqualUnknown(riid) || InlineIsEqualGUID(riid, IID_IPrintDialogCallback))
	{
		*ppvObject = (IPrintDialogCallback*)this;
//		AddRef();
		return S_OK;
	}
	else if(InlineIsEqualGUID(riid, IID_IObjectWithSite))
	{
		*ppvObject = (IObjectWithSite*)this;
//		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

ULONG CPrintDialogEx::AddRef()
{
	return 1;
}

ULONG CPrintDialogEx::Release()
{
	return 1;
}

// IPrintDialogCallback
HRESULT CPrintDialogEx::InitDone()
{
	return S_FALSE;
}

HRESULT CPrintDialogEx::SelectionChange()
{
	return S_FALSE;
}

HRESULT CPrintDialogEx::HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT* plResult)
{
	// set up m_hWnd the first time
	if(m_hWnd == NULL)
		Attach(hWnd);

	// we need to set this, because the message comes from the another loop
	_AFX_THREAD_STATE* pThreadState = _afxThreadState.GetData();
	pThreadState->m_lastSentMsg.hwnd = hWnd;
	pThreadState->m_lastSentMsg.message = message;
	pThreadState->m_lastSentMsg.wParam = wParam;
	pThreadState->m_lastSentMsg.lParam = lParam;
	pThreadState->m_lastSentMsg.time = ::GetMessageTime();
	pThreadState->m_lastSentMsg.pt = CPoint(::GetMessagePos());

	// call message map
	HRESULT hRet = OnWndMsg(message, wParam, lParam, plResult) ? S_OK : S_FALSE;

	// special case handling
	if(hRet == S_OK && message == WM_NOTIFY)	// return in DWL_MSGRESULT
		::SetWindowLongPtr(::GetParent(hWnd), DWLP_MSGRESULT, (LONG_PTR)*plResult);

	if(message == WM_INITDIALOG && hRet == S_OK && (BOOL)*plResult != FALSE)
		hRet = S_FALSE;

	return hRet;
}

// IObjectWithSite
HRESULT CPrintDialogEx::SetSite(IUnknown *pUnkSite)
{
	if (m_pUnkSite != NULL)
		m_pUnkSite->Release();
	m_pUnkSite = pUnkSite;
	if (m_pUnkSite != NULL)
		m_pUnkSite->AddRef();
	return S_OK;
}

HRESULT CPrintDialogEx::GetSite(REFIID riid, void **ppvSite)
{
	HRESULT hRes = E_POINTER;
	if (ppvSite != NULL)
	{
		if (m_pUnkSite != NULL)
		{
			hRes = m_pUnkSite->QueryInterface(riid, ppvSite);
		}
		else
		{
			*ppvSite = NULL;
			hRes = E_FAIL;
		}
	}
	return hRes;
}

////////////////////////////////////////////////////////////////////////////
// CPrintDialogEx implementation (overrides)

HWND CPrintDialogEx::PreModal()
{
	// cannot call DoModal on a dialog already constructed as modeless
	ASSERT(m_hWnd == NULL);

	// allow OLE servers to disable themselves
	CWinApp* pApp = AfxGetApp();
	if (pApp != NULL)
		pApp->EnableModeless(FALSE);

	// find parent HWND
	HWND hWnd = CWnd::GetSafeOwner_(m_pParentWnd->GetSafeHwnd(), &m_hWndTop);

	// return window to use as parent for dialog
	return hWnd;
}

void CPrintDialogEx::PostModal()
{
	Detach();               // just in case

	// re-enable windows
	if (::IsWindow(m_hWndTop))
		::EnableWindow(m_hWndTop, TRUE);
	m_hWndTop = NULL;
	CWinApp* pApp = AfxGetApp();
	if (pApp != NULL)
		pApp->EnableModeless(TRUE);
}

LRESULT CPrintDialogEx::DefWindowProc(UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	// does nothing, this is a dialog
	return 0;
}

LRESULT CPrintDialogEx::HandleInitDialog(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	PreInitDialog();

#ifndef _AFX_NO_OCC_SUPPORT
	// create OLE controls
	COccManager* pOccManager = afxOccManager;
	if ((pOccManager != NULL) && (m_pOccDialogInfo != NULL))
	{
		BOOL bDlgInit;
		if (m_lpDialogInit != NULL)
			bDlgInit = pOccManager->CreateDlgControls(this, m_lpDialogInit,
				m_pOccDialogInfo);
		else
			bDlgInit = pOccManager->CreateDlgControls(this, m_lpszTemplateName,
				m_pOccDialogInfo);

		if (!bDlgInit)
		{
			TRACE(traceAppMsg, 0, "Warning: CreateDlgControls failed during dialog init.\n");
			EndDialog(-1);
			return FALSE;
		}
	}
#endif

	// Call OnInitDialog
	BOOL bResult = OnInitDialog();


#ifndef _AFX_NO_OCC_SUPPORT
	if (bResult && (m_nFlags & WF_OLECTLCONTAINER))
	{
		CWnd* pWndNext = GetNextDlgTabItem(NULL);
		if (pWndNext != NULL)
		{
			pWndNext->SetFocus();   // UI Activate OLE control
			bResult = FALSE;
		}
	}
#endif

	return bResult;
}

////////////////////////////////////////////////////////////////////////////
// CPrintDialogEx diagnostics

#ifdef _DEBUG
void CPrintDialogEx::Dump(CDumpContext& dc) const
{
	CDialog::Dump(dc);

	dc << "m_pdex.hwndOwner = " << m_pdex.hwndOwner;

	if (m_pdex.hDC != NULL)
		dc << "\nm_pdex.hDC = " << CDC::FromHandle(m_pdex.hDC);

	dc << "\nm_pdex.Flags = ";
	dc.DumpAsHex(m_pdex.Flags);
	dc << "\nm_pdex.nMinPage = " << m_pdex.nMinPage;
	dc << "\nm_pdex.nMaxPage = " << m_pdex.nMaxPage;
	dc << "\nm_pdex.nCopies = " << m_pdex.nCopies;

	dc << "\n";
}
#endif //_DEBUG

IMPLEMENT_DYNAMIC(CPrintDialogEx, CCommonDialog)

////////////////////////////////////////////////////////////////////////////
