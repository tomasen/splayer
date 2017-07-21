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
#include "afxtagmanager.h"
#include "afxtoolbarimages.h"
#include "afxcontrolrenderer.h"
#include "afxtooltipctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static LPCTSTR s_Point     = _T("POINT");
static LPCTSTR s_Size      = _T("SIZE");
static LPCTSTR s_Rect      = _T("RECT");
static LPCTSTR s_Offset    = _T("OFFSET");
static LPCTSTR s_X         = _T("X");
static LPCTSTR s_Y         = _T("Y");
static LPCTSTR s_Width     = _T("WIDTH");
static LPCTSTR s_Height    = _T("HEIGHT");

static LPCTSTR s_Left      = _T("LEFT");
static LPCTSTR s_Right     = _T("RIGHT");
static LPCTSTR s_Top       = _T("TOP");
static LPCTSTR s_Bottom    = _T("BOTTOM");

static LPCTSTR s_LT        = _T("LEFTTOP");
static LPCTSTR s_LB        = _T("LEFTBOTTOM");
static LPCTSTR s_RT        = _T("RIGHTTOP");
static LPCTSTR s_RB        = _T("RIGHTBOTTOM");

static LPCTSTR s_A         = _T("A");
static LPCTSTR s_R         = _T("R");
static LPCTSTR s_G         = _T("G");
static LPCTSTR s_B         = _T("B");

static LPCTSTR s_True      = _T("TRUE");
static LPCTSTR s_False     = _T("FALSE");

static LPCTSTR s_FaceName = _T("FACENAME");
static LPCTSTR s_Charset  = _T("CHARSET");
static LPCTSTR s_Quality  = _T("QUALITY");
static LPCTSTR s_Weight   = _T("WEIGHT");

static LPCTSTR s_Corners     = _T("CORNERS");
static LPCTSTR s_Sides       = _T("SIDES");
static LPCTSTR s_Interior    = _T("INTERIOR");
static LPCTSTR s_Transparent = _T("TRANSPARENT");
static LPCTSTR s_PreMltCheck = _T("PREMLTCHECK");

static LPCTSTR s_TTP_BallonTooltip     = _T("BALLON");
static LPCTSTR s_TTP_DrawIcon          = _T("DRAW_ICON");
static LPCTSTR s_TTP_DrawDescription   = _T("DRAW_DESCRIPTION");
static LPCTSTR s_TTP_DrawSeparator     = _T("DRAW_SEPARATOR");
static LPCTSTR s_TTP_MaxDescrWidth     = _T("MAX_DESC_WIDTH");
static LPCTSTR s_TTP_RoundedCorners    = _T("ROUNDED_CORNERS");
static LPCTSTR s_TTP_BoldLabel         = _T("BOLD_LABEL");
static LPCTSTR s_TTP_ColorFill         = _T("COLOR_FILL");
static LPCTSTR s_TTP_ColorFillGradient = _T("COLOR_FILLGRADIENT");
static LPCTSTR s_TTP_ColorText         = _T("COLOR_TEXT");
static LPCTSTR s_TTP_ColorBorder       = _T("COLOR_BORDER");
static LPCTSTR s_TTP_GradientAngle     = _T("GRADIENT_ANGLE");

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTagManager::CTagManager(LPCTSTR lpszBuffer/* = NULL*/)
{
	SetBuffer(lpszBuffer);
}

CTagManager::~CTagManager()
{
}

void CTagManager::SetBuffer(LPCTSTR lpszBuffer)
{
	m_strBuffer = lpszBuffer == NULL ? _T("") : lpszBuffer;
}

BOOL CTagManager::LoadFromResource(UINT uiResID, LPCTSTR lpszResType)
{
	return LoadFromResource(MAKEINTRESOURCE(uiResID), lpszResType);
}

BOOL CTagManager::LoadFromResource(LPCTSTR lpszResID, LPCTSTR lpszResType)
{
	if (lpszResID == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (lpszResType == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	BOOL bRes = FALSE;

	HINSTANCE hInst = AfxFindResourceHandle(lpszResID, lpszResType);
	HRSRC hRsrc = ::FindResource(hInst, lpszResID, lpszResType);

	if (hRsrc == NULL)
	{
		return FALSE;
	}

	HGLOBAL hGlobal = LoadResource(hInst, hRsrc);

	if (hGlobal == NULL)
	{
		return FALSE;
	}

	LPWSTR lpw = NULL;

#ifdef _UNICODE
	LPSTR lpa = (LPSTR) LockResource (hGlobal);
	ENSURE(lpa != NULL);

	const int nChars = (int)strlen (lpa) + 1;

	lpw = (LPWSTR) _malloca(nChars * 2);
	ENSURE(lpw != NULL);

	LPCTSTR lpszXML = AfxA2WHelper (lpw, lpa, nChars);
#else
	LPCTSTR lpszXML = (LPCTSTR) LockResource(hGlobal);
#endif

	if (lpszXML != NULL)
	{
		SetBuffer(lpszXML);
		bRes = TRUE;
	}

	UnlockResource(hGlobal);
	FreeResource(hGlobal);

	if (lpw != NULL)
	{
		::_freea (lpw);
	}

	return bRes;
}

BOOL CTagManager::LoadFromFile(LPCTSTR lpszFileName)
{
	ASSERT_VALID(this);
	ASSERT(lpszFileName != NULL);

	SetBuffer(NULL);

	CString strFileName = lpszFileName;

	CString strBuffer;
	CString strPath = strFileName;

	if (strFileName.Find(_T("\\")) == -1 && strFileName.Find(_T("/")) == -1 && strFileName.Find(_T(":")) == -1)
	{
		TCHAR lpszFilePath [_MAX_PATH];
		if (::GetModuleFileName(NULL, lpszFilePath, _MAX_PATH) > 0)
		{
			TCHAR path_buffer[_MAX_PATH];
			TCHAR drive[_MAX_DRIVE];
			TCHAR dir[_MAX_DIR];
			TCHAR fname[_MAX_FNAME];
			TCHAR ext[_MAX_EXT];

			_tsplitpath_s(lpszFilePath, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0);
			_tsplitpath_s(strFileName, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT);

			_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, fname, ext);

			strPath = path_buffer;
		}
	}

	try
	{
		CStdioFile file;
		if (!file.Open(strPath, CFile::modeRead))
		{
			TRACE(_T("CTagManager::LoadFromFile: File not found: %s"), strFileName);
			return FALSE;
		}

		CString str;

		while (file.ReadString(str))
		{
			strBuffer += str;
		}
	}
	catch(CFileException* pEx)
	{
		pEx->ReportError();
		pEx->Delete();

		return FALSE;
	}

	SetBuffer(strBuffer);
	return TRUE;
}

BOOL CTagManager::ExcludeTag(LPCTSTR lpszTag, CString& strTag, BOOL bIsCharsList)
{
	const int iBufLen = m_strBuffer.GetLength();

	CString strTagStart = _T("<");
	strTagStart += lpszTag;
	strTagStart += _T('>');

	const int iTagStartLen = strTagStart.GetLength();

	int iStart = -1;

	int iIndexStart = m_strBuffer.Find(strTagStart);
	if (iIndexStart < 0)
	{
		return FALSE;
	}

	iStart = iIndexStart + iTagStartLen;

	CString strTagEnd = _T("</");
	strTagEnd += lpszTag;
	strTagEnd += _T('>');

	const int iTagEndLen = strTagEnd.GetLength();

	int iIndexEnd =  -1;
	int nBalanse = 1;
	for (int i = iStart; i < iBufLen - iTagEndLen + 1; i ++)
	{
		if (m_strBuffer [i] != '<')
		{
			continue;
		}

		if (i < iBufLen - iTagStartLen && _tcsncmp(m_strBuffer.Mid(i), strTagStart, iTagStartLen) == 0)
		{
			i += iTagStartLen - 1;
			nBalanse ++;
			continue;
		}

		if (_tcsncmp(m_strBuffer.Mid(i), strTagEnd, iTagEndLen) == 0)
		{
			nBalanse --;
			if (nBalanse == 0)
			{
				iIndexEnd = i;
				break;
			}

			i += iTagEndLen - 1;
		}
	}

	if (iIndexEnd == -1 || iStart > iIndexEnd)
	{
		return FALSE;
	}

	strTag = m_strBuffer.Mid(iStart, iIndexEnd - iStart);
	strTag.TrimLeft();
	strTag.TrimRight();

	m_strBuffer.Delete(iIndexStart, iIndexEnd + iTagEndLen - iIndexStart);

	if (bIsCharsList)
	{
		if (strTag.GetLength() > 1 && strTag [0] == _T('\"'))
		{
			strTag = strTag.Mid(1, strTag.GetLength() - 2);
		}

		strTag.Replace(_T("\\t"), _T("\t"));
		strTag.Replace(_T("\\n"), _T("\n"));
		strTag.Replace(_T("\\r"), _T("\r"));
		strTag.Replace(_T("\\b"), _T("\b"));
		strTag.Replace(_T("LT"), _T("<"));
		strTag.Replace(_T("GT"), _T(">"));
		strTag.Replace(_T("AMP"), _T("&"));
	}

	return TRUE;
}

BOOL __stdcall CTagManager::ParseString(const CString& str, const CString& sep, CStringArray& sa, BOOL bTrim, BOOL bIncludeEmpty)
{
	sa.RemoveAll();

	if (str.IsEmpty())
	{
		return FALSE;
	}

	CString s(str);
	if (bTrim)
	{
		s.TrimLeft();
		s.TrimRight();
	}

	if (s.IsEmpty())
	{
		return FALSE;
	}

	if (sep.IsEmpty())
	{
		return FALSE;
	}

	int pos = s.Find(sep);

	while (pos != -1)
	{
		CString sp = s.Left(pos);
		s = s.Right(s.GetLength() - sep.GetLength() - pos);

		if (bTrim)
		{
			sp.TrimLeft();
			sp.TrimRight();
			s.TrimLeft();
		}

		if ((sp.IsEmpty() && bIncludeEmpty) || !sp.IsEmpty())
		{
			sa.Add(sp);
		}

		pos = s.Find(sep);

		if (pos == -1 &&((s.IsEmpty() && bIncludeEmpty) || !s.IsEmpty()))
		{
			sa.Add(s);
		}
	}

	return sa.GetSize() > 0;
}

BOOL __stdcall CTagManager::ParseColor(const CString& strItem, COLORREF& value)
{
	CTagManager tm(strItem);

	CStringArray sa;

	CString strA, strR, strG, strB;

	tm.ExcludeTag(s_A, strA);
	strA.TrimLeft();
	strA.TrimRight();
	tm.ExcludeTag(s_R, strR);
	strR.TrimLeft();
	strR.TrimRight();
	tm.ExcludeTag(s_G, strG);
	strG.TrimLeft();
	strG.TrimRight();
	tm.ExcludeTag(s_B, strB);
	strB.TrimLeft();
	strB.TrimRight();

	if (strR.IsEmpty() || strG.IsEmpty() || strB.IsEmpty())
	{
		if (!ParseString(strItem, _T(","), sa, TRUE, FALSE))
		{
			strR = tm.GetBuffer();
			strR.TrimLeft();
			strR.TrimRight();

			sa.Add(strR);
		}
	}
	else
	{
		sa.Add(strR);
		sa.Add(strG);
		sa.Add(strB);

		if (!strA.IsEmpty())
		{
			sa.Add(strA);
		}
	}

	if (sa.GetSize() > 0)
	{
		const int count = (int) sa.GetSize();
		if (count == 3)
		{
			value = (COLORREF)RGB(_ttol(sa[0]), _ttol(sa[1]), _ttol(sa[2]));
			return TRUE;
		}
		/*
		else if (count == 4)
		{
		value = (COLORREF)RGBA(_ttol(sa[0]), _ttol(sa[1]), _ttol(sa[2]),  _ttol(sa[3]));
		return TRUE;
		}
		*/
		else if (count == 1)
		{
			value = (COLORREF)_ttol(sa[0]);
			return TRUE;
		}
	}

	return FALSE;
}

BOOL __stdcall CTagManager::ParseColorHEX(const CString& strItem, COLORREF& value)
{
	CString str(strItem);

	str.MakeUpper();
	str.TrimLeft();
	str.TrimRight();

	const int len = str.GetLength();

	if (len < 6)
	{
		return FALSE;
	}

	BOOL bRes = TRUE;
	BYTE clr[3] = {0, 0, 0};
	int nColor = 0;
	int nRead = 0;

	int val = 0;

	for (int i = 0; i < 6; i++)
	{
		TCHAR c = str[len - i - 1];

		if (TCHAR('A') <= c && c <= TCHAR('F'))
		{
			val = 10 +(c - TCHAR('A'));
		}
		else if (TCHAR('0') <= c && c <= TCHAR('9'))
		{
			val = c - TCHAR('0');
		}
		else
		{
			bRes = FALSE;
			break;
		}

		if (nRead == 0)
		{
			clr[nColor] = (BYTE)val;
		}
		else
		{
			clr[nColor] |= val << 4;
		}

		nRead++;

		if (nRead == 2)
		{
			nRead = 0;
			nColor++;
		}
	}

	if (bRes)
	{
		value = RGB(clr[2], clr[1], clr[0]);
	}

	return bRes;
}

BOOL __stdcall CTagManager::ParsePoint(const CString& strItem, CPoint& value)
{
	CTagManager tm(strItem);

	CStringArray sa;

	CString strX, strY;

	tm.ExcludeTag(s_X, strX);
	strX.TrimLeft();
	strX.TrimRight();
	tm.ExcludeTag(s_Y, strY);
	strY.TrimLeft();
	strY.TrimRight();

	if (strX.IsEmpty() || strY.IsEmpty())
	{
		if (!ParseString(tm.GetBuffer(), _T(","), sa, TRUE, FALSE))
		{
			return FALSE;
		}
	}
	else
	{
		sa.Add(strX);
		sa.Add(strY);
	}

	if (sa.GetSize() == 2)
	{
		value.x = _ttol(sa[0]);
		value.y = _ttol(sa[1]);
		return TRUE;
	}

	return FALSE;
}

BOOL __stdcall CTagManager::ParseSize(const CString& strItem, CSize& value)
{
	CTagManager tm(strItem);
	CStringArray sa;

	CString strW, strH;

	tm.ExcludeTag(s_Width, strW);
	strW.TrimLeft();
	strW.TrimRight();
	tm.ExcludeTag(s_Height, strH);
	strH.TrimLeft();
	strH.TrimRight();

	if (strW.IsEmpty() || strH.IsEmpty())
	{
		if (!ParseString(tm.GetBuffer(), _T(","), sa, TRUE, FALSE))
		{
			return FALSE;
		}
	}
	else
	{
		sa.Add(strW);
		sa.Add(strH);
	}

	if (sa.GetSize() == 2)
	{
		value.cx = _ttol(sa[0]);
		value.cy = _ttol(sa[1]);
		return TRUE;
	}

	return FALSE;
}

BOOL __stdcall CTagManager::ParseRect(const CString& strItem, CRect& value)
{
	CTagManager tm(strItem);

	CString str1;
	CString str2;

	tm.ExcludeTag(s_Offset, str1);
	str1.TrimLeft();
	str1.TrimRight();
	tm.ExcludeTag(s_Size, str2);
	str2.TrimLeft();
	str2.TrimRight();

	CPoint pt(0, 0);
	CSize  sz(0, 0);

	if (ParsePoint(str1, pt) && ParseSize(str2, sz))
	{
		value = CRect(pt, sz);
		return TRUE;
	}

	tm.SetBuffer(strItem);
	tm.ExcludeTag(s_LT, str1);
	str1.TrimLeft();
	str1.TrimRight();
	tm.ExcludeTag(s_RB, str2);
	str2.TrimLeft();
	str2.TrimRight();

	CPoint pt2(0, 0);
	if (ParsePoint(str1, pt) && ParsePoint(str2, pt2))
	{
		value = CRect(pt, pt2);
		return TRUE;
	}

	CStringArray sa;

	CString strL, strT, strR, strB;

	tm.SetBuffer(strItem);

	tm.ExcludeTag(s_Left, strL);
	strL.TrimLeft();
	strL.TrimRight();
	tm.ExcludeTag(s_Top, strT);
	strT.TrimLeft();
	strT.TrimRight();
	tm.ExcludeTag(s_Right, strR);
	strR.TrimLeft();
	strR.TrimRight();
	tm.ExcludeTag(s_Bottom, strB);
	strB.TrimLeft();
	strB.TrimRight();

	if (strL.IsEmpty() || strT.IsEmpty() || strR.IsEmpty() || strB.IsEmpty())
	{
		if (!ParseString(tm.GetBuffer(), _T(","), sa, TRUE, FALSE))
		{
			return FALSE;
		}
	}
	else
	{
		sa.Add(strL);
		sa.Add(strT);
		sa.Add(strR);
		sa.Add(strB);
	}

	if (sa.GetSize() == 4)
	{
		value.left   = _ttol(sa[0]);
		value.top    = _ttol(sa[1]);
		value.right  = _ttol(sa[2]);
		value.bottom = _ttol(sa[3]);
		return TRUE;
	}

	return FALSE;
}

BOOL __stdcall CTagManager::ParseFont(const CString& strItem, LOGFONT& value)
{
	CTagManager tm(strItem);

	CString strFontItem;

	if (tm.ExcludeTag(s_FaceName, strFontItem))
	{
		ASSERT(!strFontItem.IsEmpty());

		if (!strFontItem.IsEmpty())
		{
			memcpy(value.lfFaceName, (LPCTSTR)strFontItem, min(strFontItem.GetLength(), LF_FACESIZE) * sizeof(TCHAR));
		}
	}

	int val = 0;
	if (tm.ReadInt(s_Height, val))
	{
		value.lfHeight = val;
	}

	if (value.lfHeight > 0)
	{
		if (tm.ReadInt(s_Width, val))
		{
			value.lfWidth = val;
		}
	}

	if (tm.ExcludeTag(s_Weight, strFontItem))
	{
		struct WEIGHT_FONT_TYPE
		{
			LPCTSTR name;
			LONG    weight;
		};

		const WEIGHT_FONT_TYPE WEIGHT_FONT_TYPES[] =
		{
			{_T("DONTCARE")  , FW_DONTCARE  },
			{_T("THIN")      , FW_THIN      },
			{_T("EXTRALIGHT"), FW_EXTRALIGHT},
			{_T("LIGHT")     , FW_LIGHT     },
			{_T("NORMAL")    , FW_NORMAL    },
			{_T("MEDIUM")    , FW_MEDIUM    },
			{_T("SEMIBOLD")  , FW_SEMIBOLD  },
			{_T("BOLD")      , FW_BOLD      },
			{_T("EXTRABOLD") , FW_EXTRABOLD },
			{_T("HEAVY")     , FW_HEAVY     },
			{_T("ULTRALIGHT"), FW_ULTRALIGHT},
			{_T("REGULAR")   , FW_REGULAR   },
			{_T("DEMIBOLD")  , FW_DEMIBOLD  },
			{_T("ULTRABOLD") , FW_ULTRABOLD },
			{_T("BLACK")     , FW_BLACK     }
		};

		for (long i = 0; i < sizeof(WEIGHT_FONT_TYPES) / sizeof(WEIGHT_FONT_TYPE); i++)
		{
			if (strFontItem.CompareNoCase(WEIGHT_FONT_TYPES[i].name) == 0)
			{
				value.lfWeight = WEIGHT_FONT_TYPES[i].weight;
				break;
			}
		}
	}

	if (tm.ExcludeTag(s_Quality, strFontItem))
	{
		struct QUALITY_FONT_TYPE
		{
			LPCTSTR name;
			BYTE    quality;
		};

		const QUALITY_FONT_TYPE QUALITY_FONT_TYPES[] =
		{
			{_T("DEFAULT")          , DEFAULT_QUALITY          },
			{_T("DRAFT")            , DRAFT_QUALITY            },
			{_T("PROOF")            , PROOF_QUALITY            },
			{_T("NONANTIALIASED")   , NONANTIALIASED_QUALITY   },
			{_T("ANTIALIASED")      , ANTIALIASED_QUALITY      },
			{_T("CLEARTYPE")        , 5},//CLEARTYPE_QUALITY
			{_T("CLEARTYPE_NATURAL"), 6} //CLEARTYPE_NATURAL_QUALITY
		};

		for (long i = 0; i < sizeof(QUALITY_FONT_TYPES) / sizeof(QUALITY_FONT_TYPE); i++)
		{
			if (strFontItem.CompareNoCase(QUALITY_FONT_TYPES[i].name) == 0)
			{
				if (QUALITY_FONT_TYPES[i].quality <= ANTIALIASED_QUALITY)
				{
					value.lfQuality = QUALITY_FONT_TYPES[i].quality;
				}
				else
				{
					OSVERSIONINFO osvi;
					osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
					::GetVersionEx(&osvi);

					if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.dwMajorVersion >= 5)
					{
						value.lfQuality = QUALITY_FONT_TYPES[i].quality;
					}
				}
				break;
			}
		}
	}

	return TRUE;
}

BOOL __stdcall CTagManager::ParseToolBarImages(const CString& strItem, CMFCToolBarImages& value, UINT ID)
{
	return ParseToolBarImages(strItem, value, MAKEINTRESOURCE(ID));
}

BOOL __stdcall CTagManager::ParseToolBarImages(const CString& strItem, CMFCToolBarImages& value, LPCTSTR lpszID)
{
	CTagManager tm(strItem);

	value.Clear();
	value.SetTransparentColor((COLORREF)(-1));

	CSize size(0, 0);
	if (!tm.ReadSize(s_Size, size))
	{
		return FALSE;
	}

	if (size == CSize(0, 0))
	{
		return FALSE;
	}

	BOOL bPreMultiplyCheck = TRUE;
	tm.ReadBool(s_PreMltCheck, bPreMultiplyCheck);

	value.SetPreMultiplyAutoCheck(bPreMultiplyCheck);
	value.SetImageSize(size);

	value.LoadStr(lpszID);

	COLORREF clrTransparent = CLR_DEFAULT;
	if (tm.ReadColor(s_Transparent, clrTransparent))
	{
		value.SetTransparentColor(clrTransparent);
	}

	if (CMFCToolBarImages::IsRTL() && value.GetImageWell() != NULL && clrTransparent == CLR_DEFAULT)
	{
		BITMAP bmp;
		if (::GetObject(value.GetImageWell(), sizeof(BITMAP), &bmp) != 0)
		{
			if (bmp.bmBitsPixel == 32)
			{
				value.Mirror();
			}
		}
	}

	return TRUE;
}

BOOL __stdcall CTagManager::ParseControlRendererInfo(const CString& strItem, CMFCControlRendererInfo& value)
{
	CTagManager tm(strItem);

	CMFCControlRendererInfo params;
	params.SetResourceID(value.GetResourceID());

	if (!tm.ReadRect(s_Rect, params.m_rectImage))
	{
		CSize size;

		if (tm.ReadSize(s_Size, size))
		{
			params.m_rectImage = CRect(CPoint(0, 0), size);
		}
	}

	if (params.m_rectImage.IsRectEmpty())
	{
		return FALSE;
	}

	tm.ReadRect(s_Corners, params.m_rectCorners);
	tm.ReadRect(s_Sides, params.m_rectSides);
	tm.ReadRect(s_Interior, params.m_rectInter);
	tm.ReadColor(s_Transparent, params.m_clrTransparent);
	tm.ReadBool(s_PreMltCheck, params.m_bPreMultiplyCheck);

	value = params;

	return TRUE;
}

BOOL __stdcall CTagManager::ParseControlRenderer(const CString& strItem, CMFCControlRenderer& value, UINT ID)
{
	return ParseControlRenderer(strItem, value, MAKEINTRESOURCE(ID));
}

BOOL __stdcall CTagManager::ParseControlRenderer(const CString& strItem, CMFCControlRenderer& value, LPCTSTR lpszID)
{
	value.CleanUp();

	CMFCControlRendererInfo params(lpszID, CRect(0, 0, 0, 0), CRect(0, 0, 0, 0));

	if (ParseControlRendererInfo(strItem, params))
	{
		return value.Create(params);
	}

	return FALSE;
}

BOOL __stdcall CTagManager::ParseToolTipInfo(const CString& strItem, CMFCToolTipInfo& value)
{
	CTagManager tm(strItem);

	CMFCToolTipInfo params;

	tm.ReadBool(s_TTP_BallonTooltip    , params.m_bBalloonTooltip);
	tm.ReadBool(s_TTP_DrawIcon         , params.m_bDrawIcon);
	tm.ReadBool(s_TTP_DrawDescription  , params.m_bDrawDescription);
	tm.ReadInt(s_TTP_MaxDescrWidth     , params.m_nMaxDescrWidth);
	tm.ReadBool(s_TTP_RoundedCorners   , params.m_bRoundedCorners);
	tm.ReadBool(s_TTP_BoldLabel        , params.m_bBoldLabel);
	tm.ReadColor(s_TTP_ColorFill       , params.m_clrFill);
	tm.ReadColor(s_TTP_ColorFillGradient, params.m_clrFillGradient);
	tm.ReadInt(s_TTP_GradientAngle     , params.m_nGradientAngle);
	tm.ReadColor(s_TTP_ColorText       , params.m_clrText);
	tm.ReadColor(s_TTP_ColorBorder     , params.m_clrBorder);
	tm.ReadBool(s_TTP_DrawSeparator    , params.m_bDrawSeparator);

	value = params;

	return TRUE;
}

BOOL CTagManager::ReadBool(const CString& strValue, BOOL& value)
{
	BOOL bRes = FALSE;
	CString strItem;

	if (ExcludeTag(strValue, strItem))
	{
		value = strItem.CompareNoCase(s_True) == 0;
		bRes = TRUE;
	}

	return bRes;
}

BOOL CTagManager::ReadInt(const CString& strValue, int& value)
{
	BOOL bRes = FALSE;
	CString strItem;

	if (ExcludeTag(strValue, strItem))
	{
		strItem.TrimLeft();
		strItem.TrimRight();

		value = _ttol(strItem);
		bRes = TRUE;
	}

	return bRes;
}

BOOL CTagManager::ReadPoint(const CString& strValue, CPoint& value)
{
	CString strItem;

	if (ExcludeTag(strValue, strItem))
	{
		return ParsePoint(strItem, value);
	}

	return FALSE;
}

BOOL CTagManager::ReadSize(const CString& strValue, CSize& value)
{
	CString strItem;

	if (ExcludeTag(strValue, strItem))
	{
		return ParseSize(strItem, value);
	}

	return FALSE;
}

BOOL CTagManager::ReadRect(const CString& strValue, CRect& value)
{
	CString strItem;

	if (ExcludeTag(strValue, strItem))
	{
		return ParseRect(strItem, value);
	}

	return FALSE;
}

BOOL CTagManager::ReadColor(const CString& strValue, COLORREF& value)
{
	CString strItem;

	if (ExcludeTag(strValue, strItem))
	{
		return ParseColor(strItem, value);
	}

	return FALSE;
}

BOOL CTagManager::ReadFont(const CString& strValue, LOGFONT& value)
{
	CString strItem;

	if (ExcludeTag(strValue, strItem))
	{
		return ParseFont(strItem, value);
	}

	return FALSE;
}

BOOL CTagManager::ReadToolBarImages(const CString& strValue, CMFCToolBarImages& value, UINT ID)
{
	return ReadToolBarImages(strValue, value, MAKEINTRESOURCE(ID));
}

BOOL CTagManager::ReadToolBarImages(const CString& strValue, CMFCToolBarImages& value, LPCTSTR lpszID)
{
	CString strItem;

	if (ExcludeTag(strValue, strItem))
	{
		return ParseToolBarImages(strItem, value, lpszID);
	}

	return FALSE;
}

BOOL CTagManager::ReadControlRendererInfo(const CString& strValue, CMFCControlRendererInfo& value)
{
	CString strItem;

	if (ExcludeTag(strValue, strItem))
	{
		return ParseControlRendererInfo(strItem, value);
	}

	return FALSE;
}

BOOL CTagManager::ReadControlRenderer(const CString& strValue, CMFCControlRenderer& value, UINT ID)
{
	return ReadControlRenderer(strValue, value, MAKEINTRESOURCE(ID));
}

BOOL CTagManager::ReadControlRenderer(const CString& strValue, CMFCControlRenderer& value, LPCTSTR lpszID)
{
	CString strItem;

	if (ExcludeTag(strValue, strItem))
	{
		return ParseControlRenderer(strItem, value, lpszID);
	}

	return FALSE;
}

BOOL CTagManager::ReadToolTipInfo(const CString& strValue, CMFCToolTipInfo& value)
{
	CString strItem;

	if (ExcludeTag(strValue, strItem))
	{
		return ParseToolTipInfo(strItem, value);
	}

	return FALSE;
}



