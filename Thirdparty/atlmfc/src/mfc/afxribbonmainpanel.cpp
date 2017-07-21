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
#include "afxwinappex.h"
#include "afxcontrolbarutil.h"
#include "afxribbonbar.h"
#include "afxribbonmainpanel.h"
#include "afxribbonbuttonsgroup.h"
#include "afxribboncategory.h"
#include "afxvisualmanager.h"
#include "afxribbonlabel.h"
#include "afxribbonpanelmenu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////////
// CMFCRibbonRecentFilesList

#define AFX_SEPARATOR_HEIGHT 4
#define AFX_FILE_MARGIN 4
#define AFX_LABEL_MARGIN 4

class CMFCRibbonRecentFilesList : public CMFCRibbonButtonsGroup
{
	DECLARE_DYNCREATE(CMFCRibbonRecentFilesList)

public:
	CMFCRibbonRecentFilesList(LPCTSTR lpszLabel = NULL)
	{
		SetText(lpszLabel == NULL ? _T("") : lpszLabel);
	}

	void FillList();

protected:
	virtual void OnAfterChangeRect(CDC* pDC);
	virtual CSize GetRegularSize(CDC* pDC);
	virtual void OnDraw(CDC* pDC);
	virtual BOOL OnMenuKey(UINT nUpperChar);
};

IMPLEMENT_DYNCREATE(CMFCRibbonRecentFilesList, CMFCRibbonButtonsGroup)

void CMFCRibbonRecentFilesList::OnAfterChangeRect(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	int y = m_rect.top + 2;

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		pButton->SetParentMenu(m_pParentMenu);

		pButton->OnCalcTextSize(pDC);
		CSize sizeButton = pButton->GetSize(pDC);

		CRect rectButton = m_rect;
		rectButton.DeflateRect(1, 0);

		rectButton.top = y;
		rectButton.bottom = y + sizeButton.cy + 2 * AFX_FILE_MARGIN;

		pButton->SetRect(rectButton);
		pButton->OnAfterChangeRect(pDC);

		y = rectButton.bottom;
	}
}

CSize CMFCRibbonRecentFilesList::GetRegularSize(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	int cy = 4;

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		pButton->OnCalcTextSize(pDC);
		CSize sizeButton = pButton->GetSize(pDC);

		cy += sizeButton.cy + 2 * AFX_FILE_MARGIN;
	}

	const int nDefaultSize = 300;

	return CSize(afxGlobalData.GetRibbonImageScale() == 1. ? nDefaultSize :(int)(afxGlobalData.GetRibbonImageScale() *  nDefaultSize), cy);
}

void CMFCRibbonRecentFilesList::FillList()
{
	ASSERT_VALID(this);

	RemoveAll();

	// Add label:
	AddButton(new CMFCRibbonLabel(m_strText));

	CRecentFileList* pMRUFiles = ((CWinAppEx*)AfxGetApp())->m_pRecentFileList;

	TCHAR szCurDir [_MAX_PATH];
	::GetCurrentDirectory(_MAX_PATH, szCurDir);

	int nCurDir = lstrlen(szCurDir);
	ASSERT(nCurDir >= 0);

	szCurDir [nCurDir] = _T('\\');
	szCurDir [++ nCurDir] = _T('\0');

	int iNumOfFiles = 0; // Actual added to menu

	if (pMRUFiles != NULL)
	{
		for (int i = 0; i < pMRUFiles->GetSize(); i++)
		{
			CString strName;

			if (pMRUFiles->GetDisplayName(strName, i, szCurDir, nCurDir))
			{
				// Add shortcut number:
				CString strItem;

				if (iNumOfFiles == 9)
				{
					strItem.Format(_T("1&0 %s"), (LPCTSTR)strName);
				}
				else if (iNumOfFiles < 9)
				{
					strItem.Format(_T("&%d %s"), iNumOfFiles + 1, (LPCTSTR)strName);
				}
				else
				{
					strItem = strName;
				}

				CMFCRibbonButton* pFile = new CMFCRibbonButton;
				pFile->SetText(strItem);
				pFile->SetID(ID_FILE_MRU_FILE1 + i);
				pFile->SetToolTipText((*pMRUFiles)[i]);

				AddButton(pFile);

				iNumOfFiles++;
			}
		}
	}
}

void CMFCRibbonRecentFilesList::OnDraw(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if (m_rect.IsRectEmpty())
	{
		return;
	}

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pButton = m_arButtons [i];
		ASSERT_VALID(pButton);

		pButton->OnDraw(pDC);
	}
}

BOOL CMFCRibbonRecentFilesList::OnMenuKey(UINT nUpperChar)
{
	ASSERT_VALID(this);

	for (int i = 0; i < m_arButtons.GetSize(); i++)
	{
		CMFCRibbonButton* pButton = DYNAMIC_DOWNCAST(CMFCRibbonButton, m_arButtons [i]);

		if (pButton == NULL)
		{
			continue;
		}

		ASSERT_VALID(pButton);

		CString strLabel = pButton->GetText();

		int iAmpOffset = strLabel.Find(_T('&'));
		if (iAmpOffset >= 0 && iAmpOffset < strLabel.GetLength() - 1)
		{
			TCHAR szChar [2] = { strLabel.GetAt(iAmpOffset + 1), '\0' };
			CharUpper(szChar);

			if ((UINT)(szChar [0]) == nUpperChar && !pButton->IsDisabled())
			{
				pButton->OnClick(pButton->GetRect().TopLeft());
				return TRUE;
			}
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////
// CMFCRibbonMainPanel

IMPLEMENT_DYNCREATE(CMFCRibbonMainPanel, CMFCRibbonPanel)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFCRibbonMainPanel::CMFCRibbonMainPanel()
{
	m_nBottomElementsNum = 0;
	m_nTopMargin = 0;
	m_pElemOnRight = NULL;
	m_nRightPaneWidth = 0;
	m_bMenuMode = TRUE;
	m_pMainButton = NULL;

	m_rectMenuElements.SetRectEmpty();
}

CMFCRibbonMainPanel::~CMFCRibbonMainPanel()
{

}

void CMFCRibbonMainPanel::RecalcWidths(CDC* pDC, int /*nHeight*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	m_arWidths.RemoveAll();
	m_nCurrWidthIndex = 0;
	m_bIsCalcWidth = TRUE;

	Reposition(pDC, CRect(0, 0, 32767, 32767));
	m_arWidths.Add(m_nFullWidth);

	m_bIsCalcWidth = FALSE;
}

void CMFCRibbonMainPanel::Reposition(CDC* pDC, const CRect& rect)
{
	ASSERT_VALID(pDC);

	CSize size = rect.Size();
	size.cx -= m_nXMargin;
	size.cy -= m_nYMargin;

	int y = m_nTopMargin;
	int i = 0;

	int nMenuElements = GetMenuElements();

	m_rectMenuElements = rect;
	m_rectMenuElements.DeflateRect(m_nXMargin, m_nYMargin);
	m_rectMenuElements.top += m_nTopMargin;

	int nImageWidth = 0;

	if (m_pParent != NULL)
	{
		ASSERT_VALID(m_pParent);
		nImageWidth = m_pParent->GetImageSize(TRUE).cx;
	}

	// Reposition menu elements(on the left side):
	int nColumnWidth = 0;

	for (i = 0; i < nMenuElements; i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		pElem->OnCalcTextSize(pDC);
		pElem->SetTextAlwaysOnRight();

		CSize sizeElem = pElem->GetSize(pDC);

		if (sizeElem == CSize(0, 0))
		{
			pElem->SetRect(CRect(0, 0, 0, 0));
			continue;
		}

		CRect rectElem = CRect (CPoint(rect.left + m_nXMargin, rect.top + y + m_nYMargin), sizeElem);

		pElem->SetRect(rectElem);

		nColumnWidth = max(nColumnWidth, sizeElem.cx);
		y += sizeElem.cy;
	}

	nColumnWidth += 2 * m_nXMargin;

	m_rectMenuElements.right = m_rectMenuElements.left + nColumnWidth;
	m_rectMenuElements.bottom = y + m_nYMargin;

	m_rectMenuElements.InflateRect(1, 1);

	m_nFullWidth = nColumnWidth + 2 * m_nXMargin;

	// All menu elements should have the same width:
	for (i = 0; i < nMenuElements; i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		CRect rectElem = pElem->GetRect();

		if (!rectElem.IsRectEmpty())
		{
			rectElem.right = rectElem.left + nColumnWidth;

			if (nImageWidth > 0 && pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonSeparator)))
			{
				rectElem.left += nImageWidth + AFX_LABEL_MARGIN;
			}

			pElem->SetRect(rectElem);
		}
	}

	// Put element on right:
	if (m_pElemOnRight != NULL)
	{
		CMFCRibbonRecentFilesList* pRecentList = DYNAMIC_DOWNCAST(CMFCRibbonRecentFilesList, m_pElemOnRight);

		if (pRecentList != NULL)
		{
			ASSERT_VALID(pRecentList);

			if (pRecentList->GetCount() == 0)
			{
				pRecentList->FillList();
			}
		}

		m_pElemOnRight->SetInitialMode();
		m_pElemOnRight->OnCalcTextSize(pDC);

		CSize sizeRecentList = m_pElemOnRight->GetSize(pDC);

		int nDefaultWidth = afxGlobalData.GetRibbonImageScale() == 1. ? m_nRightPaneWidth :(int)(afxGlobalData.GetRibbonImageScale() *  m_nRightPaneWidth);

		sizeRecentList.cx = max(sizeRecentList.cx, nDefaultWidth);

		if (m_rectMenuElements.Height() < sizeRecentList.cy)
		{
			m_rectMenuElements.bottom = m_rectMenuElements.top + sizeRecentList.cy;
		}

		CRect rectRecentList = CRect(m_rectMenuElements.right, m_rectMenuElements.top, m_rectMenuElements.right + sizeRecentList.cx, m_rectMenuElements.bottom);

		if (pRecentList == NULL)
		{
			rectRecentList.DeflateRect(0, 1);
		}

		m_pElemOnRight->SetRect(rectRecentList);

		m_nFullWidth += sizeRecentList.cx;
	}

	// Put "bottom" elements on bottom:
	if (m_nBottomElementsNum > 0)
	{
		int x = rect.left + m_nFullWidth - m_nXMargin;
		int nRowHeight = 0;

		y = m_rectMenuElements.bottom + m_nYMargin;

		for (int nCount = 0; nCount < m_nBottomElementsNum; nCount++)
		{
			int nIndex = (int) m_arElements.GetSize() - nCount - 1;

			CMFCRibbonBaseElement* pElem = m_arElements [nIndex];
			ASSERT_VALID(pElem);

			pElem->OnCalcTextSize(pDC);

			CSize sizeElem = pElem->GetSize(pDC);

			if (sizeElem == CSize(0, 0))
			{
				pElem->SetRect(CRect(0, 0, 0, 0));
				continue;
			}

			sizeElem.cx += AFX_LABEL_MARGIN - 1;

			if (x - sizeElem.cx < rect.left + m_nXMargin)
			{
				x = rect.left + m_nFullWidth - m_nXMargin;
				y += nRowHeight;
				nRowHeight = 0;
			}

			CRect rectElem = CRect(CPoint(x - sizeElem.cx, y), sizeElem);
			pElem->SetRect(rectElem);

			nRowHeight = max(nRowHeight, sizeElem.cy);
			x = rectElem.left - AFX_LABEL_MARGIN;
		}

		y += nRowHeight;
	}

	for (i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		pElem->OnAfterChangeRect(pDC);
	}

	m_rect = rect;
	m_rect.bottom = m_rect.top + y + m_nYMargin;
	m_rect.right = m_rect.left + m_nFullWidth + m_nXMargin;
}

void CMFCRibbonMainPanel::AddRecentFilesList(LPCTSTR lpszLabel, int nWidth)
{
	ASSERT_VALID(this);
	ENSURE(lpszLabel != NULL);

	AddToRight(new CMFCRibbonRecentFilesList(lpszLabel), nWidth);
}

void CMFCRibbonMainPanel::AddToRight(CMFCRibbonBaseElement* pElem, int nWidth)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pElem);

	if (m_pElemOnRight != NULL)
	{
		// Already exist, delete previous
		m_arElements.RemoveAt(GetMenuElements());

		ASSERT_VALID(m_pElemOnRight);
		delete m_pElemOnRight;

		m_pElemOnRight = NULL;
	}

	pElem->SetParentCategory(m_pParent);

	m_arElements.InsertAt(GetMenuElements(), pElem);

	m_pElemOnRight = pElem;
	m_nRightPaneWidth = nWidth;
}

void CMFCRibbonMainPanel::AddToBottom(CMFCRibbonMainPanelButton* pElem)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pElem);

	m_nBottomElementsNum++;

	pElem->SetParentCategory(m_pParent);
	m_arElements.Add(pElem);
}

void CMFCRibbonMainPanel::Add(CMFCRibbonBaseElement* pElem)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pElem);

	pElem->SetParentCategory(m_pParent);
	m_arElements.InsertAt(GetMenuElements(), pElem);
}

int CMFCRibbonMainPanel::GetMenuElements() const
{
	ASSERT_VALID(this);

	int nMenuElements = (int) m_arElements.GetSize() - m_nBottomElementsNum;
	if (m_pElemOnRight != NULL)
	{
		nMenuElements--;
	}

	ASSERT(nMenuElements >= 0);
	return nMenuElements;
}

void CMFCRibbonMainPanel::CopyFrom(CMFCRibbonPanel& s)
{
	ASSERT_VALID(this);

	CMFCRibbonPanel::CopyFrom(s);

	CMFCRibbonMainPanel& src = (CMFCRibbonMainPanel&) s;

	m_nBottomElementsNum = src.m_nBottomElementsNum;
	m_nTopMargin = src.m_nTopMargin;
	m_pMainButton = src.m_pMainButton;

	m_pElemOnRight = NULL;
	m_nRightPaneWidth = src.m_nRightPaneWidth;

	if (src.m_pElemOnRight != NULL)
	{
		ASSERT_VALID(src.m_pElemOnRight);

		for (int i = 0; i < src.m_arElements.GetSize(); i++)
		{
			if (src.m_arElements [i] == src.m_pElemOnRight)
			{
				m_pElemOnRight = m_arElements [i];
				break;
			}
		}

		ASSERT_VALID(m_pElemOnRight);

		CMFCRibbonRecentFilesList* pRecentList = DYNAMIC_DOWNCAST(CMFCRibbonRecentFilesList, m_pElemOnRight);

		if (pRecentList != NULL)
		{
			ASSERT_VALID(pRecentList);
			pRecentList->RemoveAll();
		}
	}
}

BOOL CMFCRibbonMainPanel::GetPreferedMenuLocation(CRect& rect)
{
	ASSERT_VALID(this);

	if (m_pElemOnRight == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(m_pElemOnRight);

	rect = m_pElemOnRight->GetRect();
	rect.DeflateRect(1, 1);

	const int nShadowSize = CMFCMenuBar::IsMenuShadows() && !CMFCToolBar::IsCustomizeMode() && afxGlobalData.m_nBitsPerPixel > 8 ? // Don't draw shadows in 256 colors or less
		CMFCVisualManager::GetInstance()->GetMenuShadowDepth() : 0;

	rect.right -= nShadowSize + 3;
	rect.bottom -= nShadowSize + 3;

	return TRUE;
}

void CMFCRibbonMainPanel::DoPaint(CDC* pDC)
{
	ASSERT_VALID(pDC);

	if (m_rect.IsRectEmpty())
	{
		return;
	}

	CRect rectClip;
	pDC->GetClipBox(rectClip);

	CRect rectInter;

	if (!rectInter.IntersectRect(m_rect, rectClip))
	{
		return;
	}

	COLORREF clrTextOld = pDC->GetTextColor();

	// Fill panel background:
	COLORREF clrText = CMFCVisualManager::GetInstance()->OnDrawRibbonPanel(pDC, this, m_rect, CRect(0, 0, 0, 0));

	DrawApplicationButton(pDC, GetParentWnd());

	CRect rectFrame = m_rectMenuElements;

	CRect rectRecentFiles;
	rectRecentFiles.SetRectEmpty();

	if (m_pElemOnRight != NULL)
	{
		ASSERT_VALID(m_pElemOnRight);

		rectRecentFiles = m_pElemOnRight->GetRect();

		CMFCVisualManager::GetInstance()->OnDrawRibbonRecentFilesFrame(pDC, this, rectRecentFiles);
	}

	if (!rectRecentFiles.IsRectEmpty())
	{
		rectFrame.right = rectRecentFiles.right;
	}

	CMFCVisualManager::GetInstance()->OnFillRibbonMenuFrame(pDC, this, m_rectMenuElements);
	CMFCVisualManager::GetInstance()->OnDrawRibbonMainPanelFrame(pDC, this, rectFrame);

	pDC->SetTextColor(clrText);

	// Draw panel elements:
	for (int i = 0; i < m_arElements.GetSize(); i++)
	{
		CMFCRibbonBaseElement* pElem = m_arElements [i];
		ASSERT_VALID(pElem);

		if (rectInter.IntersectRect(pElem->GetRect(), rectClip))
		{
			pDC->SetTextColor(clrText);
			pElem->OnDraw(pDC);
		}
	}

	pDC->SetTextColor(clrTextOld);
}

CRect CMFCRibbonMainPanel::GetCommandsFrame() const
{
	ASSERT_VALID(this);

	CRect rectFrame = m_rectMenuElements;

	if (m_pElemOnRight != NULL)
	{
		ASSERT_VALID(m_pElemOnRight);

		CRect rectRecentFiles = m_pElemOnRight->GetRect();
		if (!rectRecentFiles.IsRectEmpty())
		{
			rectFrame.right = rectRecentFiles.right;
		}
	}

	return rectFrame;
}

void CMFCRibbonMainPanel::OnDrawMenuBorder(CDC* pDC, CMFCRibbonPanelMenuBar* pMenuBar)
{
	ASSERT_VALID(pMenuBar);
	DrawApplicationButton(pDC, pMenuBar->GetParent());
}

void CMFCRibbonMainPanel::DrawApplicationButton(CDC* pDC, CWnd* pWnd)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT_VALID(pWnd);

	if (m_pMainButton == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pMainButton);
	ASSERT_VALID(m_pMainButton->GetParentRibbonBar());

	CRect rectMainButtonSaved = m_pMainButton->GetRect();
	CRect rectMainButton = rectMainButtonSaved;

	m_pMainButton->GetParentRibbonBar()->ClientToScreen(&rectMainButton);
	pWnd->ScreenToClient(&rectMainButton);

	if (rectMainButton.top > m_rectMenuElements.bottom)
	{
		return;
	}

	m_pMainButton->SetRect(rectMainButton);

	CMFCVisualManager::GetInstance()->OnDrawRibbonApplicationButton(pDC, m_pMainButton);

	m_pMainButton->OnDraw(pDC);

	m_pMainButton->SetRect(rectMainButtonSaved);
}

CMFCRibbonBaseElement* CMFCRibbonMainPanel::MouseButtonDown(CPoint point)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement* pElement = CMFCRibbonPanel::MouseButtonDown(point);

	if (m_pMainButton != NULL)
	{
		ASSERT_VALID(m_pMainButton);
		ASSERT_VALID(m_pMainButton->GetParentRibbonBar());
		ASSERT_VALID(GetParentWnd());

		CRect rectMainButton = m_pMainButton->GetRect();

		m_pMainButton->GetParentRibbonBar()->ClientToScreen(&rectMainButton);
		GetParentWnd()->ScreenToClient(&rectMainButton);

		if (rectMainButton.PtInRect(point))
		{
			m_pMainButton->ClosePopupMenu();
			return NULL;
		}
	}

	return pElement;
}

//////////////////////////////////////////////////////////////////////////////////
// CMFCRibbonMainPanelButton

IMPLEMENT_DYNCREATE(CMFCRibbonMainPanelButton, CMFCRibbonButton)

CMFCRibbonMainPanelButton::CMFCRibbonMainPanelButton()
{
}

CMFCRibbonMainPanelButton::CMFCRibbonMainPanelButton(UINT nID, LPCTSTR lpszText, int nSmallImageIndex) : CMFCRibbonButton(nID, lpszText, nSmallImageIndex)
{
}

CMFCRibbonMainPanelButton::CMFCRibbonMainPanelButton(UINT nID, LPCTSTR lpszText, HICON hIcon) : CMFCRibbonButton(nID, lpszText, hIcon)
{
}

CMFCRibbonMainPanelButton::~CMFCRibbonMainPanelButton()
{
}

COLORREF CMFCRibbonMainPanelButton::OnFillBackground(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if (DYNAMIC_DOWNCAST(CMFCRibbonMainPanel, GetParentPanel()) == NULL)
	{
		return CMFCRibbonButton::OnFillBackground(pDC);
	}

	return CMFCVisualManager::GetInstance()->OnFillRibbonMainPanelButton(pDC, this);
}

void CMFCRibbonMainPanelButton::OnDrawBorder(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if (DYNAMIC_DOWNCAST(CMFCRibbonMainPanel, GetParentPanel()) == NULL)
	{
		CMFCRibbonButton::OnDrawBorder(pDC);
		return;
	}

	CMFCVisualManager::GetInstance()->OnDrawRibbonMainPanelButtonBorder(pDC, this);
}





