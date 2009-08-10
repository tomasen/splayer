// SkinMenu.cpp: implementation of the CSkinMenu class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "SkinMenu.h"
#include "SkinMenuMgr.h"
#include "wclassdefines.h"
#include "winclasses.h"
#include "skinglobals.h"
#include "skinbase.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

ISkinMenuRender* CSkinMenu::s_pRenderer = NULL;

enum { REDRAWALL = -2 };

#ifndef SPI_GETMENUANIMATION
#define SPI_GETMENUANIMATION  0x1002
#endif

#ifndef SPI_GETMENUFADE
#define SPI_GETMENUFADE  0x1012
#endif

//////////////////////////////////////////////////////////////////////

struct colorMapping
{
	int nSrcColor;
	int nDestColor;
};

static colorMapping colors[] = 
{
//	{ COLOR_HIGHLIGHT, COLOR_HIGHLIGHT },
	{ COLOR_WINDOWTEXT, COLOR_WINDOWTEXT },
	{ COLOR_GRAYTEXT, COLOR_MENU },
	{ COLOR_HIGHLIGHTTEXT, COLOR_HIGHLIGHTTEXT },
	{ COLOR_3DHILIGHT, COLOR_MENU },
//	{ COLOR_3DDKSHADOW, COLOR_MENU },
	{ COLOR_3DSHADOW, COLOR_3DSHADOW },
	{ COLOR_3DFACE, COLOR_MENU },
	{ COLOR_MENU, COLOR_MENU },

};

CSkinMenu::CSkinMenu(CSkinGlobals* pGlobals, DWORD dwStyle, int nSBWidth) 
	: m_pGlobals(pGlobals), m_nSidebarWidth(nSBWidth), m_dwStyle(dwStyle)
{
	m_nSelIndex = REDRAWALL; // this ensures a full repaint when we first show
	m_hContextWnd = NULL;
	m_hMenu = NULL;

	// fix for animated menus
	m_bFirstRedraw = FALSE;

	m_bAnimatedMenus = FALSE;
	//SystemParametersInfo(SPI_GETMENUANIMATION, 0, &m_bAnimatedMenus, 0);
}

CSkinMenu::~CSkinMenu()
{
}

BOOL CSkinMenu::IsMenuWnd(HWND hWnd)
{
	return CWinClasses::IsClass(hWnd, WC_MENU);
}

BOOL CSkinMenu::AttachWindow(HWND hWnd) 
{ 
	if (!IsMenuWnd(hWnd))
		return FALSE;

	if (HookWindow(hWnd))
	{
		return TRUE;
	}
	
	// else
	return FALSE;
}

BOOL CSkinMenu::DetachWindow() 
{ 
	return HookWindow((HWND)NULL); 
}

LRESULT CSkinMenu::WindowProc(HWND hRealWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	UINT uRes = 0;
	LRESULT lr = 0;

	int nOS = CSkinBase::GetOS();

	switch (msg) 
	{
	case WM_NCPAINT: 
		// the very first WM_NCPAINT appears to be responsible for
		// doing any menu animation. since our std OnNcPaint does not
		// deal with animation we must leave it to the default handler.
		// fortunately, the default handler calls WM_PRINT to implement
		// the animation.
		
		if (!m_bAnimatedMenus )
		{
			//AfxMessageBox(_T("1"));
			if(!m_bFirstRedraw ){
			//	AfxMessageBox(_T("2"));
				CWindowDC dc(GetCWnd());
				OnNcPaint(&dc);
				return 0;
			}
		}
		break; 
		
	case WM_PRINT: 
		if (nOS != SBOS_95 && nOS != SBOS_NT4)
		{
			lr = CSubclassWnd::WindowProc(hRealWnd, msg, wp, lp);
			OnNcPaint(CDC::FromHandle((HDC)wp));
			return lr;
		}
		break; 
		
	case WM_PRINTCLIENT:
		if (nOS != SBOS_95 && nOS != SBOS_NT4)
		{
			OnPrintClient(CDC::FromHandle((HDC)wp), lp);
			return 0;
		}
		break; 
		
	case WM_PAINT:
		if (nOS != SBOS_95 && nOS != SBOS_NT4)
		{
			CPaintDC dc(GetCWnd());
			SendMessage(WM_PRINTCLIENT, (WPARAM)(HDC)dc, PRF_CLIENT | PRF_CHECKVISIBLE);
			return 0;
		}
		break; 
		
		// handle keyboard navigation
	case WM_KEYDOWN:
		if (nOS != SBOS_95 && nOS != SBOS_NT4)
		{
			switch (wp)
			{
			case VK_UP:
			case VK_DOWN:
			case VK_RIGHT:
				// left is much trickier because if the currently selected item
				// has a popup menu then left will close that submenu, and if
				// we prevent the default redrawing then the submenu is not correctly
				// removed from the screen.
				// so we must always do the default drawing and follow it up with our own.
			case VK_LEFT:
				if (!m_hMenu)
				{
					if (wp != VK_LEFT) 
						SetRedraw(FALSE);

					lr = Default();

					if (wp != VK_LEFT) 
						SetRedraw(TRUE);

//					TRACE ("Invalidating entire menu in response to a cursor keypress\n");

					m_nSelIndex = -1; // reset current selection because its too risky to 
										// try to figure it out for ourselves
					Invalidate(FALSE);
					UpdateWindow(GetHwnd());

					m_bFirstRedraw = FALSE;
				}
				else // have menu handle
				{
					int nPrevSel = GetCurSel();

					if (wp != VK_LEFT) 
						SetRedraw(FALSE);

					lr = Default();

					if (wp != VK_LEFT) 
						SetRedraw(TRUE);

					// if we have the handle of the menu then 
					// we can do a selective redraw else we must redraw all
					m_nSelIndex = GetCurSel();

					if (m_nSelIndex != nPrevSel)
					{
						CRect rInvalid;
						GetInvalidRect(m_nSelIndex, nPrevSel, rInvalid);

//						TRACE ("Invalidating menu items %d & %d in response to a cursor keypress\n", m_nSelIndex, nPrevSel);

						InvalidateRect(GetHwnd(), rInvalid, FALSE);
						UpdateWindow(GetHwnd());

						m_bFirstRedraw = FALSE;
					}
				}
				return lr;
			}
		}
		break;

	case 0x1e5: 
		if (nOS != SBOS_95 && nOS != SBOS_NT4)
		{
			if (m_nSelIndex != (int)wp)
			{
				// attempt to do a partial redraw where possible
				// this needs more thought
				CRect rInvalid;
				
				if (m_hMenu)
					GetInvalidRect((int)wp, m_nSelIndex, rInvalid);
				else
					GetClientRect(rInvalid);
				
				// prevent redrawing during default message processing
				// because otherwise the item is redrawn oven ours.
				SetRedraw(FALSE);
				lr = Default();
				SetRedraw(TRUE);
				
				m_nSelIndex = (int)wp;
				
				InvalidateRect(hRealWnd, rInvalid, FALSE);

				if (!m_bFirstRedraw)
					UpdateWindow(hRealWnd);
			}

			// special fix for animated menus
			if (m_bAnimatedMenus && m_bFirstRedraw)
			{
				CWindowDC dc(GetCWnd());
				OnNcPaint(&dc);
			}

			m_bFirstRedraw = FALSE;
			return lr;
		}
		break;

	case WM_NCCALCSIZE:
		if (Sidebar())
		{
			lr = Default();
			
			LPRECT pRect = wp ? &((LPNCCALCSIZE_PARAMS)lp)->rgrc[0] : (LPRECT)lp;
			pRect->left += m_nSidebarWidth;
			
			return lr;
		}
		break;
		
	case WM_WINDOWPOSCHANGING:
		{
			WINDOWPOS* pWP = (WINDOWPOS*)lp;
			
			// adjust width for sidebar
			if (Sidebar() && !(pWP->flags & SWP_NOSIZE))
				pWP->cx += m_nSidebarWidth;

			// if we have a parent menu we may need to adjust our
			// pos to avoid client repainting issues
			if (m_pParentMenu && !(pWP->flags & SWP_NOMOVE))
			{
				// if we are on the right side of our parent
				// then we need to adjust ourselves to avoid the client rect
				CRect rParentWindow;
				::GetWindowRect(m_pParentMenu->GetHwnd(), rParentWindow);

				if (pWP->x > rParentWindow.left) // right
				{
					CRect rParentClient;
					::GetClientRect(m_pParentMenu->GetHwnd(), rParentClient);

					m_pParentMenu->ClientToScreen(rParentClient);

					pWP->x = rParentClient.right;
				}
			}
		}
		break;
		
	case WM_ERASEBKGND: 
		if (nOS != SBOS_95 && nOS != SBOS_NT4)
			return TRUE; 
		break;
		
	default:
		break;
	}

	// We don't handle it: pass along
	return CSubclassWnd::WindowProc(hRealWnd, msg, wp, lp);
}

CDC* CSkinMenu::ReplaceSystemColors(CDC* pDCSrc, CDC* pDCDest, LPRECT pRect, LPRECT pClip)
{
	int nOS = CSkinBase::GetOS();

	if (nOS == SBOS_95 || nOS == SBOS_NT4)
		return pDCSrc;

	// replace the system colors with skin colors
	CMap<COLORREF, COLORREF, int, int&> mapColors;

	// 1. replace the actual background color with COLOR_MENU
	const COLORREF COLORMENU = GetColor(COLOR_MENU);

	COLORREF crSrc, crDest = COLORMENU;

	if (m_nSelIndex != 0)
		crSrc = pDCSrc->GetPixel(pRect->right, pRect->top);
	else
		crSrc = pDCSrc->GetPixel(pRect->right, pRect->bottom);

	// see if user wants to render bkgnd
	if (crSrc != -1)
	{
		if (s_pRenderer && s_pRenderer->DrawMenuClientBkgnd(pDCDest, pRect, pClip))
		{
			// transparent blt
			CSkinBase::BitBlt(pDCDest, pRect->left, pRect->top, 
						pRect->right - pRect->left, 
						pRect->bottom - pRect->top, pDCSrc, 0, 0, SRCCOPY, crSrc);

			// swap dest and src
			SwapDCs(pDCSrc, pDCDest);
		}
		// else simple color replacement
		else if (ReplaceColor(pDCSrc, crSrc, pDCDest, crDest, pRect, pClip))
		{		
			// swap dest and src
			SwapDCs(pDCSrc, pDCDest);
		}

		mapColors[crSrc] = 1;
	}

	// 2. replace other mapped colors 
	int nColor = sizeof(colors) / sizeof(colorMapping);
	
	while (nColor--)
	{
		int nTemp;
		int nSrcColor = colors[nColor].nSrcColor;
		crSrc = GetSysColor(nSrcColor);
		
		if (mapColors.Lookup(crSrc, nTemp))
		{
//			TRACE("CSkinMenu::ReplaceSystemColors - %d already replaced\n", crSrc);
			continue;
		}

		int nDestColor = colors[nColor].nDestColor;
		crDest = GetColor(nDestColor);

		// if the dest color is COLORMENU let the user have first go
		if (crDest == COLORMENU && s_pRenderer && s_pRenderer->DrawMenuClientBkgnd(pDCDest, pRect, pClip))
		{
			// transparent blt
			CSkinBase::BitBlt(pDCDest, pRect->left, pRect->top, 
						pRect->right - pRect->left, 
						pRect->bottom - pRect->top, pDCSrc, 0, 0, SRCCOPY, crSrc);

			// swap dest and src
			SwapDCs(pDCSrc, pDCDest);
		}
		// else simple color replacement
		else if (ReplaceColor(pDCSrc, crSrc, pDCDest, crDest, pRect, pClip))
		{		
			// swap dest and src
			SwapDCs(pDCSrc, pDCDest);
		}
//		else
//			TRACE("CSkinMenu::ReplaceSystemColors - GetSysColor(%d) == CSkinBase::GetColor(%d)\n", nSrcColor, nDestColor);
		
		mapColors[crSrc] = 1;
	}

	return pDCSrc;
}

BOOL CSkinMenu::ReplaceColor(CDC* pDCSrc, COLORREF crSrc, CDC* pDCDest, COLORREF crDest, LPRECT pRect, LPRECT pClip)
{
	if (crSrc == crDest)
		return FALSE;
	
	// else
	if (pClip)
	{
		pDCDest->FillSolidRect(pClip, crDest);
		CSkinBase::BitBlt(pDCDest, pClip->left, pClip->top, pClip->right - pClip->left, 
						pClip->bottom - pClip->top, pDCSrc, pClip->left, pClip->top, SRCCOPY, crSrc);
	}
	else
	{
		pDCDest->FillSolidRect(pRect, crDest);
		CSkinBase::BitBlt(pDCDest, pRect->left, pRect->top, pRect->right - pRect->left, 
						pRect->bottom - pRect->top, pDCSrc, pRect->left, pRect->top, SRCCOPY, crSrc);
	}

	return TRUE;
}

void CSkinMenu::OnPrintClient(CDC* pDC, DWORD dwFlags)
{
	CRect rClient;
	GetClientRect(rClient);

	CRect rClip(rClient);
//	pDC->GetClipBox(rClip);

	// create standard back buffer and another dc on which 
	// to layer the background and foreground
	CDC dcMem, dcMem2;
	dcMem.CreateCompatibleDC(NULL);
	dcMem2.CreateCompatibleDC(NULL);
	
	// use screen dc for creating bitmaps because
	// menu dc's seem not to be standard.
	CDC* pDCScrn = CWnd::GetDesktopWindow()->GetDC();
	
	CBitmap bmMem, bmMem2;
	bmMem.CreateCompatibleBitmap(pDCScrn, rClient.right, rClient.bottom);
	bmMem2.CreateCompatibleBitmap(pDCScrn, rClient.right, rClient.bottom);

	// release screen dc asap
	CWnd::GetDesktopWindow()->ReleaseDC(pDCScrn);
	
	// prepare dc's
	dcMem.SetBkMode(TRANSPARENT);
	dcMem.SetBkColor(GetSysColor(COLOR_MENU));

	CBitmap* pOldBM = dcMem.SelectObject(&bmMem);
	CFont* pOldFont = dcMem.SelectObject(GetFont(SBFONT_MENU));
	CBitmap* pOldBM2 = dcMem2.SelectObject(&bmMem2);

	// trim clip rgn
	if (rClip.top)
		dcMem.ExcludeClipRect(0, 0, rClient.right, rClip.top);

	if (rClip.bottom < rClient.bottom)
		dcMem.ExcludeClipRect(0, rClip.bottom, rClient.right, rClient.bottom);

	// draw background
//	dcMem.FillSolidRect(rClient, GetSysColor(COLOR_MENU));
	dcMem.FillSolidRect(rClip, GetSysColor(COLOR_MENU));

	// default draw
	CSubclassWnd::WindowProc(GetHwnd(), WM_PRINTCLIENT, (WPARAM)(HDC)dcMem, (LPARAM)dwFlags);
	
	// replace the system colors with skin colors
	CDC* pDCSrc = ReplaceSystemColors(&dcMem, &dcMem2, rClient, rClip);
	
	// blt the lot to pDC
//	pDC->BitBlt(0, 0, rClient.right, rClient.bottom, pDCSrc, 0, 0, SRCCOPY);
	pDC->BitBlt(rClip.left, rClip.top, rClip.Width(), rClip.Height(), 
				pDCSrc, rClip.left, rClip.top, SRCCOPY);
	
	// cleanup
	dcMem.SelectObject(pOldBM);
	dcMem.SelectObject(pOldFont);
	dcMem.DeleteDC();
	bmMem.DeleteObject();
	
	dcMem2.SelectObject(pOldBM2);
	dcMem2.DeleteDC();
	bmMem2.DeleteObject();
}

void CSkinMenu::OnPaint(CDC* pDC)
{
	// construct a back buffer for the default draw
	CRect rClient;
	GetClientRect(rClient);
	
	CBitmap bmMem;
	bmMem.CreateCompatibleBitmap(pDC, rClient.right, rClient.bottom);
	
	CDC dcMem;
	dcMem.CreateCompatibleDC(NULL);
	
	dcMem.SetBkMode(TRANSPARENT);
	dcMem.SetBkColor(GetSysColor(COLOR_MENU));
	CBitmap* pOldBM = dcMem.SelectObject(&bmMem);
	CFont* pOldFont = dcMem.SelectObject(GetFont(SBFONT_MENU));
	
	// draw background
	dcMem.FillSolidRect(rClient, GetSysColor(COLOR_MENU));
	
	// default draw
	CSubclassWnd::WindowProc(GetHwnd(), WM_PAINT, (WPARAM)(HDC)dcMem, 0);
	
	// create another dc on which to layer the background and foreground
	CDC dcMem2;
	dcMem2.CreateCompatibleDC(NULL);
	
	CBitmap bmMem2;
	bmMem2.CreateCompatibleBitmap(pDC, rClient.right, rClient.bottom);
	CBitmap* pOldBM2 = dcMem2.SelectObject(&bmMem2);
	
	// replace the system colors with skin colors
	CDC* pDCSrc = ReplaceSystemColors(&dcMem, &dcMem2, rClient, NULL);
	
	// blt the lot to pDC
	pDC->BitBlt(0, 0, rClient.right, rClient.bottom, pDCSrc, 0, 0, SRCCOPY);
	
	// cleanup
	dcMem.SelectObject(pOldBM);
	dcMem.SelectObject(pOldFont);
	dcMem.DeleteDC();
	bmMem.DeleteObject();
	
	dcMem2.SelectObject(pOldBM2);
	dcMem2.DeleteDC();
	bmMem2.DeleteObject();
}

void CSkinMenu::OnNcPaint(CDC* pDC)
{
	int nOS = CSkinBase::GetOS();

	BOOL bIRender = !(nOS == SBOS_95 || nOS == SBOS_NT4);

	CRect rWindow, rClient;
	GetDrawRect(rWindow, rClient);

	CRect rClip;
	pDC->GetClipBox(rClip);

//	TRACE ("CSkinMenu::OnNcPaint(clip box = {%d x %d})\n", rClip.Width(), rClip.Height());

	// back buffer
	CBitmap bmMem;
	bmMem.CreateCompatibleBitmap(pDC, rWindow.right, rWindow.bottom);
	
	CDC dcMem;
	dcMem.CreateCompatibleDC(NULL);
	
	CBitmap* pOldBM = dcMem.SelectObject(&bmMem);
	
	COLORREF crColorMenu = GetColor(COLOR_MENU);

	// draw sidebar
	CRect rSidebar(rWindow);
	rSidebar.DeflateRect(3, 3);
	rSidebar.right = rSidebar.left + m_nSidebarWidth;
		
	if (Sidebar() && (!s_pRenderer || !s_pRenderer->DrawMenuSidebar(&dcMem, rSidebar)))
	{
		dcMem.FillSolidRect(rSidebar, CSkinBase::VaryColor(GetColor(COLOR_3DSHADOW), 0.9f));
	}

	// then clip sidebar out
	dcMem.ExcludeClipRect(rSidebar);

	// draw nc bkgnd
	// note: do this ourselves under win95 to ensure continuity with client bkgnd
	if (!bIRender || !s_pRenderer || !s_pRenderer->DrawMenuNonClientBkgnd(&dcMem, rWindow))
	{
		dcMem.FillSolidRect(rWindow, crColorMenu);
	}

	// draw nc border
	if (!bIRender || !s_pRenderer || !s_pRenderer->DrawMenuBorder(&dcMem, rWindow))
	{
		COLORREF crShadow = GetColor(COLOR_3DSHADOW);

		if (Flat()){
			dcMem.Draw3dRect(rWindow, crShadow, crShadow);
		}else
			dcMem.Draw3dRect(rWindow, GetColor(COLOR_3DHIGHLIGHT), crShadow);


		CRgn m_rgn;
		m_rgn.CreateRoundRectRgn(rWindow.left, rWindow.top, rWindow.right+1,rWindow.bottom+1, 3,3);
		CBrush myBrush;
		myBrush.CreateSolidBrush(crShadow);
		dcMem.FrameRgn(&m_rgn, &myBrush,1,1 );

	}

	// blt to screen
	int nSaveDC = pDC->SaveDC(); // must restore dc to original state

	pDC->ExcludeClipRect(rClient);
	pDC->BitBlt(0, 0, rWindow.right, rWindow.bottom, &dcMem, 0, 0, SRCCOPY);

	pDC->RestoreDC(nSaveDC);

	// cleanup
	dcMem.SelectObject(pOldBM);
}

void CSkinMenu::GetDrawRect(LPRECT pWindow, LPRECT pClient)
{
	CRect rWindow;
	GetWindowRect(rWindow);
	
	if (pClient)
	{
		GetClientRect(pClient);
		ClientToScreen(pClient);
		::OffsetRect(pClient, -rWindow.left, -rWindow.top);
	}
	
	if (pWindow)
	{
		rWindow.OffsetRect(-rWindow.TopLeft());
		*pWindow = rWindow;
	}
}

COLORREF CSkinMenu::GetColor(int nColor)
{
	return m_pGlobals ? m_pGlobals->GetColor(nColor) : CSkinGlobals::GetColorSt(nColor);
}

CFont* CSkinMenu::GetFont(int nFont)
{
	return m_pGlobals ? m_pGlobals->GetFont(nFont) : CSkinGlobals::GetFontSt(nFont);
}

void CSkinMenu::SetContextWnd(HWND hWnd) 
{ 
	if (hWnd && ::IsWindow(hWnd))
		m_hContextWnd = hWnd;
	else
		m_hContextWnd = NULL; 
}

void CSkinMenu::SetMenu(HMENU hMenu, CSkinMenu* pParentMenu) 
{ 
	if (m_hMenu && hMenu) // already set
		return;
	
	// else
	if (hMenu && ::IsMenu(hMenu)) 
		m_hMenu = hMenu;
	else
		m_hMenu = NULL; 

	TRACE("CSkinMenu::SetMenu(hwnd = %08x, hmenu = %08x)\n", m_hWndHooked, hMenu);

	m_pParentMenu = pParentMenu;
}

void CSkinMenu::GetInvalidRect(int nCurSel, int nPrevSel, LPRECT lpRect)
{
	ASSERT (lpRect);

	if (!m_hMenu || nCurSel == REDRAWALL || nPrevSel == REDRAWALL)
		GetClientRect(lpRect);

	else if (m_hMenu)
	{
		::SetRectEmpty(lpRect);

		if (nCurSel >= 0 || nPrevSel >= 0)
		{
			if (nCurSel >= 0)
			{
				GetMenuItemRect(NULL, m_hMenu, nCurSel, lpRect);
			}
					
			if (nPrevSel >= 0)
			{
				CRect rTemp;
			
				GetMenuItemRect(NULL, m_hMenu, nPrevSel, rTemp);
				::UnionRect(lpRect, lpRect, rTemp);
			}

			// convert to client coords
			ScreenToClient(lpRect);
		}
	}
}

int CSkinMenu::GetCurSel()
{
	ASSERT (m_hMenu);
	int nItem = GetMenuItemCount(m_hMenu);
	
	while (nItem--)
	{
		if (GetMenuState(m_hMenu, nItem, MF_BYPOSITION) & MF_HILITE)
			return nItem;
	}

	return -1; // nothing selected
}

