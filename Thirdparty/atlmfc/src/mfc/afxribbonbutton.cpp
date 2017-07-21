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
#include "afxribboncategory.h"
#include "afxribbonbutton.h"
#include "afxribbonbar.h"
#include "afxglobals.h"
#include "afxvisualmanager.h"
#include "afxmenuimages.h"
#include "afxribbonbuttonsgroup.h"
#include "afxribbonpanelmenu.h"
#include "afxtoolbarmenubutton.h"
#include "afxmdiframewndex.h"
#include "afxmdichildwndex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BOOL CMFCRibbonButton::m_bUseMenuHandle = FALSE;

const int nLargeButtonMarginX = 5;
const int nLargeButtonMarginY = 1;

const int nSmallButtonMarginX = 3;
const int nSmallButtonMarginY = 3;

const int nDefaultPaneButtonMargin = 2;

IMPLEMENT_DYNCREATE(CMFCRibbonButton, CMFCRibbonBaseElement)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFCRibbonButton::CMFCRibbonButton()
{
	CommonInit();
}

CMFCRibbonButton::CMFCRibbonButton(UINT nID, LPCTSTR lpszText, int nSmallImageIndex, int nLargeImageIndex, BOOL bAlwaysShowDescription)
{
	CommonInit();

	m_nID = nID;
	SetText(lpszText);

	m_nSmallImageIndex = nSmallImageIndex;
	m_nLargeImageIndex = nLargeImageIndex;

	m_bAlwaysShowDescription = bAlwaysShowDescription;
}

CMFCRibbonButton::CMFCRibbonButton( UINT nID, LPCTSTR lpszText, HICON hIcon, BOOL bAlwaysShowDescription, HICON hIconSmall, BOOL bAutoDestroyIcon, BOOL bAlphaBlendIcon)
{
	CommonInit();

	m_nID = nID;
	SetText(lpszText);
	m_hIcon = hIcon;
	m_hIconSmall = hIconSmall;
	m_bAlwaysShowDescription = bAlwaysShowDescription;
	m_bAutoDestroyIcon = bAutoDestroyIcon;
	m_bAlphaBlendIcon = bAlphaBlendIcon;
}

void CMFCRibbonButton::CommonInit()
{
	m_hMenu = NULL;
	m_bRightAlignMenu = FALSE;
	m_bIsDefaultCommand = TRUE;
	m_nMenuArrowMargin = 2;
	m_bAutodestroyMenu = FALSE;

	m_nSmallImageIndex = -1;
	m_nLargeImageIndex = -1;

	m_sizeTextRight = CSize(0, 0);
	m_sizeTextBottom = CSize(0, 0);

	m_szMargin = CSize(nSmallButtonMarginX, nSmallButtonMarginY);

	m_rectMenu.SetRectEmpty();
	m_rectCommand.SetRectEmpty();
	m_bMenuOnBottom = FALSE;
	m_bIsLargeImage = FALSE;

	m_bIsMenuHighlighted = FALSE;
	m_bIsCommandHighlighted = FALSE;

	m_hIcon = NULL;
	m_hIconSmall = NULL;
	m_bForceDrawBorder = FALSE;

	m_bToBeClosed = FALSE;
	m_bAlwaysShowDescription = FALSE;

	m_bCreatedFromMenu = FALSE;
	m_bIsWindowsMenu = FALSE;
	m_nWindowsMenuItems = 0;
	m_nWrapIndex = -1;

	m_bAutoDestroyIcon = FALSE;
	m_bAlphaBlendIcon = FALSE;
}

CMFCRibbonButton::~CMFCRibbonButton()
{
	RemoveAllSubItems();

	if (m_bAutodestroyMenu && m_hMenu != NULL)
	{
		::DestroyMenu(m_hMenu);
	}

	if (m_bAutoDestroyIcon && m_hIcon != NULL)
	{
		::DestroyIcon(m_hIcon);
	}

	if (m_bAutoDestroyIcon && m_hIconSmall != NULL)
	{
		::DestroyIcon(m_hIconSmall);
	}
}

void CMFCRibbonButton::SetText(LPCTSTR lpszText)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::SetText(lpszText);

	m_sizeTextRight = CSize(0, 0);
	m_sizeTextBottom = CSize(0, 0);

	m_arWordIndexes.RemoveAll();

	for (int nOffset = 0;;)
	{
		int nIndex = m_strText.Find(_T(' '), nOffset);
		if (nIndex >= 0)
		{
			ASSERT(nIndex != 0);
			m_arWordIndexes.Add(nIndex);
			nOffset = nIndex + 1;
		}
		else
		{
			break;
		}
	}
}

void CMFCRibbonButton::SetDescription(LPCTSTR lpszText)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::SetDescription(lpszText);

	if (m_bAlwaysShowDescription)
	{
		m_sizeTextRight = CSize(0, 0);
		m_sizeTextBottom = CSize(0, 0);
	}
}

void CMFCRibbonButton::SetMenu(HMENU hMenu, BOOL bIsDefaultCommand, BOOL bRightAlign)
{
	ASSERT_VALID(this);

	m_bIsWindowsMenu = FALSE;
	m_nWindowsMenuItems = 0;

	if (m_bAutodestroyMenu && m_hMenu != NULL)
	{
		::DestroyMenu(m_hMenu);
	}

	m_bAutodestroyMenu = FALSE;

	if (m_bUseMenuHandle)
	{
		m_hMenu = hMenu;
	}
	else
	{
		CMenu* pMenu = CMenu::FromHandle(hMenu);

		for (int i = 0; i <(int) pMenu->GetMenuItemCount(); i++)
		{
			UINT uiID = pMenu->GetMenuItemID(i);

			switch(uiID)
			{
			case 0:
				{
					CMFCRibbonSeparator* pSeparator = new CMFCRibbonSeparator(TRUE);
					pSeparator->SetDefaultMenuLook();

					AddSubItem(pSeparator);
					break;
				}

			default:
				{
					CString str;
					pMenu->GetMenuString(i, str, MF_BYPOSITION);

					//-----------------------------------
					// Remove standard aceleration label:
					//-----------------------------------
					int iTabOffset = str.Find(_T('\t'));
					if (iTabOffset >= 0)
					{
						str = m_strText.Left(iTabOffset);
					}

					CMFCRibbonButton* pItem = new CMFCRibbonButton(uiID, str);
					pItem->SetDefaultMenuLook();
					pItem->m_pRibbonBar = m_pRibbonBar;

					if (uiID == -1)
					{
						pItem->SetMenu(pMenu->GetSubMenu(i)->GetSafeHmenu(), FALSE, bRightAlign);
					}

					AddSubItem(pItem);

					if (uiID >= AFX_IDM_WINDOW_FIRST && uiID <= AFX_IDM_WINDOW_LAST)
					{
						m_bIsWindowsMenu = TRUE;
					}
				}
			}
		}
	}

	m_bIsDefaultCommand = bIsDefaultCommand;

	if (m_nID == 0 || m_nID == (UINT)-1)
	{
		m_bIsDefaultCommand = FALSE;
	}

	m_bRightAlignMenu = bRightAlign;

	m_sizeTextRight = CSize(0, 0);
	m_sizeTextBottom = CSize(0, 0);

	m_bCreatedFromMenu = TRUE;
}

void CMFCRibbonButton::SetMenu(UINT uiMenuResID, BOOL bIsDefaultCommand, BOOL bRightAlign)
{
	ASSERT_VALID(this);

	SetMenu( ::LoadMenu(AfxFindResourceHandle(MAKEINTRESOURCE(uiMenuResID), RT_MENU),
		MAKEINTRESOURCE(uiMenuResID)), bIsDefaultCommand, bRightAlign);

	m_bAutodestroyMenu = TRUE;
}

void CMFCRibbonButton::OnCalcTextSize(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if (m_strText.IsEmpty() || IsApplicationButton())
	{
		m_sizeTextRight = CSize(0, 0);
		m_sizeTextBottom = CSize(0, 0);
		return;
	}

	if (m_sizeTextRight != CSize(0, 0) && m_sizeTextBottom != CSize(0, 0))
	{
		// Already calculated
		return;
	}

	// Text placed on right will be always single line:

	const CString strDummyAmpSeq = _T("\001\001");
	CString strText = m_strText;
	strText.Replace(_T("&&"), strDummyAmpSeq);
	strText.Remove(_T('&'));
	strText.Replace(strDummyAmpSeq, _T("&"));

	if (m_bAlwaysShowDescription && !m_strDescription.IsEmpty())
	{
		CFont* pOldFont = pDC->SelectObject(&afxGlobalData.fontBold);
		ENSURE(pOldFont != NULL);

		m_sizeTextRight = pDC->GetTextExtent(strText);

		pDC->SelectObject(pOldFont);

		// Desciption will be draw below the text(in case of text on right only)
		int nTextHeight = 0;
		int nTextWidth = 0;

		strText = m_strDescription;

		for (int dx = m_sizeTextRight.cx; dx < m_sizeTextRight.cx * 10; dx += 10)
		{
			CRect rectText(0, 0, dx, 10000);

			nTextHeight = pDC->DrawText(strText, rectText, DT_WORDBREAK | DT_CALCRECT);

			nTextWidth = rectText.Width();

			if (nTextHeight <= 2 * m_sizeTextRight.cy)
			{
				break;
			}
		}

		m_sizeTextRight.cx = max(m_sizeTextRight.cx, nTextWidth);
		m_sizeTextRight.cy += min(2 * m_sizeTextRight.cy, nTextHeight) + 2 * m_szMargin.cy;
	}
	else
	{
		// Text placed on right will be always single line:
		m_sizeTextRight = pDC->GetTextExtent(strText);
	}

	CSize sizeImageLarge = GetImageSize(RibbonImageLarge);

	if (sizeImageLarge == CSize(0, 0))
	{
		m_sizeTextBottom = CSize(0, 0);
	}
	else
	{
		// Text placed on bottom will occupy large image size and 1-2 text rows:
		m_sizeTextBottom = DrawBottomText(pDC, TRUE /*bCalcOnly*/);
	}
}

void CMFCRibbonButton::OnDraw(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if (m_rect.IsRectEmpty())
	{
		return;
	}

	if (IsDefaultMenuLook() && !IsQATMode() && !m_bIsLargeImage)
	{
		CMFCToolBarMenuButton dummy;

		dummy.m_strText = m_strText;
		dummy.m_nID = m_nID;
		dummy.m_bMenuMode = TRUE;
		dummy.m_pWndParent = GetParentWnd();
		dummy.m_bIsRadio = m_bIsRadio;

		if (IsChecked())
		{
			dummy.m_nStyle |= TBBS_CHECKED;
		}

		if (HasMenu())
		{
			dummy.m_bDrawDownArrow = TRUE;
		}

		BOOL bIsHighlighted = m_bIsHighlighted;

		if (IsDisabled())
		{
			dummy.m_nStyle |= TBBS_DISABLED;

			bIsHighlighted = IsFocused();
		}

		dummy.OnDraw(pDC, m_rect, NULL, TRUE, FALSE, bIsHighlighted);
		return;
	}

	BOOL bIsDisabled = m_bIsDisabled;
	BOOL bIsDroppedDown = m_bIsDroppedDown;
	BOOL bIsHighlighted = m_bIsHighlighted;
	BOOL bMenuHighlighted = m_bIsMenuHighlighted;
	BOOL bCommandHighlighted = m_bIsCommandHighlighted;

	const int cxDropDown = GetDropDownImageWidth();

	if (m_bIsDisabled && HasMenu())
	{
		if (m_bIsDefaultCommand || (!m_bIsDefaultCommand && !(m_nID == 0 || m_nID == (UINT)-1)))
		{
			m_bIsHighlighted = FALSE;
		}
		else
		{
			m_bIsDisabled = FALSE;
		}
	}

	if (m_bToBeClosed)
	{
		m_bIsDroppedDown = FALSE;
	}

	if (m_bIsFocused)
	{
		m_bIsHighlighted = TRUE;
		m_bIsMenuHighlighted = TRUE;
		m_bIsCommandHighlighted = TRUE;
	}

	CRect rectMenuArrow;
	rectMenuArrow.SetRectEmpty();

	if (HasMenu())
	{
		rectMenuArrow = m_rect;

		rectMenuArrow.left = rectMenuArrow.right - cxDropDown - m_nMenuArrowMargin;
		if (m_sizeTextRight.cx == 0 && !m_bQuickAccessMode)
		{
			rectMenuArrow.left -= 2;
		}

		rectMenuArrow.bottom -= m_nMenuArrowMargin;

		if (m_bIsDefaultCommand)
		{
			m_rectMenu = m_rect;

			m_rectMenu.left = m_rectMenu.right - cxDropDown - m_nMenuArrowMargin - 1;

			m_rectCommand = m_rect;
			m_rectCommand.right = m_rectMenu.left;

			m_bMenuOnBottom = FALSE;
		}
	}

	CSize sizeImageLarge = GetImageSize(RibbonImageLarge);
	CSize sizeImageSmall = GetImageSize(RibbonImageSmall);

	CRect rectText = m_rect;
	BOOL bDrawText = !IsApplicationButton() && !m_bQuickAccessMode && !m_bFloatyMode;

	if (m_bQuickAccessMode || m_bFloatyMode)
	{
		bDrawText = FALSE;
	}
	else if (m_bCompactMode)
	{
		bDrawText = FALSE;
	}
	else if (sizeImageLarge != CSize(0, 0) && !m_bMenuOnBottom && m_bIsLargeImage)
	{
		if (!m_rectMenu.IsRectEmpty())
		{
			m_rectMenu.left -= cxDropDown;
			m_rectCommand.right = m_rectMenu.left;
		}

		rectMenuArrow.OffsetRect(-cxDropDown / 2, 0);
	}

	const RibbonImageType imageType = m_bIsLargeImage ? RibbonImageLarge : RibbonImageSmall;

	CSize sizeImage = GetImageSize(imageType);
	BOOL bDrawDefaultImage = FALSE;

	if ((m_bQuickAccessMode || m_bFloatyMode) && sizeImage == CSize(0, 0))
	{
		// Use default image:
		sizeImage = CSize(16, 16);

		if (afxGlobalData.GetRibbonImageScale() != 1.)
		{
			sizeImage.cx = (int)(.5 + afxGlobalData.GetRibbonImageScale() * sizeImage.cx);
			sizeImage.cy = (int)(.5 + afxGlobalData.GetRibbonImageScale() * sizeImage.cy);
		}

		bDrawDefaultImage = TRUE;
	}

	CRect rectImage = m_rect;
	rectImage.DeflateRect(m_szMargin);

	if (IsApplicationButton())
	{
		rectImage.left += (rectImage.Width () - sizeImage.cx) / 2;
		rectImage.top  += (rectImage.Height () - sizeImage.cy) / 2;
	}
	else if (m_bIsLargeImage && !m_bTextAlwaysOnRight)
	{
		rectImage.left = rectImage.CenterPoint().x - sizeImage.cx / 2;
		rectImage.top += m_szMargin.cy + 1;

		if (!bDrawText)
		{
			rectImage.top = rectImage.CenterPoint().y - sizeImage.cy / 2;
		}
	}
	else
	{
		rectImage.top = rectImage.CenterPoint().y - sizeImage.cy / 2;
	}

	rectImage.right = rectImage.left + sizeImage.cx;
	rectImage.bottom = rectImage.top + sizeImage.cy;

	if (m_bIsLargeImage && !m_bTextAlwaysOnRight && HasMenu() && m_bIsDefaultCommand)
	{
		m_rectMenu = m_rect;
		m_rectMenu.top = rectImage.bottom + 3;

		m_rectCommand = m_rect;
		m_rectCommand.bottom = m_rectMenu.top;

		m_bMenuOnBottom = TRUE;
	}

	COLORREF clrText = (COLORREF)-1;

	if (!IsApplicationButton())
	{
		clrText = OnFillBackground(pDC);
	}

	if (IsMenuMode() && IsChecked() && sizeImage != CSize(0, 0))
	{
		CMFCVisualManager::GetInstance()->OnDrawRibbonMenuCheckFrame(pDC, this, rectImage);
	}

	//------------
	// Draw image:
	//------------
	if (bDrawDefaultImage)
	{
		CMFCVisualManager::GetInstance()->OnDrawDefaultRibbonImage(pDC, rectImage, m_bIsDisabled, m_bIsPressed, m_bIsHighlighted);
	}
	else
	{
		BOOL bIsRibbonImageScale = afxGlobalData.IsRibbonImageScaleEnabled();

		if (IsMenuMode() && !m_bIsLargeImage)
		{
			if (m_pParentMenu == NULL || !m_pParentMenu->IsMainPanel())
			{
				afxGlobalData.EnableRibbonImageScale(FALSE);
			}
		}

		DrawImage(pDC, imageType, rectImage);
		afxGlobalData.EnableRibbonImageScale(bIsRibbonImageScale);
	}

	//-----------
	// Draw text:
	//-----------
	if (bDrawText)
	{
		CFont* pOldFont = NULL;

		rectText = m_rect;
		UINT uiDTFlags = 0;

		COLORREF clrTextOld = (COLORREF)-1;

		if (bIsDisabled && (m_bIsDefaultCommand || (!m_bIsDefaultCommand && !(m_nID == 0 || m_nID == (UINT)-1))))
		{
			if (m_bQuickAccessMode)
			{
				clrText = CMFCVisualManager::GetInstance()->GetRibbonQuickAccessToolBarTextColor(TRUE);
			}
			else
			{
				clrTextOld = pDC->SetTextColor( clrText == (COLORREF)-1 ? CMFCVisualManager::GetInstance()->GetToolbarDisabledTextColor() : clrText);
			}
		}
		else if (clrText != (COLORREF)-1)
		{
			clrTextOld = pDC->SetTextColor(clrText);
		}

		if (m_bIsLargeImage && !m_bTextAlwaysOnRight)
		{
			DrawBottomText(pDC, FALSE);
			rectMenuArrow.SetRectEmpty();
		}
		else
		{
			rectText.left = rectImage.right;

			if (m_nImageOffset > 0)
			{
				rectText.left = m_rect.left + m_nImageOffset + 3 * m_szMargin.cx;
			}
			else if (sizeImage.cx != 0)
			{
				rectText.left += GetTextOffset();
			}

			uiDTFlags = DT_SINGLELINE | DT_END_ELLIPSIS;

			if (!m_bAlwaysShowDescription || m_strDescription.IsEmpty())
			{
				uiDTFlags |= DT_VCENTER;
			}
			else
			{
				pOldFont = pDC->SelectObject(&afxGlobalData.fontBold);
				ENSURE(pOldFont != NULL);

				rectText.top += max(0, (m_rect.Height() - m_sizeTextRight.cy) / 2);
			}

			int nTextHeight = DrawRibbonText(pDC, m_strText, rectText, uiDTFlags);

			if (pOldFont != NULL)
			{
				pDC->SelectObject(pOldFont);
			}

			if (m_bAlwaysShowDescription && !m_strDescription.IsEmpty())
			{
				rectText.top += nTextHeight + m_szMargin.cy;
				rectText.right = m_rect.right - m_szMargin.cx;

				pDC->DrawText(m_strDescription, rectText, DT_WORDBREAK | DT_END_ELLIPSIS);
			}

			if (nTextHeight == m_sizeTextRight.cy && m_bIsLargeImage && HasMenu())
			{
				rectMenuArrow = m_rect;
				rectMenuArrow.DeflateRect(m_nMenuArrowMargin, m_nMenuArrowMargin * 2);
				rectMenuArrow.right -= 2;

				int cyMenu = CMenuImages::Size().cy;

				if (afxGlobalData.GetRibbonImageScale() > 1.)
				{
					cyMenu = (int)(.5 + afxGlobalData.GetRibbonImageScale() * cyMenu);
				}

				rectMenuArrow.top = rectMenuArrow.bottom - cyMenu;
				rectMenuArrow.bottom = rectMenuArrow.top + CMenuImages::Size().cy;
			}
		}

		if (clrTextOld != (COLORREF)-1)
		{
			pDC->SetTextColor(clrTextOld);
		}
	}

	if (!IsApplicationButton())
	{
		if (!rectMenuArrow.IsRectEmpty())
		{
			CMenuImages::IMAGES_IDS id = afxGlobalData.GetRibbonImageScale() > 1. ? CMenuImages::IdArrowDownLarge : CMenuImages::IdArrowDown;

			if (IsMenuMode())
			{
				BOOL bIsRTL = FALSE;

				CMFCRibbonBar* pTopLevelRibbon = GetTopLevelRibbonBar();
				if (pTopLevelRibbon->GetSafeHwnd() != NULL)
				{
					bIsRTL = (pTopLevelRibbon->GetExStyle() & WS_EX_LAYOUTRTL);
				}

				id = bIsRTL ? CMenuImages::IdArrowLeftLarge : CMenuImages::IdArrowRightLarge;
			}

			CRect rectWhite = rectMenuArrow;
			rectWhite.OffsetRect(0, 1);

			CMenuImages::Draw(pDC, id, rectWhite, CMenuImages::ImageWhite);
			CMenuImages::Draw(pDC, id, rectMenuArrow, m_bIsDisabled ? CMenuImages::ImageGray : CMenuImages::ImageBlack);
		}

		OnDrawBorder(pDC);
	}

	m_bIsDisabled = bIsDisabled;
	m_bIsDroppedDown = bIsDroppedDown;
	m_bIsHighlighted = bIsHighlighted;
	m_bIsMenuHighlighted = bMenuHighlighted;
	m_bIsCommandHighlighted = bCommandHighlighted;
}

void CMFCRibbonButton::OnDrawOnList(CDC* pDC, CString strText, int nTextOffset, CRect rect, BOOL /*bIsSelected*/, BOOL bHighlighted)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	BOOL bIsDisabled = m_bIsDisabled;
	m_bIsDisabled = FALSE;

	CRect rectImage = rect;
	rectImage.right = rect.left + nTextOffset;

	CSize sizeImageSmall = GetImageSize(RibbonImageSmall);
	if (sizeImageSmall != CSize(0, 0))
	{
		rectImage.DeflateRect(1, 0);
		rectImage.top += max(0, (rectImage.Height() - sizeImageSmall.cy) / 2);
		rectImage.bottom = rectImage.top + sizeImageSmall.cy;

		DrawImage(pDC, RibbonImageSmall, rectImage);
	}
	else if (m_bDrawDefaultIcon)
	{
		CMFCVisualManager::GetInstance()->OnDrawDefaultRibbonImage(pDC, rectImage);
	}

	CRect rectText = rect;

	if (HasMenu())
	{
		CRect rectMenuArrow = rect;
		rectMenuArrow.left = rectMenuArrow.right - rectMenuArrow.Height();

		CRect rectWhite = rectMenuArrow;
		rectWhite.OffsetRect(0, 1);

		BOOL bIsDarkMenu = TRUE;

		if (bHighlighted)
		{
			COLORREF clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);

			if (GetRValue(clrText) > 128 && GetGValue(clrText) > 128 && GetBValue(clrText) > 128)
			{
				bIsDarkMenu = FALSE;
			}
		}

		CMenuImages::IMAGES_IDS id = afxGlobalData.GetRibbonImageScale() > 1. ? CMenuImages::IdArrowRightLarge : CMenuImages::IdArrowRight;

		CMenuImages::Draw(pDC, id, rectWhite, bIsDarkMenu ? CMenuImages::ImageWhite : CMenuImages::ImageBlack); 
		CMenuImages::Draw(pDC, id, rectMenuArrow, bIsDarkMenu ? CMenuImages::ImageBlack : CMenuImages::ImageWhite); 
		rectText.right = rectMenuArrow.left;
	}

	rectText.left += nTextOffset;

	const int nXMargin = 3;
	rectText.DeflateRect(nXMargin, 0);

	pDC->DrawText(strText, rectText, DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	m_bIsDisabled = bIsDisabled;
}

CSize CMFCRibbonButton::GetRegularSize(CDC* pDC)
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arSubItems.GetSize(); i++)
	{
		ASSERT_VALID(m_arSubItems [i]);
		m_arSubItems [i]->SetParentCategory(m_pParent);
	}

	if (m_bQuickAccessMode || m_bFloatyMode)
	{
		return GetCompactSize(pDC);
	}

	if (!HasLargeMode())
	{
		return GetIntermediateSize(pDC);
	}

	CSize sizeImageLarge = GetImageSize(RibbonImageLarge);
	CSize sizeImageSmall = GetImageSize(RibbonImageSmall);

	if (IsApplicationButton())
	{
		return sizeImageLarge;
	}

	const int cxExtra = GetGroupButtonExtraWidth();

	if (sizeImageLarge == CSize(0, 0) || m_bTextAlwaysOnRight)
	{
		if (m_bTextAlwaysOnRight && sizeImageLarge != CSize(0, 0))
		{
			sizeImageSmall = CSize(sizeImageLarge.cx + 2, sizeImageLarge.cy + 2);
			m_szMargin.cy = 5;
		}

		int cx = sizeImageSmall.cx + 2 * m_szMargin.cx;

		if (m_sizeTextRight.cx > 0)
		{
			cx += m_szMargin.cx + m_sizeTextRight.cx;
		}

		int cy = max(sizeImageSmall.cy, m_sizeTextRight.cy) + 2 * m_szMargin.cy;

		if (sizeImageSmall.cy == 0)
		{
			cy += 2 * m_szMargin.cy;
		}

		if (HasMenu())
		{
			cx += GetDropDownImageWidth();

			if (m_bIsDefaultCommand && m_nID != -1 && m_nID != 0 && m_sizeTextRight.cx > 0)
			{
				cx += m_nMenuArrowMargin;
			}
		}

		if (IsDefaultMenuLook() && !IsQATMode())
		{
			cx += 2 * AFX_TEXT_MARGIN;
		}

		return CSize(cx + cxExtra, cy);
	}

	SetMargin(CSize(nLargeButtonMarginX, nLargeButtonMarginY));

	if (IsDefaultPanelButton())
	{
		sizeImageLarge.cx += 2 *(m_szMargin.cx + 1);
	}

	int cx = max(sizeImageLarge.cx + 2 * m_szMargin.cx, m_sizeTextBottom.cx + 5);

	if (IsDefaultPanelButton())
	{
		cx += nDefaultPaneButtonMargin;
	}

	if (IsDefaultMenuLook())
	{
		cx += 2 * AFX_TEXT_MARGIN;
	}

	const int cyText = max(m_sizeTextBottom.cy, sizeImageLarge.cy + 1);
	const int cy = sizeImageLarge.cy + cyText + 1;

	return CSize(cx + cxExtra, cy);
}

CSize CMFCRibbonButton::GetCompactSize(CDC* /*pDC*/)
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arSubItems.GetSize(); i++)
	{
		ASSERT_VALID(m_arSubItems [i]);
		m_arSubItems [i]->SetParentCategory(m_pParent);
	}

	CSize sizeImageSmall = GetImageSize(RibbonImageSmall);

	if (IsApplicationButton())
	{
		return sizeImageSmall;
	}

	const int cxDropDown = GetDropDownImageWidth();

	int cxExtra = 0;

	if (m_bQuickAccessMode || m_bFloatyMode)
	{
		SetMargin(afxGlobalData.GetRibbonImageScale() != 1. ? CSize(nSmallButtonMarginX, nSmallButtonMarginY - 1) : CSize(nSmallButtonMarginX, nSmallButtonMarginY));

		if (sizeImageSmall == CSize(0, 0))
		{
			sizeImageSmall = CSize(16, 16);

			if (afxGlobalData.GetRibbonImageScale() != 1.)
			{
				sizeImageSmall.cx = (int)(.5 + afxGlobalData.GetRibbonImageScale() * sizeImageSmall.cx);
				sizeImageSmall.cy = (int)(.5 + afxGlobalData.GetRibbonImageScale() * sizeImageSmall.cy);
			}
		}
	}
	else
	{
		SetMargin(CSize(nSmallButtonMarginX, nSmallButtonMarginY));
		cxExtra = GetGroupButtonExtraWidth();

		if (IsDefaultMenuLook())
		{
			cxExtra += 2 * AFX_TEXT_MARGIN;
		}
	}

	int nMenuArrowWidth = 0;

	if (HasMenu())
	{
		if (m_bIsDefaultCommand)
		{
			nMenuArrowWidth = cxDropDown + m_szMargin.cx / 2 + 1;
		}
		else
		{
			nMenuArrowWidth = cxDropDown - m_szMargin.cx / 2 - 1;
		}
	}

	int cx = sizeImageSmall.cx + 2 * m_szMargin.cx + nMenuArrowWidth + cxExtra;
	int cy = sizeImageSmall.cy + 2 * m_szMargin.cy;

	return CSize(cx, cy);
}

CSize CMFCRibbonButton::GetIntermediateSize(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	for (int i = 0; i < m_arSubItems.GetSize(); i++)
	{
		ASSERT_VALID(m_arSubItems [i]);
		m_arSubItems [i]->SetParentCategory(m_pParent);
	}

	if (m_bQuickAccessMode || m_bFloatyMode)
	{
		return GetCompactSize(pDC);
	}

	SetMargin(CSize(nSmallButtonMarginX, nSmallButtonMarginY));

	const int nMenuArrowWidth = HasMenu() ? GetDropDownImageWidth() : 0;

	CSize sizeImageSmall = GetImageSize(RibbonImageSmall);

	sizeImageSmall.cy = max(16, sizeImageSmall.cy);

	int cy = max(sizeImageSmall.cy, m_sizeTextRight.cy) + 2 * m_szMargin.cy;
	int cx = sizeImageSmall.cx + 2 * m_szMargin.cx + nMenuArrowWidth + m_sizeTextRight.cx + GetTextOffset() + GetGroupButtonExtraWidth() + 1;

	if (IsDefaultMenuLook())
	{
		cx += 2 * AFX_TEXT_MARGIN;
		cy += AFX_TEXT_MARGIN / 2 - 1;
	}

	return CSize(cx, cy);
}

void CMFCRibbonButton::OnLButtonDown(CPoint point)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::OnLButtonDown(point);

	if (!HasMenu() || IsMenuMode())
	{
		return;
	}

	if (!m_rectMenu.IsRectEmpty() && !m_rectMenu.PtInRect(point))
	{
		return;
	}

	if (m_bIsDefaultCommand && m_bIsDisabled)
	{
		return;
	}

	if (m_bIsDisabled && m_rectCommand.IsRectEmpty())
	{
		return;
	}

	OnShowPopupMenu();
}

void CMFCRibbonButton::OnLButtonUp(CPoint point)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::OnLButtonUp(point);

	BOOL bIsPressed = m_bIsPressed || IsMenuMode();

	if (m_bIsDisabled || !bIsPressed || !m_bIsHighlighted)
	{
		return;
	}

	if (m_bIsDroppedDown)
	{
		if (!m_rectCommand.IsRectEmpty () && m_rectCommand.PtInRect (point) && IsMenuMode ())
		{
			OnClick (point);
		}
		return;
	}

	if (!m_rectCommand.IsRectEmpty() && !m_rectCommand.PtInRect(point))
	{
		return;
	}

	OnClick(point);
}

void CMFCRibbonButton::OnMouseMove(CPoint point)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::OnMouseMove(point);

	if (!HasMenu() || m_nID == -1 || m_nID == 0)
	{
		return;
	}

	BOOL bMenuWasHighlighted = m_bIsMenuHighlighted;
	BOOL bCommandWasHighlighted = m_bIsCommandHighlighted;

	m_bIsMenuHighlighted = m_rectMenu.PtInRect(point);
	m_bIsCommandHighlighted = m_rectCommand.PtInRect(point);

	if (bMenuWasHighlighted != m_bIsMenuHighlighted ||
		bCommandWasHighlighted != m_bIsCommandHighlighted)
	{
		Redraw();

		if (m_pParentMenu != NULL)
		{
			ASSERT_VALID(m_pParentMenu);
			m_pParentMenu->OnChangeHighlighted(this);
		}
	}
}

void CMFCRibbonButton::OnClick(CPoint point)
{
	ASSERT_VALID(this);

	if (IsMenuMode() && HasMenu() && m_rectCommand.IsRectEmpty())
	{
		return;
	}

	if (m_pParentMenu != NULL)
	{
		ASSERT_VALID(m_pParentMenu);
		m_pParentMenu->OnClickButton(this, point);
		return;
	}

	NotifyCommand();
}

void CMFCRibbonButton::OnShowPopupMenu()
{
	ASSERT_VALID(this);

	if (IsDroppedDown())
	{
		// if the button already has a menu, don't create another one!
		return;
	}

	CWnd* pWndParent = GetParentWnd();
	if (pWndParent->GetSafeHwnd() == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	CMFCRibbonBar* pTopLevelRibbon = GetTopLevelRibbonBar();
	if (pTopLevelRibbon->GetSafeHwnd() == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	CMFCRibbonBaseElement::OnShowPopupMenu();

	const BOOL bIsRTL = (pTopLevelRibbon->GetExStyle() & WS_EX_LAYOUTRTL);

	CWnd* pWndOwner = pTopLevelRibbon->GetSafeOwner();

	if (m_arSubItems.GetSize() > 0)
	{
		if (m_bIsWindowsMenu)
		{
			FillWindowList();
		}

		//--------------------------------
		// Build popup menu from subitems:
		//--------------------------------
		CMFCRibbonPanelMenu* pMenu = new CMFCRibbonPanelMenu(pTopLevelRibbon, m_arSubItems);

		pMenu->SetParentRibbonElement(this);

		pMenu->SetMenuMode();

		BOOL bIsPopupDefaultMenuLook = IsPopupDefaultMenuLook();

		for (int i = 0; bIsPopupDefaultMenuLook && i < m_arSubItems.GetSize(); i++)
		{
			ASSERT_VALID(m_arSubItems [i]);

			if (!m_arSubItems [i]->IsDefaultMenuLook())
			{
				bIsPopupDefaultMenuLook = FALSE;
			}
		}

		pMenu->SetDefaultMenuLook(bIsPopupDefaultMenuLook);

		if (m_pOriginal != NULL && m_pOriginal->GetParentPanel() != NULL && m_pOriginal->GetParentPanel()->IsMainPanel())
		{
			pMenu->SetDefaultMenuLook(FALSE);
		}

		CRect rectBtn = GetRect();
		pWndParent->ClientToScreen(&rectBtn);

		int x = m_bRightAlignMenu || bIsRTL ? rectBtn.right : rectBtn.left;

		int y = rectBtn.bottom;

		if (m_bCreatedFromMenu && m_bRightAlignMenu && !bIsRTL)
		{
			pMenu->SetRightAlign();
		}

		if (IsMenuMode())
		{
			x = bIsRTL ? rectBtn.left : rectBtn.right;
			y = rectBtn.top;
		}

		CRect rectMenuLocation;
		rectMenuLocation.SetRectEmpty();

		CMFCRibbonPanel* pPanel = GetParentPanel();

		if (pPanel != NULL &&
			pPanel->GetPreferedMenuLocation(rectMenuLocation))
		{
			pWndParent->ClientToScreen(&rectMenuLocation);

			x = bIsRTL ? rectMenuLocation.right : rectMenuLocation.left;
			y = rectMenuLocation.top;

			pMenu->SetPreferedSize(rectMenuLocation.Size());
			pMenu->SetDefaultMenuLook(FALSE);
		}

		pMenu->Create(pWndOwner, x, y, (HMENU) NULL);

		SetDroppedDown(pMenu);
		return;
	}

	HMENU hPopupMenu = GetMenu();
	if (hPopupMenu == NULL)
	{
		return;
	}

	CRect rectBtn = GetRect();
	pWndParent->ClientToScreen(&rectBtn);

	CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

	pPopupMenu->SetAutoDestroy(FALSE);
	pPopupMenu->SetRightAlign(m_bRightAlignMenu && !bIsRTL);

	pPopupMenu->SetParentRibbonElement(this);

	CMFCPopupMenu* pMenuActive = CMFCPopupMenu::GetActiveMenu();
	if (pMenuActive != NULL && pMenuActive->GetSafeHwnd() != pWndParent->GetParent()->GetSafeHwnd())
	{
		pMenuActive->SendMessage(WM_CLOSE);
	}

	int x = m_bRightAlignMenu || bIsRTL ? rectBtn.right : rectBtn.left;
	int y = rectBtn.bottom;

	pPopupMenu->Create(pWndOwner, x, y, hPopupMenu, FALSE);

	SetDroppedDown(pPopupMenu);
}

void CMFCRibbonButton::SetParentCategory(CMFCRibbonCategory* pParent)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::SetParentCategory(pParent);

	for (int i = 0; i < m_arSubItems.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pSubItem = m_arSubItems [i];
		ASSERT_VALID(pSubItem);

		pSubItem->SetParentCategory(m_pParent);
		pSubItem->SetDefaultMenuLook(!m_bUseMenuHandle && !pSubItem->HasLargeMode());
	}
}

void CMFCRibbonButton::CopyFrom(const CMFCRibbonBaseElement& s)
{
	ASSERT_VALID(this);

	if (m_bAutodestroyMenu && m_hMenu != NULL)
	{
		::DestroyMenu(m_hMenu);
	}

	if (m_bAutoDestroyIcon && m_hIcon != NULL)
	{
		::DestroyIcon(m_hIcon);
	}

	if (m_bAutoDestroyIcon && m_hIconSmall != NULL)
	{
		::DestroyIcon(m_hIconSmall);
	}

	RemoveAllSubItems();

	CMFCRibbonBaseElement::CopyFrom(s);

	CMFCRibbonButton& src = (CMFCRibbonButton&) s;

	m_nSmallImageIndex = src.m_nSmallImageIndex;
	m_nLargeImageIndex = src.m_nLargeImageIndex;
	m_hMenu = src.m_hMenu;
	m_bAutodestroyMenu = FALSE;
	m_bRightAlignMenu = src.m_bRightAlignMenu;
	m_bIsDefaultCommand = src.m_bIsDefaultCommand;
	m_szMargin = src.m_szMargin;
	m_hIcon = src.m_hIcon;
	m_hIconSmall = src.m_hIconSmall;
	m_bAutoDestroyIcon = FALSE;
	m_bAlphaBlendIcon = src.m_bAlphaBlendIcon;
	m_bForceDrawBorder = src.m_bForceDrawBorder;
	m_bAlwaysShowDescription = src.m_bAlwaysShowDescription;
	m_bCreatedFromMenu = src.m_bCreatedFromMenu;
	m_bIsWindowsMenu = src.m_bIsWindowsMenu;
	m_nWindowsMenuItems = src.m_nWindowsMenuItems;

	int i = 0;

	for (i = 0; i < src.m_arSubItems.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pSrcElem = src.m_arSubItems [i];
		ASSERT_VALID(pSrcElem);

		CMFCRibbonBaseElement* pElem = (CMFCRibbonBaseElement*) pSrcElem->GetRuntimeClass()->CreateObject();
		ASSERT_VALID(pElem);

		pElem->CopyFrom(*pSrcElem);
		m_arSubItems.Add(pElem);
	}

	m_nWrapIndex = src.m_nWrapIndex;

	m_arWordIndexes.RemoveAll();

	for (i = 0; i < src.m_arWordIndexes.GetSize(); i++)
	{
		m_arWordIndexes.Add(src.m_arWordIndexes [i]);
	}
}

void CMFCRibbonButton::SetOriginal(CMFCRibbonBaseElement* pOriginal)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::SetOriginal(pOriginal);

	CMFCRibbonButton* pOriginalButton = DYNAMIC_DOWNCAST(CMFCRibbonButton, pOriginal);

	if (pOriginalButton == NULL)
	{
		return;
	}

	ASSERT_VALID(pOriginalButton);

	if (pOriginalButton->m_arSubItems.GetSize() != m_arSubItems.GetSize())
	{
		ASSERT(FALSE);
		return;
	}

	for (int i = 0; i < m_arSubItems.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arSubItems [i];
		ASSERT_VALID(pButton);

		pButton->SetOriginal(pOriginalButton->m_arSubItems [i]);
	}
}

void CMFCRibbonButton::DrawImage(CDC* pDC, RibbonImageType type, CRect rectImage)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	CMFCRibbonButton* pOrigButton = DYNAMIC_DOWNCAST(CMFCRibbonButton, m_pOriginal);

	if (pOrigButton != NULL)
	{
		ASSERT_VALID(pOrigButton);

		BOOL bIsDisabled = pOrigButton->m_bIsDisabled;
		pOrigButton->m_bIsDisabled = m_bIsDisabled;

		CRect rect = pOrigButton->m_rect;
		pOrigButton->m_rect = m_rect;

		pOrigButton->DrawImage(pDC, type, rectImage);

		pOrigButton->m_bIsDisabled = bIsDisabled;
		pOrigButton->m_rect = rect;
		return;
	}

	if (m_hIcon != NULL)
	{
		HICON hIcon = type == RibbonImageLarge || m_hIconSmall == NULL ? m_hIcon : m_hIconSmall;

		CSize sizeIcon = type == RibbonImageLarge ? CSize(32, 32) : CSize(16, 16);

		if (afxGlobalData.GetRibbonImageScale() != 1.)
		{
			sizeIcon.cx = (int)(.5 + afxGlobalData.GetRibbonImageScale() * sizeIcon.cx);
			sizeIcon.cy = (int)(.5 + afxGlobalData.GetRibbonImageScale() * sizeIcon.cy);
		}

		if (m_bIsDisabled)
		{
			CMFCToolBarImages icon;
			icon.SetImageSize(type == RibbonImageLarge ? CSize(32, 32) : CSize(16, 16));

			icon.AddIcon(hIcon, m_bAlphaBlendIcon);

			CAfxDrawState ds;
			icon.PrepareDrawImage(ds, sizeIcon);
			icon.Draw(pDC, rectImage.left, rectImage.top, 0, FALSE, TRUE);
			icon.EndDrawImage(ds);
		}
		else
		{
			UINT diFlags = DI_NORMAL;

			CWnd* pWndParent = GetParentWnd();
			if (pWndParent != NULL &&(pWndParent->GetExStyle() & WS_EX_LAYOUTRTL))
			{
				diFlags |= 0x0010 /*DI_NOMIRROR*/;
			}

			::DrawIconEx(pDC->GetSafeHdc(), rectImage.left, rectImage.top, hIcon, sizeIcon.cx, sizeIcon.cy, 0, NULL, diFlags);
		}
		return;
	}

	if (m_pParentGroup != NULL)
	{
		ASSERT_VALID(m_pParentGroup);

		if (m_pParentGroup->HasImages())
		{
			m_pParentGroup->OnDrawImage(pDC, rectImage, this, m_nSmallImageIndex);
			return;
		}
	}

	if (m_pParent == NULL || rectImage.Width() == 0 || rectImage.Height() == 0)
	{
		return;
	}

	ASSERT_VALID(m_pParent);

	m_pParent->OnDrawImage(pDC, rectImage, this, type == RibbonImageLarge, type == RibbonImageLarge ? m_nLargeImageIndex : m_nSmallImageIndex, FALSE /* no center */);
}

CSize CMFCRibbonButton::GetImageSize(RibbonImageType type) const
{
	ASSERT_VALID(this);

	CMFCRibbonButton* pOrigButton = DYNAMIC_DOWNCAST(CMFCRibbonButton, m_pOriginal);

	if (pOrigButton != NULL)
	{
		ASSERT_VALID(pOrigButton);
		return pOrigButton->GetImageSize(type);
	}

	if (m_hIcon != NULL)
	{
		CSize sizeIcon = type == RibbonImageLarge ? CSize(32, 32) : CSize(16, 16);

		if (afxGlobalData.GetRibbonImageScale() != 1.)
		{
			sizeIcon.cx = (int)(.5 + afxGlobalData.GetRibbonImageScale() * sizeIcon.cx);
			sizeIcon.cy = (int)(.5 + afxGlobalData.GetRibbonImageScale() * sizeIcon.cy);
		}

		return sizeIcon;
	}

	const int nImageIndex = type == RibbonImageLarge  ? m_nLargeImageIndex : m_nSmallImageIndex;

	if (nImageIndex < 0)
	{
		return CSize(0, 0);
	}

	if (m_pParentGroup != NULL)
	{
		ASSERT_VALID(m_pParentGroup);

		if (m_pParentGroup->HasImages())
		{
			return m_pParentGroup->GetImageSize();
		}
	}

	if (m_pParent == NULL)
	{
		return CSize(0, 0);
	}

	ASSERT_VALID(m_pParent);

	const int nImageCount = m_pParent->GetImageCount(type == RibbonImageLarge);

	if (nImageIndex >= nImageCount)
	{
		return CSize(0, 0);
	}

	return m_pParent->GetImageSize(type == RibbonImageLarge);
}

BOOL CMFCRibbonButton::CanBeStretched()
{
	ASSERT_VALID(this);
	return GetImageSize(RibbonImageLarge) != CSize(0, 0);
}

BOOL CMFCRibbonButton::SetACCData(CWnd* pParent, CAccessibilityData& data)
{
	if (!CMFCRibbonBaseElement::SetACCData(pParent, data))
	{
		return FALSE;
	}

	if (HasMenu())
	{
		data.m_nAccRole = IsCommandAreaHighlighted() ? ROLE_SYSTEM_SPLITBUTTON : ROLE_SYSTEM_BUTTONDROPDOWN;
		if (!IsCommandAreaHighlighted())
		{
			data.m_bAccState |= STATE_SYSTEM_HASPOPUP;
			data.m_strAccDefAction = _T("Open");

			if (IsDroppedDown())
			{
				data.m_bAccState |= STATE_SYSTEM_PRESSED;
				data.m_strAccDefAction = _T("Close");
			}
		}
	}
	return TRUE;
}

void CMFCRibbonButton::AddSubItem(CMFCRibbonBaseElement* pSubItem, int nIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSubItem);

	pSubItem->SetParentCategory(m_pParent);
	pSubItem->SetDefaultMenuLook(!m_bUseMenuHandle && !pSubItem->HasLargeMode());

	if (nIndex == -1)
	{
		m_arSubItems.Add(pSubItem);
	}
	else
	{
		m_arSubItems.InsertAt(nIndex, pSubItem);
	}
}

int CMFCRibbonButton::FindSubItemIndexByID(UINT uiID) const
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arSubItems.GetSize(); i++)
	{
		ASSERT_VALID(m_arSubItems [i]);

		if (m_arSubItems [i]->GetID() == uiID)
		{
			return i;
		}
	}

	return -1;
}

BOOL CMFCRibbonButton::RemoveSubItem(int nIndex)
{
	ASSERT_VALID(this);

	if (nIndex < 0 || nIndex >= m_arSubItems.GetSize())
	{
		return FALSE;
	}

	ASSERT_VALID(m_arSubItems [nIndex]);
	delete m_arSubItems [nIndex];

	m_arSubItems.RemoveAt(nIndex);

	return TRUE;
}

void CMFCRibbonButton::RemoveAllSubItems()
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arSubItems.GetSize(); i++)
	{
		ASSERT_VALID(m_arSubItems [i]);
		delete m_arSubItems [i];
	}

	m_arSubItems.RemoveAll();
}

COLORREF CMFCRibbonButton::OnFillBackground(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	const BOOL bIsDisabled = m_bIsDisabled;

	if (m_bIsDisabled && HasMenu())
	{
		m_bIsDisabled = FALSE;
	}

	COLORREF clrText = CMFCVisualManager::GetInstance()->OnFillRibbonButton(pDC, this);

	m_bIsDisabled = bIsDisabled;

	return clrText;
}

void CMFCRibbonButton::OnDrawBorder(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	const BOOL bIsDisabled = m_bIsDisabled;

	if (m_bIsDisabled && HasMenu())
	{
		m_bIsDisabled = FALSE;
	}

	CMFCVisualManager::GetInstance()->OnDrawRibbonButtonBorder(pDC, this);

	m_bIsDisabled = bIsDisabled;
}

int CMFCRibbonButton::AddToListBox(CMFCRibbonCommandsListBox* pWndListBox, BOOL bDeep)
{
	ASSERT_VALID(this);

	int nIndex = CMFCRibbonBaseElement::AddToListBox(pWndListBox, bDeep);

	if (bDeep && !m_bCreatedFromMenu)
	{
		for (int i = 0; i < m_arSubItems.GetSize(); i++)
		{
			ASSERT_VALID(m_arSubItems [i]);

			if (m_arSubItems [i]->GetID() != 0) // Don't add separators
			{
				nIndex = m_arSubItems [i]->AddToListBox(pWndListBox, TRUE);
			}
		}
	}

	return nIndex;
}

void CMFCRibbonButton::ClosePopupMenu()
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arSubItems.GetSize(); i++)
	{
		ASSERT_VALID(m_arSubItems [i]);
		m_arSubItems [i]->ClosePopupMenu();
	}

	CMFCRibbonBaseElement::ClosePopupMenu();
}

CMFCRibbonBaseElement* CMFCRibbonButton::FindByID(UINT uiCmdID)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement* pElem = CMFCRibbonBaseElement::FindByID(uiCmdID);
	if (pElem != NULL)
	{
		ASSERT_VALID(pElem);
		return pElem;
	}

	for (int i = 0; i < m_arSubItems.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arSubItems [i];
		ASSERT_VALID(pButton);

		pElem = pButton->FindByID(uiCmdID);
		if (pElem != NULL)
		{
			ASSERT_VALID(pElem);
			return pElem;
		}
	}

	return NULL;
}

CMFCRibbonBaseElement* CMFCRibbonButton::FindByData(DWORD_PTR dwData)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement* pElem = CMFCRibbonBaseElement::FindByData(dwData);
	if (pElem != NULL)
	{
		ASSERT_VALID(pElem);
		return pElem;
	}

	for (int i = 0; i < m_arSubItems.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arSubItems [i];
		ASSERT_VALID(pButton);

		pElem = pButton->FindByData(dwData);
		if (pElem != NULL)
		{
			ASSERT_VALID(pElem);
			return pElem;
		}
	}

	return NULL;
}

CString CMFCRibbonButton::GetToolTipText() const
{
	ASSERT_VALID(this);

	if (!m_bQuickAccessMode && m_bAlwaysShowDescription && !m_strDescription.IsEmpty())
	{
		return _T("");
	}

	return CMFCRibbonBaseElement::GetToolTipText();
}

int CMFCRibbonButton::DrawRibbonText(CDC* pDC, const CString& strText, CRect rectText, UINT uiDTFlags, COLORREF clrText)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if (CMFCToolBarImages::m_bIsDrawOnGlass)
	{
		const BOOL bIsZoomed = GetParentRibbonBar()->GetSafeHwnd() != NULL && GetParentRibbonBar()->GetParent()->IsZoomed();

		CMFCVisualManager::GetInstance()->DrawTextOnGlass(pDC, strText, rectText, uiDTFlags, 0, bIsZoomed ? RGB(255, 255, 255) : clrText);

		return pDC->GetTextExtent(strText).cy;
	}

	COLORREF clrTextOld = (COLORREF)-1;
	if (clrText != (COLORREF)-1)
	{
		clrTextOld = pDC->SetTextColor(clrText);
	}

	int nRes = pDC->DrawText(strText, rectText, uiDTFlags);

	if (clrTextOld != (COLORREF)-1)
	{
		pDC->SetTextColor(clrTextOld);
	}

	return nRes;
}

void CMFCRibbonButton::SetParentRibbonBar(CMFCRibbonBar* pRibbonBar)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::SetParentRibbonBar(pRibbonBar);

	for (int i = 0; i < m_arSubItems.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arSubItems [i];
		ASSERT_VALID(pButton);

		pButton->SetParentRibbonBar(pRibbonBar);
	}
}

CRect CMFCRibbonButton::GetKeyTipRect(CDC* pDC, BOOL bIsMenu)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	CSize sizeKeyTip = GetKeyTipSize(pDC);
	CRect rectKeyTip(0, 0, 0, 0);

	if (sizeKeyTip == CSize(0, 0) || m_rect.IsRectEmpty())
	{
		return rectKeyTip;
	}

	rectKeyTip.left = bIsMenu ? m_rect.right - sizeKeyTip.cx / 2 : m_rect.left + 10;
	rectKeyTip.top = m_rect.bottom - sizeKeyTip.cy / 2;

	CRect rectPanel;
	rectPanel.SetRectEmpty();

	CMFCRibbonPanel* pPanel = GetParentPanel();
	if (pPanel != NULL && !IsMenuMode() && !(m_bQuickAccessMode && m_bFloatyMode) && !IsDefaultPanelButton ())
	{
		ASSERT_VALID(pPanel);

		rectPanel = pPanel->GetRect();

		if (!rectPanel.IsRectEmpty())
		{
			rectPanel.bottom -= pPanel->GetCaptionHeight();
			rectKeyTip.top = rectPanel.bottom - sizeKeyTip.cy / 2;
		}
	}

	if (IsDefaultPanelButton() && !m_bQuickAccessMode && !m_bFloatyMode)
	{
		rectKeyTip.top = m_rect.bottom;
		rectKeyTip.left = m_rect.CenterPoint().x - sizeKeyTip.cx / 2;
	}
	else if (IsApplicationButton())
	{
		// Center key tip:
		rectKeyTip.top = m_rect.CenterPoint().y - sizeKeyTip.cy / 2;
		rectKeyTip.left = m_rect.CenterPoint().x - sizeKeyTip.cx / 2;
	}
	else if (m_bIsLargeImage || m_bFloatyMode)
	{
		if (m_bTextAlwaysOnRight)
		{
			if (!bIsMenu)
			{
				rectKeyTip.left = m_rect.left + GetImageSize(RibbonImageLarge).cx - sizeKeyTip.cx + 4;
			}

			rectKeyTip.top = m_rect.bottom - sizeKeyTip.cy - 4;
		}
		else if (!bIsMenu)
		{
			rectKeyTip.left = m_rect.CenterPoint().x - sizeKeyTip.cx / 2;
		}
	}
	else if (IsMenuMode())
	{
		rectKeyTip.top = m_rect.CenterPoint().y;
	}
	else
	{
		if (m_bQuickAccessMode)
		{
			rectKeyTip.left = m_rect.CenterPoint().x - sizeKeyTip.cx / 2;
			rectKeyTip.top = m_rect.CenterPoint().y;
		}

		if (!rectPanel.IsRectEmpty())
		{
			if (m_rect.top < rectPanel.CenterPoint().y && m_rect.bottom > rectPanel.CenterPoint().y)
			{
				rectKeyTip.top = m_rect.CenterPoint().y - sizeKeyTip.cy / 2;
			}
			else if (m_rect.top < rectPanel.CenterPoint().y)
			{
				rectKeyTip.top = m_rect.top - sizeKeyTip.cy / 2;
			}
		}
	}

	rectKeyTip.right = rectKeyTip.left + sizeKeyTip.cx;
	rectKeyTip.bottom = rectKeyTip.top + sizeKeyTip.cy;

	return rectKeyTip;
}

BOOL CMFCRibbonButton::OnKey(BOOL bIsMenuKey)
{
	ASSERT_VALID(this);

	if (IsDisabled())
	{
		return FALSE;
	}

	if (m_rect.IsRectEmpty())
	{
		return CMFCRibbonBaseElement::OnKey(bIsMenuKey);
	}

	CMFCRibbonBar* pTopLevelRibbon = GetTopLevelRibbonBar();

	if (HasMenu() &&(bIsMenuKey || m_strMenuKeys.IsEmpty()))
	{
		if (pTopLevelRibbon != NULL)
		{
			pTopLevelRibbon->HideKeyTips();
		}

		OnShowPopupMenu();
		return m_hMenu != NULL;
	}

	if (pTopLevelRibbon != NULL && pTopLevelRibbon->GetTopLevelFrame() != NULL)
	{
		pTopLevelRibbon->GetTopLevelFrame()->SetFocus();
	}

	OnClick(m_rect.TopLeft());
	return TRUE;
}

void CMFCRibbonButton::GetElementsByID(UINT uiCmdID, CArray <CMFCRibbonBaseElement*, CMFCRibbonBaseElement*>& arElements)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::GetElementsByID(uiCmdID, arElements);

	for (int i = 0; i < m_arSubItems.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arSubItems [i];
		ASSERT_VALID(pButton);

		pButton->GetElementsByID(uiCmdID, arElements);
	}
}

void CMFCRibbonButton::GetElements(CArray <CMFCRibbonBaseElement*, CMFCRibbonBaseElement*>& arElements)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::GetElements(arElements);

	for (int i = 0; i < m_arSubItems.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arSubItems [i];
		ASSERT_VALID(pButton);

		pButton->GetElements(arElements);
	}
}

void CMFCRibbonButton::OnAfterChangeRect(CDC* pDC)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::OnAfterChangeRect(pDC);

	if (IsApplicationButton())
	{
		m_bIsLargeImage = TRUE;
		return;
	}

	m_bIsLargeImage = FALSE;

	if (m_bQuickAccessMode || m_bFloatyMode)
	{
		return;
	}

	CSize sizeImageLarge = GetImageSize(RibbonImageLarge);
	CSize sizeImageSmall = GetImageSize(RibbonImageSmall);

	if (m_bCompactMode || m_bIntermediateMode)
	{
		m_bIsLargeImage = FALSE;

		if (sizeImageLarge != CSize(0, 0) && sizeImageSmall == CSize(0, 0))
		{
			m_bIsLargeImage = TRUE;
		}
	}
	else
	{
		BOOL bIsSmallIcon = FALSE;

		if (m_hIcon != NULL)
		{
			CSize sizeIcon = CSize(32, 32);

			if (afxGlobalData.GetRibbonImageScale() != 1.)
			{
				sizeIcon.cx = (int)(.5 + afxGlobalData.GetRibbonImageScale() * sizeIcon.cx);
				sizeIcon.cy = (int)(.5 + afxGlobalData.GetRibbonImageScale() * sizeIcon.cy);
			}

			bIsSmallIcon = sizeIcon.cx > m_rect.Width() || sizeIcon.cy > m_rect.Height();
		}

		if (sizeImageLarge != CSize(0, 0) && !bIsSmallIcon)
		{
			m_bIsLargeImage = TRUE;
		}
	}

	if (m_bIsLargeImage)
	{
		SetMargin(CSize(nLargeButtonMarginX, nLargeButtonMarginY));
	}
	else if (m_szMargin == CSize(nLargeButtonMarginX, nLargeButtonMarginY))
	{
		SetMargin(CSize(nSmallButtonMarginX, nSmallButtonMarginY));
	}
}

void CMFCRibbonButton::FillWindowList()
{
	if (m_nWindowsMenuItems > 0)
	{
		for (int i = 0; i < m_nWindowsMenuItems; i++)
		{
			int nIndex = (int) m_arSubItems.GetSize() - 1;

			delete m_arSubItems [nIndex];
			m_arSubItems.RemoveAt(nIndex);
		}
	}

	m_nWindowsMenuItems = 0;

	CMFCRibbonBar* pTopLevelRibbon = GetTopLevelRibbonBar();
	if (pTopLevelRibbon == NULL)
	{
		return;
	}

	CMDIFrameWndEx* pMDIFrameWnd = DYNAMIC_DOWNCAST(CMDIFrameWndEx, pTopLevelRibbon->GetTopLevelFrame());
	if (pMDIFrameWnd == NULL)
	{
		return;
	}

	const int nMaxFiles = 9;

	HWND hwndT = ::GetWindow(pMDIFrameWnd->m_hWndMDIClient, GW_CHILD);
	int i = 0;

	for (i = 0; hwndT != NULL && i < nMaxFiles; i++)
	{
		CMDIChildWndEx* pFrame = DYNAMIC_DOWNCAST(CMDIChildWndEx, CWnd::FromHandle(hwndT));
		if (pFrame == NULL)
		{
			hwndT = ::GetWindow(hwndT,GW_HWNDNEXT);
			continue;
		}

		if (!pFrame->CanShowOnWindowsList())
		{
			hwndT = ::GetWindow(hwndT,GW_HWNDNEXT);
			continue;
		}

		if (i == 0)
		{
			CMFCRibbonSeparator* pSeparator = new CMFCRibbonSeparator(TRUE);
			pSeparator->SetDefaultMenuLook();

			AddSubItem(pSeparator);
			m_nWindowsMenuItems = 1;
		}

		TCHAR szWndTitle[256];
		::GetWindowText(hwndT,szWndTitle,sizeof(szWndTitle)/sizeof(szWndTitle[0]));

		CString strItem;
		strItem.Format(_T("&%d %s"), i + 1, szWndTitle);

		CMFCRibbonButton* pItem = new CMFCRibbonButton(AFX_IDM_FIRST_MDICHILD, strItem);
		pItem->SetData((DWORD_PTR) hwndT);
		pItem->SetDefaultMenuLook();
		pItem->m_pRibbonBar = m_pRibbonBar;

		AddSubItem(pItem);

		hwndT = ::GetWindow(hwndT,GW_HWNDNEXT);
		m_nWindowsMenuItems++;
	}

	if (pMDIFrameWnd->m_uiWindowsDlgMenuId != 0 && (i == nMaxFiles || pMDIFrameWnd->m_bShowWindowsDlgAlways))
	{
		//-------------------------
		// Add "Windows..." dialog:
		//-------------------------
		CMFCRibbonButton* pItem = new CMFCRibbonButton(pMDIFrameWnd->m_uiWindowsDlgMenuId, pMDIFrameWnd->m_strWindowsDlgMenuText);
		pItem->SetDefaultMenuLook();
		pItem->m_pRibbonBar = m_pRibbonBar;

		AddSubItem(pItem);
		m_nWindowsMenuItems++;
	}
}

int CMFCRibbonButton::GetGroupButtonExtraWidth()
{
	if (m_pParentGroup == NULL)
	{
		return 0;
	}

	ASSERT_VALID(m_pParentGroup);

	switch(m_pParentGroup->GetCount())
	{
	case 1:
		return 2;

	case 2:
		if (m_Location != RibbonElementFirstInGroup)
		{
			return 0;
		}
		break;
	}

	return m_Location == RibbonElementFirstInGroup || m_Location == RibbonElementLastInGroup ? 1 : 2;
}

CSize CMFCRibbonButton::DrawBottomText(CDC* pDC, BOOL bCalcOnly)
{
	ASSERT_VALID(this);

	if (m_pParent == NULL)
	{
		return CSize(0, 0);
	}

	if (m_strText.IsEmpty())
	{
		return CSize(0, 0);
	}

	ASSERT_VALID(m_pParent);

	const CSize sizeImageLarge = m_pParent->GetImageSize(TRUE);
	if (sizeImageLarge == CSize(0, 0))
	{
		ASSERT(FALSE);
		return CSize(0, 0);
	}

	CSize sizeText = pDC->GetTextExtent(m_strText);

	const int nTextLineHeight = sizeText.cy;
	int nMenuArrowWidth = (HasMenu() || IsDefaultPanelButton()) ?(CMenuImages::Size().cx) : 0;

	if (nMenuArrowWidth != NULL && afxGlobalData.GetRibbonImageScale() > 1.)
	{
		nMenuArrowWidth = (int)(.5 + afxGlobalData.GetRibbonImageScale() * nMenuArrowWidth);
	}

	if (bCalcOnly)
	{
		const CString strDummyAmpSeq = _T("\001\001");

		m_nWrapIndex = -1;
		int nTextWidth = 0;

		if (m_arWordIndexes.GetSize() == 0) // 1 word
		{
			nTextWidth = sizeText.cx;
		}
		else
		{
			nTextWidth = 32767;

			for (int i = 0; i < m_arWordIndexes.GetSize(); i++)
			{
				int nIndex = m_arWordIndexes [i];

				CString strLineOne = m_strText.Left(nIndex);

				if (!IsDefaultPanelButton())
				{
					strLineOne.Replace(_T("&&"), strDummyAmpSeq);
					strLineOne.Remove(_T('&'));
					strLineOne.Replace(strDummyAmpSeq, _T("&"));
				}

				const int cx1 = pDC->GetTextExtent(strLineOne).cx;

				CString strLineTwo = m_strText.Mid(nIndex + 1);

				if (!IsDefaultPanelButton())
				{
					strLineTwo.Replace(_T("&&"), strDummyAmpSeq);
					strLineTwo.Remove(_T('&'));
					strLineTwo.Replace(strDummyAmpSeq, _T("&"));
				}

				const int cx2 = pDC->GetTextExtent(strLineTwo).cx + nMenuArrowWidth;

				int nWidth = max(cx1, cx2);

				if (nWidth < nTextWidth)
				{
					nTextWidth = nWidth;
					m_nWrapIndex = nIndex;
				}
			}
		}

		if (nTextWidth % 2)
		{
			nTextWidth--;
		}

		CSize size(nTextWidth, nTextLineHeight * 2);
		return size;
	}

	int y = m_rect.top + nLargeButtonMarginY + sizeImageLarge.cy + 5;
	CRect rectMenuArrow(0, 0, 0, 0);

	if (IsDefaultPanelButton())
	{
		y += nDefaultPaneButtonMargin;
	}

	CRect rectText = m_rect;
	rectText.top = y;

	UINT uiDTFlags = DT_SINGLELINE | DT_CENTER;
	if (IsDefaultPanelButton())
	{
		uiDTFlags |= DT_NOPREFIX;
	}

	if (m_nWrapIndex == -1)
	{
		// Single line text
		pDC->DrawText(m_strText, rectText, uiDTFlags);

		if (HasMenu() || IsDefaultPanelButton())
		{
			rectMenuArrow = m_rect;

			rectMenuArrow.top = y + nTextLineHeight + 2;
			rectMenuArrow.left = m_rect.CenterPoint().x - CMenuImages::Size().cx / 2 - 1;
		}
	}
	else
	{
		CString strLineOne = m_strText.Left(m_nWrapIndex);
		pDC->DrawText(strLineOne, rectText, uiDTFlags);

		rectText.top = y + nTextLineHeight;
		rectText.right -= nMenuArrowWidth;

		CString strLineTwo = m_strText.Mid(m_nWrapIndex + 1);
		pDC->DrawText(strLineTwo, rectText, uiDTFlags);

		if (HasMenu() || IsDefaultPanelButton())
		{
			rectMenuArrow = rectText;

			rectMenuArrow.top += 2;
			rectMenuArrow.left = rectText.right -(rectText.Width() - pDC->GetTextExtent(strLineTwo).cx) / 2;
		}
	}

	if (!rectMenuArrow.IsRectEmpty())
	{
		int nMenuArrowHeight = CMenuImages::Size().cy;

		if (afxGlobalData.GetRibbonImageScale() > 1.)
		{
			nMenuArrowHeight = (int)(.5 + afxGlobalData.GetRibbonImageScale() * nMenuArrowHeight);
		}

		rectMenuArrow.bottom = rectMenuArrow.top + nMenuArrowHeight;
		rectMenuArrow.right = rectMenuArrow.left + nMenuArrowWidth;

		CRect rectWhite = rectMenuArrow;
		rectWhite.OffsetRect(0, 1);

		CMenuImages::IMAGES_IDS id = afxGlobalData.GetRibbonImageScale() > 1. ? CMenuImages::IdArrowDownLarge : CMenuImages::IdArrowDown;

		CMenuImages::Draw(pDC, id, rectWhite, CMenuImages::ImageWhite);
		CMenuImages::Draw(pDC, id, rectMenuArrow, m_bIsDisabled ? CMenuImages::ImageGray : CMenuImages::ImageBlack);
	}

	return CSize(0, 0);
}



