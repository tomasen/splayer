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

#include "afxpriv.h"
#include "afxribbonres.h"
#include "afxtoolbarscustomizedialog.h"
#include "afxtoolbar.h"
#include "afxmenubar.h"
#include "afxmousemanager.h"
#include "afxcontextmenumanager.h"
#include "afxmenutearoffmanager.h"
#include "afxtoolbarscommandslistbox.h"
#include "afxmdiframewndex.h"
#include "afxframewndex.h"
#include "afxkeyboardmanager.h"
#include "afxusertoolsmanager.h"
#include "afxtoolbarmenubutton.h"
#include "afxoutlookbar.h"
#include "afxusertool.h"
#include "afxvisualmanager.h"
#include "afxdocksite.h"
#include "afxrebar.h"
#include "afximageeditordialog.h"
#include "afxoleipframewndex.h"
#include "afxoledocipframewndex.h"

CMFCToolBarsCustomizeDialog* g_pWndCustomize = NULL;

static const int nButtonMargin = 8;

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarsCustomizeDialog

IMPLEMENT_DYNAMIC(CMFCToolBarsCustomizeDialog, CPropertySheet)

CMFCToolBarsCustomizeDialog::CMFCToolBarsCustomizeDialog(CFrameWnd* pWndParentFrame, BOOL bAutoSetFromMenus /* = FALSE */,
		UINT uiFlags /* = 0xFFFF */, CList<CRuntimeClass*,CRuntimeClass*>* plistCustomPages /* = NULL */) :
	CPropertySheet(_T(""), pWndParentFrame), m_bAutoSetFromMenus(bAutoSetFromMenus), m_uiFlags(uiFlags)
{

	if ((m_uiFlags & AFX_CUSTOMIZE_MENUAMPERS) == 0)
	{
		m_bSaveMenuAmps = FALSE;
	}
	else
	{
		m_bSaveMenuAmps = TRUE;
	}

	// Add optional custom pages:
	if (plistCustomPages != NULL)
	{
		// does not use lib local resources, so moved to front
		ASSERT_VALID(plistCustomPages);
		for (POSITION pos=plistCustomPages->GetHeadPosition(); pos; )
		{
			CRuntimeClass* pClass = plistCustomPages->GetNext(pos);
			m_listCustomPages.AddTail((CPropertyPage*) pClass->CreateObject() );
		}
	}

	ENSURE(pWndParentFrame != NULL);
	m_pParentFrame = pWndParentFrame;

	m_pCustomizePage = new CMFCToolBarsCommandsPropertyPage;
	m_pToolbarsPage = new CMFCToolBarsListPropertyPage(m_pParentFrame);
	m_pKeyboardPage = new CMFCToolBarsKeyboardPropertyPage(m_pParentFrame, m_bAutoSetFromMenus);
	m_pMenuPage = new CMFCToolBarsMenuPropertyPage(m_pParentFrame, m_bAutoSetFromMenus);
	m_pMousePage = new CMFCMousePropertyPage;

	// Add two main pages(available always):
	AddPage(m_pCustomizePage);
	AddPage(m_pToolbarsPage);

	// Add tools page:
	if (m_uiFlags & AFX_CUSTOMIZE_NOTOOLS)
	{
		m_pToolsPage = NULL;
	}
	else
	{
		m_pToolsPage = new CMFCToolBarsToolsPropertyPage();
		if (afxUserToolsManager != NULL)
		{
			AddPage(m_pToolsPage);
		}
	}

	// Add keyboard customization page(available only if
	// the main application windows accelerator table is exist):
	if (afxKeyboardManager != NULL && pWndParentFrame->m_hAccelTable != NULL)
	{
		AddPage(m_pKeyboardPage);
	}

	// Add menu customization page(available only if the menu bar or
	// context menu manager are available):
	BOOL bMenuBarIsAvailable = FALSE;

	CMDIFrameWndEx* pMainMDIFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, m_pParentFrame);
	if (pMainMDIFrame != NULL)
	{
		bMenuBarIsAvailable = (pMainMDIFrame->IsMenuBarAvailable());
	}
	else
	{
		CFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CFrameWndEx, m_pParentFrame);
		if (pMainFrame != NULL)
		{
			bMenuBarIsAvailable = (pMainFrame->IsMenuBarAvailable());
		}
	}

	if (afxContextMenuManager != NULL || bMenuBarIsAvailable)
	{
		AddPage(m_pMenuPage);
	}

	if (afxMouseManager != NULL)
	{
		AddPage(m_pMousePage);
	}

	// Add optional custom pages:
	for (POSITION pos=m_listCustomPages.GetHeadPosition(); pos; )
	{
		AddPage( m_listCustomPages.GetNext(pos) );
	}

	m_pOptionsPage = new CMFCToolBarsOptionsPropertyPage(bMenuBarIsAvailable);
	AddPage(m_pOptionsPage);

	// Set property sheet caption:
	CString strCaption;
	ENSURE(strCaption.LoadString(IDS_AFXBARRES_PROPSHT_CAPTION));

	ENSURE(m_strAllCommands.LoadString(IDS_AFXBARRES_ALL_COMMANDS));
	m_pCustomizePage->SetAllCategory(m_strAllCommands);

	if (m_pKeyboardPage != NULL)
	{
		m_pKeyboardPage->SetAllCategory(m_strAllCommands);
	}

	SetTitle(strCaption);

	if (m_bAutoSetFromMenus)
	{
		SetupFromMenus();
	}

	// Add a "New menu" button:
	CString strNewMenu;
	ENSURE(strNewMenu.LoadString(IDS_AFXBARRES_NEW_MENU));

	AddButton(strNewMenu, CMFCToolBarMenuButton(0, NULL, -1, strNewMenu));
}

CMFCToolBarsCustomizeDialog::~CMFCToolBarsCustomizeDialog()
{
	POSITION pos = m_ButtonsByCategory.GetStartPosition();
	while (pos != NULL)
	{
		CObList* pCategoryButtonsList;
		CString string;

		m_ButtonsByCategory.GetNextAssoc(pos, string, pCategoryButtonsList);
		ASSERT_VALID(pCategoryButtonsList);

		while (!pCategoryButtonsList->IsEmpty())
		{
			delete pCategoryButtonsList->RemoveHead();
		}

		delete pCategoryButtonsList;
	}

	m_ButtonsByCategory.RemoveAll();

	delete m_pCustomizePage;
	delete m_pToolbarsPage;
	delete m_pKeyboardPage;
	delete m_pMenuPage;
	delete m_pMousePage;
	delete m_pOptionsPage;

	if (m_pToolsPage != NULL)
	{
		delete m_pToolsPage;
	}

	// delete all optional custom pages:
	while (!m_listCustomPages.IsEmpty())
	{
		delete m_listCustomPages.RemoveHead();
	}
}

BEGIN_MESSAGE_MAP(CMFCToolBarsCustomizeDialog, CPropertySheet)
	//{{AFX_MSG_MAP(CMFCToolBarsCustomizeDialog)
	ON_WM_CREATE()
	ON_WM_HELPINFO()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCToolBarsCustomizeDialog message handlers

int CMFCToolBarsCustomizeDialog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPropertySheet::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	if (m_uiFlags & AFX_CUSTOMIZE_CONTEXT_HELP)
	{
		ModifyStyleEx(0, WS_EX_CONTEXTHELP);
	}

	g_pWndCustomize = this;
	return 0;
}

void CMFCToolBarsCustomizeDialog::PostNcDestroy()
{
	g_pWndCustomize = NULL;
	SetFrameCustMode(FALSE);

	CPropertySheet::PostNcDestroy();
	delete this;
}

void CMFCToolBarsCustomizeDialog::AddButton(UINT uiCategoryId, const CMFCToolBarButton& button, int iInsertBefore)
{
	CString strCategory;
	ENSURE(strCategory.LoadString(uiCategoryId));

	AddButton(strCategory, button, iInsertBefore);
}

void CMFCToolBarsCustomizeDialog::AddButton(LPCTSTR lpszCategory, const CMFCToolBarButton& button, int iInsertBefore)
{
	int iId = (int) button.m_nID;

	if (!button.IsEditable())
	{
		// Don't add protected, MRU, system and Window commands:
		return;
	}

	if (!CMFCToolBar::IsCommandPermitted(button.m_nID))
	{
		return;
	}

	CString strText = button.m_strText;
	strText.TrimLeft();
	strText.TrimRight();

	BOOL bToolBtn = FALSE;

	if (afxUserToolsManager != NULL && afxUserToolsManager->IsUserToolCmd(iId))
	{
		CUserTool* pTool = afxUserToolsManager->FindTool(iId);

		if (pTool == NULL)
		{
			// Undefined user tool, skip it
			return;
		}

		ASSERT_VALID(pTool);

		// Use tool name as label:
		strText = pTool->m_strLabel;
		bToolBtn = TRUE;
	}

	if (strText.IsEmpty())
	{
		// Try to find the command name in resources:
		CString strMessage;
		int iOffset;
		if (strMessage.LoadString(button.m_nID) && (iOffset = strMessage.Find(_T('\n'))) != -1)
		{
			strText = strMessage.Mid(iOffset + 1);
		}

		if (strText.IsEmpty() && lpszCategory == m_strAllCommands)
		{
			return;
		}
	}
	else
	{
		if (!m_bSaveMenuAmps)
		{
			strText.Remove(_T('&'));
		}

		// Remove trailing label(ex.:"\tCtrl+S"):
		int iOffset = strText.Find(_T('\t'));
		if (iOffset != -1)
		{
			strText = strText.Left(iOffset);
		}
	}

	// If text is still empty, assume dummy command and don't add it:
	if (strText.IsEmpty())
	{
		return;
	}

	// Find a category entry or create new if not exist:
	CObList* pCategoryButtonsList;
	if (!m_ButtonsByCategory.Lookup(lpszCategory, pCategoryButtonsList))
	{
		// Category not found! Create new:
		pCategoryButtonsList = new CObList;
		m_ButtonsByCategory.SetAt(lpszCategory, pCategoryButtonsList);

		if (lpszCategory != m_strAllCommands)
		{
			m_strCategoriesList.AddTail(lpszCategory);
		}
	}
	else
	{
		// Category is not a new. Maybe the button is exist also?
		ENSURE(pCategoryButtonsList != NULL);

		for (POSITION pos = pCategoryButtonsList->GetHeadPosition(); pos != NULL;)
		{
			CMFCToolBarButton* pButton = (CMFCToolBarButton*) pCategoryButtonsList->GetNext(pos);
			ENSURE(pButton != NULL);
			ASSERT_VALID(pButton);

			if ((pButton->m_nID == button.m_nID && pButton->m_nID != (UINT) -1) ||(pButton->m_nID == (UINT) -1 && pButton->m_strText == button.m_strText))
				// The same exist...
			{
				if (pButton->m_strText.IsEmpty())
				{
					pButton->m_strText = button.m_strText;
				}

				return;
			}
		}
	}

	// Create a new CMFCToolBarButton object(MFC class factory is used):
	CRuntimeClass* pClass = button.GetRuntimeClass();
	ENSURE(pClass != NULL);

	CMFCToolBarButton* pButton = (CMFCToolBarButton*) pClass->CreateObject();
	ENSURE(pButton != NULL);
	ASSERT_VALID(pButton);

	pButton->CopyFrom(button);
	pButton->m_strText = strText;

	if (bToolBtn)
	{
		pButton->SetImage(0);
	}

	// Add a new button to the specific category:
	BOOL bInserted = FALSE;
	if (iInsertBefore != -1)
	{
		POSITION pos = pCategoryButtonsList->FindIndex(iInsertBefore);
		if (pos != NULL)
		{
			pCategoryButtonsList->InsertBefore(pos, pButton);
			bInserted = TRUE;
		}
	}

	if (!bInserted)
	{
		pCategoryButtonsList->AddTail(pButton);
	}

	if (lpszCategory != m_strAllCommands)
	{
		AddButton(m_strAllCommands, button);
	}

	pButton->OnAddToCustomizePage();
}

int CMFCToolBarsCustomizeDialog::RemoveButton(UINT uiCategoryId, UINT uiCmdId)
{
	if (uiCategoryId == (UINT) -1) // Remove from ALL caregories
	{
		BOOL bFinish = FALSE;
		for (POSITION posCategory = m_strCategoriesList.GetHeadPosition(); !bFinish;)
		{
			CString strCategory;
			if (posCategory == NULL)
			{
				strCategory = m_strAllCommands;
				bFinish = TRUE;
			}
			else
			{
				strCategory = m_strCategoriesList.GetNext(posCategory);
			}

			RemoveButton(strCategory, uiCmdId);
		}

		return 0;
	}

	CString strCategory;
	ENSURE(strCategory.LoadString(uiCategoryId));

	return RemoveButton(strCategory, uiCmdId);
}

int CMFCToolBarsCustomizeDialog::RemoveButton(LPCTSTR lpszCategory, UINT uiCmdId)
{
	ENSURE(lpszCategory != NULL);

	CObList* pCategoryButtonsList;
	if (!m_ButtonsByCategory.Lookup(lpszCategory, pCategoryButtonsList))
	{
		// Category not found!
		return -1;
	}

	int i = 0;
	for (POSITION pos = pCategoryButtonsList->GetHeadPosition(); pos != NULL; i ++)
	{
		POSITION posSave = pos;

		CMFCToolBarButton* pButton = (CMFCToolBarButton*) pCategoryButtonsList->GetNext(pos);
		ENSURE(pButton != NULL);
		ASSERT_VALID(pButton);

		if (pButton->m_nID == uiCmdId)
		{
			pCategoryButtonsList->RemoveAt(posSave);
			delete pButton;
			return i;
		}
	}

	return -1;
}

BOOL CMFCToolBarsCustomizeDialog::AddToolBar(UINT uiCategory, UINT uiToolbarResId)
{
	CString strCategory;
	ENSURE(strCategory.LoadString(uiCategory));

	return AddToolBar(strCategory, uiToolbarResId);
}

BOOL CMFCToolBarsCustomizeDialog::AddToolBar(LPCTSTR lpszCategory, UINT uiToolbarResId)
{
	struct CToolBarData
	{
		WORD wVersion;
		WORD wWidth;
		WORD wHeight;
		WORD wItemCount;

		WORD* items()
		{ return(WORD*)(this+1); }
	};

	LPCTSTR lpszResourceName = MAKEINTRESOURCE(uiToolbarResId);
	ENSURE(lpszResourceName != NULL);

	// determine location of the bitmap in resource fork:
	HINSTANCE hInst = AfxFindResourceHandle(lpszResourceName, RT_TOOLBAR);
	HRSRC hRsrc = ::FindResource(hInst, lpszResourceName, RT_TOOLBAR);
	if (hRsrc == NULL)
	{
		TRACE(_T("CMFCToolBarsCustomizeDialog::AddToolBar: Can't load toolbar %x\n"), uiToolbarResId);
		return FALSE;
	}

	HGLOBAL hGlobal = ::LoadResource(hInst, hRsrc);
	if (hGlobal == NULL)
	{
		TRACE(_T("CMFCToolBarsCustomizeDialog::AddToolBar: Can't load toolbar %x\n"), uiToolbarResId);
		return FALSE;
	}

	CToolBarData* pData = (CToolBarData*)LockResource(hGlobal);
	if (pData == NULL)
	{
		TRACE(_T("CMFCToolBarsCustomizeDialog::AddToolBar: Can't load toolbar %x\n"), uiToolbarResId);
		::FreeResource(hGlobal);
		return FALSE;
	}

	ASSERT(pData->wVersion == 1);

	for (int i = 0; i < pData->wItemCount; i++)
	{
		UINT uiCmd = pData->items() [i];
		if (uiCmd > 0 && uiCmd != (UINT) -1)
		{
			AddButton(lpszCategory, CMFCToolBarButton(uiCmd, -1));
		}
	}

	::UnlockResource(hGlobal);
	::FreeResource(hGlobal);

	return TRUE;
}

BOOL CMFCToolBarsCustomizeDialog::AddMenu(UINT uiMenuResId)
{
	CMenu menu;
	if (!menu.LoadMenu(uiMenuResId))
	{
		TRACE(_T("CMFCToolBarsCustomizeDialog::AddMenu: Can't load menu %x\n"), uiMenuResId);
		return FALSE;
	}

	AddMenuCommands(&menu, FALSE);
	return TRUE;
}

// Rename automatically imported categories(e.g. "?"->"Help")
BOOL CMFCToolBarsCustomizeDialog::RenameCategory(LPCTSTR lpszCategoryOld, LPCTSTR lpszCategoryNew)
{
	// New Name must not be present
	POSITION pos = m_strCategoriesList.Find(lpszCategoryNew);
	if (pos)
		return FALSE;

	// ...but the old one must be
	pos = m_strCategoriesList.Find(lpszCategoryOld);
	if (!pos)
		return FALSE;

	// Change Name in Button-map too:
	CObList* pCategoryButtonsList;

	// new Category must not be present yet
	if (m_ButtonsByCategory.Lookup(lpszCategoryNew, pCategoryButtonsList))
		return FALSE;

	// ...but the old one must be
	if (!m_ButtonsByCategory.Lookup(lpszCategoryOld, pCategoryButtonsList))
		return FALSE;

	// change both or nothing
	m_strCategoriesList.SetAt(pos, lpszCategoryNew);
	m_ButtonsByCategory.RemoveKey(lpszCategoryOld);
	m_ButtonsByCategory.SetAt(lpszCategoryNew, pCategoryButtonsList);

	return TRUE;
}

void CMFCToolBarsCustomizeDialog::ReplaceButton(UINT uiCmd, const CMFCToolBarButton& button)
{
	CRuntimeClass* pClass = button.GetRuntimeClass();
	ENSURE(pClass != NULL);

	BOOL bFinish = FALSE;
	for (POSITION posCategory = m_strCategoriesList.GetHeadPosition(); !bFinish;)
	{
		CString strCategory;
		if (posCategory == NULL)
		{
			strCategory = m_strAllCommands;
			bFinish = TRUE;
		}
		else
		{
			strCategory = m_strCategoriesList.GetNext(posCategory);
		}

		CObList* pCategoryButtonsList;
		if (!m_ButtonsByCategory.Lookup(strCategory, pCategoryButtonsList) ||
			pCategoryButtonsList == NULL)
		{
			ASSERT(FALSE);
			continue;
		}

		ASSERT_VALID(pCategoryButtonsList);

		for (POSITION pos = pCategoryButtonsList->GetHeadPosition(); pos != NULL;)
		{
			POSITION posSave = pos;
			CMFCToolBarButton* pButton = (CMFCToolBarButton*) pCategoryButtonsList->GetNext(pos);
			ENSURE(pButton != NULL);
			ASSERT_VALID(pButton);

			if (pButton->m_nID == uiCmd) // Found!
			{
				CMFCToolBarButton* pNewButton = (CMFCToolBarButton*) pClass->CreateObject();
				ASSERT_VALID(pNewButton);

				pNewButton->CopyFrom(button);
				if (pNewButton->m_strText.IsEmpty())
				{
					pNewButton->m_strText = pButton->m_strText;
				}

				pCategoryButtonsList->SetAt(posSave, pNewButton);
				delete pButton;
			}
		}
	}
}

BOOL CMFCToolBarsCustomizeDialog::SetUserCategory(LPCTSTR lpszCategory)
{
	ENSURE(lpszCategory != NULL);

	CObList* pCategoryButtonsList;
	if (!m_ButtonsByCategory.Lookup(lpszCategory, pCategoryButtonsList))
	{
		TRACE(_T("CMFCToolBarsCustomizeDialog::SetUserCategory: Can't find category '%s'\n"), lpszCategory);
		return FALSE;
	}

	m_pCustomizePage->SetUserCategory(lpszCategory);
	return TRUE;
}

void CMFCToolBarsCustomizeDialog::SetFrameCustMode(BOOL bCustMode)
{
	ASSERT_VALID(m_pParentFrame);

	CWaitCursor wait;

	// Enable/disable all parent frame child windows(except docking bars
	// and our toolbars):
	CWnd* pWndChild = m_pParentFrame->GetWindow(GW_CHILD);
	while (pWndChild != NULL)
	{
		CRuntimeClass* pChildClass = pWndChild->GetRuntimeClass();
		if (pChildClass == NULL || (!pChildClass->IsDerivedFrom(RUNTIME_CLASS(CDockBar)) &&
			!pChildClass->IsDerivedFrom(RUNTIME_CLASS(CDockSite)) && !pChildClass->IsDerivedFrom(RUNTIME_CLASS(CMFCOutlookBar)) &&
			!pChildClass->IsDerivedFrom(RUNTIME_CLASS(CMFCReBar)) && !pChildClass->IsDerivedFrom(RUNTIME_CLASS(CMFCToolBar))))
		{
			pWndChild->EnableWindow(!bCustMode);
		}

		pWndChild = pWndChild->GetNextWindow();
	}

	// Enable/Disable floating frames:
	CDockingManager* pDockManager = NULL;

	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, m_pParentFrame);
	if (pMainFrame != NULL)
	{
		pDockManager = pMainFrame->GetDockingManager();
	}
	else // Maybe, SDI frame...
	{
		CFrameWndEx* pFrame = DYNAMIC_DOWNCAST(CFrameWndEx, m_pParentFrame);
		if (pFrame != NULL)
		{
			pDockManager = pFrame->GetDockingManager();

		}
		else // Maybe, OLE frame
		{
			COleIPFrameWndEx* pOleFrame = DYNAMIC_DOWNCAST(COleIPFrameWndEx, m_pParentFrame);
			if (pOleFrame != NULL)
			{
				pDockManager = pOleFrame->GetDockingManager();
			}
			else
			{
				COleDocIPFrameWndEx* pOleDocFrame = DYNAMIC_DOWNCAST(COleDocIPFrameWndEx, m_pParentFrame);
				if (pOleDocFrame != NULL)
				{
					pDockManager = pOleDocFrame->GetDockingManager();
				}
			}
		}
	}

	if (pDockManager != NULL)
	{
		ASSERT_VALID(pDockManager);

		const CObList& lstMiniFrames = pDockManager->GetMiniFrames();
		for (POSITION pos = lstMiniFrames.GetHeadPosition(); pos != NULL;)
		{
			CPaneFrameWnd* pMiniFrame = DYNAMIC_DOWNCAST(CPaneFrameWnd, lstMiniFrames.GetNext(pos));
			if (pMiniFrame != NULL && DYNAMIC_DOWNCAST(CMFCBaseToolBar, pMiniFrame->GetPane()) == NULL)
			{
				pMiniFrame->EnableWindow(!bCustMode);
			}
		}
	}

	m_pParentFrame->LockWindowUpdate();

	// Set/reset costumize mode for ALL our toolbars:
	CMFCToolBar::SetCustomizeMode(bCustMode);

	// Inform the parent frame about mode(for additional actions):
	m_pParentFrame->SendMessage(AFX_WM_CUSTOMIZETOOLBAR, (WPARAM) bCustMode);

	m_pParentFrame->UnlockWindowUpdate();

	if (!bCustMode && m_pParentFrame->GetActiveFrame() != NULL)
	{
		// Restore active view:
		m_pParentFrame->GetActiveFrame()->PostMessage(WM_SETFOCUS);
	}
}

BOOL CMFCToolBarsCustomizeDialog::Create()
{
	DWORD dwExStyle = 0;

	if ((m_pParentFrame != NULL) &&(m_pParentFrame->GetExStyle() & WS_EX_LAYOUTRTL))
	{
		dwExStyle |= WS_EX_LAYOUTRTL;
	}

	if (!CPropertySheet::Create(m_pParentFrame, (DWORD)-1, dwExStyle))
	{
		return FALSE;
	}

	SetFrameCustMode(TRUE);
	return TRUE;
}

void CMFCToolBarsCustomizeDialog::ShowToolBar(CMFCToolBar* pToolBar, BOOL bShow)
{
	m_pToolbarsPage->ShowToolBar(pToolBar, bShow);
}

BOOL CMFCToolBarsCustomizeDialog::OnInitDialog()
{
	BOOL bResult = CPropertySheet::OnInitDialog();

	CRect rectClient; // Client area rectangle
	GetClientRect(&rectClient);

	// Show "Cancel" button:
	CWnd *pWndCancel = GetDlgItem(IDCANCEL);
	if (pWndCancel == NULL)
	{
		return bResult;
	}

	pWndCancel->ShowWindow(SW_SHOW);
	pWndCancel->EnableWindow();

	CRect rectClientCancel;
	pWndCancel->GetClientRect(&rectClientCancel);
	pWndCancel->MapWindowPoints(this, &rectClientCancel);

	// Enlarge property sheet window:
	CRect rectWnd;
	GetWindowRect(rectWnd);

	SetWindowPos(NULL, 0, 0, rectWnd.Width(), rectWnd.Height() + rectClientCancel.Height() + 2 * nButtonMargin, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	// Move "Cancel" button to the right bottom corner:
	pWndCancel->SetWindowPos(NULL, rectClient.right - rectClientCancel.Width() - nButtonMargin,
		rectClientCancel.top + nButtonMargin / 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	// Change "Cancel" button's style to "DEFPUSHBUTTON":
	CWnd *pWndOk = GetDlgItem(IDOK);
	if (pWndOk != NULL)
	{
		pWndOk->ModifyStyle(BS_DEFPUSHBUTTON, 0);
	}

	pWndCancel->ModifyStyle(0, BS_DEFPUSHBUTTON);

	// Replace "Cancel" text to "Close"
	//(CPropertyPage::CancelToClose method does nothing in a
	// modeless property sheet):
	CString strCloseText;
	ENSURE(strCloseText.LoadString(IDS_AFXBARRES_CLOSE));

	pWndCancel->SetWindowText(strCloseText);

	// Ensure that the dialog is fully visible on screen
	CRect rectDialog;
	GetWindowRect(&rectDialog);

	int cxScreen = GetSystemMetrics(SM_CXSCREEN);
	int cyScreen = GetSystemMetrics(SM_CYMAXIMIZED) - (GetSystemMetrics(SM_CYSCREEN) - GetSystemMetrics(SM_CYMAXIMIZED));

	if ((rectDialog.left < 0) || (rectDialog.top < 0))
	{
		SetWindowPos(NULL, rectDialog.left < 0 ? 0 : rectDialog.left, rectDialog.top < 0 ? 0 : rectDialog.top, 0, 0, SWP_NOSIZE);
	}
	else if ((rectDialog.right > cxScreen) || (rectDialog.bottom > cyScreen))
	{
		SetWindowPos(NULL, rectDialog.right > cxScreen ? cxScreen - rectDialog.Width() : rectDialog.left, rectDialog.bottom > cyScreen ? cyScreen - rectDialog.Height() : rectDialog.top, 0, 0, SWP_NOSIZE);
	}

	// Adjust the Help button:
	CButton *pWndHelp = (CButton*) GetDlgItem(IDHELP);
	if (pWndHelp == NULL)
	{
		return bResult;
	}

	if (m_uiFlags & AFX_CUSTOMIZE_NOHELP)
	{
		pWndHelp->ShowWindow(SW_HIDE);
		pWndHelp->EnableWindow(FALSE);
	}
	else
	{
		m_btnHelp.SubclassWindow(pWndHelp->GetSafeHwnd());
		m_btnHelp.ShowWindow(SW_SHOW);
		m_btnHelp.EnableWindow();

		// Set Help button image:
		m_btnHelp.SetImage(afxGlobalData.Is32BitIcons() ? IDB_AFXBARRES_HELP32 : IDB_AFXBARRES_HELP);
		m_btnHelp.SetWindowText(_T(""));

		// Move "Help" button to the left bottom corner and
		// adjust its size by the bitmap size:

		CSize sizeHelp = m_btnHelp.SizeToContent(TRUE);

		m_btnHelp.SetWindowPos(NULL, rectClient.left + nButtonMargin, rectClientCancel.top, sizeHelp.cx, sizeHelp.cy, SWP_NOZORDER | SWP_NOACTIVATE);
	}

	return bResult;
}

void CMFCToolBarsCustomizeDialog::OnClose()
{
	if (afxUserToolsManager != NULL && m_pToolsPage != NULL)
	{
		if (!CheckToolsValidity(afxUserToolsManager->GetUserTools()))
		{
			// Continue customization....
			if (GetActivePage() != m_pToolsPage)
			{
				SetActivePage(m_pToolsPage);
			}

			return;
		}
	}

	CPropertySheet::OnClose();
}

BOOL CMFCToolBarsCustomizeDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
	case IDCANCEL:
		if (afxUserToolsManager != NULL && m_pToolsPage != NULL)
		{
			if (!CheckToolsValidity(afxUserToolsManager->GetUserTools()))
			{
				// Continue customization....
				if (GetActivePage() != m_pToolsPage)
				{
					SetActivePage(m_pToolsPage);
				}

				return TRUE;
			}
		}

		DestroyWindow();
		return TRUE;

	case IDHELP:
		ASSERT_VALID(m_pParentFrame);
		m_pParentFrame->SendMessage(AFX_WM_CUSTOMIZEHELP, GetActiveIndex(), (LPARAM) this);
		return TRUE;
	}

	return CPropertySheet::OnCommand(wParam, lParam);
}

void CMFCToolBarsCustomizeDialog::EnableUserDefinedToolbars(BOOL bEnable)
{
	m_pToolbarsPage->EnableUserDefinedToolbars(bEnable);
}

void CMFCToolBarsCustomizeDialog::AddMenuCommands(const CMenu* pMenu, BOOL bPopup, LPCTSTR lpszCategory, LPCTSTR lpszMenuPath)
{
	ENSURE(pMenu != NULL);
	ASSERT_VALID(pMenu);

	BOOL bIsWindowsMenu = FALSE;
	int iCount = (int) pMenu->GetMenuItemCount();

	for (int i = 0; i < iCount; i ++)
	{
		UINT uiCmd = pMenu->GetMenuItemID(i);

		CString strText;
		pMenu->GetMenuString(i, strText, MF_BYPOSITION);

		if (!m_bSaveMenuAmps)
		{
			strText.Remove(_T('&'));
		}

		switch (uiCmd)
		{
		case 0: // Separator, ignore it.
			break;

		case -1: // Submenu
			{
				CMenu* pSubMenu = pMenu->GetSubMenu(i);

				UINT uiTearOffId = 0;
				if (g_pTearOffMenuManager != NULL)
				{
					uiTearOffId = g_pTearOffMenuManager->Parse(strText);
				}

				CString strCategory = strText;
				strCategory.Remove(_T('&'));

				if (lpszCategory != NULL)
				{
					strCategory = lpszCategory;
				}

				if (m_bAutoSetFromMenus)
				{
					if (bPopup)
					{
						CMFCToolBarMenuButton menuButton((UINT) -1, pSubMenu->GetSafeHmenu(), -1, strText);

						menuButton.SetTearOff(uiTearOffId);
						AddButton(strCategory, menuButton);
					}

					CString strPath;
					if (lpszMenuPath != NULL)
					{
						strPath = lpszMenuPath;
					}

					strPath += strText;
					AddMenuCommands(pSubMenu, bPopup, strCategory, strPath);
				}
				else
				{
					AddMenuCommands(pSubMenu, bPopup);
				}

			}
			break;

		default:
			if (bPopup && uiCmd >= AFX_IDM_WINDOW_FIRST && uiCmd <= AFX_IDM_WINDOW_LAST)
			{
				bIsWindowsMenu = TRUE;
			}

			if (lpszCategory != NULL && afxUserToolsManager != NULL && afxUserToolsManager->GetToolsEntryCmd() == uiCmd)
			{
				// Replace tools entry by the actual tools list:
				AddUserTools(lpszCategory);
			}
			else
			{
				CMFCToolBarButton button(uiCmd, -1, strText);

				if (lpszMenuPath != NULL)
				{
					CString strCustom = CString(lpszMenuPath) + button.m_strText;

					LPTSTR pszCustom = strCustom.GetBuffer(strCustom.GetLength() + 1);

					for (int j = 0; j < lstrlen(pszCustom) - 1; j++)
					{
						if (pszCustom [j] == _TCHAR(' '))
						{
							CharUpperBuff(&pszCustom [j + 1], 1);
						}
					}

					strCustom.ReleaseBuffer();

					strCustom.Remove(_T(' '));
					button.m_strTextCustom = strCustom.SpanExcluding(_T("\t"));
				}

				AddButton(lpszCategory == NULL ? m_strAllCommands : lpszCategory, button);
			}
		}
	}

	// Add windows manager item:
	if (bIsWindowsMenu && lpszCategory != NULL)
	{
		CMDIFrameWndEx* pMainMDIFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, m_pParentFrame);
		if (pMainMDIFrame != NULL && pMainMDIFrame->m_uiWindowsDlgMenuId != 0 && pMainMDIFrame->m_bShowWindowsDlgAlways)
		{
			AddButton(lpszCategory, CMFCToolBarButton(pMainMDIFrame->m_uiWindowsDlgMenuId, -1, pMainMDIFrame->m_strWindowsDlgMenuText));
		}
	}
}

void CMFCToolBarsCustomizeDialog::FillCategoriesComboBox(CComboBox& wndCategory, BOOL bAddEmpty) const
{
	CObList* pCategoryButtonsList;

	for (POSITION pos = m_strCategoriesList.GetHeadPosition(); pos != NULL;)
	{
		CString strCategory = m_strCategoriesList.GetNext(pos);

		if (!m_ButtonsByCategory.Lookup(strCategory, pCategoryButtonsList))
		{
			ASSERT(FALSE);
		}

		ASSERT_VALID(pCategoryButtonsList);

		BOOL bIsEmpty = FALSE;

		if (!bAddEmpty)
		{
			bIsEmpty = TRUE;
			for (POSITION posCat = pCategoryButtonsList->GetHeadPosition(); posCat != NULL;)
			{
				CMFCToolBarButton* pButton = (CMFCToolBarButton*) pCategoryButtonsList->GetNext(posCat);
				ASSERT_VALID(pButton);

				if (pButton->m_nID > 0 && pButton->m_nID != (UINT) -1)
				{
					bIsEmpty = FALSE;
					break;
				}
			}
		}

		if (!bIsEmpty)
		{
			int iIndex = wndCategory.AddString(strCategory);
			wndCategory.SetItemData(iIndex, (DWORD_PTR) pCategoryButtonsList);
		}
	}

	// "All" category should be last!
	if (!m_ButtonsByCategory.Lookup(m_strAllCommands, pCategoryButtonsList))
	{
		ASSERT(FALSE);
	}

	ASSERT_VALID(pCategoryButtonsList);

	int iIndex = wndCategory.AddString(m_strAllCommands);
	wndCategory.SetItemData(iIndex, (DWORD_PTR) pCategoryButtonsList);
}

void CMFCToolBarsCustomizeDialog::FillCategoriesListBox(CListBox& wndCategory, BOOL bAddEmpty) const
{
	CObList* pCategoryButtonsList;

	for (POSITION pos = m_strCategoriesList.GetHeadPosition(); pos != NULL;)
	{
		CString strCategory = m_strCategoriesList.GetNext(pos);

		if (!m_ButtonsByCategory.Lookup(strCategory, pCategoryButtonsList))
		{
			ASSERT(FALSE);
		}

		ASSERT_VALID(pCategoryButtonsList);

		BOOL bIsEmpty = FALSE;

		if (!bAddEmpty)
		{
			bIsEmpty = TRUE;
			for (POSITION posCat = pCategoryButtonsList->GetHeadPosition(); posCat != NULL;)
			{
				CMFCToolBarButton* pButton = (CMFCToolBarButton*) pCategoryButtonsList->GetNext(posCat);
				ASSERT_VALID(pButton);

				if (pButton->m_nID > 0 && pButton->m_nID != (UINT) -1)
				{
					bIsEmpty = FALSE;
					break;
				}
			}
		}

		if (!bIsEmpty)
		{
			int iIndex = wndCategory.AddString(strCategory);
			wndCategory.SetItemData(iIndex, (DWORD_PTR) pCategoryButtonsList);
		}
	}

	// "All" category should be last!
	if (!m_ButtonsByCategory.Lookup(m_strAllCommands, pCategoryButtonsList))
	{
		ASSERT(FALSE);
	}

	ASSERT_VALID(pCategoryButtonsList);

	int iIndex = wndCategory.AddString(m_strAllCommands);
	wndCategory.SetItemData(iIndex, (DWORD_PTR) pCategoryButtonsList);
}

void CMFCToolBarsCustomizeDialog::FillAllCommandsList(CListBox& wndListOfCommands) const
{
	wndListOfCommands.ResetContent();

	CObList* pAllButtonsList;
	if (!m_ButtonsByCategory.Lookup(m_strAllCommands, pAllButtonsList))
	{
		return;
	}

	ASSERT_VALID(pAllButtonsList);

	for (POSITION pos = pAllButtonsList->GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarButton* pButton = (CMFCToolBarButton*) pAllButtonsList->GetNext(pos);
		ASSERT_VALID(pButton);

		int iIndex = wndListOfCommands.AddString(pButton->m_strTextCustom.IsEmpty() ? pButton->m_strText : pButton->m_strTextCustom);
		wndListOfCommands.SetItemData(iIndex, (DWORD) pButton->m_nID);
	}
}

BOOL CMFCToolBarsCustomizeDialog::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	ASSERT_VALID(m_pParentFrame);
	m_pParentFrame->SendMessage(AFX_WM_CUSTOMIZEHELP, GetActiveIndex(), (LPARAM) this);

	return TRUE;
}

LPCTSTR CMFCToolBarsCustomizeDialog::GetCommandName(UINT uiCmd) const
{
	CObList* pAllButtonsList;
	if (!m_ButtonsByCategory.Lookup(m_strAllCommands, pAllButtonsList))
	{
		return NULL;
	}

	ENSURE(pAllButtonsList != NULL);
	ASSERT_VALID(pAllButtonsList);

	for (POSITION pos = pAllButtonsList->GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarButton* pButton = (CMFCToolBarButton*) pAllButtonsList->GetNext(pos);
		ENSURE(pButton != NULL);
		ASSERT_VALID(pButton);

		if (pButton->m_nID == uiCmd)
		{
			return pButton->m_strText;
		}
	}

	return NULL;
}

void CMFCToolBarsCustomizeDialog::SetupFromMenus()
{
	// Find all application document templates and add menue items to the
	// "All commands" category:
	CDocManager* pDocManager = AfxGetApp()->m_pDocManager;
	if (pDocManager != NULL)
	{
		// Walk all templates in the application:
		for (POSITION pos = pDocManager->GetFirstDocTemplatePosition(); pos != NULL;)
		{
			CMultiDocTemplate* pTemplate = DYNAMIC_DOWNCAST( CMultiDocTemplate, pDocManager->GetNextDocTemplate(pos));
			if (pTemplate != NULL)
			{
				CMenu* pDocMenu = CMenu::FromHandle(pTemplate->m_hMenuShared);
				if (pDocMenu != NULL)
				{
					AddMenuCommands(pDocMenu, FALSE);
				}
			}
		}
	}

	// Add commands from the default menu:
	CMenu* pFrameMenu = CMenu::FromHandle(m_pParentFrame->m_hMenuDefault);
	if (pFrameMenu == NULL)
	{
		CMDIFrameWndEx* pMainMDIFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, m_pParentFrame);
		const CMFCMenuBar* pMenuBar = NULL;

		if (pMainMDIFrame != NULL)
		{
			pMenuBar = pMainMDIFrame->GetMenuBar();
		}
		else
		{
			CFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CFrameWndEx, m_pParentFrame);
			if (pMainFrame != NULL)
			{
				pMenuBar = pMainFrame->GetMenuBar();
			}
		}

		if (pMenuBar != NULL)
		{
			pFrameMenu = CMenu::FromHandle(pMenuBar->GetDefaultMenu());
		}
	}

	if (pFrameMenu != NULL)
	{
		AddMenuCommands(pFrameMenu, FALSE);
	}
}

void CMFCToolBarsCustomizeDialog::AddUserTools(LPCTSTR lpszCategory)
{
	ENSURE(lpszCategory != NULL);
	ASSERT_VALID(afxUserToolsManager);

	const CObList& lstTools = afxUserToolsManager->GetUserTools();
	for (POSITION pos = lstTools.GetHeadPosition(); pos != NULL;)
	{
		CUserTool* pTool = (CUserTool*) lstTools.GetNext(pos);
		ASSERT_VALID(pTool);

		AddButton(lpszCategory, CMFCToolBarButton(pTool->GetCommandId(), 0, pTool->m_strLabel));
	}
}

int CMFCToolBarsCustomizeDialog::GetCountInCategory(LPCTSTR lpszItemName, const CObList& lstCommands) const
{
	int nCount = 0;

	for (POSITION pos = lstCommands.GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarButton* pButton = (CMFCToolBarButton*) lstCommands.GetNext(pos);
		ENSURE(pButton != NULL);
		ASSERT_VALID(pButton);

		if (pButton->m_strText == lpszItemName)
		{
			nCount++;
		}
	}

	return nCount;
}

BOOL CMFCToolBarsCustomizeDialog::OnEditToolbarMenuImage(CWnd* pWndParent, CBitmap& bitmap, int nBitsPerPixel)
{
	CMFCImageEditorDialog dlg(&bitmap, pWndParent, nBitsPerPixel);
	return(dlg.DoModal() == IDOK);
}



