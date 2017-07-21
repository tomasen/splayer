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
#include "afxcustomizebutton.h"
#include "afxglobals.h"
#include "afxtoolbar.h"
#include "afxmenuimages.h"
#include "afxtoolbarcomboboxbutton.h"
#include "afxribbonres.h"
#include "afxvisualmanager.h"
#include "afxdockingpanesrow.h"
#include "afxcustomizemenubutton.h"

BOOL CMFCCustomizeButton::m_bIgnoreLargeIconsMode = FALSE;

IMPLEMENT_SERIAL(CMFCCustomizeButton, CMFCToolBarMenuButton, VERSIONABLE_SCHEMA | 1)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFCCustomizeButton::CMFCCustomizeButton()
{
	CommonInit();
}

CMFCCustomizeButton::CMFCCustomizeButton(int iCustomizeCmdId, const CString& strCustomizeText)
{
	CommonInit();

	m_iCustomizeCmdId = iCustomizeCmdId;
	m_strCustomizeText = strCustomizeText;
}

void CMFCCustomizeButton::CommonInit()
{
	m_iCustomizeCmdId = 0;
	m_bIsEmpty = FALSE;
	m_bDefaultDraw = TRUE;
	m_sizeExtra = CSize(0, 0);
	m_pWndParentToolbar = NULL;
	m_bIsPipeStyle = TRUE;
	m_bOnRebar = FALSE;
	m_bMenuRightAlign = TRUE;
}

CMFCCustomizeButton::~CMFCCustomizeButton()
{
}

void CMFCCustomizeButton::OnChangeParentWnd(CWnd* pWndParent)
{
	CMFCToolBarButton::OnChangeParentWnd(pWndParent);

	m_pWndParentToolbar = DYNAMIC_DOWNCAST(CMFCToolBar, pWndParent);
	m_pWndParent = pWndParent;
	m_bText = FALSE;
	m_bIsEmpty = FALSE;
	m_bOnRebar = DYNAMIC_DOWNCAST(CReBar, pWndParent->GetParent()) != NULL;
}

void CMFCCustomizeButton::OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* /*pImages*/, BOOL bHorz,
	BOOL bCustomizeMode, BOOL bHighlight, BOOL /*bDrawBorder*/, BOOL /*bGrayDisabledButtons*/)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(this);

	if (m_bMenuMode)
	{
		ASSERT(FALSE); // Customize button is available for
		// the "pure" toolbars only!
		return;
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID(this);

	CRect rectBorder = rect;

	//----------------------
	// Fill button interior:
	//----------------------
	m_bDefaultDraw = TRUE;

	FillInterior(pDC, rectBorder, bHighlight || IsDroppedDown());

	int nMargin = CMFCVisualManager::GetInstance()->GetToolBarCustomizeButtonMargin();

	if (m_bDefaultDraw)
	{
		CSize sizeImage = CMenuImages::Size();
		if (CMFCToolBar::IsLargeIcons() && !m_bIgnoreLargeIconsMode)
		{
			sizeImage.cx *= 2;
			sizeImage.cy *= 2;
		}

		if (m_iCustomizeCmdId > 0)
		{
			//-----------------
			// Draw menu image:
			//-----------------
			CRect rectMenu = rect;
			if (bHorz)
			{
				rectMenu.top = rectMenu.bottom - sizeImage.cy - 2 * nMargin;
			}
			else
			{
				rectMenu.right = rectMenu.left + sizeImage.cx + 2 * nMargin;
			}

			if ((m_nStyle &(TBBS_PRESSED | TBBS_CHECKED)) || m_pPopupMenu != NULL)
			{
				if (!CMFCVisualManager::GetInstance()->IsMenuFlatLook())
				{
					rectMenu.OffsetRect(1, 1);
				}
			}

			CMenuImages::Draw(pDC, bHorz ? CMenuImages::IdArrowDown : CMenuImages::IdArrowLeft, rectMenu, CMenuImages::ImageBlack, sizeImage);
		}

		if (!m_lstInvisibleButtons.IsEmpty())
		{
			//-------------------
			// Draw "more" image:
			//-------------------
			CRect rectMore = rect;
			if (bHorz)
			{
				rectMore.bottom = rectMore.top + sizeImage.cy + 2 * nMargin;
			}
			else
			{
				rectMore.left = rectMore.right - sizeImage.cx - 2 * nMargin;
			}

			if ((m_nStyle &(TBBS_PRESSED | TBBS_CHECKED)) || m_pPopupMenu != NULL)
			{
				if (!CMFCVisualManager::GetInstance()->IsMenuFlatLook())
				{
					rectMore.OffsetRect(1, 1);
				}
			}

			CMenuImages::Draw(pDC, bHorz ? CMenuImages::IdMoreButtons : CMenuImages::IdArrowShowAll, rectMore, CMenuImages::ImageBlack, sizeImage);
		}
	}

	//--------------------
	// Draw button border:
	//--------------------
	if (!bCustomizeMode)
	{
		if ((m_nStyle &(TBBS_PRESSED | TBBS_CHECKED)) || m_pPopupMenu != NULL)
		{
			//-----------------------
			// Pressed in or checked:
			//-----------------------
			CMFCVisualManager::GetInstance()->OnDrawButtonBorder(pDC, this, rectBorder, CMFCVisualManager::ButtonsIsPressed);
		}
		else if (bHighlight && !(m_nStyle & TBBS_DISABLED) && !(m_nStyle &(TBBS_CHECKED | TBBS_INDETERMINATE)))
		{
			CMFCVisualManager::GetInstance()->OnDrawButtonBorder(pDC, this, rectBorder, CMFCVisualManager::ButtonsIsHighlighted);
		}
	}
}

CMFCPopupMenu* CMFCCustomizeButton::CreatePopupMenu()
{
	if (CMFCToolBar::m_bAltCustomizeMode || CMFCToolBar::IsCustomizeMode())
	{
		return NULL;
	}

	CMFCPopupMenu* pMenu = CMFCToolBarMenuButton::CreatePopupMenu();
	if (pMenu == NULL)
	{
		ASSERT(FALSE);
		return NULL;
	}

	if (m_pWndParentToolbar->IsLocked())
	{
		pMenu->GetMenuBar()->m_pRelatedToolbar = m_pWndParentToolbar;
	}

	pMenu->m_bRightAlign = m_bMenuRightAlign && (m_pWndParentToolbar->GetExStyle() & WS_EX_LAYOUTRTL) == 0;

	BOOL bIsLocked = (m_pWndParentToolbar == NULL || m_pWndParentToolbar->IsLocked());
	BOOL bIsFirst = TRUE;

	for (POSITION pos = m_lstInvisibleButtons.GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarButton* pButton = (CMFCToolBarButton*) m_lstInvisibleButtons.GetNext(pos);
		ASSERT_VALID(pButton);

		//--------------------------------------
		// Don't insert first or last separator:
		//--------------------------------------
		if (pButton->m_nStyle & TBBS_SEPARATOR)
		{
			if (bIsFirst)
			{
				continue;
			}

			if (pos == NULL) // Last
			{
				break;
			}
		}

		int iIndex = -1;

		bIsFirst = FALSE;

		if (pButton->IsKindOf(RUNTIME_CLASS(CMFCToolBarMenuButton)))
		{
			iIndex = pMenu->InsertItem(*((CMFCToolBarMenuButton*) pButton));
		}
		else
		{
			if (pButton->m_nID == 0)
			{
				iIndex = pMenu->InsertSeparator();
			}
			else
			{
				iIndex = pMenu->InsertItem(CMFCToolBarMenuButton(pButton->m_nID, NULL,
					bIsLocked ? - 1 : pButton->GetImage(), pButton->m_strText, pButton->m_bUserButton));
			}
		}

		if (iIndex < 0)
		{
			ASSERT(FALSE);
			continue;
		}

		CMFCToolBarMenuButton* pMenuButton = pMenu->GetMenuItem(iIndex);
		if (pMenuButton == NULL)
		{
			continue;
		}

		//-----------------------------------------------------
		// Text may be undefined, bring it from the tooltip :-(
		//-----------------------------------------------------
		if ((pMenuButton->m_strText.IsEmpty() || pButton->IsKindOf(RUNTIME_CLASS(CMFCToolBarComboBoxButton))) && pMenuButton->m_nID != 0)
		{
			CString strMessage;
			int iOffset;
			if (strMessage.LoadString(pMenuButton->m_nID) && (iOffset = strMessage.Find(_T('\n'))) != -1)
			{
				pMenuButton->m_strText = strMessage.Mid(iOffset + 1);
				if ((iOffset = pMenuButton->m_strText.Find(_T('\n'))) != -1)
				{
					pMenuButton->m_strText = pMenuButton->m_strText.Left( iOffset );
				}
			}
		}

		pMenuButton->m_bText = TRUE;
	}

	if (m_iCustomizeCmdId > 0)
	{
		if (!m_lstInvisibleButtons.IsEmpty())
		{
			pMenu->InsertSeparator();
		}

		if (m_pWndParentToolbar->IsAddRemoveQuickCustomize())
		{
			//--------------------------------
			// Prepare Quick Customize Items
			//--------------------------------

			CMFCPopupMenu* pMenuCustomize = new CMFCPopupMenu;

			CDockingPanesRow* pDockRow = m_pWndParentToolbar->GetPaneRow();
			if (pDockRow != NULL)
			{
				const CObList& list = pDockRow->GetPaneList();

				for (POSITION pos = list.GetHeadPosition(); pos != NULL;)
				{
					CMFCToolBar* pToolBar = DYNAMIC_DOWNCAST(CMFCToolBar, list.GetNext(pos));

					if (pToolBar != NULL && pToolBar->IsVisible() && pToolBar->IsExistCustomizeButton())
					{
						CString strCaption;
						pToolBar->GetWindowText(strCaption);

						strCaption.TrimLeft();
						strCaption.TrimRight();

						if (!strCaption.GetLength())
						{
							ENSURE(strCaption.LoadString(IDS_AFXBARRES_UNTITLED_TOOLBAR));
						}

						CString strToolId;
						strToolId.Format(_T("%d"), pToolBar->GetDlgCtrlID());

						//------------------------
						// Insert Dummy Menu Item
						//------------------------
						CMFCPopupMenu menuDummy;
						menuDummy.InsertItem(CMFCToolBarMenuButton(1, NULL, -1, strToolId));

						CMFCToolBarMenuButton btnToolCaption((UINT)-1, menuDummy.GetMenuBar()->ExportToMenu(), -1, strCaption);
						pMenuCustomize->InsertItem(btnToolCaption);
					}
				}
			}
			else
			{
				CString strCaption;
				m_pWndParentToolbar->GetWindowText(strCaption);

				strCaption.TrimLeft();
				strCaption.TrimRight();

				if (!strCaption.GetLength())
				{
					ENSURE(strCaption.LoadString(IDS_AFXBARRES_UNTITLED_TOOLBAR));
				}

				CString strToolId;
				strToolId.Format(_T("%d"), m_pWndParentToolbar->GetDlgCtrlID());

				//------------------------
				// Insert Dummy Menu Item
				//------------------------
				CMFCPopupMenu menuDummy;
				menuDummy.InsertItem(CMFCToolBarMenuButton(1, NULL, -1, strToolId)); //_T("DUMMY")

				CMFCToolBarMenuButton btnToolCaption((UINT)-1, menuDummy.GetMenuBar()->ExportToMenu(), -1, strCaption);

				pMenuCustomize->InsertItem(btnToolCaption);
			}

			CMFCToolBarMenuButton btnStandard(m_iCustomizeCmdId, NULL, -1, m_strCustomizeText);

			pMenuCustomize->InsertItem(btnStandard);

			CString strLabel;
			ENSURE(strLabel.LoadString(IDS_AFXBARRES_ADD_REMOVE_BTNS));

			CMFCToolBarMenuButton btnAddRemove((UINT)-1, pMenuCustomize->GetMenuBar()->ExportToMenu(), -1, strLabel);

			btnAddRemove.EnableQuickCustomize();

			delete pMenuCustomize;
			pMenuCustomize = NULL;

			//-----------------
			//Brothers Support
			//-----------------
			if (m_pWndParentToolbar != NULL && m_pWndParentToolbar->IsSibling())
			{
				if (m_pWndParentToolbar->CanHandleSiblings())
				{
					CString strText;

					if (m_pWndParentToolbar->IsOneRowWithSibling())
					{
						ENSURE(strText.LoadString(IDS_AFXBARRES_SHOWTWOROWS));
					}
					else
					{
						ENSURE(strText.LoadString(IDS_AFXBARRES_SHOWONEROW));
					}

					CMFCCustomizeMenuButton btnBrother(AFX_CUSTOMIZE_INTERNAL_ID, NULL, -1, strText, FALSE);
					CMFCCustomizeMenuButton::SetParentToolbar(m_pWndParentToolbar);
					btnBrother.SetSiblingsButton();
					pMenu->InsertItem(btnBrother);
				}
			}

			pMenu->InsertItem(btnAddRemove);
			pMenu->SetQuickMode();
			pMenu->SetQuickCustomizeType(CMFCPopupMenu::QUICK_CUSTOMIZE_ADDREMOVE);
		}
		else // for old version(< 6.5) compatibility.
		{
			CMFCToolBarMenuButton btnStandard(m_iCustomizeCmdId, NULL, -1, m_strCustomizeText);

			pMenu->InsertItem(btnStandard);
		}
	}

	//-----------------------------------------------------------
	// All menu commands should be routed via the same window as
	// parent toolbar commands:
	//-----------------------------------------------------------
	if (m_pWndParentToolbar != NULL)
	{
		pMenu->m_pMessageWnd = m_pWndParentToolbar->GetOwner();
	}

	return pMenu;
}

SIZE CMFCCustomizeButton::OnCalculateSize(CDC* /*pDC*/, const CSize& sizeDefault, BOOL bHorz)
{
	if (m_bIsEmpty)
	{
		return CSize(0, 0);
	}

	if (m_strText.IsEmpty())
	{
		ENSURE(m_strText.LoadString(IDS_AFXBARRES_TOOLBAR_OPTIONS));
		ENSURE(!m_strText.IsEmpty());
	}

	if (m_pWndParentToolbar != NULL && !m_pWndParentToolbar->IsDocked())
	{
		return CSize(0, 0);
	}

	int nMargin = CMFCVisualManager::GetInstance()->GetToolBarCustomizeButtonMargin();
	const int xLargeIcons = CMFCToolBar::IsLargeIcons() && !m_bIgnoreLargeIconsMode ? 2 : 1;

	if (bHorz)
	{
		return CSize( CMenuImages::Size().cx * xLargeIcons + 2 * nMargin, sizeDefault.cy);
	}
	else
	{
		return CSize( sizeDefault.cx, CMenuImages::Size().cy * xLargeIcons + 2 * nMargin);
	}
}

void CMFCCustomizeButton::CopyFrom(const CMFCToolBarButton& s)
{
	CMFCToolBarMenuButton::CopyFrom(s);
	const CMFCCustomizeButton& src = (const CMFCCustomizeButton&) s;

	m_iCustomizeCmdId = src.m_iCustomizeCmdId;
	m_strCustomizeText = src.m_strCustomizeText;
	m_bIsEmpty = src.m_bIsEmpty;
	m_bIsPipeStyle = src.m_bIsPipeStyle;
	m_bMenuRightAlign = src.m_bMenuRightAlign;
}

void CMFCCustomizeButton::OnCancelMode()
{
	CMFCToolBarMenuButton::OnCancelMode();

	if (m_sizeExtra != CSize(0, 0) && m_pWndParentToolbar != NULL)
	{
		int nIndex = m_pWndParentToolbar->ButtonToIndex(this);
		if (nIndex >= 0)
		{
			m_pWndParentToolbar->InvalidateButton(nIndex);
		}
	}
}

BOOL CMFCCustomizeButton::InvokeCommand(CMFCPopupMenuBar* pMenuBar, const CMFCToolBarButton* pButton)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pMenuBar);
	ASSERT_VALID(pButton);

	if (m_pWndParentToolbar == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(m_pWndParentToolbar);

	int nIndex = pMenuBar->ButtonToIndex(pButton);
	if (nIndex < 0)
	{
		return FALSE;
	}

	if (m_lstInvisibleButtons.GetCount()  > 0 )
	{
		CMFCToolBarButton* pButtonHead = (CMFCToolBarButton*)m_lstInvisibleButtons.GetHead();
		if (pButtonHead->m_nStyle & TBBS_SEPARATOR)
		{
			nIndex++;
		}
	}

	POSITION pos = m_lstInvisibleButtons.FindIndex(nIndex);
	if (pos == NULL)
	{
		return FALSE;
	}

	CMFCToolBarButton* pToolbarButton = (CMFCToolBarButton*) m_lstInvisibleButtons.GetAt(pos);
	ASSERT_VALID(pToolbarButton);

	UINT nIDCmd = pToolbarButton->m_nID;

	if (!m_pWndParentToolbar->OnSendCommand(pToolbarButton) && nIDCmd != 0 && nIDCmd != (UINT) -1)
	{
		CMFCToolBar::AddCommandUsage(nIDCmd);

		if (!pToolbarButton->OnClickUp() && (afxUserToolsManager == NULL || !afxUserToolsManager->InvokeTool(nIDCmd)))
		{
			m_pWndParentToolbar->GetOwner()->PostMessage(WM_COMMAND, nIDCmd);    // send command
		}
	}

	return TRUE;
}



