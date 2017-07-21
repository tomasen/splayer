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
#include "afxcontrolbarutil.h"
#include "afxglobals.h"
#include "afxribbonstatusbar.h"
#include "afxribbonstatusbarpane.h"
#include "afxvisualmanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMap<UINT,UINT,CMFCRibbonStatusBarPane*,CMFCRibbonStatusBarPane*> CMFCRibbonStatusBarPane::m_mapAnimations;
CCriticalSection CMFCRibbonStatusBarPane::m_CriticalSection;

IMPLEMENT_DYNCREATE(CMFCRibbonStatusBarPane, CMFCRibbonButton)

// Construction/Destruction
CMFCRibbonStatusBarPane::CMFCRibbonStatusBarPane()
{
	CommonInit();
}

CMFCRibbonStatusBarPane::CMFCRibbonStatusBarPane(UINT nCmdID, LPCTSTR lpszText, BOOL bIsStatic, HICON hIcon, LPCTSTR lpszAlmostLargeText) :
	CMFCRibbonButton(nCmdID, lpszText, hIcon)
{
	CommonInit();

	m_strAlmostLargeText = (lpszAlmostLargeText == NULL) ? _T("") : lpszAlmostLargeText;
	m_bIsStatic = bIsStatic;
}

CMFCRibbonStatusBarPane::CMFCRibbonStatusBarPane(UINT nCmdID, LPCTSTR lpszText, HBITMAP hBmpAnimationList, int cxAnimation, COLORREF clrTransp, HICON hIcon, BOOL bIsStatic) :
	CMFCRibbonButton(nCmdID, lpszText, hIcon)
{
	CommonInit();

	m_bIsStatic = bIsStatic;
	SetAnimationList(hBmpAnimationList, cxAnimation, clrTransp);
}

CMFCRibbonStatusBarPane::CMFCRibbonStatusBarPane(UINT nCmdID, LPCTSTR lpszText, UINT uiAnimationListResID, int cxAnimation, COLORREF clrTransp, HICON hIcon, BOOL bIsStatic) :
	CMFCRibbonButton(nCmdID, lpszText, hIcon)
{
	CommonInit();

	m_bIsStatic = bIsStatic;
	SetAnimationList(uiAnimationListResID, cxAnimation, clrTransp);
}

void CMFCRibbonStatusBarPane::CommonInit()
{
	m_bIsExtended = FALSE;
	m_bIsStatic = TRUE;
	m_szMargin = CSize(9, 0);
	m_bTextAlwaysOnRight = TRUE;
	m_nTextAlign = TA_LEFT;
	m_bIsTextTruncated = FALSE;
	m_nAnimTimerID = 0;
	m_nAnimationIndex = -1;
	m_nAnimationDuration = 0;
	m_dwAnimationStartTime = 0;
}

CMFCRibbonStatusBarPane::~CMFCRibbonStatusBarPane()
{
	StopAnimation();
}

COLORREF CMFCRibbonStatusBarPane::OnFillBackground(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	BOOL bIsHighlighted = m_bIsHighlighted;
	BOOL bIsPressed = m_bIsPressed;
	BOOL bIsDisabled = m_bIsDisabled;

	if (m_bIsStatic)
	{
		m_bIsDisabled = FALSE;
	}

	if (m_bIsStatic || m_bIsDisabled)
	{
		m_bIsHighlighted = FALSE;
		m_bIsPressed = FALSE;
	}

	COLORREF clrText = CMFCVisualManager::GetInstance()->OnDrawRibbonStatusBarPane(pDC, DYNAMIC_DOWNCAST(CMFCRibbonStatusBar, m_pRibbonBar), this);

	m_bIsHighlighted = bIsHighlighted;
	m_bIsPressed = bIsPressed;
	m_bIsDisabled = bIsDisabled;

	return clrText;
}

void CMFCRibbonStatusBarPane::OnCalcTextSize(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	CMFCRibbonButton::OnCalcTextSize(pDC);

	if (!m_strAlmostLargeText.IsEmpty())
	{
		const int nTextWidth = pDC->GetTextExtent(m_strAlmostLargeText).cx;

		m_bIsTextTruncated = nTextWidth < m_sizeTextRight.cx;
		m_sizeTextRight.cx = nTextWidth;
	}
}

int CMFCRibbonStatusBarPane::DrawPaneText(CDC* pDC, const CString& strText, CRect rectText, UINT /*uiDTFlags*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	UINT uiDTFlags = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS;

	if (m_nTextAlign == TA_CENTER)
	{
		uiDTFlags |= DT_CENTER;
	}
	else if (m_nTextAlign == TA_RIGHT)
	{
		uiDTFlags |= DT_RIGHT;
		rectText.right -= m_szMargin.cx;
	}

	return CMFCRibbonButton::DrawRibbonText(pDC, strText, rectText, uiDTFlags);
}

CString CMFCRibbonStatusBarPane::GetToolTipText() const
{
	ASSERT_VALID(this);

	CString str = CMFCRibbonButton::GetToolTipText();

	if (!str.IsEmpty())
	{
		return str;
	}

	if (m_bIsTextTruncated || m_AnimImages.GetCount() > 0)
	{
		str = m_strText;
	}

	if (str.IsEmpty() && !m_strDescription.IsEmpty())
	{
		str = m_strText;
	}

	return str;
}

void CMFCRibbonStatusBarPane::CopyFrom(const CMFCRibbonBaseElement& s)
{
	ASSERT_VALID(this);

	CMFCRibbonBaseElement::CopyFrom(s);

	CMFCRibbonStatusBarPane& src = (CMFCRibbonStatusBarPane&) s;

	m_bIsStatic = src.m_bIsStatic;
	m_bIsExtended = src.m_bIsExtended;
	m_strAlmostLargeText = src.m_strAlmostLargeText;
	m_nTextAlign = src.m_nTextAlign;
	m_bIsTextTruncated = src.m_bIsTextTruncated;

	src.m_AnimImages.CopyTo(m_AnimImages);
}

void CMFCRibbonStatusBarPane::SetAnimationList(HBITMAP hBmpAnimationList, int cxAnimation, COLORREF clrTransp)
{
	ASSERT_VALID(this);

	if (m_AnimImages.IsValid())
	{
		m_AnimImages.Clear();
	}

	if (hBmpAnimationList == NULL)
	{
		return;
	}

	BITMAP bitmap;
	::GetObject(hBmpAnimationList, sizeof(BITMAP), &bitmap);

	int cy = bitmap.bmHeight;

	m_AnimImages.SetImageSize(CSize(cxAnimation, cy));
	m_AnimImages.SetTransparentColor(clrTransp);
	m_AnimImages.AddImage(hBmpAnimationList, TRUE);
}

BOOL CMFCRibbonStatusBarPane::SetAnimationList(UINT uiAnimationListResID, int cxAnimation, COLORREF clrTransp)
{
	ASSERT_VALID(this);

	if (m_AnimImages.IsValid())
	{
		m_AnimImages.Clear();
	}

	if (uiAnimationListResID == 0)
	{
		return TRUE;
	}

	m_AnimImages.SetTransparentColor(clrTransp);

	if (!m_AnimImages.Load(uiAnimationListResID))
	{
		return FALSE;
	}

	BITMAP bitmap;
	::GetObject(m_AnimImages.GetImageWell(), sizeof(BITMAP), &bitmap);

	int cy = bitmap.bmHeight;

	m_AnimImages.SetImageSize(CSize(cxAnimation, cy));

	return TRUE;
}

CSize CMFCRibbonStatusBarPane::GetIntermediateSize(CDC* pDC)
{
	ASSERT_VALID(this);

	if (m_AnimImages.GetCount() > 0)
	{
		CSize imageSize = m_AnimImages.GetImageSize();

		return CSize(imageSize.cx + 2 * m_szMargin.cx, imageSize.cy + 2 * m_szMargin.cy);
	}

	CSize size = CMFCRibbonButton::GetIntermediateSize(pDC);
	size.cx -= GetTextOffset() + 1;

	return size;
}

void CMFCRibbonStatusBarPane::OnDraw(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if (m_AnimImages.GetCount() == 0)
	{
		CMFCRibbonButton::OnDraw(pDC);
		return;
	}

	if (m_rect.IsRectEmpty())
	{
		return;
	}

	OnFillBackground(pDC);

	if (m_nAnimationIndex < 0)
	{
		CString strText = m_strText;
		m_strText.Empty();

		CMFCRibbonButton::OnDraw(pDC);
		m_strText = strText;
	}
	else
	{
		if (afxGlobalData.GetRibbonImageScale() != 1.)
		{
			CAfxDrawState ds;

			CSize sizeImage = m_AnimImages.GetImageSize();
			CSize sizeDest = CSize((int)(.5 + afxGlobalData.GetRibbonImageScale() * sizeImage.cx), (int)(.5 + afxGlobalData.GetRibbonImageScale() * sizeImage.cy));

			m_AnimImages.PrepareDrawImage(ds, sizeDest);

			CPoint ptImage = m_rect.TopLeft();

			ptImage.Offset(max(0, (m_rect.Width() - sizeDest.cx) / 2), max(0, (m_rect.Height() - sizeDest.cy) / 2));

			m_AnimImages.Draw(pDC, ptImage.x, ptImage.y, m_nAnimationIndex);

			m_AnimImages.EndDrawImage(ds);
		}
		else
		{
			m_AnimImages.DrawEx(pDC, m_rect, m_nAnimationIndex, CMFCToolBarImages::ImageAlignHorzCenter, CMFCToolBarImages::ImageAlignVertCenter);
		}
	}

	OnDrawBorder(pDC);
}

void CMFCRibbonStatusBarPane::StartAnimation(UINT nFrameDelay, UINT nDuration)
{
	ASSERT_VALID(this);

	if (m_AnimImages.GetCount() == 0)
	{
		ASSERT(FALSE);
		return;
	}

	StopAnimation();

	m_nAnimationIndex = 0;

	if ((m_nAnimationDuration = nDuration) > 0)
	{
		m_dwAnimationStartTime = ::GetTickCount();
	}

	m_nAnimTimerID = (UINT) ::SetTimer(NULL, 0, nFrameDelay, AnimTimerProc);

	m_CriticalSection.Lock();
	m_mapAnimations.SetAt(m_nAnimTimerID, this);
	m_CriticalSection.Unlock();
}

void CMFCRibbonStatusBarPane::StopAnimation()
{
	ASSERT_VALID(this);

	if (m_nAnimTimerID == 0)
	{
		return;
	}

	::KillTimer(NULL, m_nAnimTimerID);

	m_CriticalSection.Lock();
	m_mapAnimations.RemoveKey(m_nAnimTimerID);
	m_CriticalSection.Unlock();

	m_nAnimTimerID = 0;
	m_nAnimationIndex = -1;

	OnFinishAnimation();

	Redraw();
}

VOID CALLBACK CMFCRibbonStatusBarPane::AnimTimerProc(HWND /*hwnd*/, UINT /*uMsg*/, UINT_PTR idEvent, DWORD dwTime)
{
	CMFCRibbonStatusBarPane* pPane = NULL;

	m_CriticalSection.Lock();

	if (!m_mapAnimations.Lookup((UINT) idEvent, pPane))
	{
		m_CriticalSection.Unlock();
		return;
	}

	ASSERT_VALID(pPane);

	m_CriticalSection.Unlock();

	if (pPane->m_nAnimationDuration > 0)
	{
		if (dwTime - pPane->m_dwAnimationStartTime >(DWORD) pPane->m_nAnimationDuration)
		{
			pPane->StopAnimation();
			return;
		}
	}

	pPane->m_nAnimationIndex++;

	if (pPane->m_nAnimationIndex >= pPane->m_AnimImages.GetCount())
	{
		pPane->m_nAnimationIndex = 0;
	}

	pPane->Redraw();
}

void CMFCRibbonStatusBarPane::OnRTLChanged(BOOL bIsRTL)
{
	ASSERT_VALID(this);

	CMFCRibbonButton::OnRTLChanged(bIsRTL);

	m_AnimImages.Mirror();
}


