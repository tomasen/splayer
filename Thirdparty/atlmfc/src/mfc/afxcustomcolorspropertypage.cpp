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
#include "afxcolordialog.h"
#include "afxdrawmanager.h"
#include "afxribbonres.h"
#include "afxcustomcolorspropertypage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static double __stdcall Int2HLS(UINT n)
{
	return min(1., (double)(.5 + n) / 255.);
}

static UINT __stdcall HLS2Int(double n)
{
	return min(255, (UINT)(.5 + n * 255.));
}

/////////////////////////////////////////////////////////////////////////////
// CMFCCustomColorsPropertyPage property page

IMPLEMENT_DYNCREATE(CMFCCustomColorsPropertyPage, CPropertyPage)

CMFCCustomColorsPropertyPage::CMFCCustomColorsPropertyPage() : CPropertyPage(CMFCCustomColorsPropertyPage::IDD)
{
	//{{AFX_DATA_INIT(CMFCCustomColorsPropertyPage)
	m_r = 0;
	m_b = 0;
	m_g = 0;
	m_l = 0;
	m_h = 0;
	m_s = 0;
	//}}AFX_DATA_INIT

	m_pDialog = NULL;
	m_bIsReady = FALSE;
	m_bInUpdate = FALSE;
}

void CMFCCustomColorsPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMFCCustomColorsPropertyPage)
	DDX_Control(pDX, IDC_AFXBARRES_LUMINANCEPLACEHOLDER, m_wndLuminance);
	DDX_Control(pDX, IDC_AFXBARRES_COLOURPLACEHOLDER, m_wndColorPicker);
	DDX_Text(pDX, IDC_AFXBARRES_R, m_r);
	DDX_Text(pDX, IDC_AFXBARRES_B, m_b);
	DDX_Text(pDX, IDC_AFXBARRES_G, m_g);
	DDX_Text(pDX, IDC_AFXBARRES_L, m_l);
	DDX_Text(pDX, IDC_AFXBARRES_H, m_h);
	DDX_Text(pDX, IDC_AFXBARRES_S, m_s);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMFCCustomColorsPropertyPage, CPropertyPage)
	//{{AFX_MSG_MAP(CMFCCustomColorsPropertyPage)
	ON_EN_CHANGE(IDC_AFXBARRES_B, &CMFCCustomColorsPropertyPage::OnRGBChanged)
	ON_EN_CHANGE(IDC_AFXBARRES_H, &CMFCCustomColorsPropertyPage::OnHLSChanged)
	ON_EN_CHANGE(IDC_AFXBARRES_G, &CMFCCustomColorsPropertyPage::OnRGBChanged)
	ON_EN_CHANGE(IDC_AFXBARRES_R, &CMFCCustomColorsPropertyPage::OnRGBChanged)
	ON_EN_CHANGE(IDC_AFXBARRES_L, &CMFCCustomColorsPropertyPage::OnHLSChanged)
	ON_EN_CHANGE(IDC_AFXBARRES_S, &CMFCCustomColorsPropertyPage::OnHLSChanged)
	ON_BN_CLICKED(IDC_AFXBARRES_LUMINANCEPLACEHOLDER, &CMFCCustomColorsPropertyPage::OnLuminance)
	ON_BN_CLICKED(IDC_AFXBARRES_COLOURPLACEHOLDER, &CMFCCustomColorsPropertyPage::OnColour)
	ON_BN_DOUBLECLICKED(IDC_AFXBARRES_COLOURPLACEHOLDER, &CMFCCustomColorsPropertyPage::OnDoubleClickedColor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMFCCustomColorsPropertyPage message handlers

BOOL CMFCCustomColorsPropertyPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_wndColorPicker.SetPalette(m_pDialog->GetPalette());
	m_wndColorPicker.SetType(CMFCColorPickerCtrl::PICKER);

	double hue, luminance, saturation;

	m_wndColorPicker.GetHLS(&hue, &luminance, &saturation);

	m_wndLuminance.SetPalette(m_pDialog->GetPalette());
	m_wndLuminance.SetType(CMFCColorPickerCtrl::LUMINANCE);
	m_wndLuminance.SetHLS(hue, luminance, saturation);
	m_wndLuminance.SetLuminanceBarWidth(14);

	// Initialize spin controls:
	for (UINT uiID = IDC_AFXBARRES_SPIN1; uiID <= IDC_AFXBARRES_SPIN6; uiID++)
	{
		CSpinButtonCtrl* pWnd = (CSpinButtonCtrl*) GetDlgItem(uiID);
		if (pWnd == NULL)
		{
			VERIFY(FALSE);
			break;
		}

		pWnd->SetRange(0, 255);
	}

	m_bIsReady = TRUE;
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CMFCCustomColorsPropertyPage::OnRGBChanged()
{
	if (m_bInUpdate || !m_bIsReady)
	{
		return;
	}

	CString str;

	GetDlgItemText(IDC_AFXBARRES_R, str);
	if (str.IsEmpty())
	{
		return;
	}

	GetDlgItemText(IDC_AFXBARRES_G, str);
	if (str.IsEmpty())
	{
		return;
	}

	GetDlgItemText(IDC_AFXBARRES_B, str);
	if (str.IsEmpty())
	{
		return;
	}

	if (!UpdateData())
	{
		return;
	}

	m_bInUpdate = TRUE;
	COLORREF color = RGB(m_r, m_g, m_b);

	m_r = min(m_r, 255);
	m_g = min(m_g, 255);
	m_b = min(m_b, 255);

	m_pDialog->SetNewColor(color);
	m_pDialog->SetPageOne((BYTE) m_r, (BYTE) m_g, (BYTE) m_b);

	double hue;
	double luminance;
	double saturation;
	CDrawingManager::RGBtoHSL(color, &hue, &saturation, &luminance);

	m_h = HLS2Int(hue);
	m_l = HLS2Int(luminance);
	m_s = HLS2Int(saturation);

	UpdateData(FALSE);

	m_wndColorPicker.SetHLS(hue, luminance, saturation, TRUE);
	m_wndLuminance.SetHLS(hue, luminance, saturation, TRUE);

	m_bInUpdate = FALSE;
}

void CMFCCustomColorsPropertyPage::OnHLSChanged()
{
	if (m_bInUpdate || !m_bIsReady)
	{
		return;
	}

	CString str;

	GetDlgItemText(IDC_AFXBARRES_H, str);
	if (str.IsEmpty())
	{
		return;
	}

	GetDlgItemText(IDC_AFXBARRES_L, str);
	if (str.IsEmpty())
	{
		return;
	}

	GetDlgItemText(IDC_AFXBARRES_S, str);
	if (str.IsEmpty())
	{
		return;
	}

	if (!UpdateData())
	{
		return;
	}

	m_bInUpdate = TRUE;

	m_h = min(m_h, 255);
	m_s = min(m_s, 255);
	m_l = min(m_l, 255);

	double dblH = Int2HLS(m_h);
	double dblS = Int2HLS(m_s);
	double dblL = Int2HLS(m_l);

	COLORREF color = CDrawingManager::HLStoRGB_ONE(dblH, dblL, dblS);

	m_r = GetRValue(color);
	m_g = GetGValue(color);
	m_b = GetBValue(color);

	UpdateData(FALSE);

	m_pDialog->SetNewColor(color);
	m_pDialog->SetPageOne((BYTE) m_r, (BYTE) m_g, (BYTE) m_b);

	m_wndColorPicker.SetHLS(dblH, dblL, dblS, TRUE);
	m_wndLuminance.SetHLS(dblH, dblL, dblS, TRUE);

	m_bInUpdate = FALSE;
}

void CMFCCustomColorsPropertyPage::Setup(BYTE R, BYTE G, BYTE B)
{
	double hue;
	double luminance;
	double saturation;
	CDrawingManager::RGBtoHSL((COLORREF)RGB(R, G, B), &hue, &saturation, &luminance);

	m_wndColorPicker.SetHLS(hue, luminance, saturation);
	m_wndLuminance.SetHLS(hue, luminance, saturation);

	m_r = R;
	m_g = G;
	m_b = B;

	m_h = HLS2Int(hue);
	m_l = HLS2Int(luminance);
	m_s = HLS2Int(saturation);

	if (GetSafeHwnd() != NULL)
	{
		UpdateData(FALSE);
	}
}

void CMFCCustomColorsPropertyPage::OnLuminance()
{
	m_bInUpdate = TRUE;

	double luminance = m_wndLuminance.GetLuminance();
	m_wndColorPicker.SetLuminance(luminance);

	double H,L,S;
	m_wndColorPicker.GetHLS(&H,&L,&S);
	m_h = HLS2Int(H);
	m_l = HLS2Int(L);
	m_s = HLS2Int(S);

	COLORREF color = CDrawingManager::HLStoRGB_ONE(H, L, S);

	m_pDialog->SetNewColor(color);

	m_r = GetRValue(color);
	m_g = GetGValue(color);
	m_b = GetBValue(color);

	m_pDialog->SetPageOne((BYTE) m_r, (BYTE) m_g, (BYTE) m_b);

	UpdateData(FALSE);
	m_bInUpdate = FALSE;
}

void CMFCCustomColorsPropertyPage::OnColour()
{
	m_bInUpdate = TRUE;
	COLORREF ref = m_wndColorPicker.GetColor();

	m_r = GetRValue(ref);
	m_g = GetGValue(ref);
	m_b = GetBValue(ref);

	double saturation = m_wndColorPicker.GetSaturation();
	double hue = m_wndColorPicker.GetHue();

	m_wndLuminance.SetHue(hue);
	m_wndLuminance.SetSaturation(saturation);
	m_wndLuminance.Invalidate();

	double H,L,S;
	m_wndColorPicker.GetHLS(&H,&L,&S);
	m_h = HLS2Int(H);
	m_l = HLS2Int(L);
	m_s = HLS2Int(S);

	// Set actual color.
	m_pDialog->SetNewColor(CDrawingManager::HLStoRGB_ONE(H, L, S));
	m_pDialog->SetPageOne((BYTE) m_r, (BYTE) m_g, (BYTE) m_b);

	UpdateData(FALSE);
	m_bInUpdate = FALSE;
}

void CMFCCustomColorsPropertyPage::OnDoubleClickedColor()
{
	m_pDialog->EndDialog(IDOK);
}


