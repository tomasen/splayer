// SVPStatic.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "SVPStatic.h"
#include "SVPDialog.h"

// CSVPStatic

IMPLEMENT_DYNAMIC(CSVPStatic, CStatic)

CSVPStatic::CSVPStatic()
: m_dwAlign(DT_LEFT)
{
	AppSettings& s = AfxGetAppSettings();
	m_textColor = s.GetColorFromTheme(_T("FloatDialogButtonTextColor"), 0xffffff);
	m_bgColor = s.GetColorFromTheme(_T("FloatDialogButtonBG"), 0x00);

}

CSVPStatic::~CSVPStatic()
{
}


BEGIN_MESSAGE_MAP(CSVPStatic, CStatic)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()



// CSVPStatic message handlers



int CSVPStatic::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	
	if (CStatic::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	CSVPDialog * pDialog = dynamic_cast<CSVPDialog*>( GetParent() );
	if(pDialog)
		m_bgColor = pDialog->m_bgColor;

	return 0;
}

void CSVPStatic::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{

	// This code only works with buttons.
	ASSERT(lpDrawItemStruct->CtlType == ODT_STATIC);

	// If drawing selected, add the pushed style to DrawFrameControl.
	DWORD buttonBG = m_bgColor;
	

	CBrush brush;
	brush.CreateSolidBrush(m_bgColor);
	::FillRect( lpDrawItemStruct->hDC ,&lpDrawItemStruct->rcItem , brush);
	CString strText;
	GetWindowText(strText);

	// Draw the button text using the text color red.
	COLORREF crOldColor = ::SetTextColor(lpDrawItemStruct->hDC, m_textColor);
	::SetBkMode( lpDrawItemStruct->hDC, TRANSPARENT);
	::DrawText(lpDrawItemStruct->hDC, strText, strText.GetLength(), 
		&lpDrawItemStruct->rcItem, DT_SINGLELINE|DT_VCENTER|m_dwAlign);
	::SetTextColor(lpDrawItemStruct->hDC, crOldColor);

}

void CSVPStatic::PreSubclassWindow()
{
	// TODO: Add your specialized code here and/or call the base class

	CStatic::PreSubclassWindow();

	 ModifyStyle(0, SS_OWNERDRAW);
}

BOOL CSVPStatic::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	CRect r;
	GetClientRect(&r);
	pDC->FillSolidRect(&r, m_bgColor);
	return TRUE;
}
