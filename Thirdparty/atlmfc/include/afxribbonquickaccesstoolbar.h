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
#include "afxribbonbuttonsgroup.h"
#include "afxribbonbutton.h"

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif

////////////////////////////////////////////////////////
// CMFCRibbonQuickAccessToolBar class

class CMFCRibbonQuickAccessCustomizeButton;

class CMFCRibbonQuickAccessToolBarDefaultState
{
	friend class CMFCRibbonQuickAccessToolBar;
	friend class CMFCRibbonBar;

public:
	CMFCRibbonQuickAccessToolBarDefaultState();

	void AddCommand(UINT uiCmd, BOOL bIsVisible = TRUE);
	void RemoveAll();

	void CopyFrom(const CMFCRibbonQuickAccessToolBarDefaultState& src);

protected:
	CArray<UINT,UINT> m_arCommands;
	CArray<BOOL,BOOL> m_arVisibleState;
};

class CMFCRibbonQuickAccessToolBar : public CMFCRibbonButtonsGroup
{
	DECLARE_DYNCREATE(CMFCRibbonQuickAccessToolBar)

	friend class CMFCRibbonBar;
	friend class CMFCRibbonBaseElement;
	friend class CMFCRibbonCustomizePropertyPage;

public:
	CMFCRibbonQuickAccessToolBar();
	virtual ~CMFCRibbonQuickAccessToolBar();

protected:
	void SetCommands(CMFCRibbonBar* pRibbonBar, const CList<UINT,UINT>& lstCommands, LPCTSTR lpszToolTip);
	void SetCommands(CMFCRibbonBar* pRibbonBar, const CList<UINT,UINT>& lstCommands, CMFCRibbonQuickAccessCustomizeButton* pCustButton);

	void GetCommands(CList<UINT,UINT>& lstCommands);
	void GetDefaultCommands(CList<UINT,UINT>& lstCommands);
	void ReplaceCommands(const CList<UINT,UINT>& lstCommands);
	void ResetCommands();

	int GetActualWidth() const;

	virtual CSize GetRegularSize(CDC* pDC);
	virtual void OnAfterChangeRect(CDC* pDC);

	virtual BOOL IsQuickAccessToolBar() const { return TRUE; }

	void Add(CMFCRibbonBaseElement* pElem);
	void Remove(CMFCRibbonBaseElement* pElem);

	void RebuildHiddenItems();
	CRect GetCommandsRect() const { return m_rectCommands; }
	void RebuildKeys();

protected:
	CMFCRibbonQuickAccessToolBarDefaultState m_DefaultState;
	CRect                  m_rectCommands;
};

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif
