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
#include "multimon.h"
#include "afxvisualmanager.h"
#include "afxdesktopalertwnd.h"

#include "afxdrawmanager.h"
#include "afxcontextmenumanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CMFCDesktopAlertWnd, CWnd)

// Timer IDs:
static const int nClosePopupTimerId = 1;
static const int nAnimTimerId = 2;
static const int nCheckActivityTimerId = 3;

static clock_t nLastAnimTime = 0;
static const int nSmallCaptionHeight = 7;

UINT AFX_WM_ON_CLOSEPOPUPWINDOW = ::RegisterWindowMessage(_T("AFX_WM_ON_CLOSEPOPUPWINDOW"));

/////////////////////////////////////////////////////////////////////////////
// CMFCDesktopAlertWndButton window

void CMFCDesktopAlertWndButton::OnFillBackground(CDC* pDC, const CRect& rectClient)
{
	CMFCVisualManager::GetInstance()->OnErasePopupWindowButton(pDC, rectClient, this);
}

void CMFCDesktopAlertWndButton::OnDrawBorder(CDC* pDC, CRect& rectClient, UINT /*uiState*/)
{
	CMFCVisualManager::GetInstance()->OnDrawPopupWindowButtonBorder(pDC, rectClient, this);
}

void CMFCDesktopAlertWndButton::OnDraw(CDC* pDC, const CRect& rect, UINT uiState)
{
	if (CMFCVisualManager::GetInstance()->IsDefaultWinXPPopupButton(this))
	{
		return;
	}

	CMFCButton::OnDraw(pDC, rect, uiState);
}

/////////////////////////////////////////////////////////////////////////////
// CMFCDesktopAlertWnd

CMFCDesktopAlertWnd::CMFCDesktopAlertWnd()
{
	m_pWndOwner = NULL;
	m_bIsActive = FALSE;

	m_nAutoCloseTime = 3000;
	m_bSmallCaption = TRUE;
	m_bHasCloseButton = TRUE;
	m_hMenu = NULL;
	m_pWndDlg = NULL;
	m_uiDlgResID = 0;

	m_nBtnMarginVert = 2;
	m_nBtnMarginHorz = 2;

	m_nTransparency = 255; // Opaque

	m_AnimationType = CMFCPopupMenu::NO_ANIMATION;
	m_AnimationSpeed = 30;
	m_nAnimationAlpha = 0;
	m_bAnimationIsDone = FALSE;
	m_AnimSize = CSize(0, 0);
	m_FinalSize = CSize(0, 0);
	m_bIsAnimRight = FALSE;
	m_bIsAnimDown = FALSE;
	m_bFadeOutAnimation = FALSE;
	m_ptLastPos = CPoint(-1, -1);
	m_bMoving = FALSE;
	m_ptStartMove = CPoint(-1, -1);
}

CMFCDesktopAlertWnd::~CMFCDesktopAlertWnd()
{
}

//{{AFX_MSG_MAP(CMFCDesktopAlertWnd)
BEGIN_MESSAGE_MAP(CMFCDesktopAlertWnd, CWnd)
	ON_WM_TIMER()
	ON_WM_NCDESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_CANCELMODE()
	ON_WM_LBUTTONDOWN()
	ON_MESSAGE(WM_PRINTCLIENT, &CMFCDesktopAlertWnd::OnPrintClient)
END_MESSAGE_MAP()
//}}AFX_MSG_MAP

/////////////////////////////////////////////////////////////////////////////
// CMFCDesktopAlertWnd message handlers

BOOL CMFCDesktopAlertWnd::Create(CWnd* pWndOwner, UINT uiDlgResID, HMENU hMenu, CPoint ptPos, CRuntimeClass* pRTIDlgBar)
{
	ENSURE(pRTIDlgBar != NULL);
	ASSERT(pRTIDlgBar->IsDerivedFrom(RUNTIME_CLASS(CMFCDesktopAlertDialog)));

	m_hMenu = hMenu;
	m_pWndOwner = pWndOwner;
	m_uiDlgResID = uiDlgResID;

	m_pWndDlg = (CMFCDesktopAlertDialog*) pRTIDlgBar->CreateObject();
	ENSURE(m_pWndDlg != NULL);
	ASSERT_VALID(m_pWndDlg);

	return CommonCreate(ptPos);
}

BOOL CMFCDesktopAlertWnd::Create(CWnd* pWndOwner, CMFCDesktopAlertWndInfo& params, HMENU hMenu, CPoint ptPos)
{
	m_hMenu = hMenu;
	m_pWndOwner = pWndOwner;

	m_pWndDlg = new CMFCDesktopAlertDialog;
	ASSERT_VALID(m_pWndDlg);

	m_pWndDlg->m_bDefault = TRUE;

	return CommonCreate(ptPos, &params);
}

BOOL CMFCDesktopAlertWnd::CommonCreate(CPoint ptPos, CMFCDesktopAlertWndInfo* pParams)
{
	m_ptLastPos = ptPos;

	int nCaptionHeight = GetCaptionHeight();

	CWnd* pWndFocus = GetFocus();
	CWnd* pWndForeground = CWnd::GetForegroundWindow();

	CString strClassName = ::AfxRegisterWndClass(CS_SAVEBITS, ::LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_BTNFACE + 1), NULL);

	CRect rectDummy(0, 0, 0, 0);
	DWORD dwStyleEx = WS_EX_TOOLWINDOW | WS_EX_TOPMOST;

	if (afxGlobalData.IsWindowsLayerSupportAvailable() && afxGlobalData.m_nBitsPerPixel > 8 && m_nTransparency < 255)
	{
		dwStyleEx |= WS_EX_LAYERED;
	}

	if (!CWnd::CreateEx(dwStyleEx, strClassName, _T(""), WS_POPUP, rectDummy, NULL, 0))
	{
		return FALSE;
	}

	CSize sizeDialog;

	m_pWndDlg->m_bDontSetFocus = TRUE;

	if (m_uiDlgResID != 0)
	{
		if (!m_pWndDlg->Create(m_uiDlgResID, this))
		{
			return FALSE;
		}

		sizeDialog = GetDialogSize();
	}
	else
	{
		ENSURE(pParams != NULL);

		if (!m_pWndDlg->CreateFromParams(*pParams, this))
		{
			return FALSE;
		}

		sizeDialog = m_pWndDlg->GetDlgSize();
	}

	m_pWndDlg->m_bDontSetFocus = FALSE;

	CSize sizeBtn = CMenuImages::Size() + CSize(6, 6);
	BOOL bButtonsOnCaption = (sizeBtn.cy + 2 <= nCaptionHeight);

	if (!bButtonsOnCaption &&(m_bHasCloseButton || m_hMenu != NULL))
	{
		sizeDialog.cx += m_nBtnMarginHorz;

		if (m_bHasCloseButton)
		{
			sizeDialog.cx += sizeBtn.cx;
		}

		if (m_hMenu != NULL)
		{
			sizeDialog.cx += sizeBtn.cx;
		}
	}

	CRect rectScreen;

	MONITORINFO mi;
	mi.cbSize = sizeof(MONITORINFO);
	if (GetMonitorInfo(MonitorFromPoint(ptPos, MONITOR_DEFAULTTONEAREST), &mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		::SystemParametersInfo(SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	sizeDialog.cx = min(rectScreen.Width() - 2, sizeDialog.cx);
	sizeDialog.cy = min(rectScreen.Height() - nCaptionHeight - 2, sizeDialog.cy);

	m_FinalSize = sizeDialog;
	m_FinalSize.cy += nCaptionHeight + 2;
	m_FinalSize.cx += 2;

	CWnd* pBtnParent = bButtonsOnCaption ?(CWnd*) this : m_pWndDlg;

	int nBtnVertOffset = bButtonsOnCaption ? (nCaptionHeight - sizeBtn.cy) / 2 + 1 : m_nBtnMarginVert;

	CRect rectBtn = CRect( CPoint(sizeDialog.cx - sizeBtn.cx - m_nBtnMarginHorz, nBtnVertOffset), sizeBtn);
	if (m_bHasCloseButton)
	{
		m_btnClose.Create(_T(""), WS_CHILD | WS_VISIBLE, rectBtn, pBtnParent, (UINT) -1);

		m_btnClose.SetStdImage(CMenuImages::IdClose, CMenuImages::ImageBlack);
		m_btnClose.m_bDrawFocus = FALSE;
		m_btnClose.m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;

		m_btnClose.m_bIsCaptionButton = bButtonsOnCaption;
		m_btnClose.m_bIsCloseButton = TRUE;

		rectBtn.OffsetRect(-sizeBtn.cx - 1, 0);
	}

	if (m_hMenu != NULL)
	{
		m_btnMenu.Create(_T(""), WS_CHILD | WS_VISIBLE, rectBtn, pBtnParent, (UINT) -1);
		m_btnMenu.SetStdImage(CMenuImages::IdArrowDownLarge, CMenuImages::ImageBlack);
		m_btnMenu.m_bDrawFocus = FALSE;
		m_btnMenu.m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;
		m_btnMenu.m_bIsCaptionButton = bButtonsOnCaption;
	}

	if (ptPos == CPoint(-1, -1))
	{
		ptPos.x = rectScreen.right - m_FinalSize.cx;
		ptPos.y = rectScreen.bottom - m_FinalSize.cy;
	}
	else
	{
		if (ptPos.x < rectScreen.left)
		{
			ptPos.x = rectScreen.left;
		}
		else if (ptPos.x + m_FinalSize.cx > rectScreen.right)
		{
			ptPos.x = rectScreen.right - m_FinalSize.cx;
		}

		if (ptPos.y < rectScreen.top)
		{
			ptPos.y = rectScreen.top;
		}
		else if (ptPos.y + m_FinalSize.cy > rectScreen.bottom)
		{
			ptPos.y = rectScreen.bottom - m_FinalSize.cy;
		}
	}

	OnBeforeShow(ptPos);

	SetWindowPos(&wndTop, ptPos.x, ptPos.y, m_FinalSize.cx, m_FinalSize.cy, SWP_NOACTIVATE | SWP_SHOWWINDOW);

	StartAnimation();

	m_pWndDlg->SetWindowPos(NULL, 1, nCaptionHeight + 1, sizeDialog.cx, sizeDialog.cy, SWP_NOZORDER | SWP_NOACTIVATE);

	SetTimer(nCheckActivityTimerId, 100, NULL);

	if (pWndForeground->GetSafeHwnd() != NULL)
	{
		pWndForeground->SetForegroundWindow();
	}

	if (pWndFocus->GetSafeHwnd() != NULL)
	{
		pWndFocus->SetFocus();
	}

	return TRUE;
}

CSize CMFCDesktopAlertWnd::GetDialogSize()
{
	CDialogTemplate dlgt;
	if (!dlgt.Load(MAKEINTRESOURCE(m_uiDlgResID)))
	{
		ASSERT(FALSE);
		return CSize(0, 0);
	}

	CSize sizeDialog;
	dlgt.GetSizeInPixels(&sizeDialog);

	return sizeDialog;
}

void CMFCDesktopAlertWnd::OnTimer(UINT_PTR nIDEvent)
{
	switch(nIDEvent)
	{
	case nAnimTimerId:
		if (!m_bAnimationIsDone)
		{
			clock_t nCurrAnimTime = clock();

			int nDuration = nCurrAnimTime - nLastAnimTime;
			int nSteps = (int)(.5 +(float) nDuration / m_AnimationSpeed);

			if (m_bFadeOutAnimation)
			{
				nSteps = -nSteps;
			}

			switch(m_AnimationType)
			{
			case CMFCPopupMenu::UNFOLD:
				m_AnimSize.cx += nSteps * m_nAnimStepX;
				// no break intentionally

			case CMFCPopupMenu::SLIDE:
				m_AnimSize.cy += nSteps * m_nAnimStepY;
				break;

			case CMFCPopupMenu::FADE:
				m_iFadePercent += m_iFadeStep;

				if (m_iFadePercent > 100 + nSteps * m_iFadeStep)
				{
					m_iFadePercent = 101;
				}
				break;
			}

			m_AnimSize.cx = max(0, min(m_AnimSize.cx, m_FinalSize.cx));
			m_AnimSize.cy = max(0, min(m_AnimSize.cy, m_FinalSize.cy));

			if (m_bFadeOutAnimation && !m_bIsActive && (m_AnimSize.cx == 0 || m_AnimSize.cy == 0 || (m_AnimationType == CMFCPopupMenu::FADE && m_iFadePercent <= 0)))
			{
				SendMessage(WM_CLOSE);
				return;
			}

			if ((m_AnimationType != CMFCPopupMenu::FADE && m_AnimSize.cy >= m_FinalSize.cy && m_AnimSize.cx >= m_FinalSize.cx) ||
				(m_AnimationType == CMFCPopupMenu::UNFOLD && m_AnimSize.cx >= m_FinalSize.cx) || (m_AnimationType == CMFCPopupMenu::FADE && m_iFadePercent > 100) || m_bIsActive)
			{
				m_AnimSize.cx = m_FinalSize.cx;
				m_AnimSize.cy = m_FinalSize.cy;

				KillTimer(nAnimTimerId);

				if (m_btnClose.GetSafeHwnd() != NULL)
				{
					m_btnClose.ShowWindow(SW_SHOWNOACTIVATE);
				}

				if (m_btnMenu.GetSafeHwnd() != NULL)
				{
					m_btnMenu.ShowWindow(SW_SHOWNOACTIVATE);
				}

				m_pWndDlg->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW | SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOACTIVATE);
				m_pWndDlg->ValidateRect(NULL);

				m_bAnimationIsDone = TRUE;

				if (m_nAutoCloseTime > 0)
				{
					SetTimer(nClosePopupTimerId, m_nAutoCloseTime, NULL);
				}
			}

			if (m_bFadeOutAnimation && m_AnimationType != CMFCPopupMenu::FADE)
			{
				CRect rectWnd;
				GetWindowRect(rectWnd);

				int x = m_bIsAnimRight ? rectWnd.left : rectWnd.right - m_AnimSize.cx;
				int y = m_bIsAnimDown ? rectWnd.top : rectWnd.bottom - m_AnimSize.cy;

				SetWindowPos(NULL, x, y, m_AnimSize.cx, m_AnimSize.cy, SWP_NOZORDER | SWP_NOACTIVATE);
			}
			else
			{
				Invalidate();
				UpdateWindow();
			}

			nLastAnimTime = nCurrAnimTime;
		}
		break;

	case nClosePopupTimerId:
		if (!m_bIsActive && !m_bMoving)
		{
			KillTimer(nClosePopupTimerId);
			StartAnimation(FALSE);
		}
		return;

	case nCheckActivityTimerId:
		if (!m_bMoving)
		{
			BOOL bWasActive = m_bIsActive;

			CRect rectWnd;
			GetWindowRect(rectWnd);

			CPoint ptCursor;
			GetCursorPos(&ptCursor);

			m_bIsActive = rectWnd.PtInRect(ptCursor) || m_pWndDlg->HasFocus();

			if (m_bIsActive != bWasActive && afxGlobalData.IsWindowsLayerSupportAvailable() && afxGlobalData.m_nBitsPerPixel > 8 && m_nTransparency < 255)
			{
				BYTE nTransparency = m_bIsActive ?(BYTE) 255 : m_nTransparency;
				afxGlobalData.SetLayeredAttrib(GetSafeHwnd(), 0, nTransparency, LWA_ALPHA);
			}
		}
	}

	CWnd::OnTimer(nIDEvent);
}

void CMFCDesktopAlertWnd::OnNcDestroy()
{
	CWnd::OnNcDestroy();
	delete this;
}

BOOL CMFCDesktopAlertWnd::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CMFCDesktopAlertWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	if (!m_bAnimationIsDone)
	{
		DrawAnimation(&dc);
	}
	else
	{
		OnDraw(&dc);
	}
}

CRect CMFCDesktopAlertWnd::GetCaptionRect()
{
	int nCaptionHeight = GetCaptionHeight();

	CRect rectClient;
	GetClientRect(&rectClient);

	CRect rectCaption = rectClient;

	rectCaption.DeflateRect(1, 1);
	rectCaption.bottom = rectCaption.top + nCaptionHeight;

	return rectCaption;
}

BOOL CMFCDesktopAlertWnd::ProcessCommand(HWND hwnd)
{
	ASSERT_VALID(m_pWndDlg);

	if (hwnd == m_btnClose.GetSafeHwnd())
	{
		SendMessage(WM_CLOSE);
		return TRUE;
	}

	if (hwnd == m_btnMenu.GetSafeHwnd() && m_hMenu != NULL)
	{
		CRect rectMenuBtn;
		m_btnMenu.GetWindowRect(rectMenuBtn);

		const int x = rectMenuBtn.left;
		const int y = rectMenuBtn.bottom;

		UINT nMenuResult = 0;

		m_pWndDlg->m_bMenuIsActive = TRUE;

		if (afxContextMenuManager != NULL)
		{
			const BOOL bMenuShadows = CMFCMenuBar::IsMenuShadows();
			CMFCMenuBar::EnableMenuShadows(FALSE);

			nMenuResult = afxContextMenuManager->TrackPopupMenu(m_hMenu, x, y, this);

			CMFCMenuBar::EnableMenuShadows(bMenuShadows);
		}
		else
		{
			nMenuResult = ::TrackPopupMenu(m_hMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, x, y, 0, GetSafeHwnd(), NULL);
		}

		m_pWndDlg->m_bMenuIsActive = FALSE;

		if (nMenuResult != 0)
		{
			if (m_pWndOwner != NULL)
			{
				m_pWndOwner->PostMessage(WM_COMMAND, nMenuResult);
			}
			else
			{
				m_pWndDlg->PostMessage(WM_COMMAND, nMenuResult);
			}
		}

		OnCancelMode();
		return TRUE;
	}

	return FALSE;
}

BOOL CMFCDesktopAlertWnd::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (ProcessCommand((HWND)lParam))
	{
		return TRUE;
	}

	return CWnd::OnCommand(wParam, lParam);
}

void CMFCDesktopAlertWnd::OnDestroy()
{
	if (m_pWndDlg != NULL)
	{
		m_pWndDlg->DestroyWindow();
		delete m_pWndDlg;
		m_pWndDlg = NULL;
	}

	CWnd::OnDestroy();
}

LRESULT CMFCDesktopAlertWnd::OnPrintClient(WPARAM wp, LPARAM lp)
{
	if (lp & PRF_CLIENT)
	{
		CDC* pDC = CDC::FromHandle((HDC) wp);
		ASSERT_VALID(pDC);

		OnDraw(pDC);
	}

	return 0;
}

void CMFCDesktopAlertWnd::OnDraw(CDC* pDC)
{
	ASSERT_VALID(pDC);

	CRect rectClient;
	GetClientRect(&rectClient);

	CMFCVisualManager::GetInstance()->OnDrawPopupWindowBorder(pDC, rectClient);

	CRect rectCaption = GetCaptionRect();

	COLORREF clrText = CMFCVisualManager::GetInstance()->OnDrawPopupWindowCaption(pDC, rectCaption, this);

	if (m_bSmallCaption)
	{
		return;
	}

	//--------------------
	// Draw icon and name:
	//--------------------
	CRect rectText = rectCaption;
	rectText.left += AFX_IMAGE_MARGIN;

	CWnd* pWndBtn = NULL;

	if (m_btnMenu.GetSafeHwnd() != NULL)
	{
		pWndBtn = &m_btnMenu;
	}
	else if (m_btnClose.GetSafeHwnd() != NULL)
	{
		pWndBtn = &m_btnClose;
	}

	if (pWndBtn != NULL)
	{
		CRect rectBtn;
		pWndBtn->GetWindowRect(&rectBtn);
		ScreenToClient(&rectBtn);

		rectText.right = rectBtn.left - AFX_IMAGE_MARGIN;
	}

	HICON hIcon = GetIcon(FALSE);
	if (hIcon != NULL)
	{
		CSize sizeImage = afxGlobalData.m_sizeSmallIcon;
		CRect rectImage = rectCaption;

		rectImage.top += (rectCaption.Height() - sizeImage.cy) / 2;
		rectImage.bottom = rectImage.top + sizeImage.cy;

		rectImage.left += AFX_IMAGE_MARGIN;
		rectImage.right = rectImage.left + sizeImage.cx;

		pDC->DrawState(rectImage.TopLeft(), rectImage.Size(), hIcon, DSS_NORMAL, (HBRUSH) NULL);

		rectText.left = rectImage.right + AFX_IMAGE_MARGIN;
	}

	CString strText;
	GetWindowText(strText);

	if (!strText.IsEmpty())
	{
		COLORREF clrTextOld = pDC->SetTextColor(clrText);
		pDC->SetBkMode(TRANSPARENT);
		CFont* pOldFont = pDC->SelectObject(&afxGlobalData.fontRegular);

		pDC->DrawText(strText, rectText, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

		pDC->SelectObject(pOldFont);
		pDC->SetTextColor(clrTextOld);
	}
}

void CMFCDesktopAlertWnd::StartAnimation(BOOL bShow/* = TRUE*/)
{
	if (m_AnimationType == CMFCPopupMenu::NO_ANIMATION || afxGlobalData.bIsRemoteSession || (m_AnimationType == CMFCPopupMenu::FADE && afxGlobalData.m_nBitsPerPixel <= 8))
	{
		if (!bShow)
		{
			SendMessage(WM_CLOSE);
			return;
		}

		if (m_btnClose.GetSafeHwnd() != NULL)
		{
			m_btnClose.ShowWindow(SW_SHOWNOACTIVATE);
		}

		if (m_btnMenu.GetSafeHwnd() != NULL)
		{
			m_btnMenu.ShowWindow(SW_SHOWNOACTIVATE);
		}

		m_bAnimationIsDone = TRUE;

		if (m_nAutoCloseTime > 0)
		{
			SetTimer(nClosePopupTimerId, m_nAutoCloseTime, NULL);
		}

		m_pWndDlg->SetWindowPos(NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOREDRAW|SWP_NOZORDER|SWP_SHOWWINDOW | SWP_NOACTIVATE);
		m_pWndDlg->ValidateRect(NULL);

		if (afxGlobalData.IsWindowsLayerSupportAvailable() && afxGlobalData.m_nBitsPerPixel > 8 && m_nTransparency < 255)
		{
			afxGlobalData.SetLayeredAttrib(GetSafeHwnd(), 0, m_nTransparency, LWA_ALPHA);
		}

		return;
	}

	m_bAnimationIsDone = FALSE;
	m_bFadeOutAnimation = !bShow;

	//-------------------------
	// Set animation direction:
	//-------------------------
	CRect rectScreen;
	::SystemParametersInfo(SPI_GETWORKAREA, 0, &rectScreen, 0);

	CRect rectWindow;
	GetWindowRect(&rectWindow);

	m_bIsAnimRight = rectWindow.left < rectScreen.CenterPoint().x;
	m_bIsAnimDown = rectWindow.top < rectScreen.CenterPoint().y;

	m_iFadePercent = m_bFadeOutAnimation ? 100 : 0;
	m_iFadeStep = m_bFadeOutAnimation ? -10 : 10;

	if (m_FinalSize.cx > m_FinalSize.cy)
	{
		m_nAnimStepY = 10;
		m_nAnimStepX = max(1, m_nAnimStepY * m_FinalSize.cx / m_FinalSize.cy);
	}
	else
	{
		m_nAnimStepX = 10;
		m_nAnimStepY = max(1, m_nAnimStepX * m_FinalSize.cy / m_FinalSize.cx);
	}

	//--------------------------
	// Adjust initial menu size:
	//--------------------------
	m_AnimSize = m_FinalSize;

	if (bShow)
	{
		switch(m_AnimationType)
		{
		case CMFCPopupMenu::UNFOLD:
			m_AnimSize.cx = m_nAnimStepX;

		case CMFCPopupMenu::SLIDE:
			m_AnimSize.cy = m_nAnimStepY;
			break;
		}
	}

	if (m_pWndDlg != NULL && m_pWndDlg->IsWindowVisible())
	{
		m_pWndDlg->ShowWindow(SW_HIDE);
	}

	SetTimer(nAnimTimerId, m_AnimationSpeed, NULL);
	nLastAnimTime = clock();
}

void CMFCDesktopAlertWnd::DrawAnimation(CDC* pPaintDC)
{
	CRect rectClient;
	GetClientRect(&rectClient);

	int cx = m_FinalSize.cx;
	int cy = m_FinalSize.cy;

	CDC dcMem;
	if (!dcMem.CreateCompatibleDC(pPaintDC))
	{
		return;
	}

	// create the three bitmaps if not done yet
	if (m_bmpScreenDst.GetSafeHandle() == NULL)
	{
		CBitmap* pBmpOld = NULL;

		if (m_AnimationType == CMFCPopupMenu::FADE || afxGlobalData.m_nBitsPerPixel > 8)
		{
			// Fill in the BITMAPINFOHEADER
			BITMAPINFOHEADER bih;
			bih.biSize = sizeof(BITMAPINFOHEADER);
			bih.biWidth = cx;
			bih.biHeight = cy;
			bih.biPlanes = 1;
			bih.biBitCount = 32;
			bih.biCompression = BI_RGB;
			bih.biSizeImage = cx * cy;
			bih.biXPelsPerMeter = 0;
			bih.biYPelsPerMeter = 0;
			bih.biClrUsed = 0;
			bih.biClrImportant = 0;

			HBITMAP hmbpDib;
			// Create a DIB section and attach it to the source bitmap
			hmbpDib = CreateDIBSection(dcMem.m_hDC, (LPBITMAPINFO)&bih, DIB_RGB_COLORS, (void **)&m_cFadeSrcBits, NULL, NULL);
			if (hmbpDib == NULL || m_cFadeSrcBits == NULL)
			{
				return;
			}

			m_bmpScreenSrc.Attach( hmbpDib );

			// Create a DIB section and attach it to the destination bitmap
			hmbpDib = CreateDIBSection(dcMem.m_hDC, (LPBITMAPINFO)&bih, DIB_RGB_COLORS, (void **)&m_cFadeDstBits, NULL, NULL);
			if (hmbpDib == NULL || m_cFadeDstBits == NULL)
			{
				return;
			}
			m_bmpScreenDst.Attach( hmbpDib );

			// Create a DIB section and attach it to the temporary bitmap
			hmbpDib = CreateDIBSection( dcMem.m_hDC, (LPBITMAPINFO)&bih, DIB_RGB_COLORS, (void **)&m_cFadeTmpBits, NULL, NULL);
			if (hmbpDib == NULL || m_cFadeTmpBits == NULL)
			{
				return;
			}

			m_bmpScreenTmp.Attach( hmbpDib );

			// get source image, representing the window below the popup menu
			pBmpOld = dcMem.SelectObject(&m_bmpScreenSrc);
			dcMem.BitBlt(0, 0, cx, cy, pPaintDC, rectClient.left, rectClient.top, SRCCOPY);

			// copy it to the destination so that shadow will be ok
			memcpy(m_cFadeDstBits, m_cFadeSrcBits, sizeof(COLORREF)* cx*cy);
			dcMem.SelectObject(&m_bmpScreenDst);
		}
		else
		{
			m_bmpScreenDst.CreateCompatibleBitmap(pPaintDC, cx, cy);
			pBmpOld = dcMem.SelectObject(&m_bmpScreenDst);
		}

		SendMessage(WM_PRINT, (WPARAM) dcMem.GetSafeHdc(), PRF_CHILDREN | PRF_CLIENT | PRF_ERASEBKGND);

		if (m_btnClose.GetSafeHwnd() != NULL)
		{
			m_btnClose.ShowWindow(SW_HIDE);
		}

		if (m_btnMenu.GetSafeHwnd() != NULL)
		{
			m_btnMenu.ShowWindow(SW_HIDE);
		}

		CRect rect;
		m_pWndDlg->GetWindowRect(&rect);
		ScreenToClient(&rect);

		dcMem.SetViewportOrg(rect.TopLeft());
		m_pWndDlg->SendMessage(WM_PRINT, (WPARAM) dcMem.GetSafeHdc(), PRF_CHILDREN | PRF_CLIENT | PRF_ERASEBKGND);

		dcMem.SetViewportOrg(CPoint(0,0));

		dcMem.SelectObject(pBmpOld);

		if (afxGlobalData.IsWindowsLayerSupportAvailable() && afxGlobalData.m_nBitsPerPixel > 8 && m_nTransparency < 255)
		{
			afxGlobalData.SetLayeredAttrib(GetSafeHwnd(), 0, m_nTransparency, LWA_ALPHA);
		}
	}

	COLORREF *src = m_cFadeSrcBits;
	COLORREF *dst = m_cFadeDstBits;
	COLORREF *tmp = m_cFadeTmpBits;

	CBitmap* pBmpOld = NULL;

	switch(m_AnimationType)
	{
	case CMFCPopupMenu::UNFOLD:
	case CMFCPopupMenu::SLIDE:
		pBmpOld = dcMem.SelectObject(&m_bmpScreenDst);

		pPaintDC->BitBlt(
			m_bIsAnimRight ? rectClient.left : rectClient.right - m_AnimSize.cx,
			m_bIsAnimDown ? rectClient.top : rectClient.bottom - m_AnimSize.cy,
			m_AnimSize.cx, m_AnimSize.cy, &dcMem, 0, 0, SRCCOPY);
		break;

	case CMFCPopupMenu::FADE:
		pBmpOld = dcMem.SelectObject(&m_bmpScreenTmp);
		for (int pixel = 0; pixel < cx * cy; pixel++)
		{
			*tmp++ = CDrawingManager::PixelAlpha(*src++, *dst++, 100 - m_iFadePercent);
		}

		pPaintDC->BitBlt(rectClient.left, rectClient.top, cx, cy, &dcMem, 0, 0, SRCCOPY);
	}

	dcMem.SelectObject(pBmpOld);
}

int CMFCDesktopAlertWnd::GetCaptionHeight()
{
	if (m_bSmallCaption)
	{
		return nSmallCaptionHeight;
	}
	else
	{
		CSize sizeBtn = CMenuImages::Size() + CSize(6, 6);
		return max(::GetSystemMetrics(SM_CYSMCAPTION), sizeBtn.cy + 2);
	}
}

void CMFCDesktopAlertWnd::OnClose()
{
	if (m_pWndOwner->GetSafeHwnd() != NULL)
	{
		m_pWndOwner->SendMessage(AFX_WM_ON_CLOSEPOPUPWINDOW, 0, (LPARAM) this);
	}

	CWnd::OnClose();
}

void CMFCDesktopAlertWnd::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos)
{
	CWnd::OnWindowPosChanged(lpwndpos);
}

void CMFCDesktopAlertWnd::StartWindowMove()
{
	m_bMoving = TRUE;
	GetCursorPos(&m_ptStartMove);

	SetCapture();
	KillTimer(nClosePopupTimerId);
}

void CMFCDesktopAlertWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bMoving)
	{
		// ---------------------
		// Calc screen rectangle
		// ---------------------
		CRect rectScreen;

		CPoint ptCursor = point;
		ClientToScreen(&ptCursor);

		MONITORINFO mi;
		mi.cbSize = sizeof(MONITORINFO);
		if (GetMonitorInfo(MonitorFromPoint(ptCursor, MONITOR_DEFAULTTONEAREST), &mi))
		{
			rectScreen = mi.rcWork;
		}
		else
		{
			::SystemParametersInfo(SPI_GETWORKAREA, 0, &rectScreen, 0);
		}

		CPoint ptMouse;
		GetCursorPos(&ptMouse);

		CPoint ptOffset = ptMouse - m_ptStartMove;
		m_ptStartMove = ptMouse;

		CRect rect;
		GetWindowRect(&rect);
		rect.OffsetRect(ptOffset);

		if (rect.left < rectScreen.left)
		{
			rect.OffsetRect(rectScreen.left - rect.left, 0);
		}
		else if (rect.right > rectScreen.right)
		{
			rect.OffsetRect(rectScreen.right - rect.right, 0);
		}

		if (rect.top < rectScreen.top)
		{
			rect.OffsetRect(0, rectScreen.top - rect.top);
		}
		else if (rect.bottom > rectScreen.bottom)
		{
			rect.OffsetRect(0, rectScreen.bottom - rect.bottom);
		}

		SetWindowPos(NULL, rect.left, rect.top, -1, -1, SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);
	}

	CWnd::OnMouseMove(nFlags, point);
}

void CMFCDesktopAlertWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bMoving)
	{
		ReleaseCapture();
		m_bMoving = FALSE;

		if (m_nAutoCloseTime > 0)
		{
			SetTimer(nClosePopupTimerId, m_nAutoCloseTime, NULL);
		}

		CRect rectWnd;
		GetWindowRect(rectWnd);

		m_ptLastPos = rectWnd.TopLeft();
	}

	CWnd::OnLButtonUp(nFlags, point);
}

void CMFCDesktopAlertWnd::OnCancelMode()
{
	CWnd::OnCancelMode();

	if (m_bMoving)
	{
		ReleaseCapture();
		m_bMoving = FALSE;

		if (m_nAutoCloseTime > 0)
		{
			SetTimer(nClosePopupTimerId, m_nAutoCloseTime, NULL);
		}

		CRect rectWnd;
		GetWindowRect(rectWnd);

		m_ptLastPos = rectWnd.TopLeft();
	}
}

void CMFCDesktopAlertWnd::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	StartWindowMove();
}


