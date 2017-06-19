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
#include "afxglobals.h"
#include "afxtoolbarimages.h"
#include "afxtoolbar.h"
#include "afxribbonres.h"
#include "afxvisualmanager.h"
#include "afxdrawmanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static BOOL __stdcall WriteDIB( LPCTSTR szFile, HANDLE hDIB);
static HANDLE __stdcall DDBToDIB(HBITMAP bitmap, DWORD dwCompression);

BOOL CMFCToolBarImages::m_bDisableTrueColorAlpha = TRUE;
CCriticalSection CMFCToolBarImages::m_CriticalSection;
BOOL CMFCToolBarImages::m_bMultiThreaded = FALSE;
BOOL CMFCToolBarImages::m_bIsDrawOnGlass = FALSE;
BYTE CMFCToolBarImages::m_nDisabledImageAlpha = 127;
BYTE CMFCToolBarImages::m_nFadedImageAlpha = 150;
BOOL CMFCToolBarImages::m_bIsRTL = FALSE;
CString CMFCToolBarImages::m_strPngResType = _T("PNG");

// globals for fast drawing(shared globals)
static HDC hDCGlyphs = NULL;
static HDC hDCMono = NULL;

/*
DIBs use RGBQUAD format:
0xbb 0xgg 0xrr 0x00

Reasonably efficient code to convert a COLORREF into an
RGBQUAD is byte-order-dependent, so we need different
code depending on the byte order we're targeting.
*/

#define AFX_RGB_TO_RGBQUAD(r,g,b)(RGB(b,g,r))
#define AFX_CLR_TO_RGBQUAD(clr)(RGB(GetBValue(clr), GetGValue(clr), GetRValue(clr)))
#define AFX_RGBQUAD_TO_CLR(clr)(RGB(GetBValue(clr), GetGValue(clr), GetRValue(clr)))

// Raster Ops
#define AFX_ROP_DSPDxax  0x00E20746L
#define AFX_ROP_PSDPxax  0x00B8074AL

// Internal images:
#define AFX_IMAGE_LIGHT  0
#define AFX_IMAGE_SHADOW 1

/////////////////////////////////////////////////////////////////////////////
// Init / Term

void __stdcall CMFCToolBarImages::CleanUp()
{
	if (hDCMono != NULL)
	{
		::DeleteDC(hDCMono);
		hDCMono = NULL;
	}

	if (hDCGlyphs != NULL)
	{
		::DeleteDC(hDCGlyphs);
		hDCGlyphs = NULL;
	}

	CPngImage::CleanUp ();
}

// a special struct that will cleanup automatically
struct _AFX_TOOLBAR_TERM
{
	~_AFX_TOOLBAR_TERM()
	{
		CMFCToolBarImages::CleanUp();
	}
};

static const _AFX_TOOLBAR_TERM toolbarTerm;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFCToolBarImages::CMFCToolBarImages()
{
	m_bModified = FALSE;
	m_bReadOnly = FALSE;
	m_bIsTemporary = FALSE;
	m_iCount = 0;
	m_bIsGray = FALSE;
	m_nGrayImageLuminancePercentage = 0;

	m_hbmImageWell = NULL;
	m_hbmImageLight = NULL;
	m_hbmImageShadow = NULL;

	m_bUserImagesList = FALSE;

	// initialize the toolbar drawing engine
	static BOOL bInitialized;
	if (!bInitialized)
	{
		hDCGlyphs = CreateCompatibleDC(NULL);

		// Mono DC and Bitmap for disabled image
		hDCMono = ::CreateCompatibleDC(NULL);

		if (hDCGlyphs == NULL || hDCMono == NULL)
			AfxThrowResourceException();

		bInitialized = TRUE;
	}

	m_clrTransparent = (COLORREF) -1;

	// UISG standard sizes
	m_sizeImage = CSize(16, 15);
	m_sizeImageDest = CSize(0, 0);
	m_rectLastDraw = CRect(0, 0, 0, 0);
	m_rectSubImage = CRect(0, 0, 0, 0);
	m_bStretch = FALSE;
	m_pBmpOriginal = NULL;

	m_bFadeInactive = FALSE;
	m_nBitsPerPixel = 0;

	m_nLightPercentage = 130;
	m_bAlwaysLight = FALSE;

	m_bMapTo3DColors = TRUE;
	m_bAutoCheckPremlt = FALSE;
	m_bCreateMonoDC = TRUE;

	OnSysColorChange();
}

CMFCToolBarImages::~CMFCToolBarImages()
{
	ENSURE(m_dcMem.GetSafeHdc() == NULL);
	ENSURE(m_bmpMem.GetSafeHandle() == NULL);
	ENSURE(m_pBmpOriginal == NULL);

	if (!m_bIsTemporary)
	{
		AfxDeleteObject((HGDIOBJ*)&m_hbmImageWell);
		AfxDeleteObject((HGDIOBJ*)&m_hbmImageLight);
		AfxDeleteObject((HGDIOBJ*)&m_hbmImageShadow);
	}
}

BOOL CMFCToolBarImages::Load(UINT uiResID, HINSTANCE hinstRes, BOOL bAdd)
{
	return LoadStr(MAKEINTRESOURCE(uiResID), hinstRes, bAdd);
}

BOOL CMFCToolBarImages::LoadStr(LPCTSTR lpszResourceName, HINSTANCE hinstRes, BOOL bAdd)
{
	if (m_bIsTemporary)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (lpszResourceName == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	UINT uiResID = IS_INTRESOURCE(lpszResourceName) ?(UINT)((UINT_PTR)(lpszResourceName)) : 0;

	if (!bAdd)
	{
		AfxDeleteObject((HGDIOBJ*)&m_hbmImageWell);     // get rid of old one
		m_lstOrigResIds.RemoveAll();
		m_lstOrigResInstances.RemoveAll();
		m_mapOrigResOffsets.RemoveAll();
	}
	else if (uiResID != 0 && m_lstOrigResIds.Find(uiResID) != NULL) // Already loaded, do nothing
	{
		return TRUE;
	}

	HBITMAP hbmp = NULL;

	// Try to load PNG image first:
	CPngImage pngImage;
	if (pngImage.Load(lpszResourceName, hinstRes))
	{
		hbmp = (HBITMAP) pngImage.Detach();
	}
	else
	{
		if (hinstRes == NULL)
		{
			hinstRes = AfxFindResourceHandle(lpszResourceName, RT_BITMAP);
		}

		UINT uiLoadImageFlags = LR_CREATEDIBSECTION;
		if (m_bMapTo3DColors && !afxGlobalData.m_bIsBlackHighContrast)
		{
			uiLoadImageFlags |= LR_LOADMAP3DCOLORS;
		}

		hbmp = (HBITMAP) ::LoadImage(hinstRes, lpszResourceName, IMAGE_BITMAP, 0, 0, uiLoadImageFlags);
	}

	if (hbmp == NULL)
	{
		if (uiResID != 0)
		{
			TRACE(_T("Can't load bitmap: %x. GetLastError() = %x\n"), uiResID, GetLastError());
		}
		else
		{
			TRACE(_T("Can't load bitmap: %s. GetLastError() = %x\n"), lpszResourceName, GetLastError());
		}
		return FALSE;
	}

	BITMAP bmp;
	if (::GetObject(hbmp, sizeof(BITMAP), &bmp) == 0)
	{
		ASSERT(FALSE);
		::DeleteObject(hbmp);
		return FALSE;
	}

	if (bmp.bmBitsPixel >= 32)
	{
		PreMultiplyAlpha(hbmp);
	}
	else if ((bmp.bmBitsPixel > 8 && m_bMapTo3DColors) || afxGlobalData.m_bIsBlackHighContrast)
	{
		// LR_LOADMAP3DCOLORS don't support > 8bpp images,
		// we should convert it now:
		MapBmpTo3dColors(hbmp, FALSE);
	}

	m_nBitsPerPixel = max(m_nBitsPerPixel, bmp.bmBitsPixel);

	if (bAdd)
	{
		if (uiResID != 0)
		{
			m_mapOrigResOffsets.SetAt(uiResID, m_iCount);
		}

		AddImage(hbmp);

		if (uiResID != 0)
		{
			m_lstOrigResIds.AddTail(uiResID);
			m_lstOrigResInstances.AddTail(hinstRes);
		}

		::DeleteObject(hbmp);
	}
	else
	{
		m_hbmImageWell = hbmp;
	}

	UpdateCount();

	AfxDeleteObject((HGDIOBJ*)&m_hbmImageLight);
	m_hbmImageLight = NULL;

	AfxDeleteObject((HGDIOBJ*)&m_hbmImageShadow);
	m_hbmImageShadow = NULL;

	return TRUE;
}

BOOL CMFCToolBarImages::Load(LPCTSTR lpszBmpFileName, DWORD dwMaxFileSize /* = 819200 */)
{
	if (m_bIsTemporary)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	ENSURE(lpszBmpFileName != NULL);

	AfxDeleteObject((HGDIOBJ*)&m_hbmImageWell);     // get rid of old one

	CString strPath = lpszBmpFileName;

	// If the image path is not defined, try to open it in the EXE directory:
	if (strPath.Find(_T("\\")) == -1 && strPath.Find(_T("/")) == -1 && strPath.Find(_T(":")) == -1)
	{
		TCHAR lpszFilePath [_MAX_PATH];
		if (::GetModuleFileName(NULL, lpszFilePath, _MAX_PATH) > 0)
		{
			TCHAR path_buffer[_MAX_PATH];
			TCHAR drive[_MAX_DRIVE];
			TCHAR dir[_MAX_DIR];
			TCHAR fname[_MAX_FNAME];
			TCHAR ext[_MAX_EXT];

			_tsplitpath_s(lpszFilePath, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0);
			_tsplitpath_s(lpszBmpFileName, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT);

			_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, fname, ext);

			strPath = path_buffer;
		}
	}

	// Check that file size does not exceed specified limit
	if (dwMaxFileSize > 0)
	{
		HANDLE hFile = CreateFile(lpszBmpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD dwFileSize = GetFileSize(hFile, NULL);
			CloseHandle(hFile);

			if (dwFileSize > dwMaxFileSize)
			{
				return FALSE;
			}
		}
	}

	// Load images from the disk file:
	UINT uiLoadImageFlags = LR_LOADFROMFILE | LR_CREATEDIBSECTION;
	if (m_bMapTo3DColors)
	{
		uiLoadImageFlags |= LR_LOADMAP3DCOLORS;
	}

	m_hbmImageWell = (HBITMAP) ::LoadImage(AfxGetInstanceHandle(), strPath, IMAGE_BITMAP, 0, 0, uiLoadImageFlags);

	if (m_hbmImageWell == NULL)
	{
		TRACE(_T("Can't load bitmap: %s. GetLastError() = %x\r\n"), lpszBmpFileName, GetLastError());
		return FALSE;
	}

	BITMAP bmp;
	if (::GetObject(m_hbmImageWell, sizeof(BITMAP), &bmp) == 0)
	{
		ASSERT(FALSE);
		::DeleteObject(m_hbmImageWell);
		m_hbmImageWell = NULL;
		return FALSE;
	}

	m_bUserImagesList = TRUE;
	m_strUDLPath = strPath;

	if (::GetFileAttributes(strPath) & FILE_ATTRIBUTE_READONLY)
	{
		m_bReadOnly = TRUE;
	}

	m_nBitsPerPixel = bmp.bmBitsPixel;
	if (m_nBitsPerPixel > 8 && m_nBitsPerPixel < 32)
	{
		// LR_LOADMAP3DCOLORS don't support > 8bpp images, // we should convert it now:
		MapTo3dColors(FALSE);
	}

	if (bmp.bmBitsPixel >= 32)
	{
		PreMultiplyAlpha(m_hbmImageWell);
	}

	UpdateCount();

	AfxDeleteObject((HGDIOBJ*)&m_hbmImageLight);
	m_hbmImageLight = NULL;

	AfxDeleteObject((HGDIOBJ*)&m_hbmImageShadow);
	m_hbmImageShadow = NULL;

	return TRUE;
}

BOOL CMFCToolBarImages::PrepareDrawImage(CAfxDrawState& ds, CSize sizeImageDest, BOOL bFadeInactive)
{
	if (m_hbmImageWell == NULL)
	{
		return FALSE;
	}

	if (m_bMultiThreaded)
	{
		m_CriticalSection.Lock();
	}

	if (bFadeInactive && m_nBitsPerPixel < 32 && m_hbmImageLight == NULL)
	{
		UpdateInternalImage(AFX_IMAGE_LIGHT);
	}

	if (m_nBitsPerPixel < 32 && m_hbmImageShadow == NULL && CMFCVisualManager::GetInstance()->IsShadowHighlightedImage() && !afxGlobalData.IsHighContrastMode())
	{
		UpdateInternalImage(AFX_IMAGE_SHADOW);
	}

	m_bStretch = FALSE;

	if (m_hbmImageLight == NULL || (m_nBitsPerPixel > 4 && !m_bAlwaysLight) || m_nBitsPerPixel == 0)
	{
		// Down't fade 256+ or unknown bitmaps
		bFadeInactive = FALSE;
	}

	m_bFadeInactive = bFadeInactive;

	ENSURE(m_hbmImageWell != NULL);
	ENSURE(m_dcMem.GetSafeHdc() == NULL);
	ENSURE(m_bmpMem.GetSafeHandle() == NULL);
	ENSURE(m_pBmpOriginal == NULL);

	// We need to kick-start the bitmap selection process.
	ds.hbmOldGlyphs = (HBITMAP)SelectObject(hDCGlyphs, bFadeInactive && m_nBitsPerPixel < 32 ? m_hbmImageLight : m_hbmImageWell);

	if (ds.hbmOldGlyphs == NULL)
	{
		TRACE0("Error: can't draw toolbar.\r\n");

		if (m_bMultiThreaded)
		{
			m_CriticalSection.Unlock();
		}

		return FALSE;
	}

	if (m_bCreateMonoDC)
	{
		ds.hbmMono = CreateBitmap(m_sizeImage.cx + 2, m_sizeImage.cy + 2, 1, 1, NULL);
		ds.hbmMonoOld = (HBITMAP)SelectObject(hDCMono, ds.hbmMono);

		if (ds.hbmMono == NULL || ds.hbmMonoOld == NULL)
		{
			TRACE0("Error: can't draw toolbar.\r\n");
			AfxDeleteObject((HGDIOBJ*)&ds.hbmMono);

			if (m_bMultiThreaded)
			{
				m_CriticalSection.Unlock();
			}

			return FALSE;
		}
	}

	if (sizeImageDest.cx <= 0 || sizeImageDest.cy <= 0)
	{
		m_sizeImageDest = m_sizeImage;
	}
	else
	{
		m_sizeImageDest = sizeImageDest;
	}

	COLORREF clrTransparent = m_nBitsPerPixel == 32 ? (COLORREF) -1 : m_clrTransparent;

	if (m_sizeImageDest != m_sizeImage || clrTransparent != (COLORREF) -1)
	{
		CWindowDC dc(NULL);

		m_bStretch = (m_sizeImageDest != m_sizeImage);

		m_dcMem.CreateCompatibleDC(NULL); // Assume display!
		m_bmpMem.CreateCompatibleBitmap(&dc, m_sizeImage.cx + 2, m_sizeImage.cy + 2);

		m_pBmpOriginal = m_dcMem.SelectObject(&m_bmpMem);
		ENSURE(m_pBmpOriginal != NULL);
	}

	return TRUE;
}

void CMFCToolBarImages::EndDrawImage(CAfxDrawState& ds)
{
	if (m_bCreateMonoDC)
	{
		SelectObject(hDCMono, ds.hbmMonoOld);
		AfxDeleteObject((HGDIOBJ*)&ds.hbmMono);
	}

	SelectObject(hDCGlyphs, ds.hbmOldGlyphs);

	m_sizeImageDest = CSize(0, 0);
	m_rectLastDraw = CRect(0, 0, 0, 0);

	COLORREF clrTransparent = m_nBitsPerPixel == 32 ? (COLORREF) -1 : m_clrTransparent;

	if (m_bStretch || clrTransparent != (COLORREF) -1)
	{
		ENSURE(m_pBmpOriginal != NULL);

		m_dcMem.SelectObject(m_pBmpOriginal);
		m_pBmpOriginal = NULL;

		::DeleteObject(m_bmpMem.Detach());
		::DeleteDC(m_dcMem.Detach());
	}

	m_bFadeInactive = FALSE;

	if (m_bMultiThreaded)
	{
		m_CriticalSection.Unlock();
	}
}

void CMFCToolBarImages::CreateMask(int iImage, BOOL bHilite, BOOL bHiliteShadow)
{
	// initalize whole area with 0's
	PatBlt(hDCMono, 0, 0, m_sizeImage.cx + 2, m_sizeImage.cy + 2, WHITENESS);

	COLORREF clrTransparent = m_nBitsPerPixel == 32 ? (COLORREF) -1 : m_clrTransparent;

	// create mask based on color bitmap
	// convert this to 1's
	SetBkColor(hDCGlyphs, clrTransparent != -1 ? clrTransparent : afxGlobalData.clrBtnFace);

	::BitBlt(hDCMono, 0, 0, m_sizeImage.cx, m_sizeImage.cy, hDCGlyphs, iImage * m_sizeImage.cx, 0, SRCCOPY);

	if (bHilite)
	{
		// convert this to 1's
		SetBkColor(hDCGlyphs, afxGlobalData.clrBtnHilite);

		// OR in the new 1's
		::BitBlt(hDCMono, 0, 0, m_sizeImage.cx, m_sizeImage.cy, hDCGlyphs, iImage * m_sizeImage.cx, 0, SRCPAINT);

		if (bHiliteShadow)
		{
			::BitBlt(hDCMono, 1, 1, m_sizeImage.cx + 1, m_sizeImage.cy + 1, hDCMono, 0, 0, SRCAND);
		}
	}
}

HBITMAP CMFCToolBarImages::GetMask(int iImage)
{
	CAfxDrawState ds;

	PrepareDrawImage(ds, FALSE);
	CreateMask(iImage, FALSE, FALSE);

	CDC memDCDest;
	CDC* pDCMono = CDC::FromHandle(hDCMono);
	ASSERT_VALID(pDCMono);

	memDCDest.CreateCompatibleDC(pDCMono);

	CBitmap bitmapMask;

	if (bitmapMask.CreateBitmap(m_sizeImage.cx, m_sizeImage.cy, 1, 1, NULL))
	{
		CBitmap* pOldBitmapDest = memDCDest.SelectObject(&bitmapMask);

		memDCDest.BitBlt(0, 0, m_sizeImage.cx, m_sizeImage.cy, pDCMono, 0, 0, SRCCOPY);
		memDCDest.SelectObject(pOldBitmapDest);
	}

	EndDrawImage(ds);

	return(HBITMAP) bitmapMask.Detach();
}

BOOL CMFCToolBarImages::Draw(CDC* pDCDest, int xDest, int yDest, int iImage, BOOL bHilite, BOOL bDisabled, BOOL bIndeterminate, BOOL bShadow, BOOL bInactive, BYTE alphaSrc/* = 255*/)
{
	if (iImage < 0 || iImage >= m_iCount)
	{
		return FALSE;
	}

	if (bShadow && afxGlobalData.m_nBitsPerPixel <= 8)
	{
		return TRUE;
	}

	m_rectLastDraw = CRect(CPoint(xDest, yDest), m_sizeImageDest);

	if (m_bStretch)
	{
		bHilite = FALSE;
		bIndeterminate = FALSE;
	}

	HBITMAP hBmpOriginal = NULL;
	if ((!bInactive || bDisabled) && m_bFadeInactive && m_nBitsPerPixel < 32)
	{
		hBmpOriginal = (HBITMAP) SelectObject(hDCGlyphs, m_hbmImageWell);
	}

	BOOL bStretchOld = m_bStretch;
	BOOL bAlphaStretch =
		(m_nBitsPerPixel == 32 && m_bStretch);

	if (bAlphaStretch)
	{
		m_bStretch = FALSE;
	}

	COLORREF clrTransparent = (m_nBitsPerPixel == 32 || m_bIsDrawOnGlass) ?(COLORREF) -1 : m_clrTransparent;

	BOOL bIsTransparent = (clrTransparent != (COLORREF) -1);

	COLORREF clrTransparentDisabled = clrTransparent;

	CDC* pDC = m_bStretch || bIsTransparent ? &m_dcMem : pDCDest;
	ASSERT_VALID(pDC);

	int x = m_bStretch || bIsTransparent ? 0 : xDest;
	int y = m_bStretch || bIsTransparent ? 0 : yDest;

	const int xOffset = m_rectSubImage.left;
	const int yOffset = m_rectSubImage.top;

	const int nWidth = m_rectSubImage.IsRectEmpty() ? m_sizeImage.cx : m_rectSubImage.Width();
	const int nHeight = m_rectSubImage.IsRectEmpty() ? m_sizeImage.cy : m_rectSubImage.Height();

	if (m_bStretch || bIsTransparent)
	{
		CRect rectImage(CPoint(0, 0), m_sizeImage);

		if (bIsTransparent && clrTransparent != afxGlobalData.clrBtnFace)
		{
			CBrush brBackgr(clrTransparent);
			pDC->FillRect(rectImage, &brBackgr);
		}
		else
		{
			pDC->FillRect(rectImage, &afxGlobalData.brBtnFace);
		}

		if (bDisabled && afxGlobalData.m_nBitsPerPixel == 16)
		{
			clrTransparentDisabled = pDC->GetPixel(rectImage.TopLeft());
		}
	}

	BOOL bDisabledTrueColor = FALSE;

	if (bDisabled && m_nBitsPerPixel >= 24)
	{
		bDisabled = FALSE;
		bDisabledTrueColor = TRUE;
	}

	BOOL bShadowTrueColor = FALSE;

	if (bShadow && m_nBitsPerPixel == 32)
	{
		bShadow = FALSE;
		bShadowTrueColor = TRUE;
	}

	if (!bHilite && !bDisabled && !bShadow)
	{
		BOOL bIsReady = FALSE;

		if ((m_nBitsPerPixel == 32 || m_bIsDrawOnGlass))
		{
			BLENDFUNCTION pixelblend = { AC_SRC_OVER, 0, alphaSrc, AC_SRC_ALPHA };

			if (bDisabledTrueColor)
			{
				pixelblend.SourceConstantAlpha = m_nDisabledImageAlpha;
			}

			if (bInactive && m_bFadeInactive)
			{
				pixelblend.SourceConstantAlpha = m_nFadedImageAlpha;
			}

			const CSize sizeDest = bAlphaStretch ? m_sizeImageDest : m_sizeImage;

			if (m_nBitsPerPixel != 32)
			{
				BITMAPINFO bi;

				// Fill in the BITMAPINFOHEADER
				bi.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
				bi.bmiHeader.biWidth         = nWidth;
				bi.bmiHeader.biHeight        = nHeight;
				bi.bmiHeader.biPlanes        = 1;
				bi.bmiHeader.biBitCount      = 32;
				bi.bmiHeader.biCompression   = BI_RGB;
				bi.bmiHeader.biSizeImage     = nWidth * nHeight;
				bi.bmiHeader.biXPelsPerMeter = 0;
				bi.bmiHeader.biYPelsPerMeter = 0;
				bi.bmiHeader.biClrUsed       = 0;
				bi.bmiHeader.biClrImportant  = 0;

				COLORREF* pBits = NULL;
				HBITMAP hbmp = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, (LPVOID*)&pBits, NULL, 0);

				if (hbmp == NULL)
				{
					ASSERT(FALSE);
					return FALSE;
				}

				CBitmap bmpMem;
				bmpMem.Attach(hbmp);

				CDC dcMem;
				dcMem.CreateCompatibleDC(NULL);
				CBitmap* pBmpOld = dcMem.SelectObject(&bmpMem);

				::BitBlt(dcMem.GetSafeHdc(), 0, 0, nWidth, nHeight, hDCGlyphs, iImage * m_sizeImage.cx + xOffset, yOffset, SRCCOPY);

				int nSizeImage = nHeight * nWidth;

				if (m_clrTransparent == -1)
				{
					for (int i = 0; i < nSizeImage; i++)
					{
						*pBits |= 0xFF000000;
						pBits++;
					}
				}
				else
				{
					COLORREF clrTrans = RGB(GetBValue(m_clrTransparent), GetGValue(m_clrTransparent), GetRValue(m_clrTransparent));

					for (int i = 0; i < nSizeImage; i++)
					{
						if (*pBits != clrTrans)
						{
							*pBits |= 0xFF000000;
						}
						else
						{
							*pBits = (COLORREF)0;
						}

						pBits++;
					}
				}

				bIsReady = pDC->AlphaBlend(x, y, sizeDest.cx, sizeDest.cy, &dcMem, 0, 0, nWidth, nHeight, pixelblend);

				dcMem.SelectObject(pBmpOld);
			}
			else
			{
				bIsReady = pDC->AlphaBlend(x, y, sizeDest.cx, sizeDest.cy, CDC::FromHandle(hDCGlyphs), iImage * m_sizeImage.cx + xOffset, yOffset, nWidth, nHeight, pixelblend);
			}
		}

		if (!bIsReady)
		{
			// normal image version:
			::BitBlt(pDC->m_hDC, x, y, nWidth, nHeight, hDCGlyphs, iImage * m_sizeImage.cx + xOffset, yOffset, SRCCOPY);

			if (bDisabledTrueColor)
			{
				CDrawingManager dm(*pDC);
				dm.GrayRect(CRect(x, y, x + nWidth + 2, y + nHeight + 2), -1, clrTransparentDisabled == -1 ? afxGlobalData.clrBtnFace : clrTransparentDisabled, CMFCVisualManager::GetInstance()->GetToolbarDisabledColor());
			}
		}
	}
	else if (bShadow && m_hbmImageShadow != NULL)
	{
		HBITMAP hbmpCurr =
			(HBITMAP) SelectObject(hDCGlyphs, m_hbmImageShadow);

		::BitBlt(pDC->m_hDC, x, y, nWidth, nHeight, hDCGlyphs, iImage * m_sizeImage.cx + xOffset, yOffset, SRCCOPY);

		SelectObject(hDCGlyphs, hbmpCurr);
	}
	else
	{
		if (bDisabled || bIndeterminate || bShadow)
		{
			// disabled or indeterminate version
			CreateMask(iImage, TRUE, FALSE);

			pDC->SetTextColor(bShadow ? m_clrImageShadow : 0L); // 0's in mono -> 0(for ROP)
			pDC->SetBkColor((COLORREF)0x00FFFFFFL); // 1's in mono -> 1

			if (bDisabled && CMFCVisualManager::GetInstance()->IsEmbossDisabledImage())
			{
				// disabled - draw the hilighted shadow
				HGDIOBJ hbrOld = pDC->SelectObject(afxGlobalData.hbrBtnHilite);
				if (hbrOld != NULL)
				{
					// draw hilight color where we have 0's in the mask
					::BitBlt(pDC->m_hDC, x + 1, y + 1, nWidth + 2, nHeight + 2, hDCMono, 0, 0, AFX_ROP_PSDPxax);

					pDC->SelectObject(hbrOld);
				}
			}

			//BLOCK: always draw the shadow
			{
				HGDIOBJ hbrOld = pDC->SelectObject(afxGlobalData.hbrBtnShadow);
				if (hbrOld != NULL)
				{
					// draw the shadow color where we have 0's in the mask
					::BitBlt(pDC->m_hDC, x, y, nWidth + 2, nHeight + 2, hDCMono, 0, 0, AFX_ROP_PSDPxax);

					pDC->SelectObject(hbrOld);
				}
			}
		}

		// if it is checked do the dither brush avoiding the glyph
		if (bHilite || bIndeterminate)
		{
			CBrush* pBrOld = pDC->SelectObject(&afxGlobalData.brLight);
			if (pBrOld != NULL)
			{
				CreateMask(iImage, !bIndeterminate, bDisabled);

				pDC->SetTextColor(0L);              // 0 -> 0
				pDC->SetBkColor((COLORREF)0x00FFFFFFL); // 1 -> 1

				// only draw the dither brush where the mask is 1's
				::BitBlt(pDC->m_hDC, x, y, nWidth, nHeight, hDCMono, 0, 0, AFX_ROP_DSPDxax);

				pDC->SelectObject(pBrOld);
			}
		}
	}

	if (m_bStretch)
	{
		TransparentBlt(pDCDest->GetSafeHdc(), xDest, yDest, nWidth, nHeight, pDC, 0, 0, bIsTransparent ? clrTransparent : afxGlobalData.clrBtnFace, m_sizeImageDest.cx, m_sizeImageDest.cy);
	}
	else if (bIsTransparent)
	{
		TransparentBlt(pDCDest->GetSafeHdc(), xDest, yDest, nWidth, nHeight, pDC, 0, 0, clrTransparent);
	}

	if (hBmpOriginal != NULL)
	{
		SelectObject(hDCGlyphs, hBmpOriginal);
	}

	m_bStretch = bStretchOld;
	return TRUE;
}

BOOL CMFCToolBarImages::DrawEx(CDC* pDC, CRect rect, int iImageIndex, ImageAlignHorz horzAlign/* = ImageAlignHorzLeft*/, ImageAlignVert vertAlign/* = ImageAlignVertTop*/, CRect rectSrc/* = CRect(0, 0, 0, 0)*/, BYTE alphaSrc/* = 255*/)
{
	ASSERT_VALID(pDC);

	if (rectSrc.IsRectEmpty())
	{
		rectSrc = CRect(CPoint(0, 0), m_sizeImage);
	}

	if (rectSrc.IsRectEmpty())
	{
		return FALSE;
	}

	CRect rectDst(rect);

	if (horzAlign != ImageAlignHorzStretch)
	{
		BOOL bUpdate = TRUE;

		if (horzAlign == ImageAlignHorzLeft)
		{
			rectDst.right = rectDst.left + rectSrc.Width();
		}
		else if (horzAlign == ImageAlignHorzRight)
		{
			rectDst.left = rectDst.right - rectSrc.Width();
		}
		else if (horzAlign == ImageAlignHorzCenter)
		{
			rectDst.left += (rectDst.Width() - rectSrc.Width()) / 2;
			rectDst.right = rectDst.left + rectSrc.Width();
		}
		else
		{
			bUpdate = FALSE;
		}

		if (bUpdate)
		{
			CRect rt(rectDst);
			rectDst.IntersectRect(rectDst, rect);

			if (0 < rectDst.Width() && rectDst.Width() !=  rectSrc.Width())
			{
				rectSrc.left += rectDst.left - rt.left;
				rectSrc.right = rectSrc.left + min(rectDst.Width(), rectSrc.Width());
			}
		}
	}

	if (vertAlign != ImageAlignVertStretch)
	{
		BOOL bUpdate = TRUE;

		if (vertAlign == ImageAlignVertTop)
		{
			rectDst.bottom = rectDst.top + rectSrc.Height();
		}
		else if (vertAlign == ImageAlignVertBottom)
		{
			rectDst.top = rectDst.bottom - rectSrc.Height();
		}
		else if (vertAlign == ImageAlignVertCenter)
		{
			rectDst.top += (rectDst.Height() - rectSrc.Height()) / 2;
			rectDst.bottom = rectDst.top + rectSrc.Height();
		}
		else
		{
			bUpdate = FALSE;
		}

		if (bUpdate)
		{
			CRect rt(rectDst);
			rectDst.IntersectRect(rectDst, rect);

			if (0 < rectDst.Height() && rectDst.Height() !=  rectSrc.Height())
			{
				rectSrc.top += rectDst.top - rt.top;
				rectSrc.bottom = rectSrc.top + min(rectDst.Height(), rectSrc.Height());
			}
		}
	}

	if (rectSrc.IsRectEmpty() || rectDst.IsRectEmpty())
	{
		return FALSE;
	}

	HBITMAP hbmOldGlyphs = (HBITMAP)SelectObject (hDCGlyphs, m_hbmImageWell);

	const int xOffset = rectSrc.left;
	const int yOffset = rectSrc.top;

	const int nWidth = rectSrc.IsRectEmpty () ? m_sizeImage.cx : rectSrc.Width ();
	const int nHeight = rectSrc.IsRectEmpty () ? m_sizeImage.cy : rectSrc.Height ();

	BOOL bRes = FALSE;

	if (m_nBitsPerPixel == 32)
	{
		BLENDFUNCTION pixelblend = { AC_SRC_OVER, 0, alphaSrc, AC_SRC_ALPHA };

		bRes = pDC->AlphaBlend(rectDst.left, rectDst.top,
			rectDst.Width (), rectDst.Height (), 
			CDC::FromHandle (hDCGlyphs), iImageIndex * m_sizeImage.cx + xOffset, yOffset, 
			nWidth, nHeight, pixelblend);
	}
	else if (m_clrTransparent == -1)
	{
		bRes = ::StretchBlt (pDC->m_hDC, rectDst.left, rectDst.top,
			rectDst.Width (), rectDst.Height (), 
			hDCGlyphs, iImageIndex * m_sizeImage.cx + xOffset, yOffset, 
			nWidth, nHeight, SRCCOPY);
	}

	SelectObject (hDCGlyphs, hbmOldGlyphs);

	if (bRes)
	{
		return TRUE;
	}

	BOOL bCreateMonoDC = m_bCreateMonoDC;
	m_bCreateMonoDC = FALSE;

	CAfxDrawState ds;
	if (!PrepareDrawImage(ds, rectDst.Size()))
	{
		m_bCreateMonoDC = bCreateMonoDC;
		return FALSE;
	}

	m_rectSubImage = rectSrc;

	bRes = Draw(pDC, rectDst.left, rectDst.top, iImageIndex, FALSE, FALSE, FALSE, FALSE, FALSE, alphaSrc);

	m_rectSubImage.SetRectEmpty();

	EndDrawImage(ds);
	m_bCreateMonoDC = bCreateMonoDC;
	return bRes;
}

void __stdcall CMFCToolBarImages::FillDitheredRect(CDC* pDC, const CRect& rect)
{
	ASSERT_VALID(pDC);
	pDC->FillRect(&rect, &afxGlobalData.brLight);
}

void CMFCToolBarImages::OnSysColorChange()
{
	if (m_bIsTemporary)
	{
		return;
	}

	int iOldCount = m_iCount;

	// re-color bitmap for toolbar
	if (m_hbmImageWell != NULL)
	{
		if (m_bUserImagesList)
		{
			Load(m_strUDLPath, 0);
		}
		else
		{
			// Image was buit from the resources...
			if (m_lstOrigResIds.IsEmpty())
			{
				return;
			}

			ASSERT(m_lstOrigResInstances.GetCount() == m_lstOrigResIds.GetCount());

			AfxDeleteObject((HGDIOBJ*)&m_hbmImageWell);     // get rid of old one

			POSITION posInst = m_lstOrigResInstances.GetHeadPosition();

			for (POSITION pos = m_lstOrigResIds.GetHeadPosition();
				pos != NULL;)
			{
				ENSURE(posInst != NULL);

				UINT uiResId = m_lstOrigResIds.GetNext(pos);
				ENSURE(uiResId > 0);

				HINSTANCE hInst = m_lstOrigResInstances.GetNext(posInst);

				HBITMAP hbmp = NULL;

				CPngImage pngImage;
				if (pngImage.Load(uiResId, hInst))
				{
					hbmp = (HBITMAP) pngImage.Detach();
				}

				if (hbmp == NULL)
				{
					UINT uiLoadImageFlags = LR_CREATEDIBSECTION;

					if (m_bMapTo3DColors && !afxGlobalData.IsHighContrastMode())
					{
						uiLoadImageFlags |= LR_LOADMAP3DCOLORS;
					}

					hbmp = (HBITMAP) ::LoadImage(hInst, MAKEINTRESOURCE(uiResId), IMAGE_BITMAP, 0, 0, uiLoadImageFlags);
				}

				BITMAP bmp;
				if (::GetObject(hbmp, sizeof(BITMAP), &bmp) == 0)
				{
					ASSERT(FALSE);
				}

				if (bmp.bmBitsPixel >= 32)
				{
					PreMultiplyAlpha(hbmp);
				}
				else if ((bmp.bmBitsPixel > 8 && m_bMapTo3DColors) || afxGlobalData.m_bIsBlackHighContrast)
				{
					// LR_LOADMAP3DCOLORS don't support > 8bpp images, // we should convert it now:
					MapBmpTo3dColors(hbmp, FALSE);
				}

				AddImage(hbmp);

				::DeleteObject(hbmp);
			}
		}
	}

	UpdateCount();
	ASSERT(iOldCount == m_iCount);

	if (m_bIsRTL)
	{
		MirrorBitmap(m_hbmImageWell, m_sizeImage.cx);
	}

	AfxDeleteObject((HGDIOBJ*)&m_hbmImageLight);
	m_hbmImageLight = NULL;

	AfxDeleteObject((HGDIOBJ*)&m_hbmImageShadow);
	m_hbmImageShadow = NULL;

	if (m_bIsGray)
	{
		GrayImages(m_nGrayImageLuminancePercentage);
	}

	m_clrImageShadow = afxGlobalData.clrBtnShadow;
}

void CMFCToolBarImages::UpdateCount()
{
	if (m_hbmImageWell == NULL)
	{
		m_iCount = 0;
		return;
	}

	BITMAP bmp;
	if (::GetObject(m_hbmImageWell, sizeof(BITMAP), &bmp) == 0)
	{
		m_iCount = 0;
		return;
	}

	int iWidth = bmp.bmWidth;
	m_iCount = iWidth / m_sizeImage.cx;
}

//////////////////////////////////////////
// Image editing methods:
//////////////////////////////////////////

int CMFCToolBarImages::AddImage(HBITMAP hbmp, BOOL bSetBitPerPixel/* = FALSE*/)
{
	if (m_bIsTemporary)
	{
		ASSERT(FALSE);
		return -1;
	}

	BOOL bIsMirror = FALSE;

	if (m_bIsRTL)
	{
		bIsMirror = TRUE;

		hbmp = (HBITMAP) ::CopyImage(hbmp, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
		MirrorBitmap(hbmp, m_sizeImage.cx);
	}

	// Create memory source DC and select an original bitmap:
	CDC memDCSrc;
	memDCSrc.CreateCompatibleDC(NULL);

	HBITMAP hOldBitmapSrc = NULL;

	int iBitmapWidth;
	int iBitmapHeight;
	int iNewBitmapWidth;

	BITMAP bmp;
	if (::GetObject(hbmp, sizeof(BITMAP), &bmp) == 0)
	{
		return -1;
	}

	if (bSetBitPerPixel)
	{
		m_nBitsPerPixel = bmp.bmBitsPixel;
	}

	iNewBitmapWidth = bmp.bmWidth;
	iBitmapHeight = bmp.bmHeight;

	if (m_hbmImageWell != NULL)
	{
		// Get original bitmap attrbutes:
		if (::GetObject(m_hbmImageWell, sizeof(BITMAP), &bmp) == 0)
		{
			return -1;
		}

		hOldBitmapSrc = (HBITMAP) memDCSrc.SelectObject(m_hbmImageWell);
		if (hOldBitmapSrc == NULL)
		{
			return -1;
		}

		iBitmapWidth = bmp.bmWidth;
		iBitmapHeight = bmp.bmHeight;
	}
	else
	{
		iBitmapWidth = 0;

		hOldBitmapSrc = (HBITMAP) memDCSrc.SelectObject(hbmp);
		if (hOldBitmapSrc == NULL)
		{
			return -1;
		}
	}

	// Create a new bitmap compatibel with the source memory DC
	//(original bitmap SHOULD BE ALREADY SELECTED!):
	HBITMAP hNewBitmap = (HBITMAP) ::CreateCompatibleBitmap(memDCSrc, iBitmapWidth + iNewBitmapWidth, iBitmapHeight);
	if (hNewBitmap == NULL)
	{
		memDCSrc.SelectObject(hOldBitmapSrc);
		return -1;
	}

	// Create memory destination DC and select a new bitmap:
	CDC memDCDst;
	memDCDst.CreateCompatibleDC(&memDCSrc);

	HBITMAP hOldBitmapDst = (HBITMAP) memDCDst.SelectObject(hNewBitmap);
	if (hOldBitmapDst == NULL)
	{
		memDCSrc.SelectObject(hOldBitmapSrc);
		::DeleteObject(hNewBitmap);
		return -1;
	}

	if (m_hbmImageWell != NULL)
	{
		// Copy original bitmap to new:
		memDCDst.BitBlt(0, 0, iBitmapWidth, iBitmapHeight, &memDCSrc, 0, 0, SRCCOPY);
	}

	// Select a new image and copy it:
	if (memDCSrc.SelectObject(hbmp) == NULL)
	{
		memDCDst.SelectObject(hOldBitmapDst);
		memDCSrc.SelectObject(hOldBitmapSrc);

		::DeleteObject(hNewBitmap);
		return -1;
	}

	memDCDst.BitBlt(iBitmapWidth, 0, iNewBitmapWidth, iBitmapHeight, &memDCSrc, 0, 0, SRCCOPY);

	memDCDst.SelectObject(hOldBitmapDst);
	memDCSrc.SelectObject(hOldBitmapSrc);

	if (m_hbmImageWell != NULL)
	{
		::DeleteObject(m_hbmImageWell);
	}

	m_hbmImageWell = hNewBitmap;
	m_bModified = TRUE;

	UpdateCount();

	AfxDeleteObject((HGDIOBJ*)&m_hbmImageLight);
	m_hbmImageLight = NULL;

	AfxDeleteObject((HGDIOBJ*)&m_hbmImageShadow);
	m_hbmImageShadow = NULL;

	if (bIsMirror)
	{
		::DeleteObject(hbmp);
	}

	return m_iCount - 1;
}

int CMFCToolBarImages::AddImage(const CMFCToolBarImages& imageList, int nIndex)
{
	if (nIndex < 0 || nIndex >= imageList.GetCount())
	{
		ASSERT(FALSE);
		return -1;
	}

	CWindowDC dc(NULL);

	m_sizeImage = imageList.m_sizeImage;
	m_sizeImageDest = imageList.m_sizeImageDest;
	m_clrTransparent = imageList.m_clrTransparent;
	m_clrImageShadow = imageList.m_clrImageShadow;
	m_bFadeInactive = imageList.m_bFadeInactive;
	m_nBitsPerPixel = imageList.m_nBitsPerPixel;

	CDC memDCSrc;
	memDCSrc.CreateCompatibleDC(NULL);

	HBITMAP hOldBitmapSrc = (HBITMAP) memDCSrc.SelectObject(imageList.m_hbmImageWell);

	CDC memDCDest;
	memDCDest.CreateCompatibleDC(NULL);

	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(&dc, m_sizeImage.cx, m_sizeImage.cy);

	CBitmap* pOldBitmapDest = memDCDest.SelectObject(&bitmap);

	memDCDest.BitBlt(0, 0, m_sizeImage.cx, m_sizeImage.cy, &memDCSrc, nIndex * m_sizeImage.cx, 0, SRCCOPY);

	memDCDest.SelectObject(pOldBitmapDest);
	memDCSrc.SelectObject(hOldBitmapSrc);

	return AddImage(bitmap);
}

int CMFCToolBarImages::AddIcon(HICON hIcon, BOOL bAlphaBlend/* = FALSE*/)
{
	CWindowDC dc(NULL);

	if (hIcon == NULL)
	{
		bAlphaBlend = FALSE;
	}

	CDC dcMem;
	dcMem.CreateCompatibleDC(NULL);

	CBitmap bmpMem;

	if (bAlphaBlend)
	{
		BITMAPINFO bi;

		// Fill in the BITMAPINFOHEADER
		bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bi.bmiHeader.biWidth = m_sizeImage.cx;
		bi.bmiHeader.biHeight = m_sizeImage.cy;
		bi.bmiHeader.biPlanes = 1;
		bi.bmiHeader.biBitCount = 32;
		bi.bmiHeader.biCompression = BI_RGB;
		bi.bmiHeader.biSizeImage = m_sizeImage.cx * m_sizeImage.cy;
		bi.bmiHeader.biXPelsPerMeter = 0;
		bi.bmiHeader.biYPelsPerMeter = 0;
		bi.bmiHeader.biClrUsed = 0;
		bi.bmiHeader.biClrImportant = 0;

		COLORREF* pBits = NULL;
		HBITMAP hbmp = CreateDIBSection(dcMem.m_hDC, &bi, DIB_RGB_COLORS, (void **)&pBits, NULL, NULL);
		if (hbmp == NULL)
		{
			ASSERT(FALSE);
			return -1;
		}

		bmpMem.Attach(hbmp);
	}
	else
	{
		bmpMem.CreateCompatibleBitmap(&dc, m_sizeImage.cx, m_sizeImage.cy);
	}

	CBitmap* pBmpOriginal = dcMem.SelectObject(&bmpMem);

	if (!bAlphaBlend)
	{
		dcMem.FillRect(CRect(0, 0, m_sizeImage.cx, m_sizeImage.cy), &afxGlobalData.brBtnFace);
	}

	if (hIcon != NULL)
	{
		dcMem.DrawState(CPoint(0, 0), m_sizeImage, hIcon, DSS_NORMAL, (CBrush*) NULL);
	}

	dcMem.SelectObject(pBmpOriginal);

	if (bAlphaBlend)
	{
		m_nBitsPerPixel = 32;
		PreMultiplyAlpha(bmpMem);
	}

	return AddImage(bmpMem);
}

BOOL CMFCToolBarImages::UpdateImage(int iImage, HBITMAP hbmp)
{
	if (m_bIsTemporary)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (!m_bUserImagesList) // Only user images can be edited!
	{
		return FALSE;
	}

	CWindowDC dc(NULL);
	CBitmap bitmap;
	CDC memDCSrc;
	CDC memDCDst;

	memDCSrc.CreateCompatibleDC(&dc);
	memDCDst.CreateCompatibleDC(&dc);

	HBITMAP hOldBitmapDst = (HBITMAP) memDCDst.SelectObject(m_hbmImageWell);
	HBITMAP hOldBitmapSrc = (HBITMAP) memDCSrc.SelectObject(hbmp);

	memDCDst.BitBlt(m_sizeImage.cx * iImage, 0, m_sizeImage.cx, m_sizeImage.cy, &memDCSrc, 0, 0, SRCCOPY);

	memDCDst.SelectObject(hOldBitmapDst);
	memDCSrc.SelectObject(hOldBitmapSrc);

	m_bModified = TRUE;

	AfxDeleteObject((HGDIOBJ*)&m_hbmImageLight);
	m_hbmImageLight = NULL;

	AfxDeleteObject((HGDIOBJ*)&m_hbmImageShadow);
	m_hbmImageShadow = NULL;

	return TRUE;
}

BOOL CMFCToolBarImages::DeleteImage(int iImage)
{
	if (m_bIsTemporary)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (!m_bUserImagesList) // Only user images can be edited!
	{
		return FALSE;
	}

	if (iImage < 0 || iImage >= GetCount()) // Wrong index
	{
		return FALSE;
	}

	// Get original bitmap attrbutes:
	BITMAP bmp;
	if (::GetObject(m_hbmImageWell, sizeof(BITMAP), &bmp) == 0)
	{
		return FALSE;
	}

	// Create memory source DC and select an original bitmap:
	CDC memDCSrc;
	memDCSrc.CreateCompatibleDC(NULL);

	HBITMAP hOldBitmapSrc = (HBITMAP) memDCSrc.SelectObject(m_hbmImageWell);
	if (hOldBitmapSrc == NULL)
	{
		return FALSE;
	}

	// Create a new bitmap compatibel with the source memory DC
	//(original bitmap SHOULD BE ALREADY SELECTED!):
	HBITMAP hNewBitmap = (HBITMAP) ::CreateCompatibleBitmap(memDCSrc, bmp.bmWidth - m_sizeImage.cx, bmp.bmHeight);
	if (hNewBitmap == NULL)
	{
		memDCSrc.SelectObject(hOldBitmapSrc);
		return FALSE;
	}

	// Create memory destination DC and select a new bitmap:
	CDC memDCDst;
	memDCDst.CreateCompatibleDC(&memDCSrc);

	HBITMAP hOldBitmapDst = (HBITMAP) memDCDst.SelectObject(hNewBitmap);
	if (hOldBitmapDst == NULL)
	{
		memDCSrc.SelectObject(hOldBitmapSrc);
		::DeleteObject(hNewBitmap);
		return FALSE;
	}

	// Copy original bitmap to new:

	if (iImage != 0)
	{
		memDCDst.BitBlt(0, 0, m_sizeImage.cx * iImage, bmp.bmHeight, &memDCSrc, 0, 0, SRCCOPY);
	}

	if (iImage != m_iCount - 1)
	{
		memDCDst.BitBlt(m_sizeImage.cx * iImage, 0, bmp.bmWidth -(m_iCount - iImage - 1) * m_sizeImage.cx, bmp.bmHeight, &memDCSrc, m_sizeImage.cx *(iImage + 1), 0, SRCCOPY);
	}

	memDCDst.SelectObject(hOldBitmapDst);
	memDCSrc.SelectObject(hOldBitmapSrc);

	::DeleteObject(m_hbmImageWell);

	m_hbmImageWell = hNewBitmap;
	m_bModified = TRUE;

	UpdateCount();
	UpdateInternalImage(AFX_IMAGE_LIGHT);
	UpdateInternalImage(AFX_IMAGE_SHADOW);

	return TRUE;
}

HICON CMFCToolBarImages::ExtractIcon(int nIndex)
{
	if (nIndex < 0 || nIndex >= GetCount()) // Wrong index
	{
		return NULL;
	}

	UINT nFlags = (m_nBitsPerPixel == 32) ? 0 : ILC_MASK;

	switch (m_nBitsPerPixel)
	{
	case 4:
	default:
		nFlags |= ILC_COLOR4;
		break;

	case 8:
		nFlags |= ILC_COLOR8;
		break;

	case 16:
		nFlags |= ILC_COLOR16;
		break;

	case 24:
		nFlags |= ILC_COLOR24;
		break;

	case 32:
		nFlags |= ILC_COLOR32;
		break;
	}

	CImageList images;
	images.Create(m_sizeImage.cx, m_sizeImage.cy, nFlags, 0, 0);

	HBITMAP hbmImageWellCopy = (HBITMAP) ::CopyImage(m_hbmImageWell, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

	images.Add(CBitmap::FromHandle(hbmImageWellCopy), m_clrTransparent == -1 ? afxGlobalData.clrBtnFace : m_clrTransparent);

	AfxDeleteObject((HGDIOBJ*)&hbmImageWellCopy);

	return images.ExtractIcon(nIndex);
}

COLORREF __stdcall CMFCToolBarImages::MapToSysColor(COLORREF color, BOOL bUseRGBQUAD)
{
	struct COLORMAP
	{
		// use DWORD instead of RGBQUAD so we can compare two RGBQUADs easily
		DWORD rgbqFrom;
		int iSysColorTo;
	};
	static const COLORMAP sysColorMap[] = {
		// mapping from color in DIB to system color
		{ AFX_RGB_TO_RGBQUAD(0x00, 0x00, 0x00),  COLOR_BTNTEXT },       // black
		{ AFX_RGB_TO_RGBQUAD(0x80, 0x80, 0x80),  COLOR_BTNSHADOW },     // dark grey
		{ AFX_RGB_TO_RGBQUAD(0xC0, 0xC0, 0xC0),  COLOR_BTNFACE },       // bright grey
		{ AFX_RGB_TO_RGBQUAD(0xFF, 0xFF, 0xFF),  COLOR_BTNHIGHLIGHT }   // white
	};
	const int nMaps = 4;

	// look for matching RGBQUAD color in original
	for (int i = 0; i < nMaps; i++)
	{
		if (color == sysColorMap[i].rgbqFrom)
		{
			return bUseRGBQUAD ? AFX_CLR_TO_RGBQUAD(afxGlobalData.GetColor(sysColorMap[i].iSysColorTo)) : afxGlobalData.GetColor(sysColorMap[i].iSysColorTo);
		}
	}

	return color;
}

COLORREF __stdcall CMFCToolBarImages::MapToSysColorAlpha(COLORREF color)
{
	BYTE r = GetRValue(color);
	BYTE g = GetGValue(color);
	BYTE b = GetBValue(color);

	const int nDelta = 10;

	if (abs(r - b) > nDelta || abs(r - g) > nDelta || abs(b - g) > nDelta)
	{
		return color;
	}

	return CDrawingManager::PixelAlpha(afxGlobalData.clrBarFace, 1. +((double) r - 192) / 255, 1. +((double) g - 192) / 255, 1. +((double) b - 192) / 255);
}

COLORREF __stdcall CMFCToolBarImages::MapFromSysColor(COLORREF color, BOOL bUseRGBQUAD)
{
	struct COLORMAP
	{
		// use DWORD instead of RGBQUAD so we can compare two RGBQUADs easily
		DWORD rgbTo;
		int iSysColorFrom;
	};
	static const COLORMAP sysColorMap[] = {
		// mapping from color in DIB to system color
		{ RGB(0x00, 0x00, 0x00),  COLOR_BTNTEXT },       // black
		{ RGB(0x80, 0x80, 0x80),  COLOR_BTNSHADOW },     // dark grey
		{ RGB(0xC0, 0xC0, 0xC0),  COLOR_BTNFACE },       // bright grey
		{ RGB(0xFF, 0xFF, 0xFF),  COLOR_BTNHIGHLIGHT }   // white
	};
	const int nMaps = 4;

	// look for matching RGBQUAD color in original
	for (int i = 0; i < nMaps; i++)
	{
		COLORREF clrSystem = afxGlobalData.GetColor(sysColorMap[i].iSysColorFrom);

		if (bUseRGBQUAD)
		{
			if (color == AFX_CLR_TO_RGBQUAD(clrSystem))
			{
				return AFX_CLR_TO_RGBQUAD(sysColorMap[i].rgbTo);
			}
		}
		else
		{
			if (color == clrSystem)
			{
				return sysColorMap[i].rgbTo;
			}
		}
	}

	return color;
}

BOOL CMFCToolBarImages::Save(LPCTSTR lpszBmpFileName)
{
	if (!m_bUserImagesList || // Only user-defined bitmaps can be saved!
		m_hbmImageWell == NULL) // Not loaded yet!
	{
		return FALSE;
	}

	if (m_bReadOnly)
	{
		return FALSE;
	}

	CString strFile;
	if (lpszBmpFileName == NULL)
	{
		strFile = m_strUDLPath;
	}
	else
	{
		strFile = lpszBmpFileName;
	}

	if (!m_bModified && strFile == m_strUDLPath)
	{
		return TRUE;
	}

	HANDLE hDib = DDBToDIB(m_hbmImageWell, 0);
	if (hDib == NULL)
	{
		TRACE(_T("CMFCToolBarImages::Save Can't convert DDB to DIB\n"));
		return FALSE;
	}

	BOOL bSuccess = WriteDIB(strFile, hDib);
	::GlobalFree(hDib);

	if (!bSuccess)
	{
		return FALSE;
	}

	m_bModified = FALSE;
	return TRUE;
}

static BOOL __stdcall WriteDIB( LPCTSTR szFile, HANDLE hDIB)
{
	BITMAPFILEHEADER hdr;
	LPBITMAPINFOHEADER lpbi;

	if (!hDIB)
		return FALSE;

	CFile file;
	if ( !file.Open(szFile, CFile::modeWrite | CFile::modeCreate))
	{
		return FALSE;
	}

	lpbi = (LPBITMAPINFOHEADER) hDIB;

	int nColors = 1 << lpbi->biBitCount;
	if (nColors > 256 || lpbi->biBitCount == 32)
		nColors = 0;

	// Fill in the fields of the file header
	hdr.bfType = ((WORD)('M' << 8) | 'B'); // is always "BM"
	hdr.bfSize = (DWORD)(GlobalSize(hDIB) + sizeof(BITMAPFILEHEADER));
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;
	hdr.bfOffBits = (DWORD)(sizeof( hdr ) + lpbi->biSize + nColors * sizeof(RGBQUAD));

	// Write the file header
	file.Write( &hdr, sizeof(hdr) );

	// Write the DIB header and the bits
	file.Write( lpbi, (UINT) GlobalSize(hDIB) );

	return TRUE;
}

static HANDLE __stdcall DDBToDIB(HBITMAP bitmap, DWORD dwCompression)
{
	BITMAP bm;
	BITMAPINFOHEADER bi;
	LPBITMAPINFOHEADER lpbi;
	DWORD dwLen;
	HANDLE hDIB;
	HANDLE handle;
	HDC hDC;
	HPALETTE hPal;

	// The function has no arg for bitfields
	if ( dwCompression == BI_BITFIELDS)
		return NULL;

	hPal = (HPALETTE) GetStockObject(DEFAULT_PALETTE);

	// Get bitmap information
	::GetObject(bitmap, sizeof(bm), (LPSTR)&bm);

	// Initialize the bitmapinfoheader
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bm.bmWidth;
	bi.biHeight = bm.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = (WORD)(bm.bmPlanes * bm.bmBitsPixel);
	bi.biCompression = dwCompression;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	// Compute the size of the  infoheader and the color table
	int nColors = (1 << bi.biBitCount);
	if (nColors > 256 || bi.biBitCount == 32)
		nColors = 0;
	dwLen  = bi.biSize + nColors * sizeof(RGBQUAD);

	// We need a device context to get the DIB from
	hDC = ::CreateCompatibleDC(NULL);
	if (hDC == NULL)
	{
		return FALSE;
	}

	HBITMAP bmp = ::CreateBitmap(1, 1, 1, bi.biBitCount, NULL);
	if (bmp == NULL)
	{
		::DeleteDC(hDC);
		return NULL;
	}

	HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hDC, bmp);

	hPal = SelectPalette(hDC,hPal,FALSE);
	RealizePalette(hDC);

	// Allocate enough memory to hold bitmapinfoheader and color table
	hDIB = GlobalAlloc(GMEM_FIXED,dwLen);

	if (!hDIB)
	{
		::SelectPalette(hDC,hPal,FALSE);

		if (hOldBitmap != NULL)
		{
			::SelectObject(hDC, hOldBitmap);
		}

		::DeleteObject(bmp);
		::DeleteDC(hDC);
		return NULL;
	}

	lpbi = (LPBITMAPINFOHEADER)hDIB;

	*lpbi = bi;

	// Call GetDIBits with a NULL lpBits param, so the device driver
	// will calculate the biSizeImage field
	GetDIBits(hDC, bitmap, 0L, (DWORD)bi.biHeight,
		(LPBYTE)NULL, (LPBITMAPINFO)lpbi, (DWORD)DIB_RGB_COLORS);

	bi = *lpbi;

	// If the driver did not fill in the biSizeImage field, then compute it
	// Each scan line of the image is aligned on a DWORD(32bit) boundary
	if (bi.biSizeImage == 0){bi.biSizeImage = ((((bi.biWidth * bi.biBitCount) + 31) & ~31) / 8) * bi.biHeight;

		// If a compression scheme is used the result may infact be larger
		// Increase the size to account for this.
		if (dwCompression != BI_RGB)
			bi.biSizeImage = (bi.biSizeImage * 3) / 2;
	}

	// Realloc the buffer so that it can hold all the bits
	dwLen += bi.biSizeImage;
	handle = GlobalReAlloc(hDIB, dwLen, GMEM_MOVEABLE);
	if (handle != NULL)
		hDIB = handle;
	else
	{
		GlobalFree(hDIB);

		// Reselect the original palette
		SelectPalette(hDC,hPal,FALSE);
		if (hOldBitmap != NULL)
		{
			::SelectObject(hDC, hOldBitmap);
		}
		::DeleteObject(bmp);
		::DeleteDC(hDC);
		return NULL;
	}

	// Get the bitmap bits
	lpbi = (LPBITMAPINFOHEADER)hDIB;

	// FINALLY get the DIB
	BOOL bGotBits = GetDIBits( hDC, bitmap, 0L, // Start scan line
		(DWORD)bi.biHeight, // # of scan lines
		(LPBYTE)lpbi 			// address for bitmap bits
		+(bi.biSize + nColors * sizeof(RGBQUAD)),
		(LPBITMAPINFO)lpbi, // address of bitmapinfo
		(DWORD)DIB_RGB_COLORS); // Use RGB for color table

	if ( !bGotBits )
	{
		GlobalFree(hDIB);

		SelectPalette(hDC,hPal,FALSE);
		if (hOldBitmap != NULL)
		{
			::SelectObject(hDC, hOldBitmap);
		}
		::DeleteObject(bmp);
		::DeleteDC(hDC);
		return NULL;
	}

	// Convert color table to the standard 3-d colors:
	DWORD* pColorTable = (DWORD*)(((LPBYTE)lpbi) +(UINT) lpbi->biSize);
	for (int iColor = 0; iColor < nColors; iColor ++)
	{
		pColorTable[iColor] = CMFCToolBarImages::MapFromSysColor(pColorTable[iColor]);
	}

	SelectPalette(hDC,hPal,FALSE);

	if (hOldBitmap != NULL)
	{
		::SelectObject(hDC, hOldBitmap);
	}

	::DeleteObject(bmp);
	::DeleteDC(hDC);

	return hDIB;
}

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarImages diagnostics

#ifdef _DEBUG
void CMFCToolBarImages::AssertValid() const
{
	CObject::AssertValid();

	ASSERT(m_hbmImageWell != NULL);
}

void CMFCToolBarImages::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	dc << "\nm_bUserImagesList = " << m_bUserImagesList;
	dc << "\nm_sizeImage = " << m_sizeImage;

	if (m_bUserImagesList)
	{
		dc << "\nm_strUDLPath = " << m_strUDLPath;
	}

	if (dc.GetDepth() > 0)
	{
	}

	dc << "\n";
}

#endif

BOOL CMFCToolBarImages::CopyImageToClipboard(int iImage)
{
	try
	{
		CWindowDC dc(NULL);

		// Create a bitmap copy:
		CDC memDCDest;
		memDCDest.CreateCompatibleDC(NULL);

		CBitmap bitmapCopy;
		if (!bitmapCopy.CreateCompatibleBitmap(&dc, m_sizeImage.cx, m_sizeImage.cy))
		{
			AfxMessageBox(IDP_AFXBARRES_CANT_COPY_BITMAP);
			return FALSE;
		}

		CBitmap* pOldBitmapDest = memDCDest.SelectObject(&bitmapCopy);

		memDCDest.FillRect(CRect(0, 0, m_sizeImage.cx, m_sizeImage.cy), &afxGlobalData.brBtnFace);

		CAfxDrawState ds;
		PrepareDrawImage(ds, FALSE);

		Draw(&memDCDest, 0, 0, iImage);
		EndDrawImage(ds);

		memDCDest.SelectObject(pOldBitmapDest);

		if (!AfxGetMainWnd()->OpenClipboard())
		{
			AfxMessageBox(IDP_AFXBARRES_CANT_COPY_BITMAP);
			return FALSE;
		}

		if (!::EmptyClipboard())
		{
			AfxMessageBox(IDP_AFXBARRES_CANT_COPY_BITMAP);
			::CloseClipboard();
			return FALSE;
		}

		HANDLE hclipData = ::SetClipboardData(CF_BITMAP, bitmapCopy.Detach());
		if (hclipData == NULL)
		{
			AfxMessageBox(IDP_AFXBARRES_CANT_COPY_BITMAP);
			TRACE(_T("CMFCToolBarImages::CopyImageToClipboard error. Error code = %x\n"), GetLastError());
		}

		::CloseClipboard();
		return TRUE;
	}
	catch(...)
	{
		AfxMessageBox(IDP_AFXBARRES_INTERLAL_ERROR);
	}

	return FALSE;
}

BOOL CMFCToolBarImages::CopyTo(CMFCToolBarImages& dest)
{
	if (dest.m_bIsTemporary)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (dest.IsValid())
	{
		dest.Clear();
	}

	if (afxGlobalData.bIsWindowsVista)
	{
		// Create memory source DC and select an original bitmap:
		CDC memDCSrc;
		memDCSrc.CreateCompatibleDC(NULL);

		HBITMAP hOldBitmapSrc = (HBITMAP) memDCSrc.SelectObject(m_hbmImageWell);

		if (hOldBitmapSrc == NULL)
		{
			return FALSE;
		}

		// Create a new bitmap compatibel with the source memory DC
		//(original bitmap SHOULD BE ALREADY SELECTED!):
		HBITMAP hNewBitmap = (HBITMAP) ::CreateCompatibleBitmap(memDCSrc, m_iCount * m_sizeImage.cx, m_sizeImage.cy);
		if (hNewBitmap == NULL)
		{
			memDCSrc.SelectObject(hOldBitmapSrc);
			return FALSE;
		}

		// Create memory destination DC and select a new bitmap:
		CDC memDCDst;
		memDCDst.CreateCompatibleDC(&memDCSrc);

		HBITMAP hOldBitmapDst = (HBITMAP) memDCDst.SelectObject(hNewBitmap);
		if (hOldBitmapDst == NULL)
		{
			memDCSrc.SelectObject(hOldBitmapSrc);
			::DeleteObject(hNewBitmap);
			return FALSE;
		}

		// Copy original bitmap to new:
		memDCDst.BitBlt(0, 0, m_iCount * m_sizeImage.cx, m_sizeImage.cy, &memDCSrc, 0, 0, SRCCOPY);

		memDCDst.SelectObject(hOldBitmapDst);
		memDCSrc.SelectObject(hOldBitmapSrc);

		dest.m_hbmImageWell = hNewBitmap;
	}
	else
	{
		dest.m_hbmImageWell = (HBITMAP) ::CopyImage(m_hbmImageWell, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
		if (m_hbmImageLight != NULL)
		{
			dest.m_hbmImageLight = (HBITMAP) ::CopyImage(m_hbmImageLight, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
		}
		if (m_hbmImageShadow != NULL)
		{
			dest.m_hbmImageShadow = (HBITMAP) ::CopyImage(m_hbmImageShadow, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
		}
	}

	dest.m_sizeImage = m_sizeImage;
	dest.m_sizeImageDest = m_sizeImageDest;
	dest.m_bUserImagesList = m_bUserImagesList;
	dest.m_strUDLPath = m_strUDLPath;
	dest.m_bModified = m_bModified;
	dest.m_iCount = m_iCount;
	dest.m_clrTransparent = m_clrTransparent;
	dest.m_bReadOnly = m_bReadOnly;
	dest.m_clrImageShadow = m_clrImageShadow;
	dest.m_bFadeInactive = m_bFadeInactive;
	dest.m_nBitsPerPixel = m_nBitsPerPixel;

	for (POSITION pos = m_lstOrigResIds.GetHeadPosition(); pos != NULL;)
	{
		UINT uiResId = m_lstOrigResIds.GetNext(pos);

		dest.m_lstOrigResIds.AddTail(uiResId);

		int iOffset = -1;
		if (m_mapOrigResOffsets.Lookup(uiResId, iOffset))
		{
			dest.m_mapOrigResOffsets.SetAt(uiResId, iOffset);
		}
	}

	for (POSITION posInst = m_lstOrigResInstances.GetHeadPosition(); posInst != NULL;)
	{
		HINSTANCE hInst = m_lstOrigResInstances.GetNext(posInst);
		dest.m_lstOrigResInstances.AddTail(hInst);
	}

	return TRUE;
}

void CMFCToolBarImages::Clear()
{
	if (m_bIsTemporary)
	{
		ASSERT(FALSE);
		return;
	}

	AfxDeleteObject((HGDIOBJ*)&m_hbmImageWell);
	m_hbmImageWell = NULL;
	AfxDeleteObject((HGDIOBJ*)&m_hbmImageLight);
	m_hbmImageLight = NULL;
	AfxDeleteObject((HGDIOBJ*)&m_hbmImageShadow);
	m_hbmImageShadow = NULL;

	m_lstOrigResIds.RemoveAll();
	m_mapOrigResOffsets.RemoveAll();
	m_lstOrigResInstances.RemoveAll();
	m_strUDLPath.Empty();

	m_bUserImagesList = FALSE;
	m_iCount = 0;
	m_bModified = FALSE;
	m_bIsGray = FALSE;
	m_nGrayImageLuminancePercentage = 0;
	m_nBitsPerPixel = 0;
}

void __stdcall CMFCToolBarImages::TransparentBlt( HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight,
	CDC* pDcSrc, int nXSrc, int nYSrc, COLORREF colorTransparent, int nWidthDest/* = -1*/, int nHeightDest/* = -1*/)
{
	int cx = nWidthDest == -1 ? nWidth : nWidthDest;
	int cy = nHeightDest == -1 ? nHeight : nHeightDest;

	if (!m_bIsRTL)
	{
		if (::TransparentBlt(hdcDest, nXDest, nYDest, cx, cy, pDcSrc->GetSafeHdc(), nXSrc, nYSrc, nWidth, nHeight, colorTransparent))
		{
			return;
		}
	}

	CDC dc, memDC, maskDC;
	dc.Attach( hdcDest );
	maskDC.CreateCompatibleDC(&dc);
	CBitmap maskBitmap;

	//add these to store return of SelectObject() calls
	CBitmap* pOldMemBmp = NULL;
	CBitmap* pOldMaskBmp = NULL;

	memDC.CreateCompatibleDC(&dc);
	CBitmap bmpImage;
	bmpImage.CreateCompatibleBitmap( &dc, cx, cy);
	pOldMemBmp = memDC.SelectObject( &bmpImage );

	if (nWidthDest == -1 ||(nWidthDest == nWidth && nHeightDest == nHeight))
	{
		memDC.BitBlt( 0,0,nWidth, nHeight, pDcSrc, nXSrc, nYSrc, SRCCOPY);
	}
	else
	{
		memDC.StretchBlt(0,0, nWidthDest, nHeightDest, pDcSrc, nXSrc, nYSrc, nWidth, nHeight, SRCCOPY);
	}

	// Create monochrome bitmap for the mask
	maskBitmap.CreateBitmap(cx, cy, 1, 1, NULL );
	pOldMaskBmp = maskDC.SelectObject( &maskBitmap );
	memDC.SetBkColor( colorTransparent );

	// Create the mask from the memory DC
	maskDC.BitBlt(0, 0, cx, cy, &memDC, 0, 0, SRCCOPY);

	// Set the background in memDC to black. Using SRCPAINT with black
	// and any other color results in the other color, thus making
	// black the transparent color
	memDC.SetBkColor(RGB(0,0,0));
	memDC.SetTextColor(RGB(255,255,255));
	memDC.BitBlt(0, 0, cx, cy, &maskDC, 0, 0, SRCAND);

	// Set the foreground to black. See comment above.
	dc.SetBkColor(RGB(255,255,255));
	dc.SetTextColor(RGB(0,0,0));

	dc.BitBlt(nXDest, nYDest, cx, cy, &maskDC, 0, 0, SRCAND);

	// Combine the foreground with the background
	dc.BitBlt(nXDest, nYDest, cx, cy, &memDC, 0, 0, SRCPAINT);

	if (pOldMaskBmp)
		maskDC.SelectObject( pOldMaskBmp );
	if (pOldMemBmp)
		memDC.SelectObject( pOldMemBmp );

	dc.Detach();
}

BOOL __stdcall CMFCToolBarImages::MapBmpTo3dColors( HBITMAP& hBmp, BOOL bUseRGBQUAD/* = TRUE*/, COLORREF clrSrc/* = (COLORREF)-1*/, COLORREF clrDest/* = (COLORREF)-1*/)
{
	if (hBmp == NULL)
	{
		return FALSE;
	}

	if (clrSrc != (COLORREF)-1 && clrDest == (COLORREF)-1)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	// Create memory source DC and select an original bitmap:
	CDC memDCSrc;
	memDCSrc.CreateCompatibleDC(NULL);

	HBITMAP hOldBitmapSrc = NULL;

	int iBitmapWidth;
	int iBitmapHeight;

	// Get original bitmap attrbutes:
	BITMAP bmp;
	if (::GetObject(hBmp, sizeof(BITMAP), &bmp) == 0)
	{
		return FALSE;
	}

	hOldBitmapSrc = (HBITMAP) memDCSrc.SelectObject(hBmp);
	if (hOldBitmapSrc == NULL)
	{
		return FALSE;
	}

	iBitmapWidth = bmp.bmWidth;
	iBitmapHeight = bmp.bmHeight;

	// Create a new bitmap compatibel with the source memory DC:
	// (original bitmap SHOULD BE ALREADY SELECTED!):
	HBITMAP hNewBitmap = (HBITMAP) ::CreateCompatibleBitmap(memDCSrc, iBitmapWidth, iBitmapHeight);
	if (hNewBitmap == NULL)
	{
		memDCSrc.SelectObject(hOldBitmapSrc);
		return FALSE;
	}

	// Create memory destination DC:
	CDC memDCDst;
	memDCDst.CreateCompatibleDC(&memDCSrc);

	HBITMAP hOldBitmapDst = (HBITMAP) memDCDst.SelectObject(hNewBitmap);
	if (hOldBitmapDst == NULL)
	{
		memDCSrc.SelectObject(hOldBitmapSrc);
		::DeleteObject(hNewBitmap);
		return FALSE;
	}

	// Copy original bitmap to new:
	memDCDst.BitBlt(0, 0, iBitmapWidth, iBitmapHeight, &memDCSrc, 0, 0, SRCCOPY);

	// Change a specific colors to system:
	for (int x = 0; x < iBitmapWidth; x ++)
	{
		for (int y = 0; y < iBitmapHeight; y ++)
		{
			COLORREF clrOrig = ::GetPixel(memDCDst, x, y);

			if (clrSrc != (COLORREF)-1)
			{
				if (clrOrig == clrSrc)
				{
					::SetPixel(memDCDst, x, y, clrDest);
				}
			}
			else
			{
				COLORREF clrNew = bmp.bmBitsPixel == 24 && !m_bDisableTrueColorAlpha ? MapToSysColorAlpha(clrOrig) : MapToSysColor(clrOrig, bUseRGBQUAD);

				if (clrOrig != clrNew)
				{
					::SetPixel(memDCDst, x, y, clrNew);
				}
			}
		}
	}

	memDCDst.SelectObject(hOldBitmapDst);
	memDCSrc.SelectObject(hOldBitmapSrc);

	::DeleteObject(hBmp);
	hBmp = hNewBitmap;

	return TRUE;
}

BOOL CMFCToolBarImages::MapTo3dColors(BOOL bUseRGBQUAD/* = TRUE*/, COLORREF clrSrc/* = (COLORREF)-1*/, COLORREF clrDest/* = (COLORREF)-1*/)
{
	return MapBmpTo3dColors(m_hbmImageWell, bUseRGBQUAD, clrSrc, clrDest);
}

void CMFCToolBarImages::CopyTemp(CMFCToolBarImages& imagesDest)
{
	imagesDest.Clear();
	imagesDest.m_bIsTemporary = TRUE;

	imagesDest.m_sizeImage = m_sizeImage;
	imagesDest.m_sizeImageDest = m_sizeImageDest;
	imagesDest.m_hbmImageWell = m_hbmImageWell;
	imagesDest.m_bUserImagesList = m_bUserImagesList;
	imagesDest.m_iCount = m_iCount;
	imagesDest.m_bReadOnly = TRUE;
	imagesDest.m_nBitsPerPixel = m_nBitsPerPixel;
}

BOOL CMFCToolBarImages::UpdateInternalImage(int nIndex)
{
	HBITMAP& hbmpInternal = (nIndex == AFX_IMAGE_LIGHT) ? m_hbmImageLight : m_hbmImageShadow;

	if (nIndex == AFX_IMAGE_LIGHT)
	{
		if ((m_nBitsPerPixel > 4 && !m_bAlwaysLight) || m_nBitsPerPixel == 0)
		{
			// Down't fade 256+ or unknown bitmaps
			return FALSE;
		}
	}

	AfxDeleteObject((HGDIOBJ*)&hbmpInternal);
	hbmpInternal = NULL;

	if (m_hbmImageWell == NULL)
	{
		return TRUE;
	}

	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	::GetVersionEx(&osvi);

	if (afxGlobalData.m_nBitsPerPixel <= 8 || osvi.dwPlatformId != VER_PLATFORM_WIN32_NT)
	{
		return TRUE;
	}

	// Create memory source DC and select an original bitmap:
	CDC memDCSrc;
	memDCSrc.CreateCompatibleDC(NULL);

	BITMAP bmp;
	if (::GetObject(m_hbmImageWell, sizeof(BITMAP), &bmp) == 0)
	{
		return FALSE;
	}

	int iBitmapWidth = bmp.bmWidth;
	int iBitmapHeight = bmp.bmHeight;

	HBITMAP hOldBitmapSrc = (HBITMAP) memDCSrc.SelectObject(m_hbmImageWell);
	if (hOldBitmapSrc == NULL)
	{
		return FALSE;
	}

	// Create memory destination DC and select a new bitmap:
	CDC memDCDst;
	memDCDst.CreateCompatibleDC(&memDCSrc);

	BITMAPINFO bi;

	// Fill in the BITMAPINFOHEADER
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = iBitmapWidth;
	bi.bmiHeader.biHeight = iBitmapHeight;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = iBitmapWidth * iBitmapHeight;
	bi.bmiHeader.biXPelsPerMeter = 0;
	bi.bmiHeader.biYPelsPerMeter = 0;
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;

	COLORREF* pBits = NULL;
	hbmpInternal = CreateDIBSection(memDCDst.m_hDC, &bi, DIB_RGB_COLORS, (void **)&pBits, NULL, NULL);

	if (hbmpInternal == NULL)
	{
		memDCSrc.SelectObject(hOldBitmapSrc);
		return FALSE;
	}

	HBITMAP hOldBitmapDst = (HBITMAP) memDCDst.SelectObject(hbmpInternal);
	if (hOldBitmapDst == NULL)
	{
		memDCSrc.SelectObject(hOldBitmapSrc);
		::DeleteObject(hbmpInternal);
		hbmpInternal = NULL;
		return FALSE;
	}

	// Copy original bitmap to new:
	COLORREF clrTransparent = m_nBitsPerPixel == 32 ? (COLORREF) -1 : m_clrTransparent;

	memDCDst.BitBlt(0, 0, iBitmapWidth, iBitmapHeight, &memDCSrc, 0, 0, SRCCOPY);

	if (nIndex == AFX_IMAGE_LIGHT)
	{
		CDrawingManager dm(memDCDst);

		dm.HighlightRect(CRect(0, 0, iBitmapWidth, iBitmapHeight), m_nLightPercentage, clrTransparent == -1 ? afxGlobalData.clrBtnFace : clrTransparent);
	}
	else
	{
		COLORREF clrTr = clrTransparent == -1 ? afxGlobalData.clrBtnFace : clrTransparent;

		COLORREF clrHL = CMFCVisualManager::GetInstance()->GetToolbarHighlightColor();
		COLORREF clrShadow = afxGlobalData.m_nBitsPerPixel <= 8 ? afxGlobalData.clrBtnShadow : CDrawingManager::PixelAlpha(clrHL, 67);

		for (int x = 0; x < iBitmapWidth; x++)
		{
			for (int y = 0; y < iBitmapHeight; y++)
			{
				COLORREF clr = memDCDst.GetPixel(x, y);
				if (clr != clrTr)
				{
					memDCDst.SetPixel(x, y, clrShadow);
				}
			}
		}
	}

	memDCDst.SelectObject(hOldBitmapDst);
	memDCSrc.SelectObject(hOldBitmapSrc);

	return TRUE;
}

BOOL __stdcall CMFCToolBarImages::PreMultiplyAlpha(HBITMAP hbmp, BOOL bAutoCheckPremlt)
{
	DIBSECTION ds;
	if (::GetObject(hbmp, sizeof(DIBSECTION), &ds) == 0)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (ds.dsBm.bmBitsPixel != 32)
	{
		return FALSE;
	}

	if (ds.dsBm.bmBits == NULL)
	{
		return FALSE;
	}

	int i = 0;

	RGBQUAD* pBits = (RGBQUAD*) ds.dsBm.bmBits;
	const int length = ds.dsBm.bmWidth * ds.dsBm.bmHeight;

	if (bAutoCheckPremlt)
	{
		BOOL bPremultiply = FALSE;

		RGBQUAD* pBit = pBits;
		for (i = 0; i < length; i++)
		{
			if (pBit->rgbRed   > pBit->rgbReserved || pBit->rgbGreen > pBit->rgbReserved || pBit->rgbBlue  > pBit->rgbReserved)
			{
				bPremultiply = TRUE;
				break;
			}

			pBit++;
		}

		if (!bPremultiply)
		{
			return TRUE;
		}
	}

	// Premultiply the R,G and B values with the Alpha channel values:
	RGBQUAD* pBit = pBits;
	for (i = 0; i < length; i++)
	{
		pBit->rgbRed   = (BYTE)(pBit->rgbRed   * pBit->rgbReserved / 255);
		pBit->rgbGreen = (BYTE)(pBit->rgbGreen * pBit->rgbReserved / 255);
		pBit->rgbBlue  = (BYTE)(pBit->rgbBlue  * pBit->rgbReserved / 255);
		pBit++;
	}

	return TRUE;
}

BOOL CMFCToolBarImages::PreMultiplyAlpha(HBITMAP hbmp)
{
	return PreMultiplyAlpha(hbmp, m_bAutoCheckPremlt);
}

BOOL CMFCToolBarImages::CreateFromImageList(const CImageList& imageList)
{
	ENSURE(imageList.GetSafeHandle() != NULL);
	ENSURE(imageList.GetImageCount() > 0);

	Clear();

	IMAGEINFO info;
	imageList.GetImageInfo(0, &info);

	CRect rectImage = info.rcImage;
	m_sizeImage = rectImage.Size();

	for (int i = 0; i < imageList.GetImageCount(); i++)
	{
		HICON hIcon = ((CImageList&) imageList).ExtractIcon(i);
		ENSURE(hIcon != NULL);

		AddIcon(hIcon);

		::DestroyIcon(hIcon);
	}

	return TRUE;
}

BOOL __stdcall CMFCToolBarImages::Is32BitTransparencySupported()
{
	return afxGlobalData.bIsOSAlphaBlendingSupport;
}

BOOL CMFCToolBarImages::GrayImages(int nGrayImageLuminancePercentage)
{
	m_bIsGray = TRUE;
	m_nGrayImageLuminancePercentage = nGrayImageLuminancePercentage;

	if (m_hbmImageWell == NULL)
	{
		return TRUE;
	}

	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	::GetVersionEx(&osvi);

	if (afxGlobalData.m_nBitsPerPixel <= 8 || osvi.dwPlatformId != VER_PLATFORM_WIN32_NT)
	{
		return TRUE;
	}

	// Create memory source DC and select an original bitmap:
	CDC memDCSrc;
	memDCSrc.CreateCompatibleDC(NULL);

	BITMAP bmp;
	if (::GetObject(m_hbmImageWell, sizeof(BITMAP), &bmp) == 0)
	{
		return FALSE;
	}

	int iBitmapWidth = bmp.bmWidth;
	int iBitmapHeight = bmp.bmHeight;

	HBITMAP hOldBitmapSrc = (HBITMAP) memDCSrc.SelectObject(m_hbmImageWell);
	if (hOldBitmapSrc == NULL)
	{
		return FALSE;
	}

	// Create memory destination DC and select a new bitmap:
	CDC memDCDst;
	memDCDst.CreateCompatibleDC(&memDCSrc);

	BITMAPINFO bi;

	// Fill in the BITMAPINFOHEADER
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = iBitmapWidth;
	bi.bmiHeader.biHeight = iBitmapHeight;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = iBitmapWidth * iBitmapHeight;
	bi.bmiHeader.biXPelsPerMeter = 0;
	bi.bmiHeader.biYPelsPerMeter = 0;
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;

	COLORREF* pBits = NULL;
	HBITMAP hNewBitmap = CreateDIBSection(memDCDst.m_hDC, &bi, DIB_RGB_COLORS, (void **)&pBits, NULL, NULL);

	if (hNewBitmap == NULL)
	{
		memDCSrc.SelectObject(hOldBitmapSrc);
		return FALSE;
	}

	HBITMAP hOldBitmapDst = (HBITMAP) memDCDst.SelectObject(hNewBitmap);
	if (hOldBitmapDst == NULL)
	{
		memDCSrc.SelectObject(hOldBitmapSrc);
		::DeleteObject(hNewBitmap);
		hNewBitmap = NULL;
		return FALSE;
	}

	// Copy original bitmap to new:
	memDCDst.BitBlt(0, 0, iBitmapWidth, iBitmapHeight, &memDCSrc, 0, 0, SRCCOPY);

	int nPercentage = m_nGrayImageLuminancePercentage <= 0 ? 130 : m_nGrayImageLuminancePercentage;

	if (m_nBitsPerPixel == 32)
	{
		DIBSECTION ds;
		if (::GetObject(hNewBitmap, sizeof(DIBSECTION), &ds) == 0)
		{
			ASSERT(FALSE);
			return FALSE;
		}

		if (ds.dsBm.bmBitsPixel != 32)
		{
			ASSERT(FALSE);
			return FALSE;
		}

		if (ds.dsBm.bmBits == NULL)
		{
			ASSERT(FALSE);
			return FALSE;
		}

		RGBQUAD* pBits32 = (RGBQUAD*) ds.dsBm.bmBits;

		// Premultiply the R,G and B values with the Alpha channel values:
		for (int i = 0; i < ds.dsBm.bmWidth * ds.dsBm.bmHeight; i++)
		{
			RGBQUAD* pBit = pBits32 + i;

			double H,S,L;
			CDrawingManager::RGBtoHSL(RGB(pBit->rgbRed, pBit->rgbGreen, pBit->rgbBlue), &H, &S, &L);
			COLORREF color = CDrawingManager::PixelAlpha(CDrawingManager::HLStoRGB_ONE(H,L,0), .01 * nPercentage, .01 * nPercentage, .01 * nPercentage);

			pBit->rgbRed = (BYTE)(GetRValue(color) * pBit->rgbReserved / 255);
			pBit->rgbGreen = (BYTE)(GetGValue(color) * pBit->rgbReserved / 255);
			pBit->rgbBlue = (BYTE)(GetBValue(color) * pBit->rgbReserved / 255);
		}
	}
	else
	{
		CDrawingManager dm(memDCDst);

		dm.GrayRect(CRect(0, 0, iBitmapWidth, iBitmapHeight), nPercentage, m_clrTransparent == -1 ? afxGlobalData.clrBtnFace : m_clrTransparent);
	}

	memDCDst.SelectObject(hOldBitmapDst);
	memDCSrc.SelectObject(hOldBitmapSrc);

	::DeleteObject(m_hbmImageWell);
	m_hbmImageWell = hNewBitmap;

	return TRUE;
}

BOOL __stdcall CMFCToolBarImages::MirrorBitmap(HBITMAP& hbmp, int cxImage)
{
	if (hbmp == NULL)
	{
		return TRUE;
	}

	BITMAP bmp;
	if (::GetObject(hbmp, sizeof(BITMAP), &bmp) == 0)
	{
		return FALSE;
	}

	const int cx = bmp.bmWidth;
	const int cy = bmp.bmHeight;
	const int iCount = cx / cxImage;

	if (bmp.bmBitsPixel == 32)
	{
		DIBSECTION ds;
		if (::GetObject(hbmp, sizeof(DIBSECTION), &ds) == 0)
		{
			ASSERT(FALSE);
			return FALSE;
		}

		if (ds.dsBm.bmBitsPixel != 32)
		{
			ASSERT(FALSE);
			return FALSE;
		}

		if (ds.dsBm.bmBits == NULL)
		{
			ASSERT(FALSE);
			return FALSE;
		}

		DWORD* pBits = (DWORD*) ds.dsBm.bmBits;

		for (int iImage = 0; iImage < iCount; iImage++)
		{
			for (int y = 0; y < cy; y++)
			{
				DWORD* pRow1 = pBits + cx * y + iImage * cxImage;
				DWORD* pRow2 = pRow1 + cxImage - 1;

				for (int x = 0; x < cxImage / 2; x++)
				{
					DWORD color = *pRow1;

					*pRow1 = *pRow2;
					*pRow2 = color;

					pRow1++;
					pRow2--;
				}
			}
		}

		return TRUE;
	}

	CDC memDC;
	memDC.CreateCompatibleDC(NULL);

	HBITMAP hOldBitmap = (HBITMAP) memDC.SelectObject(hbmp);
	if (hOldBitmap == NULL)
	{
		return FALSE;
	}

	for (int iImage = 0; iImage < iCount; iImage++)
	{
		for (int y = 0; y < cy; y++)
		{
			int x1 = iImage * cxImage;
			int x2 = x1 + cxImage - 1;

			for (int x = 0; x < cxImage / 2; x++)
			{
				COLORREF color = memDC.GetPixel(x1, y);

				memDC.SetPixel(x1, y, memDC.GetPixel(x2, y));
				memDC.SetPixel(x2, y, color);

				x1++;
				x2--;
			}
		}
	}

	memDC.SelectObject(hOldBitmap);

	return TRUE;
}

BOOL CMFCToolBarImages::Mirror()
{
	if (!MirrorBitmap(m_hbmImageWell, m_sizeImage.cx))
	{
		return FALSE;
	}

	if (m_hbmImageLight != NULL)
	{
		MirrorBitmap(m_hbmImageLight, m_sizeImage.cx);
	}

	if (m_hbmImageShadow != NULL)
	{
		MirrorBitmap(m_hbmImageShadow, m_sizeImage.cx);
	}

	return TRUE;
}

BOOL __stdcall CMFCToolBarImages::MirrorBitmapVert(HBITMAP& hbmp, int cyImage)
{
	if (hbmp == NULL)
	{
		return TRUE;
	}

	BITMAP bmp;
	if (::GetObject(hbmp, sizeof(BITMAP), &bmp) == 0)
	{
		return FALSE;
	}

	const int cx = bmp.bmWidth;
	const int cy = bmp.bmHeight;
	const int iCount = cy / cyImage;

	if (bmp.bmBitsPixel >= 16)
	{
		DIBSECTION ds;
		if (::GetObject(hbmp, sizeof(DIBSECTION), &ds) == 0)
		{
			ASSERT(FALSE);
			return FALSE;
		}

		if (ds.dsBm.bmBitsPixel != bmp.bmBitsPixel)
		{
			ASSERT(FALSE);
			return FALSE;
		}

		if (ds.dsBm.bmBits == NULL)
		{
			ASSERT(FALSE);
			return FALSE;
		}

		LPBYTE pBits    = (LPBYTE)ds.dsBm.bmBits;
		DWORD pitch     = cx * ds.dsBm.bmBitsPixel / 8;
		if (pitch % 4)
		{
			pitch = (DWORD)(pitch / 4) * 4 + 4;
		}

		LPBYTE pRowTemp = new BYTE[pitch];

		for (int iImage = 0; iImage < iCount; iImage++)
		{
			LPBYTE pRowBits1 = pBits + iImage * cyImage * pitch;
			LPBYTE pRowBits2 = pRowBits1 +(cyImage - 1) * pitch;

			for (int y = 0; y < cyImage / 2; y++)
			{
				memcpy(pRowTemp, pRowBits1, pitch);
				memcpy(pRowBits1, pRowBits2, pitch);
				memcpy(pRowBits2, pRowTemp, pitch);

				pRowBits1 += pitch;
				pRowBits2 -= pitch;
			}
		}

		delete [] pRowTemp;

		return TRUE;
	}

	CDC memDC;
	memDC.CreateCompatibleDC(NULL);

	HBITMAP hOldBitmap = (HBITMAP) memDC.SelectObject(hbmp);
	if (hOldBitmap == NULL)
	{
		return FALSE;
	}

	for (int iImage = 0; iImage < iCount; iImage++)
	{
		int y1 = iImage * cyImage;
		int y2 = y1 + cyImage - 1;

		for (int y = 0; y < cyImage / 2; y++)
		{
			for (int x = 0; x < cx; x++)
			{
				COLORREF color = memDC.GetPixel(x, y1);

				memDC.SetPixel(x, y1, memDC.GetPixel(x, y2));
				memDC.SetPixel(x, y2, color);
			}

			y1++;
			y2--;
		}
	}

	memDC.SelectObject(hOldBitmap);

	return TRUE;
}

BOOL CMFCToolBarImages::MirrorVert()
{
	if (!MirrorBitmapVert(m_hbmImageWell, m_sizeImage.cy))
	{
		return FALSE;
	}

	if (m_hbmImageLight != NULL)
	{
		MirrorBitmapVert(m_hbmImageLight, m_sizeImage.cy);
	}

	if (m_hbmImageShadow != NULL)
	{
		MirrorBitmapVert(m_hbmImageShadow, m_sizeImage.cy);
	}

	return TRUE;
}

void __stdcall CMFCToolBarImages::EnableRTL(BOOL bIsRTL/* = TRUE*/)
{
	m_bIsRTL = bIsRTL;
}

void CMFCToolBarImages::AdaptColors(COLORREF clrBase, COLORREF clrTone)
{
	double dSrcH, dSrcS, dSrcL;
	CDrawingManager::RGBtoHSL(clrBase, &dSrcH, &dSrcS, &dSrcL);

	double dDestH, dDestS, dDestL;
	CDrawingManager::RGBtoHSL(clrTone, &dDestH, &dDestS, &dDestL);

	double DH = dDestH - dSrcH;
	double DL = dDestL - dSrcL;
	double DS = dDestS - dSrcS;

	if (m_nBitsPerPixel == 32)
	{
		DIBSECTION ds;
		if (::GetObject(m_hbmImageWell, sizeof(DIBSECTION), &ds) == 0)
		{
			ASSERT(FALSE);
			return;
		}

		if (ds.dsBm.bmBitsPixel != 32)
		{
			ASSERT(FALSE);
			return;
		}

		if (ds.dsBm.bmBits == NULL)
		{
			ASSERT(FALSE);
			return;
		}

		RGBQUAD* pBits = (RGBQUAD*) ds.dsBm.bmBits;

		for (int i = 0; i < ds.dsBm.bmWidth * ds.dsBm.bmHeight; i++)
		{
			RGBQUAD* pBit = pBits + i;

			if (pBit->rgbReserved == 0)
			{
				continue;
			}

			COLORREF clrOrig = RGB(pBit->rgbRed, pBit->rgbGreen, pBit->rgbBlue);

			double H,S,L;
			CDrawingManager::RGBtoHSL(clrOrig, &H, &S, &L);

			double HNew = max(0, min(1., H + DH));
			double SNew = max(0, min(1., S + DS));
			double LNew = max(0, min(1., L + DL));

			COLORREF color = CDrawingManager::HLStoRGB_ONE(HNew, dDestH > 0.5 ? L : LNew, SNew);

			pBit->rgbRed = (BYTE)(GetRValue(color));
			pBit->rgbGreen = (BYTE)(GetGValue(color));
			pBit->rgbBlue = (BYTE)(GetBValue(color));
		}

		return;
	}

	// Create memory source DC and select an original bitmap:
	CDC memDCSrc;
	memDCSrc.CreateCompatibleDC(NULL);

	HBITMAP hOldBitmapSrc = NULL;

	int iBitmapWidth;
	int iBitmapHeight;

	// Get original bitmap attrbutes:
	BITMAP bmp;
	if (::GetObject(m_hbmImageWell, sizeof(BITMAP), &bmp) == 0)
	{
		return;
	}

	hOldBitmapSrc = (HBITMAP) memDCSrc.SelectObject(m_hbmImageWell);
	if (hOldBitmapSrc == NULL)
	{
		return;
	}

	iBitmapWidth = bmp.bmWidth;
	iBitmapHeight = bmp.bmHeight;

	// Create a new bitmap compatibel with the source memory DC:
	//(original bitmap SHOULD BE ALREADY SELECTED!):
	HBITMAP hNewBitmap = (HBITMAP) ::CreateCompatibleBitmap(memDCSrc, iBitmapWidth, iBitmapHeight);
	if (hNewBitmap == NULL)
	{
		memDCSrc.SelectObject(hOldBitmapSrc);
		return;
	}

	// Create memory destination DC:
	CDC memDCDst;
	memDCDst.CreateCompatibleDC(&memDCSrc);

	HBITMAP hOldBitmapDst = (HBITMAP) memDCDst.SelectObject(hNewBitmap);
	if (hOldBitmapDst == NULL)
	{
		memDCSrc.SelectObject(hOldBitmapSrc);
		::DeleteObject(hNewBitmap);
		return;
	}

	// Copy original bitmap to new:
	memDCDst.BitBlt(0, 0, iBitmapWidth, iBitmapHeight, &memDCSrc, 0, 0, SRCCOPY);

	COLORREF clrTransparent = m_clrTransparent == (COLORREF)-1 ? afxGlobalData.clrBtnFace : m_clrTransparent;

	for (int x = 0; x < iBitmapWidth; x ++)
	{
		for (int y = 0; y < iBitmapHeight; y ++)
		{
			COLORREF clrOrig = ::GetPixel(memDCDst, x, y);

			if (clrOrig == clrTransparent)
			{
				continue;
			}

			double H, L, S;
			CDrawingManager::RGBtoHSL(clrOrig, &H, &S, &L);

			double HNew = max(0, min(1., H + DH));
			double SNew = max(0, min(1., S + DS));
			double LNew = max(0, min(1., L + DL));

			COLORREF clrNew = CDrawingManager::HLStoRGB_ONE(HNew, dDestH > 0.5 ? L : LNew, SNew);

			if (clrOrig != clrNew)
			{
				::SetPixel(memDCDst, x, y, clrNew);
			}
		}
	}

	memDCDst.SelectObject(hOldBitmapDst);
	memDCSrc.SelectObject(hOldBitmapSrc);

	::DeleteObject(m_hbmImageWell);
	m_hbmImageWell = hNewBitmap;

	AfxDeleteObject((HGDIOBJ*)&m_hbmImageLight);
	m_hbmImageLight = NULL;

	AfxDeleteObject((HGDIOBJ*)&m_hbmImageShadow);
	m_hbmImageShadow = NULL;
}

HRGN __stdcall CMFCToolBarImages::CreateRegionFromImage(HBITMAP hbmp, COLORREF clrTransparent)
{
	if (hbmp == NULL)
	{
		ASSERT(FALSE);
		return NULL;
	}

	BITMAP bmp;
	if (::GetObject(hbmp, sizeof(BITMAP), &bmp) == 0)
	{
		ASSERT(FALSE);
		return NULL;
	}

	CDC dc;
	dc.CreateCompatibleDC(NULL);

	HBITMAP hbmpOld = (HBITMAP) dc.SelectObject(hbmp);

	int iBitmapWidth = bmp.bmWidth;
	int iBitmapHeight = bmp.bmHeight;

	CRgn rgnAll;
	rgnAll.CreateRectRgn(0, 0, iBitmapWidth, iBitmapHeight);

	for (int y = 0; y < iBitmapHeight; y++)
	{
		for (int x = 0; x < iBitmapWidth; x++)
		{
			COLORREF color = dc.GetPixel(x, y);

			if (color == clrTransparent)
			{
				CRgn rgnPoint;
				rgnPoint.CreateRectRgn(x, y, x + 1, y + 1);

				rgnAll.CombineRgn(&rgnAll, &rgnPoint, RGN_DIFF);
			}
		}
	}

	dc.SelectObject(hbmpOld);

	return(HRGN) rgnAll.Detach();
}

void CMFCToolBarImages::SetSingleImage()
{
	if (m_hbmImageWell == NULL)
	{
		return;
	}

	BITMAP bmp;
	if (::GetObject(m_hbmImageWell, sizeof(BITMAP), &bmp) == 0)
	{
		ASSERT(FALSE);
		return;
	}

	m_sizeImage.cx = bmp.bmWidth;
	m_sizeImage.cy = bmp.bmHeight;

	m_iCount = 1;

	UpdateInternalImage(AFX_IMAGE_LIGHT);
	UpdateInternalImage(AFX_IMAGE_SHADOW);
}
//////////////////////////////////////////////////////////////////////
// CPngImage

CImage* CPngImage::m_pImage;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPngImage::CPngImage()
{
}

CPngImage::~CPngImage()
{
}

//////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////

BOOL CPngImage::Load(UINT uiResID, HINSTANCE hinstRes)
{
	return Load(MAKEINTRESOURCE(uiResID), hinstRes);
}

//*******************************************************************************
BOOL CPngImage::Load (LPCTSTR lpszResourceName, HINSTANCE hinstRes)
{
	if (!afxGlobalData.bGDIPlusSupport)
	{
		return FALSE;
	}

	if (hinstRes == NULL)
	{
		hinstRes = AfxFindResourceHandle(lpszResourceName, CMFCToolBarImages::m_strPngResType);
	}

	HRSRC hRsrc = ::FindResource(hinstRes, lpszResourceName, CMFCToolBarImages::m_strPngResType);
	if (hRsrc == NULL)
	{
		return FALSE;
	}

	HGLOBAL hGlobal = LoadResource (hinstRes, hRsrc);
	if (hGlobal == NULL)
	{
		return FALSE;
	}

	LPVOID lpBuffer = ::LockResource(hGlobal);
	if (lpBuffer == NULL)
	{
		FreeResource(hGlobal);
		return FALSE;
	}

	BOOL bRes = LoadFromBuffer ((LPBYTE) lpBuffer, (UINT) ::SizeofResource (hinstRes, hRsrc));

	UnlockResource(hGlobal);
	FreeResource(hGlobal);

	return bRes;
}
//*******************************************************************************
BOOL CPngImage::LoadFromFile (LPCTSTR lpszPath)
{
	if (!afxGlobalData.bGDIPlusSupport)
	{
		return FALSE;
	}

	if (CMFCToolBarImages::m_bMultiThreaded)
	{
		CMFCToolBarImages::m_CriticalSection.Lock ();
	}

	BOOL bRes = FALSE;

	if (m_pImage == NULL)
	{
		m_pImage = new CImage;
		ENSURE(m_pImage != NULL);
	}

	if (m_pImage->Load (lpszPath) == S_OK)
	{
		bRes = Attach (m_pImage->Detach ());
	}

	if (CMFCToolBarImages::m_bMultiThreaded)
	{
		CMFCToolBarImages::m_CriticalSection.Unlock ();
	}

	return bRes;
}
//*******************************************************************************
BOOL CPngImage::LoadFromBuffer (LPBYTE lpBuffer, UINT uiSize)
{
	if (!afxGlobalData.bGDIPlusSupport)
	{
		return FALSE;
	}

	ASSERT(lpBuffer != NULL);

	HGLOBAL hRes = ::GlobalAlloc (GMEM_MOVEABLE, uiSize);
	if (hRes == NULL)
	{
		return FALSE;
	}

	IStream* pStream = NULL;
	LPVOID lpResBuffer = ::GlobalLock (hRes);
	ASSERT (lpResBuffer != NULL);

	memcpy (lpResBuffer, lpBuffer, uiSize);

	HRESULT hResult = ::CreateStreamOnHGlobal (hRes, FALSE, &pStream);

	if (hResult != S_OK)
	{
		return FALSE;
	}

	if (CMFCToolBarImages::m_bMultiThreaded)
	{
		CMFCToolBarImages::m_CriticalSection.Lock ();
	}

	if (m_pImage == NULL)
	{
		m_pImage = new CImage;
		ENSURE(m_pImage != NULL);
	}

	m_pImage->Load (pStream);
	pStream->Release ();

	BOOL bRes = Attach (m_pImage->Detach ());

	if (CMFCToolBarImages::m_bMultiThreaded)
	{
		CMFCToolBarImages::m_CriticalSection.Unlock ();
	}

	return bRes;
}
