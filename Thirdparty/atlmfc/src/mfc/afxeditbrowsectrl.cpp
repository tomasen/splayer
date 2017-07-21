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
#include "afxglobals.h"
#include "afxtoolbarimages.h"
#include "afxvisualmanager.h"
#include "afxribbonres.h"
#include "afxshellmanager.h"
#include "afxeditbrowsectrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCEditBrowseCtrl

CMFCEditBrowseCtrl::CMFCEditBrowseCtrl()
{
	m_rectBtn.SetRectEmpty();
	m_bIsButtonPressed = FALSE;
	m_bIsButtonHighlighted = FALSE;
	m_bIsButtonCaptured = FALSE;
	m_Mode = BrowseMode_None;
	m_sizeImage = CSize(0, 0);
	m_nBrowseButtonWidth = 20;
	m_bDefaultImage = TRUE;
}

CMFCEditBrowseCtrl::~CMFCEditBrowseCtrl()
{
}

BEGIN_MESSAGE_MAP(CMFCEditBrowseCtrl, CEdit)
	//{{AFX_MSG_MAP(CMFCEditBrowseCtrl)
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_NCHITTEST()
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_NCMOUSEMOVE()
	ON_WM_CANCELMODE()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCEditBrowseCtrl message handlers

void CMFCEditBrowseCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bIsButtonCaptured)
	{
		ReleaseCapture();

		m_bIsButtonPressed = FALSE;
		m_bIsButtonCaptured = FALSE;
		m_bIsButtonHighlighted = FALSE;

		RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE);

		if (m_rectBtn.PtInRect(point))
		{
			OnBrowse();
		}

		return;
	}

	CEdit::OnLButtonUp(nFlags, point);
}

void CMFCEditBrowseCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bIsButtonCaptured)
	{
		BOOL bIsButtonPressed = m_rectBtn.PtInRect(point);
		if (bIsButtonPressed != m_bIsButtonPressed)
		{
			m_bIsButtonPressed = bIsButtonPressed;
			RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
		}

		return;
	}

	if (m_bIsButtonHighlighted)
	{
		if (!m_rectBtn.PtInRect(point))
		{
			m_bIsButtonHighlighted = FALSE;
			ReleaseCapture();

			RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
		}
	}

	CEdit::OnMouseMove(nFlags, point);
}

void CMFCEditBrowseCtrl::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	CEdit::OnNcCalcSize(bCalcValidRects, lpncsp);

	if (m_Mode != BrowseMode_None)
	{
		lpncsp->rgrc [0].right -= m_nBrowseButtonWidth;
	}
}

void CMFCEditBrowseCtrl::OnNcPaint()
{
	CEdit::OnNcPaint();

	if (m_Mode == BrowseMode_None)
	{
		return;
	}

	CWindowDC dc(this);

	CRect rectWindow;
	GetWindowRect(rectWindow);

	m_rectBtn = rectWindow;
	m_rectBtn.left = m_rectBtn.right -  m_nBrowseButtonWidth;

	CRect rectClient;
	GetClientRect(rectClient);
	ClientToScreen(&rectClient);

	m_rectBtn.OffsetRect(rectClient.right + m_nBrowseButtonWidth - rectWindow.right, 0);
	m_rectBtn.top += rectClient.top - rectWindow.top;
	m_rectBtn.bottom -= rectWindow.bottom - rectClient.bottom;

	CRect rect = m_rectBtn;
	rect.OffsetRect(-rectWindow.left, -rectWindow.top);

	CRgn rgnClip;
	rgnClip.CreateRectRgnIndirect(&rect);

	dc.SelectClipRgn(&rgnClip);

	OnDrawBrowseButton(&dc, rect, m_bIsButtonPressed, m_bIsButtonHighlighted);

	dc.SelectClipRgn(NULL);

	ScreenToClient(&m_rectBtn);
}

LRESULT CMFCEditBrowseCtrl::OnNcHitTest(CPoint point)
{
	CPoint ptClient = point;
	ScreenToClient(&ptClient);

	if (m_Mode != BrowseMode_None && m_rectBtn.PtInRect(ptClient))
	{
		return HTCAPTION;
	}

	return CEdit::OnNcHitTest(point);
}

void CMFCEditBrowseCtrl::OnDrawBrowseButton(CDC* pDC, CRect rect, BOOL bIsButtonPressed, BOOL bHighlight)
{
	ASSERT(m_Mode != BrowseMode_None);
	ASSERT_VALID(pDC);

	CMFCVisualManager::AFX_BUTTON_STATE state = CMFCVisualManager::ButtonsIsRegular;

	if (bIsButtonPressed)
	{
		state = CMFCVisualManager::ButtonsIsPressed;
	}
	else if (bHighlight)
	{
		state = CMFCVisualManager::ButtonsIsHighlighted;
	}

	COLORREF clrText = afxGlobalData.clrBtnText;

	if (!CMFCVisualManager::GetInstance()->OnDrawBrowseButton(pDC, rect, this, state, clrText))
	{
		return;
	}

	int iImage = 0;

	if (m_ImageBrowse.GetSafeHandle() != NULL)
	{
		if (m_bDefaultImage)
		{
			switch (m_Mode)
			{
			case BrowseMode_Folder:
				iImage = 0;
				break;

			case BrowseMode_File:
				iImage = 1;
				break;
			}
		}

		CPoint ptImage;
		ptImage.x = rect.CenterPoint().x - m_sizeImage.cx / 2;
		ptImage.y = rect.CenterPoint().y - m_sizeImage.cy / 2;

		if (bIsButtonPressed && CMFCVisualManager::GetInstance()->IsOffsetPressedButton())
		{
			ptImage.x++;
			ptImage.y++;
		}

		m_ImageBrowse.Draw(pDC, iImage, ptImage, ILD_NORMAL);
	}
	else if (!m_strLabel.IsEmpty())
	{
		COLORREF clrTextOld = pDC->SetTextColor(clrText);
		int nTextMode = pDC->SetBkMode(TRANSPARENT);
		CFont* pFont = (CFont*) pDC->SelectStockObject(DEFAULT_GUI_FONT);

		CRect rectText = rect;
		rectText.DeflateRect(1, 2);
		rectText.OffsetRect(0, -2);

		if (bIsButtonPressed)
		{
			rectText.OffsetRect(1, 1);
		}

		pDC->DrawText(m_strLabel, rectText, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

		pDC->SetTextColor(clrTextOld);
		pDC->SetBkMode(nTextMode);
		pDC->SelectObject(pFont);
	}
}

void CMFCEditBrowseCtrl::EnableBrowseButton(BOOL bEnable/* = TRUE*/, LPCTSTR szLabel/* = _T("...")*/)
{
	ASSERT_VALID(this);
	ENSURE(GetSafeHwnd() != NULL);
	ENSURE(szLabel != NULL);

	m_Mode = bEnable ? BrowseMode_Default : BrowseMode_None;
	m_strLabel = szLabel;

	m_ImageBrowse.DeleteImageList();
	m_sizeImage = CSize(0, 0);

	OnChangeLayout();
}

void CMFCEditBrowseCtrl::OnChangeLayout()
{
	ASSERT_VALID(this);
	ENSURE(GetSafeHwnd() != NULL);

	m_nBrowseButtonWidth = max(20, m_sizeImage.cx + 8);

	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER | SWP_NOMOVE);

	if (m_Mode != BrowseMode_None)
	{
		GetWindowRect(m_rectBtn);
		m_rectBtn.left = m_rectBtn.right -  m_nBrowseButtonWidth;

		ScreenToClient(&m_rectBtn);
	}
	else
	{
		m_rectBtn.SetRectEmpty();
	}
}

void CMFCEditBrowseCtrl::OnNcLButtonDblClk(UINT /*nHitTest*/, CPoint /*point*/)
{
}

void CMFCEditBrowseCtrl::OnBrowse()
{
	ASSERT_VALID(this);
	ENSURE(GetSafeHwnd() != NULL);

	switch (m_Mode)
	{
	case BrowseMode_Folder:
		if (afxShellManager != NULL)
		{
			CString strFolder;
			GetWindowText(strFolder);

			CString strResult;
			if (afxShellManager->BrowseForFolder(strResult, this, strFolder) &&
				(strResult != strFolder))
			{
				SetWindowText(strResult);
				SetModify(TRUE);
				OnAfterUpdate();
			}
		}
		else
		{
			ASSERT(FALSE);
		}
		break;

	case BrowseMode_File:
		{
			CString strFile;
			GetWindowText(strFile);

			if (!strFile.IsEmpty())
			{
				TCHAR fname [_MAX_FNAME];

				_tsplitpath_s(strFile, NULL, 0, NULL, 0, fname, _MAX_FNAME, NULL, 0);

				CString strFileName = fname;
				strFileName.TrimLeft();
				strFileName.TrimRight();

				if (strFileName.IsEmpty())
				{
					strFile.Empty();
				}
			}

			CFileDialog dlg(TRUE, !m_strDefFileExt.IsEmpty() ? (LPCTSTR)m_strDefFileExt : (LPCTSTR)NULL, strFile, 0, !m_strFileFilter.IsEmpty() ? (LPCTSTR)m_strFileFilter : (LPCTSTR)NULL, NULL);
			if (dlg.DoModal() == IDOK && strFile != dlg.GetPathName())
			{
				SetWindowText(dlg.GetPathName());
				SetModify(TRUE);
				OnAfterUpdate();
			}

			if (GetParent() != NULL)
			{
				GetParent()->RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
			}
		}
		break;
	}

	SetFocus();
}

void CMFCEditBrowseCtrl::SetBrowseButtonImage(HICON hIcon, BOOL bAutoDestroy)
{
	if (m_ImageBrowse.GetSafeHandle() != NULL)
	{
		m_ImageBrowse.DeleteImageList();
	}

	if (hIcon == NULL)
	{
		m_sizeImage = CSize(0, 0);
		return;
	}

	ICONINFO info;
	::GetIconInfo(hIcon, &info);

	BITMAP bmp;
	::GetObject(info.hbmColor, sizeof(BITMAP), (LPVOID) &bmp);

	m_sizeImage.cx = bmp.bmWidth;
	m_sizeImage.cy = bmp.bmHeight;

	::DeleteObject(info.hbmColor);
	::DeleteObject(info.hbmMask);

	UINT nFlags = ILC_MASK;

	switch (bmp.bmBitsPixel)
	{
	case 4:
	default:
		nFlags |= ILC_COLOR4;
		break;

	case 8:
		nFlags |= ILC_COLOR8;
		break;

	case 16:
		nFlags |= ILC_COLOR16;
		break;

	case 24:
		nFlags |= ILC_COLOR24;
		break;

	case 32:
		nFlags |= ILC_COLOR32;
		break;
	}

	m_ImageBrowse.Create(bmp.bmWidth, bmp.bmHeight, nFlags, 0, 0);
	m_ImageBrowse.Add(hIcon);

	m_bDefaultImage = FALSE;

	if (bAutoDestroy)
	{
		::DestroyIcon(hIcon);
	}
}

void CMFCEditBrowseCtrl::SetBrowseButtonImage(HBITMAP hBitmap, BOOL bAutoDestroy)
{
	if (m_ImageBrowse.GetSafeHandle() != NULL)
	{
		m_ImageBrowse.DeleteImageList();
	}

	if (hBitmap == NULL)
	{
		m_sizeImage = CSize(0, 0);
		return;
	}

	BITMAP bmp;
	::GetObject(hBitmap, sizeof(BITMAP), (LPVOID) &bmp);

	m_sizeImage.cx = bmp.bmWidth;
	m_sizeImage.cy = bmp.bmHeight;

	UINT nFlags = ILC_MASK;

	switch (bmp.bmBitsPixel)
	{
	case 4:
	default:
		nFlags |= ILC_COLOR4;
		break;

	case 8:
		nFlags |= ILC_COLOR8;
		break;

	case 16:
		nFlags |= ILC_COLOR16;
		break;

	case 24:
		nFlags |= ILC_COLOR24;
		break;

	case 32:
		nFlags |= ILC_COLOR32;
		break;
	}

	m_ImageBrowse.Create(bmp.bmWidth, bmp.bmHeight, nFlags, 0, 0);

	HBITMAP hbmpCopy = (HBITMAP) ::CopyImage(hBitmap, IMAGE_BITMAP, 0, 0, 0);
	m_ImageBrowse.Add(CBitmap::FromHandle(hbmpCopy), RGB(192, 192, 192));

	::DeleteObject(hbmpCopy);

	m_bDefaultImage = FALSE;

	if (bAutoDestroy)
	{
		::DeleteObject(hBitmap);
	}
}

void CMFCEditBrowseCtrl::SetBrowseButtonImage(UINT uiBmpResId)
{
	if (m_ImageBrowse.GetSafeHandle() != NULL)
	{
		m_ImageBrowse.DeleteImageList();
	}

	if (uiBmpResId == 0)
	{
		m_sizeImage = CSize(0, 0);
		return;
	}

	CMFCToolBarImages images;
	if (!images.Load(uiBmpResId))
	{
		ASSERT(FALSE);
		return;
	}

	SetBrowseButtonImage((HBITMAP) ::CopyImage(images.GetImageWell(), IMAGE_BITMAP, 0, 0, 0), TRUE /* bAutoDestroy */);
	m_bDefaultImage = FALSE;
}

void CMFCEditBrowseCtrl::EnableFileBrowseButton(LPCTSTR lpszDefExt/* = NULL*/, LPCTSTR lpszFilter/* = NULL*/)
{
	ASSERT_VALID(this);
	ENSURE(GetSafeHwnd() != NULL);

	m_Mode = BrowseMode_File;

	m_strDefFileExt = lpszDefExt == NULL ? _T("") : lpszDefExt;
	m_strFileFilter = lpszFilter == NULL ? _T("") : lpszFilter;

	SetInternalImage();
	OnChangeLayout();
}

void CMFCEditBrowseCtrl::EnableFolderBrowseButton()
{
	ASSERT_VALID(this);
	ENSURE(GetSafeHwnd() != NULL);
	ENSURE(afxShellManager != NULL); // You need to call CWinAppEx::InitShellManager() first!

	m_Mode = BrowseMode_Folder;
	SetInternalImage();
	OnChangeLayout();
}

void CMFCEditBrowseCtrl::SetInternalImage()
{
	if (m_ImageBrowse.GetSafeHandle() != NULL)
	{
		m_ImageBrowse.DeleteImageList();
	}

	UINT uiImageListResID = afxGlobalData.Is32BitIcons() ? IDB_AFXBARRES_BROWSE32 : IDB_AFXBARRES_BROWSE;

	LPCTSTR lpszResourceName = MAKEINTRESOURCE(uiImageListResID);
	ENSURE(lpszResourceName != NULL);

	HBITMAP hbmp = (HBITMAP) ::LoadImage(
		AfxFindResourceHandle(lpszResourceName, RT_BITMAP), lpszResourceName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

	if (hbmp == NULL)
	{
		TRACE(_T("Can't load bitmap: %x\n"), uiImageListResID);
		return;
	}

	BITMAP bmpObj;
	::GetObject(hbmp, sizeof(BITMAP), &bmpObj);

	UINT nFlags = ILC_MASK;

	switch (bmpObj.bmBitsPixel)
	{
	case 4:
	default:
		nFlags |= ILC_COLOR4;
		break;

	case 8:
		nFlags |= ILC_COLOR8;
		break;

	case 16:
		nFlags |= ILC_COLOR16;
		break;

	case 24:
		nFlags |= ILC_COLOR24;
		break;

	case 32:
		nFlags |= ILC_COLOR32;
		break;
	}

	m_ImageBrowse.Create(16, 16, nFlags, 0, 0);
	m_ImageBrowse.Add(CBitmap::FromHandle(hbmp), RGB(255, 0, 255));

	m_sizeImage = CSize(16, 16);
	m_bDefaultImage = TRUE;
}

void CMFCEditBrowseCtrl::OnAfterUpdate()
{
	if (GetOwner() == NULL)
	{
		return;
	}

	GetOwner()->PostMessage(EN_CHANGE, GetDlgCtrlID(), (LPARAM) GetSafeHwnd());
	GetOwner()->PostMessage(EN_UPDATE, GetDlgCtrlID(), (LPARAM) GetSafeHwnd());
}

void CMFCEditBrowseCtrl::OnNcMouseMove(UINT nHitTest, CPoint point)
{
	if (!m_bIsButtonCaptured)
	{
		CPoint ptClient = point;
		ScreenToClient(&ptClient);

		if (m_rectBtn.PtInRect(ptClient))
		{
			SetCapture();
			m_bIsButtonHighlighted = TRUE;

			RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
		}
	}

	CEdit::OnNcMouseMove(nHitTest, point);
}

void CMFCEditBrowseCtrl::OnCancelMode()
{
	CEdit::OnCancelMode();

	if (IsWindowEnabled())
	{
		ReleaseCapture();
	}

	m_bIsButtonPressed = FALSE;
	m_bIsButtonCaptured = FALSE;
	m_bIsButtonHighlighted = FALSE;

	RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
}

void CMFCEditBrowseCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_Mode != BrowseMode_None && m_rectBtn.PtInRect(point))
	{
		SetFocus();

		m_bIsButtonPressed = TRUE;
		m_bIsButtonCaptured = TRUE;

		SetCapture();

		RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
		return;
	}

	CEdit::OnLButtonDown(nFlags, point);
}

void CMFCEditBrowseCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (m_Mode != BrowseMode_None && m_rectBtn.PtInRect(point))
	{
		return;
	}

	CEdit::OnRButtonDown(nFlags, point);
}

void CMFCEditBrowseCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (m_Mode != BrowseMode_None && m_rectBtn.PtInRect(point))
	{
		return;
	}

	CEdit::OnRButtonUp(nFlags, point);
}

BOOL CMFCEditBrowseCtrl::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_SYSKEYDOWN:
		if (m_Mode != BrowseMode_None && (pMsg->wParam == VK_DOWN || pMsg->wParam == VK_RIGHT))
		{
			OnBrowse();
			return TRUE;
		}
		break;
	}

	return CEdit::PreTranslateMessage(pMsg);
}


