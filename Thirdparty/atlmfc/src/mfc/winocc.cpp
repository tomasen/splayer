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

#ifndef _AFX_NO_OCC_SUPPORT

/////////////////////////////////////////////////////////////////////////////
// CWnd overridable for ambient properties

BOOL CWnd::OnAmbientProperty(COleControlSite* pSite, DISPID dispid,
	VARIANT* pvar)
{
	ASSERT(m_pCtrlCont != NULL);
	return m_pCtrlCont->GetAmbientProp(pSite, dispid, pvar);
}

/////////////////////////////////////////////////////////////////////////////
// CWnd access to underlying OLE control interface

LPUNKNOWN CWnd::GetControlUnknown()
{
	if (m_pCtrlSite == NULL)
		return NULL;

	return m_pCtrlSite->m_pObject;
}

/////////////////////////////////////////////////////////////////////////////
// CWnd functions with special cases for OLE Control containment

BOOL CWnd::PaintWindowlessControls(CDC *pDC)
{
   if (m_pCtrlCont != NULL)
   {
	  // Paint windowless controls
	  return m_pCtrlCont->OnPaint(pDC);
   }
   return FALSE;
}

COleControlSite* CWnd::GetOleControlSite(UINT idControl) const
{
   if (m_pCtrlCont != NULL )
   {
	  return( m_pCtrlCont->FindItem(idControl) );
   }
   else
   {
	  return( NULL );
   }
}

void CWnd::CheckDlgButton(int nIDButton, UINT nCheck)
{
	ASSERT(::IsWindow(m_hWnd));

	if (m_pCtrlCont == NULL)
		::CheckDlgButton(m_hWnd, nIDButton, nCheck);
	else
		m_pCtrlCont->CheckDlgButton(nIDButton, nCheck);
}

void CWnd::CheckRadioButton(int nIDFirstButton, int nIDLastButton,
	int nIDCheckButton)
{
	ASSERT(::IsWindow(m_hWnd));

	if (m_pCtrlCont == NULL)
		::CheckRadioButton(m_hWnd, nIDFirstButton, nIDLastButton,
			nIDCheckButton);
	else
		m_pCtrlCont->CheckRadioButton(nIDFirstButton, nIDLastButton,
			nIDCheckButton);
}

CWnd* CWnd::GetDlgItem(int nID) const
{
	ASSERT(::IsWindow(m_hWnd));

	if (m_pCtrlCont == NULL)
		return CWnd::FromHandle(::GetDlgItem(m_hWnd, nID));
	else
		return m_pCtrlCont->GetDlgItem(nID);
}

void CWnd::GetDlgItem(int nID, HWND* phWnd) const
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT(phWnd != NULL);

	if (m_pCtrlCont == NULL)
		*phWnd = ::GetDlgItem(m_hWnd, nID);
	else
		m_pCtrlCont->GetDlgItem(nID, phWnd);
}

UINT CWnd::GetDlgItemInt(int nID, BOOL* lpTrans, BOOL bSigned) const
{
	ASSERT(::IsWindow(m_hWnd));

	if (m_pCtrlCont == NULL)
		return ::GetDlgItemInt(m_hWnd, nID, lpTrans, bSigned);
	else
		return m_pCtrlCont->GetDlgItemInt(nID, lpTrans, bSigned);
}

int CWnd::GetDlgItemText(_In_ int nID, _Out_z_cap_post_count_(nMaxCount, return + 1) LPTSTR lpStr, _In_ int nMaxCount) const
{
	ASSERT(::IsWindow(m_hWnd));

	if (m_pCtrlCont == NULL)
		return ::GetDlgItemText(m_hWnd, nID, lpStr, nMaxCount);
	else
		return m_pCtrlCont->GetDlgItemText(nID, lpStr, nMaxCount);
}

LRESULT CWnd::SendDlgItemMessage(int nID, UINT message, WPARAM wParam,
	LPARAM lParam)
{
	ASSERT(::IsWindow(m_hWnd));

	if (m_pCtrlCont == NULL)
		return ::SendDlgItemMessage(m_hWnd, nID, message, wParam, lParam);
	else
		return m_pCtrlCont->SendDlgItemMessage(nID, message, wParam, lParam);
}

void CWnd::SetDlgItemInt(int nID, UINT nValue, BOOL bSigned)
{
	ASSERT(::IsWindow(m_hWnd));

	if (m_pCtrlCont == NULL)
		::SetDlgItemInt(m_hWnd, nID, nValue, bSigned);
	else
		m_pCtrlCont->SetDlgItemInt(nID, nValue, bSigned);
}

void CWnd::SetDlgItemText(int nID, LPCTSTR lpszString)
{
	ASSERT(::IsWindow(m_hWnd));

	if (m_pCtrlCont == NULL)
		::SetDlgItemText(m_hWnd, nID, lpszString);
	else
		m_pCtrlCont->SetDlgItemText(nID, lpszString);
}

UINT CWnd::IsDlgButtonChecked(int nIDButton) const
{
	ASSERT(::IsWindow(m_hWnd));

	if (m_pCtrlCont == NULL)
		return ::IsDlgButtonChecked(m_hWnd, nIDButton);
	else
		return m_pCtrlCont->IsDlgButtonChecked(nIDButton);
}

int CWnd::ScrollWindowEx(int dx, int dy, LPCRECT lpRectScroll,
	LPCRECT lpRectClip, CRgn* prgnUpdate, LPRECT lpRectUpdate,
	UINT flags)
{
	ASSERT(::IsWindow(m_hWnd));

	int iReturn = ::ScrollWindowEx(m_hWnd, dx, dy, lpRectScroll, lpRectClip,
			(HRGN)prgnUpdate->GetSafeHandle(), lpRectUpdate, flags);

	if ((m_pCtrlCont == NULL) || !(flags & SW_SCROLLCHILDREN))
		return iReturn;

	// the following code is for OLE control containers only

	m_pCtrlCont->ScrollChildren(dx, dy);
	return iReturn;
}

BOOL CWnd::IsDialogMessage(LPMSG lpMsg)
{
	ASSERT(::IsWindow(m_hWnd));

	if (m_nFlags & WF_OLECTLCONTAINER)
		return afxOccManager->IsDialogMessage(this, lpMsg);
	else
		return ::IsDialogMessage(m_hWnd, lpMsg);
}

/////////////////////////////////////////////////////////////////////////////
// CWnd functions with special cases for OLE Control wrappers

DWORD CWnd::GetStyle() const
{
	ASSERT(::IsWindow(m_hWnd) || (m_pCtrlSite != NULL));

	if (m_pCtrlSite == NULL)
		return (DWORD)GetWindowLong(m_hWnd, GWL_STYLE);
	else
		return m_pCtrlSite->GetStyle();
}

DWORD CWnd::GetExStyle() const
{
	ASSERT(::IsWindow(m_hWnd) || (m_pCtrlSite != NULL));

	if (m_pCtrlSite == NULL)
		return (DWORD)GetWindowLong(m_hWnd, GWL_EXSTYLE);
	else
		return m_pCtrlSite->GetExStyle();
}

BOOL CWnd::ModifyStyle(DWORD dwRemove, DWORD dwAdd, UINT nFlags)
{
	ASSERT(::IsWindow(m_hWnd) || (m_pCtrlSite != NULL));

	if (m_pCtrlSite == NULL)
		return ModifyStyle(m_hWnd, dwRemove, dwAdd, nFlags);
	else
		return m_pCtrlSite->ModifyStyle(dwRemove, dwAdd, nFlags);
}

BOOL CWnd::ModifyStyleEx(DWORD dwRemove, DWORD dwAdd, UINT nFlags)
{
	ASSERT(::IsWindow(m_hWnd) || (m_pCtrlSite != NULL));

	if (m_pCtrlSite == NULL)
		return ModifyStyleEx(m_hWnd, dwRemove, dwAdd, nFlags);
	else
		return m_pCtrlSite->ModifyStyleEx(dwRemove, dwAdd, nFlags);
}

void CWnd::SetWindowText(LPCTSTR lpszString)
{
	ENSURE(this);
	ENSURE(::IsWindow(m_hWnd) || (m_pCtrlSite != NULL));

	if (m_pCtrlSite == NULL)
		::SetWindowText(m_hWnd, lpszString);
	else
		m_pCtrlSite->SetWindowText(lpszString);
}

int CWnd::GetWindowText(_Out_z_cap_post_count_(nMaxCount, return + 1) LPTSTR lpszString, _In_ int nMaxCount) const
{
	ASSERT(::IsWindow(m_hWnd) || (m_pCtrlSite != NULL));

	if (m_pCtrlSite == NULL)
		return ::GetWindowText(m_hWnd, lpszString, nMaxCount);
	else
	{
		CString str;

		m_pCtrlSite->GetWindowText(str);
		Checked::tcsncpy_s(lpszString, nMaxCount, str, _TRUNCATE);
		return lstrlen(lpszString);
	}
}

int CWnd::GetWindowTextLength() const
{
	ASSERT(::IsWindow(m_hWnd) || (m_pCtrlSite != NULL));

	if (m_pCtrlSite == NULL)
		return ::GetWindowTextLength(m_hWnd);
	else
	{
		CString str;

		m_pCtrlSite->GetWindowText(str);
		return (int)str.GetLength();
	}
}

int CWnd::GetDlgCtrlID() const
{
	ASSERT(::IsWindow(m_hWnd) || (m_pCtrlSite != NULL));

	if (m_pCtrlSite == NULL)
		return ::GetDlgCtrlID(m_hWnd);
	else
		return m_pCtrlSite->GetDlgCtrlID();
}

int CWnd::SetDlgCtrlID(int nID)
{
	ASSERT(::IsWindow(m_hWnd) || (m_pCtrlSite != NULL));

	if (m_pCtrlSite == NULL)
		return (int)::SetWindowLong(m_hWnd, GWL_ID, nID);
	else
		return m_pCtrlSite->SetDlgCtrlID(nID);
}

void CWnd::MoveWindow(int x, int y, int nWidth, int nHeight, BOOL bRepaint)
{
	ASSERT(::IsWindow(m_hWnd) || (m_pCtrlSite != NULL));

	if (m_pCtrlSite == NULL)
		::MoveWindow(m_hWnd, x, y, nWidth, nHeight, bRepaint);
	else
		m_pCtrlSite->MoveWindow(x, y, nWidth, nHeight);
}

BOOL CWnd::SetWindowPos(const CWnd* pWndInsertAfter, int x, int y, int cx,
	int cy, UINT nFlags)
{
	ASSERT(::IsWindow(m_hWnd) || (m_pCtrlSite != NULL));

	if (m_pCtrlSite == NULL)
		return ::SetWindowPos(m_hWnd, pWndInsertAfter->GetSafeHwnd(),
			x, y, cx, cy, nFlags);
	else
		return m_pCtrlSite->SetWindowPos(pWndInsertAfter, x, y, cx, cy, nFlags);
}

BOOL CWnd::ShowWindow(int nCmdShow)
{
	ASSERT(::IsWindow(m_hWnd) || (m_pCtrlSite != NULL));

	if (m_pCtrlSite == NULL)
		return ::ShowWindow(m_hWnd, nCmdShow);
	else
		return m_pCtrlSite->ShowWindow(nCmdShow);
}

BOOL CWnd::IsWindowEnabled() const
{
	ASSERT(::IsWindow(m_hWnd) || (m_pCtrlSite != NULL));

	if (m_pCtrlSite == NULL)
		return ::IsWindowEnabled(m_hWnd);
	else
		return m_pCtrlSite->IsWindowEnabled();
}

BOOL CWnd::EnableWindow(BOOL bEnable)
{
	ASSERT(::IsWindow(m_hWnd) || (m_pCtrlSite != NULL));

	if (m_pCtrlSite == NULL)
		return ::EnableWindow(m_hWnd, bEnable);
	else
		return m_pCtrlSite->EnableWindow(bEnable);
}

CWnd* CWnd::SetFocus()
{
	ASSERT(::IsWindow(m_hWnd) || (m_pCtrlSite != NULL));

	if (m_pCtrlSite == NULL)
		return CWnd::FromHandle(::SetFocus(m_hWnd));
	else
		return m_pCtrlSite->SetFocus();
}

/////////////////////////////////////////////////////////////////////////////
// CWnd invoke helpers for OLE Control wrappers

void AFX_CDECL CWnd::InvokeHelper(DISPID dwDispID, WORD wFlags, VARTYPE vtRet,
	void* pvRet, const BYTE* pbParamInfo, ...)
{
	ASSERT(m_pCtrlSite != NULL);    // not an OLE control (not yet, at least)

	if (m_pCtrlSite == NULL)
		return;

	va_list argList;
	va_start(argList, pbParamInfo);
	m_pCtrlSite->InvokeHelperV(dwDispID, wFlags, vtRet, pvRet, pbParamInfo,
		argList);
	va_end(argList);
}

void CWnd::GetProperty(DISPID dwDispID, VARTYPE vtProp,
	void* pvProp) const
{
	ASSERT(m_pCtrlSite != NULL);    // not an OLE control (not yet, at least)

	if (m_pCtrlSite == NULL)
		return;

	const_cast<CWnd*>(this)->InvokeHelper(dwDispID, DISPATCH_PROPERTYGET,
		vtProp, pvProp, NULL);
}

void AFX_CDECL CWnd::SetProperty(DISPID dwDispID, VARTYPE vtProp, ...)
{
	ASSERT(m_pCtrlSite != NULL);    // not an OLE control (not yet, at least)

	if (m_pCtrlSite == NULL)
		return;

	va_list argList;    // really only one arg, but...
	va_start(argList, vtProp);
	m_pCtrlSite->SetPropertyV(dwDispID, vtProp, argList);
	va_end(argList);
}

IUnknown* CWnd::GetDSCCursor()
{
	ASSERT(m_pCtrlSite != NULL);    // not an OLE control (not yet, at least)
	if (m_pCtrlSite == NULL)
		return NULL;

	m_pCtrlSite->EnableDSC();

	IUnknown* pCursor = m_pCtrlSite->m_pDataSourceControl->GetCursor();
	ASSERT(pCursor != NULL);  // data source control has no cursor

	return pCursor;
}

void CWnd::BindDefaultProperty(DISPID dwDispID, VARTYPE vtProp, LPCTSTR szFieldName, CWnd* pDSCWnd)
{
	ASSERT(m_pCtrlSite != NULL); // not an OLE control (not yet, at least)
	m_pCtrlSite->BindDefaultProperty(dwDispID, vtProp, szFieldName, pDSCWnd);
}

void CWnd::BindProperty(DISPID dwDispId, CWnd* pWndDSC)
{
	ASSERT(m_pCtrlSite != NULL); // not an OLE control (not yet, at least)
	m_pCtrlSite->BindProperty(dwDispId, pWndDSC);
}

/////////////////////////////////////////////////////////////////////////////
// CWnd implementation helpers

void CWnd::AttachControlSite(CHandleMap* pMap)
{
	if (this != NULL && m_pCtrlSite == NULL)
	{
		// Determine if parent is an OLE control container
		CWnd* pWndParent = (CWnd*)pMap->LookupPermanent(::GetParent(m_hWnd));
		if (pWndParent != NULL && pWndParent->m_pCtrlCont != NULL)
		{
			// delegate through helper in COleControlSite
			pWndParent->m_pCtrlCont->AttachControlSite(this);
		}
	}
}

void CWnd::AttachControlSite(CWnd* pWndParent, UINT nIDC)
{
	ASSERT(this != NULL);
	ASSERT(pWndParent != NULL);

	if (m_pCtrlSite == NULL && pWndParent->m_pCtrlCont != NULL)
	{
		// delegate through helper in COleControlSite
		pWndParent->m_pCtrlCont->AttachControlSite(this, nIDC);
	}
}

DWORD COleControlSiteOrWnd::GetStyle() const
{
	if(m_pSite)
		return m_pSite->GetStyle();
	else
		return (DWORD) ::GetWindowLong(m_hWnd, GWL_STYLE);
}

COleControlSiteOrWnd* CWnd::GetNextDlgTabItem(COleControlSiteOrWnd *pCurSiteOrWnd, BOOL bPrevious) const
{
	if(!m_pCtrlCont)
		return NULL;
	if ((m_nFlags & WF_NOWIN32ISDIALOGMSG)==0)
	{
		CWnd * pChildWnd = GetWindow(GW_CHILD);
		while (NULL != pChildWnd)
		{
			if ( (pChildWnd->GetExStyle() & WS_EX_CONTROLPARENT) 
				&& (pChildWnd->GetStyle() & WS_VISIBLE) 
				&& !(pChildWnd->GetStyle() & WS_DISABLED) )
			{
				return NULL;
			}
			pChildWnd = pChildWnd->GetNextWindow(GW_HWNDNEXT);
		}
	}

	DWORD dwStyle;
	COleControlSiteOrWnd *pSiteOrWnd, *pSiteOrWndFocus = NULL;

	POSITION pos;
	typedef CTypedPtrList<CPtrList, COleControlSiteOrWnd *> coll_t;
	COleControlSiteOrWnd *&(coll_t::*Next)(POSITION &);
	POSITION (coll_t::*Head)() const;

	if(bPrevious)
	{
		Next = &coll_t::GetPrev;
		Head = &coll_t::GetTailPosition;
	}
	else
	{
		Next = &coll_t::GetNext;
		Head = &coll_t::GetHeadPosition;
	}

	pos = (m_pCtrlCont->m_listSitesOrWnds.*Head)();
	// Find current control
	while(pos)
	{
		pSiteOrWnd = (m_pCtrlCont->m_listSitesOrWnds.*Next)(pos);
		ASSERT(pSiteOrWnd);
		if (pCurSiteOrWnd)
		{
			if (pCurSiteOrWnd == pSiteOrWnd)
			{
				pSiteOrWndFocus = pSiteOrWnd;
				break;
			}
		}
		else
		{
			HWND hwndControl=pSiteOrWnd->m_pSite ? pSiteOrWnd->m_pSite->m_hWnd : pSiteOrWnd->m_hWnd;
			if((hwndControl && hwndControl == ::GetFocus()) ||
				(pSiteOrWnd->m_pSite && pSiteOrWnd->m_pSite == m_pCtrlCont->m_pSiteFocus))
			{
				pSiteOrWndFocus = pSiteOrWnd;
				break;
			}
		}
	}

	// Didn't find current control
	if(!pSiteOrWndFocus)
		return NULL;

	// Search for the next control with the WS_TABSTOP style.
	
	
	do
	{
		// Start over if we've reached the "end". pos is NULL if pSiteOrWndFocus
		// is last in m_listSitesOrWnds.
		if(!pos)
		{
			pos = (m_pCtrlCont->m_listSitesOrWnds.*Head)();
		}
		pSiteOrWnd = (m_pCtrlCont->m_listSitesOrWnds.*Next)(pos);
		dwStyle = pSiteOrWnd->GetStyle();

		// if we've wrapped or there's only one control OR
		// the control has the tabstop style set and it's
		// not disabled and it is also visible.
		if(pSiteOrWnd == pSiteOrWndFocus ||
			((dwStyle & WS_TABSTOP) && !(dwStyle & WS_DISABLED) && (dwStyle & WS_VISIBLE) ))
		{
			return pSiteOrWnd;
		}
	}
	while(true);

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////

template<class BASE_CLASS, class TYPE>
class CTypedPtrListIterator
{
	typedef CTypedPtrList<BASE_CLASS, TYPE> CCollection;

	const CCollection *m_pList;
	POSITION m_pos;

public:
	CTypedPtrListIterator(const CCollection &list) : 
		m_pList(&list), m_pos(m_pList->GetHeadPosition()) {}
	
	CTypedPtrListIterator(const CCollection &list, POSITION position) : 
		m_pList(&list), m_pos(position) {}
	
	CTypedPtrListIterator(const CTypedPtrListIterator &iterator) : m_pList(iterator.m_pList)
	{
		ENSURE(m_pList == iterator.m_pList);
		m_pos = iterator.m_pos;
	}

	CTypedPtrListIterator &operator=(const CTypedPtrListIterator &iterator)
	{
		ENSURE(m_pList == iterator.m_pList);
		m_pos = iterator.m_pos;
		return *this;
	}

	POSITION GetPosition() const
	{
		return m_pos;
	}

	bool IsEnd() const 
	{
		return !m_pos;
	}
	
	bool operator!() const
	{
		return IsEnd();
	}

	bool operator==(const CTypedPtrListIterator &iterator) const
	{
		ENSURE(m_pList == iterator.m_pList);
		return m_pos == iterator.m_pos;
	}

	void MoveNext() 
	{ 
		if (m_pos)
		{
			m_pList->GetNext(m_pos); 
		}
	}

	void MovePrev() 
	{
		if (m_pos)
		{
			m_pList->GetPrev(m_pos);
		}
	}

	CTypedPtrListIterator Skip(int nSteps) const
	{
		CTypedPtrListIterator iterator(*this);
		if (nSteps < 0)
		{
			for (; nSteps > 0 && !IsEnd(); --nSteps)
			{
				iterator.MovePrev();
			}
		}
		else
		{
			for (; nSteps > 0 && !IsEnd(); --nSteps)
			{
				iterator.MoveNext();
			}
		}
		return iterator;
	}

	TYPE operator*()
	{ 
		ENSURE(m_pos != NULL);
		return m_pList->GetAt(m_pos); 
	}

	const TYPE operator*() const
	{ 
		return const_cast<CTypedPtrListIterator*>(this)->operator*();
	}
};

class CDlgControlIterator : public CTypedPtrListIterator<CPtrList, COleControlSiteOrWnd*>
{
	typedef CTypedPtrListIterator<CPtrList, COleControlSiteOrWnd*> CBase;

public:
	CDlgControlIterator(const COleControlContainer &controls) : 
		CBase(controls.m_listSitesOrWnds) {}

	CDlgControlIterator(const COleControlContainer &controls, POSITION position) : 
		CBase(controls.m_listSitesOrWnds, position) {}
	
	CDlgControlIterator(const CDlgControlIterator &iterator) : CBase(iterator) {}
	
protected:
	CDlgControlIterator(const CBase &iterator) : CBase(iterator) {}

public:
	CDlgControlIterator Skip(int nSteps) const { return CBase::Skip(nSteps); }
};

/////////////////////////////////////////////////////////////////////////////

class CDlgGroupRadioButtonIterator
{
	CDlgControlIterator m_iterator;

public:
	CDlgGroupRadioButtonIterator(COleControlContainer &controls, POSITION position) : 
		m_iterator(controls, position) {}

	CDlgGroupRadioButtonIterator(const CDlgGroupRadioButtonIterator &iterator) : 
		m_iterator(iterator.m_iterator) {}

	bool IsEnd() const 
	{
		return m_iterator.IsEnd();
	}
	
	void MoveNext()
	{
		CDlgControlIterator iterator = m_iterator;
		BOOL bLoop = FALSE;
		while (TRUE)
		{
			iterator.MoveNext();
			if (iterator.IsEnd() || IsGroup(*iterator))
			{
				iterator = GetFirstButton();
				if (iterator.IsEnd() || bLoop)
				{
					return;
				}
				bLoop = TRUE;
			}
			if (!IsDisabled(*iterator))
			{
				break;
			}
		}
		m_iterator = iterator;
	}

	void MovePrev()
	{
		CDlgControlIterator iterator = m_iterator;
		BOOL bLoop = FALSE;
		while (TRUE)
		{
			if (iterator.IsEnd() || IsGroup(*iterator))
			{
				iterator = GetLastButton();
				if (!iterator || bLoop)
				{
					return;
				}
				bLoop = TRUE;
			}
			else
			{
				iterator.MovePrev();
			}
			if (!iterator.IsEnd() && !IsDisabled(*iterator))
			{
				break;
			}
		}
		m_iterator = iterator;
	}
	
	operator COleControlSiteOrWnd*() const
	{ 
		ENSURE(!m_iterator.IsEnd());
		return *m_iterator; 
	}
	
	static bool IsDisabled(const COleControlSiteOrWnd *pButton)
	{
		if (pButton->m_hWnd && SendMessage(pButton->m_hWnd, WM_GETDLGCODE, 0, 0) & DLGC_STATIC)
		{
			return true;
		}
		DWORD dwStyle = pButton->GetStyle();
		return !(dwStyle & WS_VISIBLE) || !!(dwStyle & WS_DISABLED);
	}
	
	static bool IsGroup(const COleControlSiteOrWnd *pButton)
	{
		return !!(pButton->GetStyle() & WS_GROUP);
	}
	
	CDlgControlIterator GetFirstButton() const
	{
		// The first button in a group is marked with WS_GROUP
		// If there is no button marked this way, the first button on the dialog is selected
		CDlgControlIterator iterator = m_iterator, lastPosition = iterator;
		while (!iterator.IsEnd() && !IsGroup(*iterator))
		{
			lastPosition = iterator;
			iterator.MovePrev();
		}
		if (!iterator)
		{
			return lastPosition;
		}
		return iterator;
	}
	
	CDlgControlIterator GetLastButton() const
	{
		// The last button in a group is one button prior to the next button 
		// marked with WS_GROUP, if such exists, or otherwise, the last button 
		// on the dialog
		CDlgControlIterator iterator = m_iterator, nextPosition = iterator.Skip(1);
		while (!nextPosition.IsEnd() && !IsGroup(*nextPosition))
		{
			nextPosition.MoveNext();
			iterator.MoveNext();
		}
		return iterator;
	}
};

/////////////////////////////////////////////////////////////////////////////

POSITION CWnd::FindSiteOrWnd(const COleControlSiteOrWnd *pSiteOrWnd) const
{
	if (!m_pCtrlCont)
	{
		return NULL;
	}
	CDlgControlIterator iterCtl = *m_pCtrlCont;
	while (!iterCtl.IsEnd())
	{
		if (*iterCtl == pSiteOrWnd)
		{
			return iterCtl.GetPosition();
		}
		iterCtl.MoveNext();
	}
	return NULL;
}

POSITION CWnd::FindSiteOrWndWithFocus() const
{
	if (!m_pCtrlCont)
	{
		return NULL;
	}
	CDlgControlIterator iterCtl= *m_pCtrlCont;
	while (!iterCtl.IsEnd())
	{
		COleControlSiteOrWnd *pSiteOrWnd = *iterCtl;
		HWND hwndControl = pSiteOrWnd->m_pSite ? pSiteOrWnd->m_pSite->m_hWnd : pSiteOrWnd->m_hWnd;
		if ((hwndControl && hwndControl == ::GetFocus()) ||
			(pSiteOrWnd->m_pSite && pSiteOrWnd->m_pSite == m_pCtrlCont->m_pSiteFocus))
		{
			return iterCtl.GetPosition();
		}
		iterCtl.MoveNext();
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////

COleControlSiteOrWnd* CWnd::GetPrevDlgGroupItem(COleControlSiteOrWnd *pCurSiteOrWnd) const
{
	if (!m_pCtrlCont)
	{
		return NULL;
	}

	POSITION pos = pCurSiteOrWnd ? FindSiteOrWnd(pCurSiteOrWnd) : FindSiteOrWndWithFocus();
	if (!pos)
	{
		return NULL;
	}

	CDlgGroupRadioButtonIterator iterator(*m_pCtrlCont, pos);
	iterator.MovePrev();;
	if (!iterator.IsEnd())
	{
		return iterator;
	}
	return NULL;
}

COleControlSiteOrWnd* CWnd::GetNextDlgGroupItem(COleControlSiteOrWnd *pCurSiteOrWnd) const
{
	if (!m_pCtrlCont)
	{
		return NULL;
	}

	POSITION pos = pCurSiteOrWnd ? FindSiteOrWnd(pCurSiteOrWnd) : FindSiteOrWndWithFocus();
	if (!pos)
	{
		return NULL;
	}

	CDlgGroupRadioButtonIterator iterator(*m_pCtrlCont, pos);
	iterator.MoveNext();
	if (!iterator.IsEnd())
	{
		return iterator;
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////

void CWnd::RemoveRadioCheckFromGroup(const COleControlSiteOrWnd *pSiteOrWnd) const
{
	if(!m_pCtrlCont || !pSiteOrWnd)
		return;

	// Try a short cut (i.e., if this is the one that's checked uncheck it
	// and return)
	if(pSiteOrWnd->m_bAutoRadioButton &&
		BST_CHECKED == ::SendMessage(pSiteOrWnd->m_hWnd, BM_GETCHECK, 0, 0))
	{
		::SendMessage(pSiteOrWnd->m_hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
		return;
	}

	// Dang.
	COleControlSiteOrWnd *pSite;
	POSITION pos, posCur =
		m_pCtrlCont->m_listSitesOrWnds.Find(const_cast<COleControlSiteOrWnd *>(pSiteOrWnd));

	// Start at the control after the passed in control and go "down" through the
	// control list looking for a checked auto-radio button.
	pos = posCur;
	m_pCtrlCont->m_listSitesOrWnds.GetNext(pos);
	while(pos)
	{
		pSite = m_pCtrlCont->m_listSitesOrWnds.GetNext(pos);
		if(pSite->GetStyle() & WS_GROUP)
			break;

		if(pSite->m_bAutoRadioButton &&
			BST_CHECKED == ::SendMessage(pSite->m_hWnd, BM_GETCHECK, 0, 0))
		{
			::SendMessage(pSite->m_hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
			return;
		}
	}

	// Double dang.  Start at the control before the passed in control and go "up"
	// through the control list looking for a checked auto-radio button.
	pos = posCur;
	m_pCtrlCont->m_listSitesOrWnds.GetPrev(pos);
	while(pos)
	{
		pSite = m_pCtrlCont->m_listSitesOrWnds.GetPrev(pos);
		if(pSite->m_bAutoRadioButton &&
			BST_CHECKED == ::SendMessage(pSite->m_hWnd, BM_GETCHECK, 0, 0))
		{
			::SendMessage(pSite->m_hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
			return;
		}
		if(pSite->GetStyle() & WS_GROUP)
			break;
	}
}

void COleControlContainer::AttachControlSite(CWnd* pWnd, UINT nIDC)
{
	ASSERT(this != NULL);
	ASSERT(pWnd != NULL);

   COleControlSite* pSite;

	// If a matching control site exists, it's an OLE control
   if (nIDC == 0)
   {
	   pSite = (COleControlSite*)m_siteMap.GetValueAt(pWnd->m_hWnd);
   }
   else
   {
	  pSite = FindItem(nIDC);
   }
	if (pSite != NULL)
	{
		// detach any existing CWnd from this site (last one wins)
		CWnd* pOldCtrl = pSite->m_pWndCtrl;
		if (pOldCtrl != NULL && pOldCtrl->m_pCtrlSite == pSite)
			pOldCtrl->m_pCtrlSite = NULL;

		// now wire the site and CWnd together
		pWnd->m_pCtrlSite = pSite;
		pSite->m_pWndCtrl = pWnd;
	}
}

#endif // !_AFX_NO_OCC_SUPPORT
