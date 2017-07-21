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
#include "afxtoolbar.h"
#include "afxribbonres.h"
#include "afxtoolbarfontcombobox.h"
#include "afxfontcombobox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CMFCToolBarFontComboBox

IMPLEMENT_SERIAL(CMFCToolBarFontComboBox, CMFCToolBarComboBoxButton, 1)

CObList CMFCToolBarFontComboBox::m_lstFonts;
int CMFCToolBarFontComboBox::m_nCount = 0;
int CMFCToolBarFontComboBox::m_nFontHeight = 0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFCToolBarFontComboBox::CMFCToolBarFontComboBox() :
	m_nCharSet(DEFAULT_CHARSET), m_nFontType(DEVICE_FONTTYPE | RASTER_FONTTYPE | TRUETYPE_FONTTYPE), m_nPitchAndFamily(DEFAULT_PITCH)
{
	m_pLstFontsExternal = NULL;
	m_nCount++;
}

CMFCToolBarFontComboBox::CMFCToolBarFontComboBox(UINT uiID, int iImage, int nFontType, BYTE nCharSet, DWORD dwStyle, int iWidth, BYTE nPitchAndFamily) :
	CMFCToolBarComboBoxButton(uiID, iImage, dwStyle, iWidth), m_nFontType(nFontType), m_nCharSet(nCharSet), m_nPitchAndFamily(nPitchAndFamily)
{
	m_pLstFontsExternal = NULL;

	if (m_nCount++ == 0)
	{
		RebuildFonts();
	}

	SetContext();
}

CMFCToolBarFontComboBox::CMFCToolBarFontComboBox(CObList* pLstFontsExternal, int nFontType, BYTE nCharSet, BYTE nPitchAndFamily) :
	m_nFontType(nFontType), m_nCharSet(nCharSet), m_nPitchAndFamily(nPitchAndFamily), m_pLstFontsExternal(pLstFontsExternal)
{
	ASSERT_VALID(m_pLstFontsExternal);
	RebuildFonts();
}

CMFCToolBarFontComboBox::~CMFCToolBarFontComboBox()
{
	if (m_pLstFontsExternal == NULL)
	{
		if (--m_nCount == 0)
		{
			ClearFonts();
		}
	}
}

void CMFCToolBarFontComboBox::RebuildFonts()
{
	CObList& lstFonts = m_pLstFontsExternal != NULL ? *m_pLstFontsExternal : m_lstFonts;
	ASSERT(lstFonts.IsEmpty());

	// First, take the screen fonts:
	CWindowDC dc(NULL);

	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfCharSet = m_nCharSet;

	::EnumFontFamiliesEx(dc.GetSafeHdc(), &lf, (FONTENUMPROC) EnumFamScreenCallBackEx, (LPARAM) this, NULL);

	// Now, take the printer fonts:
	CPrintDialog dlgPrint(FALSE);

	if (AfxGetApp()->GetPrinterDeviceDefaults(&dlgPrint.m_pd))
	{
		HDC hDCPrint = dlgPrint.CreatePrinterDC();
		ENSURE(hDCPrint != NULL);

		::EnumFontFamiliesEx(hDCPrint, &lf, (FONTENUMPROC) EnumFamPrinterCallBackEx, (LPARAM) this, NULL);

		::DeleteObject(hDCPrint);
	}
}

void __stdcall CMFCToolBarFontComboBox::ClearFonts()
{
	while (!m_lstFonts.IsEmpty())
	{
		delete(CMFCFontInfo*) m_lstFonts.RemoveHead();
	}
}

void CMFCToolBarFontComboBox::SetContext()
{
	for (POSITION pos = m_lstFonts.GetHeadPosition(); pos != NULL;)
	{
		CMFCFontInfo* pDesc = (CMFCFontInfo*) m_lstFonts.GetNext(pos);
		ASSERT_VALID(pDesc);

		if ((m_nFontType & pDesc->m_nType) != 0)
		{
			BOOL bIsUnique = GetFontsCount(pDesc->m_strName) <= 1;
			AddItem(bIsUnique ? pDesc->m_strName : pDesc->GetFullName(), (DWORD_PTR) pDesc);
		}
	}
}

BOOL CALLBACK AFX_EXPORT CMFCToolBarFontComboBox::EnumFamScreenCallBackEx(ENUMLOGFONTEX* pelf, NEWTEXTMETRICEX* /*lpntm*/, int FontType, LPVOID pThis)
{
	CMFCToolBarFontComboBox* pCombo = (CMFCToolBarFontComboBox*) pThis;
	ASSERT_VALID(pCombo);

	pCombo->AddFont((ENUMLOGFONT*)pelf, FontType, CString(pelf->elfScript));
	return 1;
}

BOOL CALLBACK AFX_EXPORT CMFCToolBarFontComboBox::EnumFamPrinterCallBackEx(ENUMLOGFONTEX* pelf, NEWTEXTMETRICEX* /*lpntm*/, int FontType, LPVOID pThis)
{
	CMFCToolBarFontComboBox* pCombo = (CMFCToolBarFontComboBox*) pThis;
	ASSERT_VALID(pCombo);

	CString strName = pelf->elfLogFont.lfFaceName;

	pCombo->AddFont((ENUMLOGFONT*)pelf, FontType, CString(pelf->elfScript));
	return 1;
}

BOOL CMFCToolBarFontComboBox::AddFont(ENUMLOGFONT* pelf, int nType, LPCTSTR lpszScript)
{
	LOGFONT& lf = pelf->elfLogFont;

	CObList& lstFonts = m_pLstFontsExternal != NULL ? *m_pLstFontsExternal : m_lstFonts;

	// Don't put in MAC fonts, commdlg doesn't either
	if (lf.lfCharSet == MAC_CHARSET)
	{
		return FALSE;
	}

	BYTE bFontPitch    = lf.lfPitchAndFamily & 0x0F;
	BYTE bFontFamily   = lf.lfPitchAndFamily & 0xF0;
	BYTE bFilterPitch  = m_nPitchAndFamily & 0x0F;
	BYTE bFilterFamily = m_nPitchAndFamily & 0xF0;

	if ((bFilterPitch != DEFAULT_PITCH) && (bFilterPitch != bFontPitch))
	{
		return FALSE;
	}

	if ((bFilterFamily != FF_DONTCARE) && (bFilterFamily != bFontFamily))
	{
		return FALSE;
	}

	POSITION pos = NULL;
	for (pos = lstFonts.GetHeadPosition(); pos != NULL;)
	{
		CMFCFontInfo* pDesc = (CMFCFontInfo*) lstFonts.GetNext(pos);
		ASSERT_VALID(pDesc);

		if (pDesc->m_strName == lf.lfFaceName)
		{
			// Already in list
			return FALSE;
		}
	}

	// Don't display vertical font for FE platform:
	if ((GetSystemMetrics(SM_DBCSENABLED)) &&(lf.lfFaceName[0] == '@'))
	{
		return FALSE;
	}

	CMFCFontInfo* pDesc = new CMFCFontInfo(lf.lfFaceName, lpszScript, lf.lfCharSet, lf.lfPitchAndFamily, nType);
	ASSERT_VALID(pDesc);

	// Fonts list is sorted by name:
	BOOL bInserted = FALSE;
	for (pos = lstFonts.GetHeadPosition(); !bInserted && pos != NULL;)
	{
		POSITION posSave = pos;

		CMFCFontInfo* pDescList = (CMFCFontInfo*) lstFonts.GetNext(pos);
		ASSERT_VALID(pDescList);

		if (pDescList->GetFullName() >= pDesc->GetFullName())
		{
			lstFonts.InsertBefore(posSave, pDesc);
			bInserted = TRUE;
		}
	}

	if (!bInserted)
	{
		lstFonts.AddTail(pDesc);
	}

	return TRUE;
}

void CMFCToolBarFontComboBox::Serialize(CArchive& ar)
{
	// Override to disable item's data serialization!

	CMFCToolBarButton::Serialize(ar);

	if (ar.IsLoading())
	{
		ar >> m_iWidth;
		m_rect.right = m_rect.left + m_iWidth;
		ar >> m_dwStyle;
		ar >> m_iSelIndex;
		ar >> m_strEdit;
		ar >> m_nDropDownHeight;
		ar >> m_nFontType;
		ar >> m_nCharSet;

		if (m_lstFonts.IsEmpty())
		{
			RebuildFonts();
		}

		SetContext();
		SelectItem(m_iSelIndex);
	}
	else
	{
		ar << m_iWidth;
		ar << m_dwStyle;
		ar << m_iSelIndex;
		ar << m_strEdit;
		ar << m_nDropDownHeight;
		ar << m_nFontType;
		ar << m_nCharSet;
	}
}

int __stdcall CMFCToolBarFontComboBox::GetFontsCount(LPCTSTR lpszName)
{
	ASSERT(!m_lstFonts.IsEmpty());

	int nCount = 0;

	for (POSITION pos = m_lstFonts.GetHeadPosition(); pos != NULL;)
	{
		CMFCFontInfo* pDesc = (CMFCFontInfo*) m_lstFonts.GetNext(pos);
		ASSERT_VALID(pDesc);

		if (pDesc->m_strName == lpszName)
		{
			nCount++;
		}
	}

	return nCount;
}

CComboBox* CMFCToolBarFontComboBox::CreateCombo(CWnd* pWndParent, const CRect& rect)
{
	CMFCFontComboBox* pWndCombo = new CMFCFontComboBox;
	if (!pWndCombo->Create(m_dwStyle | CBS_HASSTRINGS | CBS_OWNERDRAWFIXED, rect, pWndParent, m_nID))
	{
		delete pWndCombo;
		return NULL;
	}

	return pWndCombo;
}

BOOL CMFCToolBarFontComboBox::SetFont(LPCTSTR lpszName, BYTE nCharSet, BOOL bExact)
{
	ENSURE(lpszName != NULL);
	CString strNameFind = lpszName;
	strNameFind.MakeLower();

	for (POSITION pos = m_lstItemData.GetHeadPosition(); pos != NULL;)
	{
		BOOL bFound = FALSE;

		CMFCFontInfo* pDesc = (CMFCFontInfo*) m_lstItemData.GetNext(pos);
		ASSERT_VALID(pDesc);

		CString strName = pDesc->GetFullName();
		strName.MakeLower();

		if (bExact)
		{
			if (strName == strNameFind || (pDesc->m_strName.CompareNoCase(lpszName) == 0 && (nCharSet == pDesc->m_nCharSet || nCharSet == DEFAULT_CHARSET)))
			{
				bFound = TRUE;
			}
		}
		else if (strName.Find(strNameFind) == 0 && (nCharSet == DEFAULT_CHARSET || pDesc->m_nCharSet == nCharSet))
		{
			bFound = TRUE;
		}

		if (bFound)
		{
			SelectItem((DWORD_PTR) pDesc);
			return TRUE;
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// CMFCToolBarFontSizeComboBox

static int nFontSizes[] = {8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72};

IMPLEMENT_SERIAL(CMFCToolBarFontSizeComboBox, CMFCToolBarComboBoxButton, 1)

// Construction/Destruction
CMFCToolBarFontSizeComboBox::CMFCToolBarFontSizeComboBox()
{
	m_nLogVert = 0;
}

CMFCToolBarFontSizeComboBox::CMFCToolBarFontSizeComboBox(UINT uiID, int iImage, DWORD dwStyle, int iWidth) :
	CMFCToolBarComboBoxButton(uiID, iImage, dwStyle, iWidth)
{
	m_nLogVert = 0;
}

CMFCToolBarFontSizeComboBox::~CMFCToolBarFontSizeComboBox()
{
}

void CMFCToolBarFontSizeComboBox::RebuildFontSizes(const CString& strFontName)
{
	if (strFontName.IsEmpty())
	{
		return;
	}

	CString strText = m_strEdit;

	if (m_pWndCombo != NULL)
	{
		m_pWndCombo->SetRedraw(FALSE);
	}

	CWindowDC dc(NULL);

	RemoveAllItems();

	m_nLogVert = dc.GetDeviceCaps(LOGPIXELSY);
	::EnumFontFamilies(dc.GetSafeHdc(), strFontName, (FONTENUMPROC) EnumSizeCallBack, (LPARAM) this);

	if (!SelectItem(strText))
	{
		m_strEdit = strText;
		if (m_pWndCombo != NULL)
		{
			m_pWndCombo->SetWindowText(m_strEdit);
		}
	}

	// Synchronize context with other comboboxes with the same ID:
	CObList listButtons;
	if (CMFCToolBar::GetCommandButtons(m_nID, listButtons) > 0)
	{
		for (POSITION posCombo = listButtons.GetHeadPosition(); posCombo != NULL;)
		{
			CMFCToolBarComboBoxButton* pCombo = DYNAMIC_DOWNCAST(CMFCToolBarComboBoxButton, listButtons.GetNext(posCombo));

			if (pCombo != NULL && pCombo != this)
			{
				if (pCombo->GetComboBox() != NULL)
				{
					pCombo->GetComboBox()->SetRedraw(FALSE);
				}

				pCombo->RemoveAllItems();

				POSITION pos;
				POSITION posData;

				for (pos = m_lstItems.GetHeadPosition(), posData = m_lstItemData.GetHeadPosition(); pos != NULL && posData != NULL;)
				{
					pCombo->AddItem(m_lstItems.GetNext(pos), m_lstItemData.GetNext(posData));
				}

				if (pCombo->GetComboBox() != NULL)
				{
					pCombo->GetComboBox()->SetRedraw();
				}
			}
		}
	}

	if (m_pWndCombo != NULL)
	{
		m_pWndCombo->SetRedraw();
	}
}

BOOL FAR PASCAL CMFCToolBarFontSizeComboBox::EnumSizeCallBack(LOGFONT FAR* /*lplf*/, LPNEWTEXTMETRIC lpntm,int FontType, LPVOID lpv)
{
	CMFCToolBarFontSizeComboBox* pThis = (CMFCToolBarFontSizeComboBox*) lpv;
	ASSERT_VALID(pThis);

	if ((FontType & TRUETYPE_FONTTYPE) || !((FontType & TRUETYPE_FONTTYPE) ||(FontType & RASTER_FONTTYPE))) // if truetype or vector font
	{
		// this occurs when there is a truetype and nontruetype version of a font
		for (int i = 0; i < 16; i++)
		{
			CString strSize;
			strSize.Format(_T("%d"), nFontSizes[i]);

			pThis->AddItem(strSize);
		}

		return FALSE; // don't call me again
	}

	// calc character height in pixels
	pThis->InsertSize(MulDiv(lpntm->tmHeight-lpntm->tmInternalLeading, 1440, pThis->m_nLogVert));

	return TRUE; // call me again
}

CString CMFCToolBarFontSizeComboBox::TwipsToPointString(int nTwips)
{
	CString str;
	if (nTwips >= 0)
	{
		// round to nearest half point
		nTwips = (nTwips + 5) / 10;

		if ((nTwips % 2) == 0)
		{
			str.Format(_T("%ld"), nTwips/2);
		}
		else
		{
			str.Format(_T("%.1f"), (float) nTwips / 2.F);
		}
	}

	return str;
}

void CMFCToolBarFontSizeComboBox::SetTwipSize(int nTwips)
{
	SetText(TwipsToPointString(nTwips));
}

int CMFCToolBarFontSizeComboBox::GetTwipSize() const
{
	// return values
	// -2 -- error
	// -1 -- edit box empty
	// >=0 -- font size in twips

	CString str = GetItem() == NULL ? m_strEdit : GetItem();
	if (m_lstItems.Find(m_strEdit) == NULL)
	{
		str = m_strEdit;
	}

	LPCTSTR lpszText = str;

	while (*lpszText == ' ' || *lpszText == '\t')
		lpszText++;

	if (lpszText[0] == NULL)
		return -1; // no text in control

	double d = _tcstod(lpszText, (LPTSTR*)&lpszText);
	while (*lpszText == ' ' || *lpszText == '\t')
		lpszText++;

	if (*lpszText != NULL)
		return -2;   // not terminated properly

	return(d<0.) ? 0 :(int)(d*20.);
}

void CMFCToolBarFontSizeComboBox::InsertSize(int nSize)
{
	ASSERT(nSize > 0);
	AddItem(TwipsToPointString(nSize), (DWORD) nSize);
}

CComboBox* CMFCToolBarFontSizeComboBox::CreateCombo(CWnd* pWndParent, const CRect& rect)
{
	CMFCFontComboBox* pWndCombo = new CMFCFontComboBox;
	if (!pWndCombo->Create(m_dwStyle, rect, pWndParent, m_nID))
	{
		delete pWndCombo;
		return NULL;
	}

	return pWndCombo;
}

void CMFCToolBarFontComboBox::CopyFrom(const CMFCToolBarButton& s)
{
	CMFCToolBarComboBoxButton::CopyFrom(s);

	const CMFCToolBarFontComboBox& src = (const CMFCToolBarFontComboBox&) s;

	m_nCharSet = src.m_nCharSet;
	m_nFontType = src.m_nFontType;
	m_nPitchAndFamily = src.m_nPitchAndFamily;
}


