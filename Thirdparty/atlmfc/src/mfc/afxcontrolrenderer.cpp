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
#include "afxcontrolrenderer.h"
#include "afxdrawmanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMFCControlRendererInfo::CMFCControlRendererInfo()
{
	CommonInit();
}

CMFCControlRendererInfo::~CMFCControlRendererInfo()
{
}

CMFCControlRendererInfo::CMFCControlRendererInfo(UINT uiBmpResID, const CRect& rtImage, const CRect& rtCorners,
	const CRect& rtSides/* = CRect(0, 0, 0, 0)*/, const CRect& rtInner/* = CRect(0, 0, 0, 0)*/)
{
	CommonInit();
	SetResourceID(MAKEINTRESOURCE(uiBmpResID));

	m_rectImage      = rtImage;
	m_rectCorners    = rtCorners;
	m_rectSides      = rtSides;
	m_rectInter      = rtInner;
}

CMFCControlRendererInfo::CMFCControlRendererInfo(LPCTSTR lpszBmpResID, const CRect& rtImage, const CRect& rtCorners,
	const CRect& rtSides/* = CRect(0, 0, 0, 0)*/, const CRect& rtInner/* = CRect(0, 0, 0, 0)*/)
{
	CommonInit();
	SetResourceID(lpszBmpResID);

	m_rectImage      = rtImage;
	m_rectCorners    = rtCorners;
	m_rectSides      = rtSides;
	m_rectInter      = rtInner;
}

CMFCControlRendererInfo::CMFCControlRendererInfo(UINT uiBmpResID, COLORREF clrTransparent, const CRect& rtImage, const CRect& rtCorners,
	const CRect& rtSides/* = CRect(0, 0, 0, 0)*/, const CRect& rtInner/* = CRect(0, 0, 0, 0)*/, BOOL bPreMultiplyCheck/* = TRUE*/)
{
	CommonInit();
	SetResourceID(MAKEINTRESOURCE(uiBmpResID));

	m_rectImage         = rtImage;
	m_rectCorners	    = rtCorners;
	m_rectSides		    = rtSides;
	m_rectInter         = rtInner;
	m_clrTransparent    = clrTransparent;
	m_bPreMultiplyCheck = bPreMultiplyCheck;
}

CMFCControlRendererInfo::CMFCControlRendererInfo(LPCTSTR lpszBmpResID, COLORREF clrTransparent, const CRect& rtImage, const CRect& rtCorners,
	const CRect& rtSides/* = CRect(0, 0, 0, 0)*/, const CRect& rtInner/* = CRect(0, 0, 0, 0)*/, BOOL bPreMultiplyCheck/* = TRUE*/)
{
	CommonInit();
	SetResourceID(lpszBmpResID);

	m_rectImage         = rtImage;
	m_rectCorners       = rtCorners;
	m_rectSides         = rtSides;
	m_rectInter         = rtInner;
	m_clrTransparent    = clrTransparent;
	m_bPreMultiplyCheck = bPreMultiplyCheck;
}

CMFCControlRendererInfo::CMFCControlRendererInfo(const CMFCControlRendererInfo& rSrc)
{
	CommonInit();

	(*this) = rSrc;
}

void CMFCControlRendererInfo::CommonInit()
{
	m_uiBmpResID   = 0;
	m_strBmpResID.Empty();
	m_rectImage.SetRectEmpty();
	m_rectCorners.SetRectEmpty();
	m_rectSides.SetRectEmpty();
	m_rectInter.SetRectEmpty();
	m_clrTransparent    = CLR_DEFAULT;
	m_bPreMultiplyCheck = TRUE;
}

LPCTSTR CMFCControlRendererInfo::GetResourceID() const
{
	if (m_strBmpResID.IsEmpty())
	{
		return MAKEINTRESOURCE(m_uiBmpResID);
	}

	return m_strBmpResID;
}

void CMFCControlRendererInfo::SetResourceID(LPCTSTR lpszBmpResID)
{
	if (IS_INTRESOURCE(lpszBmpResID))
	{
		m_uiBmpResID = (UINT)((UINT_PTR)(lpszBmpResID));
	}
	else
	{
		m_strBmpResID = lpszBmpResID;
	}
}

CMFCControlRendererInfo& CMFCControlRendererInfo::operator = (const CMFCControlRendererInfo& rSrc)
{
	m_uiBmpResID        = rSrc.m_uiBmpResID;
	m_strBmpResID       = rSrc.m_strBmpResID;
	m_rectImage         = rSrc.m_rectImage;
	m_rectCorners       = rSrc.m_rectCorners;
	m_rectSides         = rSrc.m_rectSides;
	m_rectInter         = rSrc.m_rectInter;
	m_clrTransparent    = rSrc.m_clrTransparent;
	m_bPreMultiplyCheck = rSrc.m_bPreMultiplyCheck;

	return *this;
}

IMPLEMENT_DYNCREATE(CMFCControlRenderer, CObject)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFCControlRenderer::CMFCControlRenderer()
{
	m_bMirror = FALSE;
}

CMFCControlRenderer::~CMFCControlRenderer()
{
	CleanUp();
}

//////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////

BOOL CMFCControlRenderer::Create(const CMFCControlRendererInfo& params, BOOL bFlipvert /*= FALSE*/)
{
	CleanUp();

	m_Params = params;

	LPCTSTR lpszResID = m_Params.GetResourceID();
	if (lpszResID != NULL)
	{
		m_Bitmap.SetImageSize(m_Params.m_rectImage.Size());
		m_Bitmap.SetPreMultiplyAutoCheck(m_Params.m_bPreMultiplyCheck);
		m_Bitmap.SetMapTo3DColors(FALSE);
		m_Bitmap.LoadStr(lpszResID);

		if (bFlipvert)
		{
			m_Bitmap.MirrorVert();
		}

		if (m_Params.m_clrTransparent != CLR_DEFAULT)
		{
			m_Bitmap.SetTransparentColor(m_Params.m_clrTransparent);
		}

		if (CMFCToolBarImages::IsRTL() && m_Bitmap.GetImageWell() != NULL && m_Params.m_clrTransparent == CLR_DEFAULT)
		{
			BITMAP bmp;
			if (::GetObject(m_Bitmap.GetImageWell(), sizeof(BITMAP), &bmp) != 0)
			{
				if (bmp.bmBitsPixel == 32)
				{
					Mirror();
				}
			}
		}

		if (m_Params.m_rectSides.IsRectNull())
		{
			m_Params.m_rectSides = m_Params.m_rectCorners;
		}

		if (m_Params.m_rectInter.IsRectNull())
		{
			m_Params.m_rectInter = CRect(CPoint(0, 0), m_Params.m_rectImage.Size());
			m_Params.m_rectInter.left   += m_Params.m_rectCorners.left;
			m_Params.m_rectInter.top    += m_Params.m_rectCorners.top;
			m_Params.m_rectInter.right  -= m_Params.m_rectCorners.right;
			m_Params.m_rectInter.bottom -= m_Params.m_rectCorners.bottom;
		}

		if (bFlipvert)
		{
			long temp;
			temp = m_Params.m_rectCorners.top;
			m_Params.m_rectCorners.top = m_Params.m_rectCorners.bottom;
			m_Params.m_rectCorners.bottom = temp;

			temp = m_Params.m_rectSides.top;
			m_Params.m_rectSides.top = m_Params.m_rectSides.bottom;
			m_Params.m_rectSides.bottom = temp;

			long height = m_Params.m_rectImage.Height();
			temp = m_Params.m_rectInter.top;
			m_Params.m_rectInter.top = height - m_Params.m_rectInter.bottom;
			m_Params.m_rectInter.bottom = height - temp;
		}
	}

	return TRUE;
}

void CMFCControlRenderer::Mirror()
{
	if (m_Bitmap.Mirror())
	{
		m_bMirror = !m_bMirror;
	}
}

void CMFCControlRenderer::CleanUp()
{
	m_Bitmap.Clear();
	m_Bitmap.SetTransparentColor((COLORREF)(-1));

	CMFCControlRendererInfo emptyParams;
	m_Params = emptyParams;
	m_bMirror = FALSE;
}

void CMFCControlRenderer::Draw(CDC* pDC, CRect rect, UINT index, BYTE alphaSrc/* = 255*/)
{
	CRect rectInter(rect);
	rectInter.left   += m_Params.m_rectSides.left;
	rectInter.top    += m_Params.m_rectSides.top;
	rectInter.right  -= m_Params.m_rectSides.right;
	rectInter.bottom -= m_Params.m_rectSides.bottom;

	FillInterior(pDC, rectInter, index, alphaSrc);

	DrawFrame(pDC, rect, index, alphaSrc);
}

void CMFCControlRenderer::DrawFrame(CDC* pDC, CRect rect, UINT index, BYTE alphaSrc/* = 255*/)
{
	struct XHVTypes
	{
		CMFCToolBarImages::ImageAlignHorz horz;
		CMFCToolBarImages::ImageAlignVert vert;
	};

	XHVTypes corners[4] = 
	{
		{CMFCToolBarImages::ImageAlignHorzLeft , CMFCToolBarImages::ImageAlignVertTop},
		{CMFCToolBarImages::ImageAlignHorzRight, CMFCToolBarImages::ImageAlignVertTop},
		{CMFCToolBarImages::ImageAlignHorzLeft , CMFCToolBarImages::ImageAlignVertBottom},
		{CMFCToolBarImages::ImageAlignHorzRight, CMFCToolBarImages::ImageAlignVertBottom}
	};

	XHVTypes sides[4] =
	{
		{CMFCToolBarImages::ImageAlignHorzLeft   , CMFCToolBarImages::ImageAlignVertStretch},
		{CMFCToolBarImages::ImageAlignHorzRight  , CMFCToolBarImages::ImageAlignVertStretch},
		{CMFCToolBarImages::ImageAlignHorzStretch, CMFCToolBarImages::ImageAlignVertTop},
		{CMFCToolBarImages::ImageAlignHorzStretch, CMFCToolBarImages::ImageAlignVertBottom}
	};

	CRect rectImage(m_Params.m_rectImage);
	int ind = index;
	if (m_Bitmap.GetCount() == 1)
	{
		rectImage.OffsetRect(0, m_Params.m_rectImage.Size().cy * ind);
		ind = 0;
	}

	CRect rt(rect);
	CRect rectCorners(m_Params.m_rectCorners);
	CRect rectSides(m_Params.m_rectSides);

	rt.left   += rectCorners.left;
	rt.top    += rectCorners.top;
	rt.right  -= rectCorners.right;
	rt.bottom -= rectCorners.bottom;

	if (rt.Width() > 0 || rt.Height() > 0)
	{
		if (rt.Height() > 0)
		{
			if (rectSides.left > 0)
			{
				CRect r(rt);
				r.left  = rect.left;
				r.right = r.left + rectSides.left;

				CRect rectPart;
				if (m_bMirror)
				{
					rectPart = CRect(rectImage.right - rectSides.left, rectImage.top + rectCorners.top, rectImage.right, rectImage.bottom - rectCorners.bottom);
				}
				else
				{
					rectPart = CRect(rectImage.left, rectImage.top + rectCorners.top, rectImage.left + rectSides.left, rectImage.bottom - rectCorners.bottom);
				}

				m_Bitmap.DrawEx(pDC, r, ind, sides[0].horz, sides[0].vert, rectPart, alphaSrc);
			}

			if (rectSides.right > 0)
			{
				CRect r(rt);
				r.right = rect.right;
				r.left  = r.right - rectSides.right;

				CRect rectPart;
				if (m_bMirror)
				{
					rectPart = CRect(rectImage.left, rectImage.top + rectCorners.top, rectImage.left + rectSides.right, rectImage.bottom - rectCorners.bottom);
				}
				else
				{
					rectPart = CRect(rectImage.right - rectSides.right, rectImage.top + rectCorners.top, rectImage.right, rectImage.bottom - rectCorners.bottom);
				}

				m_Bitmap.DrawEx(pDC, r, ind, sides[1].horz, sides[1].vert, rectPart, alphaSrc);
			}
		}

		if (rt.Width() > 0)
		{
			if (rectSides.top > 0)
			{
				CRect r(rt);
				r.top    = rect.top;
				r.bottom = r.top + rectSides.top;

				CRect rectPart;
				if (m_bMirror)
				{
					rectPart = CRect(rectImage.left + rectCorners.right, rectImage.top, rectImage.right - rectCorners.left, rectImage.top + rectSides.top);
				}
				else
				{
					rectPart = CRect(rectImage.left + rectCorners.left, rectImage.top, rectImage.right - rectCorners.right, rectImage.top + rectSides.top);
				}

				m_Bitmap.DrawEx(pDC, r, ind, sides[2].horz, sides[2].vert, rectPart, alphaSrc);
			}

			if (rectSides.bottom > 0)
			{
				CRect r(rt);
				r.bottom = rect.bottom;
				r.top    = r.bottom - rectSides.bottom;

				CRect rectPart;
				if (m_bMirror)
				{
					rectPart = CRect(rectImage.left + rectCorners.right, rectImage.bottom - rectSides.bottom, rectImage.right - rectCorners.left, rectImage.bottom);
				}
				else
				{
					rectPart = CRect(rectImage.left + rectCorners.left, rectImage.bottom - rectSides.bottom, rectImage.right - rectCorners.right, rectImage.bottom);
				}

				m_Bitmap.DrawEx(pDC, r, ind, sides[3].horz, sides[3].vert, rectPart, alphaSrc);
			}
		}

		if (rectCorners.left > 0 && rectCorners.top > 0)
		{
			CRect rectPart;
			if (m_bMirror)
			{
				rectPart = CRect(CPoint(rectImage.right - rectCorners.left, rectImage.top), CSize(rectCorners.left, rectCorners.top));
			}
			else
			{
				rectPart = CRect(CPoint(rectImage.left, rectImage.top), CSize(rectCorners.left, rectCorners.top));
			}

			m_Bitmap.DrawEx(pDC, rect, ind, corners[0].horz, corners[0].vert, rectPart, alphaSrc);
		}

		if (rectCorners.right > 0 && rectCorners.top > 0)
		{
			CRect rectPart;
			if (m_bMirror)
			{
				rectPart = CRect(CPoint(rectImage.left, rectImage.top), CSize(rectCorners.right, rectCorners.top));
			}
			else
			{
				rectPart = CRect(CPoint(rectImage.right - rectCorners.right, rectImage.top), CSize(rectCorners.right, rectCorners.top));
			}

			m_Bitmap.DrawEx(pDC, rect, ind, corners[1].horz, corners[1].vert, rectPart, alphaSrc);
		}

		if (rectCorners.left > 0 && rectCorners.bottom > 0)
		{
			CRect rectPart;
			if (m_bMirror)
			{
				rectPart = CRect(CPoint(rectImage.right - rectCorners.left, rectImage.bottom - rectCorners.bottom), CSize(rectCorners.left, rectCorners.bottom));
			}
			else
			{
				rectPart = CRect(CPoint(rectImage.left, rectImage.bottom - rectCorners.bottom), CSize(rectCorners.left, rectCorners.bottom));
			}

			m_Bitmap.DrawEx(pDC, rect, ind, corners[2].horz, corners[2].vert, rectPart, alphaSrc);
		}

		if (rectCorners.right > 0 && rectCorners.bottom > 0)
		{
			CRect rectPart;
			if (m_bMirror)
			{
				rectPart = CRect(CPoint(rectImage.left, rectImage.bottom - rectCorners.bottom), CSize(rectCorners.right, rectCorners.bottom));
			}
			else
			{
				rectPart = CRect(CPoint(rectImage.right - rectCorners.right, rectImage.bottom - rectCorners.bottom), CSize(rectCorners.right, rectCorners.bottom));
			}

			m_Bitmap.DrawEx(pDC, rect, ind, corners[3].horz, corners[3].vert, rectPart, alphaSrc);
		}
	}
}

void CMFCControlRenderer::FillInterior(CDC* pDC, CRect rect, CMFCToolBarImages::ImageAlignHorz horz, CMFCToolBarImages::ImageAlignVert vert, UINT index, BYTE alphaSrc/* = 255*/)
{
	if (m_Params.m_rectInter.IsRectEmpty())
	{
		return;
	}

	CRect rectImage(m_Params.m_rectInter);

	if (m_bMirror)
	{
		rectImage.left  = m_Params.m_rectImage.Size().cx - m_Params.m_rectInter.right;
		rectImage.right = rectImage.left + m_Params.m_rectInter.Width();
	}

	rectImage.OffsetRect(m_Params.m_rectImage.TopLeft());

	int ind = index;
	if (m_Bitmap.GetCount() == 1)
	{
		rectImage.OffsetRect(0, m_Params.m_rectImage.Size().cy * ind);
		ind = 0;
	}

	m_Bitmap.DrawEx(pDC, rect, ind, horz, vert, rectImage, alphaSrc);
}

void CMFCControlRenderer::FillInterior(CDC* pDC, CRect rect, UINT index, BYTE alphaSrc/* = 255*/)
{
	FillInterior(pDC, rect, CMFCToolBarImages::ImageAlignHorzStretch, CMFCToolBarImages::ImageAlignVertStretch, index, alphaSrc);
}

void CMFCControlRenderer::OnSysColorChange()
{
	if (m_Bitmap.GetImageWell() != NULL)
	{
		m_Bitmap.OnSysColorChange();
	}
}


IMPLEMENT_DYNCREATE(CMFCShadowRenderer, CMFCControlRenderer)

CMFCShadowRenderer::CMFCShadowRenderer()
{
}

CMFCShadowRenderer::~CMFCShadowRenderer()
{
}

BOOL CMFCShadowRenderer::Create (const CMFCControlRendererInfo& /*params*/, BOOL /*bFlipvert*/ /*= FALSE*/)
{
	return FALSE;
}

BOOL CMFCShadowRenderer::Create (int nDepth,
								  COLORREF clrBase,
                                  int iMinBrightness/* = 0*/, int iMaxBrightness/* = 100*/)
{
	CleanUp ();

	HBITMAP hBitmap = CDrawingManager::PrepareShadowMask (nDepth, clrBase, iMinBrightness, iMaxBrightness);
	if (hBitmap == NULL)
	{
		return FALSE;
	}

	int nSize     = nDepth < 3 ? 3 : nDepth;
	int nDestSize = nSize * 2 + 1;

	m_Params.m_rectImage   = CRect (0, 0, nDestSize, nDestSize);
	m_Params.m_rectCorners = CRect (nSize, nSize, nSize, nSize);
	m_Params.m_rectSides = m_Params.m_rectCorners;

	m_Params.m_rectInter = CRect (CPoint (0, 0), m_Params.m_rectImage.Size ());
	m_Params.m_rectInter.left   += m_Params.m_rectCorners.left;
	m_Params.m_rectInter.top    += m_Params.m_rectCorners.top;
	m_Params.m_rectInter.right  -= m_Params.m_rectCorners.right;
	m_Params.m_rectInter.bottom -= m_Params.m_rectCorners.bottom;

	m_Bitmap.SetImageSize (m_Params.m_rectImage.Size ());
	m_Bitmap.SetPreMultiplyAutoCheck (m_Params.m_bPreMultiplyCheck);
	m_Bitmap.SetMapTo3DColors (FALSE);

	m_Bitmap.AddImage (hBitmap, TRUE);

	::DeleteObject (hBitmap);

	return m_Bitmap.GetCount () == 1;
}

void CMFCShadowRenderer::OnSysColorChange ()
{
}

void CMFCShadowRenderer::Draw (CDC* pDC, CRect rect, UINT index/* = 0*/, BYTE alphaSrc/* = 255*/)
{
	CRect rectInter (rect);
	
	if (CMFCToolBarImages::IsRTL ())
	{
		rectInter.left   += m_Params.m_rectSides.left;
		rectInter.right   = rectInter.left + m_Params.m_rectSides.left;
	}
	else
	{
		rectInter.right  -= m_Params.m_rectSides.right;
		rectInter.left    = rectInter.right - m_Params.m_rectSides.right;
	}

	rectInter.bottom -= m_Params.m_rectSides.bottom;
	rectInter.top     = rectInter.bottom - m_Params.m_rectSides.bottom;

	FillInterior (pDC, rectInter, index, alphaSrc);

	DrawFrame (pDC, rect, index, alphaSrc);
}

void CMFCShadowRenderer::DrawFrame (CDC* pDC, CRect rect, UINT index/* = 0*/, BYTE alphaSrc/* = 255*/)
{
	struct XHVTypes
	{
		CMFCToolBarImages::ImageAlignHorz horz;
		CMFCToolBarImages::ImageAlignVert vert;
	};

	XHVTypes corners[4] = 
	{
		{CMFCToolBarImages::ImageAlignHorzLeft , CMFCToolBarImages::ImageAlignVertTop},
		{CMFCToolBarImages::ImageAlignHorzRight, CMFCToolBarImages::ImageAlignVertTop},
		{CMFCToolBarImages::ImageAlignHorzLeft , CMFCToolBarImages::ImageAlignVertBottom},
		{CMFCToolBarImages::ImageAlignHorzRight, CMFCToolBarImages::ImageAlignVertBottom}
	};

	XHVTypes sides[4] = 
	{
		{CMFCToolBarImages::ImageAlignHorzLeft   , CMFCToolBarImages::ImageAlignVertStretch},
		{CMFCToolBarImages::ImageAlignHorzRight  , CMFCToolBarImages::ImageAlignVertStretch},
		{CMFCToolBarImages::ImageAlignHorzStretch, CMFCToolBarImages::ImageAlignVertTop},
		{CMFCToolBarImages::ImageAlignHorzStretch, CMFCToolBarImages::ImageAlignVertBottom}
	};

	CRect rectImage (m_Params.m_rectImage);
	int ind = index;
	if (m_Bitmap.GetCount () == 1)
	{
		rectImage.OffsetRect (0, m_Params.m_rectImage.Size ().cy * ind);
		ind = 0;
	}

	CRect rt  (rect);
	CRect rectCorners (m_Params.m_rectCorners);
	CRect rectSides   (m_Params.m_rectSides);

	rt.left   += rectCorners.left;
	rt.top    += rectCorners.top;
	rt.right  -= rectCorners.right;
	rt.bottom -= rectCorners.bottom;

	BOOL bRTL = CMFCToolBarImages::IsRTL ();

	if (rt.Width () > 0 || rt.Height () > 0)
	{
		if (rt.Height () > 0)
		{
			if (bRTL)
			{
				if (rectSides.left > 0)
				{
					CRect r (rt);
					r.left  = rect.left;
					r.right = r.left + rectSides.left;

					CRect rectPart (rectImage.left, 
						rectImage.top + rectCorners.top, rectImage.left + rectSides.left, rectImage.bottom - rectCorners.bottom);

					m_Bitmap.DrawEx (pDC, r, ind, sides[0].horz, sides[0].vert, rectPart, alphaSrc);
				}
			}
			else
			{
				if (rectSides.right > 0)
				{
					CRect r (rt);
					r.right = rect.right;
					r.left  = r.right - rectSides.right;

					CRect rectPart  (rectImage.right - rectSides.right, 
    						rectImage.top + rectCorners.top, rectImage.right, rectImage.bottom - rectCorners.bottom);

					m_Bitmap.DrawEx (pDC, r, ind, sides[1].horz, sides[1].vert, rectPart, alphaSrc);
				}
			}	
		}

		if (rt.Width () > 0)
		{
			if (rectSides.bottom > 0)
			{
				CRect r (rt);
				r.bottom = rect.bottom;
				r.top    = r.bottom - rectSides.bottom;
				
				CRect rectPart  (rectImage.left + rectCorners.left, 
    					rectImage.bottom - rectSides.bottom, rectImage.right - rectCorners.right, rectImage.bottom);

				m_Bitmap.DrawEx (pDC, r, ind, sides[3].horz, sides[3].vert, rectPart, alphaSrc);
			}
		}

		if (bRTL)
		{
			if (rectCorners.left > 0 && rectCorners.top > 0)
			{
				CRect rectPart (CPoint (rectImage.left, rectImage.top), 
						CSize (rectCorners.left, rectCorners.top));

				m_Bitmap.DrawEx (pDC, rect, ind, corners[0].horz, corners[0].vert, rectPart, alphaSrc);
			}
		}
		else
		{
			if (rectCorners.right > 0 && rectCorners.top > 0)
			{
				CRect rectPart (CPoint (rectImage.right - rectCorners.right, rectImage.top), 
						CSize (rectCorners.right, rectCorners.top));

				m_Bitmap.DrawEx (pDC, rect, ind, corners[1].horz, corners[1].vert, rectPart, alphaSrc);
			}
		}

		if (rectCorners.left > 0 && rectCorners.bottom > 0)
		{
			CRect rectPart (CPoint (rectImage.left, rectImage.bottom - rectCorners.bottom), 
					CSize (rectCorners.left, rectCorners.bottom));

			m_Bitmap.DrawEx (pDC, rect, ind, corners[2].horz, corners[2].vert, rectPart, alphaSrc);
		}

		if (rectCorners.right > 0 && rectCorners.bottom > 0)
		{
			CRect rectPart (CPoint (rectImage.right - rectCorners.right, rectImage.bottom - rectCorners.bottom), 
					CSize (rectCorners.right, rectCorners.bottom));

			m_Bitmap.DrawEx (pDC, rect, ind, corners[3].horz, corners[3].vert, rectPart, alphaSrc);
		}
	}
}

