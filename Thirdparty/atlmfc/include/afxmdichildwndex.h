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
#include "afxframeimpl.h"

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif

class CMDIFrameWndEx;

/////////////////////////////////////////////////////////////////////////////
// CMDIChildWndEx frame

class CMDIChildWndEx : public CMDIChildWnd
{
	friend class CMDIClientAreaWnd;
	friend class CMDIFrameWndEx;

	DECLARE_DYNCREATE(CMDIChildWndEx)
protected:
	CMDIChildWndEx(); // protected constructor used by dynamic creation

// Attributes
public:
	AFX_IMPORT_DATA static BOOL  m_bEnableFloatingBars;
	AFX_IMPORT_DATA static DWORD m_dwExcludeStyle;

	BOOL IsTabbedPane() const { return m_pTabbedControlBar != NULL; }
	CDockablePane* GetTabbedPane() const { return m_pTabbedControlBar; }

protected:
	// ---- MDITabGroup+
	CMFCTabCtrl* m_pRelatedTabGroup;
	// ---- MDITabGroup-

	BOOL  m_bToBeDestroyed;
	BOOL  m_bWasMaximized;
	BOOL  m_bIsMinimized;
	CRect m_rectOriginal;
	// set during OnMDIActivate to prevent unnecessary 
	// RecalcLayout in CMDIFrameWnd in OLE InPlace mode
	BOOL  m_bActivating;

	CFrameImpl       m_Impl;
	CDockingManager        m_dockManager;
	CDockablePane* m_pTabbedControlBar;
	CMDIFrameWndEx* m_pMDIFrame;

// Operations
public:
	// ---- MDITabGroup+
	CMFCTabCtrl* GetRelatedTabGroup() { return m_pRelatedTabGroup; }
	void SetRelatedTabGroup(CMFCTabCtrl* p) { m_pRelatedTabGroup = p; }
	// ---- MDITabGroup-

	void AddDockSite();
	BOOL AddPane(CBasePane* pControlBar, BOOL bTail = TRUE);
	BOOL InsertPane(CBasePane* pControlBar, CBasePane* pTarget, BOOL bAfter = TRUE);
	void RemovePaneFromDockManager(CBasePane* pControlBar, BOOL bDestroy, BOOL bAdjustLayout, BOOL bAutoHide, CBasePane* pBarReplacement);
	void DockPane(CBasePane* pBar, UINT nDockBarID = 0, LPCRECT lpRect = NULL);

	void AddTabbedPane(CDockablePane* pControlBar);

	CBasePane* PaneFromPoint(CPoint point, int nSensitivity, bool bExactBar, CRuntimeClass* pRTCBarType) const;
	CBasePane* PaneFromPoint(CPoint point, int nSensitivity, DWORD& dwAlignment, CRuntimeClass* pRTCBarType) const;
	BOOL IsPointNearDockSite(CPoint point, DWORD& dwBarAlignment, BOOL& bOuterEdge) const;
	virtual void AdjustDockingLayout(HDWP hdwp = NULL);
	BOOL EnableDocking(DWORD dwDockStyle);
	BOOL EnableAutoHidePanes(DWORD dwDockStyle);

	CBasePane* GetPane(UINT nID);
	void ShowPane(CBasePane* pBar, BOOL bShow, BOOL bDelay, BOOL bActivate);

	virtual BOOL OnMoveMiniFrame(CWnd* pFrame);
	virtual void RecalcLayout(BOOL bNotify = TRUE);

	virtual BOOL GetToolbarButtonToolTipText(CMFCToolBarButton* /*pButton*/, CString& /*strTTText*/) { return FALSE; }

	BOOL DockPaneLeftOf(CPane* pBar, CPane* pLeftOf);

// Overrides

	// Next methods used by MDI tabs:
	virtual CString GetFrameText() const;
	virtual HICON GetFrameIcon() const;

	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);

	virtual BOOL CanShowOnMDITabs() { return TRUE; }
	virtual BOOL CanShowOnWindowsList() { return TRUE; }
	virtual BOOL IsReadOnly() { return FALSE; }

	CDockingManager* GetDockingManager() { return &m_dockManager; }

	virtual LPCTSTR GetDocumentName(CObject** pObj);

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void ActivateFrame(int nCmdShow = -1);
	virtual void OnSetPreviewMode(BOOL bPreview, CPrintPreviewState* pState);

protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
protected:
	virtual ~CMDIChildWndEx();

	//{{AFX_MSG(CMDIChildWndEx)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnDestroy();
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnNcPaint();
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnSetText(WPARAM,LPARAM);
	afx_msg LRESULT OnSetIcon(WPARAM,LPARAM);
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
	afx_msg LRESULT OnChangeVisualManager(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void AdjustClientArea();
};

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif
