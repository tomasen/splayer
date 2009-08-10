// SkinColors.h: interface for the CSkinColors class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINCOLORS_H__CC5BF634_6783_41C7_9CE6_321CCB130425__INCLUDED_)
#define AFX_SKINCOLORS_H__CC5BF634_6783_41C7_9CE6_321CCB130425__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// color mapping
struct ColorItem { LPCTSTR szName; int nColor; };

// new colors
enum
{
	COLOR_PARENTBKGND = 0x00ff,

	COLOR_LAST,
};

static ColorItem colorMap[] = 
{
	{ _T("SCROLLBAR"), COLOR_SCROLLBAR },
	{ _T("BACKGROUND"), COLOR_BACKGROUND },
	{ _T("ACTIVECAPTION"), COLOR_ACTIVECAPTION },
	{ _T("INACTIVECAPTION"), COLOR_INACTIVECAPTION },
	{ _T("MENU"), COLOR_MENU },
	{ _T("WINDOW"), COLOR_WINDOW },
	{ _T("WINDOWFRAME"), COLOR_WINDOWFRAME },
	{ _T("MENUTEXT"), COLOR_MENUTEXT },
	{ _T("WINDOWTEXT"), COLOR_WINDOWTEXT },
	{ _T("CAPTIONTEXT"), COLOR_CAPTIONTEXT },
	{ _T("ACTIVEBORDER"), COLOR_ACTIVEBORDER },
	{ _T("INACTIVEBORDER"), COLOR_INACTIVEBORDER },
	{ _T("APPWORKSPACE"), COLOR_APPWORKSPACE },
	{ _T("HIGHLIGHT"), COLOR_HIGHLIGHT },
	{ _T("HIGHLIGHTTEXT"), COLOR_HIGHLIGHTTEXT },
	{ _T("BTNFACE"), COLOR_BTNFACE },
	{ _T("BTNSHADOW"), COLOR_BTNSHADOW },
	{ _T("GRAYTEXT"), COLOR_GRAYTEXT },
	{ _T("BTNTEXT"), COLOR_BTNTEXT },
	{ _T("INACTIVECAPTIONTEXT"), COLOR_INACTIVECAPTIONTEXT },
	{ _T("BTNHIGHLIGHT"), COLOR_BTNHIGHLIGHT },
	{ _T("3DDKSHADOW"), COLOR_3DDKSHADOW },
	{ _T("3DLIGHT"), COLOR_3DLIGHT },
	{ _T("INFOTEXT"), COLOR_INFOTEXT },
	{ _T("INFOBK"), COLOR_INFOBK },

#ifndef _WIN32_WCE

#if(WINVER >= 0x0500) // xp
	{ _T("HOTLIGHT"), COLOR_HOTLIGHT },
	{ _T("GRADIENTACTIVECAPTION"), COLOR_GRADIENTACTIVECAPTION },
	{ _T("GRADIENTINACTIVECAPTION"), COLOR_GRADIENTINACTIVECAPTION },
#endif /* WINVER >= 0x0500 */

#endif

	{ _T("DESKTOP"), COLOR_DESKTOP },
	{ _T("3DFACE"), COLOR_3DFACE },
	{ _T("3DSHADOW"), COLOR_3DSHADOW },
	{ _T("3DHIGHLIGHT"), COLOR_3DHIGHLIGHT }, 
	{ _T("3DHILIGHT"), COLOR_3DHILIGHT },

#ifndef _WIN32_WCE
	{ _T("BTNHILIGHT"), COLOR_BTNHILIGHT },
#endif

	// new colors
	{ _T("PARENTBKGND"), COLOR_PARENTBKGND },

};

const int NUM_COLORS = sizeof(colorMap) / sizeof(ColorItem);

static LPCTSTR GetColorName(int nColor)
{
	int nCount = NUM_COLORS;

	while (nCount--)
	{
		if (colorMap[nCount].nColor == nColor)
			return colorMap[nCount].szName;
	}

	return NULL;
}

#endif // !defined(AFX_SKINCOLORS_H__CC5BF634_6783_41C7_9CE6_321CCB130425__INCLUDED_)
