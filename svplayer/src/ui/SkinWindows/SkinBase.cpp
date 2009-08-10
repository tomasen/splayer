// SkinBase.cpp: implementation of the CSkinBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SkinBase.h"
#include "wclassdefines.h"
#include "winclasses.h"

//#define ACTIVATE_VIEWER
//#include "imageviewer.h"

#include <afxpriv.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// for transparency
typedef BOOL (WINAPI *LPSetLayeredWindowAttributes)
(
  HWND hwnd,           // handle to the layered window
  COLORREF crKey,      // specifies the color key
  BYTE bAlpha,         // value for the blend function
  DWORD dwFlags        // action
);

#ifndef SPI_SETMENUFADE
	#define SPI_GETMENUFADE 0x1012
	#define SPI_SETMENUFADE 0x1013
#endif

#ifndef WS_EX_LAYERED
	#define WS_EX_LAYERED           0x00080000

	// win 2000 layered windows support
	#define LWA_COLORKEY            0x00000001
	#define LWA_ALPHA               0x00000002

	#define ULW_COLORKEY            0x00000001
	#define ULW_ALPHA               0x00000002
	#define ULW_OPAQUE              0x00000004
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

int CSkinBase::s_nOSVer = -1;
PFNTRANSPARENTBLT CSkinBase::s_pfnFastTransparentBlt = NULL;
PFNGRADIENTFILL CSkinBase::s_pfnFastGradientFill = NULL;
BOOL CSkinBase::s_bThemingEnabled = (GetOS() >= SBOS_XP);

// don't use fast trasparent blt on win95/98 because msimg32.dll has a resource leak
BOOL CSkinBase::s_bSupportsFastTransparentBlt = (GetOS() < SBOS_ME) ? FALSE : -1;
BOOL CSkinBase::s_bSupportsFastGradientFill = (GetOS() < SBOS_ME) ? FALSE : -1;

CSkinBase::CSkinBase()
{
}

CSkinBase::~CSkinBase()
{
}

int CSkinBase::GetOS()
{
	if (s_nOSVer == -1) // first time
	{
		OSVERSIONINFO vinfo;
		vinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		
		BOOL rslt = GetVersionEx(&vinfo);
		
		if (rslt)
		{
			switch (vinfo.dwPlatformId)
			{
			case VER_PLATFORM_WIN32_NT:
				switch (vinfo.dwMajorVersion)
				{
				case 3: // nt351
					ASSERT (0); // not supported
					break;
					
				case 4: // nt4
					s_nOSVer = SBOS_NT4;
					break;
					
				case 5: // >= w2k
					switch (vinfo.dwMinorVersion)
					{
					case 0: // w2k
						s_nOSVer = SBOS_2K;
						break;
						
					case 1: // xp
						s_nOSVer = SBOS_XP;
						break;
						
					default: // > xp
						s_nOSVer = SBOS_XPP;
						break;
					}
					break;
					
					default: // > xp
						s_nOSVer = SBOS_XPP;
						break;
				}
				break;
				
				case VER_PLATFORM_WIN32_WINDOWS:
					ASSERT (vinfo.dwMajorVersion == 4);
					
					switch (vinfo.dwMinorVersion)
					{
					case 0: // nt4
						s_nOSVer = SBOS_95;
						break;
						
					case 10: // xp
						s_nOSVer = SBOS_98;
						break;
						
					case 90: // > xp
						s_nOSVer = SBOS_ME;
						break;
						
					default:
						ASSERT (0);
						break;
					}
					break;
					
					default:
						ASSERT (0);
						break;
			}
		}
	}
	
	return s_nOSVer;
}

BOOL CSkinBase::SupportsFastTransparentBlt()
{
	if (s_bSupportsFastTransparentBlt == -1) // first time
	{
		HINSTANCE hInst = LoadLibrary(L"msimg32.dll");
		
		if (hInst)
		{
			s_pfnFastTransparentBlt = (PFNTRANSPARENTBLT)GetProcAddress(hInst, "TransparentBlt");
			s_bSupportsFastTransparentBlt = (s_pfnFastTransparentBlt != NULL);
		}
		else
			s_bSupportsFastTransparentBlt = FALSE;
	}
	
	return s_bSupportsFastTransparentBlt;
}

BOOL CSkinBase::SupportsFastGradientFill()
{
	if (s_bSupportsFastGradientFill == -1) // first time
	{
		HINSTANCE hInst = LoadLibrary(L"msimg32.dll");
		
		if (hInst)
		{
			s_pfnFastGradientFill = (PFNGRADIENTFILL)GetProcAddress(hInst, "GradientFill");
			s_bSupportsFastGradientFill = (s_pfnFastGradientFill != NULL);
		}
		else
			s_bSupportsFastGradientFill = FALSE;
	}
	
	return s_bSupportsFastGradientFill;
}

BOOL CSkinBase::GradientFill(CDC* pDCDest, LPRECT lpRect, COLORREF crFrom, COLORREF crTo, BOOL bHorz)
{
	if (!lpRect)
		return FALSE;

	if (::IsRectEmpty(lpRect))
		return FALSE;

	if (crFrom == crTo)
	{
		pDCDest->FillSolidRect(lpRect, crFrom);
		return TRUE;
	}

	if (GradientFillFast(pDCDest, lpRect, crFrom, crTo, bHorz))
		return TRUE;

	// else
	return GradientFillSlow(pDCDest, lpRect, crFrom, crTo, bHorz);
}

BOOL CSkinBase::TransparentBlt(CDC* pDCDest, 
							   int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest,
							   CDC* pDCSrc, 
							   int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc, 	
							   UINT crTransparent)
{
	if (nWidthDest < 1) 
		return FALSE;
	
	if (nWidthSrc < 1) 
		return FALSE; 
	
	if (nHeightDest < 1) 
		return FALSE;
	
	if (nHeightSrc < 1) 
		return FALSE;
	
	if (TransparentBltFast(pDCDest, 
		nXOriginDest, nYOriginDest, nWidthDest, nHeightDest,
		pDCSrc, 
		nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc,	 
		crTransparent))
		return TRUE;
	
	// else 
	return TransparentBltSlow(pDCDest, 
		nXOriginDest, nYOriginDest, nWidthDest, nHeightDest,
		pDCSrc, 
		nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc,	 
		crTransparent);
}

BOOL CSkinBase::TransparentBltFast(CDC* pDCDest, 		
								   int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest,	
								   CDC* pDCSrc, 		 
								   int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc,	 
								   UINT crTransparent) 
{
	if (!SupportsFastTransparentBlt() || !s_pfnFastTransparentBlt)
		return FALSE;
	
	return s_pfnFastTransparentBlt(*pDCDest, 
									nXOriginDest, 
									nYOriginDest, 
									nWidthDest, 
									nHeightDest,
									*pDCSrc, 
									nXOriginSrc, 
									nYOriginSrc, 
									nWidthSrc, 
									nHeightSrc,	   
									crTransparent);
}

BOOL CSkinBase::TransparentBltSlow(CDC* pDCDest, 		
								   int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest,	
								   CDC* pDCSrc, 		 
								   int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc,	 
								   UINT crTransparent) 
{
	CDC dcMem, dcMask;

	dcMask.CreateCompatibleDC(pDCDest);
	dcMem.CreateCompatibleDC(pDCDest);

	CBitmap bmMask, bmMem;

	// copy src bitmap to mem dc
	bmMem.CreateCompatibleBitmap(pDCDest, nWidthSrc, nHeightSrc);

	CBitmap* pOldBMMem = dcMem.SelectObject(&bmMem);
	dcMem.BitBlt(0, 0, nWidthSrc, nHeightSrc, pDCSrc, nXOriginSrc, nYOriginSrc, SRCCOPY);

//	ShowDC(dcMem);
	
	// Create monochrome bitmap for the mask
	bmMask.CreateBitmap(nWidthSrc, nHeightSrc, 1, 1, NULL);
	CBitmap* pOldBMMask = dcMask.SelectObject(&bmMask);
	dcMem.SetBkColor(crTransparent);
	
	// Create the mask from the memory DC
	dcMask.BitBlt(0, 0, nWidthSrc, nHeightSrc, &dcMem, 0, 0, SRCCOPY);

//	ShowDC(dcMask);

	// Set the background in dcMem to black. Using SRCPAINT with black
	// and any other color results in the other color, thus making
	// black the transparent color
	dcMem.SetBkColor(RGB(0,0,0));
	dcMem.SetTextColor(RGB(255,255,255));
	dcMem.BitBlt(0, 0, nWidthSrc, nHeightSrc, &dcMask, 0, 0, SRCAND);

//	ShowDC(dcMem);
	
	// Set the foreground to black. See comment above.
//	pDCDest->SetStretchBltMode(COLORONCOLOR);
	pDCDest->SetStretchBltMode(HALFTONE);
	pDCDest->SetBkColor(RGB(255,255,255));
	pDCDest->SetTextColor(RGB(0,0,0));
	
//	CPoint ptOrg;
//	::GetBrushOrgEx(*pDCDest, &ptOrg);
//	::SetBrushOrgEx(*pDCDest, 0, 0, &ptOrg);

	if (nWidthDest == nWidthSrc && nHeightDest == nHeightSrc)
	{
		pDCDest->BitBlt(nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, &dcMask, 0, 0, SRCAND);
//		ShowDC(*pDCDest);
	}
	else
	{
		pDCDest->StretchBlt(nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, 
						&dcMask, 0, 0, nWidthSrc, nHeightSrc, SRCAND);
//		ShowDC(*pDCDest);
	}
	
	// Combine the foreground with the background
	if (nWidthDest == nWidthSrc && nHeightDest == nHeightSrc)
	{
		pDCDest->BitBlt(nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, &dcMem, 0, 0, SRCPAINT);
//		ShowDC(*pDCDest);
	}
	else
	{
		pDCDest->StretchBlt(nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, 
						&dcMem, 0, 0, nWidthSrc, nHeightSrc, SRCPAINT);
//		ShowDC(*pDCDest);
	}

	dcMask.SelectObject(pOldBMMask);
	dcMem.SelectObject(pOldBMMem);

	return TRUE;
}
/*
BOOL CSkinBase::TransparentBltSlow(CDC* pDCDest, 		
								   int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest,	
								   CDC* pDCSrc, 		 
								   int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc,	 
								   UINT crTransparent) 
{
	CDC dcMem, dcMask;

	dcMask.CreateCompatibleDC(pDCDest);
	dcMem.CreateCompatibleDC(pDCDest);

	CBitmap bmMask, bmMem;

	// copy src bitmap to mem dc
	bmMem.CreateCompatibleBitmap(pDCDest, nWidthDest, nHeightDest);

	CBitmap* pOldBMMem = dcMem.SelectObject(&bmMem);

	dcMem.SetStretchBltMode(HALFTONE);
	CPoint ptOrg;
	::GetBrushOrgEx(dcMem, &ptOrg);
	::SetBrushOrgEx(dcMem, 0, 0, &ptOrg);

	dcMem.StretchBlt(0, 0, nWidthDest, nHeightDest, 
					pDCSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, SRCCOPY);

//	ShowDC(dcMem);
	
	// Create monochrome bitmap for the mask
	bmMask.CreateBitmap(nWidthDest, nHeightDest, 1, 1, NULL);
	CBitmap* pOldBMMask = dcMask.SelectObject(&bmMask);
	dcMem.SetBkColor(crTransparent);
	
	// Create the mask from the memory DC
	dcMask.BitBlt(0, 0, nWidthDest, nHeightDest, &dcMem, 0, 0, SRCCOPY);

//	ShowDC(dcMask);

	// Set the background in dcMem to black. Using SRCPAINT with black
	// and any other color results in the other color, thus making
	// black the transparent color
	dcMem.SetBkColor(RGB(0,0,0));
	dcMem.SetTextColor(RGB(255,255,255));
	dcMem.BitBlt(0, 0, nWidthDest, nHeightDest, &dcMask, 0, 0, SRCAND);

//	ShowDC(dcMem);
	
	// Set the foreground to black. See comment above.
	pDCDest->SetBkColor(RGB(255,255,255));
	pDCDest->SetTextColor(RGB(0,0,0));
	
	pDCDest->BitBlt(nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, &dcMask, 0, 0, SRCAND);
//	ShowDC(*pDCDest);
	
	// Combine the foreground with the background
	pDCDest->BitBlt(nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, &dcMem, 0, 0, SRCPAINT);
//	ShowDC(*pDCDest);

	dcMask.SelectObject(pOldBMMask);
	dcMem.SelectObject(pOldBMMem);

	return TRUE;
}
*/

BOOL CSkinBase::GradientFillFast(CDC* pDCDest, LPRECT lpRect, COLORREF crFrom, COLORREF crTo, BOOL bHorz)
{
	if (!SupportsFastGradientFill() || !s_pfnFastGradientFill)
		return FALSE;
	
	TRIVERTEX vert[2];
	vert[0].x      = lpRect->left;
	vert[0].y      = lpRect->top;
	vert[0].Red    = GetRValue(crFrom) << 8;
	vert[0].Green  = GetGValue(crFrom) << 8;
	vert[0].Blue   = GetBValue(crFrom) << 8;
	vert[0].Alpha  = 0x0000;

	vert[1].x      = lpRect->right;
	vert[1].y      = lpRect->bottom; 
	vert[1].Red    = GetRValue(crTo) << 8;
	vert[1].Green  = GetGValue(crTo) << 8;
	vert[1].Blue   = GetBValue(crTo) << 8;
	vert[1].Alpha  = 0x0000;

	GRADIENT_RECT gRect = { 0, 1 };

	return s_pfnFastGradientFill(*pDCDest, 
									vert,
									2,
									&gRect,
									1,
									bHorz ? GRADIENT_FILL_RECT_H : GRADIENT_FILL_RECT_V);
}

BOOL CSkinBase::GradientFillSlow(CDC* pDCDest, LPRECT lpRect, COLORREF crFrom, COLORREF crTo, BOOL bHorz)
{
	if (!pDCDest || !lpRect)
		return FALSE;

	int nWidth = lpRect->right - lpRect->left;
	int nHeight = lpRect->bottom - lpRect->top;

	if (bHorz)
	{
		for (int nX = lpRect->left; nX < lpRect->right; nX++)
			pDCDest->FillSolidRect(nX, lpRect->top, 1, nHeight, BlendColors(crFrom, crTo, (lpRect->right - nX) / (float)nWidth));
	}
	else
	{
		for (int nY = lpRect->top; nY < lpRect->bottom; nY++)
			pDCDest->FillSolidRect(lpRect->left, nY, nWidth, 1, BlendColors(crFrom, crTo, (lpRect->bottom - nY) / (float)nHeight));
	}

	return TRUE;
}

COLORREF CSkinBase::BlendColors(COLORREF crA, COLORREF crB, float fAmountA)
{
	BYTE btRed = (BYTE)min(255, (int)(GetRValue(crA) * fAmountA + GetRValue(crB) * (1.0f - fAmountA)));
	BYTE btGreen = (BYTE)min(255, (int)(GetGValue(crA) * fAmountA + GetGValue(crB) * (1.0f - fAmountA)));
	BYTE btBlue = (BYTE)min(255, (int)(GetBValue(crA) * fAmountA + GetBValue(crB) * (1.0f - fAmountA)));
	
	return RGB(btRed, btGreen, btBlue);
}

COLORREF CSkinBase::VaryColor(COLORREF crColor, float fFactor)
{
	BYTE btRed = (BYTE)min(255, (int)(GetRValue(crColor) * fFactor));
	BYTE btGreen = (BYTE)min(255, (int)(GetGValue(crColor) * fFactor));
	BYTE btBlue = (BYTE)min(255, (int)(GetBValue(crColor) * fFactor));
	
	return RGB(btRed, btGreen, btBlue);
}

HRGN CSkinBase::BitmapToRegion(CBitmap* pBmp, COLORREF color)
{
	const DWORD RGNDATAHEADER_SIZE	= sizeof(RGNDATAHEADER);
	const DWORD ADD_RECTS_COUNT 	= 40;			// number of rects to be appended
	
	// get image properties
	BITMAP BM = { 0 };
	pBmp->GetBitmap(&BM);
	
	// create temporary dc
	CBitmap bmpMem;
	CDC dc, dcMem1, dcMem2;
	CDC* pDC = CWnd::GetDesktopWindow()->GetDC();
	
	dcMem1.CreateCompatibleDC(pDC);
	dcMem2.CreateCompatibleDC(pDC);
	bmpMem.CreateCompatibleBitmap(pDC, BM.bmWidth, BM.bmHeight);
	
	CWnd::GetDesktopWindow()->ReleaseDC(pDC);
	
	CBitmap* pOldBM1 = dcMem1.SelectObject(pBmp);
	CBitmap* pOldBM2 = dcMem2.SelectObject(&bmpMem);

	// verify that the mask color is correct for the current bit depth
	color = dcMem2.SetPixel(0, 0, color);
	
	dcMem2.BitBlt(0, 0, BM.bmWidth, BM.bmHeight, &dcMem1, 0, 0, SRCCOPY);
	dcMem1.SelectObject(pOldBM1);
	dcMem1.DeleteDC();
	
	DWORD	dwRectsCount = BM.bmHeight; 		// number of rects in allocated buffer
	int 	nY, nX; 								// current position in mask image
	// where mask was found
	bool	bWasMask;						// set when mask has been found in current scan line
	bool	bIsMask;								// set when current color is mask color
	CRect	rLine;
	
	// allocate memory for region data
	// region data here is set of regions that are rectangles with height 1 pixel (scan line)
	// that's why nRgnStart allocation is <bm.biHeight> RECTs - number of scan lines in image
	RGNDATAHEADER* pRgnData = (RGNDATAHEADER*)new BYTE[ RGNDATAHEADER_SIZE + dwRectsCount * sizeof(RECT) ];
	
	// get pointer to RECT table
	LPRECT pRects = (LPRECT)((LPBYTE)pRgnData + RGNDATAHEADER_SIZE);
	
	// zero region data header memory (header  part only)
	memset( pRgnData, 0, RGNDATAHEADER_SIZE + dwRectsCount * sizeof(RECT) );
	
	// fill it by default
	pRgnData->dwSize	= RGNDATAHEADER_SIZE;
	pRgnData->iType 	= RDH_RECTANGLES;
	
	for ( nY = 0; nY < BM.bmHeight; nY++ )
	{
		bWasMask = true;
		rLine.SetRect(0, nY, 0, nY + 1);
		
		for ( nX = 0; nX < BM.bmWidth; nX++ )
		{
			// get color
			COLORREF crPixel = dcMem2.GetPixel(nX, nY);
			bIsMask = (crPixel == color);
			
			if (!bIsMask && bWasMask) // start of the rgn
			{
				rLine.left = nX;
				bWasMask = FALSE;
			}
			
			if (!bWasMask && (bIsMask || nX == BM.bmWidth - 1)) // end of rgn
			{
				bWasMask = true;
				rLine.right = bIsMask ? nX : nX + 1;
				
				// save current RECT
				// if this was a full line append to the last if it was full too
				BOOL bAdded = FALSE;
				
				if (pRgnData->nCount)
				{
					LPRECT pLastRect = &pRects[ pRgnData->nCount - 1];
					
					if (!pLastRect->left && !rLine.left && 
						pLastRect->right == BM.bmWidth - 1 && rLine.right == BM.bmWidth - 1)
					{
						pLastRect->bottom = rLine.bottom;
						bAdded = TRUE;
					}
				}
				
				// else add as a new line
				if (!bAdded)
				{
					pRects[ pRgnData->nCount++ ] = rLine;
					
					// if buffer full reallocate it with more room
					if ( pRgnData->nCount >= dwRectsCount )
					{
						dwRectsCount += ADD_RECTS_COUNT;
						
						// allocate new buffer
						LPBYTE pRgnDataNew = new BYTE[ RGNDATAHEADER_SIZE + dwRectsCount * sizeof(RECT) ];
						
						// copy current region data to it
						memcpy( pRgnDataNew, pRgnData, RGNDATAHEADER_SIZE + pRgnData->nCount * sizeof(RECT) );
						
						// delte old region data buffer
						delete pRgnData;
						
						// set pointer to new regiondata buffer to current
						pRgnData = (RGNDATAHEADER*)pRgnDataNew;
						
						// correct pointer to RECT table
						pRects = (LPRECT)((LPBYTE)pRgnData + RGNDATAHEADER_SIZE);
					}
				}
			}
		}
	}
	
	// create region
	HRGN hRgn = ExtCreateRegion( NULL, RGNDATAHEADER_SIZE + pRgnData->nCount * sizeof(RECT), (LPRGNDATA)pRgnData );
	CRect rBox;
	
	::GetRgnBox(hRgn, rBox);
	
	// release region data
	delete pRgnData;
	
	dcMem2.SelectObject(pOldBM2);
	dcMem2.DeleteDC();
	bmpMem.DeleteObject();
	
	return hRgn;
}

HMENU CSkinBase::MakeMenuCopy(const CMenu* pSrc)
{
	if (!pSrc)
		return NULL;
	
	CMenu menu;
	
	VERIFY (menu.CreatePopupMenu());
	ASSERT (::IsMenu(menu.m_hMenu));
	
	int nNumItems = pSrc->GetMenuItemCount();
	CString sLabel;
	
	MENUITEMINFO mii;
	ZeroMemory(&mii, sizeof(mii));
	mii.cbSize = sizeof(mii); // must fill up this field
	mii.fMask = MIIM_STATE | MIIM_DATA; 			// get the state of the menu item
				
	for (int nItem = 0; nItem < nNumItems; nItem++)
	{
		UINT uIDItem = pSrc->GetMenuItemID(nItem);
		pSrc->GetMenuString(nItem, sLabel, MF_BYPOSITION);
		UINT uFlags = (uIDItem == 0) ? MF_SEPARATOR : (uIDItem == (UINT)-1) ? MF_POPUP : MF_STRING;
		
		// special case: if a popup menu we must copy it too
		if (uFlags == MF_POPUP)
		{
			HMENU hPopup = MakeMenuCopy(pSrc->GetSubMenu(nItem));
			ASSERT (hPopup);
			
			uIDItem = (UINT)hPopup;
		}
		
		menu.AppendMenu(uFlags, uIDItem, sLabel);
		
		// make sure we copy the state too
		::GetMenuItemInfo(*pSrc, nItem, TRUE, &mii);
		::SetMenuItemInfo(menu, nItem, TRUE, &mii);
	}
	
	return menu.Detach();
}

// this one copies the menu without deleting the root
BOOL CSkinBase::CopyMenu(const CMenu* pSrc, CMenu* pDest)
{
	ASSERT (::IsMenu(pDest->m_hMenu));
	
	if (!::IsMenu(pDest->m_hMenu))
		return FALSE;
	
	ASSERT (::IsMenu(pSrc->m_hMenu));
	
	if (!::IsMenu(pSrc->m_hMenu))
		return FALSE;
	
	// delete all the existing items
	while (pDest->GetMenuItemCount())
		pDest->DeleteMenu(0, MF_BYPOSITION);
	
	// copy across
	int nNumItems = pSrc->GetMenuItemCount();
	CString sLabel;
	
	MENUITEMINFO mii;
	ZeroMemory(&mii, sizeof(mii));
	mii.cbSize = sizeof(mii); // must fill up this field
	mii.fMask = MIIM_STATE | MIIM_DATA; 			// get the state of the menu item
				
	for (int nItem = 0; nItem < nNumItems; nItem++)
	{
		UINT uIDItem = pSrc->GetMenuItemID(nItem);
		pSrc->GetMenuString(nItem, sLabel, MF_BYPOSITION);
		UINT uFlags = (uIDItem == 0) ? MF_SEPARATOR : (uIDItem == (UINT)-1) ? MF_POPUP : MF_STRING;
		
		// special case: if a popup menu we must copy it too
		if (uFlags == MF_POPUP)
		{
			HMENU hPopup = MakeMenuCopy(pSrc->GetSubMenu(nItem));
			ASSERT (hPopup);
			
			uIDItem = (UINT)hPopup;
		}
		
		pDest->AppendMenu(uFlags, uIDItem, sLabel);
		
		// make sure we copy the state too
		::GetMenuItemInfo(*pSrc, nItem, TRUE, &mii);
		::SetMenuItemInfo(*pDest, nItem, TRUE, &mii);
	}
	
	return TRUE;
}

BOOL CSkinBase::CopyBitmap(const CBitmap* pSrc, CBitmap* pDest)
{
	ASSERT (pDest);
	
	if (!pDest)
		return FALSE;
	
	pDest->DeleteObject();
	
	if (!pSrc || !pSrc->GetSafeHandle())
		return FALSE;
	
	CDC* pDC = CWnd::GetDesktopWindow()->GetDC();
	CDC dcMem1, dcMem2;
	BOOL bRes = FALSE;
	
	if (dcMem1.CreateCompatibleDC(pDC) && dcMem2.CreateCompatibleDC(pDC))
	{
		BITMAP bm;
		((CBitmap*)pSrc)->GetBitmap(&bm);
		
		if (pDest->CreateCompatibleBitmap(pDC, bm.bmWidth, bm.bmHeight))
		{
			ASSERT (CBitmap::FromHandle((HBITMAP)pDest->GetSafeHandle()) == pDest);
			
			CBitmap* pOldBM1 = dcMem1.SelectObject((CBitmap*)pSrc);
			CBitmap* pOldBM2 = dcMem2.SelectObject(pDest);
			
			dcMem2.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &dcMem1, 0, 0, SRCCOPY);
			bRes = TRUE;
			
			dcMem1.SelectObject(pOldBM1);
			dcMem2.SelectObject(pOldBM2);
			
		}
	}
	
	dcMem1.DeleteDC();
	dcMem2.DeleteDC();
	
	CWnd::GetDesktopWindow()->ReleaseDC(pDC);
	
	ASSERT (CBitmap::FromHandle((HBITMAP)pDest->GetSafeHandle()) == pDest);
	HANDLE* ph = (HANDLE*)((BYTE*)pDest + 4);  // after CObject
	ASSERT (ph[0] == pDest->GetSafeHandle());
	
	return bRes;
}

BOOL CSkinBase::ExtractResource(UINT nID, LPCTSTR szType, CString& sTempFilePath, HINSTANCE hInst)
{
	if (!hInst)
		hInst = AfxFindResourceHandle(MAKEINTRESOURCE(nID), szType);
	
	if (!hInst)
		return FALSE;
	
	// compare time with that of module from which it was loaded
	CString sTempPath;
	CFileStatus fsRes, fsModule;
	CString sModulePath;
	::GetModuleFileName(hInst, sModulePath.GetBuffer(MAX_PATH + 1), MAX_PATH);
	sModulePath.ReleaseBuffer();
	
	if (!CFile::GetStatus(sModulePath, fsModule))
		return FALSE;
	
	// create temp filename
	::GetTempPath(MAX_PATH, sTempPath.GetBuffer(MAX_PATH));
	sTempPath.ReleaseBuffer();
	sTempFilePath.Format(L"%s%s_skin_%d.tmp", sTempPath, szType, nID);
	
	// see if the file has been created before
	if (!CFile::GetStatus(sTempFilePath, fsRes) || fsRes.m_mtime < fsModule.m_mtime)
	{
		// Load the resource into memory
		HRSRC hRes = FindResource(hInst, (LPCWSTR)nID, szType);
		
		if (!hRes) 
		{
			TRACE("Couldn't find %s resource %d!\n", szType, nID);
			return FALSE;
		}
		
		DWORD len = SizeofResource(hInst, hRes);
		
		BYTE* lpRes = (BYTE*)LoadResource(hInst, hRes);
		ASSERT(lpRes);
		
		CFile file;
		
		if (file.Open(sTempFilePath, CFile::modeCreate | CFile::modeWrite))
		{
			file.Write(lpRes, len);
			file.Close();
			FreeResource((HANDLE)lpRes);
		}
		else
		{
			FreeResource((HANDLE)lpRes);
			return FALSE;
		}
	}
	
	return TRUE;
}

CWnd* CSkinBase::GetChildWnd(CWnd* pParent, LPCTSTR szClass, int nID)
{
	CWnd* pChild = pParent->GetWindow(GW_CHILD); 
	
	while (pChild)	  
	{
		if (CWinClasses::IsClass(*pChild, szClass))
		{
			if (nID == -1 || pChild->GetDlgCtrlID() == nID)
				return pChild;
		}
		pChild = pChild->GetNextWindow();
	}
	
	return NULL;
}

BOOL CSkinBase::BitBlt(CDC* pDCDest, 
					   int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest,
					   CDC* pDCSrc, int nXOriginSrc, int nYOriginSrc, 
					   UINT uROP,	   
					   COLORREF crTransparent)
{
	if (crTransparent != (COLORREF)-1)
	{
		return TransparentBlt(pDCDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest,
							pDCSrc, nXOriginSrc, nYOriginSrc, nWidthDest, nHeightDest, crTransparent);
	}
	else
	{
		return pDCDest->BitBlt(nXOriginDest, nYOriginDest, nWidthDest, nHeightDest,
							pDCSrc, nXOriginSrc, nYOriginSrc, uROP);
	}
	
	return TRUE;
}

BOOL CSkinBase::StretchBlt(CDC* pDCDest, 
						   int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest,
						   CDC* pDCSrc, int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc,	  
						   UINT uROP,	   
						   COLORREF crTransparent)
{
	// check to see if this is really just a BitBlt
	if (nWidthDest == nWidthSrc && nHeightDest == nHeightSrc)
	{
		return BitBlt(pDCDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest,
					   pDCSrc, nXOriginSrc, nYOriginSrc, uROP, crTransparent);
	}

	// else pick whether its transparent or not
	if (crTransparent != (COLORREF)-1)
	{
		return TransparentBlt(pDCDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest,
								pDCSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, crTransparent);
	}
	else
	{
		return pDCDest->StretchBlt(nXOriginDest, nYOriginDest, nWidthDest, nHeightDest,
								pDCSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, uROP);
	}
	
	return TRUE;
}

CString CSkinBase::GetTipText(LPCTSTR szText, BOOL bToolbar)
{
	CString sText(szText), sTip;
	
	if (sText.IsEmpty())
		return "";
	
	// tip text starts at '\n' 
	int nStartTip = bToolbar ? sText.Find('\n') : -1;
	
	if (bToolbar && nStartTip == -1) // no tip
		return "";
	
	sText = sText.Right(sText.GetLength() - nStartTip - 1);
	
	// strip '&' and '...' if present
	int nLen = sText.GetLength();
	sTip.Empty();
	
	for (int nPos = 0; nPos < nLen; nPos++)
	{
		if (sText[nPos] != '&' && sText[nPos] != '.')
			sTip += sText[nPos];
	}
	
	return sTip;
}

BOOL CSkinBase::ConvertToGrayScale(CBitmap* pBitmap, COLORREF crMask)
{
	CDC dcTemp;
	dcTemp.CreateCompatibleDC(NULL);
	
	BITMAP BM;
	pBitmap->GetBitmap(&BM);
	
	CBitmap* pBMOld = dcTemp.SelectObject(pBitmap);
	
	// now iterate all the pixels, converting each to grayscale
	// a bit daggy, but....
	int nXPixel = BM.bmWidth;
	COLORREF crPixel;
	
	while (nXPixel--)
	{
		int nYPixel = BM.bmHeight;
		
		while (nYPixel--)
		{
			crPixel = dcTemp.GetPixel(nXPixel, nYPixel);
			
			// leave the mask color as-is
			if (crPixel == crMask)
				continue;
			
			const BYTE btRed = GetRValue(crPixel);
			const BYTE btGreen = GetGValue(crPixel);
			const BYTE btBlue = GetBValue(crPixel);
			
			// if gray already goto next
			if (btRed == btGreen && btGreen == btBlue)
				continue;
			
			const BYTE btGray = (BYTE)((btRed / 3) + (btGreen / 2) + (btBlue / 4));
			
			// note: SetPixelV() quicker than SetPixel()
			dcTemp.SetPixelV(nXPixel, nYPixel, RGB(btGray, btGray, btGray));
		}
	}
	
	dcTemp.SelectObject(pBMOld);
	dcTemp.DeleteDC();
	
	return TRUE;
}

BOOL CSkinBase::DoSysMenu(CWnd* pWnd, CPoint ptCursor, LPRECT prExclude, BOOL bCopy)
{
	CMenu* pMenu = pWnd->GetSystemMenu(FALSE);
	ASSERT (pMenu);
	
	if (pMenu)
	{
		TPMPARAMS tpmp;
		tpmp.cbSize = sizeof(tpmp);

		if (prExclude)
			tpmp.rcExclude = *prExclude;
		else
			SetRectEmpty(&tpmp.rcExclude);

		UINT uAlignFlags = TPM_LEFTALIGN | TPM_TOPALIGN | TPM_VERTICAL | TPM_RIGHTBUTTON | TPM_RETURNCMD;
		UINT uID = 0;
		
		if (bCopy) // skinning
		{
			HMENU hSysMenu = CSkinBase::MakeMenuCopy(pMenu);
			ASSERT (hSysMenu);
			
			if (hSysMenu)
			{
				InitSysMenu(CMenu::FromHandle(hSysMenu), pWnd);
				uID = ::TrackPopupMenuEx(hSysMenu, uAlignFlags, 
										ptCursor.x, ptCursor.y, *pWnd, &tpmp);
				
				::DestroyMenu(hSysMenu); // cleanup
			}
		}
		else
		{
			InitSysMenu(pMenu, pWnd);
			uID = ::TrackPopupMenuEx(pMenu->GetSafeHmenu(), uAlignFlags, 
									ptCursor.x, ptCursor.y, *pWnd, &tpmp);
		}
		
		if (uID & 0xf000) // syscommand
		{
			MSG& curMsg = AfxGetThreadState()->m_lastSentMsg;
			
			// always post this command to allow this function to unwind
			// correctly before the command is handled
			pWnd->PostMessage(WM_SYSCOMMAND, (uID & 0xfff0), MAKELPARAM(curMsg.pt.x, curMsg.pt.y));
		}
	}
	
	return TRUE;
}

void CSkinBase::InitSysMenu(CMenu* pMenu, CWnd* pWnd)
{
	// iterate all the menu items looking for something resembling a sys menu item
	int nItem = pMenu->GetMenuItemCount();
	
	while (nItem--)
	{
		UINT uID = pMenu->GetMenuItemID(nItem);
		
		if (uID >= 0xF000)
		{
			BOOL bEnable = TRUE;
			
			switch (uID & 0xFFF0)
			{
			case SC_MINIMIZE:
				bEnable = (pWnd->GetStyle() & WS_MINIMIZEBOX) && !pWnd->IsIconic();
				break;
				
			case SC_MAXIMIZE:
				bEnable = (pWnd->GetStyle() & WS_MAXIMIZEBOX) && !pWnd->IsZoomed();
				break;
				
			case SC_RESTORE:
				bEnable = pWnd->IsIconic() || pWnd->IsZoomed();
				break;
				
			case SC_MOVE:
			case SC_SIZE:
				bEnable = !pWnd->IsIconic() && !pWnd->IsZoomed();
				break;
			}
			
			pMenu->EnableMenuItem(uID, bEnable ? MF_ENABLED : MF_GRAYED);
		}
	}
	
	// set close as default item
	pMenu->SetDefaultItem(SC_CLOSE);
}

CSize CSkinBase::GetTextExtent(CDC* pDC, LPCTSTR szText)
{
	ASSERT (pDC && szText);

	if (pDC && szText)
	{
		CRect rText(0, 0, 0, SHRT_MAX);
		pDC->DrawText(szText, rText, DT_SINGLELINE | DT_CALCRECT);
		return rText.Size();
	}

	// else
	return 0;
}

const LPCTSTR WORDBREAK = L"_-+=,.:;\n";
const int CHUNK = 32;

int CSkinBase::FormatText(CDC* pDC, CString& sText, CRect& rect, UINT uDTFlags)
{
	CString sOrgText(sText);
	sText.Empty();

	int nPos = 0, nLinePos = 0, nLen = sOrgText.GetLength();
	int nLastWhiteSpace = -1;
	int nHeight = 0;
	const int LINEHEIGHT = pDC->GetTextExtent("W").cy;
	int nLongestLine = 0;

	// we always allow at least one line of text
	BOOL bSingleLine = (uDTFlags & DT_SINGLELINE);
	BOOL bCalcRect = (uDTFlags & DT_CALCRECT);
	BOOL bEllipsis = (uDTFlags & DT_END_ELLIPSIS);
	BOOL bModString = (uDTFlags & DT_MODIFYSTRING);

	BOOL bFinished = FALSE;
	const int CHUNK = 32;

	TCHAR* pBuffer = sOrgText.GetBuffer(sOrgText.GetLength() + 1);

	while (!bFinished)
	{
		int nChunk = CHUNK;
		BOOL bLonger = TRUE;

		int nLinePos = 0;
		int nPrevPos = 0;

		TCHAR* pLine = pBuffer + nPos;

		// add chunks till we go over 
		BOOL bContinue = TRUE;
		int nWidth = 0;

		while (bContinue)
		{
			nChunk = min(nChunk, nLen - (nPos + nLinePos));
			nLinePos += nChunk;

			nWidth = pDC->GetTextExtent(pLine, nLinePos).cx;
			bContinue = (nChunk == CHUNK && nWidth < rect.Width());
		}

		// then iterate back and forth with sub chunks till we are just under
		for (int i = 0; i < 5; i++)
		{
			nChunk /= 2;

			if (nWidth == rect.Width())
				break;
			else if (nWidth > rect.Width())
				nLinePos -= nChunk;
			else
				nLinePos += nChunk;

			nWidth = pDC->GetTextExtent(pLine, nLinePos).cx;
		}

		// one final check to see we haven't ended up over 
		if (nWidth > rect.Width())
			nLinePos--;

		// then work back to the previous word break
		int nSavePos = nLinePos;

		while (nLinePos)
		{
			// we word break either if the current character is whitespace or
			// if the preceding character is a wordbreak character
			// or we've reached the end of the string
			BOOL bWordBreak = (nLinePos == nLen) || _istspace(pLine[nLinePos]) ||
								(wcschr(WORDBREAK, pLine[nLinePos - 1]) != NULL);

			if (bWordBreak)
				break;

			nLinePos--;
		}

		if (!nLinePos)
			nLinePos = nSavePos; // single word spans entire line

		nWidth = pDC->GetTextExtent(pLine, nLinePos).cx;

		// check for last line and add ellipsis if required
		nHeight += LINEHEIGHT;
		BOOL bAddEllipsis = FALSE;

		if (nHeight + LINEHEIGHT > rect.Height() || bSingleLine)
		{
			if (bEllipsis && nPos + nLinePos < nLen - 1)
			{
				const int LEN_ELLIPSIS = pDC->GetTextExtent(L"...", 3).cx;
				
				// modify the last line to add ellipsis
				while (nLinePos)
				{
					nWidth = pDC->GetTextExtent(pLine, nLinePos).cx + LEN_ELLIPSIS;

					if (nWidth > rect.Width())
						nLinePos--;
					else
						break;
				}
				
				bAddEllipsis = TRUE;
			}

			sText += sOrgText.Mid(nPos, nLinePos);

			if (bAddEllipsis)
				sText += "...";

			nLongestLine = max(nLongestLine, nWidth);
			break;
		}

		sText += sOrgText.Mid(nPos, nLinePos);
		sText += '\n';

		nPos += nLinePos;

		bFinished = (nPos >= nLen - 1 || nHeight + LINEHEIGHT > rect.Height());
		nLongestLine = max(nLongestLine, nWidth);

		// jump white space at the start of the next line
		if (!bFinished)
		{
			while (nPos < nLen - 1)
			{
				if (!_istspace(pBuffer[nPos]))
					break;

				// else
				nPos++;
			}
		}
	}

	sOrgText.ReleaseBuffer();

	if (!bModString)
		sText = sOrgText;

	if (bCalcRect)
	{
		rect.right = rect.left + nLongestLine;
		rect.bottom = rect.top + nHeight;
	}

	return nHeight;
}

//////////////////////////////

//////////////////////////////

int CALLBACK CheckFontProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, DWORD FontType, LPARAM lParam)
{
	BOOL* pPresent = (BOOL*)lParam;
	ASSERT (pPresent);

	if (pPresent && FontType == TRUETYPE_FONTTYPE) // only TT accepted for now
		*pPresent = TRUE; // at least one font found to match facename

	return 0;
}

BOOL CSkinBase::FontIsPresent(LPCTSTR szFaceName)
{
	LOGFONT lf;

	HDC hdc = ::GetDC(NULL);
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfPitchAndFamily = 0;

	lstrcpy(lf.lfFaceName, szFaceName);

	BOOL bPresent = FALSE;
	EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)CheckFontProc, (LPARAM)&bPresent, 0);
	
	::ReleaseDC(NULL, hdc);

	return bPresent;
}

//////////////////////////////////
// theming

#ifndef STAP_ALLOW_NONCLIENT
	#define STAP_ALLOW_NONCLIENT    (1 << 0)
	#define STAP_ALLOW_CONTROLS     (1 << 1)
	#define STAP_ALLOW_WEBCONTENT   (1 << 2)
#endif


typedef void (STDAPICALLTYPE* SETTHEMEAPPPROPERTIES)(DWORD);

void CSkinBase::EnableTheming(BOOL bEnable)
{
	if (GetOS() < SBOS_XP || bEnable == s_bThemingEnabled)
		return;

	static HMODULE hUXTheme = ::LoadLibrary(L"UXTheme.dll");

	if (hUXTheme)
	{
		SETTHEMEAPPPROPERTIES SetThemeAppProperties = 
						(SETTHEMEAPPPROPERTIES)GetProcAddress(hUXTheme, "SetThemeAppProperties");

		if (SetThemeAppProperties)
		{
			SetThemeAppProperties(bEnable ? 
								STAP_ALLOW_NONCLIENT | STAP_ALLOW_CONTROLS | STAP_ALLOW_WEBCONTENT :
								0);
			s_bThemingEnabled = bEnable;
		}
	}
}

BOOL CSkinBase::CreateThemeManifest(LPCTSTR szName, LPCTSTR szDescription)
{
//	if (GetOS() < SBOS_XP)
//		return FALSE;

	// create the manifest only if one does not exist
	CString sFilePath;
	::GetModuleFileName(NULL, sFilePath.GetBuffer(MAX_PATH + 1), MAX_PATH);
	sFilePath.ReleaseBuffer();

	sFilePath += ".Manifest";

	CFileStatus fs;

	if (CFile::GetStatus(sFilePath, fs))
		return TRUE; // already exists

	ASSERT (szName && wcslen(szName) && szDescription && wcslen(szDescription));

	if (!(szName && wcslen(szName) && szDescription && wcslen(szDescription)))
		return FALSE;

	LPCTSTR szManifestFmt = L" \
<?xml version='1.0' encoding='UTF-8' standalone='yes'?> \
<assembly \
    xmlns='urn:schemas-microsoft-com:asm.v1' \
    manifestVersion='1.0'> \
\
    <assemblyIdentity \
        version='1.0.0.0' \
        processorArchitecture='X86' \
        name='%s' \
        type='win32' \
    /> \
    <description>%s</description> \
    <dependency> \
        <dependentAssembly> \
            <assemblyIdentity \
                type='win32' \
                name='Microsoft.Windows.Common-Controls' \
                version='6.0.0.0' \
                processorArchitecture='X86' \
                publicKeyToken='6595b64144ccf1df' \
                language='*' \
            /> \
        </dependentAssembly> \
    </dependency> \
</assembly> ";

	CString sManifest;
	sManifest.Format(szManifestFmt, szName, szDescription);

	CStdioFile file;

	if (!file.Open(sFilePath, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
		return FALSE;

	file.WriteString(sManifest);
	file.Close();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////

HICON CSkinBase::GetWindowIcon(CWnd* pWnd)
{
	ASSERT (pWnd);

	if (!pWnd)
		return NULL;

	HICON hIcon = pWnd->GetIcon(FALSE); // small icon
				
	if (!hIcon)
		hIcon = pWnd->GetIcon(TRUE); // large icon

	if (!hIcon)
	{
		WNDCLASS wndcls;
		CString sClass(CWinClasses::GetClass(*pWnd));

		if (GetClassInfo(AfxGetInstanceHandle(), sClass, &wndcls))
			hIcon = wndcls.hIcon;
	}

	return hIcon;
}

