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

#pragma once

#define AFX_IDW_RIBBON_BAR     0xE806  // CMFCRibbonBar
#define AFX_RIBBON_CAPTION_BUTTONS 3

#include "afxcontrolbarutil.h"

#include "afxpane.h"
#include "afxribbonbutton.h"
#include "afxribbonquickaccesstoolbar.h"

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif

class CMFCRibbonCategory;
class CMFCRibbonBar;
class CMFCRibbonApplicationButton;
class CMFCToolBarMenuButton;
class CMFCRibbonMainPanel;
class CMFCRibbonRichEditCtrl;
class CMFCRibbonKeyTip;
class CMFCRibbonPanel;

/////////////////////////////////////////////////////////////////////////////
// AFX_RibbonCategoryColor

enum AFX_RibbonCategoryColor
{
	AFX_CategoryColor_None,
	AFX_CategoryColor_Red,
	AFX_CategoryColor_Orange,
	AFX_CategoryColor_Yellow,
	AFX_CategoryColor_Green,
	AFX_CategoryColor_Blue,
	AFX_CategoryColor_Indigo,
	AFX_CategoryColor_Violet
};

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonCaptionButton

class CMFCRibbonCaptionButton : public CMFCRibbonButton
{
	DECLARE_DYNCREATE(CMFCRibbonCaptionButton)

	friend class CMFCRibbonBar;

public:
	BOOL IsMDIChildButton() const { return m_hwndMDIChild != NULL; }

protected:
	CMFCRibbonCaptionButton(UINT uiCmd = 0, HWND hwndMDIChild = NULL);

	virtual void OnDraw(CDC* pDC);
	virtual void OnLButtonUp(CPoint point);

	virtual CSize GetRegularSize(CDC* pDC);
	virtual CSize GetCompactSize(CDC* pDC) { return GetRegularSize(pDC); }
	virtual BOOL IsShowTooltipOnBottom() const { return FALSE; }
	virtual int AddToListBox(CMFCRibbonCommandsListBox* /*pWndListBox*/, BOOL /*bDeep*/) { return -1; }

	HWND m_hwndMDIChild;
};

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonContextCaption

class CMFCRibbonContextCaption : public CMFCRibbonButton
{
	DECLARE_DYNCREATE(CMFCRibbonContextCaption)

	friend class CMFCRibbonBar;

public:
	AFX_RibbonCategoryColor	GetColor() const { return m_Color; }
	int GetRightTabX() const { return m_nRightTabX; }

protected:
	CMFCRibbonContextCaption(LPCTSTR lpszName, UINT uiID, AFX_RibbonCategoryColor clrContext);
	CMFCRibbonContextCaption();

	virtual void OnDraw(CDC* pDC);
	virtual void OnLButtonUp(CPoint point);

	AFX_RibbonCategoryColor m_Color;
	UINT m_uiID;
	int  m_nRightTabX;
};

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonBar window

#define AFX_RIBBONBAR_HIDE_ELEMENTS  0x0001
#define AFX_RIBBONBAR_HIDE_ALL       0x0002

class CMFCRibbonBar : public CPane
{
	friend class CMFCRibbonCategory;
	friend class CMFCRibbonCustomizePropertyPage;
	friend class CPreviewViewEx;
	friend class CMFCRibbonApplicationButton;
	friend class CMFCRibbonPanelMenuBar;
	friend class CFrameImpl;

	DECLARE_DYNAMIC(CMFCRibbonBar)

// Construction
public:
	CMFCRibbonBar(BOOL bReplaceFrameCaption = TRUE);

	BOOL Create(CWnd* pParentWnd, DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP, UINT nID = AFX_IDW_RIBBON_BAR);
	BOOL CreateEx(CWnd* pParentWnd, DWORD dwCtrlStyle = 0, DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP, UINT nID = AFX_IDW_RIBBON_BAR);

// Operations
public:
	virtual void RecalcLayout();

	//----------------------------------------------------------------------
	// Enable/disable ribbon main button (large rounded button on top left):
	//----------------------------------------------------------------------
	void SetApplicationButton(CMFCRibbonApplicationButton* pButton, CSize sizeButton);

	//--------------------------
	// Ribbon categories (tabs):
	//--------------------------
	CMFCRibbonMainPanel* AddMainCategory(LPCTSTR lpszName, UINT uiSmallImagesResID, UINT uiLargeImagesResID,
		CSize sizeSmallImage = CSize(16, 16), CSize sizeLargeImage = CSize(32, 32));
	CMFCRibbonCategory* AddCategory(LPCTSTR lpszName, UINT uiSmallImagesResID, UINT uiLargeImagesResID,
		CSize sizeSmallImage = CSize(16, 16), CSize sizeLargeImage = CSize(32, 32), int nInsertAt = -1, CRuntimeClass* pRTI = NULL);
	CMFCRibbonCategory* AddContextCategory(LPCTSTR lpszName, LPCTSTR lpszContextName, UINT uiContextID, AFX_RibbonCategoryColor clrContext,
		UINT uiSmallImagesResID, UINT uiLargeImagesResID, CSize sizeSmallImage = CSize(16, 16), CSize sizeLargeImage = CSize(32, 32), CRuntimeClass* pRTI = NULL);
	CMFCRibbonCategory* AddQATOnlyCategory(LPCTSTR lpszName, UINT uiSmallImagesResID, CSize sizeSmallImage = CSize(16, 16));
	CMFCRibbonCategory* AddPrintPreviewCategory();

	void EnablePrintPreview(BOOL bEnable = TRUE);
	BOOL IsPrintPreviewEnabled() const { return m_bIsPrintPreview; }

	int GetCategoryCount() const;
	int GetVisibleCategoryCount() const;
	CMFCRibbonCategory* GetCategory(int nIndex) const;
	int GetCategoryIndex(CMFCRibbonCategory* pCategory) const;

	void ShowCategory(int nIndex, BOOL bShow = TRUE);
	void ShowContextCategories(UINT uiContextID, BOOL bShow = TRUE);
	BOOL HideAllContextCategories();
	BOOL ActivateContextCategory(UINT uiContextID);

	BOOL RemoveCategory(int nIndex);
	void RemoveAllCategories();

	virtual BOOL SetActiveCategory(CMFCRibbonCategory* pCategory, BOOL bForceRestore = FALSE);
	CMFCRibbonCategory* GetActiveCategory() const { return m_pActiveCategory; }

	int FindCategoryIndexByData(DWORD dwData) const;
	BOOL GetContextName(UINT uiContextID, CString& strName) const;

	//-------------------------------
	// Ribbon elements direct access:
	//-------------------------------
	CMFCRibbonBaseElement* FindByID(UINT uiCmdID, BOOL bVisibleOnly = TRUE, BOOL bExcludeQAT = FALSE) const;
	CMFCRibbonBaseElement* FindByData(DWORD_PTR dwData, BOOL bVisibleOnly = TRUE) const;

	BOOL SetElementKeys(UINT uiCmdID, LPCTSTR lpszKeys, LPCTSTR lpszMenuKeys = NULL);

	void GetElementsByID(UINT uiCmdID, CArray<CMFCRibbonBaseElement*, CMFCRibbonBaseElement*>& arButtons);

	void SetQuickAccessDefaultState(const CMFCRibbonQuickAccessToolBarDefaultState& state);
	void SetQuickAccessCommands(const CList<UINT,UINT>& lstCommands, BOOL bRecalcLayout = TRUE);
	void GetQuickAccessCommands(CList<UINT,UINT>& lstCommands);

	//--------------------------------------------------
	// Additional elements located on the right of tabs:
	//--------------------------------------------------
	void AddToTabs(CMFCRibbonBaseElement* pElement);
	void RemoveAllFromTabs();

	//------------------
	// Tooltips support:
	//------------------
	void EnableToolTips(BOOL bEnable = TRUE, BOOL bEnableDescr = TRUE);
	
	BOOL IsToolTipEnabled() const { return m_bToolTip; }
	BOOL IsToolTipDescrEnabled() const { return m_bToolTipDescr; }

	int GetTooltipFixedWidthRegular() const { return m_nTooltipWidthRegular; }
	int GetTooltipFixedWidthLargeImage() const { return m_nTooltipWidthLargeImage; }

	void SetTooltipFixedWidth(int nWidthRegular, int nWidthLargeImage);	// 0 - set variable size

	//-----------------
	// Key tip support:
	//-----------------
	void EnableKeyTips(BOOL bEnable = TRUE);

	BOOL IsKeyTipEnabled() const { return m_bKeyTips; }

	void GetItemIDsList(CList<UINT,UINT>& lstItems, BOOL bHiddenOnly = FALSE) const;

	void ToggleMimimizeState();

	BOOL OnSysKeyDown(CFrameWnd* pFrameWnd, WPARAM wParam, LPARAM lParam);
	BOOL OnSysKeyUp(CFrameWnd* pFrameWnd, WPARAM wParam, LPARAM lParam);

// Attributes
public:
	int GetCaptionHeight() const { return m_nCaptionHeight; }
	int GetCategoryHeight() const { return m_nCategoryHeight; }
	BOOL IsReplaceFrameCaption() const { return m_bReplaceFrameCaption; }
	CMFCRibbonApplicationButton* GetApplicationButton() const { return m_pMainButton; }
	CMFCRibbonCategory* GetMainCategory() const { return m_pMainCategory; }

	virtual CMFCRibbonBaseElement* HitTest(CPoint point, BOOL bCheckActiveCategory = FALSE, BOOL bCheckPanelCaption = FALSE);

	//---------------------------------
	// Quick access toolbar attributes:
	//---------------------------------
	void SetQuickAccessToolbarOnTop(BOOL bOnTop);

	BOOL IsQuickAccessToolbarOnTop() const { return m_bQuickAccessToolbarOnTop && m_bReplaceFrameCaption; }
	CRect GetQuickAccessToolbarLocation() const { return m_QAToolbar.GetRect(); }
	CRect GetQATCommandsLocation() const { return m_QAToolbar.GetCommandsRect(); }

	BOOL IsQATEmpty() const { return (int) m_QAToolbar.m_arButtons.GetSize() <= 1; }
	CMFCRibbonBaseElement* GetQATDroppedDown() { return m_QAToolbar.GetDroppedDown(); }

	DWORD GetHideFlags() const { return m_dwHideFlags; }
	int GetTabTrancateRatio() const { return m_nTabTrancateRatio; }

	void SetMaximizeMode(BOOL bMax, CWnd* pWnd = NULL);
	void SetActiveMDIChild(CWnd* pWnd);

	virtual CMFCRibbonBaseElement* GetDroppedDown();

	BOOL IsTransparentCaption() const { return m_bIsTransparentCaption; }
	int GetKeyboardNavigationLevel() const { return m_nKeyboardNavLevel; }

	void SetKeyboardNavigationLevel(CObject* pLevel, BOOL bSetFocus = TRUE);

	CObject* GetKeyboardNavLevelParent() const { return m_pKeyboardNavLevelParent; }
	CObject* GetKeyboardNavLevelCurrent() const { return m_pKeyboardNavLevelCurrent; }

	virtual BOOL OnSetAccData(long lVal);

protected:
	int m_nTabsHeight;
	int m_nCategoryHeight;
	int m_nCategoryMinWidth;
	int m_nHighlightedTab;
	int m_nCaptionHeight;
	int m_nTabTrancateRatio;
	int m_nSystemButtonsNum;
	int m_nKeyboardNavLevel;
	int m_nCurrKeyChar;

	BOOL m_bRecalcCategoryHeight;
	BOOL m_bRecalcCategoryWidth;
	BOOL m_bTracked;
	BOOL m_bIsPrintPreview;
	BOOL m_bQuickAccessToolbarOnTop;
	BOOL m_bForceRedraw;
	BOOL m_bMaximizeMode;
	BOOL m_bAutoCommandTimer;
	BOOL m_bPrintPreviewMode;
	BOOL m_bIsTransparentCaption;
	BOOL m_bIsMaximized;
	BOOL m_bToolTip;
	int m_nTooltipWidthRegular;
	int m_nTooltipWidthLargeImage;
	BOOL m_bToolTipDescr;
	BOOL m_bKeyTips;
	BOOL m_bIsCustomizeMenu;

	const BOOL m_bReplaceFrameCaption;

	HFONT m_hFont;
	DWORD m_dwHideFlags;

	CMFCRibbonApplicationButton*  m_pMainButton;
	CMFCRibbonBaseElement* m_pHighlighted;
	CMFCRibbonBaseElement* m_pPressed;
	CMFCRibbonButtonsGroup m_TabElements;
	CMFCRibbonCategory*    m_pActiveCategory;
	CMFCRibbonCategory*    m_pActiveCategorySaved;
	CMFCRibbonCategory*    m_pMainCategory;
	CMFCRibbonCategory*    m_pPrintPreviewCategory;

	CArray<CMFCRibbonContextCaption*, CMFCRibbonContextCaption*> m_arContextCaptions;
	CArray<CMFCRibbonCategory*,CMFCRibbonCategory*> m_arCategories;
	CArray<CMFCRibbonKeyTip*,CMFCRibbonKeyTip*> m_arKeyElements;
	CArray<int,int> m_arVisibleCategoriesSaved;

	CRect m_rectCaption;
	CRect m_rectCaptionText;
	CRect m_rectSysButtons;
	CSize m_sizeMainButton;

	CToolTipCtrl* m_pToolTip;
	CObject*      m_pKeyboardNavLevelParent;
	CObject*      m_pKeyboardNavLevelCurrent;

	CMFCRibbonCaptionButton      m_CaptionButtons[AFX_RIBBON_CAPTION_BUTTONS];
	CMFCRibbonQuickAccessToolBar m_QAToolbar;

// Overrides
public:
	virtual void OnClickButton(CMFCRibbonButton* pButton, CPoint point);
	virtual BOOL IsMainRibbonBar() const { return TRUE; }
	virtual BOOL IsShowGroupBorder(CMFCRibbonButtonsGroup* /*pGroup*/) const { return FALSE; }
	virtual void OnEditContextMenu(CMFCRibbonRichEditCtrl* pEdit, CPoint point);

	virtual void DWMCompositionChanged();

	virtual BOOL OnShowRibbonQATMenu(CWnd* pWnd, int x, int y, CMFCRibbonBaseElement* pHit);

	virtual BOOL TranslateChar(UINT nChar);

	virtual void OnRTLChanged(BOOL bIsRTL);

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual void OnPaneContextMenu(CWnd* pParentFrame, CPoint point);

protected:
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual BOOL AllowShowOnPaneMenu() const { return FALSE; }

	virtual void OnFillBackground(CDC* pDC, CRect rectClient);

	virtual BOOL LoadState(LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);
	virtual BOOL SaveState(LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);

	virtual void OnSetPrintPreviewKeys(CMFCRibbonPanel* pPrintPanel, CMFCRibbonPanel* pZoomPanel, CMFCRibbonPanel* pPreviewPanel);
	virtual BOOL HideInPrintPreviewMode() const { return FALSE; }
	virtual void OnBeforeProcessKey(int& nChar);

// Implementation
public:
	virtual ~CMFCRibbonBar();

	void PopTooltip();
	BOOL DrawMenuImage(CDC* pDC, const CMFCToolBarMenuButton* pMenuItem, const CRect& rectImage);

	virtual BOOL OnShowRibbonContextMenu(CWnd* pWnd, int x, int y, CMFCRibbonBaseElement* pHit);

	void ForceRecalcLayout();
	void DeactivateKeyboardFocus(BOOL bSetFocus = TRUE);

	void ShowKeyTips();
	void HideKeyTips();

protected:
	void ShowSysMenu(const CPoint& point);
	void SetPrintPreviewMode(BOOL bSet = TRUE);

	CMFCRibbonContextCaption* FindContextCaption(UINT uiID) const;

	void UpdateToolTipsRect();
	BOOL ProcessKey(int nChar);

	void RemoveAllKeys();

protected:
	//{{AFX_MSG(CMFCRibbonBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSysColorChange();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg LRESULT OnSetFont(WPARAM, LPARAM);
	afx_msg LRESULT OnGetFont(WPARAM, LPARAM);
	afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);
	afx_msg LRESULT OnUpdateToolTips(WPARAM, LPARAM);
	afx_msg BOOL OnNeedTipText(UINT id, NMHDR* pNMH, LRESULT* pResult);
	afx_msg LRESULT OnPostRecalcLayout(WPARAM,LPARAM);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonBar idle update through CMFCRibbonCmdUI class

class CMFCRibbonCmdUI : public CCmdUI
{
public:
	CMFCRibbonCmdUI();

	virtual void Enable(BOOL bOn);
	virtual void SetCheck(int nCheck);
	virtual void SetText(LPCTSTR lpszText);
	virtual void SetRadio(BOOL bOn = TRUE);

	CMFCRibbonBaseElement* m_pUpdated;
};

//////////////////////////////////////////////////////////////////////
// CMFCRibbonApplicationButton

class CMFCRibbonApplicationButton : public CMFCRibbonButton
{
	DECLARE_DYNCREATE(CMFCRibbonApplicationButton);

public:
	CMFCRibbonApplicationButton() { }

	CMFCRibbonApplicationButton(UINT uiBmpResID) { SetImage(uiBmpResID); }
	CMFCRibbonApplicationButton(HBITMAP hBmp) { SetImage(hBmp); }

	void SetImage(UINT uiBmpResID);
	void SetImage(HBITMAP hBmp);

protected:
	virtual BOOL IsShowTooltipOnBottom() const { return FALSE; }
	virtual BOOL IsApplicationButton() const { return TRUE; }
	virtual BOOL CanBeAddedToQuickAccessToolBar() const { return FALSE; }
	virtual int AddToListBox(CMFCRibbonCommandsListBox* /*pWndListBox*/, BOOL /*bDeep*/) { return -1; }

	virtual CSize GetImageSize(RibbonImageType /*type*/) const 
	{
		ASSERT_VALID(this);
		return m_Image.GetImageSize();
	}

	virtual void DrawImage(CDC* pDC, RibbonImageType /*type*/, CRect rectImage)
	{
		ASSERT_VALID(this);
		ASSERT_VALID(pDC);
		m_Image.SetTransparentColor(afxGlobalData.clrBtnFace);
		m_Image.DrawEx(pDC, rectImage, 0);
	}

	virtual BOOL SetACCData(CWnd* pParent, CAccessibilityData& data)
	{
		CMFCRibbonButton::SetACCData(pParent, data);
		data.m_nAccRole = ROLE_SYSTEM_BUTTONDROPDOWNGRID;
		data.m_bAccState |= STATE_SYSTEM_HASPOPUP;
		return TRUE;
	}

	virtual void OnLButtonDown(CPoint point);
	virtual void OnLButtonDblClk(CPoint point);
	virtual BOOL OnKey(BOOL bIsMenuKey);

	virtual BOOL IsDrawTooltipImage() const { return FALSE; }

	BOOL ShowMainMenu();

	CMFCToolBarImages m_Image;
};

extern AFX_IMPORT_DATA UINT AFX_WM_ON_CHANGE_RIBBON_CATEGORY;
extern AFX_IMPORT_DATA UINT AFX_WM_ON_RIBBON_CUSTOMIZE;
extern AFX_IMPORT_DATA UINT AFX_WM_ON_HIGHLIGHT_RIBBON_LIST_ITEM;
extern AFX_IMPORT_DATA UINT AFX_WM_ON_BEFORE_SHOW_RIBBON_ITEM_MENU;

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif
