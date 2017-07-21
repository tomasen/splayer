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

#include "afxcontrolbarutil.h"
#include "afxbaseribbonelement.h"
#include "afxtoolbarimages.h"
#include "afxribbonbar.h"

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonTab

class CMFCRibbonCategory;

class CMFCRibbonTab : public CMFCRibbonBaseElement
{
	friend class CMFCRibbonCategory;
	friend class CMFCRibbonBar;

	CMFCRibbonTab();
	virtual void OnDraw(CDC* pDC);
	virtual CSize GetRegularSize(CDC* pDC);
	virtual void OnLButtonDown(CPoint point);
	virtual void OnLButtonDblClk(CPoint point);
	virtual CString GetToolTipText() const;
	virtual void CopyFrom(const CMFCRibbonBaseElement& src);
	virtual CRect GetKeyTipRect(CDC* pDC, BOOL bIsMenu);
	virtual BOOL OnKey(BOOL bIsMenuKey);
	virtual BOOL IsShowTooltipOnBottom() const { return FALSE; }

	AFX_RibbonCategoryColor m_Color;
	BOOL m_bIsTruncated;
	int  m_nFullWidth;

public:
	BOOL IsSelected() const;

	virtual BOOL SetACCData(CWnd* pParent, CAccessibilityData& data);
};

/////////////////////////////////////////////////////////////////////////////////
// CRibbonCategoryScroll

class CRibbonCategoryScroll : public CMFCRibbonButton
{
	friend class CMFCRibbonCategory;

	CRibbonCategoryScroll();

	virtual void OnMouseMove(CPoint point);
	virtual void OnDraw(CDC* pDC);
	virtual BOOL OnAutoRepeat();

	virtual BOOL IsAutoRepeatMode(int& /*nDelay*/) const { return TRUE; }
	virtual void CopyFrom (const CMFCRibbonBaseElement& src);
	virtual void OnClick (CPoint /*point*/) { OnAutoRepeat (); }

	BOOL	m_bIsLeft;

public:
	BOOL IsLeftScroll() const { return m_bIsLeft; }
};

/////////////////////////////////////////////////////////////////////////////
// CMFCRibbonCategory

class CMFCRibbonPanel;
class CMFCRibbonBar;

class CMFCRibbonCategory : public CObject
{
	friend class CMFCRibbonBar;
	friend class CMFCRibbonTab;
	friend class CMFCRibbonPanel;
	friend class CMFCRibbonPanelMenuBar;
	friend class CMFCRibbonBaseElement;

	DECLARE_DYNCREATE(CMFCRibbonCategory)

// Construction
protected:
	CMFCRibbonCategory();

	CMFCRibbonCategory(CMFCRibbonBar* pParentRibbonBar, LPCTSTR lpszName, UINT uiSmallImagesResID,
		UINT uiLargeImagesResID, CSize sizeSmallImage = CSize(16, 16), CSize sizeLargeImage = CSize(32, 32));

	void CommonInit(CMFCRibbonBar* pParentRibbonBar = NULL, LPCTSTR lpszName = NULL,
		UINT uiSmallImagesResID = 0, UINT uiLargeImagesResID = 0, CSize sizeSmallImage = CSize(0, 0), CSize sizeLargeImage = CSize(0, 0));

// Attributes
public:
	CMFCRibbonBar* GetParentRibbonBar() const { return m_pParentRibbonBar; }
	CMFCRibbonPanelMenuBar* GetParentMenuBar() const { return m_pParentMenuBar; }

	CMFCRibbonBaseElement* HitTest(CPoint point, BOOL bCheckPanelCaption = FALSE) const;
	CMFCRibbonBaseElement* HitTestScrollButtons(CPoint point) const;
	int HitTestEx(CPoint point) const;
	CMFCRibbonPanel* GetPanelFromPoint(CPoint point) const;

	CRect GetTabRect() const { return m_Tab.m_rect; }
	CRect GetRect() const { return m_rect; }

	BOOL IsActive() const { return m_bIsActive; }

	CSize GetImageSize(BOOL bIsLargeImage) const;

	int GetImageCount(BOOL bIsLargeImage) const
	{
		return bIsLargeImage ? m_LargeImages.GetCount() : m_SmallImages.GetCount();
	}

	LPCTSTR GetName() const { return m_strName; }
	void SetName(LPCTSTR lpszName);

	DWORD_PTR GetData() const { return m_dwData; }
	void SetData(DWORD_PTR dwData) { m_dwData = dwData; }

	UINT GetContextID() const { return m_uiContextID; }

	void SetTabColor(AFX_RibbonCategoryColor color) { m_Tab.m_Color = color; }
	AFX_RibbonCategoryColor GetTabColor() const { return m_Tab.m_Color; }

	BOOL IsVisible() const { return m_bIsVisible; }

	CMFCRibbonBaseElement* GetDroppedDown();
	CMFCRibbonBaseElement* GetParentButton() const;

	CMFCToolBarImages& GetSmallImages() { return m_SmallImages; }
	CMFCToolBarImages& GetLargeImages() { return m_LargeImages; }

	void SetKeys(LPCTSTR lpszKeys);

// Operations
public:
	CMFCRibbonPanel* AddPanel(LPCTSTR lpszPanelName, HICON hIcon = 0, CRuntimeClass* pRTI = NULL);

	void SetCollapseOrder(const CArray<int, int>& arCollapseOrder);
	
	int GetPanelCount() const;
	CMFCRibbonPanel* GetPanel(int nIndex);

	int GetPanelIndex(const CMFCRibbonPanel* pPanel) const;

	int GetMaxHeight(CDC* pDC);

	CMFCRibbonBaseElement* FindByID(UINT uiCmdID, BOOL bVisibleOnly = TRUE) const;
	CMFCRibbonBaseElement* FindByData(DWORD_PTR dwData, BOOL bVisibleOnly = TRUE) const;

	CMFCRibbonPanel* HighlightPanel(CMFCRibbonPanel* pHLPanel, CPoint point);
	CMFCRibbonPanel* FindPanelWithElem(const CMFCRibbonBaseElement* pElement);

	void AddHidden(CMFCRibbonBaseElement* pElem);

	void GetElements(CArray <CMFCRibbonBaseElement*, CMFCRibbonBaseElement*>& arElements);
	void GetElementsByID(UINT uiCmdID, CArray <CMFCRibbonBaseElement*, CMFCRibbonBaseElement*>& arElements);

	void GetItemIDsList(CList<UINT,UINT>& lstItems, BOOL bHiddenOnly = FALSE) const;

	int GetTextTopLine() const;

// Overrides
	virtual void RecalcLayout(CDC* pDC);
	virtual void OnDraw(CDC* pDC);
	virtual void OnCancelMode();
	virtual CMFCRibbonBaseElement* OnLButtonDown(CPoint point);
	virtual void OnLButtonUp(CPoint point);
	virtual void OnMouseMove(CPoint point);
	virtual void OnUpdateCmdUI(CMFCRibbonCmdUI* pCmdUI, CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

	virtual BOOL NotifyControlCommand(BOOL bAccelerator, int nNotifyCode, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnDrawImage(CDC* pDC, CRect rect, CMFCRibbonBaseElement* pElement, BOOL bIsLargeImage, BOOL nImageIndex, BOOL bCenter);

	virtual void CopyFrom(CMFCRibbonCategory& src);
	virtual void OnDrawMenuBorder(CDC* /*pDC*/, CMFCRibbonPanelMenuBar* /*pMenuBar*/)	{}
	virtual void OnRTLChanged(BOOL bIsRTL);

	virtual BOOL OnScrollHorz(BOOL bScrollLeft, int nScrollOffset = 0);
	virtual void ReposPanels(CDC* pDC);

// Implementation
public:
	virtual ~CMFCRibbonCategory();

protected:
	void SetActive(BOOL bIsActive = TRUE);
	void ShowElements(BOOL bShow = TRUE);

	void ShowFloating(CRect rectFloating);

	void RecalcPanelWidths(CDC* pDC);
	void CleanUpSizes();
	int GetMinWidth(CDC* pDC);

	BOOL SetPanelsLayout(int nWidth);
	void ResetPanelsLayout();

	void UpdateScrollButtons();
	void EnsureVisible(CMFCRibbonButton* pButton);

	BOOL m_bMouseIsPressed;
	BOOL m_bIsActive;
	BOOL m_bIsVisible;
	UINT m_uiContextID;
	int  m_nLastCategoryWidth;
	int  m_nLastCategoryOffsetY;

	DWORD_PTR				m_dwData;
	CRect					m_rect;
	CString					m_strName;
	CMFCRibbonTab				m_Tab;
	CRibbonCategoryScroll	m_ScrollLeft;
	CRibbonCategoryScroll	m_ScrollRight;
	int						m_nScrollOffset;
	CMFCRibbonBar*				m_pParentRibbonBar;

	CMFCRibbonPanelMenuBar*	m_pParentMenuBar;

	int						m_nMinWidth;
	CArray<int, int>		m_arCollapseOrder;
	CArray<CMFCRibbonPanel*,CMFCRibbonPanel*> m_arPanels;

	clock_t					m_ActiveTime;

	//----------------------
	// Category image lists:
	//----------------------
	CMFCToolBarImages m_SmallImages;
	CMFCToolBarImages m_LargeImages;

	//---------------------------------
	// Category elements (non-visible):
	//---------------------------------
	CArray<CMFCRibbonBaseElement*, CMFCRibbonBaseElement*> m_arElements;

private:
	void NormalizeFloatingRect (CMFCRibbonBar* pRibbonBar, CRect& rectCategory);
};

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif
