// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#pragma once

#include "afxcontrolbarutil.h"
#include "afxtabctrl.h"

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif

class CMDIFrameWndEx;

extern AFX_IMPORT_DATA UINT AFX_WM_ON_MOVETOTABGROUP;

/////////////////////////////////////////////////////////////////////////////
// CMDITabInfo

class CMDITabInfo
{
public:
	CMDITabInfo();
	void Serialize(CArchive& ar);

	CMFCTabCtrl::Location m_tabLocation;
	CMFCTabCtrl::Style    m_style;

	BOOL m_bTabIcons;
	BOOL m_bTabCloseButton;
	BOOL m_bTabCustomTooltips;
	BOOL m_bAutoColor;
	BOOL m_bDocumentMenu;
	BOOL m_bEnableTabSwap;
	BOOL m_bFlatFrame;
	BOOL m_bActiveTabCloseButton;
	int  m_nTabBorderSize;
};

/////////////////////////////////////////////////////////////////////////////
// CMDIClientAreaWnd window

class CMDIClientAreaWnd : public CWnd
{
	DECLARE_DYNAMIC(CMDIClientAreaWnd)
	friend class CMDIFrameWndEx;

// Construction
public:
	CMDIClientAreaWnd();

// Attributes
public:
	CMFCTabCtrl& GetMDITabs()
	{
		return m_wndTab;
	}

protected:
	CMFCTabCtrl m_wndTab;
	BOOL    m_bTabIsVisible;
	BOOL    m_bTabIsEnabled;

	CImageList m_TabIcons;
	CMap<HICON,HICON,int,int> m_mapIcons; // Icons already loaded into the image list

	// ---- MDITabGroup+
	enum GROUP_ALIGNMENT
	{
		GROUP_NO_ALIGN,
		GROUP_VERT_ALIGN,
		GROUP_HORZ_ALIGN
	};

	CMDITabInfo m_mdiTabParams;
	CObList       m_lstTabbedGroups;

	CMap<CWnd*, CWnd*, CImageList*, CImageList*>m_mapTabIcons;

	BOOL m_bIsMDITabbedGroup;
	BOOL m_bNewVericalGroup;
	BOOL m_bDisableUpdateTabs;
	CObList m_lstRemovedTabbedGroups;

	int m_nResizeMargin;
	int m_nNewGroupMargin;
	int m_nTotalResizeRest;
	CRect m_rectNewTabGroup;
	CStringList m_lstLoadedTabDocuments;
	GROUP_ALIGNMENT m_groupAlignment;
	// ---- MDITabGroup-

// Operations
public:
	void EnableMDITabs(BOOL bEnable, const CMDITabInfo& params);
	BOOL DoesMDITabExist() const { return m_bTabIsEnabled; }

	void SetActiveTab(HWND hwnd);
	void UpdateTabs(BOOL bSetActiveTabVisible = FALSE);

	// ---- MDITabGroup+
	void EnableMDITabbedGroups(BOOL bEnable, const CMDITabInfo& mdiTabParams);
	BOOL IsMDITabbedGroup() const { return m_bIsMDITabbedGroup; }

	virtual CMFCTabCtrl* CreateTabGroup(CMFCTabCtrl* pWndTab);
	void UpdateMDITabbedGroups(BOOL bSetActiveTabVisible);
	void CalcWindowRectForMDITabbedGroups(LPRECT lpClientRect, UINT nAdjustType);

	DWORD GetMDITabsContextMenuAllowedItems();

	BOOL IsMemberOfMDITabGroup(CWnd* pWnd);
	CMFCTabCtrl* FindActiveTabWndByActiveChild();
	CMFCTabCtrl* FindActiveTabWnd();
	CMFCTabCtrl* GetFirstTabWnd();
	const CObList& GetMDITabGroups() const { return m_lstTabbedGroups; }

	void MDITabMoveToNextGroup(BOOL bNext = TRUE);
	void MDITabNewGroup(BOOL bVert = TRUE);
	BOOL MoveWindowToTabGroup(CMFCTabCtrl* pTabWndFrom, CMFCTabCtrl* pTabWndTo, int nIdxFrom = -1);

	void RemoveTabGroup(CMFCTabCtrl* pTabWnd, BOOL bRecalcLayout = TRUE);
	void CloseAllWindows(CMFCTabCtrl* pTabWnd);

	BOOL SaveState(LPCTSTR lpszProfileName, UINT nFrameID);
	BOOL LoadState(LPCTSTR lpszProfileName, UINT nFrameID);
	void Serialize(CArchive& ar);
	// ---- MDITabGroup-

// Overrides
protected:
	virtual void CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType = adjustBorder);
	virtual void PreSubclassWindow();

// Implementation
public:
	virtual ~CMDIClientAreaWnd();

protected:
	//{{AFX_MSG(CMDIClientAreaWnd)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnStyleChanging(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
	afx_msg LRESULT OnSetMenu(WPARAM wp, LPARAM);
	afx_msg LRESULT OnUpdateTabs(WPARAM, LPARAM);
	afx_msg LRESULT OnMDIRefreshMenu(WPARAM wp, LPARAM);
	afx_msg LRESULT OnMDIDestroy(WPARAM wp, LPARAM);
	afx_msg LRESULT OnMDINext(WPARAM wp, LPARAM);
	afx_msg LRESULT OnGetDragBounds(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnDragComplete(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnTabGroupMouseMove(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnCancelTabMove(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnMoveTabComplete(WPARAM wp, LPARAM lp);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CMFCTabCtrl* GetNextTabWnd(CMFCTabCtrl* pOrgTabWnd, BOOL bWithoutAsserts = FALSE);
	void AdjustMDIChildren(CMFCTabCtrl* pTabWnd);
	void DrawNewGroupRect(LPCRECT rectNew, LPCRECT rectOld);
	CMFCTabCtrl* TabWndFromPoint(CPoint ptScreen);
	CMFCTabCtrl* CreateNewTabGroup(CMFCTabCtrl* pTabWndAfter, CRect rectGroup, BOOL bVertical);
	void ApplyParams(CMFCTabCtrl* pTabWnd);
	void SerializeTabGroup(CArchive& ar, CMFCTabCtrl* pTabWnd, BOOL bSetRelation = FALSE);
	void SerializeOpenChildren(CArchive& ar);

	BOOL IsKeepClientEdge();
};

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif
