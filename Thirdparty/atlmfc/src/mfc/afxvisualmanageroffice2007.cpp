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
#include "afxglobalutils.h"
#include "afxvisualmanageroffice2007.h"
#include "afxtoolbar.h"
#include "afxdrawmanager.h"
#include "afxpopupmenubar.h"
#include "afxmenubar.h"
#include "afxglobals.h"
#include "afxtoolbarmenubutton.h"
#include "afxcustomizebutton.h"
#include "afxmenuimages.h"
#include "afxcaptionbar.h"
#include "afxbasetabctrl.h"
#include "afxcolorbar.h"
#include "afxtabctrl.h"
#include "afxtaskspane.h"
#include "afxstatusbar.h"
#include "afxautohidebutton.h"
#include "afxheaderctrl.h"
#include "afxrebar.h"
#include "afxdesktopalertwnd.h"
#include "afxdropdowntoolbar.h"
#include "afxtagmanager.h"
#include "afxframewndex.h"
#include "afxmdiframewndex.h"
#include "afxdockablepane.h"
#include "afxoutlookbartabctrl.h"
#include "afxtoolbarcomboboxbutton.h"

#include "afxribbonbar.h"
#include "afxribbonpanel.h"
#include "afxribboncategory.h"
#include "afxribbonbutton.h"
#include "afxribbonquickaccesstoolbar.h"
#include "afxribboncombobox.h"
#include "afxribbonmainpanel.h"
#include "afxribbonpanelmenu.h"
#include "afxribbonlabel.h"
#include "afxribbonpalettegallery.h"
#include "afxribbonstatusbar.h"
#include "afxribbonstatusbarpane.h"
#include "afxribbonprogressbar.h"
#include "afxribbonlinkctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define AFX_RT_STYLE_XML _T("STYLE_XML")

const CMFCVisualManagerOffice2007::Style c_StyleDefault = CMFCVisualManagerOffice2007::Office2007_LunaBlue;
CMFCVisualManagerOffice2007::Style CMFCVisualManagerOffice2007::m_Style = c_StyleDefault;
CString CMFCVisualManagerOffice2007::m_strStylePrefix;
HINSTANCE CMFCVisualManagerOffice2007::m_hinstRes = NULL;
BOOL CMFCVisualManagerOffice2007::m_bAutoFreeRes = FALSE;

CMFCVisualManagerBitmapCache::CMFCVisualManagerBitmapCacheItem::CMFCVisualManagerBitmapCacheItem()
{
	m_bMirror = FALSE;
}

CMFCVisualManagerBitmapCache::CMFCVisualManagerBitmapCacheItem::~CMFCVisualManagerBitmapCacheItem()
{
}

void CMFCVisualManagerBitmapCache::CMFCVisualManagerBitmapCacheItem::AddImage(HBITMAP hBmp)
{
	m_Images.AddImage(hBmp, TRUE);
}

void CMFCVisualManagerBitmapCache::CMFCVisualManagerBitmapCacheItem::Cache(const CSize& size, CMFCControlRenderer& renderer)
{
	m_Images.Clear();

	const int nCount = renderer.GetImageCount();

	if (nCount > 0)
	{
		m_Images.SetImageSize(size);
		m_Images.SetTransparentColor((COLORREF)-1);

		for (int i = 0; i < nCount; i++)
		{
			BITMAPINFO bi;
			memset(&bi, 0, sizeof(BITMAPINFO));

			// Fill in the BITMAPINFOHEADER
			bi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
			bi.bmiHeader.biWidth       = size.cx;
			bi.bmiHeader.biHeight      = size.cy;
			bi.bmiHeader.biPlanes      = 1;
			bi.bmiHeader.biBitCount    = 32;
			bi.bmiHeader.biCompression = BI_RGB;
			bi.bmiHeader.biSizeImage   = size.cy * size.cx * 4;

			LPBYTE pBits = NULL;
			HBITMAP hDib = ::CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, (void **)&pBits, NULL, NULL);

			if (hDib == NULL || pBits == NULL)
			{
				ASSERT(FALSE);
				break;
			}

			CDC dc;
			dc.CreateCompatibleDC(NULL);

			HBITMAP hOldDib = (HBITMAP)::SelectObject(dc.GetSafeHdc(), hDib);

			m_bMirror = renderer.IsMirror();
			if (m_bMirror)
			{
				renderer.Mirror();
			}

			renderer.Draw(&dc, CRect(0, 0, size.cx, size.cy), i);

			if (m_bMirror)
			{
				renderer.Mirror();
			}

			::SelectObject(dc.GetSafeHdc(), hOldDib);

			AddImage(hDib);

			::DeleteObject(hDib);
		}
	}
}

void CMFCVisualManagerBitmapCache::CMFCVisualManagerBitmapCacheItem::Draw(CDC* pDC, CRect rect, int iImageIndex/* = 0*/, BYTE alphaSrc/* = 255*/)
{
	m_Images.DrawEx(pDC, CRect(rect.TopLeft(), m_Images.GetImageSize()), iImageIndex, CMFCToolBarImages::ImageAlignHorzLeft,
		CMFCToolBarImages::ImageAlignVertTop, CRect(0, 0, 0, 0), alphaSrc);
}

void CMFCVisualManagerBitmapCache::CMFCVisualManagerBitmapCacheItem::DrawY(CDC* pDC, CRect rect, CSize sides, int iImageIndex/* = 0*/, BYTE alphaSrc/* = 255*/)
{
	CRect rectImage(CPoint(0, 0), m_Images.GetImageSize());

	ASSERT(rect.Height() == rectImage.Height());

	if (sides.cx > 0)
	{
		CRect rt(rectImage);
		if (m_bMirror)
		{
			rt.left = rectImage.right - sides.cx;
			rectImage.right = rt.left;
		}
		else
		{
			rt.right = rt.left + sides.cx;
			rectImage.left = rt.right;
		}

		m_Images.DrawEx(pDC, rect, iImageIndex, CMFCToolBarImages::ImageAlignHorzLeft, CMFCToolBarImages::ImageAlignVertTop, rt, alphaSrc);
	}

	if (sides.cy > 0)
	{
		CRect rt(rectImage);
		if (m_bMirror)
		{
			rt.right = rectImage.left + sides.cy;
			rectImage.left = rt.right;
		}
		else
		{
			rt.left = rectImage.right - sides.cy;
			rectImage.right = rt.left;
		}

		m_Images.DrawEx(pDC, rect, iImageIndex, CMFCToolBarImages::ImageAlignHorzRight, CMFCToolBarImages::ImageAlignVertTop, rt, alphaSrc);
	}

	if (rectImage.Width() > 0)
	{
		rect.DeflateRect(sides.cx, 0, sides.cy, 0);
		m_Images.DrawEx(pDC, rect, iImageIndex, CMFCToolBarImages::ImageAlignHorzStretch, CMFCToolBarImages::ImageAlignVertTop, rectImage, alphaSrc);
	}
}

CMFCVisualManagerBitmapCache::CMFCVisualManagerBitmapCache()
{
}

CMFCVisualManagerBitmapCache::~CMFCVisualManagerBitmapCache()
{
	Clear();
}

void CMFCVisualManagerBitmapCache::Clear()
{
	for (int i = 0; i < m_Cache.GetSize(); i++)
	{
		if (m_Cache[i] != NULL)
		{
			delete m_Cache[i];
		}
	}

	m_Cache.RemoveAll();
	m_Sizes.RemoveAll();
}

int CMFCVisualManagerBitmapCache::Cache(const CSize& size, CMFCControlRenderer& renderer)
{
	if (FindIndex(size) != -1)
	{
		ASSERT(FALSE);
		return -1;
	}

	CMFCVisualManagerBitmapCacheItem* pItem = new CMFCVisualManagerBitmapCacheItem;
	pItem->Cache(size, renderer);

	int nCache = (int) m_Cache.Add(pItem);
	int nSize  = (int) m_Sizes.Add(size);

	ASSERT(nCache == nSize);

	return nCache;
}

int CMFCVisualManagerBitmapCache::CacheY(int height, CMFCControlRenderer& renderer)
{
	CSize size(renderer.GetParams().m_rectImage.Width(), height);

	return Cache(size, renderer);
}

BOOL CMFCVisualManagerBitmapCache::IsCached(const CSize& size) const
{
	return FindIndex(size) != -1;
}

int CMFCVisualManagerBitmapCache::FindIndex(const CSize& size) const
{
	int nRes = -1;
	for (int i = 0; i < m_Sizes.GetSize(); i++)
	{
		if (size == m_Sizes[i])
		{
			nRes = i;
			break;
		}
	}

	return nRes;
}

CMFCVisualManagerBitmapCache::CMFCVisualManagerBitmapCacheItem* CMFCVisualManagerBitmapCache::Get(const CSize& size)
{
	int nIndex = FindIndex(size);

	if (nIndex != -1)
	{
		return m_Cache[nIndex];
	}

	return NULL;
}

CMFCVisualManagerBitmapCache::CMFCVisualManagerBitmapCacheItem* CMFCVisualManagerBitmapCache::Get(int nIndex)
{
	if (0 <= nIndex && nIndex < m_Cache.GetSize())
	{
		return m_Cache[nIndex];
	}

	return NULL;
}


IMPLEMENT_DYNCREATE(CMFCVisualManagerOffice2007, CMFCVisualManagerOffice2003)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFCVisualManagerOffice2007::CMFCVisualManagerOffice2007() : m_bNcTextCenter(FALSE), m_bLoaded(FALSE)
{
	m_szNcBtnSize[0] = CSize(0, 0);
	m_szNcBtnSize[1] = CSize(0, 0);
}

CMFCVisualManagerOffice2007::~CMFCVisualManagerOffice2007()
{
	CMenuImages::SetColor(CMenuImages::ImageBlack, (COLORREF)-1);
}

CString __stdcall CMFCVisualManagerOffice2007::MakeResourceID(LPCTSTR lpszID)
{
	CString strResID(lpszID);
	ASSERT(!strResID.IsEmpty());

	if (!m_strStylePrefix.IsEmpty())
	{
		strResID = m_strStylePrefix + strResID;
	}

	return strResID;
}

CString __stdcall CMFCVisualManagerOffice2007::GetStyleResourceID(Style style)
{
	CString strResID(_T("IDX_OFFICE2007_STYLE"));
	CString strStylePrefix;

	switch(style)
	{
	case Office2007_LunaBlue:
		strStylePrefix = _T("BLUE_");
		break;

	case Office2007_ObsidianBlack:
		strStylePrefix = _T("BLACK_");
		break;

	case Office2007_Aqua:
		strStylePrefix = _T("AQUA_");
		break;

	case Office2007_Silver:
		strStylePrefix = _T("SILVER_");
		break;

	default:
		ASSERT(FALSE);
	}

	strResID = strStylePrefix + strResID;
	return strResID;
};

BOOL __stdcall CMFCVisualManagerOffice2007::SetStyle(Style style, LPCTSTR lpszPath)
{
	if (m_Style == style && m_hinstRes >(HINSTANCE) 32)
	{
		return TRUE;
	}

	UNREFERENCED_PARAMETER(lpszPath);
	CString strStyle(GetStyleResourceID(style));
	HINSTANCE hinstRes = AfxFindResourceHandle(strStyle, AFX_RT_STYLE_XML);

	if (::FindResource(hinstRes, strStyle, AFX_RT_STYLE_XML) == NULL)
	{
		TRACE(_T("Cannot load Style: %s\r\n"), strStyle);
		ASSERT(FALSE);
		return FALSE;
	}

	CleanStyle();
	m_Style = style;
	SetResourceHandle(hinstRes);
	m_bAutoFreeRes = TRUE;

	return TRUE;
}

CMFCVisualManagerOffice2007::Style __stdcall CMFCVisualManagerOffice2007::GetStyle()
{
	return m_Style;
}

void __stdcall CMFCVisualManagerOffice2007::SetResourceHandle(HINSTANCE hinstRes)
{
	m_bAutoFreeRes = FALSE;

	if (m_hinstRes != hinstRes)
	{
		m_hinstRes = hinstRes;

		if (CMFCVisualManager::GetInstance()->IsKindOf(RUNTIME_CLASS(CMFCVisualManagerOffice2007)))
		{
			CMFCVisualManager::GetInstance()->OnUpdateSystemColors();
		}
	}
}

void __stdcall CMFCVisualManagerOffice2007::CleanStyle()
{
	if (m_bAutoFreeRes && m_hinstRes >(HINSTANCE) 32)
	{
		::FreeLibrary(m_hinstRes);
	}

	m_hinstRes = NULL;
	m_Style = c_StyleDefault;
	m_strStylePrefix.Empty();
}

void CMFCVisualManagerOffice2007::CleanUp()
{
	m_clrEditBorder                = (COLORREF)(-1);
	m_clrEditBorderDisabled        = (COLORREF)(-1);
	m_clrEditBorderHighlighted     = (COLORREF)(-1);
	m_clrEditSelection             = (COLORREF)(-1);
	m_clrComboBorder               = (COLORREF)(-1);
	m_clrComboBorderDisabled       = (COLORREF)(-1);
	m_clrComboBorderPressed        = (COLORREF)(-1);
	m_clrComboBorderHighlighted    = (COLORREF)(-1);
	m_clrComboBtnStart             = (COLORREF)(-1);
	m_clrComboBtnFinish            = (COLORREF)(-1);
	m_clrComboBtnBorder            = (COLORREF)(-1);
	m_clrComboBtnDisabledStart     = (COLORREF)(-1);
	m_clrComboBtnDisabledFinish    = (COLORREF)(-1);
	m_clrComboBtnBorderDisabled    = (COLORREF)(-1);
	m_clrComboBtnPressedStart      = (COLORREF)(-1);
	m_clrComboBtnPressedFinish     = (COLORREF)(-1);
	m_clrComboBtnBorderPressed     = (COLORREF)(-1);
	m_clrComboBtnHighlightedStart  = (COLORREF)(-1);
	m_clrComboBtnHighlightedFinish = (COLORREF)(-1);
	m_clrComboBtnBorderHighlighted = (COLORREF)(-1);
	m_clrComboSelection            = (COLORREF)(-1);
	m_ctrlComboBoxBtn.CleanUp();

	m_ToolBarGripper.Clear();
	m_ToolBarTear.Clear();
	m_ctrlToolBarBorder.CleanUp();

	m_ctrlStatusBarBack.CleanUp();
	m_ctrlStatusBarBack_Ext.CleanUp();
	m_StatusBarPaneBorder.Clear();
	m_StatusBarSizeBox.Clear();

	m_SysBtnBack[0].CleanUp();
	m_SysBtnBack[1].CleanUp();
	m_SysBtnClose[0].Clear();
	m_SysBtnClose[1].Clear();
	m_SysBtnRestore[0].Clear();
	m_SysBtnRestore[1].Clear();
	m_SysBtnMaximize[0].Clear();
	m_SysBtnMaximize[1].Clear();
	m_SysBtnMinimize[0].Clear();
	m_SysBtnMinimize[1].Clear();

	m_brMainClientArea.DeleteObject();

	m_AppCaptionFont.DeleteObject();
	m_penSeparator2.DeleteObject();

	m_brGroupBackground.DeleteObject();
	m_clrGroupText = (COLORREF)-1;

	m_penSeparatorDark.DeleteObject();

	m_ctrlMainBorder.CleanUp();
	m_ctrlMDIChildBorder.CleanUp();
	m_ctrlMainBorderCaption.CleanUp();
	m_ctrlPopupBorder.CleanUp();
	m_ctrlPopupResizeBar.CleanUp();
	m_PopupResizeBar_HV.Clear();
	m_PopupResizeBar_HVT.Clear();
	m_PopupResizeBar_V.Clear();

	m_ctrlMenuBarBtn.CleanUp();

	m_ctrlMenuItemBack.CleanUp();
	m_MenuItemMarkerC.Clear();
	m_MenuItemMarkerR.Clear();
	m_ctrlMenuItemShowAll.CleanUp();
	m_ctrlMenuHighlighted[0].CleanUp();
	m_ctrlMenuHighlighted[1].CleanUp();
	m_ctrlMenuButtonBorder.CleanUp();
	m_ctrlMenuScrollBtn[0].CleanUp();
	m_ctrlMenuScrollBtn[1].CleanUp();

	m_ctrlToolBarBtn.CleanUp();

	m_ctrlTaskScrollBtn.CleanUp();

	m_ctrlTab3D[0].CleanUp();
	m_ctrlTab3D[1].CleanUp();
	m_ctrlTabFlat[0].CleanUp();
	m_ctrlTabFlat[1].CleanUp();
	m_clrTabFlatBlack = CLR_DEFAULT;
	m_clrTabTextActive = CLR_DEFAULT;
	m_clrTabTextInactive = CLR_DEFAULT;
	m_clrTabFlatHighlight = CLR_DEFAULT;
	m_penTabFlatInner[0].DeleteObject();
	m_penTabFlatInner[1].DeleteObject();
	m_penTabFlatOuter[0].DeleteObject();
	m_penTabFlatOuter[1].DeleteObject();

	m_ctrlOutlookWndBar.CleanUp();
	m_ctrlOutlookWndPageBtn.CleanUp();

	m_ctrlRibbonCaptionQA.CleanUp();
	m_ctrlRibbonCaptionQA_Glass.CleanUp();
	m_ctrlRibbonCategoryBack.CleanUp();
	m_ctrlRibbonCategoryTab.CleanUp();
	m_ctrlRibbonCategoryTabSep.CleanUp();
	m_ctrlRibbonCategoryBtnPage[0].CleanUp();
	m_ctrlRibbonCategoryBtnPage[1].CleanUp();
	m_ctrlRibbonPanelBack_T.CleanUp();
	m_ctrlRibbonPanelBack_B.CleanUp();
	m_RibbonPanelSeparator.Clear();
	m_ctrlRibbonPanelQAT.CleanUp();
	m_ctrlRibbonMainPanel.CleanUp();
	m_ctrlRibbonMainPanelBorder.CleanUp();
	m_ctrlRibbonBtnMainPanel.CleanUp();

	m_ctrlRibbonBtnGroup_S.CleanUp();
	m_ctrlRibbonBtnGroup_F.CleanUp();
	m_ctrlRibbonBtnGroup_M.CleanUp();
	m_ctrlRibbonBtnGroup_L.CleanUp();
	m_ctrlRibbonBtnGroupMenu_F[0].CleanUp();
	m_ctrlRibbonBtnGroupMenu_F[1].CleanUp();
	m_ctrlRibbonBtnGroupMenu_M[0].CleanUp();
	m_ctrlRibbonBtnGroupMenu_M[1].CleanUp();
	m_ctrlRibbonBtnGroupMenu_L[0].CleanUp();
	m_ctrlRibbonBtnGroupMenu_L[1].CleanUp();
	m_ctrlRibbonBtn[0].CleanUp();
	m_ctrlRibbonBtn[1].CleanUp();
	m_ctrlRibbonBtnMenuH[0].CleanUp();
	m_ctrlRibbonBtnMenuH[1].CleanUp();
	m_ctrlRibbonBtnMenuV[0].CleanUp();
	m_ctrlRibbonBtnMenuV[1].CleanUp();
	m_ctrlRibbonBtnLaunch.CleanUp();
	m_RibbonBtnLaunchIcon.Clear();
	m_RibbonBtnMain.CleanUp();
	m_ctrlRibbonBtnDefault.CleanUp();
	m_ctrlRibbonBtnDefaultIcon.CleanUp();
	m_RibbonBtnDefaultImage.Clear();
	m_ctrlRibbonBtnDefaultQATIcon.CleanUp();
	m_ctrlRibbonBtnDefaultQAT.CleanUp();
	m_ctrlRibbonBtnCheck.CleanUp();
	m_ctrlRibbonBtnPalette[0].CleanUp();
	m_ctrlRibbonBtnPalette[1].CleanUp();
	m_ctrlRibbonBtnPalette[2].CleanUp();
	m_ctrlRibbonBtnStatusPane.CleanUp();
	m_ctrlRibbonSliderThumb.CleanUp();
	m_ctrlRibbonSliderBtnPlus.CleanUp();
	m_ctrlRibbonSliderBtnMinus.CleanUp();
	m_ctrlRibbonProgressBack.CleanUp();
	m_ctrlRibbonProgressNormal.CleanUp();
	m_ctrlRibbonProgressNormalExt.CleanUp();
	m_ctrlRibbonProgressInfinity.CleanUp();
	m_ctrlRibbonBorder_QAT.CleanUp();
	m_ctrlRibbonBorder_Floaty.CleanUp();

	m_ctrlRibbonKeyTip.CleanUp();
	m_clrRibbonKeyTipTextNormal   = (COLORREF)(-1);
	m_clrRibbonKeyTipTextDisabled = (COLORREF)(-1);

	m_ctrlRibbonComboBoxBtn.CleanUp();

	m_cacheRibbonCategoryBack.Clear();
	m_cacheRibbonPanelBack_T.Clear();
	m_cacheRibbonPanelBack_B.Clear();
	m_cacheRibbonBtnDefault.Clear();

	m_cacheRibbonBtnGroup_S.Clear();
	m_cacheRibbonBtnGroup_F.Clear();
	m_cacheRibbonBtnGroup_M.Clear();
	m_cacheRibbonBtnGroup_L.Clear();
	m_cacheRibbonBtnGroupMenu_F[0].Clear();
	m_cacheRibbonBtnGroupMenu_M[0].Clear();
	m_cacheRibbonBtnGroupMenu_L[0].Clear();
	m_cacheRibbonBtnGroupMenu_F[1].Clear();
	m_cacheRibbonBtnGroupMenu_M[1].Clear();
	m_cacheRibbonBtnGroupMenu_L[1].Clear();

	m_ctrlRibbonContextPanelBack_T.CleanUp();
	m_ctrlRibbonContextPanelBack_B.CleanUp();
	m_cacheRibbonContextPanelBack_T.Clear();
	m_cacheRibbonContextPanelBack_B.Clear();
	m_ctrlRibbonContextSeparator.CleanUp();

	for (int i = 0; i < AFX_RIBBON_CATEGORY_COLOR_COUNT; i++)
	{
		m_ctrlRibbonContextCategory[i].CleanUp();
	}

	m_clrCaptionBarText = afxGlobalData.clrWindow;

	m_bToolTipParams = FALSE;
	CMFCToolTipInfo dummy;
	m_ToolTipParams = dummy;

	m_ActivateFlag.RemoveAll();

	m_clrRibbonHyperlinkInactive = (COLORREF)-1;
	m_clrRibbonHyperlinkActive = (COLORREF)-1;
	m_clrRibbonStatusbarHyperlinkInactive = (COLORREF)-1;
	m_clrRibbonStatusbarHyperlinkActive = (COLORREF)-1;

	m_bLoaded = FALSE;
}

void CMFCVisualManagerOffice2007::OnUpdateSystemColors()
{
	CleanUp();

	CMFCVisualManagerOffice2003::OnUpdateSystemColors();

	if (!afxGlobalData.bIsOSAlphaBlendingSupport || afxGlobalData.IsHighContrastMode() || afxGlobalData.m_nBitsPerPixel <= 8)
	{
		return;
	}

	m_nMenuBorderSize = 1;

	HINSTANCE hinstResOld = NULL;

	if (m_hinstRes == NULL)
	{
		SetStyle(c_StyleDefault);
	}

	if (m_hinstRes != NULL)
	{
		hinstResOld = AfxGetResourceHandle();
		AfxSetResourceHandle(m_hinstRes);
	}

	CTagManager tm;

	if (!tm.LoadFromResource(GetStyleResourceID(m_Style), AFX_RT_STYLE_XML))
	{
#if !defined _AFXDLL
		TRACE(_T("\r\nImportant: to enable the Office 2007 look in static link,\r\n"));
		TRACE(_T("include afxribbon.rc from the RC file in your project.\r\n\r\n"));
		ASSERT(FALSE);
#endif
		if (hinstResOld != NULL)
		{
			AfxSetResourceHandle(hinstResOld);
		}

		return;
	}

	{
		CString strStyle;
		tm.ExcludeTag(_T("STYLE"), strStyle);
		tm.SetBuffer(strStyle);
	}

	CString strItem;

	m_nType = 20;

	if (!tm.IsEmpty())
	{
		int nVersion = 0;

		if (tm.ExcludeTag(_T("VERSION"), strItem))
		{
			CTagManager tmItem(strItem);

			tmItem.ReadInt(_T("NUMBER"), nVersion);

			if (nVersion == 2007)
			{
				tmItem.ReadInt(_T("TYPE"), m_nType);

				if (m_nType < 10)
				{
					m_nType *= 10;
				}

				m_bLoaded = TRUE;
			}

			if (m_bLoaded)
			{
				if (tmItem.ExcludeTag(_T("ID_PREFIX"), strItem))
				{
					strItem.Trim();
					m_strStylePrefix = strItem;
				}
			}
		}
	}

	if (!m_bLoaded)
	{
		if (hinstResOld != NULL)
		{
			::AfxSetResourceHandle(hinstResOld);
		}

		return;
	}

	// globals
	if (tm.ExcludeTag(_T("GLOBALS"), strItem))
	{
		CTagManager tmItem(strItem);

		tmItem.ReadColor(_T("BarText"), afxGlobalData.clrBarText);

		if (tmItem.ReadColor(_T("BarFace"), afxGlobalData.clrBarFace))
		{
			afxGlobalData.brBarFace.DeleteObject();
			afxGlobalData.brBarFace.CreateSolidBrush(afxGlobalData.clrBarFace);
			m_clrMenuShadowBase = afxGlobalData.clrBarFace;
		}
		if (tmItem.ReadColor(_T("ActiveCaption"), afxGlobalData.clrActiveCaption))
		{
			afxGlobalData.clrInactiveCaption     = afxGlobalData.clrActiveCaption;
			afxGlobalData.brActiveCaption.DeleteObject();
			afxGlobalData.brActiveCaption.CreateSolidBrush(afxGlobalData.clrActiveCaption);
		}
		if (tmItem.ReadColor(_T("CaptionText"), afxGlobalData.clrCaptionText))
		{
			afxGlobalData.clrInactiveCaptionText = afxGlobalData.clrCaptionText;
		}

		tmItem.ReadColor(_T("InactiveCaption"), afxGlobalData.clrInactiveCaption);
		afxGlobalData.brInactiveCaption.DeleteObject();
		afxGlobalData.brInactiveCaption.CreateSolidBrush(afxGlobalData.clrInactiveCaption);
		tmItem.ReadColor(_T("InactiveCaptionText"), afxGlobalData.clrInactiveCaptionText);

		tmItem.ReadColor(_T("BarShadow"), afxGlobalData.clrBarShadow);
		tmItem.ReadColor(_T("BarDkShadow"), afxGlobalData.clrBarDkShadow);
		tmItem.ReadColor(_T("BarLight"), afxGlobalData.clrBarLight);

		COLORREF clrFloatToolBarBorder;
		tmItem.ReadColor(_T("FloatToolBarBorder"), clrFloatToolBarBorder);
		m_brFloatToolBarBorder.DeleteObject();
		m_brFloatToolBarBorder.CreateSolidBrush(clrFloatToolBarBorder);

		tmItem.ReadColor(_T("HighlightGradientDark"), m_clrHighlightGradientDark);
		tmItem.ReadColor(_T("HighlightGradientLight"), m_clrHighlightGradientLight);

		m_clrHighlightDnGradientDark = m_clrHighlightGradientLight;
		m_clrHighlightDnGradientLight = m_clrHighlightGradientDark;
		tmItem.ReadColor(_T("HighlightDnGradientDark"), m_clrHighlightDnGradientDark);
		tmItem.ReadColor(_T("HighlightDnGradientLight"), m_clrHighlightDnGradientLight);

		m_clrHighlightCheckedGradientDark = m_clrHighlightDnGradientLight;
		m_clrHighlightCheckedGradientLight = m_clrHighlightDnGradientDark;
		tmItem.ReadColor(_T("HighlightCheckedGradientDark"), m_clrHighlightCheckedGradientDark);
		tmItem.ReadColor(_T("HighlightCheckedGradientLight"), m_clrHighlightCheckedGradientLight);

		tmItem.ReadColor(_T("PressedButtonBorder"), m_clrPressedButtonBorder);

		COLORREF clrHB = afxGlobalData.clrHilite;
		COLORREF clrHT = afxGlobalData.clrTextHilite;
		if (tmItem.ReadColor(_T("Highlight"), clrHB) && tmItem.ReadColor(_T("HighlightText"), clrHT))
		{
			afxGlobalData.clrHilite = clrHB;

			afxGlobalData.brHilite.DeleteObject();
			afxGlobalData.brHilite.CreateSolidBrush(clrHB);

			afxGlobalData.clrTextHilite = clrHT;
		}

		tmItem.ReadColor(_T("MenuShadowColor"), m_clrMenuShadowBase);

		// ToolTipParams
		m_bToolTipParams = tmItem.ReadToolTipInfo(_T("TOOLTIP"), m_ToolTipParams);
	}

	// mainwnd
	if (tm.ExcludeTag(_T("MAINWND"), strItem))
	{
		CTagManager tmItem(strItem);

		// caption
		CString strCaption;
		if (tmItem.ExcludeTag(_T("CAPTION"), strCaption))
		{
			CTagManager tmCaption(strCaption);

			NONCLIENTMETRICS ncm;
			if (afxGlobalData.GetNonClientMetrics (ncm))
			{
				tmCaption.ReadFont(_T("FONT"), ncm.lfCaptionFont);
				m_AppCaptionFont.DeleteObject();
				m_AppCaptionFont.CreateFontIndirect(&ncm.lfCaptionFont);
			}

			tmCaption.ReadColor(_T("ActiveStart"), m_clrAppCaptionActiveStart);
			tmCaption.ReadColor(_T("ActiveFinish"), m_clrAppCaptionActiveFinish);
			tmCaption.ReadColor(_T("InactiveStart"), m_clrAppCaptionInactiveStart);
			tmCaption.ReadColor(_T("InactiveFinish"), m_clrAppCaptionInactiveFinish);
			tmCaption.ReadColor(_T("ActiveText"), m_clrAppCaptionActiveText);
			tmCaption.ReadColor(_T("InactiveText"), m_clrAppCaptionInactiveText);
			tmCaption.ReadColor(_T("ActiveTitleText"), m_clrAppCaptionActiveTitleText);
			tmCaption.ReadColor(_T("InactiveTitleText"), m_clrAppCaptionInactiveTitleText);

			tmCaption.ReadBool(_T("TextCenter"), m_bNcTextCenter);

			tmCaption.ReadControlRenderer(_T("BORDER"), m_ctrlMainBorderCaption, MakeResourceID(_T("IDB_OFFICE2007_MAINBORDER_CAPTION")));

			m_szNcBtnSize[0] = CSize(::GetSystemMetrics(SM_CXSIZE), ::GetSystemMetrics(SM_CYSIZE));
			m_szNcBtnSize[1] = CSize(::GetSystemMetrics(SM_CXSMSIZE), ::GetSystemMetrics(SM_CYSMSIZE));

			// buttons
			CString strButtons;
			if (tmCaption.ExcludeTag(_T("BUTTONS"), strButtons))
			{
				CTagManager tmButtons(strButtons);

				for (int i = 0; i < 2; i++)
				{
					CString str;
					CString suffix;
					if (i == 1)
					{
						suffix = _T("_S");
					}
					if (tmButtons.ExcludeTag(i == 0 ? _T("NORMAL") : _T("SMALL"), str))
					{
						CTagManager tmBtn(str);

						tmBtn.ReadSize(_T("ConstSize"), m_szNcBtnSize[i]);

						CSize sizeIcon(0, 0);
						if (tmBtn.ReadSize(_T("IconSize"), sizeIcon))
						{
							m_SysBtnClose[i].Clear();
							m_SysBtnClose[i].SetPreMultiplyAutoCheck(TRUE);
							m_SysBtnClose[i].SetImageSize(sizeIcon);
							m_SysBtnClose[i].LoadStr(MakeResourceID(_T("IDB_OFFICE2007_SYS_BTN_CLOSE") + suffix));

							m_SysBtnRestore[i].Clear();
							m_SysBtnRestore[i].SetPreMultiplyAutoCheck(TRUE);
							m_SysBtnRestore[i].SetImageSize(sizeIcon);
							m_SysBtnRestore[i].LoadStr(MakeResourceID(_T("IDB_OFFICE2007_SYS_BTN_RESTORE") + suffix));

							m_SysBtnMaximize[i].Clear();
							m_SysBtnMaximize[i].SetPreMultiplyAutoCheck(TRUE);
							m_SysBtnMaximize[i].SetImageSize(sizeIcon);
							m_SysBtnMaximize[i].LoadStr(MakeResourceID(_T("IDB_OFFICE2007_SYS_BTN_MAXIMIZE") + suffix));

							m_SysBtnMinimize[i].Clear();
							m_SysBtnMinimize[i].SetPreMultiplyAutoCheck(TRUE);
							m_SysBtnMinimize[i].SetImageSize(sizeIcon);
							m_SysBtnMinimize[i].LoadStr(MakeResourceID(_T("IDB_OFFICE2007_SYS_BTN_MINIMIZE") + suffix));
						}

						CTagManager::ParseControlRenderer(tmBtn.GetBuffer(), m_SysBtnBack[i], MakeResourceID(_T("IDB_OFFICE2007_SYS_BTN_BACK")));
					}
				}
			}
		}

		// border
		tmItem.ReadControlRenderer(_T("BORDER"), m_ctrlMainBorder, MakeResourceID(_T("IDB_OFFICE2007_MAINBORDER")));
		tmItem.ReadControlRenderer(_T("BORDER_MDICHILD"), m_ctrlMDIChildBorder, MakeResourceID(_T("IDB_OFFICE2007_MDICHILDBORDER")));

		if (tmItem.ReadColor(_T("MainClientArea"), m_clrMainClientArea))
		{
			m_brMainClientArea.DeleteObject();
			m_brMainClientArea.CreateSolidBrush(m_clrMainClientArea);
		}
	}

	// menu
	if (tm.ExcludeTag(_T("MENU"), strItem))
	{
		CTagManager tmItem(strItem);

		if (tmItem.ReadColor(_T("Light"), m_clrMenuLight))
		{
			m_brMenuLight.DeleteObject();
			m_brMenuLight.CreateSolidBrush(m_clrMenuLight);
		}

		m_clrMenuRarelyUsed = CLR_DEFAULT;
		tmItem.ReadColor(_T("Rarely"), m_clrMenuRarelyUsed);

		tmItem.ReadColor(_T("Border"), m_clrMenuBorder);

		if (tmItem.ReadColor(_T("Separator1"), m_clrSeparator1))
		{
			m_penSeparator.DeleteObject();
			m_penSeparator.CreatePen(PS_SOLID, 1, m_clrSeparator1);
		}

		if (tmItem.ReadColor(_T("Separator2"), m_clrSeparator2))
		{
			m_penSeparator2.DeleteObject();
			m_penSeparator2.CreatePen(PS_SOLID, 1, m_clrSeparator2);
		}

		COLORREF clrGroupBack = (COLORREF)-1;
		if (tmItem.ReadColor(_T("GroupBackground"), clrGroupBack))
		{
			m_brGroupBackground.DeleteObject();
			m_brGroupBackground.CreateSolidBrush(clrGroupBack);
		}

		tmItem.ReadColor(_T("GroupText"), m_clrGroupText);

		if (tmItem.ReadColor(_T("ItemBorder"), m_clrMenuItemBorder))
		{
			m_penMenuItemBorder.DeleteObject();
			m_penMenuItemBorder.CreatePen(PS_SOLID, 1, m_clrMenuItemBorder);
		}

		tmItem.ReadInt(_T("BorderSize"), m_nMenuBorderSize);

		tmItem.ReadControlRenderer(_T("ItemBack"), m_ctrlMenuItemBack, MakeResourceID(_T("IDB_OFFICE2007_MENU_ITEM_BACK")));
		tmItem.ReadToolBarImages(_T("ItemCheck"), m_MenuItemMarkerC, MakeResourceID(_T("IDB_OFFICE2007_MENU_ITEM_MARKER_C")));
		tmItem.ReadToolBarImages(_T("ItemRadio"), m_MenuItemMarkerR, MakeResourceID(_T("IDB_OFFICE2007_MENU_ITEM_MARKER_R")));
		tmItem.ReadControlRenderer(_T("ItemShowAll"), m_ctrlMenuItemShowAll, MakeResourceID(_T("IDB_OFFICE2007_MENU_ITEM_SHOWALL")));
		tmItem.ReadControlRenderer(_T("Highlighted"), m_ctrlMenuHighlighted[0], MakeResourceID(_T("IDB_OFFICE2007_MENU_BTN")));
		tmItem.ReadControlRenderer(_T("HighlightedDisabled"), m_ctrlMenuHighlighted[1], MakeResourceID(_T("IDB_OFFICE2007_MENU_BTN_DISABLED")));
		tmItem.ReadControlRenderer(_T("ButtonBorder"), m_ctrlMenuButtonBorder, MakeResourceID(_T("IDB_OFFICE2007_MENU_BTN_VERT_SEPARATOR")));
		tmItem.ReadControlRenderer(_T("ScrollBtn_T"), m_ctrlMenuScrollBtn[0], MakeResourceID(_T("IDB_OFFICE2007_MENU_BTN_SCROLL_T")));
		tmItem.ReadControlRenderer(_T("ScrollBtn_B"), m_ctrlMenuScrollBtn[1], MakeResourceID(_T("IDB_OFFICE2007_MENU_BTN_SCROLL_B")));

		tmItem.ReadColor(_T("TextNormal"), m_clrMenuText);
		tmItem.ReadColor(_T("TextHighlighted"), m_clrMenuTextHighlighted);
		tmItem.ReadColor(_T("TextDisabled"), m_clrMenuTextDisabled);

		COLORREF clrImages = m_clrMenuText;

		CString strColors;
		if (tmItem.ExcludeTag(_T("COLORS"), strColors))
		{
			CTagManager tmColors(strColors);

			tmColors.ReadColor(_T("Black"), clrImages);
			CMenuImages::SetColor(CMenuImages::ImageBlack, clrImages);

			tmColors.ReadColor(_T("Black2"), clrImages);
			CMenuImages::SetColor(CMenuImages::ImageBlack2, clrImages);

			struct XColors
			{
				CMenuImages::IMAGE_STATE state;
				LPCTSTR name;
			};
			XColors colors[4] =
			{
				{CMenuImages::ImageGray, _T("Gray")},
				{CMenuImages::ImageLtGray, _T("LtGray")},
				{CMenuImages::ImageWhite, _T("White")},
				{CMenuImages::ImageDkGray, _T("DkGray")}
			};

			for (int ic = 0; ic < 4; ic++)
			{
				if (tmColors.ReadColor(colors[ic].name, clrImages))
				{
					CMenuImages::SetColor(colors[ic].state, clrImages);
				}
			}
		}
		else
		{
			tmItem.ReadColor(_T("ImagesColor"), clrImages);
			CMenuImages::SetColor(CMenuImages::ImageBlack, clrImages);
			CMenuImages::SetColor(CMenuImages::ImageBlack2, clrImages);
		}
	}

	// bars
	if (tm.ExcludeTag(_T("BARS"), strItem))
	{
		CTagManager tmItem(strItem);

		CString strBar;
		if (tmItem.ExcludeTag(_T("DEFAULT"), strBar))
		{
			CTagManager tmBar(strBar);

			if (tmBar.ReadColor(_T("Bkgnd"), m_clrBarBkgnd))
			{
				m_brBarBkgnd.DeleteObject();
				m_brBarBkgnd.CreateSolidBrush(m_clrBarBkgnd);
			}

			tmBar.ReadColor(_T("GradientLight"), m_clrBarGradientLight);
			m_clrBarGradientDark = m_clrBarGradientLight;
			tmBar.ReadColor(_T("GradientDark"), m_clrBarGradientDark);
		}

		if (tmItem.ExcludeTag(_T("TOOLBAR"), strBar))
		{
			CTagManager tmBar(strBar);

			m_clrToolBarGradientLight = m_clrBarGradientLight;
			m_clrToolBarGradientDark  = m_clrBarGradientDark;

			m_clrToolbarDisabled = CDrawingManager::SmartMixColors(m_clrToolBarGradientDark, m_clrToolBarGradientLight);

			tmBar.ReadColor(_T("GradientLight"), m_clrToolBarGradientLight);
			tmBar.ReadColor(_T("GradientDark"), m_clrToolBarGradientDark);

			m_clrToolBarGradientVertLight = m_clrToolBarGradientLight;
			m_clrToolBarGradientVertDark  = m_clrToolBarGradientDark;

			tmBar.ReadColor(_T("GradientVertLight"), m_clrToolBarGradientVertLight);
			tmBar.ReadColor(_T("GradientVertDark"), m_clrToolBarGradientVertDark);

			tmBar.ReadColor(_T("CustomizeButtonGradientLight"), m_clrCustomizeButtonGradientLight);
			tmBar.ReadColor(_T("CustomizeButtonGradientDark"), m_clrCustomizeButtonGradientDark);

			tmBar.ReadToolBarImages(_T("GRIPPER"), m_ToolBarGripper, MakeResourceID(_T("IDB_OFFICE2007_GRIPPER")));
			tmBar.ReadToolBarImages(_T("TEAR"), m_ToolBarTear, MakeResourceID(_T("IDB_OFFICE2007_TEAR")));

			tmBar.ReadControlRenderer(_T("BUTTON"), m_ctrlToolBarBtn, MakeResourceID(_T("IDB_OFFICE2007_TOOLBAR_BTN")));
			tmBar.ReadControlRenderer(_T("BORDER"), m_ctrlToolBarBorder, MakeResourceID(_T("IDB_OFFICE2007_TOOLBAR_BORDER")));

			m_clrToolBarBtnText = afxGlobalData.clrBarText;
			m_clrToolBarBtnTextHighlighted = m_clrToolBarBtnText;
			tmBar.ReadColor(_T("TextNormal"), m_clrToolBarBtnText);
			tmBar.ReadColor(_T("TextHighlighted"), m_clrToolBarBtnTextHighlighted);
			tmBar.ReadColor(_T("TextDisabled"), m_clrToolBarBtnTextDisabled);

			if (tmBar.ReadColor(_T("BottomLineColor"), m_clrToolBarBottomLine))
			{
				m_penBottomLine.DeleteObject();
				m_penBottomLine.CreatePen(PS_SOLID, 1, m_clrToolBarBottomLine);
			}

			m_penSeparatorDark.DeleteObject();
			m_penSeparatorDark.CreatePen(PS_SOLID, 1, CDrawingManager::PixelAlpha(m_clrToolBarBottomLine, RGB(255, 255, 255), 95));

			m_penSeparatorLight.DeleteObject();
			m_penSeparatorLight.CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
		}

		if (tmItem.ExcludeTag(_T("MENUBAR"), strBar))
		{
			CTagManager tmBar(strBar);

			m_clrMenuBarGradientLight = m_clrToolBarGradientLight;
			m_clrMenuBarGradientDark  = m_clrToolBarGradientDark;

			tmBar.ReadColor(_T("GradientLight"), m_clrMenuBarGradientLight);
			tmBar.ReadColor(_T("GradientDark"), m_clrMenuBarGradientDark);

			m_clrMenuBarGradientVertLight = m_clrMenuBarGradientLight;
			m_clrMenuBarGradientVertDark  = m_clrMenuBarGradientDark;

			tmBar.ReadColor(_T("GradientVertLight"), m_clrMenuBarGradientVertLight);
			tmBar.ReadColor(_T("GradientVertDark"), m_clrMenuBarGradientVertDark);

			m_clrMenuBarBtnText            = m_clrToolBarBtnText;
			m_clrMenuBarBtnTextHighlighted = m_clrToolBarBtnTextHighlighted;
			m_clrMenuBarBtnTextDisabled    = m_clrToolBarBtnTextDisabled;
			tmBar.ReadColor(_T("TextNormal"), m_clrMenuBarBtnText);
			tmBar.ReadColor(_T("TextHighlighted"), m_clrMenuBarBtnTextHighlighted);
			tmBar.ReadColor(_T("TextDisabled"), m_clrMenuBarBtnTextDisabled);

			tmBar.ReadControlRenderer(_T("BUTTON"), m_ctrlMenuBarBtn, MakeResourceID(_T("IDB_OFFICE2007_MENUBAR_BTN")));
		}

		if (tmItem.ExcludeTag(_T("POPUPBAR"), strBar))
		{
			CTagManager tmBar(strBar);
			tmBar.ReadControlRenderer(_T("BORDER"), m_ctrlPopupBorder, MakeResourceID(_T("IDB_OFFICE2007_POPUPMENU_BORDER")));

			CString strResize;
			if (tmBar.ExcludeTag(_T("RESIZEBAR"), strResize))
			{
				CTagManager tmResize(strResize);
				tmResize.ReadControlRenderer(_T("BACK"), m_ctrlPopupResizeBar, MakeResourceID(_T("IDB_OFFICE2007_POPUPMENU_RESIZEBAR")));
				tmResize.ReadToolBarImages(_T("ICON_HV"), m_PopupResizeBar_HV, MakeResourceID(_T("IDB_OFFICE2007_POPUPMENU_RESIZEBAR_ICON_HV")));
				tmResize.ReadToolBarImages(_T("ICON_HVT"), m_PopupResizeBar_HVT, MakeResourceID(_T("IDB_OFFICE2007_POPUPMENU_RESIZEBAR_ICON_HVT")));
				tmResize.ReadToolBarImages(_T("ICON_V"), m_PopupResizeBar_V, MakeResourceID(_T("IDB_OFFICE2007_POPUPMENU_RESIZEBAR_ICON_V")));
			}
		}

		if (tmItem.ExcludeTag(_T("STATUSBAR"), strBar))
		{
			CTagManager tmBar(strBar);

			tmBar.ReadControlRenderer(_T("BACK"), m_ctrlStatusBarBack, MakeResourceID(_T("IDB_OFFICE2007_STATUSBAR_BACK")));
			tmBar.ReadControlRenderer(_T("BACK_EXT"), m_ctrlStatusBarBack_Ext, MakeResourceID(_T("IDB_OFFICE2007_STATUSBAR_BACK_EXT")));

			tmBar.ReadToolBarImages(_T("PANEBORDER"), m_StatusBarPaneBorder, MakeResourceID(_T("IDB_OFFICE2007_STATUSBAR_PANEBORDER")));
			tmBar.ReadToolBarImages(_T("SIZEBOX"), m_StatusBarSizeBox, MakeResourceID(_T("IDB_OFFICE2007_STATUSBAR_SIZEBOX")));

			m_clrStatusBarText = m_clrMenuBarBtnText;
			m_clrStatusBarTextDisabled = m_clrMenuBarBtnTextDisabled;
			m_clrExtenedStatusBarTextDisabled = m_clrMenuBarBtnTextDisabled;

			tmBar.ReadColor(_T("TextNormal"), m_clrStatusBarText);
			tmBar.ReadColor(_T("TextDisabled"), m_clrStatusBarTextDisabled);
			tmBar.ReadColor(_T("TextExtendedDisabled"), m_clrExtenedStatusBarTextDisabled);
		}

		if (tmItem.ExcludeTag(_T("CAPTIONBAR"), strBar))
		{
			CTagManager tmBar(strBar);

			tmBar.ReadColor(_T("GradientLight"), m_clrCaptionBarGradientLight);
			tmBar.ReadColor(_T("GradientDark"), m_clrCaptionBarGradientDark);
			tmBar.ReadColor(_T("TextNormal"), m_clrCaptionBarText);
		}
	}

	if (m_clrMenuRarelyUsed == CLR_DEFAULT)
	{
		m_clrMenuRarelyUsed = m_clrBarBkgnd;
	}

	m_brMenuRarelyUsed.DeleteObject();
	m_brMenuRarelyUsed.CreateSolidBrush(m_clrMenuRarelyUsed);

	m_clrEditBorder            = afxGlobalData.clrWindow;
	m_clrEditBorderDisabled    = afxGlobalData.clrBtnShadow;
	m_clrEditBorderHighlighted = m_clrMenuItemBorder;
	m_clrEditSelection         = afxGlobalData.clrHilite;

	// edit
	if (tm.ExcludeTag(_T("EDIT"), strItem))
	{
		CTagManager tmItem(strItem);

		tmItem.ReadColor(_T("BorderNormal"), m_clrEditBorder);
		tmItem.ReadColor(_T("BorderHighlighted"), m_clrEditBorderHighlighted);
		tmItem.ReadColor(_T("BorderDisabled"), m_clrEditBorderDisabled);
		tmItem.ReadColor(_T("Selection"), m_clrEditSelection);
	}

	m_clrComboBorder               = afxGlobalData.clrWindow;
	m_clrComboBorderDisabled       = afxGlobalData.clrBtnShadow;
	m_clrComboBorderHighlighted    = m_clrMenuItemBorder;
	m_clrComboBorderPressed        = m_clrComboBorderHighlighted;
	m_clrComboBtnBorder            = m_clrComboBorder;
	m_clrComboBtnBorderHighlighted = m_clrComboBorderHighlighted;
	m_clrComboBtnBorderPressed     = m_clrComboBorderHighlighted;
	m_clrComboSelection            = afxGlobalData.clrHilite;
	m_clrComboBtnStart             = m_clrToolBarGradientDark;
	m_clrComboBtnFinish            = m_clrToolBarGradientLight;
	m_clrComboBtnDisabledStart     = afxGlobalData.clrBtnFace;
	m_clrComboBtnDisabledFinish    = m_clrComboBtnDisabledStart;
	m_clrComboBtnHighlightedStart  = m_clrHighlightGradientDark;
	m_clrComboBtnHighlightedFinish = m_clrHighlightGradientLight;
	m_clrComboBtnPressedStart      = m_clrHighlightDnGradientDark;
	m_clrComboBtnPressedFinish     = m_clrHighlightDnGradientLight;

	// combobox
	if (tm.ExcludeTag(_T("COMBO"), strItem))
	{
		CTagManager tmItem(strItem);

		tmItem.ReadColor(_T("BorderNormal"), m_clrComboBorder);
		tmItem.ReadColor(_T("BorderHighlighted"), m_clrComboBorderHighlighted);
		tmItem.ReadColor(_T("BorderDisabled"), m_clrComboBorderDisabled);

		m_clrComboBorderPressed = m_clrComboBorderHighlighted;
		tmItem.ReadColor(_T("BorderPressed"), m_clrComboBorderPressed);

		tmItem.ReadColor(_T("Selection"), m_clrComboSelection);

		CString strButton;
		if (tmItem.ExcludeTag(_T("BUTTON"), strButton))
		{
			CTagManager tmButton(strButton);

			tmButton.ReadColor(_T("GradientStartNormal"), m_clrComboBtnStart);
			tmButton.ReadColor(_T("GradientFinishNormal"), m_clrComboBtnFinish);
			tmButton.ReadColor(_T("BtnBorderNormal"), m_clrComboBtnBorder);

			if (!tmButton.ReadControlRenderer(_T("IMAGE"), m_ctrlComboBoxBtn, MakeResourceID(_T("IDB_OFFICE2007_COMBOBOX_BTN"))))
			{
				tmButton.ReadColor(_T("GradientStartHighlighted"), m_clrComboBtnHighlightedStart);
				tmButton.ReadColor(_T("GradientFinishHighlighted"), m_clrComboBtnHighlightedFinish);
				tmButton.ReadColor(_T("GradientStartDisabled"), m_clrComboBtnDisabledStart);
				tmButton.ReadColor(_T("GradientFinishDisabled"), m_clrComboBtnDisabledFinish);
				tmButton.ReadColor(_T("GradientStartPressed"), m_clrComboBtnPressedStart);
				tmButton.ReadColor(_T("GradientFinishPressed"), m_clrComboBtnPressedFinish);

				tmButton.ReadColor(_T("BtnBorderHighlighted"), m_clrComboBtnBorderHighlighted);
				tmButton.ReadColor(_T("BtnBorderDisabled"), m_clrComboBtnBorderDisabled);

				m_clrComboBtnBorderPressed = m_clrComboBtnBorderHighlighted;
				tmButton.ReadColor(_T("BtnBorderPressed"), m_clrComboBtnBorderPressed);
			}
		}
	}

	m_clrRibbonEditBorder            = m_clrEditBorder;
	m_clrRibbonEditBorderDisabled    = m_clrEditBorderDisabled;
	m_clrRibbonEditBorderHighlighted = m_clrEditBorderHighlighted;
	m_clrRibbonEditBorderPressed     = m_clrRibbonEditBorderHighlighted;
	m_clrRibbonEditSelection         = m_clrEditSelection;

	m_clrRibbonComboBtnBorder            = m_clrComboBtnBorder;
	m_clrRibbonComboBtnBorderHighlighted = m_clrComboBtnBorderHighlighted;
	m_clrRibbonComboBtnBorderPressed     = m_clrComboBtnBorderPressed;
	m_clrRibbonComboBtnStart             = m_clrComboBtnStart;
	m_clrRibbonComboBtnFinish            = m_clrComboBtnFinish;
	m_clrRibbonComboBtnDisabledStart     = m_clrComboBtnDisabledStart;
	m_clrRibbonComboBtnDisabledFinish    = m_clrComboBtnDisabledFinish;
	m_clrRibbonComboBtnHighlightedStart  = m_clrComboBtnHighlightedStart;
	m_clrRibbonComboBtnHighlightedFinish = m_clrComboBtnHighlightedFinish;
	m_clrRibbonComboBtnPressedStart      = m_clrComboBtnPressedStart;
	m_clrRibbonComboBtnPressedFinish     = m_clrComboBtnPressedFinish;

	// task pane
	m_clrTaskPaneGradientDark       = m_clrBarGradientLight;
	m_clrTaskPaneGradientLight      = m_clrTaskPaneGradientDark;

	if (tm.ExcludeTag(_T("TASK"), strItem))
	{
		CTagManager tmItem(strItem);

		tmItem.ReadColor(_T("GradientDark"), m_clrTaskPaneGradientDark);
		tmItem.ReadColor(_T("GradientLight"), m_clrTaskPaneGradientLight);

		CString strGroup;
		if (tmItem.ExcludeTag(_T("GROUP"), strGroup))
		{
			CTagManager tmGroup(strGroup);

			CString strState;
			if (tmGroup.ExcludeTag(_T("NORMAL"), strState))
			{
				CTagManager tmState(strState);

				CString str;

				if (tmState.ExcludeTag(_T("CAPTION"), str))
				{
					CTagManager tmCaption(str);

					tmCaption.ReadColor(_T("DarkNormal"), m_clrTaskPaneGroupCaptionDark);
					tmCaption.ReadColor(_T("LightNormal"), m_clrTaskPaneGroupCaptionLight);
					tmCaption.ReadColor(_T("DarkHighlighted"), m_clrTaskPaneGroupCaptionHighDark);
					tmCaption.ReadColor(_T("LightHighlighted"), m_clrTaskPaneGroupCaptionHighLight);
					tmCaption.ReadColor(_T("TextNormal"), m_clrTaskPaneGroupCaptionText);
					tmCaption.ReadColor(_T("TextHighlighted"), m_clrTaskPaneGroupCaptionTextHigh);
				}

				if (tmState.ExcludeTag(_T("AREA"), str))
				{
					CTagManager tmArea(str);

					tmArea.ReadColor(_T("DarkNormal"), m_clrTaskPaneGroupAreaDark);
					tmArea.ReadColor(_T("LightNormal"), m_clrTaskPaneGroupAreaLight);
				}
			}

			if (tmGroup.ExcludeTag(_T("SPECIAL"), strState))
			{
				CTagManager tmState(strState);

				CString str;
				if (tmState.ExcludeTag(_T("CAPTION"), str))
				{
					CTagManager tmCaption(str);

					tmCaption.ReadColor(_T("DarkNormal"), m_clrTaskPaneGroupCaptionSpecDark);
					tmCaption.ReadColor(_T("LightNormal"), m_clrTaskPaneGroupCaptionSpecLight);
					tmCaption.ReadColor(_T("DarkHighlighted"), m_clrTaskPaneGroupCaptionHighSpecDark);
					tmCaption.ReadColor(_T("LightHighlighted"), m_clrTaskPaneGroupCaptionHighSpecLight);
					tmCaption.ReadColor(_T("TextNormal"), m_clrTaskPaneGroupCaptionTextSpec);
					tmCaption.ReadColor(_T("TextHighlighted"), m_clrTaskPaneGroupCaptionTextHighSpec);
				}

				if (tmState.ExcludeTag(_T("AREA"), str))
				{
					CTagManager tmArea(str);

					tmArea.ReadColor(_T("DarkNormal"), m_clrTaskPaneGroupAreaSpecDark);
					tmArea.ReadColor(_T("LightNormal"), m_clrTaskPaneGroupAreaSpecLight);
				}
			}

			if (tmGroup.ReadColor(_T("BORDER"), m_clrTaskPaneGroupBorder))
			{
				m_penTaskPaneGroupBorder.DeleteObject();
				m_penTaskPaneGroupBorder.CreatePen(PS_SOLID, 1, m_clrTaskPaneGroupBorder);
			}
		}

		tmItem.ReadControlRenderer(_T("SCROLL_BUTTON"), m_ctrlTaskScrollBtn, MakeResourceID(_T("IDB_OFFICE2007_TASKPANE_SCROLL_BTN")));
	}

	if (tm.ExcludeTag(_T("TABS"), strItem))
	{
		CTagManager tmItem(strItem);

		tmItem.ReadColor(_T("TextColorActive"), m_clrTabTextActive);
		tmItem.ReadColor(_T("TextColorInactive"), m_clrTabTextInactive);

		CString strTab;
		if (tmItem.ExcludeTag(_T("3D"), strTab))
		{
			CTagManager tmTab(strTab);

			CString strBtn;
			if (tmTab.ExcludeTag(_T("BUTTON"), strBtn))
			{
				CMFCControlRendererInfo params(MakeResourceID(_T("IDB_OFFICE2007_TAB_3D")), CRect(0, 0, 0, 0), CRect(0, 0, 0, 0));
				if (CTagManager::ParseControlRendererInfo(strBtn, params))
				{
					m_ctrlTab3D[0].Create(params);
					m_ctrlTab3D[1].Create(params, TRUE);
				}
			}
		}

		if (tmItem.ExcludeTag(_T("FLAT"), strTab))
		{
			CTagManager tmTab(strTab);

			CString strBtn;
			if (tmTab.ExcludeTag(_T("BUTTON"), strBtn))
			{
				CMFCControlRendererInfo params(MakeResourceID(_T("IDB_OFFICE2007_TAB_FLAT")), CRect(0, 0, 0, 0), CRect(0, 0, 0, 0));
				if (CTagManager::ParseControlRendererInfo(strBtn, params))
				{
					m_ctrlTabFlat[0].Create(params);
					m_ctrlTabFlat[1].Create(params, TRUE);
				}
			}

			tmTab.ReadColor(_T("Black"), m_clrTabFlatBlack);
			tmTab.ReadColor(_T("Highlight"), m_clrTabFlatHighlight);

			COLORREF clr;
			if (tmTab.ReadColor(_T("BorderInnerNormal"), clr))
			{
				m_penTabFlatInner[0].DeleteObject();
				m_penTabFlatInner[0].CreatePen(PS_SOLID, 1, clr);
			}
			if (tmTab.ReadColor(_T("BorderInnerActive"), clr))
			{
				m_penTabFlatInner[1].DeleteObject();
				m_penTabFlatInner[1].CreatePen(PS_SOLID, 1, clr);
			}
			if (tmTab.ReadColor(_T("BorderOuterNormal"), clr))
			{
				m_penTabFlatOuter[0].DeleteObject();
				m_penTabFlatOuter[0].CreatePen(PS_SOLID, 1, clr);
			}
			if (tmTab.ReadColor(_T("BorderOuterActive"), clr))
			{
				m_penTabFlatOuter[1].DeleteObject();
				m_penTabFlatOuter[1].CreatePen(PS_SOLID, 1, clr);
			}
		}
	}

	if (tm.ExcludeTag(_T("HEADER"), strItem))
	{
		CTagManager tmItem(strItem);

		tmItem.ReadColor(_T("NormalStart"), m_clrHeaderNormalStart);
		tmItem.ReadColor(_T("NormalFinish"), m_clrHeaderNormalFinish);
		tmItem.ReadColor(_T("NormalBorder"), m_clrHeaderNormalBorder);
		tmItem.ReadColor(_T("HighlightedStart"), m_clrHeaderHighlightedStart);
		tmItem.ReadColor(_T("HighlightedFinish"), m_clrHeaderHighlightedFinish);
		tmItem.ReadColor(_T("HighlightedBorder"), m_clrHeaderHighlightedBorder);
		tmItem.ReadColor(_T("PressedStart"), m_clrHeaderPressedStart);
		tmItem.ReadColor(_T("PressedFinish"), m_clrHeaderPressedFinish);
		tmItem.ReadColor(_T("PressedBorder"), m_clrHeaderPressedBorder);
	}

	m_clrRibbonCategoryText                = m_clrMenuBarBtnText;
	m_clrRibbonCategoryTextHighlighted     = m_clrMenuBarBtnTextHighlighted;
	m_clrRibbonCategoryTextDisabled		   = m_clrMenuBarBtnTextDisabled;
	m_clrRibbonPanelText                   = m_clrToolBarBtnText;
	m_clrRibbonPanelTextHighlighted        = m_clrToolBarBtnTextHighlighted;
	m_clrRibbonPanelCaptionText            = m_clrRibbonPanelText;
	m_clrRibbonPanelCaptionTextHighlighted = m_clrRibbonPanelTextHighlighted;

	m_clrRibbonEdit                        = afxGlobalData.clrBarLight;
	m_clrRibbonEditHighlighted             = afxGlobalData.clrWindow;
	m_clrRibbonEditPressed                 = m_clrRibbonEditHighlighted;
	m_clrRibbonEditDisabled                = afxGlobalData.clrBtnFace;

	if (tm.ExcludeTag(_T("RIBBON"), strItem))
	{
		CTagManager tmItem(strItem);

		CString str;

		if (tmItem.ExcludeTag(_T("CATEGORY"), str))
		{
			CTagManager tmCategory(str);
			tmCategory.ReadControlRenderer(_T("BACK"), m_ctrlRibbonCategoryBack, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_CATEGORY_BACK")));

			CString strTab;
			if (tmCategory.ExcludeTag(_T("TAB"), strTab))
			{
				CTagManager tmTab(strTab);
				tmTab.ReadControlRenderer(_T("BUTTON"), m_ctrlRibbonCategoryTab, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_CATEGORY_TAB")));

				tmTab.ReadColor(_T("TextNormal"), m_clrRibbonCategoryText);
				tmTab.ReadColor(_T("TextHighlighted"), m_clrRibbonCategoryTextHighlighted);
				tmTab.ReadColor(_T("TextDisabled"), m_clrRibbonCategoryTextDisabled);
			}

			tmCategory.ReadControlRenderer(_T("TAB_SEPARATOR"), m_ctrlRibbonCategoryTabSep, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_CATEGORY_TAB_SEP")));

			tmCategory.ReadControlRenderer(_T("BUTTON_PAGE_L"), m_ctrlRibbonCategoryBtnPage[0], MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_PAGE_L")));
			tmCategory.ReadControlRenderer(_T("BUTTON_PAGE_R"), m_ctrlRibbonCategoryBtnPage[1], MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_PAGE_R")));
		}

		if (tmItem.ExcludeTag(_T("PANEL"), str))
		{
			CTagManager tmPanel(str);

			{
				CString strBack;
				if (tmPanel.ExcludeTag(_T("BACK"), strBack))
				{
					CTagManager tmBack(strBack);

					tmBack.ReadControlRenderer(_T("TOP"), m_ctrlRibbonPanelBack_T, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_PANEL_BACK_T")));
					tmBack.ReadControlRenderer(_T("BOTTOM"), m_ctrlRibbonPanelBack_B, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_PANEL_BACK_B")));
				}
			}

			{
				CString strCaption;
				if (tmPanel.ExcludeTag(_T("CAPTION"), strCaption))
				{
					CTagManager tmCaption(strCaption);

					tmCaption.ReadControlRenderer(_T("LAUNCH_BTN"), m_ctrlRibbonBtnLaunch, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_LAUNCH")));
					tmCaption.ReadToolBarImages(_T("LAUNCH_ICON"), m_RibbonBtnLaunchIcon, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_LAUNCH_ICON")));
					tmCaption.ReadColor(_T("TextNormal"), m_clrRibbonPanelCaptionText);
					tmCaption.ReadColor(_T("TextHighlighted"), m_clrRibbonPanelCaptionTextHighlighted);
				}
			}

			tmPanel.ReadToolBarImages(_T("SEPARATOR"), m_RibbonPanelSeparator, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_PANEL_SEPARATOR")));
			tmPanel.ReadControlRenderer(_T("QAT"), m_ctrlRibbonPanelQAT, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_PANEL_QAT")));

			{
				CString strButtons;
				if (tmPanel.ExcludeTag(_T("BUTTONS"), strButtons))
				{
					CTagManager tmButtons(strButtons);

					tmButtons.ReadControlRenderer(_T("BUTTON_GROUP_F"), m_ctrlRibbonBtnGroup_F, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_GROUP_F")));
					tmButtons.ReadControlRenderer(_T("BUTTON_GROUP_M"), m_ctrlRibbonBtnGroup_M, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_GROUP_M")));
					tmButtons.ReadControlRenderer(_T("BUTTON_GROUP_L"), m_ctrlRibbonBtnGroup_L, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_GROUP_L")));
					tmButtons.ReadControlRenderer(_T("BUTTON_GROUP_S"), m_ctrlRibbonBtnGroup_S, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_GROUP_S")));

					tmButtons.ReadControlRenderer(_T("BUTTON_GROUPMENU_F_C"), m_ctrlRibbonBtnGroupMenu_F[0], MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_GROUPMENU_F_C")));
					tmButtons.ReadControlRenderer(_T("BUTTON_GROUPMENU_F_M"), m_ctrlRibbonBtnGroupMenu_F[1], MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_GROUPMENU_F_M")));
					tmButtons.ReadControlRenderer(_T("BUTTON_GROUPMENU_M_C"), m_ctrlRibbonBtnGroupMenu_M[0], MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_GROUPMENU_M_C")));
					tmButtons.ReadControlRenderer(_T("BUTTON_GROUPMENU_M_M"), m_ctrlRibbonBtnGroupMenu_M[1], MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_GROUPMENU_M_M")));
					tmButtons.ReadControlRenderer(_T("BUTTON_GROUPMENU_L_C"), m_ctrlRibbonBtnGroupMenu_L[0], MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_GROUPMENU_L_C")));
					tmButtons.ReadControlRenderer(_T("BUTTON_GROUPMENU_L_M"), m_ctrlRibbonBtnGroupMenu_L[1], MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_GROUPMENU_L_M")));

					tmButtons.ReadControlRenderer(_T("BUTTON_NORMAL_S"), m_ctrlRibbonBtn[0], MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_NORMAL_S")));
					tmButtons.ReadControlRenderer(_T("BUTTON_NORMAL_B"), m_ctrlRibbonBtn[1], MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_NORMAL_B")));

					tmButtons.ReadControlRenderer(_T("BUTTON_DEFAULT"), m_ctrlRibbonBtnDefault, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_DEFAULT")));
					tmButtons.ReadControlRenderer(_T("BUTTON_DEFAULT_ICON"), m_ctrlRibbonBtnDefaultIcon, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_DEFAULT_ICON")));
					tmButtons.ReadToolBarImages(_T("BUTTON_DEFAULT_IMAGE"), m_RibbonBtnDefaultImage, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_DEFAULT_IMAGE")));
					tmButtons.ReadControlRenderer(_T("BUTTON_DEFAULT_QAT"), m_ctrlRibbonBtnDefaultQAT, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_DEFAULT_QAT")));

					if (!m_ctrlRibbonBtnDefaultQAT.IsValid())
					{
						tmButtons.ReadControlRenderer(_T("BUTTON_DEFAULT_QAT_ICON"), m_ctrlRibbonBtnDefaultQATIcon, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_DEFAULT_QAT_ICON")));
					}

					tmButtons.ReadControlRenderer(_T("BUTTON_MENU_H_C"), m_ctrlRibbonBtnMenuH[0], MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_MENU_H_C")));
					tmButtons.ReadControlRenderer(_T("BUTTON_MENU_H_M"), m_ctrlRibbonBtnMenuH[1], MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_MENU_H_M")));
					tmButtons.ReadControlRenderer(_T("BUTTON_MENU_V_C"), m_ctrlRibbonBtnMenuV[0], MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_MENU_V_C")));
					tmButtons.ReadControlRenderer(_T("BUTTON_MENU_V_M"), m_ctrlRibbonBtnMenuV[1], MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_MENU_V_M")));
					tmButtons.ReadControlRenderer(_T("BUTTON_CHECK"), m_ctrlRibbonBtnCheck, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_CHECK")));

					tmButtons.ReadControlRenderer(_T("BUTTON_PNL_T"), m_ctrlRibbonBtnPalette[0], MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_PALETTE_T")));
					tmButtons.ReadControlRenderer(_T("BUTTON_PNL_M"), m_ctrlRibbonBtnPalette[1], MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_PALETTE_M")));
					tmButtons.ReadControlRenderer(_T("BUTTON_PNL_B"), m_ctrlRibbonBtnPalette[2], MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_PALETTE_B")));
				}
			}

			{
				CString strEdit;
				if (tmPanel.ExcludeTag(_T("EDIT"), strEdit))
				{
					CTagManager tmEdit(strEdit);

					tmEdit.ReadColor(_T("Normal"), m_clrRibbonEdit);
					tmEdit.ReadColor(_T("Highlighted"), m_clrRibbonEditHighlighted);
					tmEdit.ReadColor(_T("Disabled"), m_clrRibbonEditDisabled);
					tmEdit.ReadColor(_T("Pressed"), m_clrRibbonEditPressed);

					tmEdit.ReadColor(_T("BorderNormal"), m_clrRibbonEditBorder);
					tmEdit.ReadColor(_T("BorderHighlighted"), m_clrRibbonEditBorderHighlighted);
					tmEdit.ReadColor(_T("BorderDisabled"), m_clrRibbonEditBorderDisabled);
					tmEdit.ReadColor(_T("BorderPressed"), m_clrRibbonEditBorderPressed);
					tmEdit.ReadColor(_T("Selection"), m_clrRibbonEditSelection);

					CString strButton;
					if (tmEdit.ExcludeTag(_T("BUTTON"), strButton))
					{
						CTagManager tmButton(strButton);

						tmButton.ReadColor(_T("GradientStartNormal"), m_clrRibbonComboBtnStart);
						tmButton.ReadColor(_T("GradientFinishNormal"), m_clrRibbonComboBtnFinish);
						tmButton.ReadColor(_T("BtnBorderNormal"), m_clrRibbonComboBtnBorder);

						if (!tmButton.ReadControlRenderer(_T("IMAGE"), m_ctrlRibbonComboBoxBtn, MakeResourceID(_T("IDB_OFFICE2007_COMBOBOX_BTN"))))
						{
							tmButton.ReadColor(_T("GradientStartHighlighted"), m_clrRibbonComboBtnHighlightedStart);
							tmButton.ReadColor(_T("GradientFinishHighlighted"), m_clrRibbonComboBtnHighlightedFinish);
							tmButton.ReadColor(_T("GradientStartDisabled"), m_clrRibbonComboBtnDisabledStart);
							tmButton.ReadColor(_T("GradientFinishDisabled"), m_clrRibbonComboBtnDisabledFinish);
							tmButton.ReadColor(_T("GradientStartPressed"), m_clrRibbonComboBtnPressedStart);
							tmButton.ReadColor(_T("GradientFinishPressed"), m_clrRibbonComboBtnPressedFinish);

							tmButton.ReadColor(_T("BtnBorderHighlighted"), m_clrRibbonComboBtnBorderHighlighted);
							tmButton.ReadColor(_T("BtnBorderDisabled"), m_clrRibbonComboBtnBorderDisabled);

							m_clrRibbonComboBtnBorderPressed = m_clrRibbonComboBtnBorderHighlighted;
							tmButton.ReadColor(_T("BtnBorderPressed"), m_clrRibbonComboBtnBorderPressed);
						}
					}
				}
			}

			tmPanel.ReadColor(_T("TextNormal"), m_clrRibbonPanelText);
			tmPanel.ReadColor(_T("TextHighlighted"), m_clrRibbonPanelTextHighlighted);
		}

		if (tmItem.ExcludeTag(_T("CONTEXT"), str))
		{
			CTagManager tmContext(str);

			CString strCategory;
			if (tmContext.ExcludeTag(_T("CATEGORY"), strCategory))
			{
				CTagManager tmCategory(strCategory);

				CMFCControlRendererInfo prBack;
				CMFCControlRendererInfo prCaption;
				CMFCControlRendererInfo prTab;
				CMFCControlRendererInfo prDefault;
				COLORREF clrText = m_clrRibbonCategoryText;
				COLORREF clrTextHighlighted = m_clrRibbonCategoryTextHighlighted;
				COLORREF clrCaptionText = clrText;

				tmCategory.ReadControlRendererInfo(_T("BACK"), prBack);

				CString strTab;
				if (tmCategory.ExcludeTag(_T("TAB"), strTab))
				{
					CTagManager tmTab(strTab);

					tmTab.ReadControlRendererInfo(_T("BUTTON"), prTab);
					tmTab.ReadColor(_T("TextNormal"), clrText);
					tmTab.ReadColor(_T("TextHighlighted"), clrTextHighlighted);
				}

				CString strCaption;
				if (tmCategory.ExcludeTag(_T("CAPTION"), strCaption))
				{
					CTagManager tmCaption(strCaption);

					tmCaption.ReadControlRendererInfo(_T("BACK"), prCaption);
					tmCaption.ReadColor(_T("TextNormal"), clrCaptionText);
				}

				tmCategory.ReadControlRendererInfo(_T("BUTTON_DEFAULT"), prDefault);

				CString strID[AFX_RIBBON_CATEGORY_COLOR_COUNT] =
				{
					MakeResourceID(_T("IDB_OFFICE2007_RIBBON_CONTEXT_R_")),
					MakeResourceID(_T("IDB_OFFICE2007_RIBBON_CONTEXT_O_")),
					MakeResourceID(_T("IDB_OFFICE2007_RIBBON_CONTEXT_Y_")),
					MakeResourceID(_T("IDB_OFFICE2007_RIBBON_CONTEXT_G_")),
					MakeResourceID(_T("IDB_OFFICE2007_RIBBON_CONTEXT_B_")),
					MakeResourceID(_T("IDB_OFFICE2007_RIBBON_CONTEXT_I_")),
					MakeResourceID(_T("IDB_OFFICE2007_RIBBON_CONTEXT_V_"))
				};

				for (int i = 0; i < AFX_RIBBON_CATEGORY_COLOR_COUNT; i++)
				{
					CMFCRibbonContextCategory& cat = m_ctrlRibbonContextCategory[i];

					prDefault.m_strBmpResID = strID[i] + _T("BTN_DEFAULT");
					prTab.m_strBmpResID     = strID[i] + _T("CATEGORY_TAB");
					prCaption.m_strBmpResID = strID[i] + _T("CATEGORY_CAPTION");
					prBack.m_strBmpResID    = strID[i] + _T("CATEGORY_BACK");

					cat.m_ctrlBtnDefault.Create(prDefault);
					cat.m_ctrlCaption.Create(prCaption);
					cat.m_ctrlTab.Create(prTab);
					cat.m_ctrlBack.Create(prBack);
					cat.m_clrText            = clrText;
					cat.m_clrTextHighlighted = clrTextHighlighted;
					cat.m_clrCaptionText     = clrCaptionText;
				}
			}

			CString strPanel;
			if (tmContext.ExcludeTag(_T("PANEL"), strPanel))
			{
				CTagManager tmPanel(strPanel);

				CString strBack;
				if (tmPanel.ExcludeTag(_T("BACK"), strBack))
				{
					CTagManager tmBack(strBack);

					tmBack.ReadControlRenderer(_T("TOP"), m_ctrlRibbonContextPanelBack_T, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_CONTEXT_PANEL_BACK_T")));
					tmBack.ReadControlRenderer(_T("BOTTOM"), m_ctrlRibbonContextPanelBack_B, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_CONTEXT_PANEL_BACK_B")));
				}

				CString strCaption;
				if (tmPanel.ExcludeTag(_T("CAPTION"), strCaption))
				{
					CTagManager tmCaption(strCaption);

					tmCaption.ReadColor(_T("TextNormal"), m_clrRibbonContextPanelCaptionText);
					tmCaption.ReadColor(_T("TextHighlighted"), m_clrRibbonContextPanelCaptionTextHighlighted);
				}

				tmPanel.ReadColor(_T("TextNormal"), m_clrRibbonContextPanelText);
				tmPanel.ReadColor(_T("TextHighlighted"), m_clrRibbonContextPanelTextHighlighted);
			}

			tmContext.ReadControlRenderer(_T("SEPARATOR"), m_ctrlRibbonContextSeparator, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_CONTEXT_SEPARATOR")));
		}

		tmItem.ReadControlRenderer(_T("MAIN_BUTTON"), m_RibbonBtnMain, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_MAIN")));

		if (tmItem.ExcludeTag(_T("MAIN"), str))
		{
			CTagManager tmMain(str);
			tmMain.ReadControlRenderer(_T("BACK"), m_ctrlRibbonMainPanel, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_PANEL_MAIN")));
			tmMain.ReadControlRenderer(_T("BORDER"), m_ctrlRibbonMainPanelBorder, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_PANEL_MAIN_BORDER")));
			tmMain.ReadControlRenderer(_T("BUTTON"), m_ctrlRibbonBtnMainPanel, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_PANEL_MAIN")));
		}

		if (tmItem.ExcludeTag(_T("CAPTION"), str))
		{
			CTagManager tmCaption(str);
			tmCaption.ReadControlRenderer(_T("QA"), m_ctrlRibbonCaptionQA, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_CAPTION_QA")));
			tmCaption.ReadControlRenderer(_T("QA_GLASS"), m_ctrlRibbonCaptionQA_Glass, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_CAPTION_QA_GLASS")));
		}

		if (tmItem.ExcludeTag(_T("STATUS"), str))
		{
			CTagManager tmStatus(str);
			tmStatus.ReadControlRenderer(_T("PANE_BUTTON"), m_ctrlRibbonBtnStatusPane, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BTN_STATUS_PANE")));

			CString strSlider;
			if (tmStatus.ExcludeTag(_T("SLIDER"), strSlider))
			{
				CTagManager tmSlider(strSlider);

				tmSlider.ReadControlRenderer(_T("THUMB"), m_ctrlRibbonSliderThumb, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_SLIDER_THUMB")));
				tmSlider.ReadControlRenderer(_T("PLUS"), m_ctrlRibbonSliderBtnPlus, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_SLIDER_BTN_PLUS")));
				tmSlider.ReadControlRenderer(_T("MINUS"), m_ctrlRibbonSliderBtnMinus, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_SLIDER_BTN_MINUS")));
			}

			CString strProgress;
			if (tmStatus.ExcludeTag(_T("PROGRESS"), strProgress))
			{
				CTagManager tmProgress(strProgress);

				tmProgress.ReadControlRenderer(_T("BACK"), m_ctrlRibbonProgressBack, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_PROGRESS_BACK")));
				tmProgress.ReadControlRenderer(_T("NORMAL"), m_ctrlRibbonProgressNormal, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_PROGRESS_NORMAL")));
				tmProgress.ReadControlRenderer(_T("NORMAL_EXT"), m_ctrlRibbonProgressNormalExt, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_PROGRESS_NORMAL_EXT")));
				tmProgress.ReadControlRenderer(_T("INFINITY"), m_ctrlRibbonProgressInfinity, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_PROGRESS_INFINITY")));
			}
		}

		if (tmItem.ExcludeTag(_T("BORDERS"), str))
		{
			CTagManager tmBorders(str);

			tmBorders.ReadControlRenderer(_T("QAT"), m_ctrlRibbonBorder_QAT, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BORDER_QAT")));
			tmBorders.ReadControlRenderer(_T("FLOATY"), m_ctrlRibbonBorder_Floaty, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_BORDER_FLOATY")));
		}

		if (tmItem.ExcludeTag(_T("KEYTIP"), str))
		{
			CTagManager tmKeyTip(str);

			tmKeyTip.ReadControlRenderer(_T("BACK"), m_ctrlRibbonKeyTip, MakeResourceID(_T("IDB_OFFICE2007_RIBBON_KEYTIP_BACK")));
			tmKeyTip.ReadColor(_T("TextNormal"), m_clrRibbonKeyTipTextNormal);

			BOOL bSystem = FALSE;
			if (m_clrRibbonKeyTipTextNormal == (COLORREF)(-1))
			{
				if (m_bToolTipParams && m_ToolTipParams.m_clrText != (COLORREF)(-1))
				{
					m_clrRibbonKeyTipTextNormal = m_ToolTipParams.m_clrText;
				}
				else
				{
					bSystem = TRUE;
					m_clrRibbonKeyTipTextNormal = ::GetSysColor(COLOR_INFOTEXT);
				}
			}

			tmKeyTip.ReadColor(_T("TextDisabled"), m_clrRibbonKeyTipTextDisabled);

			if (m_clrRibbonKeyTipTextDisabled == (COLORREF)(-1))
			{
				if (bSystem)
				{
					m_clrRibbonKeyTipTextDisabled = afxGlobalData.clrGrayedText;
				}
				else
				{
					m_clrRibbonKeyTipTextDisabled = CDrawingManager::PixelAlpha(
						m_clrRibbonKeyTipTextNormal, afxGlobalData.clrWindow, 50);
				}
			}
		}

		if (tmItem.ExcludeTag(_T("HYPERLINK"), str))
		{
			CTagManager tmHyperlink(str);

			tmHyperlink.ReadColor(_T("Inactive"), m_clrRibbonHyperlinkInactive);
			tmHyperlink.ReadColor(_T("Active"), m_clrRibbonHyperlinkActive);
			tmHyperlink.ReadColor(_T("StatusbarInactive"), m_clrRibbonStatusbarHyperlinkInactive);
			tmHyperlink.ReadColor(_T("StatusbarActive"), m_clrRibbonStatusbarHyperlinkActive);
		}
	}

	m_clrOutlookCaptionTextNormal   = m_clrCaptionBarText;
	m_clrOutlookPageTextNormal      = m_clrOutlookCaptionTextNormal;
	m_clrOutlookPageTextHighlighted = m_clrOutlookPageTextNormal;
	m_clrOutlookPageTextPressed     = m_clrOutlookPageTextNormal;

	if (tm.ExcludeTag(_T("OUTLOOK"), strItem))
	{
		CTagManager tmItem(strItem);

		CString str;
		if (tmItem.ExcludeTag(_T("CAPTION"), str))
		{
			CTagManager tmCaption(str);

			tmCaption.ReadColor(_T("TextNormal"), m_clrOutlookCaptionTextNormal);
		}

		if (tmItem.ExcludeTag(_T("PAGEBUTTON"), str))
		{
			CTagManager tmPage(str);

			tmPage.ReadControlRenderer(_T("BACK"), m_ctrlOutlookWndPageBtn, MakeResourceID(_T("IDB_OFFICE2007_OUTLOOK_BTN_PAGE")));

			tmPage.ReadColor(_T("TextNormal"), m_clrOutlookPageTextNormal);
			tmPage.ReadColor(_T("TextHighlighted"), m_clrOutlookPageTextHighlighted);
			tmPage.ReadColor(_T("TextPressed"), m_clrOutlookPageTextPressed);
		}

		if (tmItem.ExcludeTag(_T("BAR"), str))
		{
			CTagManager tmBar(str);

			tmBar.ReadControlRenderer(_T("BACK"), m_ctrlOutlookWndBar, MakeResourceID(_T("IDB_OFFICE2007_OUTLOOK_BAR_BACK")));
		}
	}

	// Popup Window:
	m_clrPopupGradientLight = m_clrBarGradientLight;
	m_clrPopupGradientDark = m_clrBarGradientDark;

	if (tm.ExcludeTag(_T("POPUP"), strItem))
	{
		CTagManager tmItem(strItem);

		tmItem.ReadColor(_T("GradientFillLight"), m_clrPopupGradientLight);
		tmItem.ReadColor(_T("GradientFillDark"), m_clrPopupGradientDark);
	}

	if (hinstResOld != NULL)
	{
		AfxSetResourceHandle(hinstResOld);
	}
}

BOOL CMFCVisualManagerOffice2007::IsWindowActive(CWnd* pWnd) const
{
	BOOL bActive = FALSE;

	HWND hWnd = pWnd->GetSafeHwnd();

	if (hWnd != NULL)
	{
		if (!m_ActivateFlag.Lookup(pWnd->GetSafeHwnd(), bActive))
		{
			//ASSERT(FALSE);
			bActive = TRUE;
		}
	}

	return bActive;
}

BOOL CMFCVisualManagerOffice2007::OnNcActivate(CWnd* pWnd, BOOL bActive)
{
	ASSERT_VALID(pWnd);

	if (pWnd->GetSafeHwnd() == NULL)
	{
		return FALSE;
	}

	if (afxGlobalData.DwmIsCompositionEnabled())
	{
		return FALSE;
	}

	// stay active if WF_STAYACTIVE bit is on
	if (pWnd->m_nFlags & WF_STAYACTIVE)
	{
		bActive = TRUE;
	}

	// but do not stay active if the window is disabled
	if (!pWnd->IsWindowEnabled())
	{
		bActive = FALSE;
	}

	BOOL bIsMDIFrame = FALSE;
	BOOL bWasActive = FALSE;

	// If the active state of an owner-draw MDI frame window changes, we need to
	// invalidate the MDI client area so the MDI child window captions are redrawn.
	if (IsOwnerDrawCaption())
	{
		bIsMDIFrame = pWnd->IsKindOf(RUNTIME_CLASS(CMDIFrameWnd));
		bWasActive = IsWindowActive(pWnd);
	}

	m_ActivateFlag[pWnd->GetSafeHwnd()] = bActive;
	pWnd->SendMessage(WM_NCPAINT, 0, 0);

	if (IsOwnerDrawCaption())
	{
		if (bIsMDIFrame && (bWasActive != bActive))
		{
			::RedrawWindow(((CMDIFrameWnd *)pWnd)->m_hWndMDIClient, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
		}
	}

	return TRUE;
}

void CMFCVisualManagerOffice2007::DrawNcBtn(CDC* pDC, const CRect& rect, UINT nButton, AFX_BUTTON_STATE state, BOOL bSmall, BOOL bActive, BOOL bMDI/* = FALSE*/)
{
	ASSERT_VALID(pDC);

	CMFCToolBarImages* pImage = NULL;

	int nIndex = bSmall ? 1 : 0;

	if (nButton == SC_CLOSE)
	{
		pImage = &m_SysBtnClose[nIndex];
	}
	else if (nButton == SC_MINIMIZE)
	{
		pImage = &m_SysBtnMinimize[nIndex];
	}
	else if (nButton == SC_MAXIMIZE)
	{
		pImage = &m_SysBtnMaximize[nIndex];
	}
	else if (nButton == SC_RESTORE)
	{
		pImage = &m_SysBtnRestore[nIndex];
	}
	else
	{
		return;
	}

	CMFCToolBarImages::ImageAlignHorz horz = afxGlobalData.GetRibbonImageScale() != 1. ? CMFCToolBarImages::ImageAlignHorzStretch : CMFCToolBarImages::ImageAlignHorzCenter;
	CMFCToolBarImages::ImageAlignVert vert = afxGlobalData.GetRibbonImageScale() != 1. ? CMFCToolBarImages::ImageAlignVertStretch : CMFCToolBarImages::ImageAlignVertCenter;

	CRect rtBtnImage(CPoint(0, 0), pImage->GetImageSize());

	if (!bActive)
	{
		rtBtnImage.OffsetRect(0, pImage->GetImageSize().cy * 3);
	}
	else
	{
		if (state != ButtonsIsRegular)
		{
			if (!IsBeta() && bMDI)
			{
				m_ctrlRibbonBtn[0].Draw(pDC, rect, state == ButtonsIsHighlighted ? 0 : 1);
			}
			else
			{
				m_SysBtnBack[nIndex].Draw(pDC, rect, state == ButtonsIsHighlighted ? 0 : 1);
			}

			rtBtnImage.OffsetRect(0, pImage->GetImageSize().cy * (state == ButtonsIsHighlighted ? 1 : 2));
		}
	}

	pImage->DrawEx(pDC, rect, 0, horz, vert, rtBtnImage);
}

void CMFCVisualManagerOffice2007::DrawNcText(CDC* pDC, CRect& rect, const CString& strTitle, const CString& strDocument, BOOL bPrefix, BOOL bActive,
	BOOL bIsRTL, BOOL bTextCenter, BOOL bGlass/* = FALSE*/, int nGlassGlowSize/* = 0*/, COLORREF clrGlassText/* = (COLORREF)-1*/)
{
	if ((strTitle.IsEmpty() && strDocument.IsEmpty()) || rect.right <= rect.left)
	{
		return;
	}

	ASSERT_VALID(pDC);

	int nOldMode = pDC->SetBkMode(TRANSPARENT);
	COLORREF clrOldText = pDC->GetTextColor();

	DWORD dwTextStyle = DT_END_ELLIPSIS | DT_SINGLELINE | DT_VCENTER | (bIsRTL ? DT_RTLREADING : 0);

	if (strDocument.IsEmpty())
	{
		COLORREF clrText = bActive ? m_clrAppCaptionActiveTitleText : m_clrAppCaptionInactiveTitleText;

		int widthFull = rect.Width();
		int width = pDC->GetTextExtent(strTitle).cx;

		if (bTextCenter && width < widthFull)
		{
			rect.left += (widthFull - width) / 2;
		}

		rect.right = min(rect.left + width, rect.right);

		if (rect.right > rect.left)
		{
			if (bGlass)
			{
				DrawTextOnGlass(pDC, strTitle, rect, dwTextStyle, nGlassGlowSize, clrGlassText);
			}
			else
			{
				pDC->SetTextColor(clrText);
				pDC->DrawText(strTitle, rect, dwTextStyle);
			}
		}
	}
	else
	{
		const CString& str1 = bPrefix ? strDocument : strTitle;
		const CString& str2 = bPrefix ? strTitle : strDocument;

		COLORREF clrText1 = bActive ? m_clrAppCaptionActiveText : m_clrAppCaptionInactiveText;
		COLORREF clrText2 = bActive ? m_clrAppCaptionActiveTitleText : m_clrAppCaptionInactiveTitleText;

		if (!bPrefix)
		{
			COLORREF clr = clrText1;
			clrText1 = clrText2;
			clrText2 = clr;
		}

		int widthFull = rect.Width();
		CSize sz1 = pDC->GetTextExtent(str1);
		CSize sz2 = pDC->GetTextExtent(str2);
		int width = sz1.cx + sz2.cx;
		int left = rect.left;

		if (bTextCenter && width < widthFull)
		{
			rect.left += (widthFull - width) / 2;
		}

		rect.right = min(rect.left + width, rect.right);

		if (bIsRTL)
		{
			if (width <= rect.Width())
			{
				rect.left += sz2.cx;
			}
			else
			{
				if (sz1.cx < rect.Width())
				{
					rect.left += max(0, sz2.cx +(rect.Width() - width));
				}
			}
		}

		if (bGlass)
		{
			DrawTextOnGlass(pDC, str1, rect, dwTextStyle, nGlassGlowSize, clrGlassText);
		}
		else
		{
			pDC->SetTextColor(clrText1);
			pDC->DrawText(str1, rect, dwTextStyle);
		}

		if (bIsRTL)
		{
			if (width <= (rect.right - left))
			{
				rect.right = rect.left;
				rect.left  = rect.right - sz2.cx;
			}
			else
			{
				rect.left = left;
				rect.right -= sz1.cx;
			}
		}
		else
		{
			rect.left += sz1.cx;
		}

		if (rect.right > rect.left)
		{
			if (bGlass)
			{
				DrawTextOnGlass(pDC, str2, rect, dwTextStyle, nGlassGlowSize, clrGlassText);
			}
			else
			{
				pDC->SetTextColor(clrText2);
				pDC->DrawText(str2, rect, dwTextStyle);
			}
		}
	}

	pDC->SetBkMode(nOldMode);
	pDC->SetTextColor(clrOldText);
}

void CMFCVisualManagerOffice2007::DrawNcCaption(CDC* pDC, CRect rectCaption, DWORD dwStyle, DWORD dwStyleEx, const CString& strTitle,
	const CString& strDocument, HICON hIcon, BOOL bPrefix, BOOL bActive, BOOL bTextCenter, const CObList& lstSysButtons)
{
	const BOOL bIsRTL = (dwStyleEx & WS_EX_LAYOUTRTL) == WS_EX_LAYOUTRTL;
	const int nSysCaptionHeight = ::GetSystemMetrics(SM_CYCAPTION);
	CSize szSysBorder(GetSystemBorders(FALSE));

	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	CBitmap memBmp;
	memBmp.CreateCompatibleBitmap(pDC, rectCaption.Width(), rectCaption.Height());
	CBitmap* pBmpOld = memDC.SelectObject(&memBmp);
	memDC.BitBlt(0, 0, rectCaption.Width(), rectCaption.Height(), pDC, 0, 0, SRCCOPY);

	BOOL bMaximized = (dwStyle & WS_MAXIMIZE) == WS_MAXIMIZE;

	{
		if (IsBeta())
		{
			COLORREF clr1  = bActive ? m_clrAppCaptionActiveStart : m_clrAppCaptionInactiveStart;
			COLORREF clr2  = bActive ? m_clrAppCaptionActiveFinish : m_clrAppCaptionInactiveFinish;

			CRect rectCaption1(rectCaption);
			CRect rectBorder(m_ctrlMainBorderCaption.GetParams().m_rectSides);

			rectCaption1.DeflateRect(rectBorder.left, rectBorder.top, rectBorder.right, rectBorder.bottom);

			{
				CDrawingManager dm(memDC);
				dm.Fill4ColorsGradient(rectCaption1, clr1, clr2, clr2, clr1, FALSE);
			}

			m_ctrlMainBorderCaption.DrawFrame(&memDC, rectCaption, bActive ? 0 : 1);
		}
		else
		{
			m_ctrlMainBorderCaption.Draw(&memDC, rectCaption, bActive ? 0 : 1);
		}
	}

	CRect rect(rectCaption);
	rect.DeflateRect(szSysBorder.cx, bMaximized ? szSysBorder.cy : 0, szSysBorder.cx, 0);

	// Draw icon:
	if (hIcon != NULL)
	{
		CSize szIcon(::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
		CRect rectIcon(rect.TopLeft(), CSize(nSysCaptionHeight, rect.Height()));

		long x = rect.left + max(0, (rectIcon.Width()  - szIcon.cx) / 2);
		long y = rect.top  + max(0, (rectIcon.Height() - szIcon.cy) / 2);

		::DrawIconEx(memDC.GetSafeHdc(), x, y, hIcon, szIcon.cx, szIcon.cy, 0, NULL, DI_NORMAL);

		rect.left += rectIcon.Width();
	}

	// Draw system buttons:
	int xButtonsRight = rect.right;

	for (POSITION pos = lstSysButtons.GetHeadPosition(); pos != NULL;)
	{
		CMFCCaptionButtonEx* pButton = (CMFCCaptionButtonEx*)lstSysButtons.GetNext(pos);
		ASSERT_VALID(pButton);

		AFX_BUTTON_STATE state = ButtonsIsRegular;

		if (pButton->m_bPushed && pButton->m_bFocused)
		{
			state = ButtonsIsPressed;
		}
		else if (pButton->m_bFocused)
		{
			state = ButtonsIsHighlighted;
		}

		UINT uiHit = pButton->GetHit();
		UINT nButton = 0;

		switch(uiHit)
		{
		case AFX_HTCLOSE:
			nButton = SC_CLOSE;
			break;

		case AFX_HTMAXBUTTON:
			nButton = (dwStyle & WS_MAXIMIZE) == WS_MAXIMIZE ? SC_RESTORE : SC_MAXIMIZE;
			break;

		case AFX_HTMINBUTTON:
			nButton = (dwStyle & WS_MINIMIZE) == WS_MINIMIZE ? SC_RESTORE : SC_MINIMIZE;
			break;
		}

		CRect rectBtn(pButton->GetRect());
		if (bMaximized)
		{
			rectBtn.OffsetRect(szSysBorder.cx, szSysBorder.cy);
		}

		DrawNcBtn(&memDC, rectBtn, nButton, state, FALSE, bActive, FALSE);

		xButtonsRight = min(xButtonsRight, pButton->GetRect().left);
	}

	// Draw text:
	if ((!strTitle.IsEmpty() || !strDocument.IsEmpty()) && rect.left < rect.right)
	{
		CFont* pOldFont = (CFont*)memDC.SelectObject(&m_AppCaptionFont);

		CRect rectText = rect;
		rectText.right = xButtonsRight - 1;

		DrawNcText(&memDC, rectText, strTitle, strDocument, bPrefix, bActive, bIsRTL, bTextCenter);

		memDC.SelectObject(pOldFont);
	}

	pDC->BitBlt(rectCaption.left, rectCaption.top, rectCaption.Width(), rectCaption.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pBmpOld);
}

CMFCRibbonBar* CMFCVisualManagerOffice2007::GetRibbonBar(CWnd* pWnd) const
{
	CMFCRibbonBar* pBar = NULL;

	if (pWnd == NULL)
	{
		pWnd = AfxGetMainWnd();
	}

	if (pWnd->GetSafeHwnd() == NULL)
	{
		return NULL;
	}

	if (pWnd->IsKindOf(RUNTIME_CLASS(CFrameWndEx)))
	{
		pBar = ((CFrameWndEx*) pWnd)->GetRibbonBar();
	}
	else if (pWnd->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		pBar = ((CMDIFrameWndEx*) pWnd)->GetRibbonBar();
	}

	return pBar;
}

BOOL CMFCVisualManagerOffice2007::IsRibbonPresent(CWnd* pWnd) const
{
	CMFCRibbonBar* pBar = GetRibbonBar(pWnd);

	return pBar != NULL && pBar->IsWindowVisible();
}

BOOL CMFCVisualManagerOffice2007::OnNcPaint(CWnd* pWnd, const CObList& lstSysButtons, CRect rectRedraw)
{
	if (!CanDrawImage())
	{
		return CMFCVisualManagerOffice2003::OnNcPaint(pWnd, lstSysButtons, rectRedraw);
	}

	ASSERT_VALID(pWnd);

	if (pWnd->GetSafeHwnd() == NULL)
	{
		return FALSE;
	}

	CWindowDC dc(pWnd);

	if (dc.GetSafeHdc() != NULL)
	{
		CRgn rgn;
		if (!rectRedraw.IsRectEmpty())
		{
			rgn.CreateRectRgnIndirect(rectRedraw);
			dc.SelectClipRgn(&rgn);
		}


		CMFCRibbonBar* pBar = GetRibbonBar(pWnd);
		BOOL bRibbonCaption  = pBar != NULL && pBar->IsWindowVisible() && pBar->IsReplaceFrameCaption();

		CRect rtWindow;
		pWnd->GetWindowRect(rtWindow);
		pWnd->ScreenToClient(rtWindow);

		CRect rtClient;
		pWnd->GetClientRect(rtClient);

		rtClient.OffsetRect(-rtWindow.TopLeft());
		dc.ExcludeClipRect(rtClient);

		rtWindow.OffsetRect(-rtWindow.TopLeft());

		BOOL bActive = IsWindowActive(pWnd);

		// Modify bActive (if currently TRUE) for owner-drawn MDI child windows: draw child
		// frame active only if window is active MDI child and the MDI frame window is active.
		if (bActive && IsOwnerDrawCaption() && pWnd->IsKindOf(RUNTIME_CLASS(CMDIChildWnd)))
		{
			CMDIFrameWnd *pParent = ((CMDIChildWnd *)pWnd)->GetMDIFrame();
			if (pParent)
			{
				CMDIChildWnd *pActiveChild = pParent->MDIGetActive(NULL);
				if (pActiveChild)
				{
					bActive = ((pActiveChild->GetSafeHwnd() == pWnd->GetSafeHwnd()) && IsWindowActive(pParent));
				}
			}
		}

		CRect rectCaption(rtWindow);
		CSize szSysBorder(GetSystemBorders(bRibbonCaption));

		rectCaption.bottom = rectCaption.top + szSysBorder.cy;

		const DWORD dwStyle = pWnd->GetStyle();
		BOOL bMaximized = (dwStyle & WS_MAXIMIZE) == WS_MAXIMIZE;

		if (!bRibbonCaption)
		{
			const int nSysCaptionHeight = ::GetSystemMetrics(SM_CYCAPTION);
			rectCaption.bottom += nSysCaptionHeight;

			const DWORD dwStyleEx = pWnd->GetExStyle();

			HICON hIcon = afxGlobalUtils.GetWndIcon(pWnd);

			CString strText;
			pWnd->GetWindowText(strText);

			CString strTitle(strText);
			CString strDocument;

			BOOL bPrefix = FALSE;
			if ((dwStyle & FWS_ADDTOTITLE) == FWS_ADDTOTITLE)
			{
				bPrefix = (dwStyle & FWS_PREFIXTITLE) == FWS_PREFIXTITLE;
				CFrameWnd* pFrameWnd = DYNAMIC_DOWNCAST(CFrameWnd, pWnd);

				if (pFrameWnd != NULL)
				{
					strTitle = pFrameWnd->GetTitle();

					if (!strTitle.IsEmpty())
					{
						int pos = strText.Find(strTitle);

						if (pos != -1)
						{
							if (strText.GetLength() > strTitle.GetLength())
							{
								if (pos == 0)
								{
									bPrefix = FALSE; // avoid exception
									strTitle = strText.Left(strTitle.GetLength() + 3);
									strDocument = strText.Right(strText.GetLength() - strTitle.GetLength());
								}
								else
								{
									strTitle = strText.Right(strTitle.GetLength() + 3);
									strDocument = strText.Left(strText.GetLength() - strTitle.GetLength());
								}
							}
						}
					}
					else
					{
						strDocument = strText;
					}
				}
			}

			if (bMaximized)
			{
				rectCaption.InflateRect(szSysBorder.cx, szSysBorder.cy, szSysBorder.cx, 0);
			}

			DrawNcCaption(&dc, rectCaption, dwStyle, dwStyleEx, strTitle, strDocument, hIcon, bPrefix, bActive, m_bNcTextCenter, lstSysButtons);

			if (bMaximized)
			{
				return TRUE;
			}
		}

		else
		{
			if (bMaximized)
			{
				return TRUE;
			}

			rectCaption.bottom += pBar->GetCaptionHeight();

			CRect rectBorder(m_ctrlMainBorderCaption.GetParams().m_rectSides);

			if (IsBeta())
			{
				COLORREF clr1  = bActive ? m_clrAppCaptionActiveStart : m_clrAppCaptionInactiveStart;
				COLORREF clr2  = bActive ? m_clrAppCaptionActiveFinish : m_clrAppCaptionInactiveFinish;

				CRect rectCaption2(rectCaption);
				rectCaption2.DeflateRect(rectBorder.left, rectBorder.top, rectBorder.right, rectBorder.bottom);

				{
					CDrawingManager dm(dc);
					dm.Fill4ColorsGradient(rectCaption2, clr1, clr2, clr2, clr1, FALSE);
				}

				m_ctrlMainBorderCaption.DrawFrame(&dc, rectCaption, bActive ? 0 : 1);
			}
			else
			{
				m_ctrlMainBorderCaption.Draw(&dc, rectCaption, bActive ? 0 : 1);
			}
		}

		rtWindow.top = rectCaption.bottom;
		dc.ExcludeClipRect(rectCaption);

		if (pWnd->IsKindOf(RUNTIME_CLASS(CMDIChildWnd)))
		{
			m_ctrlMDIChildBorder.DrawFrame(&dc, rtWindow, bActive ? 0 : 1);
		}
		else
		{
			m_ctrlMainBorder.DrawFrame(&dc, rtWindow, bActive ? 0 : 1);
		}

		//-------------------------------
		// Find status bar extended area:
		//-------------------------------
		CRect rectExt(0, 0, 0, 0);
		BOOL bExtended    = FALSE;
		BOOL bBottomFrame = FALSE;
		BOOL bIsStatusBar = FALSE;

		CWnd* pStatusBar = pWnd->GetDescendantWindow(AFX_IDW_STATUS_BAR, TRUE);

		if (pStatusBar->GetSafeHwnd() != NULL && pStatusBar->IsWindowVisible())
		{
			CMFCStatusBar* pClassicStatusBar = DYNAMIC_DOWNCAST(CMFCStatusBar, pStatusBar);
			if (pClassicStatusBar != NULL)
			{
				bExtended = pClassicStatusBar->GetExtendedArea(rectExt);
				bIsStatusBar = TRUE;
			}

			else
			{
				CMFCRibbonStatusBar* pRibbonStatusBar = DYNAMIC_DOWNCAST(CMFCRibbonStatusBar, pStatusBar);
				if (pRibbonStatusBar != NULL)
				{
					bExtended    = pRibbonStatusBar->GetExtendedArea(rectExt);
					bBottomFrame = pRibbonStatusBar->IsBottomFrame();
					bIsStatusBar = TRUE;
				}
			}

		}

		if (bIsStatusBar)
		{
			CRect rectStatus;
			pStatusBar->GetClientRect(rectStatus);

			int nHeight = rectStatus.Height();
			rectStatus.bottom = rtWindow.bottom;
			rectStatus.top    = rectStatus.bottom - nHeight -(bBottomFrame ? -1 : szSysBorder.cy);
			rectStatus.left   = rtWindow.left;
			rectStatus.right  = rtWindow.right;

			if (bExtended)
			{
				rectExt.left   = rectStatus.right - rectExt.Width() - szSysBorder.cx;
				rectExt.top    = rectStatus.top;
				rectExt.bottom = rectStatus.bottom;
				rectExt.right  = rtWindow.right;
			}

			m_ctrlStatusBarBack.Draw(&dc, rectStatus, bActive ? 0 : 1);

			if (bExtended)
			{
				rectExt.left -= m_ctrlStatusBarBack_Ext.GetParams().m_rectCorners.left;
				m_ctrlStatusBarBack_Ext.Draw(&dc, rectExt, bActive ? 0 : 1);
			}
		}

		dc.SelectClipRgn(NULL);

		return TRUE;
	}

	return CMFCVisualManagerOffice2003::OnNcPaint(pWnd, lstSysButtons, rectRedraw);
}

BOOL CMFCVisualManagerOffice2007::OnSetWindowRegion(CWnd* pWnd, CSize sizeWindow)
{
	ASSERT_VALID(pWnd);

	if (pWnd->GetSafeHwnd() == NULL)
	{
		return FALSE;
	}

	if (!CanDrawImage())
	{
		return FALSE;
	}

	if (afxGlobalData.DwmIsCompositionEnabled())
	{
		return FALSE;
	}

	CSize sz(0, 0);

	BOOL bMainWnd = FALSE;

	if (DYNAMIC_DOWNCAST(CMFCPopupMenu, pWnd) != NULL)
	{
		sz  = CSize(3, 3);
	}
	else if (DYNAMIC_DOWNCAST(CMFCRibbonBar, pWnd) != NULL)
	{
		return FALSE;
	}
	else
	{
		if ((pWnd->GetStyle() & WS_MAXIMIZE) == WS_MAXIMIZE)
		{
			pWnd->SetWindowRgn(NULL, TRUE);
			return TRUE;
		}

		sz  = CSize(9, 9);
		bMainWnd = TRUE;
	}

	if (sz != CSize(0, 0))
	{
		CRgn rgn;
		BOOL bCreated = FALSE;

		bCreated = rgn.CreateRoundRectRgn(0, 0, sizeWindow.cx + 1, sizeWindow.cy + 1, sz.cx, sz.cy);

		if (bCreated)
		{
			if (pWnd->IsKindOf(RUNTIME_CLASS(CMDIChildWnd)))
			{
				CRgn rgnWinodw;
				rgnWinodw.CreateRectRgn(0, sz.cy, sizeWindow.cx, sizeWindow.cy);

				rgn.CombineRgn(&rgn, &rgnWinodw, RGN_OR);
			}

			pWnd->SetWindowRgn((HRGN)rgn.Detach(), TRUE);
			return TRUE;
		}
	}

	return FALSE;
}

CSize CMFCVisualManagerOffice2007::GetNcBtnSize(BOOL bSmall) const
{
	return m_szNcBtnSize[bSmall ? 1 : 0];
}

void CMFCVisualManagerOffice2007::DrawSeparator(CDC* pDC, const CRect& rect, BOOL bHorz)
{
	DrawSeparator(pDC, rect, m_penSeparator, m_penSeparator2, bHorz);
}

void CMFCVisualManagerOffice2007::DrawSeparator(CDC* pDC, const CRect& rect, CPen& pen1, CPen& pen2, BOOL bHorz)
{
	CRect rect1(rect);
	CRect rect2;

	if (bHorz)
	{
		rect1.top += rect.Height() / 2 - 1;
		rect1.bottom = rect1.top;
		rect2 = rect1;
		rect2.OffsetRect(0, 1);
	}
	else
	{
		rect1.left += rect.Width() / 2 - 1;
		rect1.right = rect1.left;
		rect2 = rect1;
		rect2.OffsetRect(1, 0);
	}

	if (CMFCToolBarImages::m_bIsDrawOnGlass)
	{
		CDrawingManager dm(*pDC);

		LOGPEN logpen;

		pen1.GetLogPen(&logpen);
		dm.DrawLine(rect1.left, rect1.top, rect1.right, rect1.bottom, logpen.lopnColor);

		pen2.GetLogPen(&logpen);
		dm.DrawLine(rect2.left, rect2.top, rect2.right, rect2.bottom, logpen.lopnColor);
	}
	else
	{
		CPen* pOldPen = pDC->SelectObject(&pen1);
		pDC->MoveTo(rect1.TopLeft());
		pDC->LineTo(rect1.BottomRight());

		pDC->SelectObject(&pen2);
		pDC->MoveTo(rect2.TopLeft());
		pDC->LineTo(rect2.BottomRight());

		pDC->SelectObject(pOldPen);
	}
}

COLORREF CMFCVisualManagerOffice2007::GetCaptionBarTextColor(CMFCCaptionBar* pBar)
{
	if (!CanDrawImage())
	{
		return CMFCVisualManagerOffice2003::GetCaptionBarTextColor(pBar);
	}

	return m_clrCaptionBarText;
}

void CMFCVisualManagerOffice2007::OnDrawCaptionBarInfoArea(CDC* pDC, CMFCCaptionBar* pBar, CRect rect)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawCaptionBarInfoArea(pDC, pBar, rect);
		return;
	}

	ASSERT_VALID(pDC);

	CDrawingManager dm(*pDC);
	dm.FillGradient(rect, afxGlobalData.clrBarFace, RGB(255, 255, 255));

	pDC->Draw3dRect(rect, afxGlobalData.clrBarDkShadow, afxGlobalData.clrBarDkShadow);
}

void CMFCVisualManagerOffice2007::OnFillOutlookPageButton(CDC* pDC, const CRect& rect, BOOL bIsHighlighted, BOOL bIsPressed, COLORREF& clrText)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnFillOutlookPageButton(pDC, rect, bIsHighlighted, bIsPressed, clrText);
		return;
	}

	ASSERT_VALID(pDC);

	CRect rt(rect);

	if (m_ctrlOutlookWndPageBtn.IsValid())
	{
		int index = 0;

		if (bIsPressed)
		{
			index = 2;

			if (bIsHighlighted)
			{
				index = 3;
			}

			clrText = m_clrOutlookPageTextPressed;
		}
		else if (bIsHighlighted)
		{
			index = 1;

			clrText = m_clrOutlookPageTextHighlighted;
		}

		m_ctrlOutlookWndPageBtn.Draw(pDC, rt, index);
	}
	else
	{
		COLORREF clr1 = m_clrBarGradientDark;
		COLORREF clr2 = m_clrBarGradientLight;

		if (bIsPressed)
		{
			if (bIsHighlighted)
			{
				clr1 = m_clrHighlightDnGradientDark;
				clr2 = m_clrHighlightDnGradientLight;
			}
			else
			{
				clr1 = m_clrHighlightDnGradientLight;
				clr2 = m_clrHighlightDnGradientDark;
			}
		}
		else if (bIsHighlighted)
		{
			clr1 = m_clrHighlightGradientDark;
			clr2 = m_clrHighlightGradientLight;
		}

		CDrawingManager dm(*pDC);
		dm.FillGradient(rect, clr1, clr2, TRUE);
	}

	clrText = m_clrOutlookPageTextNormal;

	if (bIsPressed)
	{
		clrText = m_clrOutlookPageTextPressed;
	}
	else if (bIsHighlighted)
	{
		clrText = m_clrOutlookPageTextHighlighted;
	}
}

void CMFCVisualManagerOffice2007::OnDrawOutlookPageButtonBorder(CDC* pDC, CRect& rectBtn, BOOL bIsHighlighted, BOOL bIsPressed)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawOutlookPageButtonBorder(pDC, rectBtn, bIsHighlighted, bIsPressed);
		return;
	}

	pDC->Draw3dRect(rectBtn, afxGlobalData.clrBtnHilite, m_clrToolBarBottomLine);
}

void CMFCVisualManagerOffice2007::OnDrawOutlookBarSplitter(CDC* pDC, CRect rectSplitter)
{
	ASSERT_VALID(pDC);

	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawOutlookBarSplitter(pDC, rectSplitter);
		return;
	}

	CDrawingManager dm(*pDC);

	dm.FillGradient(rectSplitter, m_clrCaptionBarGradientDark, m_clrCaptionBarGradientLight, TRUE);

	rectSplitter.OffsetRect(0, 1);
	m_ToolBarTear.DrawEx(pDC, rectSplitter, 0, CMFCToolBarImages::ImageAlignHorzCenter, CMFCToolBarImages::ImageAlignVertCenter);
	rectSplitter.OffsetRect(0, -1);

	CPen* pOldPen = pDC->SelectObject(&m_penBottomLine);

	pDC->MoveTo(rectSplitter.left, rectSplitter.top);
	pDC->LineTo(rectSplitter.right, rectSplitter.top);

	pDC->MoveTo(rectSplitter.left, rectSplitter.bottom - 1);
	pDC->LineTo(rectSplitter.right, rectSplitter.bottom - 1);

	pDC->SelectObject(pOldPen);
}

void CMFCVisualManagerOffice2007::OnFillOutlookBarCaption(CDC* pDC, CRect rectCaption, COLORREF& clrText)
{
	CMFCVisualManagerOffice2003::OnFillOutlookBarCaption(pDC, rectCaption, clrText);

	if (CanDrawImage())
	{
		clrText = m_clrOutlookCaptionTextNormal;
	}
}

void CMFCVisualManagerOffice2007::OnFillBarBackground(CDC* pDC, CBasePane* pBar, CRect rectClient, CRect rectClip, BOOL bNCArea/* = FALSE*/)
{
	CRuntimeClass* pBarClass = pBar->GetRuntimeClass();

	if (!CanDrawImage() || pBar->IsDialogControl() || pBarClass->IsDerivedFrom(RUNTIME_CLASS(CMFCColorBar)))
	{
		CMFCVisualManagerOffice2003::OnFillBarBackground(pDC, pBar, rectClient, rectClip, bNCArea);
		return;
	}

	if (pBar->IsKindOf(RUNTIME_CLASS(CMFCMenuBar)))
	{
		BOOL bIsHorz = (pBar->GetPaneStyle() & CBRS_ORIENT_HORZ);
		COLORREF clr1 = bIsHorz ? m_clrMenuBarGradientDark : m_clrMenuBarGradientVertLight;
		COLORREF clr2 = bIsHorz ? m_clrMenuBarGradientLight : m_clrMenuBarGradientVertDark;

		CDrawingManager dm(*pDC);
		dm.Fill4ColorsGradient(rectClient, clr1, clr2, clr2, clr1, !bIsHorz);
		return;
	}
	else if (pBar->IsKindOf(RUNTIME_CLASS(CMFCPopupMenuBar)))
	{
		pDC->FillRect(rectClip, &m_brMenuLight);

		BOOL bQuickMode = FALSE;

		CMFCPopupMenuBar* pMenuBar = DYNAMIC_DOWNCAST(CMFCPopupMenuBar, pBar);
		if (!pMenuBar->m_bDisableSideBarInXPMode)
		{
			CWnd* pWnd = pMenuBar->GetParent();

			if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CMFCPopupMenu)))
			{
				CMFCPopupMenu* pMenu = DYNAMIC_DOWNCAST(CMFCPopupMenu, pWnd);

				if (pMenu->IsCustomizePane())
				{
					bQuickMode = TRUE;
				}
			}

			CRect rectImages = rectClient;
			//rectImages.DeflateRect(0, 1);

			if (bQuickMode)
			{
				rectImages.right = rectImages.left + 2 * CMFCToolBar::GetMenuImageSize().cx + 4 * GetMenuImageMargin() + 4;
			}
			else
			{
				rectImages.right = rectImages.left + CMFCToolBar::GetMenuImageSize().cx + 2 * GetMenuImageMargin() + 2;
			}

			pDC->FillRect(rectImages, &m_brBarBkgnd);

			CRect rect(rectImages);
			rectImages.left = rectImages.right;
			rectImages.right += 2;
			DrawSeparator(pDC, rectImages, FALSE);
		}

		return;
	}
	else if (pBar->IsKindOf(RUNTIME_CLASS(CMFCStatusBar)))
	{
		CSize szSysBorder(GetSystemBorders(TRUE));

		CRect rect(rectClient);
		CRect rectExt(0, 0, 0, 0);
		BOOL bExtended = ((CMFCStatusBar*)pBar)->GetExtendedArea(rectExt);

		if (bExtended)
		{
			rect.right = rectExt.left;
		}

		CWnd* pWnd = ((CMFCStatusBar*)pBar)->GetParent();
		ASSERT_VALID(pWnd);

		BOOL bActive = IsWindowActive(pWnd);

		rect.InflateRect(szSysBorder.cx, 0, szSysBorder.cx, szSysBorder.cy);
		m_ctrlStatusBarBack.Draw(pDC, rect, bActive ? 0 : 1);

		if (bExtended)
		{
			rectExt.InflateRect(0, 0, szSysBorder.cx, szSysBorder.cy);
			rectExt.left -= m_ctrlStatusBarBack_Ext.GetParams().m_rectCorners.left;
			m_ctrlStatusBarBack_Ext.Draw(pDC, rectExt, bActive ? 0 : 1);
		}

		return;
	}
	else if (pBar->IsKindOf(RUNTIME_CLASS(CMFCRibbonStatusBar)))
	{
		CMFCRibbonStatusBar* pRibbonStatusBar = DYNAMIC_DOWNCAST(CMFCRibbonStatusBar, pBar);

		CSize szSysBorder(GetSystemBorders(TRUE));

		CRect rect(rectClient);
		CRect rectExt(0, 0, 0, 0);

		BOOL bExtended    = pRibbonStatusBar->GetExtendedArea(rectExt);
		BOOL bBottomFrame = pRibbonStatusBar->IsBottomFrame();

		if (bExtended)
		{
			rect.right = rectExt.left;
		}

		CWnd* pWnd = pBar->GetParent();
		ASSERT_VALID(pWnd);

		BOOL bActive = IsWindowActive(pWnd);

		rect.InflateRect(szSysBorder.cx, 0, szSysBorder.cx, bBottomFrame ? -1 : szSysBorder.cy);
		m_ctrlStatusBarBack.Draw(pDC, rect, bActive ? 0 : 1);

		if (bExtended)
		{
			rectExt.InflateRect(0, 0, szSysBorder.cx, bBottomFrame ? -1 : szSysBorder.cy);
			rectExt.left -= m_ctrlStatusBarBack_Ext.GetParams().m_rectCorners.left;
			m_ctrlStatusBarBack_Ext.Draw(pDC, rectExt, bActive ? 0 : 1);
		}

		return;
	}
	else if (pBar->IsKindOf(RUNTIME_CLASS(CMFCOutlookBarToolBar)))
	{
		if (m_ctrlOutlookWndBar.IsValid())
		{
			m_ctrlOutlookWndBar.Draw(pDC, rectClient);
		}
		else
		{
			CDrawingManager dm(*pDC);
			dm.FillGradient(rectClient, m_clrToolBarGradientDark, m_clrToolBarGradientLight, TRUE);
		}

		return;
	}

	CMFCVisualManagerOffice2003::OnFillBarBackground(pDC, pBar, rectClient, rectClip, bNCArea);
}

void CMFCVisualManagerOffice2007::OnFillHighlightedArea(CDC* pDC, CRect rect, CBrush* pBrush, CMFCToolBarButton* pButton)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnFillHighlightedArea(pDC, rect, pBrush, pButton);
		return;
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID(pBrush);

	BOOL bIsHorz = TRUE;

	COLORREF clr1 = (COLORREF)-1;
	COLORREF clr2 = (COLORREF)-1;

	if (pButton != NULL)
	{
		ASSERT_VALID(pButton);

		bIsHorz = pButton->IsHorizontal();

		CMFCToolBarMenuButton* pCustButton = DYNAMIC_DOWNCAST(CMFCCustomizeButton, pButton);

		if (pCustButton != NULL)
		{
			if (pButton->IsDroppedDown())
			{
				clr1 = m_clrHighlightDnGradientDark;
				clr2 = m_clrHighlightDnGradientLight;
			}
		}
	}

	if (pBrush == &m_brHighlight)
	{
		clr1 = m_clrHighlightGradientDark;
		clr2 = m_clrHighlightGradientLight;//bIsPopupMenu ? clr1 : m_clrHighlightGradientLight;
	}
	else if (pBrush == &m_brHighlightDn)
	{
		clr1 = m_clrHighlightDnGradientDark;//bIsPopupMenu ? m_clrHighlightDnGradientLight : m_clrHighlightDnGradientDark;
		clr2 = m_clrHighlightDnGradientLight;
	}
	else if (pBrush == &m_brHighlightChecked)
	{
		clr1 = m_clrHighlightCheckedGradientDark;//bIsPopupMenu ? m_clrHighlightCheckedGradientLight : m_clrHighlightCheckedGradientDark;
		clr2 = m_clrHighlightCheckedGradientLight;
	}

	if (clr1 == (COLORREF)-1 || clr2 == (COLORREF)-1)
	{
		CMFCVisualManagerOffice2003::OnFillHighlightedArea(pDC, rect, pBrush, pButton);
		return;
	}

	CDrawingManager dm(*pDC);
	dm.FillGradient(rect, clr1, clr2, bIsHorz);
}

void CMFCVisualManagerOffice2007::OnDrawMenuBorder(CDC* pDC, CMFCPopupMenu* pMenu, CRect rect)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawMenuBorder(pDC, pMenu, rect);
		return;
	}

	if (pMenu != NULL)
	{
		CMFCRibbonPanelMenuBar* pRibbonMenuBar = DYNAMIC_DOWNCAST(CMFCRibbonPanelMenuBar, pMenu->GetMenuBar());

		if (pRibbonMenuBar != NULL)
		{
			ASSERT_VALID(pRibbonMenuBar);

			if (pRibbonMenuBar->IsMainPanel())
			{
				if (m_ctrlRibbonMainPanel.IsValid())
				{
					m_ctrlRibbonMainPanel.DrawFrame(pDC, rect);
				}
				else
				{
					m_ctrlPopupBorder.DrawFrame(pDC, rect);
				}

				return;
			}

			if (!pRibbonMenuBar->IsMenuMode())
			{
				if (pRibbonMenuBar->IsQATPopup() && m_ctrlRibbonBorder_QAT.IsValid())
				{
					m_ctrlRibbonBorder_QAT.DrawFrame(pDC, rect);
					return;
				}
				else if (pRibbonMenuBar->IsCategoryPopup())
				{
					if (IsBeta1())
					{
						m_ctrlRibbonCategoryBack.DrawFrame(pDC, rect);
					}

					return;
				}
				else if (pRibbonMenuBar->IsRibbonMiniToolBar() && m_ctrlRibbonBorder_Floaty.IsValid())
				{
					m_ctrlRibbonBorder_Floaty.DrawFrame(pDC, rect);
					return;
				}
				else
				{
					if (pRibbonMenuBar->GetPanel() != NULL)
					{
						if (IsBeta1())
						{
							m_ctrlRibbonCategoryBack.DrawFrame(pDC, rect);
						}

						return;
					}

					// draw standard
				}
			}
		}
	}

	CBasePane* pTopLevelBar = NULL;

	for (CMFCPopupMenu* pParentMenu = pMenu; pParentMenu != NULL; pParentMenu = pParentMenu->GetParentPopupMenu())
	{
		CMFCToolBarMenuButton* pParentButton = pParentMenu->GetParentButton();
		if (pParentButton == NULL)
		{
			break;
		}

		pTopLevelBar = DYNAMIC_DOWNCAST(CBasePane, pParentButton->GetParentWnd());
	}

	if (pTopLevelBar == NULL || pTopLevelBar->IsKindOf(RUNTIME_CLASS(CMFCPopupMenuBar)))
	{
		m_ctrlPopupBorder.DrawFrame(pDC, rect);
	}
	else
	{
		CMFCVisualManagerOffice2003::OnDrawMenuBorder(pDC, pMenu, rect);
	}
}

void CMFCVisualManagerOffice2007::OnDrawBarGripper(CDC* pDC, CRect rectGripper, BOOL bHorz, CBasePane* pBar)
{
	if (!CanDrawImage() || (pBar != NULL && pBar->IsDialogControl()) || m_ToolBarGripper.GetCount() == 0)
	{
		CMFCVisualManagerOffice2003::OnDrawBarGripper(pDC, rectGripper, bHorz, pBar);
		return;
	}

	CSize szBox(m_ToolBarGripper.GetImageSize());

	if (szBox != CSize(0, 0))
	{
		if (bHorz)
		{
			rectGripper.left = rectGripper.right - szBox.cx;
		}
		else
		{
			rectGripper.top = rectGripper.bottom - szBox.cy;
		}

		CMFCToolBar* pToolBar = DYNAMIC_DOWNCAST(CMFCToolBar, pBar);
		if (pToolBar != NULL)
		{
			if (bHorz)
			{
				const int nHeight = CMFCToolBar::IsLargeIcons() ? pToolBar->GetRowHeight() : pToolBar->GetButtonSize().cy;
				const int nDelta = max(0, (nHeight - pToolBar->GetImageSize().cy) / 2);
				rectGripper.DeflateRect(0, nDelta);
			}
			else
			{
				const int nWidth = CMFCToolBar::IsLargeIcons() ? pToolBar->GetColumnWidth() : pToolBar->GetButtonSize().cx;
				const int nDelta = max(0, (nWidth - pToolBar->GetImageSize().cx) / 2);
				rectGripper.DeflateRect(nDelta, 0);
			}
		}

		const int nBoxesNumber = bHorz ? (rectGripper.Height() - szBox.cy) / szBox.cy : (rectGripper.Width() - szBox.cx) / szBox.cx;
		int nOffset = bHorz ? (rectGripper.Height() - nBoxesNumber * szBox.cy) / 2 : (rectGripper.Width() - nBoxesNumber * szBox.cx) / 2;

		for (int nBox = 0; nBox < nBoxesNumber; nBox++)
		{
			int x = bHorz ? rectGripper.left :
			rectGripper.left + nOffset;

			int y = bHorz ? rectGripper.top + nOffset :
			rectGripper.top;

			m_ToolBarGripper.DrawEx(pDC, CRect(CPoint(x, y), szBox), 0);
			nOffset += bHorz ? szBox.cy : szBox.cx;
		}
	}
}

void CMFCVisualManagerOffice2007::OnDrawSeparator(CDC* pDC, CBasePane* pBar, CRect rect, BOOL bHorz)
{
	ASSERT_VALID(pDC);

	if (!CanDrawImage() || pBar == NULL || pBar->IsDialogControl())
	{
		CMFCVisualManagerOffice2003::OnDrawSeparator(pDC, pBar, rect, bHorz);
		return;
	}

	ASSERT_VALID(pBar);

	CRect rectSeparator(rect);


	if (pBar->IsKindOf(RUNTIME_CLASS(CMFCRibbonStatusBar)))
	{
		CMFCRibbonStatusBar* pRibbonStatusBar = DYNAMIC_DOWNCAST(CMFCRibbonStatusBar, pBar);

		rect.InflateRect(1, 5, 1, pRibbonStatusBar->IsBottomFrame() ? 2 : 5);

		m_StatusBarPaneBorder.DrawEx(pDC, rect, 0, CMFCToolBarImages::ImageAlignHorzRight, CMFCToolBarImages::ImageAlignVertStretch);
		return;
	}

	if (pBar->IsKindOf(RUNTIME_CLASS(CMFCRibbonBar)) || (bHorz && pBar->IsKindOf(RUNTIME_CLASS(CMFCRibbonPanelMenuBar))))
	{
		if (rect.Width() < m_RibbonPanelSeparator.GetImageSize().cx)
		{
			rect.left = rect.right - m_RibbonPanelSeparator.GetImageSize().cx;
		}

		m_RibbonPanelSeparator.DrawEx(pDC, rect, 0, CMFCToolBarImages::ImageAlignHorzCenter, CMFCToolBarImages::ImageAlignVertCenter);
		return;
	}

	BOOL bPopupMenu = FALSE;

	if (!bHorz)
	{
		BOOL bIsRibbon = FALSE;
		bIsRibbon = pBar->IsKindOf(RUNTIME_CLASS(CMFCRibbonPanelMenuBar));

		if (bIsRibbon &&((CMFCRibbonPanelMenuBar*) pBar)->IsDefaultMenuLook())
		{
			bIsRibbon = FALSE;
		}

		bPopupMenu = pBar->IsKindOf(RUNTIME_CLASS(CMFCPopupMenuBar));

		if (bPopupMenu && !bIsRibbon && !pBar->IsKindOf(RUNTIME_CLASS(CMFCColorBar)))
		{
			rectSeparator.left = rect.left + CMFCToolBar::GetMenuImageSize().cx + GetMenuImageMargin() + 1;

			CRect rectBar;
			pBar->GetClientRect(rectBar);

			if (rectBar.right - rectSeparator.right < 50) // Last item in row
			{
				rectSeparator.right = rectBar.right;
			}

			if (((CMFCPopupMenuBar*) pBar)->m_bDisableSideBarInXPMode)
			{
				rectSeparator.left = 0;
			}

			//---------------------------------
			// Maybe Quick Customize separator
			//---------------------------------
			if (bPopupMenu)
			{
				CWnd* pWnd = pBar->GetParent();
				if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CMFCPopupMenu)))
				{
					CMFCPopupMenu* pMenu = (CMFCPopupMenu*)pWnd;
					if (pMenu->IsCustomizePane())
					{
						rectSeparator.left = rect.left + 2 * CMFCToolBar::GetMenuImageSize().cx + 3 * GetMenuImageMargin() + 2;
					}
				}
			}
		}
	}

	if (bPopupMenu)
	{
		DrawSeparator(pDC, rectSeparator, !bHorz);
	}
	else
	{
		if (bHorz)
		{
			int nHeight = rectSeparator.Height() / 5;
			rectSeparator.top    += nHeight;
			rectSeparator.bottom -= nHeight;
		}
		else
		{
			int nWidth = rectSeparator.Width() / 5;
			rectSeparator.left  += nWidth;
			rectSeparator.right -= nWidth;
		}

		DrawSeparator(pDC, rectSeparator, m_penSeparatorDark, m_penSeparatorLight, !bHorz);
	}
}

COLORREF CMFCVisualManagerOffice2007::OnDrawPaneCaption(CDC* pDC, CDockablePane* pBar, BOOL bActive, CRect rectCaption, CRect rectButtons)
{
	if (!CanDrawImage() || pBar == NULL || pBar->IsDialogControl())
	{
		return CMFCVisualManagerOffice2003::OnDrawPaneCaption(pDC, pBar, bActive, rectCaption, rectButtons);
	}

	ASSERT_VALID(pDC);

	CPen pen(PS_SOLID, 1, afxGlobalData.clrBarFace);
	CPen* pOldPen = pDC->SelectObject(&pen);

	rectCaption.bottom += 2;

	pDC->MoveTo(rectCaption.left, rectCaption.bottom);
	pDC->LineTo(rectCaption.left, rectCaption.top);

	pDC->MoveTo(rectCaption.left  + 1, rectCaption.top);
	pDC->LineTo(rectCaption.right - 1, rectCaption.top);

	pDC->MoveTo(rectCaption.right - 1, rectCaption.top + 1);
	pDC->LineTo(rectCaption.right - 1, rectCaption.bottom);

	pDC->SelectObject(pOldPen);

	rectCaption.DeflateRect(1, 1, 1, 0);
	pDC->FillRect(rectCaption, bActive ? &afxGlobalData.brActiveCaption : &afxGlobalData.brInactiveCaption);

	return bActive ? afxGlobalData.clrCaptionText : afxGlobalData.clrInactiveCaptionText;
}

void CMFCVisualManagerOffice2007::OnDrawStatusBarPaneBorder(CDC* pDC, CMFCStatusBar* pBar, CRect rectPane, UINT uiID, UINT nStyle)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawStatusBarPaneBorder(pDC, pBar, rectPane, uiID, nStyle);
		return;
	}

	BOOL bExtended = pBar->GetDrawExtendedArea();
	if (!bExtended ||((nStyle & SBPS_STRETCH) == 0 && bExtended))
	{
		rectPane.OffsetRect(1, 0);
		m_StatusBarPaneBorder.DrawEx(pDC, rectPane, 0, CMFCToolBarImages::ImageAlignHorzRight, CMFCToolBarImages::ImageAlignVertStretch);
	}
}

void CMFCVisualManagerOffice2007::OnDrawStatusBarSizeBox(CDC* pDC, CMFCStatusBar* pStatBar, CRect rectSizeBox)
{
	if (!CanDrawImage() ||
		m_StatusBarSizeBox.GetCount() == 0)
	{
		CMFCVisualManagerOffice2003::OnDrawStatusBarSizeBox(pDC, pStatBar, rectSizeBox);
		return;
	}

	m_StatusBarSizeBox.DrawEx(pDC, rectSizeBox, 0, CMFCToolBarImages::ImageAlignHorzRight, CMFCToolBarImages::ImageAlignVertBottom);
}

void CMFCVisualManagerOffice2007::OnDrawComboDropButton(CDC* pDC, CRect rect, BOOL bDisabled, BOOL bIsDropped, BOOL bIsHighlighted, CMFCToolBarComboBoxButton* pButton)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawComboDropButton(pDC, rect, bDisabled, bIsDropped, bIsHighlighted, pButton);
		return;
	}

	BOOL bRibbon = pButton != NULL && pButton->IsRibbonButton();

	BOOL bActive = bIsHighlighted || bIsDropped;

	CMFCControlRenderer* pRenderer = bRibbon ? &m_ctrlRibbonComboBoxBtn : &m_ctrlComboBoxBtn;

	if (!pRenderer->IsValid())
	{
		COLORREF color1 = bRibbon ? m_clrRibbonComboBtnStart : m_clrComboBtnStart;
		COLORREF color2 = bRibbon ? m_clrRibbonComboBtnFinish : m_clrComboBtnFinish;
		COLORREF colorBorder = bRibbon ? m_clrRibbonComboBtnBorder : m_clrComboBtnBorder;
		if (bDisabled)
		{
			color1 = bRibbon ? m_clrRibbonComboBtnDisabledStart : m_clrComboBtnDisabledStart;
			color2 = bRibbon ? m_clrRibbonComboBtnDisabledFinish : m_clrComboBtnDisabledFinish;
			colorBorder = bRibbon ? m_clrRibbonComboBtnBorderDisabled : m_clrComboBtnBorderDisabled;
		}
		else if (bActive)
		{
			if (bIsDropped)
			{
				color1 = bRibbon ? m_clrRibbonComboBtnPressedStart : m_clrComboBtnPressedStart;
				color2 = bRibbon ? m_clrRibbonComboBtnPressedFinish : m_clrComboBtnPressedFinish;
				colorBorder = bRibbon ? m_clrRibbonComboBtnBorderPressed : m_clrComboBtnBorderPressed;
			}
			else
			{
				color1 = bRibbon ? m_clrRibbonComboBtnHighlightedStart : m_clrComboBtnHighlightedStart;
				color2 = bRibbon ? m_clrRibbonComboBtnHighlightedFinish : m_clrComboBtnHighlightedFinish;
				colorBorder = bRibbon ? m_clrRibbonComboBtnBorderHighlighted : m_clrComboBtnBorderHighlighted;
			}
		}

		if (bRibbon || !bDisabled || (bDisabled && colorBorder != (COLORREF)(-1)))
		{
			if (!bDisabled)
			{
				rect.InflateRect(0, 1, 1, 1);
			}

			if (CMFCToolBarImages::m_bIsDrawOnGlass)
			{
				CDrawingManager dm(*pDC);
				dm.DrawRect(rect, (COLORREF)-1, colorBorder);
			}
			else
			{
				pDC->Draw3dRect(rect, colorBorder, colorBorder);
			}

			if (!bDisabled)
			{
				rect.DeflateRect(0, 1, 1, 1);
			}
		}

		if (bDisabled)
		{
			rect.DeflateRect(0, 1, 1, 1);
		}
		else if (bActive)
		{
			rect.DeflateRect(1, 0, 0, 0);
		}

		CDrawingManager dm(*pDC);
		dm.FillGradient(rect, color1, color2, TRUE);

		if (bDisabled)
		{
			rect.InflateRect(0, 1, 1, 1);
		}
		else if (bActive)
		{
			rect.InflateRect(1, 0, 0, 0);
		}
	}
	else
	{
		rect.InflateRect(0, 1, 1, 1);

		int nIndex = 0;
		if (bDisabled)
		{
			nIndex = 3;
		}
		else
		{
			if (bIsDropped)
			{
				nIndex = 2;
			}
			else if (bIsHighlighted)
			{
				nIndex = 1;
			}
		}

		pRenderer->Draw(pDC, rect, nIndex);

		rect.DeflateRect(0, 1, 1, 1);
	}

	rect.bottom -= 2;

	CMenuImages::Draw(pDC, CMenuImages::IdArrowDown, rect, bDisabled ? CMenuImages::ImageGray : CMenuImages::ImageBlack);
}

void CMFCVisualManagerOffice2007::OnDrawComboBorder(CDC* pDC, CRect rect, BOOL bDisabled, BOOL bIsDropped, BOOL bIsHighlighted, CMFCToolBarComboBoxButton* pButton)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawComboBorder(pDC, rect, bDisabled, bIsDropped, bIsHighlighted, pButton);
		return;
	}

	rect.DeflateRect(1, 1);

	COLORREF colorBorder = m_clrComboBorder;

	if (bDisabled)
	{
		colorBorder = m_clrComboBorderDisabled;
	}
	else if (bIsHighlighted || bIsDropped)
	{
		colorBorder = bIsDropped ? m_clrComboBorderPressed : m_clrComboBorderHighlighted;
	}

	if (CMFCToolBarImages::m_bIsDrawOnGlass)
	{
		CDrawingManager dm(*pDC);
		dm.DrawRect(rect, (COLORREF)-1, colorBorder);
	}
	else
	{
		pDC->Draw3dRect(&rect, colorBorder, colorBorder);
	}
}

void CMFCVisualManagerOffice2007::OnDrawEditBorder(CDC* pDC, CRect rect, BOOL bDisabled, BOOL bIsHighlighted, CMFCToolBarEditBoxButton* pButton)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawEditBorder(pDC, rect, bDisabled, bIsHighlighted, pButton);
		return;
	}

	rect.DeflateRect(1, 1);

	COLORREF colorBorder = m_clrEditBorder;

	if (bDisabled)
	{
		colorBorder = m_clrEditBorderDisabled;
	}
	else if (bIsHighlighted)
	{
		colorBorder = m_clrEditBorderHighlighted;
	}

	if (CMFCToolBarImages::m_bIsDrawOnGlass)
	{
		CDrawingManager dm(*pDC);
		dm.DrawRect(rect, (COLORREF)-1, colorBorder);
	}
	else
	{
		pDC->Draw3dRect(&rect, colorBorder, colorBorder);
	}
}

void CMFCVisualManagerOffice2007::OnDrawTearOffCaption(CDC* pDC, CRect rect, BOOL bIsActive)
{
	if (!CanDrawImage() || m_ToolBarTear.GetCount() == 0)
	{
		CMFCVisualManagerOffice2003::OnDrawTearOffCaption(pDC, rect, bIsActive);
		return;
	}

	pDC->FillRect(rect, &m_brBarBkgnd);
	if (bIsActive)
	{
		m_ctrlMenuHighlighted[0].Draw(pDC, rect);
	}

	m_ToolBarTear.DrawEx(pDC, rect, 0, CMFCToolBarImages::ImageAlignHorzCenter, CMFCToolBarImages::ImageAlignVertCenter);
}

void CMFCVisualManagerOffice2007::OnDrawMenuResizeBar(CDC* pDC, CRect rect, int nResizeFlags)
{
	CMFCToolBarImages& images = (nResizeFlags == (int) CMFCPopupMenu::MENU_RESIZE_BOTTOM_RIGHT) ? m_PopupResizeBar_HV :
	(nResizeFlags == (int) CMFCPopupMenu::MENU_RESIZE_TOP_RIGHT) ? m_PopupResizeBar_HVT : m_PopupResizeBar_V;

	if (!CanDrawImage() || !m_ctrlPopupResizeBar.IsValid() || !images.IsValid())
	{
		CMFCVisualManagerOffice2003::OnDrawMenuResizeBar(pDC, rect, nResizeFlags);
		return;
	}

	ASSERT_VALID(pDC);

	m_ctrlPopupResizeBar.Draw(pDC, rect);

	if (nResizeFlags == (int) CMFCPopupMenu::MENU_RESIZE_BOTTOM_RIGHT || nResizeFlags == (int) CMFCPopupMenu::MENU_RESIZE_TOP_RIGHT)
	{
		images.DrawEx(pDC, rect, 0, CMFCToolBarImages::ImageAlignHorzRight,
			nResizeFlags == (int) CMFCPopupMenu::MENU_RESIZE_TOP_RIGHT ? CMFCToolBarImages::ImageAlignVertTop : CMFCToolBarImages::ImageAlignVertBottom);
	}
	else
	{
		images.DrawEx(pDC, rect, 0, CMFCToolBarImages::ImageAlignHorzCenter, CMFCToolBarImages::ImageAlignVertCenter);
	}
}

void CMFCVisualManagerOffice2007::OnDrawMenuScrollButton(CDC* pDC, CRect rect, BOOL bIsScrollDown, BOOL bIsHighlited, BOOL bIsPressed, BOOL bIsDisabled)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawMenuScrollButton(pDC, rect, bIsScrollDown, bIsHighlited, bIsPressed, bIsDisabled);
		return;
	}

	ASSERT_VALID(pDC);

	CMFCControlRenderer* pRenderer = &m_ctrlMenuScrollBtn[0];

	if (bIsScrollDown && m_ctrlMenuScrollBtn[1].IsValid())
	{
		pRenderer = &m_ctrlMenuScrollBtn[1];
	}

	rect.top --;

	pRenderer->Draw(pDC, rect, bIsHighlited ? 1 : 0);

	CMenuImages::Draw(pDC, bIsScrollDown ? CMenuImages::IdArrowDown : CMenuImages::IdArrowUp, rect);
}

void CMFCVisualManagerOffice2007::OnDrawMenuSystemButton(CDC* pDC, CRect rect, UINT uiSystemCommand, UINT nStyle, BOOL bHighlight)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawMenuSystemButton(pDC, rect, uiSystemCommand, nStyle, bHighlight);
		return;
	}

	ASSERT_VALID(pDC);

	CMFCToolBarImages* pImage = NULL;

	switch(uiSystemCommand)
	{
	case SC_CLOSE:
		pImage = &m_SysBtnClose[0];
		break;

	case SC_MINIMIZE:
		pImage = &m_SysBtnMinimize[0];
		break;

	case SC_RESTORE:
		pImage = &m_SysBtnRestore[0];
		break;

	default:
		return;
	}

	BOOL bDisabled = (nStyle & TBBS_DISABLED);
	BOOL bPressed = (nStyle & TBBS_PRESSED);

	CRect rtBtnImage(CPoint(0, 0), pImage->GetImageSize());

	int nImage = 0;
	if (bDisabled)
	{
		nImage = 3;
	}
	else if (bPressed || bHighlight)
	{
		int index = -1;
		if (bPressed)
		{
			if (bHighlight)
			{
				index = 1;
			}
		}
		else if (bHighlight)
		{
			index = 0;
		}

		if (index != -1)
		{
			m_ctrlRibbonBtn[0].Draw(pDC, rect, index);
		}
	}

	rtBtnImage.OffsetRect(0, pImage->GetImageSize().cy * nImage);
	pImage->DrawEx(pDC, rect, 0, CMFCToolBarImages::ImageAlignHorzCenter, CMFCToolBarImages::ImageAlignVertCenter, rtBtnImage);
}

void CMFCVisualManagerOffice2007::OnFillButtonInterior(CDC* pDC, CMFCToolBarButton* pButton, CRect rect, CMFCVisualManager::AFX_BUTTON_STATE state)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnFillButtonInterior(pDC, pButton, rect, state);
		return;
	}

	CMFCCustomizeButton* pCustButton = DYNAMIC_DOWNCAST(CMFCCustomizeButton, pButton);

	if (pCustButton == NULL)
	{
		if (CMFCToolBar::IsCustomizeMode() && !CMFCToolBar::IsAltCustomizeMode() && !pButton->IsLocked())
		{
			return;
		}

		CMFCControlRenderer* pRenderer = NULL;
		int index = 0;

		BOOL bDisabled = (pButton->m_nStyle & TBBS_DISABLED) == TBBS_DISABLED;
		BOOL bPressed  = (pButton->m_nStyle & TBBS_PRESSED ) == TBBS_PRESSED;
		BOOL bChecked  = (pButton->m_nStyle & TBBS_CHECKED ) == TBBS_CHECKED;
		BOOL bHandled  = FALSE;

		CBasePane* pBar = DYNAMIC_DOWNCAST(CBasePane, pButton->GetParentWnd());

		CMFCToolBarMenuButton* pMenuButton = DYNAMIC_DOWNCAST(CMFCToolBarMenuButton, pButton);
		if (pMenuButton != NULL && pBar != NULL)
		{
			if (pBar->IsKindOf(RUNTIME_CLASS(CMFCMenuBar)))
			{
				if (state == ButtonsIsPressed || state == ButtonsIsHighlighted)
				{
					if (pMenuButton->IsDroppedDown())
					{
						ExtendMenuButton(pMenuButton, rect);
						index = 1;
					}

					pRenderer = &m_ctrlMenuBarBtn;

					bHandled = TRUE;
				}
				else
				{
					return;
				}

				bHandled = TRUE;
			}
			else if (pBar->IsKindOf(RUNTIME_CLASS(CMFCPopupMenuBar)))
			{
				if (bChecked)
				{
					pRenderer = &m_ctrlMenuItemBack;

					if (bDisabled)
					{
						index = 1;
					}

					rect.InflateRect(0, 0, 0, 1);
					bHandled = TRUE;
				}
				else if (state == ButtonsIsPressed || state == ButtonsIsHighlighted)
				{
					pRenderer = &m_ctrlMenuHighlighted[bDisabled ? 1 : 0];
					bHandled = TRUE;
				}
				else
				{
					return;
				}
			}
			else if (pBar->IsKindOf(RUNTIME_CLASS(CMFCToolBar)))
			{
				if (pMenuButton->IsDroppedDown())
				{
					ExtendMenuButton(pMenuButton, rect);
				}
			}
		}
		else if (pBar != NULL && pBar->IsKindOf(RUNTIME_CLASS(CMFCColorBar)))
		{
			if (bChecked)
			{
				pRenderer = &m_ctrlMenuItemBack;

				if (bDisabled)
				{
					index = 1;
				}
			}


			if (!bDisabled)
			{
				if (state == ButtonsIsHighlighted)
				{
					pRenderer = &m_ctrlMenuHighlighted[0];
					index = 0;
				}
			}

			bHandled = TRUE;
		}
		else if (pBar != NULL && pBar->IsKindOf(RUNTIME_CLASS(CMFCOutlookBarToolBar)))
		{
			bHandled = TRUE;
		}

		if (!bHandled)
		{
			index = -1;

			if (bChecked)
			{
				if (bDisabled)
				{
					index = 0;
				}
				else if (state == ButtonsIsPressed || state == ButtonsIsHighlighted)
				{
					index = 3;
				}
			}

			if (!bDisabled)
			{
				if (bPressed)
				{
					index = 2;
				}
				else if (state == ButtonsIsHighlighted)
				{
					if (index == -1)
					{
						index = 0;
					}

					index++;
				}
			}

			if (index == -1)
			{
				return;
			}

			pRenderer = &m_ctrlToolBarBtn;
		}

		if (pRenderer != NULL)
		{
			pRenderer->Draw(pDC, rect, index);
			return;
		}
	}

	CMFCVisualManagerOffice2003::OnFillButtonInterior(pDC, pButton, rect, state);
}

void CMFCVisualManagerOffice2007::OnDrawButtonBorder(CDC* pDC, CMFCToolBarButton* pButton, CRect rect, CMFCVisualManager::AFX_BUTTON_STATE state)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawButtonBorder(pDC, pButton, rect, state);
		return;
	}

	//------------------------------------------------
	// Draw shadow under the dropped-down menu button:
	//------------------------------------------------
	if (state != ButtonsIsPressed && state != ButtonsIsHighlighted)
	{
		return;
	}

	if (!m_bShdowDroppedDownMenuButton || !CMFCMenuBar::IsMenuShadows() || CMFCToolBar::IsCustomizeMode())
	{
		return;
	}

	CMFCToolBarMenuButton* pMenuButton = DYNAMIC_DOWNCAST(CMFCToolBarMenuButton, pButton);
	if (pMenuButton == NULL || !pMenuButton->IsDroppedDown())
	{
		return;
	}

	BOOL bIsPopupMenu = pMenuButton->GetParentWnd() != NULL && pMenuButton->GetParentWnd()->IsKindOf(RUNTIME_CLASS(CMFCPopupMenuBar));

	if (bIsPopupMenu)
	{
		return;
	}

	CMFCPopupMenu* pPopupMenu= pMenuButton->GetPopupMenu();
	if (pPopupMenu != NULL && (pPopupMenu->IsWindowVisible() || pPopupMenu->IsShown()) && !pPopupMenu->IsRightAlign() && !(pPopupMenu->GetExStyle() & WS_EX_LAYOUTRTL))
	{
		ExtendMenuButton(pMenuButton, rect);

		CDrawingManager dm(*pDC);

		dm.DrawShadow(rect, m_nMenuShadowDepth, 100, 75, NULL, NULL, m_clrMenuShadowBase);
	}
}

void CMFCVisualManagerOffice2007::OnDrawButtonSeparator(CDC* pDC, CMFCToolBarButton* pButton, CRect rect, CMFCVisualManager::AFX_BUTTON_STATE state, BOOL bHorz)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawButtonSeparator(pDC, pButton, rect, state, bHorz);
		return;
	}

	CPen* pPen = &m_penMenuItemBorder;

	CPen* pOldPen = pDC->SelectObject(pPen);
	ENSURE(pOldPen != NULL);

	if (bHorz)
	{
		pDC->MoveTo(rect.left, rect.top + 2);
		pDC->LineTo(rect.left, rect.bottom - 2);
	}
	else
	{
		pDC->MoveTo(rect.left  + 2, rect.top);
		pDC->LineTo(rect.right - 2, rect.top);
	}

	pDC->SelectObject(pOldPen);
}

void CMFCVisualManagerOffice2007::OnHighlightMenuItem(CDC *pDC, CMFCToolBarMenuButton* pButton, CRect rect, COLORREF& clrText)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnHighlightMenuItem(pDC, pButton, rect, clrText);
		return;
	}

	m_ctrlMenuHighlighted[(pButton->m_nStyle & TBBS_DISABLED) == TBBS_DISABLED ? 1 : 0].Draw(pDC, rect);
}

void CMFCVisualManagerOffice2007::OnHighlightRarelyUsedMenuItems(CDC* pDC, CRect rectRarelyUsed)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnHighlightRarelyUsedMenuItems(pDC, rectRarelyUsed);
	}

	rectRarelyUsed.left --;
	rectRarelyUsed.right = rectRarelyUsed.left + CMFCToolBar::GetMenuImageSize().cx + 2 * GetMenuImageMargin() + 2;

	pDC->FillRect(rectRarelyUsed, &m_brMenuRarelyUsed);
}

void CMFCVisualManagerOffice2007::OnDrawMenuCheck(CDC* pDC, CMFCToolBarMenuButton* pButton, CRect rect, BOOL bHighlight, BOOL bIsRadio)
{
	ASSERT_VALID(pButton);

	CMFCToolBarImages& img = bIsRadio ? m_MenuItemMarkerR : m_MenuItemMarkerC;

	if (!CanDrawImage() || img.GetCount() == 0)
	{
		CMFCVisualManagerOffice2003::OnDrawMenuCheck(pDC, pButton, rect, bHighlight, bIsRadio);
		return;
	}

	CSize size(img.GetImageSize());
	CRect rectImage(0, 0, size.cx, size.cy);

	if ((pButton->m_nStyle & TBBS_DISABLED) == TBBS_DISABLED)
	{
		rectImage.OffsetRect(0, size.cy);
	}

	if (afxGlobalData.m_bIsRTL)
	{
		img.Mirror();
	}

	img.DrawEx(pDC, rect, 0, CMFCToolBarImages::ImageAlignHorzCenter, CMFCToolBarImages::ImageAlignVertCenter, rectImage);

	if (afxGlobalData.m_bIsRTL)
	{
		img.Mirror();
	}
}

void CMFCVisualManagerOffice2007::OnDrawMenuItemButton(CDC* pDC, CMFCToolBarMenuButton* pButton, CRect rectButton, BOOL bHighlight, BOOL bDisabled)
{
	if (!CanDrawImage() || !m_ctrlMenuButtonBorder.IsValid())
	{
		CMFCVisualManagerOffice2003::OnDrawMenuItemButton(pDC, pButton, rectButton, bHighlight, bDisabled);
		return;
	}

	ASSERT_VALID(pDC);

	CRect rect = rectButton;
	rect.right = rect.left + 1;
	rect.left--;
	rect.DeflateRect(0, 1);

	if (bHighlight)
	{
		m_ctrlMenuButtonBorder.Draw(pDC, rect);
	}
	else
	{
		CBrush br(afxGlobalData.clrBtnShadow);

		rect.DeflateRect(0, 3);
		rect.right--;
		pDC->FillRect(rect, &br);
	}
}

void CMFCVisualManagerOffice2007::OnDrawShowAllMenuItems(CDC* pDC, CRect rect, CMFCVisualManager::AFX_BUTTON_STATE state)
{
	if (!CanDrawImage() || !m_ctrlMenuItemShowAll.IsValid())
	{
		CMFCVisualManagerOffice2003::OnDrawShowAllMenuItems(pDC, rect, state);
		return;
	}

	m_ctrlMenuItemShowAll.FillInterior(pDC, rect, CMFCToolBarImages::ImageAlignHorzCenter, CMFCToolBarImages::ImageAlignVertCenter, state == ButtonsIsHighlighted ? 1 : 0);
}

int CMFCVisualManagerOffice2007::GetShowAllMenuItemsHeight(CDC* pDC, const CSize& sizeDefault)
{
	return(CanDrawImage() && m_ctrlMenuItemShowAll.IsValid()) ?
		m_ctrlMenuItemShowAll.GetParams().m_rectImage.Size().cy + 2 * AFX_TEXT_MARGIN : CMFCVisualManagerOffice2003::GetShowAllMenuItemsHeight(pDC, sizeDefault);
}

COLORREF CMFCVisualManagerOffice2007::OnFillMiniFrameCaption(CDC* pDC, CRect rectCaption, CPaneFrameWnd* pFrameWnd, BOOL bActive)
{
	if (!CanDrawImage())
	{
		return CMFCVisualManagerOffice2003::OnFillMiniFrameCaption(pDC, rectCaption, pFrameWnd, bActive);
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID(pFrameWnd);

	BOOL bIsToolBar = FALSE;

	if (DYNAMIC_DOWNCAST(CMFCBaseToolBar, pFrameWnd->GetPane()) != NULL)
	{
		bActive = FALSE;
		bIsToolBar = TRUE;
	}

	pDC->FillRect(rectCaption, bActive ? &afxGlobalData.brActiveCaption : &afxGlobalData.brInactiveCaption);

	// get the text color
	return bActive ? afxGlobalData.clrCaptionText : afxGlobalData.clrInactiveCaptionText;
}

void CMFCVisualManagerOffice2007::OnDrawMiniFrameBorder(CDC* pDC, CPaneFrameWnd* pFrameWnd, CRect rectBorder, CRect rectBorderSize)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawMiniFrameBorder(pDC, pFrameWnd, rectBorder, rectBorderSize);
		return;
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID(pFrameWnd);

	BOOL bIsTasksPane = pFrameWnd->IsKindOf( RUNTIME_CLASS( CMFCTasksPaneFrameWnd ) );

	if (bIsTasksPane)
	{
		CBrush* pOldBrush = pDC->SelectObject(&m_brFloatToolBarBorder);
		ENSURE(pOldBrush != NULL);

		pDC->PatBlt(rectBorder.left, rectBorder.top, rectBorderSize.left, rectBorder.Height(), PATCOPY);
		pDC->PatBlt(rectBorder.left, rectBorder.top, rectBorder.Width(), rectBorderSize.top, PATCOPY);
		pDC->PatBlt(rectBorder.right - rectBorderSize.right, rectBorder.top, rectBorderSize.right, rectBorder.Height(), PATCOPY);
		pDC->PatBlt(rectBorder.left, rectBorder.bottom - rectBorderSize.bottom, rectBorder.Width(), rectBorderSize.bottom, PATCOPY);

		rectBorderSize.DeflateRect(2, 2);
		rectBorder.DeflateRect(2, 2);

		pDC->SelectObject(&afxGlobalData.brBarFace);

		pDC->PatBlt(rectBorder.left, rectBorder.top + 1, rectBorderSize.left, rectBorder.Height() - 1, PATCOPY);
		pDC->PatBlt(rectBorder.left + 1, rectBorder.top, rectBorder.Width() - 2, rectBorderSize.top, PATCOPY);
		pDC->PatBlt(rectBorder.right - rectBorderSize.right, rectBorder.top + 1, rectBorderSize.right, rectBorder.Height() - 1, PATCOPY);
		pDC->PatBlt(rectBorder.left + 1, rectBorder.bottom - rectBorderSize.bottom, rectBorder.Width() - 2, rectBorderSize.bottom, PATCOPY);

		pDC->SelectObject(pOldBrush);
	}
	else
	{
		CMFCVisualManagerOffice2003::OnDrawMiniFrameBorder(pDC, pFrameWnd, rectBorder, rectBorderSize);
	}
}

void CMFCVisualManagerOffice2007::OnDrawFloatingToolbarBorder(CDC* pDC, CMFCBaseToolBar* pToolBar, CRect rectBorder, CRect rectBorderSize)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawFloatingToolbarBorder(pDC, pToolBar, rectBorder, rectBorderSize);
		return;
	}

	ASSERT_VALID(pDC);

	CBrush* pOldBrush = pDC->SelectObject(&m_brFloatToolBarBorder);
	ENSURE(pOldBrush != NULL);

	pDC->PatBlt(rectBorder.left, rectBorder.top, rectBorderSize.left, rectBorder.Height(), PATCOPY);
	pDC->PatBlt(rectBorder.left, rectBorder.top, rectBorder.Width(), rectBorderSize.top, PATCOPY);
	pDC->PatBlt(rectBorder.right - rectBorderSize.right, rectBorder.top, rectBorderSize.right, rectBorder.Height(), PATCOPY);
	pDC->PatBlt(rectBorder.left, rectBorder.bottom - rectBorderSize.bottom, rectBorder.Width(), rectBorderSize.bottom, PATCOPY);

	rectBorderSize.DeflateRect(2, 2);
	rectBorder.DeflateRect(2, 2);

	pDC->SelectObject(&afxGlobalData.brBarFace);

	pDC->PatBlt(rectBorder.left, rectBorder.top + 1, rectBorderSize.left, rectBorder.Height() - 1, PATCOPY);
	pDC->PatBlt(rectBorder.left + 1, rectBorder.top, rectBorder.Width() - 2, rectBorderSize.top, PATCOPY);
	pDC->PatBlt(rectBorder.right - rectBorderSize.right, rectBorder.top + 1, rectBorderSize.right, rectBorder.Height() - 1, PATCOPY);
	pDC->PatBlt(rectBorder.left + 1, rectBorder.bottom - rectBorderSize.bottom, rectBorder.Width() - 2, rectBorderSize.bottom, PATCOPY);

	pDC->SelectObject(pOldBrush);
}

BOOL CMFCVisualManagerOffice2007::IsOwnerDrawMenuCheck()
{
	return CanDrawImage() ? FALSE : CMFCVisualManagerOffice2003::IsOwnerDrawMenuCheck();
}

BOOL CMFCVisualManagerOffice2007::IsHighlightWholeMenuItem()
{
	return CanDrawImage() ? TRUE : CMFCVisualManagerOffice2003::IsHighlightWholeMenuItem();
}

COLORREF CMFCVisualManagerOffice2007::GetStatusBarPaneTextColor(CMFCStatusBar* pStatusBar, CMFCStatusBarPaneInfo* pPane)
{
	if (!CanDrawImage())
	{
		return CMFCVisualManagerOffice2003::GetStatusBarPaneTextColor(pStatusBar, pPane);
	}

	ENSURE(pPane != NULL);

	return(pPane->nStyle & SBPS_DISABLED) ? m_clrStatusBarTextDisabled : pPane->clrText == (COLORREF)-1 ? m_clrStatusBarText : pPane->clrText;
}

COLORREF CMFCVisualManagerOffice2007::GetToolbarButtonTextColor(CMFCToolBarButton* pButton, CMFCVisualManager::AFX_BUTTON_STATE state)
{
	if (!CanDrawImage())
	{
		return CMFCVisualManagerOffice2003::GetToolbarButtonTextColor(pButton, state);
	}

	ASSERT_VALID(pButton);

	BOOL bDisabled = (CMFCToolBar::IsCustomizeMode() && !pButton->IsEditable()) || (!CMFCToolBar::IsCustomizeMode() &&(pButton->m_nStyle & TBBS_DISABLED));

	if (pButton->GetParentWnd() != NULL && pButton->GetParentWnd()->IsKindOf(RUNTIME_CLASS(CMFCMenuBar)))
	{
		if (CMFCToolBar::IsCustomizeMode())
		{
			return m_clrMenuBarBtnText;
		}

		return bDisabled ? m_clrMenuBarBtnTextDisabled :
			((state == ButtonsIsHighlighted || state == ButtonsIsPressed || pButton->IsDroppedDown()) ? m_clrMenuBarBtnTextHighlighted : m_clrMenuBarBtnText);
	}

	return bDisabled ? m_clrToolBarBtnTextDisabled :
		((state == ButtonsIsHighlighted || state == ButtonsIsPressed) ? m_clrToolBarBtnTextHighlighted : m_clrToolBarBtnText);
}

COLORREF CMFCVisualManagerOffice2007::GetMenuItemTextColor(CMFCToolBarMenuButton* pButton, BOOL bHighlighted, BOOL bDisabled)
{
	if (!CanDrawImage())
	{
		return CMFCVisualManagerOffice2003::GetMenuItemTextColor(pButton, bHighlighted, bDisabled);
	}

	return bDisabled ? m_clrMenuTextDisabled : m_clrMenuText;
}

COLORREF CMFCVisualManagerOffice2007::GetHighlightedMenuItemTextColor(CMFCToolBarMenuButton* pButton)
{
	if (!CanDrawImage())
	{
		return CMFCVisualManagerOffice2003::GetHighlightedMenuItemTextColor(pButton);
	}

	return m_clrMenuTextHighlighted;
}

void CMFCVisualManagerOffice2007::GetTabFrameColors(const CMFCBaseTabCtrl* pTabWnd, COLORREF& clrDark, COLORREF& clrBlack, COLORREF& clrHighlight,
	COLORREF& clrFace, COLORREF& clrDarkShadow, COLORREF& clrLight, CBrush*& pbrFace, CBrush*& pbrBlack)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::GetTabFrameColors(pTabWnd, clrDark, clrBlack, clrHighlight, clrFace, clrDarkShadow, clrLight, pbrFace, pbrBlack);
		return;
	}

	ASSERT_VALID(pTabWnd);

	CMFCVisualManagerOffice2003::GetTabFrameColors(pTabWnd, clrDark, clrBlack, clrHighlight, clrFace, clrDarkShadow, clrLight, pbrFace, pbrBlack);

	if (pTabWnd->IsFlatTab() && !pTabWnd->IsDialogControl())
	{
		if (m_clrTabFlatBlack != CLR_DEFAULT)
		{
			clrBlack = m_clrTabFlatBlack;
		}

		if (m_clrTabFlatHighlight != CLR_DEFAULT)
		{
			clrHighlight = m_clrTabFlatHighlight;
		}
	}
}

void CMFCVisualManagerOffice2007::OnEraseTabsArea(CDC* pDC, CRect rect, const CMFCBaseTabCtrl* pTabWnd)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pTabWnd);

	if (!CanDrawImage() || pTabWnd->IsDialogControl())
	{
		CMFCVisualManagerOffice2003::OnEraseTabsArea(pDC, rect, pTabWnd);
		return;
	}

	if (pTabWnd->IsOneNoteStyle() || pTabWnd->IsColored() || pTabWnd->IsVS2005Style() || pTabWnd->IsLeftRightRounded())
	{
		CMFCVisualManagerOffice2003::OnEraseTabsArea(pDC, rect, pTabWnd);
		return;
	}

	const BOOL bBottom = pTabWnd->GetLocation() == CMFCTabCtrl::LOCATION_BOTTOM;

	if (pTabWnd->IsFlatTab())
	{
		m_ctrlTabFlat[bBottom ? 1 : 0].Draw(pDC, rect);
	}
	else
	{
		CDrawingManager dm(*pDC);

		COLORREF clr1 = m_clrBarGradientDark;
		COLORREF clr2 = m_clrBarGradientLight;

		if (bBottom)
		{
			dm.FillGradient(rect, clr1, clr2, TRUE);
		}
		else
		{
			dm.FillGradient(rect, clr2, clr1, TRUE);
		}
	}
}

void CMFCVisualManagerOffice2007::OnDrawTab(CDC* pDC, CRect rectTab, int iTab, BOOL bIsActive, const CMFCBaseTabCtrl* pTabWnd)
{
	ASSERT_VALID(pTabWnd);
	ASSERT_VALID(pDC);

	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawTab(pDC, rectTab, iTab, bIsActive, pTabWnd);
		return;
	}

	if (pTabWnd->IsOneNoteStyle() || pTabWnd->IsColored() || pTabWnd->IsVS2005Style() || pTabWnd->IsLeftRightRounded())
	{
		CMFCVisualManagerOffice2003::OnDrawTab(pDC, rectTab, iTab, bIsActive, pTabWnd);
		return;
	}

	const BOOL bBottom = pTabWnd->GetLocation() == CMFCTabCtrl::LOCATION_BOTTOM;
	const BOOL bIsHighlight = iTab == pTabWnd->GetHighlightedTab();

	COLORREF clrText = pTabWnd->GetTabTextColor(iTab);

	if (pTabWnd->IsFlatTab())
	{
		int nImage = (bIsActive || bIsHighlight) ? 2 : 1;

		CRgn rgn;

		POINT pts[4];

		if (bBottom)
		{
			rectTab.bottom++;

			pts[0].x = rectTab.left;
			pts[0].y = rectTab.bottom + 1;
			pts[1].x = rectTab.left;
			pts[1].y = rectTab.top;
			pts[2].x = rectTab.right + 1;
			pts[2].y = rectTab.top;
			pts[3].x = rectTab.right - rectTab.Height() + 1;
			pts[3].y = rectTab.bottom + 1;

			rectTab.top++;
		}
		else
		{
			pts[0].x = rectTab.left;
			pts[0].y = rectTab.bottom + 1;
			pts[1].x = rectTab.left;
			pts[1].y = rectTab.top;
			pts[2].x = rectTab.right - rectTab.Height() + 1;
			pts[2].y = rectTab.top;
			pts[3].x = rectTab.right + 1;
			pts[3].y = rectTab.bottom + 1;
		}

		rgn.CreatePolygonRgn(pts, 4, WINDING);

		int isave = pDC->SaveDC();

		pDC->SelectClipRgn(&rgn, RGN_AND);

		m_ctrlTabFlat[bBottom ? 1 : 0].Draw(pDC, rectTab, nImage);

		CPen* pOldPen = pDC->SelectObject(&m_penTabFlatOuter[bIsActive ? 1 : 0]);

		if (bBottom)
		{
			pDC->MoveTo(pts[2].x, pts[2].y);
			pDC->LineTo(pts[3].x, pts[3].y - 1);
		}
		else
		{
			pDC->MoveTo(pts[2].x - 1, pts[2].y);
			pDC->LineTo(pts[3].x - 1, pts[3].y - 1);
		}

		pDC->SelectObject(&m_penTabFlatInner[bIsActive ? 1 : 0]);

		if (bBottom)
		{
			pDC->MoveTo(pts[2].x - 2, pts[2].y + 1);
			pDC->LineTo(pts[3].x, pts[3].y - 2);
		}
		else
		{
			pDC->MoveTo(pts[2].x - 1, pts[2].y + 1);
			pDC->LineTo(pts[3].x - 2, pts[3].y - 1);
		}

		pDC->SelectObject(pOldPen);

		pDC->SelectClipRgn(NULL);

		clrText = afxGlobalData.clrBarText;
		pDC->RestoreDC(isave);
	}
	else
	{
		if (clrText == (COLORREF)-1)
		{
			clrText = bIsActive ? m_clrMenuBarBtnTextHighlighted : m_clrMenuBarBtnText;
		}

		int nImage = bIsActive ? 3 : 0;
		if (bIsHighlight)
		{
			nImage += 1;
		}

		m_ctrlTab3D[bBottom ? 1 : 0].Draw(pDC, rectTab, nImage);

		if (pTabWnd->IsDialogControl())
		{
			clrText = afxGlobalData.clrBtnText;
		}
	}

	OnDrawTabContent(pDC, rectTab, iTab, bIsActive, pTabWnd, clrText);
}

void CMFCVisualManagerOffice2007::OnFillTab(CDC* pDC, CRect rectFill, CBrush* pbrFill, int iTab, BOOL bIsActive, const CMFCBaseTabCtrl* pTabWnd)
{
	ASSERT_VALID(pTabWnd);

	if (!CanDrawImage() || pTabWnd->IsDialogControl())
	{
		CMFCVisualManagerOffice2003::OnFillTab(pDC, rectFill, pbrFill, iTab, bIsActive, pTabWnd);
		return;
	}

	if (pTabWnd->IsFlatTab() || pTabWnd->IsOneNoteStyle() || pTabWnd->IsColored() || pTabWnd->IsVS2005Style() || pTabWnd->IsLeftRightRounded())
	{
		CMFCVisualManagerOffice2003::OnFillTab(pDC, rectFill, pbrFill, iTab, bIsActive, pTabWnd);
		return;
	}

	ASSERT_VALID(pDC);

	const BOOL bBottom = pTabWnd->GetLocation() == CMFCTabCtrl::LOCATION_BOTTOM;
	const BOOL bIsHighlight = iTab == pTabWnd->GetHighlightedTab();

	BOOL bIsBeta = IsBeta();
	if (!bIsBeta ||(bIsActive || bIsHighlight))
	{
		int nImage = bIsActive ?(bIsBeta ? 2 : 3) :(bIsBeta ? -1 : 0);
		if (bIsHighlight)
		{
			nImage += 1;
		}

		m_ctrlTab3D[bBottom ? 1 : 0].Draw(pDC, rectFill, nImage);
	}
}

COLORREF CMFCVisualManagerOffice2007::GetTabTextColor(const CMFCBaseTabCtrl* pTabWnd, int iTab, BOOL bIsActive)
{
	if (!CanDrawImage() || pTabWnd->IsDialogControl())
	{
		return CMFCVisualManagerOffice2003::GetTabTextColor(pTabWnd, iTab, bIsActive);
	}

	ASSERT_VALID(pTabWnd);

	if (pTabWnd->IsOneNoteStyle() || pTabWnd->GetTabBkColor(iTab) != (COLORREF)-1)
	{
		return CMFCVisualManagerOffice2003::GetTabTextColor(pTabWnd, iTab, bIsActive);
	}

	return bIsActive ? m_clrTabTextActive : m_clrTabTextInactive;
}

int CMFCVisualManagerOffice2007::GetTabHorzMargin(const CMFCBaseTabCtrl* pTabWnd)
{
	if (!CanDrawImage())
	{
		return CMFCVisualManagerOffice2003::GetTabHorzMargin(pTabWnd);
	}

	CMFCControlRenderer* pRenderer = pTabWnd->IsFlatTab() ? &m_ctrlTabFlat[0] : &m_ctrlTab3D[0];

	if (pTabWnd->IsOneNoteStyle() || pTabWnd->IsColored() || pTabWnd->IsVS2005Style() || pTabWnd->IsLeftRightRounded() || !pRenderer->IsValid())
	{
		return CMFCVisualManagerOffice2003::GetTabHorzMargin(pTabWnd);
	}

	return pRenderer->GetParams().m_rectSides.right / 2;
}

BOOL CMFCVisualManagerOffice2007::OnEraseTabsFrame(CDC* pDC, CRect rect, const CMFCBaseTabCtrl* pTabWnd)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pTabWnd);

	if (!CanDrawImage() || pTabWnd->IsDialogControl())
	{
		return CMFCVisualManagerOffice2003::OnEraseTabsFrame(pDC, rect, pTabWnd);
	}

	if (pTabWnd->IsOneNoteStyle() || pTabWnd->IsColored() || pTabWnd->IsVS2005Style() || pTabWnd->IsLeftRightRounded())
	{
		return CMFCVisualManagerOffice2003::OnEraseTabsFrame(pDC, rect, pTabWnd);
	}

	if (pTabWnd->IsFlatTab())
	{
		pDC->FillRect(rect, &afxGlobalData.brWindow);

		if (pTabWnd->GetLocation() != CMFCTabCtrl::LOCATION_BOTTOM)
		{
			CPen pen(PS_SOLID, 1, m_clrTabFlatBlack);
			CPen* pOldPen = pDC->SelectObject(&pen);

			pDC->MoveTo(rect.left, rect.top + pTabWnd->GetTabsHeight() + 1);
			pDC->LineTo(rect.right, rect.top + pTabWnd->GetTabsHeight() + 1);

			pDC->SelectObject(pOldPen);
		}

		return TRUE;
	}

	return FALSE;
}

void CMFCVisualManagerOffice2007::OnEraseTabsButton(CDC* pDC, CRect rect, CMFCButton* pButton, CMFCBaseTabCtrl* pBaseTab)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);
	ASSERT_VALID(pBaseTab);

	CMFCTabCtrl* pWndTab = DYNAMIC_DOWNCAST(CMFCTabCtrl, pBaseTab);

	if (!CanDrawImage() || pWndTab == NULL || pBaseTab->IsDialogControl())
	{
		CMFCVisualManagerOffice2003::OnEraseTabsButton(pDC, rect, pButton, pBaseTab);
		return;
	}

	if (pBaseTab->IsFlatTab() || pBaseTab->IsOneNoteStyle() || pBaseTab->IsColored() ||
		pBaseTab->IsVS2005Style() || pBaseTab->IsLeftRightRounded() || (!pButton->IsPressed() && !pButton->IsHighlighted()))
	{
		CMFCVisualManagerOffice2003::OnEraseTabsButton(pDC, rect, pButton, pBaseTab);
		return;
	}

	CRgn rgn;
	rgn.CreateRectRgnIndirect(rect);

	pDC->SelectClipRgn(&rgn);

	CRect rectTabs;
	pWndTab->GetClientRect(&rectTabs);

	CRect rectTabArea;
	pWndTab->GetTabsRect(rectTabArea);

	if (pWndTab->GetLocation() == CMFCBaseTabCtrl::LOCATION_BOTTOM)
	{
		rectTabs.top = rectTabArea.top;
	}
	else
	{
		rectTabs.bottom = rectTabArea.bottom;
	}

	pWndTab->MapWindowPoints(pButton, rectTabs);
	OnEraseTabsArea(pDC, rectTabs, pWndTab);

	pDC->SelectClipRgn(NULL);

	int index = pButton->IsPressed() ? 2 : 1;
	m_ctrlToolBarBtn.Draw(pDC, rect, index);
}

void CMFCVisualManagerOffice2007::OnDrawTabsButtonBorder(CDC* pDC, CRect& rect, CMFCButton* pButton, UINT uiState, CMFCBaseTabCtrl* pWndTab)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawTabsButtonBorder(pDC, rect, pButton, uiState, pWndTab);
	}
}

void CMFCVisualManagerOffice2007::OnDrawTasksGroupCaption(CDC* pDC, CMFCTasksPaneTaskGroup* pGroup, BOOL bIsHighlighted /*= FALSE*/, BOOL bIsSelected /*= FALSE*/, BOOL bCanCollapse /*= FALSE*/)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pGroup);
	ASSERT_VALID(pGroup->m_pPage);

	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawTasksGroupCaption(pDC, pGroup, bIsHighlighted, bIsSelected, bCanCollapse);
		return;
	}

	CRect rectGroup = pGroup->m_rect;

	// -----------------------
	// Draw caption background
	// -----------------------
	CDrawingManager dm(*pDC);

	if (pGroup->m_bIsSpecial)
	{
		if (IsBeta())
		{
			dm.FillGradient(pGroup->m_rect, bIsHighlighted ? m_clrTaskPaneGroupCaptionHighSpecLight : m_clrTaskPaneGroupCaptionSpecLight,
				bIsHighlighted ? m_clrTaskPaneGroupCaptionHighSpecDark : m_clrTaskPaneGroupCaptionSpecDark, TRUE);
		}
		else
		{
			dm.Fill4ColorsGradient(pGroup->m_rect, bIsHighlighted ? m_clrTaskPaneGroupCaptionHighSpecDark  : m_clrTaskPaneGroupCaptionSpecDark,
				bIsHighlighted ? m_clrTaskPaneGroupCaptionHighSpecLight : m_clrTaskPaneGroupCaptionSpecLight,
				bIsHighlighted ? m_clrTaskPaneGroupCaptionHighSpecLight : m_clrTaskPaneGroupCaptionSpecLight,
				bIsHighlighted ? m_clrTaskPaneGroupCaptionHighSpecDark  : m_clrTaskPaneGroupCaptionSpecDark, FALSE);
		}
	}
	else
	{
		if (IsBeta())
		{
			dm.FillGradient(pGroup->m_rect, bIsHighlighted ? m_clrTaskPaneGroupCaptionHighLight : m_clrTaskPaneGroupCaptionLight,
				bIsHighlighted ? m_clrTaskPaneGroupCaptionHighDark : m_clrTaskPaneGroupCaptionDark, TRUE);
		}
		else
		{
			dm.Fill4ColorsGradient(pGroup->m_rect, bIsHighlighted ? m_clrTaskPaneGroupCaptionHighDark  : m_clrTaskPaneGroupCaptionDark,
				bIsHighlighted ? m_clrTaskPaneGroupCaptionHighLight : m_clrTaskPaneGroupCaptionLight,
				bIsHighlighted ? m_clrTaskPaneGroupCaptionHighLight : m_clrTaskPaneGroupCaptionLight,
				bIsHighlighted ? m_clrTaskPaneGroupCaptionHighDark  : m_clrTaskPaneGroupCaptionDark, FALSE);
		}
	}

	// ---------------------------
	// Draw an icon if it presents
	// ---------------------------
	BOOL bShowIcon = (pGroup->m_hIcon != NULL && pGroup->m_sizeIcon.cx < rectGroup.Width() - rectGroup.Height());
	if (bShowIcon)
	{
		OnDrawTasksGroupIcon(pDC, pGroup, 5, bIsHighlighted, bIsSelected, bCanCollapse);
	}

	// -----------------------
	// Draw group caption text
	// -----------------------
	CFont* pFontOld = pDC->SelectObject(&afxGlobalData.fontBold);
	COLORREF clrTextOld = pDC->GetTextColor();

	if (bCanCollapse && bIsHighlighted)
	{
		pDC->SetTextColor(pGroup->m_clrTextHot == (COLORREF)-1 ?
			(pGroup->m_bIsSpecial ? m_clrTaskPaneGroupCaptionTextHighSpec : m_clrTaskPaneGroupCaptionTextHigh) : pGroup->m_clrTextHot);
	}
	else
	{
		pDC->SetTextColor(pGroup->m_clrText == (COLORREF)-1 ?
			(pGroup->m_bIsSpecial ? m_clrTaskPaneGroupCaptionTextSpec : m_clrTaskPaneGroupCaptionText) : pGroup->m_clrText);
	}

	int nBkModeOld = pDC->SetBkMode(TRANSPARENT);

	int nTaskPaneHOffset = pGroup->m_pPage->m_pTaskPane->GetGroupCaptionHorzOffset();
	int nTaskPaneVOffset = pGroup->m_pPage->m_pTaskPane->GetGroupCaptionVertOffset();
	int nCaptionHOffset = (nTaskPaneHOffset != -1 ? nTaskPaneHOffset : m_nGroupCaptionHorzOffset);

	CRect rectText = rectGroup;
	rectText.left += (bShowIcon ? pGroup->m_sizeIcon.cx + 5: nCaptionHOffset);
	rectText.top += (nTaskPaneVOffset != -1 ? nTaskPaneVOffset : m_nGroupCaptionVertOffset);
	rectText.right = max(rectText.left, rectText.right -(bCanCollapse ? rectGroup.Height() : nCaptionHOffset));

	pDC->DrawText(pGroup->m_strName, rectText, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

	pDC->SetBkMode(nBkModeOld);
	pDC->SelectObject(pFontOld);
	pDC->SetTextColor(clrTextOld);

	// -------------------------
	// Draw group caption button
	// -------------------------
	if (bCanCollapse && !pGroup->m_strName.IsEmpty())
	{
		CSize sizeButton = CMenuImages::Size();
		CRect rectButton = rectGroup;
		rectButton.left = max(rectButton.left, rectButton.right -(rectButton.Height() + 1) / 2 -(sizeButton.cx + 1) / 2);
		rectButton.top = max(rectButton.top, rectButton.bottom -(rectButton.Height() + 1) / 2 -(sizeButton.cy + 1) / 2);
		rectButton.right = rectButton.left + sizeButton.cx;
		rectButton.bottom = rectButton.top + sizeButton.cy;

		if (rectButton.right <= rectGroup.right && rectButton.bottom <= rectGroup.bottom)
		{
			if (bIsHighlighted)
			{
				// Draw button frame
				CBrush* pBrushOld = (CBrush*) pDC->SelectObject(&afxGlobalData.brBarFace);
				COLORREF clrBckOld = pDC->GetBkColor();

				pDC->Draw3dRect(&rectButton, afxGlobalData.clrWindow, afxGlobalData.clrBarShadow);

				pDC->SetBkColor(clrBckOld);
				pDC->SelectObject(pBrushOld);
			}

			CMenuImages::Draw(pDC, pGroup->m_bIsCollapsed ? CMenuImages::IdArrowDown : CMenuImages::IdArrowUp, rectButton.TopLeft(), CMenuImages::ImageBlack);
		}
	}
}

void CMFCVisualManagerOffice2007::OnDrawTask(CDC* pDC, CMFCTasksPaneTask* pTask, CImageList* pIcons, BOOL bIsHighlighted /*= FALSE*/, BOOL bIsSelected /*= FALSE*/)
{
	ASSERT_VALID(pTask);

	if (CanDrawImage() && pTask->m_bIsSeparator)
	{
		DrawSeparator(pDC, pTask->m_rect, TRUE);
		return;
	}

	CMFCVisualManagerOffice2003::OnDrawTask(pDC, pTask, pIcons, bIsHighlighted, bIsSelected);
}

void CMFCVisualManagerOffice2007::OnDrawScrollButtons(CDC* pDC, const CRect& rect, const int nBorderSize, int iImage, BOOL bHilited)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawScrollButtons(pDC, rect, nBorderSize, iImage, bHilited);
		return;
	}

	CRect rt(rect);
	rt.top--;
	m_ctrlTaskScrollBtn.Draw(pDC, rt, bHilited ? 1 : 0);

	CMenuImages::Draw(pDC, (CMenuImages::IMAGES_IDS) iImage, rect);
}

void CMFCVisualManagerOffice2007::OnDrawHeaderCtrlBorder(CMFCHeaderCtrl* pCtrl, CDC* pDC, CRect& rect, BOOL bIsPressed, BOOL bIsHighlighted)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawHeaderCtrlBorder(pCtrl, pDC, rect, bIsPressed, bIsHighlighted);
		return;
	}

	COLORREF clrStart  = m_clrHeaderNormalStart;
	COLORREF clrFinish = m_clrHeaderNormalFinish;
	COLORREF clrBorder = m_clrHeaderNormalBorder;

	if (bIsPressed)
	{
		clrStart  = m_clrHeaderPressedStart;
		clrFinish = m_clrHeaderPressedFinish;
		clrBorder = m_clrHeaderPressedBorder;
	}
	else if (bIsHighlighted)
	{
		clrStart  = m_clrHeaderHighlightedStart;
		clrFinish = m_clrHeaderHighlightedFinish;
		clrBorder = m_clrHeaderHighlightedBorder;
	}

	{
		CDrawingManager dm(*pDC);
		dm.FillGradient(rect, clrFinish, clrStart);
	}

	CPen pen(PS_SOLID, 0, clrBorder);
	CPen* pOldPen = pDC->SelectObject(&pen);

	if (bIsPressed || bIsHighlighted)
	{
		pDC->MoveTo(rect.right - 1, rect.top);
		pDC->LineTo(rect.right - 1, rect.bottom - 1);
		pDC->LineTo(rect.left, rect.bottom - 1);
		pDC->LineTo(rect.left, rect.top - 1);
	}
	else
	{
		pDC->MoveTo(rect.right - 1, rect.top);
		pDC->LineTo(rect.right - 1, rect.bottom - 1);
		pDC->LineTo(rect.left - 1, rect.bottom - 1);
	}

	pDC->SelectObject(pOldPen);
}

void CMFCVisualManagerOffice2007::OnDrawCheckBoxEx(CDC *pDC, CRect rect, int nState, BOOL bHighlighted, BOOL bPressed, BOOL bEnabled)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawCheckBoxEx(pDC, rect, nState, bHighlighted, bPressed, bEnabled);
		return;
	}

	int index = nState * 4;

	if (!bEnabled)
	{
		index += 3;
	}
	else if (bPressed)
	{
		if (bHighlighted)
		{
			index += 2;
		}
	}
	else if (bHighlighted)
	{
		index += 1;
	}

	if (afxGlobalData.m_bIsRTL)
	{
		m_ctrlRibbonBtnCheck.Mirror();
	}

	m_ctrlRibbonBtnCheck.FillInterior(pDC, rect, afxGlobalData.GetRibbonImageScale() != 1. ? CMFCToolBarImages::ImageAlignHorzStretch : CMFCToolBarImages::ImageAlignHorzCenter,
		afxGlobalData.GetRibbonImageScale() != 1. ? CMFCToolBarImages::ImageAlignVertStretch : CMFCToolBarImages::ImageAlignVertCenter, index);

	if (afxGlobalData.m_bIsRTL)
	{
		m_ctrlRibbonBtnCheck.Mirror();
	}
}

void CMFCVisualManagerOffice2007::OnDrawRibbonCaption(CDC* pDC, CMFCRibbonBar* pBar, CRect rectCaption, CRect rectText)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawRibbonCaption(pDC, pBar, rectCaption, rectText);
		return;
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID(pBar);

	CWnd* pWnd = pBar->GetParent();
	ASSERT_VALID(pWnd);

	const DWORD dwStyle   = pWnd->GetStyle();
	const DWORD dwStyleEx = pWnd->GetExStyle();

	const BOOL bIsRTL     = (dwStyleEx & WS_EX_LAYOUTRTL) == WS_EX_LAYOUTRTL;
	const BOOL bActive    = IsWindowActive(pWnd);
	const BOOL bGlass	  = pBar->IsTransparentCaption();

	//int nExtraWidth = 0;

	{
		CSize szSysBorder(GetSystemBorders(TRUE));
		CRect rectCaption1(rectCaption);
		CRect rectBorder(m_ctrlMainBorderCaption.GetParams().m_rectSides);
		CRect rectQAT = pBar->GetQuickAccessToolbarLocation();

		if (rectQAT.left > rectQAT.right)
		{
			rectText.left = rectQAT.left + 1;
		}

		rectCaption1.InflateRect(szSysBorder.cx, szSysBorder.cy, szSysBorder.cx, 0);

		BOOL bHide  = (pBar->GetHideFlags() & AFX_RIBBONBAR_HIDE_ALL) != 0;
		BOOL bExtra = !bHide && pBar->IsQuickAccessToolbarOnTop() && rectQAT.left < rectQAT.right && (!pBar->IsQATEmpty() || IsBeta1());

		if (!bGlass)
		{
			if (IsBeta())
			{
				COLORREF clr1  = bActive ? m_clrAppCaptionActiveStart : m_clrAppCaptionInactiveStart;
				COLORREF clr2  = bActive ? m_clrAppCaptionActiveFinish : m_clrAppCaptionInactiveFinish;

				CRect rectCaption2(rectCaption1);
				rectCaption2.DeflateRect(rectBorder.left, rectBorder.top, rectBorder.right, rectBorder.bottom);

				CDrawingManager dm(*pDC);
				dm.Fill4ColorsGradient(rectCaption2, clr1, clr2, clr2, clr1, FALSE);

				m_ctrlMainBorderCaption.DrawFrame(pDC, rectCaption1, bActive ? 0 : 1);
			}
			else
			{
				m_ctrlMainBorderCaption.Draw(pDC, rectCaption1, bActive ? 0 : 1);
			}
		}

		if (bExtra)
		{
			CMFCControlRenderer* pCaptionQA = bGlass ? &m_ctrlRibbonCaptionQA_Glass : &m_ctrlRibbonCaptionQA;

			if (pCaptionQA->IsValid())
			{
				const CMFCControlRendererInfo& params = pCaptionQA->GetParams();

				CRect rectQAFrame(rectQAT);
				rectQAFrame.InflateRect(params.m_rectCorners.left - 2, 1, 0, 1);
				rectQAFrame.right   = pBar->GetQATCommandsLocation().right + GetRibbonQuickAccessToolBarRightMargin() + 1;

				if (rectQAFrame.Height() < params.m_rectImage.Height())
				{
					rectQAFrame.top = rectQAFrame.bottom - params.m_rectImage.Height();
				}

				if (bGlass)
				{
					const int dxFrame = GetSystemMetrics(SM_CXSIZEFRAME) / 2;

					const int nTop = afxGlobalData.GetRibbonImageScale () != 1. ? -2 : 1;
					rectQAFrame.DeflateRect (1, nTop, dxFrame, 0);
				}

				pCaptionQA->Draw(pDC, rectQAFrame, bActive ? 0 : 1);
			}
		}
		else if (bHide)
		{
			HICON hIcon = afxGlobalUtils.GetWndIcon(pWnd);

			if (hIcon != NULL)
			{
				CSize szIcon(::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));

				CRect rectIcon(rectCaption.TopLeft(), CSize(min(::GetSystemMetrics(SM_CYCAPTION), rectCaption.Height()), rectCaption.Height()));

				long x = rectCaption.left + max(0, (rectIcon.Width()  - szIcon.cx) / 2);
				long y = rectCaption.top  + max(0, (rectIcon.Height() - szIcon.cy) / 2);

				pDC->DrawState(CPoint(x, y), szIcon, hIcon, DSS_NORMAL, (CBrush*)NULL);

				if (rectText.left < rectIcon.right)
				{
					rectText.left = rectIcon.right;
				}
			}
		}
	}

	CString strText;
	pWnd->GetWindowText(strText);

	CFont* pOldFont = (CFont*)pDC->SelectObject(&m_AppCaptionFont);
	ENSURE(pOldFont != NULL);

	CString strTitle(strText);
	CString strDocument;

	BOOL bPrefix = FALSE;
	if ((dwStyle & FWS_ADDTOTITLE) == FWS_ADDTOTITLE)
	{
		bPrefix = (dwStyle & FWS_PREFIXTITLE) == FWS_PREFIXTITLE;
		CFrameWnd* pFrameWnd = DYNAMIC_DOWNCAST(CFrameWnd, pWnd);

		if (pFrameWnd != NULL)
		{
			strTitle = pFrameWnd->GetTitle();

			if (!strTitle.IsEmpty())
			{
				int pos = strText.Find(strTitle);

				if (pos != -1)
				{
					if (strText.GetLength() > strTitle.GetLength())
					{
						if (pos == 0)
						{
							bPrefix = FALSE; // avoid exception
							strTitle = strText.Left(strTitle.GetLength() + 3);
							strDocument = strText.Right(strText.GetLength() - strTitle.GetLength());
						}
						else
						{
							strTitle = strText.Right(strTitle.GetLength() + 3);
							strDocument = strText.Left(strText.GetLength() - strTitle.GetLength());
						}
					}
				}
			}
			else
			{
				strDocument = strText;
			}
		}
	}

	DrawNcText(pDC, rectText, strTitle, strDocument, bPrefix, bActive, bIsRTL, m_bNcTextCenter, bGlass,
		pWnd->IsZoomed() ? 0 : 10, pWnd->IsZoomed() ? RGB(255, 255, 255) :(COLORREF)-1);

	pDC->SelectObject(pOldFont);
}

int CMFCVisualManagerOffice2007::GetRibbonQuickAccessToolBarRightMargin()
{
	if (!CanDrawImage() || !m_ctrlRibbonCaptionQA.IsValid())
	{
		return CMFCVisualManagerOffice2003::GetRibbonQuickAccessToolBarRightMargin();
	}

	return m_ctrlRibbonCaptionQA.GetParams().m_rectSides.right;
}

void CMFCVisualManagerOffice2007::OnDrawRibbonCaptionButton(CDC* pDC, CMFCRibbonCaptionButton* pButton)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawRibbonCaptionButton(pDC, pButton);
		return;
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	const BOOL bHighlighted = pButton->IsHighlighted();
	const BOOL bPressed = pButton->IsPressed();

	AFX_BUTTON_STATE state = ButtonsIsRegular;
	if (bPressed)
	{
		if (bHighlighted)
		{
			state = ButtonsIsPressed;
		}
	}
	else if (bHighlighted)
	{
		state = ButtonsIsHighlighted;
	}

	const BOOL bMDI = pButton->IsMDIChildButton();
	BOOL bActive = TRUE;

	if (!bMDI)
	{
		CMFCRibbonBar* pBar = pButton->GetParentRibbonBar();
		if (pBar->GetSafeHwnd() != NULL)
		{
			CWnd* pWnd = pBar->GetParent();
			ASSERT_VALID(pWnd);

			bActive = IsWindowActive(pWnd);
		}
	}

	DrawNcBtn(pDC, pButton->GetRect(), pButton->GetID(), state, FALSE, bActive, pButton->IsMDIChildButton());
}

COLORREF CMFCVisualManagerOffice2007::OnDrawRibbonButtonsGroup(CDC* pDC, CMFCRibbonButtonsGroup* pGroup, CRect rectGroup)
{
	if (!CanDrawImage())
	{
		return CMFCVisualManagerOffice2003::OnDrawRibbonButtonsGroup(pDC, pGroup, rectGroup);
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID(pGroup);

	if (pGroup->IsKindOf(RUNTIME_CLASS(CMFCRibbonQuickAccessToolBar)) && m_ctrlRibbonPanelQAT.IsValid())
	{
		CMFCRibbonBar* pBar = pGroup->GetParentRibbonBar();

		if (pBar != NULL && (pBar->GetHideFlags() & AFX_RIBBONBAR_HIDE_ALL) == 0 && !pBar->IsQuickAccessToolbarOnTop())
		{
			m_ctrlRibbonPanelQAT.Draw(pDC, rectGroup);
		}
	}

	return(COLORREF)-1;
}

void CMFCVisualManagerOffice2007::OnDrawDefaultRibbonImage(CDC* pDC, CRect rectImage, BOOL bIsDisabled/* = FALSE*/, BOOL bIsPressed/* = FALSE*/, BOOL bIsHighlighted/* = FALSE*/)
{
	if (!CanDrawImage() || !m_RibbonBtnDefaultImage.IsValid())
	{
		CMFCVisualManagerOffice2003::OnDrawDefaultRibbonImage(pDC, rectImage, bIsDisabled, bIsPressed, bIsHighlighted);
		return;
	}

	m_RibbonBtnDefaultImage.DrawEx(pDC, rectImage, bIsDisabled ? 1 : 0, CMFCToolBarImages::ImageAlignHorzCenter, CMFCToolBarImages::ImageAlignVertCenter);
}

void CMFCVisualManagerOffice2007::OnDrawRibbonApplicationButton(CDC* pDC, CMFCRibbonButton* pButton)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawRibbonApplicationButton(pDC, pButton);
		return;
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	BOOL bHighlighted = pButton->IsHighlighted();
	BOOL bPressed = pButton->IsPressed();

	if (pButton->IsDroppedDown())
	{
		bPressed = TRUE;
		bHighlighted = TRUE;
	}

	CRect rect = pButton->GetRect();
	rect.OffsetRect(1, -1);

	int index = 0;
	if (bPressed)
	{
		if (bHighlighted)
		{
			index = 2;
		}
	}
	else if (bHighlighted)
	{
		index = 1;
	}

	CRect rectImage(m_RibbonBtnMain.GetParams().m_rectImage);

	CMFCToolBarImages::ImageAlignHorz horz = CMFCToolBarImages::ImageAlignHorzStretch;
	CMFCToolBarImages::ImageAlignVert vert = CMFCToolBarImages::ImageAlignVertStretch;

	if (rect.Width() >= rectImage.Width() && rect.Height() >= rectImage.Height())
	{
		horz = CMFCToolBarImages::ImageAlignHorzCenter;
		vert = CMFCToolBarImages::ImageAlignVertCenter;
	}

	m_RibbonBtnMain.FillInterior(pDC, rect, horz, vert, index);
}

COLORREF CMFCVisualManagerOffice2007::OnDrawRibbonTabsFrame(CDC* pDC, CMFCRibbonBar* pWndRibbonBar, CRect rectTab)
{
	if (!CanDrawImage())
	{
		return CMFCVisualManagerOffice2003::OnDrawRibbonTabsFrame(pDC, pWndRibbonBar, rectTab);
	}
	/*
	if (IsBeta())
	{
	CDrawingManager dm(*pDC);
	dm.Fill4ColorsGradient(rectTab, m_clrAppCaptionActiveStart, m_clrAppCaptionActiveFinish,
	m_clrAppCaptionActiveFinish, m_clrAppCaptionActiveStart, FALSE);
	}
	*/
	return m_clrRibbonCategoryText;
}

void CMFCVisualManagerOffice2007::OnDrawRibbonCategory(CDC* pDC, CMFCRibbonCategory* pCategory, CRect rectCategory)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawRibbonCategory(pDC, pCategory, rectCategory);
		return;
	}

	CMFCControlRenderer* pRenderer = &m_ctrlRibbonCategoryBack;
	CMFCVisualManagerBitmapCache* pCache = &m_cacheRibbonCategoryBack;

	CMFCRibbonBaseElement* pParentButton = pCategory->GetParentButton();

	if (pCategory->GetTabColor() != AFX_CategoryColor_None && (pParentButton == NULL || !pParentButton->IsQATMode()))
	{
		CMFCRibbonContextCategory& context = m_ctrlRibbonContextCategory[pCategory->GetTabColor() - 1];

		pRenderer = &context.m_ctrlBack;
		pCache    = &context.m_cacheBack;
	}

	const CMFCControlRendererInfo& params = pRenderer->GetParams();

	CMFCRibbonPanelMenuBar* pMenuBar = pCategory->GetParentMenuBar();
	if (pMenuBar != NULL)
	{
		if (pMenuBar->GetCategory() != NULL)
		{
			if (rectCategory.left < 0 || rectCategory.top < 0)
			{
				CDrawingManager dm(*pDC);
				dm.FillGradient(rectCategory, m_clrBarGradientDark, m_clrBarGradientLight, TRUE);

				return;
			}
		}
		else if (pMenuBar->GetPanel() != NULL)
		{
			if (IsBeta())
			{
				pRenderer->FillInterior(pDC, rectCategory);
				return;
			}
		}
	}

	int nCacheIndex = -1;
	if (pCache != NULL)
	{
		CSize size(params.m_rectImage.Width(), rectCategory.Height());
		nCacheIndex = pCache->FindIndex(size);
		if (nCacheIndex == -1)
		{
			nCacheIndex = pCache->CacheY(size.cy, *pRenderer);
		}
	}

	if (nCacheIndex != -1)
	{
		pCache->Get(nCacheIndex)->DrawY(pDC, rectCategory, CSize(params.m_rectInter.left, params.m_rectImage.right - params.m_rectInter.right));
	}
	else
	{
		pRenderer->Draw(pDC, rectCategory);
	}
}

void CMFCVisualManagerOffice2007::OnDrawRibbonCategoryScroll (
					CDC* pDC, 
					CRibbonCategoryScroll* pScroll)
{
	if (!CanDrawImage ())
	{
		CMFCVisualManagerOffice2003::OnDrawRibbonCategoryScroll (pDC, pScroll);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pScroll);

	CRect rect = pScroll->GetRect ();

	CMFCControlRenderer* pRenderer = 
		&m_ctrlRibbonCategoryBtnPage[pScroll->IsLeftScroll () ? 0 : 1];
	int index = 0;

	if (pScroll->IsPressed ())
	{
		index = 1;
		if (pScroll->IsHighlighted ())
		{
			index = 2;
		}
	}
	else if (pScroll->IsHighlighted ())
	{
		index = 1;
	}

	pRenderer->Draw (pDC, rect, index);
	
	BOOL bIsLeft = pScroll->IsLeftScroll ();
	if (afxGlobalData.m_bIsRTL)
	{
		bIsLeft = !bIsLeft;
	}

	CMenuImages::Draw (pDC,
		bIsLeft ? CMenuImages::IdArrowLeftLarge : CMenuImages::IdArrowRightLarge, 
		rect);
}

COLORREF CMFCVisualManagerOffice2007::OnDrawRibbonCategoryTab(CDC* pDC, CMFCRibbonTab* pTab,
	BOOL bIsActive)
{
	if (!CanDrawImage())
	{
		return CMFCVisualManagerOffice2003::OnDrawRibbonCategoryTab(pDC, pTab, bIsActive);
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID(pTab);

	CMFCRibbonCategory* pCategory = pTab->GetParentCategory();
	ASSERT_VALID(pCategory);
	CMFCRibbonBar* pBar = pCategory->GetParentRibbonBar();
	ASSERT_VALID(pBar);

	bIsActive = bIsActive && ((pBar->GetHideFlags() & AFX_RIBBONBAR_HIDE_ELEMENTS) == 0 || pTab->GetDroppedDown() != NULL);

	const BOOL bPressed     = pTab->IsPressed();
	const BOOL bIsHighlight = pTab->IsHighlighted() && !pTab->IsDroppedDown();

	CRect rectTab(pTab->GetRect());
	rectTab.bottom++;

	int ratio = 0;
	if (!IsBeta() && m_ctrlRibbonCategoryTabSep.IsValid())
	{
		ratio = pBar->GetTabTrancateRatio();
	}

	if (ratio > 0)
	{
		rectTab.left++;
	}

	int nImage = bIsActive ? 3 : 0;

	if (bPressed)
	{
		if (bIsHighlight)
		{
			nImage = bIsActive ? 2 : 1;
		}
	}

	if (bIsHighlight)
	{
		nImage += 1;
	}

	CMFCControlRenderer* pRenderer = &m_ctrlRibbonCategoryTab;
	COLORREF clrText = m_clrRibbonCategoryText;
	COLORREF clrTextHighlighted = m_clrRibbonCategoryTextHighlighted;

	if (pCategory->GetTabColor() != AFX_CategoryColor_None || pTab->IsSelected())
	{
		CMFCRibbonContextCategory& context = m_ctrlRibbonContextCategory[(pTab->IsSelected() || nImage == 4) ? AFX_CategoryColor_Orange - 1 : pCategory->GetTabColor() - 1];
		pRenderer = &context.m_ctrlTab;
		clrText  = context.m_clrText;
		clrTextHighlighted = context.m_clrTextHighlighted;
	}

	pRenderer->Draw(pDC, rectTab, nImage);

	if (ratio > 0)
	{
		CRect rectSep(rectTab);
		rectSep.left = rectSep.right;
		rectSep.right++;
		rectSep.bottom--;

		m_ctrlRibbonCategoryTabSep.Draw(pDC, rectSep, 0, (BYTE)min(ratio * 255 / 100, 255));
	}

	return bIsActive ? clrTextHighlighted : clrText;
}

COLORREF CMFCVisualManagerOffice2007::OnDrawRibbonPanel(CDC* pDC, CMFCRibbonPanel* pPanel, CRect rectPanel, CRect rectCaption)
{
	if (!CanDrawImage())
	{
		return CMFCVisualManagerOffice2003::OnDrawRibbonPanel(pDC, pPanel, rectPanel, rectCaption);
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID(pPanel);

	COLORREF clrText = m_clrRibbonPanelText;

	if (pPanel->IsKindOf(RUNTIME_CLASS(CMFCRibbonMainPanel)))
	{
		const int nBorderSize = GetPopupMenuBorderSize();
		rectPanel.InflateRect(nBorderSize, nBorderSize);

		m_ctrlRibbonMainPanel.Draw(pDC, rectPanel);
	}
	else
	{
		BOOL bHighlighted = pPanel->IsHighlighted();

		if (bHighlighted)
		{
			clrText = m_clrRibbonPanelTextHighlighted;
		}

		CMFCControlRenderer* pRendererB = &m_ctrlRibbonPanelBack_B;
		CMFCControlRenderer* pRendererT = &m_ctrlRibbonPanelBack_T;
		CMFCVisualManagerBitmapCache* pCacheB = &m_cacheRibbonPanelBack_B;
		CMFCVisualManagerBitmapCache* pCacheT = &m_cacheRibbonPanelBack_T;

		CMFCRibbonCategory* pCategory = pPanel->GetParentCategory();
		ASSERT_VALID(pCategory);

		CMFCRibbonBaseElement* pParentButton = pPanel->GetParentButton();

		if (pCategory->GetTabColor() != AFX_CategoryColor_None && (pParentButton == NULL || !pParentButton->IsQATMode()))
		{
			pRendererB = &m_ctrlRibbonContextPanelBack_B;
			pRendererT = &m_ctrlRibbonContextPanelBack_T;
			pCacheB = &m_cacheRibbonContextPanelBack_B;
			pCacheT = &m_cacheRibbonContextPanelBack_T;

			clrText = bHighlighted ? m_clrRibbonContextPanelTextHighlighted : m_clrRibbonContextPanelText;
		}

		if (!pPanel->IsCollapsed())
		{
			CRect rect(rectPanel);

			BOOL bDrawCaption = rectCaption.Height() > 0 && pRendererT->IsValid();

			if (bDrawCaption)
			{
				BOOL bBottomEnabled = pRendererB->IsValid();

				if (bBottomEnabled)
				{
					rect.bottom -= rectCaption.Height() == 0 ? pRendererB->GetParams().m_rectImage.Height() : rectCaption.Height();
				}

				{
					const CMFCControlRendererInfo& params = pRendererT->GetParams();

					int nCacheIndex = -1;
					if (pCacheT != NULL)
					{
						CSize size(params.m_rectImage.Width(), rect.Height());
						nCacheIndex = pCacheT->FindIndex(size);
						if (nCacheIndex == -1)
						{
							nCacheIndex = pCacheT->CacheY(size.cy, *pRendererT);
						}
					}

					if (nCacheIndex != -1)
					{
						pCacheT->Get(nCacheIndex)->DrawY(pDC, rect, CSize(params.m_rectInter.left, params.m_rectImage.right - params.m_rectInter.right), bHighlighted ? 1 : 0);
					}
					else
					{
						pRendererT->Draw(pDC, rect, bHighlighted ? 1 : 0);
					}
				}

				if (bBottomEnabled)
				{
					rect.top = rect.bottom;
					rect.bottom = rectPanel.bottom;

					const CMFCControlRendererInfo& params = pRendererB->GetParams();

					int nCacheIndex = -1;
					if (pCacheB != NULL)
					{
						CSize size(params.m_rectImage.Width(), rect.Height());
						nCacheIndex = pCacheB->FindIndex(size);
						if (nCacheIndex == -1)
						{
							nCacheIndex = pCacheB->CacheY(size.cy, *pRendererB);
						}
					}

					if (nCacheIndex != -1)
					{
						pCacheB->Get(nCacheIndex)->DrawY(pDC, rect, CSize(params.m_rectInter.left, params.m_rectImage.right - params.m_rectInter.right), bHighlighted ? 1 : 0);
					}
					else
					{
						pRendererB->Draw(pDC, rect, bHighlighted ? 1 : 0);
					}
				}
			}
		}
	}

	return clrText;
}

void CMFCVisualManagerOffice2007::OnDrawRibbonPanelCaption(CDC* pDC, CMFCRibbonPanel* pPanel, CRect rectCaption)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawRibbonPanelCaption(pDC, pPanel, rectCaption);
		return;
	}

	if (pPanel->IsKindOf(RUNTIME_CLASS(CMFCRibbonMainPanel)))
	{
		return;
	}

	CString str = pPanel->GetName();

	if (!str.IsEmpty())
	{
		if (pPanel->GetLaunchButton().GetID() > 0)
		{
			rectCaption.right = pPanel->GetLaunchButton().GetRect().left;

			rectCaption.DeflateRect(1, 1);
			rectCaption.OffsetRect(-1, -1);
		}
		else
		{
			rectCaption.DeflateRect(1, 1);

			if ((rectCaption.Width() % 2) == 0)
			{
				rectCaption.right--;
			}

			rectCaption.OffsetRect(0, -1);
		}

		COLORREF clrTextOld = pDC->SetTextColor(pPanel->IsHighlighted() ? m_clrRibbonPanelCaptionTextHighlighted : m_clrRibbonPanelCaptionText);

		pDC->DrawText( str, rectCaption, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
		pDC->SetTextColor(clrTextOld);
	}
}

void CMFCVisualManagerOffice2007::OnDrawRibbonLaunchButton(CDC* pDC, CMFCRibbonLaunchButton* pButton, CMFCRibbonPanel* pPanel)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawRibbonLaunchButton(pDC, pButton, pPanel);
		return;
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);
	ASSERT_VALID(pPanel);

	CRect rect(pButton->GetRect());

	if (!IsBeta())
	{
		rect.right--;
		rect.bottom--;
	}

	BOOL bHighlighted = pButton->IsHighlighted();

	int index = 0;

	if (m_RibbonBtnLaunchIcon.GetCount() > 3)
	{
		if (pButton->IsDisabled())
		{
			index = 3;
		}
		else if (pButton->IsPressed())
		{
			if (bHighlighted)
			{
				index = 2;
			}
		}
		else if (bHighlighted)
		{
			index = 1;
		}
	}
	else
	{
		if (!pButton->IsDisabled())
		{
			if (pButton->IsPressed())
			{
				if (bHighlighted)
				{
					index = 2;
				}
			}
			else if (bHighlighted)
			{
				index = 1;
			}
		}
	}

	if (m_ctrlRibbonBtnLaunch.IsValid())
	{
		m_ctrlRibbonBtnLaunch.Draw(pDC, rect, index);
	}

	if (m_RibbonBtnLaunchIcon.IsValid())
	{
		const double dblImageScale = afxGlobalData.GetRibbonImageScale();

		if (dblImageScale == 1.)
		{
			m_RibbonBtnLaunchIcon.DrawEx(pDC, rect, index, CMFCToolBarImages::ImageAlignHorzCenter, CMFCToolBarImages::ImageAlignVertCenter);
		}
		else
		{
			CSize sizeImage = m_RibbonBtnLaunchIcon.GetImageSize();

			sizeImage.cx = (int)(.5 + dblImageScale * sizeImage.cx);
			sizeImage.cy = (int)(.5 + dblImageScale * sizeImage.cy);

			rect.left = rect.CenterPoint().x - sizeImage.cx / 2;
			rect.right = rect.left + sizeImage.cx;

			rect.top = rect.CenterPoint().y - sizeImage.cy / 2;
			rect.bottom = rect.top + sizeImage.cy;

			m_RibbonBtnLaunchIcon.DrawEx(pDC, rect, index, CMFCToolBarImages::ImageAlignHorzStretch, CMFCToolBarImages::ImageAlignVertStretch);
		}
	}
}

COLORREF CMFCVisualManagerOffice2007::OnFillRibbonButton(CDC* pDC, CMFCRibbonButton* pButton)
{
	if (!CanDrawImage())
	{
		return CMFCVisualManagerOffice2003::OnFillRibbonButton(pDC, pButton);
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	const BOOL bIsMenuMode = pButton->IsMenuMode();

	CRect rect(pButton->GetRect());

	CMFCControlRenderer* pRenderer = NULL;
	CMFCVisualManagerBitmapCache* pCache = NULL;
	int index = 0;

	BOOL bDisabled    = pButton->IsDisabled();
	BOOL bFocused     = pButton->IsFocused();
	BOOL bPressed     = pButton->IsPressed() && !bIsMenuMode;
	BOOL bChecked     = pButton->IsChecked();
	BOOL bHighlighted = pButton->IsHighlighted();

	BOOL bDefaultPanelButton = pButton->IsDefaultPanelButton() && !pButton->IsQATMode();

	if (pButton->IsDroppedDown() && !bIsMenuMode)
	{
		bChecked     = TRUE;
		bPressed     = FALSE;
		bHighlighted = FALSE;
	}

	CMFCRibbonBaseElement::RibbonElementLocation location = pButton->GetLocationInGroup();

	if (pButton->IsKindOf(RUNTIME_CLASS(CMFCRibbonEdit)))
	{
		rect.left = pButton->GetCommandRect().left;

		COLORREF color1 = m_clrRibbonEdit;
		if (bDisabled)
		{
			color1 = m_clrRibbonEditDisabled;
		}
		else if (bChecked || bHighlighted)
		{
			color1 = m_clrRibbonEditHighlighted;
		}

		COLORREF color2 = color1;

		{
			CDrawingManager dm(*pDC);
			dm.FillGradient(rect, color1, color2, TRUE);
		}

		return(COLORREF)-1;
	}

	if (bChecked && bIsMenuMode && !pButton->IsGalleryIcon())
	{
		bChecked = FALSE;
	}

	if (location != CMFCRibbonBaseElement::RibbonElementNotInGroup && pButton->IsShowGroupBorder())
	{
		if (!pButton->GetMenuRect().IsRectEmpty())
		{
			CRect rectC = pButton->GetCommandRect();
			CRect rectM = pButton->GetMenuRect();

			CMFCControlRenderer* pRendererC = NULL;
			CMFCControlRenderer* pRendererM = NULL;

			CMFCVisualManagerBitmapCache* pCacheC = NULL;
			CMFCVisualManagerBitmapCache* pCacheM = NULL;

			if (location == CMFCRibbonBaseElement::RibbonElementSingleInGroup)
			{
				pRendererC = &m_ctrlRibbonBtnGroupMenu_F[0];
				pRendererM = &m_ctrlRibbonBtnGroupMenu_L[1];

				pCacheC = &m_cacheRibbonBtnGroupMenu_F[0];
				pCacheM = &m_cacheRibbonBtnGroupMenu_L[1];
			}
			else if (location == CMFCRibbonBaseElement::RibbonElementFirstInGroup)
			{
				pRendererC = &m_ctrlRibbonBtnGroupMenu_F[0];
				pRendererM = &m_ctrlRibbonBtnGroupMenu_F[1];

				pCacheC = &m_cacheRibbonBtnGroupMenu_F[0];
				pCacheM = &m_cacheRibbonBtnGroupMenu_F[1];
			}
			else if (location == CMFCRibbonBaseElement::RibbonElementLastInGroup)
			{
				pRendererC = &m_ctrlRibbonBtnGroupMenu_L[0];
				pRendererM = &m_ctrlRibbonBtnGroupMenu_L[1];

				pCacheC = &m_cacheRibbonBtnGroupMenu_L[0];
				pCacheM = &m_cacheRibbonBtnGroupMenu_L[1];
			}
			else
			{
				pRendererC = &m_ctrlRibbonBtnGroupMenu_M[0];
				pRendererM = &m_ctrlRibbonBtnGroupMenu_M[1];

				pCacheC = &m_cacheRibbonBtnGroupMenu_M[0];
				pCacheM = &m_cacheRibbonBtnGroupMenu_M[1];
			}

			int indexC = 0;
			int indexM = 0;

			BOOL bHighlightedC = pButton->IsCommandAreaHighlighted();
			BOOL bHighlightedM = pButton->IsMenuAreaHighlighted();

			if (IsBeta())
			{
				if (bChecked)
				{
					indexC = 3;
				}

				if (bDisabled)
				{
					indexC = 0;
				}
				else
				{
					if (pButton->IsDroppedDown() && !bIsMenuMode)
					{
						indexC = pButton->IsChecked() ? 3 : 0;
						indexM = 3;
					}
					else
					{
						if (bPressed)
						{
							if (bHighlightedC)
							{
								indexM = 1;
								indexC = 2;
							}
						}
						else if (bHighlighted)
						{
							indexC++;
							indexM = 1;
						}
					}
				}
			}
			else
			{
				if (bChecked)
				{
					indexC = 3;

					if (bHighlighted)
					{
						indexM = 5;
					}
				}

				if (bDisabled)
				{
					if (bChecked)
					{
						indexC = 5;
						indexM = 4;
					}
				}
				else
				{
					if (pButton->IsDroppedDown() && !bIsMenuMode)
					{
						indexC = pButton->IsChecked() ? 3 : 6;
						indexM = 3;
					}
					else
					{
						if (bHighlightedC || bHighlightedM)
						{
							if (bChecked)
							{
								indexC = bHighlightedC ? 4 : 3;
							}
							else
							{
								indexC = bHighlightedC ? 1 : 6;
							}

							indexM = bHighlightedM ? 1 : 5;
						}

						if (bPressed)
						{
							if (bHighlightedC)
							{
								indexC = 2;
							}
						}
					}
				}
			}

			if (indexC != -1 && indexM != -1)
			{
				int nCacheIndex = -1;
				if (pCacheC != NULL)
				{
					CSize size(rectC.Size());
					nCacheIndex = pCacheC->FindIndex(size);
					if (nCacheIndex == -1)
					{
						nCacheIndex = pCacheC->Cache(size, *pRendererC);
					}
				}

				if (nCacheIndex != -1)
				{
					pCacheC->Get(nCacheIndex)->Draw(pDC, rectC, indexC);
				}
				else
				{
					pRendererC->Draw(pDC, rectC, indexC);
				}

				nCacheIndex = -1;
				if (pCacheM != NULL)
				{
					CSize size(rectM.Size());
					nCacheIndex = pCacheM->FindIndex(size);
					if (nCacheIndex == -1)
					{
						nCacheIndex = pCacheM->Cache(size, *pRendererM);
					}
				}

				if (nCacheIndex != -1)
				{
					pCacheM->Get(nCacheIndex)->Draw(pDC, rectM, indexM);
				}
				else
				{
					pRendererM->Draw(pDC, rectM, indexM);
				}
			}

			return(COLORREF)-1;
		}
		else
		{
			if (location == CMFCRibbonBaseElement::RibbonElementSingleInGroup)
			{
				pRenderer = &m_ctrlRibbonBtnGroup_S;
				pCache    = &m_cacheRibbonBtnGroup_S;
			}
			else if (location == CMFCRibbonBaseElement::RibbonElementFirstInGroup)
			{
				pRenderer = &m_ctrlRibbonBtnGroup_F;
				pCache    = &m_cacheRibbonBtnGroup_F;
			}
			else if (location == CMFCRibbonBaseElement::RibbonElementLastInGroup)
			{
				pRenderer = &m_ctrlRibbonBtnGroup_L;
				pCache    = &m_cacheRibbonBtnGroup_L;
			}
			else
			{
				pRenderer = &m_ctrlRibbonBtnGroup_M;
				pCache    = &m_cacheRibbonBtnGroup_M;
			}

			if (bChecked)
			{
				index = 3;
			}

			if (bDisabled)
			{
				index = 0;
			}
			else
			{
				if (bPressed)
				{
					if (bHighlighted)
					{
						index = 2;
					}
				}
				else if (bHighlighted)
				{
					index++;
				}
			}
		}
	}
	else if (bDefaultPanelButton)
	{
		if (bPressed)
		{
			if (bHighlighted)
			{
				index = 2;
			}
		}
		else if (bHighlighted)
		{
			index = 1;
		}
		else if (bChecked)
		{
			index = 2;
		}

		if (index != -1)
		{
			pRenderer = &m_ctrlRibbonBtnDefault;
			CMFCVisualManagerBitmapCache* pCacheDefault = &m_cacheRibbonBtnDefault;

			CMFCRibbonCategory* pCategory = pButton->GetParentCategory();
			ASSERT_VALID(pCategory);

			if (pCategory->GetTabColor() != AFX_CategoryColor_None)
			{
				CMFCRibbonContextCategory& context = m_ctrlRibbonContextCategory[pCategory->GetTabColor() - 1];

				pRenderer = &context.m_ctrlBtnDefault;
				pCacheDefault = &context.m_cacheBtnDefault;
			}

			const CMFCControlRendererInfo& params = pRenderer->GetParams();

			int nCacheIndex = -1;
			if (pCacheDefault != NULL)
			{
				CSize size(params.m_rectImage.Width(), rect.Height());
				nCacheIndex = pCacheDefault->FindIndex(size);
				if (nCacheIndex == -1)
				{
					nCacheIndex = pCacheDefault->CacheY(size.cy, *pRenderer);
				}
			}

			if (nCacheIndex != -1)
			{
				pCacheDefault->Get(nCacheIndex)->DrawY(pDC, rect, CSize(params.m_rectInter.left, params.m_rectImage.right - params.m_rectInter.right), index);

				return m_clrToolBarBtnTextHighlighted;
			}
		}
	}
	else if ((!bDisabled &&(bPressed || bChecked || bHighlighted)) || (bDisabled && bFocused))
	{
		if (!pButton->GetMenuRect().IsRectEmpty()/* && (pButton->IsHighlighted() || bChecked)*/)
		{
			CRect rectC = pButton->GetCommandRect();
			CRect rectM = pButton->GetMenuRect();

			CMFCControlRenderer* pRendererC = pButton->IsMenuOnBottom() ? &m_ctrlRibbonBtnMenuV[0] : &m_ctrlRibbonBtnMenuH[0];
			CMFCControlRenderer* pRendererM = pButton->IsMenuOnBottom() ? &m_ctrlRibbonBtnMenuV[1] : &m_ctrlRibbonBtnMenuH[1];

			int indexC = -1;
			int indexM = -1;

			BOOL bDropped      = pButton->IsDroppedDown();
			BOOL bHighlightedC = pButton->IsCommandAreaHighlighted();
			BOOL bHighlightedM = pButton->IsMenuAreaHighlighted();

			if (IsBeta())
			{
				if (bChecked)
				{
					indexC = 2;
					indexM = 2;
				}

				if (bDisabled)
				{
					if (bChecked)
					{
					}
				}
				else
				{
					if (bDropped && !bIsMenuMode)
					{
						indexC = bChecked ? 2 : 4;
						indexM = 2;
					}
					else
					{
						if (bPressed)
						{
							if (bHighlighted)
							{
								if (bHighlightedC)
								{
									indexC = 1;
								}
								else
								{
									indexC = bChecked ? indexC : 0;
								}

								indexM = bChecked ? indexM : 0;
							}
						}
						else if (bHighlighted)
						{
							indexC++;
							indexM++;
						}
					}
				}
			}
			else
			{
				if (bDisabled)
				{
					if (bHighlightedC || bHighlightedM)
					{
						indexC = 4;
						indexM = 4;

						if (bHighlightedM)
						{
							indexM = 0;

							if (bDropped && !bIsMenuMode)
							{
								indexC = 5;
								indexM = 2;
							}
							else if (bPressed)
							{
								indexM = 1;
							}
						}
					}
				}
				else
				{
					if (bDropped && !bIsMenuMode)
					{
						indexC = 5;
						indexM = 2;
					}
					else
					{
						if (bChecked)
						{
							indexC = 2;
							indexM = 2;
						}

						if (bHighlightedC || bHighlightedM)
						{
							indexM = 4;

							if (bPressed)
							{
								if (bHighlightedC)
								{
									indexC = 1;
								}
								else if (bHighlightedM)
								{
									indexC = bChecked ? 3 : 5;
								}
							}
							else
							{
								indexC = bChecked ? 3 : 0;

								if (bHighlightedM)
								{
									indexC = bChecked ? 3 : 5;
									indexM = 0;
								}
							}
						}
					}
				}
			}

			if (indexC != -1)
			{
				pRendererC->Draw(pDC, rectC, indexC);
			}

			if (indexM != -1)
			{
				pRendererM->Draw(pDC, rectM, indexM);
			}

			return(COLORREF)-1;
		}
		else
		{
			index = -1;

			pRenderer = &m_ctrlRibbonBtn[0];
			if (rect.Height() > pRenderer->GetParams().m_rectImage.Height() * 1.5 && m_ctrlRibbonBtn[1].IsValid())
			{
				pRenderer = &m_ctrlRibbonBtn[1];
			}

			if (bDisabled && bFocused)
			{
				if (pRenderer->GetImageCount() > 4)
				{
					index = 4;
				}
				else
				{
					index = 0;
				}
			}

			if (!bDisabled)
			{
				if (bChecked)
				{
					index = 2;
				}

				if (bPressed)
				{
					if (bHighlighted)
					{
						index = 1;
					}
				}
				else if (bHighlighted)
				{
					index++;
				}
			}
		}
	}

	COLORREF clrText = bDisabled ? m_clrToolBarBtnTextDisabled : COLORREF(-1);

	if (pRenderer != NULL)
	{
		if (index != -1)
		{
			int nCacheIndex = -1;
			if (pCache != NULL)
			{
				CSize size(rect.Size());
				nCacheIndex = pCache->FindIndex(size);
				if (nCacheIndex == -1)
				{
					nCacheIndex = pCache->Cache(size, *pRenderer);
				}
			}

			if (nCacheIndex != -1)
			{
				pCache->Get(nCacheIndex)->Draw(pDC, rect, index);
			}
			else
			{
				pRenderer->Draw(pDC, rect, index);
			}

			if (!bDisabled)
			{
				clrText = m_clrToolBarBtnTextHighlighted;
			}
		}
	}

	return clrText;
}

void CMFCVisualManagerOffice2007::OnDrawRibbonButtonBorder(CDC* pDC, CMFCRibbonButton* pButton)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawRibbonButtonBorder(pDC, pButton);
	}

	if (pButton->IsKindOf(RUNTIME_CLASS(CMFCRibbonEdit)))
	{
		CRect rect(pButton->GetRect());

		COLORREF colorBorder = m_clrRibbonEditBorder;

		if (pButton->IsDisabled())
		{
			colorBorder = m_clrRibbonEditBorderDisabled;
		}
		else if (pButton->IsHighlighted() || pButton->IsDroppedDown())
		{
			colorBorder = pButton->IsDroppedDown() ? m_clrRibbonEditBorderPressed : m_clrRibbonEditBorderHighlighted;
		}

		rect.left = pButton->GetCommandRect().left;

		if (CMFCToolBarImages::m_bIsDrawOnGlass)
		{
			CDrawingManager dm(*pDC);
			dm.DrawRect(rect, (COLORREF)-1, colorBorder);
		}
		else
		{
			pDC->Draw3dRect(rect, colorBorder, colorBorder);
		}
	}
}

void CMFCVisualManagerOffice2007::OnDrawRibbonMenuCheckFrame(CDC* pDC, CMFCRibbonButton* pButton, CRect rect)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawRibbonMenuCheckFrame(pDC, pButton, rect);
		return;
	}

	ASSERT_VALID(pDC);

	m_ctrlMenuItemBack.Draw(pDC, rect);
}

void CMFCVisualManagerOffice2007::OnDrawRibbonDefaultPaneButton(CDC* pDC, CMFCRibbonButton* pButton)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawRibbonDefaultPaneButton(pDC, pButton);
		return;
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	OnFillRibbonButton(pDC, pButton);

	BOOL bIsQATMode = pButton->IsQATMode();

	CRect rectFrame(pButton->GetRect());

	if (!bIsQATMode)
	{
		if (m_ctrlRibbonBtnDefaultIcon.IsValid())
		{
			const CSize sizeImage = pButton->GetImageSize(CMFCRibbonButton::RibbonImageSmall);
			const int nMarginX = 11;
			const int nMarginY = 10;

			rectFrame.top += nMarginY / 2;
			rectFrame.bottom = rectFrame.top + sizeImage.cy + 2 * nMarginY;
			rectFrame.top -= 2;
			rectFrame.left = rectFrame.CenterPoint().x - sizeImage.cx / 2 - nMarginX;
			rectFrame.right = rectFrame.left + sizeImage.cx + 2 * nMarginX;

			m_ctrlRibbonBtnDefaultIcon.Draw(pDC, rectFrame);
		}
	}
	else
	{
		if (m_ctrlRibbonBtnDefaultQAT.IsValid())
		{
			int index = 0;
			if (pButton->IsDroppedDown())
			{
				index = 2;
			}
			else if (pButton->IsPressed())
			{
				if (pButton->IsHighlighted())
				{
					index = 2;
				}
			}
			else if (pButton->IsHighlighted())
			{
				index = 1;
			}

			m_ctrlRibbonBtnDefaultQAT.Draw(pDC, rectFrame, index);
		}
		else if (m_ctrlRibbonBtnDefaultQATIcon.IsValid())
		{
			int nHeight = m_ctrlRibbonBtnDefaultQATIcon.GetParams().m_rectImage.Height();

			if (rectFrame.Height() < nHeight)
			{
				nHeight = rectFrame.Height() / 2;
			}

			rectFrame.DeflateRect(1, 0);
			rectFrame.top = rectFrame.bottom - nHeight;

			m_ctrlRibbonBtnDefaultQATIcon.Draw(pDC, rectFrame);
		}
	}

	OnDrawRibbonDefaultPaneButtonContext(pDC, pButton);
}

void CMFCVisualManagerOffice2007::OnDrawRibbonDefaultPaneButtonIndicator(CDC* pDC, CMFCRibbonButton* pButton, CRect rect, BOOL bIsSelected, BOOL bHighlighted)
{
	if (!CanDrawImage() || !m_ctrlRibbonBtnDefaultIcon.IsValid())
	{
		CMFCVisualManagerOffice2003::OnDrawRibbonDefaultPaneButtonIndicator(pDC, pButton, rect, bIsSelected, bHighlighted);
		return;
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	rect.left = rect.right - m_ctrlRibbonBtnDefaultIcon.GetParams().m_rectImage.Width();
	m_ctrlRibbonBtnDefaultIcon.Draw(pDC, rect);

	CRect rectWhite = rect;
	rectWhite.OffsetRect(0, 1);

	CMenuImages::Draw(pDC, CMenuImages::IdArrowDown, rectWhite, CMenuImages::ImageWhite);
	CMenuImages::Draw(pDC, CMenuImages::IdArrowDown, rect, CMenuImages::ImageBlack);
}

void CMFCVisualManagerOffice2007::OnFillRibbonEdit(CDC* pDC, CMFCRibbonRichEditCtrl* pEdit, CRect rect, BOOL bIsHighlighted, BOOL bIsPaneHighlighted,
	BOOL bIsDisabled, COLORREF& clrText, COLORREF& clrSelBackground, COLORREF& clrSelText)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnFillRibbonEdit(pDC, pEdit, rect, bIsHighlighted, bIsPaneHighlighted, bIsDisabled, clrText, clrSelBackground, clrSelText);
		return;
	}

	ASSERT_VALID(pDC);

	COLORREF color1 = m_clrRibbonEdit;

	if (bIsDisabled)
	{
		color1 = m_clrRibbonEditDisabled;
	}
	else
	{
		if (bIsHighlighted)
		{
			color1 = m_clrRibbonEditHighlighted;
		}
	}

	COLORREF color2 = color1;

	CDrawingManager dm(*pDC);
	dm.FillGradient(rect, color1, color2, TRUE);

	if (bIsDisabled)
	{
		clrText = afxGlobalData.clrGrayedText;
	}
	else
	{
		clrText = m_clrMenuText;
		clrSelText = m_clrMenuText;
		clrSelBackground = m_clrRibbonEditSelection;
	}
}

void CMFCVisualManagerOffice2007::OnDrawRibbonMainPanelFrame(CDC* pDC, CMFCRibbonMainPanel* pPanel, CRect rect)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawRibbonMainPanelFrame(pDC, pPanel, rect);
		return;
	}

	if (!IsBeta())
	{
		ASSERT_VALID(pDC);

		rect.right += 2;
		m_ctrlRibbonMainPanelBorder.DrawFrame(pDC, rect);
	}
}

void CMFCVisualManagerOffice2007::OnFillRibbonMenuFrame(CDC* pDC, CMFCRibbonMainPanel* pPanel, CRect rect)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnFillRibbonMenuFrame(pDC, pPanel, rect);
		return;
	}

	ASSERT_VALID(pDC);

	pDC->FillRect(rect, &m_brMenuLight);
}

void CMFCVisualManagerOffice2007::OnDrawRibbonRecentFilesFrame(CDC* pDC, CMFCRibbonMainPanel* pPanel, CRect rect)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawRibbonRecentFilesFrame(pDC, pPanel, rect);
		return;
	}

	ASSERT_VALID(pDC);

	rect.right += 2;
	pDC->FillRect(rect, &m_brBarBkgnd);

	CRect rectSeparator = rect;
	rectSeparator.right = rectSeparator.left + 2;

	DrawSeparator(pDC, rectSeparator, FALSE);
}

COLORREF CMFCVisualManagerOffice2007::OnFillRibbonMainPanelButton(CDC* pDC, CMFCRibbonButton* pButton)
{
	if (!CanDrawImage())
	{
		return CMFCVisualManagerOffice2003::OnFillRibbonMainPanelButton(pDC, pButton);
	}

	BOOL bHighlighted = pButton->IsHighlighted();

	COLORREF clrText = bHighlighted ? m_clrMenuTextHighlighted : pButton->IsDisabled() ? m_clrMenuTextDisabled : m_clrMenuText;

	const int index = bHighlighted ? 1 : 0;
	m_ctrlRibbonBtnMainPanel.Draw(pDC, pButton->GetRect(), index);

	return clrText;
}

void CMFCVisualManagerOffice2007::OnDrawRibbonMainPanelButtonBorder(CDC* pDC, CMFCRibbonButton* pButton)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawRibbonMainPanelButtonBorder(pDC, pButton);
		return;
	}
}

void CMFCVisualManagerOffice2007::OnDrawRibbonGalleryButton(CDC* pDC, CMFCRibbonGalleryIcon* pButton)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawRibbonGalleryButton(pDC, pButton);
		return;
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	int index = 0;
	if (pButton->IsDisabled())
	{
		index = 3;
	}
	else
	{
		if (pButton->IsPressed())
		{
			if (pButton->IsHighlighted())
			{
				index = 2;
			}
		}
		else if (pButton->IsHighlighted())
		{
			index = 1;
		}
	}

	int nBtn = 1;
	if (pButton->IsLast())
	{
		nBtn = 2;
	}
	else if (pButton->IsFirst())
	{
		nBtn = 0;
	}

	m_ctrlRibbonBtnPalette[nBtn].Draw(pDC, pButton->GetRect(), index);
}

void CMFCVisualManagerOffice2007::OnDrawRibbonGalleryBorder(CDC* pDC, CMFCRibbonGallery* pButton, CRect rectBorder)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawRibbonGalleryBorder(pDC, pButton, rectBorder);
		return;
	}

	rectBorder.right -= 5;

	ASSERT_VALID(pDC);
	pDC->Draw3dRect(rectBorder, afxGlobalData.clrBarShadow, afxGlobalData.clrBarShadow);
}

COLORREF CMFCVisualManagerOffice2007::OnDrawRibbonCategoryCaption(CDC* pDC, CMFCRibbonContextCaption* pContextCaption)
{
	if (!CanDrawImage() || pContextCaption->GetColor() == AFX_CategoryColor_None)
	{
		return CMFCVisualManagerOffice2003::OnDrawRibbonCategoryCaption(pDC, pContextCaption);
	}

	CMFCRibbonContextCategory& context = m_ctrlRibbonContextCategory[pContextCaption->GetColor() - 1];

	CRect rect(pContextCaption->GetRect());
	context.m_ctrlCaption.Draw(pDC, rect);

	int xTabRight = pContextCaption->GetRightTabX();

	if (xTabRight > 0)
	{
		CRect rectTab(pContextCaption->GetParentRibbonBar()->GetActiveCategory()->GetTabRect());
		rect.top = rectTab.top;
		rect.bottom = rectTab.bottom;
		rect.right = xTabRight;

		m_ctrlRibbonContextSeparator.DrawFrame(pDC, rect);
	}

	return context.m_clrCaptionText;
}

COLORREF CMFCVisualManagerOffice2007::OnDrawRibbonStatusBarPane(CDC* pDC, CMFCRibbonStatusBar* pBar, CMFCRibbonStatusBarPane* pPane)
{
	if (!CanDrawImage())
	{
		return CMFCVisualManagerOffice2003::OnDrawRibbonStatusBarPane(pDC, pBar, pPane);
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID(pBar);
	ASSERT_VALID(pPane);

	CRect rectPane = pPane->GetRect();

	const BOOL bHighlighted = pPane->IsHighlighted();
	const BOOL bChecked     = pPane->IsChecked();
	const BOOL bExtended = pPane->IsExtended();

	if (bHighlighted || bChecked)
	{
		CRect rectButton = rectPane;
		rectButton.DeflateRect(1, 1);

		int index = 0;
		if (pPane->IsPressed())
		{
			if (bHighlighted)
			{
				index = 1;
			}
		}
		else if (bChecked)
		{
			if (bHighlighted)
			{
				index = 0;
			}
			else
			{
				index = 1;
			}
		}

		m_ctrlRibbonBtnStatusPane.Draw(pDC, rectButton, index);
	}

	if (pPane->IsDisabled())
	{
		return bExtended ? m_clrExtenedStatusBarTextDisabled : m_clrStatusBarTextDisabled;
	}

	return bHighlighted ? m_clrToolBarBtnTextHighlighted : m_clrStatusBarText;
}

void CMFCVisualManagerOffice2007::OnDrawRibbonSliderZoomButton( CDC* pDC, CMFCRibbonSlider* pSlider, CRect rect, BOOL bIsZoomOut, BOOL bIsHighlighted, BOOL bIsPressed, BOOL bIsDisabled)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawRibbonSliderZoomButton( pDC, pSlider, rect, bIsZoomOut, bIsHighlighted, bIsPressed, bIsDisabled);
		return;
	}

	ASSERT_VALID(pDC);

	CMFCControlRenderer* pRenderer = bIsZoomOut ? &m_ctrlRibbonSliderBtnMinus : &m_ctrlRibbonSliderBtnPlus;

	int index = 0;
	if (bIsPressed)
	{
		if (bIsHighlighted)
		{
			index = 2;
		}
	}
	else if (bIsHighlighted)
	{
		index = 1;
	}

	pRenderer->FillInterior(pDC, rect, afxGlobalData.GetRibbonImageScale() != 1. ? CMFCToolBarImages::ImageAlignHorzStretch : CMFCToolBarImages::ImageAlignHorzCenter,
		afxGlobalData.GetRibbonImageScale() != 1. ? CMFCToolBarImages::ImageAlignVertStretch : CMFCToolBarImages::ImageAlignVertCenter, index);
}

void CMFCVisualManagerOffice2007::OnDrawRibbonSliderChannel(CDC* pDC, CMFCRibbonSlider* pSlider, CRect rect)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawRibbonSliderChannel( pDC, pSlider, rect);
		return;
	}

	ASSERT_VALID(pDC);

	DrawSeparator(pDC, rect, m_penSeparatorDark, m_penSeparator2, TRUE);

	rect.left += rect.Width() / 2 - 1;
	rect.right = rect.left + 2;
	rect.top -= 2;
	rect.bottom += 2;
	DrawSeparator(pDC, rect, m_penSeparatorDark, m_penSeparator2, FALSE);
}

void CMFCVisualManagerOffice2007::OnDrawRibbonSliderThumb(CDC* pDC, CMFCRibbonSlider* pSlider, CRect rect, BOOL bIsHighlighted, BOOL bIsPressed, BOOL bIsDisabled)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawRibbonSliderThumb(pDC, pSlider, rect, bIsHighlighted, bIsPressed, bIsDisabled);
		return;
	}

	ASSERT_VALID(pDC);

	int index = 0;
	if (bIsPressed)
	{
		index = 2;
	}
	else if (bIsHighlighted)
	{
		index = 1;
	}

	m_ctrlRibbonSliderThumb.FillInterior(pDC, rect, afxGlobalData.GetRibbonImageScale() != 1. ? CMFCToolBarImages::ImageAlignHorzStretch : CMFCToolBarImages::ImageAlignHorzCenter,
		afxGlobalData.GetRibbonImageScale() != 1. ? CMFCToolBarImages::ImageAlignVertStretch : CMFCToolBarImages::ImageAlignVertCenter, index);
}

void CMFCVisualManagerOffice2007::OnDrawRibbonProgressBar(CDC* pDC, CMFCRibbonProgressBar* pProgress, CRect rectProgress, CRect rectChunk, BOOL bInfiniteMode)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnDrawRibbonProgressBar(pDC, pProgress, rectProgress, rectChunk, bInfiniteMode);
		return;
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID(pProgress);

	m_ctrlRibbonProgressBack.Draw(pDC, rectProgress);

	CRect rectInternal(rectProgress);
	CRect rectBorders(m_ctrlRibbonProgressBack.GetParams().m_rectCorners);

	rectInternal.DeflateRect(rectBorders.left, rectBorders.top, rectBorders.right, rectBorders.bottom);

	if (!bInfiniteMode)
	{
		// normal
		rectChunk.IntersectRect(rectChunk, rectInternal);

		if (!rectChunk.IsRectEmpty() || pProgress->GetPos() != pProgress->GetRangeMin())
		{
			CRgn rgn;
			rgn.CreateRectRgnIndirect(rectInternal);
			pDC->SelectClipRgn(&rgn);

			if (!rectChunk.IsRectEmpty())
			{
				rectChunk.left = rectChunk.right - rectInternal.Width();
				m_ctrlRibbonProgressNormal.Draw(pDC, rectChunk);
			}
			else
			{
				rectChunk = rectInternal;
				rectChunk.right  = rectInternal.left;
			}

			if (rectChunk.right != rectInternal.right)
			{
				rectChunk.left = rectChunk.right;
				rectChunk.right += m_ctrlRibbonProgressNormalExt.GetParams().m_rectImage.Width();

				m_ctrlRibbonProgressNormalExt.Draw(pDC, rectChunk);
			}

			pDC->SelectClipRgn(NULL);
		}
	}
	else if (pProgress->GetPos() != pProgress->GetRangeMin())
	{
		int index = (pProgress->GetPos() - pProgress->GetRangeMin()) % m_ctrlRibbonProgressInfinity.GetImageCount();

		m_ctrlRibbonProgressInfinity.Draw(pDC, rectInternal, index);
	}
}

void CMFCVisualManagerOffice2007::OnFillRibbonQuickAccessToolBarPopup(CDC* pDC, CMFCRibbonPanelMenuBar* pMenuBar, CRect rect)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnFillRibbonQuickAccessToolBarPopup(pDC, pMenuBar, rect);
		return;
	}

	ASSERT_VALID(pDC);

	if (m_ctrlRibbonBorder_QAT.IsValid())
	{
		m_ctrlRibbonBorder_QAT.FillInterior(pDC, rect);
	}
	else
	{
		CDrawingManager dm(*pDC);
		dm.FillGradient(rect, m_clrBarGradientDark, m_clrBarGradientLight, TRUE);
	}
}

int CMFCVisualManagerOffice2007::GetRibbonPopupBorderSize(const CMFCRibbonPanelMenu* pPopup) const
{
	if (!CanDrawImage())
	{
		return CMFCVisualManagerOffice2003::GetRibbonPopupBorderSize(pPopup);
	}

	if (pPopup != NULL)
	{
		ASSERT_VALID(pPopup);

		CMFCRibbonPanelMenuBar* pRibbonMenuBar = DYNAMIC_DOWNCAST(CMFCRibbonPanelMenuBar, (const_cast<CMFCRibbonPanelMenu*>(pPopup))->GetMenuBar());

		if (pRibbonMenuBar != NULL)
		{
			if (pRibbonMenuBar->IsMainPanel())
			{
				return(int)GetPopupMenuBorderSize();
			}

			if (!pRibbonMenuBar->IsMenuMode())
			{
				if (pRibbonMenuBar->IsQATPopup())
				{
					if (m_ctrlRibbonBorder_QAT.IsValid())
					{
						return m_ctrlRibbonBorder_QAT.GetParams().m_rectSides.left;
					}
				}
				else if (pRibbonMenuBar->IsCategoryPopup())
				{
					return 0;
				}
				else if (pRibbonMenuBar->IsRibbonMiniToolBar())
				{
					if (m_ctrlRibbonBorder_Floaty.IsValid())
					{
						return m_ctrlRibbonBorder_Floaty.GetParams().m_rectSides.left;
					}
				}
				else
				{
					if (pRibbonMenuBar->GetPanel() != NULL)
					{
						if (!IsBeta1())
						{
							return 0;
						}
					}

					// standard size
				}
			}
		}
	}

	return(int)GetPopupMenuBorderSize();
}

void CMFCVisualManagerOffice2007::OnDrawRibbonKeyTip(CDC* pDC, CMFCRibbonBaseElement* pElement, CRect rect, CString str)
{
	if (!CanDrawImage() ||
		!m_ctrlRibbonKeyTip.IsValid())
	{
		CMFCVisualManagerOffice2003::OnDrawRibbonKeyTip(pDC, pElement, rect, str);
		return;
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID(pElement);

	BOOL bDisabled = pElement->IsDisabled();

	m_ctrlRibbonKeyTip.Draw(pDC, rect, 0);

	str.MakeUpper();

	COLORREF clrTextOld = pDC->SetTextColor(bDisabled ? m_clrRibbonKeyTipTextDisabled : m_clrRibbonKeyTipTextNormal);

	pDC->DrawText(str, rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	pDC->SetTextColor(clrTextOld);
}

void CMFCVisualManagerOffice2007::OnDrawRibbonCheckBoxOnList(CDC* pDC, CMFCRibbonCheckBox* pCheckBox, CRect rect, BOOL bIsSelected, BOOL bHighlighted)
{
	ASSERT_VALID(pDC);

	CMFCToolBarImages& img = m_MenuItemMarkerC;

	if (!CanDrawImage() || img.GetCount() == 0)
	{
		CMFCVisualManagerOffice2003::OnDrawRibbonCheckBoxOnList(pDC, pCheckBox, rect, bIsSelected, bHighlighted);
		return;
	}

	if (afxGlobalData.GetRibbonImageScale() != 1)
	{
		rect.DeflateRect(5, 5);
		img.DrawEx(pDC, rect, 0, CMFCToolBarImages::ImageAlignHorzStretch, CMFCToolBarImages::ImageAlignVertStretch);
	}
	else
	{
		img.DrawEx(pDC, rect, 0, CMFCToolBarImages::ImageAlignHorzCenter, CMFCToolBarImages::ImageAlignVertCenter);
	}
}

COLORREF CMFCVisualManagerOffice2007::GetRibbonHyperlinkTextColor(CMFCRibbonLinkCtrl* pHyperLink)
{
	ASSERT_VALID(pHyperLink);

	if (!CanDrawImage() || pHyperLink->IsDisabled())
	{
		return CMFCVisualManagerOffice2003::GetRibbonHyperlinkTextColor(pHyperLink);
	}

	COLORREF clrText = pHyperLink->IsHighlighted() ? m_clrRibbonHyperlinkActive : m_clrRibbonHyperlinkInactive;

	if (m_clrRibbonStatusbarHyperlinkActive != (COLORREF)-1 && m_clrRibbonStatusbarHyperlinkInactive != (COLORREF)-1)
	{
		CMFCRibbonStatusBar* pParentStatusBar = DYNAMIC_DOWNCAST(CMFCRibbonStatusBar, pHyperLink->GetParentRibbonBar());

		if (pParentStatusBar != NULL)
		{
			ASSERT_VALID(pParentStatusBar);

			if (!pParentStatusBar->IsExtendedElement(pHyperLink))
			{
				clrText = pHyperLink->IsHighlighted() ? m_clrRibbonStatusbarHyperlinkActive : m_clrRibbonStatusbarHyperlinkInactive;
			}
		}
	}

	if (clrText == (COLORREF)-1)
	{
		return CMFCVisualManagerOffice2003::GetRibbonHyperlinkTextColor(pHyperLink);
	}

	return clrText;
}

COLORREF CMFCVisualManagerOffice2007::GetRibbonEditBackgroundColor(BOOL bIsHighlighted, BOOL bIsDisabled)
{
	if (!CanDrawImage())
	{
		return (bIsHighlighted && !bIsDisabled) ? afxGlobalData.clrWindow : afxGlobalData.clrBarFace;
	}

	COLORREF color = m_clrRibbonEdit;

	if (bIsDisabled)
	{
		color = m_clrRibbonEditDisabled;
	}
	else
	{
		if (bIsHighlighted)
		{
			color = m_clrRibbonEditHighlighted;
		}
	}

	return color;
}

COLORREF CMFCVisualManagerOffice2007::GetRibbonStatusBarTextColor(CMFCRibbonStatusBar* pStatusBar)
{
	if (!CanDrawImage())
	{
		return CMFCVisualManagerOffice2003::GetRibbonStatusBarTextColor(pStatusBar);
	}

	return m_clrStatusBarText;
}

CSize CMFCVisualManagerOffice2007::GetSystemBorders(BOOL bRibbonPresent) const
{
	CSize size(::GetSystemMetrics(SM_CYSIZEFRAME), ::GetSystemMetrics(SM_CXSIZEFRAME));

	if (bRibbonPresent)
	{
		size.cx--;
		size.cy--;
	}

	return size;
}

BOOL CMFCVisualManagerOffice2007::OnEraseMDIClientArea(CDC* pDC, CRect rectClient)
{
	if (!CanDrawImage() || m_brMainClientArea.GetSafeHandle() == NULL)
	{
		return CMFCVisualManagerOffice2003::OnEraseMDIClientArea(pDC, rectClient);
	}

	pDC->FillRect(rectClient, &m_brMainClientArea);
	return TRUE;
}

BOOL CMFCVisualManagerOffice2007::GetToolTipInfo(CMFCToolTipInfo& params, UINT /*nType*/ /*= (UINT)(-1)*/)
{
	if (!CanDrawImage() || !m_bToolTipParams)
	{
		return CMFCVisualManagerOffice2003::GetToolTipInfo(params);
	}

	params = m_ToolTipParams;
	return TRUE;
}

void CMFCVisualManagerOffice2007::OnFillPopupWindowBackground(CDC* pDC, CRect rect)
{
	if (!CanDrawImage())
	{
		CMFCVisualManagerOffice2003::OnFillPopupWindowBackground(pDC, rect);
		return;
	}

	ASSERT_VALID(pDC);

	CDrawingManager dm(*pDC);
	dm.FillGradient(rect, m_clrPopupGradientDark, m_clrPopupGradientLight);
}

COLORREF CMFCVisualManagerOffice2007::OnDrawPopupWindowCaption(CDC* pDC, CRect rectCaption, CMFCDesktopAlertWnd* pPopupWnd)
{
	COLORREF clrText = CMFCVisualManagerOffice2003::OnDrawPopupWindowCaption(pDC, rectCaption, pPopupWnd);

	if (CanDrawImage())
	{
		clrText = m_clrOutlookCaptionTextNormal;
	}

	return clrText;
}

COLORREF CMFCVisualManagerOffice2007::OnDrawPropertySheetListItem(CDC* pDC, CMFCPropertySheet* pParent, CRect rect, BOOL bIsHighlihted, BOOL bIsSelected)
{
	if (!CanDrawImage() || !m_ctrlRibbonBtn[0].IsValid())
	{
		return CMFCVisualManagerOffice2003::OnDrawPropertySheetListItem(pDC, pParent, rect, bIsHighlihted, bIsSelected);
	}

	rect.DeflateRect(2, 1);

	int nIndex = 0;

	if (bIsSelected)
	{
		nIndex = bIsHighlihted ? 1 : 2;
	}

	m_ctrlRibbonBtn [0].Draw(pDC, rect, nIndex);
	return m_clrToolBarBtnTextHighlighted;
}

COLORREF CMFCVisualManagerOffice2007::OnDrawMenuLabel(CDC* pDC, CRect rect)
{
	ASSERT_VALID(pDC);

	pDC->FillRect(rect, m_brGroupBackground.GetSafeHandle() != NULL ? &m_brGroupBackground : &m_brBarBkgnd);

	CRect rectSeparator = rect;
	rectSeparator.top = rectSeparator.bottom - 2;

	DrawSeparator(pDC, rectSeparator, TRUE);

	return m_clrGroupText != (COLORREF)-1 ? m_clrGroupText : m_clrMenuText;
}

COLORREF CMFCVisualManagerOffice2007::OnFillCaptionBarButton(CDC* pDC, CMFCCaptionBar* pBar, CRect rect,
	BOOL bIsPressed, BOOL bIsHighlighted, BOOL bIsDisabled, BOOL bHasDropDownArrow, BOOL bIsSysButton)
{
	COLORREF clrText = CMFCVisualManagerOffice2003::OnFillCaptionBarButton(pDC, pBar, rect, bIsPressed, bIsHighlighted, bIsDisabled, bHasDropDownArrow, bIsSysButton);

	ASSERT_VALID(pBar);

	if (CanDrawImage() && pBar->IsMessageBarMode() && bIsSysButton && !bIsHighlighted)
	{
		clrText = m_clrMenuBarBtnText;
	}

	return clrText;
}



