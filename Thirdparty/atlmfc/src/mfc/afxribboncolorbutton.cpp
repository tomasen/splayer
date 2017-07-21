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
#include "afxribbonbar.h"
#include "afxribboncategory.h"
#include "afxribboncolorbutton.h"
#include "afxribbonpanelmenu.h"
#include "afxdrawmanager.h"
#include "afxribbonminitoolbar.h"
#include "afxcolordialog.h"
#include "afxribbonres.h"
#include "afxglobals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int nMenuButtonAuto = 1;
const int nMenuButtonOther = 2;

class CMFCRibbonColorMenuButton : public CMFCRibbonButton
{
	DECLARE_DYNCREATE(CMFCRibbonColorMenuButton)

	CMFCRibbonColorMenuButton(int nType = 0, CMFCRibbonColorButton* pColorButton = NULL, LPCTSTR lpszLabel = NULL, BOOL bIsChecked = FALSE) :
	CMFCRibbonButton(0, lpszLabel)
	{
		m_bIsChecked = bIsChecked;
		m_pColorButton = pColorButton;
		m_nType = nType;
	}

	virtual void CopyFrom(const CMFCRibbonBaseElement& s)
	{
		ASSERT_VALID(this);
		CMFCRibbonButton::CopyFrom(s);

		m_bIsChecked = s.IsChecked();

		CMFCRibbonColorMenuButton& src = (CMFCRibbonColorMenuButton&) s;
		m_pColorButton = src.m_pColorButton;
		m_nType = src.m_nType;
	}

	virtual void OnDraw(CDC* pDC)
	{
		ASSERT_VALID(this);
		ASSERT_VALID(pDC);
		ASSERT_VALID(m_pColorButton);

		if (m_rect.IsRectEmpty())
		{
			return;
		}

		const int cxImageBar = CMFCToolBar::GetMenuImageSize().cx + 2 * CMFCVisualManager::GetInstance()->GetMenuImageMargin() + 2;

		COLORREF clrText = OnFillBackground(pDC);
		COLORREF clrTextOld = (COLORREF)-1;

		if (m_bIsDisabled)
		{
			clrTextOld = pDC->SetTextColor(clrText == (COLORREF)-1 ? CMFCVisualManager::GetInstance()->GetToolbarDisabledTextColor() : clrText);
		}
		else if (clrText != (COLORREF)-1)
		{
			clrTextOld = pDC->SetTextColor(clrText);
		}

		CRect rectText = m_rect;
		rectText.left += cxImageBar + AFX_TEXT_MARGIN;
		rectText.DeflateRect(m_szMargin.cx, m_szMargin.cx);

		pDC->DrawText(m_strText, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);

		if (clrTextOld != (COLORREF)-1)
		{
			pDC->SetTextColor(clrTextOld);
		}

		if (m_nType == nMenuButtonOther)
		{
			CRect rectImage = m_rect;
			rectImage.right = rectImage.left + cxImageBar;

			const int nIconSize = 16;

			if (afxGlobalData.m_hiconColors == NULL)
			{
				LPCTSTR lpszResourceName = MAKEINTRESOURCE(IDI_AFXRES_COLORS);
				ENSURE(lpszResourceName != NULL);

				afxGlobalData.m_hiconColors = (HICON) ::LoadImage(
					AfxFindResourceHandle (lpszResourceName, RT_ICON), 
					lpszResourceName, IMAGE_ICON, 16, 16, LR_SHARED);
			}

			::DrawIconEx(pDC->GetSafeHdc(), rectImage.left +(rectImage.Width() - nIconSize) / 2,
				rectImage.top +(rectImage.Height() - nIconSize) / 2, afxGlobalData.m_hiconColors, nIconSize, nIconSize, 0, NULL, DI_NORMAL);
		}
		else if (m_nType == nMenuButtonAuto)
		{
			CRect rectColorBox = m_rect;
			rectColorBox.right = rectColorBox.left + cxImageBar;
			rectColorBox.DeflateRect(2, 2);

			int nBoxSize = min(rectColorBox.Width(), rectColorBox.Height());

			rectColorBox = CRect(CPoint(rectColorBox.left +(rectColorBox.Width() - nBoxSize) / 2,
				rectColorBox.top +(rectColorBox.Height() - nBoxSize) / 2), CSize(nBoxSize, nBoxSize));

			m_pColorButton->OnDrawPaletteIcon(pDC, rectColorBox, -1, NULL, (COLORREF)-1);
		}
	}

	CMFCRibbonColorButton* m_pColorButton;
	int m_nType;
};

IMPLEMENT_DYNCREATE(CMFCRibbonColorMenuButton, CMFCRibbonButton)

IMPLEMENT_DYNCREATE(CMFCRibbonColorButton, CMFCRibbonGallery)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFCRibbonColorButton::CMFCRibbonColorButton()
{
	m_Color = (COLORREF)-1;
	CommonInit();
}

CMFCRibbonColorButton::CMFCRibbonColorButton(UINT nID, LPCTSTR lpszText, int nSmallImageIndex, COLORREF color) :
CMFCRibbonGallery(nID, lpszText, nSmallImageIndex, -1)
{
	m_Color = color;
	CommonInit();
}

CMFCRibbonColorButton::CMFCRibbonColorButton(UINT nID, LPCTSTR lpszText, BOOL bSimpleButtonLook, int nSmallImageIndex, int nLargeImageIndex, COLORREF color) :
	CMFCRibbonGallery(nID, lpszText, nSmallImageIndex, nLargeImageIndex)
{
	CommonInit();

	m_Color = color;
	m_bSimpleButtonLook = bSimpleButtonLook;
}

void CMFCRibbonColorButton::CommonInit()
{
	m_ColorAutomatic = RGB(0, 0, 0);
	m_ColorHighlighted = (COLORREF)-1;
	m_bIsAutomaticButton = FALSE;
	m_bIsAutomaticButtonOnTop = TRUE;
	m_bIsAutomaticButtonBorder = FALSE;
	m_bIsOtherButton = FALSE;
	m_bIsDefaultCommand = TRUE;
	m_bSimpleButtonLook = FALSE;

	m_bIsOwnerDraw = TRUE;
	m_bDefaultButtonStyle = FALSE;

	SetButtonMode();

	m_bHasGroups = FALSE;

	// Add default colors:
	SetPalette(NULL);
	m_nIconsInRow = 5;

	m_pOtherButton = NULL;
	m_pAutoButton = NULL;

	m_bSmallIcons = TRUE;

	SetColorBoxSize(CSize(22, 22));
}

CMFCRibbonColorButton::~CMFCRibbonColorButton()
{
}

void CMFCRibbonColorButton::DrawImage(CDC* pDC, RibbonImageType type, CRect rectImage)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if (m_bSimpleButtonLook)
	{
		CMFCRibbonButton::DrawImage(pDC, type, rectImage);
		return;
	}

	CRect rectColor = rectImage;

	int nColorHeight = 5;

	if (afxGlobalData.GetRibbonImageScale() != 1.)
	{
		nColorHeight = (int)(.5 + afxGlobalData.GetRibbonImageScale() * nColorHeight);
	}

	rectColor.top = rectColor.bottom - nColorHeight + 1;

	if ((m_rect.Width() % 2) == 0)
	{
		rectColor.left++;
		rectColor.right++;
	}

	rectImage.OffsetRect(0, -1);
	CMFCRibbonButton::DrawImage(pDC, type, rectImage);

	COLORREF color = (IsDisabled()) ? afxGlobalData.clrBarShadow : (m_Color == (COLORREF)-1 ? m_ColorAutomatic : m_Color);
	COLORREF clrBorder = (COLORREF)-1;

	if (m_bIsAutomaticButtonBorder && m_Color == (COLORREF)-1)
	{
		clrBorder = RGB(197, 197, 197);
	}

	if (CMFCToolBarImages::m_bIsDrawOnGlass)
	{
		CDrawingManager dm(*pDC);

		rectColor.DeflateRect(1, 1);
		dm.DrawRect(rectColor, color, clrBorder);
	}
	else
	{
		CBrush br(PALETTERGB( GetRValue(color), GetGValue(color), GetBValue(color)));

		pDC->FillRect(rectColor, &br);

		if (clrBorder != (COLORREF)-1)
		{
			pDC->Draw3dRect(rectColor, clrBorder, clrBorder);
		}
	}
}

void CMFCRibbonColorButton::UpdateColor(COLORREF color)
{
	ASSERT_VALID(this);

	if (m_Color == color)
	{
		return;
	}

	m_Color = color;

	CMFCRibbonBar* pRibbonBar = GetTopLevelRibbonBar();
	if (pRibbonBar != NULL)
	{
		ASSERT_VALID(pRibbonBar);

		CArray<CMFCRibbonBaseElement*, CMFCRibbonBaseElement*> arButtons;
		pRibbonBar->GetElementsByID(m_nID, arButtons);

		for (int i = 0; i < arButtons.GetSize(); i++)
		{
			CMFCRibbonColorButton* pOther = DYNAMIC_DOWNCAST(CMFCRibbonColorButton, arButtons [i]);
			if (pOther != NULL && pOther != this)
			{
				ASSERT_VALID(pOther);

				pOther->m_Color = color;
				pOther->Redraw();
			}
		}
	}

	if (m_pParentMenu != NULL)
	{
		ASSERT_VALID(m_pParentMenu);

		if (m_pParentMenu->IsRibbonMiniToolBar())
		{
			CMFCRibbonMiniToolBar* pFloaty = DYNAMIC_DOWNCAST(CMFCRibbonMiniToolBar, m_pParentMenu->GetParent());
			if (pFloaty != NULL && !pFloaty->IsContextMenuMode())
			{
				return;
			}
		}

		CFrameWnd* pParentFrame = AFXGetParentFrame(m_pParentMenu);
		ASSERT_VALID(pParentFrame);

		pParentFrame->PostMessage(WM_CLOSE);
	}
	else
	{
		Redraw();
	}
}

BOOL CMFCRibbonColorButton::SetACCData(CWnd* pParent, CAccessibilityData& data)
{
	CMFCRibbonGallery::SetACCData(pParent, data);

	CString strValue;
	strValue.Format(_T("RGB(%d, %d, %d)"), GetRValue(m_Color), GetGValue(m_Color), GetBValue(m_Color));
	data.m_strAccValue = strValue;

	return TRUE;
}

void CMFCRibbonColorButton::EnableAutomaticButton(LPCTSTR lpszLabel, COLORREF colorAutomatic, BOOL bEnable, LPCTSTR lpszToolTip, BOOL bOnTop, BOOL bDrawBorder)
{
	ASSERT_VALID(this);

	m_strAutomaticButtonLabel = (bEnable && lpszLabel == NULL) ? _T("") : lpszLabel;
	m_strAutomaticButtonToolTip = (lpszToolTip == NULL) ? m_strAutomaticButtonLabel : lpszToolTip;
	m_strAutomaticButtonToolTip.Remove(_T('&'));
	m_ColorAutomatic = colorAutomatic;
	m_bIsAutomaticButton = bEnable;
	m_bIsAutomaticButtonOnTop = bOnTop;
	m_bIsAutomaticButtonBorder = bDrawBorder;
}

void CMFCRibbonColorButton::EnableOtherButton(LPCTSTR lpszLabel, LPCTSTR lpszToolTip)
{
	ASSERT_VALID(this);

	m_bIsOtherButton = (lpszLabel != NULL);
	m_strOtherButtonLabel = (lpszLabel == NULL) ? _T("") : lpszLabel;
	m_strOtherButtonToolTip = (lpszToolTip == NULL) ? m_strOtherButtonLabel : lpszToolTip;
	m_strOtherButtonToolTip.Remove(_T('&'));
}

void CMFCRibbonColorButton::CopyFrom(const CMFCRibbonBaseElement& s)
{
	ASSERT_VALID(this);

	int i = 0;

	CMFCRibbonGallery::CopyFrom(s);

	if (!s.IsKindOf(RUNTIME_CLASS(CMFCRibbonColorButton)))
	{
		return;
	}

	CMFCRibbonColorButton& src = (CMFCRibbonColorButton&) s;

	m_Color = src.m_Color;
	m_ColorAutomatic = src.m_ColorAutomatic;

	m_Colors.RemoveAll();
	m_DocumentColors.RemoveAll();
	m_arContColumnsRanges.RemoveAll();

	for (i = 0; i < src.m_Colors.GetSize(); i++)
	{
		m_Colors.Add(src.m_Colors [i]);
	}

	for (i = 0; i < src.m_DocumentColors.GetSize(); i++)
	{
		m_DocumentColors.Add(src.m_DocumentColors [i]);
	}

	for (i = 0; i < src.m_arContColumnsRanges.GetSize(); i++)
	{
		m_arContColumnsRanges.Add(src.m_arContColumnsRanges [i]);
	}

	m_bIsAutomaticButton = src.m_bIsAutomaticButton;
	m_bIsAutomaticButtonOnTop = src.m_bIsAutomaticButtonOnTop;
	m_bIsAutomaticButtonBorder = src.m_bIsAutomaticButtonBorder;
	m_bIsOtherButton = src.m_bIsOtherButton;

	m_strAutomaticButtonLabel = src.m_strAutomaticButtonLabel;
	m_strAutomaticButtonToolTip = src.m_strAutomaticButtonToolTip;
	m_strOtherButtonLabel = src.m_strOtherButtonLabel;
	m_strOtherButtonToolTip = src.m_strOtherButtonToolTip;
	m_strDocumentColorsLabel = src.m_strDocumentColorsLabel;

	m_bHasGroups = src.m_bHasGroups;

	m_sizeBox = src.m_sizeBox;
	m_bSimpleButtonLook = src.m_bSimpleButtonLook;
	m_imagesPalette.SetImageSize (src.m_imagesPalette.GetImageSize ());
}

void CMFCRibbonColorButton::SetPalette(CPalette* pPalette)
{
	ASSERT_VALID(this);

	if (m_bHasGroups)
	{
		// You cannot call this method when the color gallery has groups!
		ASSERT(FALSE);
		return;
	}

	if (pPalette != NULL)
	{
		// For backward compatibility
		SetColorBoxSize(CSize(16, 16));
	}

	m_Colors.RemoveAll();
	CMFCColorBar::InitColors(pPalette, m_Colors);
}

COLORREF CMFCRibbonColorButton::GetHighlightedColor() const
{
	ASSERT_VALID(this);
	return m_ColorHighlighted;
}

void CMFCRibbonColorButton::SetDocumentColors(LPCTSTR lpszLabel, CList<COLORREF,COLORREF>& lstColors)
{
	ASSERT_VALID(this);

	m_strDocumentColorsLabel = (lpszLabel == NULL) ? _T(" ") : lpszLabel;

	m_DocumentColors.RemoveAll();

	for (POSITION pos = lstColors.GetHeadPosition(); pos != NULL;)
	{
		m_DocumentColors.Add(lstColors.GetNext(pos));
	}
}

void CMFCRibbonColorButton::OnDrawPaletteIcon(CDC* pDC, CRect rectIcon, int nIconIndex, CMFCRibbonGalleryIcon* pIcon, COLORREF /*clrText*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	COLORREF color = (COLORREF)-1;

	BOOL bIsHighlighted = FALSE;
	BOOL bIsChecked = FALSE;
	BOOL bIsContColumn = FALSE;
	BOOL bDrawTopEdge = TRUE;
	BOOL bDrawBottomEdge = TRUE;

	int nYMargin = m_arContColumnsRanges.GetSize() > 0 ? 0 : 2;

	if (pIcon == NULL)
	{
		color = m_ColorAutomatic;
		bIsChecked = (m_Color == (COLORREF)-1);
		nYMargin = 2;
	}
	else
	{
		ASSERT_VALID(pIcon);

		color = GetColorByIndex(nIconIndex);

		bIsChecked = (m_Color == color);
		bIsHighlighted = pIcon->IsHighlighted();

		if (nIconIndex < m_Colors.GetSize())
		{
			for (int i = 0; i < m_arContColumnsRanges.GetSize(); i++)
			{
				int nIndex1 = LOWORD(m_arContColumnsRanges [i]);
				int nIndex2 = HIWORD(m_arContColumnsRanges [i]);

				if (nIconIndex >= nIndex1 && nIconIndex <= nIndex2)
				{
					bIsContColumn = TRUE;
					break;
				}
			}
		}

		if (bIsContColumn)
		{
			nYMargin = 0;

			bDrawTopEdge = bDrawBottomEdge = FALSE;

			if (pIcon->IsFirstInColumn())
			{
				rectIcon.top++;
				bDrawTopEdge = TRUE;
			}

			if (pIcon->IsLastInColumn())
			{
				rectIcon.bottom--;
				bDrawBottomEdge = TRUE;
			}
		}
		else if (m_arContColumnsRanges.GetSize() > 0)
		{
			rectIcon.bottom--;
		}
	}

	rectIcon.DeflateRect(2, nYMargin);

	CMFCVisualManager::GetInstance()->OnDrawRibbonColorPaletteBox(pDC, this, pIcon, color, rectIcon,
		bDrawTopEdge, bDrawBottomEdge, bIsHighlighted, bIsChecked, FALSE);
}

void CMFCRibbonColorButton::OnShowPopupMenu()
{
	ASSERT_VALID(this);

	m_ColorHighlighted = (COLORREF)-1;

	CMFCRibbonBaseElement::OnShowPopupMenu(); // For AFX_WM_ON_BEFORE_SHOW_RIBBON_ITEM_MENU notification

	for (int i = 0; i < m_arSubItems.GetSize();)
	{
		ASSERT_VALID(m_arSubItems [i]);

		CMFCRibbonColorMenuButton* pMyButton = DYNAMIC_DOWNCAST(CMFCRibbonColorMenuButton, m_arSubItems [i]);

		if (pMyButton != NULL)
		{
			ASSERT_VALID(pMyButton);
			delete pMyButton;
			m_arSubItems.RemoveAt(i);
		}
		else
		{
			i++;
		}
	}

	if (!m_bHasGroups)
	{
		Clear();
		AddGroup(_T(""), (int) m_Colors.GetSize());
	}

	const int nDocColors = (int) m_DocumentColors.GetSize();
	if (nDocColors > 0)
	{
		// Add temporary group
		AddGroup(m_strDocumentColorsLabel, nDocColors);
	}

	if (m_bIsOtherButton)
	{
		m_pOtherButton = new CMFCRibbonColorMenuButton(nMenuButtonOther, this, m_strOtherButtonLabel);
		m_pOtherButton->SetToolTipText(m_strOtherButtonToolTip);

		AddSubItem(m_pOtherButton, 0);
	}

	if (m_bIsAutomaticButton)
	{
		m_pAutoButton = new CMFCRibbonColorMenuButton(nMenuButtonAuto, this, m_strAutomaticButtonLabel, m_Color == (COLORREF)-1);
		m_pAutoButton->SetToolTipText(m_strAutomaticButtonToolTip);

		AddSubItem(m_pAutoButton, 0, m_bIsAutomaticButtonOnTop); // Add to top
	}

	if (m_bHasGroups && m_arContColumnsRanges.GetSize() > 0)
	{
		m_imagesPalette.SetImageSize(CSize(m_sizeBox.cx, m_sizeBox.cy - 3));
	}
	else
	{
		m_imagesPalette.SetImageSize(m_sizeBox);
	}

	CMFCRibbonGallery::OnShowPopupMenu();

	if (nDocColors > 0)
	{
		// Remove "Document Colors" group:
		m_arGroupNames.RemoveAt(m_arGroupNames.GetSize() - 1);
		m_arGroupLen.RemoveAt(m_arGroupLen.GetSize() - 1);

		m_nIcons -= nDocColors;
	}
}

BOOL CMFCRibbonColorButton::OnClickPaletteSubItem(CMFCRibbonButton* pButton, CMFCRibbonPanelMenuBar* pMenuBar)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pButton);

	if (pButton->GetOriginal() == m_pOtherButton && m_pOtherButton != NULL)
	{
		CMFCRibbonColorButton* pColorButton = this;
		if (GetOriginal() != NULL)
		{
			pColorButton = (CMFCRibbonColorButton*) GetOriginal();
		}

		ASSERT_VALID(pColorButton);

		ClosePopupMenu();

		CMFCColorDialog dlg(m_Color);
		if (dlg.DoModal() == IDOK)
		{
			pColorButton->UpdateColor(dlg.GetColor());
			NotifyCommand();
		}

		return TRUE;
	}

	if (pButton->GetOriginal() == m_pAutoButton && m_pAutoButton != NULL)
	{
		UpdateColor((COLORREF)-1);
		NotifyCommand();
	}

	return CMFCRibbonGallery::OnClickPaletteSubItem(pButton, pMenuBar);
}

void CMFCRibbonColorButton::OnClickPaletteIcon(CMFCRibbonGalleryIcon* pIcon)
{
	ASSERT_VALID(this);

	COLORREF color = GetColorByIndex(pIcon->GetIndex());

	if (color != (COLORREF)-1)
	{
		UpdateColor(color);
	}

	CMFCRibbonGallery::OnClickPaletteIcon(pIcon);
}

COLORREF CMFCRibbonColorButton::GetColorByIndex(int nIconIndex) const
{
	if (nIconIndex < 0)
	{
		return(COLORREF)-1;
	}

	if (nIconIndex < m_Colors.GetSize())
	{
		return m_Colors [nIconIndex];
	}

	nIconIndex -= (int) m_Colors.GetSize();

	if (nIconIndex < m_DocumentColors.GetSize())
	{
		return m_DocumentColors [nIconIndex];
	}

	return(COLORREF)-1;
}

void CMFCRibbonColorButton::AddColorsGroup(LPCTSTR lpszName, const CList<COLORREF,COLORREF>& lstColors, BOOL bContiguousColumns)
{
	ASSERT_VALID(this);

	if (lstColors.IsEmpty())
	{
		return;
	}

	if (!m_bHasGroups)
	{
		m_Colors.RemoveAll();
		m_arContColumnsRanges.RemoveAll();
		Clear();
	}

	int nCurrSize = (int) m_Colors.GetSize();

	for (POSITION pos = lstColors.GetHeadPosition(); pos != NULL;)
	{
		m_Colors.Add(lstColors.GetNext(pos));
	}

	AddGroup(lpszName == NULL ? _T("") : lpszName, (int) lstColors.GetCount());

	if (bContiguousColumns)
	{
		m_arContColumnsRanges.Add(MAKELPARAM(nCurrSize, m_Colors.GetSize() - 1));
	}

	m_bHasGroups = TRUE;
}

void CMFCRibbonColorButton::RemoveAllColorGroups()
{
	ASSERT_VALID(this);

	m_Colors.RemoveAll();
	m_bHasGroups = FALSE;

	m_arContColumnsRanges.RemoveAll();

	Clear();
}

void CMFCRibbonColorButton::SetColorBoxSize(CSize sizeBox)
{
	ASSERT_VALID(this);

	if (afxGlobalData.GetRibbonImageScale () != 1.)
	{
		sizeBox.cx = (int) (.5 + afxGlobalData.GetRibbonImageScale () * sizeBox.cx);
		sizeBox.cy = (int) (.5 + afxGlobalData.GetRibbonImageScale () * sizeBox.cy);
	}

	m_sizeBox = sizeBox;

	if (m_bHasGroups && m_arContColumnsRanges.GetSize() > 0)
	{
		m_imagesPalette.SetImageSize(CSize(m_sizeBox.cx, m_sizeBox.cy - 3));
	}
	else
	{
		m_imagesPalette.SetImageSize(m_sizeBox);
	}
}

void CMFCRibbonColorButton::NotifyHighlightListItem(int nIndex)
{
	ASSERT_VALID(this);

	m_ColorHighlighted = GetColorByIndex(nIndex);

	CMFCRibbonGallery::NotifyHighlightListItem(nIndex);
}

CString CMFCRibbonColorButton::GetIconToolTip(const CMFCRibbonGalleryIcon* pIcon) const
{
	ASSERT_VALID(this);
	ASSERT_VALID(pIcon);

	COLORREF color = GetColorByIndex(pIcon->GetIndex());
	if (color != (COLORREF)-1)
	{
		CString str;

		if (!CMFCColorBar::m_ColorNames.Lookup(color, str))
		{
			str.Format(_T("Hex={%02X,%02X,%02X}"), GetRValue(color), GetGValue(color), GetBValue(color));
		}

		return str;
	}

	return CMFCRibbonGallery::GetIconToolTip(pIcon);
}



