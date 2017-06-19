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

#include "afxribbonres.h"
#include "afxcontrolbarutil.h"
#include "afxcolordialog.h"
#include "afxdrawmanager.h"
#include "afxstandardcolorspropertypage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFCStandardColorsPropertyPage property page

IMPLEMENT_DYNCREATE(CMFCStandardColorsPropertyPage, CPropertyPage)

CMFCStandardColorsPropertyPage::CMFCStandardColorsPropertyPage() : CPropertyPage(CMFCStandardColorsPropertyPage::IDD)
{
	m_nColorPickerOffset = 4; // the offset is taken from the dialog template. Anyway, it will be adjusted in InitDialog
}

void CMFCStandardColorsPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMFCStandardColorsPropertyPage)
	DDX_Control(pDX, IDC_AFXBARRES_HEXPLACEHOLDER, m_hexpicker);
	DDX_Control(pDX, IDC_AFXBARRES_GREYSCALEPLACEHOLDER, m_hexpicker_greyscale);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMFCStandardColorsPropertyPage, CPropertyPage)
	//{{AFX_MSG_MAP(CMFCStandardColorsPropertyPage)
	ON_BN_CLICKED(IDC_AFXBARRES_GREYSCALEPLACEHOLDER, &CMFCStandardColorsPropertyPage::OnGreyscale)
	ON_BN_CLICKED(IDC_AFXBARRES_HEXPLACEHOLDER, &CMFCStandardColorsPropertyPage::OnHexColor)
	ON_BN_DOUBLECLICKED(IDC_AFXBARRES_GREYSCALEPLACEHOLDER, &CMFCStandardColorsPropertyPage::OnDoubleClickedColor)
	ON_BN_DOUBLECLICKED(IDC_AFXBARRES_HEXPLACEHOLDER, &CMFCStandardColorsPropertyPage::OnDoubleClickedColor)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCStandardColorsPropertyPage message handlers

BOOL CMFCStandardColorsPropertyPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_hexpicker.SetPalette(m_pDialog->GetPalette());
	m_hexpicker.SetType(CMFCColorPickerCtrl::HEX);

	m_hexpicker_greyscale.SetPalette(m_pDialog->GetPalette());
	m_hexpicker_greyscale.SetType(CMFCColorPickerCtrl::HEX_GREYSCALE);

	CRect rectColorPicker;

	m_hexpicker.GetWindowRect (rectColorPicker);
	ScreenToClient (rectColorPicker);

	m_nColorPickerOffset = rectColorPicker.left; // m_hexpicker and m_hexpicker_greyscale have the same offset

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CMFCStandardColorsPropertyPage::OnGreyscale()
{
	double H,L,S;
	m_hexpicker_greyscale.GetHLS(&H,&L,&S);

	COLORREF color = CDrawingManager::HLStoRGB_TWO(H, L, S);

	m_pDialog->SetNewColor(color);

	BYTE R = GetRValue(color);
	BYTE G = GetGValue(color);
	BYTE B = GetBValue(color);

	m_pDialog->SetPageTwo(R, G, B);

	m_hexpicker.SelectCellHexagon(R, G, B);
	m_hexpicker.Invalidate();
}

void CMFCStandardColorsPropertyPage::OnHexColor()
{
	COLORREF color = m_hexpicker.GetColor();

	BYTE R = GetRValue(color);
	BYTE G = GetGValue(color);
	BYTE B = GetBValue(color);

	double H,L,S;
	m_hexpicker.GetHLS(&H,&L,&S);

	// Set actual color.
	m_pDialog->SetNewColor(color);

	m_pDialog->SetPageTwo(R, G, B);

	m_hexpicker_greyscale.SelectCellHexagon(R, G, B);
	m_hexpicker_greyscale.Invalidate();
}

void CMFCStandardColorsPropertyPage::OnDoubleClickedColor()
{
	m_pDialog->EndDialog(IDOK);
}

void CMFCStandardColorsPropertyPage::OnSize(UINT nType, int cx, int cy)
{
	CPropertyPage::OnSize(nType, cx, cy);

	AdjustControlWidth (&m_hexpicker, cx);
	AdjustControlWidth (&m_hexpicker_greyscale, cx);
}
void CMFCStandardColorsPropertyPage::AdjustControlWidth (CMFCColorPickerCtrl* pControl, int cx)
{
	ASSERT_VALID (pControl);
	if (!IsWindow (pControl->GetSafeHwnd ()))
	{
		return;
	}

	CRect rect;  

	pControl->GetWindowRect (rect);
	ScreenToClient (rect);
	pControl->SetWindowPos (NULL, m_nColorPickerOffset, rect.top, cx - m_nColorPickerOffset * 2, rect.Height (), 
							SWP_NOZORDER | SWP_NOACTIVATE);
}
