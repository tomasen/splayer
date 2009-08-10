// SkinGlobals.h: interface for the CSkinGlobals class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINGLOBALS_H__71878295_0054_4004_AC6D_08F61A5E248F__INCLUDED_)
#define AFX_SKINGLOBALS_H__71878295_0054_4004_AC6D_08F61A5E248F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "skinglobalsdata.h"
#include <afxtempl.h>

enum
{
	SBFONT_MARLETT,
	SBFONT_TIP,
	SBFONT_MENU,
	SBFONT_MESSAGE,
	SBFONT_CAPTION,
};

#ifndef NO_SKIN_INI
class CSkinIniGlobalsFile;
#endif

class CSkinGlobals  
{
public:
	CSkinGlobals();
	virtual ~CSkinGlobals();

#ifndef NO_SKIN_INI
	BOOL LoadSkin(const CSkinIniGlobalsFile* pIniFile);
	void UnloadSkin() { Reset(); }
#endif

	void Reset(); // clears colors and deletes brushes and pens

	// fonts
	CFont* GetFont(int nFont);
	BOOL SetFont(LPCTSTR szFaceName, int nPointSize = 0, int nHeight = 0);
	LPCTSTR BaseFontName() { return m_sFontName; }

	// controls
	CBitmap* GetControlBitmap(int nItem, int nState = 0, COLORREF* pMask = NULL, int nAltItem = -1);
	BOOL SetControlBitmap(int nItem, CBitmap bitmap[IM_LAST], COLORREF crMask = -1);
	BOOL SetControlBitmap(int nItem, UINT uIDBitmap[IM_LAST], COLORREF crMask = -1);

	// colors
	void SetColor(int nColor, COLORREF color);
	COLORREF GetColor(int nColor);
	CBrush* GetColorBrush(int nColor);
	CBrush* GetColorBrush(COLORREF color);
	CPen* GetColorPen(int nColor, int nWidth = 0);
	CPen* GetColorPen(COLORREF color, int nWidth = 0);

	// static versions
	static COLORREF GetColorSt(int nColor);
	static CBrush* GetColorBrushSt(int nColor);
	static CBrush* GetColorBrushSt(COLORREF color);
	static CPen* GetColorPenSt(int nColor, int nWidth = 0);
	static CPen* GetColorPenSt(COLORREF color, int nWidth = 0);
	static CFont* GetFontSt(int nFont);

private:
	CMap<int, int, COLORREF, COLORREF&> m_mapColors; 
	CMap<COLORREF, COLORREF, CBrush*, CBrush*&> m_mapBrushes; 
	CMap<COLORREF, COLORREF, CPen*, CPen*&> m_mapPens; 
	CFont m_fontMarlett, m_fontMenu, m_fontTip, m_fontMessage, m_fontCaption;
	CString m_sFontName;
	int m_nFontPointSize, m_nFontHeight;
	CMap<int, int, CSkinBitmap*, CSkinBitmap*&> m_mapCtrlBitmaps;

	static CPen s_pen;
	static CBrush s_brush;
	static CFont s_font;

private:
	void ResetColors();
	void ResetFonts();
	void ResetCtrls();

	CFont* GetMarlett();
	CFont* GetTipFont();
	CFont* GetMenuFont();
	CFont* GetMessageFont();
	CFont* GetCaptionFont();

	BOOL CreateFont(CFont* pFont, LOGFONT* pLF);

};

#endif // !defined(AFX_SKINGLOBALm_H__71878295_0054_4004_AC6D_08F61A5E248F__INCLUDED_)
