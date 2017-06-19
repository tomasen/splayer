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
#include "afxcontrolbarutil.h"
#include "afxsmartdockingguide.h"
#include "afxribbonres.h"
#include "afxglobals.h"
#include "afxdockingmanager.h"
#include "afxvisualmanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int CSmartDockingStandaloneGuide::m_nLeftOffsetX = 16;
const int CSmartDockingStandaloneGuide::m_nRightOffsetX = 16;
const int CSmartDockingStandaloneGuide::m_nTopOffsetY = 16;
const int CSmartDockingStandaloneGuide::m_nBottomOffsetY = 16;

IMPLEMENT_DYNCREATE(CSmartDockingStandaloneGuide, CObject)
IMPLEMENT_DYNCREATE(CSmartDockingGroupGuidesManager, CObject)

#define COLOR_HIGHLIGHT_FRAME RGB(65, 112, 202)

static void __stdcall ShadeRect(CDC* pDC, CRect rect, BOOL bIsVert)
{
	ASSERT_VALID(pDC);

	COLORREF colors [2] =
	{
		RGB(198, 198, 198),
		RGB(206, 206, 206),
	};

	rect.DeflateRect(1, 1);

	for (int i = 0; i < 2; i++)
	{
		CPen pen(PS_SOLID, 1, colors [i]);
		CPen* pOldPen = pDC->SelectObject(&pen);

		if (bIsVert)
		{
			pDC->MoveTo(rect.left + i, rect.top);
			pDC->LineTo(rect.left + i, rect.bottom);
		}
		else
		{
			pDC->MoveTo(rect.left, rect.top + i);
			pDC->LineTo(rect.right, rect.top + i);
		}

		pDC->SelectObject(pOldPen);
	}
}

// CSmartDockingStandaloneGuide

CSmartDockingStandaloneGuide::CSmartDockingStandaloneGuide() : m_nSideNo(sdNONE), m_cx(-1), m_cy(-1), m_bHiLited(FALSE), m_bLayered(FALSE), m_bIsDefaultImage(TRUE)
{
}

CSmartDockingStandaloneGuide::~CSmartDockingStandaloneGuide()
{
	Destroy();
}

void CSmartDockingStandaloneGuide::Create(SDMarkerPlace nSideNo, CWnd* pwndOwner)
{
	ASSERT(nSideNo >= sdLEFT && nSideNo <= sdBOTTOM);

	m_nSideNo = nSideNo;

	CSmartDockingInfo& params = CDockingManager::GetSmartDockingParams();

	InitImages(params);

	m_Rgn.Attach(CMFCToolBarImages::CreateRegionFromImage(m_Image.GetImageWell(), params.m_clrTransparent));

	CRect rect;
	m_Rgn.GetRgnBox(rect);

	m_cx = rect.Width();
	m_cy = rect.Height();

	HBITMAP hBmp = m_Image.GetImageWellLight();
	if (hBmp == NULL)
	{
		hBmp = m_Image.GetImageWell();
	}

	BOOL bIsVert = m_nSideNo == sdTOP || m_nSideNo == sdBOTTOM;

	if (afxGlobalData.IsWindowsLayerSupportAvailable())
	{
		m_wndBmp.Create(&rect, hBmp, NULL, pwndOwner, m_bIsDefaultImage, bIsVert);
		m_wndBmp.ModifyStyleEx(0, WS_EX_LAYERED);
		afxGlobalData.SetLayeredAttrib(m_wndBmp.GetSafeHwnd(), params.m_clrTransparent, 0, LWA_COLORKEY);
		m_bLayered = TRUE;
	}
	else
	{
		m_wndBmp.Create(&rect, hBmp, m_Rgn, pwndOwner, m_bIsDefaultImage, bIsVert);
		m_bLayered = FALSE;
	}

	m_wndBmp.ModifyStyleEx(0, WS_EX_TOPMOST);
}

void CSmartDockingStandaloneGuide::Destroy()
{
	if (::IsWindow(m_wndBmp.m_hWnd))
	{
		m_wndBmp.DestroyWindow();
	}
}

void CSmartDockingStandaloneGuide::Show(BOOL bShow)
{
	if (::IsWindow(m_wndBmp.m_hWnd))
	{
		m_wndBmp.ShowWindow(bShow ? SW_SHOW : SW_HIDE);
	}
}

void CSmartDockingStandaloneGuide::AdjustPos(CRect rcHost)
{
	int x = 0;
	int y = 0;

	switch (m_nSideNo)
	{
	case sdLEFT:
		x = rcHost.left + m_nLeftOffsetX;
		y = ((rcHost.bottom + rcHost.top) >> 1) -(m_cy>>1);
		break;

	case sdRIGHT:
		x = rcHost.right - m_nRightOffsetX - m_cx;
		y = ((rcHost.bottom + rcHost.top) >> 1) -(m_cy>>1);
		break;

	case sdTOP:
		x = ((rcHost.left + rcHost.right) >> 1) -(m_cx >> 1);
		y = rcHost.top + m_nTopOffsetY;
		break;

	case sdBOTTOM:
		x = ((rcHost.left + rcHost.right) >> 1) -(m_cx >> 1);
		y = rcHost.bottom - m_nBottomOffsetY - m_cy;
		break;

	default:
		ASSERT(FALSE);
		return;
	}

	if (m_wndBmp.GetSafeHwnd() != NULL)
	{
		m_wndBmp.SetWindowPos(&CWnd::wndTopMost, x, y, -1, -1, SWP_NOSIZE | SWP_NOACTIVATE);
	}
}

void CSmartDockingStandaloneGuide::Highlight(BOOL bHiLite)
{
	if (m_bHiLited == bHiLite)
	{
		return;
	}

	m_bHiLited = bHiLite;
	m_wndBmp.Highlight(m_bHiLited);

	if (!m_bIsDefaultImage)
	{
		HBITMAP hBmpLight = m_Image.GetImageWellLight();
		if (hBmpLight == NULL)
		{
			hBmpLight = m_Image.GetImageWell();
		}

		m_wndBmp.Assign(bHiLite ? m_Image.GetImageWell() : hBmpLight, TRUE);
	}
}

BOOL CSmartDockingStandaloneGuide::IsPtIn(CPoint point) const
{
	if (m_wndBmp.GetSafeHwnd() == NULL || !m_wndBmp.IsWindowVisible())
	{
		return FALSE;
	}
	m_wndBmp.ScreenToClient(&point);

	if (m_bLayered)
	{
		return m_Rgn.PtInRegion(point);
	}
	else
	{
		CRgn rgn;
		rgn.CreateRectRgn(0, 0, 0, 0);

		m_wndBmp.GetWindowRgn(rgn);

		return rgn.PtInRegion(point);
	}
}

void CSmartDockingStandaloneGuide::InitImages(CSmartDockingInfo& params)
{
	static UINT uiDefaultMarkerIDs [] =
	{
		IDB_AFXBARRES_SD_LEFT,
		IDB_AFXBARRES_SD_RIGHT,
		IDB_AFXBARRES_SD_TOP,
		IDB_AFXBARRES_SD_BOTTOM,
		IDB_AFXBARRES_SD_MIDDLE
	};

	m_Image.Clear();
	m_Image.SetLightPercentage(-1);

	int nIndex = -1;

	switch (m_nSideNo)
	{
	case sdLEFT:
	case sdCLEFT:
		nIndex = 0;
		break;

	case sdRIGHT:
	case sdCRIGHT:
		nIndex = 1;
		break;

	case sdTOP:
	case sdCTOP:
		nIndex = 2;
		break;

	case sdBOTTOM:
	case sdCBOTTOM:
		nIndex = 3;
		break;

	case sdCMIDDLE:
		nIndex = 4;
		break;

	default:
		ASSERT(FALSE);
		return;
	}

	UINT uiResID = params.m_uiMarkerBmpResID [nIndex];
	m_bIsDefaultImage = uiResID == 0;

	if (m_bIsDefaultImage)
	{
		// Use default marker:
		uiResID = uiDefaultMarkerIDs [nIndex];
	}

	m_Image.SetMapTo3DColors(FALSE);
	m_Image.SetAlwaysLight();
	m_Image.Load(uiResID);
	m_Image.SetSingleImage();

	m_Image.SetTransparentColor(params.m_clrTransparent);

	COLORREF clrToneSrc = m_bIsDefaultImage ?(COLORREF)-1 : params.m_clrToneSrc;
	COLORREF clrToneDst = m_bIsDefaultImage && params.m_clrToneDest == -1 ? CMFCVisualManager::GetInstance()->GetSmartDockingHighlightToneColor() : params.m_clrToneDest;

	if (clrToneSrc != (COLORREF)-1 && clrToneDst != (COLORREF)-1)
	{
		m_Image.AdaptColors(clrToneSrc, clrToneDst);
	}

	HWND hwndBmp = m_wndBmp.GetSafeHwnd();
	if (hwndBmp != NULL)
	{
		HBITMAP hBmpLight = m_Image.GetImageWellLight();
		if (hBmpLight == NULL)
		{
			hBmpLight = m_Image.GetImageWell();
		}

		m_wndBmp.Assign(hBmpLight, FALSE);
		afxGlobalData.SetLayeredAttrib(hwndBmp, params.m_clrTransparent, 0, LWA_COLORKEY);
	}
}

// CSmartDockingGroupGuidesWnd

CSmartDockingGroupGuidesWnd::CSmartDockingGroupGuidesWnd() : m_pCentralGroup(NULL)
{
	CSmartDockingInfo& params = CDockingManager::GetSmartDockingParams();

	COLORREF clrBaseGroupBackground;
	COLORREF clrBaseGroupBorder;

	CMFCVisualManager::GetInstance()->GetSmartDockingBaseGuideColors(clrBaseGroupBackground, clrBaseGroupBorder);

	m_brBaseBackground.CreateSolidBrush(params.m_clrBaseBackground == -1 ? clrBaseGroupBackground : params.m_clrBaseBackground);
	m_brBaseBorder.CreateSolidBrush(params.m_clrBaseBorder == -1 ? clrBaseGroupBorder : params.m_clrBaseBorder);
}

BEGIN_MESSAGE_MAP(CSmartDockingGroupGuidesWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CSmartDockingGroupGuidesWnd::OnPaint()
{
	CSmartDockingInfo& params = CDockingManager::GetSmartDockingParams();

	CPaintDC dc(this); // device context for painting

	CMemDC memDC(dc, this);
	CDC* pDC = &memDC.GetDC();

	CRect rectClient;
	GetClientRect(rectClient);

	if (afxGlobalData.IsWindowsLayerSupportAvailable())
	{
		CBrush brBack;
		brBack.CreateSolidBrush(params.m_clrTransparent);

		pDC->FillRect(rectClient, &brBack);
	}

	ASSERT_VALID(m_pCentralGroup);

	m_pCentralGroup->DrawCentralGroupGuides(*pDC, m_brBaseBackground, m_brBaseBorder);
}

void CSmartDockingGroupGuidesWnd::OnClose()
{
	// so that it does not get destroyed
}

BOOL CSmartDockingGroupGuidesWnd::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

// CSmartDockingGroupGuide

CSmartDockingGroupGuide::CSmartDockingGroupGuide() : m_pCentralGroup(NULL), m_bVisible(TRUE), m_clrFrame((COLORREF)-1)
{
}

CSmartDockingGroupGuide::~CSmartDockingGroupGuide()
{
}

void CSmartDockingGroupGuide::Create(SDMarkerPlace, CWnd*)
{
	// should never be called
	ASSERT(FALSE);
}

void CSmartDockingGroupGuide::Destroy()
{
	// should never be called
	ASSERT(FALSE);
}

void CSmartDockingGroupGuide::Show(BOOL)
{
	// should never be called
	ASSERT(FALSE);
}

void CSmartDockingGroupGuide::AdjustPos(CRect)
{
	// should never be called
	ASSERT(FALSE);
}

void CSmartDockingGroupGuide::Highlight(BOOL bHiLite)
{
	if (m_bHiLited == bHiLite)
	{
		return;
	}

	ASSERT_VALID(m_pCentralGroup);

	m_bHiLited = bHiLite;
	m_pCentralGroup->m_Wnd.RedrawWindow();
}

void CSmartDockingGroupGuide::SetVisible(BOOL bVisible/* = TRUE*/, BOOL bRedraw/* = TRUE*/)
{
	m_bVisible = bVisible;

	if (bRedraw && m_pCentralGroup != NULL)
	{
		ASSERT_VALID(m_pCentralGroup);
		m_pCentralGroup->m_Wnd.RedrawWindow();
	}
}

BOOL CSmartDockingGroupGuide::IsPtIn(CPoint point) const
{
	if (!m_bVisible)
	{
		return FALSE;
	}

	m_pCentralGroup->m_Wnd.ScreenToClient(&point);
	return m_Rgn.PtInRegion(point.x, point.y);
}

void CSmartDockingGroupGuide::Create(SDMarkerPlace nSideNo, CSmartDockingGroupGuidesManager* pCentralGroup)
{
	ASSERT(nSideNo >= sdCLEFT && nSideNo <= sdCMIDDLE);

	m_nSideNo = nSideNo;
	m_pCentralGroup = pCentralGroup;

	CSmartDockingInfo& params = CDockingManager::GetSmartDockingParams();

	InitImages(params);

	if (m_bIsDefaultImage)
	{
		params.m_nCentralGroupOffset = 9;
		params.m_sizeTotal = CSize(88, 88);
	}

	COLORREF clrBaseGroupBackground;
	CMFCVisualManager::GetInstance()->GetSmartDockingBaseGuideColors(clrBaseGroupBackground, m_clrFrame);

	m_penFrame.CreatePen(PS_SOLID, 1, m_clrFrame);
	m_penHighlight.CreatePen(PS_SOLID, 1, COLOR_HIGHLIGHT_FRAME);

	const CSize sizeGroup = params.m_sizeTotal;
	const CSize sizeImage = m_Image.GetImageSize();

	switch (m_nSideNo)
	{
	case sdCLEFT:
		m_nOffsetX = 0;
		m_nOffsetY = (sizeGroup.cy  - sizeImage.cy) / 2;
		break;

	case sdCRIGHT:
		m_nOffsetX = sizeGroup.cx  - sizeImage.cx;
		m_nOffsetY = (sizeGroup.cy  - sizeImage.cy) / 2;
		break;

	case sdCTOP:
		m_nOffsetX = (sizeGroup.cx  - sizeImage.cx) / 2;
		m_nOffsetY = 0;
		break;

	case sdCBOTTOM:
		m_nOffsetX = (sizeGroup.cx  - sizeImage.cx) / 2;
		m_nOffsetY = sizeGroup.cy  - sizeImage.cy;
		break;

	case sdCMIDDLE:
		m_nOffsetX = (sizeGroup.cx  - sizeImage.cx) / 2;
		m_nOffsetY = (sizeGroup.cy  - sizeImage.cy) / 2;
		break;
	}

	m_Rgn.Attach(CMFCToolBarImages::CreateRegionFromImage(m_Image.GetImageWell(), params.m_clrTransparent));
	m_Rgn.OffsetRgn(m_nOffsetX, m_nOffsetY);
}

void CSmartDockingGroupGuide::DestroyImages()
{
	m_Rgn.DeleteObject();
}

void CSmartDockingGroupGuide::Draw(CDC& dc)
{
	const BOOL bFadeImage = !m_bHiLited && !m_bIsDefaultImage;

	CAfxDrawState ds;
	m_Image.PrepareDrawImage(ds, CSize(0, 0), bFadeImage);

	m_Image.Draw(&dc, m_nOffsetX, m_nOffsetY, 0, FALSE, FALSE, FALSE, FALSE, bFadeImage);
	m_Image.EndDrawImage(ds);

	if (!m_bIsDefaultImage)
	{
		return;
	}

	// For the default image we need to draw the border:
	CRect rect;
	m_Rgn.GetRgnBox(rect);

	CPen* pOldPen = dc.SelectObject(m_bHiLited ? &m_penHighlight : &m_penFrame);

	switch (m_nSideNo)
	{
	case sdCLEFT:
		rect.right -= 7;
		dc.MoveTo(rect.right, rect.top);
		dc.LineTo(rect.left, rect.top);
		dc.LineTo(rect.left, rect.bottom);
		dc.LineTo(rect.right, rect.bottom);
		ShadeRect(&dc, rect, FALSE);
		break;

	case sdCRIGHT:
		rect.left += 7;
		dc.MoveTo(rect.left, rect.top);
		dc.LineTo(rect.right - 1, rect.top);
		dc.LineTo(rect.right - 1, rect.bottom);
		dc.LineTo(rect.left, rect.bottom);
		ShadeRect(&dc, rect, FALSE);
		break;

	case sdCTOP:
		rect.bottom -= 7;
		dc.MoveTo(rect.left, rect.bottom);
		dc.LineTo(rect.left, rect.top);
		dc.LineTo(rect.right, rect.top);
		dc.LineTo(rect.right, rect.bottom);
		ShadeRect(&dc, rect, TRUE);
		break;

	case sdCBOTTOM:
		rect.top += 7;
		dc.MoveTo(rect.left, rect.top);
		dc.LineTo(rect.left, rect.bottom - 1);
		dc.LineTo(rect.right, rect.bottom - 1);
		dc.LineTo(rect.right, rect.top);
		ShadeRect(&dc, rect, TRUE);
		break;

	case sdCMIDDLE:
		break;
	}

	dc.SelectObject(pOldPen);
}

// CSmartDockingGroupGuidesManager

CSmartDockingGroupGuidesManager::CSmartDockingGroupGuidesManager() : m_bCreated(FALSE), m_bMiddleIsOn(FALSE), m_bLayered(FALSE)
{
}

CSmartDockingGroupGuidesManager::~CSmartDockingGroupGuidesManager()
{
	Destroy();
}

void CSmartDockingGroupGuidesManager::Create(CWnd* pwndOwner)
{
	if (m_bCreated)
	{
		return;
	}

	CRgn rgnAll;
	rgnAll.CreateRectRgn(0, 0, 0, 0);

	CSmartDockingStandaloneGuide::SDMarkerPlace i;
	for (i = CSmartDockingStandaloneGuide::sdCLEFT; i <= CSmartDockingStandaloneGuide::sdCMIDDLE; ++reinterpret_cast<int&>(i))
	{
		m_arMarkers[i - CSmartDockingStandaloneGuide::sdCLEFT].Create(i, this);
		rgnAll.CombineRgn(&rgnAll, &m_arMarkers[i - CSmartDockingStandaloneGuide::sdCLEFT].m_Rgn, RGN_OR);
	}

	CSmartDockingInfo& params = CDockingManager::GetSmartDockingParams();

	CRect rectBase;
	rgnAll.GetRgnBox(rectBase);
	rectBase.DeflateRect(params.m_nCentralGroupOffset, params.m_nCentralGroupOffset);

#define AFX_BASE_PTS 4
	POINT ptBase [AFX_BASE_PTS];

	ptBase [0].x = rectBase.left;
	ptBase [0].y = rectBase.CenterPoint().y;
	ptBase [1].x = rectBase.CenterPoint().x;
	ptBase [1].y = rectBase.bottom;
	ptBase [2].x = rectBase.right;
	ptBase [2].y = rectBase.CenterPoint().y;
	ptBase [3].x = rectBase.CenterPoint().x;
	ptBase [3].y = rectBase.top;

	m_rgnBase.CreatePolygonRgn(ptBase, AFX_BASE_PTS, ALTERNATE);

	rgnAll.CombineRgn(&rgnAll, &m_rgnBase, RGN_OR);

	CRect rcGroup;
	rgnAll.GetRgnBox(rcGroup);

	BOOL bResult = FALSE;

	bResult = m_Wnd.CreateEx(WS_EX_TOPMOST, GetSmartDockingWndClassName<CS_OWNDC | CS_SAVEBITS>(), _T(""), WS_POPUP, rcGroup, pwndOwner, NULL);

	if (bResult)
	{
		m_Wnd.m_pCentralGroup = this;

		if (afxGlobalData.IsWindowsLayerSupportAvailable())
		{
			m_Wnd.ModifyStyleEx(0, WS_EX_LAYERED);
			afxGlobalData.SetLayeredAttrib(m_Wnd.GetSafeHwnd(), params.m_clrTransparent, 0, LWA_COLORKEY);

			m_bLayered = TRUE;
		}
		else
		{
			m_Wnd.SetWindowRgn(rgnAll, FALSE);
			m_bLayered = FALSE;
		}

		m_bCreated = TRUE;
	}
}

void CSmartDockingGroupGuidesManager::Destroy()
{
	if (!m_bCreated)
	{
		return;
	}

	CSmartDockingStandaloneGuide::SDMarkerPlace i;
	for (i = CSmartDockingStandaloneGuide::sdCLEFT; i <= CSmartDockingStandaloneGuide::sdCMIDDLE; ++reinterpret_cast<int&>(i))
	{
		m_arMarkers[i - CSmartDockingStandaloneGuide::sdCLEFT].DestroyImages();
	}

	m_Wnd.DestroyWindow();

	m_rgnBase.DeleteObject();

	m_bCreated = FALSE;
}

void CSmartDockingGroupGuidesManager::Show(BOOL bShow)
{
	if (::IsWindow(m_Wnd.m_hWnd))
	{
		m_Wnd.ShowWindow(bShow ? SW_SHOW : SW_HIDE);
	}
}
void CSmartDockingGroupGuidesManager::GetWindowRect(CRect& rect)
{
	rect.SetRectEmpty();
	if (m_Wnd.GetSafeHwnd() != NULL)
	{
		m_Wnd.GetWindowRect(rect);
	}
}
BOOL CSmartDockingGroupGuidesManager::AdjustPos(CRect rcHost, int nMiddleIsOn)
{
	CRect rcWnd;
	if (m_Wnd.GetSafeHwnd() != NULL)
	{
		if (nMiddleIsOn != -1)
		{
			if (nMiddleIsOn == 0 && m_bMiddleIsOn)
			{
				m_bMiddleIsOn = FALSE;
				m_Wnd.RedrawWindow();
			}
			else
				if (nMiddleIsOn == 1 && !m_bMiddleIsOn)
				{
					m_bMiddleIsOn = TRUE;
					m_Wnd.RedrawWindow();
				}
		}

		m_Wnd.GetClientRect(rcWnd);

		int x = ((rcHost.right + rcHost.left) - rcWnd.Width()) >> 1;
		int y = ((rcHost.bottom + rcHost.top) - rcWnd.Height()) >> 1;

		CRect rcCurrentPos;
		m_Wnd.GetWindowRect(rcCurrentPos);

		if (rcCurrentPos.left != x || rcCurrentPos.top != y)
		{
			m_Wnd.SetWindowPos(&CWnd::wndTopMost, x, y, -1, -1, SWP_NOSIZE);

			return TRUE;
		}
	}

	return FALSE;
}

void CSmartDockingGroupGuidesManager::ShowGuide( CSmartDockingStandaloneGuide::SDMarkerPlace nMarkerNo, BOOL bShow/* = TRUE*/, BOOL bRedraw/* = TRUE*/)
{
	CSmartDockingGroupGuide* pMarker = GetGuide(nMarkerNo);
	if (pMarker == NULL)
	{
		return;
	}

	if (pMarker->IsVisible() != bShow)
	{
		pMarker->SetVisible(bShow, bRedraw);
	}
}

void CSmartDockingGroupGuidesManager::DrawCentralGroupGuides(CDC& dc, CBrush& brBaseBackground, CBrush& brBaseBorder)
{
	CSmartDockingInfo& params = CDockingManager::GetSmartDockingParams();

	CDC cmpdc;
	cmpdc.CreateCompatibleDC(&dc);

	// fill with the transparent color
	CRect rect;
	dc.GetBoundsRect(rect, 0);
	{
		CBrush brBack;
		brBack.CreateSolidBrush(params.m_clrTransparent);
		dc.FillRect(rect, &brBack);
	}

	dc.FillRgn(&m_rgnBase, &brBaseBackground);

	if (m_bMiddleIsOn && params.m_uiMarkerBmpResID [0] == 0) // Default images
	{
		CSmartDockingGroupGuide& centerMarker = m_arMarkers [CSmartDockingStandaloneGuide::sdCMIDDLE - CSmartDockingStandaloneGuide::sdCLEFT];

		if (centerMarker.IsVisible() && centerMarker.m_bHiLited)
		{
			CBrush br(COLOR_HIGHLIGHT_FRAME);
			dc.FrameRgn(&m_rgnBase, &br, 1, 1);
		}
		else
		{
			dc.FrameRgn(&m_rgnBase, &brBaseBorder, 1, 1);
		}
	}
	else
	{
		dc.FrameRgn(&m_rgnBase, &brBaseBorder, 1, 1);
	}

	CSmartDockingStandaloneGuide::SDMarkerPlace i;
	CSmartDockingStandaloneGuide::SDMarkerPlace last = m_bMiddleIsOn ? CSmartDockingStandaloneGuide::sdCMIDDLE : CSmartDockingStandaloneGuide::sdCBOTTOM;

	for (i = CSmartDockingStandaloneGuide::sdCLEFT; i <= last; ++reinterpret_cast<int&>(i))
	{
		CSmartDockingGroupGuide& marker = m_arMarkers[i - CSmartDockingStandaloneGuide::sdCLEFT];

		if (marker.IsVisible())
		{
			marker.Draw(dc);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSmartDockingStandaloneGuideWnd

CSmartDockingStandaloneGuideWnd::CSmartDockingStandaloneGuideWnd()
{
	m_bIsHighlighted = FALSE;
	m_bIsDefaultImage = FALSE;
	m_clrFrame = (COLORREF)-1;
	m_bIsVert = FALSE;
}

CSmartDockingStandaloneGuideWnd::~CSmartDockingStandaloneGuideWnd()
{
}

BEGIN_MESSAGE_MAP(CSmartDockingStandaloneGuideWnd, CWnd)
	//{{AFX_MSG_MAP(CSmartDockingStandaloneGuideWnd)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// if the window gets created, the region is deeded to Windows
BOOL CSmartDockingStandaloneGuideWnd::Create(LPRECT pWndRect, HBITMAP hbmpFace, HRGN hrgnShape, CWnd* pwndOwner, BOOL bIsDefaultImage, BOOL bIsVert)
{
	// save data needed
	m_hbmpFace = hbmpFace;
	m_bIsDefaultImage = bIsDefaultImage;
	m_bIsVert = bIsVert;

	// create window with specified params
	BOOL res = CreateEx(0, GetSmartDockingWndClassName<CS_OWNDC | CS_SAVEBITS>(), _T(""), WS_POPUP, *pWndRect, pwndOwner, NULL);

	// if succeeded, set the region
	if (res)
	{
		SetWindowRgn(hrgnShape, FALSE);
	}

	COLORREF clrBaseGroupBackground;
	CMFCVisualManager::GetInstance()->GetSmartDockingBaseGuideColors(clrBaseGroupBackground, m_clrFrame);

	return res;
}

void CSmartDockingStandaloneGuideWnd::Highlight(BOOL bSet)
{
	m_bIsHighlighted = bSet;

	if (GetSafeHwnd() != NULL)
	{
		RedrawWindow();
	}
}

BOOL CSmartDockingStandaloneGuideWnd::Assign(HBITMAP hbmpFace, BOOL bRedraw)
{
	if (hbmpFace != NULL)
	{
		m_hbmpFace = hbmpFace;
	}

	Invalidate(bRedraw);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CSmartDockingStandaloneGuideWnd message handlers

// simply splash the bitmap onto window's surface
void CSmartDockingStandaloneGuideWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rectClient;
	GetClientRect(rectClient);

	dc.DrawState(CPoint(0, 0), rectClient.Size(), m_hbmpFace, DSS_NORMAL);

	if (!m_bIsDefaultImage)
	{
		return;
	}

	COLORREF clrFrame = m_bIsHighlighted ? COLOR_HIGHLIGHT_FRAME : m_clrFrame;
	dc.Draw3dRect(rectClient, clrFrame, clrFrame);

	ShadeRect(&dc, rectClient, m_bIsVert);
}

void CSmartDockingStandaloneGuideWnd::OnClose()
{
	// so that it does not get destroyed
}

BOOL CSmartDockingStandaloneGuideWnd::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}



