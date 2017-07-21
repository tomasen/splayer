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
#include "afxribbonres.h"
#include "afximagepaintarea.h"
#include "afxtoolbarimages.h"
#include "afximageeditordialog.h"
#include "afxglobals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCImagePaintArea

CMFCImagePaintArea::CMFCImagePaintArea(CMFCImageEditorDialog* pParentDlg)
{
	ENSURE(pParentDlg != NULL);

	m_pParentDlg = pParentDlg;

	m_sizeImage.cx = 0;
	m_sizeImage.cy = 0;

	m_pBitmap = NULL;
	m_rgbColor = RGB(0, 0, 0); // Black

	m_rectParentPreviewArea.SetRectEmpty();
	m_memDC.CreateCompatibleDC(NULL);

	m_Mode = IMAGE_EDIT_MODE_PEN;
	m_rectDraw.SetRectEmpty();

	m_penDraw.CreatePen(PS_SOLID, 1, m_rgbColor);
}

CMFCImagePaintArea::~CMFCImagePaintArea()
{
	::DestroyCursor(m_hcurPen);
	::DestroyCursor(m_hcurFill);
	::DestroyCursor(m_hcurLine);
	::DestroyCursor(m_hcurRect);
	::DestroyCursor(m_hcurEllipse);
	::DestroyCursor(m_hcurColor);
}

BEGIN_MESSAGE_MAP(CMFCImagePaintArea, CButton)
	//{{AFX_MSG_MAP(CMFCImagePaintArea)
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_CANCELMODE()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCImagePaintArea message handlers

void CMFCImagePaintArea::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	ASSERT_VALID(pDC);

	CRect rectClient = lpDIS->rcItem;

	pDC->FillRect(&rectClient, &afxGlobalData.brBtnFace);
	rectClient.InflateRect(-1, -1);

	CRect rectDraw = rectClient;

	rectDraw.right = rectDraw.left + m_sizeCell.cx * m_sizeImage.cx;
	rectDraw.bottom = rectDraw.top + m_sizeCell.cy * m_sizeImage.cy;

	rectClient = rectDraw;
	rectClient.InflateRect(1, 1);

	pDC->Draw3dRect(rectDraw, afxGlobalData.clrBtnDkShadow, afxGlobalData.clrBtnHilite);

	// Draw grid:
	CPen penGrid(PS_SOLID, 1, afxGlobalData.clrBtnShadow);
	CPen* pOldPen = (CPen*) pDC->SelectObject(&penGrid);

	int x = 0;
	int y = 0;

	for (x = rectDraw.left + m_sizeCell.cx;
		x <= rectDraw.right - m_sizeCell.cx; x += m_sizeCell.cx)
	{
		pDC->MoveTo(x, rectDraw.top + 1);
		pDC->LineTo(x, rectDraw.bottom - 1);
	}

	for (y = rectDraw.top + m_sizeCell.cy;
		y <= rectDraw.bottom - m_sizeCell.cy; y += m_sizeCell.cy)
	{
		pDC->MoveTo(rectDraw.left + 1, y);
		pDC->LineTo(rectDraw.right - 1, y);
	}

	pDC->SelectObject(pOldPen);

	// Draw bitmap:
	if (m_pBitmap == NULL)
	{
		return;
	}

	CBitmap* pOldBitmap = m_memDC.SelectObject(m_pBitmap);

	for (x = 0; x < m_sizeImage.cx; x ++)
	{
		for (y = 0; y < m_sizeImage.cy; y ++)
		{
			COLORREF rgbPixel = CMFCToolBarImages::MapFromSysColor(m_memDC.GetPixel(x, y), FALSE);
			if (rgbPixel != (COLORREF) -1)
			{
				CRect rect(CPoint(rectDraw.left + x * m_sizeCell.cx, rectDraw.top + y * m_sizeCell.cy), m_sizeCell);
				rect.InflateRect(-1, -1);

				pDC->FillSolidRect(rect, rgbPixel);
			}
		}
	}

	m_memDC.SelectObject(pOldBitmap);
}

BOOL CMFCImagePaintArea::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CMFCImagePaintArea::OnMouseMove(UINT nFlags, CPoint point)
{
	if ((nFlags & MK_LBUTTON) == 0)
	{
		return;
	}

	CRect rectClient; // Client area rectangle
	GetClientRect(&rectClient);

	point.x = min(max(point.x, rectClient.left), rectClient.right);
	point.y = min(max(point.y, rectClient.top), rectClient.bottom);

	if (m_Mode == IMAGE_EDIT_MODE_PEN)
	{
		DrawPixel(point);
		return;
	}

	if (m_Mode != IMAGE_EDIT_MODE_LINE && m_Mode != IMAGE_EDIT_MODE_RECT && m_Mode != IMAGE_EDIT_MODE_ELLIPSE)
	{
		return;
	}

	CRect rectDraw = rectClient;

	rectDraw.right = rectDraw.left + m_sizeCell.cx * m_sizeImage.cx;
	rectDraw.bottom = rectDraw.top + m_sizeCell.cy * m_sizeImage.cy;

	rectDraw.DeflateRect(1, 1);

	if (m_rectDraw == rectDraw)
	{
		return;
	}

	CRgn rgnClip;
	rgnClip.CreateRectRgnIndirect(&rectDraw);

	CClientDC dc(this);
	dc.SelectObject(&rgnClip);

	CPen* pOldPen = (CPen*) dc.SelectObject(&m_penStretch);
	CBrush* pOldBrush = (CBrush*) dc.SelectStockObject(NULL_BRUSH);
	int nOldROP = dc.SetROP2(R2_NOT);

	switch (m_Mode)
	{
	case IMAGE_EDIT_MODE_LINE:
		{
			// Erase old:
			CRect rectScreen = m_rectDraw;
			BitmapToClient(rectScreen);

			if (m_rectDraw.Width() != 0 || m_rectDraw.Height() != 0)
			{
				dc.MoveTo(rectScreen.left, rectScreen.top);
				dc.LineTo(rectScreen.right, rectScreen.bottom);
			}

			ScreenToBitmap(point);
			m_rectDraw.right = point.x;
			m_rectDraw.bottom = point.y;

			// Draw new:
			rectScreen = m_rectDraw;
			BitmapToClient(rectScreen);

			if (m_rectDraw.Width() != 0 || m_rectDraw.Height() != 0)
			{
				dc.MoveTo(rectScreen.left, rectScreen.top);
				dc.LineTo(rectScreen.right, rectScreen.bottom);
			}
		}
		break;

	case IMAGE_EDIT_MODE_RECT:
		{
			// Erase old:
			CRect rectScreen = m_rectDraw;
			BitmapToClient(rectScreen);

			if (m_rectDraw.Width() != 0 || m_rectDraw.Height() != 0)
			{
				dc.Rectangle(rectScreen);
			}

			ScreenToBitmap(point);
			m_rectDraw.right = point.x;
			m_rectDraw.bottom = point.y;

			// Draw new:
			rectScreen = m_rectDraw;
			BitmapToClient(rectScreen);

			if (m_rectDraw.Width() != 0 || m_rectDraw.Height() != 0)
			{
				dc.Rectangle(rectScreen);
			}
		}
		break;

	case IMAGE_EDIT_MODE_ELLIPSE:
		{
			// Erase old:
			CRect rectScreen = m_rectDraw;
			BitmapToClient(rectScreen);

			if (m_rectDraw.Width() != 0 || m_rectDraw.Height() != 0)
			{
				dc.Ellipse(rectScreen);
			}

			ScreenToBitmap(point);
			m_rectDraw.right = point.x;
			m_rectDraw.bottom = point.y;

			// Draw new:
			rectScreen = m_rectDraw;
			BitmapToClient(rectScreen);

			if (m_rectDraw.Width() != 0 || m_rectDraw.Height() != 0)
			{
				dc.Ellipse(rectScreen);
			}
		}
		break;
	}

	dc.SetROP2(nOldROP);
	dc.SelectObject(pOldBrush);
	dc.SelectObject(pOldPen);
	dc.SelectClipRgn(NULL);
}

void CMFCImagePaintArea::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	if (m_pBitmap == NULL)
	{
		return;
	}

	CRect rectDraw;
	GetClientRect(rectDraw);

	rectDraw.right = rectDraw.left + m_sizeCell.cx * m_sizeImage.cx;
	rectDraw.bottom = rectDraw.top + m_sizeCell.cy * m_sizeImage.cy;

	rectDraw.DeflateRect(1, 1);

	if (!rectDraw.PtInRect(point))
	{
		return;
	}

	switch (m_Mode)
	{
	case IMAGE_EDIT_MODE_PEN:
		DrawPixel(point);
		break;

	case IMAGE_EDIT_MODE_LINE:
	case IMAGE_EDIT_MODE_RECT:
	case IMAGE_EDIT_MODE_ELLIPSE:
		ScreenToBitmap(point);
		m_rectDraw = CRect(point, CSize(0, 0));
		break;
	}

	SetCapture();
}

void CMFCImagePaintArea::OnLButtonUp(UINT /*nFlags*/, CPoint point)
{
	if (GetCapture() == this)
	{
		ReleaseCapture();
	}

	if (m_Mode == IMAGE_EDIT_MODE_RECT || m_Mode == IMAGE_EDIT_MODE_ELLIPSE)
	{
		if (m_rectDraw.Height() != 0)
		{
			if (m_rectDraw.top < m_rectDraw.bottom)
			{
				m_rectDraw.bottom ++;
			}
			else
			{
				m_rectDraw.top ++;
			}
		}

		if (m_rectDraw.Width() != 0)
		{
			if (m_rectDraw.left < m_rectDraw.right)
			{
				m_rectDraw.right ++;
			}
			else
			{
				m_rectDraw.left ++;
			}
		}
	}

	switch (m_Mode)
	{
	case IMAGE_EDIT_MODE_PEN:
		DrawPixel(point);
		break;

	case IMAGE_EDIT_MODE_LINE:
		if (m_rectDraw.Height() == 0 && m_rectDraw.Width() == 0)
		{
			DrawPixel(point);
		}
		else
		{
			CBitmap* pOldBitmap = m_memDC.SelectObject(m_pBitmap);
			CPen* pOldPen = (CPen*) m_memDC.SelectObject(&m_penDraw);

			m_memDC.MoveTo(m_rectDraw.left, m_rectDraw.top);
			m_memDC.LineTo(m_rectDraw.right, m_rectDraw.bottom);

			DrawPixel(point);

			m_memDC.SelectObject(pOldBitmap);
			m_memDC.SelectObject(pOldPen);

			Invalidate();
			UpdateWindow();

			GetParent()->InvalidateRect(m_rectParentPreviewArea);
		}
		break;

	case IMAGE_EDIT_MODE_RECT:
		if (m_rectDraw.Height() == 0 && m_rectDraw.Width() == 0)
		{
			DrawPixel(point);
		}
		else
		{
			CBitmap* pOldBitmap = m_memDC.SelectObject(m_pBitmap);
			CPen* pOldPen = (CPen*) m_memDC.SelectObject(&m_penDraw);
			CBrush* pOldBrush = (CBrush*) m_memDC.SelectStockObject(NULL_BRUSH);

			m_memDC.Rectangle(m_rectDraw);

			m_memDC.SelectObject(pOldBitmap);
			m_memDC.SelectObject(pOldPen);
			m_memDC.SelectObject(pOldBrush);

			Invalidate();
			UpdateWindow();

			GetParent()->InvalidateRect(m_rectParentPreviewArea);
		}
		break;

	case IMAGE_EDIT_MODE_ELLIPSE:
		if (m_rectDraw.Height() == 0 && m_rectDraw.Width() == 0)
		{
			DrawPixel(point);
		}
		else
		{
			CBitmap* pOldBitmap = m_memDC.SelectObject(m_pBitmap);
			CPen* pOldPen = (CPen*) m_memDC.SelectObject(&m_penDraw);
			CBrush* pOldBrush = (CBrush*) m_memDC.SelectStockObject(NULL_BRUSH);

			m_memDC.Ellipse(m_rectDraw);

			m_memDC.SelectObject(pOldBitmap);
			m_memDC.SelectObject(pOldPen);
			m_memDC.SelectObject(pOldBrush);

			Invalidate();
			UpdateWindow();

			GetParent()->InvalidateRect(m_rectParentPreviewArea);
		}
		break;

	case IMAGE_EDIT_MODE_FILL:
		ScreenToBitmap(point);
		FloodFill(point);
		break;

	case IMAGE_EDIT_MODE_COLOR:
		{
			ScreenToBitmap(point);

			CBitmap* pOldBitmap = m_memDC.SelectObject(m_pBitmap);
			COLORREF color = m_memDC.GetPixel(point);
			m_memDC.SelectObject(pOldBitmap);

			m_pParentDlg->OnPickColor(color);
		}
		break;
	}

	m_rectDraw.SetRectEmpty();
}

void CMFCImagePaintArea::OnCancelMode()
{
	if (GetCapture() == this)
	{
		ReleaseCapture();
	}
}

void CMFCImagePaintArea::DrawPixel(POINT point)
{
	CRect rectClient; // Client area rectangle
	GetClientRect(&rectClient);

	rectClient.InflateRect(-1, -1);

	CPoint ptBmp = point;
	ScreenToBitmap(ptBmp);

	CRect rect(CPoint(rectClient.left + ptBmp.x * m_sizeCell.cx, rectClient.top + ptBmp.y * m_sizeCell.cy), m_sizeCell);
	rect.InflateRect(-1, -1);

	CClientDC dc(this);
	dc.FillSolidRect(rect, m_rgbColor);

	// Update bitmap:
	CBitmap* pOldBitmap = m_memDC.SelectObject(m_pBitmap);
	m_memDC.SetPixel(ptBmp.x, ptBmp.y, CMFCToolBarImages::MapToSysColor(m_rgbColor));
	m_memDC.SelectObject(pOldBitmap);

	GetParent()->InvalidateRect(m_rectParentPreviewArea);
}

void CMFCImagePaintArea::SetBitmap(CBitmap* pBitmap)
{
	m_pBitmap = pBitmap;
	m_sizeCell = CSize(0, 0);

	if (m_pBitmap == NULL)
	{
		m_sizeImage.cx = 0;
		m_sizeImage.cy = 0;
		return;
	}

	BITMAP bmp;
	m_pBitmap->GetBitmap(&bmp);

	m_sizeImage.cx = bmp.bmWidth;
	m_sizeImage.cy = bmp.bmHeight;

	CRect rectClient;
	GetClientRect(rectClient);
	rectClient.DeflateRect(1, 1);

	m_sizeCell = CSize(rectClient.Width() / m_sizeImage.cx, rectClient.Height() / m_sizeImage.cy);

	if (m_penStretch.GetSafeHandle() != NULL)
	{
		m_penStretch.DeleteObject();
	}

	m_penStretch.CreatePen(PS_SOLID, min(m_sizeCell.cx, m_sizeCell.cy), afxGlobalData.clrBtnText);
}

BOOL CMFCImagePaintArea::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	CPoint ptCursor;
	::GetCursorPos(&ptCursor);
	ScreenToClient(&ptCursor);

	CRect rectDraw;
	GetClientRect(rectDraw);

	rectDraw.right = rectDraw.left + m_sizeCell.cx * m_sizeImage.cx;
	rectDraw.bottom = rectDraw.top + m_sizeCell.cy * m_sizeImage.cy;

	if (rectDraw.PtInRect(ptCursor))
	{
		switch (m_Mode)
		{
		case IMAGE_EDIT_MODE_PEN:
			::SetCursor(m_hcurPen);
			return TRUE;

		case IMAGE_EDIT_MODE_FILL:
			::SetCursor(m_hcurFill);
			return TRUE;

		case IMAGE_EDIT_MODE_LINE:
			::SetCursor(m_hcurLine);
			return TRUE;

		case IMAGE_EDIT_MODE_RECT:
			::SetCursor(m_hcurRect);
			return TRUE;

		case IMAGE_EDIT_MODE_ELLIPSE:
			::SetCursor(m_hcurEllipse);
			return TRUE;

		case IMAGE_EDIT_MODE_COLOR:
			::SetCursor(m_hcurColor);
			return TRUE;
		}
	}

	return CButton::OnSetCursor(pWnd, nHitTest, message);
}

void CMFCImagePaintArea::PreSubclassWindow()
{
	m_hcurPen = AfxGetApp()->LoadCursor(IDC_AFXBARRES_PEN);
	m_hcurFill = AfxGetApp()->LoadCursor(IDC_AFXBARRES_FILL);
	m_hcurLine = AfxGetApp()->LoadCursor(IDC_AFXBARRES_LINE);
	m_hcurRect = AfxGetApp()->LoadCursor(IDC_AFXBARRES_RECT);
	m_hcurEllipse = AfxGetApp()->LoadCursor(IDC_AFXBARRES_ELLIPSE);
	m_hcurColor = AfxGetApp()->LoadCursor(IDC_AFXBARRES_COLOR);

	CButton::PreSubclassWindow();
}

void CMFCImagePaintArea::ScreenToBitmap(CPoint& point)
{
	int x = (point.x - 1) / m_sizeCell.cx;
	int y = (point.y - 1) / m_sizeCell.cy;

	point.x = max(min(x, m_sizeImage.cx - 1), 0);
	point.y = max(min(y, m_sizeImage.cy - 1), 0);
}

void CMFCImagePaintArea::BitmapToClient(CRect& rect)
{
	rect.left = rect.left * m_sizeCell.cx + 1;
	rect.top = rect.top * m_sizeCell.cy + 1;
	rect.right = rect.right * m_sizeCell.cx + 1;
	rect.bottom = rect.bottom * m_sizeCell.cy + 1;

	rect.OffsetRect(m_sizeCell.cx / 2, m_sizeCell.cy / 2);
}

void CMFCImagePaintArea::FloodFill(const CPoint& point)
{
	ENSURE(m_pBitmap != NULL);

	CBitmap* pOldBitmap = m_memDC.SelectObject(m_pBitmap);

	CBrush br(m_rgbColor);
	CBrush* pBrOld = (CBrush*) m_memDC.SelectObject(&br);

	m_memDC.ExtFloodFill(point.x, point.y, m_memDC.GetPixel(point), FLOODFILLSURFACE);

	m_memDC.SelectObject(pOldBitmap);
	m_memDC.SelectObject(pBrOld);

	Invalidate();
	UpdateWindow();

	GetParent()->InvalidateRect(m_rectParentPreviewArea);
}

void CMFCImagePaintArea::SetColor(COLORREF color)
{
	m_rgbColor = color;

	if (m_penDraw.GetSafeHandle() != NULL)
	{
		m_penDraw.DeleteObject();
	}

	m_penDraw.CreatePen(PS_SOLID, 1, m_rgbColor);
}


