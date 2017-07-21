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

#include "afxmenuhash.h"
#include "afxcontrolbarutil.h"
#include "afxtoolbarmenubutton.h"
#include "afxmenubar.h"
#include "afxpopupmenubar.h"
#include "afxcommandmanager.h"
#include "afxglobals.h"
#include "afxkeyboardmanager.h"

#include "afxframewndex.h"
#include "afxmdiframewndex.h"
#include "afxoleipframewndex.h"
#include "afxoledocipframewndex.h"

#include "afxmenuimages.h"
#include "afxusertoolsmanager.h"
#include "afxmenutearoffmanager.h"
#include "afxusertool.h"
#include "afxsettingsstore.h"
#include "afxvisualmanager.h"
#include "afxribbonres.h"

#include "afxtabctrl.h"
#include "afxdropdownlistbox.h"
#include "afxbaseribbonelement.h"
#include "afxribbonbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CMFCToolBarMenuButton, CMFCToolBarButton, VERSIONABLE_SCHEMA | 1)

BOOL CMFCToolBarMenuButton::m_bAlwaysCallOwnerDraw = FALSE;

// Construction/Destruction
CMFCToolBarMenuButton::CMFCToolBarMenuButton()
{
	Initialize();
}

CMFCToolBarMenuButton::CMFCToolBarMenuButton(UINT uiID, HMENU hMenu, int iImage, LPCTSTR lpszText, BOOL bUserButton)
{
	Initialize(uiID, hMenu, iImage, lpszText, bUserButton);
}

void CMFCToolBarMenuButton::Initialize()
{
	m_bDrawDownArrow = FALSE;
	m_bMenuMode = FALSE;
	m_pPopupMenu = NULL;
	m_bDefault = FALSE;
	m_bClickedOnMenu = FALSE;
	m_bHorz = TRUE;
	m_bMenuOnly = FALSE; //JRG
	m_bToBeClosed = FALSE;
	m_uiTearOffBarID = 0;
	m_bIsRadio = FALSE;
	m_pWndMessage = NULL;
	m_bMenuPaletteMode = FALSE;
	m_nPaletteRows = 1;
	m_bQuickCustomMode = FALSE;
	m_bShowAtRightSide = FALSE;
	m_rectArrow.SetRectEmpty();
	m_rectButton.SetRectEmpty();
}

void CMFCToolBarMenuButton::Initialize(UINT uiID, HMENU hMenu, int iImage, LPCTSTR lpszText, BOOL bUserButton)
{
	Initialize();

	m_nID = uiID;
	m_bUserButton = bUserButton;

	SetImage(iImage);
	m_strText = (lpszText == NULL) ? _T("") : lpszText;

	CreateFromMenu(hMenu);
}

CMFCToolBarMenuButton::CMFCToolBarMenuButton(const CMFCToolBarMenuButton& src)
{
	m_nID = src.m_nID;
	m_nStyle = src.m_nStyle;
	m_bUserButton = src.m_bUserButton;

	SetImage(src.GetImage());
	m_strText = src.m_strText;
	m_bDragFromCollection = FALSE;
	m_bText = src.m_bText;
	m_bImage = src.m_bImage;
	m_bDrawDownArrow = src.m_bDrawDownArrow;
	m_bMenuMode = src.m_bMenuMode;
	m_bDefault = src.m_bDefault;
	m_bMenuOnly = src.m_bMenuOnly;
	m_bIsRadio = src.m_bIsRadio;

	SetTearOff(src.m_uiTearOffBarID);

	HMENU hmenu = src.CreateMenu();
	ENSURE(hmenu != NULL);

	CreateFromMenu(hmenu);
	::DestroyMenu(hmenu);

	m_rect.SetRectEmpty();

	m_pPopupMenu = NULL;
	m_pWndParent = NULL;

	m_bClickedOnMenu = FALSE;
	m_bHorz = TRUE;

	m_bMenuPaletteMode = src.m_bMenuPaletteMode;
	m_nPaletteRows = src.m_nPaletteRows;
	m_bQuickCustomMode = src.m_bQuickCustomMode;
	m_bShowAtRightSide = src.m_bShowAtRightSide;
}

CMFCToolBarMenuButton::~CMFCToolBarMenuButton()
{
	if (m_pPopupMenu != NULL)
	{
		m_pPopupMenu->m_pParentBtn = NULL;
	}

	while (!m_listCommands.IsEmpty())
	{
		delete m_listCommands.RemoveHead();
	}

	if (m_uiTearOffBarID != 0 && g_pTearOffMenuManager != NULL)
	{
		g_pTearOffMenuManager->SetInUse(m_uiTearOffBarID, FALSE);
	}
}

//////////////////////////////////////////////////////////////////////
// Overrides:

void CMFCToolBarMenuButton::CopyFrom(const CMFCToolBarButton& s)
{
	CMFCToolBarButton::CopyFrom(s);

	const CMFCToolBarMenuButton& src = (const CMFCToolBarMenuButton&) s;

	m_bDefault = src.m_bDefault;
	m_bMenuOnly = src.m_bMenuOnly;
	m_bIsRadio = src.m_bIsRadio;
	m_pWndMessage = src.m_pWndMessage;
	m_bMenuPaletteMode = src.m_bMenuPaletteMode;
	m_nPaletteRows = src.m_nPaletteRows;
	m_bQuickCustomMode = src.m_bQuickCustomMode;
	m_bShowAtRightSide = src.m_bShowAtRightSide;

	SetTearOff(src.m_uiTearOffBarID);

	while (!m_listCommands.IsEmpty())
	{
		delete m_listCommands.RemoveHead();
	}

	for (POSITION pos = src.m_listCommands.GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarMenuButton* pItem = (CMFCToolBarMenuButton*) src.m_listCommands.GetNext(pos);
		ENSURE(pItem != NULL);
		ASSERT_VALID(pItem);
		ASSERT_KINDOF(CMFCToolBarMenuButton, pItem);

		CRuntimeClass* pSrcClass = pItem->GetRuntimeClass();
		ENSURE(pSrcClass != NULL);

		CMFCToolBarMenuButton* pNewItem = (CMFCToolBarMenuButton*) pSrcClass->CreateObject();
		ENSURE(pNewItem != NULL);
		ASSERT_VALID(pNewItem);
		ASSERT_KINDOF(CMFCToolBarMenuButton, pNewItem);

		pNewItem->CopyFrom(*pItem);
		m_listCommands.AddTail(pNewItem);
	}
}

void CMFCToolBarMenuButton::Serialize(CArchive& ar)
{
	CMFCToolBarButton::Serialize(ar);

	if (ar.IsLoading())
	{
		while (!m_listCommands.IsEmpty())
		{
			delete m_listCommands.RemoveHead();
		}

		UINT uiTearOffBarID;
		ar >> uiTearOffBarID;

		SetTearOff(uiTearOffBarID);

		ar >> m_bMenuPaletteMode;
		ar >> m_nPaletteRows;
	}
	else
	{
		ar << m_uiTearOffBarID;

		ar << m_bMenuPaletteMode;
		ar << m_nPaletteRows;
	}

	m_listCommands.Serialize(ar);
}

void CMFCToolBarMenuButton::OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages, BOOL bHorz, BOOL bCustomizeMode, BOOL bHighlight, BOOL bDrawBorder, BOOL bGrayDisabledButtons)
{
	m_rectArrow.SetRectEmpty();
	m_rectButton.SetRectEmpty();

	if (m_bMenuMode)
	{
		DrawMenuItem(pDC, rect, pImages, bCustomizeMode, bHighlight, bGrayDisabledButtons);
		return;
	}

	BOOL bIsFlatLook = CMFCVisualManager::GetInstance()->IsMenuFlatLook();

	const int nSeparatorSize = 2;

	if (m_bMenuPaletteMode)
	{
		m_nStyle &= ~TBBS_CHECKED;
	}

	//----------------------
	// Fill button interior:
	//----------------------
	FillInterior(pDC, rect, bHighlight || IsDroppedDown());

	CSize sizeImage = CMenuImages::Size();
	if (CMFCToolBar::IsLargeIcons())
	{
		sizeImage.cx *= 2;
		sizeImage.cy *= 2;
	}

	CRect rectInternal = rect;
	CSize sizeExtra = m_bExtraSize ? CMFCVisualManager::GetInstance()->GetButtonExtraBorder() : CSize(0, 0);

	if (sizeExtra != CSize(0, 0))
	{
		rectInternal.DeflateRect(sizeExtra.cx / 2 + 1, sizeExtra.cy / 2 + 1);
	}

	CRect rectParent = rect;
	m_rectArrow = rectInternal;

	const int nMargin = CMFCVisualManager::GetInstance()->GetMenuImageMargin();
	const int nXMargin = bHorz ? nMargin : 0;
	const int nYMargin = bHorz ? 0 : nMargin;

	rectParent.DeflateRect(nXMargin, nYMargin);

	if (m_bDrawDownArrow)
	{
		if (bHorz)
		{
			rectParent.right -= sizeImage.cx + nSeparatorSize - 2 + sizeExtra.cx;
			m_rectArrow.left = rectParent.right + 1;

			if (sizeExtra != CSize(0, 0))
			{
				m_rectArrow.OffsetRect(-sizeExtra.cx / 2 + 1, -sizeExtra.cy / 2 + 1);
			}
		}
		else
		{
			rectParent.bottom -= sizeImage.cy + nSeparatorSize - 1;
			m_rectArrow.top = rectParent.bottom;
		}
	}

	UINT uiStyle = m_nStyle;

	if (bIsFlatLook)
	{
		m_nStyle &= ~(TBBS_PRESSED | TBBS_CHECKED);
	}
	else
	{
		if (m_bClickedOnMenu && m_nID != 0 && m_nID != (UINT) -1 && !m_bMenuOnly)
		{
			m_nStyle &= ~TBBS_PRESSED;
		}
		else if (m_pPopupMenu != NULL)
		{
			m_nStyle |= TBBS_PRESSED;
		}
	}

	BOOL bDisableFill = m_bDisableFill;
	m_bDisableFill = TRUE;

	CMFCToolBarButton::OnDraw(pDC, rectParent, pImages, bHorz, bCustomizeMode, bHighlight, bDrawBorder, bGrayDisabledButtons);

	m_bDisableFill = bDisableFill;

	if (m_bDrawDownArrow)
	{
		if ((m_nStyle &(TBBS_PRESSED | TBBS_CHECKED)) && !bIsFlatLook)
		{
			m_rectArrow.OffsetRect(1, 1);
		}

		if ((bHighlight ||(m_nStyle & TBBS_PRESSED) || m_pPopupMenu != NULL) && m_nID != 0 && m_nID != (UINT) -1 && !m_bMenuOnly)
		{
			//----------------
			// Draw separator:
			//----------------
			CRect rectSeparator = m_rectArrow;

			if (bHorz)
			{
				rectSeparator.right = rectSeparator.left + nSeparatorSize;
			}
			else
			{
				rectSeparator.bottom = rectSeparator.top + nSeparatorSize;
			}

			CMFCVisualManager::AFX_BUTTON_STATE state = CMFCVisualManager::ButtonsIsRegular;

			if (bHighlight ||(m_nStyle &(TBBS_PRESSED | TBBS_CHECKED)))
			{
				//-----------------------
				// Pressed in or checked:
				//-----------------------
				state = CMFCVisualManager::ButtonsIsPressed;
			}

			if (!m_bClickedOnMenu)
			{
				CMFCVisualManager::GetInstance()->OnDrawButtonSeparator(pDC, this, rectSeparator, state, bHorz);
			}
		}

		BOOL bDisabled = (bCustomizeMode && !IsEditable()) || (!bCustomizeMode &&(m_nStyle & TBBS_DISABLED));

		int iImage;
		if (bHorz && !m_bMenuOnly)
		{
			iImage = CMenuImages::IdArrowDown;
		}
		else
		{
			iImage = CMenuImages::IdArrowRight;
		}

		CMenuImages::Draw(pDC, (CMenuImages::IMAGES_IDS) iImage, m_rectArrow, bDisabled ? CMenuImages::ImageGray : CMenuImages::ImageBlack, sizeImage);
	}

	m_nStyle = uiStyle;

	if (!bCustomizeMode)
	{
		if ((m_nStyle &(TBBS_PRESSED | TBBS_CHECKED)) || m_pPopupMenu != NULL)
		{
			//-----------------------
			// Pressed in or checked:
			//-----------------------
			if (!bIsFlatLook && m_bClickedOnMenu && m_nID != 0 && m_nID != (UINT) -1 && !m_bMenuOnly) //JRG
			{
				rectParent.right++;

				CMFCVisualManager::GetInstance()->OnDrawButtonBorder(pDC, this, rectParent, CMFCVisualManager::ButtonsIsHighlighted);
				CMFCVisualManager::GetInstance()->OnDrawButtonBorder(pDC, this, m_rectArrow, CMFCVisualManager::ButtonsIsPressed);
			}
			else
			{
				CMFCVisualManager::GetInstance()->OnDrawButtonBorder(pDC, this, rect, CMFCVisualManager::ButtonsIsPressed);
			}
		}
		else if (bHighlight && !(m_nStyle & TBBS_DISABLED) && !(m_nStyle &(TBBS_CHECKED | TBBS_INDETERMINATE)))
		{
			CMFCVisualManager::GetInstance()->OnDrawButtonBorder(pDC, this, rect, CMFCVisualManager::ButtonsIsHighlighted);
		}
	}
}

SIZE CMFCToolBarMenuButton::OnCalculateSize(CDC* pDC, const CSize& sizeDefault, BOOL bHorz)
{
	m_bHorz = bHorz;

	if (!IsVisible())
	{
		return CSize(0,0);
	}

	int nArrowSize = 0;
	const int nSeparatorSize = 2;

	if (m_bDrawDownArrow || m_bMenuMode)
	{
		if (m_bMenuMode)
		{
			nArrowSize = (bHorz) ? afxGlobalData.GetTextWidth() : afxGlobalData.GetTextHeight();
		}
		else
		{
			nArrowSize = (bHorz) ? CMenuImages::Size().cx : CMenuImages::Size().cy;

			if (CMFCToolBar::IsLargeIcons())
			{
				nArrowSize *= 2;
			}
		}

		nArrowSize += nSeparatorSize - AFX_TEXT_MARGIN - 1;
	}

	//--------------------
	// Change accelerator:
	//--------------------
	if (afxKeyboardManager != NULL && m_bMenuMode && (m_nID < 0xF000 || m_nID >= 0xF1F0)) // Not system.
	{
		//-----------------------------------
		// Remove standard aceleration label:
		//-----------------------------------
		int iTabOffset = m_strText.Find(_T('\t'));
		if (iTabOffset >= 0)
		{
			m_strText = m_strText.Left(iTabOffset);
		}

		//---------------------------------
		// Add an actual accelartion label:
		//---------------------------------
		CString strAccel;
		CFrameWnd* pParent = m_pWndParent == NULL ? DYNAMIC_DOWNCAST(CFrameWnd, AfxGetMainWnd()) :
		AFXGetTopLevelFrame(m_pWndParent);

		if (pParent != NULL && (CKeyboardManager::FindDefaultAccelerator(m_nID, strAccel, pParent, TRUE) ||
			CKeyboardManager::FindDefaultAccelerator(m_nID, strAccel, pParent->GetActiveFrame(), FALSE)))
		{
			m_strText += _T('\t');
			m_strText += strAccel;
		}
	}

	CFont* pOldFont = NULL;

	if (m_nID == AFX_MENU_GROUP_ID)
	{
		pOldFont = pDC->SelectObject(&afxGlobalData.fontBold);
		ASSERT_VALID(pOldFont);
	}

	CSize size = CMFCToolBarButton::OnCalculateSize(pDC, sizeDefault, bHorz);

	if (pOldFont != NULL)
	{
		pDC->SelectObject(pOldFont);
	}

	CMFCPopupMenuBar* pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenuBar, m_pWndParent);

	if (pParentMenu != NULL)
	{
		size.cy = pParentMenu->GetRowHeight();

		if (pParentMenu->IsDropDownListMode())
		{
			CMFCDropDownListBox* pList = DYNAMIC_DOWNCAST(CMFCDropDownListBox, pParentMenu->GetParent());

			if (pList != NULL)
			{
				return pList->OnGetItemSize(pDC, this, size);
			}
		}
	}

	if (bHorz)
	{
		size.cx += nArrowSize;
	}
	else
	{
		size.cy += nArrowSize;
	}

	if (m_bMenuMode)
	{
		size.cx += sizeDefault.cx + 2 * AFX_TEXT_MARGIN;
	}

	if (!m_bMenuMode)
	{
		const int nMargin = CMFCVisualManager::GetInstance()->GetMenuImageMargin();

		if (bHorz)
		{
			size.cx += nMargin * 2;
		}
		else
		{
			size.cy += nMargin * 2;
		}
	}

	return size;
}

BOOL CMFCToolBarMenuButton::OnClick(CWnd* pWnd, BOOL bDelay)
{
	ASSERT_VALID(pWnd);

	m_bClickedOnMenu = FALSE;

	if (m_bDrawDownArrow && !bDelay && !m_bMenuMode)
	{
		if (m_nID == 0 || m_nID == (UINT) -1)
		{
			m_bClickedOnMenu = TRUE;
		}
		else
		{
			CPoint ptMouse;
			::GetCursorPos(&ptMouse);
			pWnd->ScreenToClient(&ptMouse);

			m_bClickedOnMenu = m_rectArrow.PtInRect(ptMouse);
			if (!m_bClickedOnMenu)
			{
				return FALSE;
			}
		}
	}

	if (HasButton() && !bDelay)
	{
		CPoint ptMouse;
		::GetCursorPos(&ptMouse);
		pWnd->ScreenToClient(&ptMouse);

		if (m_rectButton.PtInRect(ptMouse))
		{
			return FALSE;
		}
	}

	if (!m_bClickedOnMenu && m_nID > 0 && m_nID != (UINT) -1 && !m_bDrawDownArrow && !m_bMenuOnly)
	{
		return FALSE;
	}

	CMFCMenuBar* pMenuBar = DYNAMIC_DOWNCAST(CMFCMenuBar, m_pWndParent);

	if (m_pPopupMenu != NULL)
	{
		//-----------------------------------------------------
		// Second click to the popup menu item closes the menu:
		//-----------------------------------------------------
		ASSERT_VALID(m_pPopupMenu);

		m_pPopupMenu->m_bAutoDestroyParent = FALSE;
		m_pPopupMenu->DestroyWindow();
		m_pPopupMenu = NULL;

		if (pMenuBar != NULL)
		{
			pMenuBar->SetHot(NULL);
		}
	}
	else
	{
		CMFCPopupMenuBar* pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenuBar, m_pWndParent);

		if (bDelay && pParentMenu != NULL && !CMFCToolBar::IsCustomizeMode())
		{
			pParentMenu->StartPopupMenuTimer(this);
		}
		else
		{
			if (pMenuBar != NULL)
			{
				CMFCToolBarMenuButton* pCurrPopupMenuButton = pMenuBar->GetDroppedDownMenu();
				if (pCurrPopupMenuButton != NULL)
				{
					pCurrPopupMenuButton->OnCancelMode();
				}
			}

			if (!OpenPopupMenu(pWnd))
			{
				return FALSE;
			}
		}

		if (pMenuBar != NULL)
		{
			pMenuBar->SetHot(this);
		}
	}

	if (m_pWndParent != NULL)
	{
		CRect rect = m_rect;

		const int nShadowSize = CMFCVisualManager::GetInstance()->GetMenuShadowDepth();

		rect.InflateRect(nShadowSize, nShadowSize);
		m_pWndParent->RedrawWindow(rect, NULL, RDW_FRAME | RDW_INVALIDATE);
	}

	return TRUE;
}

void CMFCToolBarMenuButton::OnChangeParentWnd(CWnd* pWndParent)
{
	CMFCToolBarButton::OnChangeParentWnd(pWndParent);

	if (pWndParent != NULL)
	{
		if (pWndParent->IsKindOf(RUNTIME_CLASS(CMFCMenuBar)))
		{
			m_bDrawDownArrow = (m_nID != 0 && !m_listCommands.IsEmpty()) || ((CMFCMenuBar *)pWndParent)->GetForceDownArrows();
			m_bText = TRUE;
			m_bImage = FALSE;
		}
		else
		{
			m_bDrawDownArrow = (m_nID == 0 || !m_listCommands.IsEmpty());
		}

		if (pWndParent->IsKindOf(RUNTIME_CLASS(CMFCPopupMenuBar)))
		{
			m_bMenuMode = TRUE;
			m_bText = TRUE;
			m_bImage = FALSE;
			m_bDrawDownArrow = (m_nID == 0 || !m_listCommands.IsEmpty()) || HasButton();
		}
		else
		{
			m_bMenuMode = FALSE;
		}
	}
}

void CMFCToolBarMenuButton::CreateFromMenu(HMENU hMenu)
{
	while (!m_listCommands.IsEmpty())
	{
		delete m_listCommands.RemoveHead();
	}

	if (!::IsMenu(hMenu))
	{
		return;
	}

	CMenu* pMenu = CMenu::FromHandle(hMenu);
	if (pMenu == NULL)
	{
		return;
	}

	UINT uiDefaultCmd = ::GetMenuDefaultItem(hMenu, FALSE, GMDI_USEDISABLED);

	int iCount = (int) pMenu->GetMenuItemCount();
	for (int i = 0; i < iCount; i ++)
	{
		CMFCToolBarMenuButton* pItem = STATIC_DOWNCAST(CMFCToolBarMenuButton, GetRuntimeClass()->CreateObject());
		ASSERT_VALID(pItem);

		pItem->m_nID = pMenu->GetMenuItemID(i);
		pMenu->GetMenuString(i, pItem->m_strText, MF_BYPOSITION);

		if (pItem->m_nID == -1) // Sub-menu...
		{
			if (g_pTearOffMenuManager != NULL)
			{
				pItem->SetTearOff(g_pTearOffMenuManager->Parse(pItem->m_strText));
			}

			CMenu* pSubMenu = pMenu->GetSubMenu(i);
			pItem->CreateFromMenu(pSubMenu->GetSafeHmenu());
		}
		else if (pItem->m_nID == uiDefaultCmd)
		{
			pItem->m_bDefault = TRUE;
		}

		UINT uiState = pMenu->GetMenuState(i, MF_BYPOSITION);

		if (uiState & MF_MENUBREAK)
		{
			pItem->m_nStyle |= AFX_TBBS_BREAK;
		}

		if ((uiState & MF_DISABLED) ||(uiState & MF_GRAYED))
		{
			pItem->m_nStyle |= TBBS_DISABLED;
		}

		m_listCommands.AddTail(pItem);
	}
}

HMENU CMFCToolBarMenuButton::CreateMenu() const
{
	if (m_listCommands.IsEmpty() && m_nID != (UINT) -1 && m_nID != 0 && !m_bMenuOnly)
	{
		return NULL;
	}

	CMenu menu;
	if (!menu.CreatePopupMenu())
	{
		TRACE(_T("CMFCToolBarMenuButton::CreateMenu(): Can't create popup menu!\n"));
		return NULL;
	}

	BOOL bRes = TRUE;
	DWORD dwLastError = 0;

	UINT uiDefaultCmd = (UINT) -1;

	int i = 0;
	for (POSITION pos = m_listCommands.GetHeadPosition(); pos != NULL; i ++)
	{
		CMFCToolBarMenuButton* pItem = (CMFCToolBarMenuButton*) m_listCommands.GetNext(pos);
		ENSURE(pItem != NULL);
		ASSERT_VALID(pItem);
		ASSERT_KINDOF(CMFCToolBarMenuButton, pItem);

		UINT uiStyle = MF_STRING;

		if (pItem->m_nStyle & AFX_TBBS_BREAK)
		{
			uiStyle |= MF_MENUBREAK;
		}

		if (pItem->m_nStyle & TBBS_DISABLED)
		{
			uiStyle |= MF_DISABLED;
		}


		if (pItem->IsTearOffMenu())
		{
			uiStyle |= MF_MENUBARBREAK;
		}

		switch(pItem->m_nID)
		{
		case 0: // Separator
			bRes = menu.AppendMenu(MF_SEPARATOR);
			if (!bRes)
			{
				dwLastError = GetLastError();
			}
			break;

		case -1: // Sub-menu
			{
				HMENU hSubMenu = pItem->CreateMenu();
				ENSURE(hSubMenu != NULL);

				CString strText = pItem->m_strText;
				if (pItem->m_uiTearOffBarID != 0 && g_pTearOffMenuManager != NULL)
				{
					g_pTearOffMenuManager->Build(pItem->m_uiTearOffBarID, strText);
				}

				bRes = menu.AppendMenu(uiStyle | MF_POPUP, (UINT_PTR) hSubMenu, strText);
				if (!bRes)
				{
					dwLastError = GetLastError();
				}
			}
			break;

		default:
			if (pItem->m_bDefault)
			{
				uiDefaultCmd = pItem->m_nID;
			}

			bRes = menu.AppendMenu(uiStyle, pItem->m_nID, pItem->m_strText);
			if (!bRes)
			{
				dwLastError = GetLastError();
			}
		}

		if (!bRes)
		{
			TRACE(_T("CMFCToolBarMenuButton::CreateMenu(): Can't add menu item: %d\n Last error = %x\n"), pItem->m_nID, dwLastError);
			return NULL;
		}
	}

	HMENU hMenu = menu.Detach();
	if (uiDefaultCmd != (UINT)-1)
	{
		::SetMenuDefaultItem(hMenu, uiDefaultCmd, FALSE);
	}

	return hMenu;
}

void CMFCToolBarMenuButton::DrawMenuItem(CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages,
	BOOL bCustomizeMode, BOOL bHighlight, BOOL bGrayDisabledButtons, BOOL bContentOnly)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(this);

	if (m_nID == AFX_MENU_GROUP_ID)
	{
		COLORREF clrText = CMFCVisualManager::GetInstance()->OnDrawMenuLabel(pDC, rect);

		COLORREF clrTextOld = pDC->SetTextColor(clrText);

		CRect rectText = rect;
		rectText.DeflateRect(AFX_TEXT_MARGIN, 0);
		rectText.bottom -= 2;

		CFont* pOldFont = pDC->SelectObject(&afxGlobalData.fontBold);
		ASSERT_VALID(pOldFont);

		pDC->DrawText(m_strText, rectText, DT_SINGLELINE | DT_VCENTER);

		pDC->SetTextColor(clrTextOld);
		pDC->SelectObject(pOldFont);
		return;
	}

	BOOL bDisabled = (bCustomizeMode && !IsEditable()) || (!bCustomizeMode &&(m_nStyle & TBBS_DISABLED));

	CMFCToolBarImages* pLockedImages = NULL;
	CMFCToolBarImages* pUserImages = NULL;
	CAfxDrawState ds;

	CMFCPopupMenuBar* pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenuBar, m_pWndParent);

	CSize sizeMenuImage = CMFCToolBar::GetMenuImageSize();

	if (pParentMenu != NULL)
	{
		if (pParentMenu->IsDropDownListMode())
		{
			CMFCDropDownListBox* pList = DYNAMIC_DOWNCAST(CMFCDropDownListBox, pParentMenu->GetParent());

			if (pList != NULL)
			{
				COLORREF clrText = CMFCVisualManager::GetInstance()->GetMenuItemTextColor(this, bHighlight, FALSE);

				if (bHighlight)
				{
					CMFCVisualManager::GetInstance()-> OnHighlightMenuItem(pDC, this, rect, clrText);
				}

				COLORREF clrTextOld = pDC->SetTextColor(clrText);

				pList->OnDrawItem(pDC, this, bHighlight);

				pDC->SetTextColor(clrTextOld);
				return;
			}
		}

		if (pParentMenu->m_pRelatedToolbar != NULL && pParentMenu->m_pRelatedToolbar->IsLocked())
		{
			pLockedImages = (CMFCToolBarImages*) pParentMenu->m_pRelatedToolbar->GetLockedMenuImages();

			if (pLockedImages != NULL)
			{
				CSize sizeDest(0, 0);

				if (sizeMenuImage != pParentMenu->GetCurrentMenuImageSize())
				{
					sizeDest = sizeMenuImage;
				}

				pLockedImages->PrepareDrawImage(ds, sizeDest);

				pImages = pLockedImages;
			}
		}
	}

	BOOL bDisableImage = afxCommandManager->IsMenuItemWithoutImage(m_nID);
	if (m_nID == ID_AFXBARRES_TASKPANE_BACK || m_nID == ID_AFXBARRES_TASKPANE_FORWARD)
	{
		bDisableImage = TRUE;
	}

	CUserTool* pUserTool = NULL;
	if (afxUserToolsManager != NULL && !m_bUserButton)
	{
		pUserTool = afxUserToolsManager->FindTool(m_nID);
	}

	HICON hDocIcon = CMFCTabCtrl::GetDocumentIcon(m_nID);

	CSize sizeImage = CMenuImages::Size();

	if (m_pPopupMenu != NULL && !m_bToBeClosed)
	{
		bHighlight = TRUE;
	}

	COLORREF clrText = CMFCVisualManager::GetInstance()->GetMenuItemTextColor(this, bHighlight, bDisabled);

	BOOL bDrawImageFrame = !CMFCVisualManager::GetInstance()->IsHighlightWholeMenuItem();

	if (bHighlight && !bContentOnly && CMFCVisualManager::GetInstance()->IsHighlightWholeMenuItem())
	{
		CMFCVisualManager::GetInstance()->OnHighlightMenuItem(pDC, this, rect, clrText);
		bDrawImageFrame = FALSE;
	}

	if ((m_nStyle & TBBS_CHECKED) && !CMFCVisualManager::GetInstance()->IsOwnerDrawMenuCheck())
	{
		bDrawImageFrame = TRUE;
	}

	CFont* pOldFont = NULL;

	if (m_nID != 0 && m_nID != (UINT) -1 && !m_bMenuOnly && pParentMenu != NULL && pParentMenu->GetDefaultMenuId() == m_nID)
	{
		pOldFont = (CFont*) pDC->SelectObject(&afxGlobalData.fontBold);
	}

	CRect rectImage;
	rectImage = rect;
	rectImage.left += CMFCVisualManager::GetInstance()->GetMenuImageMargin();
	rectImage.right = rectImage.left + sizeMenuImage.cx + CMFCVisualManager::GetInstance()->GetMenuImageMargin();

	CRect rectFrameBtn = rectImage;

	if (CMFCVisualManager::GetInstance()->IsHighlightWholeMenuItem())
	{
		rectFrameBtn = rect;

		rectFrameBtn.left += 2;
		rectFrameBtn.top++;
		rectFrameBtn.bottom -= 2;
		rectFrameBtn.right = rectImage.right;
	}
	else
	{
		rectFrameBtn.InflateRect(1, -1);
	}

	BOOL bIsRarelyUsed = (CMFCMenuBar::IsRecentlyUsedMenus() && CMFCToolBar::IsCommandRarelyUsed(m_nID));

	if (bIsRarelyUsed)
	{
		bIsRarelyUsed = FALSE;

		CMFCPopupMenuBar* pParentMenuBar = DYNAMIC_DOWNCAST(CMFCPopupMenuBar, m_pWndParent);

		if (pParentMenuBar != NULL)
		{
			CMFCPopupMenu* pParentMenuCurr = DYNAMIC_DOWNCAST(CMFCPopupMenu, pParentMenuBar->GetParent());
			if (pParentMenuCurr != NULL && pParentMenuCurr->HideRarelyUsedCommands())
			{
				bIsRarelyUsed = TRUE;
			}
		}
	}

	BOOL bLightImage = FALSE;
	BOOL bFadeImage = !bHighlight && CMFCVisualManager::GetInstance()->IsFadeInactiveImage();

	if (bIsRarelyUsed)
	{
		bLightImage = TRUE;
		if (bHighlight &&(m_nStyle &(TBBS_CHECKED | TBBS_INDETERMINATE)))
		{
			bLightImage = FALSE;
		}

		if (GetImage() < 0 && !(m_nStyle &(TBBS_CHECKED | TBBS_INDETERMINATE)))
		{
			bLightImage = FALSE;
		}
	}
	else if (m_nStyle &(TBBS_CHECKED | TBBS_INDETERMINATE))
	{
		bLightImage = !bHighlight;
	}

	//----------------
	// Draw the image:
	//----------------
	if (!IsDrawImage() && hDocIcon == NULL) // Try to find a matched image
	{
		BOOL bImageSave = m_bImage;
		BOOL bUserButton = m_bUserButton;
		BOOL bSuccess = TRUE;

		m_bImage = TRUE; // Always try to draw image!
		m_bUserButton = TRUE;

		if (GetImage() < 0)
		{
			m_bUserButton = FALSE;

			if (GetImage() < 0)
			{
				bSuccess = FALSE;
			}
		}

		if (!bSuccess)
		{
			m_bImage = bImageSave;
			m_bUserButton = bUserButton;
		}

		if (m_bUserButton && pImages != CMFCToolBar::GetUserImages())
		{
			pUserImages = CMFCToolBar::GetUserImages();

			if (pUserImages != NULL)
			{
				ASSERT_VALID(pUserImages);

				pUserImages->PrepareDrawImage(ds);
				pImages = pUserImages;
			}
		}
	}

	BOOL bImageIsReady = FALSE;

	CRgn rgnClip;
	rgnClip.CreateRectRgnIndirect(&rectImage);

	if (bDrawImageFrame && !bContentOnly)
	{
		FillInterior(pDC, rectFrameBtn, bHighlight);
	}

	if (!bDisableImage &&(IsDrawImage() && pImages != NULL) || hDocIcon != NULL)
	{
		BOOL bDrawImageShadow = bHighlight && !bCustomizeMode && CMFCVisualManager::GetInstance()->IsShadowHighlightedImage() &&
			!afxGlobalData.IsHighContrastMode() && ((m_nStyle & TBBS_CHECKED) == 0) && ((m_nStyle & TBBS_DISABLED) == 0);

		pDC->SelectObject(&rgnClip);

		CPoint ptImageOffset((rectImage.Width() - sizeMenuImage.cx) / 2, (rectImage.Height() - sizeMenuImage.cy) / 2);

		if ((m_nStyle & TBBS_PRESSED) || !(m_nStyle & TBBS_DISABLED) || !bGrayDisabledButtons || bCustomizeMode)
		{
			CRect rectIcon(CPoint(rectImage.left + ptImageOffset.x, rectImage.top + ptImageOffset.y), sizeMenuImage);

			if (hDocIcon != NULL)
			{
				DrawDocumentIcon(pDC, rectIcon, hDocIcon);
			}
			else if (pUserTool != NULL)
			{
				pUserTool->DrawToolIcon(pDC, rectIcon);
			}
			else
			{
				CPoint pt = rectImage.TopLeft();
				pt += ptImageOffset;

				if (bDrawImageShadow)
				{
					pt.Offset(1, 1);

					pImages->Draw(pDC, pt.x, pt.y, GetImage(), FALSE, FALSE, FALSE, TRUE);

					pt.Offset(-2, -2);
				}

				pImages->Draw(pDC, pt.x, pt.y, GetImage(), FALSE, bDisabled && bGrayDisabledButtons, FALSE, FALSE, bFadeImage);
			}

			bImageIsReady = TRUE;
		}

		if (!bImageIsReady)
		{
			CRect rectIcon(CPoint(rectImage.left + ptImageOffset.x, rectImage.top + ptImageOffset.y), sizeMenuImage);

			if (hDocIcon != NULL)
			{
				DrawDocumentIcon(pDC, rectIcon, hDocIcon);
			}
			else if (pUserTool != NULL)
			{
				pUserTool->DrawToolIcon(pDC, rectIcon);
			}
			else
			{
				if (bDrawImageShadow)
				{
					rectImage.OffsetRect(1, 1);
					pImages->Draw(pDC, rectImage.left + ptImageOffset.x, rectImage.top + ptImageOffset.y, GetImage(), FALSE, FALSE, FALSE, TRUE);

					rectImage.OffsetRect(-2, -2);
				}

				pImages->Draw(pDC, rectImage.left + ptImageOffset.x, rectImage.top + ptImageOffset.y, GetImage(), FALSE, bDisabled && bGrayDisabledButtons, FALSE, FALSE, bFadeImage);
			}

			bImageIsReady = TRUE;
		}
	}

	if (m_bAlwaysCallOwnerDraw || !bImageIsReady)
	{
		CFrameWnd* pParentFrame = m_pWndParent == NULL ? DYNAMIC_DOWNCAST(CFrameWnd, AfxGetMainWnd()) : AFXGetTopLevelFrame(m_pWndParent);

		//------------------------------------
		// Get chance to user draw menu image:
		//------------------------------------
		CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, pParentFrame);
		if (pMainFrame != NULL)
		{
			bImageIsReady = pMainFrame->OnDrawMenuImage(pDC, this, rectImage);
		}
		else // Maybe, SDI frame...
		{
			CFrameWndEx* pFrame = DYNAMIC_DOWNCAST(CFrameWndEx, pParentFrame);
			if (pFrame != NULL)
			{
				bImageIsReady = pFrame->OnDrawMenuImage(pDC, this, rectImage);
			}
			else // Maybe, OLE frame...
			{
				COleIPFrameWndEx* pOleFrame = DYNAMIC_DOWNCAST(COleIPFrameWndEx, pParentFrame);
				if (pOleFrame != NULL)
				{
					bImageIsReady = pOleFrame->OnDrawMenuImage(pDC, this, rectImage);
				}
				else
				{
					COleDocIPFrameWndEx* pOleDocFrame = DYNAMIC_DOWNCAST(COleDocIPFrameWndEx, pParentFrame);
					if (pOleDocFrame != NULL)
					{
						bImageIsReady = pOleDocFrame->OnDrawMenuImage(pDC, this, rectImage);
					}
				}
			}
		}
	}

	pDC->SelectClipRgn(NULL);

	if (m_nStyle & TBBS_CHECKED)
	{
		if (bDrawImageFrame)
		{
			UINT nStyleSaved = m_nStyle;

			if (bHighlight && CMFCVisualManager::GetInstance()->IsFrameMenuCheckedItems())
			{
				m_nStyle |= TBBS_MARKED;
			}

			CMFCVisualManager::GetInstance()->OnDrawButtonBorder(pDC, this, rectFrameBtn, CMFCVisualManager::ButtonsIsPressed);

			m_nStyle = nStyleSaved;
		}

		if (!bImageIsReady)
		{
			CMFCVisualManager::GetInstance()->OnDrawMenuCheck(pDC, this, rectFrameBtn, bHighlight, m_bIsRadio);
		}
	}
	else if (!bContentOnly && bImageIsReady && bHighlight && bDrawImageFrame)
	{
		CMFCVisualManager::GetInstance()->OnDrawButtonBorder(pDC, this, rectFrameBtn, CMFCVisualManager::ButtonsIsHighlighted);
	}

	rectImage.InflateRect(1, 0);
	int iSystemImageId = -1;

	//-------------------------------
	// Try to draw system menu icons:
	//-------------------------------
	if (!bImageIsReady)
	{
		switch(m_nID)
		{
		case SC_MINIMIZE:
			iSystemImageId = CMenuImages::IdMinimize;
			break;

		case SC_RESTORE:
			iSystemImageId = CMenuImages::IdRestore;
			break;

		case SC_CLOSE:
			iSystemImageId = CMenuImages::IdClose;
			break;

		case SC_MAXIMIZE:
			iSystemImageId = CMenuImages::IdMaximize;
			break;
		}

		if (iSystemImageId != -1)
		{
			CRect rectSysImage = rectImage;
			rectSysImage.DeflateRect(CMFCVisualManager::GetInstance()->GetMenuImageMargin(), CMFCVisualManager::GetInstance()->GetMenuImageMargin());

			if (!bContentOnly && bDrawImageFrame)
			{
				FillInterior(pDC, rectFrameBtn, bHighlight);
			}

			CMenuImages::Draw(pDC, (CMenuImages::IMAGES_IDS) iSystemImageId, rectSysImage, bDisabled ? CMenuImages::ImageGray : CMenuImages::ImageBlack);

			if (bHighlight && !bContentOnly && bDrawImageFrame)
			{
				CMFCVisualManager::GetInstance()->OnDrawButtonBorder(pDC, this, rectFrameBtn, CMFCVisualManager::ButtonsIsHighlighted);
			}
		}
	}

	//-------------------------------
	// Fill text area if highlighted:
	//-------------------------------
	CRect rectText = rect;
	rectText.left = rectFrameBtn.right + CMFCVisualManager::GetInstance()->GetMenuImageMargin() + 2;

	if (bHighlight)
	{
		if (!CMFCVisualManager::GetInstance()->IsHighlightWholeMenuItem())
		{
			CRect rectFill = rectFrameBtn;

			if ((m_nStyle &(TBBS_CHECKED) || bImageIsReady) || iSystemImageId != -1)
			{
				rectFill.left = rectText.left - 1;
			}

			rectFill.right = rect.right - 1;

			if (!bContentOnly)
			{
				CMFCVisualManager::GetInstance()->OnHighlightMenuItem(pDC, this, rectFill, clrText);
			}
			else
			{
				clrText = CMFCVisualManager::GetInstance()->GetHighlightedMenuItemTextColor(this);
			}
		}
		else if (bContentOnly)
		{
			clrText = CMFCVisualManager::GetInstance()->GetHighlightedMenuItemTextColor(this);
		}
	}

	//-------------------------
	// Find acceleration label:
	//-------------------------
	CString strText = m_strText;
	CString strAccel;

	int iTabOffset = m_strText.Find(_T('\t'));
	if (iTabOffset >= 0)
	{
		strText = strText.Left(iTabOffset);
		strAccel = m_strText.Mid(iTabOffset + 1);
	}

	//-----------
	// Draw text:
	//-----------
	COLORREF clrTextOld = pDC->GetTextColor();

	rectText.left += AFX_TEXT_MARGIN;

	if (!m_bWholeText)
	{
		CString strEllipses(_T("..."));
		while (strText.GetLength() > 0 && pDC->GetTextExtent(strText + strEllipses).cx > rectText.Width())
		{
			strText = strText.Left(strText.GetLength() - 1);
		}

		strText += strEllipses;
	}

	if (bDisabled && !bHighlight && CMFCVisualManager::GetInstance()->IsEmbossDisabledImage())
	{
		pDC->SetTextColor(afxGlobalData.clrBtnHilite);

		CRect rectShft = rectText;
		rectShft.OffsetRect(1, 1);
		pDC->DrawText(strText, &rectShft, DT_SINGLELINE | DT_VCENTER);
	}

	pDC->SetTextColor(clrText);
	pDC->DrawText(strText, &rectText, DT_SINGLELINE | DT_VCENTER);

	//------------------------
	// Draw accelerator label:
	//------------------------
	if (!strAccel.IsEmpty())
	{
		CRect rectAccel = rectText;
		rectAccel.right -= AFX_TEXT_MARGIN + sizeImage.cx;

		if (bDisabled && !bHighlight && CMFCVisualManager::GetInstance()->IsEmbossDisabledImage())
		{
			pDC->SetTextColor(afxGlobalData.clrBtnHilite);

			CRect rectAccelShft = rectAccel;
			rectAccelShft.OffsetRect(1, 1);
			pDC->DrawText(strAccel, &rectAccelShft, DT_SINGLELINE | DT_RIGHT | DT_VCENTER);
		}

		pDC->SetTextColor(clrText);
		pDC->DrawText(strAccel, &rectAccel, DT_SINGLELINE | DT_RIGHT | DT_VCENTER);
	}

	//--------------------------------------------
	// Draw triangle image for the cascade menues:
	//--------------------------------------------
	if (m_nID == (UINT) -1 || m_bDrawDownArrow || m_bMenuOnly)
	{
		CFont* pRegFont = pDC->SelectObject(&afxGlobalData.fontMarlett);
		ENSURE(pRegFont != NULL);

		CRect rectTriangle = rect;

		CString strTriangle = (m_pWndParent->GetExStyle() & WS_EX_LAYOUTRTL) ? _T("3") : _T("4"); // Marlett's right arrow

		if (m_bQuickCustomMode)
		{
			strTriangle = _T("6");  	// Marlett's down arrow
		}

		if (HasButton())
		{
			m_rectButton = rect;

			m_rectButton.left = m_rectButton.right - pDC->GetTextExtent(strTriangle).cx;

			CMFCVisualManager::GetInstance()->OnDrawMenuItemButton(pDC, this, m_rectButton, bHighlight, bDisabled);
		}

		pDC->DrawText(strTriangle, &rectTriangle, DT_SINGLELINE | DT_RIGHT | DT_VCENTER);

		pDC->SelectObject(pRegFont);
	}

	if (pOldFont != NULL)
	{
		pDC->SelectObject(pOldFont);
	}

	pDC->SetTextColor(clrTextOld);

	if (pLockedImages != NULL)
	{
		pLockedImages->EndDrawImage(ds);
	}

	if (pUserImages != NULL)
	{
		ASSERT_VALID(pUserImages);
		pUserImages->EndDrawImage(ds);
	}
}

void CMFCToolBarMenuButton::OnCancelMode()
{
	if (m_pPopupMenu != NULL && ::IsWindow(m_pPopupMenu->m_hWnd))
	{
		if (m_pPopupMenu->InCommand())
		{
			return;
		}

		for (int i = 0; i < m_pPopupMenu->GetMenuItemCount(); i++)
		{
			CMFCToolBarMenuButton* pSubItem = m_pPopupMenu->GetMenuItem(i);
			if (pSubItem != NULL)
			{
				pSubItem->OnCancelMode();
			}
		}

		m_pPopupMenu->SaveState();
		m_pPopupMenu->m_bAutoDestroyParent = FALSE;
		m_pPopupMenu->CloseMenu();
	}

	m_pPopupMenu = NULL;

	if (m_pWndParent != NULL && ::IsWindow(m_pWndParent->m_hWnd))
	{
		CRect rect = m_rect;

		const int nShadowSize = CMFCVisualManager::GetInstance()->GetMenuShadowDepth();

		rect.InflateRect(nShadowSize, nShadowSize);

		m_pWndParent->InvalidateRect(rect);
		m_pWndParent->UpdateWindow();
	}

	m_bToBeClosed = FALSE;
}

BOOL CMFCToolBarMenuButton::OpenPopupMenu(CWnd* pWnd)
{
	if (m_pPopupMenu != NULL)
	{
		return FALSE;
	}

	if (pWnd == NULL)
	{
		pWnd = m_pWndParent;
	}

	ENSURE(pWnd != NULL);

	HMENU hMenu = CreateMenu();
	if (hMenu == NULL && !IsEmptyMenuAllowed())
	{
		return FALSE;
	}

	m_pPopupMenu = CreatePopupMenu();

	if (m_pPopupMenu == NULL)
	{
		::DestroyMenu(hMenu);
		return FALSE;
	}

	if (m_pPopupMenu->GetMenuItemCount() > 0 && hMenu != NULL)
	{
		::DestroyMenu(hMenu);
		hMenu = NULL;
	}

	//---------------------------------------------------------------
	// Define a new menu position. Place the menu in the right side
	// of the current menu in the poup menu case or under the current
	// item by default:
	//---------------------------------------------------------------
	CPoint point;
	CMFCPopupMenu::DROP_DIRECTION dropDir = CMFCPopupMenu::DROP_DIRECTION_NONE;

	CMFCPopupMenuBar* pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenuBar, m_pWndParent);
	CMFCMenuBar* pParentMenuBar = DYNAMIC_DOWNCAST(CMFCMenuBar, m_pWndParent);

	if (pParentMenu != NULL)
	{
		point = CPoint(0, m_rect.top - 2);
		pWnd->ClientToScreen(&point);

		CRect rectParent;
		pParentMenu->GetWindowRect(rectParent);

		int nMenuGap = CMFCVisualManager::GetInstance()->GetPopupMenuGap();

		if (pParentMenu->GetExStyle() & WS_EX_LAYOUTRTL)
		{
			point.x = rectParent.left - nMenuGap;
			dropDir = CMFCPopupMenu::DROP_DIRECTION_LEFT;
		}
		else
		{
			point.x = rectParent.right + nMenuGap;
			dropDir = CMFCPopupMenu::DROP_DIRECTION_RIGHT;
		}
	}
	else if (pParentMenuBar != NULL && (pParentMenuBar->IsHorizontal()) == 0)
	{
		//------------------------------------------------
		// Parent menu bar is docked vertical, place menu
		// in the left or right side of the parent frame:
		//------------------------------------------------
		point = CPoint(m_rect.right, m_rect.top);
		pWnd->ClientToScreen(&point);

		dropDir = CMFCPopupMenu::DROP_DIRECTION_RIGHT;
	}
	else
	{
		if (m_bShowAtRightSide)
		{
			point = CPoint(m_rect.right - 1, m_rect.top);
		}
		else
		{
			if (m_pPopupMenu->IsRightAlign())
			{
				point = CPoint(m_rect.right - 1, m_rect.bottom - 1);
			}
			else
			{
				point = CPoint(m_rect.left, m_rect.bottom - 1);
			}
		}

		dropDir = CMFCPopupMenu::DROP_DIRECTION_BOTTOM;
		pWnd->ClientToScreen(&point);
	}

	m_pPopupMenu->m_pParentBtn = this;
	m_pPopupMenu->m_DropDirection = dropDir;

	if (!m_pPopupMenu->Create(pWnd, point.x, point.y, hMenu))
	{
		m_pPopupMenu = NULL;
		return FALSE;
	}

	OnAfterCreatePopupMenu();

	if (m_pWndMessage != NULL)
	{
		ASSERT_VALID(m_pWndMessage);
		m_pPopupMenu->SetMessageWnd(m_pWndMessage);
	}
	else
	{
		// If parent menu has a message window, the child should have the same
		CMFCPopupMenu* pCallerMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, pWnd->GetParent());
		if (pCallerMenu != NULL && pCallerMenu->GetMessageWnd() != NULL)
		{
			m_pPopupMenu->SetMessageWnd(pCallerMenu->GetMessageWnd());
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarMenuButton diagnostics

#ifdef _DEBUG
void CMFCToolBarMenuButton::AssertValid() const
{
	CObject::AssertValid();
}

void CMFCToolBarMenuButton::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	CString strId;
	strId.Format(_T("%x"), m_nID);

	dc << "[" << m_strText << " >>>>> ]";
	dc.SetDepth(dc.GetDepth() + 1);

	dc << "{\n";
	for (POSITION pos = m_listCommands.GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarButton* pButton = (CMFCToolBarButton*) m_listCommands.GetNext(pos);
		ASSERT_VALID(pButton);

		pButton->Dump(dc);
		dc << "\n";
	}

	dc << "}\n";
	dc.SetDepth(dc.GetDepth() - 1);
	dc << "\n";
}

#endif


int CMFCToolBarMenuButton::OnDrawOnCustomizeList(
	CDC* pDC, const CRect& rect, BOOL bSelected)
{
	CMFCToolBarButton::OnDrawOnCustomizeList(pDC, rect, bSelected);

	if (m_nID == 0 || !m_listCommands.IsEmpty() || HasButton()) // Popup menu
	{
		CMFCVisualManager::GetInstance()->OnDrawMenuArrowOnCustomizeList(pDC, rect, bSelected);
	}

	return rect.Width();
}

BOOL CMFCToolBarMenuButton::OnBeforeDrag() const
{
	if (m_pPopupMenu != NULL) // Is dropped down
	{
		m_pPopupMenu->CollapseSubmenus();
		m_pPopupMenu->SendMessage(WM_CLOSE);
	}

	return CMFCToolBarButton::OnBeforeDrag();
}

void __stdcall CMFCToolBarMenuButton::GetTextHorzOffsets(int& xOffsetLeft, int& xOffsetRight)
{
	xOffsetLeft = CMFCToolBar::GetMenuImageSize().cx / 2 + AFX_TEXT_MARGIN;
	xOffsetRight = CMenuImages::Size().cx;
}

void CMFCToolBarMenuButton::SaveBarState()
{
	if (m_pWndParent == NULL)
	{
		return;
	}

	CMFCPopupMenu* pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, m_pWndParent->GetParent());
	if (pParentMenu == NULL)
	{
		return;
	}

	ASSERT_VALID(pParentMenu);

	CMFCPopupMenu* pTopLevelMenu = pParentMenu;
	while ((pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, pParentMenu->GetParent())) != NULL)
	{
		pTopLevelMenu = pParentMenu;
	}

	ASSERT_VALID(pTopLevelMenu);
	pTopLevelMenu->SaveState();
}

void CMFCToolBarMenuButton::GetImageRect(CRect& rectImage)
{
	ASSERT_VALID(this);

	rectImage = m_rect;
	rectImage.left += CMFCVisualManager::GetInstance()->GetMenuImageMargin();

	rectImage.right = rectImage.left + CMFCToolBar::GetMenuImageSize().cx + CMFCVisualManager::GetInstance()->GetMenuImageMargin();
}

void CMFCToolBarMenuButton::SetTearOff(UINT uiBarID)
{
	if (m_uiTearOffBarID == uiBarID)
	{
		return;
	}

	if (g_pTearOffMenuManager != NULL)
	{
		if (m_uiTearOffBarID != 0)
		{
			g_pTearOffMenuManager->SetInUse(m_uiTearOffBarID, FALSE);
		}

		if (uiBarID != 0)
		{
			g_pTearOffMenuManager->SetInUse(uiBarID);
		}
	}

	m_uiTearOffBarID = uiBarID;
}

void CMFCToolBarMenuButton::SetMenuPaletteMode(BOOL bMenuPaletteMode/* = TRUE*/, int nPaletteRows/* = 1*/)
{
	ASSERT_VALID(this);
	ASSERT(!IsDroppedDown());

	m_bMenuPaletteMode = bMenuPaletteMode;
	m_nPaletteRows = nPaletteRows;
}

void CMFCToolBarMenuButton::SetRadio()
{
	m_bIsRadio = TRUE;

	if (m_pWndParent != NULL)
	{
		CRect rectImage;
		GetImageRect(rectImage);

		m_pWndParent->InvalidateRect(rectImage);
		m_pWndParent->UpdateWindow();
	}
}

void CMFCToolBarMenuButton::ResetImageToDefault()
{
	ASSERT_VALID(this);

	CMFCToolBarButton::ResetImageToDefault();

	for (POSITION pos = m_listCommands.GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarMenuButton* pItem = (CMFCToolBarMenuButton*) m_listCommands.GetNext(pos);
		ASSERT_VALID(pItem);

		pItem->ResetImageToDefault();
	}
}

BOOL CMFCToolBarMenuButton::CompareWith(const CMFCToolBarButton& other) const
{
	if (m_nID != other.m_nID)
	{
		return FALSE;
	}

	const CMFCToolBarMenuButton& otherMenuBtn = (const CMFCToolBarMenuButton&) other;

	if (m_listCommands.GetCount() != otherMenuBtn.m_listCommands.GetCount())
	{
		return FALSE;
	}

	POSITION pos1 = otherMenuBtn.m_listCommands.GetHeadPosition();

	for (POSITION pos = m_listCommands.GetHeadPosition(); pos != NULL;)
	{
		ENSURE(pos1 != NULL);

		CMFCToolBarMenuButton* pItem = (CMFCToolBarMenuButton*) m_listCommands.GetNext(pos);
		ASSERT_VALID(pItem);

		CMFCToolBarMenuButton* pItem1 = (CMFCToolBarMenuButton*) otherMenuBtn.m_listCommands.GetNext(pos1);
		ASSERT_VALID(pItem1);

		if (!pItem->CompareWith(*pItem1))
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CMFCToolBarMenuButton::SetACCData(CWnd* pParent, CAccessibilityData& data)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pParent);

	if (!CMFCToolBarButton::SetACCData(pParent, data))
	{
		return FALSE;
	}

	data.m_nAccRole = ROLE_SYSTEM_MENUITEM;
	data.m_bAccState = STATE_SYSTEM_FOCUSED | STATE_SYSTEM_FOCUSABLE;

	if (m_nStyle & TBBS_CHECKED)
	{
		data.m_bAccState |= STATE_SYSTEM_CHECKED;
	}

	if (m_nStyle & TBBS_DISABLED)
	{
		data.m_bAccState |= STATE_SYSTEM_UNAVAILABLE;
	}

	data.m_strAccHelp = L"CMFCToolBarMenuButton";
	data.m_strAccDefAction = m_bMenuMode ? _T("Execute") : _T("Open");

	return TRUE;
}

void CMFCToolBarMenuButton::DrawDocumentIcon(CDC* pDC, const CRect& rectImage, HICON hIcon)
{
	ASSERT_VALID(pDC);

	int cx = afxGlobalData.m_sizeSmallIcon.cx;
	int cy = afxGlobalData.m_sizeSmallIcon.cy;

	if (cx > rectImage.Width() ||
		cy > rectImage.Height())
	{
		// Small icon is too large, stretch it
		cx = rectImage.Width();
		cy = rectImage.Height();
	}

	int x = max(0, (rectImage.Width() - cx) / 2);
	int y = max(0, (rectImage.Height() - cy) / 2);

	::DrawIconEx(pDC->GetSafeHdc(), rectImage.left + x, rectImage.top + y, hIcon, cx, cy, 0, NULL, DI_NORMAL);
}


