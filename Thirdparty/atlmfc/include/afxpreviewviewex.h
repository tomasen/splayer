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
#include <afxpriv.h> //for CPreviewView
#include "afxtoolbar.h"

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif

class CMFCStatusBar;
class CMFCRibbonBar;
class CMFCRibbonButton;

void AFXPrintPreview(CView* pView);

/////////////////////////////////////////////////////////////////////////////
// CMFCPrintPreviewToolBar toolbar

class CMFCPrintPreviewToolBar : public CMFCToolBar
{
	friend class CPreviewViewEx;

	DECLARE_DYNAMIC(CMFCPrintPreviewToolBar)

protected:
	//{{AFX_MSG(CMFCPrintPreviewToolBar)
	afx_msg void OnContextMenu(CWnd *pWnd, CPoint pos);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	virtual BOOL AllowShowOnPaneMenu() const { return FALSE; }
};

/////////////////////////////////////////////////////////////////////////////
// CPreviewViewEx window

class CPreviewViewEx : public CPreviewView
{
protected:
	CPreviewViewEx();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPreviewViewEx)

// Attributes
public:
	static void __stdcall EnableScaleLargeImages(BOOL bScaleLargeImages = TRUE) { m_bScaleLargeImages = bScaleLargeImages; }

// Operations
protected:
	void SetToolbarSize();

// Overrides
	void OnDisplayPageNumber(UINT nPage, UINT nPagesDisplayed);

// Implementation
protected:
	virtual ~CPreviewViewEx();

	CMFCPrintPreviewToolBar m_wndToolBar;
	CMFCStatusBar*    m_pWndStatusBar;
	CMFCRibbonBar*          m_pWndRibbonBar;
	CMFCRibbonButton*       m_pNumPageButton;

	int m_iOnePageImageIndex;
	int m_iTwoPageImageIndex;
	int m_iPagesBtnIndex;
	int m_nSimpleType;

	BOOL  m_bIsStatusBarSimple;
	CSize m_recentToolbarSize;

	AFX_IMPORT_DATA static BOOL m_bScaleLargeImages;

protected:
	//{{AFX_MSG(CPreviewViewEx)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdatePreviewNumPage(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif
