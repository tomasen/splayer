// This MFC Library source code supports the Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Classes Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// http://msdn.microsoft.com/officeui.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

#include "stdafx.h"

#include <math.h>
#include "afxcontrolbarutil.h"
#include "afxcolorbar.h"
#include "afxcolormenubutton.h"
#include "afxpopupmenu.h"
#include "afxcolordialog.h"
#include "afxcolorbutton.h"
#include "afxtrackmouse.h"
#include "afxvisualmanager.h"
#include "afxpropertygridctrl.h"
#include "afxribboncolorbutton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const int SEPARATOR_SIZE = 2;

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarColorButton

class CMFCToolBarColorButton : public CMFCToolBarButton
{
	friend class CMFCColorBar;

	DECLARE_SERIAL(CMFCToolBarColorButton)

protected:
	CMFCToolBarColorButton(COLORREF color = RGB(0, 0, 0), BOOL bIsAutomatic = FALSE, BOOL bIsOther = FALSE,
		LPCTSTR lpszColorName = NULL, BOOL bHighlight = FALSE, BOOL bIsDocument = FALSE, BOOL bIsOtherColor = FALSE)
	{
		m_Color = color;
		m_bHighlight = bHighlight;
		m_strText = (lpszColorName == NULL) ? _T("") : lpszColorName;
		m_bIsAutomatic = bIsAutomatic;
		m_bIsOther = bIsOther;
		m_bIsLabel = FALSE;
		m_bIsDocument = bIsDocument;
		m_bIsOtherColor = bIsOtherColor;
		m_pParentBar = NULL;
		m_bLocked = TRUE;
	}

	CMFCToolBarColorButton(LPCTSTR lpszColorName, BOOL bIsDocument = FALSE)
	{
		ENSURE(lpszColorName != NULL);

		m_Color = (COLORREF)-1;
		m_bHighlight = FALSE;
		m_strText = lpszColorName;
		m_bIsAutomatic = FALSE;
		m_bIsOther = FALSE;
		m_bIsLabel = TRUE;
		m_bIsDocument = bIsDocument;
		m_bIsOtherColor = FALSE;
		m_pParentBar = NULL;
		m_bLocked = TRUE;
	}

	virtual void OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages, BOOL bHorz = TRUE, BOOL bCustomizeMode = FALSE, BOOL bHighlight = FALSE, BOOL bDrawBorder = TRUE, BOOL bGrayDisabledButtons = TRUE);

	virtual BOOL OnToolHitTest(const CWnd* pWnd, TOOLINFO* pTI)
	{
		UNREFERENCED_PARAMETER(pWnd);

		if (m_nStyle & TBBS_DISABLED)
		{
			return FALSE;
		}

		if (!CMFCToolBar::GetShowTooltips() || pTI == NULL)
		{
			return FALSE;
		}

		CString str = m_strText;
		if (!m_bIsAutomatic && !m_bIsOther && !m_bIsLabel)
		{
			if (!CMFCColorBar::m_ColorNames.Lookup(m_Color, str))
			{
				str.Format(_T("Hex={%02X,%02X,%02X}"), GetRValue(m_Color), GetGValue(m_Color), GetBValue(m_Color));
			}
		}

		pTI->lpszText = (LPTSTR) ::calloc((str.GetLength() + 1), sizeof(TCHAR));
		if (pTI->lpszText == NULL)
		{
			return FALSE;
		}

		lstrcpy(pTI->lpszText, str);

		return TRUE;
	}

	virtual void OnChangeParentWnd(CWnd* pWndParent)
	{
		CMFCToolBarButton::OnChangeParentWnd(pWndParent);
		m_pParentBar = DYNAMIC_DOWNCAST(CMFCColorBar, pWndParent);
	}

	COLORREF m_Color;
	BOOL m_bHighlight;
	BOOL m_bIsAutomatic;
	BOOL m_bIsOther;
	BOOL m_bIsLabel;
	BOOL m_bIsDocument;
	BOOL m_bIsOtherColor;
	CMFCColorBar* m_pParentBar;
};

IMPLEMENT_SERIAL(CMFCToolBarColorButton, CMFCToolBarButton, 1)

void CMFCToolBarColorButton::OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* /*pImages*/, BOOL bHorz, BOOL bCustomizeMode, BOOL bHighlight, BOOL bDrawBorder, BOOL /*bGrayDisabledButtons*/)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(this);

	CPalette* pOldPalette = NULL;

	if (m_pParentBar != NULL)
	{
		pOldPalette = m_pParentBar->SelectPalette(pDC);
	}

	BOOL bDisabled = (m_nStyle & TBBS_DISABLED);

	UINT nStyle = m_nStyle;

	if (m_bHighlight)
	{
		m_nStyle |= TBBS_CHECKED;
	}

	// Fill button interior:
	FillInterior(pDC, rect, bHighlight && !m_bIsLabel);

	// Draw button border:
	if (!bDisabled && !m_bIsLabel && HaveHotBorder() && bDrawBorder && !bCustomizeMode)
	{
		if (m_nStyle &(TBBS_PRESSED | TBBS_CHECKED))
		{
			// Pressed in or checked:
			CMFCVisualManager::GetInstance()->OnDrawButtonBorder(pDC, this, rect, CMFCVisualManager::ButtonsIsPressed);
		}
		else if (bHighlight && !(m_nStyle &(TBBS_CHECKED | TBBS_INDETERMINATE)))
		{
			CMFCVisualManager::GetInstance()->OnDrawButtonBorder(pDC, this, rect, CMFCVisualManager::ButtonsIsHighlighted);
		}
	}

	CRect rectFrame = rect;
	rectFrame.DeflateRect(afxData.cxBorder2, afxData.cyBorder2);

	if (m_bIsOther || m_bIsAutomatic || m_bIsLabel)
	{
		CRect rectText = rect;

		if (m_bIsAutomatic && m_Color != (COLORREF)-1)
		{
			CRect rectColor = rectFrame;
			rectColor.DeflateRect(afxData.cxBorder2 + 1, afxData.cyBorder2 + 1);
			rectColor.right = rectColor.left + rectColor.Height();

			if (!bDisabled)
			{
				CBrush br(PALETTERGB(GetRValue(m_Color), GetGValue(m_Color), GetBValue(m_Color)));
				CPen pen(PS_SOLID, 1, afxGlobalData.clrBarShadow);

				CBrush* pOldBrush = pDC->SelectObject(&br);
				CPen* pOldPen = pDC->SelectObject(&pen);

				pDC->Rectangle(rectColor);

				pDC->SelectObject(pOldPen);
				pDC->SelectObject(pOldBrush);
			}
			else
			{
				pDC->Draw3dRect(rectColor, afxGlobalData.clrBarHilite, afxGlobalData.clrBarShadow);
				rectColor.OffsetRect(1, 1);
				pDC->Draw3dRect(rectColor, afxGlobalData.clrBarShadow, afxGlobalData.clrBarHilite);
			}

			rectText.left = rectColor.right + afxData.cxBorder2;
		}

		// Draw label:
		pDC->SetTextColor((m_nStyle & TBBS_DISABLED) && !m_bIsLabel ? afxGlobalData.clrGrayedText : afxGlobalData.clrBarText);

		UINT nFormat = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS;
		if (!m_bIsLabel)
		{
			nFormat |= DT_CENTER;
		}

		CFont* pCurrFont = NULL;
		if (!bHorz)
		{
			pCurrFont = pDC->SelectObject(&afxGlobalData.fontRegular);
			ENSURE(pCurrFont != NULL);
		}

		pDC->DrawText(m_strText, &rectText, nFormat);

		if (pCurrFont != NULL)
		{
			pDC->SelectObject(pCurrFont);
		}
	}
	else if (!bDisabled)
	{
		// Draw color box:
		CBrush br(PALETTERGB(GetRValue(m_Color), GetGValue(m_Color), GetBValue(m_Color)));

		CBrush* pOldBrush = pDC->SelectObject(&br);
		CPen* pOldPen = (CPen*) pDC->SelectObject(&afxGlobalData.penBarShadow);

		rectFrame.right--;
		rectFrame.bottom--;

		pDC->Rectangle(rectFrame);

		pDC->SelectObject(pOldPen);
		pDC->SelectObject(pOldBrush);
	}

	// Draw frame:
	if (!m_bIsOther && !m_bIsLabel)
	{
		if (!bDisabled)
		{
			pDC->Draw3dRect(rectFrame, afxGlobalData.clrBarShadow, afxGlobalData.clrBarShadow);
		}
		else
		{
			pDC->Draw3dRect(rectFrame, afxGlobalData.clrBarHilite, afxGlobalData.clrBarShadow);
			rectFrame.OffsetRect(1, 1);
			pDC->Draw3dRect(rectFrame, afxGlobalData.clrBarShadow, afxGlobalData.clrBarHilite);
		}
	}

	if (pOldPalette != NULL)
	{
		pDC->SelectPalette(pOldPalette, FALSE);
	}

	m_nStyle = nStyle;
}

/////////////////////////////////////////////////////////////////////////////
// CMFCColorBar

IMPLEMENT_SERIAL(CMFCColorBar, CMFCPopupMenuBar, 1)

CMap<COLORREF,COLORREF,CString, LPCTSTR> CMFCColorBar::m_ColorNames;

CMFCColorBar::CMFCColorBar()
{
	m_nNumColumns = 0;
	m_nNumRowsHorz = 0;
	m_nNumColumnsVert = 0;
	m_BoxSize = CSize(0, 0);
	m_nRowHeight = 0;
	m_ColorSelected = (COLORREF) -1;
	m_ColorAutomatic = (COLORREF) -1;
	m_nCommandID = 0;
	m_bStdColorDlg = FALSE;
	m_bIsTearOff = TRUE;
	m_bShowDocColorsWhenDocked = FALSE;
	m_bLocked = TRUE;
	m_bIsEnabled = TRUE;
	m_pParentBtn = NULL;
	m_pParentRibbonBtn = NULL;
	m_pWndPropList = NULL;
	m_nHorzOffset = m_nVertOffset = 0;
	m_bInternal = FALSE;
	m_nVertMargin = 4;
	m_nHorzMargin = 4;
}

CMFCColorBar::CMFCColorBar(const CArray<COLORREF, COLORREF>& colors, COLORREF color, LPCTSTR lpszAutoColor, LPCTSTR lpszOtherColor, LPCTSTR lpszDocColors,
		CList<COLORREF,COLORREF>& lstDocColors, int nColumns, int nRowsDockHorz, int nColDockVert, COLORREF colorAutomatic, UINT nCommandID, CMFCColorButton* pParentBtn) :
	m_ColorSelected(color), m_strAutoColor(lpszAutoColor == NULL ? _T("") : lpszAutoColor), m_strOtherColor(lpszOtherColor == NULL ? _T("") : lpszOtherColor),
	m_strDocColors(lpszDocColors == NULL ? _T("") : lpszDocColors), m_nNumColumns(nColumns), m_nNumRowsHorz(nRowsDockHorz), m_nNumColumnsVert(nColDockVert),
	m_ColorAutomatic(colorAutomatic), m_bIsTearOff(FALSE), m_bStdColorDlg(FALSE), m_nCommandID(nCommandID), m_pParentBtn(pParentBtn)
{
	m_pWndPropList = NULL;
	m_pParentRibbonBtn = NULL;

	m_colors.SetSize(colors.GetSize());

	for (int i = 0; i < colors.GetSize(); i++)
	{
		m_colors [i] = colors [i];
	}

	m_lstDocColors.AddTail(&lstDocColors);
	m_bLocked = TRUE;
	m_bIsEnabled = TRUE;
	m_bShowDocColorsWhenDocked = TRUE;

	if (m_pParentBtn != NULL)
	{
		m_bStdColorDlg = !m_pParentBtn->m_bAltColorDlg;
	}

	m_nHorzOffset = m_nVertOffset = 0;
	m_bInternal = FALSE;
	m_nVertMargin = 4;
	m_nHorzMargin = 4;
}

CMFCColorBar::CMFCColorBar(const CArray<COLORREF, COLORREF>& colors, COLORREF color, LPCTSTR lpszAutoColor, LPCTSTR lpszOtherColor, LPCTSTR lpszDocColors,
		CList<COLORREF,COLORREF>& lstDocColors, int nColumns, COLORREF colorAutomatic, UINT nCommandID, CMFCRibbonColorButton* pParentRibbonBtn) :
	m_ColorSelected(color), m_strAutoColor(lpszAutoColor == NULL ? _T("") : lpszAutoColor), m_strOtherColor(lpszOtherColor == NULL ? _T("") : lpszOtherColor),
	m_strDocColors(lpszDocColors == NULL ? _T("") : lpszDocColors), m_nNumColumns(nColumns), m_nNumRowsHorz(0), m_nNumColumnsVert(0), m_ColorAutomatic(colorAutomatic),
	m_bIsTearOff(FALSE), m_bStdColorDlg(FALSE), m_nCommandID(nCommandID), m_pParentRibbonBtn(pParentRibbonBtn)
{
	m_pWndPropList = NULL;
	m_pParentBtn = NULL;

	m_colors.SetSize(colors.GetSize());

	for (int i = 0; i < colors.GetSize(); i++)
	{
		m_colors [i] = colors [i];
	}

	m_lstDocColors.AddTail(&lstDocColors);
	m_bLocked = TRUE;
	m_bIsEnabled = TRUE;
	m_bShowDocColorsWhenDocked = TRUE;

	m_bStdColorDlg = FALSE;

	m_nHorzOffset = m_nVertOffset = 0;
	m_bInternal = FALSE;
	m_nVertMargin = 4;
	m_nHorzMargin = 4;
}

CMFCColorBar::CMFCColorBar(CMFCColorBar& src, UINT uiCommandID) :
	m_ColorSelected(src.m_ColorSelected), m_strAutoColor(src.m_strAutoColor), m_strOtherColor(src.m_strOtherColor), m_strDocColors(src.m_strDocColors),
	m_ColorAutomatic(src.m_ColorAutomatic), m_nNumColumns(src.m_nNumColumns), m_nNumRowsHorz(src.m_nNumRowsHorz), m_nNumColumnsVert(src.m_nNumColumnsVert),
	m_bIsTearOff(TRUE), m_nCommandID(uiCommandID), m_bStdColorDlg(src.m_bStdColorDlg)
{
	m_colors.SetSize(src.m_colors.GetSize());

	for (int i = 0; i < src.m_colors.GetSize(); i++)
	{
		m_colors [i] = src.m_colors [i];
	}

	m_lstDocColors.AddTail(&src.m_lstDocColors);
	m_bLocked = TRUE;
	m_bIsEnabled = TRUE;
	m_bShowDocColorsWhenDocked = FALSE;
	m_pParentBtn = NULL;
	m_pParentRibbonBtn = NULL;
	m_pWndPropList = NULL;
	m_nHorzOffset = m_nVertOffset = 0;
	m_bInternal = FALSE;
	m_nVertMargin = 4;
	m_nHorzMargin = 4;
}

CMFCColorBar::~CMFCColorBar()
{
}

void CMFCColorBar::AdjustLocations()
{
	if (GetSafeHwnd() == NULL || !::IsWindow(m_hWnd) || m_bInUpdateShadow)
	{
		return;
	}

	ASSERT_VALID(this);

	CRect rectClient; // Client area rectangle
	GetClientRect(&rectClient);

	rectClient.DeflateRect(m_nHorzMargin + m_nHorzOffset, m_nVertMargin + m_nVertOffset);

	int x = rectClient.left;
	int y = rectClient.top;
	int i = 0;

	BOOL bPrevSeparator = FALSE;
	BOOL bIsOtherColor = (m_strAutoColor.IsEmpty() || m_ColorSelected != (COLORREF)-1);

	for (POSITION pos = m_Buttons.GetHeadPosition(); pos != NULL; i ++)
	{
		CRect rectButton(0, 0, 0, 0);

		CMFCToolBarButton* pButton = (CMFCToolBarButton*) m_Buttons.GetNext(pos);
		if (pButton->m_nStyle & TBBS_SEPARATOR)
		{
			if (bPrevSeparator)
			{
				rectButton.SetRectEmpty();
			}
			else
			{
				if (x > rectClient.left)
				{
					// Next line
					x = rectClient.left;
					y += m_BoxSize.cy + m_nVertMargin;
				}

				rectButton = CRect(CPoint(x, y), CSize(rectClient.Width(), SEPARATOR_SIZE));

				y += SEPARATOR_SIZE + 2;
				x = rectClient.left;
			}

			bPrevSeparator = TRUE;
		}
		else
		{
			CMFCToolBarColorButton* pColorButton = DYNAMIC_DOWNCAST(CMFCToolBarColorButton, pButton);
			if (pColorButton == NULL)
			{
				continue;
			}

			ASSERT_VALID(pColorButton);

			if (pColorButton->m_bIsDocument && !m_bShowDocColorsWhenDocked && !IsFloating())
			{
				rectButton.SetRectEmpty();
			}
			else if (pColorButton->m_bIsAutomatic || pColorButton->m_bIsOther || pColorButton->m_bIsLabel)
			{
				if (x > rectClient.left)
				{
					// Next line
					x = rectClient.left;
					y += m_BoxSize.cy + m_nVertMargin;
				}

				if (pColorButton->m_bIsOther && bIsOtherColor)
				{
					rectButton = CRect(CPoint(x, y), CSize(rectClient.Width() - m_BoxSize.cx, m_nRowHeight - m_nVertMargin / 2));
					x = rectButton.right;
					y += (rectButton.Height() - m_BoxSize.cy) / 2;
				}
				else
				{
					rectButton = CRect(CPoint(x, y), CSize(rectClient.Width(), m_nRowHeight - m_nVertMargin / 2));
					y += m_nRowHeight - m_nVertMargin / 2;
					x = rectClient.left;
				}

				if (pColorButton->m_bIsOther)
				{
					rectButton.DeflateRect(m_nHorzMargin / 2, m_nVertMargin / 2);
				}

				bPrevSeparator = FALSE;
			}
			else
			{
				if (x + m_BoxSize.cx > rectClient.right)
				{
					x = rectClient.left;
					y += m_BoxSize.cy;
				}

				if (pColorButton->m_bIsOtherColor && !bIsOtherColor)
				{
					rectButton.SetRectEmpty();
				}
				else
				{
					rectButton = CRect(CPoint(x, y), m_BoxSize);
					x += m_BoxSize.cx;

					bPrevSeparator = FALSE;
				}

				if (pColorButton->m_Color == m_ColorSelected && !pColorButton->m_bIsOtherColor)
				{
					bIsOtherColor = FALSE;
				}
			}
		}

		pButton->SetRect(rectButton);
	}

	UpdateTooltips();
}

CSize CMFCColorBar::CalcSize(BOOL bVertDock)
{
	CSize sizeGrid = GetColorGridSize(bVertDock);

	return CSize(sizeGrid.cx * m_BoxSize.cx + 2 * m_nVertMargin, sizeGrid.cy * m_BoxSize.cy + GetExtraHeight(sizeGrid.cx) + 2 * m_nHorzMargin);
}

CSize CMFCColorBar::GetColorGridSize(BOOL bVertDock) const
//
// Calculate number of columns and rows in the color grid
//
{
	int nNumColumns = 0;
	int nNumRows = 0;

	int nColors = (int) m_colors.GetSize();

	if (!m_bIsTearOff || IsFloating() || bVertDock || m_nNumRowsHorz <= 0)
	{
		nNumColumns = !m_bIsTearOff || IsFloating() || m_nNumColumnsVert <= 0 ? m_nNumColumns : m_nNumColumnsVert;
		if (nNumColumns <= 0)
		{
			nNumColumns = (int)(sqrt((double) nColors)) + 1;
		}

		nNumRows = nColors / nNumColumns;
		if ((nColors % nNumColumns) != 0)
		{
			nNumRows ++;
		}
	}
	else // Horz dock
	{
		nNumRows = m_nNumRowsHorz;
		nNumColumns = nColors / nNumRows;

		if ((nColors % nNumRows) != 0)
		{
			nNumColumns ++;
		}
	}

	return CSize(nNumColumns, nNumRows);
}

int CMFCColorBar::GetExtraHeight(int nNumColumns) const
//
// Calculate additional height required by the misc. elements such
// as "Other" button, document colors, e.t.c
//
{
	int nExtraHeight = 0;

	if (!m_strAutoColor.IsEmpty())
	{
		nExtraHeight += m_nRowHeight;
	}
	else if (!m_strOtherColor.IsEmpty())
	{
		nExtraHeight += m_nVertMargin;
	}

	if (!m_strOtherColor.IsEmpty())
	{
		nExtraHeight += m_nRowHeight;
	}

	if (!m_strDocColors.IsEmpty() && !m_lstDocColors.IsEmpty() && (m_bShowDocColorsWhenDocked || IsFloating()))
	{
		int nDocColorRows = (int) m_lstDocColors.GetCount() / nNumColumns;
		if ((m_lstDocColors.GetCount() % nNumColumns) != 0)
		{
			nDocColorRows++;
		}

		nExtraHeight += m_nRowHeight + nDocColorRows * m_BoxSize.cy + 2 * SEPARATOR_SIZE + m_nVertMargin;
	}

	return nExtraHeight;
}

//{{AFX_MSG_MAP(CMFCColorBar)
BEGIN_MESSAGE_MAP(CMFCColorBar, CMFCPopupMenuBar)
	ON_WM_CREATE()
	ON_WM_QUERYNEWPALETTE()
	ON_WM_PALETTECHANGED()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_MOUSELEAVE, &CMFCColorBar::OnMouseLeave)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// CMFCColorBar message handlers

void CMFCColorBar::SetDocumentColors(LPCTSTR lpszCaption, CList<COLORREF,COLORREF>& lstDocColors, BOOL bShowWhenDocked)
{
	m_strDocColors = lpszCaption == NULL ? _T("") : lpszCaption;

	if (m_lstDocColors.GetCount() == lstDocColors.GetCount())
	{
		BOOL bChanged = FALSE;

		POSITION posCur = m_lstDocColors.GetHeadPosition();
		POSITION posNew = lstDocColors.GetHeadPosition();

		while (posCur != NULL && posNew != NULL)
		{
			if (m_lstDocColors.GetNext(posCur) != lstDocColors.GetNext(posNew))
			{
				bChanged = TRUE;
				break;
			}
		}

		if (!bChanged)
		{
			return;
		}
	}

	m_lstDocColors.RemoveAll();
	m_lstDocColors.AddTail(&lstDocColors);

	m_bShowDocColorsWhenDocked = bShowWhenDocked;

	Rebuild();
	AdjustLayout();
}

void CMFCColorBar::ContextToSize(BOOL bSquareButtons, BOOL bCenterButtons)
{
	ENSURE(GetSafeHwnd() != NULL);

	CRect rectClient;
	GetClientRect(rectClient);

	// First, adjust height:
	int nCurrHeight = CalcSize(TRUE).cy;
	int yDelta = nCurrHeight < rectClient.Height() ? 1 : -1;

	while ((nCurrHeight = CalcSize(TRUE).cy) != rectClient.Height())
	{
		if (yDelta < 0)
		{
			if (nCurrHeight < rectClient.Height())
			{
				break;
			}
		}
		else if (nCurrHeight > rectClient.Height())
		{
			m_BoxSize.cy -= yDelta;
			m_nRowHeight = m_BoxSize.cy * 3 / 2;
			break;
		}

		m_BoxSize.cy += yDelta;
		m_nRowHeight = m_BoxSize.cy * 3 / 2;
	}

	// Now, adjust width:
	int nCurrWidth = CalcSize(TRUE).cx;
	int xDelta = nCurrWidth < rectClient.Width() ? 1 : -1;

	while ((nCurrWidth = CalcSize(TRUE).cx) != rectClient.Width())
	{
		if (xDelta < 0)
		{
			if (nCurrWidth < rectClient.Width())
			{
				break;
			}
		}
		else if (nCurrWidth > rectClient.Width())
		{
			m_BoxSize.cy -= xDelta;
			break;
		}

		m_BoxSize.cx += xDelta;
	}

	m_BoxSize.cx--;
	m_BoxSize.cy--;

	if (bSquareButtons)
	{
		m_BoxSize.cx = m_BoxSize.cy = min(m_BoxSize.cx, m_BoxSize.cy);
		m_nRowHeight = m_BoxSize.cy * 3 / 2;
	}

	if (bCenterButtons)
	{
		// Finaly, calculate offset to center buttons area:
		CSize size = CalcSize(TRUE);

		m_nHorzOffset = (rectClient.Width() - size.cx) / 2;
		m_nVertOffset = (rectClient.Height() - size.cy) / 2;
	}
	else
	{
		m_nHorzOffset = m_nVertOffset = 0;
	}

	AdjustLocations();
}

void CMFCColorBar::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	if (m_bIsTearOff)
	{
		CMFCToolBar::OnNcCalcSize(bCalcValidRects, lpncsp);
	}
	else
	{
		CMFCPopupMenuBar::OnNcCalcSize(bCalcValidRects, lpncsp);
	}
}

void CMFCColorBar::OnNcPaint()
{
	if (m_bIsTearOff)
	{
		CMFCToolBar::OnNcPaint();
	}
	else
	{
		CMFCPopupMenuBar::OnNcPaint();
	}
}

int CMFCColorBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMFCPopupMenuBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_BoxSize = GetMenuImageSize();
	m_BoxSize.cx ++;
	m_BoxSize.cy ++;

	m_bLeaveFocus = FALSE;
	m_nRowHeight = m_BoxSize.cy * 3 / 2;
	Rebuild();

	if (m_pParentBtn != NULL)
	{
		SetCapture();
		m_pParentBtn->m_bCaptured = FALSE;
	}
	else if (m_pWndPropList != NULL)
	{
		SetCapture();
	}

	return 0;
}

void CMFCColorBar::Rebuild()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	RemoveAllButtons();

	BOOL bAlreadySelected = FALSE;
	if (!m_strAutoColor.IsEmpty()) // Automatic
	{
		InsertButton(new CMFCToolBarColorButton(m_ColorAutomatic, TRUE, FALSE, m_strAutoColor, m_ColorSelected == (COLORREF) -1));

		if (!bAlreadySelected)
		{
			bAlreadySelected = (m_ColorSelected == (COLORREF) -1);
		}
	}

	for (int i = 0; i < m_colors.GetSize(); i ++)
	{
		InsertButton(new CMFCToolBarColorButton(m_colors [i], FALSE, FALSE, NULL, m_ColorSelected == m_colors [i]));

		if (!bAlreadySelected)
		{
			bAlreadySelected = (m_ColorSelected == m_colors [i]);
		}
	}

	if (!m_strDocColors.IsEmpty() && !m_lstDocColors.IsEmpty())
	{
		InsertSeparator();
		InsertButton(new CMFCToolBarColorButton(m_strDocColors, TRUE)); // Label

		for (POSITION pos = m_lstDocColors.GetHeadPosition(); pos != NULL;)
		{
			COLORREF color = m_lstDocColors.GetNext(pos);
			InsertButton(new CMFCToolBarColorButton(color, FALSE, FALSE, NULL, !bAlreadySelected && m_ColorSelected == color, TRUE));
		}
	}

	if (!m_strOtherColor.IsEmpty()) // Other color button
	{
		InsertSeparator();
		InsertButton(new CMFCToolBarColorButton((COLORREF) -1,FALSE, TRUE, m_strOtherColor));
		InsertButton(new CMFCToolBarColorButton(m_ColorSelected, FALSE, FALSE, NULL, !bAlreadySelected, FALSE, TRUE));
	}
}

class CMFCColorBarCmdUI  : public CCmdUI
{
public:
	CMFCColorBarCmdUI ();

public: // re-implementations only
	virtual void Enable(BOOL bOn);
	BOOL m_bEnabled;
};

CMFCColorBarCmdUI ::CMFCColorBarCmdUI ()
{
	m_bEnabled = TRUE;  // assume it is enabled
}

void CMFCColorBarCmdUI ::Enable(BOOL bOn)
{
	m_bEnabled = bOn;
	m_bEnableChanged = TRUE;
}

void CMFCColorBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	ASSERT_VALID(this);

	if (m_nCommandID == 0 || m_nCommandID == (UINT)-1)
	{
		CMFCPopupMenuBar::OnUpdateCmdUI(pTarget, bDisableIfNoHndler);
		return;
	}

	CMFCColorBarCmdUI  state;
	state.m_pOther = this;
	state.m_nIndexMax = 1;
	state.m_nID = m_nCommandID;

	BOOL bIsEnabled = FALSE;
	if (pTarget->OnCmdMsg(m_nCommandID, CN_UPDATE_COMMAND_UI, &state, NULL))
	{
		bIsEnabled = state.m_bEnabled;
	}
	else if (bDisableIfNoHndler && !state.m_bEnableChanged)
	{
		AFX_CMDHANDLERINFO info;
		info.pTarget = NULL;

		bIsEnabled = pTarget->OnCmdMsg(m_nCommandID, CN_COMMAND, &state, &info);
	}

	if (bIsEnabled != m_bIsEnabled)
	{
		m_bIsEnabled = bIsEnabled;

		for (POSITION pos = m_Buttons.GetHeadPosition(); pos != NULL;)
		{
			CMFCToolBarColorButton* pColorButton = DYNAMIC_DOWNCAST(CMFCToolBarColorButton, m_Buttons.GetNext(pos));
			if (pColorButton != NULL)
			{
				pColorButton->m_nStyle &= ~TBBS_DISABLED;
				if (!bIsEnabled)
				{
					pColorButton->m_nStyle |= TBBS_DISABLED;
				}
			}
		}

		Invalidate();
		UpdateWindow();
	}

	CMFCPopupMenuBar::OnUpdateCmdUI(pTarget, bDisableIfNoHndler);
}

void CMFCColorBar::DoPaint(CDC* pDC)
{
	CPalette* pOldPalette = SelectPalette(pDC);

	CMFCPopupMenuBar::DoPaint(pDC);

	if (pOldPalette != NULL)
	{
		pDC->SelectPalette(pOldPalette, FALSE);
	}
}

BOOL CMFCColorBar::OnQueryNewPalette()
{
	Invalidate();
	UpdateWindow();
	return CMFCPopupMenuBar::OnQueryNewPalette();
}

void CMFCColorBar::OnPaletteChanged(CWnd* pFocusWnd)
{
	CMFCPopupMenuBar::OnPaletteChanged(pFocusWnd);

	if (pFocusWnd->GetSafeHwnd() != GetSafeHwnd())
	{
		Invalidate();
		UpdateWindow();
	}
}

BOOL CMFCColorBar::OnSendCommand(const CMFCToolBarButton* pButton)
{
	if (m_pParentBtn != NULL || m_pWndPropList != NULL)
	{
		ReleaseCapture();
	}

	COLORREF color = (COLORREF) -1;

	CMFCColorMenuButton* pColorMenuButton = NULL;

	CMFCPopupMenu* pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, GetParent());
	if (pParentMenu != NULL)
	{
		pColorMenuButton = DYNAMIC_DOWNCAST(CMFCColorMenuButton, pParentMenu->GetParentButton());
	}

	CMFCToolBarColorButton* pColorButton = DYNAMIC_DOWNCAST(CMFCToolBarColorButton, pButton);
	if (pColorButton == NULL)
	{
		ASSERT(FALSE);
	}
	else if (pColorButton->m_bIsLabel)
	{
		return FALSE;
	}
	else if (pColorButton->m_bIsOther)
	{
		SetInCommand();

		if (pParentMenu != NULL)
		{
			pParentMenu->ShowWindow(SW_HIDE);

			if (AFXGetTopLevelFrame(this) != NULL)
			{
				CMFCPopupMenu::ActivatePopupMenu(AFXGetTopLevelFrame(this), NULL);
			}
		}

		HWND hwnd = GetSafeHwnd();

		InvalidateRect(pButton->Rect());
		UpdateWindow();

		// Invoke color dialog:
		if (!OpenColorDialog(m_ColorSelected == (COLORREF)-1 ? m_ColorAutomatic : m_ColorSelected, color))
		{
			if (!::IsWindow(hwnd))
			{
				return TRUE;
			}

			SetInCommand(FALSE);

			if (m_pParentBtn != NULL || m_pWndPropList != NULL || m_pParentRibbonBtn != NULL)
			{
				GetParent()->SendMessage(WM_CLOSE);
			}
			else if (pColorMenuButton != NULL)
			{
				InvokeMenuCommand(0, pColorMenuButton);
			}
			else if (AFXGetTopLevelFrame(this) != NULL)
			{
				AFXGetTopLevelFrame(this)->SetFocus();
			}

			return TRUE;
		}

		if (!::IsWindow(hwnd))
		{
			return TRUE;
		}

		SetInCommand(FALSE);
	}
	else if (pColorButton->m_bIsAutomatic)
	{
		color = (COLORREF) -1;
	}
	else
	{
		color = pColorButton->m_Color;
	}

	if (pColorMenuButton != NULL)
	{
		pColorMenuButton->SetColor(color);
		InvokeMenuCommand(pColorMenuButton->m_nID, pColorMenuButton);
	}
	else if (m_pParentBtn != NULL)
	{
		m_pParentBtn->UpdateColor(color);
		GetParent()->SendMessage(WM_CLOSE);
	}

	else if (m_pParentRibbonBtn != NULL)
	{
		m_pParentRibbonBtn->UpdateColor(color);
		GetParent()->SendMessage(WM_CLOSE);
	}
	else if (m_pWndPropList != NULL)
	{
		m_pWndPropList->UpdateColor(color);
		GetParent()->SendMessage(WM_CLOSE);
	}
	else
	{
		ASSERT(m_nCommandID != 0);

		SetColor(color);

		CObList listButtons;
		if (CMFCToolBar::GetCommandButtons(m_nCommandID, listButtons) > 0)
		{
			for (POSITION pos = listButtons.GetHeadPosition(); pos != NULL;)
			{
				CMFCColorMenuButton* pCurrColorButton = NULL;
				pCurrColorButton = DYNAMIC_DOWNCAST(CMFCColorMenuButton, listButtons.GetNext(pos));
				if (pCurrColorButton != NULL)
				{
					ASSERT_VALID(pCurrColorButton);
					pCurrColorButton->SetColor(color, FALSE);
				}
			}
		}

		CMFCColorMenuButton::SetColorByCmdID(m_nCommandID, color);
		GetOwner()->SendMessage(WM_COMMAND, m_nCommandID);    // send command

		if (AFXGetTopLevelFrame(this) != NULL)
		{
			AFXGetTopLevelFrame(this)->SetFocus();
		}
	}

	return TRUE;
}

BOOL CMFCColorBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID, CPalette* pPalette/* = NULL*/, int nColumns/* = 0*/, int nRowsDockHorz/* = 0*/, int nColDockVert/* = 0*/)
{
	if (m_colors.GetSize() != NULL)
	{
		return CMFCPopupMenuBar::Create(pParentWnd, dwStyle, nID);
	}

	m_nNumColumns = nColumns;
	m_nNumColumnsVert = nColDockVert;
	m_nNumRowsHorz = nRowsDockHorz;

	InitColors(pPalette, m_colors);
	return CMFCPopupMenuBar::Create(pParentWnd, dwStyle, nID);
}

BOOL CMFCColorBar::CreateControl(CWnd* pParentWnd, const CRect& rect, UINT nID, int nColumns, CPalette* pPalette/* = NULL*/)
{
	ASSERT_VALID(pParentWnd);

	EnableLargeIcons(FALSE);

	if (nColumns <= 0)
	{
		const int nColorsCount = (pPalette == NULL) ? 20 : pPalette->GetEntryCount();
		ASSERT(nColorsCount > 0);

		// Optimal fill
		for (nColumns = nColorsCount; nColumns > 0; nColumns--)
		{
			int nCellSize = (rect.Width() - 2 * m_nHorzMargin) / nColumns;
			if (nCellSize == 0)
			{
				continue;
			}

			int nRows = nColorsCount / nColumns;
			if (nRows * nCellSize > rect.Height() - 2 * m_nVertMargin)
			{
				nColumns++;
				break;
			}
		}

		if (nColumns <= 0)
		{
			nColumns = -1;
		}
	}

	if (!Create(pParentWnd, WS_CHILD | WS_VISIBLE | CBRS_ALIGN_TOP, nID, pPalette, nColumns))
	{
		return FALSE;
	}

	SetPaneStyle(GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	CRect rectWnd = rect;
	MoveWindow(rectWnd);
	ContextToSize();

	SetWindowPos(&wndTop, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	SetOwner(pParentWnd);
	SetCommandID(nID);

	// All commands will be routed via this dialog, not via the parent frame:
	SetRouteCommandsViaFrame(FALSE);
	return TRUE;
}

int __stdcall CMFCColorBar::InitColors(CPalette* pPalette, CArray<COLORREF, COLORREF>& arColors)
{
	int nColorsCount = (pPalette == NULL) ? 20 : pPalette->GetEntryCount();
	arColors.SetSize(nColorsCount);

	if (pPalette == NULL)
	{
		// Use system palette:
		pPalette = CPalette::FromHandle((HPALETTE) ::GetStockObject(DEFAULT_PALETTE));
		ASSERT_VALID(pPalette);
	}

	PALETTEENTRY palEntry;
	for (int i = 0; i < nColorsCount; i++)
	{
		pPalette->GetPaletteEntries(i, 1, &palEntry);
		arColors [i] = RGB(palEntry.peRed, palEntry.peGreen, palEntry.peBlue);
	}

	return nColorsCount;
}

void CMFCColorBar::Serialize(CArchive& ar)
{
	CMFCPopupMenuBar::Serialize(ar);

	if (ar.IsLoading())
	{
		ar >> m_nNumColumns;
		ar >> m_nNumRowsHorz;
		ar >> m_nNumColumnsVert;
		ar >> m_ColorAutomatic;
		ar >> m_strAutoColor;
		ar >> m_strOtherColor;
		ar >> m_strDocColors;
		ar >> m_bIsTearOff;
		ar >> m_nCommandID;
		ar >> m_bStdColorDlg;

		int nColors = 0;
		ar >> nColors;

		m_colors.SetSize(nColors);

		for (int i = 0; i < nColors; i ++)
		{
			COLORREF color;
			ar >> color;

			m_colors [i] = color;
		}

		Rebuild();
		AdjustLocations();
	}
	else
	{
		ar << m_nNumColumns;
		ar << m_nNumRowsHorz;
		ar << m_nNumColumnsVert;
		ar << m_ColorAutomatic;
		ar << m_strAutoColor;
		ar << m_strOtherColor;
		ar << m_strDocColors;
		ar << m_bIsTearOff;
		ar << m_nCommandID;
		ar << m_bStdColorDlg;

		ar <<(int) m_colors.GetSize();

		for (int i = 0; i < m_colors.GetSize(); i ++)
		{
			ar << m_colors [i];
		}
	}
}

void CMFCColorBar::ShowCommandMessageString(UINT /*uiCmdId*/)
{
	GetOwner()->SendMessage(WM_SETMESSAGESTRING, m_nCommandID == (UINT) -1 ? AFX_IDS_IDLEMESSAGE :(WPARAM) m_nCommandID);
}

BOOL CMFCColorBar::OpenColorDialog(const COLORREF colorDefault, COLORREF& colorRes)
{
	CMFCColorMenuButton* pColorMenuButton = NULL;

	CMFCPopupMenu* pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, GetParent());
	if (pParentMenu != NULL)
	{
		pColorMenuButton = DYNAMIC_DOWNCAST(CMFCColorMenuButton, pParentMenu->GetParentButton());
		if (pColorMenuButton != NULL)
		{
			return pColorMenuButton->OpenColorDialog(colorDefault, colorRes);
		}
	}

	BOOL bResult = FALSE;

	if (m_bStdColorDlg)
	{
		CColorDialog dlg(colorDefault, CC_FULLOPEN | CC_ANYCOLOR);
		if (dlg.DoModal() == IDOK)
		{
			colorRes = dlg.GetColor();
			bResult = TRUE;
		}
	}
	else
	{
		CMFCColorDialog dlg(colorDefault);
		if (dlg.DoModal() == IDOK)
		{
			colorRes = dlg.GetColor();
			bResult = TRUE;
		}
	}

	return bResult;
}

void CMFCColorBar::EnableAutomaticButton(LPCTSTR lpszLabel, COLORREF colorAutomatic, BOOL bEnable)
{
	m_ColorAutomatic = colorAutomatic;
	m_strAutoColor = (!bEnable || lpszLabel == NULL) ? _T("") : lpszLabel;

	Rebuild();
	AdjustLayout();
}

void CMFCColorBar::EnableOtherButton(LPCTSTR lpszLabel, BOOL bAltColorDlg, BOOL bEnable)
{
	m_bStdColorDlg = !bAltColorDlg;
	m_strOtherColor = (!bEnable || lpszLabel == NULL) ? _T("") : lpszLabel;

	Rebuild();
	AdjustLayout();
}

void CMFCColorBar::SetColor(COLORREF color)
{
	if (m_ColorSelected == color)
	{
		return;
	}

	m_ColorSelected = color;

	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	BOOL bIsOtherColor = !(m_ColorAutomatic != (UINT)-1 && m_ColorSelected == (COLORREF)-1);
	BOOL bWasOtherColor = FALSE;

	m_iHighlighted = -1;
	int iButton = 0;

	for (POSITION pos = m_Buttons.GetHeadPosition(); pos != NULL; iButton++)
	{
		CRect rectButton;

		CMFCToolBarButton* pButton = (CMFCToolBarButton*) m_Buttons.GetNext(pos);
		if (pButton->m_nStyle & TBBS_SEPARATOR)
		{
			continue;
		}

		CMFCToolBarColorButton* pColorButton = DYNAMIC_DOWNCAST(CMFCToolBarColorButton, pButton);
		if (pColorButton == NULL)
		{
			continue;
		}

		ASSERT_VALID(pColorButton);

		if (pColorButton->m_bIsOther || pColorButton->m_bIsLabel)
		{
			continue;
		}

		if (pColorButton->m_bHighlight)
		{
			pColorButton->m_bHighlight = FALSE;
			InvalidateRect(pColorButton->Rect());
		}

		if (pColorButton->m_bIsAutomatic && color == (COLORREF)-1)
		{
			pColorButton->m_bHighlight = TRUE;
			m_iHighlighted = iButton;
			InvalidateRect(pColorButton->Rect());
		}
		else if (pColorButton->m_Color == color)
		{
			pColorButton->m_bHighlight = TRUE;
			m_iHighlighted = iButton;
			InvalidateRect(pColorButton->Rect());
			bIsOtherColor = FALSE;
		}

		if (pColorButton->m_bIsOtherColor)
		{
			pColorButton->m_Color = m_ColorSelected;
			pColorButton->m_bHighlight = TRUE;

			InvalidateRect(pColorButton->Rect());
			bWasOtherColor = !(pColorButton->Rect().IsRectEmpty());
		}
	}

	if (bWasOtherColor != bIsOtherColor)
	{
		AdjustLocations();
		Invalidate();
	}

	UpdateWindow();
}

void CMFCColorBar::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!IsCustomizeMode() || m_bInternal)
	{
		CMFCToolBar::OnMouseMove(nFlags, point);
	}
}

void CMFCColorBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (IsCustomizeMode() && !m_bInternal)
	{
		return;
	}

	if (HitTest(point) == -1)
	{
		CMFCToolBar::OnLButtonDown(nFlags, point);
	}
}

void CMFCColorBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (!IsCustomizeMode() || m_bInternal)
	{
		int iHit = HitTest(point);
		if (iHit >= 0)
		{
			m_iButtonCapture = iHit;
		}

		CMFCToolBar::OnLButtonUp(nFlags, point);
	}
}

void CMFCColorBar::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (!IsCustomizeMode() || m_bInternal)
	{
		CMFCToolBar::OnLButtonDblClk(nFlags, point);
	}
}

BOOL CMFCColorBar::PreTranslateMessage(MSG* pMsg)
{
	if ((m_pParentBtn != NULL || m_pWndPropList != NULL || m_pParentRibbonBtn != NULL)
		&& !m_bInCommand)
	{
		switch (pMsg->message)
		{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
			{
				CRect rect;
				GetClientRect(rect);

				CPoint pt(AFX_GET_X_LPARAM(pMsg->lParam), AFX_GET_Y_LPARAM(pMsg->lParam));
				if (!rect.PtInRect(pt))
				{
					GetParent()->SendMessage(WM_CLOSE);
					return TRUE;
				}
			}
			break;

		case WM_SYSKEYDOWN:
		case WM_CONTEXTMENU:
			GetParent()->SendMessage(WM_CLOSE);
			return TRUE;

		case WM_KEYDOWN:
			if (pMsg->wParam == VK_ESCAPE)
			{
				GetParent()->SendMessage(WM_CLOSE);
				return TRUE;
			}
		}
	}

	return CMFCPopupMenuBar::PreTranslateMessage(pMsg);
}

void CMFCColorBar::OnDestroy()
{
	if (m_pParentBtn != NULL)
	{
		m_pParentBtn->m_pPopup = NULL;
		m_pParentBtn->SetFocus();
	}
	else if (m_pWndPropList != NULL)
	{
		m_pWndPropList->CloseColorPopup();
		m_pWndPropList->SetFocus();
	}

	CMFCPopupMenuBar::OnDestroy();
}

BOOL CMFCColorBar::OnKey(UINT nChar)
{
	POSITION posSel = (m_iHighlighted < 0) ? NULL : m_Buttons.FindIndex(m_iHighlighted);
	CMFCToolBarButton* pSelButton = (posSel == NULL) ? NULL :(CMFCToolBarButton*) m_Buttons.GetAt(posSel);

	switch (nChar)
	{
	case VK_RETURN:
		if (pSelButton != NULL)
		{
			GetOwner()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
			OnSendCommand(pSelButton);
			return TRUE;
		}
		break;
	}

	return CMFCPopupMenuBar::OnKey(nChar);
}

void CMFCColorBar::OnChangeHot(int iHot)
{
	CMFCPopupMenuBar::OnChangeHot(iHot);

	if (m_pParentRibbonBtn != NULL)
	{
		ASSERT_VALID(m_pParentRibbonBtn);

		CMFCToolBarColorButton* pColorButton = DYNAMIC_DOWNCAST(CMFCToolBarColorButton, GetButton(iHot));

		if (pColorButton == NULL || pColorButton->m_bIsOther || pColorButton->m_bIsLabel)
		{
			iHot = -1;
		}

		m_pParentRibbonBtn->NotifyHighlightListItem(iHot);
	}
}

afx_msg LRESULT CMFCColorBar::OnMouseLeave(WPARAM wp,LPARAM lp)
{
	if (m_pParentBtn == NULL && m_pWndPropList == NULL)
	{
		return CMFCToolBar::OnMouseLeave(wp, lp);
	}

	if (m_hookMouseHelp != NULL ||
		(m_bMenuMode && !IsCustomizeMode() && GetDroppedDownMenu() != NULL))
	{
		return 0;
	}

	m_bTracked = FALSE;
	m_ptLastMouse = CPoint(-1, -1);

	if (m_iHighlighted >= 0)
	{
		int iButton = m_iHighlighted;
		m_iHighlighted = -1;

		OnChangeHot(m_iHighlighted);

		InvalidateButton(iButton);
		UpdateWindow(); // immediate feedback

		GetOwner()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);

		if (m_pParentRibbonBtn != NULL)
		{
			ASSERT_VALID(m_pParentRibbonBtn);
			m_pParentRibbonBtn->NotifyHighlightListItem(-1);
		}
	}

	return 0;
}

BOOL __stdcall CMFCColorBar::CreatePalette(const CArray<COLORREF, COLORREF>& arColors, CPalette& palette)
{
	if (palette.GetSafeHandle() != NULL)
	{
		::DeleteObject(palette.Detach());
		ENSURE(palette.GetSafeHandle() == NULL);
	}

	if (afxGlobalData.m_nBitsPerPixel != 8)
	{
		return FALSE;
	}

#define AFX_MAX_COLOURS 100
	int nNumColours = (int) arColors.GetSize();
	if (nNumColours == 0)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	ASSERT(nNumColours <= AFX_MAX_COLOURS);
	if (nNumColours > AFX_MAX_COLOURS)
	{
		nNumColours = AFX_MAX_COLOURS;
	}

	// Create the palette
	struct
	{
		LOGPALETTE    LogPalette;
		PALETTEENTRY  PalEntry [AFX_MAX_COLOURS];
	}
	pal;

	LOGPALETTE* pLogPalette = (LOGPALETTE*) &pal;
	pLogPalette->palVersion    = 0x300;
	pLogPalette->palNumEntries = (WORD) nNumColours;

	for (int i = 0; i < nNumColours; i++)
	{
		pLogPalette->palPalEntry[i].peRed   = GetRValue(arColors[i]);
		pLogPalette->palPalEntry[i].peGreen = GetGValue(arColors[i]);
		pLogPalette->palPalEntry[i].peBlue  = GetBValue(arColors[i]);
		pLogPalette->palPalEntry[i].peFlags = 0;
	}

	palette.CreatePalette(pLogPalette);
	return TRUE;
}

CPalette* CMFCColorBar::SelectPalette(CDC* pDC)
{
	ASSERT_VALID(pDC);

	if (afxGlobalData.m_nBitsPerPixel != 8)
	{
		if (m_Palette.GetSafeHandle() != NULL)
		{
			::DeleteObject(m_Palette.Detach());
		}

		return NULL;
	}

	CPalette* pOldPalette = NULL;

	if (m_pParentBtn != NULL && m_pParentBtn->m_pPalette != NULL)
	{
		pOldPalette = pDC->SelectPalette(m_pParentBtn->m_pPalette, FALSE);
	}
	else
	{
		if (m_Palette.GetSafeHandle() == NULL)
		{
			// Palette not created yet; create it now
			CreatePalette(m_colors, m_Palette);
		}

		pOldPalette = pDC->SelectPalette(&m_Palette, FALSE);
	}

	ENSURE(pOldPalette != NULL);
	pDC->RealizePalette();

	return pOldPalette;
}

void CMFCColorBar::SetVertMargin(int nVertMargin)
{
	ASSERT_VALID(this);

	m_nVertMargin = nVertMargin;
	AdjustLayout();
}

void CMFCColorBar::SetHorzMargin(int nHorzMargin)
{
	ASSERT_VALID(this);

	m_nHorzMargin = nHorzMargin;
	AdjustLayout();
}

COLORREF CMFCColorBar::GetHighlightedColor() const
{
	ASSERT_VALID(this);

	if (m_iHot < 0)
	{
		return(COLORREF)-1;
	}

	CMFCToolBarColorButton* pColorButton = DYNAMIC_DOWNCAST(CMFCToolBarColorButton, GetButton(m_iHot));

	if (pColorButton == NULL)
	{
		return(COLORREF)-1;
	}

	return pColorButton->m_Color;
}



