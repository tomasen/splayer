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

#ifdef MACOCX
#define DBINITCONSTANTS
#endif
#include "occimpl.h"
#include "ocdb.h"



#define new DEBUG_NEW

#ifndef _AFX_NO_OCC_SUPPORT

/////////////////////////////////////////////////////////////////////////////
// AfxEnableControlContainer - wire up control container functions

PROCESS_LOCAL(COccManager, _afxOccManager)

PROCESS_LOCAL(CControlSiteFactoryMgr, _afxControlFactoryMgr)

void AFX_CDECL AfxEnableControlContainer(COccManager* pOccManager)
{
	if (pOccManager == NULL)
		afxOccManager = _afxOccManager.GetData();
	else
		afxOccManager = pOccManager;
}

/////////////////////////////////////////////////////////////////////////////
// Helper functions for cracking dialog templates

static inline BOOL IsDialogEx(const DLGTEMPLATE* pTemplate)
{
	ENSURE_ARG(pTemplate!=NULL);
	return ((DLGTEMPLATEEX*)pTemplate)->signature == 0xFFFF;
}

static inline WORD& DlgTemplateItemCount(DLGTEMPLATE* pTemplate)
{
	if (IsDialogEx(pTemplate))
		return reinterpret_cast<DLGTEMPLATEEX*>(pTemplate)->cDlgItems;
	else
		return pTemplate->cdit;
}

static inline const WORD& DlgTemplateItemCount(const DLGTEMPLATE* pTemplate)
{
	if (IsDialogEx(pTemplate))
		return reinterpret_cast<const DLGTEMPLATEEX*>(pTemplate)->cDlgItems;
	else
		return pTemplate->cdit;
}

AFX_STATIC DLGITEMTEMPLATE* AFXAPI _AfxFindFirstDlgItem(const DLGTEMPLATE* pTemplate)
{
	DWORD dwStyle = pTemplate->style;
	BOOL bDialogEx = IsDialogEx(pTemplate);

	WORD* pw;
	if (bDialogEx)
	{
		pw = (WORD*)((DLGTEMPLATEEX*)pTemplate + 1);
		dwStyle = ((DLGTEMPLATEEX*)pTemplate)->style;
	}
	else
	{
		pw = (WORD*)(pTemplate + 1);
	}

	if (*pw == (WORD)-1)        // Skip menu name ordinal or string
		pw += 2; // WORDs
	else
		while (*pw++);

	if (*pw == (WORD)-1)        // Skip class name ordinal or string
		pw += 2; // WORDs
	else
		while (*pw++);

	while (*pw++);              // Skip caption string

	if (dwStyle & DS_SETFONT)
	{
		pw += bDialogEx ? 3 : 1;    // Skip font size, weight, (italic, charset)
		while (*pw++);              // Skip font name
	}

	// Dword-align and return
	return (DLGITEMTEMPLATE*)(((DWORD_PTR)pw + 3) & ~DWORD_PTR(3));
}

AFX_STATIC DLGITEMTEMPLATE* AFXAPI _AfxFindNextDlgItem(DLGITEMTEMPLATE* pItem, BOOL bDialogEx)
{
	WORD* pw;

	if (bDialogEx)
		pw = (WORD*)((DLGITEMTEMPLATEEX*)pItem + 1);
	else
		pw = (WORD*)(pItem + 1);

	if (*pw == (WORD)-1)            // Skip class name ordinal or string
		pw += 2; // WORDs
	else
		while (*pw++);

	if (*pw == (WORD)-1)            // Skip text ordinal or string
		pw += 2; // WORDs
	else
		while (*pw++);

	WORD cbExtra = *pw++;           // Skip extra data

	if (cbExtra != 0 && !bDialogEx)
		cbExtra -=2;

	// Dword-align and return
	return (DLGITEMTEMPLATE*)(((DWORD_PTR)pw + cbExtra + 3) & ~DWORD_PTR(3));
}

/////////////////////////////////////////////////////////////////////////////
// COccManager

BOOL COccManager::OnEvent(CCmdTarget* pCmdTarget, UINT idCtrl,
	AFX_EVENT* pEvent, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	return pCmdTarget->OnEvent(idCtrl, pEvent, pHandlerInfo);
}

COleControlContainer* COccManager::CreateContainer(CWnd* pWnd)
{
	// advanced control container apps may want to override
	return new COleControlContainer(pWnd);
}


//For backward compatibility with user code that overrides CreateSite. 
COleControlSite* COccManager::CreateSite(COleControlContainer* pCtrlCont)
{
	pCtrlCont; // unused
	return NULL;
}


COleControlSite* COccManager::CreateSite(COleControlContainer* pCtrlCont,const CControlCreationInfo& creationInfo)
{
	COleControlSite* pSite=CreateSite(pCtrlCont);
	if (pSite!=NULL && creationInfo.IsManaged())
	{
		TRACE(traceOle, 0, "Warning: User overrides COccManager::CreateSite that prevent CControlCreationInfo (WinForms control) special site to be created.\nCan fix by overriding the new COccManager::CreateSite(COleControlContainer* pCtrlCont,const CControlCreationInfo& creationInfo) and allowing WinForms site to get created.");
		return NULL;
	}

	if (pSite==NULL)
	{
		//Search among the registered control site factories, one that wants to create the 
		//control site for this clsid. If none is found, default to COleControlSite.	
		pSite=_afxControlFactoryMgr->CreateSite(pCtrlCont,creationInfo);
		ENSURE(pSite != NULL); //At least the default factory (COleControlSite) must be registered.
	}
	return pSite;
}

const DLGTEMPLATE* COccManager::PreCreateDialog(_AFX_OCC_DIALOG_INFO* pDlgInfo,
	const DLGTEMPLATE* pOrigTemplate)
{
	ASSERT(pDlgInfo != NULL);

	pDlgInfo->m_ppOleDlgItems =
		(DLGITEMTEMPLATE**)calloc(sizeof(DLGITEMTEMPLATE*),
			(DlgTemplateItemCount(pOrigTemplate) + 1));

	if (pDlgInfo->m_ppOleDlgItems == NULL)
		return NULL;

	DLGTEMPLATE* pNewTemplate = SplitDialogTemplate(pOrigTemplate,
		pDlgInfo->m_ppOleDlgItems);
	pDlgInfo->m_pNewTemplate = pNewTemplate;

	DLGITEMTEMPLATE *pItem = _AfxFindFirstDlgItem(pOrigTemplate);
	DLGITEMTEMPLATE *pNextItem;
	BOOL bDialogEx = IsDialogEx(pOrigTemplate);

	int iItem, iItems = DlgTemplateItemCount(pOrigTemplate);
	pDlgInfo->m_pItemInfo = new _AFX_OCC_DIALOG_INFO::ItemInfo[iItems];
	memset(pDlgInfo->m_pItemInfo, 0,
		sizeof(_AFX_OCC_DIALOG_INFO::ItemInfo) * iItems);
	pDlgInfo->m_cItems = iItems;
	LPCWSTR pszClass;
	DWORD dwStyle;

	for(iItem = 0; iItem < iItems; iItem++)
	{
		pNextItem = _AfxFindNextDlgItem(pItem, bDialogEx);
		if(bDialogEx)
		{
			DLGITEMTEMPLATEEX *pItemEx = (DLGITEMTEMPLATEEX *) pItem;
			pDlgInfo->m_pItemInfo[iItem].nId = pItemEx->id;
			pszClass = (LPCWSTR) (pItemEx + 1);
			dwStyle = pItemEx->style;
		}
		else
		{
			pDlgInfo->m_pItemInfo[iItem].nId = pItem->id;
			pszClass = (LPCWSTR) (pItem + 1);
			dwStyle = pItem->style;
		}

		// this is a good place to store for later use gobs 'o info
		// about the controls on the dialog.
		pDlgInfo->m_pItemInfo[iItem].bAutoRadioButton =
			pszClass[0] == 0xffff && pszClass[1] == 0x0080 &&
			(dwStyle & 0x0f) == BS_AUTORADIOBUTTON;

		pItem = pNextItem;
	}

	return (pNewTemplate != NULL) ? pNewTemplate : pOrigTemplate;
}

void COccManager::PostCreateDialog(_AFX_OCC_DIALOG_INFO* pDlgInfo)
{
	if (pDlgInfo->m_pNewTemplate != NULL)
		GlobalFree(pDlgInfo->m_pNewTemplate);

	if (pDlgInfo->m_ppOleDlgItems != NULL)
		free(pDlgInfo->m_ppOleDlgItems);

	delete[] pDlgInfo->m_pItemInfo;
}

DLGTEMPLATE* COccManager::SplitDialogTemplate(const DLGTEMPLATE* pTemplate,
	DLGITEMTEMPLATE** ppOleDlgItems)
{
	DLGITEMTEMPLATE* pFirstItem = _AfxFindFirstDlgItem(pTemplate);
   //IA64: Assume max header size of 4GB
	ULONG cbHeader = ULONG((BYTE*)pFirstItem - (BYTE*)pTemplate);
	ULONG cbNewTemplate = cbHeader;

	BOOL bDialogEx = IsDialogEx(pTemplate);

	int iItem;
	int nItems = (int)DlgTemplateItemCount(pTemplate);
	DLGITEMTEMPLATE* pItem = pFirstItem;
	DLGITEMTEMPLATE* pNextItem = pItem;
	LPWSTR pszClassName;
	BOOL bHasOleControls = FALSE;

	// Make first pass through the dialog template.  On this pass, we're
	// interested in determining:
	//    1. Does this template contain any OLE controls?
	//    2. If so, how large a buffer is needed for a template containing
	//       only the non-OLE controls?

	for (iItem = 0; iItem < nItems; iItem++)
	{
		pNextItem = _AfxFindNextDlgItem(pItem, bDialogEx);

		pszClassName = bDialogEx ?
			(LPWSTR)(((DLGITEMTEMPLATEEX*)pItem) + 1) :
			(LPWSTR)(pItem + 1);

		if (pszClassName[0] == L'{')
		{
			// Item is an OLE control.
			bHasOleControls = TRUE;
		}
		else
		{
			// Item is not an OLE control: make room for it in new template.
		 //IA64: Assume max template size < 4GB 
			cbNewTemplate += ULONG((BYTE*)pNextItem - (BYTE*)pItem);
		}

		pItem = pNextItem;
	}

	// No OLE controls were found, so there's no reason to go any further.
	if (!bHasOleControls)
	{
		ppOleDlgItems[0] = (DLGITEMTEMPLATE*)(-1);
		return NULL;
	}

	// Copy entire header into new template.
	BYTE* pNew = (BYTE*)GlobalAlloc(GMEM_FIXED, cbNewTemplate);
	ENSURE_THROW(pNew!=NULL	, ::AfxThrowMemoryException() );
	DLGTEMPLATE* pNewTemplate = (DLGTEMPLATE*)pNew;
	Checked::memcpy_s(pNew, cbNewTemplate, pTemplate, cbHeader);
	pNew += cbHeader;

	// Initialize item count in new header to zero.
	DlgTemplateItemCount(pNewTemplate) = 0;

	pItem = pFirstItem;
	pNextItem = pItem;

	// Second pass through the dialog template.  On this pass, we want to:
	//    1. Copy all the non-OLE controls into the new template.
	//    2. Build an array of item templates for the OLE controls.

	for (iItem = 0; iItem < nItems; iItem++)
	{
		pNextItem = _AfxFindNextDlgItem(pItem, bDialogEx);

		pszClassName = bDialogEx ?
			(LPWSTR)(((DLGITEMTEMPLATEEX*)pItem) + 1) :
			(LPWSTR)(pItem + 1);

		if (pszClassName[0] == L'{')
		{
			// Item is OLE control: add it to the array.
			ppOleDlgItems[iItem] = pItem;
		}
		else
		{
			// Item is not an OLE control: copy it to the new template.
		 //IA64: Assume max item size < 4GB
			ULONG cbItem = ULONG((BYTE*)pNextItem - (BYTE*)pItem);
			ASSERT(cbItem >= (size_t)(bDialogEx ?
				sizeof(DLGITEMTEMPLATEEX) :
				sizeof(DLGITEMTEMPLATE)));

			Checked::memcpy_s(pNew, cbItem, pItem, cbItem);
			pNew += cbItem;

			// Incrememt item count in new header.
			++DlgTemplateItemCount(pNewTemplate);

			// Put placeholder in OLE item array.
			ppOleDlgItems[iItem] = NULL;
		}

		pItem = pNextItem;
	}
	ppOleDlgItems[nItems] = (DLGITEMTEMPLATE*)(-1);

	return pNewTemplate;
}

BOOL COccManager::CreateDlgControls(CWnd* pWndParent, LPCTSTR lpszResourceName,
	_AFX_OCC_DIALOG_INFO* pOccDlgInfo)
{
	// find resource handle
	void* lpResource = NULL;
	HGLOBAL hResource = NULL;
	if (lpszResourceName != NULL)
	{
		HINSTANCE hInst = AfxFindResourceHandle(lpszResourceName, RT_DLGINIT);
		HRSRC hDlgInit = ::FindResource(hInst, lpszResourceName, RT_DLGINIT);
		if (hDlgInit != NULL)
		{
			// load it
			hResource = LoadResource(hInst, hDlgInit);
			if (hResource == NULL)
			{
				TRACE(traceOle, 0, "DLGINIT resource was found, but could not be loaded.\n");
				return FALSE;
			}

			// lock it
			lpResource = LockResource(hResource);
			ASSERT(lpResource != NULL);
		}
#ifdef _DEBUG
		else
		{
			// If we didn't find a DLGINIT resource, check whether we were
			// expecting to find one
			DLGITEMTEMPLATE** ppOleDlgItems = pOccDlgInfo->m_ppOleDlgItems;
			ASSERT(ppOleDlgItems != NULL);

			while (*ppOleDlgItems != (DLGITEMTEMPLATE*)-1)
			{
				if (*ppOleDlgItems != NULL)
				{
					TRACE(traceOle, 0, "Dialog has OLE controls, but no matching DLGINIT resource.\n");
					break;
				}
				++ppOleDlgItems;
			}
		}
#endif
	}

	// execute it
	BOOL bResult = TRUE;
	if (lpResource != NULL)
		bResult = CreateDlgControls(pWndParent, lpResource, pOccDlgInfo);

	// cleanup
	if (lpResource != NULL && hResource != NULL)
	{
		UnlockResource(hResource);
		FreeResource(hResource);
	}

	if(pWndParent->m_pCtrlCont)
	{
		pWndParent->m_pCtrlCont->FillListSitesOrWnds(pOccDlgInfo);
	}

	return bResult;
}

BOOL COccManager::CreateDlgControls(CWnd* pWndParent, void* lpResource,
	_AFX_OCC_DIALOG_INFO* pOccDlgInfo)
{
	// if there are no OLE controls in this dialog, then there's nothing to do
	if (pOccDlgInfo->m_pNewTemplate == NULL)
		return TRUE;

	ASSERT(pWndParent != NULL);
	HWND hwParent = pWndParent->GetSafeHwnd();

	BOOL bDialogEx = IsDialogEx(pOccDlgInfo->m_pNewTemplate);
	BOOL bSuccess = TRUE;
	if (lpResource != NULL)
	{
		ASSERT(pOccDlgInfo != NULL);
		ASSERT(pOccDlgInfo->m_ppOleDlgItems != NULL);

		DLGITEMTEMPLATE** ppOleDlgItems = pOccDlgInfo->m_ppOleDlgItems;

		UNALIGNED WORD* lpnRes = (WORD*)lpResource;
		int iItem = 0;
		HWND hwAfter = HWND_TOP;
		while (bSuccess && *lpnRes != 0)
		{
			WORD nIDC = *lpnRes++;
			WORD nMsg = *lpnRes++;
			DWORD dwLen = *((UNALIGNED DWORD*&)lpnRes)++;

			#define WIN16_LB_ADDSTRING  0x0401
			#define WIN16_CB_ADDSTRING  0x0403

			ASSERT(nMsg == LB_ADDSTRING || nMsg == CB_ADDSTRING ||
				nMsg == WIN16_LB_ADDSTRING || nMsg == WIN16_CB_ADDSTRING ||
				nMsg == WM_OCC_LOADFROMSTREAM ||
				nMsg == WM_OCC_LOADFROMSTREAM_EX ||
				nMsg == WM_OCC_LOADFROMSTORAGE ||
				nMsg == WM_OCC_LOADFROMSTORAGE_EX ||
				nMsg == WM_OCC_INITNEW);

			if (nMsg == WM_OCC_LOADFROMSTREAM ||
				nMsg == WM_OCC_LOADFROMSTREAM_EX ||
				nMsg == WM_OCC_LOADFROMSTORAGE ||
				nMsg == WM_OCC_LOADFROMSTORAGE_EX ||
				nMsg == WM_OCC_INITNEW)
			{
				// Locate the DLGITEMTEMPLATE for the new control, and the control
				// that should precede it in z-order.
				DLGITEMTEMPLATE* pDlgItem;
				while (((pDlgItem = ppOleDlgItems[iItem++]) == NULL) &&
					(pDlgItem != (DLGITEMTEMPLATE*)(-1)))
				{
					if (hwAfter == HWND_TOP)
						hwAfter = GetWindow(hwParent, GW_CHILD);
					else
						hwAfter = GetWindow(hwAfter, GW_HWNDNEXT);

					ASSERT(hwAfter != NULL);  // enough non-OLE controls?
				}

				ASSERT(pDlgItem != NULL);   // enough dialog item templates?

				HWND hwNew = NULL;
			BOOL bCreated = FALSE;
				if (pDlgItem != (DLGITEMTEMPLATE*)(-1))
				{
#ifdef _DEBUG
					WORD id = bDialogEx ?
						(WORD)((DLGITEMTEMPLATEEX*)pDlgItem)->id :
						pDlgItem->id;
					ASSERT(id == nIDC); // make sure control IDs match!
#endif

					// Create the OLE control now.
					bCreated = CreateDlgControl(pWndParent, hwAfter, bDialogEx,
						pDlgItem, nMsg, (BYTE*)lpnRes, dwLen, &hwNew);
				}

			if (bCreated)
			{
				   if (hwNew != NULL)
				   {
					   if (bDialogEx)
						   SetWindowContextHelpId(hwNew,
							   ((DLGITEMTEMPLATEEX*)pDlgItem)->helpID);
					   if (GetParent(hwNew) == hwParent)
						   hwAfter = hwNew;
				   }
			}
				else
					bSuccess = FALSE;
			}

			// skip past data
			lpnRes = (WORD*)((LPBYTE)lpnRes + (UINT)dwLen);
		}
	}

	if (bSuccess)
	{
		// unfreeze events now that all controls are loaded
		if (pWndParent->m_pCtrlCont != NULL)
			pWndParent->m_pCtrlCont->FreezeAllEvents(FALSE);

		BindControls(pWndParent);
	}

	return bSuccess;
}


void COccManager::BindControls(CWnd* pWndParent)
{
	COleControlSiteOrWnd* pSiteOrWnd;

	if (pWndParent->m_pCtrlCont != NULL)
	{
		// Now initialize bound controls
		POSITION pos = pWndParent->m_pCtrlCont->m_listSitesOrWnds.GetHeadPosition();
		while (pos != NULL)
		{
			pSiteOrWnd = pWndParent->m_pCtrlCont->m_listSitesOrWnds.GetNext( pos );
			ASSERT(pSiteOrWnd);

			if(pSiteOrWnd->m_pSite)
			{
				// For each cursor bound property initialize pClientSite ptr and bind to DSC
				CDataBoundProperty* pBinding = pSiteOrWnd->m_pSite->m_pBindings;
				while(pBinding)
				{
					pBinding->SetClientSite(pSiteOrWnd->m_pSite);
					if (pBinding->m_ctlid != 0)
					{
						CWnd* pWnd = pWndParent->GetDlgItem(pBinding->m_ctlid);
						ASSERT(pWnd);
						ASSERT(pWnd->m_pCtrlSite);
						pBinding->SetDSCSite(pWnd->m_pCtrlSite);
					}
					pBinding = pBinding->GetNext();
				}

				// Bind default bound property
				if (pSiteOrWnd->m_pSite->m_ctlidRowSource != NULL)
				{
					CWnd* pWnd = pWndParent->GetDlgItem(pSiteOrWnd->m_pSite->m_ctlidRowSource);
					ASSERT(pWnd);  // gotta be a legitimate control id
					ASSERT(pWnd->m_pCtrlSite);  // and it has to be an OLE Control

					pWnd->m_pCtrlSite->EnableDSC();

					ASSERT(pWnd->m_pCtrlSite->m_pDataSourceControl);  // and a Data Source Control
					pSiteOrWnd->m_pSite->m_pDSCSite = pWnd->m_pCtrlSite;
					pWnd->m_pCtrlSite->m_pDataSourceControl->BindProp(pSiteOrWnd->m_pSite);
				}
			}
		}

		// Finally, set up bindings on all DataSource controls
		pos = pWndParent->m_pCtrlCont->m_listSitesOrWnds.GetHeadPosition();
		while (pos != NULL)
		{
			pSiteOrWnd = pWndParent->m_pCtrlCont->m_listSitesOrWnds.GetNext( pos );
			ASSERT(pSiteOrWnd);
			if (pSiteOrWnd->m_pSite && pSiteOrWnd->m_pSite->m_pDataSourceControl)
				pSiteOrWnd->m_pSite->m_pDataSourceControl->BindColumns();
		}
	}
}


BOOL COccManager::CreateDlgControl(CWnd* pWndParent, HWND hwAfter,
	BOOL bDialogEx, LPDLGITEMTEMPLATE pItem, WORD nMsg, BYTE* lpData, DWORD cb, HWND* phWnd)
{
	LPWSTR pszClass = (LPWSTR)(pItem + 1);
	DLGITEMTEMPLATE dlgItemTmp;

	if (bDialogEx)
	{
		// We have an extended dialog template: copy relevant parts into an
		// ordinary dialog template, because their layouts are different
		DLGITEMTEMPLATEEX* pItemEx = (DLGITEMTEMPLATEEX*)pItem;
		dlgItemTmp.style = pItemEx->style;
		dlgItemTmp.dwExtendedStyle = pItemEx->exStyle;
		dlgItemTmp.x = pItemEx->x;
		dlgItemTmp.y = pItemEx->y;
		dlgItemTmp.cx = pItemEx->cx;
		dlgItemTmp.cy = pItemEx->cy;
		dlgItemTmp.id = (WORD)pItemEx->id;
		pItem = &dlgItemTmp;
		pszClass = (LPWSTR)(pItemEx + 1);
	}

	CRect rect(pItem->x, pItem->y, pItem->x + pItem->cx, pItem->y + pItem->cy);
	::MapDialogRect(pWndParent->m_hWnd, &rect);

	BSTR bstrLicKey = NULL;

	// extract license key data, if any
	if (cb >= sizeof(ULONG))
	{
		ULONG cchLicKey = *(UNALIGNED ULONG*)lpData;
		lpData += sizeof(ULONG);
		cb -= sizeof(ULONG);
		if (cchLicKey > 0)
		{
			bstrLicKey = SysAllocStringLen((LPCOLESTR)lpData, cchLicKey);
			lpData += cchLicKey * sizeof(WCHAR);
			cb -= cchLicKey * sizeof(WCHAR);
		}
	}

	// If WM_OCC_INITNEW, we should have exhausted all of the data by now.
	ASSERT((nMsg != WM_OCC_INITNEW) || (cb == 0));

	CDataBoundProperty* pBindings = NULL;
	CString strDataField;
	WORD ctlidRowSource = 0;
	DISPID defdispid = 0;
	UINT dwType = 0;

	if (nMsg == WM_OCC_LOADFROMSTREAM_EX ||
		nMsg == WM_OCC_LOADFROMSTORAGE_EX)
	{
		// Read the size of the section
		ULONG cbOffset = *(UNALIGNED ULONG*)lpData;
		ULONG cbBindInfo = cbOffset - sizeof(DWORD);
		lpData += sizeof(DWORD);

		ULONG dwFlags = *(UNALIGNED ULONG*)lpData;
		cbBindInfo -= sizeof(DWORD);
		lpData += sizeof(DWORD);
		ASSERT(dwFlags == 1);

		// ULONG cbBinding = *(UNALIGNED ULONG*)lpData;
		cbBindInfo -= sizeof(DWORD);
		lpData += sizeof(DWORD);

		while (cbBindInfo > 0)
		{
			DISPID dispid;
			WORD ctlid;

			dispid = *(UNALIGNED DISPID *)lpData;
			lpData += sizeof(DISPID);
			cbBindInfo -= sizeof(DISPID);
			ctlid =  *(UNALIGNED WORD *)lpData;
			lpData += sizeof(WORD);
			cbBindInfo -= sizeof(WORD);

			if(dispid == DISPID_DATASOURCE)
			{
				defdispid = *(UNALIGNED ULONG*)lpData;
				cbBindInfo -= sizeof(DISPID);
				lpData += sizeof(DISPID);
				dwType = *(UNALIGNED ULONG*)lpData;
				cbBindInfo -= sizeof(DWORD);
				lpData += sizeof(DWORD);

				ASSERT(*(UNALIGNED DISPID *)lpData == DISPID_DATAFIELD);
				lpData += sizeof(DISPID);
				cbBindInfo -= sizeof(DISPID);
				// Skip the string length
				lpData += sizeof(DWORD);
				cbBindInfo -= sizeof(DWORD);
				strDataField = (char *)lpData;
				lpData += strDataField.GetLength()+1;
				cbBindInfo -= ULONG(strDataField.GetLength()+1);
				ctlidRowSource = ctlid;
			} else
				pBindings = new CDataBoundProperty(pBindings, dispid, ctlid);
		}
		cb -= cbOffset;

		// From now on act as a regular type
		nMsg -= (WM_OCC_LOADFROMSTREAM_EX - WM_OCC_LOADFROMSTREAM);
	}

	GUID clsid;
	HRESULT hr;
	if (pszClass[0] == L'{')
		hr = CLSIDFromString(pszClass, &clsid);
	else
		hr = CLSIDFromProgID(pszClass, &clsid);

#ifdef _DEBUG
	if (FAILED(hr))
	{
		TRACE(traceOle, 0, "Unable to convert \"%ls\" to a class ID.\n", pszClass);
		TRACE(traceOle, 0, ">>> Result code: 0x%08lx\n", hr);
		if (pszClass[0] != L'{')
			TRACE(traceOle, 0, ">>> Is the control properly registered?\n");
	}
#endif

	CMemFile memFile(lpData, cb);
	CMemFile* pMemFile = (nMsg == WM_OCC_INITNEW) ? NULL : &memFile;

	COleControlSite* pSite = NULL;

	if (SUCCEEDED(hr) &&
		pWndParent->InitControlContainer(/*bCreateFromResource=*/TRUE) &&
		pWndParent->m_pCtrlCont->CreateControl(NULL, clsid, NULL, pItem->style,
			rect, pItem->id, pMemFile, (nMsg == WM_OCC_LOADFROMSTORAGE),
			bstrLicKey, &pSite))
	{
		ASSERT(pSite != NULL);

		// freeze events until all controls are loaded
		pSite->FreezeEvents(TRUE);

		// set ZOrder only!
		SetWindowPos(pSite->m_hWnd, hwAfter, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

		pSite->m_pBindings = pBindings;
		pSite->m_strDataField = strDataField;
		pSite->m_ctlidRowSource = ctlidRowSource;
		pSite->m_defdispid = defdispid;
		pSite->m_dwType = dwType;
	}

	if (bstrLicKey != NULL)
		SysFreeString(bstrLicKey);

   if (pSite != NULL)
   {
	  *phWnd = pSite->m_hWnd;
	  return TRUE;
   }
   else
   {
	  *phWnd = NULL;
	  return FALSE;
   }
}

/////////////////////////////////////////////////////////////////////////////
// CDataExchange::PrepareOleCtrl

COleControlSite* CDataExchange::PrepareOleCtrl(int nIDC)
{
	ASSERT(nIDC != 0);
	ASSERT(nIDC != -1); // not allowed
   COleControlSite* pSite;
   pSite = m_pDlgWnd->GetOleControlSite(nIDC);
	if (pSite == NULL)
	{
		TRACE(traceOle, 0, "Error: no data exchange control with ID 0x%04X\n", nIDC);
		ASSERT(FALSE);
		AfxThrowNotSupportedException();
	}
	m_idLastControl = nIDC;
	m_bEditLastControl = FALSE; // not an edit item by default
   return pSite;
}

/////////////////////////////////////////////////////////////////////
// COleControlSiteFactory - new COleControlSite for ActiveX controls.
// This is the default factory if none are registered or rejected the 
//site instantiation request.
class COleControlSiteFactory : public IControlSiteFactory
{
public:
	COleControlSiteFactory() { }
	COleControlSite* CreateSite(COleControlContainer* pCtrlCont,const CControlCreationInfo&)
	{
		return new COleControlSite(pCtrlCont);
	}	
};

CControlSiteFactoryMgr::CControlSiteFactoryMgr()
{
	//The last item in the list is the default factory
	m_pOleControlSiteDefaultFactory = new COleControlSiteFactory();
	try
	{
		RegisterSiteFactory(m_pOleControlSiteDefaultFactory);
	}
	catch(...)
	{
		delete m_pOleControlSiteDefaultFactory;
		m_pOleControlSiteDefaultFactory=NULL;
	}
}


CControlSiteFactoryMgr::~CControlSiteFactoryMgr()
{
	delete m_pOleControlSiteDefaultFactory;
	m_lstFactory.RemoveAll();
}

//Search for factory interested in creating the site.

COleControlSite* CControlSiteFactoryMgr::CreateSite(COleControlContainer* pCtrlCont,const CControlCreationInfo& creationInfo)
{
	COleControlSite* pSite=NULL;
	POSITION pos = m_lstFactory.GetHeadPosition();
	while( pos != NULL )
	{		
		IControlSiteFactory* pFactory = m_lstFactory.GetNext( pos );	
		if(pFactory!=NULL)
		{
			pSite=pFactory->CreateSite(pCtrlCont,creationInfo);
			if (pSite!=NULL)
			{
				break;
			}
		}
	}	
	return pSite;
}

//Add if not exists
BOOL CControlSiteFactoryMgr::RegisterSiteFactory(IControlSiteFactory* pFactory)
{
	ENSURE_ARG(pFactory!=NULL);
	POSITION position=NULL;
	position=m_lstFactory.Find(pFactory);
	BOOL bAdded=FALSE;
	if (position==NULL)
	{
		m_lstFactory.AddHead(pFactory);
	}
	return bAdded;
}
//Remove if exists.
BOOL CControlSiteFactoryMgr::UnregisterSiteFactory(IControlSiteFactory* pFactory)
{
	ENSURE_ARG(pFactory!=NULL);
	POSITION position=NULL;
	position=m_lstFactory.Find(pFactory);
	BOOL bRemoved=FALSE;
	if (position!=NULL)
	{
		m_lstFactory.RemoveAt(position);
		bRemoved=TRUE;
	}
	return bRemoved;
}


BOOL AFXAPI AfxRegisterSiteFactory(IControlSiteFactory* pFactory)
{
	return _afxControlFactoryMgr->RegisterSiteFactory(pFactory);
}

BOOL AFXAPI AfxUnregisterSiteFactory(IControlSiteFactory* pFactory)
{
	return _afxControlFactoryMgr->UnregisterSiteFactory(pFactory);
}



#endif //!_AFX_NO_OCC_SUPPORT
