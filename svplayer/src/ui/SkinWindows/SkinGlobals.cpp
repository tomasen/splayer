// SkinGlobals.cpp: implementation of the CSkinGlobals class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SkinGlobals.h"
#include "skinbase.h"
#include "skincolors.h"

#ifndef NO_SKIN_INI
#include "Skininifile.h"
#endif

#ifdef _DEBUG
#undef THIm_FILE
static char THIm_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// statics
CPen CSkinGlobals::s_pen;
CBrush CSkinGlobals::s_brush;
CFont CSkinGlobals::s_font;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSkinGlobals::CSkinGlobals() : m_nFontPointSize(0), m_nFontHeight(0)
{
}

CSkinGlobals::~CSkinGlobals()
{
	ResetFonts();
	ResetColors();
	ResetCtrls();
}

void CSkinGlobals::ResetFonts()
{
	m_fontMarlett.DeleteObject();
	m_fontMenu.DeleteObject();
	m_fontTip.DeleteObject();
	m_fontMessage.DeleteObject();
	m_fontCaption.DeleteObject();
}

void CSkinGlobals::ResetCtrls()
{
	// delete control bitmap info
	for (int nCtl = 0; nCtl < SKCB_LAST; nCtl++)
	{
		CSkinBitmap* pSkb = NULL;
		
		if (m_mapCtrlBitmaps.Lookup(nCtl, pSkb))
		{
			delete pSkb;
			m_mapCtrlBitmaps.RemoveKey(nCtl);
		}
	}
}

void CSkinGlobals::ResetColors()
{
	CBrush* pBrush = NULL;
	COLORREF color = 0;
	POSITION pos = m_mapBrushes.GetStartPosition();
	
	while (pos)
	{
		m_mapBrushes.GetNextAssoc(pos, color, pBrush);
		
		if (pBrush)
			pBrush->DeleteObject();
		
		delete pBrush;
	}
	
	CPen* pPen = NULL;
	pos = m_mapPens.GetStartPosition();
	
	while (pos)
	{
		m_mapPens.GetNextAssoc(pos, color, pPen);
		
		if (pPen)
			pPen->DeleteObject();
		
		delete pPen;
	}
	
	m_mapBrushes.RemoveAll();
	m_mapPens.RemoveAll();
	m_mapColors.RemoveAll();
}

#ifndef NO_SKIN_INI
BOOL CSkinGlobals::LoadSkin(const CSkinIniGlobalsFile* pIniFile)
{
	if (!pIniFile)
		return FALSE;
	
	// font
	ResetFonts();

	CSkinFont font;
	pIniFile->GetGlobalFont(font);

	// find the first acceptable font
	CString sFont;
	int nPointSize = 0, nHeight = 0;

	for (int nFont = 0; nFont < font.aFonts.GetSize(); nFont++)
	{
		sFont = font.aFonts[nFont].sFaceName;

		if (sFont.IsEmpty())
			continue;

		if (CSkinBase::FontIsPresent(sFont))
		{
			nPointSize = font.aFonts[nFont].nPointSize;
			nHeight = font.aFonts[nFont].nHeight;
			break;
		}
		else
		{
			// if there is a font file path try adding before trying again
			if (!font.aFonts[nFont].sFilePath.IsEmpty())
			{
				if (AddFontResource(font.aFonts[nFont].sFilePath))
				{
					// verify
					if (CSkinBase::FontIsPresent(sFont))
					{
						nPointSize = font.aFonts[nFont].nPointSize;
						nHeight = font.aFonts[nFont].nHeight;
						break;
					}
				}
			}
			// else
			sFont.Empty();
		}
	}

	SetFont(sFont, nPointSize);

	// colors
	ResetColors();
	
	int nColor = COLOR_LAST;
	
	while (nColor--)
	{
		COLORREF color = pIniFile->GetColor(nColor);
		SetColor(nColor, color);
	}

	// controls
	ResetCtrls();

	pIniFile->GetControlBitmapInfo(m_mapCtrlBitmaps);

	return TRUE;
}

#endif

void CSkinGlobals::Reset()
{
	ResetFonts();
	ResetColors();
	ResetCtrls();
}

COLORREF CSkinGlobals::GetColor(int nColor)
{
	COLORREF color = (COLORREF)-1;
	
	if (m_mapColors.Lookup(nColor, color) && color >= 0)
	{
		return color;
	}

	// else
	color = ::GetSysColor((nColor != COLOR_PARENTBKGND) ? nColor : COLOR_3DFACE);
	
	// else save 0 for transparency
	if (color == 0)
		color = 1;
	
	SetColor(nColor, color);
	
	return color;
}

CBrush* CSkinGlobals::GetColorBrush(int nColor)
{
	COLORREF color = GetColor(nColor);
	
	if (color == ::GetSysColor(nColor))
		return CBrush::FromHandle(::GetSysColorBrush(nColor));
	
	// else
	return GetColorBrush(color);
}

CBrush* CSkinGlobals::GetColorBrush(COLORREF color)
{
	CBrush* pBrush = NULL;
	
	if (m_mapBrushes.Lookup(color, pBrush) && pBrush)
		return pBrush;
	
	pBrush = new CBrush;
	
	if (pBrush->CreateSolidBrush(color))
	{
		m_mapBrushes[color] = pBrush;
		return pBrush;
	}
	
	return NULL;
}

CPen* CSkinGlobals::GetColorPen(int nColor, int nWidth)
{
	COLORREF color = GetColor(nColor);
	
	return GetColorPen(GetColor(nColor), nWidth);
}

CPen* CSkinGlobals::GetColorPen(COLORREF color, int nWidth)
{
	CPen* pPen = NULL;
	
	m_mapPens.Lookup(color, pPen);
	
	// check width matches
	if (pPen)
	{
		LOGPEN lp;
		
		if (pPen->GetLogPen(&lp) && (lp.lopnWidth.x == nWidth))
			return pPen;
		
		// else
		pPen->DeleteObject();
		delete pPen;
	}
	
	pPen = new CPen;
	
	if (pPen->CreatePen(PS_SOLID, nWidth, color))
	{
		m_mapPens[color] = pPen;
		return pPen;
	}
	
	return NULL;
}

void CSkinGlobals::SetColor(int nColor, COLORREF color)
{
	COLORREF crPrev = (COLORREF)-1;
	
	// save 0 for transparency
	if (color == 0)
		color = 1;
	
#ifdef _DEBUG
	CString sColor = GetColorName(nColor);
#endif
	
	// delete any brush or pen associated with the old color
	if (m_mapColors.Lookup(nColor, crPrev) && crPrev >= 0 && crPrev != color)
	{
		CBrush* pBrush = NULL;
		
		if (m_mapBrushes.Lookup(color, pBrush) && pBrush)
		{
			pBrush->DeleteObject();
			delete pBrush;
		}
		
		CPen* pPen = NULL;
		
		if (m_mapPens.Lookup(color, pPen) && pPen)
		{
			pPen->DeleteObject();
			delete pPen;
		}
	}
	
	if (color == -1)
		m_mapColors.RemoveKey(nColor);
	else
		m_mapColors[nColor] = color;
}

CFont* CSkinGlobals::GetFont(int nFont)
{
	switch (nFont)
	{
	case SBFONT_MARLETT:
		return GetMarlett();

	case SBFONT_TIP:
		return GetTipFont();

	case SBFONT_MENU:
		return GetMenuFont();

	case SBFONT_MESSAGE:
		return GetMessageFont();

	case SBFONT_CAPTION:
		return GetCaptionFont();
	}

	ASSERT (0);
	return NULL;
}

CFont* CSkinGlobals::GetMarlett()
{
	if (!m_fontMarlett.GetSafeHandle())
	{
		VERIFY(m_fontMarlett.CreateFont(GetSystemMetrics(SM_CYMENUSIZE) - 6, 0, 0, 0,
			FW_THIN, 0, 0, 0, SYMBOL_CHARSET, 0, 0, 0, 0, _T("Marlett")));
	}
	
	return &m_fontMarlett;
}

CFont* CSkinGlobals::GetMenuFont()
{
	if (!m_fontMenu.GetSafeHandle())
	{
		NONCLIENTMETRICS ncm;
		ZeroMemory(&ncm,sizeof(ncm));
		ncm.cbSize = sizeof(ncm);
	
		// Get the system font
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, (PVOID)&ncm, FALSE);
	
		VERIFY(CreateFont(&m_fontMenu, &ncm.lfMenuFont));
	}

	return &m_fontMenu;
}

CFont* CSkinGlobals::GetTipFont()
{
	if (!m_fontTip.GetSafeHandle())
	{
		NONCLIENTMETRICS ncm;
		ZeroMemory(&ncm,sizeof(ncm));
		ncm.cbSize = sizeof(ncm);
	
		// Get the system font
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, (PVOID)&ncm, FALSE);
	
		VERIFY(CreateFont(&m_fontTip, &ncm.lfStatusFont));
	}

	return &m_fontTip;
}

CFont* CSkinGlobals::GetMessageFont()
{
	if (!m_fontMessage.GetSafeHandle())
	{
		NONCLIENTMETRICS ncm;
		ZeroMemory(&ncm,sizeof(ncm));
		ncm.cbSize = sizeof(ncm);
	
		// Get the system font
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, (PVOID)&ncm, FALSE);
	
		VERIFY(CreateFont(&m_fontMessage, &ncm.lfMessageFont));
	}

	return &m_fontMessage;
}

CFont* CSkinGlobals::GetCaptionFont()
{
	if (!m_fontCaption.GetSafeHandle())
	{
		NONCLIENTMETRICS ncm;
		ZeroMemory(&ncm,sizeof(ncm));
		ncm.cbSize = sizeof(ncm);
	
		// Get the system font
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, (PVOID)&ncm, FALSE);
	
		VERIFY(CreateFont(&m_fontCaption, &ncm.lfCaptionFont));
	}

	return &m_fontCaption;
}

BOOL CSkinGlobals::CreateFont(CFont* pFont, LOGFONT* pLF)
{
	ASSERT (pFont && pLF);

	if (!(pFont && pLF))
		return FALSE;
	
	// override ANSI_CHARSET because some fonts do not show with it
	if (pLF->lfCharSet == ANSI_CHARSET)
		pLF->lfCharSet = DEFAULT_CHARSET;

	pLF->lfQuality = ANTIALIASED_QUALITY;
	pLF->lfOutPrecision = OUT_TT_PRECIS;

	if (!m_sFontName.IsEmpty())
		lstrcpy(pLF->lfFaceName, m_sFontName);

	if (m_nFontPointSize > 0)
	{
		HDC hDC = GetDC(NULL);
		pLF->lfHeight = -MulDiv(m_nFontPointSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
		ReleaseDC(NULL, hDC);
	}
	else if (m_nFontHeight > 0)
		pLF->lfHeight = -m_nFontHeight;

	return pFont->CreateFont(pLF->lfHeight,0,0,0,pLF->lfWeight,FALSE, FALSE, FALSE,DEFAULT_CHARSET ,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH, pLF->lfFaceName);
}

BOOL CSkinGlobals::SetFont(LPCTSTR szFaceName, int nPointSize, int nHeight)
{
	nPointSize = abs(nPointSize);

	if (nPointSize && (nPointSize < 6 || nPointSize > 24))
		return FALSE;

	if (m_sFontName.CompareNoCase(szFaceName) != 0 || m_nFontPointSize != nPointSize)
	{
		if (!CString(szFaceName).IsEmpty() && !CSkinBase::FontIsPresent(szFaceName))
			return FALSE;

		m_sFontName = szFaceName;
		m_nFontPointSize = nPointSize;
		m_nFontHeight = nHeight;

		// delete existing fonts
		ResetFonts();

		return TRUE;
	}

	return FALSE;
}

// static versions
CBrush* CSkinGlobals::GetColorBrushSt(int nColor)
{
	return GetColorBrushSt(::GetSysColor(nColor));
}

CBrush* CSkinGlobals::GetColorBrushSt(COLORREF color)
{
	s_brush.DeleteObject();
	s_brush.CreateSolidBrush(color);

	return &s_brush;
}

CPen* CSkinGlobals::GetColorPenSt(int nColor, int nWidth)
{
	return GetColorPenSt(::GetSysColor(nColor));
}

CPen* CSkinGlobals::GetColorPenSt(COLORREF color, int nWidth)
{
	s_pen.DeleteObject();
	s_pen.CreatePen(PS_SOLID, nWidth, color);

	return &s_pen;
}

CFont* CSkinGlobals::GetFontSt(int nFont)
{
	s_font.DeleteObject();

	if (nFont == SBFONT_MARLETT)
	{
		VERIFY(s_font.CreateFont(GetSystemMetrics(SM_CYMENUSIZE) - 6, 0, 0, 0,
				FW_THIN, 0, 0, 0, SYMBOL_CHARSET, 0, 0, 0, 0, _T("Microsoft Sans Serif")));

		return &s_font;
	}

	return CFont::FromHandle((HFONT)::GetStockObject(ANSI_VAR_FONT));
}

COLORREF CSkinGlobals::GetColorSt(int nColor)
{
	if (nColor == COLOR_PARENTBKGND)
		nColor = COLOR_3DFACE;

	return ::GetSysColor(nColor);
}

CBitmap* CSkinGlobals::GetControlBitmap(int nItem, int nState, COLORREF* pMask, int nAltItem)
{
	ASSERT (nState == IM_HOT || nState == IM_COLD || nState == IM_DOWN || nState == IM_DISABLED);

	CSkinBitmap* pSkb = NULL;

	if ((m_mapCtrlBitmaps.Lookup(nItem, pSkb) && pSkb) ||
		(nAltItem != -1 && m_mapCtrlBitmaps.Lookup(nAltItem, pSkb) && pSkb))
	{
		if (pSkb->IsValid(IM_HOT))
		{
			if (pMask)
				*pMask = pSkb->crMask;

			return pSkb->Image(nState);
		}
	}

	// else
	return NULL;
}

BOOL CSkinGlobals::SetControlBitmap(int nItem, CBitmap bitmap[IM_LAST], COLORREF crMask)
{
	ASSERT (bitmap[IM_HOT].GetSafeHandle());

	if (!bitmap[IM_HOT].GetSafeHandle())
		return FALSE;

	// delete existing bitmaps first
	CSkinBitmap* pSkb = NULL;
	
	if (m_mapCtrlBitmaps.Lookup(nItem, pSkb) && pSkb)
	{
		m_mapCtrlBitmaps.RemoveKey(nItem);
		delete pSkb;
	}
	
	// assign new bitmaps to item
	m_mapCtrlBitmaps[nItem] = new CSkinBitmap(bitmap, crMask);
	
	return TRUE;
}

BOOL CSkinGlobals::SetControlBitmap(int nItem, UINT uIDBitmap[IM_LAST], COLORREF crMask)
{
	CBitmap bitmap[IM_LAST];

	int nBM = IM_LAST;

	while (nBM--)
	{
		if (uIDBitmap[nBM])
			bitmap[nBM].LoadBitmap(uIDBitmap[nBM]);
	}

	return SetControlBitmap(nItem, bitmap, crMask);
}
