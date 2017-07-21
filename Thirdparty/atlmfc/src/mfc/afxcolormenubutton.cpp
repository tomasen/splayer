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

#include "afxmenutearoffmanager.h"
#include "afxcontrolbarutil.h"
#include "afxmenuimages.h"
#include "afxpopupmenubar.h"
#include "afxcolormenubutton.h"
#include "afxcolordialog.h"
#include "afxcolorbar.h"
#include "afxsettingsstore.h"
#include "afxcolorpopupmenu.h"
#include "afxglobals.h"
#include "afxvisualmanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const int SEPARATOR_SIZE = 2;

CMap<UINT,UINT,COLORREF, COLORREF> CMFCColorMenuButton::m_ColorsByID;

UINT AFX_WM_GETDOCUMENTCOLORS = ::RegisterWindowMessage(_T("TOOLBAR__GETDOCUMENTCOLORS"));

IMPLEMENT_SERIAL(CMFCColorMenuButton, CMFCToolBarMenuButton, VERSIONABLE_SCHEMA | 1)

// Construction/Destruction
CMFCColorMenuButton::CMFCColorMenuButton()
{
	Initialize();
}

CMFCColorMenuButton::CMFCColorMenuButton(UINT uiCmdID, LPCTSTR lpszText, CPalette* pPalette) :
	CMFCToolBarMenuButton(uiCmdID, NULL, afxCommandManager->GetCmdImage(uiCmdID, FALSE), lpszText)
{
	Initialize();

	CMFCColorBar::InitColors(pPalette, m_Colors);
	m_Color = GetColorByCmdID(uiCmdID);
}

void CMFCColorMenuButton::Initialize()
{
	m_Color = (COLORREF) -1; // Default(automatic) color
	m_colorAutomatic = 0;
	m_nColumns = -1;
	m_nVertDockColumns = -1;
	m_nHorzDockRows = -1;
	m_bIsAutomaticButton = FALSE;
	m_bIsOtherButton = FALSE;
	m_bIsDocumentColors = FALSE;
	m_bStdColorDlg = FALSE;
}

CMFCColorMenuButton::~CMFCColorMenuButton()
{
}

void CMFCColorMenuButton::EnableAutomaticButton(LPCTSTR lpszLabel, COLORREF colorAutomatic, BOOL bEnable)
{
	m_bIsAutomaticButton = bEnable;
	if (bEnable)
	{
		ENSURE(lpszLabel != NULL);
		m_strAutomaticButtonLabel = lpszLabel;

		m_colorAutomatic = colorAutomatic;
	}
}

void CMFCColorMenuButton::EnableOtherButton(LPCTSTR lpszLabel, BOOL bAltColorDlg, BOOL bEnable)
{
	m_bIsOtherButton = bEnable;

	if (bEnable)
	{
		ENSURE(lpszLabel != NULL);
		m_strOtherButtonLabel = lpszLabel;

		m_bStdColorDlg = !bAltColorDlg;
	}
}

void CMFCColorMenuButton::EnableDocumentColors(LPCTSTR lpszLabel, BOOL bEnable)
{
	m_bIsDocumentColors = bEnable;
	if (bEnable)
	{
		ENSURE(lpszLabel != NULL);
		m_strDocumentColorsLabel = lpszLabel;
	}
}

void CMFCColorMenuButton::EnableTearOff(UINT uiID, int nVertDockColumns, int nHorzDockRows)
{
	if (g_pTearOffMenuManager != NULL && g_pTearOffMenuManager->IsDynamicID(uiID))
	{
		ASSERT(FALSE); // SHould be static ID!
		uiID = 0;
	}

	m_uiTearOffBarID = uiID;

	m_nVertDockColumns = nVertDockColumns;
	m_nHorzDockRows = nHorzDockRows;
}

void CMFCColorMenuButton::OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages, BOOL bHorz, BOOL bCustomizeMode, BOOL bHighlight, BOOL bDrawBorder, BOOL bGrayDisabledButtons)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	CMFCToolBarMenuButton::OnDraw(pDC, rect, pImages, bHorz, bCustomizeMode, bHighlight, bDrawBorder, bGrayDisabledButtons);

	if (!IsDrawImage() || pImages == NULL)
	{
		return;
	}

	CPalette* pOldPalette = NULL;
	if (afxGlobalData.m_nBitsPerPixel == 8) // 256 colors
	{
		if (m_Palette.GetSafeHandle() == NULL)
		{
			// Palette not created yet; create it now
			CMFCColorBar::CreatePalette(m_Colors, m_Palette);
		}

		ENSURE(m_Palette.GetSafeHandle() != NULL);

		pOldPalette = pDC->SelectPalette(&m_Palette, FALSE);
		pDC->RealizePalette();
	}
	else if (m_Palette.GetSafeHandle() != NULL)
	{
		::DeleteObject(m_Palette.Detach());
		ENSURE(m_Palette.GetSafeHandle() == NULL);
	}

	ENSURE(pImages != NULL);
	CRect rectColor = pImages->GetLastImageRect();
	const int nColorBoxSize = CMFCToolBar::IsLargeIcons() && !m_bMenuMode ? 10 : 5;

	rectColor.top = rectColor.bottom - nColorBoxSize;
	rectColor.OffsetRect(0, 1);

	// Draw color bar:
	BOOL bDrawImageShadow = bHighlight && !bCustomizeMode && CMFCVisualManager::GetInstance()->IsShadowHighlightedImage() && !afxGlobalData.IsHighContrastMode() &&
		((m_nStyle & TBBS_PRESSED) == 0) && ((m_nStyle & TBBS_CHECKED) == 0) && ((m_nStyle & TBBS_DISABLED) == 0);

	if (bDrawImageShadow)
	{
		CBrush brShadow(afxGlobalData.clrBarShadow);
		pDC->FillRect(rectColor, &brShadow);
		rectColor.OffsetRect(-1, -1);
	}

	COLORREF color = (m_nStyle & TBBS_DISABLED) ? afxGlobalData.clrBarShadow : (m_Color == (COLORREF)-1 ? m_colorAutomatic : m_Color);

	CBrush br(PALETTERGB( GetRValue(color), GetGValue(color), GetBValue(color)));

	CBrush* pOldBrush = pDC->SelectObject(&br);
	CPen* pOldPen = (CPen*) pDC->SelectStockObject(NULL_PEN);

	pDC->Rectangle(&rectColor);

	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);

	if (CMFCVisualManager::GetInstance()->IsMenuFlatLook())
	{
		if (color == afxGlobalData.clrBarFace)
		{
			pDC->Draw3dRect(rectColor, afxGlobalData.clrBarDkShadow, afxGlobalData.clrBarDkShadow);
		}
	}
	else
	{
		pDC->Draw3dRect(rectColor, afxGlobalData.clrBarShadow, afxGlobalData.clrBarLight);
	}

	if (pOldPalette != NULL)
	{
		pDC->SelectPalette(pOldPalette, FALSE);
	}
}

int CMFCColorMenuButton::OnDrawOnCustomizeList(CDC* pDC, const CRect& rect, BOOL bSelected)
{
	int nID = m_nID;
	m_nID = 0; // Force draw right arrow

	CRect rectColor = rect;
	rectColor.DeflateRect(1, 0);

	int iRes = CMFCToolBarMenuButton::OnDrawOnCustomizeList(pDC, rect, bSelected);

	m_nID = nID;

	return iRes;
}

void CMFCColorMenuButton::SetColor(COLORREF clr, BOOL bNotify)
{
	m_Color = clr;
	m_ColorsByID.SetAt(m_nID, m_Color);

	if (m_pWndParent->GetSafeHwnd() != NULL)
	{
		m_pWndParent->InvalidateRect(m_rect);
	}

	if (bNotify)
	{
		CObList listButtons;
		if (CMFCToolBar::GetCommandButtons(m_nID, listButtons) > 0)
		{
			for (POSITION pos = listButtons.GetHeadPosition(); pos != NULL;)
			{
				CMFCColorMenuButton* pOther = DYNAMIC_DOWNCAST(CMFCColorMenuButton, listButtons.GetNext(pos));

				if (pOther != NULL && pOther != this)
				{
					pOther->SetColor(clr, FALSE);
				}
			}
		}

		const CObList& lstToolBars = CMFCToolBar::GetAllToolbars();
		for (POSITION pos = lstToolBars.GetHeadPosition(); pos != NULL;)
		{
			CMFCColorBar* pColorBar = DYNAMIC_DOWNCAST(CMFCColorBar, lstToolBars.GetNext(pos));
			if (pColorBar != NULL && pColorBar->m_nCommandID == m_nID)
			{
				pColorBar->SetColor(clr);
			}
		}
	}
}

void CMFCColorMenuButton::OnChangeParentWnd(CWnd* pWndParent)
{
	CMFCToolBarButton::OnChangeParentWnd(pWndParent);

	if (pWndParent != NULL)
	{
		if (pWndParent->IsKindOf(RUNTIME_CLASS(CMFCMenuBar)))
		{
			m_bText = TRUE;
		}

		if (pWndParent->IsKindOf(RUNTIME_CLASS(CMFCPopupMenuBar)))
		{
			m_bMenuMode = TRUE;
			m_bText = TRUE;
		}
		else
		{
			m_bMenuMode = FALSE;
		}
	}

	m_bDrawDownArrow = TRUE;
	m_pWndParent = pWndParent;
}

void CMFCColorMenuButton::Serialize(CArchive& ar)
{
	CMFCToolBarMenuButton::Serialize(ar);

	if (ar.IsLoading())
	{
		int nColorsCount;
		ar >> nColorsCount;

		m_Colors.SetSize(nColorsCount);

		for (int i = 0; i < nColorsCount; i++)
		{
			COLORREF color;
			ar >> color;

			m_Colors [i] = color;
		}

		ar >> m_nColumns;
		ar >> m_nVertDockColumns;
		ar >> m_nHorzDockRows;

		ar >> m_bIsAutomaticButton;
		ar >> m_bIsOtherButton;
		ar >> m_bIsDocumentColors;

		ar >> m_strAutomaticButtonLabel;
		ar >> m_strOtherButtonLabel;
		ar >> m_strDocumentColorsLabel;

		ar >> m_colorAutomatic;
		ar >> m_bStdColorDlg;

		// Synchromize color with another buttons with the same ID:
		CObList listButtons;
		if (CMFCToolBar::GetCommandButtons(m_nID, listButtons) > 0)
		{
			for (POSITION pos = listButtons.GetHeadPosition(); pos != NULL;)
			{
				CMFCColorMenuButton* pOther = DYNAMIC_DOWNCAST(CMFCColorMenuButton, listButtons.GetNext(pos));
				if (pOther != NULL && pOther != this && pOther->m_Color != (COLORREF) -1)
				{
					m_Color = pOther->m_Color;
				}
			}
		}
	}
	else
	{
		ar <<(int) m_Colors.GetSize();
		for (int i = 0; i < m_Colors.GetSize(); i++)
		{
			ar << m_Colors [i];
		}

		ar << m_nColumns;

		ar << m_nVertDockColumns;
		ar << m_nHorzDockRows;

		ar << m_bIsAutomaticButton;
		ar << m_bIsOtherButton;
		ar << m_bIsDocumentColors;

		ar << m_strAutomaticButtonLabel;
		ar << m_strOtherButtonLabel;
		ar << m_strDocumentColorsLabel;

		ar << m_colorAutomatic;
		ar << m_bStdColorDlg;
	}
}

void CMFCColorMenuButton::CopyFrom(const CMFCToolBarButton& s)
{
	CMFCToolBarMenuButton::CopyFrom(s);

	const CMFCColorMenuButton& src = (const CMFCColorMenuButton&) s;

	m_Color = src.m_Color;
	m_ColorsByID.SetAt(m_nID, m_Color); // Just to be happy :-)

	m_Colors.SetSize(src.m_Colors.GetSize());

	for (int i = 0; i < m_Colors.GetSize(); i++)
	{
		m_Colors [i] = src.m_Colors [i];
	}

	m_bIsAutomaticButton = src.m_bIsAutomaticButton;
	m_colorAutomatic = src.m_colorAutomatic;
	m_bIsOtherButton = src.m_bIsOtherButton;
	m_bIsDocumentColors = src.m_bIsDocumentColors;

	m_strAutomaticButtonLabel = src.m_strAutomaticButtonLabel;
	m_strOtherButtonLabel = src.m_strOtherButtonLabel;
	m_strDocumentColorsLabel =  src.m_strDocumentColorsLabel;

	m_nColumns = src.m_nColumns;
	m_nVertDockColumns = src.m_nVertDockColumns;
	m_nHorzDockRows = src.m_nHorzDockRows;

	m_bStdColorDlg = src.m_bStdColorDlg;
}

BOOL CMFCColorMenuButton::OpenColorDialog(const COLORREF colorDefault, COLORREF& colorRes)
{
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

CMFCPopupMenu* CMFCColorMenuButton::CreatePopupMenu()
{
	CList<COLORREF,COLORREF> lstDocColors;
	if (m_bIsDocumentColors && m_pWndParent != NULL)
	{
		CFrameWnd* pOwner = AFXGetTopLevelFrame(m_pWndParent);
		ASSERT_VALID(pOwner);

		// Fill document colors list:
		pOwner->SendMessage(AFX_WM_GETDOCUMENTCOLORS, (WPARAM) m_nID, (LPARAM) &lstDocColors);
	}

	return new CMFCColorPopupMenu(m_Colors, m_Color, (m_bIsAutomaticButton ?(LPCTSTR) m_strAutomaticButtonLabel : NULL), (m_bIsOtherButton ?(LPCTSTR) m_strOtherButtonLabel : NULL),
		(m_bIsDocumentColors ?(LPCTSTR) m_strDocumentColorsLabel : NULL), lstDocColors, m_nColumns, m_nHorzDockRows, m_nVertDockColumns, m_colorAutomatic, m_nID, m_bStdColorDlg);
}

void __stdcall CMFCColorMenuButton::SetColorName(COLORREF color, const CString& strName)
{
	CMFCColorBar::m_ColorNames.SetAt(color, strName);
}

COLORREF __stdcall CMFCColorMenuButton::GetColorByCmdID(UINT uiCmdID)
{
	COLORREF color = (COLORREF)-1;
	m_ColorsByID.Lookup(uiCmdID, color);

	return color;
}


