// SVPButton.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "SVPButton.h"


// CSVPButton

IMPLEMENT_DYNAMIC(CSVPButton, CButton)

CSVPButton::CSVPButton():
m_textColor(0xffffff)
,m_btnBgColor(0)
,m_bgColor(0)
, m_pushedColor(0x232323)
, m_borderColor(0xffffff)
, m_textGrayColor(0x787878)
{

}

CSVPButton::~CSVPButton()
{
}


BEGIN_MESSAGE_MAP(CSVPButton, CButton)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()



// CSVPButton message handlers



int CSVPButton::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CButton::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	return 0;
}

void CSVPButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{

	UINT uStyle = DFCS_BUTTONPUSH;

	// This code only works with buttons.
	ASSERT(lpDrawItemStruct->CtlType == ODT_BUTTON);

	// If drawing selected, add the pushed style to DrawFrameControl.
	DWORD buttonBG = m_btnBgColor;
	DWORD textColor = m_textColor;
	if (lpDrawItemStruct->itemState & ODS_SELECTED){
		uStyle |= DFCS_PUSHED;
		buttonBG = m_pushedColor;
	}if (lpDrawItemStruct->itemState & (ODS_DISABLED|ODS_GRAYED)){
		textColor = m_textGrayColor;
	}

	// Draw the button frame.
	//::DrawFrameControl(lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem, 
	//	DFC_BUTTON, uStyle);

	CRgn mRgn;
	mRgn.CreateRoundRectRgn(lpDrawItemStruct->rcItem.left , lpDrawItemStruct->rcItem.top
		,lpDrawItemStruct->rcItem.right , lpDrawItemStruct->rcItem.bottom , 3,3);
	CBrush brush;
	brush.CreateSolidBrush(buttonBG);

	CBrush brushBorder;
	brushBorder.CreateSolidBrush(m_borderColor);

	::FillRgn( lpDrawItemStruct->hDC, mRgn  , brush);
	::FrameRgn(  lpDrawItemStruct->hDC, mRgn , brushBorder , 1, 1);


	// Get the button's text.
	CString strText;
	GetWindowText(strText);

	// Draw the button text using the text color red.
	COLORREF crOldColor = ::SetTextColor(lpDrawItemStruct->hDC, textColor);
	::SetBkMode( lpDrawItemStruct->hDC, TRANSPARENT);
	CRect textRc(lpDrawItemStruct->rcItem);
	textRc.bottom -=2;
	::DrawText(lpDrawItemStruct->hDC, strText, strText.GetLength(), 
		&textRc, DT_SINGLELINE|DT_VCENTER|DT_CENTER);
	::SetTextColor(lpDrawItemStruct->hDC, crOldColor);

}

void CSVPButton::PreSubclassWindow()
{
	// TODO: Add your specialized code here and/or call the base class

	CButton::PreSubclassWindow();

	 ModifyStyle(0, BS_OWNERDRAW);
}

BOOL CSVPButton::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	CRect r;
	GetClientRect(&r);
	pDC->FillSolidRect(&r, m_bgColor);
	return TRUE;
}
