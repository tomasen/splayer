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
#include <afxpriv.h>

#pragma warning(disable : 4201)
#include "mmsystem.h"
#pragma warning(default : 4201)

#include "afxwinappex.h"
#include "afxpopupmenubar.h"
#include "afxtoolbarbutton.h"
#include "afxtoolbarmenubutton.h"
#include "afxpopupmenu.h"
#include "afxcommandmanager.h"
#include "afxmenutearoffmanager.h"
#include "afxglobals.h"
#include "afxtoolbarmenubutton.h"
#include "afxribbonres.h"
#include "afxmenubar.h"
#include "afxtoolbarcomboboxbutton.h"
#include "afxusertoolsmanager.h"
#include "afxsettingsstore.h"
#include "afxkeyboardmanager.h"
#include "afxsound.h"
#include "afxframeimpl.h"
#include "afxmenuhash.h"
#include "afxvisualmanager.h"
#include "afxdrawmanager.h"
#include "afxcontextmenumanager.h"
#include "afxshowallbutton.h"
#include "afxcustomizemenubutton.h"
#include "afxcustomizebutton.h"
#include "afxtooltipmanager.h"
#include "afxdropdownlistbox.h"
#include "afxbaseribbonelement.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const int nVertMargin = 1;
static const int nHorzMargin = 1;
static const int nSeparatorHeight = 8;
static const int nMinTabSpace = 10;
static const int nEmptyMenuWidth = 50;
static const int nEmptyMenuHeight = 20;

static const int nPopupTimerEvent = 1;
static const int nRemovePopupTimerEvent = 2;

UINT CMFCPopupMenuBar::m_uiPopupTimerDelay = (UINT) -1;
int CMFCPopupMenuBar::m_nLastCommandIndex = -1;

/////////////////////////////////////////////////////////////////////////////
// CMFCPopupMenuBar

IMPLEMENT_SERIAL(CMFCPopupMenuBar, CMFCToolBar, 1)

CMFCPopupMenuBar::CMFCPopupMenuBar() : m_uiDefaultMenuCmdId(0), m_pDelayedPopupMenuButton(NULL), m_pDelayedClosePopupMenuButton(NULL), m_bFirstClick(TRUE), m_bFirstMove(TRUE), m_iOffset(0), m_xSeparatorOffsetLeft(0), m_xSeparatorOffsetRight(0), m_iMaxWidth(-1), m_iMinWidth(-1), m_bAreAllCommandsShown(TRUE), m_bInCommand(FALSE), m_bTrackMode(FALSE)
{
	m_bMenuMode = TRUE;
	m_bIsClickOutsideItem = TRUE;
	m_bEnableIDChecking = FALSE;
	m_bDisableSideBarInXPMode = FALSE;
	m_bPaletteMode = FALSE;
	m_bPaletteRows = 1;
	m_pRelatedToolbar = NULL;
	m_bDropDownListMode = FALSE;
	m_bInScrollMode = FALSE;
	m_bResizeTracking = FALSE;
	m_nDropDownPageSize = 0;
	m_ptCursor = CPoint(-1, -1);
}

CMFCPopupMenuBar::~CMFCPopupMenuBar()
{
}

//{{AFX_MSG_MAP(CMFCPopupMenuBar)
BEGIN_MESSAGE_MAP(CMFCPopupMenuBar, CMFCToolBar)
	ON_WM_NCPAINT()
	ON_WM_NCCALCSIZE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_CREATE()
	ON_MESSAGE(WM_IDLEUPDATECMDUI, &CMFCPopupMenuBar::OnIdleUpdateCmdUI)
	ON_COMMAND(ID_AFXBARRES_TOOLBAR_IMAGE_AND_TEXT, &CMFCPopupMenuBar::OnToolbarImageAndText)
	ON_COMMAND(ID_AFXBARRES_TOOLBAR_TEXT, &CMFCPopupMenuBar::OnToolbarText)
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// CMFCPopupMenuBar message handlers

BOOL CMFCPopupMenuBar::OnSendCommand(const CMFCToolBarButton* pButton)
{
	ASSERT_VALID(pButton);

	if (pButton->m_nID == AFX_MENU_GROUP_ID)
	{
		return TRUE;
	}

	CMFCCustomizeMenuButton* pCustomMenuButton = DYNAMIC_DOWNCAST(CMFCCustomizeMenuButton, pButton);

	if ((pCustomMenuButton != NULL) &&
		((pButton->m_nStyle & TBBS_DISABLED) != 0 ))
	{
		pCustomMenuButton->OnClickMenuItem();

		return TRUE;
	}

	if ((pButton->m_nStyle & TBBS_DISABLED) != 0 || pButton->m_nID < 0 || pButton->m_nID == (UINT)-1)
	{
		return FALSE;
	}

	CMFCToolBarMenuButton* pMenuButton = DYNAMIC_DOWNCAST(CMFCToolBarMenuButton, pButton);

	if (pMenuButton != NULL && pMenuButton->HasButton())
	{
		CPoint ptCursor;
		::GetCursorPos(&ptCursor);
		ScreenToClient(&ptCursor);

		if (pMenuButton->m_rectButton.PtInRect(ptCursor))
		{
			return TRUE;
		}

		if (pMenuButton->m_pPopupMenu != NULL)
		{
			pMenuButton->m_pPopupMenu->PostMessage(WM_CLOSE);
			return FALSE;
		}
	}

	if (pMenuButton != NULL && pMenuButton->m_pPopupMenu != NULL)
	{
		return FALSE;
	}

	if (pMenuButton != NULL && pMenuButton->OnClickMenuItem())
	{
		return TRUE;
	}

	if (pMenuButton != NULL && pMenuButton->IsKindOf(RUNTIME_CLASS(CMFCShowAllButton)))
	{
		pMenuButton->OnClick(this, FALSE);
		return TRUE;
	}

	InvokeMenuCommand(pButton->m_nID, pButton);
	return TRUE;
}

void CMFCPopupMenuBar::InvokeMenuCommand(UINT uiCmdId, const CMFCToolBarButton* pMenuItem)
{
	ASSERT(uiCmdId != (UINT) -1);

	CMFCPopupMenu* pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, GetParent());

	if (pParentMenu != NULL && pParentMenu->GetMessageWnd() != NULL)
	{
		pParentMenu->GetMessageWnd()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
	}
	else
	{
		GetOwner()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
	}

	// Deactivate menubar:
	if (pParentMenu != NULL)
	{
		CMFCToolBar* pToolBar = NULL;
		for (CMFCPopupMenu* pMenu = pParentMenu; pMenu != NULL; pMenu = pMenu->GetParentPopupMenu())
		{
			CMFCToolBarMenuButton* pParentButton = pMenu->GetParentButton();
			if (pParentButton == NULL)
			{
				break;
			}

			pToolBar = DYNAMIC_DOWNCAST(CMFCToolBar, pParentButton->GetParentWnd());
		}

		if (pToolBar != NULL)
		{
			pToolBar->Deactivate();
		}
	}

	if (uiCmdId != 0)
	{
		SetInCommand();

		AFXPlaySystemSound(AFX_SOUND_MENU_COMMAND);

		if (m_bDropDownListMode)
		{
			if (pParentMenu != NULL)
			{
				pParentMenu->OnChooseItem(uiCmdId);
			}
		}
		else if (!m_bTrackMode)
		{
			BOOL bDone = FALSE;

			pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, GetParent());
			if (pParentMenu != NULL)
			{
				ASSERT_VALID(pParentMenu);

				CMFCCustomizeButton* pCustomizeButton = DYNAMIC_DOWNCAST(CMFCCustomizeButton, pParentMenu->GetParentButton());
				if (pCustomizeButton != NULL)
				{
					bDone = pCustomizeButton->InvokeCommand(this, pMenuItem);
				}
			}

			if (!bDone)
			{
				// Send command to the parent frame:
				AddCommandUsage(uiCmdId);

				if (pParentMenu != NULL)
				{
					ASSERT_VALID(pParentMenu);

					if (!pParentMenu->PostCommand(uiCmdId) &&
						(afxUserToolsManager == NULL || !afxUserToolsManager->InvokeTool(uiCmdId)))
					{
						BOOL bIsSysCommand = (uiCmdId >= 0xF000 && uiCmdId < 0xF1F0);
						GetOwner()->PostMessage(bIsSysCommand ? WM_SYSCOMMAND : WM_COMMAND, uiCmdId);

						if (pParentMenu->m_pParentRibbonElement != NULL)
						{
							CMFCRibbonBaseElement* pElement = pParentMenu->m_pParentRibbonElement;
							ASSERT_VALID(pElement);

							pParentMenu->m_pParentRibbonElement->SetDroppedDown(NULL);

							pParentMenu->m_pParentRibbonElement = NULL;
							pElement->PostMenuCommand(uiCmdId);
						}
					}
				}
			}
		}
		else
		{
			if (afxContextMenuManager == NULL)
			{
				ASSERT(FALSE);
			}
			else
			{
				afxContextMenuManager->m_nLastCommandID = uiCmdId;
			}
		}
	}

	m_nLastCommandIndex = pMenuItem == NULL ? -1 : ButtonToIndex(pMenuItem);

	if (m_bPaletteMode)
	{
		pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, GetParent());
		if (pParentMenu != NULL)
		{
			ASSERT_VALID(pParentMenu);

			CMFCToolBarMenuButton* pParentButton = pParentMenu->GetParentButton();
			if (pParentButton != NULL && pParentButton->GetParentWnd() != NULL)
			{
				ASSERT_VALID(pParentButton);
				pParentButton->m_nID = uiCmdId;
				pParentButton->SetImage(afxCommandManager->GetCmdImage(uiCmdId));

				CRect rectImage;
				pParentButton->GetImageRect(rectImage);

				pParentButton->GetParentWnd()->InvalidateRect(rectImage);
				pParentButton->GetParentWnd()->UpdateWindow();
			}
		}
	}

	CFrameWnd* pParentFrame = AFXGetParentFrame(this);
	ASSERT_VALID(pParentFrame);

	SetInCommand(FALSE);
	pParentFrame->DestroyWindow();
}

void CMFCPopupMenuBar::AdjustLocations()
{
	if (GetSafeHwnd() == NULL || !::IsWindow(m_hWnd) || m_bInUpdateShadow)
	{
		return;
	}

	if (m_bPaletteMode)
	{
		CMFCToolBar::AdjustLocations();
		UpdateTooltips();
		return;
	}

	ASSERT_VALID(this);

	if (m_xSeparatorOffsetLeft == 0)
	{
		// To enable MS Office 2000 look, we'll draw the separators
		// bellow the menu text only(in the previous versions
		// separator has been drawn on the whole menu row). Ask
		// menu button about text area offsets:
		CMFCToolBarMenuButton::GetTextHorzOffsets(m_xSeparatorOffsetLeft, m_xSeparatorOffsetRight);
	}

	CRect rectClient; // Client area rectangle
	GetClientRect(&rectClient);

	CClientDC dc(this);
	CFont* pOldFont = (CFont*) dc.SelectObject(&afxGlobalData.fontRegular);
	ENSURE(pOldFont != NULL);

	int y = rectClient.top + nVertMargin - m_iOffset * GetRowHeight();

	/// support for the menu with breaks:
	int origy = y;
	int x = rectClient.left;
	int right = (m_arColumns.GetSize() == 0 || CMFCToolBar::IsCustomizeMode()) ? rectClient.Width() : m_arColumns [0];
	int nColumn = 0;
	/////////

	CSize sizeMenuButton = GetMenuImageSize();
	sizeMenuButton += CSize(2 * nHorzMargin, 2 * nVertMargin);

	sizeMenuButton.cy = max(sizeMenuButton.cy, afxGlobalData.GetTextHeight());

	for (POSITION pos = m_Buttons.GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarButton* pButton = (CMFCToolBarButton*) m_Buttons.GetNext(pos);
		ENSURE(pButton != NULL);
		ASSERT_VALID(pButton);

		/// support for the menu with breaks:
		if ((pButton->m_nStyle & AFX_TBBS_BREAK) &&(y != origy) && !CMFCToolBar::IsCustomizeMode())
		{
			y = origy;
			nColumn ++;
			x = right + nHorzMargin;
			right = m_arColumns [nColumn];
		}
		////////////////////

		CRect rectButton;
		rectButton.top = y;

		if (pButton->m_nStyle & TBBS_SEPARATOR)
		{
			rectButton.left = x + m_xSeparatorOffsetLeft;
			rectButton.right = right + rectClient.left - m_xSeparatorOffsetRight;
			rectButton.bottom = rectButton.top + nSeparatorHeight;
		}
		else
		{
			CSize sizeButton = pButton->OnCalculateSize(&dc, sizeMenuButton, TRUE);

			rectButton.left = x;
			rectButton.right = right + rectClient.left;
			rectButton.bottom = rectButton.top + sizeButton.cy;
		}

		pButton->SetRect(rectButton);
		y += rectButton.Height();
	}

	dc.SelectObject(pOldFont);

	// Something may changed, rebuild acceleration keys:
	RebuildAccelerationKeys();

	CPoint ptCursor;
	::GetCursorPos(&ptCursor);
	ScreenToClient(&ptCursor);

	if (HitTest(ptCursor) >= 0)
	{
		m_bIsClickOutsideItem = FALSE;
	}

	UpdateTooltips();
}

void CMFCPopupMenuBar::DrawSeparator(CDC* pDC, const CRect& rect, BOOL /*bHorz*/)
{
	CMFCVisualManager::GetInstance()->OnDrawSeparator(pDC, this, rect, FALSE);
}

CSize CMFCPopupMenuBar::CalcSize(BOOL /*bVertDock*/)
{
	if (m_bPaletteMode)
	{
		return CMFCToolBar::CalcSize(FALSE);
	}

	CSize size(0, 0);

	CClientDC dc(this);
	CFont* pOldFont = (CFont*) dc.SelectObject(&afxGlobalData.fontRegular);
	ENSURE(pOldFont != NULL);

	if (m_Buttons.IsEmpty())
	{
		size = CSize(nEmptyMenuWidth, nEmptyMenuHeight);
	}
	else
	{
		//support for the menu with breaks:
		CSize column(0, 0);
		m_arColumns.RemoveAll();
		//////////////////////////

		CSize sizeMenuButton = GetMenuImageSize();
		sizeMenuButton += CSize(2 * nHorzMargin, 2 * nVertMargin);

		sizeMenuButton.cy = max(sizeMenuButton.cy, afxGlobalData.GetTextHeight());

		for (POSITION pos = m_Buttons.GetHeadPosition(); pos != NULL;)
		{
			CMFCToolBarButton* pButton = (CMFCToolBarButton*) m_Buttons.GetNext(pos);
			ENSURE(pButton != NULL);
			ASSERT_VALID(pButton);

			BOOL bRestoreFont = FALSE;

			if (m_uiDefaultMenuCmdId != 0 && pButton->m_nID == m_uiDefaultMenuCmdId)
			{
				dc.SelectObject(&afxGlobalData.fontBold);
				bRestoreFont = TRUE;
			}

			CSize sizeButton = pButton->OnCalculateSize(&dc, sizeMenuButton, TRUE);

			// support for the menu with breaks:
			if ((pButton->m_nStyle & AFX_TBBS_BREAK) && !CMFCToolBar::IsCustomizeMode())
			{
				if ((column.cx != 0) &&(column.cy != 0))
				{
					size.cy = max(column.cy, size.cy);
					size.cx += column.cx + nHorzMargin;
					m_arColumns.Add(size.cx);
				}
				column.cx = column.cy = 0;
			}
			///////////////////////////////

			int iHeight = sizeButton.cy;

			if (pButton->m_nStyle & TBBS_SEPARATOR)
			{
				iHeight = nSeparatorHeight;
			}
			else
			{
				if (pButton->IsDrawText() && pButton->m_strText.Find(_T('\t')) > 0)
				{
					sizeButton.cx += nMinTabSpace;
				}

				pButton->m_bWholeText =
					(m_iMaxWidth <= 0 || sizeButton.cx <= m_iMaxWidth - 2 * nHorzMargin);

				column.cx = max(sizeButton.cx, column.cx);
			}

			column.cy += iHeight;

			if (bRestoreFont)
			{
				dc.SelectObject(&afxGlobalData.fontRegular);
			}
		}

		size.cy = max(column.cy, size.cy);
		size.cx += column.cx;
	}

	size.cy += 2 * nVertMargin;
	size.cx += 2 * nHorzMargin;

	if (m_iMaxWidth > 0 && size.cx > m_iMaxWidth)
	{
		size.cx = m_iMaxWidth;
	}

	if (m_iMinWidth > 0 && size.cx < m_iMinWidth)
	{
		size.cx = m_iMinWidth;
	}

	m_arColumns.Add(size.cx);

	dc.SelectObject(pOldFont);
	return size;
}

void CMFCPopupMenuBar::OnNcPaint()
{
	// Disable gripper and borders painting!
}

void CMFCPopupMenuBar::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS FAR* /*lpncsp*/)
{
	// Don't leave space for the gripper and borders!
}

void CMFCPopupMenuBar::DrawDragCursor(CDC* pDC)
{
	if (m_bPaletteMode)
	{
		return;
	}

	CPen* pOldPen = (CPen*) pDC->SelectObject(&m_penDrag);

	for (int i = 0; i < 2; i ++)
	{
		pDC->MoveTo(m_rectDrag.left, m_rectDrag.top + m_rectDrag.Height() / 2 + i - 1);
		pDC->LineTo(m_rectDrag.right, m_rectDrag.top + m_rectDrag.Height() / 2 + i - 1);

		pDC->MoveTo(m_rectDrag.left + i, m_rectDrag.top + i);
		pDC->LineTo(m_rectDrag.left + i, m_rectDrag.bottom - i);

		pDC->MoveTo(m_rectDrag.right - i - 1, m_rectDrag.top + i);
		pDC->LineTo(m_rectDrag.right - i - 1, m_rectDrag.bottom - i);
	}

	pDC->SelectObject(pOldPen);
}

int CMFCPopupMenuBar::FindDropIndex(const CPoint p, CRect& rectDrag) const
{
	if (m_bPaletteMode)
	{
		return -1;
	}

	const int iCursorSize = 6;

	GetClientRect(&rectDrag);

	if (m_Buttons.IsEmpty())
	{
		rectDrag.bottom = rectDrag.top + iCursorSize;
		return 0;
	}

	CPoint point = p;
	if (point.y < 0)
	{
		point.y = 0;
	}

	int iDragButton = -1;
	int iIndex = 0;
	for (POSITION pos = m_Buttons.GetHeadPosition(); pos != NULL; iIndex ++)
	{
		CMFCToolBarButton* pButton = (CMFCToolBarButton*) m_Buttons.GetNext(pos);
		ENSURE(pButton != NULL);
		ASSERT_VALID(pButton);

		CRect rect = pButton->Rect();
		if (point.y < rect.top)
		{
			iDragButton = iIndex;
			rectDrag.top = rect.top;
			break;
		}
		else if (point.y <= rect.bottom)
		{
			rectDrag = rect;
			if (point.y - rect.top > rect.bottom - point.y)
			{
				iDragButton = iIndex + 1;
				rectDrag.top = rectDrag.bottom;
			}
			else
			{
				iDragButton = iIndex;
				rectDrag.top = rect.top;
			}
			break;
		}
	}

	if (iDragButton == -1)
	{
		rectDrag.top = rectDrag.bottom - iCursorSize;
		iDragButton = iIndex;
	}

	rectDrag.bottom = rectDrag.top + iCursorSize;
	rectDrag.OffsetRect(0, -iCursorSize / 2);

	return iDragButton;
}

CMFCToolBarButton* CMFCPopupMenuBar::CreateDroppedButton(COleDataObject* pDataObject)
{
	CMFCToolBarButton* pButton = CMFCToolBarButton::CreateFromOleData(pDataObject);
	ENSURE(pButton != NULL);
	ASSERT_VALID(pButton);

	CMFCToolBarMenuButton* pMenuButton = DYNAMIC_DOWNCAST(CMFCToolBarMenuButton, pButton);

	if (pMenuButton == NULL)
	{
		pMenuButton = new CMFCToolBarMenuButton(pButton->m_nID, NULL, pButton->IsLocked() ? -1 : pButton->GetImage(), pButton->m_strText, pButton->m_bUserButton);
		ENSURE(pMenuButton != NULL);

		pMenuButton->m_bText = TRUE;
		pMenuButton->m_bImage = !pButton->IsLocked();

		BOOL bRes = pButton->ExportToMenuButton(*pMenuButton);
		delete pButton;

		if (!bRes || pMenuButton->m_strText.IsEmpty())
		{
			delete pMenuButton;
			return NULL;
		}
	}

	return pMenuButton;
}

BOOL CMFCPopupMenuBar::ImportFromMenu(HMENU hMenu, BOOL bShowAllCommands)
{
	RemoveAllButtons();
	m_bAreAllCommandsShown = TRUE;
	m_HiddenItemsAccel.RemoveAll();

	if (hMenu == NULL)
	{
		return FALSE;
	}

	CMenu* pMenu = CMenu::FromHandle(hMenu);
	if (pMenu == NULL)
	{
		return FALSE;
	}

	// We need to update the menu items first(OnUpdate*** for the target message
	// window need to be invoked:
	CWnd* pMsgWindow = AFXGetTopLevelFrame(this);

	if (pMsgWindow == NULL)
	{
		pMsgWindow = AfxGetMainWnd();
	}

	if (GetSafeHwnd() != NULL)
	{
		CMFCPopupMenu* pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, GetParent());
		if (pParentMenu != NULL && pParentMenu->GetMessageWnd() != NULL)
		{
			pMsgWindow = pParentMenu->GetMessageWnd();
		}

		if (m_hookMouseHelp != NULL && pParentMenu != NULL && pParentMenu->GetParentPopupMenu() != NULL)
		{
			bShowAllCommands = TRUE;
		}
	}

	if (pMsgWindow != NULL)
	{
		WPARAM theMenu = WPARAM(hMenu);
		LPARAM theItem = MAKELPARAM(m_iOffset, 0);
		pMsgWindow->SendMessage(WM_INITMENUPOPUP, theMenu, theItem);
	}

	int iCount = (int) pMenu->GetMenuItemCount();
	BOOL bPrevWasSeparator = FALSE;
	BOOL bFirstItem = TRUE;

	int nPaletteColumns = 1;
	if (m_bPaletteMode)
	{
		nPaletteColumns = max(1, (int)(.5 +(double) iCount / m_bPaletteRows));
	}

	for (int i = 0; i < iCount; i ++)
	{
		UINT uiTearOffId = 0;

		HMENU hSubMenu = NULL;

		CString strText;
		pMenu->GetMenuString(i, strText, MF_BYPOSITION);

		MENUITEMINFO mii;
		ZeroMemory(&mii, sizeof(MENUITEMINFO));

		mii.cbSize = sizeof(mii);
		mii.cch = 0;
		mii.dwTypeData = 0;
		mii.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_ID | MIIM_STATE | MIIM_DATA;
		pMenu->GetMenuItemInfo(i, &mii, TRUE);

		UINT uiCmd = mii.wID;
		UINT uiState = pMenu->GetMenuState(i, MF_BYPOSITION);
		DWORD dwMenuItemData = (DWORD) mii.dwItemData;

		if (mii.fType == MFT_SEPARATOR)
		{
			if (!bPrevWasSeparator && !bFirstItem && i != iCount - 1 && !m_bPaletteMode)
			{
				InsertSeparator();
				bFirstItem = FALSE;
				bPrevWasSeparator = TRUE;
			}
		}
		else
		{
			if (mii.hSubMenu != NULL)
			{
				uiCmd = (UINT)-1;  // force value(needed due to Windows bug)
				hSubMenu = mii.hSubMenu;
				ENSURE(hSubMenu != NULL);

				if (g_pTearOffMenuManager != NULL)
				{
					uiTearOffId = g_pTearOffMenuManager->Parse(strText);
				}
			}

			if (m_bTrackMode || bShowAllCommands || CMFCMenuBar::IsShowAllCommands() || !CMFCToolBar::IsCommandRarelyUsed(uiCmd) || m_bPaletteMode)
			{
				int iIndex = -1;

				if (m_bPaletteMode)
				{
					CMFCToolBarButton item(uiCmd, afxCommandManager->GetCmdImage(uiCmd, FALSE), strText);

					if (i > 0 &&((i + 1) % nPaletteColumns) == 0)
					{
						item.m_bWrap = TRUE;
					}

					iIndex = InsertButton(item);
				}
				else
				{
					CMFCToolBarMenuButton item(uiCmd, hSubMenu, -1, strText);
					item.m_bText = TRUE;
					item.m_bImage = FALSE;
					item.m_iUserImage = afxCommandManager->GetMenuUserImage(uiCmd);

					if (item.m_iUserImage != -1)
					{
						item.m_bUserButton = TRUE;
					}

					iIndex = InsertButton(item);
				}

				if (iIndex >= 0)
				{
					CMFCToolBarButton* pButton = GetButton(iIndex);
					ENSURE(pButton != NULL);
					ASSERT_VALID(pButton);

					pButton->m_bImage = (pButton->GetImage() >= 0);
					pButton->m_dwdItemData = dwMenuItemData;

					if (afxUserToolsManager == NULL || !afxUserToolsManager->IsUserToolCmd(uiCmd))
					{
						if ((uiState & MF_DISABLED) ||(uiState & MF_GRAYED))
						{
							pButton->m_nStyle |= TBBS_DISABLED;
						}
					}

					CMFCToolBarMenuButton* pMenuButton = DYNAMIC_DOWNCAST(CMFCToolBarMenuButton, pButton);
					if (pMenuButton != NULL)
					{
						pMenuButton->SetTearOff(uiTearOffId);
					}

					if (uiState & MF_CHECKED)
					{
						pButton->m_nStyle |= TBBS_CHECKED;
					}

					//support for the menu with breaks:
					if (mii.fType & MF_MENUBREAK)
					{
						pButton->m_nStyle |= AFX_TBBS_BREAK;
					}
				}

				bPrevWasSeparator = FALSE;
				bFirstItem = FALSE;
			}
			else if (CMFCToolBar::IsCommandRarelyUsed(uiCmd) && CMFCToolBar::IsCommandPermitted(uiCmd))
			{
				m_bAreAllCommandsShown = FALSE;

				int iAmpOffset = strText.Find(_T('&'));
				if (iAmpOffset >= 0 && iAmpOffset < strText.GetLength() - 1)
				{
					TCHAR szChar[2] = {strText.GetAt(iAmpOffset + 1), '\0'};
					CharUpper(szChar);

					UINT uiHotKey = (UINT)(szChar [0]);
					m_HiddenItemsAccel.SetAt(uiHotKey, uiCmd);
				}
			}
		}
	}

	m_uiDefaultMenuCmdId = ::GetMenuDefaultItem(hMenu, FALSE, GMDI_USEDISABLED);
	return TRUE;
}

HMENU CMFCPopupMenuBar::ExportToMenu() const
{
	CMenu menu;
	menu.CreatePopupMenu();

	for (POSITION pos = m_Buttons.GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarButton* pButton = (CMFCToolBarButton*) m_Buttons.GetNext(pos);
		ENSURE(pButton != NULL);
		ASSERT_VALID(pButton);

		if (pButton->m_nStyle & TBBS_SEPARATOR)
		{
			menu.AppendMenu(MF_SEPARATOR);
			continue;
		}

		if (!pButton->IsKindOf(RUNTIME_CLASS(CMFCToolBarMenuButton)))
		{
			continue;
		}

		CMFCToolBarMenuButton* pMenuButton = (CMFCToolBarMenuButton*) pButton;

		HMENU hPopupMenu = pMenuButton->CreateMenu();
		if (hPopupMenu != NULL)
		{
			UINT uiStyle = (MF_STRING | MF_POPUP);

			//support for the menu with breaks:
			if (pButton->m_nStyle & AFX_TBBS_BREAK)
			{
				uiStyle |= MF_MENUBREAK;
			}
			//////////////////////

			CString strText = pMenuButton->m_strText;
			if (pMenuButton->m_uiTearOffBarID != 0 && g_pTearOffMenuManager != NULL)
			{
				g_pTearOffMenuManager->Build(pMenuButton->m_uiTearOffBarID, strText);
			}

			menu.AppendMenu(uiStyle, (UINT_PTR) hPopupMenu, strText);
		}
		else
		{
			menu.AppendMenu(MF_STRING, pMenuButton->m_nID, pMenuButton->m_strText);
		}
	}

	HMENU hMenu = menu.Detach();

	::SetMenuDefaultItem(hMenu, m_uiDefaultMenuCmdId, FALSE);
	return hMenu;
}

void CMFCPopupMenuBar::OnChangeHot(int iHot)
{
	ASSERT_VALID(this);
	ENSURE(::IsWindow(GetSafeHwnd()));

	if (iHot == -1)
	{
		CPoint ptCursor;
		::GetCursorPos(&ptCursor);
		ScreenToClient(&ptCursor);

		if (HitTest(ptCursor) == m_iHot)
		{
			m_iHighlighted = m_iHot;
			return;
		}
	}

	CMFCToolBarMenuButton* pCurrPopupMenu = NULL;

	for (POSITION pos = m_Buttons.GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarButton* pButton = (CMFCToolBarButton*) m_Buttons.GetNext(pos);
		ASSERT_VALID(pButton);

		CMFCToolBarMenuButton* pMenuButton = DYNAMIC_DOWNCAST(CMFCToolBarMenuButton, pButton);

		if (pMenuButton != NULL && pMenuButton->IsDroppedDown())
		{
			pCurrPopupMenu = pMenuButton;
			break;
		}
	}

	CMFCToolBarMenuButton* pMenuButton = NULL;
	if (iHot >= 0)
	{
		CMFCToolBarButton* pButton = GetButton(iHot);
		ENSURE(pButton != NULL);
		ASSERT_VALID(pButton);

		pMenuButton = DYNAMIC_DOWNCAST(CMFCToolBarMenuButton, pButton);
	}

	if (pMenuButton != pCurrPopupMenu)
	{
		CMFCPopupMenu* pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, GetParent());

		if (pCurrPopupMenu != NULL)
		{
			const MSG* pMsg = GetCurrentMessage();

			if (CMFCToolBar::IsCustomizeMode() ||
				(pMsg != NULL && pMsg->message == WM_KEYDOWN))
			{
				KillTimer(nRemovePopupTimerEvent);
				m_pDelayedClosePopupMenuButton = NULL;

				pCurrPopupMenu->OnCancelMode();

				if (pParentMenu != NULL)
				{
					CMFCPopupMenu::ActivatePopupMenu(AFXGetTopLevelFrame(this), pParentMenu);
				}
			}
			else
			{
				m_pDelayedClosePopupMenuButton = pCurrPopupMenu;
				m_pDelayedClosePopupMenuButton->m_bToBeClosed = TRUE;

				SetTimer(nRemovePopupTimerEvent, max(0, m_uiPopupTimerDelay - 1), NULL);

				InvalidateRect(pCurrPopupMenu->Rect());
				UpdateWindow();
			}
		}

		if (pMenuButton != NULL &&
			(pMenuButton->m_nID == (UINT) -1 || pMenuButton->m_bDrawDownArrow))
		{
			pMenuButton->OnClick(this);
		}

		// Maybe, this menu will be closed by the parent menu bar timer proc.?
		CMFCPopupMenuBar* pParentBar = NULL;

		if (pParentMenu != NULL && pParentMenu->GetParentPopupMenu() != NULL &&
			(pParentBar = pParentMenu->GetParentPopupMenu()->GetMenuBar()) != NULL && pParentBar->m_pDelayedClosePopupMenuButton == pParentMenu->GetParentButton())
		{
			pParentBar->RestoreDelayedSubMenu();
		}
	}
	else if (pMenuButton != NULL && pMenuButton == m_pDelayedClosePopupMenuButton)
	{
		m_pDelayedClosePopupMenuButton->m_bToBeClosed = FALSE;
		m_pDelayedClosePopupMenuButton = NULL;

		KillTimer(nRemovePopupTimerEvent);
	}

	m_iHot = iHot;

	if (m_bDropDownListMode)
	{
		CMFCPopupMenu* pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, GetParent());
		if (pParentMenu != NULL)
		{
			pParentMenu->OnChangeHot(m_iHot);
		}
	}

	if (CMFCPopupMenu::IsSendMenuSelectMsg())
	{
		CWnd* pMsgWindow = AFXGetTopLevelFrame(this);
		if (pMsgWindow == NULL)
		{
			pMsgWindow = AfxGetMainWnd();
		}

		CMFCPopupMenu* pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, GetParent());
		if (pParentMenu != NULL && pParentMenu->GetMessageWnd() != NULL)
		{
			pMsgWindow = pParentMenu->GetMessageWnd();
		}

		if (pMsgWindow != NULL && pParentMenu != NULL)
		{
			UINT nFlags = MF_HILITE;
			UINT nItem = 0;

			if (pMenuButton != NULL)
			{
				if ((pMenuButton->m_nStyle & TBBS_DISABLED) != 0)
				{
					nFlags |= MF_DISABLED;
				}

				if ((pMenuButton->m_nStyle & TBBS_CHECKED) != 0)
				{
					nFlags |= MF_CHECKED;
				}

				if ((nItem = pMenuButton->m_nID) == (UINT)-1)
				{
					nItem = iHot;
					nFlags |= MF_POPUP;
				}
			}

			pMsgWindow->SendMessage(WM_MENUSELECT, MAKEWPARAM(nItem, nFlags), (WPARAM) pParentMenu->GetHMenu());
		}
	}
}

void CMFCPopupMenuBar::OnDestroy()
{
	KillTimer(nPopupTimerEvent);
	KillTimer(nRemovePopupTimerEvent);

	m_pDelayedPopupMenuButton = NULL;
	m_pDelayedClosePopupMenuButton = NULL;

	for (POSITION pos = m_Buttons.GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarButton* pButton = (CMFCToolBarButton*) m_Buttons.GetNext(pos);
		ASSERT_VALID(pButton);

		CMFCToolBarMenuButton* pMenuButton = DYNAMIC_DOWNCAST(CMFCToolBarMenuButton, pButton);

		if (pMenuButton != NULL && pMenuButton->IsDroppedDown())
		{
			CMFCPopupMenu* pMenu = pMenuButton->m_pPopupMenu;
			if (pMenu != NULL && ::IsWindow(pMenu->m_hWnd))
			{
				pMenu->SaveState();
				pMenu->PostMessage(WM_CLOSE);
			}
		}
	}

	CMFCToolBar::OnDestroy();
}

BOOL CMFCPopupMenuBar::OnKey(UINT nChar)
{
	BOOL bProcessed = FALSE;

	POSITION posSel = (m_iHighlighted < 0) ? NULL : m_Buttons.FindIndex(m_iHighlighted);
	CMFCToolBarButton* pOldSelButton = (posSel == NULL) ? NULL :(CMFCToolBarButton*) m_Buttons.GetAt(posSel);
	CMFCToolBarButton* pNewSelButton = pOldSelButton;
	int iNewHighlight = m_iHighlighted;

	BOOL bSendEvent = FALSE;

	if (nChar == VK_TAB)
	{
		if (::GetKeyState(VK_SHIFT) & 0x80)
		{
			nChar = VK_UP;
		}
		else
		{
			nChar = VK_DOWN;
		}
	}

	const POSITION posSelSaved = posSel;

	switch (nChar)
	{
	case VK_RETURN:
		{
			bProcessed = TRUE;

			// Try to cascase a popup menu and, if failed
			CMFCToolBarMenuButton* pMenuButton = DYNAMIC_DOWNCAST(CMFCToolBarMenuButton, pOldSelButton);
			if (pMenuButton != NULL && (pMenuButton->HasButton() || !pMenuButton->OpenPopupMenu()))
			{
				GetOwner()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
				OnSendCommand(pMenuButton);
			}
		}
		break;

	case VK_HOME:
		posSel = NULL;
		// Like "Before first"...

	case VK_DOWN:
		// Find next "selecteble" item:
		{
			if (m_bDropDownListMode && posSelSaved == m_Buttons.GetTailPosition() && nChar != VK_HOME)
			{
				return TRUE;
			}

			bProcessed = TRUE;
			if (m_Buttons.IsEmpty())
			{
				break;
			}

			POSITION pos = posSel;
			if (pos != NULL)
			{
				m_Buttons.GetNext(pos);
			}

			if (pos == NULL)
			{
				pos = m_Buttons.GetHeadPosition();
				iNewHighlight = 0;
			}
			else
			{
				iNewHighlight ++;
			}

			POSITION posFound = NULL;
			while (pos != posSel)
			{
				posFound = pos;

				CMFCToolBarButton* pButton = (CMFCToolBarButton*) m_Buttons.GetNext(pos);
				ASSERT_VALID(pButton);

				if ((pButton->m_nStyle & TBBS_SEPARATOR) == 0 && !pButton->Rect().IsRectEmpty() && pButton->m_nID != AFX_MENU_GROUP_ID)
				{
					break;
				}

				iNewHighlight ++;
				if (pos == NULL)
				{
					if (m_bDropDownListMode)
					{
						return TRUE;
					}

					pos = m_Buttons.GetHeadPosition();
					iNewHighlight = 0;
				}
			}

			if (posFound != NULL)
			{
				pNewSelButton = (CMFCToolBarButton*) m_Buttons.GetAt(posFound);
				bSendEvent = TRUE;
			}
		}
		break;

	case VK_END:
		posSel = NULL;
		// Like "After last"....

	case VK_UP:
		// Find previous "selecteble" item:
		{
			if (m_bDropDownListMode && posSelSaved == m_Buttons.GetHeadPosition() && nChar != VK_END)
			{
				return TRUE;
			}

			bProcessed = TRUE;
			if (m_Buttons.IsEmpty())
			{
				break;
			}

			POSITION pos = posSel;
			if (pos != NULL)
			{
				m_Buttons.GetPrev(pos);
			}
			if (pos == NULL)
			{
				pos = m_Buttons.GetTailPosition();
				iNewHighlight = (int) m_Buttons.GetCount() - 1;
			}
			else
			{
				iNewHighlight --;
			}

			POSITION posFound = NULL;
			while (pos != posSel)
			{
				posFound = pos;

				CMFCToolBarButton* pButton = (CMFCToolBarButton*) m_Buttons.GetPrev(pos);
				ASSERT_VALID(pButton);

				if ((pButton->m_nStyle & TBBS_SEPARATOR) == 0 && !pButton->Rect().IsRectEmpty() && pButton->m_nID != AFX_MENU_GROUP_ID)
				{
					break;
				}

				iNewHighlight --;
				if (pos == NULL)
				{
					if (m_bDropDownListMode)
					{
						return TRUE;
					}

					pos = m_Buttons.GetTailPosition();
					iNewHighlight = (int) m_Buttons.GetCount() - 1;
				}
			}

			if (posFound != NULL)
			{
				pNewSelButton = (CMFCToolBarButton*) m_Buttons.GetAt(posFound);
				bSendEvent = TRUE;
			}
		}
		break;

	case VK_PRIOR:
	case VK_NEXT:
		if (m_bDropDownListMode && m_nDropDownPageSize > 0)
		{
			m_bInScrollMode = TRUE;
			int iHighlightedPrev = m_iHighlighted;

			for (int i = 0; i < m_nDropDownPageSize; i++)
			{
				OnKey(nChar == VK_PRIOR ? VK_UP : VK_DOWN);
			}

			m_bInScrollMode = FALSE;

			if (iHighlightedPrev != m_iHighlighted)
			{
				AccNotifyObjectFocusEvent(m_iHighlighted);
			}

			return TRUE;
		}
		break;

	default:
		// Process acceleration key:
		if (!IsCustomizeMode() && (::GetAsyncKeyState(VK_CONTROL) & 0x8000) == 0)
		{
			BOOL bKeyIsPrintable = CKeyboardManager::IsKeyPrintable(nChar);

			UINT nUpperChar = nChar;
			if (bKeyIsPrintable)
			{
				nUpperChar = CKeyboardManager::TranslateCharToUpper(nChar);
			}

			CMFCToolBarButton* pButton;
			if (bKeyIsPrintable && m_AccelKeys.Lookup(nUpperChar, pButton))
			{
				ASSERT_VALID(pButton);

				pNewSelButton = pButton;

				// Find button index:
				int iIndex = 0;
				for (POSITION pos = m_Buttons.GetHeadPosition(); pos != NULL; iIndex ++)
				{
					CMFCToolBarButton* pListButton = (CMFCToolBarButton*) m_Buttons.GetNext(pos);
					ENSURE(pListButton != NULL);

					if (pListButton == pButton)
					{
						iNewHighlight = iIndex;
						break;
					}
				}

				CMFCToolBarMenuButton* pMenuButton = DYNAMIC_DOWNCAST(CMFCToolBarMenuButton, pButton);

				if (pMenuButton != NULL)
				{
					if (pMenuButton->OpenPopupMenu())
					{
						if (pMenuButton->m_pPopupMenu != NULL)
						{
							// Select a first menu item:
							pMenuButton->m_pPopupMenu->SendMessage(WM_KEYDOWN, VK_HOME);
						}
					}
					else
					{
						// If the newly selected item is not highlighted,
						// then make the menu go away.

						if ((pButton->m_nStyle & TBBS_DISABLED) != 0)
						{
							InvokeMenuCommand(0, pButton);
							return TRUE;
						}

						bProcessed = OnSendCommand(pMenuButton);
						if (bProcessed)
						{
							return TRUE;
						}
					}
				}
			}
			else if (CMFCMenuBar::m_bRecentlyUsedMenus && !m_bAreAllCommandsShown)
			{
				// Maybe, this accelerator is belong to "hidden' item?
				UINT uiCmd = 0;
				if (m_HiddenItemsAccel.Lookup(nUpperChar, uiCmd))
				{
					InvokeMenuCommand(uiCmd, NULL);
					return TRUE;
				}
			}
		}
	}

	if (pNewSelButton != pOldSelButton)
	{
		ASSERT_VALID(pNewSelButton);
		ASSERT(iNewHighlight >= 0 && iNewHighlight < m_Buttons.GetCount());
		ASSERT(GetButton(iNewHighlight) == pNewSelButton);

		if (bSendEvent && !m_bInScrollMode)
		{
			AccNotifyObjectFocusEvent(iNewHighlight);
		}

		if (IsCustomizeMode())
		{
			m_iSelected = iNewHighlight;
		}

		m_iHighlighted = iNewHighlight;

		CRect rectClient;
		GetClientRect(rectClient);

		CRect rectNew = pNewSelButton->Rect();

		if (rectNew.top < rectClient.top || rectNew.bottom > rectClient.bottom)
		{
			// don't redraw items, popup menu will be scrolled now
		}
		else
		{
			if (pOldSelButton != NULL)
			{
				InvalidateRect(pOldSelButton->Rect());
			}

			InvalidateRect(rectNew);
			UpdateWindow();
		}

		if (pNewSelButton->m_nID != (UINT) -1)
		{
			ShowCommandMessageString(pNewSelButton->m_nID);
		}
	}

	return bProcessed;
}

void CMFCPopupMenuBar::OnTimer(UINT_PTR nIDEvent)
{
	CPoint ptCursor;
	::GetCursorPos(&ptCursor);
	ScreenToClient(&ptCursor);

	if (nIDEvent == nPopupTimerEvent)
	{
		KillTimer(nPopupTimerEvent);

		// Remove current tooltip(if any):
		if (m_pToolTip->GetSafeHwnd() != NULL)
		{
			m_pToolTip->ShowWindow(SW_HIDE);
		}

		if (m_pDelayedClosePopupMenuButton != NULL && m_pDelayedClosePopupMenuButton->Rect().PtInRect(ptCursor))
		{
			return;
		}

		CloseDelayedSubMenu();

		CMFCToolBarMenuButton* pDelayedPopupMenuButton = m_pDelayedPopupMenuButton;
		m_pDelayedPopupMenuButton = NULL;

		if (pDelayedPopupMenuButton != NULL && m_iHighlighted >= 0 && m_iHighlighted < m_Buttons.GetCount() && GetButton(m_iHighlighted) == pDelayedPopupMenuButton)
		{
			ASSERT_VALID(pDelayedPopupMenuButton);
			pDelayedPopupMenuButton->OpenPopupMenu(this);
		}
	}
	else if (nIDEvent == nRemovePopupTimerEvent)
	{
		KillTimer(nRemovePopupTimerEvent);

		if (m_pDelayedClosePopupMenuButton != NULL)
		{
			ASSERT_VALID(m_pDelayedClosePopupMenuButton);
			CMFCPopupMenu* pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, GetParent());

			if (m_pDelayedClosePopupMenuButton->Rect().PtInRect(ptCursor))
			{
				return;
			}

			m_pDelayedClosePopupMenuButton->OnCancelMode();
			m_pDelayedClosePopupMenuButton = NULL;

			if (pParentMenu != NULL)
			{
				CMFCPopupMenu::ActivatePopupMenu(AFXGetTopLevelFrame(this), pParentMenu);
			}
		}
	}
	else if (nIDEvent == AFX_ACCELERATOR_NOTIFY_EVENT)
	{
		KillTimer(AFX_ACCELERATOR_NOTIFY_EVENT);

		CRect rc;
		GetClientRect(&rc);
		if (!rc.PtInRect(ptCursor))
		{
			return;
		}

		int nIndex = HitTest(ptCursor);
		if (m_iAccHotItem == nIndex && m_iAccHotItem != -1)
		{
			AccNotifyObjectFocusEvent(nIndex);
		}
	}
}

void CMFCPopupMenuBar::StartPopupMenuTimer(CMFCToolBarMenuButton* pMenuButton, int nDelayFactor/* = 1*/)
{
	ASSERT(nDelayFactor > 0);

	if (m_pDelayedPopupMenuButton != NULL)
	{
		KillTimer(nPopupTimerEvent);
	}

	if ((m_pDelayedPopupMenuButton = pMenuButton) != NULL)
	{
		if (m_pDelayedPopupMenuButton == m_pDelayedClosePopupMenuButton)
		{
			RestoreDelayedSubMenu();
			m_pDelayedPopupMenuButton = NULL;
		}
		else
		{
			SetTimer(nPopupTimerEvent, m_uiPopupTimerDelay * nDelayFactor, NULL);
		}
	}
}

void CMFCPopupMenuBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bFirstClick = FALSE;
	m_bIsClickOutsideItem = TRUE;

	CRect rectClient;
	GetClientRect(&rectClient);

	if (!IsCustomizeMode() && !rectClient.PtInRect(point))
	{
		CMFCToolBar* pDestBar = FindDestintationToolBar(point);
		if (pDestBar != NULL)
		{
			ASSERT_VALID(pDestBar);

			CPoint ptDest = point;
			MapWindowPoints(pDestBar, &ptDest, 1);

			pDestBar->SendMessage( WM_LBUTTONDOWN, nFlags, MAKELPARAM(ptDest.x, ptDest.y));
		}
	}

	CMFCToolBar::OnLButtonDown(nFlags, point);
}

void CMFCPopupMenuBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	CRect rectClient;
	GetClientRect(&rectClient);

	if (!m_bFirstClick && !IsCustomizeMode() && !rectClient.PtInRect(point))
	{
		CMFCToolBar* pDestBar = FindDestintationToolBar(point);
		if (pDestBar != NULL)
		{
			MapWindowPoints(pDestBar, &point, 1);
			pDestBar->SendMessage( WM_LBUTTONUP, nFlags, MAKELPARAM(point.x, point.y));
		}

		CFrameWnd* pParentFrame = AFXGetParentFrame(this);
		ASSERT_VALID(pParentFrame);

		pParentFrame->DestroyWindow();
		return;
	}

	if (!IsCustomizeMode() && m_iHighlighted >= 0)
	{
		m_iButtonCapture = m_iHighlighted;
	}

	m_bFirstClick = FALSE;
	if (m_bIsClickOutsideItem)
	{
		CMFCToolBar::OnLButtonUp(nFlags, point);
	}
}

BOOL CMFCPopupMenuBar::OnSetDefaultButtonText(CMFCToolBarButton* pButton)
{
	ASSERT_VALID(pButton);

	CMFCPopupMenu* pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, GetParent());
	if (pParentMenu != NULL)
	{
		CMFCToolBar* pToolBar = pParentMenu->GetParentToolBar();
		if (pToolBar != NULL && pToolBar->OnSetDefaultButtonText(pButton))
		{
			return TRUE;
		}
	}

	return CMFCToolBar::OnSetDefaultButtonText(pButton);
}

BOOL CMFCPopupMenuBar::EnableContextMenuItems(CMFCToolBarButton* pButton, CMenu* pPopup)
{
	if (!CMFCToolBar::IsCustomizeMode())
	{
		// Disable context menu
		return FALSE;
	}

	ASSERT_VALID(pButton);
	ASSERT_VALID(pPopup);

	pButton->m_bText = TRUE;
	CMFCToolBar::EnableContextMenuItems(pButton, pPopup);

	pPopup->EnableMenuItem(ID_AFXBARRES_TOOLBAR_IMAGE, MF_GRAYED | MF_BYCOMMAND);
	pPopup->EnableMenuItem(ID_AFXBARRES_TOOLBAR_TEXT, MF_ENABLED | MF_BYCOMMAND);

	int iImage = pButton->GetImage();
	if (iImage < 0)
	{
		pPopup->EnableMenuItem(ID_AFXBARRES_TOOLBAR_IMAGE_AND_TEXT, ((CMFCToolBar::GetUserImages() != NULL) ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
	}
	else
	{
		pPopup->EnableMenuItem(ID_AFXBARRES_TOOLBAR_IMAGE_AND_TEXT, MF_ENABLED | MF_BYCOMMAND);
	}

	if (afxCommandManager->IsMenuItemWithoutImage(pButton->m_nID))
	{
		pPopup->CheckMenuItem(ID_AFXBARRES_TOOLBAR_TEXT, MF_CHECKED  | MF_BYCOMMAND);
		pPopup->CheckMenuItem(ID_AFXBARRES_TOOLBAR_IMAGE_AND_TEXT, MF_UNCHECKED  | MF_BYCOMMAND);
	}

	return TRUE;
}

void CMFCPopupMenuBar::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bFirstMove)
	{
		m_bFirstMove = FALSE;
		return;
	}

	if (m_ptCursor != CPoint(-1, -1))
	{
		CPoint ptCursor;
		::GetCursorPos(&ptCursor);

		if (ptCursor == m_ptCursor)
		{
			return;
		}

		m_ptCursor = ptCursor;
	}

	CRect rectClient;
	GetClientRect(&rectClient);

	if (IsCustomizeMode() || rectClient.PtInRect(point))
	{
		CMFCToolBar::OnMouseMove(nFlags, point);
	}
	else
	{
		CMFCToolBar* pDestBar = FindDestintationToolBar(point);
		if (pDestBar != NULL)
		{
			MapWindowPoints(pDestBar, &point, 1);
			pDestBar->SendMessage( WM_MOUSEMOVE, nFlags, MAKELPARAM(point.x, point.y));
		}
	}
}

CMFCToolBar* CMFCPopupMenuBar::FindDestintationToolBar(CPoint point)
{
	ScreenToClient(&point);

	CRect rectClient;

	CMFCPopupMenu* pPopupMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, GetParent());
	if (pPopupMenu == NULL)
	{
		return NULL;
	}

	ASSERT_VALID(pPopupMenu);

	CMFCPopupMenu* pLastPopupMenu = pPopupMenu;

	// Go up trougth all popup menus:
	while ((pPopupMenu = pPopupMenu->GetParentPopupMenu()) != NULL)
	{
		CMFCPopupMenuBar* pPopupMenuBar = pPopupMenu->GetMenuBar();
		ASSERT_VALID(pPopupMenuBar);

		pPopupMenuBar->GetClientRect(&rectClient);
		pPopupMenuBar->MapWindowPoints(this, &rectClient);

		if (rectClient.PtInRect(point))
		{
			return pPopupMenuBar;
		}

		pLastPopupMenu = pPopupMenu;
	}

	ASSERT_VALID(pLastPopupMenu);

	// Try parent toolbar:
	CMFCToolBar* pToolBar = pLastPopupMenu->GetParentToolBar();
	if (pToolBar != NULL)
	{
		pToolBar->GetClientRect(&rectClient);
		pToolBar->MapWindowPoints(this, &rectClient);

		if (rectClient.PtInRect(point))
		{
			return pToolBar;
		}
	}

	return NULL;
}

DROPEFFECT CMFCPopupMenuBar::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	// Disable MOVING menu item into one of submenus!
	if ((dwKeyState & MK_CONTROL) == 0)
	{
		CMFCPopupMenu* pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, GetParent());
		if (pParentMenu != NULL)
		{
			CMFCToolBar* pParentBar = pParentMenu->GetParentToolBar();
			CMFCToolBarMenuButton* pParentButton = pParentMenu->GetParentButton();

			if (pParentBar != NULL && pParentButton != NULL && pParentBar->IsDragButton(pParentButton))
			{
				return DROPEFFECT_NONE;
			}
		}
	}

	return CMFCToolBar::OnDragOver(pDataObject, dwKeyState, point);
}

void CMFCPopupMenuBar::OnFillBackground(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT(::IsWindow(GetSafeHwnd()));
	ASSERT_VALID(pDC);

	if (CMFCToolBar::IsCustomizeMode() || !CMFCMenuBar::m_bRecentlyUsedMenus || m_bPaletteMode)
	{
		return;
	}

	// Only menubar first-level menus may hide rarely used commands:
	CMFCPopupMenu* pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, GetParent());
	if (pParentMenu == NULL || !pParentMenu->HideRarelyUsedCommands())
	{
		return;
	}

	BOOL bFirstRarelyUsedButton = TRUE;
	CRect rectRarelyUsed;

	for (POSITION pos = m_Buttons.GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarButton* pButton = (CMFCToolBarButton*) m_Buttons.GetNext(pos);
		ENSURE(pButton != NULL);
		ASSERT_VALID(pButton);

		if (pButton->m_nStyle & TBBS_SEPARATOR)
		{
			if (pos != NULL && CMFCToolBar::IsCommandRarelyUsed(((CMFCToolBarButton*) m_Buttons.GetAt(pos))->m_nID))
			{
				continue;
			}
		}

		BOOL bDraw = FALSE;

		if (CMFCToolBar::IsCommandRarelyUsed(pButton->m_nID))
		{
			if (bFirstRarelyUsedButton)
			{
				bFirstRarelyUsedButton = FALSE;
				rectRarelyUsed = pButton->Rect();
			}

			if (pos == NULL) // Last button
			{
				rectRarelyUsed.bottom = pButton->Rect().bottom;
				bDraw = TRUE;
			}
		}
		else
		{
			if (!bFirstRarelyUsedButton)
			{
				rectRarelyUsed.bottom = pButton->Rect().top;
				bDraw = TRUE;
			}

			bFirstRarelyUsedButton = TRUE;
		}

		if (bDraw)
		{
			CMFCVisualManager::GetInstance()->OnHighlightRarelyUsedMenuItems(pDC, rectRarelyUsed);
		}
	}
}

INT_PTR CMFCPopupMenuBar::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	ASSERT_VALID(this);

	if (m_bPaletteMode)
	{
		return CMFCToolBar::OnToolHitTest(point, pTI);
	}

	int nHit = ((CMFCPopupMenuBar*)this)->HitTest(point);
	if (nHit != -1)
	{
		CMFCToolBarButton* pButton = DYNAMIC_DOWNCAST(CMFCToolBarButton, GetButton(nHit));

		if (pButton != NULL)
		{
			if (pTI != NULL)
			{
				pTI->uId = pButton->m_nID;
				pTI->hwnd = GetSafeHwnd();
				pTI->rect = pButton->Rect();
			}

			if (!pButton->OnToolHitTest(this, pTI))
			{
				nHit = pButton->m_nID;
			}
			else if (pTI != NULL && pTI->lpszText != NULL)
			{
				CString strText;

				if (pTI->lpszText != NULL)
				{
					strText = pTI->lpszText;
					::free(pTI->lpszText);
				}

				CString strDescr;
				CFrameWnd* pParent = GetParentFrame();
				if (pParent->GetSafeHwnd() != NULL && !pButton->IsKindOf (RUNTIME_CLASS(CMFCShowAllButton)))
				{
					pParent->GetMessageString(pButton->m_nID, strDescr);
				}

				CTooltipManager::SetTooltipText(pTI, m_pToolTip, AFX_TOOLTIP_TYPE_TOOLBAR, strText, strDescr);
			}
		}
	}

	return nHit;
}

int CMFCPopupMenuBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMFCToolBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (m_uiPopupTimerDelay == (UINT) -1) // Not defined yet
	{
		m_uiPopupTimerDelay = 500;

		CSettingsStoreSP regSP;
		CSettingsStore& reg = regSP.Create(FALSE, TRUE);

		if (reg.Open(_T("Control Panel\\Desktop")))
		{
			CString strVal;

			if (reg.Read(_T("MenuShowDelay"), strVal))
			{
				m_uiPopupTimerDelay = (UINT) _ttol(strVal);

				// Just limit it to 5 sec:
				m_uiPopupTimerDelay = min(5000, m_uiPopupTimerDelay);
			}
		}
	}

	::GetCursorPos(&m_ptCursor);

	return 0;
}

void CMFCPopupMenuBar::SetButtonStyle(int nIndex, UINT nStyle)
{
	CMFCToolBarButton* pButton = GetButton(nIndex);
	ENSURE(pButton != NULL);

	UINT nOldStyle = pButton->m_nStyle;
	if (nOldStyle != nStyle)
	{
		// update the style and invalidate
		pButton->m_nStyle = nStyle;

		// invalidate the button only if both styles not "pressed"
		if (!(nOldStyle & nStyle & TBBS_PRESSED))
		{
			CMFCToolBarMenuButton* pMenuButton = DYNAMIC_DOWNCAST(CMFCToolBarMenuButton, GetButton(nIndex));

			BOOL bWasChecked = nOldStyle & TBBS_CHECKED;
			BOOL bChecked = nStyle & TBBS_CHECKED;

			// If checked style was changed. redraw check box(or image) area only:
			if (pMenuButton != NULL && bWasChecked != bChecked)
			{
				CRect rectImage;
				pMenuButton->GetImageRect(rectImage);

				rectImage.InflateRect(afxData.cxBorder2 * 2, afxData.cyBorder2 * 2);

				InvalidateRect(rectImage);
				UpdateWindow();
			}
			else if ((nOldStyle ^ nStyle) != TBSTATE_PRESSED)
			{
				InvalidateButton(nIndex);
			}
		}
	}
}

LRESULT CMFCPopupMenuBar::OnIdleUpdateCmdUI(WPARAM, LPARAM)
{
	if (m_bTrackMode)
	{
		return 0;
	}

	// the style must be visible and if it is docked
	// the dockbar style must also be visible
	if (GetStyle() & WS_VISIBLE)
	{
		CFrameWnd* pTarget = (CFrameWnd*) GetCommandTarget();
		if (pTarget == NULL || !pTarget->IsFrameWnd())
		{
			pTarget = AFXGetParentFrame(this);
		}

		if (pTarget != NULL)
		{
			BOOL bAutoMenuEnable = FALSE;
			if (pTarget->IsFrameWnd())
			{
				bAutoMenuEnable = ((CFrameWnd*) pTarget)->m_bAutoMenuEnable;
			}

			OnUpdateCmdUI(pTarget, bAutoMenuEnable);
		}
	}

	return 0L;
}

CWnd* CMFCPopupMenuBar::GetCommandTarget() const
{
	if (m_bTrackMode)
	{
		return NULL;
	}

	CMFCPopupMenu* pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, GetParent());
	if (pParentMenu != NULL && pParentMenu->GetMessageWnd() != NULL)
	{
		return pParentMenu;
	}

	return CMFCToolBar::GetCommandTarget();
}

void CMFCPopupMenuBar::CloseDelayedSubMenu()
{
	ASSERT_VALID(this);

	if (m_pDelayedClosePopupMenuButton != NULL)
	{
		ASSERT_VALID(m_pDelayedClosePopupMenuButton);

		KillTimer(nRemovePopupTimerEvent);

		m_pDelayedClosePopupMenuButton->OnCancelMode();
		m_pDelayedClosePopupMenuButton = NULL;
	}
}

void CMFCPopupMenuBar::RestoreDelayedSubMenu()
{
	ASSERT_VALID(this);

	if (m_pDelayedClosePopupMenuButton == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pDelayedClosePopupMenuButton);
	m_pDelayedClosePopupMenuButton->m_bToBeClosed = FALSE;

	int iPrevHighlighted = m_iHighlighted;

	SetHot(m_pDelayedClosePopupMenuButton);

	m_iHighlighted = m_iHot;

	m_pDelayedClosePopupMenuButton = NULL;

	if (iPrevHighlighted != m_iHighlighted)
	{
		if (iPrevHighlighted >= 0)
		{
			InvalidateButton(iPrevHighlighted);
		}

		InvalidateButton(m_iHighlighted);
		UpdateWindow();
	}

	KillTimer(nRemovePopupTimerEvent);
}

BOOL CMFCPopupMenuBar::LoadFromHash(HMENU hMenu)
{
	return afxMenuHash.LoadMenuBar(hMenu, this);
}

void CMFCPopupMenuBar::SetInCommand(BOOL bInCommand)
{
	ASSERT_VALID(this);

	m_bInCommand = bInCommand;

	CMFCPopupMenu* pMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, GetParent());
	if (pMenu != NULL)
	{
		while ((pMenu = pMenu->GetParentPopupMenu()) != NULL)
		{
			CMFCPopupMenuBar* pMenuBar = pMenu->GetMenuBar();
			if (pMenuBar != NULL)
			{
				pMenuBar->SetInCommand(bInCommand);
			}
		}
	}
}

void CMFCPopupMenuBar::OnToolbarImageAndText()
{
	ASSERT(m_iSelected >= 0);

	CMFCToolBarButton* pButton = GetButton(m_iSelected);
	ENSURE(pButton != NULL);

	int iImage = pButton->GetImage();

	if (iImage < 0)
	{
		OnToolbarAppearance();
	}
	else
	{
		afxCommandManager->EnableMenuItemImage(pButton->m_nID, TRUE, pButton->m_bUserButton ? iImage : -1);
	}

	AdjustLayout();
}

void CMFCPopupMenuBar::OnToolbarText()
{
	ASSERT(m_iSelected >= 0);

	CMFCToolBarButton* pButton = GetButton(m_iSelected);
	ENSURE(pButton != NULL);

	afxCommandManager->EnableMenuItemImage(pButton->m_nID, FALSE);
	AdjustLayout();
}

void CMFCPopupMenuBar::AdjustLayout()
{
	ASSERT_VALID(this);

	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	AdjustLocations();

	Invalidate();
	UpdateWindow();

	if (!CMFCToolBar::IsCustomizeMode())
	{
		return;
	}

	CMFCPopupMenu* pParentMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, GetParent());
	if (pParentMenu != NULL)
	{
		ASSERT_VALID(pParentMenu);
		pParentMenu->RecalcLayout(FALSE);
	}
}

void CMFCPopupMenuBar::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	int iItem = HitTest(point);

	if (iItem >= 0)
	{
		CMFCToolBarMenuButton* pMenuItem = DYNAMIC_DOWNCAST(CMFCToolBarMenuButton, GetButton(iItem));
		if (pMenuItem != NULL && pMenuItem->m_nID == (UINT) -1)
		{
			CWnd::OnLButtonDblClk(nFlags, point);
			return;
		}
	}

	CMFCToolBar::OnLButtonDblClk(nFlags, point);
}

void CMFCPopupMenuBar::OnCalcSeparatorRect(CMFCToolBarButton* pButton, CRect& rectSeparator, BOOL bHorz)
{
	CRect rectClient;
	GetClientRect(rectClient);

	rectSeparator = pButton->Rect();

	if (pButton->m_bWrap && bHorz && m_bPaletteMode)
	{
		rectSeparator.right = rectClient.right;

		rectSeparator.top = pButton->Rect().bottom;
		rectSeparator.bottom = rectSeparator.top + AFX_TOOLBAR_LINE_OFFSET;
	}

}

void CMFCPopupMenuBar::OnAfterButtonDelete()
{
	AdjustLayout();
	RedrawWindow();
}

BOOL CMFCPopupMenuBar::BuildOrigItems(UINT uiMenuResID)
{
	ASSERT_VALID(this);

	while (!m_OrigButtons.IsEmpty())
	{
		delete m_OrigButtons.RemoveHead();
	}

	CWinAppEx* pApp = DYNAMIC_DOWNCAST(CWinAppEx, AfxGetApp());
	if (pApp == NULL || !pApp->IsResourceSmartUpdate())
	{
		return FALSE;
	}

	CMenu menu;
	if (!menu.LoadMenu(uiMenuResID))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CMenu* pMenu = menu.GetSubMenu(0);
	if (pMenu == NULL)
	{
		return FALSE;
	}

	int iCount = (int) pMenu->GetMenuItemCount();
	for (int i = 0; i < iCount; i ++)
	{
		UINT uiID = pMenu->GetMenuItemID(i);

		CString strText;

#ifdef _DEBUG
		pMenu->GetMenuString(i, strText, MF_BYPOSITION);
#endif

		switch (uiID)
		{
		case -1: // Pop-up menu
			{
				CMenu* pPopupMenu = pMenu->GetSubMenu(i);
				ENSURE(pPopupMenu != NULL);

				CMFCToolBarMenuButton* pButton = new CMFCToolBarMenuButton;
				ENSURE(pButton != NULL);
				ASSERT_VALID(pButton);

				pButton->Initialize(0, pPopupMenu->GetSafeHmenu(), -1, strText);
				m_OrigButtons.AddTail(pButton);
			}
			break;

		case 0: // Separator
			{
				CMFCToolBarButton* pButton = new CMFCToolBarButton;
				ENSURE(pButton != NULL);
				ASSERT_VALID(pButton);

				pButton->m_nStyle = TBBS_SEPARATOR;
				m_OrigButtons.AddTail(pButton);
			}
			break;

		default: // Regular command

			m_OrigButtons.AddTail(new CMFCToolBarButton(uiID, -1, strText));
			break;
		}
	}

	return TRUE;
}

void CMFCPopupMenuBar::ShowCommandMessageString(UINT uiCmdId)
{
	ASSERT_VALID(this);

	if (m_bDropDownListMode)
	{
		GetOwner()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
		return;
	}

	CMFCToolBar::ShowCommandMessageString(uiCmdId);
}



