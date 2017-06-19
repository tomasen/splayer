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

namespace Microsoft {
	namespace VisualC {
			namespace MFC {

//////////////////////////////////////////////////////////////////////////
//CWinFormsControlSite
//

HRESULT CWinFormsControlSite::DoVerb(LONG nVerb, LPMSG lpMsg)
{
	HRESULT hr=S_OK;
	switch(nVerb)
	{
		case OLEIVERB_SHOW:
			get_Control()->Visible = true;
			break;
		case OLEIVERB_HIDE:
			get_Control()->Visible = false;
			break;
		default:
			hr=__super::DoVerb(nVerb, lpMsg);
	} //End switch
	return hr;
}

HRESULT CWinFormsControlSite::CreateOrLoad(const CControlCreationInfo& creationInfo)
{
	typedef System::Runtime::InteropServices::GCHandle GCHandle;
	HRESULT hr=E_FAIL;
	//Create an instance of the managed object and set the m_gcEventHelper
	//to reference it and hook HWND related events.
	ASSERT(creationInfo.IsManaged());
	System::Windows::Forms::Control^ pControl=nullptr;
	if (creationInfo.m_hk == CControlCreationInfo::ReflectionType)
	{
		System::Type^ pType=safe_cast<System::Type^>( (GCHandle::operator GCHandle(System::IntPtr(creationInfo.m_nHandle))).Target );
		System::Object^ pObj = System::Activator::CreateInstance(pType);		
		pControl=safe_cast<System::Windows::Forms::Control^>(pObj);
		
	} else if (creationInfo.m_hk == CControlCreationInfo::ControlInstance)
	{
			pControl=safe_cast<System::Windows::Forms::Control^>( (GCHandle::operator GCHandle(System::IntPtr(creationInfo.m_nHandle))).Target );
	}

	if (pControl!=nullptr)
	{
		m_gcEventHelper->Control::set( pControl );
		//Marshal the control into IUnknown for usage of MFC Native ActiveX code.
		System::IntPtr pUnknAsInt = System::Runtime::InteropServices::Marshal::GetIUnknownForObject(get_Control());
		IUnknown* pUnkn = static_cast<IUnknown*>(pUnknAsInt.ToPointer());
		if (pUnkn!=NULL)
		{
			hr=pUnkn->QueryInterface(IID_IOleObject, (void**)&m_pObject);
			System::Runtime::InteropServices::Marshal::Release(pUnknAsInt);
		}
		if (SUCCEEDED(hr))
		{   //Now that m_pObject is set, call the original CreateOrLoad method.
			hr=__super::CreateOrLoad(GUID_NULL, NULL,FALSE, NULL);
		}
	}
	return hr;		
}


HRESULT CWinFormsControlSite::CreateControlCommon(CWnd* pWndCtrl, REFCLSID clsid,const CControlCreationInfo& creationInfo,
		LPCTSTR lpszWindowName, DWORD dwStyle, const POINT* ppt, const SIZE* psize, UINT nID,
		CFile* pPersist, BOOL bStorage, BSTR bstrLicKey)
{
	HRESULT hr=COleControlSite::CreateControlCommon(pWndCtrl, clsid,creationInfo,
								  				lpszWindowName, dwStyle, ppt, 
												psize, nID, pPersist, bStorage, bstrLicKey);
	if (SUCCEEDED(hr))
	{
		get_Control()->TabStop = m_dwStyle & WS_TABSTOP ? true : false;	
	}
	return hr;
}

void CWinFormsControlSite::GetProperty(DISPID dwDispID, VARTYPE vtProp, void* pvProp) const
{
	switch(dwDispID)
	{
	case DISPID_ENABLED:
		ENSURE_ARG(vtProp==VT_BOOL);
		ENSURE_ARG(pvProp!=NULL);
		*(bool*)pvProp = get_Control()->Enabled;
		break;		
	default:
		__super::GetProperty(dwDispID, vtProp, pvProp);
	}		
}

#pragma warning( push )
#pragma warning( disable : 4793 )

void CWinFormsControlSite::SetPropertyV(DISPID dwDispID, VARTYPE vtProp, va_list argList)
{
	switch(dwDispID)
	{
		case DISPID_ENABLED:
		{
			ENSURE(vtProp==VT_BOOL);
			BOOL bEnable=va_arg(argList, BOOL);
			SetControlEnabled(bEnable ? true : false);
			break;		
		}
	default:
		__super::SetPropertyV(dwDispID, vtProp, argList);

	}		
}

#pragma warning( pop )

DWORD CWinFormsControlSite::GetStyle() const
{
	DWORD dwStyle = __super::GetStyle();		
	dwStyle = get_Control()->Visible ? dwStyle | WS_VISIBLE : dwStyle & ~WS_VISIBLE;
	dwStyle = get_Control()->TabStop ? dwStyle | WS_TABSTOP : dwStyle & ~WS_TABSTOP;
	return dwStyle;
}

void CWinFormsControlSite::OnHandleCreatedHandler()
{
		AttachWindow();			
	//Fix Z-order after WinForms ReCreate a control (Ex: Button style changed).
	//First find current site in the list, and then
	//iterate backward, until a valid hWnd is found. Insert the recreated control
	//in the correct Win32 z-order pos - after the found hWnd.
	COleControlSiteOrWnd *pSiteOrWnd = NULL,*pPrevZorderSiteWnd=NULL;		
	ENSURE(m_pCtrlCont != NULL);
	POSITION currentPos = NULL;
	POSITION pos = m_pCtrlCont->m_listSitesOrWnds.GetHeadPosition();
	BOOL bFoundSelf=FALSE;
	while(pos)
	{
		currentPos = pos;
		pSiteOrWnd = m_pCtrlCont->m_listSitesOrWnds.GetNext(pos);
		if (pSiteOrWnd && pSiteOrWnd->m_pSite == this)
		{
			bFoundSelf = TRUE;
			break;
		}

	}//End while(pos)

	//Move backward to find valid hWnd to insert after in Z-order.
	if (bFoundSelf)
	{
		m_pCtrlCont->m_listSitesOrWnds.GetPrev(currentPos);

		HWND hWndBeforeInOrder = NULL;				
		while(currentPos)
		{
			pPrevZorderSiteWnd = m_pCtrlCont->m_listSitesOrWnds.GetPrev(currentPos);
			if (pPrevZorderSiteWnd)
			{ 							
				if (pPrevZorderSiteWnd->m_hWnd!=NULL)
				{
					hWndBeforeInOrder = pPrevZorderSiteWnd->m_hWnd;
				} else if (pPrevZorderSiteWnd->m_pSite && pPrevZorderSiteWnd->m_pSite->m_hWnd!=NULL)
				{
					hWndBeforeInOrder=pPrevZorderSiteWnd->m_pSite->m_hWnd;
				}

				if (hWndBeforeInOrder!=NULL)
				{								
					break;
				}
			}
		}//End while(currentPos)

		if (hWndBeforeInOrder == NULL)
		{
			//If first on z-order, there is no valid hWnd in the m_listSitesOrWnds before
			//this control.
			hWndBeforeInOrder = HWND_TOP;
		}
		BOOL ok=::SetWindowPos(m_hWnd, hWndBeforeInOrder, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		ASSERT(ok);

	} //End if (bFoundSelf)
}

void CWinFormsControlSite::OnHandleCreated( gcroot<System::Object^> , gcroot<System::EventArgs^> )
{					
	OnHandleCreatedHandler();
}


class CWinFormsControlSiteFactory : public IControlSiteFactory
{
public:
	CWinFormsControlSiteFactory() { }
	COleControlSite* CreateSite(COleControlContainer* pCtrlCont,const CControlCreationInfo& creationInfo)
	{
		COleControlSite* pSite=NULL;
		if (InlineIsEqualGUID(creationInfo.m_clsid , CLSID_WinFormsControl))
		{		
			pSite=new CWinFormsControlSite(pCtrlCont);
		}
		return pSite;
	}	

};


struct CRegisterWinFormsFactory
{
	IControlSiteFactory* m_pFactory;
	CRegisterWinFormsFactory()
	{				
		m_pFactory=new CWinFormsControlSiteFactory();
		AfxRegisterSiteFactory(m_pFactory);
	}
	~CRegisterWinFormsFactory()
	{		
		delete m_pFactory;
	}
};

//Register our WinForms control site factory
extern "C" {  CRegisterWinFormsFactory g_registerWinFormsFactory; }

#if defined(_M_IX86)
#pragma comment(linker, "/INCLUDE:_g_registerWinFormsFactory")
#elif defined(_M_IA64) || defined(_M_AMD64)
#pragma comment(linker, "/INCLUDE:g_registerWinFormsFactory")
#else
#pragma message("Unknown platform.  Make sure the linker includes g_registerWinFormsFactory")
#endif





		} //MFC
	} //VisualC
} //Microsoft
