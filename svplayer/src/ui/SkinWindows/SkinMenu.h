// SkinMenu.h: interface for the CSkinMenu class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINMENU_H__E2EFA8F0_B9CD_41AB_98FD_812C963B7ACC__INCLUDED_)
#define AFX_SKINMENU_H__E2EFA8F0_B9CD_41AB_98FD_812C963B7ACC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef NO_SKIN_MENUS

#include "subclass.h"
#include "skinglobals.h"

// style flags
enum
{
	SKMS_ROUNDCORNERS = 0x0001,
	SKMS_SIDEBAR = 0x0002,
	SKMS_FLAT = 0x0004,
};

class ISkinMenuRender
{
public:
	virtual BOOL DrawMenuBorder(CDC* pDC, LPRECT pRect) { return FALSE; }
	virtual BOOL DrawMenuSidebar(CDC* pDC, LPRECT pRect, LPCTSTR szTitle = NULL) { return FALSE; }
	virtual BOOL DrawMenuClientBkgnd(CDC* pDC, LPRECT pRect, LPRECT pClip) { return FALSE; }
	virtual BOOL DrawMenuNonClientBkgnd(CDC* pDC, LPRECT pRect) { return FALSE; }
};

class CSkinGlobals;

class CSkinMenu : public CSubclassWnd
{
public:
	CSkinMenu(CSkinGlobals* pGlobals = NULL, DWORD dwStyle = SKMS_SIDEBAR, int nSBWidth = 10);
	virtual ~CSkinMenu();

	virtual BOOL AttachWindow(HWND hWnd);
	BOOL DetachWindow();

	void SetContextWnd(HWND hWnd);
	void SetMenu(HMENU hMenu, CSkinMenu* pParentMenu = NULL);

	const HMENU GetMenu() const { return m_hMenu; }

	static BOOL IsMenuWnd(HWND hWnd);
	static void SetRenderer(ISkinMenuRender* pRenderer) { s_pRenderer = pRenderer; }

protected:
	int m_nSelIndex;
	DWORD m_dwStyle;
	CSkinGlobals* m_pGlobals;
	int m_nSidebarWidth;
	HWND m_hContextWnd;
	HMENU m_hMenu;
	CSkinMenu* m_pParentMenu;

	BOOL m_bAnimatedMenus;
	BOOL m_bFirstRedraw; // fix for animated menus

	static ISkinMenuRender* s_pRenderer;

protected:
	virtual void OnNcPaint(CDC* pDC);
	virtual void OnPrintClient(CDC* pDC, DWORD dwFlags);
	virtual void OnPaint(CDC* pDC);

	virtual LRESULT WindowProc(HWND hRealWnd, UINT msg, WPARAM wp, LPARAM lp); 
	BOOL IsValid() { return IsHooked(); } // is it hooked up and ready to go

	void GetDrawRect(LPRECT pWindow, LPRECT pClient = NULL);
	void SetTransparent();
	CDC* ReplaceSystemColors(CDC* pDCDest, CDC* pDestSrc, LPRECT pRect, LPRECT pClip);

	void GetInvalidRect(int nCurSel, int nPrevSel, LPRECT lpRect); // in client coords
	int GetCurSel();

	// style helpers
	BOOL Sidebar() { return m_dwStyle & SKMS_SIDEBAR; }
	BOOL RoundCorners() { return m_dwStyle & SKMS_ROUNDCORNERS; }
	BOOL Flat() { return m_dwStyle & SKMS_FLAT; }

	static BOOL ReplaceColor(CDC* pDCSrc, COLORREF crSrc, CDC* pDCDest, COLORREF crDest, LPRECT pRect, LPRECT pClip);

	// CSkinGlobals wrappers
	COLORREF GetColor(int nColor);
	CFont* GetFont(int nFont);

	inline void SwapDCs(CDC*& pDC1, CDC*& pDC2)
	{
		CDC* pTemp = pDC1;
		pDC1 = pDC2;
		pDC2 = pTemp;
	}

};

#endif // NO_SKIN_CTRLS

#endif // !defined(AFX_SKINMENU_H__E2EFA8F0_B9CD_41AB_98FD_812C963B7ACC__INCLUDED_)

