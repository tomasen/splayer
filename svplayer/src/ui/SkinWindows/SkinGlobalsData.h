// SkinIniGlobalsFileData.h
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINGLOBALSDATA_H__6E42348F_E57E_41AE_961F_549E69516E9F__INCLUDED_)
#define AFX_SKINGLOBALSDATA_H__6E42348F_E57E_41AE_961F_549E69516E9F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>
#include "skinbase.h"

// for controls that can support 'hot cold and down' buttons.
// eg. buttons, headers, scrollbars, comboboxes, spinbuttons
enum 
{
	SKCB_BUTTONLEFT,
	SKCB_BUTTONRIGHT,
	SKCB_BUTTONMIDDLE,
	SKCB_BUTTONCLIPLEFT,
	SKCB_BUTTONCLIPRIGHT,
	SKCB_HEADERLEFT,
	SKCB_HEADERRIGHT,
	SKCB_HEADERMIDDLE,
	SKCB_HEADERLEFTEND,
	SKCB_HEADERRIGHTEND,
	SKCB_SCROLLALL, // if this is specified then its assumed for SKCB_SPINxxx also
	SKCB_SCROLLLEFT,
	SKCB_SCROLLRIGHT,
	SKCB_SCROLLDOWN,
	SKCB_SCROLLUP,
	SKCB_SCROLLDROPDOWN, // for anything that needs a dropdown
	SKCB_SPINLEFT, // also used for clipped scrollbars
	SKCB_SPINRIGHT,
	SKCB_SPINDOWN,
	SKCB_SPINUP,
	SKCB_TREEPLUS,
	SKCB_TREEMINUS,
	SKCB_TREEALL,
	SKCB_SLIDERTHUMBHORZ,
	SKCB_SLIDERTHUMBVERT,
	SKCB_PROGRESSLEFT,
	SKCB_PROGRESSRIGHT,
	SKCB_PROGRESSMIDDLE,

	SKCB_LAST
};

//////////////////////////////////////////////////////////////////////////////////

class CSkinBitmap
{
public:
	CSkinBitmap() { Reset(); }
	CSkinBitmap(const CSkinBitmap& ski) { *this = ski; }
	CSkinBitmap(const CBitmap bitmap[IM_HOT], COLORREF mask = -1)
	{
		Reset();

		for (int nIm = IM_HOT; nIm < IM_LAST; nIm++)
		{
			if (bitmap[nIm].GetSafeHandle())
				CSkinBase::CopyBitmap(&bitmap[nIm], &bmImage[nIm]);
		}

		crMask = mask;
	}

	virtual ~CSkinBitmap() { Reset(); }

	const CSkinBitmap& CSkinBitmap::operator=(const CSkinBitmap& ski)
	{
		Reset();

		for (int nIm = IM_HOT; nIm < IM_LAST; nIm++)
		{
			if (ski.bmImage[nIm].GetSafeHandle())
				CSkinBase::CopyBitmap(&ski.bmImage[nIm], &bmImage[nIm]);
		}

		crMask = ski.crMask;

		return *this;
	}

	virtual void Reset() 
	{ 
		for (int nIm = IM_HOT; nIm < IM_LAST; nIm++) 
			bmImage[nIm].DeleteObject(); 
	
		crMask = (COLORREF)-1; 
	}

	CBitmap* Images() { return bmImage; }
	CBitmap* Image(int nIm, BOOL bWrap = TRUE)
	{ 
		ASSERT (nIm >= IM_HOT && nIm < IM_LAST);

		if (!bWrap || bmImage[nIm].GetSafeHandle())
			return &bmImage[nIm]; 

		else
			return &bmImage[IM_HOT];
	}

	BOOL IsValid(int nIm = IM_HOT) const
	{ 
		ASSERT (nIm >= IM_HOT && nIm < IM_LAST);

		return (bmImage[nIm].GetSafeHandle() != NULL);
	}

	COLORREF crMask;

protected:
	CBitmap bmImage[IM_LAST];
};

//////////////////////////////////////////////////////////////////////////////////

class CSkinSound
{
public:
	CSkinSound() { Reset(); }
	CSkinSound(const CSkinSound& ski) { *this = ski; }
	virtual ~CSkinSound() { Reset(); }

	const CSkinSound& CSkinSound::operator=(const CSkinSound& ski)
	{
		for (int nIm = 0; nIm < SND_LAST; nIm++)
			sSoundPath[nIm] = ski.sSoundPath[nIm];

		return *this;
	}

	virtual void Reset() 
	{ 
		for (int nSnd = 0; nSnd < SND_LAST; nSnd++) 
			sSoundPath[nSnd].Empty(); 
	}

	void Merge(const CSkinSound& ski) // merge only non empty items
	{
		int nSound = SND_LAST;

		while (nSound--)
		{
			if (sSoundPath[nSound].IsEmpty())
				sSoundPath[nSound] = ski.sSoundPath[nSound];
		}
	}
			
	CString sSoundPath[SND_LAST];
};

//////////////////////////////////////////////////////////////////////////////////

class CSkinFont
{
public:
	CSkinFont() { Reset(); }
	CSkinFont(const CSkinFont& ski) { *this = ski; }
	virtual ~CSkinFont() { Reset(); }

	const CSkinFont& CSkinFont::operator=(const CSkinFont& ski)
	{
		aFonts.Copy(ski.aFonts);

		return *this;
	}

	virtual void Reset() 
	{ 
		aFonts.RemoveAll();
	}

	CArray<FontInfo, FontInfo&> aFonts; // in order 
};

//////////////////////////////////////////////////////////////////////////////////
#endif