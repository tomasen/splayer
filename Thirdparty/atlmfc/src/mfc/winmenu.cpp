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



#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// Map from HMENU to CMenu *

CHandleMap* PASCAL afxMapHMENU(BOOL bCreate)
{
	AFX_MODULE_THREAD_STATE* pState = AfxGetModuleThreadState();
	if (pState->m_pmapHMENU == NULL && bCreate)
	{
		BOOL bEnable = AfxEnableMemoryTracking(FALSE);
#ifndef _AFX_PORTABLE
		_PNH pnhOldHandler = AfxSetNewHandler(&AfxCriticalNewHandler);
#endif
		pState->m_pmapHMENU = new CHandleMap(RUNTIME_CLASS(CMenu),
			ConstructDestruct<CMenu>::Construct, ConstructDestruct<CMenu>::Destruct, 
			offsetof(CMenu, m_hMenu)),

#ifndef _AFX_PORTABLE
		AfxSetNewHandler(pnhOldHandler);
#endif
		AfxEnableMemoryTracking(bEnable);
	}
	return pState->m_pmapHMENU;
}

CMenu* PASCAL CMenu::FromHandle(HMENU hMenu)
{
	CHandleMap* pMap = afxMapHMENU(TRUE); // create map if not exist
	ASSERT(pMap != NULL);
	CMenu* pMenu = (CMenu*)pMap->FromHandle(hMenu);
	ASSERT(pMenu == NULL || pMenu->m_hMenu == hMenu);
	return pMenu;
}

CMenu* PASCAL CMenu::FromHandlePermanent(HMENU hMenu)
{
	CHandleMap* pMap = afxMapHMENU();
	CMenu* pMenu = NULL;
	if (pMap != NULL)
	{
		// only look in the permanent map - does no allocations
		pMenu = (CMenu*)pMap->LookupPermanent(hMenu);
		ASSERT(pMenu == NULL || pMenu->m_hMenu == hMenu);
	}
	return pMenu;
}

/////////////////////////////////////////////////////////////////////////////
// CMenu

#ifdef _DEBUG
void CMenu::AssertValid() const
{
	CObject::AssertValid();
	ASSERT(m_hMenu == NULL || ::IsMenu(m_hMenu));
}

void CMenu::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	dc << "m_hMenu = " << m_hMenu;
	dc << "\n";
}
#endif

BOOL CMenu::Attach(HMENU hMenu)
{
	ASSERT(m_hMenu == NULL);        // only attach once, detach on destroy
	if (hMenu == NULL)
	{
		return FALSE;
	}
    
	// Capture menu in object first to ensure it does not leak if the map cannot be allocated/expanded 
	m_hMenu=hMenu;

	CHandleMap* pMap = afxMapHMENU(TRUE); // create map if not exist
	ASSERT(pMap != NULL);
	pMap->SetPermanent(m_hMenu, this);
	return TRUE;
}

HMENU CMenu::Detach()
{
	HMENU hMenu;
	if ((hMenu = m_hMenu) != NULL)
	{
		CHandleMap* pMap = afxMapHMENU(); // don't create if not exist
		if (pMap != NULL)
			pMap->RemoveHandle(m_hMenu);
	}
	m_hMenu = NULL;
	return hMenu;
}

BOOL CMenu::DestroyMenu()
{
	if (m_hMenu == NULL)
		return FALSE;
	return ::DestroyMenu(Detach());
}

int CMenu::GetMenuString(UINT nIDItem, CString& rString, UINT nFlags) const
{
	ASSERT(::IsMenu(m_hMenu)); 

	// offer no buffer first
	int nStringLen = ::GetMenuString(m_hMenu, nIDItem, NULL, 0, nFlags);

	// use exact buffer length
	if (nStringLen > 0)
	{
		LPTSTR pstrString = rString.GetBufferSetLength(nStringLen+1);
		::GetMenuString(m_hMenu, nIDItem, pstrString, nStringLen+1, nFlags);
		rString.ReleaseBuffer();
	}
	else
		rString.Empty();

	return nStringLen;
}

/////////////////////////////////////////////////////////////////////////////
// Self-drawing menu items
void CMenu::DrawItem(LPDRAWITEMSTRUCT /*pdis*/)
{
	// default does nothing
}

void CMenu::MeasureItem(LPMEASUREITEMSTRUCT /*pmis*/)
{
	// default does nothing
}

CChevronOwnerDrawMenu::CChevronOwnerDrawMenu()
{
	struct AFX_OLDNONCLIENTMETRICS
	{
		UINT    cbSize;
		int     iBorderWidth;
		int     iScrollWidth;
		int     iScrollHeight;
		int     iCaptionWidth;
		int     iCaptionHeight;
		LOGFONT lfCaptionFont;
		int     iSmCaptionWidth;
		int     iSmCaptionHeight;
		LOGFONT lfSmCaptionFont;
		int     iMenuWidth;
		int     iMenuHeight;
		LOGFONT lfMenuFont;
		LOGFONT lfStatusFont;
		LOGFONT lfMessageFont;
	};

	const UINT cbProperSize = (_AfxGetComCtlVersion() < MAKELONG(1, 6))
		? sizeof(AFX_OLDNONCLIENTMETRICS) : sizeof(NONCLIENTMETRICS);

	NONCLIENTMETRICS NonClientMetrics;
	NonClientMetrics.cbSize = cbProperSize;

	::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, cbProperSize, &NonClientMetrics, 0);

	m_MenuFont.CreateFontIndirect(&NonClientMetrics.lfMenuFont);
}

void CChevronOwnerDrawMenu::DrawItem(LPDRAWITEMSTRUCT pdis)
{
	ASSERT(pdis->CtlType == ODT_MENU);

	CString strText(_T(""));
	CSize Size;   

	CDC *pDC;
	pDC = CDC::FromHandle(pdis->hDC);
	int nSave = pDC->SaveDC();
	
	// getting the text (on the right of the bitmap)
	MENUITEMINFO info;
	ZeroMemory(&info, sizeof(MENUITEMINFO));
	info.cbSize = sizeof(MENUITEMINFO);
	info.fMask = MIIM_STRING;
	BOOL bGotText = FALSE;
	if(GetMenuItemInfo(pdis->itemID, &info))
	{
		LPTSTR pszText;

		pszText = strText.GetBuffer(info.cch);
		info.dwTypeData = pszText;
		info.cch++; // space for zero terminator
		bGotText = GetMenuItemInfo(pdis->itemID, &info);
		strText.ReleaseBuffer();
	}

	CBitmap *pBmp;
	pBmp = (CBitmap *) pdis->itemData;

	// Find the rect that will center the bitmap in the menu item
	CRect rc, rcItem(pdis->rcItem);
	BOOL bGotBitmap = FALSE;
	int nBitmapHeight = 0;
	int nBitmapWidth = 0;
	if(pBmp && pBmp->IsKindOf(RUNTIME_CLASS(CBitmap)))
	{
		BITMAP bitmap;
		bGotBitmap = TRUE;
		pBmp->GetObject(sizeof(BITMAP), &bitmap);
		nBitmapHeight = bitmap.bmHeight;
		nBitmapWidth = bitmap.bmWidth;
	}
	else
	{
		// using default icon size
		bGotBitmap = FALSE;
		nBitmapHeight = ::GetSystemMetrics(SM_CYSMICON);
		nBitmapWidth = ::GetSystemMetrics(SM_CXSMICON);
	}
	rc.top = rcItem.Height() / 2 - nBitmapHeight / 2 + rcItem.top - 1;
	rc.left = 0;
	rc.right = nBitmapWidth + 1;
	rc.bottom = nBitmapHeight + 1;
	rc.bottom += rc.top;
	rc.right += rc.left;

	
	//the actual drawing begins
	COLORREF crMenu = ::GetSysColor(COLOR_MENU);
	CDC dcMem;
	dcMem.CreateCompatibleDC(NULL);

	pDC->SelectObject(&m_MenuFont);
	Size = pDC->GetTextExtent(strText);

	// Selected (possibly grayed)
	if(pdis->itemState & ODS_SELECTED)
	{
		// MenuColor
		CRect rcFill(pdis->rcItem);
		rcFill.left = rc.right + 2;
		pDC->FillSolidRect(rcFill, ::GetSysColor(COLOR_HIGHLIGHT));

		// if not grayed and not checked, raise the button
		if (bGotBitmap)
		{
			if(!(pdis->itemState & (ODS_GRAYED | ODS_CHECKED)))
			{
				pDC->Draw3dRect(rc.left, rc.top, rc.Width() + 1, rc.Height() + 1,
					::GetSysColor(COLOR_BTNHIGHLIGHT), ::GetSysColor(COLOR_BTNSHADOW));
			}
		}
		
		// Text
		if (bGotText)
		{
			pDC->SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
			pDC->SetTextColor(
				(pdis->itemState & ODS_GRAYED) ? crMenu : ::GetSysColor(COLOR_HIGHLIGHTTEXT));

			pDC->ExtTextOut(rc.right + 3, rc.top + rc.Height() / 2 - Size.cy / 2,
				ETO_OPAQUE, NULL, strText, NULL);
		}
	}
	else
	{
		pDC->FillSolidRect(&pdis->rcItem, crMenu);
		pDC->SetBkColor(crMenu);

		// Grayed (disabled)
		if(pdis->itemState & ODS_GRAYED)
		{
			pDC->SetTextColor(::GetSysColor(COLOR_3DHILIGHT));
			pDC->SetBkMode(TRANSPARENT);

			// Text
			if (bGotText)
			{
				pDC->ExtTextOut(rc.right + 4, rc.top + rc.Height() / 2 - Size.cy / 2 + 1, ETO_OPAQUE, NULL, strText, NULL);
				pDC->SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
				pDC->ExtTextOut(rc.right + 3, rc.top + rc.Height() / 2 - Size.cy / 2, 0, NULL, strText, NULL);
			}
		}
		// Everything else
		else
		{
			// if checked draw as pushed
			if (bGotBitmap)
			{
				if(pdis->itemState & ODS_CHECKED)
				{
					pDC->Draw3dRect(rc.left, rc.top, rc.Width() + 1, rc.Height() + 1,
						::GetSysColor(COLOR_BTNSHADOW), ::GetSysColor(COLOR_BTNHIGHLIGHT));
				}
			}
			
			// text
			if (bGotText)
			{
				pDC->SetBkColor(crMenu);
				pDC->SetTextColor(::GetSysColor(COLOR_MENUTEXT));
				pDC->ExtTextOut(rc.right + 3, rc.top + rc.Height() / 2 - Size.cy / 2, ETO_OPAQUE, NULL, strText, NULL);
			}
		}
	}

	// The bitmap...
	if (bGotBitmap)
	{
		CBitmap bmp;
		int x = 0, y = 0;
		if(pdis->itemState & ODS_GRAYED)
		{
			::AfxGetGrayBitmap(*pBmp, &bmp, crMenu);
			pBmp = &bmp;
		}
		else
		{
			if(pdis->itemState & ODS_CHECKED)
			{
				::AfxGetDitheredBitmap(*pBmp, &bmp, crMenu, RGB(255, 255, 255));
				pBmp = &bmp;
			}
		}

		CDC dcMem;
		dcMem.CreateCompatibleDC(NULL);
		dcMem.SelectObject(pBmp);

		rc.InflateRect(-1, -1);

		pDC->BitBlt(rc.left, rc.top, rc.right, rc.bottom, &dcMem, x, y, SRCCOPY);
	}
	
	pDC->RestoreDC(nSave);
}

void CChevronOwnerDrawMenu::MeasureItem(LPMEASUREITEMSTRUCT pmis)
{
	// measuring the bitmp
	int nWidth =0;
	int nHeight =0;

	if(pmis->itemData)
	{
		CBitmap *pBmp = (CBitmap *) pmis->itemData;
		ASSERT_KINDOF(CBitmap, pBmp);

		BITMAP bitmap;
		
		pBmp->GetBitmap(&bitmap);
		
		// 2 pixel of padding
		nHeight=bitmap.bmHeight + 2;
		nWidth += bitmap.bmWidth+2;
	}
	else
	{
		// use the default size of the pixels
		// 2 pixel of padding
		nHeight = ::GetSystemMetrics(SM_CYSMICON) + 2;
		nWidth = ::GetSystemMetrics(SM_CXSMICON) + 2;
	}

	// measuring the text
	CString strText;
	LPTSTR pszText;
	MENUITEMINFO info;

	ZeroMemory(&info, sizeof(MENUITEMINFO));
	info.cbSize = sizeof(MENUITEMINFO);
	info.fMask = MIIM_STRING;
	if(GetMenuItemInfo(pmis->itemID, &info))
	{
		pszText = strText.GetBuffer(info.cch);
		info.dwTypeData = pszText;
		info.cch++;
		BOOL bCheck = GetMenuItemInfo(pmis->itemID, &info);
		strText.ReleaseBuffer();

		if(bCheck)
		{
			CWindowDC dc(NULL);

			CFont *pFont = dc.SelectObject(&m_MenuFont);
			CSize size = dc.GetTextExtent(strText);
			dc.SelectObject(pFont);

			// 1 pixel for gap, 2 for menu item = 3
			// (disregard checkmark gap)
			nWidth +=  size.cx + 3;
		}
	}

	//setting the info
	pmis->itemHeight= max(::GetSystemMetrics(SM_CYMENU), nHeight);
	pmis->itemWidth = nWidth;
}


IMPLEMENT_DYNCREATE(CMenu, CObject)

/////////////////////////////////////////////////////////////////////////////
