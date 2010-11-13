// SVPButton.cpp : implementation file
//

#include "stdafx.h"
#include "mplayerc.h"
#include "SVPButton.h"
#include "SVPDialog.h"


// CSVPButton

IMPLEMENT_DYNAMIC(CSVPButton, CButton)

CSVPButton::CSVPButton():
  m_btnMode(0),
  m_fischecked(false)
{
	AppSettings& s = AfxGetAppSettings();
	m_textColor = s.GetColorFromTheme(_T("FloatDialogButtonTextColor"), 0xffffff);
	m_bgColor = s.GetColorFromTheme(_T("FloatDialogButtonBG"), 0x202020);
	m_btnBgColor = s.GetColorFromTheme(_T("FloatDialogButtonColor"), 0x00);
	m_pushedColor = s.GetColorFromTheme(_T("FloatDialogButtonPushedColor"), 0x232323);
	m_borderColor = s.GetColorFromTheme(_T("FloatDialogButtonBorder"), 0xffffff);
	m_textGrayColor = s.GetColorFromTheme(_T("FloatDialogButtonGray"), 0x454545);

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

	
		CSVPDialog * pDialog = dynamic_cast<CSVPDialog*>( GetParent() );
		if(pDialog)
			this->m_bgColor = pDialog->m_bgColor;

	return 0;
}

void CSVPButton::SetCheckStatus(bool checkstatues)
{
  m_fischecked = checkstatues;
}

bool CSVPButton::IsChecked()
{
  return m_fischecked;
}

void CSVPButton::SetButtonMode(int btnmode)
{
  m_btnMode = btnmode;
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
	} else if (lpDrawItemStruct->itemState & (ODS_DISABLED|ODS_GRAYED)){
		textColor = m_textGrayColor;
	}

  //Draw the button frame.
  //  ::DrawFrameControl(lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem, 
  //  DFC_BUTTON, uStyle);

  if (m_btnMode != 2)
  {
    CRgn mRgn;
    mRgn.CreateRoundRectRgn(lpDrawItemStruct->rcItem.left , lpDrawItemStruct->rcItem.top
      ,lpDrawItemStruct->rcItem.right , lpDrawItemStruct->rcItem.bottom , 3,3);
    CBrush brush;
    brush.CreateSolidBrush(buttonBG);

    CBrush brushBorder;
    brushBorder.CreateSolidBrush(m_borderColor);

    ::FillRgn( lpDrawItemStruct->hDC, mRgn  , brush);
    ::FrameRgn(  lpDrawItemStruct->hDC, mRgn , brushBorder , 1, 1);
  }

	// Get the button's text.
	CString strText;
	GetWindowText(strText);

	switch(m_btnMode)
  {
		case 1:
			//x
			{
				CRect xRect(lpDrawItemStruct->rcItem);
				CPoint center = xRect.CenterPoint();
				center.Offset(-1,-1);
				HPEN line = CreatePen(PS_INSIDEFRAME, 1, textColor);
				HPEN old = (HPEN)::SelectObject(lpDrawItemStruct->hDC, (HPEN)line);
				::MoveToEx(lpDrawItemStruct->hDC, center.x -2 , center.y - 2, NULL);
				::LineTo( lpDrawItemStruct->hDC,center.x +3 , center.y + 3);
				::MoveToEx(lpDrawItemStruct->hDC,  center.x +2 , center.y - 2, NULL);
				::LineTo( lpDrawItemStruct->hDC, center.x -3 , center.y + 3);
				::SelectObject( lpDrawItemStruct->hDC,  old);
				DeleteObject(line);

			}
			break;

    case 2:
      {
        WTL::CFont   font;
        wchar_t boxstr[2];
        font.CreateFont(12, 0, 0, 0, FW_NORMAL, 0, 0, 0,
          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
          CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
          DEFAULT_PITCH|FF_SWISS, L"Marlett");

        WTL::CDCHandle dc(lpDrawItemStruct->hDC);
        HFONT    pOldFont   = dc.SelectFont(font);
        COLORREF crOldColor = ::SetTextColor(lpDrawItemStruct->hDC, textColor);
        ::SetBkMode(lpDrawItemStruct->hDC, TRANSPARENT);
        CRect textRc(lpDrawItemStruct->rcItem);
        //Draw the small box of a checkbox
        swprintf_s(boxstr, 2, L"%c", 0x65);
        ::DrawText(lpDrawItemStruct->hDC, boxstr, 1, 
          &textRc, DT_SINGLELINE|DT_VCENTER|DT_LEFT);
        swprintf_s(boxstr, 2, L"%c", 0x66);
        ::DrawText(lpDrawItemStruct->hDC, boxstr, 1, 
          &textRc, DT_SINGLELINE|DT_VCENTER|DT_LEFT);
        if (m_fischecked)
        {
          //if is checked, draw the tick inside the box
          swprintf_s(boxstr, 2, L"%c", 0x61);
          ::DrawText(lpDrawItemStruct->hDC, boxstr, 1,
          &textRc, DT_SINGLELINE|DT_VCENTER|DT_LEFT);
        }
        dc.SelectFont(pOldFont);
        DeleteObject(font);
        textRc.left += 18;
        textRc.bottom -= 2;
        //Draw the text
        ::DrawText(lpDrawItemStruct->hDC, strText, strText.GetLength(),
          &textRc, DT_SINGLELINE|DT_VCENTER|DT_LEFT);
        ::SetTextColor(lpDrawItemStruct->hDC, crOldColor);
        
        //int boxlength = 12;
        //int boxtop = (lpDrawItemStruct->rcItem.bottom - lpDrawItemStruct->rcItem.top - boxlength) / 2;
        //lpDrawItemStruct->rcItem.right += boxlength;
        //CRgn mRgnBox;
        //mRgnBox.CreateRoundRectRgn(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top + boxtop
        //  ,lpDrawItemStruct->rcItem.left + boxlength , lpDrawItemStruct->rcItem.top + boxtop + boxlength , 3, 3);
        //CBrush brush;
        //brush.CreateSolidBrush(buttonBG);

        //CBrush brushBorder;
        //brushBorder.CreateSolidBrush(m_borderColor);

        //::FillRgn(lpDrawItemStruct->hDC, mRgnBox, brush);
        //::FrameRgn(lpDrawItemStruct->hDC, mRgnBox, brushBorder, 1, 1);

        //if (m_fischecked)
        //{
        //  HPEN line = CreatePen(PS_INSIDEFRAME, 2, textColor);
        //  HPEN old = (HPEN)::SelectObject(lpDrawItemStruct->hDC, (HPEN)line);
        //  ::MoveToEx(lpDrawItemStruct->hDC, lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top + boxtop + 5, NULL);
        //  ::LineTo(lpDrawItemStruct->hDC, lpDrawItemStruct->rcItem.left + 4 , lpDrawItemStruct->rcItem.top + boxtop + 9);
        //  ::MoveToEx(lpDrawItemStruct->hDC, lpDrawItemStruct->rcItem.left + 4 , lpDrawItemStruct->rcItem.top + boxtop + 9, NULL);
        //  ::LineTo(lpDrawItemStruct->hDC, lpDrawItemStruct->rcItem.left + 12, lpDrawItemStruct->rcItem.top + boxtop + 1);
        //  ::SelectObject(lpDrawItemStruct->hDC, old);
        //  DeleteObject(line);
        //}
      }
      break;

		default:
			{
				COLORREF crOldColor = ::SetTextColor(lpDrawItemStruct->hDC, textColor);
				::SetBkMode( lpDrawItemStruct->hDC, TRANSPARENT);
				CRect textRc(lpDrawItemStruct->rcItem);
				textRc.bottom -= 2;
				::DrawText(lpDrawItemStruct->hDC, strText, strText.GetLength(), 
					&textRc, DT_SINGLELINE|DT_VCENTER|DT_CENTER);
				::SetTextColor(lpDrawItemStruct->hDC, crOldColor);
			}
			break;
	}
	// Draw the button text using the text color red.
	
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
