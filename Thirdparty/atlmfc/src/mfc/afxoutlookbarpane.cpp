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
#include "afxmenuimages.h"
#include "afxribbonres.h"
#include "afxsettingsstore.h"
#include "afxvisualmanager.h"
#include "afxpaneframewnd.h"
#include "afxoutlookbarpane.h"
#include "afxoutlookbarpanebutton.h"
#include "afxoutlookbar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define AFX_SCROLL_BUTTON_OFFSET 5

//------------------
// Timer event IDs:
//------------------
static const UINT idScrollUp = 1;
static const UINT idScrollDn = 2;

static const int nScrollButtonMargin = 3;

static const UINT uiScrollDelay = 200; // ms

CMFCToolBarImages CMFCOutlookBarPane::m_Images;
CSize CMFCOutlookBarPane::m_csImage = CSize(0, 0);

/////////////////////////////////////////////////////////////////////////////
// CMFCOutlookBarPane

IMPLEMENT_SERIAL(CMFCOutlookBarPane, CMFCToolBar, 1)

CMFCOutlookBarPane::CMFCOutlookBarPane()
{
	m_nSize = -1;

	m_iScrollOffset = 0;
	m_iFirstVisibleButton = 0;
	m_bScrollDown = FALSE;

	m_clrRegText = (COLORREF)-1;
	m_clrBackColor = afxGlobalData.clrBtnShadow;

	m_clrTransparentColor = RGB(255, 0, 255);
	m_Images.SetTransparentColor(m_clrTransparentColor);

	m_uiBackImageId = 0;

	m_btnUp.m_nFlatStyle = CMFCButton::BUTTONSTYLE_3D;
	m_btnUp.m_bDrawFocus = FALSE;

	m_btnDown.m_nFlatStyle = CMFCButton::BUTTONSTYLE_3D;
	m_btnDown.m_bDrawFocus = FALSE;

	m_bDrawShadedHighlight = FALSE;

	m_bDisableControlsIfNoHandler = FALSE;
	m_nExtraSpace = 0;
	m_hRecentOutlookWnd = NULL;

	m_bPageScrollMode = FALSE;
	m_bDontAdjustLayout = FALSE;

	m_bLocked = TRUE;
}

CMFCOutlookBarPane::~CMFCOutlookBarPane()
{
}

BEGIN_MESSAGE_MAP(CMFCOutlookBarPane, CMFCToolBar)
	//{{AFX_MSG_MAP(CMFCOutlookBarPane)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_NCCALCSIZE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_TIMER()
	ON_WM_LBUTTONUP()
	ON_WM_SETFOCUS()
	ON_WM_CONTEXTMENU()
	ON_WM_NCPAINT()
	ON_WM_NCDESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CMFCOutlookBarPane::Create(CWnd* pParentWnd, DWORD dwStyle/* = AFX_DEFAULT_TOOLBAR_STYLE*/, UINT uiID/* = (UINT)-1*/, DWORD dwControlBarStyle/* = 0*/)
{
	if (!CMFCToolBar::Create(pParentWnd, dwStyle, uiID))
	{
		return FALSE;
	}

	m_dwControlBarStyle = dwControlBarStyle;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMFCOutlookBarPane message handlers

BOOL CMFCOutlookBarPane::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

BOOL CMFCOutlookBarPane::AddButton(LPCTSTR szBmpFileName, LPCTSTR szLabel, UINT iIdCommand, int iInsertAt)
{
	// Adds a button by loading the image from disk instead of a resource
	ENSURE(szBmpFileName != NULL);

	HBITMAP hBmp = (HBITMAP) ::LoadImage(NULL, szBmpFileName, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
	if (hBmp == NULL)
	{
		TRACE(_T("Can't load bitmap resource: %s"), szBmpFileName);
		ASSERT(FALSE);

		return FALSE;
	}

	int iImageIndex = AddBitmapImage(hBmp);
	ASSERT(iImageIndex >= 0);

	::DeleteObject(hBmp);

	return InternalAddButton(iImageIndex, szLabel, iIdCommand, iInsertAt);
}

BOOL CMFCOutlookBarPane::AddButton(UINT uiImage, UINT uiLabel, UINT iIdCommand, int iInsertAt)
{
	CString strLable;
	ENSURE(strLable.LoadString(uiLabel));

	return AddButton(uiImage, strLable, iIdCommand, iInsertAt);
}

BOOL CMFCOutlookBarPane::AddButton(UINT uiImage, LPCTSTR lpszLabel, UINT iIdCommand, int iInsertAt)
{
	int iImageIndex = -1;
	if (uiImage != 0)
	{
		CBitmap bmp;
		if (!bmp.LoadBitmap(uiImage))
		{
			TRACE(_T("Can't load bitmap resource: %d"), uiImage);
			return FALSE;
		}

		iImageIndex = AddBitmapImage((HBITMAP) bmp.GetSafeHandle());
	}

	return InternalAddButton(iImageIndex, lpszLabel, iIdCommand, iInsertAt);
}

BOOL CMFCOutlookBarPane::AddButton(HBITMAP hBmp, LPCTSTR lpszLabel, UINT iIdCommand, int iInsertAt)
{
	ENSURE(hBmp != NULL);

	int iImageIndex = AddBitmapImage(hBmp);
	return InternalAddButton(iImageIndex, lpszLabel, iIdCommand, iInsertAt);
}

BOOL CMFCOutlookBarPane::AddButton(HICON hIcon, LPCTSTR lpszLabel, UINT iIdCommand, int iInsertAt, BOOL bAlphaBlend)
{
	ENSURE(hIcon != NULL);

	int iImageIndex = -1;

	ICONINFO iconInfo;
	::GetIconInfo(hIcon, &iconInfo);

	BITMAP bitmap;
	::GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bitmap);

	CSize size(bitmap.bmWidth, bitmap.bmHeight);

	if (bAlphaBlend)
	{
		if (m_Images.GetCount() == 0) // First image
		{
			m_csImage = size;
			m_Images.SetImageSize(size);
		}

		iImageIndex = m_Images.AddIcon(hIcon, TRUE);
	}
	else
	{
		CClientDC dc(this);

		CDC dcMem;
		dcMem.CreateCompatibleDC(&dc);

		CBitmap bmp;
		bmp.CreateCompatibleBitmap(&dc, size.cx, size.cy);

		CBitmap* pOldBmp = dcMem.SelectObject(&bmp);

		if (m_clrTransparentColor != (COLORREF)-1)
		{
			dcMem.FillSolidRect(0, 0, size.cx, size.cy, m_clrTransparentColor);
		}

		::DrawIconEx(dcMem.GetSafeHdc(), 0, 0, hIcon, size.cx, size.cy, 0, NULL, DI_NORMAL);

		dcMem.SelectObject(pOldBmp);

		::DeleteObject(iconInfo.hbmColor);
		::DeleteObject(iconInfo.hbmMask);

		iImageIndex = AddBitmapImage((HBITMAP) bmp.GetSafeHandle());
	}

	return InternalAddButton(iImageIndex, lpszLabel, iIdCommand, iInsertAt);
}

BOOL CMFCOutlookBarPane::RemoveButton(UINT iIdCommand)
{
	for (POSITION pos = m_Buttons.GetHeadPosition(); pos != NULL;)
	{
		POSITION posSave = pos;

		CMFCOutlookBarPaneButton* pButton = (CMFCOutlookBarPaneButton*) m_Buttons.GetNext(pos);
		ENSURE(pButton != NULL);

		if (pButton->m_nID == iIdCommand)
		{
			m_Buttons.RemoveAt(posSave);
			delete pButton;

			if (GetSafeHwnd() != NULL)
			{
				AdjustLocations();
				UpdateWindow();
				Invalidate();
			}

			return TRUE;
		}
	}

	return FALSE;
}

BOOL CMFCOutlookBarPane::InternalAddButton(int iImageIndex, LPCTSTR lpszLabel, UINT iIdCommand, int iInsertAt)
{
	CMFCOutlookBarPaneButton* pButton = new CMFCOutlookBarPaneButton;
	ENSURE(pButton != NULL);

	pButton->m_nID = iIdCommand;
	pButton->m_strText = (lpszLabel == NULL) ? _T("") : lpszLabel;
	pButton->SetImage(iImageIndex);
	pButton->m_bTextBelow = m_bTextLabels;

	if (iInsertAt == -1)
	{
		iInsertAt = (int) m_Buttons.GetCount();
	}

	InsertButton(pButton, iInsertAt);

	AdjustLayout();
	return TRUE;
}

int CMFCOutlookBarPane::AddBitmapImage(HBITMAP hBitmap)
{
	ENSURE(hBitmap != NULL);

	BITMAP bitmap;
	::GetObject(hBitmap, sizeof(BITMAP), &bitmap);

	CSize csImage = CSize(bitmap.bmWidth, bitmap.bmHeight);

	if (m_Images.GetCount() == 0) // First image
	{
		m_csImage = csImage;
		m_Images.SetImageSize(csImage);
	}
	else
	{
		ASSERT(m_csImage == csImage); // All buttons should be of the same size!
	}

	return m_Images.AddImage(hBitmap);
}

void CMFCOutlookBarPane::OnSize(UINT nType, int cx, int cy)
{
	CMFCToolBar::OnSize(nType, cx, cy);

	if (!m_bDontAdjustLayout)
	{
		AdjustLayout();
	}
	else
	{
		AdjustLocations();
	}

	int iButtons = (int) m_Buttons.GetCount();
	if (iButtons > 0)
	{
		POSITION posLast = m_Buttons.FindIndex(iButtons - 1);
		CMFCOutlookBarPaneButton* pButtonLast = (CMFCOutlookBarPaneButton*) m_Buttons.GetAt(posLast);
		ENSURE(pButtonLast != NULL);

		while (m_iScrollOffset > 0 &&
			pButtonLast->Rect().bottom < cy)
		{
			ScrollUp();
		}
	}
}

void CMFCOutlookBarPane::SetTextColor(COLORREF clrRegText, COLORREF/* clrSelText obsolete*/)
{
	m_clrRegText = clrRegText;
	if (GetSafeHwnd() != NULL)
	{
		Invalidate();
		UpdateWindow();
	}
}

int CMFCOutlookBarPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMFCToolBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetPaneStyle(m_dwStyle & ~(CBRS_BORDER_ANY | CBRS_GRIPPER));

	m_cxLeftBorder = m_cxRightBorder = 0;
	m_cyTopBorder = m_cyBottomBorder = 0;

	//-------------------------------------------
	// Adjust Z-order in the parent frame window:
	//-------------------------------------------
	SetWindowPos(&wndBottom, 0,0,0,0,SWP_NOSIZE|SWP_NOMOVE | SWP_NOACTIVATE);

	//-----------------------
	// Create scroll buttons:
	//-----------------------
	CRect rectDummy(CPoint(0, 0), CMenuImages::Size());
	rectDummy.InflateRect(nScrollButtonMargin, nScrollButtonMargin);

	m_btnUp.Create(_T(""), WS_CHILD | BS_PUSHBUTTON, rectDummy, this, (UINT)-1);
	m_btnUp.SetStdImage(CMenuImages::IdArrowUpLarge);

	m_btnDown.Create(_T(""), WS_CHILD | BS_PUSHBUTTON, rectDummy, this, (UINT)-1);
	m_btnDown.SetStdImage(CMenuImages::IdArrowDownLarge);

	return 0;
}

void CMFCOutlookBarPane::ScrollUp()
{
	if (m_iScrollOffset <= 0 || m_iFirstVisibleButton <= 0)
	{
		m_iScrollOffset = 0;
		m_iFirstVisibleButton = 0;

		KillTimer(idScrollUp);
		return;
	}

	CMFCToolBarButton* pFirstVisibleButton = GetButton(m_iFirstVisibleButton);
	if (pFirstVisibleButton == NULL)
	{
		KillTimer(idScrollDn);
		return;
	}

	m_iFirstVisibleButton--;
	m_iScrollOffset -= pFirstVisibleButton->Rect().Height();

	if (m_iFirstVisibleButton == 0)
	{
		m_iScrollOffset = 0;
	}

	ASSERT(m_iScrollOffset >= 0);

	AdjustLocations();
	Invalidate();
	UpdateWindow();
}

void CMFCOutlookBarPane::ScrollDown()
{
	if (!m_bScrollDown || m_iFirstVisibleButton + 1 >= GetCount())
	{
		KillTimer(idScrollDn);
		return;
	}

	CMFCToolBarButton* pFirstVisibleButton = GetButton(m_iFirstVisibleButton);
	if (pFirstVisibleButton == NULL)
	{
		KillTimer(idScrollDn);
		return;
	}

	m_iFirstVisibleButton++;
	m_iScrollOffset += pFirstVisibleButton->Rect().Height();

	AdjustLocations();
	Invalidate();
	UpdateWindow();
}

CSize CMFCOutlookBarPane::CalcFixedLayout(BOOL /*bStretch*/, BOOL /*bHorz*/)
{
	CRect rect;
	GetClientRect(rect);

	return rect.Size();
}

void CMFCOutlookBarPane::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	CRect rect;
	rect.SetRectEmpty();

	CPane::CalcInsideRect(rect, FALSE);

	// adjust non-client area for border space
	lpncsp->rgrc[0].left += rect.left;
	lpncsp->rgrc[0].top += rect.top;
	lpncsp->rgrc[0].right += rect.right;
	lpncsp->rgrc[0].bottom += rect.bottom;
}

void CMFCOutlookBarPane::SetBackImage(UINT uiImageID)
{
	if (m_uiBackImageId == uiImageID)
	{
		return;
	}

	m_bDrawShadedHighlight = FALSE;
	if (m_bmpBack.GetCount() > 0)
	{
		m_bmpBack.Clear();
	}

	m_uiBackImageId = 0;
	if (uiImageID != 0)
	{
		LPCTSTR lpszResourceName = MAKEINTRESOURCE(uiImageID);
		ENSURE(lpszResourceName != NULL);

		HBITMAP hbmp = (HBITMAP) ::LoadImage(AfxFindResourceHandle(lpszResourceName, RT_BITMAP), 
			lpszResourceName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS);
		if (hbmp != NULL)
		{
			BITMAP bitmap;
			::GetObject(hbmp, sizeof(BITMAP), (LPVOID) &bitmap);

			m_bmpBack.SetImageSize(CSize(bitmap.bmWidth, bitmap.bmHeight));
			m_bmpBack.AddImage(hbmp);
			m_uiBackImageId = uiImageID;
		}

		m_bDrawShadedHighlight = (afxGlobalData.m_nBitsPerPixel > 8); // For 16 bits or greater
	}

	if (GetSafeHwnd() != NULL)
	{
		Invalidate();
		UpdateWindow();
	}
}

void CMFCOutlookBarPane::SetBackColor(COLORREF color)
{
	m_clrBackColor = color;
	if (GetSafeHwnd() != NULL)
	{
		Invalidate();
		UpdateWindow();
	}
}

void CMFCOutlookBarPane::SetTransparentColor(COLORREF color)
{
	m_clrTransparentColor = color;
	if (GetSafeHwnd() != NULL)
	{
		m_Images.SetTransparentColor(m_clrTransparentColor);
		Invalidate();
		UpdateWindow();
	}
}

void CMFCOutlookBarPane::OnSysColorChange()
{
	CMFCToolBar::OnSysColorChange();

	m_clrBackColor = afxGlobalData.clrBtnShadow;

	if (m_uiBackImageId != 0)
	{
		int uiImage = m_uiBackImageId;
		m_uiBackImageId = (UINT) -1;

		SetBackImage(uiImage);
	}
	else
	{
		Invalidate();
	}
}

void CMFCOutlookBarPane::AdjustLocations()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	ASSERT_VALID(this);


	CSize sizeBtn = CMenuImages::Size() + CSize(2 * nScrollButtonMargin, 2 * nScrollButtonMargin);

	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontRegular);

	CRect rectClient;
	GetClientRect(rectClient);

	CSize sizeDefault = CSize(rectClient.Width() - 2, m_csImage.cy);

	if (IsButtonExtraSizeAvailable())
	{
		sizeDefault += CMFCVisualManager::GetInstance()->GetButtonExtraBorder();
	}

	int iOffset = rectClient.top - m_iScrollOffset + m_nExtraSpace;

	if (m_iFirstVisibleButton > 0 && sizeBtn.cx <= rectClient.Width() - AFX_SCROLL_BUTTON_OFFSET && sizeBtn.cy <= rectClient.Height() - AFX_SCROLL_BUTTON_OFFSET)
	{
		int nAdjButton = AFX_SCROLL_BUTTON_OFFSET;

		m_btnUp.SetWindowPos(NULL, rectClient.right - sizeBtn.cx - nAdjButton, rectClient.top + AFX_SCROLL_BUTTON_OFFSET, -1, -1, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
		m_btnUp.ShowWindow(SW_SHOWNOACTIVATE);
	}
	else
	{
		m_btnUp.ShowWindow(SW_HIDE);
	}

	for (POSITION pos = m_Buttons.GetHeadPosition(); pos != NULL;)
	{
		CMFCOutlookBarPaneButton* pButton = (CMFCOutlookBarPaneButton*) m_Buttons.GetNext(pos);
		ENSURE(pButton != NULL);

		pButton->m_bTextBelow = m_bTextLabels;
		pButton->m_sizeImage = m_csImage;

		CSize sizeButton = pButton->OnCalculateSize(&dc, sizeDefault, FALSE);

		CRect rectButton;

		int nWidth = rectClient.Width() - 1;
		sizeButton.cx = min(nWidth, sizeButton.cx);

		rectButton = CRect(CPoint(rectClient.left +(nWidth - sizeButton.cx) / 2, iOffset), sizeButton);
		iOffset = rectButton.bottom + m_nExtraSpace;

		pButton->SetRect(rectButton);
	}

	m_bScrollDown = (iOffset > rectClient.bottom);

	if (m_bScrollDown && sizeBtn.cx <= rectClient.Width() - AFX_SCROLL_BUTTON_OFFSET && sizeBtn.cy <= rectClient.Height() - AFX_SCROLL_BUTTON_OFFSET)
	{
		int nAdjButton = AFX_SCROLL_BUTTON_OFFSET;

		m_btnDown.SetWindowPos(&wndTop, rectClient.right - sizeBtn.cx - nAdjButton, rectClient.bottom - sizeBtn.cy - AFX_SCROLL_BUTTON_OFFSET, -1, -1, SWP_NOSIZE | SWP_NOACTIVATE);
		m_btnDown.ShowWindow(SW_SHOWNOACTIVATE);
	}
	else
	{
		m_btnDown.ShowWindow(SW_HIDE);
	}

	dc.SelectObject(pOldFont);

	m_btnUp.RedrawWindow();
	m_btnDown.RedrawWindow();

	OnMouseLeave(0, 0);
	UpdateTooltips();
}

void CMFCOutlookBarPane::DoPaint(CDC* pDCPaint)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDCPaint);

	CRect rectClip;

	GetClientRect(rectClip);

	CRect rectClient;
	GetClientRect(rectClient);

	CMemDC memDC(*pDCPaint, this);
	CDC* pDC = &memDC.GetDC();

	CMFCVisualManager::GetInstance()->OnFillBarBackground(pDC, this, rectClient, rectClient);

	if (!m_Buttons.IsEmpty())
	{
		pDC->SetTextColor(afxGlobalData.clrBtnText);
		pDC->SetBkMode(TRANSPARENT);

		CAfxDrawState ds;
		if (!m_Images.PrepareDrawImage(ds))
		{
			ASSERT(FALSE);
			return;     // something went wrong
		}

		CFont* pOldFont = pDC->SelectObject(&afxGlobalData.fontRegular);

		//--------------
		// Draw buttons:
		//--------------
		int iButton = 0;
		for (POSITION pos = m_Buttons.GetHeadPosition(); pos != NULL; iButton ++)
		{
			CMFCOutlookBarPaneButton* pButton = (CMFCOutlookBarPaneButton*) m_Buttons.GetNext(pos);
			ASSERT_VALID(pButton);

			CRect rect = pButton->Rect();

			BOOL bHighlighted = FALSE;

			if (IsCustomizeMode() && !m_bLocked)
			{
				bHighlighted = FALSE;
			}
			else
			{
				bHighlighted = ((iButton == m_iHighlighted || iButton == m_iButtonCapture) && (m_iButtonCapture == -1 || iButton == m_iButtonCapture));
			}

			CRect rectInter;
			if (rectInter.IntersectRect(rect, rectClip))
			{
				pButton->OnDraw(pDC, rect, &m_Images, FALSE, IsCustomizeMode(), bHighlighted);
			}
		}

		//-------------------------------------------------------------
		// Highlight selected button in the toolbar customization mode:
		//-------------------------------------------------------------
		if (m_iSelected >= m_Buttons.GetCount())
		{
			m_iSelected = -1;
		}

		if (IsCustomizeMode() && m_iSelected >= 0 && !m_bLocked)
		{
			CMFCToolBarButton* pSelButton = GetButton(m_iSelected);
			ENSURE(pSelButton != NULL);

			if (pSelButton != NULL && pSelButton->CanBeStored())
			{
				CRect rectDrag = pSelButton->Rect();
				if (pSelButton->GetHwnd() != NULL)
				{
					rectDrag.InflateRect(0, 1);
				}

				pDC->DrawDragRect(&rectDrag, CSize(2, 2), NULL, CSize(2, 2));
			}
		}

		if (IsCustomizeMode() && m_iDragIndex >= 0 && !m_bLocked)
		{
			DrawDragCursor(pDC);
		}

		pDC->SelectClipRgn(NULL);
		pDC->SelectObject(pOldFont);

		m_Images.EndDrawImage(ds);
	}
}

DROPEFFECT CMFCOutlookBarPane::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	CMFCToolBarButton* pButton = CMFCToolBarButton::CreateFromOleData(pDataObject);
	if (pButton == NULL)
	{
		return DROPEFFECT_NONE;
	}

	BOOL bAllowDrop = pButton->IsKindOf(RUNTIME_CLASS(CMFCOutlookBarPaneButton));
	delete pButton;

	if (!bAllowDrop)
	{
		return DROPEFFECT_NONE;
	}

	CRect rectUp;
	m_btnUp.GetWindowRect(rectUp);
	ScreenToClient(rectUp);

	if (rectUp.PtInRect(point))
	{
		ScrollUp();
		return DROPEFFECT_NONE;
	}

	CRect rectDown;
	m_btnDown.GetWindowRect(rectDown);
	ScreenToClient(rectDown);

	if (rectDown.PtInRect(point))
	{
		ScrollDown();
		return DROPEFFECT_NONE;
	}

	return CMFCToolBar::OnDragOver(pDataObject, dwKeyState, point);
}

CMFCToolBarButton* CMFCOutlookBarPane::CreateDroppedButton(COleDataObject* pDataObject)
{
	CMFCToolBarButton* pButton = CMFCToolBar::CreateDroppedButton(pDataObject);
	ENSURE(pButton != NULL);

	CMFCOutlookBarPaneButton* pOutlookButton = DYNAMIC_DOWNCAST(CMFCOutlookBarPaneButton, pButton);
	if (pOutlookButton == NULL)
	{
		delete pButton;

		ASSERT(FALSE);
		return NULL;
	}

	return pButton;
}

BOOL CMFCOutlookBarPane::EnableContextMenuItems(CMFCToolBarButton* pButton, CMenu* pPopup)
{
	ASSERT_VALID(pButton);
	ASSERT_VALID(pPopup);

	if (IsCustomizeMode())
	{
		pPopup->EnableMenuItem(ID_AFXBARRES_TOOLBAR_IMAGE, MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(ID_AFXBARRES_TOOLBAR_TEXT, MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(ID_AFXBARRES_TOOLBAR_IMAGE_AND_TEXT, MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(ID_AFXBARRES_TOOLBAR_APPEARANCE, MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(ID_AFXBARRES_TOOLBAR_START_GROUP, MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(ID_AFXBARRES_TOOLBAR_RESET, MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(ID_AFXBARRES_COPY_IMAGE, MF_GRAYED | MF_BYCOMMAND);
	}

	CMFCToolBar::EnableContextMenuItems(pButton, pPopup);
	return TRUE;
}

BOOL CMFCOutlookBarPane::PreTranslateMessage(MSG* pMsg)
{
	switch(pMsg->message)
	{
	case WM_LBUTTONUP:
		KillTimer(idScrollUp);
		KillTimer(idScrollDn);

	case WM_LBUTTONDOWN:
	case WM_MOUSEMOVE:
		{
			CPoint ptCursor;
			::GetCursorPos(&ptCursor);
			ScreenToClient(&ptCursor);

			CRect rect;
			m_btnDown.GetClientRect(rect);
			m_btnDown.MapWindowPoints(this, rect);

			if (rect.PtInRect(ptCursor))
			{
				m_btnDown.SendMessage(pMsg->message, pMsg->wParam, pMsg->wParam);
				if (pMsg->message == WM_LBUTTONDOWN)
				{
					SetTimer(idScrollDn, uiScrollDelay, NULL);

					if (m_bPageScrollMode)
					{
						ScrollPageDown();

					}else
					{
						ScrollDown();
					}
				}
			}

			m_btnUp.GetClientRect(rect);
			m_btnUp.MapWindowPoints(this, rect);

			if (rect.PtInRect(ptCursor))
			{
				m_btnUp.SendMessage(pMsg->message, pMsg->wParam, pMsg->wParam);

				if (pMsg->message == WM_LBUTTONDOWN)
				{
					SetTimer(idScrollUp, uiScrollDelay, NULL);

					if (m_bPageScrollMode)
					{
						ScrollPageUp();
					}
					else
					{
						ScrollUp();
					}
				}
			}
		}
		break;
	}

	return CMFCToolBar::PreTranslateMessage(pMsg);
}

void CMFCOutlookBarPane::OnTimer(UINT_PTR nIDEvent)
{
	switch(nIDEvent)
	{
	case idScrollUp:
		if (m_btnUp.IsPressed())
		{
			if (m_bPageScrollMode)
			{
				ScrollPageUp();
			}
			else
			{
				ScrollUp();
			}
		}
		return;

	case idScrollDn:
		if (m_btnDown.IsPressed())
		{
			if (m_bPageScrollMode)
			{
				ScrollPageDown();
			}
			else
			{
				ScrollDown();
			}
		}
		return;
	}

	CMFCToolBar::OnTimer(nIDEvent);
}

void CMFCOutlookBarPane::OnLButtonUp(UINT nFlags, CPoint point)
{
	HWND hWnd = GetSafeHwnd();
	CMFCToolBar::OnLButtonUp(nFlags, point);

	if (::IsWindow(hWnd))
	{
		OnMouseLeave(0, 0);
	}
}

void CMFCOutlookBarPane::RemoveAllButtons()
{
	CMFCToolBar::RemoveAllButtons();

	m_iFirstVisibleButton = 0;
	m_iScrollOffset = 0;

	AdjustLocations();

	if (m_hWnd != NULL)
	{
		UpdateWindow();
		Invalidate();
	}
}

void CMFCOutlookBarPane::ClearAll()
{
	m_Images.Clear();
}

void CMFCOutlookBarPane::OnEraseWorkArea(CDC* pDC, CRect rectWorkArea)
{
	ASSERT_VALID(pDC);

	if (m_bmpBack.GetCount() == 0)
	{
		CBrush br(m_clrBackColor);
		pDC->FillRect(rectWorkArea, &br);
	}
	else
	{
		ASSERT(m_bmpBack.GetCount() == 1);

		CAfxDrawState ds;
		m_bmpBack.PrepareDrawImage(ds);
		CSize sizeBack = m_bmpBack.GetImageSize();

		for (int x = rectWorkArea.left; x < rectWorkArea.right; x += sizeBack.cx)
		{
			for (int y = rectWorkArea.top; y < rectWorkArea.bottom; y += sizeBack.cy)
			{
				m_bmpBack.Draw(pDC, x, y, 0);
			}
		}

		m_bmpBack.EndDrawImage(ds);
	}
}

AFX_CS_STATUS CMFCOutlookBarPane::IsChangeState(int /*nOffset*/, CBasePane** ppTargetBar) const
{
	ASSERT_VALID(this);
	ENSURE(ppTargetBar != NULL);

	CPoint ptMousePos;
	GetCursorPos(&ptMousePos);

	*ppTargetBar = NULL;

	// check whether the mouse is around a dock bar
	CMFCOutlookBar* pOutlookBar = DYNAMIC_DOWNCAST(CMFCOutlookBar, PaneFromPoint(ptMousePos, 0, FALSE, RUNTIME_CLASS(CMFCOutlookBar)));

	if (pOutlookBar != NULL)
	{
		*ppTargetBar = pOutlookBar;
		return CS_DOCK_IMMEDIATELY;
	}
	return CS_NOTHING;
}

BOOL CMFCOutlookBarPane::Dock(CBasePane* pDockBar, LPCRECT /*lpRect*/, AFX_DOCK_METHOD dockMethod)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDockBar);

	CPaneFrameWnd* pParentMiniFrame = GetParentMiniFrame();

	CString strText;
	GetWindowText(strText);

	CMFCOutlookBar* pOutlookBar = NULL;

	if (dockMethod == DM_DBL_CLICK)
	{
		pOutlookBar = DYNAMIC_DOWNCAST(CMFCOutlookBar, CWnd::FromHandlePermanent(m_hRecentOutlookWnd));
	}
	else if (dockMethod == DM_MOUSE)
	{
		pOutlookBar = DYNAMIC_DOWNCAST(CMFCOutlookBar, pDockBar);
	}

	if (pOutlookBar == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(pOutlookBar);
	if (pParentMiniFrame != NULL)
	{
		pParentMiniFrame->RemovePane(this);
	}

	pOutlookBar->AddTab(this);

	CMFCBaseTabCtrl* pTabWnd = pOutlookBar->GetUnderlyingWindow();
	ASSERT_VALID(pTabWnd);

	int nAddedTab = pTabWnd->GetTabsNum() - 1;
	pTabWnd->SetTabLabel(nAddedTab, strText);
	pTabWnd->SetActiveTab(nAddedTab);

	return TRUE;
}

BOOL CMFCOutlookBarPane::OnBeforeFloat(CRect& /*rectFloat*/, AFX_DOCK_METHOD dockMethod)
{
	if (dockMethod == DM_MOUSE)
	{
		CPoint ptMouse;
		GetCursorPos(&ptMouse);

		CWnd* pParent = GetParent();

		// make it float only when the mouse is out of parent bounds
		CRect rect;
		pParent->GetWindowRect(rect);
		BOOL bFloat = !rect.PtInRect(ptMouse);
		if (bFloat)
		{
			if (pParent->IsKindOf(RUNTIME_CLASS(CMFCOutlookBar)))
			{
				m_hRecentOutlookWnd = pParent->GetSafeHwnd();
			}
			else
			{
				m_hRecentOutlookWnd = pParent->GetParent()->GetSafeHwnd();
			}
		}
		return bFloat;
	}

	return TRUE;
}

void CMFCOutlookBarPane::OnSetFocus(CWnd* pOldWnd)
{
	// bypass the standard toolbar's set focus, because it sets the focus back
	// to old focused wnd
	CPane::OnSetFocus(pOldWnd);
}

void CMFCOutlookBarPane::OnNcDestroy()
{
	CMFCToolBar::OnNcDestroy();
}

void CMFCOutlookBarPane::SetDefaultState()
{
	CopyButtonsList(m_Buttons, m_OrigButtons);
}

BOOL CMFCOutlookBarPane::RestoreOriginalstate()
{
	if (m_OrigButtons.IsEmpty())
	{
		return FALSE;
	}

	CopyButtonsList(m_OrigButtons, m_Buttons);

	AdjustLayout();
	RedrawWindow();
	return TRUE;
}

BOOL CMFCOutlookBarPane::SmartUpdate(const CObList& lstPrevButtons)
{
	if (lstPrevButtons.IsEmpty())
	{
		return FALSE;
	}

	m_bResourceWasChanged = FALSE; // Outlook bar has its own resources

	BOOL bIsModified = FALSE;

	//-----------------------------------
	// Compare current and prev. buttons:
	//------------------------------------
	if (lstPrevButtons.GetCount() != m_OrigButtons.GetCount())
	{
		bIsModified = TRUE;
	}
	else
	{
		POSITION posCurr, posPrev;
		for (posCurr = m_OrigButtons.GetHeadPosition(), posPrev = lstPrevButtons.GetHeadPosition(); posCurr != NULL;)
		{
			ENSURE(posPrev != NULL);

			CMFCToolBarButton* pButtonCurr = DYNAMIC_DOWNCAST(CMFCToolBarButton, m_OrigButtons.GetNext(posCurr));
			ASSERT_VALID(pButtonCurr);

			CMFCToolBarButton* pButtonPrev = DYNAMIC_DOWNCAST(CMFCToolBarButton, lstPrevButtons.GetNext(posPrev));
			ASSERT_VALID(pButtonPrev);

			if (!pButtonCurr->CompareWith(*pButtonPrev))
			{
				bIsModified = TRUE;
				break;
			}
		}
	}

	if (bIsModified)
	{
		RestoreOriginalstate();
	}

	return bIsModified;
}

void CMFCOutlookBarPane::CopyButtonsList(const CObList& lstSrc, CObList& lstDst)
{
	while (!lstDst.IsEmpty())
	{
		delete lstDst.RemoveHead();
	}

	for (POSITION pos = lstSrc.GetHeadPosition(); pos != NULL;)
	{
		CMFCToolBarButton* pButtonSrc = (CMFCToolBarButton*) lstSrc.GetNext(pos);
		ASSERT_VALID(pButtonSrc);

		CRuntimeClass* pClass = pButtonSrc->GetRuntimeClass();
		ENSURE(pClass != NULL);

		CMFCToolBarButton* pButton = (CMFCToolBarButton*) pClass->CreateObject();
		ASSERT_VALID(pButton);

		pButton->CopyFrom(*pButtonSrc);
		pButton->OnChangeParentWnd(this);

		lstDst.AddTail(pButton);
	}
}

void CMFCOutlookBarPane::ScrollPageUp()
{
	if (m_iScrollOffset <= 0 ||
		m_iFirstVisibleButton <= 0)
	{
		m_iScrollOffset = 0;
		m_iFirstVisibleButton = 0;

		KillTimer(idScrollUp);
		return;
	}

	CMFCToolBarButton* pFirstVisibleButton = GetButton(m_iFirstVisibleButton);
	if (pFirstVisibleButton == NULL)
	{
		KillTimer(idScrollDn);
		return;
	}

	CRect rcArea;
	GetClientRect(rcArea);
	int nVisibleCount = 0;

	nVisibleCount = (rcArea.Height())/(pFirstVisibleButton->Rect().Height() + m_nExtraSpace);

	for (int i=0; i<nVisibleCount; i++)
	{
		ScrollUp();
	}
}

void CMFCOutlookBarPane::ScrollPageDown()
{
	if (!m_bScrollDown || m_iFirstVisibleButton + 1 >= GetCount())
	{
		KillTimer(idScrollDn);
		return;
	}

	CMFCToolBarButton* pFirstVisibleButton = GetButton(m_iFirstVisibleButton);
	if (pFirstVisibleButton == NULL)
	{
		KillTimer(idScrollDn);
		return;
	}

	CRect rcArea;
	GetClientRect(rcArea);
	int nVisibleCount = 0;

	nVisibleCount = (rcArea.Height())/(pFirstVisibleButton->Rect().Height() + m_nExtraSpace);

	for (int i=0; i<nVisibleCount; i++)
	{
		ScrollDown();
	}
}



