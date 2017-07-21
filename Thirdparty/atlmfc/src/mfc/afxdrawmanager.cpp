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
#include <math.h>
#include "afxcontrolbarutil.h"
#include "afxglobals.h"
#include "afxdrawmanager.h"
#include "afxtoolbarimages.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const double AFX_PI = 3.1415926;

#define AFX_CLR_TO_RGBA(c) c | 0xFF000000
#define AFX_RGB_TO_RGBA(r, g, b) AFX_CLR_TO_RGBA(RGB(r, g, b))
#define AFX_RGBA(r, g, b, a) RGB(r, g, b) |(a << 24)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

HBITMAP CDrawingManager::CreateBitmap_32(const CSize& size, void** pBits)
{
	ASSERT(0 < size.cx);
	ASSERT(0 != size.cy);

	if (size.cx <= 0 || size.cy == 0)
	{
		return NULL;
	}

	BITMAPINFO bi = {0};

	// Fill in the BITMAPINFOHEADER
	bi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth       = size.cx;
	bi.bmiHeader.biHeight      = size.cy;
	bi.bmiHeader.biSizeImage   = size.cx * size.cy;
	bi.bmiHeader.biPlanes      = 1;
	bi.bmiHeader.biBitCount    = 32;
	bi.bmiHeader.biCompression = BI_RGB;

	*pBits = NULL;
	HBITMAP hbmp = ::CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, pBits, NULL, 0);

	return hbmp;
}

CDrawingManager::CDrawingManager(CDC& m_dc) : m_dc(m_dc)
{
}

CDrawingManager::~CDrawingManager()
{
}

BOOL CDrawingManager::HighlightRect(CRect rect, int nPercentage, COLORREF clrTransparent, int nTolerance, COLORREF clrBlend)
{
	if (nPercentage == 100)
	{
		// Nothing to do
		return TRUE;
	}

	if (rect.Height() <= 0 || rect.Width() <= 0)
	{
		return TRUE;
	}

	if (afxGlobalData.m_nBitsPerPixel <= 8)
	{
		CMFCToolBarImages::FillDitheredRect(&m_dc, rect);
		return TRUE;
	}

	if (clrBlend != (COLORREF)-1 && nPercentage > 100)
	{
		return FALSE;
	}

	int cx = rect.Width();
	int cy = rect.Height();

	// Copy screen content into the memory bitmap:
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC(&m_dc))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	// Gets the whole menu and changes the shadow.
	CBitmap bmpMem;
	if (!bmpMem.CreateCompatibleBitmap(&m_dc, cx, cy))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	ENSURE(pOldBmp != NULL);

	COLORREF* pBits;
	HBITMAP hmbpDib = CreateBitmap_32(CSize(cx, cy), (LPVOID*)&pBits);

	if (hmbpDib == NULL || pBits == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	dcMem.SelectObject(hmbpDib);
	dcMem.BitBlt(0, 0, cx, cy, &m_dc, rect.left, rect.top, SRCCOPY);

	if (clrTransparent != -1)
	{
		clrTransparent = RGB(GetBValue(clrTransparent), GetGValue(clrTransparent), GetRValue(clrTransparent));
	}

#pragma warning(disable : 6269)
	for (int pixel = 0; pixel < (cx * cy); pixel++, *pBits++)
	{
		COLORREF color = (COLORREF) *pBits;

		BOOL bIgnore = FALSE;

		if (nTolerance > 0)
		{
			bIgnore = ( abs(GetRValue(color) - GetRValue(clrTransparent)) < nTolerance &&
				abs(GetGValue(color) - GetGValue(clrTransparent)) < nTolerance && abs(GetBValue(color) - GetBValue(clrTransparent)) < nTolerance);
		}
		else
		{
			bIgnore = color == clrTransparent;
		}

		if (!bIgnore)
		{
			if (nPercentage == -1)
			{
				*pBits = AFX_RGB_TO_RGBA(min(255, (2 * GetRValue(color) + GetBValue(afxGlobalData.clrBtnHilite)) / 3),
					min(255, (2 * GetGValue(color) + GetGValue(afxGlobalData.clrBtnHilite)) / 3), min(255, (2 * GetBValue(color) + GetRValue(afxGlobalData.clrBtnHilite)) / 3));
			}
			else
			{
				if (clrBlend == (COLORREF)-1)
				{
					*pBits = AFX_CLR_TO_RGBA(PixelAlpha(color, .01 * nPercentage, .01 * nPercentage, .01 * nPercentage));
				}
				else
				{
					long R = GetRValue(color);
					long G = GetGValue(color);
					long B = GetBValue(color);

					*pBits = AFX_RGB_TO_RGBA(min(255, R + ::MulDiv(GetBValue(clrBlend) - R, nPercentage, 100)),
						min(255, G + ::MulDiv(GetGValue(clrBlend) - G, nPercentage, 100)), min(255, B + ::MulDiv(GetRValue(clrBlend) - B, nPercentage, 100)));
				}
			}
		}
	}
#pragma warning(default : 6269)

	// Copy highligted bitmap back to the screen:
	m_dc.BitBlt(rect.left, rect.top, cx, cy, &dcMem, 0, 0, SRCCOPY);

	dcMem.SelectObject(pOldBmp);
	DeleteObject(hmbpDib);

	return TRUE;
}

void CDrawingManager::MirrorRect(CRect rect, BOOL bHorz/* = TRUE*/)
{
	if (rect.Height() <= 0 || rect.Width() <= 0)
	{
		return;
	}

	CRect rectClip;
	m_dc.GetClipBox(rectClip);

	CRect rectUnion;
	rectUnion.UnionRect(rectClip, rect);

	if (rectUnion != rectClip)
	{
		return;
	}

	int cx = rect.Width();
	int cy = rect.Height();

	// Copy screen content into the memory bitmap:
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC(&m_dc))
	{
		ASSERT(FALSE);
		return;
	}

	// Gets the whole menu and changes the shadow.
	CBitmap bmpMem;
	if (!bmpMem.CreateCompatibleBitmap(&m_dc, cx, cy))
	{
		ASSERT(FALSE);
		return;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	ENSURE(pOldBmp != NULL);

	COLORREF* pBits;
	HBITMAP hmbpDib = CreateBitmap_32(CSize(cx, cy), (LPVOID*)&pBits);

	if (hmbpDib == NULL || pBits == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	dcMem.SelectObject(hmbpDib);
	dcMem.BitBlt(0, 0, cx, cy, &m_dc, rect.left, rect.top, SRCCOPY);

	if (bHorz)
	{
		for (int y = 0; y <= cy; y++)
		{
			for (int x = 0; x <= cx / 2; x++)
			{
				int xRight = cx - x - 1;
				int y1 = cy - y;

				if (cx * y1 + x >= cx * cy || cx * y1 + xRight >= cx * cy)
				{
					continue;
				}

				COLORREF* pColorLeft = (COLORREF*)(pBits + cx * y1 + x);
				COLORREF colorSaved = *pColorLeft;

				COLORREF* pColorRight = (COLORREF*)(pBits + cx * y1 + xRight);

				*pColorLeft = *pColorRight;
				*pColorRight = colorSaved;
			}
		}
	}
	else
	{
		for (int y = 0; y <= cy / 2; y++)
		{
			for (int x = 0; x < cx; x++)
			{
				int yBottom = cy - y - 1;

				COLORREF* pColorTop = (COLORREF*)(pBits + cx * y + x);
				COLORREF colorSaved = *pColorTop;

				COLORREF* pColorBottom = (COLORREF*)(pBits + cx * yBottom + x);

				*pColorTop = *pColorBottom;
				*pColorBottom = colorSaved;
			}
		}
	}

	m_dc.BitBlt(rect.left, rect.top, cx, cy, &dcMem, 0, 0, SRCCOPY);

	dcMem.SelectObject(pOldBmp);
	DeleteObject(hmbpDib);
}

BOOL CDrawingManager::GrayRect(CRect rect, int nPercentage, COLORREF clrTransparent, COLORREF clrDisabled)
{
	if (rect.Height() <= 0 || rect.Width() <= 0)
	{
		return TRUE;
	}

	if (afxGlobalData.m_nBitsPerPixel <= 8)
	{
		CMFCToolBarImages::FillDitheredRect(&m_dc, rect);
		return TRUE;
	}

	int cx = rect.Width();
	int cy = rect.Height();

	// Copy screen content into the memory bitmap:
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC(&m_dc))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	// Gets the whole menu and changes the shadow.
	CBitmap bmpMem;
	if (!bmpMem.CreateCompatibleBitmap(&m_dc, cx, cy))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	ENSURE(pOldBmp != NULL);

	COLORREF* pBits;
	HBITMAP hmbpDib = CreateBitmap_32(CSize(cx, cy), (LPVOID*)&pBits);

	if (hmbpDib == NULL || pBits == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	dcMem.SelectObject(hmbpDib);
	dcMem.BitBlt(0, 0, cx, cy, &m_dc, rect.left, rect.top, SRCCOPY);

	if (clrTransparent != (COLORREF)-1)
	{
		clrTransparent = RGB(GetBValue(clrTransparent), GetGValue(clrTransparent), GetRValue(clrTransparent));
	}

	if (clrDisabled == (COLORREF)-1)
	{
		clrDisabled = afxGlobalData.clrBtnHilite;
	}

#pragma warning(disable : 6269)
	for (int pixel = 0; pixel < (cx * cy); pixel++, *pBits++)
	{
		COLORREF color = (COLORREF) *pBits;
		if (color != clrTransparent)
		{
			double H,S,L;
			RGBtoHSL(color, &H, &S, &L);
			color = HLStoRGB_ONE(H,L,0);

			if (nPercentage == -1)
			{
				*pBits = AFX_RGB_TO_RGBA(min(255, GetRValue(color) +((GetBValue(clrDisabled) - GetRValue(color)) / 2)),
					min(255, GetGValue(color) +((GetGValue(clrDisabled) - GetGValue(color)) / 2)), min(255, GetBValue(color) +((GetRValue(clrDisabled) - GetBValue(color)) / 2)));
			}
			else
			{
				*pBits = AFX_CLR_TO_RGBA(PixelAlpha(color, .01 * nPercentage, .01 * nPercentage, .01 * nPercentage));
			}
		}
	}
#pragma warning(default : 6269)

	// Copy highligted bitmap back to the screen:
	m_dc.BitBlt(rect.left, rect.top, cx, cy, &dcMem, 0, 0, SRCCOPY);

	dcMem.SelectObject(pOldBmp);
	DeleteObject(hmbpDib);

	return TRUE;
}

void CDrawingManager::_FillGradient(CRect rect, COLORREF colorStart, COLORREF colorFinish, BOOL bHorz/* = TRUE*/, int nStartFlatPercentage/* = 0*/, int nEndFlatPercentage/* = 0*/)
{
	if (colorStart == colorFinish)
	{
		CBrush br(colorStart);
		m_dc.FillRect(rect, &br);
		return;
	}

	if (nStartFlatPercentage > 0)
	{
		ASSERT(nStartFlatPercentage <= 100);

		if (bHorz)
		{
			CRect rectTop = rect;
			rectTop.bottom = rectTop.top + rectTop.Height() * nStartFlatPercentage / 100;
			rect.top = rectTop.bottom;

			CBrush br(colorFinish);
			m_dc.FillRect(rectTop, &br);
		}
		else
		{
			CRect rectLeft = rect;
			rectLeft.right = rectLeft.left + rectLeft.Width() * nStartFlatPercentage / 100;
			rect.left = rectLeft.right;

			CBrush br(colorStart);
			m_dc.FillRect(rectLeft, &br);
		}
	}

	if (nEndFlatPercentage > 0)
	{
		ASSERT(nEndFlatPercentage <= 100);

		if (bHorz)
		{
			CRect rectBottom = rect;
			rectBottom.top = rectBottom.bottom - rectBottom.Height() * nEndFlatPercentage / 100;
			rect.bottom = rectBottom.top;

			CBrush br(colorStart);
			m_dc.FillRect(rectBottom, &br);
		}
		else
		{
			CRect rectRight = rect;
			rectRight.left = rectRight.right - rectRight.Width() * nEndFlatPercentage / 100;
			rect.right = rectRight.left;

			CBrush br(colorFinish);
			m_dc.FillRect(rectRight, &br);
		}
	}

	if (nEndFlatPercentage + nStartFlatPercentage > 100)
	{
		ASSERT(FALSE);
		return;
	}

	// this will make 2^6 = 64 fountain steps
	int nShift = 6;
	int nSteps = 1 << nShift;

	for (int i = 0; i < nSteps; i++)
	{
		// do a little alpha blending
		BYTE bR = (BYTE)((GetRValue(colorStart) *(nSteps - i) + GetRValue(colorFinish) * i) >> nShift);
		BYTE bG = (BYTE)((GetGValue(colorStart) *(nSteps - i) + GetGValue(colorFinish) * i) >> nShift);
		BYTE bB = (BYTE)((GetBValue(colorStart) *(nSteps - i) + GetBValue(colorFinish) * i) >> nShift);

		CBrush br(RGB(bR, bG, bB));

		// then paint with the resulting color
		CRect r2 = rect;
		if (bHorz)
		{
			r2.bottom = rect.bottom - ((i * rect.Height()) >> nShift);
			r2.top = rect.bottom - (((i + 1) * rect.Height()) >> nShift);
			if (r2.Height() > 0)
				m_dc.FillRect(r2, &br);
		}
		else
		{
			r2.left = rect.left + ((i * rect.Width()) >> nShift);
			r2.right = rect.left + (((i + 1) * rect.Width()) >> nShift);
			if (r2.Width() > 0)
				m_dc.FillRect(r2, &br);
		}
	}
}
void CDrawingManager::FillGradient(CRect rect, COLORREF colorStart, COLORREF colorFinish, BOOL bHorz/* = TRUE*/, int nStartFlatPercentage/* = 0*/, int nEndFlatPercentage/* = 0*/)
{
	if (!CMFCToolBarImages::m_bIsDrawOnGlass)
	{
		_FillGradient(rect, colorStart, colorFinish, bHorz, nStartFlatPercentage, nEndFlatPercentage);
	}
	else
	{
		CRect rt(rect);
		rt.NormalizeRect();

		CSize size(rt.Size());
		if (size.cx == 0 || size.cy == 0)
		{
			return;
		}

		// Copy screen content into the memory bitmap:
		CDC dcMem;
		if (!dcMem.CreateCompatibleDC(&m_dc))
		{
			ASSERT(FALSE);
			return;
		}

		// Gets the whole menu and changes the shadow.
		CBitmap bmpMem;
		if (!bmpMem.CreateCompatibleBitmap(&m_dc, size.cx, size.cy))
		{
			ASSERT(FALSE);
			return;
		}

		CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
		ENSURE(pOldBmp != NULL);

		COLORREF* pBits;
		HBITMAP hmbpDib = CreateBitmap_32(size, (LPVOID*)&pBits);

		if (hmbpDib == NULL || pBits == NULL)
		{
			ASSERT(FALSE);
			return;
		}

		dcMem.SelectObject(hmbpDib);

		CDrawingManager dm(dcMem);
		dm._FillGradient(CRect(CPoint(0, 0), size), colorStart, colorFinish, bHorz, nStartFlatPercentage, nEndFlatPercentage);

		int sizeImage = size.cx * size.cy;
		for (int i = 0; i < sizeImage; i++)
		{
			*pBits |= 0xFF000000;
			pBits++;
		}

		// Copy bitmap back to the screen:
		m_dc.BitBlt(rt.left, rt.top, size.cx, size.cy, &dcMem, 0, 0, SRCCOPY);

		dcMem.SelectObject(pOldBmp);
		DeleteObject(hmbpDib);
	}
}

void CDrawingManager::FillGradient2(CRect rect, COLORREF colorStart, COLORREF colorFinish, int nAngle)
{
	if (colorStart == colorFinish)
	{
		CBrush br(colorStart);
		m_dc.FillRect(rect, &br);
		return;
	}

	// Process simple cases:
	switch (nAngle)
	{
	case 0:
	case 360:
		FillGradient(rect, colorStart, colorFinish, FALSE);
		return;

	case 90:
		FillGradient(rect, colorStart, colorFinish, TRUE);
		return;

	case 180:
		FillGradient(rect, colorFinish, colorStart, FALSE);
		return;

	case 270:
		FillGradient(rect, colorFinish, colorStart, TRUE);
		return;
	}

	// Copy screen content into the memory bitmap:
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC(&m_dc))
	{
		ASSERT(FALSE);
		return;
	}

	CBitmap bmpMem;
	if (!bmpMem.CreateCompatibleBitmap(&m_dc, rect.Width(), rect.Height()))
	{
		ASSERT(FALSE);
		return;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	ENSURE(pOldBmp != NULL);

	CPen* pOldPen = (CPen*) dcMem.SelectStockObject(NULL_PEN);

	int nShift = 6;
	int nSteps = 1 << nShift;

	const double fAngle = AFX_PI *(nAngle + 180) / 180;
	const int nOffset = (int)(cos(fAngle) * rect.Height());
	const int nTotalWidth = rect.Width() + abs(nOffset);

	const int xStart = nOffset > 0 ? - nOffset : 0;

	for (int i = 0; i < nSteps; i++)
	{
		// do a little alpha blending
		BYTE bR = (BYTE)((GetRValue(colorStart) *(nSteps - i) + GetRValue(colorFinish) * i) >> nShift);
		BYTE bG = (BYTE)((GetGValue(colorStart) *(nSteps - i) + GetGValue(colorFinish) * i) >> nShift);
		BYTE bB = (BYTE)((GetBValue(colorStart) *(nSteps - i) + GetBValue(colorFinish) * i) >> nShift);

		CBrush br(RGB(bR, bG, bB));

		int x11 = xStart +((i * nTotalWidth) >> nShift);
		int x12 = xStart +(((i + 1) * nTotalWidth) >> nShift);

		if (x11 == x12)
		{
			continue;
		}

		int x21 = x11 + nOffset;
		int x22 = x21 +(x12 - x11);

		POINT points [4];
		points [0].x = x11;
		points [0].y = 0;
		points [1].x = x12;
		points [1].y = 0;
		points [2].x = x22;
		points [2].y = rect.Height();
		points [3].x = x21;
		points [3].y = rect.Height();

		CBrush* pOldBrush = dcMem.SelectObject(&br);
		dcMem.Polygon(points, 4);
		dcMem.SelectObject(pOldBrush);
	}

	dcMem.SelectObject(pOldPen);

	// Copy bitmap back to the screen:
	m_dc.BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), &dcMem, 0, 0, SRCCOPY);
	dcMem.SelectObject(pOldBmp);
}

void CDrawingManager::Fill4ColorsGradient(CRect rect, COLORREF colorStart1, COLORREF colorFinish1,
	COLORREF colorStart2, COLORREF colorFinish2, BOOL bHorz/* = TRUE*/, int nPercentage) /* = 50, 0 - 100 */
{
	ASSERT(nPercentage >= 0);
	ASSERT(nPercentage <= 100);

	CRect rectFirst = rect;
	CRect rectSecond = rect;

	if (!bHorz)
	{
		rectFirst.right = rectFirst.left + rectFirst.Width() * nPercentage / 100;
		rectSecond.left = rectFirst.right;
	}
	else
	{
		rectFirst.bottom = rectFirst.top + rectFirst.Height() * nPercentage / 100;
		rectSecond.top = rectFirst.bottom;
	}

	FillGradient(rectFirst, colorStart1, colorFinish1, bHorz);
	FillGradient(rectSecond, colorStart2, colorFinish2, bHorz);
}

void CDrawingManager::FillAlpha(const CRect& rect, BYTE bValue /* = 255*/)
{
	const int cx = rect.Width();
	const int cy = rect.Height();

	COLORREF* pBits = NULL;
	HBITMAP hmbpDib = CreateBitmap_32(CSize (cx, cy), (LPVOID*)&pBits);
	if (hmbpDib == NULL || pBits == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	CDC dcMem;
	dcMem.CreateCompatibleDC(&m_dc);

	HBITMAP hbmpOld = (HBITMAP)dcMem.SelectObject(hmbpDib);
	dcMem.BitBlt(0, 0, cx, cy, &m_dc, rect.left, rect.top, SRCCOPY);

	const DWORD dwMask = (bValue << 24) & 0xFF000000;
	for (int i = 0; i < cx * cy; i++)
	{
		*pBits |= dwMask;
		pBits++;
	}

	m_dc.BitBlt(rect.left, rect.top, cx, cy, &dcMem, 0, 0, SRCCOPY);

	dcMem.SelectObject(hbmpOld);
	DeleteObject(hmbpDib);
}

BOOL CDrawingManager::DrawGradientRing(CRect rect, COLORREF colorStart, COLORREF colorFinish, COLORREF colorBorder, int nAngle /* 0 - 360 */, int nWidth, COLORREF clrFace /* = -1 */)
{
	int cx = rect.Width();
	int cy = rect.Height();

	if (cx <= 4 || cy <= 4)
	{
		// Rectangle too small
		return FALSE;
	}

	int xOrig = rect.left;
	int yOrig = rect.top;

	// Copy screen content into the memory bitmap:
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC(&m_dc))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CBitmap bmpMem;
	if (!bmpMem.CreateCompatibleBitmap(&m_dc, cx, cy))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	ENSURE(pOldBmp != NULL);

	COLORREF* pBits;
	HBITMAP hmbpDib = CreateBitmap_32(CSize(cx, cy), (LPVOID*)&pBits);

	if (hmbpDib == NULL || pBits == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	dcMem.SelectObject(hmbpDib);
	dcMem.BitBlt(0, 0, cx, cy, &m_dc, rect.left, rect.top, SRCCOPY);

	rect.OffsetRect(-xOrig, -yOrig);

	const int xCenter = (rect.left + rect.right) / 2;
	const int yCenter = (rect.top + rect.bottom) / 2;

	const int nSteps = 360;
	const double fDelta = 2. * AFX_PI / nSteps;
	const double fStart = AFX_PI * nAngle / 180;
	const double fFinish = fStart + 2. * AFX_PI;

	double rDelta = (double)(.5 + GetRValue(colorFinish) - GetRValue(colorStart)) / nSteps * 2;
	double gDelta = (double)(.5 + GetGValue(colorFinish) - GetGValue(colorStart)) / nSteps * 2;
	double bDelta = (double)(.5 + GetBValue(colorFinish) - GetBValue(colorStart)) / nSteps * 2;

	for (int nLevel = 0; nLevel < nWidth; nLevel++)
	{
		int i = 0;
		const int nRadius = min(rect.Width(), rect.Height()) / 2;
		const int nRectDelta = rect.Width() - rect.Height();

		if (clrFace != (COLORREF) -1 && nLevel == 0)
		{
			// Fill interior:
			CBrush brFill(clrFace);
			CBrush* pOldBrush = dcMem.SelectObject(&brFill);
			CPen* pOldPen = (CPen*) dcMem.SelectStockObject(NULL_PEN);

			if (nRectDelta == 0) // Circle
			{
				dcMem.Ellipse(rect);
			}
			else if (nRectDelta > 0) // Horizontal
			{
				dcMem.Ellipse(rect.left, rect.top, rect.left + rect.Height(), rect.bottom);
				dcMem.Ellipse(rect.right - rect.Height(), rect.top, rect.right, rect.bottom);
				dcMem.Rectangle(rect.left + rect.Height() / 2, rect.top, rect.right - rect.Height() / 2, rect.bottom);
			}
			else // Vertical
			{
				dcMem.Ellipse(rect.left, rect.top, rect.right, rect.top + rect.Width());
				dcMem.Ellipse(rect.left, rect.bottom - rect.Width(), rect.right, rect.bottom);
				dcMem.Rectangle(rect.left, rect.top + rect.Width() / 2, rect.right, rect.bottom - rect.Width() / 2);
			}

			dcMem.SelectObject(pOldBrush);
			dcMem.SelectObject(pOldPen);
		}

		int xPrev = -1;
		int yPrev = -1;

		for (double fAngle = fStart; fAngle < fFinish + fDelta; fAngle += fDelta, i ++)
		{
			const int nStep = fAngle <= (fFinish + fStart) / 2 ? i : nSteps - i;

			const BYTE bR = (BYTE) max(0, min(255, (.5 + rDelta * nStep + GetRValue(colorStart))));
			const BYTE bG = (BYTE) max(0, min(255, (.5 + gDelta * nStep + GetGValue(colorStart))));
			const BYTE bB = (BYTE) max(0, min(255, (.5 + bDelta * nStep + GetBValue(colorStart))));

			COLORREF color = nLevel == 0 && colorBorder != -1 ? colorBorder : RGB(bR, bG, bB);

			double dx = /*(fAngle >= 0 && fAngle <= AFX_PI / 2) ||(fAngle >= 3 * AFX_PI / 2) ? .5 : -.5*/0;
			double dy = /*(fAngle <= AFX_PI) ? .5 : -.5*/0;

			int x = xCenter +(int)(dx + cos(fAngle) * nRadius);
			int y = yCenter +(int)(dy + sin(fAngle) * nRadius);

			if (nRectDelta > 0)
			{
				if (x > xCenter)
				{
					x += (int)(.5 * nRectDelta);
				}
				else
				{
					x -= (int)(.5 * nRectDelta);
				}

				if (xPrev != -1 &&(xPrev > xCenter) != (x > xCenter))
				{
					for (int x1 = min(x, xPrev); x1 < max(x, xPrev); x1++)
					{
						SetPixel(pBits, cx, cy, x1, y, color);
					}
				}
			}
			else if (nRectDelta < 0)
			{
				if (y > yCenter)
				{
					y -= (int)(.5 * nRectDelta);
				}
				else
				{
					y += (int)(.5 * nRectDelta);
				}

				if (yPrev != -1 &&(yPrev > yCenter) != (y > yCenter))
				{
					for (int y1 = min(y, yPrev); y1 < max(y, yPrev); y1++)
					{
						SetPixel(pBits, cx, cy, x, y1, color);
					}
				}
			}

			SetPixel(pBits, cx, cy, x, y, color);

			xPrev = x;
			yPrev = y;
		}

		rect.DeflateRect(1, 1);
	}

	// Copy bitmap back to the screen:
	m_dc.BitBlt(xOrig, yOrig, cx, cy, &dcMem, 0, 0, SRCCOPY);

	dcMem.SelectObject(pOldBmp);
	DeleteObject(hmbpDib);

	return TRUE;
}

BOOL CDrawingManager::DrawShadow(CRect rect, int nDepth, int iMinBrightness, int iMaxBrightness,
	CBitmap* pBmpSaveBottom, CBitmap* pBmpSaveRight, COLORREF clrBase, BOOL bRightShadow/* = TRUE*/)
{
	ASSERT(nDepth >= 0);

	if (nDepth == 0 || rect.IsRectEmpty())
	{
		return TRUE;
	}

	int cx = rect.Width();
	int cy = rect.Height();

	const BOOL bIsLeft = !bRightShadow;

	if (pBmpSaveRight != NULL && pBmpSaveRight->GetSafeHandle() != NULL && pBmpSaveBottom != NULL && pBmpSaveBottom->GetSafeHandle() != NULL)
	{
		// Shadows are already implemented, put them directly
		// to the DC:
		m_dc.DrawState(CPoint(bIsLeft ? rect.left - nDepth : rect.right, rect.top), CSize(nDepth, cy + nDepth), pBmpSaveRight, DSS_NORMAL);

		m_dc.DrawState(CPoint(bIsLeft ? rect.left - nDepth : rect.left, rect.bottom), CSize(cx + nDepth, nDepth), pBmpSaveBottom, DSS_NORMAL);
		return TRUE;
	}

	ENSURE(pBmpSaveRight == NULL || pBmpSaveRight->GetSafeHandle() == NULL);
	ENSURE(pBmpSaveBottom == NULL || pBmpSaveBottom->GetSafeHandle() == NULL);

	// Copy screen content into the memory bitmap:
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC(&m_dc))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	// Gets the whole menu and changes the shadow.
	CBitmap bmpMem;
	if (!bmpMem.CreateCompatibleBitmap(&m_dc, cx + nDepth, cy + nDepth))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	ENSURE(pOldBmp != NULL);

	COLORREF* pBits;
	HBITMAP hmbpDib = CreateBitmap_32(CSize(cx + nDepth, cy + nDepth), (LPVOID*)&pBits);

	if (hmbpDib == NULL || pBits == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	dcMem.SelectObject(hmbpDib);
	dcMem.BitBlt(0, 0, cx + nDepth, cy + nDepth, &m_dc, bIsLeft ? rect.left - nDepth : rect.left, rect.top, SRCCOPY);

	// Process shadowing:
	// For having a very nice shadow effect, its actually hard work. Currently,
	// I'm using a more or less "hardcoded" way to set the shadows(by using a
	// hardcoded algorythm):
	//
	// This algorithm works as follows:
	//
	// It always draws a few lines, from left to bottom, from bottom to right,
	// from right to up, and from up to left). It does this for the specified
	// shadow width and the color settings.

	// For speeding up things, iShadowOffset is the
	// value which is needed to multiply for each shadow step
	int iShadowOffset = (iMaxBrightness - iMinBrightness) / nDepth;

	// Loop for drawing the shadow
	// Actually, this was simpler to implement than I thought
	for (int c = 0; c < nDepth; c++)
	{
		// Draw the shadow from left to bottom
		for (int y = cy; y < cy +(nDepth - c); y++)
		{
			SetAlphaPixel(pBits, rect, c + nDepth, y, iMaxBrightness -((nDepth  - c) *(iShadowOffset)), nDepth, clrBase, bIsLeft);
		}

		// Draw the shadow from left to right
		for (int x = nDepth +(nDepth - c); x < cx + c; x++)
		{
			SetAlphaPixel(pBits, rect,x, cy + c, iMaxBrightness -((c) *(iShadowOffset)),nDepth, clrBase, bIsLeft);
		}

		// Draw the shadow from top to bottom
		for (int y1 = nDepth +(nDepth - c); y1 < cy + c + 1; y1++)
		{
			SetAlphaPixel(pBits, rect, cx+c, y1, iMaxBrightness -((c) *(iShadowOffset)), nDepth, clrBase, bIsLeft);
		}

		// Draw the shadow from top to left
		for (int x1 = cx; x1 < cx +(nDepth - c); x1++)
		{
			SetAlphaPixel(pBits, rect, x1, c + nDepth, iMaxBrightness -((nDepth - c) *(iShadowOffset)), nDepth, clrBase, bIsLeft);
		}
	}

	// Copy shadowed bitmap back to the screen:
	m_dc.BitBlt(bIsLeft ? rect.left - nDepth : rect.left, rect.top, cx + nDepth, cy + nDepth, &dcMem, 0, 0, SRCCOPY);

	// Save shadows in the memory bitmaps:
	if (pBmpSaveRight != NULL)
	{
		pBmpSaveRight->CreateCompatibleBitmap(&m_dc, nDepth + 1, cy + nDepth);

		dcMem.SelectObject(pBmpSaveRight);
		dcMem.BitBlt(0, 0, nDepth, cy + nDepth, &m_dc, bIsLeft ? 0 : rect.right, rect.top, SRCCOPY);
	}

	if (pBmpSaveBottom != NULL)
	{
		pBmpSaveBottom->CreateCompatibleBitmap(&m_dc, cx + nDepth, nDepth + 1);

		dcMem.SelectObject(pBmpSaveBottom);
		dcMem.BitBlt(0, 0, cx + nDepth, nDepth, &m_dc, bIsLeft ? rect.left - nDepth : rect.left, rect.bottom, SRCCOPY);
	}

	dcMem.SelectObject(pOldBmp);
	DeleteObject(hmbpDib);

	return TRUE;
}

inline int sqr(int value)
{
	return value * value;
};

inline double sqr(double value)
{
	return value * value;
};

inline int sign(int value)
{
	if (value == 0)
	{
		return 0;
	}
	else
	{
		if (value > 0)
		{
			return 1;
		}
	}

	return -1;
}

inline double sign(double value)
{
	if (value == 0.0)
	{
		return 0.0;
	}
	else
	{
		if (value > 0.0)
		{
			return 1.0;
		}
	}

	return -1.0;
}

inline double frac(double value)
{
	return value - floor(value);
}

void CDrawingManager::DrawLine(int x1, int y1, int x2, int y2, COLORREF clrLine)
{
	if (clrLine == -1)
	{
		return;
	}

	int x  = x1;
	int y  = y1;
	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);
	int sx = sign(x2 - x1);
	int sy = sign(y2 - y1);

	if (dx == 0 && dy == 0)
	{
		return;
	}

	CRect rect(x1, y1, x2, y2);
	rect.NormalizeRect();
	rect.InflateRect(0, 0, 1, 1);

	CSize size(rect.Size());
	if (size.cx == 0 || size.cy == 0)
	{
		ASSERT(FALSE);
		return;
	}

	// Copy screen content into the memory bitmap:
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC(&m_dc))
	{
		ASSERT(FALSE);
		return;
	}

	// Gets the whole menu and changes the shadow.
	CBitmap bmpMem;
	if (!bmpMem.CreateCompatibleBitmap(&m_dc, size.cx, size.cy))
	{
		ASSERT(FALSE);
		return;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	ENSURE(pOldBmp != NULL);

	COLORREF* pBits;
	HBITMAP hmbpDib = CreateBitmap_32(size, (LPVOID*)&pBits);

	if (hmbpDib == NULL || pBits == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	dcMem.SelectObject(hmbpDib);

	bool exch = false;

	if (dy > dx)
	{
		long z = dx;
		dx = dy;
		dy = z;
		exch = true;
	}

	long e = 2 * dy - dx;
	long i = 1;

	clrLine = RGB(GetBValue(clrLine), GetGValue(clrLine), GetRValue(clrLine)) | 0xFF000000;

	do
	{
		*(pBits +(size.cy -(y - rect.top) - 1) * size.cx +(x - rect.left)) = clrLine;

		while (e >= 0)
		{
			if (exch)
			{
				x += sx;
			}
			else
			{
				y += sy;
			}

			e -= 2 * dx;
		}

		if (exch)
		{
			y += sy;
		}
		else
		{
			x += sx;
		}

		e += 2 * dy;

		i++;
	}
	while (i <= dx);

	*(pBits +(size.cy -(y - rect.top) - 1) * size.cx +(x - rect.left)) = clrLine;

	// Copy bitmap back to the screen:

	DrawAlpha(&m_dc, rect, &dcMem, CRect(CPoint(0, 0), size));

	dcMem.SelectObject(pOldBmp);
	DeleteObject(hmbpDib);
}

void CDrawingManager::DrawLineA(double x1, double y1, double x2, double y2, COLORREF clrLine)
{
	if (clrLine == -1)
	{
		return;
	}

	double xd = x2 - x1;
	double yd = y2 - y1;

	if (xd == 0 && yd == 0)
	{
		return;
	}

	bool exch = false;

	if (fabs(xd) <= fabs(yd))
	{
		exch = true;

		double tmpreal = x1;
		x1 = y1;
		y1 = tmpreal;

		tmpreal = x2;
		x2 = y2;
		y2 = tmpreal;

		tmpreal = xd;
		xd = yd;
		yd = tmpreal;
	}

	if (x1 > x2)
	{
		double tmpreal = x1;
		x1 = x2;
		x2 = tmpreal;

		tmpreal = y1;
		y1 = y2;
		y2 = tmpreal;

		xd = x2 - x1;
		yd = y2 - y1;
	}

	double f1 = 0.0;
	double f2 = 0.0;
	double f3 = 0.0;
	double f4 = 0.0;

	double grad = yd / xd;
	double yf;

	int ix1, ix2, iy1, iy2;

	{
		double xend = floor(x1 + 0.5);
		double yend = y1 + grad *(xend - x1);
		double xgap = 1.0 - frac(x1 + 0.5);
		ix1         = (int)floor(x1 + 0.5);
		iy1         = (int)floor(yend);

		f1 = (1.0 - frac(yend)) * xgap;
		f2 = frac(yend) * xgap;

		yf          = yend + grad;
		xend        = floor(x2 + 0.5);
		yend        = y2 + grad *(xend - x2);
		xgap        = 1.0 - frac(x2 - 0.5);
		ix2         = (int)floor(x2 + 0.5);
		iy2         = (int)floor(yend);

		f3 = (1.0 - frac(yend)) * xgap;
		f4 = frac(yend) * xgap;
	}

	CRect rect(ix1, iy1, ix2, iy2);

	if (exch)
	{
		rect = CRect(iy1, ix1, iy2, ix2);
	}

	rect.NormalizeRect();
	rect.InflateRect(0, 0, 1, 1);

	if (exch)
	{
		rect.right++;
	}
	else
	{
		rect.bottom++;
	}

	CSize size(rect.Size());
	if (size.cx == 0 || size.cy == 0)
	{
		ASSERT(FALSE);
		return;
	}

	// Copy screen content into the memory bitmap:
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC(&m_dc))
	{
		ASSERT(FALSE);
		return;
	}

	// Gets the whole menu and changes the shadow.
	CBitmap bmpMem;
	if (!bmpMem.CreateCompatibleBitmap(&m_dc, size.cx, size.cy))
	{
		ASSERT(FALSE);
		return;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	ENSURE(pOldBmp != NULL);

	COLORREF* pBits;
	HBITMAP hmbpDib = CreateBitmap_32(size, (LPVOID*)&pBits);

	if (hmbpDib == NULL || pBits == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	dcMem.SelectObject(hmbpDib);

	int clrR = GetRValue(clrLine);
	int clrG = GetGValue(clrLine);
	int clrB = GetBValue(clrLine);
	int clrA = 255;

	if (exch)
	{
		COLORREF* pRow = pBits +(size.cy -(ix1 - rect.top) - 1) * size.cx +(iy1 - rect.left);
		*pRow = RGB(clrB * f1, clrG * f1, clrR * f1) |((int)(clrA * f1) << 24);
		pRow++;
		*pRow = RGB(clrB * f2, clrG * f2, clrR * f2) |((int)(clrA * f2) << 24);

		pRow = pBits +(size.cy -(ix2 - rect.top) - 1) * size.cx +(iy2 - rect.left);
		*pRow = RGB(clrB * f3, clrG * f3, clrR * f3) |((int)(clrA * f3) << 24);
		pRow++;
		*pRow = RGB(clrB * f4, clrG * f4, clrR * f4) |((int)(clrA * f4) << 24);
	}
	else
	{
		*(pBits +(size.cy -(iy1 - rect.top) - 1) * size.cx +(ix1 - rect.left)) = RGB(clrB * f1, clrG * f1, clrR * f1) |((int)(clrA * f1) << 24);
		*(pBits +(size.cy -(iy1 - rect.top + 1) - 1) * size.cx +(ix1 - rect.left)) = RGB(clrB * f2, clrG * f2, clrR * f2) |((int)(clrA * f2) << 24);

		*(pBits +(size.cy -(iy2 - rect.top) - 1) * size.cx +(ix2 - rect.left)) = RGB(clrB * f3, clrG * f3, clrR * f3) |((int)(clrA * f3) << 24);
		*(pBits +(size.cy -(iy2 - rect.top + 1) - 1) * size.cx +(ix2 - rect.left)) = RGB(clrB * f4, clrG * f4, clrR * f4) |((int)(clrA * f4) << 24);
	}

	for (int x = ix1 + 1; x <= ix2 - 1; x++)
	{
		double f = frac(yf);

		int y = (int)floor(yf);

		int B = (int)(clrB * f);
		int G = (int)(clrG * f);
		int R = (int)(clrR * f);
		int A = (int)(clrA * f);

		if (exch)
		{
			COLORREF* pRow = pBits +(size.cy -(x - rect.top) - 1) * size.cx +(y - rect.left);
			*pRow = RGB((clrB - B), (clrG - G), (clrR - R)) |((clrA - A) << 24);
			pRow++;
			*pRow = RGB(B, G, R) |(A << 24);
		}
		else
		{
			*(pBits +(size.cy -(y - rect.top) - 1) * size.cx +(x - rect.left)) = RGB((clrB - B), (clrG - G), (clrR - R)) |((clrA - A) << 24);
			*(pBits +(size.cy -(y - rect.top + 1) - 1) * size.cx +(x - rect.left)) = RGB(B, G, R) |(A << 24);
		}

		yf = yf + grad;
	}

	// Copy bitmap back to the screen:

	DrawAlpha(&m_dc, rect, &dcMem, CRect(CPoint(0, 0), size));

	dcMem.SelectObject(pOldBmp);
	DeleteObject(hmbpDib);
}

void CDrawingManager::DrawEllipse(const CRect& rect, COLORREF clrFill, COLORREF clrLine)
{
	if (clrFill == -1 && clrLine == -1)
	{
		ASSERT(FALSE);
		return;
	}

	CRect rt(rect);
	rt.NormalizeRect();

	CSize size(rt.Size());
	if (size.cx == 0 || size.cy == 0)
	{
		ASSERT(FALSE);
		return;
	}

	// Copy screen content into the memory bitmap:
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC(&m_dc))
	{
		ASSERT(FALSE);
		return;
	}

	// Gets the whole menu and changes the shadow.
	CBitmap bmpMem;
	if (!bmpMem.CreateCompatibleBitmap(&m_dc, size.cx, size.cy))
	{
		ASSERT(FALSE);
		return;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	ENSURE(pOldBmp != NULL);

	COLORREF* pBits;
	HBITMAP hmbpDib = CreateBitmap_32(size, (LPVOID*)&pBits);

	if (hmbpDib == NULL || pBits == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	dcMem.SelectObject(hmbpDib);

	if (clrLine == -1)
	{
		clrLine = clrFill;
	}

	int brdR = GetRValue(clrLine);
	int brdG = GetGValue(clrLine);
	int brdB = GetBValue(clrLine);

	int filR = GetRValue(clrFill);
	int filG = GetGValue(clrFill);
	int filB = GetBValue(clrFill);

	BOOL bFill = clrFill != -1;
	if (bFill)
	{
		clrFill = RGB(filB, filG, filR) | 0xFF000000;
	}

	int R, G, B, A;
	COLORREF clrN, clrI;

	bool exch = false;
	double a;
	double b;
	double cx;
	double cy;

	{
		double x1 = 0;
		double x2 = size.cx - 1;
		double y1 = 0;
		double y2 = size.cy - 1;

		if (x2 < x1 )
		{
			double t  = x1;
			x1 = x2;
			x2 = t;
		}

		if (y2 < y1)
		{
			double t  = y1;
			y1 = y2;
			y2 = t;
		}

		if (y2 - y1 <= x2 - x1)
		{
			exch = true;
			double t = x1;
			x1 = y1;
			y1 = t;

			t = x2;
			x2 = y2;
			y2 = t;
		}

		a  = (x2 - x1) / 2.0;
		b  = (y2 - y1) / 2.0;
		cx = (x1 + x2) / 2.0;
		cy = (y1 + y2) / 2.0;
	}

	if (bFill)
	{
		int i1 = (int)ceil(cx - a);
		int i2 = (int)floor(cx + a);
		for (int ix = i1; ix <= i2; ix++)
		{
			double dist = 1.0 - sqr((ix - cx) / a);
			if (dist < 0)
			{
				continue;
			}

			double y = b * sqrt(dist);

			if (!exch)
			{
				int y1 = (int)ceil(cy - y);
				int y2 = (int)floor(cy + y);
				COLORREF* pRow = pBits + y1 * size.cx + ix;
				for (int i = y1; i <= y2; i++)
				{
					*pRow = clrFill;
					pRow += size.cx;
				}
			}
			else
			{
				int x1 = (int)ceil(cy - y);
				int x2 = (int)floor(cy + y);
				COLORREF* pRow = pBits + ix * size.cx + x1;
				for (int i = x1; i <= x2; i++)
				{
					*pRow = clrFill;
					pRow++;
				}
			}
		}
	}

	double t  = a * a / sqrt(a * a + b * b);
	int i1 = (int)floor(cx - t);
	int i2 = (int)ceil(cx + t);

	for (int ix = i1; ix <= i2; ix++)
	{
		double dist = 1.0 - sqr((ix - cx) / a);
		if (dist < 0)
		{
			continue;
		}

		double y  = b * sqrt(dist);
		int iy = (int)ceil(cy + y);
		double f  = iy - cy - y;

		B = (int)(brdB * f);
		G = (int)(brdG * f);
		R = (int)(brdR * f);
		A = (int)(255  * f);

		if (bFill)
		{
			double fi = 1.0 - f;
			clrN = RGB(filB * fi + B, filG * fi + G, filR * fi + R) | 0xFF000000;
		}
		else
		{
			clrN = RGB(B, G, R) |(A << 24);
		}

		clrI = RGB((brdB - B), (brdG - G), (brdR - R)) |((255 - A) << 24);

		if (!exch)
		{
			*(pBits + iy * size.cx + ix) = clrI;
			*(pBits +(iy - 1) * size.cx + ix) = clrN;
		}
		else
		{
			*(pBits + ix * size.cx + iy) = clrI;
			*(pBits + ix * size.cx + iy - 1) = clrN;
		}

		iy = (int)floor(cy - y);
		f  = cy - y - iy;

		B = (int)(brdB * f);
		G = (int)(brdG * f);
		R = (int)(brdR * f);
		A = (int)(255  * f);

		if (bFill)
		{
			double fi = 1.0 - f;
			clrN = RGB(filB * fi + B, filG * fi + G, filR * fi + R) | 0xFF000000;
		}
		else
		{
			clrN = RGB(B, G, R) |(A << 24);
		}

		clrI = RGB((brdB - B), (brdG - G), (brdR - R)) |((255 - A) << 24);

		if (!exch)
		{
			*(pBits + iy * size.cx + ix) = clrI;
			*(pBits +(iy + 1) * size.cx + ix) = clrN;
		}
		else
		{
			*(pBits + ix * size.cx + iy) = clrI;
			*(pBits + ix * size.cx + iy + 1) = clrN;
		}
	}

	t  = b * b / sqrt(a * a + b * b);
	i1 = (int)ceil(cy - t);
	i2 = (int)floor(cy + t);

	for (int iy = i1; iy <= i2; iy++)
	{
		double dist = 1.0 - sqr((iy - cy) / b);
		if (dist < 0)
		{
			continue;
		}

		double x  = a * sqrt(dist);
		int ix = (int)floor(cx - x);
		double f  = cx - x - ix;

		B = (int)(brdB * f);
		G = (int)(brdG * f);
		R = (int)(brdR * f);
		A = (int)(255  * f);

		if (bFill)
		{
			double fi = 1.0 - f;
			clrN = RGB(filB * fi + B, filG * fi + G, filR * fi + R) | 0xFF000000;
		}
		else
		{
			clrN = RGB(B, G, R) |(A << 24);
		}

		clrI = RGB((brdB - B), (brdG - G), (brdR - R)) |((255 - A) << 24);

		if (!exch)
		{
			*(pBits + iy * size.cx + ix) = clrI;
			*(pBits + iy * size.cx + ix + 1) = clrN;
		}
		else
		{
			*(pBits + ix * size.cx + iy) = clrI;
			*(pBits +(ix + 1) * size.cx + iy) = clrN;
		}

		ix = (int)ceil(cx + x);
		f  = ix - cx - x;

		B = (int)(brdB * f);
		G = (int)(brdG * f);
		R = (int)(brdR * f);
		A = (int)(255  * f);

		if (bFill)
		{
			double fi = 1.0 - f;
			clrN = RGB(filB * fi + B, filG * fi + G, filR * fi + R) | 0xFF000000;
		}
		else
		{
			clrN = RGB(B, G, R) |(A << 24);
		}

		clrI = RGB((brdB - B), (brdG - G), (brdR - R)) |((255 - A) << 24);

		if (!exch)
		{
			*(pBits + iy * size.cx + ix) = clrI;
			*(pBits + iy * size.cx + ix - 1) = clrN;
		}
		else
		{
			*(pBits + ix * size.cx + iy) = clrI;
			*(pBits +(ix - 1) * size.cx + iy) = clrN;
		}
	}

	// Copy bitmap back to the screen:
	DrawAlpha(&m_dc, rt, &dcMem, CRect(CPoint(0, 0), size));

	dcMem.SelectObject(pOldBmp);
	DeleteObject(hmbpDib);
}

void CDrawingManager::DrawRect(const CRect& rect, COLORREF clrFill, COLORREF clrLine)
{
	if (clrFill == -1 && clrLine == -1)
	{
		ASSERT(FALSE);
		return;
	}

	CRect rt(rect);
	rt.NormalizeRect();

	CSize size(rt.Size());
	if (size.cx == 0 || size.cy == 0)
	{
		ASSERT(FALSE);
		return;
	}

	// Copy screen content into the memory bitmap:
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC(&m_dc))
	{
		ASSERT(FALSE);
		return;
	}

	// Gets the whole menu and changes the shadow.
	CBitmap bmpMem;
	if (!bmpMem.CreateCompatibleBitmap(&m_dc, size.cx, size.cy))
	{
		ASSERT(FALSE);
		return;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	ENSURE(pOldBmp != NULL);

	COLORREF* pBits;
	HBITMAP hmbpDib = CreateBitmap_32(size, (LPVOID*)&pBits);

	if (hmbpDib == NULL || pBits == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	dcMem.SelectObject(hmbpDib);

	int xB = 0;
	int xE = size.cx;
	int yB = 1;
	int yE = size.cy;

	if (clrLine != -1)
	{
		COLORREF clr = RGB(GetBValue(clrLine), GetGValue(clrLine), GetRValue(clrLine)) | 0xFF000000;

		for (int x = 0; x < size.cx; x++)
		{
			*pBits = clr;
			pBits++;
		}

		if (1 < size.cy)
		{
			memcpy((LPVOID)(pBits +(size.cy - 2) * size.cx), (LPCVOID)(pBits - size.cx), size.cx * sizeof(COLORREF));

			if (2 < size.cy)
			{
				*pBits = clr;
				if (2 <= size.cx)
				{
					*(pBits + size.cx - 1) = clr;
				}
				pBits++;
			}
		}

		xB++;
		xE--;
		yB++;
		yE--;
	}

	COLORREF clr = clrFill == -1 ? 0 : RGB(GetBValue(clrFill), GetGValue(clrFill), GetRValue(clrFill)) | 0xFF000000;

	if (yB <= yE)
	{
		for (int x = xB; x < xE; x++)
		{
			*pBits = clr;
			pBits++;
		}

		if (xB < xE && clrLine != -1)
		{
			pBits++;
		}
	}

	for (int y = yB; y < yE; y++)
	{
		memcpy((LPVOID)(pBits), (LPCVOID)(pBits - size.cx), size.cx * sizeof(COLORREF));
		pBits += size.cx;
	}

	// Copy bitmap back to the screen:

	if (clrFill != -1)
	{
		m_dc.BitBlt(rt.left, rt.top, size.cx, size.cy, &dcMem, 0, 0, SRCCOPY);
	}
	else
	{
		DrawAlpha(&m_dc, rt, &dcMem, CRect(CPoint(0, 0), size));
	}

	dcMem.SelectObject(pOldBmp);
	DeleteObject(hmbpDib);
}

inline void __stdcall CDrawingManager::SetAlphaPixel(COLORREF* pBits, CRect rect, int x, int y, int percent, int m_iShadowSize, COLORREF clrBase, BOOL bIsRight)
{
	// Our direct bitmap access swapped the y coordinate...
	y = (rect.Height()+m_iShadowSize)- y;

	COLORREF* pColor = (COLORREF*)(bIsRight ? (pBits +(rect.Width() + m_iShadowSize) *(y + 1) - x) : (pBits +(rect.Width() + m_iShadowSize) * y + x));

	*pColor = PixelAlpha(*pColor, percent);

	if (clrBase == (COLORREF)-1)
	{
		return;
	}

	*pColor = RGB( min(255, (3 * GetRValue(*pColor) + GetBValue(clrBase)) / 4), min(255, (3 * GetGValue(*pColor) + GetGValue(clrBase)) / 4), min(255, (3 * GetBValue(*pColor) + GetRValue(clrBase)) / 4));
}

COLORREF  __stdcall CDrawingManager::PixelAlpha(COLORREF srcPixel, int percent)
{
	// My formula for calculating the transpareny is as
	// follows(for each single color):
	//
	//							   percent
	// destPixel = sourcePixel *( ------- )
	//                               100
	//
	// This is not real alpha blending, as it only modifies the brightness,
	// but not the color(a real alpha blending had to mix the source and
	// destination pixels, e.g. mixing green and red makes yellow).
	// For our nice "menu" shadows its good enough.

	COLORREF clrFinal = RGB( min(255, (GetRValue(srcPixel) * percent) / 100), min(255, (GetGValue(srcPixel) * percent) / 100), min(255, (GetBValue(srcPixel) * percent) / 100));

	// TRACE("%d %d %d\n", GetRValue(clrFinal), GetGValue(clrFinal), GetBValue(clrFinal));
	return(clrFinal);

}

static inline int __stdcall AdjustChannel(int nValue, double nPercent)
{
	int nNewValue = (int)(.5 + nPercent * nValue);
	if (nValue == 0 && nPercent > 1.)
	{
		nNewValue = (int)(.5 +(nPercent - 1.) * 255);
	}

	return min(nNewValue, 255);
}

COLORREF __stdcall CDrawingManager::PixelAlpha(COLORREF srcPixel, double percentR, double percentG, double percentB)
{
	COLORREF clrFinal = RGB( AdjustChannel(GetRValue(srcPixel), percentR), AdjustChannel(GetGValue(srcPixel), percentG), AdjustChannel(GetBValue(srcPixel), percentB));

	return(clrFinal);

}

COLORREF __stdcall CDrawingManager::PixelAlpha(COLORREF srcPixel, COLORREF dstPixel, int percent)
{
	int ipercent = 100 - percent;
	COLORREF clrFinal = RGB((GetRValue(srcPixel) * percent + GetRValue(dstPixel) * ipercent) / 100,
		(GetGValue(srcPixel) * percent + GetGValue(dstPixel) * ipercent) / 100, (GetBValue(srcPixel) * percent + GetBValue(dstPixel) * ipercent) / 100);

	return(clrFinal);
}

void __stdcall CDrawingManager::SetPixel(COLORREF* pBits, int cx, int cy, int x, int y, COLORREF color)
{
	// Our direct bitmap access swapped the y coordinate...
	y = cy - y;

	COLORREF* pColor = (COLORREF*)(pBits + cx * y + x);
	*pColor = RGB(GetBValue(color), GetGValue(color), GetRValue(color));
}

//----------------------------------------------------------------------
// Conversion between the HSL(Hue, Saturation, and Luminosity)
// and RBG color model.
//----------------------------------------------------------------------
// The conversion algorithms presented here come from the book by
// Fundamentals of Interactive Computer Graphics by Foley and van Dam.
// In the example code, HSL values are represented as floating point
// number in the range 0 to 1. RGB tridrants use the Windows convention
// of 0 to 255 of each element.
//----------------------------------------------------------------------

double __stdcall CDrawingManager::HuetoRGB(double m1, double m2, double h )
{
	if ( h < 0 ) h += 1.0;
	if ( h > 1 ) h -= 1.0;
	if ( 6.0*h < 1 )
		return(m1+(m2-m1)*h*6.0);
	if ( 2.0*h < 1 )
		return m2;
	if ( 3.0*h < 2.0 )
		return(m1+(m2-m1)*((2.0/3.0)-h)*6.0);
	return m1;
}

BYTE __stdcall CDrawingManager::HueToRGB(float rm1, float rm2, float rh)
{
	if (rh > 360.0f)
		rh -= 360.0f;
	else if (rh < 0.0f)
		rh += 360.0f;

	if (rh <  60.0f)
		rm1 = rm1 +(rm2 - rm1) * rh / 60.0f;
	else if (rh < 180.0f)
		rm1 = rm2;
	else if (rh < 240.0f)
		rm1 = rm1 +(rm2 - rm1) *(240.0f - rh) / 60.0f;

	return static_cast<BYTE>(rm1 * 255);
}

COLORREF __stdcall CDrawingManager::HLStoRGB_ONE( double H, double L, double S )
{
	double r, g, b;
	double m1, m2;

	if (S==0)
	{
		r=g=b=L;
	}
	else
	{
		if (L <=0.5)
			m2 = L*(1.0+S);
		else if (L == 1.0)
			m2 = L;
		else
			m2 = L+S-L*S;
		m1 = 2.0*L-m2;
		r = HuetoRGB(m1, m2, H+1.0/3.0);
		g = HuetoRGB(m1, m2, H);
		b = HuetoRGB(m1, m2, H-1.0/3.0);
	}
	return RGB((BYTE)(r*255), (BYTE)(g*255), (BYTE)(b*255));
}

COLORREF __stdcall CDrawingManager::HLStoRGB_TWO( double H, double L, double S)
{
	WORD R, G, B; // RGB component values

	if (S == 0.0)
	{
		R = G = B = unsigned char(L * 255.0);
	}
	else
	{
		float rm1, rm2;

		if (L <= 0.5f)
			rm2 = (float)(L + L * S);
		else if (L == 1.0)
			rm2 = (float)L;
		else
			rm2 = (float)(L + S - L * S);

		rm1 = (float)(2.0f * L - rm2);

		R = HueToRGB(rm1, rm2, (float)(H + 120.0f));
		G = HueToRGB(rm1, rm2, (float)(H));
		B = HueToRGB(rm1, rm2, (float)(H - 120.0f));
	}

	return RGB(R, G, B);
}

void __stdcall CDrawingManager::RGBtoHSL( COLORREF rgb, double *H, double *S, double *L )
{
	double delta;
	double r = (double)GetRValue(rgb)/255;
	double g = (double)GetGValue(rgb)/255;
	double b = (double)GetBValue(rgb)/255;
	double cmax = max(r, max(g, b));
	double cmin = min(r, min(g, b));
	*L= (cmax+cmin)/2.0;

	if (cmax==cmin)
	{
		*S = 0;
		*H = 0; // it's really undefined
	}
	else
	{
		if (*L < 0.5)
			*S = (cmax-cmin)/(cmax+cmin);
		else
			*S = (cmax-cmin)/(2.0-cmax-cmin);

		delta = cmax - cmin;
		if (r==cmax)
			*H = (g-b)/delta;
		else if (g==cmax)
			*H = 2.0 +(b-r)/delta;
		else
			*H=4.0+(r-g)/delta;
		*H /= 6.0;
		if (*H < 0.0)
			*H += 1;
	}
}

void __stdcall CDrawingManager::RGBtoHSV(COLORREF rgb, double *H, double *S, double *V)
// Algorithm by A. R. Smith
{
	double r = (double) GetRValue(rgb) / 255;
	double g = (double) GetGValue(rgb) / 255;
	double b = (double) GetBValue(rgb) / 255;

	double dblMin = min(r, min(g, b));
	double dblMax = max(r, max(g, b));

	*V = dblMax; // v
	double delta = dblMax - dblMin;

	if ( dblMax != 0 )
	{
		*S = delta / dblMax; // s
	}
	else
	{
		// r = g = b = 0
		// s = 0, v is undefined
		*S = 0;
		*H = -1;
		return;
	}

	if (delta == 0.)
	{
		*H = 1;
	}
	else
	{
		if (r == dblMax)
			*H = (g - b) / delta; // between yellow & magenta
		else if ( g == dblMax )
			*H = 2 +( b - r ) / delta; // between cyan & yellow
		else
			*H = 4 +( r - g ) / delta; // between magenta & cyan
	}

	*H *= 60; // degrees

	if (*H < 0)
		*H += 360;
}

COLORREF __stdcall CDrawingManager::HSVtoRGB(double h, double s, double v)
// Algoritm by A. R. Smith
{
	int i;
	double f, p, q, t;
	double r, g, b;

	if ( s == 0 )
	{
		// achromatic(grey)
		r = g = b = v;
	}
	else
	{
		h /= 60; // sector 0 to 5
		i = (int) floor( h );
		f = h - i; // factorial part of h
		p = v *( 1 - s );
		q = v *( 1 - s * f );
		t = v *( 1 - s *( 1 - f ) );

		switch ( i )
		{
		case 0:
			r = v;
			g = t;
			b = p;
			break;

		case 1:
			r = q;
			g = v;
			b = p;
			break;

		case 2:
			r = p;
			g = v;
			b = t;
			break;

		case 3:
			r = p;
			g = q;
			b = v;
			break;

		case 4:
			r = t;
			g = p;
			b = v;
			break;

		default: // case 5:
			r = v;
			g = p;
			b = q;
			break;
		}
	}

	return RGB( (int)(.5 + r * 255), (int)(.5 + g * 255), (int)(.5 + b * 255));
}

COLORREF __stdcall CDrawingManager::SmartMixColors(COLORREF color1, COLORREF color2, double dblLumRatio, int k1, int k2)
{
	ENSURE(k1 >= 0);
	ENSURE(k2 >= 0);

	if (k1 + k2 == 0)
	{
		ASSERT(FALSE);
		return RGB(0, 0, 0);
	}

	COLORREF color = RGB( (GetRValue(color1) * k1 + GetRValue(color2) * k2) /(k1 + k2),
		(GetGValue(color1) * k1 + GetGValue(color2) * k2) /(k1 + k2), (GetBValue(color1) * k1 + GetBValue(color2) * k2) /(k1 + k2));

	double h1, s1, v1;
	RGBtoHSV(color, &h1, &s1, &v1);

	double h2, s2, v2;
	RGBtoHSV(color2, &h2, &s2, &v2);

	v1 = v2;
	s1 = (s1 *  k1 + s2 *  k2) /(k1 + k2);

	color = HSVtoRGB(h1, s1, v1);

	if (dblLumRatio != 1.)
	{
		double H, S, L;
		RGBtoHSL(color, &H, &S, &L);

		color = HLStoRGB_ONE(H, min(1., L * dblLumRatio), S);
	}

	return color;
}

void CDrawingManager::DrawAlpha(CDC* pDstDC, const CRect& rectDst, CDC* pSrcDC, const CRect& rectSrc)
{
	BLENDFUNCTION pixelblend = { AC_SRC_OVER, 0, 255, 1 /*AC_SRC_ALPHA*/};

	pDstDC->AlphaBlend(rectDst.left, rectDst.top, rectDst.Width(), rectDst.Height(), pSrcDC, rectSrc.left, rectSrc.top, rectSrc.Width(), rectSrc.Height(), pixelblend);
}

HBITMAP CDrawingManager::PrepareShadowMask (int nDepth,
											 COLORREF clrBase,
                                             int iMinBrightness/* = 0*/, int iMaxBrightness/* = 100*/)
{
	if (nDepth == 0)
	{
		return NULL;
	}

	int nSize     = nDepth < 3 ? 3 : nDepth;
	int nDestSize = nSize * 2 + 1;

	LPBYTE lpBits = NULL;
	HBITMAP hBitmap = CreateBitmap_32 (CSize (nDestSize, nDestSize), (void**)&lpBits);

	if (hBitmap == NULL || lpBits == NULL)
	{
		return NULL;
	}

	// Create mask
	int nDestLength = nDestSize * nDestSize;
	double* mask = new double[nDestLength];

	double dispersion = 0.75;
	double minValue   = iMinBrightness / 100.0;
	double maxValue   = iMaxBrightness / 100.0;
	double delta      = maxValue - minValue;

	long size2      = nDestSize / 2;
	double size2S   = nDestSize * nDestSize / 4.0;

	for(long y = -size2; y <= size2; y++)
	{
		int index = (y + size2) * nDestSize;
		double y2 = y * y;

		for(long x = -size2; x <= size2; x++)
		{
			double d = y2 + x * x;

			if(d <= size2S)
			{
				double e = exp (-(d / size2S) / dispersion * 2.0) * delta + minValue;
				mask [index + x + size2] = min (maxValue, max (e, minValue));
			}
		}
	}

	BYTE r = GetRValue (clrBase);
	BYTE g = GetGValue (clrBase);
	BYTE b = GetBValue (clrBase);

	double* pMask = mask;
	LPRGBQUAD pQuad = (LPRGBQUAD)lpBits;
	for (int i = 0; i < nDestLength; i++)
	{
		pQuad->rgbRed      = (BYTE)(*pMask * r);
		pQuad->rgbGreen    = (BYTE)(*pMask * g);
		pQuad->rgbBlue     = (BYTE)(*pMask * b);
		pQuad->rgbReserved = (BYTE)(*pMask * 255);

		pMask++;
		pQuad++;
	}

	if (mask != NULL)
	{
		delete [] mask;
	}

	return hBitmap;
}
