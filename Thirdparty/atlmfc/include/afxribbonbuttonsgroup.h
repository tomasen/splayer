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

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif

class CMFCRibbonButtonsGroup : public CMFCRibbonBaseElement
{
	friend class CMFCRibbonBar;

	DECLARE_DYNCREATE(CMFCRibbonButtonsGroup)

// Construction
public:
	CMFCRibbonButtonsGroup();
	CMFCRibbonButtonsGroup(CMFCRibbonBaseElement* pButton);

	virtual ~CMFCRibbonButtonsGroup();

// Attributes
public:
	void SetImages(CMFCToolBarImages* pImages, CMFCToolBarImages* pHotImages, CMFCToolBarImages* pDisabledImages);

	BOOL HasImages() const { return m_Images.GetCount() > 0; }
	const CSize GetImageSize() const;
	int GetCount() const { return (int) m_arButtons.GetSize(); }

	CMFCRibbonBaseElement* GetButton(int i) const
	{
		ASSERT_VALID(m_arButtons [i]);
		return m_arButtons [i];
	}

// Operations
public:
	void AddButton(CMFCRibbonBaseElement* pButton);
	void AddButtons(const CList<CMFCRibbonBaseElement*, CMFCRibbonBaseElement*>& lstButtons);

	void RemoveAll();

// Overrides
public:
	virtual CSize GetRegularSize(CDC* pDC);

	virtual void OnDrawImage(CDC* pDC, CRect rectImage, CMFCRibbonBaseElement* pButton, int nImageIndex);
	virtual void SetParentCategory(CMFCRibbonCategory* pCategory);

protected:
	virtual void OnDraw(CDC* pDC);
	virtual void OnUpdateCmdUI(CMFCRibbonCmdUI* pCmdUI, CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual void OnAfterChangeRect(CDC* pDC);
	virtual void OnShow(BOOL bShow);

	virtual CMFCRibbonBaseElement* HitTest(CPoint point);

	virtual BOOL CanBeStretched() { return FALSE; }
	virtual BOOL IsAlignByColumn() const { return FALSE; }
	virtual BOOL IsQuickAccessToolBar() const { return FALSE; }

	virtual CMFCRibbonBaseElement* Find(const CMFCRibbonBaseElement* pElement);
	virtual CMFCRibbonBaseElement* FindByID(UINT uiCmdID);
	virtual CMFCRibbonBaseElement* FindByData(DWORD_PTR dwData);
	virtual CMFCRibbonBaseElement* FindByOriginal(CMFCRibbonBaseElement* pOriginal);
	virtual CMFCRibbonBaseElement* GetPressed();
	virtual CMFCRibbonBaseElement* GetDroppedDown();
	virtual CMFCRibbonBaseElement* GetHighlighted();

	virtual BOOL ReplaceByID(UINT uiCmdID, CMFCRibbonBaseElement* pElem);
	virtual void CopyFrom(const CMFCRibbonBaseElement& src);
	virtual void SetParentMenu(CMFCRibbonPanelMenuBar* pMenuBar);
	virtual void SetOriginal(CMFCRibbonBaseElement* pOriginal);

	virtual void GetElementsByID(UINT uiCmdID, CArray<CMFCRibbonBaseElement*, CMFCRibbonBaseElement*>& arButtons);
	virtual void GetItemIDsList(CList<UINT,UINT>& lstItems) const;

	virtual int AddToListBox(CMFCRibbonCommandsListBox* pWndListBox, BOOL bDeep);
	virtual void AddToKeyList(CArray<CMFCRibbonKeyTip*,CMFCRibbonKeyTip*>& arElems);

	virtual void OnRTLChanged(BOOL bIsRTL);
	virtual void CleanUpSizes();

	virtual void SetParentRibbonBar(CMFCRibbonBar* pRibbonBar);

// Attributes
protected:
	CArray<CMFCRibbonBaseElement*, CMFCRibbonBaseElement*> m_arButtons;

	CMFCToolBarImages m_Images;
	CMFCToolBarImages m_HotImages;
	CMFCToolBarImages m_DisabledImages;
};

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif
