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
#include "occimpl.h"
#include "sal.h"



#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// CWnd support for OLE Control containment

BOOL CWnd::CreateControl(LPCTSTR lpszClass, LPCTSTR lpszWindowName,
	DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID,
	CFile* pPersist, BOOL bStorage, BSTR bstrLicKey)
{
	ASSERT(lpszClass != NULL);

	CLSID clsid;
	HRESULT hr = AfxGetClassIDFromString(lpszClass, &clsid);
	if (FAILED(hr))
		return FALSE;

	return CreateControl(clsid, lpszWindowName, dwStyle, rect, pParentWnd, nID,
		pPersist, bStorage, bstrLicKey);
}

BOOL CWnd::CreateControl( REFCLSID clsid, LPCTSTR lpszWindowName,
   DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID,
   CFile* pPersist, BOOL bStorage, BSTR bstrLicKey )
{
   CRect rect2( rect );
   CPoint pt;
   CSize size;

   pt = rect2.TopLeft();
   size = rect2.Size();

   return( CreateControl( clsid, lpszWindowName, dwStyle, &pt, &size,
	  pParentWnd, nID, pPersist, bStorage, bstrLicKey ) );
}

BOOL CWnd::CreateControl(REFCLSID clsid, LPCTSTR lpszWindowName, DWORD dwStyle,
	const POINT* ppt, const SIZE* psize, CWnd* pParentWnd, UINT nID,
   CFile* pPersist, BOOL bStorage, BSTR bstrLicKey)
{
	ASSERT(pParentWnd != NULL);

#ifdef _DEBUG
	if (afxOccManager == NULL)
	{
		TRACE(traceOle, 0, "Warning: AfxEnableControlContainer has not been called yet.\n");
		TRACE(traceOle, 0, ">>> You should call it in your app's InitInstance function.\n");
	}
#endif

	if ((pParentWnd == NULL) || !pParentWnd->InitControlContainer())
		return FALSE;

	return pParentWnd->m_pCtrlCont->CreateControl(this, clsid, lpszWindowName,
		dwStyle, ppt, psize, nID, pPersist, bStorage, bstrLicKey);
}

BOOL CWnd::CreateControl(const CControlCreationInfo& creationInfo, DWORD dwStyle,
	const POINT* ppt, const SIZE* psize, CWnd* pParentWnd, UINT nID)
{
	ASSERT(pParentWnd != NULL);

#ifdef _DEBUG
	if (afxOccManager == NULL)
	{
		TRACE(traceOle, 0, "Warning: AfxEnableControlContainer has not been called yet.\n");
		TRACE(traceOle, 0, ">>> You should call it in your app's InitInstance function.\n");
	}
#endif

	if ((pParentWnd == NULL) || !pParentWnd->InitControlContainer())
		return FALSE;

	return pParentWnd->m_pCtrlCont->CreateControl(this, creationInfo, dwStyle, ppt, psize, nID);
}


BOOL CWnd::InitControlContainer(BOOL bCreateFromResource)
{
   if (m_pCtrlCont == NULL)
   {
	  BOOL bSuccess;

	  bSuccess = CreateControlContainer( &m_pCtrlCont );
	  if (bSuccess && (m_pCtrlCont == NULL))
	  {
		 // The window wants to use the default control container.
		  TRY
		  {
			m_pCtrlCont = afxOccManager->CreateContainer(this);
		  }
		  END_TRY
	  }
	  //When Container is created and it is not during creation from resources, 
	  //populate the list with all resource created Win32 controls.
	  if (!bCreateFromResource)
	  {
			m_pCtrlCont->FillListSitesOrWnds(GetOccDialogInfo());		
	  }
   }

	// Mark all ancestor windows as containing OLE controls.
	if (m_pCtrlCont != NULL)
	{
		CWnd* pWnd = this;
		while ((pWnd != NULL) && !(pWnd->m_nFlags & WF_OLECTLCONTAINER))
		{
			pWnd->m_nFlags |= WF_OLECTLCONTAINER;
			pWnd = pWnd->GetParent();
			if (! (GetWindowLong(pWnd->GetSafeHwnd(), GWL_STYLE) & WS_CHILD))
				break;
		}
	}
	
	return (m_pCtrlCont != NULL);
}

/////////////////////////////////////////////////////////////////////////////
// COleControlContainer

BEGIN_INTERFACE_MAP(COleControlContainer, CCmdTarget)
	INTERFACE_PART(COleControlContainer, IID_IOleInPlaceFrame, OleIPFrame)
	INTERFACE_PART(COleControlContainer, IID_IOleContainer, OleContainer)
END_INTERFACE_MAP()

BEGIN_DISPATCH_MAP(COleControlContainer, CCmdTarget)
END_DISPATCH_MAP()


COleControlSiteOrWnd::COleControlSiteOrWnd()
: m_hWnd(NULL), m_pSite(NULL), m_bAutoRadioButton(FALSE)
{
}
COleControlSiteOrWnd::COleControlSiteOrWnd(COleControlSite *pSite)
		: m_hWnd(NULL), m_pSite(pSite), m_bAutoRadioButton(FALSE)
{
}
COleControlSiteOrWnd::COleControlSiteOrWnd(HWND hWnd, BOOL bAutoRadioButton)
		: m_hWnd(hWnd), m_pSite(NULL), m_bAutoRadioButton(bAutoRadioButton)
{
}


COleControlSiteOrWnd::~COleControlSiteOrWnd()
{	
	delete m_pSite;
}

COleControlContainer::COleControlContainer(CWnd* pWnd) :
	m_pWnd(pWnd),
	m_crBack((COLORREF)-1),
	m_crFore((COLORREF)-1),
	m_pOleFont(NULL),
	m_pSiteUIActive(NULL),
   m_pSiteCapture(NULL),
   m_pSiteFocus(NULL),
   m_nWindowlessControls(0)
{
}

COleControlContainer::~COleControlContainer()
{
	COleControlSiteOrWnd* pSiteOrWnd;
	POSITION pos;
	POSITION posFollow;

	pos = m_listSitesOrWnds.GetHeadPosition();
	while( pos != NULL )
	{
		posFollow = pos;
		pSiteOrWnd = m_listSitesOrWnds.GetNext( pos );
		ASSERT( pSiteOrWnd != NULL );
		if (pSiteOrWnd && pSiteOrWnd->m_pSite && 
			pSiteOrWnd->m_pSite->m_pDataSourceControl == NULL )
		{
			m_listSitesOrWnds.RemoveAt( posFollow );
			delete pSiteOrWnd;
		}
	}

	while( !m_listSitesOrWnds.IsEmpty() )
	{
		pSiteOrWnd = m_listSitesOrWnds.RemoveHead();
		delete pSiteOrWnd;
	}

	m_siteMap.RemoveAll();

	RELEASE(m_pOleFont);

	// Tell the GC to release references on 'this' object.

	// This check is necessary to make sure the spIUnknown->Release() does not
	// cause the same object to be deleted while it is already being deleted.
	if( m_dwRef > 0 )
	{
		HRESULT hr = S_OK;
		REFIID iid = IID_IUnknown;
		CComPtr<IUnknown> spIUnknown = NULL;

		hr = this->InternalQueryInterface(&iid, reinterpret_cast<void**>(&spIUnknown) );
		if( SUCCEEDED( hr ) )
		{
			_AfxReleaseManagedRefs( spIUnknown );
		}
	}
}

void COleControlContainer::BroadcastAmbientPropertyChange(DISPID dispid)
{
	POSITION pos;
	COleControlSiteOrWnd* pSiteOrWnd;
	IOleControl* pControl;

	pos = m_listSitesOrWnds.GetHeadPosition();
	while( pos != NULL )
	{
		pSiteOrWnd = m_listSitesOrWnds.GetNext( pos );
		ASSERT( pSiteOrWnd != NULL );
		if( pSiteOrWnd->m_pSite && pSiteOrWnd->m_pSite->m_pObject != NULL )
		{
			pControl = NULL;
			pSiteOrWnd->m_pSite->m_pObject->QueryInterface( IID_IOleControl, (void**)&pControl );
			if( pControl != NULL )
			{
				pControl->OnAmbientPropertyChange( dispid );
				pControl->Release();
			}
		}
	}
}

BOOL COleControlContainer::CreateControl( CWnd* pWndCtrl, REFCLSID clsid,
	LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, UINT nID,
	CFile* pPersist, BOOL bStorage, BSTR bstrLicKey,
	COleControlSite** ppNewSite )
{
   CRect rect2( rect );
   CPoint pt;
   CSize size;

   pt = rect2.TopLeft();
   size = rect2.Size();

   return( CreateControl( pWndCtrl, clsid, lpszWindowName, dwStyle, &pt, &size,
	  nID, pPersist, bStorage, bstrLicKey, ppNewSite ) );
}

BOOL COleControlContainer::CreateControl(CWnd* pWndCtrl,const CControlCreationInfo& creationInfo,
	DWORD dwStyle, const POINT* ppt, const SIZE* psize,UINT nID)
{
	return CreateControlCommon(pWndCtrl, creationInfo.m_clsid,creationInfo,NULL, 
								dwStyle, ppt, psize,nID, NULL, FALSE, NULL,
								NULL);
}

BOOL COleControlContainer::CreateControlCommon(CWnd* pWndCtrl, REFCLSID clsid,
	const CControlCreationInfo& creationInfo,
	LPCTSTR lpszWindowName, DWORD dwStyle, const POINT* ppt, const SIZE* psize,
   UINT nID, CFile* pPersist, BOOL bStorage, BSTR bstrLicKey,
   COleControlSite** ppNewSite)
{
	COleControlSite* pSite = NULL;
	COleControlSiteOrWnd *pSiteOrWnd = NULL;

	TRY
	{
	  BOOL bSuccess;
	  bSuccess = m_pWnd->CreateControlSite(this, &pSite, nID, clsid);
	  if (bSuccess && (pSite == NULL))
	  {
		 ENSURE(afxOccManager!=NULL);
		 // Use the default site.
		 pSite = afxOccManager->CreateSite(this,creationInfo);
	  }
	}
	END_TRY

	if (pSite == NULL)
		return FALSE;

	BOOL bCreated = FALSE;
	//If creation information, call that overload, else, call the CLSID overload
	if (creationInfo.IsManaged())
	{
		bCreated = SUCCEEDED( pSite->CreateControl(pWndCtrl, creationInfo,
			dwStyle, ppt, psize, nID) );
	} else
	{
		bCreated = SUCCEEDED( pSite->CreateControl(pWndCtrl, clsid,
		lpszWindowName, dwStyle, ppt, psize, nID, pPersist, bStorage, bstrLicKey ) );
	}

	if (bCreated)
	{
		//WinForms control on CDialog Visible=False --> The handle is not yet created.
		ASSERT( (pSite->m_hWnd != NULL) || pSite->m_bIsWindowless );
		pSiteOrWnd = new COleControlSiteOrWnd(pSite);
		m_listSitesOrWnds.AddTail(pSiteOrWnd);
		if( pSite->m_hWnd != NULL )
		{
		   m_siteMap.SetAt(pSite->m_hWnd, pSite);
		}
		if (ppNewSite != NULL)
			*ppNewSite = pSite;
	}
	else
	{
		delete pSite;
	}

	return bCreated;
}



BOOL COleControlContainer::CreateControl(CWnd* pWndCtrl, REFCLSID clsid,
	LPCTSTR lpszWindowName, DWORD dwStyle, const POINT* ppt, const SIZE* psize,
   UINT nID, CFile* pPersist, BOOL bStorage, BSTR bstrLicKey,
   COleControlSite** ppNewSite)
{
	CControlCreationInfo controlInfo;
	controlInfo.m_clsid=clsid;
	return CreateControlCommon(pWndCtrl, clsid,controlInfo,lpszWindowName, 
								dwStyle, ppt, psize,nID, pPersist, bStorage, bstrLicKey,
								ppNewSite);
}

COleControlSite* COleControlContainer::FindItem(UINT nID) const
{
	POSITION pos = m_listSitesOrWnds.GetHeadPosition();
	while (pos != NULL)
	{
		COleControlSiteOrWnd* pSiteOrWnd;

		pSiteOrWnd = m_listSitesOrWnds.GetNext( pos );
		ENSURE( pSiteOrWnd != NULL );
		if( pSiteOrWnd->m_pSite && pSiteOrWnd->m_pSite->GetDlgCtrlID() == int(nID) )
		{
			return( pSiteOrWnd->m_pSite );
		}
	}
	return NULL;
}

BOOL COleControlContainer::GetAmbientProp(COleControlSite* pSite, DISPID dispid,
	VARIANT* pvarResult)
{
	switch (dispid)
	{
	case DISPID_AMBIENT_AUTOCLIP:
	case DISPID_AMBIENT_MESSAGEREFLECT:
	case DISPID_AMBIENT_SUPPORTSMNEMONICS:
	case DISPID_AMBIENT_USERMODE:
		V_VT(pvarResult) = VT_BOOL;
		V_BOOL(pvarResult) = (VARIANT_BOOL)-1;
		return TRUE;

	case DISPID_AMBIENT_SHOWGRABHANDLES:
	case DISPID_AMBIENT_SHOWHATCHING:
	case DISPID_AMBIENT_UIDEAD:
		V_VT(pvarResult) = VT_BOOL;
		V_BOOL(pvarResult) = 0;
		return TRUE;

	case DISPID_AMBIENT_APPEARANCE:     // ambient appearance is 3D
		V_VT(pvarResult) = VT_I2;
		V_I2(pvarResult) = 1;
		return TRUE;

	case DISPID_AMBIENT_BACKCOLOR:
	case DISPID_AMBIENT_FORECOLOR:
		if (m_crBack == (COLORREF)-1)   // ambient colors not initialized
		{
			CWindowDC dc(m_pWnd);
			m_pWnd->SendMessage(WM_CTLCOLORSTATIC, (WPARAM)dc.m_hDC,
				(LPARAM)m_pWnd->m_hWnd);
			m_crBack = dc.GetBkColor();
			m_crFore = dc.GetTextColor();
		}

		V_VT(pvarResult) = VT_COLOR;
		V_I4(pvarResult) = (dispid == DISPID_AMBIENT_BACKCOLOR) ?
			m_crBack : m_crFore;
		return TRUE;

	case DISPID_AMBIENT_FONT:
		if (m_pOleFont == NULL)         // ambient font not initialized
			CreateOleFont(m_pWnd->GetFont());

		ASSERT(m_pOleFont != NULL);
		if (m_pOleFont == NULL)         // failed to create font
			return FALSE;

		V_VT(pvarResult) = VT_FONT;
		m_pOleFont->AddRef();
		V_DISPATCH(pvarResult) = m_pOleFont;
		return TRUE;

	case DISPID_AMBIENT_DISPLAYASDEFAULT:
		V_VT(pvarResult) = VT_BOOL;
		V_BOOL(pvarResult) = (VARIANT_BOOL)(pSite->IsDefaultButton() ? -1 : 0);
		return TRUE;

	case DISPID_AMBIENT_LOCALEID:
		V_VT(pvarResult) = VT_I4;
		V_I4(pvarResult) = GetThreadLocale();
		return TRUE;

	case DISPID_AMBIENT_DISPLAYNAME:
		{
			CString str;                // return blank string
			V_VT(pvarResult) = VT_BSTR;
			V_BSTR(pvarResult) = str.AllocSysString();
		}
		return TRUE;

	case DISPID_AMBIENT_SCALEUNITS:
		{
			CString str;
			str.LoadString(AFX_IDS_OCC_SCALEUNITS_PIXELS);
			V_VT(pvarResult) = VT_BSTR;
			V_BSTR(pvarResult) = str.AllocSysString();
		}
		return TRUE;
	}

	return FALSE;
}

void COleControlContainer::CreateOleFont(CFont* pFont)
{
	CFont fontSys;
	if ((pFont == NULL) || (pFont->m_hObject == NULL))
	{
		// no font was provided, so use the system font
		if (fontSys.CreateStockObject(DEFAULT_GUI_FONT) ||
			fontSys.CreateStockObject(SYSTEM_FONT))
		{
			pFont = &fontSys;
		}
		else
		{
			m_pOleFont = NULL;
			return;
		}
	}

	LOGFONT logfont;
	pFont->GetLogFont(&logfont);

	FONTDESC fd;
	fd.cbSizeofstruct = sizeof(FONTDESC);
	CStringW strFaceName(logfont.lfFaceName);
	fd.lpstrName = const_cast<LPOLESTR>(strFaceName.GetString());
	fd.sWeight = (short)logfont.lfWeight;
	fd.sCharset = logfont.lfCharSet;
	fd.fItalic = logfont.lfItalic;
	fd.fUnderline = logfont.lfUnderline;
	fd.fStrikethrough = logfont.lfStrikeOut;

	long lfHeight = logfont.lfHeight;
	if (lfHeight < 0)
		lfHeight = -lfHeight;

	CWindowDC dc(m_pWnd);
	int ppi = dc.GetDeviceCaps(LOGPIXELSY);
	fd.cySize.Lo = lfHeight * 720000 / ppi;
	fd.cySize.Hi = 0;

	RELEASE(m_pOleFont);

	if (FAILED(::OleCreateFontIndirect(&fd, IID_IFontDisp, (void**)&m_pOleFont)))
		m_pOleFont = NULL;
}

void COleControlContainer::FreezeAllEvents(BOOL bFreeze)
{
	COleControlSiteOrWnd* pSiteOrWnd;
	POSITION pos = m_listSitesOrWnds.GetHeadPosition();
	while (pos != NULL)
	{
		pSiteOrWnd = m_listSitesOrWnds.GetNext( pos );
		ASSERT(pSiteOrWnd);
		if(pSiteOrWnd->m_pSite)
			pSiteOrWnd->m_pSite->FreezeEvents(bFreeze);
	}
}

void COleControlContainer::ScrollChildren(int dx, int dy)
{
	COleControlSiteOrWnd* pSiteOrWnd;
	POSITION pos = m_listSitesOrWnds.GetHeadPosition();
	while (pos != NULL)
	{
		pSiteOrWnd = m_listSitesOrWnds.GetNext( pos );
		ASSERT(pSiteOrWnd);
		if(pSiteOrWnd->m_pSite)
		{
			ASSERT(pSiteOrWnd->m_pSite->m_pInPlaceObject != NULL);
			ASSERT(pSiteOrWnd->m_pSite->m_pObject != NULL);
			pSiteOrWnd->m_pSite->m_rect.OffsetRect(dx, dy);
			pSiteOrWnd->m_pSite->m_pInPlaceObject->SetObjectRects(
				pSiteOrWnd->m_pSite->m_rect, pSiteOrWnd->m_pSite->m_rect);
		}
	}
}

void COleControlContainer::OnUIActivate(COleControlSite* pSite)
{
	// deactivate currently active site if it isn't the site
	// making the activation request.
	if (m_pSiteUIActive != NULL && m_pSiteUIActive != pSite)
		m_pSiteUIActive->m_pInPlaceObject->UIDeactivate();

	m_pSiteUIActive = pSite;
}

void COleControlContainer::OnUIDeactivate(COleControlSite* pSite)
{
	UNUSED(pSite);

	if (m_pSiteUIActive == pSite)
	{
		m_pSiteUIActive = NULL;
	
	}
	if (m_pSiteFocus == pSite)
	{		
		m_pSiteFocus = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
// special cases for CWnd functions

void COleControlContainer::CheckDlgButton(int nIDButton, UINT nCheck)
{
	CWnd* pWnd = GetDlgItem(nIDButton);
	if (pWnd == NULL)
		return;

	if (pWnd->m_pCtrlSite == NULL)
	{
		pWnd->SendMessage(BM_SETCHECK, nCheck, 0);
		return;
	}

	pWnd->m_pCtrlSite->SafeSetProperty(DISPID_VALUE, VT_I4, (DWORD)nCheck);
}

void COleControlContainer::CheckRadioButton(int nIDFirstButton, int nIDLastButton,
	int nIDCheckButton)
{
	ASSERT(nIDFirstButton <= nIDCheckButton);
	ASSERT(nIDCheckButton <= nIDLastButton);

	// the following code is for OLE control containers only
	for (int nID = nIDFirstButton; nID <= nIDLastButton; nID++)
		CheckDlgButton(nID, (nID == nIDCheckButton));
}

CWnd* COleControlContainer::GetDlgItem(int nID) const
{
	HWND hWnd;
	GetDlgItem(nID, &hWnd);
	return CWnd::FromHandle(hWnd);
}

void COleControlContainer::GetDlgItem(int nID, HWND* phWnd) const
{
	// first, look for a non-OLE control
	HWND hWnd = ::GetDlgItem(m_pWnd->GetSafeHwnd(), nID);
	if (hWnd == NULL)
	{
		// now, look for an OLE control
		COleControlSite* pSite = FindItem(nID);
		if (pSite != NULL)
			hWnd = pSite->m_hWnd;
	}

	*phWnd = hWnd;
}

UINT COleControlContainer::GetDlgItemInt(int nID, BOOL* lpTrans, BOOL bSigned) const
{
	TCHAR szText[256];
	if (GetDlgItemText(nID, szText, 256) == 0)
	{
		if (lpTrans != NULL)
			*lpTrans = FALSE;
		return 0;
	}

	// Quick check for valid number
	LPTSTR pch = szText;

	while (_istspace(static_cast<_TUCHAR>(*pch)))
		pch = CharNext(pch);    // skip whitespace

	if ((*pch) == '+' || (*pch) == '-')
		pch = CharNext(pch);    // skip sign

	BOOL bTrans = _istdigit(static_cast<_TUCHAR>(*pch));    // did we find a digit?

	if (lpTrans != NULL)
		*lpTrans = bTrans;

	if (!bTrans)
		return 0;

	if (bSigned)
		return _tcstol(szText, NULL, 10);
	else
		return _tcstoul(szText, NULL, 10);
}

int COleControlContainer::GetDlgItemText(_In_ int nID, _Out_cap_post_count_(nMaxCount, return + 1) _Pre_notnull_ _Post_z_ LPTSTR lpStr, _In_ int nMaxCount) const
{
	CWnd* pWnd = GetDlgItem(nID);
	if (pWnd == NULL)
		return 0;

	return pWnd->GetWindowText(lpStr, nMaxCount);
}

LRESULT COleControlContainer::SendDlgItemMessage(int nID, UINT message, WPARAM wParam,
	LPARAM lParam)
{
	CWnd* pWnd = GetDlgItem(nID);
	if (pWnd == NULL)
		return 0;

	return pWnd->SendMessage(message, wParam, lParam);
}

void COleControlContainer::SetDlgItemInt(int nID, UINT nValue, BOOL bSigned)
{
	TCHAR szText[34];
	if (bSigned)
	{
		Checked::ltot_s((long)nValue, szText, 34, 10);
	}
	else
	{
		Checked::ultot_s((unsigned long)nValue, szText, 34, 10);
	}

	SetDlgItemText(nID, szText);
}

void COleControlContainer::SetDlgItemText(int nID, LPCTSTR lpszString)
{
	CWnd* pWnd = GetDlgItem(nID);
	if (pWnd == NULL)
		return;

	pWnd->SetWindowText(lpszString);
}

UINT COleControlContainer::IsDlgButtonChecked(int nIDButton) const
{
	CWnd* pWnd = GetDlgItem(nIDButton);
	if (pWnd == NULL)
		return 0;

	if (pWnd->m_pCtrlSite == NULL)
	  //IA64: Assume retval of BM_GETCHECK is still 32-bit
		return UINT(pWnd->SendMessage(BM_GETCHECK, 0, 0));

	DWORD dwValue;

	TRY
	{
		pWnd->GetProperty(DISPID_VALUE, VT_I4, &dwValue);
	}
	CATCH_ALL(e)
	{
		DELETE_EXCEPTION(e);
		dwValue = 0;
	}
	END_CATCH_ALL

	if (dwValue == 0x0000ffff)  // VARIANT_BOOL TRUE
		dwValue = 1;

	return dwValue;
}


BOOL COleControlContainer::HandleSetFocus()
{
   if( m_pSiteFocus != NULL )
   {
	  return( TRUE );
   }
   else
   {
	  return( FALSE );
   }
}

BOOL COleControlContainer::HandleWindowlessMessage(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* plResult)
{
   HRESULT hResult;
   POSITION pos;
   COleControlSiteOrWnd* pSiteOrWnd;
   COleControlSite* pMouseMessageSite;

   ASSERT( plResult != NULL );
   *plResult = 0;

   switch (message)
   {
   case WM_MOUSEMOVE:
   case WM_LBUTTONDOWN:
   case WM_LBUTTONUP:
   case WM_LBUTTONDBLCLK:
   case WM_RBUTTONDOWN:
   case WM_RBUTTONUP:
   case WM_RBUTTONDBLCLK:
   case WM_MBUTTONDOWN:
   case WM_MBUTTONUP:
   case WM_MBUTTONDBLCLK:
	  if (m_pSiteCapture != NULL)
	  {
		 ASSERT( m_pSiteCapture->m_pWindowlessObject != NULL );
		 pMouseMessageSite = m_pSiteCapture;
	  }
	  else
	  {
		 pMouseMessageSite = NULL;
		 pos = m_listSitesOrWnds.GetHeadPosition();
		 while ((pos != NULL) && (pMouseMessageSite == NULL))
		 {
			pSiteOrWnd = m_listSitesOrWnds.GetNext(pos);
			ASSERT(pSiteOrWnd);
			if (pSiteOrWnd->m_pSite && pSiteOrWnd->m_pSite->m_bIsWindowless)
			{
			   ASSERT( pSiteOrWnd->m_pSite->m_pWindowlessObject != NULL );

			   CPoint point((DWORD)lParam);

			   if (pSiteOrWnd->m_pSite->m_rect.PtInRect(point))
			   {
				  pMouseMessageSite = pSiteOrWnd->m_pSite;
				  break;
			   }
			}
		 }
	  }
	  if (pMouseMessageSite != NULL)
	  {
		 hResult = pMouseMessageSite->m_pWindowlessObject->OnWindowMessage(
			message, wParam, lParam, plResult);
		 return hResult == S_OK;
	  }
	  break;

   case WM_KEYDOWN:
   case WM_KEYUP:
   case WM_CHAR:
   case WM_DEADCHAR:
   case WM_SYSKEYDOWN:
   case WM_SYSKEYUP:
   case WM_SYSDEADCHAR:
   case WM_HELP:
   case WM_CANCELMODE:
   case WM_IME_STARTCOMPOSITION:
   case WM_IME_ENDCOMPOSITION:
   case WM_IME_COMPOSITION:
   case WM_IME_SETCONTEXT:
   case WM_IME_NOTIFY:
   case WM_IME_CONTROL:
   case WM_IME_COMPOSITIONFULL:
   case WM_IME_SELECT:
   case WM_IME_CHAR:
//   case WM_IME_REQUEST:
   case WM_IME_KEYDOWN:
   case WM_IME_KEYUP:
	  //Call OnWindowMessage in case a Windowless ActiveX is in focus.
	  //m_pSiteFocus contains the current ActiveX site in focus (not necessarily windowless).
	  if (m_pSiteFocus != NULL && m_pSiteFocus->m_pWindowlessObject != NULL)
	  {
		 
		 hResult = m_pSiteFocus->m_pWindowlessObject->OnWindowMessage(
			message, wParam, lParam, plResult);
		 if (hResult == S_OK)
		 {
			return TRUE;
		 }
		 else
		 {
			return FALSE;
		 }
	  }
	  break;
   }

   return FALSE;
}

BOOL COleControlContainer::OnPaint( CDC* pDC )
{
   POSITION pos;
   COleControlSiteOrWnd* pSiteOrWnd;
   IViewObject* pViewObject;

   pos = m_listSitesOrWnds.GetHeadPosition();
   HRESULT hr = E_NOTIMPL;
   BOOL bRet = FALSE;

   while( pos != NULL )
   {
	  pSiteOrWnd = m_listSitesOrWnds.GetNext( pos );
	  if( pSiteOrWnd->m_pSite && pSiteOrWnd->m_pSite->m_bIsWindowless )
	  {
		 pViewObject = NULL;
		 pSiteOrWnd->m_pSite->m_pObject->QueryInterface( IID_IViewObject, (void**)&pViewObject );
		 if( pViewObject != NULL )
		 {
			hr = pViewObject->Draw( DVASPECT_CONTENT, -1, NULL, NULL, NULL, *pDC, NULL, NULL, NULL, 0 );
			if (SUCCEEDED(hr))
            {
				bRet = TRUE;
            }
            pViewObject->Release();
            pViewObject = NULL;
		 }
	  }
   }

   return bRet;
}

//Searches a child window with nID in z-order list, starting from hwndStart.
AFX_STATIC HWND AfxGetDlgItemStartFromHWND(int nID,HWND hwndStart)
{	
	//Search start from one after hwndStart (As opposed to 
	//::GetDlgItem, which always start at the first child).
	HWND hwndFound = hwndStart;
	while (hwndFound!=NULL)
	{
		if (GetDlgCtrlID(hwndFound) == nID)
		{
			break;
		}
		hwndFound=GetWindow(hwndFound,GW_HWNDNEXT);
	}		
	return hwndFound;
}

BOOL COleControlContainer::FillListSitesOrWnds(_AFX_OCC_DIALOG_INFO* pOccDlgInfo)
{
	if (pOccDlgInfo==NULL) { return FALSE; }
	COleControlSiteOrWnd *pSiteOrWnd = NULL;
	//When this function is called, the m_listSitesOrWnds contains only ActiveX controls
	//created by MFC from resource, or by user who called CreateControl.
	//The code inserts the Win32 controls (found in dialog template resource) into
	//the list, before, in between or after the ActiveX controls, according to the 
	//order in the dialog template.
	POSITION pos = m_listSitesOrWnds.GetHeadPosition();
	POSITION posOld = pos;
	if(pos)
	{
		pSiteOrWnd = m_listSitesOrWnds.GetNext(pos);
	}

	HWND hwndStart=NULL;
	for(unsigned i = 0; i < pOccDlgInfo->m_cItems; i++)
	{
		ASSERT(!pSiteOrWnd || pSiteOrWnd->m_pSite); //pSiteOrWnd must point at either ActiveX or NULL (list end).
		if(pSiteOrWnd!=NULL && pSiteOrWnd->m_pSite!=NULL &&
			pSiteOrWnd->m_pSite->m_nID == pOccDlgInfo->m_pItemInfo[i].nId)
		{
			//If not Windowless control, update the starting position in z-order search.
			if (pSiteOrWnd->m_pSite->m_hWnd != NULL)
			{
				hwndStart = pSiteOrWnd->m_pSite->m_hWnd;
			}
			posOld = pos;
			pSiteOrWnd = pos ? m_listSitesOrWnds.GetNext(pos) :	NULL;
		}
		else
		{
			if(pOccDlgInfo->m_pItemInfo[i].nId)
			{
				//AfxGetDlgItemStartFromHWND fix , is to handle non-unique ids 
				//of resource items (IDC_STATIC), that causes ::GetDlgItem
				//to always find the first (in Win32 z-order child list) window with
				//this id, and thus m_listSitesOrWnds contained several instances of the same hwnd.
				
				//First time start searching from first z-order, else
				//search from 1 after the previous window.
				HWND hwndSearchFrom= (hwndStart == NULL) ? 
									 GetWindow(m_pWnd->GetSafeHwnd(),GW_CHILD) :
									 ::GetWindow(hwndStart,GW_HWNDNEXT);
				
				HWND hwndCtrl=AfxGetDlgItemStartFromHWND(pOccDlgInfo->m_pItemInfo[i].nId, hwndSearchFrom);
				//If not found, revert to prev method of GetDlgItem, this means Win32 children list and 
				//resource item array are out of sync
				if (hwndCtrl == NULL)
				{
					hwndCtrl = ::GetDlgItem(m_pWnd->GetSafeHwnd(),pOccDlgInfo->m_pItemInfo[i].nId);
					TRACE(traceAppMsg, 0, "Warning: Resource items and Win32 Z-order lists are out of sync. Tab order may be not defined well.\n");
				}
				COleControlSiteOrWnd *pTemp =
					new COleControlSiteOrWnd(
						hwndCtrl,
						pOccDlgInfo->m_pItemInfo[i].bAutoRadioButton);
				ASSERT(IsWindow(pTemp->m_hWnd));
				if (IsWindow(pTemp->m_hWnd))
				{
					hwndStart = pTemp->m_hWnd;
					// These transfer inserts will ensure pTemp is freed if insertion fails
					if(posOld)
					{
						m_listSitesOrWnds.TransferInsertBefore(posOld, pTemp);
					}
					else
					{
						m_listSitesOrWnds.TransferAddTail(pTemp);
					}
				}
			}
		}
	}
	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
// COleControlContainer::XOleIPFrame

STDMETHODIMP COleControlContainer::XOleIPFrame::QueryInterface(
	REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE_EX_(COleControlContainer, OleIPFrame)
	return (HRESULT)pThis->InternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP_(ULONG) COleControlContainer::XOleIPFrame::AddRef()
{
	METHOD_PROLOGUE_EX_(COleControlContainer, OleIPFrame)
	return (ULONG)pThis->InternalAddRef();
}

STDMETHODIMP_(ULONG) COleControlContainer::XOleIPFrame::Release()
{
	METHOD_PROLOGUE_EX_(COleControlContainer, OleIPFrame)
	return (ULONG)pThis->InternalRelease();
}

STDMETHODIMP COleControlContainer::XOleIPFrame::GetWindow(HWND* phWnd)
{
	METHOD_PROLOGUE_EX_(COleControlContainer, OleIPFrame)

	*phWnd = pThis->m_pWnd->m_hWnd;
	return S_OK;
}

STDMETHODIMP COleControlContainer::XOleIPFrame::ContextSensitiveHelp(BOOL)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlContainer::XOleIPFrame::GetBorder(LPRECT)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlContainer::XOleIPFrame::RequestBorderSpace(
	LPCBORDERWIDTHS)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlContainer::XOleIPFrame::SetBorderSpace(
	LPCBORDERWIDTHS)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlContainer::XOleIPFrame::SetActiveObject(
	LPOLEINPLACEACTIVEOBJECT pActiveObject, LPCOLESTR)
{
	METHOD_PROLOGUE_EX_(COleControlContainer, OleIPFrame)

	if (pThis->m_pSiteUIActive != NULL)
	{
		LPOLEINPLACEACTIVEOBJECT pOldActiveObject = pThis->m_pSiteUIActive->m_pActiveObject;
		if (pActiveObject != NULL)
			pActiveObject->AddRef();
		pThis->m_pSiteUIActive->m_pActiveObject = pActiveObject;
		if (pOldActiveObject != NULL)
			pOldActiveObject->Release();
	}
	return S_OK;
}

STDMETHODIMP COleControlContainer::XOleIPFrame::InsertMenus(HMENU,
	LPOLEMENUGROUPWIDTHS)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlContainer::XOleIPFrame::SetMenu(HMENU, HOLEMENU, HWND)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlContainer::XOleIPFrame::RemoveMenus(HMENU)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlContainer::XOleIPFrame::SetStatusText(LPCOLESTR)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlContainer::XOleIPFrame::EnableModeless(BOOL)
{
   // As long as we don't create any modeless dialogs, we can just return S_OK.
	return S_OK;
}

STDMETHODIMP COleControlContainer::XOleIPFrame::TranslateAccelerator(LPMSG,
	WORD)
{
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////
// CEnumUnknown - enumerator for IUnknown pointers

class CEnumUnknown : public CEnumArray
{
public:
	CEnumUnknown(const void* pvEnum, UINT nSize) :
		CEnumArray(sizeof(LPUNKNOWN), pvEnum, nSize, TRUE) {}
	~CEnumUnknown();

protected:
	virtual BOOL OnNext(void* pv);

	DECLARE_INTERFACE_MAP()
};

BEGIN_INTERFACE_MAP(CEnumUnknown, CEnumArray)
	INTERFACE_PART(CEnumUnknown, IID_IEnumUnknown, EnumVOID)
END_INTERFACE_MAP()

CEnumUnknown::~CEnumUnknown()
{
	if (m_pClonedFrom == NULL)
	{
		LPUNKNOWN* ppUnk = (LPUNKNOWN*)(void*)m_pvEnum;
		for (UINT i = 0; i < m_nSize; i++)
		{
			ASSERT(ppUnk[i] != NULL);
			ppUnk[i]->Release();
		}
	}
	// destructor will free the actual array (if it was not a clone)
}

BOOL CEnumUnknown::OnNext(void* pv)
{
	if (!CEnumArray::OnNext(pv))
		return FALSE;

	// AddRef the pointer (the caller has responsibility to Release it)
	ASSERT(*(LPUNKNOWN*)pv != NULL);
	(*(LPUNKNOWN*)pv)->AddRef();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// COleControlContainer::XOleContainer

STDMETHODIMP COleControlContainer::XOleContainer::QueryInterface(
	REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE_EX_(COleControlContainer, OleContainer)
	return (HRESULT)pThis->InternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP_(ULONG) COleControlContainer::XOleContainer::Release()
{
	METHOD_PROLOGUE_EX_(COleControlContainer, OleContainer)
	return (ULONG)pThis->InternalRelease();
}

STDMETHODIMP_(ULONG) COleControlContainer::XOleContainer::AddRef()
{
	METHOD_PROLOGUE_EX_(COleControlContainer, OleContainer)
	return (ULONG)pThis->InternalAddRef();
}

STDMETHODIMP COleControlContainer::XOleContainer::ParseDisplayName(LPBINDCTX,
	LPOLESTR, ULONG*, LPMONIKER*)
{
	return E_NOTIMPL;
}

STDMETHODIMP COleControlContainer::XOleContainer::EnumObjects(DWORD dwFlags,
	LPENUMUNKNOWN* ppEnumUnknown)
{
	METHOD_PROLOGUE_EX_(COleControlContainer, OleContainer)

	*ppEnumUnknown = NULL;
	HRESULT hr = S_OK;
	CEnumUnknown* pEnum = NULL;
	UINT_PTR cObjects = 0;
	LPUNKNOWN* ppUnk = NULL;

	TRY
	{
		if (dwFlags & OLECONTF_EMBEDDINGS)
		{
			ppUnk = new LPUNKNOWN[pThis->m_listSitesOrWnds.GetCount()];
			POSITION pos = pThis->m_listSitesOrWnds.GetHeadPosition();
			COleControlSiteOrWnd* pSiteOrWnd;
			while (pos != NULL)
			{
				pSiteOrWnd = pThis->m_listSitesOrWnds.GetNext( pos );
				ASSERT(pSiteOrWnd);
				if(pSiteOrWnd->m_pSite)
				{
					ASSERT(pSiteOrWnd->m_pSite->m_pObject != NULL);
					pSiteOrWnd->m_pSite->m_pObject->AddRef();
					ppUnk[cObjects++] = pSiteOrWnd->m_pSite->m_pObject;
				}
			}
		}
		pEnum = new CEnumUnknown(ppUnk, (UINT)cObjects);
	}
	CATCH_ALL(e)
	{
		// Note: DELETE_EXCEPTION(e) not necessary
		hr = E_OUTOFMEMORY;
	}
	END_CATCH_ALL

	// clean up in case of failure
	if (SUCCEEDED(hr))
	{
		ASSERT(pEnum != NULL);
		*ppEnumUnknown = (IEnumUnknown*)&pEnum->m_xEnumVOID;
	}
	else
	{
		ASSERT(pEnum == NULL);
		ASSERT(*ppEnumUnknown == NULL);

		if (ppUnk != NULL)
		{
			for (UINT_PTR i = 0; i < cObjects; i++)
			{
				ASSERT(ppUnk[i] != NULL);
				ppUnk[i]->Release();
			}
		}
	}

	return hr;
}

STDMETHODIMP COleControlContainer::XOleContainer::LockContainer(BOOL)
{
	return E_NOTIMPL;
}
//////////////////////////////////////
//CControlCreationInfo
CControlCreationInfo::CControlCreationInfo()
{
	m_hk = NullHandle;
	m_nHandle = 0;
}
BOOL CControlCreationInfo::IsManaged() const
{
	ASSERT((m_hk == NullHandle && m_nHandle == 0) ||(m_hk != NullHandle && m_nHandle != 0));
	return m_hk != NullHandle;
}
