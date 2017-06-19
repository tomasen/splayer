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
// CImageList

BOOL CImageList::DrawIndirect(IMAGELISTDRAWPARAMS* pimldp)
{
	ASSERT(m_hImageList != NULL);
	ASSERT_POINTER(pimldp, IMAGELISTDRAWPARAMS);

	DWORD dwMajor = 0, dwMinor = 0;
	AtlGetCommCtrlVersion(&dwMajor, &dwMinor);
	if (dwMajor < 6)
	{
		pimldp->cbSize = IMAGELISTDRAWPARAMS_V3_SIZE;
	}
	else
	{
		pimldp->cbSize = sizeof(IMAGELISTDRAWPARAMS);
	}
	pimldp->himl = m_hImageList;
	return AfxImageList_DrawIndirect(pimldp);
}

BOOL CImageList::DrawIndirect(CDC* pDC, int nImage, POINT pt,
		SIZE sz, POINT ptOrigin, UINT fStyle /* = ILD_NORMAL */,
		DWORD dwRop /* = SRCCOPY */, COLORREF rgbBack /* = CLR_DEFAULT */,
		COLORREF rgbFore /* = CLR_DEFAULT */,
		DWORD fState /*= ILS_NORMAL*/, DWORD Frame /*= ILS_PULSE*/, COLORREF crEffect /*= CLR_DEFAULT*/)
{
	ASSERT_POINTER(pDC, CDC);
	ASSERT(pDC->m_hDC != NULL);

	IMAGELISTDRAWPARAMS drawing;

	drawing.i = nImage;
	drawing.hdcDst = pDC->m_hDC;
	drawing.x = pt.x;
	drawing.y = pt.y;
	drawing.cx = sz.cx;
	drawing.cy = sz.cy;
	drawing.xBitmap = ptOrigin.x;
	drawing.yBitmap = ptOrigin.y;
	drawing.rgbBk = rgbBack;
	drawing.rgbFg = rgbFore;
	drawing.fStyle = fStyle;
	drawing.dwRop = dwRop;
	drawing.fState = fState;
	drawing.Frame = Frame;
	drawing.crEffect = crEffect;

	// this call initializes cbSize and himl;
	return DrawIndirect(&drawing);
}

BOOL CImageList::Create(CImageList* pImageList)
{
	ASSERT(pImageList != NULL);
	return Attach(AfxImageList_Duplicate(pImageList->m_hImageList));
}

/////////////////////////////////////////////////////////////////////////////
